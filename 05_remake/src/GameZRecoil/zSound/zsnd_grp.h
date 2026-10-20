// SUBSYSTEM: sound
// Declarations for src/GameZRecoil/zSound/zsnd_grp.cpp.
#pragma once

namespace recoil {

// 0x004a5020
int __fastcall Sound_StreamCue_State3_Cooldown(int ecx, int edx);
// 0x004a4d10
int __fastcall Sound_StreamCue_PickWeightedRandomVariant(int ecx, int edx);
// 0x004a4cb0
int __fastcall Sound_StreamCue_State0_SelectVariant(int ecx, int edx);
// 0x004a4fd0
int __fastcall Sound_StreamCue_State2_ReplayGapWait(int ecx, int edx);
// 0x004a55c0
int __fastcall SoundFile_Unload(int ecx, int edx);
// 0x004a5460
int __fastcall Wav_ParseRiffChunks(int ecx, int edx);
// 0x004a5440
int __fastcall SoundFile_Destroy(int ecx, int edx);
// 0x004a5540
int __fastcall SoundFile_LoadFromDisk(int ecx, int edx);
// 0x004a5600
int __fastcall SoundFile_LoadFromArchive(int ecx, int edx, int arg1);
// 0x004a53f0
int __fastcall NamedRecord_Construct(int ecx, int edx, int arg1, int arg2);
// 0x004a49b0
int __fastcall SoundGroup_ParseVariantNode(int ecx, int edx, int arg1);
// 0x004a50a0
int __fastcall SoundStreamCue_Shutdown(int ecx, int edx);
// 0x004a4590
int __fastcall SoundGroup_ParseNode(int ecx, int edx);
// 0x004a51e0
int __fastcall Container_MatchPointerPredicate(int ecx, int edx);
// 0x004a5220
int __fastcall SoundStreamCue_MatchResourcePredicate(int ecx, int edx);
// 0x004a51f0
int __fastcall Sound_StreamCue_RetireByPredicate(int ecx, int edx);
// 0x004a4c40
int __fastcall Sound_StreamCue_TickDispatch(int ecx, int edx);
// 0x004a4ea0
int __fastcall Sound_StreamCue_State1_PlaySequenceEvents(int ecx, int edx);
// 0x004a5050
int __fastcall Sound_ServiceActiveStreamCues(int ecx, int edx);
// 0x004a5230
int __fastcall Sound_StartStreamResourceSimple(int ecx, int edx, int arg1);
// 0x004a5250
int __fastcall Sound_StartStreamResource(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004a5350
int __fastcall Sound_StreamCueSubsystem_LazyInit(int ecx, int edx);
// 0x004a53d0
int __fastcall Sound_StartStreamResourceAtPosition(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
