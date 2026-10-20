# Scene-graph render traversal

Subsystem `scene_render` (19 members, membership CLOSED): the per-frame walk of the scene graph,
from `Camera.c`. Entered from the frame drivers `Render_CameraFrame_SW/HW` through `0x0044a9f0`
and `0x0044d240`.

## Node class types - `CONFIRMED-BINARY`
From `Render_DispatchNodeByType`'s jump table at `0x0044c184`, read from `Recoil.exe`
(index = class - 1):

| class (`node+0x34`) | handler |
|---|---|
| 1 | camera - `0x0044ada0` |
| 2, 3, 4 | none - falls to the default |
| 5 | object (3-D model) - `0x0044b300` |
| 6 | LOD / distance - `0x0044b8c0` |
| 7 | group - `0x0044bea0` |
| 8 | `0x0044b710` |
| 9 | light - `0x0044b140` |
| 10 | sound - `0x0044af60` |
| 11 | child bitmask - `0x0044bfb0` |
| 0 or > 11 | default |

The default writes `"Unrecognized node rendering type"` with `fprintf` to `_iob + 0x40` - **stderr**,
which a Windows GUI program has nowhere to display. Unlike `0x00404e80` this is a real call, but it
has no visible effect either.

Elsewhere: class 2 is what a camera links to (probably the world root, `INFERRED`); class 3 is
checked for by a `Window` function.

## Dispatch - `0x0044c0e0`
```
if (!render_filter(node->type_byte /* +0x30 */)) return;
switch (node->class /* +0x34 */) { case ...: handler(ECX = node, EDX = mode); }
```
The mode in `EDX` is forwarded unchanged. `0x0044d200` dispatches each child with `EDX = 2`.

## Render filter - `0x00476400` (`VERIFIED-ORACLE`)
A node is drawn if any of these hold:
- filtering is off: `[0x004dd90c] == 0`;
- its type byte is `0xff` (the default for a new node);
- the filter record at `0x0057da28` is empty (count byte 0);
- its type, or `0xff`, appears among the record's first `count` type bytes.

The record format - one count byte then type bytes, `0xff` meaning "any", 4 bytes in all - is the
same one `0x00476320` initialises (`00 ff ff ff`) and the one each camera keeps at `+0x1e4`
(`0x0044d260`). **Measured in `Recoil18`:** filtering is on and the record is `01 00 ff ff` (type 0
only); nodes of type 2 are rejected (four calls, all returned 0).

## Diamond angle - `0x0044c1b0` (a math approximation a remake must copy)
For points `a` (`ECX`) and `b` (`EDX`):
```
dx = trunc(b.x - a.x);  dz = trunc(a.z - b.z)        // CRT _ftol: truncate to integers FIRST
s  = |dx| + |dz|;       t = s ? dz / s : 0.0
if (dx < 0)       angle = (2.0 - t) * 1.570796
else if (dz < 0)  angle = (t + 4.0) * 1.570796       // code subtracts -4.0
else              angle = t * 1.570796
```
A piecewise-linear stand-in for `atan2` over `[0, 2 pi)`. Two details matter: the deltas are
truncated to whole units before use, and the scale is the float literal `1.570796`
(`0x004d2350`), slightly short of pi/2. Used by `0x0044c230` to order points. Never runs in either
trace.

## Frame entry and child loop
- `0x0044d240(node, p)`: `Render_CullAndQueueSceneObjects(node, p)`, then `0x0044d200(node)`.
- `0x0044d200(node)`: `*[0x0057da24] := 0x3f` (measured 0x21 -> 0x3f), then dispatch every child with
  mode 2.
- `0x0044a9f0(node)`: validate, then `Camera_UpdateFrustumAndTransform(node, 0)`.

## Per-frame camera update
- **`0x0044aa30` `Camera_UpdateFrustumAndTransform`** (`VERIFIED-ORACLE`): rebuilds the camera's
  world/view state (`0x0044abf0`), then - only if the dirty flag `+0xf8` is set - the far-plane
  corners `(+-tan(h/2).far, +-tan(v/2).far, -far)` at `+0x108..+0x134`, and every frame the near-
  and far-plane centres `pos + fwd.near` (`+0xb8`) and `pos + fwd.far` (`+0xc4`). Both centres
  measured at return in `Recoil18`, to float32 rounding.
