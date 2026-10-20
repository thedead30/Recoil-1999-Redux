// SUBSYSTEM: turret
// Declarations for src/Battlesport/turret.cpp.
#pragma once

namespace recoil {

// 0x00436630
int __fastcall Turret_InitializeDefaults(int ecx, int edx);
// 0x00436e00
int __fastcall Turret_ReleaseSounds(int ecx, int edx);
// 0x00436e20
int __fastcall TurretTarget_IsValid(int ecx, int edx);
// 0x00437430
int __fastcall Turret_ComputeAimOrigin(int ecx, int edx);
// 0x00437990
int __fastcall Turret_TickReload(int ecx, int edx, int arg1);
// 0x00437aa0
int __fastcall Turret_ModuleReset(int ecx, int edx);
// 0x00437d40
int __fastcall Turrets_Release(int ecx, int edx);
// 0x00437e60
int __fastcall Node_SetOwnerAndFlagsRecursive(int ecx, int edx, int arg1);
// 0x00437ea0
int __fastcall Node_ApplyToMeshesRecursive(int ecx, int edx);
// 0x00437dc0
int __fastcall Turrets_FreeAll(int ecx, int edx);
// 0x00438020
int __fastcall Fader_Add(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00438180
int __fastcall TurretList_FreeAll(int ecx, int edx);
// 0x004381d0
int __fastcall Engine_TweenInterpolationTick(int ecx, int edx);
// 0x00437ab0
int __fastcall Turret_ModuleShutdown(int ecx, int edx);
// 0x004374a0
int __fastcall Turret_TrackTarget(int ecx, int edx, int arg1);
// 0x00437730
int __fastcall Turret_UpdateAimVector(int ecx, int edx, int arg1);
// 0x004367a0
int __fastcall Turret_Construct(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00437820
int __fastcall Turret_Fire(int ecx, int edx);
// 0x004379f0
int __fastcall Turret_ApplyHealthDamage(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00437d60
int __fastcall Turret_TakeDamage(int ecx, int edx, int arg1, int arg2);
// 0x004ca3d0
int __fastcall EH_Unwind_Turret_SpawnAllFromConfig_0(int ecx, int edx);
// 0x004ca3db
int __fastcall EH_Handler_Turret_SpawnAllFromConfig(int ecx, int edx);

// 0x00436e40
int __fastcall Turret_UpdateAITick(int ecx, int edx, int arg1);
// 0x00437ac0
int __fastcall Turret_SpawnAllFromConfig(int ecx, int edx);
// 0x00437d50
int __fastcall Turrets_ForEachNode(int ecx, int edx);
}  // namespace recoil
