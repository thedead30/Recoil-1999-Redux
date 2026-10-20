# zGeometry: Weiler-Atherton clipping and triangulation

**Status:** membership CLOSED, 87/87 CONFIRMED. Mostly Ghidra decompile; some hidden-argument
helpers were read from bytes or disassembly. `0x00458ae0` (area-partition lookup) is delegated to
declient. Tag: `CONFIRMED-BINARY`.

**Caution:** `WeilerClip_FindIntersections` `0x00465ac0` and `WeilerClip_MergeContours` `0x00467710`
are recorded at structure level. Their per-code relink tables must be itemised before
implementation.

## Pipeline — `WeilerClip_Run` `0x00464810`
1. Swap axes when the clip's axis mode is set (`[+4]`: 2 swaps y/z, otherwise x/z). Translate to the
   origin when the first region point is beyond ±65536.
2. Bounds test: disjoint → **1**. One box inside the other: identical point sets → **4** (identical),
   otherwise contained → **3**.
3. Build two node rings (region, subject). Compute side-of-edge tables, weed out coincident edges,
   find intersections (edge crossing codes 4–23, collinear code 3), validate, and merge contours.
4. Classify the contours, emit them by class into three outputs (mode bits 0/1/2), undo the
   translation, restore z from the subject plane, undo the axis swap → **2** (intersecting).
5. No crossings: if the subject is inside the region, build a *keyhole* polygon joining both rings
   through their rightmost vertices → **4**.
6. Any failure → **0**. Failures are reported with the zg source line.

Tolerances: 1e-5 (collinear), 0.001 (coincident point), 0.01 (point on edge / find), 0.1 (snap),
rect overlap margin 1.0.

## Model clipping (`zg2`)
- `ClipObject_Dispatch`: object flag 0x20000 → `ClipModel_AgainstRegion`; 0x10000 →
  `ClipModel_SplitByRegion`. Objects with flag 0x200 are skipped when their bounds lie more than
  1.0 outside the region.
- Split by region, per polygon:
  - outside → copied unchanged;
  - crossing → the inside pieces are convexified and added as child polygons (UVs from the parent's
    gradients);
  - inside → dropped;
  - keyhole → ring-pair triangulation.
- `AreaPartition_ClipObjects`: per partition cell, snaps then clips the flagged objects.

## Triangulation
- `Triangulate_RingPair`: fixed global triangle list of **1024** entries (unchecked); edge buffer of
  (n+m)² entries.
- `Triangulate_PolygonRecursive` / `Triangulate_SplitPolygon`: split at the leftmost vertex, towards
  the nearest vertex inside its ear or else along the ear diagonal.
- `Convexify`: triangles are kept; a quad is kept if convex, else split at the reflex vertex;
  5+ vertices are triangulated.

## Other constants
Random debug material colour = rand()·255/32767. A fast-sqrt variant of the Newell plane
approximates the length from the float bits (`>>1 + 0x1FC00000`).

## Defects to reproduce
- Allocations unchecked throughout; the clip output buffer is only regrown past 128 points.
- "Bad clip region" is reported but processing continues.
- Disjoint early return leaves the translation and axis swap applied.
- Point-in-polygon returns an FPU-status value on the boundary.
- The 1024-triangle limit is unchecked.
- `Plane_SolveZForPoints` does not check for c = 0.

## Function index additions (2026-09-24) - polygon clipping and triangulation
Mostly the Weiler-Atherton clipper used to cut hazard discs (crater, quicksand) into the terrain
(`declient.md`), plus its triangulator. Tolerances **0.01, 0.001 and 1e-5 are part of the
behaviour**; reproduce them exactly.

- **Clipper lifetime and set-up:**
  - `WeilerClip_Init` `0x00464b90`, `WeilerClip_ResetRegion` `0x00464790`,
    `WeilerClip_ResetWorkArrays` `0x00469a30` (uses `IntLine_EvalAt` `0x00469ae0`: `count = n; value = n*a + b`);
  - `WeilerClip_FreeBuffers` `0x00464b30`, `WeilerClip_Destroy` `0x004647d0`;
  - `WeilerClip_GetOutput` `0x00464670`, `ClipResult_Free` `0x0046ab10`.
- **Precision guards:**
  - `WeilerClip_TranslateToOrigin` `0x00469960`: translates when the first point lies outside
    ±65536;
  - `WeilerClip_TranslateBack` `0x00469af0`;
  - `WeilerClip_RestoreZFromPlane` `0x00469b60`: z from the plane through the first three
    output points.
