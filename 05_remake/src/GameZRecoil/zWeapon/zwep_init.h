// SUBSYSTEM: weapon
// Declarations for src/GameZRecoil/zWeapon/zwep_init.cpp.
#pragma once

namespace recoil {

// 0x004ae4a0
int __fastcall Weapon_SetGlobalPair0077895c(int ecx, int edx);
// 0x004b1160
int __fastcall Weapon_RefreshDamageAttributionCache(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004b1d80
int __fastcall Weapon_SetGlobal_00779a98(int ecx, int edx, int arg1);
// 0x004b21c0
int __fastcall Weapon_ResetLightSlot(int ecx, int edx);
// 0x004ae380
int __fastcall Weapon_LerpVec3PerAxis(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004b0530
int __fastcall Weapon_ComputeAimPitch(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004b0ba0
int __fastcall Weapon_TraceHit(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004b0ca0
int __fastcall Weapon_SortPointsAlongDirection(int ecx, int edx, int arg1);
// 0x004ae3c0
int __fastcall Weapon_FindMountByName(int ecx, int edx);
// 0x004ae450
int __fastcall Weapon_FindMountByField10(int ecx, int edx);
// 0x004b1f90
int __fastcall Weapon_FreeBeamInstance(int ecx, int edx);
// 0x004b25f0
int __fastcall Weapon_PropagateTargetTag(int ecx, int edx);
// 0x004b2670
int __fastcall Weapon_ClearTargetTag(int ecx, int edx);
// 0x004b2880
int __fastcall Weapon_RecordFirePoseAndDispatch(int ecx, int edx, int arg1);
// 0x004b1140
int __fastcall Weapon_BuildSubsystemSaveRecord(int ecx, int edx);
// 0x004b2570
int __fastcall Weapon_ReturnLightToPool(int ecx, int edx);
// 0x004b25a0
int __fastcall Weapon_TagTargetTree(int ecx, int edx, int arg1);
// 0x004b2630
int __fastcall Weapon_UntagTargetTree(int ecx, int edx);
// 0x004b26b0
int __fastcall Weapon_TagTargetTreeAlt(int ecx, int edx, int arg1);
// 0x004b1090
int __fastcall Weapon_InitSubsystem(int ecx, int edx);
// 0x004b2520
int __fastcall Weapon_TakeLightFromPool(int ecx, int edx);
// 0x004b2210
int __fastcall Weapon_UpdateLightSlot(int ecx, int edx, int arg1);
// 0x004b22d0
int __fastcall Weapon_ReleaseLightSlot(int ecx, int edx);
// 0x004b2300
int __fastcall Weapon_TickLightSlot(int ecx, int edx, int arg1);
// 0x004ae520
int __fastcall Weapon_ClearField18_Callback(int ecx, int edx, int arg1);
// 0x004b1fa0
int __fastcall Weapon_LoadImpactEntry(int ecx, int edx, int arg1);
// 0x004b0f70
int __fastcall Weapon_PlaceEffectNode(int ecx, int edx);
// 0x004b2130
int __fastcall Weapon_CreateBeamSegmentNode(int ecx, int edx);
// 0x004b21e0
int __fastcall Weapon_DestroyLightPool(int ecx, int edx);
// 0x004b1d90
int __fastcall Weapon_Shutdown(int ecx, int edx);
// 0x004b1ec0
int __fastcall Weapon_CreateBeamInstance(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004b2160
int __fastcall Weapon_CreateLightPool(int ecx, int edx);
// 0x004b1180
int __fastcall Weapon_Shutdown_Thunk(int ecx, int edx);
// 0x004ae4b0
int __fastcall Weapon_PopPooledNodeOrCreate(int ecx, int edx);
// 0x004ae4e0
int __fastcall Weapon_DetachAndPoolNode(int ecx, int edx);
// 0x004ae530
int __fastcall Weapon_TakeRecordFromPool(int ecx, int edx);
// 0x004ae590
int __fastcall Weapon_RecycleProjectileIfExpired(int ecx, int edx);
// 0x004ae660
int __fastcall Weapon_FireProjectile(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x004aeaa0
int __fastcall Weapon_SpawnProjectile(int ecx, int edx, int arg1);
// 0x004aeb50
int __fastcall Weapon_FreeProjectile(int ecx, int edx);
// 0x004aebc0
int __fastcall Weapon_FreeChainAt58(int ecx, int edx);
// 0x004aee40
int __fastcall Weapon_ActivateProjectile(int ecx, int edx);
// 0x004aefb0
int __fastcall Weapon_UnlinkActiveProjectile(int ecx, int edx);
// 0x004b0600
int __fastcall Weapon_PlaySound_Thunk1(int ecx, int edx);
// 0x004b0620
int __fastcall Weapon_PlaySound_Thunk2(int ecx, int edx);
// 0x004b0640
int __fastcall Weapon_PlaySound_Thunk3(int ecx, int edx);
// 0x004b0a50
int __fastcall Weapon_ApplyRadiusDamageFalloff(int ecx, int edx, int arg1, int arg2);
// 0x004b0e20
int __fastcall Weapon_BeamDamageTick(int ecx, int edx, int arg1, int arg2);
// 0x004b0fd0
int __fastcall Weapon_PlayRandomSoundA(int ecx, int edx, int arg1, int arg2);
// 0x004b1030
int __fastcall Weapon_PlayRandomSoundB(int ecx, int edx, int arg1, int arg2);
// 0x004b26f0
int __fastcall Weapon_ApplyDamageToTarget(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004b0660
int __fastcall Weapon_ApplyRadiusEffectA(int ecx, int edx, int arg1, int arg2);
// 0x004b0710
int __fastcall Weapon_ApplyRadiusEffectB(int ecx, int edx, int arg1, int arg2);
// 0x004aebf0
int __fastcall Weapon_DetonateProjectiles(int ecx, int edx, int arg1);
// 0x004aed00
int __fastcall Weapon_ProjectileDetonate(int ecx, int edx);
// 0x004af060
int __fastcall Weapon_ProjectileUpdateTick(int ecx, int edx);
// 0x004b07d0
int __fastcall Weapon_ProjectileImpact_ApplyDamage(int ecx, int edx, int arg1);
// 0x004b0980
int __fastcall Weapon_ImpactFromRecord(int ecx, int edx);
// 0x004b09d0
int __fastcall Weapon_IsPathClear(int ecx, int edx, int arg1, int arg2);
// 0x004b1190
int __fastcall Weapon_LoadMountArrayFromConfig(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
