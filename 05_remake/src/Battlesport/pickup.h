// SUBSYSTEM: pickup
// Declarations for src/Battlesport/pickup.cpp.
#pragma once

namespace recoil {

// 0x0041cca0
int __fastcall PickupTable_FreeImages(int ecx, int edx);
// 0x0041ceb0
int __fastcall Node_ClearPickupFlagsRecursive(int ecx, int edx);
// 0x0041cef0
int __fastcall Node_SetPickupFlagsRecursive(int ecx, int edx);
// 0x0041cf30
int __fastcall Pickup_ResolveOwnerNode(int ecx, int edx);
// 0x0041db40
int __fastcall PickupTable_EntryByIndexSignedLt40(int ecx, int edx);
// 0x0041dd60
int __fastcall PickupTable_IndexOfNameOut(int ecx, int edx);
// 0x0041ddf0
int __fastcall Pickups_ConfigFileForDifficulty(int ecx, int edx);
// 0x0041de30
int __fastcall Player_HasWeaponForPickup(int ecx, int edx);
// 0x0041e1c0
int __fastcall PickupTable_EntryByIndex(int ecx, int edx);
// 0x0041e1e0
int __fastcall PickupTable_IndexOfName(int ecx, int edx);
// 0x0041e430
int __fastcall PickupList_AnyNear(int ecx, int edx, int arg1);
// 0x0041e780
int __fastcall Pickup_BuildSaveRecords(int ecx, int edx);
// 0x0041e900
int __fastcall PickupList_ContainsSorted(int ecx, int edx);
// 0x0041e930
int __fastcall PickupList_FindById(int ecx, int edx);
// 0x0041e950
int __fastcall Pickup_GetField40(int ecx, int edx);
// 0x0041e960
int __fastcall Pickups_SwapGlobal_004f3330(int ecx, int edx);
// 0x0041e970
int __fastcall Pickups_GetGlobal_004f3330(int ecx, int edx);
// 0x0041e980
int __fastcall PickupTable_EntryForPrimaryWeapon(int ecx, int edx);
// 0x0041ea00
int __fastcall PickupTable_LookupByKey(int ecx, int edx);
// 0x0041e1a0
int __fastcall PickupTable_EntryByName(int ecx, int edx);
// 0x0041e270
int __fastcall LinkedList_FreeAll(int ecx, int edx);
// 0x0041e540
int __fastcall Pickup_WeaponSlotCode(int ecx, int edx);
// 0x0041e480
int __fastcall Pickup_CycleWeaponSelection(int ecx, int edx);
// 0x0041d920
int __fastcall PickupList_AddNode(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0041db60
int __fastcall Pickup_SetupObject(int ecx, int edx);
// 0x0041e6c0
int __fastcall Pickup_Respawn(int ecx, int edx);
// 0x0041d8a0
int __fastcall PickupList_RemoveNode(int ecx, int edx);
// 0x0041e5d0
int __fastcall Engine_TimedEventQueueTick(int ecx, int edx);
// 0x0041e240
int __fastcall PickupList_Clear(int ecx, int edx);
// 0x0041e2f0
int __fastcall PickupList_RemoveByType(int ecx, int edx);
// 0x0041ccd0
int __fastcall Pickups_ReleaseLists(int ecx, int edx);
// 0x0041ccf0
int __fastcall Pickups_Init(int ecx, int edx);
// 0x0041cf50
int __fastcall Pickup_Remove(int ecx, int edx, int arg1);
// 0x0041d0c0
int __fastcall Pickup_OnTouch(int ecx, int edx);
// 0x0041d220
int __fastcall Pickup_ApplyToPlayer(int ecx, int edx, int arg1);
// 0x0041d650
int __fastcall Pickup_GiveAmmo(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x0041da20
int __fastcall Pickup_Spawn(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0041dab0
int __fastcall Pickup_CreateObject(int ecx, int edx);
// 0x0041dc30
int __fastcall PickupSpawnRecord_Spawn(int ecx, int edx);
// 0x0041dc60
int __fastcall Pickup_SpawnWithAnim(int ecx, int edx);
// 0x0041de70
int __fastcall Pickups_SpawnAllFromConfig(int ecx, int edx);
// 0x0041e330
int __fastcall Pickup_SampleGroundLight(int ecx, int edx);
// 0x0041e840
int __fastcall Pickups_ReadSaveRecord(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0041e890
int __fastcall Pickups_SyncNetworkLists(int ecx, int edx);
// 0x0041ea30
int __fastcall Pickup_SpawnAtNamedNode(int ecx, int edx, int arg1);
}  // namespace recoil
