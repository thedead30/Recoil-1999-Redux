// SUBSYSTEM: menus
// Declarations for src/Battlesport/Briefing.cpp.
#pragma once

namespace recoil {

// 0x004046b0
int __fastcall BriefStep_Show_Run(int ecx, int edx, int arg1);
// 0x00404740
int __fastcall BriefStep_FadeIn_Run(int ecx, int edx, int arg1);
// 0x00404850
int __fastcall BriefStep_SetText_Run(int ecx, int edx, int arg1);
// 0x00404960
int __fastcall BriefStep_FadeOut_Run(int ecx, int edx, int arg1);
// 0x00404140
int __fastcall Briefing_WaitKeyOrTimeout(int ecx, int edx);
// 0x004045b0
int __fastcall BriefQueue_AddStep_4cca84(int ecx, int edx, int arg1);
// 0x00404640
int __fastcall BriefQueue_AddStep_4cca88(int ecx, int edx, int arg1);
// 0x004046d0
int __fastcall BriefQueue_AddShowStep(int ecx, int edx, int arg1);
// 0x00404b40
int __fastcall BriefQueue_AddWaitStep(int ecx, int edx, int arg1);
// 0x00404070
int __fastcall Screen_TickSequenceAndChildren(int ecx, int edx, int arg1);
// 0x00404400
int __fastcall Briefing_BuildObjectiveSequence(int ecx, int edx, int arg1);
// 0x00404780
int __fastcall BriefQueue_AddTextStep(int ecx, int edx, int arg1, int arg2);
// 0x004048a0
int __fastcall BriefQueue_AddFadeInStep(int ecx, int edx, int arg1, int arg2);
// 0x004049d0
int __fastcall BriefQueue_AddSoundStep(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00404aa0
int __fastcall BriefStep_PlaySound_Run(int ecx, int edx, int arg1);
// 0x004c8328
int __fastcall EH_Unwind_BriefingScreen_Dtor_1(int ecx, int edx);
// 0x004c8336
int __fastcall EH_Unwind_BriefingScreen_Dtor_2(int ecx, int edx);
// 0x004c8344
int __fastcall EH_Unwind_BriefingScreen_Dtor_3(int ecx, int edx);
// 0x004c8352
int __fastcall EH_Unwind_BriefingScreen_Dtor_4(int ecx, int edx);
// 0x004c8360
int __fastcall EH_Unwind_BriefingScreen_Dtor_5(int ecx, int edx);
// 0x004c836e
int __fastcall EH_Unwind_BriefingScreen_Dtor_6(int ecx, int edx);
// 0x004c837c
int __fastcall EH_Unwind_BriefingScreen_Dtor_7(int ecx, int edx);
// 0x004c838a
int __fastcall EH_Unwind_BriefingScreen_Dtor_8(int ecx, int edx);
// 0x004c8398
int __fastcall EH_Unwind_BriefingScreen_Dtor_9(int ecx, int edx);
// 0x004c83b0
int __fastcall EH_Unwind_BriefingScreen_Dtor_10(int ecx, int edx);
// 0x004c83d0
int __fastcall EH_Unwind_Briefing_StartLoaderThread_0(int ecx, int edx);
// 0x004c83f0
int __fastcall EH_Unwind_BriefQueue_AddTextStep_0(int ecx, int edx);
// 0x004c8410
int __fastcall EH_Unwind_BriefQueue_AddFadeInStep_0(int ecx, int edx);
// 0x004c8430
int __fastcall EH_Unwind_BriefQueue_AddSoundStep_0(int ecx, int edx);
// 0x004c83db
int __fastcall EH_Handler_Briefing_StartLoaderThread(int ecx, int edx);

// 0x004c83fb
int __fastcall EH_Handler_BriefQueue_AddTextStep(int ecx, int edx);

// 0x004c841b
int __fastcall EH_Handler_BriefQueue_AddFadeInStep(int ecx, int edx);

// 0x004c843b
int __fastcall EH_Handler_BriefQueue_AddSoundStep(int ecx, int edx);

// 0x00403ed0
int __fastcall BriefingScreen_Dtor(int ecx, int edx);
// 0x00404180
int __fastcall Briefing_StartLoaderThread(int ecx, int edx);
// 0x004c8320
int __fastcall EH_Unwind_BriefingScreen_Dtor_0(int ecx, int edx);
// 0x004c83b8
int __fastcall EH_Handler_BriefingScreen_Dtor(int ecx, int edx);

}  // namespace recoil