- **Classification:**
  - `WeilerClip_ClassifyBounds` `0x00464c90` (touching boxes count as disjoint);
  - `WeilerClip_ClassifyResult` `0x00464ea0`, `WeilerClip_ClassifyContours` `0x004680b0`;
  - `WeilerClip_ComputeSideTables` `0x00468470` (2D cross products);
  - `WeilerClip_WeedOutCoincident` `0x00464f70`, `WeilerClip_FindCrossingVertex` `0x00469d60`.
- **Edges and segments:**
  - `ClipEdge_ComputeBounds` `0x00468410`, `ClipArray_Call468410Each` `0x004693a0`;
  - `ClipEdge_Divide` `0x00468580` (no split within 0.001 of an endpoint);
  - `ClipEdge_AddPair` `0x00468650`, `ClipEdge_OrientFirst` `0x00469430`;
  - `ClipEdge_ClassifyJoin` `0x00469450`, `ClipEdge_ClassifyTouch` `0x00469560`;
  - `ClipSegments_Classify` `0x00468fa0`, `ClipSegments_Intersect2D` `0x00468c40`;
  - `ClipNodes_BuildRing` `0x004693c0` (0x3C-byte nodes).
- **Output:**
  - `WeilerClip_OutputContours` `0x004681a0`, `ClipContour_Output` `0x004682c0`,
    `ClipContour_AppendToOutput` `0x00468700`;
  - `WeilerClip_GenerateOutsideResult` `0x004687b0`, `ClipContour_SwapAxes` `0x004683a0`,
    `Contour_EnsureBuffer` `0x004676c0`.
- **Point tests:**
  - `Point2_InPolygonQuadrant` `0x00468a10` (crossing parity at x ≥ point);
  - `Point2_OnSegmentRange` `0x00469ca0` (1e-5 axis choice);
  - `Point2_NearlyEqual` `0x00469e50`, `Point2_SnapOntoSegment` `0x00469e90`;
  - `PointArray_RemoveAdjacentDuplicates` `0x0046a080` (0.01);
  - `PointArray_SnapToReference` `0x0046a130`, `PointArray_Find2D` `0x0046ab40` (0.01);
  - `Rect_OverlapMargin1` `0x0046a620` (margin 1.0), `Rect_FromPoints2D` `0x0046a9c0`;
  - `Segment2_IntersectStrict` `0x0046be20` (strictly inside both segments).
- **Clip polygons:**
  - `ClipPolygon_Create` `0x0046aa40`, `ClipPolygon_CopyOutRotated` `0x0046aab0`;
  - `ClipPolygon_MergePoints` `0x0046ab90`, `ClipPolygon_FindEdgeContaining` `0x0046ac80`;
  - `ClipPolygon_ComputeUVs` `0x0046a7f0` (UV gradients from the parent polygon via
    `Triangle_SolveGradientXZ` `0x0046a8e0`);
  - `ClipPolygon_EmitToModel` `0x0046bb30`, `Geometry_AddChildPolygon` `0x0046ba90` (fewer than
    3 points is an error);
  - `ModelPolygon_GatherPoints` `0x0046b650`, `ClipModel_SnapToRegion` `0x0046b030`;
  - `Vec3Array_RotateX90` / `Inverse` `0x0046a5e0/0x0046a600`: the clip works in the XZ plane by
    rotating y and z.
- **Groups and nodes.** `ClipGroup_BuildModelNode` `0x0046af40`, `ClipGroupList_Release`
  `0x0046ae40`.
- **Triangulation:**
  - `Triangulate_TryAddEdge` `0x0046bd50`, `Triangulate_EmitEar` `0x0046bfc0`;
  - `EdgeList_FindEdge` `0x0046bf70`, `EdgeList_FindUsingVertex` `0x0046bf30`;
  - `Triangle_EnsureCounterClockwise` `0x0046c620`, `Vec3Array_ReverseKeepFirst` `0x0046c5b0`;
  - `Geometry_Call46c3a0_Global53d758` `0x0046c390`.
- **Utilities:**
  - `DynArray_Init/Free/Push` `0x00467600/0x00467630/0x00467660` (growth = n + 16, unchecked
    `realloc`);
  - `Alloc16Zeroed` / `Alloc16_Free` `0x0046af00/0x0046af20`, `Alloc3Ptr_Free` `0x0046c720`;
  - `Material_CreateRandomColour` `0x0046a690` (rand × 255/32767, 5-6-5 pack; consumes `rand()`,
    so it affects the random sequence);
  - `Geom_GetBFETolerance` `0x00476470`.