- **`0x0044abf0`** (`VERIFIED-ORACLE` for its matrix outputs): identity, then walk the parent chain
  (mode 1), copy the result into the camera (`+0x2c` position, `+0x44` matrix, `+0x74` forward =
  `-row2`), extract world Euler angles into `+0x38`, and build the view/camera matrices. If camera
  flag bit 0 is set it also computes **velocity = (position - previous) / `[0x0056b424]`**, zeroed
  when its length reaches the limit from `0x004a2e70`, and passes it to `0x004a2950`. `INFERRED`:
  that is the **sound listener velocity for Doppler** - both callees live in the sound code, and
  world units are metres (Doppler constant 1/345).

## Hierarchical frustum culling
**Plane-mask stack.** `0x0057da24` points into an array of plane masks (base `0x0057d9e4`). The
frame starts with **`0x3f`** = all six planes to test. A node that passes pushes its *reduced* mask
(only the planes it straddles), dispatches, then pops - so children never retest planes their
parent was already fully inside.

**Planes** live in the active camera's class data, reached through `[0x00576214]` (= `0x08726b08`,
the camera, measured in `Recoil18`):

| mask bit | plane | point | normal |
|---|---|---|---|
| `0x01` `0x02` `0x04` `0x08` | four sides | camera position `+0x2c` | `+0x188`, `+0x194`, `+0x1a0`, `+0x1ac` |
| `0x10` | near | near-plane centre `+0xb8` | `+0x1b8` |
| `0x20` | far | far-plane centre `+0xc4` | `+0x1c4` |

**Sphere test `0x00478c70`.** For each plane bit set in the incoming mask, `d = (centre - point) . normal`:
`d <= -r` returns that bit at once (**fully outside, culled**); `d < r` keeps the bit in the output
mask (**straddling**); otherwise the bit is dropped (**fully inside**). Returns 0 when not culled.

**Bounds.** Sphere centre at `node+0x64`, radius at `node+0x70`, rebuilt when `node+0x2c` bit 4 is
set (or globally when `[0x004ddd28]` is set) from eight box corners (`0x00448920`) by
`0x00452650`: centre = midpoint of the corners' min and max; **radius = fast square root**
`(bits(h.h) >> 1) + 0x1fc00000` of the squared half-diagonal - the approximation, not `sqrt()`.

**Culling is only attempted when the mode in `EDX` exceeds 1.** The frame's child loop passes 2.
The class-7 and class-11 handlers pass **the parent's child count (`node+0x5c`)**, loaded
explicitly before each child dispatch. Read from bytes; neither handler runs in `Recoil18` or
`Recoil21`. `INFERRED` consequence: a child of a node with a single child would skip sphere culling.
A remake should reproduce the argument as the bytes pass it.

**Class 7 ("group")** dispatches one child selected from a table in its class data
(`data + 0x20 + 8 * data[+0x14]`). **Class 11** dispatches each child whose bit is set in
`data[data[0] + 2]`.

**Verified on real calls (`Recoil18`):** three sphere tests reproduce plane by plane (return 0,
output masks `0x22`, `0x02`, `0x24`); the culled early return was not hit. The bounding-sphere
radius is **bit-exact with the fast square root** - 5.562306 measured, 5.562306 predicted, where a
true square root gives 5.338394. **Bounding radii in the shipped game are inflated by the
approximation (~4% here); a remake using `sqrt()` would cull differently.**

## Box corners - `0x00448920` (`VERIFIED-ORACLE`)
Corners = the node's stored box (`node+0x74..+0x88`, min then max) transformed by
**class-local matrix composed with the current stack matrix**, with shortcuts: identity level and no
local matrix -> the box copied untransformed; identity level -> local matrix only; no local matrix ->
current matrix only. Per class (jump table `0x00448c94`, read from the exe): object (5) uses
`data+0x30` unless `data[0]` bit 3 is set; camera (1) and class 8 have their own; 2, 6, 7 none;
3 and 4 return 3. Requires `node+0x24` bit `0x100`.

