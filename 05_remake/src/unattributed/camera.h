// SUBSYSTEM: camera
// Declarations for src/unattributed/camera.cpp.
#pragma once

namespace recoil {

// 0x00406450
int __fastcall Camera_SetViewVehicleToPlayer(int ecx, int edx);
// 0x0042b6e0
int __fastcall CollisionHits_Nearest(int ecx, int edx, int arg1, int arg2);
// 0x0047a0c0
int __fastcall Camera_BuildFrustumNormalTemplates(int ecx, int edx);
// 0x0047a1d0
int __fastcall Camera_RotateFrustumNormalsToWorld(int ecx, int edx);
// 0x00406430
int __fastcall Camera_ReleaseViewVehicleIfSinglePlayer(int ecx, int edx);
// 0x004063f0
int __fastcall Camera_Mode7_FollowAnimObject(int ecx, int edx);
// 0x00406510
int __fastcall Camera_MergeZoneSet(int ecx, int edx, int arg1);
// 0x004057d0
int __fastcall Camera_Mode2_Cockpit(int ecx, int edx);
// 0x00405870
int __fastcall Camera_Mode6_LookAtVehicle(int ecx, int edx);
// 0x004059a0
int __fastcall Camera_Mode3_Chase(int ecx, int edx);
// 0x00406890
int __fastcall Dlg_UpdateDataThenOKIfFloatChanged_Off64_00406890(int ecx, int edx);
// 0x004068c0
int __fastcall Dlg_UpdateDataThenOKIfFloatChanged_Off68_004068c0(int ecx, int edx);
// 0x004068f0
int __fastcall Dlg_UpdateDataThenOKIfFloatChanged_Off6C_004068f0(int ecx, int edx);
// 0x00406920
int __fastcall Dlg_OnSpinDeltaPos_Off64_00406920(int ecx, int edx, int arg1, int arg2);
// 0x00404e90
int __fastcall Camera_UpdateForVehicle(int ecx, int edx);
// 0x00405040
int __fastcall Camera_Mode1_Follow(int ecx, int edx);
// 0x00405ee0
int __fastcall Camera_SideProbePushout(int ecx, int edx, int arg1);
// 0x00406110
int __fastcall Camera_ResolveCollision(int ecx, int edx, int arg1, int arg2);
// 0x00406470
int __fastcall Camera_ClampAboveGround(int ecx, int edx);
// 0x00406610
int __fastcall Camera_UpdateOverlay4f3138(int ecx, int edx);
// 0x00406730
int __fastcall CollisionHits_FilterForCamera(int ecx, int edx);
// 0x004067a0
int __fastcall Camera_SubModeWaterCheck(int ecx, int edx);
// 0x00406960
int __fastcall DataTarget_00406960(int ecx, int edx, int arg1, int arg2);
// 0x004069a0
int __fastcall DataTarget_004069a0(int ecx, int edx, int arg1, int arg2);
// 0x004069e0
int __fastcall DataTarget_004069e0(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
