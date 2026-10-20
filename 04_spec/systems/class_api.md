# Class.c node API

Subsystem `class_api` (registry: membership **CLOSED** 2026-09-15; boundary evidence in
`03_re/ledger/subsystem_registry.csv`). Functions citing `zClass/Class.c` (string `0x004dd9e8`),
`0x004478c0..0x00449af0`. Node layout facts used here are measured in `camera.md` and
`scene_update.md`.

## Bounding boxes
A node keeps three axis-aligned boxes, each min xyz then max xyz:

| offset | box | valid flag (`+0x24`) |
|---|---|---|
| `+0x74..+0x88` | the node's box, used by culling | `0x100` |
| `+0x8c..+0xa0` | its model's box | `0x200` |
| `+0xa4..+0xb8` | union of its children's boxes, in its local space | `0x400` |

- `0x00448e90` (CONFIRMED, bytes) sets the node box to the union of the model and children boxes,
  or whichever one exists, and then propagates:
  - a **world** parent re-buckets the node into a grid cell from its XZ extent;
  - any **other** parent is flagged and queued on list 7 for its own update.
- `0x004487c0` (CONFIRMED, bytes) gives the node's 8 corners. For classes 5, 1 and 8 they are
  transformed by the node's local matrix; otherwise the raw corners, in the fixed order
  (minx,miny,maxz), (maxx,miny,maxz), (maxx,miny,minz), (minx,miny,minz), then the same with maxy.
- `0x004491b0` (CONFIRMED, bytes) rebuilds the children box from every child's corners that has a
  valid box. `0x00449420` (CONFIRMED, bytes) asks the model (gmod `0x00483ad0`) for its box.
  `gwNodeUpdate` runs them when `+0x2c` bit 1 (children changed) or bit 0 (model changed) is set.

## Per-frame action callbacks
A node can carry an **action callback** at `+0x48`. It lives on one of six callback lists (index
at `+0x44`, 0 to 5), which `NodeList_RunAll` runs every frame (see `scene_update.md`):
- `0x00447f30` (CONFIRMED, `VERIFIED-ORACLE` on the clear path) sets the callback, appending the
  node at the list tail the first time. Clearing it marks the node's link, so it leaves the list
  at the next flush. Measured four times in Recoil18: mark 0 -> 1.
- `0x00447fe0` (bytes) is the same but inserts at the head, so its callback runs first.
- `0x00448090` (bytes) moves a node to another callback list.

An out-of-range index is an error (returns 1). A failed insert frees the node unless flag `0x800`
is set.

## Small accessors (CONFIRMED, bytes)
- Node-flag setters and getters for bits `0x8`, `0x10`, `0x20`, `0x40`, `0x80`, `0x10000`, `0x20000`
  and `0x800000` (flag word `+0x24`). `0x00448360` can only clear `0x1000000`. `0x00449b40` sets or
  clears `0x80000` over a whole subtree, refusing nodes with several parents.
- `0x00447f00` returns the model `+0x3c`; `0x00448760` the node box; `0x00448330` stores one byte
  at `+0x30`.
- **Two functions typed `void` actually return EAX:** `0x00449af0` (the node's root just below the
  world) and `0x00447e30` (the node itself, 0 if null).
- **Node names are stored inline** at `node+0` (a 0x24-byte field). `0x00447dc0` handles a name of
  36 bytes or more with `strncpy(.., 0x22)` and a terminator at `0x23`, so byte `0x22` keeps
  whatever was there before. A faithful remake keeps that quirk.
- `0x00449ba0` was **removed** from this subsystem. It sets a render frame-rate cap that only
  `Render_CameraFrame_SW/HW` read (see its ledger row).

## Lifecycle, activity, model, world queries (CONFIRMED, bytes)
- **Delete `0x00447b60`** frees the node at once if a flush can run. When flushing is switched off,
  because a list runner is iterating, it defers the node to the deferred chain, which frees it at
  the next flush. This is what makes deletion during iteration safe.
- **`gwNodeSetActive` `0x00447c60`** toggles flag 4 for classes 1, 2, 5, 6 and 9, and hands class 10
  (sound) to `gwSoundNodeSetActive`. Other classes are an error (3). Only 0 and 1 change the flag.
- **`gwNodeSetModel` `0x00447e60`** swaps the model, refreshes the model box and queues the node.
- `0x00447bc0` finds a node by name in a subtree, depth-first.
- World queries build the node-to-root matrix on an identity level:
  - position `0x004497b0`, using the cached world matrix when there is one;
  - local point to world `0x00449850`;
  - orientation `0x004498e0`, from two transformed template vectors.

## Per-class dispatch (CONFIRMED, bytes; tables checked entry by entry)
Class numbers in use: 0-11. Each has its own handler module:

| class | destroy `0x00447980` | attach child `0x004483f0` | detach child `0x00448570` |
|---|---|---|---|
| 0 | delete (returns the node pointer) | - | - |
| 1 camera | delete | `0x00449c90` | `0x00449cd0` |
| 2 world | `0x00450240` | `0x004510e0` | `0x00451240` |
| 3 | delete | `0x004484d0` | `0x0044f870` (List.c) |
| 4 | delete | `0x004484d0` | `0x0044fe50` |
| 5 object | delete | `0x0044db10` | `0x0044db60` |
| 6 | delete | `0x004484d0` | `0x00454320` |
| 7 | delete | error "Please use ..." | `0x00454000` |
| 8 | `0x00453b10` | `0x00453b40` | `0x00453b80` |
| 9 | `0x00453110` | `0x004484d0` | `0x004531c0` |
| 10 sound | `0x00452ab0` | `0x004484d0` | `0x00452b80` |
| 11 | delete | `0x00452920` | `0x00452970` |

Destroy refuses a node that still has parents (returns 1).
- Per-class handlers confirmed so far (bytes):
  - Light and Sound **refuse to be destroyed while still registered with a world**: they return 1
    while their back-list count is non-zero. Sound stops its voice first.
  - Animate's destroy, and the Light / Animate / Sound attach and detach handlers, validate and then
    hand over to Class.c's generic delete, attach or detach.
- **Per-class registry lists** (from `List_UnregisterNode` `0x0044f000`, jump table checked):
  camera 8, world 13, class 3 → 14, Display 15, Seq 11, Animate 12, Light 9, Sound 10; every node
  is also on list 6. Deleting a node marks it on its class list, list 6, list 7 (if queued) and its
  callback list (if any).

## Function index additions (2026-09-24)
- **Node flags.** Each setter sets or clears one bit of `+0x24`; a null node reports and returns
  5.
  - `gwNodeSetFlag10000` `0x00447d20` and `gwNodeSetFlag20000` `0x00447d70`;
  - `gwNodeSetFlag40` `0x004482b0` and `gwNodeSetFlag80` `0x004482f0`;
  - `gwNodeSetFlag800000` `0x004483a0`.
- **Node byte.** `gwNodeGetByte30` `0x00448180`.
- **Window class (type 3):**
  - `WindowClass_SetField_e4` `0x0044fad0`;
  - `WindowClass_SetClearPolygonActive` `0x0044fb40` (bit 31 of `+0xe0`);
  - `WindowClass_AddClearPolygonVertex` `0x0044fbd0` (at most 4 polygons of up to 4 vertices).
- **Display class.** `DisplayClass_Create` `0x0044fdd0`: type 4, `{w=1, h=1, 0.392, 0.392, 1.0}`.

