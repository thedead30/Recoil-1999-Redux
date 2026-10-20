// SUBSYSTEM: sound
// Declarations for src/GameZRecoil/zSound/zsnd_parm.cpp.
#pragma once

namespace recoil {

// 0x0049fa00
int __fastcall Sound_LoadFloatArg(int ecx, int edx, int arg1);
// 0x004a1270
int __fastcall SoundResource_ClearBusy(int ecx, int edx);
// 0x004a0ec0
int __fastcall SoundBank_FindLoadedDescriptorByName(int ecx, int edx, int arg1);
// 0x0049f830
int __fastcall Sound_DuplicateHardwareBuffer(int ecx, int edx);
// 0x004a0300
int __fastcall SoundVoice_Snapshot(int ecx, int edx);
// 0x004a07f0
int __fastcall SoundArchive_SetEnabled(int ecx, int edx);
// 0x004a0810
int __fastcall SoundBankVector_StaticInit(int ecx, int edx);
// 0x004a08d0
int __fastcall SoundBankVector_At(int ecx, int edx);
// 0x004a0900
int __fastcall SoundBankVector_Size(int ecx, int edx);
// 0x004a0e90
int __fastcall Sound_Bank_GetDescriptorByIndex(int ecx, int edx, int arg1);
// 0x004a1090
int __fastcall Sound_SetMasterVolume(int ecx, int edx, int arg1);
// 0x004a10b0
int __fastcall Sound_ScaleMasterVolume(int ecx, int edx, int arg1);
// 0x004a10d0
int __fastcall Sound_SetGatedResourcesEnabled(int ecx, int edx);
// 0x004a1240
int __fastcall SoundResource_SetCallback(int ecx, int edx);
// 0x004a1290
int __fastcall Sound_SetAPIModePreInit(int ecx, int edx);
// 0x004a12b0
int __fastcall Sound_GetAPIMode(int ecx, int edx);
// 0x004a0400
int __fastcall SoundVoice_AdjustAndReplay_DS(int ecx, int edx, int arg1, int arg2);
// 0x004a0920
int __fastcall SoundBank_FindByName(int ecx, int edx);
// 0x004a12c0
int __fastcall zSndInit(int ecx, int edx);
// 0x004a0e40
int __fastcall SoundBank_ReleaseAllBuffers(int ecx, int edx);
// 0x004a0870
int __fastcall SoundBank_FindAndReleaseBuffers(int ecx, int edx);
// 0x004a0c00
int __fastcall SoundBank_Destroy(int ecx, int edx);
// 0x0049f9a0
int __fastcall Sound_VolumeToAttenuation(int ecx, int edx, int arg1);
// 0x004a05f0
int __fastcall SoundList_Destroy(int ecx, int edx);
// 0x004a07c0
int __fastcall SoundList_AllocNode(int ecx, int edx, int arg1, int arg2);
// 0x004a0880
int __fastcall SoundBankVector_Clear(int ecx, int edx);
// 0x004a10e0
int __fastcall SoundVoice_SetFrequencyRatio(int ecx, int edx, int arg1);
// 0x0049f6f0
int __fastcall Sound_DuplicateA3DVoice(int ecx, int edx);
// 0x0049fec0
int __fastcall Sound_StopAllVoicesForResource(int ecx, int edx);
// 0x004a0380
int __fastcall SoundVoice_AdjustAndReplay_A3D(int ecx, int edx, int arg1, int arg2);
// 0x0049f6d0
int __fastcall Sound_AllocateVoiceSlot(int ecx, int edx);
// 0x004a0490
int __fastcall SoundVoice_AdjustAndReplay(int ecx, int edx, int arg1, int arg2);
// 0x004a0590
int __fastcall SoundList_ApplyMasterVolume(int ecx, int edx);
// 0x004a07a0
int __fastcall Sound_IsMuted(int ecx, int edx);
// 0x004a1250
int __fastcall SoundResource_TrySetBusy(int ecx, int edx);
// 0x0049fa60
int __fastcall Sound_StartVoice_Mode1(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0049fbb0
int __fastcall Sound_StartVoice_DirectSound(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004a11d0
int __fastcall SoundVoice_SetGain(int ecx, int edx, int arg1);
// 0x0049fa10
int __fastcall Sound_StartVoice(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0049fd50
int __fastcall Sound_PlayResourceVariant(int ecx, int edx, int arg1, int arg2);
// 0x004a0840
int __fastcall SoundBankVector_StaticDtor(int ecx, int edx);
// 0x004a0830
int __fastcall SoundBankVector_RegisterStaticDtor(int ecx, int edx);
// 0x0049fda0
int __fastcall Sound_StopOrRestoreVoice(int ecx, int edx);
// 0x004a0800
int __fastcall SoundBankVector_CrtInitStub(int ecx, int edx);
// 0x004a0990
int __fastcall Sound_ResolveResourceByName(int ecx, int edx);
// 0x004a0500
int __fastcall SoundList_StopPlayingVoices(int ecx, int edx);
// 0x0049f620
int __fastcall Sound_UpdateFrame(int ecx, int edx);
// 0x0049f960
int __fastcall Sound_PlayResourceAuto(int ecx, int edx, int arg1);
// 0x0049fcf0
int __fastcall Sound_PlayResourceSimple(int ecx, int edx, int arg1, int arg2);
// 0x0049fff0
int __fastcall Sound_BuildPlayingVoiceList(int ecx, int edx);
// 0x004a0670
int __fastcall Sound_PushPopMute(int ecx, int edx);
// 0x004a0860
int __fastcall SoundBank_FindAndLoad(int ecx, int edx);
// 0x004a09e0
int __fastcall Sound_CreateBankAndRegisterProvider(int ecx, int edx, int arg1, int arg2);
// 0x004a0c40
int __fastcall SoundBank_Load(int ecx, int edx);
// 0x004a0fb0
int __fastcall SoundBank_LoadBuffers(int ecx, int edx, int arg1);
// 0x004cb120
int __fastcall EH_Unwind_Sound_BuildPlayingVoiceList_0(int ecx, int edx);
// 0x004cb140
int __fastcall EH_Unwind_SoundBank_Load_0(int ecx, int edx);
// 0x004cb148
int __fastcall EH_Unwind_SoundBank_Load_1(int ecx, int edx);
// 0x004cb160
int __fastcall EH_Unwind_SoundBank_LoadBuffers_0(int ecx, int edx);
// 0x004cb12b
int __fastcall EH_Handler_Sound_BuildPlayingVoiceList(int ecx, int edx);

// 0x004cb153
int __fastcall EH_Handler_SoundBank_Load(int ecx, int edx);

// 0x004cb16b
int __fastcall EH_Handler_SoundBank_LoadBuffers(int ecx, int edx);

}  // namespace recoil
