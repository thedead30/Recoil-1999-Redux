// SUBSYSTEM: sound
// Declarations for src/unattributed/sound.cpp.
#pragma once

namespace recoil {

// 0x0040c1c0
int __fastcall Value_AssignDword(int ecx, int edx);
// 0x004a2ea0
int __fastcall SoundBuffer_Create_Dispatch(int ecx, int edx);
// 0x004a4530
int __fastcall Sound_BuildNameTable(int ecx, int edx);
// 0x004a2060
int __fastcall SoundCDTrackList_StaticDtor(int ecx, int edx);
// 0x004a13d0
int __fastcall zSnd_Shutdown(int ecx, int edx);
// 0x0042bf40
int __fastcall Sound_PlayPowerup(int ecx, int edx);
}  // namespace recoil
