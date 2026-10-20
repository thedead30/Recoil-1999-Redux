// SUBSYSTEM: avi
// Declarations for src/GameZRecoil/zFMV/fmv_script.cpp.
#pragma once

namespace recoil {

// 0x00462660
int __fastcall Seq_ResetOrClear(int ecx, int edx, int arg1);
// 0x00462e30
int __fastcall Seq_RunBlocking(int ecx, int edx);
// 0x00462ed0
int __fastcall Seq_SetDuration_00462ed0(int ecx, int edx, int arg1, int arg2);
// 0x00462ee0
int __fastcall Seq_IsFinishedAt_00462ee0(int ecx, int edx, int arg1, int arg2);
// 0x00462f00
int __fastcall Seq_Present_00462f00(int ecx, int edx);
// 0x00462f10
int __fastcall Seq_AppendItem(int ecx, int edx, int arg1);
// 0x00463300
int __fastcall SeqItem_LoadImage(int ecx, int edx, int arg1, int arg2);
// 0x004633a0
int __fastcall SeqItem_FreeImage(int ecx, int edx);
// 0x004633c0
int __fastcall SeqFade_Ctor(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x00463410
int __fastcall SeqItem_SetTextParams(int ecx, int edx, int arg1, int arg2);
// 0x00463550
int __fastcall SeqItemB_FreeImage(int ecx, int edx);
// 0x00463850
int __fastcall SeqBlur_Ctor(int ecx, int edx, int arg1, int arg2);
// 0x00463870
int __fastcall SeqAvi_SetupScreen(int ecx, int edx, int arg1, int arg2);
// 0x00463920
int __fastcall Seq_SetFramebuffer_00463920(int ecx, int edx);
// 0x00463c90
int __fastcall VMethod_ReturnFalse8_00463c90(int ecx, int edx, int arg1, int arg2);
// 0x00462630
int __fastcall Seq_Dtor(int ecx, int edx);
// 0x00462e70
int __fastcall SeqBase_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00463120
int __fastcall Seq_Finish(int ecx, int edx, int arg1);
// 0x00463950
int __fastcall SeqFade_StepMode3(int ecx, int edx, int arg1, int arg2);
// 0x004639e0
int __fastcall SeqFade_StepMode1(int ecx, int edx, int arg1, int arg2);
// 0x00463a70
int __fastcall SeqFade_StepMode2(int ecx, int edx, int arg1, int arg2);
// 0x00463ca0
int __fastcall Seq_Stop_00463ca0(int ecx, int edx, int arg1, int arg2);
// 0x00463320
int __fastcall SeqFade_Draw_00463320(int ecx, int edx, int arg1, int arg2);
// 0x00463cc0
int __fastcall SeqItemC_Draw(int ecx, int edx);
// 0x00463440
int __fastcall SeqFade_Tick(int ecx, int edx, int arg1, int arg2);
// 0x004625e0
int __fastcall Seq_Ctor(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004626b0
int __fastcall Seq_Load(int ecx, int edx, int arg1, int arg2);
// 0x00462e90
int __fastcall SeqItem_PlaySound(int ecx, int edx, int arg1, int arg2);
// 0x00462f50
int __fastcall Seq_Play(int ecx, int edx, int arg1);
// 0x00462f90
int __fastcall Seq_Begin(int ecx, int edx, int arg1, int arg2);
// 0x00463000
int __fastcall Seq_Tick(int ecx, int edx, int arg1, int arg2);
// 0x004630a0
int __fastcall Seq_BeginAtNow(int ecx, int edx);
// 0x004630e0
int __fastcall Seq_TickAtNow(int ecx, int edx);
// 0x00463130
int __fastcall SeqImage_Ctor(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004631d0
int __fastcall SeqItemA_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x004631f0
int __fastcall SeqImage_CtorCentered(int ecx, int edx, int arg1, int arg2);
// 0x004632a0
int __fastcall SeqItemA_Dtor(int ecx, int edx);
// 0x00463570
int __fastcall SeqAvi_Ctor(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00463650
int __fastcall SeqItemB_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00463670
int __fastcall SeqItemB_Dtor(int ecx, int edx);
// 0x00463b00
int __fastcall SeqMci_Ctor(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00463bf0
int __fastcall SeqItemC_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00463c10
int __fastcall SeqItemC_Dtor(int ecx, int edx);
// 0x004cae60
int __fastcall EH_Unwind_Seq_Load_0(int ecx, int edx);
// 0x004cae6b
int __fastcall EH_Unwind_Seq_Load_1(int ecx, int edx);
// 0x004cae76
int __fastcall EH_Unwind_Seq_Load_2(int ecx, int edx);
// 0x004cae81
int __fastcall EH_Unwind_Seq_Load_3(int ecx, int edx);
// 0x004cae8c
int __fastcall EH_Unwind_Seq_Load_4(int ecx, int edx);
// 0x004cae97
int __fastcall EH_Unwind_Seq_Load_5(int ecx, int edx);
// 0x004caea2
int __fastcall EH_Unwind_Seq_Load_6(int ecx, int edx);
// 0x004caead
int __fastcall EH_Unwind_Seq_Load_7(int ecx, int edx);
// 0x004caeb8
int __fastcall EH_Unwind_Seq_Load_8(int ecx, int edx);
// 0x004caec3
int __fastcall EH_Unwind_Seq_Load_9(int ecx, int edx);
// 0x004caece
int __fastcall EH_Unwind_Seq_Load_10(int ecx, int edx);
// 0x004caed9
int __fastcall EH_Unwind_Seq_Load_11(int ecx, int edx);
// 0x004caee4
int __fastcall EH_Unwind_Seq_Load_12(int ecx, int edx);
// 0x004caf00
int __fastcall EH_Unwind_SeqImage_Ctor_0(int ecx, int edx);
// 0x004caf20
int __fastcall EH_Unwind_SeqImage_CtorCentered_0(int ecx, int edx);
// 0x004caf40
int __fastcall EH_Unwind_SeqItemA_Dtor_0(int ecx, int edx);
// 0x004caf60
int __fastcall EH_Unwind_SeqAvi_Ctor_0(int ecx, int edx);
// 0x004caf80
int __fastcall EH_Unwind_SeqItemB_Dtor_0(int ecx, int edx);
// 0x004cafa0
int __fastcall EH_Unwind_SeqAvi_Begin_0(int ecx, int edx);
// 0x004cafc0
int __fastcall EH_Unwind_SeqMci_Ctor_0(int ecx, int edx);
// 0x004cafc8
int __fastcall EH_Unwind_SeqMci_Ctor_1(int ecx, int edx);
// 0x004cafe0
int __fastcall EH_Unwind_SeqItemC_Dtor_0(int ecx, int edx);
// 0x004caeec
int __fastcall EH_Handler_Seq_Load(int ecx, int edx);

// 0x004caf08
int __fastcall EH_Handler_SeqImage_Ctor(int ecx, int edx);

// 0x004caf28
int __fastcall EH_Handler_SeqImage_CtorCentered(int ecx, int edx);

// 0x004caf48
int __fastcall EH_Handler_SeqItemA_Dtor(int ecx, int edx);

// 0x004caf68
int __fastcall EH_Handler_SeqAvi_Ctor(int ecx, int edx);

// 0x004caf88
int __fastcall EH_Handler_SeqItemB_Dtor(int ecx, int edx);

// 0x004cafab
int __fastcall EH_Handler_SeqAvi_Begin(int ecx, int edx);

// 0x004cafd3
int __fastcall EH_Handler_SeqMci_Ctor(int ecx, int edx);

// 0x004cafe8
int __fastcall EH_Handler_SeqItemC_Dtor(int ecx, int edx);

// 0x004636d0
int __fastcall SeqAvi_ShowFrame_004636d0(int ecx, int edx, int arg1, int arg2);
// 0x00463790
int __fastcall SeqAvi_Begin(int ecx, int edx, int arg1, int arg2);
// 0x00463820
int __fastcall SeqAvi_FreePlayer(int ecx, int edx);
}  // namespace recoil
