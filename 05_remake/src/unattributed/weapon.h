// SUBSYSTEM: weapon
// Declarations for src/unattributed/weapon.cpp.
#pragma once

namespace recoil {

// 0x0043c630
int __fastcall Weapon_IsUsableByClass(int ecx, int edx);
// 0x004b2910
int __fastcall Weapon_GetGlobalBlock00778940(int ecx, int edx);
// 0x004b2920
int __fastcall Weapon_GetGlobal00778958(int ecx, int edx);
// 0x004b2930
int __fastcall Weapon_ProjectileIterBegin(int ecx, int edx);
// 0x004b2940
int __fastcall Weapon_ProjectileIterNext(int ecx, int edx);
// 0x0043a4f0
int __fastcall Weapon_BuildAimNodeMatrices(int ecx, int edx, int arg1);
// 0x0043aa30
int __fastcall Weapon_ComputePrimaryMuzzle(int ecx, int edx);
// 0x0043b1b0
int __fastcall Weapon_ComposeMountMatrix(int ecx, int edx);
// 0x0043b3e0
int __fastcall Weapon_ComputeMuzzleWorldPos(int ecx, int edx);
// 0x0043c800
int __fastcall Weapon_ResetBarrelScale(int ecx, int edx);
// 0x004383e0
int __fastcall PlayerEntry_Construct(int ecx, int edx);
// 0x004384e0
int __fastcall PlayerEntry_AddMount(int ecx, int edx);
// 0x00438b60
int __fastcall Vehicle_StopWeaponSounds(int ecx, int edx);
// 0x0043c9c0
int __fastcall Weapon_FindMountSlotByType(int ecx, int edx);
// 0x004390d0
int __fastcall Vehicle_BindGunPoints(int ecx, int edx);
// 0x0043cc70
int __fastcall Weapon_BuildMineSaveRecords(int ecx, int edx);
// 0x0043acf0
int __fastcall Weapon_ComputeSecondaryMuzzle(int ecx, int edx);
// 0x00438430
int __fastcall PlayerEntry_Destruct(int ecx, int edx);
// 0x0043a600
int __fastcall Weapon_UpdateAimDirection(int ecx, int edx);
// 0x0043a900
int __fastcall Weapon_DecayRecoilAngle(int ecx, int edx, int arg1, int arg2);
// 0x0043ca20
int __fastcall Weapon_MountInit_LoadKillVerb(int ecx, int edx);
// 0x0043ca90
int __fastcall Weapon_IsAvailableForLevel(int ecx, int edx, int arg1, int arg2);
// 0x0043a980
int __fastcall Weapon_ClearChargeTimers(int ecx, int edx);
// 0x00438660
int __fastcall PlayerEntry_StopSlotSound(int ecx, int edx, int arg1);
// 0x00439600
int __fastcall Player_SelectSecondaryGroup(int ecx, int edx, int arg1);
// 0x004385a0
int __fastcall PlayerEntry_SetSlotSound(int ecx, int edx, int arg1, int arg2);
// 0x004385f0
int __fastcall PlayerEntry_PlaySlotSound(int ecx, int edx, int arg1, int arg2);
// 0x00438630
int __fastcall PlayerEntry_EnsureSlotSound(int ecx, int edx, int arg1, int arg2);
// 0x00438ba0
int __fastcall PlayerEntry_InitWeaponMounts(int ecx, int edx);
// 0x00439260
int __fastcall Player_SelectPrimaryWeapon(int ecx, int edx);
// 0x00439460
int __fastcall Player_SelectSecondaryAfterPickup(int ecx, int edx);
// 0x00439540
int __fastcall Player_SelectPrimaryGroup(int ecx, int edx, int arg1);
// 0x0043a400
int __fastcall Weapon_FireSecondary(int ecx, int edx);
// 0x0043c2d0
int __fastcall Weapon_StartBeam(int ecx, int edx);
// 0x0043c330
int __fastcall Weapon_SpawnProjectileFrom(int ecx, int edx, int arg1);
// 0x0043c550
int __fastcall Weapon_SpawnPrimaryProjectile(int ecx, int edx);
// 0x0043c660
int __fastcall Weapon_AutoSelectNextUsable(int ecx, int edx);
// 0x0043cdf0
int __fastcall Weapon_Slot_IterateMounts_0043cdf0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043cf20
int __fastcall Thunk_0043cf30_0043cf20(int ecx, int edx);
// 0x0043cf30
int __fastcall StaticInit_Empty_0043cf30(int ecx, int edx);
// 0x00439ba0
int __fastcall Weapon_UpdateDeployAnimState(int ecx, int edx);
// 0x0043c190
int __fastcall Weapon_FirePrimary(int ecx, int edx);
// 0x0043c430
int __fastcall Weapon_SpawnHomingProjectile(int ecx, int edx);
// 0x0043c950
int __fastcall Weapon_DetonateMountedProjectiles(int ecx, int edx);
// 0x0043ce80
int __fastcall Net_CheckWinsock20_0043ce80(int ecx, int edx);
}  // namespace recoil
