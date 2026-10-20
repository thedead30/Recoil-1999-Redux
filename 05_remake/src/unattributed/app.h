// SUBSYSTEM: app
// Declarations for src/unattributed/app.cpp. Spec: 04_spec/systems/app.md
#pragma once

namespace recoil {

// 0x004c6350 Crt_SetFpuControl
void Crt_SetFpuControl();

// 0x0042df90
int __fastcall ScreenBase_ResetVtbl(int ecx, int edx);
// 0x0042eea0
int __fastcall FrameState_Ctor_004d0b90(int ecx, int edx);
// 0x0042eecd
int __fastcall Stub_Ret4(int ecx, int edx, int arg1);
// 0x00430240
int __fastcall RuntimeClass_Get_004d0bf0(int ecx, int edx);
// 0x00438980
int __fastcall Version_GetString(int ecx, int edx);
// 0x00442260
int __fastcall RuntimeClass_Get_004d1eb0(int ecx, int edx);
// 0x004428a0
int __fastcall RecoilApp_GetClassInfoPtr(int ecx, int edx);
// 0x00442c00
int __fastcall RecoilApp_GetField20(int ecx, int edx);
// 0x004437a0
int __fastcall RuntimeClass_Get_004d20e0(int ecx, int edx);
// 0x004a59a0
int __fastcall Global_Set_0056b564(int ecx, int edx);
// 0x004a59b0
int __fastcall Video_IsWindowedNonGlide(int ecx, int edx);
// 0x004a75e0
int __fastcall EmptyStub_004a75e0(int ecx, int edx);
// 0x0042db50
int __fastcall Atl_InternalQueryInterface(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x0042de00
int __fastcall ComPtr_Release(int ecx, int edx);
// 0x0042faa0
int __fastcall ComPtr_StopAndReset(int ecx, int edx);
// 0x0042dda0
int __fastcall DsBuffer_Init(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00442860
int __fastcall CritSec_Delete(int ecx, int edx);
// 0x00442770
int __fastcall App_Callback_ImportCall_00442770(int ecx, int edx, int arg1);
// 0x004a5ad0
int __fastcall Messages_LoadDll(int ecx, int edx);
// 0x004a5b00
int __fastcall App_FreeLoadedLibrary(int ecx, int edx);
// 0x004a5980
int __fastcall App_ExitProcess(int ecx, int edx);
// 0x004a5780
int __fastcall App_RedirectStdioToLogs(int ecx, int edx);
// 0x0040c370
int __fastcall RecoilApp_ProbeDirectXCapabilities(int ecx, int edx);
// 0x004427d0
int __fastcall App_Forward_0042db50_004427d0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00404bd0
int __fastcall Loader_WaitForThread(int ecx, int edx);
// 0x0042e0f0
int __fastcall ScreenBase_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0042eec0
int __fastcall AppScreen_OnArg_0042eec0(int ecx, int edx);
// 0x0042f890
int __fastcall OperatorDelete_Thunk_0042f890(int ecx, int edx, int arg1);
// 0x00431b50
int __fastcall ScalarDeletingDtor_00431b50(int ecx, int edx, int arg1);
// 0x0044263e
int __fastcall EH_Continuation_0044263e(int ecx, int edx, int arg1);
// 0x004a5670
int __fastcall Timer_Reset(int ecx, int edx);
// 0x0042ee50
int __fastcall AppScreen_OnEnter_0042ee50(int ecx, int edx);
// 0x00442c10
int __fastcall RecoilApp_OnIdleStep(int ecx, int edx);
// 0x00430680
int __fastcall MainWnd_SetWindowedChrome(int ecx, int edx, int arg1);
// 0x00431330
int __fastcall MainWnd_OnToggleSoundArchive(int ecx, int edx);
// 0x00431380
int __fastcall MainWnd_OnToggleTextureFlag(int ecx, int edx);
// 0x00442270
int __fastcall StatusDialog_Printf(int ecx, int edx);
// 0x00442660
int __fastcall App_Callback_Set5392a4_On(int ecx, int edx, int arg1);
// 0x00442680
int __fastcall App_Callback_Set5392a4_Off(int ecx, int edx, int arg1, int arg2);
// 0x004426b0
int __fastcall StatusDialog_OnProgress(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00442720
int __fastcall WolPatch_OnStatus_00442720(int ecx, int edx, int arg1, int arg2);
// 0x00443650
int __fastcall App_ForwardMciNotifyToScreen(int ecx, int edx, int arg1, int arg2);
// 0x004306f0
int __fastcall MainWnd_GetTitle(int ecx, int edx, int arg1);
// 0x00441c60
int __fastcall ComboDialog_GetSelection(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00441f40
int __fastcall WolLoginDialog_OnOK(int ecx, int edx);
// 0x00442010
int __fastcall ComboDialog_CommitEdit(int ecx, int edx);
// 0x00442080
int __fastcall ComboDialog_OnSelChange(int ecx, int edx);
// 0x00442100
int __fastcall ComboDialog_OnEditChanged(int ecx, int edx);
// 0x00442220
int __fastcall Dialog9D_Ctor(int ecx, int edx, int arg1);
// 0x004428b0
int __fastcall RecoilApp_Dtor(int ecx, int edx);
// 0x004429d0
int __fastcall RecoilApp_InitInstance(int ecx, int edx);
// 0x00442c70
int __fastcall RecoilApp_Constructor(int ecx, int edx);
// 0x004c81c0
int __fastcall WinMain_Thunk(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c81d8
int __fastcall Afx_SetMbcsState(int ecx, int edx, int arg1, int arg2);
// 0x004429b0
int __fastcall RecoilApp_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0042de10
int __fastcall Slot_ReturnConstAddr_004d0990_0042de10(int ecx, int edx);
// 0x0042eb00
int __fastcall Slot_ReturnTrue_Ret8_0042eb00(int ecx, int edx, int arg1, int arg2);
// 0x004308c0
int __fastcall MainWnd_EnableVideoModes(int ecx, int edx);
// 0x00431a90
int __fastcall Slot_ZeroECX_Jmp_RecoilApp_ApplySoundAPISetting_00431a90(int ecx, int edx);
// 0x004420c0
int __fastcall Slot_SetField118_To1_004420c0(int ecx, int edx);
// 0x004420d0
int __fastcall Thunk_ComboDialog_CommitEdit_004420d0(int ecx, int edx);
// 0x00442240
int __fastcall Dialog9D_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00442890
int __fastcall Slot_ReturnGlobal_004cc248_00442890(int ecx, int edx);
// 0x00443790
int __fastcall Slot_ReturnGlobal_004cc25c_00443790(int ecx, int edx);
// 0x004437b0
int __fastcall Slot_ReturnGlobal_004cc260_004437b0(int ecx, int edx);
// 0x004437c0
int __fastcall Slot_ReturnConstAddr_004d20f8_004437c0(int ecx, int edx);
// 0x004438f0
int __fastcall Thunk_004c5e88_004438f0(int ecx, int edx);
// 0x00443a40
int __fastcall Window_CacheClientRectScreen_00443a40(int ecx, int edx);
// 0x00472d30
int __fastcall Crt_MathErrHandler_00472d30(int ecx, int edx);
// 0x004309b0
int __fastcall MainWnd_OnVideoMode2(int ecx, int edx);
// 0x004309d0
int __fastcall MainWnd_OnVideoMode3(int ecx, int edx);
// 0x004309f0
int __fastcall MainWnd_OnVideoMode4(int ecx, int edx);
// 0x00430a10
int __fastcall MainWnd_OnVideoMode5(int ecx, int edx);
// 0x00430a30
int __fastcall MainWnd_OnVideoMode6(int ecx, int edx);
// 0x00430a50
int __fastcall MainWnd_OnVideoMode7(int ecx, int edx);
// 0x004c6140
int __fastcall entry(int ecx, int edx);
// 0x0042e9f0
int __fastcall App_SwallowSysKeyMessage_IfHWCard_0042e9f0(int ecx, int edx, int arg1);
// 0x004308a0
int __fastcall Slot_PostMessage_WMCLOSE_004308a0(int ecx, int edx);
// 0x004313d0
int __fastcall Screen_UpdateButtonPair_ByState1E0_004313d0(int ecx, int edx, int arg1);
// 0x00431430
int __fastcall Screen_UpdateButtonPair_ByState1E4_00431430(int ecx, int edx, int arg1);
// 0x00431490
int __fastcall Screen_UpdateButtonPair_ByState1E8_00431490(int ecx, int edx, int arg1);
// 0x004314f0
int __fastcall Screen_UpdateButtonPair_ByState1EC_004314f0(int ecx, int edx, int arg1);
// 0x00431550
int __fastcall Screen_UpdateButtonPair_ByState1F0_00431550(int ecx, int edx, int arg1);
// 0x004315b0
int __fastcall Screen_UpdateButtonPair_ByState1F4_004315b0(int ecx, int edx, int arg1);
// 0x00431870
int __fastcall Slot_CallVirtual0_1_ThenVirtual4_ByMode8_00431870(int ecx, int edx, int arg1);
// 0x004318e0
int __fastcall Slot_RemoveMenuItem_9c4e_004318e0(int ecx, int edx, int arg1);
// 0x00431a80
int __fastcall Slot_CallVirtual0_Arg1_00431a80(int ecx, int edx, int arg1);
// 0x00442a30
int __fastcall Slot_ExchangeField0xD0_With1_00442a30(int ecx, int edx);
// 0x004438a0
int __fastcall Slot_CallVirtual10_IsZero_004438a0(int ecx, int edx, int arg1);
// 0x004438c0
int __fastcall App_GetCompanyNameString_004438c0(int ecx, int edx, int arg1);
// 0x00443a60
int __fastcall Window_LoadGameBitmap_00443a60(int ecx, int edx, int arg1);
// 0x00443b50
int __fastcall Slot_ForwardVirtualB4_Ret8_00443b50(int ecx, int edx, int arg1, int arg2);
// 0x004c6300
int __fastcall type_info_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00430a70
int __fastcall Settings_StoreHUDFlagFromSplitScreen_00430a70(int ecx, int edx);
// 0x00431900
int __fastcall Settings_ToggleCDAudio_00431900(int ecx, int edx);
// 0x00431950
int __fastcall Settings_ToggleJoystickEnabled_00431950(int ecx, int edx);
// 0x00431ad0
int __fastcall Settings_ApplySoundAPISetting1_00431ad0(int ecx, int edx);
// 0x00443a50
int __fastcall Wnd_OnMove_DefaultThenCacheClientRect_00443a50(int ecx, int edx, int arg1, int arg2);
// 0x004318b0
int __fastcall Slot_Call4317d0_Mode1_004318b0(int ecx, int edx, int arg1);
// 0x004318c0
int __fastcall Slot_Call4317d0_Mode2_004318c0(int ecx, int edx, int arg1);
// 0x004318d0
int __fastcall Slot_Call4317d0_Mode3_004318d0(int ecx, int edx, int arg1);
// 0x00404c80
int __fastcall Loader_StartIfReady(int ecx, int edx);
// 0x0042dc30
int __fastcall Com_QueryAndCallSlot14(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x0042dcf0
int __fastcall Com_QueryAndCallSlot18(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0042de20
int __fastcall StaticInitWrapper_0042de20(int ecx, int edx);
// 0x0042de30
int __fastcall StaticInit_004f3ca8(int ecx, int edx);
// 0x0042de40
int __fastcall StaticInit_RegisterAtexit_0042de50(int ecx, int edx);
// 0x0042de50
int __fastcall AtexitStub_MainApp_Dtor_0042de50(int ecx, int edx);
// 0x0042de60
int __fastcall MainApp_Dtor(int ecx, int edx);
// 0x0042df10
int __fastcall SeqScreen_Dtor_A(int ecx, int edx);
// 0x0042df50
int __fastcall SeqScreen_Dtor_B(int ecx, int edx);
// 0x0042dfa0
int __fastcall MainApp_Ctor(int ecx, int edx);
// 0x0042e070
int __fastcall SeqScreen_Dtor_C(int ecx, int edx);
// 0x0042e0b0
int __fastcall Screen_0042de60_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0042e0d0
int __fastcall Screen_0042df50_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0042e930
int __fastcall MainApp_ExitInstance_0042e930(int ecx, int edx);
// 0x0042ed30
int __fastcall SeqScreenB_Ctor(int ecx, int edx);
// 0x0042ed90
int __fastcall Screen_0042e070_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0042edb0
int __fastcall AppScreen_PlayMissionFmv_0042edb0(int ecx, int edx);
// 0x0042ee70
int __fastcall AppScreen_Tick_0042ee70(int ecx, int edx);
// 0x004305f0
int __fastcall MainWnd_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00430610
int __fastcall MainWnd_Dtor(int ecx, int edx);
// 0x00430c30
int __fastcall MainWnd_OnAbout(int ecx, int edx);
// 0x00442180
int __fastcall RecoilApp_SetString4b8(int ecx, int edx, int arg1);
// 0x004421d0
int __fastcall RecoilApp_SetString4bc(int ecx, int edx, int arg1);
// 0x004422f0
int __fastcall StatusDialog_Refresh(int ecx, int edx);
// 0x00442790
int __fastcall RefCounted_Release(int ecx, int edx, int arg1);
// 0x004427f0
int __fastcall RefCounted_Dtor(int ecx, int edx);
// 0x00442bc0
int __fastcall Engine_ShutdownSubsystems(int ecx, int edx);
// 0x00443810
int __fastcall GameFrame_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00443830
int __fastcall GameFrame_Dtor(int ecx, int edx);
// 0x00443900
int __fastcall GameFrame_OnPaint(int ecx, int edx);
// 0x00443b70
int __fastcall GdiObjA_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00443b90
int __fastcall GdiObjA_Dtor(int ecx, int edx);
// 0x00443be0
int __fastcall GdiObjB_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00443c00
int __fastcall GdiObjB_Dtor(int ecx, int edx);
// 0x004c9ce0
int __fastcall EH_Unwind_Com_QueryAndCallSlot14_0(int ecx, int edx);
// 0x004c9ce8
int __fastcall EH_Unwind_Com_QueryAndCallSlot14_1(int ecx, int edx);
// 0x004c9d00
int __fastcall EH_Unwind_Com_QueryAndCallSlot18_0(int ecx, int edx);
// 0x004c9d08
int __fastcall EH_Unwind_Com_QueryAndCallSlot18_1(int ecx, int edx);
// 0x004c9d20
int __fastcall EH_Unwind_MainApp_Dtor_0(int ecx, int edx);
// 0x004c9d28
int __fastcall EH_Unwind_MainApp_Dtor_1(int ecx, int edx);
// 0x004c9d36
int __fastcall EH_Unwind_MainApp_Dtor_2(int ecx, int edx);
// 0x004c9d44
int __fastcall EH_Unwind_MainApp_Dtor_3(int ecx, int edx);
// 0x004c9d52
int __fastcall EH_Unwind_MainApp_Dtor_4(int ecx, int edx);
// 0x004c9d60
int __fastcall EH_Unwind_MainApp_Dtor_5(int ecx, int edx);
// 0x004c9d68
int __fastcall EH_Unwind_MainApp_Dtor_6(int ecx, int edx);
// 0x004c9d70
int __fastcall EH_Unwind_MainApp_Dtor_7(int ecx, int edx);
// 0x004c9d90
int __fastcall EH_Unwind_SeqScreen_Dtor_A_0(int ecx, int edx);
// 0x004c9db0
int __fastcall EH_Unwind_SeqScreen_Dtor_B_0(int ecx, int edx);
// 0x004c9dd0
int __fastcall EH_Unwind_MainApp_Ctor_0(int ecx, int edx);
// 0x004c9dd8
int __fastcall EH_Unwind_MainApp_Ctor_1(int ecx, int edx);
// 0x004c9de6
int __fastcall EH_Unwind_MainApp_Ctor_2(int ecx, int edx);
// 0x004c9dee
int __fastcall EH_Unwind_MainApp_Ctor_3(int ecx, int edx);
// 0x004c9dfc
int __fastcall EH_Unwind_MainApp_Ctor_4(int ecx, int edx);
// 0x004c9e0a
int __fastcall EH_Unwind_MainApp_Ctor_5(int ecx, int edx);
// 0x004c9e18
int __fastcall EH_Unwind_MainApp_Ctor_6(int ecx, int edx);
// 0x004c9e30
int __fastcall EH_Unwind_SeqScreen_Dtor_C_0(int ecx, int edx);
// 0x004c9e50
int __fastcall EH_Unwind_App_CreateMainWindow_0(int ecx, int edx);
// 0x004c9e90
int __fastcall EH_Unwind_SeqScreenB_Ctor_0(int ecx, int edx);
// 0x004c9eb0
int __fastcall EH_Unwind_Game_EndSequenceTick_0(int ecx, int edx);
// 0x004c9eb8
int __fastcall EH_Unwind_Game_EndSequenceTick_1(int ecx, int edx);
// 0x004c9ed0
int __fastcall EH_Unwind_App_EnterMissionOver_0(int ecx, int edx);
// 0x004c9ef0
int __fastcall EH_Unwind_MainWnd_New_0(int ecx, int edx);
// 0x004c9f10
int __fastcall EH_Unwind_MainWnd_Ctor_0(int ecx, int edx);
// 0x004c9f18
int __fastcall EH_Unwind_MainWnd_Ctor_1(int ecx, int edx);
// 0x004c9f26
int __fastcall EH_Unwind_MainWnd_Ctor_2(int ecx, int edx);
// 0x004c9f2e
int __fastcall EH_Unwind_MainWnd_Ctor_3(int ecx, int edx);
// 0x004c9f36
int __fastcall EH_Unwind_MainWnd_Ctor_4(int ecx, int edx);
// 0x004c9f50
int __fastcall EH_Unwind_MainWnd_Dtor_0(int ecx, int edx);
// 0x004c9f58
int __fastcall EH_Unwind_MainWnd_Dtor_1(int ecx, int edx);
// 0x004c9f70
int __fastcall EH_Unwind_MainWnd_OnAbout_0(int ecx, int edx);
// 0x004c9f90
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_0(int ecx, int edx);
// 0x004c9f9b
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_1(int ecx, int edx);
// 0x004c9fa6
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_2(int ecx, int edx);
// 0x004c9fb1
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_3(int ecx, int edx);
// 0x004c9fbc
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_4(int ecx, int edx);
// 0x004c9fc7
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_5(int ecx, int edx);
// 0x004c9fd2
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_6(int ecx, int edx);
// 0x004c9fda
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_7(int ecx, int edx);
// 0x004c9fe5
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_8(int ecx, int edx);
// 0x004c9ff0
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_9(int ecx, int edx);
// 0x004c9ffb
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_10(int ecx, int edx);
// 0x004ca006
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_11(int ecx, int edx);
// 0x004ca011
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_12(int ecx, int edx);
// 0x004ca01c
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_13(int ecx, int edx);
// 0x004ca027
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_14(int ecx, int edx);
// 0x004ca032
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_15(int ecx, int edx);
// 0x004ca03d
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_16(int ecx, int edx);
// 0x004ca048
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_17(int ecx, int edx);
// 0x004ca050
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_18(int ecx, int edx);
// 0x004ca05b
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_19(int ecx, int edx);
// 0x004ca066
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_20(int ecx, int edx);
// 0x004ca071
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_21(int ecx, int edx);
// 0x004ca07c
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_22(int ecx, int edx);
// 0x004ca087
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_23(int ecx, int edx);
// 0x004ca092
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_24(int ecx, int edx);
// 0x004ca09d
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_25(int ecx, int edx);
// 0x004ca0a8
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_26(int ecx, int edx);
// 0x004ca0b3
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_27(int ecx, int edx);
// 0x004ca0be
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_28(int ecx, int edx);
// 0x004ca0c6
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_29(int ecx, int edx);
// 0x004ca0d1
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_30(int ecx, int edx);
// 0x004ca0dc
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_31(int ecx, int edx);
// 0x004ca0e7
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_32(int ecx, int edx);
// 0x004ca0f2
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_33(int ecx, int edx);
// 0x004ca0fd
int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_34(int ecx, int edx);
// 0x004cab6b
int __fastcall EH_Unwind_ComboDialog_RunModal_1(int ecx, int edx);
// 0x004cab76
int __fastcall EH_Unwind_ComboDialog_RunModal_2(int ecx, int edx);
// 0x004cab81
int __fastcall EH_Unwind_ComboDialog_RunModal_3(int ecx, int edx);
// 0x004cab8c
int __fastcall EH_Unwind_ComboDialog_RunModal_4(int ecx, int edx);
// 0x004cab94
int __fastcall EH_Unwind_ComboDialog_RunModal_5(int ecx, int edx);
// 0x004cab9c
int __fastcall EH_Unwind_ComboDialog_RunModal_6(int ecx, int edx);
// 0x004cabaf
int __fastcall EH_Unwind_ComboDialog_RunModal_7(int ecx, int edx);
// 0x004cabc2
int __fastcall EH_Unwind_ComboDialog_RunModal_8(int ecx, int edx);
// 0x004cabd5
int __fastcall EH_Unwind_ComboDialog_RunModal_9(int ecx, int edx);
// 0x004cabe0
int __fastcall EH_Unwind_ComboDialog_RunModal_10(int ecx, int edx);
// 0x004cabeb
int __fastcall EH_Unwind_ComboDialog_RunModal_11(int ecx, int edx);
// 0x004cabf6
int __fastcall EH_Unwind_ComboDialog_RunModal_12(int ecx, int edx);
// 0x004cabfe
int __fastcall EH_Unwind_ComboDialog_RunModal_13(int ecx, int edx);
// 0x004cac06
int __fastcall EH_Unwind_ComboDialog_RunModal_14(int ecx, int edx);
// 0x004cac19
int __fastcall EH_Unwind_ComboDialog_RunModal_15(int ecx, int edx);
// 0x004cac2c
int __fastcall EH_Unwind_ComboDialog_RunModal_16(int ecx, int edx);
// 0x004cac50
int __fastcall EH_Unwind_RecoilApp_SetString4b8_0(int ecx, int edx);
// 0x004cac70
int __fastcall EH_Unwind_RecoilApp_SetString4bc_0(int ecx, int edx);
// 0x004cacb0
int __fastcall EH_Unwind_RefCounted_Dtor_0(int ecx, int edx);
// 0x004cace0
int __fastcall EH_Unwind_GameFrame_New_0(int ecx, int edx);
// 0x004cad00
int __fastcall EH_Unwind_GameFrame_Dtor_0(int ecx, int edx);
// 0x004cad08
int __fastcall EH_Unwind_GameFrame_Dtor_1(int ecx, int edx);
// 0x004cad20
int __fastcall EH_Unwind_GameFrame_OnPaint_0(int ecx, int edx);
// 0x004cad40
int __fastcall EH_Unwind_GdiObjA_Dtor_0(int ecx, int edx);
// 0x004cad60
int __fastcall EH_Unwind_GdiObjB_Dtor_0(int ecx, int edx);
// 0x004c9cf0
int __fastcall EH_Handler_Com_QueryAndCallSlot14(int ecx, int edx);

// 0x004c9d10
int __fastcall EH_Handler_Com_QueryAndCallSlot18(int ecx, int edx);

// 0x004c9d78
int __fastcall EH_Handler_MainApp_Dtor(int ecx, int edx);

// 0x004c9d98
int __fastcall EH_Handler_SeqScreen_Dtor_A(int ecx, int edx);

// 0x004c9db8
int __fastcall EH_Handler_SeqScreen_Dtor_B(int ecx, int edx);

// 0x004c9e26
int __fastcall EH_Handler_MainApp_Ctor(int ecx, int edx);

// 0x004c9e38
int __fastcall EH_Handler_SeqScreen_Dtor_C(int ecx, int edx);

// 0x004c9e5a
int __fastcall EH_Handler_App_CreateMainWindow(int ecx, int edx);

// 0x004c9e98
int __fastcall EH_Handler_SeqScreenB_Ctor(int ecx, int edx);

// 0x004c9ec0
int __fastcall EH_Handler_Game_EndSequenceTick(int ecx, int edx);

// 0x004c9ed8
int __fastcall EH_Handler_App_EnterMissionOver(int ecx, int edx);

// 0x004c9efa
int __fastcall EH_Handler_MainWnd_New(int ecx, int edx);

// 0x004c9f3e
int __fastcall EH_Handler_MainWnd_Ctor(int ecx, int edx);

// 0x004c9f60
int __fastcall EH_Handler_MainWnd_Dtor(int ecx, int edx);

// 0x004c9f78
int __fastcall EH_Handler_MainWnd_OnAbout(int ecx, int edx);

// 0x004ca108
int __fastcall EH_Handler_Menus_RunNetSetupThenStartMission(int ecx, int edx);

// 0x004cac58
int __fastcall EH_Handler_RecoilApp_SetString4b8(int ecx, int edx);

// 0x004cac78
int __fastcall EH_Handler_RecoilApp_SetString4bc(int ecx, int edx);

// 0x004cacb8
int __fastcall EH_Handler_RefCounted_Dtor(int ecx, int edx);

// 0x004cacea
int __fastcall EH_Handler_GameFrame_New(int ecx, int edx);

// 0x004cad10
int __fastcall EH_Handler_GameFrame_Dtor(int ecx, int edx);

// 0x004cad28
int __fastcall EH_Handler_GameFrame_OnPaint(int ecx, int edx);

// 0x004cad48
int __fastcall EH_Handler_GdiObjA_Dtor(int ecx, int edx);

// 0x004cad68
int __fastcall EH_Handler_GdiObjB_Dtor(int ecx, int edx);

// 0x00430c90
int __fastcall App_FatalErrorShutdown_00430c90(int ecx, int edx);
// 0x00441cb0
int __fastcall ComboDialog_RunModal(int ecx, int edx);
// 0x004422a0
int __fastcall StatusDialog_CreateComObject(int ecx, int edx);
// 0x00442320
int __fastcall WolPatch_DialogProc_00442320(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00442530
int __fastcall App_PromptForEachListItem(int ecx, int edx);
// 0x004425c0
int __fastcall RefCounted_Create(int ecx, int edx);
// 0x00442632
int __fastcall EH_Catch_RefCounted_Create_0(int ecx, int edx);
// 0x00442638
int __fastcall EH_Cont_RefCounted_Create_0(int ecx, int edx);
// 0x00442d00
int __fastcall RecoilApp_Run(int ecx, int edx);
// 0x0044300b
int __fastcall AppRun_CatchMemoryException(int ecx, int edx);
// 0x00443032
int __fastcall AppRun_CatchFileException(int ecx, int edx);
// 0x004430cc
int __fastcall AppRun_CatchGeneralException(int ecx, int edx);
// 0x004430f3
int __fastcall EH_Cont_RecoilApp_Run_0(int ecx, int edx);
// 0x00443ab0
int __fastcall GameFrame_OnDestroy(int ecx, int edx);
// 0x004cab60
int __fastcall EH_Unwind_ComboDialog_RunModal_0(int ecx, int edx);
// 0x004cac90
int __fastcall EH_Unwind_RefCounted_Create_0(int ecx, int edx);
// 0x004cac9b
int __fastcall EH_Unwind_RefCounted_Create_1(int ecx, int edx);
// 0x004cac3f
int __fastcall EH_Handler_ComboDialog_RunModal(int ecx, int edx);

// 0x004caca3
int __fastcall EH_Handler_RefCounted_Create(int ecx, int edx);

// 0x004cacd0
int __fastcall EH_Handler_RecoilApp_Run(int ecx, int edx);

// 0x0042f9d0
int __fastcall AppScreen_Shutdown_0042f9d0(int ecx, int edx);
// 0x004c8201
int __fastcall AfxInitAppState_Ctor(int ecx, int edx);
// 0x004c8214
int __fastcall Thunk_004c8219_004c8214(int ecx, int edx);
// 0x004317d0
int __fastcall Screen_UpdatePanelButtonAndAccelerator_004317d0(int ecx, int edx, int arg1, int arg2);
// 0x004c8219
int __fastcall StaticInit_AfxInitAppState_0056cc28(int ecx, int edx);
// 0x00442a10
int __fastcall RecoilApp_TakeFieldD0(int ecx, int edx);
// 0x00442a50
int __fastcall Engine_InitSubsystems(int ecx, int edx, int arg1);
// 0x0042e110
int __fastcall App_CreateMainWindow(int ecx, int edx);
// 0x0042e520
int __fastcall MainApp_InitInstance_0042e520(int ecx, int edx);
// 0x004301e0
int __fastcall MainWnd_New(int ecx, int edx);
// 0x00430250
int __fastcall MainWnd_Ctor(int ecx, int edx);
// 0x00431610
int __fastcall MainWnd_SelectDriverWindowedDefault(int ecx, int edx, int arg1);
// 0x00431680
int __fastcall MainWnd_SelectDriverAndMode(int ecx, int edx);
// 0x004316c0
int __fastcall Screen_SelectExclusiveState_004316c0(int ecx, int edx, int arg1);
// 0x00431730
int __fastcall MainWnd_InitDisplayModeList(int ecx, int edx);
// 0x00431790
int __fastcall Slot_Call_004316c0_Arg0_00431790(int ecx, int edx);
// 0x004317a0
int __fastcall Slot_Call_004316c0_Arg1_004317a0(int ecx, int edx);
// 0x004317b0
int __fastcall Slot_Call_004316c0_Arg2_004317b0(int ecx, int edx);
// 0x004317c0
int __fastcall Slot_Call_004316c0_Arg3_004317c0(int ecx, int edx);
// 0x00443730
int __fastcall GameFrame_New(int ecx, int edx);
// 0x004437d0
int __fastcall GameFrame_Ctor(int ecx, int edx, int arg1);
// 0x0042f8a0
int __fastcall DataTarget_0042f8a0(int ecx, int edx, int arg1);
// 0x004306e0
int __fastcall DataTarget_004306e0(int ecx, int edx);
// 0x00430a90
int __fastcall DataTarget_00430a90(int ecx, int edx, int arg1);
// 0x00430ab0
int __fastcall DataTarget_00430ab0(int ecx, int edx);
// 0x00431920
int __fastcall DataTarget_00431920(int ecx, int edx, int arg1);
// 0x00431970
int __fastcall DataTarget_00431970(int ecx, int edx, int arg1);
// 0x00431aa0
int __fastcall DataTarget_00431aa0(int ecx, int edx, int arg1);
int __fastcall DataTarget_00430ad0(int ecx, int edx);
int __fastcall DataTarget_004c62b7(int ecx, int edx);
int __fastcall DataTarget_00431ae0(int ecx, int edx, int arg1);
// 0x00431b10
int __fastcall Slot_Forward443a20_ThenVirtualA8_00431b10(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004420e0
int __fastcall DataTarget_004420e0(int ecx, int edx);
// 0x00443a20
int __fastcall DataTarget_00443a20(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004c62a2
int __fastcall DataTarget_004c62a2(int ecx, int edx);
// 0x00443ae0
int __fastcall MainWnd_OnActivate_00443ae0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0042f280
int __fastcall App_RenderFrameAndPresent(int ecx, int edx, int arg1);
// 0x0042f5e0
int __fastcall Game_EndSequenceTick(int ecx, int edx);
// 0x0048ea20
int __fastcall Unnamed_0048ea20(int ecx, int edx);
// 0x0042bb30
int __fastcall Game_HandleStateEvent(int ecx, int edx, int arg1);
// 0x0042eed0
int __fastcall App_EnterGameplay(int ecx, int edx);
// 0x0042f8e0
int __fastcall App_EnterMissionOver_0042f8e0(int ecx, int edx);
// 0x00430740
int __fastcall Slot_ClearFlagsThenLoadArchives_00430740(int ecx, int edx);
// 0x00430d80
int __fastcall Menus_RunNetSetupThenStartMission_00430d80(int ecx, int edx);
// 0x00431270
int __fastcall Slot_StartMission_1_00431270(int ecx, int edx);
// 0x00431290
int __fastcall Slot_StartMission_2_00431290(int ecx, int edx);
// 0x004312b0
int __fastcall Slot_StartMission_3_004312b0(int ecx, int edx);
// 0x004312d0
int __fastcall Slot_StartMission_4_004312d0(int ecx, int edx);
// 0x004312f0
int __fastcall Slot_StartMission_5_004312f0(int ecx, int edx);
// 0x00431310
int __fastcall Slot_StartMission_6_00431310(int ecx, int edx);
// 0x00430760
int __fastcall DataTarget_00430760(int ecx, int edx);
// 0x00430770
int __fastcall DataTarget_00430770(int ecx, int edx);
// 0x004319a0
int __fastcall Menus_StartZoneLobbyThenMission_004319a0(int ecx, int edx);
}  // namespace recoil
