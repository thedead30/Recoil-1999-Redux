# Collision queries (scene database) — first pass

**Scope:** the scene-graph intersection queries used by rendering (lens-flare visibility) and by
gameplay. Every statement here is summarised from CONFIRMED ledger rows (bytes read); see each
row's notes in `03_re/ledger/functions.csv` for offsets. Membership is not registered as a
subsystem yet, so there is **no gate** — do not write code from this file.

## Two query kinds share one traversal shape
| | ray / segment query | point query |
|---|---|---|
| hierarchy | `Collision_TestNodeHierarchy` `0x00445650` | `Collision_QueryPointHierarchy` `0x00443f80` |
| node flag required | bits 4 **and** `0x10` | bits 4 **and** 8 |
| bounds pre-test | `Collision_NodeBoundsReject` `0x00447540` (3-axis overlap, touching = miss) | `Collision_NodeBoundsContainsPointXZ` `0x004472c0` (**X/Z only, Y ignored**) |
| mesh test | `Collision_TestMeshPolygons` `0x00484fc0` | `Collision_MeshPointQueryXZ` `0x00484960` |
| per class | 1 camera `0x00445b20`, 8 `0x00445a00`, 9 light `0x00445c20` | 8 `0x00444310`, 9 light `0x004443e0` |

Shared state: query ray start/end `[0x005396d8]` / `[0x005396e4]` (point = start), query box
`[0x005396f0..0x00539704]`, exclusion mask `[0x005396d4]`, first-hit mode `[0x005396d0]`, hit
counter `*[0x00539810]`, next hit record `[0x00539814]` (0x28 bytes, node pointer at +0x24).

## Behaviours a remake must reproduce
- **At most 32 hits** per query; the 33rd logs "Database intersections array is full" and stops.
- **First hit in polygon order, not the nearest** — both mesh tests stop at the first matching
  polygon in list order.
- **Ray test uses a transpose inverse** to bring the segment into mesh space: correct only for
  unscaled transforms.
- **Collision follows animated vertices** when a mesh's deform gate is active (the same gate the
  shaders use).
- **Point query is "surface below the point"**: a 2D X/Z point-in-polygon test that only accepts
  surfaces whose height is at or below the query y.
- **Point bounds ignore Y** — a point far above or below an object passes its box test.
- **Class 8 point query tests the mesh before applying the node's own transform** (parent space).
- **Light nodes build their transform Translate → RotateY → RotateX → RotateZ.**
- **LOD nodes are skipped when `[data+4]` > 5.0**; empty group nodes are skipped.
- **Sound nodes**: the ray query returns the node pointer (non-zero, read as no hit); the point
  query returns an **uninitialised value** — a real defect.
- **World grid wraps**: `World_QueryObjectsAtPoint` `0x00443d20` shifts the query by whole tiles
  when the point is outside a wrapping world's grid.

## A third query kind: point/segment SETS — `CONFIRMED-BINARY` (partial)
Functions `0x004473e0`, `0x004476f0`, `0x00446880` query a whole set at once: points/segments
in `[0x0053981c]` (12-byte entries), count `[0x00539820]`, each with an **active mask** entry
that bounds tests clear as the traversal descends. Per-segment boxes live at `0x005396f0` in
6-float strides. Each branch copies the mask locally so pruning in one subtree does not affect
its siblings. The point-set bounds test again ignores Y.

**Set-size limits — `CONFIRMED-BINARY`.** Segment-set branches keep a 12-dword mask copy and point-set branches a 24-dword copy, copied without a check — but both entry points clamp first: `World_QueryPointSet` (`0x004444b0`) to 24 points and `World_QuerySegmentSet` (`0x00445d40`) to 24 endpoints = 12 segments, logging "More test pnts than space for…". So there is **no overrun through the entries**; a remake should keep the 24-point / 12-segment caps and the log. CORRECTION: an earlier version of this note called the copies a stack overrun; the entry clamps rule that out.

The segment entry **always clears the exclusion mask** on exit, so a mask lasts exactly one query. Out-of-grid points in a wrapping world are shifted by whole tiles for the query and shifted back afterwards.

**Segment grid walk.** `World_QuerySegmentSetGridWalk` (`0x00445f60`) tests **every grid cell in the
union rectangle** of all segment boxes — not a line-traversal of cells — so long diagonal segments
are costly and test many cells the segment never crosses. (The single-ray path uses
`Collision_GridDDATraversal` instead.)

## Function index (added 2026-09-24)
- **Queries:**
  - `Collision_SurfaceUnderPoint` `0x00443c70`;
  - `Collision_RaycastSegment` `0x00444de0` (grid DDA; with fewer than 2 hits the count becomes 0);
  - `Collision_TestChildNodes` `0x004455f0` (children with flag bits 4 and 0x10 whose type is
    enabled for rendering).
- **Hierarchy walkers.** Point-set: `Collision_MultiPointHierarchy` `0x00444890`,
  `Collision_MultiPointClass8` `0x00444c50`, `Collision_MultiPointLightNode` `0x00444d10`.
  Segment-set: `Collision_MultiSegmentHierarchy` `0x00446440`, `Collision_MultiSegmentLightNode`
  `0x00446970`.
  - Each copies the active mask into a fixed local array: 24 dwords for points, 12 for segments.
  - The copy loop is bounded only by the entry clamp in
    `World_QueryPointSet` / `World_QuerySegmentSet`.
- **Polygon tests:**
  - `Collision_PointSetVsPolygon` `0x00484b70` (with a maximum height from `[0x00539818]`);
  - `Collision_PointInPolygonXZ_Height` `0x004856d0` (convex, X/Z plane, starting with edge
    last→first);
  - `Collision_RayPolygonTest` `0x004857f0` (one-sided unless the two-sided flag);
  - `Collision_RayPolygonTestWithUV` `0x00485d10`;
  - `Collision_SegmentSetVsPolygon` `0x00486290` and `...WithUV` `0x004869a0`.
- **Meshes and boxes:**
  - `Collision_MeshPointSetQuery` `0x00484e00` and `Collision_MeshSegmentSetQuery` `0x00487350`
    (animated vertices, world transform, every polygon);
  - `Collision_RayVsTransformedBox` `0x00485380` and `Collision_SegmentSetVsTransformedBox`
    `0x00487540` test the box as six quads.
- **UV output.** `Collision_UVQueryEnabled` `0x00479c80` (`[0x0057d9a0]`) and
  `Collision_StoreLastHitUV` `0x00479c90` (`[0x0057d9b4/b8]`).

