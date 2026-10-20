// SUBSYSTEM: geometry
// Declarations for src/GameZRecoil/zGeometry/zgeo_weiler.cpp (original file GameZRecoil\zGeometry\zgeo_weiler.cpp,
// ledger orig_file): Weiler-Atherton polygon clipping. Spec: 04_spec/systems/geometry.md
#pragma once

namespace recoil {

// Coverage build only (RECOIL_ENTRY_COUNTS): entry counts of the clipper functions below, in this order.
constexpr int kWeilerEntryCountSlots = 29;  // PLATFORM: remake bookkeeping (one slot per counted port), not a game value
#ifdef RECOIL_ENTRY_COUNTS
extern unsigned g_WeilerEntryCounts[kWeilerEntryCountSlots];
#endif

// 0x00464670 WeilerClip_GetOutput (ECX clip, EDX out): null -> 0; *out = [clip+0x18]; returns [clip+0x14].
int __fastcall WeilerClip_GetOutput(const void*, int*);
// 0x00468410 ClipEdge_ComputeBounds (ECX edge): min/max of the two endpoint x and y ([+0xc], [+0x10]) into +0x18..+0x28.
void __fastcall ClipEdge_ComputeBounds(void*);
// 0x004683a0 ClipContour_SwapAxes (ECX contour): per point, swaps a component pair chosen by [+0x4] == 2.
void __fastcall ClipContour_SwapAxes(void*);
// 0x004693a0 ClipArray_Call468410Each (ECX array of 0x3c-byte edges, EDX count): ClipEdge_ComputeBounds on each.
void __fastcall ClipArray_Call468410Each(void*, int);
// 0x00469430 ClipEdge_OrientFirst (ECX edge): if the owner's second link is this edge, swaps the owner's link pairs.
void __fastcall ClipEdge_OrientFirst(void*);
// 0x00469ae0 IntLine_EvalAt (ECX line, EDX t): [+0x8] = t; [+0x10] = t * [+0x0] + [+0xc] (integer).
void __fastcall IntLine_EvalAt(void*, int);
// 0x00469e50 Point2_NearlyEqual (ECX a, EDX b, stack eps; ret 4): |ax - bx| <= eps and |ay - by| <= eps -> 1.
int __fastcall Point2_NearlyEqual(const float*, const float*, float);
// 0x0046a5e0 Vec3Array_RotateX90 (ECX count, EDX points): (x, y, z) -> (x, z, -y) in place.
void __fastcall Vec3Array_RotateX90(int, float*);
// 0x0046a600 Vec3Array_RotateX90Inverse (ECX count, EDX points): (x, y, z) -> (x, -z, y) in place.
void __fastcall Vec3Array_RotateX90Inverse(int, float*);
// 0x00467600 DynArray_Init (ECX array, EDX capacity, stack element size; ret 4): calloc(capacity, size); count 0.
void __fastcall DynArray_Init(void*, int, int);
// 0x00467630 DynArray_Free (ECX array): frees the buffer at [+0xc] if any and zeroes the header.
void __fastcall DynArray_Free(void*);
// 0x00467660 DynArray_Push (ECX array, EDX n, stack out-old-buffer or null; ret 4): grows by realloc when full, then
void __fastcall DynArray_Push(void*, int, void**);
// 0x004647d0 WeilerClip_Destroy (ECX clip or null): frees the four dynamic arrays (+0x34, +0x48, +0x5c, +0xc), then the clip.
void __fastcall WeilerClip_Destroy(void*);
// 0x00464b30 WeilerClip_FreeBuffers (ECX clip or null): frees and nulls [+0x1c], [+0x4], [+0xc], [+0x14].
void __fastcall WeilerClip_FreeBuffers(void*);
// 0x00468700 ClipContour_AppendToOutput (ECX contour, EDX pass): appends the pass's points to the output (realloc above
bool __fastcall ClipContour_AppendToOutput(void*, int);

// 0x00468a10 Point2_InPolygonQuadrant (ECX point, EDX count, stack xyz polygon; ret 4): crossing parity -> 1 inside,
int __fastcall Point2_InPolygonQuadrant(const float*, int, const float*);
// 0x00469ca0 Point2_OnSegmentRange (ECX p, EDX a, stack b; ret 4): p within the a..b range on x (or y when |a.x - b.x| < 1e-5).
int __fastcall Point2_OnSegmentRange(const float*, const float*, const float*);
// 0x00469e90 Point2_SnapOntoSegment (ECX a, EDX b, stack point, stack eps; ret 8): snaps the point onto a..b when within eps -> 1.
int __fastcall Point2_SnapOntoSegment(const float*, const float*, float*, float);
// 0x0046a080 PointArray_RemoveAdjacentDuplicates (ECX xyz, EDX count) -> new count (Point2_NearlyEqual 0.01).
int __fastcall PointArray_RemoveAdjacentDuplicates(float*, unsigned);
// 0x0046a620 Rect_OverlapMargin1 (ECX a, EDX b: minx, miny, maxx, maxy): overlap with a margin of 1.0 -> 1, else 0.
int __fastcall Rect_OverlapMargin1(const float*, const float*);
// 0x00464680 WeilerClip_Create
int __fastcall WeilerClip_Create(int ecx, int edx, int arg1);
// 0x00464790 WeilerClip_ResetRegion
int __fastcall WeilerClip_ResetRegion(int ecx, int edx, int arg1);
// 0x00464810 WeilerClip_Run
int __fastcall WeilerClip_Run(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00464b90 WeilerClip_Init
int __fastcall WeilerClip_Init(int ecx, int edx, int arg1, int arg2);
// 0x00464c90 WeilerClip_ClassifyBounds
int __fastcall WeilerClip_ClassifyBounds(int ecx, int edx);
// 0x00464ea0 WeilerClip_ClassifyResult
int __fastcall WeilerClip_ClassifyResult(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00464f70 WeilerClip_WeedOutCoincident
int __fastcall WeilerClip_WeedOutCoincident(int ecx, int edx);
// 0x00465ac0 WeilerClip_FindIntersections
int __fastcall WeilerClip_FindIntersections(int ecx, int edx);
// 0x004676c0 Contour_EnsureBuffer
int __fastcall Contour_EnsureBuffer(int ecx, int edx);
// 0x00467710 WeilerClip_MergeContours
int __fastcall WeilerClip_MergeContours(int ecx, int edx);
// 0x004680b0 WeilerClip_ClassifyContours
int __fastcall WeilerClip_ClassifyContours(int ecx, int edx);
// 0x004681a0 WeilerClip_OutputContours
int __fastcall WeilerClip_OutputContours(int ecx, int edx);
// 0x004682c0 ClipContour_Output
int __fastcall ClipContour_Output(int ecx, int edx, int arg1, int arg2);
// 0x00468470 WeilerClip_ComputeSideTables
int __fastcall WeilerClip_ComputeSideTables(int ecx, int edx);
// 0x00468580 ClipEdge_Divide
int __fastcall ClipEdge_Divide(int ecx, int edx, int arg1, int arg2);
// 0x00468650 ClipEdge_AddPair
int __fastcall ClipEdge_AddPair(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004687b0 WeilerClip_GenerateOutsideResult
int __fastcall WeilerClip_GenerateOutsideResult(int ecx, int edx);
// 0x00468c40 ClipSegments_Intersect2D
int __fastcall ClipSegments_Intersect2D(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12);
// 0x00468fa0 ClipSegments_Classify
int __fastcall ClipSegments_Classify(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004693c0 ClipNodes_BuildRing
int __fastcall ClipNodes_BuildRing(int ecx, int edx, int arg1, int arg2);
// 0x00469450 ClipEdge_ClassifyJoin
int __fastcall ClipEdge_ClassifyJoin(int ecx, int edx, int arg1);
// 0x00469560 ClipEdge_ClassifyTouch
int __fastcall ClipEdge_ClassifyTouch(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00469960 WeilerClip_TranslateToOrigin
int __fastcall WeilerClip_TranslateToOrigin(int ecx, int edx);
// 0x00469a30 WeilerClip_ResetWorkArrays
int __fastcall WeilerClip_ResetWorkArrays(int ecx, int edx);
// 0x00469af0 WeilerClip_TranslateBack
int __fastcall WeilerClip_TranslateBack(int ecx, int edx);
// 0x00469b60 WeilerClip_RestoreZFromPlane
int __fastcall WeilerClip_RestoreZFromPlane(int ecx, int edx);
// 0x00469d60 WeilerClip_FindCrossingVertex
int __fastcall WeilerClip_FindCrossingVertex(int ecx, int edx, int arg1);
// 0x0046a130 PointArray_SnapToReference
int __fastcall PointArray_SnapToReference(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x0046a1f0 ClipCrossings_Validate
int __fastcall ClipCrossings_Validate(int ecx, int edx, int arg1);
}  // namespace recoil
