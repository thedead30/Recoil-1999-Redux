// SUBSYSTEM: player
// Declarations for src/Battlesport/player.cpp.
#pragma once

namespace recoil {

// 0x0041ef40
int __fastcall Player_ResetGlobals_004f3a58(int ecx, int edx);
// 0x0041efa0
int __fastcall Player_ApplyNodeStates(int ecx, int edx);
// 0x0041f6a0
int __fastcall Vehicle_BuildListSaveRecord(int ecx, int edx);
// 0x0041fe40
int __fastcall Player_AivConfigName(int ecx, int edx);
// 0x0041fe50
int __fastcall Player_VehicleConfigForDifficulty(int ecx, int edx);
// 0x00421d60
int __fastcall Node_AndFlags28Recursive(int ecx, int edx);
// 0x00421da0
int __fastcall Node_OrFlags28Recursive(int ecx, int edx);
// 0x00421de0
int __fastcall Node_OrFlags24Recursive(int ecx, int edx);
// 0x00421e20
int __fastcall Path_GetDirectoryOfFound(int ecx, int edx);
// 0x00423150
int __fastcall Name_StripNumericSuffix(int ecx, int edx);
// 0x00423380
int __fastcall Vehicle_IsMovementType9BCD(int ecx, int edx);
// 0x004233b0
int __fastcall Array10_CopyRange(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00423400
int __fastcall Array10_FillCopy(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00420be0
int __fastcall Movers_LoadFromConfig(int ecx, int edx);
// 0x0041ecd0
int __fastcall Player_RecordNodeState(int ecx, int edx);
// 0x0041fd20
int __fastcall Player_Destroy(int ecx, int edx);
// 0x004231b0
int __fastcall Hud_RefreshFromPlayer(int ecx, int edx);
// 0x0041fb80
int __fastcall Players_FreeAll(int ecx, int edx);
// 0x0041f010
int __fastcall Player_FillSaveRecordFields(int ecx, int edx);
// 0x0041f5f0
int __fastcall Player_BuildSaveRecord(int ecx, int edx);
// 0x0041ef60
int __fastcall Player_StaticInit_RegisterDtor_0041ef70(int ecx, int edx);
// 0x00421ed0
int __fastcall VehicleClass_ReadCollidePoints(int ecx, int edx);
// 0x004220f0
int __fastcall Vehicle_ReadSupportPoints(int ecx, int edx);
// 0x00422170
int __fastcall VehicleClass_LoadCommonMode(int ecx, int edx, int arg1);
// 0x004226d0
int __fastcall Vehicle_LoadClassPhysicsConfig(int ecx, int edx, int arg1);
// 0x00421470
int __fastcall Vehicle_BindModelParts(int ecx, int edx, int arg1, int arg2);
// 0x0041ec40
int __fastcall Player_StaticInit_Construct_004f37b0(int ecx, int edx);
// 0x0041ec60
int __fastcall Player_StaticInit_RegisterDtor_0041ec70(int ecx, int edx);
// 0x0041ec90
int __fastcall Player_StaticInit_Construct_004f33a8(int ecx, int edx);
// 0x0041ecb0
int __fastcall Player_StaticInit_RegisterDtor_0041ecc0(int ecx, int edx);
// 0x0041f1d0
int __fastcall Player_RestoreSaveRecord(int ecx, int edx);
// 0x0041f5b0
int __fastcall Player_RegisterSaveHandlers(int ecx, int edx);
// 0x00420c60
int __fastcall Movers_FlagNumberedNodes(int ecx, int edx);
// 0x00420d10
int __fastcall Vehicle_ResolveClassConfigAndNetAssignment(int ecx, int edx, int arg1);
// 0x00421790
int __fastcall Player_ResetVehiclePhysicsState(int ecx, int edx);
// 0x00421830
int __fastcall Vehicle_SnapToGround(int ecx, int edx);
// 0x00421a40
int __fastcall Player_InstantiateVehicleModel(int ecx, int edx);
// 0x004c9bf0
int __fastcall EH_Unwind_Player_InitPhysicsGlobalsAndVehicleClasses_0(int ecx, int edx);
// 0x004c9c10
int __fastcall EH_Unwind_Movers_FlagNumberedNodes_0(int ecx, int edx);
// 0x004c9c30
int __fastcall EH_Unwind_Vehicle_SpawnInstanceFromConfig_0(int ecx, int edx);
// 0x004c9bfb
int __fastcall EH_Handler_Player_InitPhysicsGlobalsAndVehicleClasses(int ecx, int edx);

// 0x004c9c18
int __fastcall EH_Handler_Movers_FlagNumberedNodes(int ecx, int edx);

// 0x004c9c3b
int __fastcall EH_Handler_Vehicle_SpawnInstanceFromConfig(int ecx, int edx);

// 0x0041fe90
int __fastcall Player_InitPhysicsGlobalsAndVehicleClasses(int ecx, int edx);
// 0x00421ab0
int __fastcall Vehicle_SpawnInstanceFromConfig(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00421ea0
int __fastcall Player_SpawnVehicleGetList(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
