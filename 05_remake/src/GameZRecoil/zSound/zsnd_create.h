// SUBSYSTEM: sound
// Declarations for src/GameZRecoil/zSound/zsnd_create.cpp.
#pragma once

namespace recoil {

// 0x004a4330
int __fastcall Sound_ReportDirectSoundError(int ecx, int edx, int arg1);
// 0x004a3e90
int __fastcall SoundVoiceList_IteratorPostIncrement(int ecx, int edx, int arg1, int arg2);
// 0x004a3620
int __fastcall SoundBuffer_GetPlayPosition(int ecx, int edx);
// 0x004a3690
int __fastcall SoundResource_ReleaseBuffers(int ecx, int edx);
// 0x004a3910
int __fastcall SoundVoiceSet_Destroy(int ecx, int edx);
// 0x004a3180
int __fastcall SoundBuffer_Create_DirectSound(int ecx, int edx);
// 0x004a3940
int __fastcall SoundFadeLists_StaticInit(int ecx, int edx);
// 0x004a3a80
int __fastcall SoundVoiceList_PushBack(int ecx, int edx);
// 0x004a3e50
int __fastcall SoundVoiceList_Erase(int ecx, int edx, int arg1, int arg2);
// 0x004a3ef0
int __fastcall Sound_ReportA3DError(int ecx, int edx, int arg1);
// 0x004a2ec0
int __fastcall SoundBuffer_Create_A3D(int ecx, int edx);
// 0x004a34e0
int __fastcall SoundBuffer_Lock(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004a3590
int __fastcall SoundBuffer_Unlock(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004a3ea0
int __fastcall SoundCD_ReportMCIError(int ecx, int edx, int arg1);
// 0x004a39b0
int __fastcall SoundFadeLists_StaticDtor(int ecx, int edx);
// 0x004a44e0
int __fastcall SoundNameTable_MatchNamePredicate(int ecx, int edx);
// 0x004a39a0
int __fastcall SoundFadeLists_RegisterStaticDtor(int ecx, int edx);
// 0x004a44c0
int __fastcall Sound_NameTable_Find(int ecx, int edx);
// 0x004a3930
int __fastcall SoundFadeLists_CrtInitStub(int ecx, int edx);
// 0x004a3ad0
int __fastcall SoundFade_Step(int ecx, int edx, int arg1);
// 0x004a3d20
int __fastcall SoundFade_Shutdown(int ecx, int edx);
// 0x004a3c20
int __fastcall SoundVoiceList_RemoveIf(int ecx, int edx, int arg1);
// 0x004a3850
int __fastcall SoundStream_CreateDefaultBuffer(int ecx, int edx, int arg1);
// 0x004cb1e0
int __fastcall EH_Unwind_SoundStream_CreateDefaultBuffer_0(int ecx, int edx);
// 0x004cb1e8
int __fastcall EH_Handler_SoundStream_CreateDefaultBuffer(int ecx, int edx);

}  // namespace recoil
