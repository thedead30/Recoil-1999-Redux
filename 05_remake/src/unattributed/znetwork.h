// SUBSYSTEM: znetwork
// Declarations for src/unattributed/znetwork.cpp.
#pragma once

namespace recoil {

// 0x0043cf40
int __fastcall FormatIPv4(int ecx, int edx);
// 0x0043cf90
int __fastcall ZoneDlg_SendChatLine(int ecx, int edx);
// 0x0043d060
int __fastcall ZoneDlg_LogPrintf(int ecx, int edx);
// 0x0043d130
int __fastcall Zone_ComInit(int ecx, int edx);
// 0x0043d280
int __fastcall Zone_ComShutdown(int ecx, int edx);
// 0x0043d2e0
int __fastcall Zone_LaunchSession(int ecx, int edx);
// 0x0043d650
int __fastcall ZoneDlg_LogMessageId(int ecx, int edx, int arg1);
// 0x0043d6a0
int __fastcall ZoneDlg_OnCancel(int ecx, int edx);
// 0x0043d6b0
int __fastcall ZoneDlg_EnableControls(int ecx, int edx, int arg1);
// 0x0043d720
int __fastcall ZoneDlg_EnableControl260(int ecx, int edx, int arg1);
// 0x0043d740
int __fastcall ZoneDlg_ctor(int ecx, int edx, int arg1);
// 0x0043d980
int __fastcall ZoneDlg_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x0043d9a0
int __fastcall ZoneDlg_dtor(int ecx, int edx);
// 0x0043db20
int __fastcall ZoneDlg_DoDataExchange(int ecx, int edx, int arg1);
// 0x0043dcc0
int __fastcall Slot_ReturnConstAddr_004d1918_0043dcc0(int ecx, int edx);
// 0x0043dcd0
int __fastcall ZoneDlg_OnInitDialog(int ecx, int edx);
// 0x0043dfe0
int __fastcall ZoneDlg_OnTimer(int ecx, int edx, int arg1);
// 0x0043e040
int __fastcall ZoneDlg_OnJoinRoom(int ecx, int edx);
// 0x0043e160
int __fastcall ZoneDlg_CloseSession(int ecx, int edx);
// 0x0043e3b0
int __fastcall ZoneDlg_ResetRoomState(int ecx, int edx);
// 0x0043e450
int __fastcall ZoneDlg_ShowConnecting(int ecx, int edx);
// 0x0043e4b0
int __fastcall Slot_SubmitEditTextViaVirtual70_0043e4b0(int ecx, int edx);
// 0x0043e520
int __fastcall Slot_GetEditTextThenCallVirtual60_0043e520(int ecx, int edx);
// 0x0043e550
int __fastcall ZoneDlg_SendDirectedChat_0043e550(int ecx, int edx);
// 0x0043e680
int __fastcall Slot_CallVirtual18_Global4f53c8_0043e680(int ecx, int edx);
// 0x0043e6a0
int __fastcall ZoneDlg_EnterRoomByTypedName_0043e6a0(int ecx, int edx);
// 0x0043e900
int __fastcall ZoneDlg_OnCreateRoom(int ecx, int edx);
// 0x0043ebd0
int __fastcall ZoneDlg_ClearLog(int ecx, int edx);
// 0x0043ec00
int __fastcall ZoneDlg_ApplyToSelectedPlayers_0043ec00(int ecx, int edx);
// 0x0043ed10
int __fastcall ZoneDlg_ApplyToSelectedPlayersSetFlag_0043ed10(int ecx, int edx);
// 0x0043ee40
int __fastcall Slot_ResetMode0_CallVirtual18_0043ee40(int ecx, int edx);
// 0x0043ee60
int __fastcall Slot_SetMode11_CallVirtual18_0043ee60(int ecx, int edx);
// 0x0043ee80
int __fastcall ZoneDlg_OnModeComboChanged(int ecx, int edx);
// 0x0043efe0
int __fastcall Zone_RunLobby(int ecx, int edx);
// 0x0043f4d0
int __fastcall Dlg_ClampAndShowInt_1to1000_Off4A0_0043f4d0(int ecx, int edx);
// 0x0043f550
int __fastcall Dlg_ClampAndShowInt_2to2000_Off4A8_0043f550(int ecx, int edx);
// 0x0043f5d0
int __fastcall String_TruncateAtFirstSpace(int ecx, int edx);
// 0x0043f610
int __fastcall ZoneSink_Create(int ecx, int edx);
// 0x0043f682
int __fastcall EH_Catch_ZoneSink_Create_0(int ecx, int edx);
// 0x0043f688
int __fastcall EH_Cont_ZoneSink_Create_0(int ecx, int edx);
// 0x0043f68e
int __fastcall ZoneSink_Create_epilogue(int ecx, int edx, int arg1);
// 0x0043f6b0
int __fastcall WolLobby_OnServerList_0043f6b0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043f830
int __fastcall WolLobby_OnChannelEvent_0043f830(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043f9d0
int __fastcall ZoneSink_OnPlayerLeft_RemoveFromList_0043f9d0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043fa70
int __fastcall ZNet_MessageBoxHook_0043fa70(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043fa90
int __fastcall WolLobby_OnError_0043fa90(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043fde0
int __fastcall WolLobby_OnChatLine_0043fde0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043fe50
int __fastcall ZoneSink_OnEnterRoomResult_0043fe50(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0043ff80
int __fastcall ZoneSink_OnPlayerJoined_0043ff80(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004401d0
int __fastcall ZoneSink_OnPlayerLeft_004401d0(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004402c0
int __fastcall ZoneSink_OnPlayerListResult_004402c0(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004407e0
int __fastcall ZoneSink_OnRoomSettingsA(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00440a30
int __fastcall ZoneSink_OnRoomSettingsB(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00440c80
int __fastcall ZoneSink_LogMessage_301b_00440c80(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00440ce0
int __fastcall ZoneSink_LogMessage_301c_00440ce0(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00440d40
int __fastcall ZoneSink_LogMessage_301d_00440d40(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00440d90
int __fastcall ZoneSink_LogEvent_00440d90(int ecx, int edx, int arg1, int arg2);
// 0x00440e10
int __fastcall ZoneSink_OnLobbyStatusMessage_00440e10(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00440ef0
int __fastcall ZoneSink_LogMessage_3026_00440ef0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00440f40
int __fastcall ZoneSink_OnConnectionStatus_00440f40(int ecx, int edx, int arg1, int arg2);
// 0x00441040
int __fastcall ZoneDlg_UpdateSignalBars_00441040(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004411c0
int __fastcall ZoneSink_LogFormatted_004411c0(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00441200
int __fastcall ZoneSink_LogFormatted_00441200(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00441240
int __fastcall Slot_ReturnZero_Ret10_00441240(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00441250
int __fastcall Slot_ReturnZero_Retc_00441250(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00441260
int __fastcall ZoneSink_LogTimeMessage_302a_00441260(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004412c0
int __fastcall ZoneSink_LogMessage_302b_or_302c_004412c0(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00441350
int __fastcall ZoneDlg_RefreshPlayerListEntry_00441350(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00441480
int __fastcall ZoneSink_OnPlayerKicked_00441480(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00441600
int __fastcall CritSecHolder_Init(int ecx, int edx);
// 0x00441620
int __fastcall ZoneSink_Release(int ecx, int edx, int arg1);
// 0x00441660
int __fastcall ZNet_Forward_0042db50_00441660(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00441680
int __fastcall ZoneSink_Dtor(int ecx, int edx);
// 0x004416f0
int __fastcall ZoneDlg_GetString4b8(int ecx, int edx, int arg1);
// 0x00441720
int __fastcall ZoneDlg_GetString4bc(int ecx, int edx, int arg1);
// 0x00441750
int __fastcall WolLoginDialog_Ctor(int ecx, int edx, int arg1);
// 0x00441890
int __fastcall ComboDialog_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x004418b0
int __fastcall ComboDialog_Dtor(int ecx, int edx);
// 0x004419a0
int __fastcall ComboDialog_DoDataExchange(int ecx, int edx, int arg1);
// 0x00441a00
int __fastcall Slot_ReturnGlobal_004cc2b0_00441a00(int ecx, int edx);
// 0x00441a10
int __fastcall Slot_ReturnConstAddr_004d1d10_00441a10(int ecx, int edx);
// 0x00441a20
int __fastcall Slot_SendMessage_WMPASTE_00441a20(int ecx, int edx);
// 0x00441a40
int __fastcall ZoneDlg_OnInitDialog_00441a40(int ecx, int edx);
// 0x00489e10
int __fastcall Net_Shutdown(int ecx, int edx);
// 0x00489f30
int __fastcall NetPlayerList_FreeAll(int ecx, int edx);
// 0x00489f90
int __fastcall Net_SetGlobal_0056aafc_00489f90(int ecx, int edx);
// 0x0048b660
int __fastcall ZNet_AddNamedEntry_0048b660(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x0048bfa0
int __fastcall StaticInitWrapper_0048bfa0(int ecx, int edx);
// 0x0048bff0
int __fastcall ZNet_FreeList_0048bff0(int ecx, int edx);
// 0x004ca430
int __fastcall EH_Unwind_ZoneDlg_SendChatLine_0(int ecx, int edx);
// 0x004ca450
int __fastcall EH_Unwind_Zone_LaunchSession_0(int ecx, int edx);
// 0x004ca45b
int __fastcall EH_Unwind_Zone_LaunchSession_1(int ecx, int edx);
// 0x004ca470
int __fastcall EH_Unwind_ZoneDlg_ctor_0(int ecx, int edx);
// 0x004ca478
int __fastcall EH_Unwind_ZoneDlg_ctor_1(int ecx, int edx);
// 0x004ca483
int __fastcall EH_Unwind_ZoneDlg_ctor_2(int ecx, int edx);
// 0x004ca491
int __fastcall EH_Unwind_ZoneDlg_ctor_3(int ecx, int edx);
// 0x004ca49f
int __fastcall EH_Unwind_ZoneDlg_ctor_4(int ecx, int edx);
// 0x004ca4ad
int __fastcall EH_Unwind_ZoneDlg_ctor_5(int ecx, int edx);
// 0x004ca4bb
int __fastcall EH_Unwind_ZoneDlg_ctor_6(int ecx, int edx);
// 0x004ca4c9
int __fastcall EH_Unwind_ZoneDlg_ctor_7(int ecx, int edx);
// 0x004ca4d7
int __fastcall EH_Unwind_ZoneDlg_ctor_8(int ecx, int edx);
// 0x004ca4e5
int __fastcall EH_Unwind_ZoneDlg_ctor_9(int ecx, int edx);
// 0x004ca4f3
int __fastcall EH_Unwind_ZoneDlg_ctor_10(int ecx, int edx);
// 0x004ca501
int __fastcall EH_Unwind_ZoneDlg_ctor_11(int ecx, int edx);
// 0x004ca50f
int __fastcall EH_Unwind_ZoneDlg_ctor_12(int ecx, int edx);
// 0x004ca51d
int __fastcall EH_Unwind_ZoneDlg_ctor_13(int ecx, int edx);
// 0x004ca52b
int __fastcall EH_Unwind_ZoneDlg_ctor_14(int ecx, int edx);
// 0x004ca539
int __fastcall EH_Unwind_ZoneDlg_ctor_15(int ecx, int edx);
// 0x004ca547
int __fastcall EH_Unwind_ZoneDlg_ctor_16(int ecx, int edx);
// 0x004ca555
int __fastcall EH_Unwind_ZoneDlg_ctor_17(int ecx, int edx);
// 0x004ca563
int __fastcall EH_Unwind_ZoneDlg_ctor_18(int ecx, int edx);
// 0x004ca571
int __fastcall EH_Unwind_ZoneDlg_ctor_19(int ecx, int edx);
// 0x004ca590
int __fastcall EH_Unwind_ZoneDlg_dtor_0(int ecx, int edx);
// 0x004ca598
int __fastcall EH_Unwind_ZoneDlg_dtor_1(int ecx, int edx);
// 0x004ca5a3
int __fastcall EH_Unwind_ZoneDlg_dtor_2(int ecx, int edx);
// 0x004ca5b1
int __fastcall EH_Unwind_ZoneDlg_dtor_3(int ecx, int edx);
// 0x004ca5bf
int __fastcall EH_Unwind_ZoneDlg_dtor_4(int ecx, int edx);
// 0x004ca5cd
int __fastcall EH_Unwind_ZoneDlg_dtor_5(int ecx, int edx);
// 0x004ca5db
int __fastcall EH_Unwind_ZoneDlg_dtor_6(int ecx, int edx);
// 0x004ca5e9
int __fastcall EH_Unwind_ZoneDlg_dtor_7(int ecx, int edx);
// 0x004ca5f7
int __fastcall EH_Unwind_ZoneDlg_dtor_8(int ecx, int edx);
// 0x004ca605
int __fastcall EH_Unwind_ZoneDlg_dtor_9(int ecx, int edx);
// 0x004ca613
int __fastcall EH_Unwind_ZoneDlg_dtor_10(int ecx, int edx);
// 0x004ca621
int __fastcall EH_Unwind_ZoneDlg_dtor_11(int ecx, int edx);
// 0x004ca62f
int __fastcall EH_Unwind_ZoneDlg_dtor_12(int ecx, int edx);
// 0x004ca63d
int __fastcall EH_Unwind_ZoneDlg_dtor_13(int ecx, int edx);
// 0x004ca64b
int __fastcall EH_Unwind_ZoneDlg_dtor_14(int ecx, int edx);
// 0x004ca659
int __fastcall EH_Unwind_ZoneDlg_dtor_15(int ecx, int edx);
// 0x004ca667
int __fastcall EH_Unwind_ZoneDlg_dtor_16(int ecx, int edx);
// 0x004ca675
int __fastcall EH_Unwind_ZoneDlg_dtor_17(int ecx, int edx);
// 0x004ca683
int __fastcall EH_Unwind_ZoneDlg_dtor_18(int ecx, int edx);
// 0x004ca691
int __fastcall EH_Unwind_ZoneDlg_dtor_19(int ecx, int edx);
// 0x004ca6b0
int __fastcall EH_Unwind_ZoneDlg_OnInitDialog_0(int ecx, int edx);
// 0x004ca6b8
int __fastcall EH_Unwind_ZoneDlg_OnInitDialog_1(int ecx, int edx);
// 0x004ca6c0
int __fastcall EH_Unwind_ZoneDlg_OnInitDialog_2(int ecx, int edx);
// 0x004ca6c8
int __fastcall EH_Unwind_ZoneDlg_OnInitDialog_3(int ecx, int edx);
// 0x004ca6d0
int __fastcall EH_Unwind_ZoneDlg_OnInitDialog_4(int ecx, int edx);
// 0x004ca6d8
int __fastcall EH_Unwind_ZoneDlg_OnInitDialog_5(int ecx, int edx);
// 0x004ca6e0
int __fastcall EH_Unwind_ZoneDlg_OnInitDialog_6(int ecx, int edx);
// 0x004ca700
int __fastcall EH_Unwind_ZoneDlg_EnterRoomByTypedName_0(int ecx, int edx);
// 0x004ca720
int __fastcall EH_Unwind_ZoneDlg_OnCreateRoom_0(int ecx, int edx);
// 0x004ca740
int __fastcall EH_Unwind_Zone_RunLobby_0(int ecx, int edx);
// 0x004ca74b
int __fastcall EH_Unwind_Zone_RunLobby_1(int ecx, int edx);
// 0x004ca756
int __fastcall EH_Unwind_Zone_RunLobby_2(int ecx, int edx);
// 0x004ca761
int __fastcall EH_Unwind_Zone_RunLobby_3(int ecx, int edx);
// 0x004ca76c
int __fastcall EH_Unwind_Zone_RunLobby_4(int ecx, int edx);
// 0x004ca777
int __fastcall EH_Unwind_Zone_RunLobby_5(int ecx, int edx);
// 0x004ca782
int __fastcall EH_Unwind_Zone_RunLobby_6(int ecx, int edx);
// 0x004ca78d
int __fastcall EH_Unwind_Zone_RunLobby_7(int ecx, int edx);
// 0x004ca798
int __fastcall EH_Unwind_Zone_RunLobby_8(int ecx, int edx);
// 0x004ca7a3
int __fastcall EH_Unwind_Zone_RunLobby_9(int ecx, int edx);
// 0x004ca7ae
int __fastcall EH_Unwind_Zone_RunLobby_10(int ecx, int edx);
// 0x004ca7b9
int __fastcall EH_Unwind_Zone_RunLobby_11(int ecx, int edx);
// 0x004ca7c4
int __fastcall EH_Unwind_Zone_RunLobby_12(int ecx, int edx);
// 0x004ca7cf
int __fastcall EH_Unwind_Zone_RunLobby_13(int ecx, int edx);
// 0x004ca7da
int __fastcall EH_Unwind_Zone_RunLobby_14(int ecx, int edx);
// 0x004ca7e5
int __fastcall EH_Unwind_Zone_RunLobby_15(int ecx, int edx);
// 0x004ca7f0
int __fastcall EH_Unwind_Zone_RunLobby_16(int ecx, int edx);
// 0x004ca7fb
int __fastcall EH_Unwind_Zone_RunLobby_17(int ecx, int edx);
// 0x004ca806
int __fastcall EH_Unwind_Zone_RunLobby_18(int ecx, int edx);
// 0x004ca811
int __fastcall EH_Unwind_Zone_RunLobby_19(int ecx, int edx);
// 0x004ca819
int __fastcall EH_Unwind_Zone_RunLobby_20(int ecx, int edx);
// 0x004ca821
int __fastcall EH_Unwind_Zone_RunLobby_21(int ecx, int edx);
// 0x004ca829
int __fastcall EH_Unwind_Zone_RunLobby_22(int ecx, int edx);
// 0x004ca834
int __fastcall EH_Unwind_Zone_RunLobby_23(int ecx, int edx);
// 0x004ca83f
int __fastcall EH_Unwind_Zone_RunLobby_24(int ecx, int edx);
// 0x004ca84a
int __fastcall EH_Unwind_Zone_RunLobby_25(int ecx, int edx);
// 0x004ca855
int __fastcall EH_Unwind_Zone_RunLobby_26(int ecx, int edx);
// 0x004ca860
int __fastcall EH_Unwind_Zone_RunLobby_27(int ecx, int edx);
// 0x004ca86b
int __fastcall EH_Unwind_Zone_RunLobby_28(int ecx, int edx);
// 0x004ca876
int __fastcall EH_Unwind_Zone_RunLobby_29(int ecx, int edx);
// 0x004ca881
int __fastcall EH_Unwind_Zone_RunLobby_30(int ecx, int edx);
// 0x004ca88c
int __fastcall EH_Unwind_Zone_RunLobby_31(int ecx, int edx);
// 0x004ca897
int __fastcall EH_Unwind_Zone_RunLobby_32(int ecx, int edx);
// 0x004ca8a2
int __fastcall EH_Unwind_Zone_RunLobby_33(int ecx, int edx);
// 0x004ca8ad
int __fastcall EH_Unwind_Zone_RunLobby_34(int ecx, int edx);
// 0x004ca8b8
int __fastcall EH_Unwind_Zone_RunLobby_35(int ecx, int edx);
// 0x004ca8c3
int __fastcall EH_Unwind_Zone_RunLobby_36(int ecx, int edx);
// 0x004ca8ce
int __fastcall EH_Unwind_Zone_RunLobby_37(int ecx, int edx);
// 0x004ca8d9
int __fastcall EH_Unwind_Zone_RunLobby_38(int ecx, int edx);
// 0x004ca8e4
int __fastcall EH_Unwind_Zone_RunLobby_39(int ecx, int edx);
// 0x004ca8ec
int __fastcall EH_Unwind_Zone_RunLobby_40(int ecx, int edx);
// 0x004ca8f4
int __fastcall EH_Unwind_Zone_RunLobby_41(int ecx, int edx);
// 0x004ca910
int __fastcall EH_Unwind_ZoneSink_Create_0(int ecx, int edx);
// 0x004ca91b
int __fastcall EH_Unwind_ZoneSink_Create_1(int ecx, int edx);
// 0x004ca930
int __fastcall EH_Unwind_ZoneSink_OnLaunchGame_0(int ecx, int edx);
// 0x004ca93b
int __fastcall EH_Unwind_ZoneSink_OnLaunchGame_1(int ecx, int edx);
// 0x004ca946
int __fastcall EH_Unwind_ZoneSink_OnLaunchGame_2(int ecx, int edx);
// 0x004ca960
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_0(int ecx, int edx);
// 0x004ca968
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_1(int ecx, int edx);
// 0x004ca970
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_2(int ecx, int edx);
// 0x004ca978
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_3(int ecx, int edx);
// 0x004ca980
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_4(int ecx, int edx);
// 0x004ca988
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_5(int ecx, int edx);
// 0x004ca990
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_6(int ecx, int edx);
// 0x004ca998
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsA_7(int ecx, int edx);
// 0x004ca9b0
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_0(int ecx, int edx);
// 0x004ca9b8
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_1(int ecx, int edx);
// 0x004ca9c0
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_2(int ecx, int edx);
// 0x004ca9c8
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_3(int ecx, int edx);
// 0x004ca9d0
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_4(int ecx, int edx);
// 0x004ca9d8
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_5(int ecx, int edx);
// 0x004ca9e0
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_6(int ecx, int edx);
// 0x004ca9e8
int __fastcall EH_Unwind_ZoneSink_OnRoomSettingsB_7(int ecx, int edx);
// 0x004caa00
int __fastcall EH_Unwind_ZoneSink_Dtor_0(int ecx, int edx);
// 0x004caa20
int __fastcall EH_Unwind_WolLoginDialog_Ctor_0(int ecx, int edx);
// 0x004caa28
int __fastcall EH_Unwind_WolLoginDialog_Ctor_1(int ecx, int edx);
// 0x004caa33
int __fastcall EH_Unwind_WolLoginDialog_Ctor_2(int ecx, int edx);
// 0x004caa41
int __fastcall EH_Unwind_WolLoginDialog_Ctor_3(int ecx, int edx);
// 0x004caa4f
int __fastcall EH_Unwind_WolLoginDialog_Ctor_4(int ecx, int edx);
// 0x004caa5d
int __fastcall EH_Unwind_WolLoginDialog_Ctor_5(int ecx, int edx);
// 0x004caa75
int __fastcall EH_Unwind_WolLoginDialog_Ctor_6(int ecx, int edx);
// 0x004caa8d
int __fastcall EH_Unwind_WolLoginDialog_Ctor_7(int ecx, int edx);
// 0x004caaa5
int __fastcall EH_Unwind_WolLoginDialog_Ctor_8(int ecx, int edx);
// 0x004caad0
int __fastcall EH_Unwind_ComboDialog_Dtor_0(int ecx, int edx);
// 0x004caad8
int __fastcall EH_Unwind_ComboDialog_Dtor_1(int ecx, int edx);
// 0x004caae3
int __fastcall EH_Unwind_ComboDialog_Dtor_2(int ecx, int edx);
// 0x004caaf1
int __fastcall EH_Unwind_ComboDialog_Dtor_3(int ecx, int edx);
// 0x004caaff
int __fastcall EH_Unwind_ComboDialog_Dtor_4(int ecx, int edx);
// 0x004cab0d
int __fastcall EH_Unwind_ComboDialog_Dtor_5(int ecx, int edx);
// 0x004cab25
int __fastcall EH_Unwind_ComboDialog_Dtor_6(int ecx, int edx);
// 0x004cab3d
int __fastcall EH_Unwind_ComboDialog_Dtor_7(int ecx, int edx);
// 0x004cb0c0
int __fastcall EH_Unwind_ZNet_AddNamedEntry_0(int ecx, int edx);
// 0x004ca438
int __fastcall EH_Handler_ZoneDlg_SendChatLine(int ecx, int edx);

// 0x004ca466
int __fastcall EH_Handler_Zone_LaunchSession(int ecx, int edx);

// 0x004ca57f
int __fastcall EH_Handler_ZoneDlg_ctor(int ecx, int edx);

// 0x004ca69f
int __fastcall EH_Handler_ZoneDlg_dtor(int ecx, int edx);

// 0x004ca6e8
int __fastcall EH_Handler_ZoneDlg_OnInitDialog(int ecx, int edx);

// 0x004ca70b
int __fastcall EH_Handler_ZoneDlg_EnterRoomByTypedName(int ecx, int edx);

// 0x004ca72b
int __fastcall EH_Handler_ZoneDlg_OnCreateRoom(int ecx, int edx);

// 0x004ca8fc
int __fastcall EH_Handler_Zone_RunLobby(int ecx, int edx);

// 0x004ca923
int __fastcall EH_Handler_ZoneSink_Create(int ecx, int edx);

// 0x004ca951
int __fastcall EH_Handler_ZoneSink_OnLaunchGame(int ecx, int edx);

// 0x004ca9a0
int __fastcall EH_Handler_ZoneSink_OnRoomSettingsA(int ecx, int edx);

// 0x004ca9f0
int __fastcall EH_Handler_ZoneSink_OnRoomSettingsB(int ecx, int edx);

// 0x004caa08
int __fastcall EH_Handler_ZoneSink_Dtor(int ecx, int edx);

// 0x004caabd
int __fastcall EH_Handler_WolLoginDialog_Ctor(int ecx, int edx);

// 0x004cab55
int __fastcall EH_Handler_ComboDialog_Dtor(int ecx, int edx);

// 0x004cb0cb
int __fastcall EH_Handler_ZNet_AddNamedEntry(int ecx, int edx);

// 0x0043ef10
int __fastcall DataTarget_0043ef10(int ecx, int edx);
// 0x0043efc0
int __fastcall DataTarget_0043efc0(int ecx, int edx);
// 0x0043efd0
int __fastcall DataTarget_0043efd0(int ecx, int edx);
// 0x0043f450
int __fastcall DataTarget_0043f450(int ecx, int edx);
// 0x0043e1c0
int __fastcall ZoneDlg_FormatAndSendChat_0043e1c0(int ecx, int edx);
// 0x0043e3a0
int __fastcall GlobalObj_Call_004c5b88_0053855c(int ecx, int edx);
}  // namespace recoil
