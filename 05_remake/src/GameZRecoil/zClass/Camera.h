// SUBSYSTEM: camera
// Declarations for src/GameZRecoil/zClass/Camera.cpp.
#pragma once

namespace recoil {

// 0x0044a410
int __fastcall Camera_SetAspectRatio(int ecx, int edx, int arg1, int arg2);
// 0x0044a610
int __fastcall Camera_SetFOV(int ecx, int edx, int arg1, int arg2);
// 0x0044a250
int __fastcall Camera_GetWorldPosition(int ecx, int edx, int arg1, int arg2);
// 0x0044a580
int __fastcall Camera_GetAspectRatio(int ecx, int edx, int arg1);
// 0x0044a760
int __fastcall Camera_GetFOV(int ecx, int edx, int arg1);
// 0x00449c90
int __fastcall Camera_AttachChild(int ecx, int edx);
// 0x00449cd0
int __fastcall Camera_DetachChild(int ecx, int edx);
// 0x00449d20
int __fastcall Camera_SetDataFlagBit0(int ecx, int edx);
// 0x00449da0
int __fastcall Camera_SetGlobal_004ddd38(int ecx, int edx);
// 0x00449db0
int __fastcall Camera_SetGlobal_004ddd34(int ecx, int edx);
// 0x00449dc0
int __fastcall Camera_SetGlobal_004ddd10(int ecx, int edx);
// 0x00449dd0
int __fastcall Camera_SetWorld(int ecx, int edx);
// 0x00449e80
int __fastcall Camera_GetWorld(int ecx, int edx);
// 0x00449e90
int __fastcall Camera_SetData04_NoCheck(int ecx, int edx);
// 0x00449f50
int __fastcall Camera_PropagateChange(int ecx, int edx);
// 0x0044a060
int __fastcall Camera_GetData20Vec(int ecx, int edx, int arg1, int arg2);
// 0x0044a2f0
int __fastcall Camera_SetNearFar(int ecx, int edx, int arg1, int arg2);
// 0x0044a380
int __fastcall Camera_GetNearFar(int ecx, int edx, int arg1);
// 0x0044a7f0
int __fastcall Camera_GetD0(int ecx, int edx);
// 0x0044a870
int __fastcall Camera_SetD0AndInverseSquare(int ecx, int edx, int arg1);
// 0x0044a910
int __fastcall Camera_SetData08(int ecx, int edx);
// 0x0044a980
int __fastcall Camera_SetData0C(int ecx, int edx);
// 0x0044d260
int __fastcall Camera_SetRecord1E4(int ecx, int edx);
// 0x0044c1b0
int __fastcall Math_DiamondAngleXZ(int ecx, int edx);
// 0x00449be0
int __fastcall Camera_Create(int ecx, int edx);
// 0x00449ea0
int __fastcall Camera_SetLocalAngles(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00449fb0
int __fastcall Camera_AddLocalAngles(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044a0f0
int __fastcall Camera_SetPosition(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044a1a0
int __fastcall Camera_TranslatePosition(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044abf0
int __fastcall Camera_UpdateWorldViewAndListener(int ecx, int edx, int arg1);
// 0x0044c230
int __fastcall find_convex_hull(int ecx, int edx);
// 0x0044aa30
int __fastcall Camera_UpdateFrustumAndTransform(int ecx, int edx);
// 0x0044c3c0
int __fastcall SceneRender_BuildDiamondBuckets(int ecx, int edx, int arg1);
// 0x0044c8e0
int __fastcall SceneRender_BuildDiamondBucketsTiled(int ecx, int edx, int arg1);
// 0x0044a9f0
int __fastcall Camera_UpdateNodeValidated(int ecx, int edx);
// 0x0044ada0
int __fastcall Render_ProcessCameraNode(int ecx, int edx);
// 0x0044af60
int __fastcall Render_ProcessSoundNode(int ecx, int edx);
// 0x0044b140
int __fastcall Render_ProcessLightNode(int ecx, int edx);
// 0x0044b300
int __fastcall Render_ProcessObject3DNode(int ecx, int edx);
// 0x0044b710
int __fastcall Render_ProcessUnusedNodeType8(int ecx, int edx);
// 0x0044b8c0
int __fastcall Render_ProcessLodDistanceNode(int ecx, int edx);
// 0x0044bea0
int __fastcall Render_ProcessGroupNode(int ecx, int edx);
// 0x0044bfb0
int __fastcall Render_ProcessChildBitmaskNode(int ecx, int edx);
// 0x0044c0e0
int __fastcall Render_DispatchNodeByType(int ecx, int edx);
// 0x0044ce70
int __fastcall Render_CullAndQueueSceneObjects(int ecx, int edx, int arg1);
// 0x0044d200
int __fastcall Camera_DispatchChildrenMode2(int ecx, int edx);
// 0x0044d240
int __fastcall Camera_RenderScene(int ecx, int edx, int arg1);
}  // namespace recoil
