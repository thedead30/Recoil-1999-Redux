// SUBSYSTEM: sound
// Declarations for src/GameZRecoil/zSound/zsnd_init.cpp.
#pragma once

namespace recoil {

// 0x004a2020
int __fastcall SoundCDTrackList_StaticInit(int ecx, int edx);
// 0x004a1d10
int __fastcall Sound_InitA3DDevice(int ecx, int edx);
// 0x004a1f40
int __fastcall zSnd_ReleaseDevice(int ecx, int edx);
// 0x004a2050
int __fastcall SoundCDTrackList_RegisterStaticDtor(int ecx, int edx);
// 0x004a2010
int __fastcall SoundCDTrackList_CrtInitStub(int ecx, int edx);
// 0x004a1510
int __fastcall SoundConfig_ParseSyntax1(int ecx, int edx);
// 0x004a1870
int __fastcall SoundConfig_ParseSyntax2(int ecx, int edx);
// 0x004cb180
int __fastcall EH_Unwind_SoundConfig_ParseSyntax1_0(int ecx, int edx);
// 0x004cb1a0
int __fastcall EH_Unwind_SoundConfig_ParseSyntax2_0(int ecx, int edx);
// 0x004cb18b
int __fastcall EH_Handler_SoundConfig_ParseSyntax1(int ecx, int edx);

// 0x004cb1ab
int __fastcall EH_Handler_SoundConfig_ParseSyntax2(int ecx, int edx);

// 0x004a1420
int __fastcall zSnd_CreateDevice(int ecx, int edx);
// 0x004a1e50
int __fastcall zSnd_CreateDeviceAndPrimaryBuffer(int ecx, int edx);
}  // namespace recoil