**Decompiler omission, fourth of the session.** The local-with-current concatenation is inlined at
`0x00448ae0`-`0x00448c7a` and does not appear in Ghidra's output at all, which shows the box function
called with the wrong inputs. Verified on a real object node: the matrix actually passed equals
local composed with current to 3.3e-5; the reverse order misses by 20.

## Node handlers (classes 1, 8, 9, 10)
All share the culling prologue above. Classes 1, 9 and 10 also rebuild bounds and test whenever
`node+0x24` bit `0x80000` is clear, whatever the mode. When visible each one pushes a matrix level
backed by its own class data, places the node, draws any render object at `node+0x3c` through the
queue function pointer `[0x0057d9e0]`, dispatches its children (with `EDX` = child count) and pops.

| class | matrix storage | placement |
|---|---|---|
| 1 camera | `data+0x44` | `FUN_00474010(angles data+0x20, position data+0x14, scale 1)` |
| 9 light | `data+0x38` | `FUN_00474010(angles data+0x8, position data+0x14, scale 1)` |
| 10 sound | `data+0x48` | `FUN_00474010(angles 0, position data+0x30, scale 1)` - a point, never rotated |
| 8 | `data+0x38` | `zTransformConcatenateLocal(data+0x8, mode 3)` - a stored local matrix, only if `data+4` bit 4 |

The sound handler's inputs were captured in `Recoil18` (angles 0, `EDX` = `data+0x30`, scale 1). Its
class data is the same sound object whose world position `FUN_00452ec0` writes at `+0x3c`.

## Object handler - `0x0044b300` (class 5, `VERIFIED-ORACLE`)
- **World matrix cached in class data `+0x60`.** Recomputed as local (`data+0x30`) composed with the
  parent, mode 3, when node flag `0x80000` is clear or `data[0]` bit `0x20` asks (the bit is then
  cleared); otherwise the cache is pushed by reference. `data[0]` bit 3 means no local matrix at all.
  Measured: the level's matrix is `data+0x60` and equals local composed with parent (error 0).
- **Render-state overrides**, unwound on exit: `data[0]` bit 2 pushes `data[1]` (stack `0x00539830`,
  via `0x00476070`; restores 1.0 when empty); bit 4 pushes `data[2..5]` (stack `0x00539988`, via
  `0x00476040`; restores 0). Node flag `0x800000` toggles a render mode via `0x00476080`, guarded
  by `[0x00539b94]` so only the outermost such node switches it.
- Optional extra visibility test `0x00476700(sphere)` when `[0x004ddd10]` is set, `[0x004ddd2c] >= 1`
  and the mode is not 1.
- Class-5 children skip the dispatch: type filter, then this handler directly.

## Diamond tiler: bucket builder `0x0044c3c0` and hull `0x0044c230` - CONFIRMED
- **Buckets** at `0x0056ccc0`: 50 buckets, stride `0x2d4` = 30 entries of `0x18` bytes plus a
  count at `+0x2d0`. Entry: `+0` cell x, `+4` cell y, `+8` flag (0), `+0x14` mask (`0x3f` bits).
- Camera cell from `0x00450790`. Footprint: template points at camera data `+0xfc`, translated
  and rotated by yaw; pitch and roll rotations added only when their magnitude exceeds
  `0.174533` rad (10 deg, `CONFIRMED-BINARY`), giving 3 or 5 points at `0x0056cc40`.
- More than 3 points: gift-wrap hull `0x0044c230` in XZ, angle from the diamond atan2
  substitute `0x0044c1b0` (current point in ECX, candidate in EDX), limit `6.2831855`; returns
  the hull size.
- Cell box = footprint min/max, clamped to the grid size at data `+0x78/+0x7c`. Each cell whose
  mask has the level bit: `0x004879c0`, then a sphere test; accepted cells go to bucket
  **|dx| + |dy| from the camera cell** (must be < 50), max 30 per bucket, else the report
  "Need more diamond tiler".
