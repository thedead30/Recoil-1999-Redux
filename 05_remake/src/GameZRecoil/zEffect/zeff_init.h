// SUBSYSTEM: zeffect
// Declarations for src/GameZRecoil/zEffect/zeff_init.cpp.
#pragma once

namespace recoil {

// 0x00460470
int __fastcall EffectList_Count(int ecx, int edx);
// 0x004603d0
int __fastcall EffectList_Free(int ecx, int edx);
// 0x00460400
int __fastcall EffectList_Contains(int ecx, int edx);
// 0x00460070
int __fastcall EffectDefs_LoadOnce(int ecx, int edx, int arg1);
// 0x00460330
int __fastcall EffectConfig_Release(int ecx, int edx);
}  // namespace recoil
