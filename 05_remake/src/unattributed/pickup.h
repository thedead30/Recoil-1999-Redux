// SUBSYSTEM: pickup
// Declarations for src/unattributed/pickup.cpp.
#pragma once

namespace recoil {

// 0x00438a70
int __fastcall PickupCrate_GetWorldPos(int ecx, int edx);
// 0x00438a20
int __fastcall PickupCrate_CanDropAt(int ecx, int edx, int arg1);
// 0x0041dcf0
int __fastcall Pickup_Reposition_0041dcf0(int ecx, int edx, int arg1);
// 0x004389c0
int __fastcall PickupCrate_TryDrop(int ecx, int edx, int arg1);
// 0x00438b30
int __fastcall PickupCrate_TryDropIfClear(int ecx, int edx);
// 0x0041ea90
int __fastcall Thunk_0041eaa0_0041ea90(int ecx, int edx);
// 0x0041eac0
int __fastcall Thunk_0041ead0_0041eac0(int ecx, int edx);
// 0x0041eaa0
int __fastcall StaticInit_ZeroList_004f3a68(int ecx, int edx);
// 0x0041ead0
int __fastcall StaticInit_ZeroList_004f3688(int ecx, int edx);
}  // namespace recoil