- `VERIFIED-ORACLE` (Recoil18 `C2FF:1E41`): 20 entries in buckets 0-7, all at exactly their
  bucket's Manhattan distance from one common cell (8, 11). Footprint geometry is from the
  body only.

## Callees outside the subsystem - CONFIRMED (not members; recorded here because only scene_render calls them)
- **Footprint clip polygon** `0x00487900` (`VERIFIED-ORACLE`): ECX points, EDX count (max 64). Copies
  the points to `[0x0057d984]` and writes edge normals `normalise(Pj.z - Pi.z, 0, Pi.x - Pj.x)`,
  `j = i+1 mod n`, to `[0x0057d988]`; count at `[0x0057d98c]`. Measured: normals to 2.5e-8.
- **Point in footprint** `0x004879c0` (`VERIFIED-ORACLE`, 6/6): fails as soon as any edge gives
  `(p - Pi) . ni < margin` in XZ. The builder passes camera data `+0x6c` = **-192.0**
  (`MEASURED`), so a cell centre may lie up to 192 units outside the footprint.
- **World cell lookup** `0x00450650` (bytes): clamps a point into the grid by 0.1
  (`0x004d2398` = -0.1, `0x004d239c` = +0.1, `CONFIRMED-BINARY`), truncates to cell indices, and
  also returns the unclamped `floor` cell and an inside flag.
- **Sphere occlusion test** `0x00476700` (bytes): projects the bounding sphere, builds a screen
  rectangle of half-size `trunc(p5 p3 r / (z - r))` pixels and probes the span buffer row by row
  with `0x00491dd0` (first row, last row, middle, then every 8th row out from the middle).
  Near test `z - r <= 0.0001` counts as visible. Screen bounds measured 0, 0, 640.001, 400.001.
  Gated by `[0x004ddd10]`; 0 calls in both traces. The span buffer itself (`0x00491dd0`,
  `0x004907c0`) is not yet read.
- **Error wrapper** `0x004622f0` (bytes): forwards the buffer `0x00575de0` with `zerr_old.c`
  line 0x23 to the no-op reporter `0x00404e80`.

## Tiled bucket builder `0x0044c8e0` - CONFIRMED (bytes; 0 calls in both traces)
Chosen instead of `0x0044c3c0` when camera data `+0x50` is non-zero. Same buckets and ring rule,
but the footprint's cell box may run **past the edge of the grid**. Such a cell is clamped to the
edge cell, its entry flag (`+8`) is set to 1, and the offset `(dx * data+0x54, dz * data+0x58)`
goes to `+0xc/+0x10`. Out-of-grid cells skip the sphere test, so they are always kept. The
culling pass then translates by the negated offset and draws only the children whose name
contains `VAP_statics`, so the map's edge cells are **repeated beyond the map**.
`INFERRED`: this is a mode for showing terrain past the playable edge. Alternative not excluded:
a debug or editor view. The switch at data `+0x50` is 0 in both traces.

## LOD handler `0x0044b8c0` - CONFIRMED (`VERIFIED-ORACLE` for distance and range)
- A class-4 node whose data word 0 is set measures **dist2 = k * |camera - node centre|^2**, where
  `k` is camera data `+0xd4` = `1 / (+0xd0)^2` (measured 4, from `+0xd0` = 0.5). This is stored
  in a per-depth slot (`0x00539900 + 16 * depth`); child LOD nodes with word 0 clear reuse it.
- Visible only while **near^2 <= dist2 < far^2** (data `[1]`, `[3]`); data `[2]` is far itself.
  Measured pairs: 128 / 16384, 256 / 65536, 900 / 810000.
- `dist` = fast sqrt of dist2 (`(i >> 1) + 0x1fc00000`, must be copied), clamped to far. Optional
  per-axis scale fades, an alpha fade and a near fade are read from the body; none were active in
  the samples.
- Six Recoil18 calls: dist2 exact, every accept or reject path as predicted.

