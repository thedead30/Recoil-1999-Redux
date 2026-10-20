// SUBSYSTEM: geometry
// Declarations for src/GameZRecoil/zGeometry/zgeo_model.cpp (original file GameZRecoil\zGeometry\zgeo_model.cpp,
// ledger orig_file). Spec: 04_spec/systems/geometry.md
#pragma once

#include <cstdint>

namespace recoil {

// 0x0053a750 triangle list (vertex-index triples, 1024), 0x0053d750 its count, 0x0053d758 plane a, b, c, d
extern std::uint32_t g_TriangleList_0053a750[3 * 1024];
extern std::uint32_t g_TriangleCount_0053d750;
extern float g_NewellPlane_0053d758[4];
// 0x0053a748 triangulator vertex array; 0x0053d754 word next to the triangle count
extern void* g_TriangulateVertices_0053a748;
extern std::uint32_t g_TriangulateWord_0053d754;

// 0x0046a9c0 Rect_FromPoints2D (ECX rect min/max, EDX 3-float points, stack count; ret 4): x/y bounding rectangle.
void __fastcall Rect_FromPoints2D(float*, const float*, int);
// 0x0046aab0 ClipPolygon_CopyOutRotated (ECX polygon, EDX out count, stack in/out buffer; ret 4): realloc the buffer,
int __fastcall ClipPolygon_CopyOutRotated(const void*, int*, float**);
// 0x0046ab10 ClipResult_Free (ECX result): frees [+0x4], destroys the clip at [+0x0], frees the result.
void __fastcall ClipResult_Free(void*);
// 0x0046af00 Alloc16Zeroed -> malloc(16) with four zero words.
void* Alloc16Zeroed();
// 0x0046af20 Alloc16_Free (ECX block): frees [+0xc] if any, then the block.
void __fastcall Alloc16_Free(void*);
// 0x0046b650 ModelPolygon_GatherPoints (ECX model, EDX polygon, stack buffer; ret 4): realloc to the polygon's point
void* __fastcall ModelPolygon_GatherPoints(const void*, const void*, void*);
// 0x0046bf30 EdgeList_FindUsingVertex (ECX vertex, EDX edge count, stack edges, stack out indices; ret 8) -> count.
int __fastcall EdgeList_FindUsingVertex(int, int, const void*, int*);
// 0x0046bf70 EdgeList_FindEdge (ECX a, EDX b, stack count, stack edges; ret 8) -> the edge, or null.
const void* __fastcall EdgeList_FindEdge(int, int, int, const void*);
// 0x0046c5b0 Vec3Array_ReverseKeepFirst (ECX count, EDX points): reverses points 1..count-1 in place.
void __fastcall Vec3Array_ReverseKeepFirst(int, float*);

// 0x0046a8e0 Triangle_SolveGradientXZ (ECX p1, EDX p0, stack p2, v1, v0, v2, out; ret 0x14): gradient of v over x/z; det 0 -> (0, 0).
void __fastcall Triangle_SolveGradientXZ(const float*, const float*, const float*, float, float, float, float*);
// 0x0046ab40 PointArray_Find2D (ECX array: points +4, count +8; EDX point) -> first index within 0.01, else -1.
int __fastcall PointArray_Find2D(const void*, const float*);
// 0x0046be20 Segment2_IntersectStrict (ECX a0, EDX a1, stack b0, b1; ret 8): 1 only when both parameters are strictly in (0, 1).
int __fastcall Segment2_IntersectStrict(const float*, const float*, const float*, const float*);
// 0x0046bfc0 Triangulate_EmitEar (ECX edge a, EDX edge b, stack apex, edge count, edges; ret 0xc): appends a triangle to
int __fastcall Triangulate_EmitEar(int, int, int, int, void*);
// 0x0046c3a0 Polygon_NewellPlaneFastSqrt (ECX count, EDX xyz, stack plane out a,b,c,d; ret 4): Newell normal scaled by
void __fastcall Polygon_NewellPlaneFastSqrt(int, const float*, float*);
// 0x0046c390 (ledger Geometry_Call46c3a0_Global53d758): Polygon_NewellPlaneFastSqrt into the global plane 0x0053d758.
void __fastcall Polygon_NewellPlaneToGlobal(int, const float*);
// 0x0046c570 Plane_SolveZForPoints (ECX count, EDX xyz): z = -(a*x + b*y + d) / c with the global plane (no zero check on c).
void __fastcall Plane_SolveZForPoints(int, float*);
// 0x0046c620 Triangle_EnsureCounterClockwise (ECX unused, EDX xyz of 3 vertices, stack fix; ret 4): 2D cross <= 0 -> fix ?
int __fastcall Triangle_EnsureCounterClockwise(int, float*, int);
// 0x0046aa40 ClipPolygon_Create
int __fastcall ClipPolygon_Create(int ecx, int edx);
// 0x0046ac80 ClipPolygon_FindEdgeContaining
int __fastcall ClipPolygon_FindEdgeContaining(int ecx, int edx);
// 0x0046ab90 ClipPolygon_MergePoints
int __fastcall ClipPolygon_MergePoints(int ecx, int edx, int arg1);
// 0x0046bd50 Triangulate_TryAddEdge
int __fastcall Triangulate_TryAddEdge(int ecx, int edx, int arg1);
// 0x0046c070 Triangulate_RingPair
int __fastcall Triangulate_RingPair(int ecx, int edx, int arg1, int arg2);
// 0x0046a7f0 ClipPolygon_ComputeUVs
int __fastcall ClipPolygon_ComputeUVs(int ecx, int edx, int arg1, int arg2);
// 0x0046b030 ClipModel_SnapToRegion
int __fastcall ClipModel_SnapToRegion(int ecx, int edx);
// 0x0046bb90 ClipModel_AgainstRegion
int __fastcall ClipModel_AgainstRegion(int ecx, int edx);
// 0x0046ae40
int __fastcall ClipGroupList_Release(int ecx, int edx);
// 0x0046bb30
int __fastcall ClipPolygon_EmitToModel(int ecx, int edx, int arg1);
// 0x0046ba90
int __fastcall Geometry_AddChildPolygon(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0046b6d0
int __fastcall ClipModel_SplitByRegion(int ecx, int edx, int arg1);
// 0x0046b550
int __fastcall ClipObject_Dispatch(int ecx, int edx, int arg1);
// 0x0046b1f0
int __fastcall AreaPartition_ClipObjects(int ecx, int edx, int arg1, int arg2);
// 0x0046af40
int __fastcall ClipGroup_BuildModelNode(int ecx, int edx, int arg1);
// 0x0046a770
int __fastcall Geometry_AddPolygonToModel(int ecx, int edx, int arg1, int arg2, int arg3);
}  // namespace recoil
