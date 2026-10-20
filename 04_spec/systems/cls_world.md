# World node and its cell grid

Subsystem `cls_world` (registry: membership **CLOSED** 2026-09-15; boundary evidence in
`03_re/ledger/subsystem_registry.csv`). `zClass/cls_world.c`, span `0x00450030..0x004518a0`.

## The grid
The world node (class 2) keeps its children in a 2D grid of 0x40-byte **cells** over XZ:
- row `z` is `[data+0x80][z]`; cell `x` is `row + x*0x40`;
- grid size is `data+0x78` (x) by `data+0x7c` (z);
- a cell holds its children in an array at `+0x3c` with a short count at `+0x3a`;
- a node records its cell at `+0x4c` (x) and `+0x50` (z); (-1, -1) means "not in a cell".

The scene_render culling pass reads the same cells (`scene_render.md`).

## Confirmed (bytes)
- **Attach `0x004510e0`**: finds the child's cell from its XZ extent (its box corners; a child
  without a box counts as the whole world). Children flagged `0x80` get no cell.
- **Detach `0x00451240`**: removes the child from its cell and the world from the child's parents.
  A child with no cell takes Class.c's generic detach.
- **Destroy `0x00450240`**: frees the grid arrays and deletes the node, if `0x00450e40` agrees.
- **Insert `0x00450f60`**: appends the child to the cell's array. With no cell, or a cell already
  holding 0x7fff children, it goes on the world node's own child list instead. The world becomes
  one of the child's parents; a child that gains a second parent has flag `0x80000` cleared in its
  subtree.
- `0x00450a70` refreshes a cell through `0x00450030` (not read yet) unless the cell's flag bit 0 is
  set. `0x00450a00` returns a cell pointer. `0x00450f20` stores a byte "partition max" (at most 255).

## Which cell a node goes in: `0x00450840` - CONFIRMED (`VERIFIED-ORACLE`, 12/12 in two traces)
A node lives in **the cell containing the centre of its XZ box**, and only if the box overhangs that
cell by no more than a margin on every side. Otherwise it stays on the world node's own child
list, which is the same for boxes outside the world. The index is
`trunc((centre - origin) * inverse cell size)`, clamped to the grid.

Measured world (Recoil18/21), `MEASURED`: origin x 0, z 4352 running downward, **256-unit cells**,
grid 16 x 17, margins 3 units. Twelve real lookups were predicted exactly. All twelve were
accepts, so the two rejections come from the bytes only.
- **Lights and sound emitters are registered with the world** in lists of their own:
  - lights at `data+0x90` (count), `+0x94` (node array) and `+0x98` (a parallel array);
  - sounds at `+0x9c`, `+0xa0` and `+0xa4`.

  Each light and emitter keeps the matching back-list. `0x00451410` (light) and `0x00451640`
  (sound) remove one from both sides (CONFIRMED, bytes).

## World data layout (from the confirmed setters and getters)
| offset | meaning | set by |
|---|---|---|
| `+0x00` | dirty bits, one per parameter block below | the setters |
| `+0x10` | parameter (bit 1) | `0x00450ae0` |
| `+0x14..+0x1c` | three parameters (bit 2) | `0x00450af0` |
| `+0x20`, `+0x24` | two parameters (bit 4) | `0x00450b40` |
| `+0x28`, `+0x2c` | two parameters (bit 0x20); the setter stores its 2nd argument first | `0x00450b20` |
| `+0x30` | parameter (bit 8) | `0x00450b60` |
| `+0x34`, `+0x38` | grid origin (x, z) | `0x00450c00` |
| `+0x3c`, `+0x40` | grid extent (x, z); z is negative | `0x00450c30` |
| `+0x44`, `+0x48` | far corner = origin + extent (kept in sync) | both of the above |
| `+0x4c` | partition max (byte, at most 255) | `0x00450f20` |
| `+0x50` | non-zero re-buckets the border cells | `0x00450510` |
| `+0x70`, `+0x74` | cell overhang margins | `0x00450f00` |
| `+0x78`, `+0x7c`, `+0x80` | grid size and row table | - |
| `+0x90..+0x98` / `+0x9c..+0xa4` | light / sound registrations | - |

