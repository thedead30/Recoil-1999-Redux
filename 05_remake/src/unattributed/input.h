// SUBSYSTEM: input
// Declarations for src/unattributed/input.cpp.
#pragma once

namespace recoil {

// 0x0040c190
int __fastcall Array_FillN(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0042a480
int __fastcall CommandTable_Count(int ecx, int edx);
// 0x0042a4a0
int __fastcall CommandTable_GetFirstField(int ecx, int edx);
// 0x0042a4b0
int __fastcall CommandTable_EntrySubCount(int ecx, int edx);
// 0x0042a4d0
int __fastcall CommandTable_EntrySubItem(int ecx, int edx);
// 0x00471d10
int __fastcall Input_SetJoystickFlag2(int ecx, int edx);
// 0x00471d80
int __fastcall Joystick_ReleaseCount(int ecx, int edx);
// 0x00471dd0
int __fastcall Joystick_GetAcquireCount(int ecx, int edx);
// 0x004722b0
int __fastcall Joystick_IsPresent(int ecx, int edx);
// 0x00472390
int __fastcall GetGlobalStruct_00565bd4(int ecx, int edx);
// 0x00472480
int __fastcall Input_Joystick_GetField565e18(int ecx, int edx);
// 0x00472410
int __fastcall Joystick_ResetState(int ecx, int edx);
// 0x004723a0
int __fastcall Input_GetJoystickButtonEdgeState(int ecx, int edx);
// 0x004716d0
int __fastcall InputMgr_GetPrimaryKeyBinding(int ecx, int edx);
// 0x004716e0
int __fastcall InputMgr_GetSecondaryKeyBinding(int ecx, int edx);
// 0x004716f0
int __fastcall InputMgr_GetJoystickButtonBinding(int ecx, int edx);
// 0x00471700
int __fastcall InputMgr_GetMouseButtonBinding(int ecx, int edx);
// 0x00471710
int __fastcall InputMgr_CommandForPrimaryKey(int ecx, int edx);
// 0x00471720
int __fastcall InputMgr_CommandForSecondaryKey(int ecx, int edx);
// 0x00471730
int __fastcall InputMgr_CommandForJoystickButton(int ecx, int edx);
// 0x00471740
int __fastcall InputMgr_CommandForMouseButton(int ecx, int edx);
// 0x00471750
int __fastcall InputMgr_SetPrimaryKeyBinding(int ecx, int edx);
// 0x00471760
int __fastcall InputMgr_SetSecondaryKeyBinding(int ecx, int edx);
// 0x00471770
int __fastcall InputMgr_SetJoystickButtonBinding(int ecx, int edx);
// 0x004717e0
int __fastcall InputMgr_Call_00470f50(int ecx, int edx, int arg1);
// 0x00471800
int __fastcall InputMgr_Call_00470f80(int ecx, int edx, int arg1);
// 0x00471820
int __fastcall InputMgr_Call_00471040(int ecx, int edx, int arg1);
// 0x00471840
int __fastcall InputMgr_Call_00471070(int ecx, int edx, int arg1);
// 0x00471d50
int __fastcall Joystick_AcquireCount(int ecx, int edx);
// 0x00471fb0
int __fastcall Input_Joystick_Acquire(int ecx, int edx);
// 0x004721a0
int __fastcall Joystick_SetDeadzone(int ecx, int edx);
// 0x004721e0
int __fastcall Joystick_SetAxisRange(int ecx, int edx, int arg1);
// 0x00472230
int __fastcall Joystick_GetAxisRange(int ecx, int edx, int arg1);
// 0x00472280
int __fastcall Input_Joystick_ShutdownDevice(int ecx, int edx);
// 0x00472450
int __fastcall Input_Joystick_AcquireOk(int ecx, int edx);
// 0x00472490
int __fastcall DInput_ReportError(int ecx, int edx, int arg1);
// 0x00430070
int __fastcall ForceFeedback_CreateConstantEffect(int ecx, int edx);
// 0x00471790
int __fastcall InputMgr_Call_00470cd0(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004717d0
int __fastcall Input_QueryAction(int ecx, int edx);
// 0x00471fd0
int __fastcall Joystick_ApplyAxisRanges(int ecx, int edx);
// 0x0042e170
int __fastcall Joystick_EnableAndSetRanges(int ecx, int edx);
// 0x004722c0
int __fastcall Input_Joystick_Poll(int ecx, int edx);
// 0x004723d0
int __fastcall Input_WaitJoystickButton(int ecx, int edx);
// 0x0042a4e0
int __fastcall CommandSlot_Call_004a5bf0(int ecx, int edx);
// 0x0042a4f0
int __fastcall CommandSlot_Call_004a5bf0_Plus1(int ecx, int edx);
// 0x0042fa80
int __fastcall ForceFeedback_IsAvailable(int ecx, int edx);
// 0x0042fac0
int __fastcall ForceFeedback_PlayConstant(int ecx, int edx, int arg1);
// 0x0042fb50
int __fastcall ForceFeedback_PlayDirectional(int ecx, int edx, int arg1, int arg2);
// 0x0042fc90
int __fastcall ForceFeedback_PlayHitDirectional(int ecx, int edx, int arg1, int arg2);
// 0x0042fdc0
int __fastcall ForceFeedback_UpdateEffects(int ecx, int edx);
// 0x0042ffa0
int __fastcall ForceFeedback_CreatePeriodicEffect(int ecx, int edx, int arg1);
// 0x00430100
int __fastcall ForceFeedback_CreateRampEffect(int ecx, int edx, int arg1);
// 0x0042f9f0
int __fastcall ForceFeedbackEffects_Create(int ecx, int edx);
// 0x00471cd0
int __fastcall Input_ReleaseKeyboardSuspend(int ecx, int edx);
// 0x00471d20
int __fastcall Input_Keyboard_AcquireRef(int ecx, int edx);
// 0x00471da0
int __fastcall Input_Mouse_AcquireRef(int ecx, int edx);
// 0x00471de0
int __fastcall Input_PollAllDevices(int ecx, int edx);
// 0x00470e80
int __fastcall Input_DispatchKeyHandler_00470e80(int ecx, int edx);
// 0x00471a10
int __fastcall AtexitStub_InputState_ClearLists_00471a10(int ecx, int edx);
// 0x00471f60
int __fastcall Input_TryCreateDirectInputDevice_00471f60(int ecx, int edx, int arg1, int arg2);
// 0x004717c0
int __fastcall Registry_AddWithDefault(int ecx, int edx);
// 0x004716c0
int __fastcall Bindings_Forward_004716c0(int ecx, int edx);
// 0x00414550
int __fastcall Input_FeedTypedCharToScreen_00414550(int ecx, int edx);
// 0x004719e0
int __fastcall StaticInitWrapper_004719e0(int ecx, int edx);
// 0x00471c60
int __fastcall Input_IsBit1Clear_Byte00561cba_00471c60(int ecx, int edx);
// 0x00471c70
int __fastcall Input_IsBit1Clear_Byte00561cb9_00471c70(int ecx, int edx);
// 0x00471c80
int __fastcall Input_IsBit1Clear_Byte00561cb8_00471c80(int ecx, int edx);
// 0x00471c90
int __fastcall Input_ClearBit1Byte00561cba_ResetMouseDeltas_00471c90(int ecx, int edx);
// 0x00471cb0
int __fastcall Input_ClearBit1Byte00561cb9_ResetJoystick_00471cb0(int ecx, int edx);
// 0x00471cf0
int __fastcall Input_SetBit1_Byte00561cba_00471cf0(int ecx, int edx);
// 0x00471d00
int __fastcall Input_SetBit1_Byte00561cb9_00471d00(int ecx, int edx);
// 0x00471ae0
int __fastcall Input_SetAllDeviceBit1AndAcquireMouse_00471ae0(int ecx, int edx);
// 0x00429f80
int __fastcall CommandTable_Clear(int ecx, int edx);
// 0x0042a000
int __fastcall CommandTableEntry_Dtor(int ecx, int edx);
// 0x004c9c90
int __fastcall EH_Unwind_CommandTableEntry_Dtor_0(int ecx, int edx);
// 0x004c9c98
int __fastcall EH_Unwind_CommandTableEntry_Dtor_1(int ecx, int edx);
// 0x004c9cb0
int __fastcall EH_Unwind_CommandTable_AddEntry_0(int ecx, int edx);
// 0x004c9cbb
int __fastcall EH_Unwind_CommandTable_AddEntry_1(int ecx, int edx);
// 0x004c9cc3
int __fastcall EH_Unwind_CommandTable_AddEntry_2(int ecx, int edx);
// 0x004c9ca3
int __fastcall EH_Handler_CommandTableEntry_Dtor(int ecx, int edx);

// 0x004c9cce
int __fastcall EH_Handler_CommandTable_AddEntry(int ecx, int edx);

// 0x0042a070
int __fastcall CommandTable_AddEntry(int ecx, int edx);
// 0x0042a2c0
int __fastcall CommandTableEntry_PushValue(int ecx, int edx);
// 0x0042a500
int __fastcall CommandSlot_Bind(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00471e40
int __fastcall Input_Joystick_CreateDevice(int ecx, int edx);
// 0x00471b20
int __fastcall Input_ResumeOnActivate(int ecx, int edx);
// 0x00426150
int __fastcall Game_DispatchCommand_00426150(int ecx, int edx);
}  // namespace recoil