## Small callees - CONFIRMED (bytes)
- `0x00476040`, `0x00476070`, `0x00476080`: render-state setters for `0x0057d960..74`.
- `0x0047a0c0` / `0x0047a1d0`: six camera-space frustum plane normals built from the half FOV
  angles (`+0xf0`, `+0xf4`), then rotated into world space at `+0x188` once per frame.
- `0x00450790`: clamped grid cell of a point (0.1 inset).
- `0x004a2950`: listener update. Mode 0 stores the camera matrix and velocity; mode 1 calls a 3D
  listener object with position, orientation (front = -row 2, up = row 1) and velocity.
  `INFERRED`: A3D-style listener (slot layout). Recoil18 runs mode 0.
- `0x004a2e70` (`Sound_GetSpeedOfSound`, see `sound.md`): returns the current speed of sound (default 345.0, overridable by the `SPEED_OF_SOUND` config key). The sound node zeroes its velocity when its magnitude is not below this value, so Doppler is disabled rather than inverted at supersonic speed. `CONFIRMED-BINARY`.

## Culling pass `0x0044ce70` - CONFIRMED (`VERIFIED-ORACLE`, partial sample)
Builds the buckets (tiled builder when camera data `+0x50` is set), then walks buckets 0..49 in
order, **nearest ring first**. For each entry: the grid cell's children are dispatched with EDX =
child count. A flagged (out-of-map) entry first shifts the camera by the negated offset and draws
only `VAP_statics` children. Extras:
- occlusion test (`0x00476700`) only when `[0x004ddd10]` is set and not in ring 0;
- objects listed at camera data `+0x94/+0x98` get a "near this cell" flag at `+0xc8`;
- fade selection uses `distance + 1.1 * radius` (`0x004d2370` = 1.1, `CONFIRMED-BINARY`).

Recoil18: rings 0, 1, 2 visited in order; 24 and 15 dispatches, equal to the cells' child counts.

## Node-to-ancestor matrix `0x00449480` - CONFIRMED (bytes; `VERIFIED-ORACLE` on two paths)
Builds a node's matrix onto the current transform level by walking its single-parent chain, root
first. Per class: camera = TRS from its angles and local position (or a concat of `+0x80`);
object = its local matrix `+0x30`, with a world-matrix cache at `+0x60` (flag `0x80000` plus data
bit `0x20` refreshes the cache; flag without the bit loads it); class 8 = conditional concat;
class 9 = TRS; class 10 = pure translation; classes 2 and 6 contribute nothing. A node with two
parents is an error (returns 1). Measured: camera result exact to 2.1e-8; a class-10 node under a
cached object returns the object's cache.
- Render-state ranges (CONFIRMED, bytes): `0x00476190` / `0x004761e0` set range A's start and end
  (`0x0057d944` / `0x0057d948`); `0x00476220` / `0x00476260` set range B (`0x0057d950` / `0x0057d954`).
  Each setter re-caches `1 / (difference)` (`0x0057d94c` / `0x0057d958`), keeping the old value when
  the difference is 0. `INFERRED`: fog-style start/end ranges. `World_Update` feeds them from world
  data `+0x20/+0x24` and `+0x28/+0x2c`.
- Render colour (CONFIRMED, bytes):
  - `0x004762c0` stores a float triple;
  - `0x004762f0` → `0x0049b1e0` clamps it to [0, 1] and **re-applies only if a component changed by
    0.01 or more**. It rounds each channel as `trunc(255 c + 0.5)` and packs them with the display's
    per-channel shifts and masks (`0x0057de40..54`; `INFERRED` 16-bit pixel format) for `0x0049e0e0`;
  - `0x004a7220` keeps a 0..255 copy.

  `0x004076f0` is an empty stub (a single `ret`) that `World_Update` calls anyway.

## Function index additions (2026-09-24)
- **Getters.** `FUN_00476180` `0x00476180` returns `[0x0057d930]`; `FUN_004761d0` `0x004761d0`
  returns the float `[0x0057d944]`. Both are called every frame.