The meaning of the parameter blocks at `+0x10..+0x30` is not yet known; only their storage is.

Per frame the world passes its lights to the light-slot selector (`0x00451540`, which hands the
node array, data array and count to `0x00487a30`) and moves each light to view space
(`0x00451560`).

## Creation, dirty cells, teardown (CONFIRMED, bytes)
- **`World_Create` `0x004501c0`** makes a class-2 node with 0xac bytes of data and registers it
  on **list 13, the world list**. Partition max starts at 16, and `+0x84..+0x8c` start at 1.0.
- **Dirty cells** (`0x00450030`): a changed cell is appended once, marked by its flag bit 0, to a
  list in the world data (`+4` count, `+8` capacity, `+0xc` array). That list grows **one entry per
  realloc**. The world node is then queued on list 7 for its update.
- **`0x00450e40`** frees every cell's child array and the rows, unless `data+0xa8` says the rows
  are owned elsewhere.
- Lights and sound emitters are added by `0x00451360` / `0x00451590` and removed by
  `0x00451410` / `0x00451640`, always on both sides.

## Per-frame world update, cell bounds, map edge (CONFIRMED, bytes)
- **`World_Update` `0x00450530`** pushes each dirty parameter block to the render state (setters
  `0x00476170..0x004762f0`, not read). If there are several worlds, it pushes all of them every
  frame. It then refreshes every queued dirty cell.
- **Cell bounds `0x004500b0`**: a cell's box keeps its fixed XZ square and takes its **Y range from
  its children's corners**. Its bounding sphere (`+0x28` centre, `+0x34` radius) is what culling
  tests. QUIRK: the search for the first child with a valid box steps through memory 0xC0 bytes
  at a time instead of walking the child array, so it is correct only when the first child
  already qualifies.
- **Map edge `0x004502b0`**: all children of each border cell are regrouped under a new node named
  `VAP_statics`. That is how the edge terrain the culling pass repeats beyond the map comes to
  exist.
- **Save records `0x004517a0`**: each world contributes its nine parameter dwords (0x24 bytes).

## Building the grid - `0x00450c60` (CONFIRMED, bytes)
The game calls the grid the **virtual area partition** (its own error string), hence `VAP_statics`.
- It sets the cell size, derives the margins (1/8 of a cell; a later `World_SetCellMargins` call
  sets the 3 units measured in the traces), half cell and inverse size, and a field at `+0x6c` =
  -0.5 x fast sqrt of the squared cell diagonal (**stored negative**, kept as found).
- The cell count per axis is `trunc(extent / cell)`, rounded up to cover the extent.
- Each 0x40-byte cell gets its XZ square, a box whose Y range children fill in later, and a
  bounding sphere.

## Lights per frame (CONFIRMED, bytes)
- **Selection `0x00487a30`**: up to **64 active lights** (node flag 4) per frame go into a slot
  table; more is an error ("Not enough MAX_LIGHTS"). The last selected light flagged at data `+0xbc`
  is remembered. `INFERRED`: that flag marks a directional (sun) light. On the software path its
  colour (`+0xac..`) and `1 - data+0xa8` feed the lighting constants; with no such light, the
  ambient comes from the render colour.
- **`Light_TransformToViewSpace` `0x00453880`**: moves each active light into view space. It
  rotates and negates the direction (`+0x8c` → `+0x98`) and transforms the position (`+0x2c` →
  `+0x80`).
- Cell spheres come from `0x004525d0`: centre = box midpoint, radius = **fast sqrt** of the squared
  half-diagonal.

## Function index additions (2026-09-24)
- **World data getters.** `World_GetParam30` `0x00450b80`, `World_GetParam10` `0x00450b90`,
  `World_GetParams14` `0x00450ba0`, `World_GetParams20` `0x00450bc0`, `World_GetParams28`
  `0x00450be0`.
- **Sound emitters.** `Sound_ProcessWorldNodeEmitters` `0x00451770` runs the world's sound
  emitters (`VERIFIED-ORACLE`: two emitters in the trace).

