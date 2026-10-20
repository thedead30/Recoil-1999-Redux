// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/Sound.cpp.
#pragma once

namespace recoil {

// 0x00452b80
int __fastcall Sound_DetachChild(int ecx, int edx);
// 0x00452d00
int __fastcall Sound_SetVec30(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00452d60
int __fastcall Sound_GetVec30(int ecx, int edx, int arg1, int arg2);
// 0x00452ab0
int __fastcall Sound_Destroy(int ecx, int edx);
// 0x00452bc0
int __fastcall Sound_SetResourceName(int ecx, int edx);
// 0x00452c60
int __fastcall gwSoundNodeSetActive(int ecx, int edx);
// 0x004529c0
int __fastcall Sound_Create(int ecx, int edx);
// 0x00452dc0
int __fastcall gwSoundNodeProcessPlay(int ecx, int edx);
}  // namespace recoil
