// SUBSYSTEM: player
// Declarations for src/unattributed/player.cpp.
#pragma once

namespace recoil {

// 0x00424010
int __fastcall ListNode_PickByXZDot(int ecx, int edx);
// 0x004248e0
int __fastcall VehicleCtrl_HandleList488_Collide(int ecx, int edx);
// 0x00424bf0
int __fastcall Vec3_EnforceMinLength(int ecx, int edx);
// 0x00425770
int __fastcall Vehicle_BlockMotion(int ecx, int edx);
// 0x0042aa40
int __fastcall PlayerList_GetHead(int ecx, int edx);
// 0x00438920
int __fastcall NameRegistry_Add(int ecx, int edx);
// 0x00424c90
int __fastcall Segment_ClipToHit(int ecx, int edx);
// 0x0041eb00
int __fastcall Player_StaticInit_Construct_004f3778(int ecx, int edx);
// 0x0041eb60
int __fastcall Player_StaticInit_Construct_004f3650(int ecx, int edx);
// 0x00423530
int __fastcall Vehicle_ClearControllerLists(int ecx, int edx);
// 0x00424d00
int __fastcall VehicleCtrl_HandleList4c8(int ecx, int edx);
// 0x00413600
int __fastcall Hud_ApplyTypeByMode_00413600(int ecx, int edx);
// 0x0041bd10
int __fastcall Player_Slot_ClearField594_0041bd10(int ecx, int edx);
// 0x0041eb20
int __fastcall AtexitStub_Widget_BaseDtor_0041eb20(int ecx, int edx);
// 0x0041eb80
int __fastcall AtexitStub_Widget_BaseDtor_0041eb80(int ecx, int edx);
// 0x0041ef70
int __fastcall Player_ClearRecordVector(int ecx, int edx);
// 0x00437ef0
int __fastcall Video_ReapplyModeByDriverType_00437ef0(int ecx, int edx);
// 0x0041eb10
int __fastcall Player_StaticInit_RegisterDtor_0041eb20(int ecx, int edx);
// 0x0041eb70
int __fastcall Player_StaticInit_RegisterDtor_0041eb80(int ecx, int edx);
// 0x0041eaf0
int __fastcall StaticInitWrapper_0041eaf0(int ecx, int edx);
// 0x0041eb50
int __fastcall StaticInitWrapper_0041eb50(int ecx, int edx);
// 0x0041ef30
int __fastcall StaticInitWrapper_0041ef30(int ecx, int edx);
// 0x004143d0
int __fastcall Net_SetupTableLoops_004143d0(int ecx, int edx);
// 0x0041bb30
int __fastcall Player_ResetForDeathAnim(int ecx, int edx, int arg1);
// 0x0041ec30
int __fastcall StaticInitWrapper_0041ec30(int ecx, int edx);
// 0x0041ec70
int __fastcall AtexitStub_ListLabel_Dtor_0041ec70(int ecx, int edx);
// 0x0041ecc0
int __fastcall AtexitStub_ListLabel_Dtor_0041ecc0(int ecx, int edx);
// 0x0041f640
int __fastcall Player_Slot_0041f640(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004236b0
int __fastcall Vehicle_SweepCollisionPoints(int ecx, int edx);
// 0x00423b10
int __fastcall Vehicle_TestSweepSegments(int ecx, int edx, int arg1, int arg2);
// 0x00423c20
int __fastcall Vehicle_ClassifySweepHit(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00423fc0
int __fastcall VehicleCtrl_HandleList498(int ecx, int edx);
// 0x00424110
int __fastcall VehicleCtrl_HandleList488(int ecx, int edx);
// 0x00424150
int __fastcall Pickup_HasLineOfSight(int ecx, int edx);
// 0x00424270
int __fastcall Vehicle_ResolveSurfaceHit(int ecx, int edx);
// 0x00424ac0
int __fastcall Vehicle_ResolveVehicleContact(int ecx, int edx);
// 0x00424ed0
int __fastcall Vehicle_TestStationaryPoints(int ecx, int edx);
// 0x00425060
int __fastcall Node_GetNumericSuffixIfFlagged(int ecx, int edx);
// 0x004251f0
int __fastcall Vehicle_TestFootprintAtHeight(int ecx, int edx, int arg1);
// 0x00425a20
int __fastcall Vehicle_ReadPlayerControlInput(int ecx, int edx);
// 0x0043b660
int __fastcall Player_ApplyHealthPickup(int ecx, int edx, int arg1);
// 0x004c9c50
int __fastcall EH_Unwind_Node_GetNumericSuffixIfFlagged_0(int ecx, int edx);
// 0x004c9c58
int __fastcall EH_Unwind_Node_GetNumericSuffixIfFlagged_1(int ecx, int edx);
// 0x004c9c70
int __fastcall EH_Unwind_Player_RegisterResources_0(int ecx, int edx);
// 0x004c9c60
int __fastcall EH_Handler_Node_GetNumericSuffixIfFlagged(int ecx, int edx);

// 0x004c9c7b
int __fastcall EH_Handler_Player_RegisterResources(int ecx, int edx);

// 0x0041bbf0
int __fastcall Player_RespawnReset(int ecx, int edx, int arg1);
// 0x0041bca0
int __fastcall Player_Slot_0041bca0(int ecx, int edx);
// 0x0041f850
int __fastcall Player_LoadVehicleFromSaveRecord(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00423460
int __fastcall Vehicle_UpdateControllers(int ecx, int edx);
// 0x00424210
int __fastcall VehicleCtrl_CollectPickups(int ecx, int edx);
// 0x00425150
int __fastcall Race_OnCheckpointHit(int ecx, int edx);
// 0x0041ec80
int __fastcall StaticInit_Player_004f33a8(int ecx, int edx);
// 0x00425920
int __fastcall Player_RegisterResources(int ecx, int edx);
}  // namespace recoil
