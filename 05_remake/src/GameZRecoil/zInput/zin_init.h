// SUBSYSTEM: input
// Declarations for src/GameZRecoil/zInput/zin_init.cpp.
#pragma once

namespace recoil {

// 0x00471ab0
int __fastcall InputState_Ctor(int ecx, int edx);
// 0x004719f0
int __fastcall StaticInit_00561cb0(int ecx, int edx);
// 0x00471c10
int __fastcall zInShutdown(int ecx, int edx);
// 0x00471a20
int __fastcall InputState_ClearLists(int ecx, int edx);
// 0x00471a00
int __fastcall StaticInit_RegisterAtexit_00471a10(int ecx, int edx);
// 0x00471b50
int __fastcall zInInit(int ecx, int edx);
}  // namespace recoil
