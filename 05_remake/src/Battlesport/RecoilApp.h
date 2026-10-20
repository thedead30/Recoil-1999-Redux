// SUBSYSTEM: app
// Declarations for src/Battlesport/RecoilApp.cpp.
#pragma once

namespace recoil {

// 0x0042eb20
int __fastcall Screen_ApplyVideoRect(int ecx, int edx);
// 0x0042eb10
int __fastcall AppScreen_Slot8_Enable_0042eb10(int ecx, int edx);
// 0x0042eca0
int __fastcall AppScreen_Slot10_Disable_0042eca0(int ecx, int edx);
// 0x0042eb60
int __fastcall AppScreen_Call415650_0042eb60(int ecx, int edx);
// 0x0042e990
int __fastcall App_ActivateExistingInstance(int ecx, int edx);
// 0x0042ea20
int __fastcall App_StartIntroSequence(int ecx, int edx);
// 0x0042eac0
int __fastcall AppScreen_Tick_0042eac0(int ecx, int edx);
// 0x0042eb70
int __fastcall SeqScreenA_Ctor(int ecx, int edx);
// 0x0042ebd0
int __fastcall Screen_0042df10_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0042ebf0
int __fastcall App_StartAttractSequence(int ecx, int edx);
// 0x0042ec80
int __fastcall AppScreen_Slot10_Tick_0042ec80(int ecx, int edx);
// 0x004c9e70
int __fastcall EH_Unwind_SeqScreenA_Ctor_0(int ecx, int edx);
// 0x004c9e78
int __fastcall EH_Handler_SeqScreenA_Ctor(int ecx, int edx);

// 0x0042e430
int __fastcall App_ShutdownGame(int ecx, int edx);
// 0x0042e5da
int __fastcall RecoilApp_BootstrapAndCDCheck(int ecx, int edx);
// 0x0042e220
int __fastcall RecoilApp_PostSubsystemInit(int ecx, int edx, int arg1);
// 0x0042e330
int __fastcall RecoilApp_InitVideoAndHSEDevice(int ecx, int edx);
// 0x0042e490
int __fastcall Screen_LoadArchivesThenIdle(int ecx, int edx);
// 0x0042e4d0
int __fastcall App_StartMission(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
}  // namespace recoil
