# Scene update and node lists

Subsystem `scene_update` (registry: membership **PROVISIONAL**; boundary questions in
`03_re/ledger/subsystem_registry.csv`). The string-less block `0x0044e630..0x0044eed0` between
`zClass/Object3d.c` and `zClass/List.c`.

## Node lists - MEASURED (Recoil18 `C215:1784`)
- 16 lists. Pointer table `0x004ddef8[i]` gives each list's 12-byte record: **tail `+0`**
  (newest link, where `NodeRegistry_Insert` appends), `+4` (head side, see `camera.md`), **dirty
  flag `+8`**. For example, list 7 (nodes needing an update) has its record at `0x00539c00`,
  flag `0x00539c08`; list 6 at `0x00539bac`.
- A link is **`[0]` node, `[1]` next (newer), `[2]` prev (older), `[3]` mark**, the layout
  `NodeRegistry_Insert` established. Measured along three links of list 7 from the tail: the tail's
  `[1]` is 0, each `[2]` leads to an older link, whose `[1]` points back.
- Freed links go on a chain at `0x00539c6c`.
- Correction, same day: a first draft of this section called `+0` the head and `[2]` next. The
  chain dump fits both readings; the confirmed insert function settles it.

## Confirmed functions
- `0x0044eed0` mark node (`VERIFIED-ORACLE`): walks list ECX from the tail and marks the node's first unmarked link, sets
  the mark and the list's dirty flag. A repeated call on a marked node changes nothing.
- `0x0044eb00` update pending subtree (bytes): children first (those with node flag bit 0), then
  `gwNodeUpdate` (`0x00448cc0`, Class.c) and a mark in list 7.
- `0x0044e6d0` free the link pool (bytes; shutdown only).
- List runners (bytes), all walking tail-to-older and skipping marked links:
  - `0x0044eaa0` calls each node's callback at `+0x48` (flag 4 required); a node without one is
    marked;
  - `0x0044ebe0` runs list 11 through `Seq.c` `0x004541c0`;
  - `0x0044ec30` runs list 12 through `Animate.c` `0x00453bd0`;
  - `0x0044eba0` updates list-7 nodes until none is left unmarked;
  - `0x0044ec80` runs those three in that order.

  Each runner turns the flush switch `0x004dded8` off during its loop and flushes afterwards.
- `0x0044ecf0` finds a node by name: **the node begins with its name string**.
- `0x0044ecb0` is a debug print (`"Node %d desc %s"`).
- `0x0044ea70` per-frame entry: for lists 0 to 5, run callbacks (`0x0044eaa0`), then the client
  runners (`0x0044ec80`). `0x0044ec90` counts a list (`VERIFIED-ORACLE`, weak: every sample had one link).
- Link pool (bytes): `0x0044e630` allocates a link, recycling from the free chain `0x00539c6c` or
  callocing 16 bytes, and counts live links (`0x00539c74`, peak `0x00539c78`). `0x0044e690` frees
  one back onto the chain.
- `0x0044ee10` pushes at the head end (`0x004ddf38` = record `+4`). For list 7 it flags the node
  (`+0x24` bit 0) and pushes each unflagged, non-class-2 parent too.
- `0x0044eb50` updates a node, then walks **up** through its parents (`+0x54`/`+0x58`) and flushes.
- Deferred chain `0x00539c70`: `0x0044ed60` pushes a node; `0x0044eea0` drains it through Class.c
  `0x00447a70` (not read). Neither runs in the traces.
- `0x0044ed50` returns a list's tail link. It was previously named for one caller's use (the
  camera list).

## Mark, then flush
Marking a link (`0x0044eed0`) schedules it for **removal** at the next flush:
- **`0x0044e700` flushes one list** (`VERIFIED-ORACLE`). It removes every marked link and frees
  it. On list 7, a node whose flag bit 2 is set is only unmarked and stays queued; removal clears
  the node's bit 0 ("in list 7"). Measured: 6 marked links of 11, 5 removed, the one with bit 2
  kept, the dirty flag cleared.
- **`0x0044e920` flushes all dirty lists** in a fixed order: 6, 0-5, 7-10, 13, 14, 15, 11, 12,
  then drains the deferred chain. The runners switch flushing off (`0x004dded8` = 0) during their
  loops and flush afterwards, so lists never change under an iteration.

## Boundary - membership CLOSED
Members are the 22 functions of `0x0044e630..0x0044eed0`; only they touch the private state (dirty
flags, flush switch, link pool, counters, deferred chain). Clients:
- List.c `0x0044f120`, which deletes a list's nodes and then resets its head pointer;
- collision and `0x00455350`, which read list records 7 and 13 directly;
- Class.c: `gwNodeUpdate` `0x00448cc0`, node API.

The deferred chain ends in Class.c's node free `0x00447a70` (CONFIRMED, bytes). That function
refuses a node that still has children, parents or a model; otherwise it frees its arrays and
class data and returns the slot to the pool's free list, threaded through `+0xc0` (see
`camera.md`).
- List-11 client `0x004541c0` (Seq.c, CONFIRMED from bytes; no calls in either trace): advances a
  keyframe sequence by the frame time. It steps the key index while the accumulated time exceeds
  the current key's duration. At either end it **wraps** or **bounces** (reversing the step); a
  non-repeating sequence stops.
- List-12 client `0x00453bd0` (Animate.c, CONFIRMED from bytes; no calls in either trace).
  Per animated node, it:
  1. advances the channel clock `0x00453c90`, which loops back to its start point or finishes;
  2. interpolates the keyframes `0x00453d20`. A key is 9 floats, taken in three triplets. Each is
     lerped between the neighbouring keys at `x = t (N - 1) / end`, then scaled per component;
  3. marks the node's local matrix dirty and queues it on list 7 for `gwNodeUpdate`.

## `gwNodeUpdate` `0x00448cc0` - CONFIRMED (`VERIFIED-ORACLE` on the class-5 path)
Called for each node queued on list 7. It rebuilds a node's **local matrix from its TRS fields**
when that class's dirty bit is set:
- **object (class 5):** `data+0x30` = TRS(angles `+0x18`, position `+0x54`, scale `+0x24`). This is
  the only rebuild that runs in Recoil18 (1942 times). Measured three times to 4.3e-8.
- **camera (class 1):** `data+0x80` from `+0x20`/`+0x14`.
- **class 8:** `data+8` from `+0x70`/`+0x7c`/`+0x88`.

It then propagates through `0x00448e90` and clears node flag bit 1. Class 2 (world) goes to
`0x00450530`. `0x00448e90`, `0x004491b0`, `0x00449420` and `0x00450530` are not read yet; they
belong to a future `class_api` subsystem.
