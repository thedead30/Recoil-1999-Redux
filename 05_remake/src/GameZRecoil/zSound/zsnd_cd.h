// SUBSYSTEM: sound
// Declarations for src/GameZRecoil/zSound/zsnd_cd.cpp.
#pragma once

namespace recoil {

// 0x004a2490
int __fastcall SoundCD_ResetTrackState(int ecx, int edx);
// 0x004a27d0
int __fastcall SoundCD_HasVolumeControl(int ecx, int edx);
// 0x004a2930
int __fastcall SoundCD_GetTrackCount(int ecx, int edx);
// 0x004a2e70
int __fastcall Sound_GetSpeedOfSound(int ecx, int edx);
// 0x004a2e80
int __fastcall Sound_SetSpeedOfSound(int ecx, int edx, int arg1);
// 0x004a2950
int __fastcall Sound_SetListenerFromMatrix(int ecx, int edx);
// 0x004a2600
int __fastcall SoundCD_Play(int ecx, int edx);
// 0x004a26f0
int __fastcall SoundCD_Stop(int ecx, int edx);
// 0x004a2750
int __fastcall SoundCD_PlayTrack(int ecx, int edx);
// 0x004a27f0
int __fastcall SoundCD_GetVolume(int ecx, int edx);
// 0x004a2880
int __fastcall SoundCD_SetVolume(int ecx, int edx);
// 0x004a24d0
int __fastcall SoundCD_Shutdown(int ecx, int edx);
// 0x004a25e0
int __fastcall SoundCD_PlayIfPrepared(int ecx, int edx);
// 0x004a26b0
int __fastcall SoundCD_OnMciNotify(int ecx, int edx);
// 0x004a2a70
int __fastcall Sound_Set3DParams_A3D(int ecx, int edx, int arg1, int arg2);
// 0x004a2b40
int __fastcall Sound_Set3DParams_DirectSound(int ecx, int edx, int arg1, int arg2);
// 0x004a2a30
int __fastcall Sound_Set3DParams_Dispatch(int ecx, int edx, int arg1, int arg2);
// 0x004a20d0
int __fastcall SoundCD_OpenDevice(int ecx, int edx);
// 0x004cb1c0
int __fastcall EH_Unwind_SoundCD_OpenDevice_0(int ecx, int edx);
// 0x004cb1cb
int __fastcall EH_Handler_SoundCD_OpenDevice(int ecx, int edx);

}  // namespace recoil
