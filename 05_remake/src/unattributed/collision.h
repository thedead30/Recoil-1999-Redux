// SUBSYSTEM: collision
// Declarations for src/unattributed/collision.cpp.
#pragma once

namespace recoil {

// 0x00479c90
int __fastcall Collision_StoreLastHitUV(int ecx, int edx, int arg1, int arg2);
// 0x00485d10
int __fastcall Collision_RayPolygonTestWithUV(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x004472c0
int __fastcall Collision_NodeBoundsContainsPointXZ(int ecx, int edx);
// 0x004473e0
int __fastcall Collision_NodeBoundsMultiPointXZ(int ecx, int edx);
// 0x00479c80
int __fastcall Collision_UVQueryEnabled(int ecx, int edx);
// 0x00484b70
int __fastcall Collision_PointSetVsPolygon(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x004856d0
int __fastcall Collision_PointInPolygonXZ_Height(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00486290
int __fastcall Collision_SegmentSetVsPolygon(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00484960
int __fastcall Collision_MeshPointQueryXZ(int ecx, int edx, int arg1);
// 0x00484e00
int __fastcall Collision_MeshPointSetQuery(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004869a0
int __fastcall Collision_SegmentSetVsPolygonWithUV(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x00487540
int __fastcall Collision_SegmentSetVsTransformedBox(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004476f0
int __fastcall Collision_NodeBoundsMultiSegment(int ecx, int edx);
// 0x00487350
int __fastcall Collision_MeshSegmentSetQuery(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004857f0
int __fastcall Collision_RayPolygonTest(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00484fc0
int __fastcall Collision_TestMeshPolygons(int ecx, int edx, int arg1, int arg2);
// 0x00485380
int __fastcall Collision_RayVsTransformedBox(int ecx, int edx, int arg1, int arg2);
// 0x00447540
int __fastcall Collision_NodeBoundsReject(int ecx, int edx);
}  // namespace recoil
