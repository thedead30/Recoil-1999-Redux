// SUBSYSTEM: input
// Declarations for src/GameZRecoil/zInput/zin_kbd.cpp.
#pragma once

namespace recoil {

// 0x0046f980
int __fastcall Input_Keyboard_ConsumeKeyEdge(int ecx, int edx);
// 0x0046f9b0
int __fastcall Input_Keyboard_SetKeyHandler(int ecx, int edx, int arg1);
// 0x0046f9d0
int __fastcall Input_ClearBindingSlot(int ecx, int edx);
// 0x0046f9f0
int __fastcall Input_Keyboard_ClearHeldTable(int ecx, int edx);
// 0x00470190
int __fastcall Input_GetMouseAvailable(int ecx, int edx);
// 0x004701a0
int __fastcall Input_Mouse_SetScreenSize(int ecx, int edx);
// 0x004703a0
int __fastcall Input_GetMouseButtonEdgeTable(int ecx, int edx);
// 0x00470670
int __fastcall Input_SetMouseMode_Exchange(int ecx, int edx);
// 0x00470a10
int __fastcall Binding_Pack(int ecx, int edx, int arg1, int arg2);
// 0x004705f0
int __fastcall CopyStruct_00561c80(int ecx, int edx);
// 0x004715e0
int __fastcall Input_InitJoystickButtonNames(int ecx, int edx);
// 0x00471640
int __fastcall Input_InitMouseButtonNames(int ecx, int edx);
// 0x0046fd20
int __fastcall Input_InitKeyNameTable_0046fd20(int ecx, int edx);
// 0x00471120
int __fastcall Input_InitKeyNameTable(int ecx, int edx);
// 0x004702e0
int __fastcall Input_GetMouseButtonEdgeState(int ecx, int edx);
// 0x00470a40
int __fastcall Input_GetPrimaryKeyBinding(int ecx, int edx, int arg1);
// 0x00470a60
int __fastcall Input_GetSecondaryKeyBinding(int ecx, int edx, int arg1);
// 0x00470a80
int __fastcall Input_GetJoystickButtonBinding(int ecx, int edx, int arg1);
// 0x00470aa0
int __fastcall Input_GetMouseButtonBinding(int ecx, int edx, int arg1);
// 0x00470ac0
int __fastcall InputMap_CommandForPrimaryKey(int ecx, int edx, int arg1);
// 0x00470ad0
int __fastcall InputMap_CommandForSecondaryKey(int ecx, int edx, int arg1);
// 0x00470b00
int __fastcall InputMap_CommandForJoystickButton(int ecx, int edx, int arg1);
// 0x00470b10
int __fastcall InputMap_CommandForMouseButton(int ecx, int edx, int arg1);
// 0x00470b20
int __fastcall InputMap_SetPrimaryKeyBinding(int ecx, int edx, int arg1, int arg2);
// 0x00470b80
int __fastcall InputMap_SetSecondaryKeyBinding(int ecx, int edx, int arg1, int arg2);
// 0x00470bf0
int __fastcall InputMap_SetJoystickButtonBinding(int ecx, int edx, int arg1, int arg2);
// 0x00470c60
int __fastcall InputMap_SetMouseButtonBinding(int ecx, int edx, int arg1, int arg2);
// 0x004707a0
int __fastcall InputMap_FreeArrays(int ecx, int edx);
// 0x00470960
int __fastcall Bindings_Free(int ecx, int edx);
// 0x00470f50
int __fastcall InputMap_CopyCommandName(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00470f80
int __fastcall Input_FormatKeyName(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00471040
int __fastcall Input_CopyKeyName_5662bc(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00471070
int __fastcall Input_CopyKeyName_5662fc(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00470ae0
int __fastcall InputMap_CommandForKey(int ecx, int edx, int arg1);
// 0x00470cd0
int __fastcall InputMap_SetCommand(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x00470eb0
int __fastcall Input_QueryActionState(int ecx, int edx, int arg1);
// 0x00470d40
int __fastcall Input_Mouse_DispatchButtonEdgeHandlers(int ecx, int edx);
// 0x0046f420
int __fastcall Input_Keyboard_ShutdownDevice(int ecx, int edx);
// 0x00470310
int __fastcall Input_Mouse_SetAcquired(int ecx, int edx);
// 0x0046fa10
int __fastcall Input_Keyboard_WaitForKeyPress(int ecx, int edx);
// 0x00470360
int __fastcall Input_Mouse_ShutdownDevice(int ecx, int edx);
// 0x00470db0
int __fastcall Input_Joystick_DispatchButtonHandlers(int ecx, int edx);

// 0x0046f970
int __fastcall Input_Keyboard_SetCharCallback(int ecx, int edx);
// 0x0046f450
int __fastcall Input_Keyboard_InitialPollAndClear(int ecx, int edx);
// 0x0046fba0
int __fastcall Input_Keyboard_ScancodeToAscii(int ecx, int edx);
// 0x004704f0
int __fastcall Input_Mouse_UpdateCursorPosition(int ecx, int edx);
// 0x0046f690
int __fastcall Input_Keyboard_PollAndTranslate(int ecx, int edx);
// 0x004703c0
int __fastcall Input_Mouse_PollAndTranslate(int ecx, int edx);
// 0x00470610
int __fastcall Mouse_ResetDeltas(int ecx, int edx);
// 0x004703b0
int __fastcall Input_Mouse_Poll(int ecx, int edx);
// 0x00470680
int __fastcall Input_Mouse_WaitForClick(int ecx, int edx);
// 0x00470020
int __fastcall Input_Mouse_SetCursorToCenter(int ecx, int edx);
// 0x00470060
int __fastcall Input_Mouse_UpdateFromClientRect(int ecx, int edx);
// 0x004700a0
int __fastcall Input_Mouse_SetNormalizedPos(int ecx, int edx, int arg1, int arg2);
// 0x00470150
int __fastcall Input_Mouse_RecenterCursor(int ecx, int edx);
// 0x00470180
int __fastcall Input_Mouse_Recenter(int ecx, int edx);
// 0x00470df0
int __fastcall InputMap_SetCommandHandler(int ecx, int edx, int arg1, int arg2);
// 0x00470820
int __fastcall InputMap_RebuildReverseTables(int ecx, int edx);
// 0x004706c0
int __fastcall InputMap_CopyCtor(int ecx, int edx, int arg1);
// 0x004709d0
int __fastcall Bindings_ResetAll(int ecx, int edx);
// 0x00471660
int __fastcall Input_DestroyBindings(int ecx, int edx);
// 0x004708f0
int __fastcall Bindings_Alloc(int ecx, int edx, int arg1);
// 0x004710a0
int __fastcall Input_CreateBindings(int ecx, int edx);
// 0x004cb000
int __fastcall EH_Unwind_Input_CreateBindings_0(int ecx, int edx);
// 0x004cb00b
int __fastcall EH_Handler_Input_CreateBindings(int ecx, int edx);

// 0x0046f300
int __fastcall Input_Keyboard_CreateDevice(int ecx, int edx);
// 0x004701f0
int __fastcall Input_Mouse_CreateDevice(int ecx, int edx);
}  // namespace recoil
