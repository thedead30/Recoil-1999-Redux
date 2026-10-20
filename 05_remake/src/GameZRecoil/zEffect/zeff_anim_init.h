// SUBSYSTEM: zeffect
// Declarations for src/GameZRecoil/zEffect/zeff_anim_init.cpp.
#pragma once

namespace recoil {

// 0x0045e200
int __fastcall SetGlobal_00575db8(int ecx, int edx);
// 0x0045e270
int __fastcall SetGlobal_004df730(int ecx, int edx);
// 0x00460010
int __fastcall Effect_GetField40(int ecx, int edx);
// 0x0045e280
int __fastcall AnimNode_FindSoundByName(int ecx, int edx);
// 0x0045e300
int __fastcall AnimNode_FindLightByName(int ecx, int edx);
// 0x0045ff10
int __fastcall Anim_FindByName(int ecx, int edx);
// 0x0045ffa0
int __fastcall AnimNode_NextWithFlag10(int ecx, int edx);
// 0x0045e210
int __fastcall AnimData_SetZbdName(int ecx, int edx);
// 0x0045e650
int __fastcall Node_FindByNameRecursive(int ecx, int edx);
// 0x0045e5c0
int __fastcall AnimNode_ResolveTargetByName(int ecx, int edx);
// 0x0045ed80
int __fastcall AnimInstance_BindNode(int ecx, int edx);
// 0x0045fd10
int __fastcall AnimInstance_Free(int ecx, int edx);
// 0x0045fe50
int __fastcall AnimData_FreeAll(int ecx, int edx);
// 0x0045fef0
int __fastcall EffectList_FreeIfLoaded(int ecx, int edx);
// 0x0045e380
int __fastcall AnimNode_AddSoundEntry(int ecx, int edx);
// 0x0045e4a0
int __fastcall AnimNode_AddLightEntry(int ecx, int edx);
// 0x0045e100
int __fastcall Anim_ResetGlobals(int ecx, int edx);
// 0x0045e6d0
int __fastcall AnimNode_EnsureModel(int ecx, int edx);
// 0x0045e730
int __fastcall AnimInstance_Clone(int ecx, int edx);
// 0x0045efb0
int __fastcall Anim_LoadFile(int ecx, int edx);
// 0x0045fb30
int __fastcall Anim_InitAllOnce(int ecx, int edx);
// 0x00460020
int __fastcall EffectConfig_Reset(int ecx, int edx);
}  // namespace recoil
