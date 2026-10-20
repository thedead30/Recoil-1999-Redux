// SUBSYSTEM: collision
// Declarations for src/GameZRecoil/zClass/cls_di.cpp.
#pragma once

namespace recoil {

// 0x00443c50
int __fastcall Collision_SetQueryMode(int ecx, int edx);
// 0x00443c60
int __fastcall Collision_SetQueryMask(int ecx, int edx);
// 0x00443c70
int __fastcall Collision_SurfaceUnderPoint(int ecx, int edx, int arg1);
// 0x00443d20
int __fastcall World_QueryObjectsAtPoint(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00443f80
int __fastcall Collision_QueryPointHierarchy(int ecx, int edx);
// 0x00444310
int __fastcall Collision_QueryPointClass8(int ecx, int edx);
// 0x004443e0
int __fastcall Collision_QueryPointLightNode(int ecx, int edx);
// 0x004444b0
int __fastcall World_QueryPointSet(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00444890
int __fastcall Collision_MultiPointHierarchy(int ecx, int edx, int arg1);
// 0x00444c50
int __fastcall Collision_MultiPointClass8(int ecx, int edx, int arg1);
// 0x00444d10
int __fastcall Collision_MultiPointLightNode(int ecx, int edx, int arg1);
// 0x00444de0
int __fastcall Collision_RaycastSegment(int ecx, int edx, int arg1, int arg2);
// 0x00444e90
int __fastcall Collision_GridDDATraversal(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x004455f0
int __fastcall Collision_TestChildNodes(int ecx, int edx);
// 0x00445650
int __fastcall Collision_TestNodeHierarchy(int ecx, int edx);
// 0x00445a00
int __fastcall Collision_TestUnusedNodeType8(int ecx, int edx);
// 0x00445b20
int __fastcall Collision_TestCameraNode(int ecx, int edx);
// 0x00445c20
int __fastcall Collision_TestLightNode(int ecx, int edx);
// 0x00445d40
int __fastcall World_QuerySegmentSet(int ecx, int edx, int arg1, int arg2);
// 0x00445f60
int __fastcall World_QuerySegmentSetGridWalk(int ecx, int edx);
// 0x00446440
int __fastcall Collision_MultiSegmentHierarchy(int ecx, int edx, int arg1);
// 0x00446880
int __fastcall Collision_MultiSegmentClass8(int ecx, int edx, int arg1);
// 0x00446970
int __fastcall Collision_MultiSegmentLightNode(int ecx, int edx, int arg1);
// 0x00446a80
int __fastcall Collision_FindTargetsInRadius(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00446f60
int __fastcall ClassDb_CollectIntersections(int ecx, int edx);
}  // namespace recoil
