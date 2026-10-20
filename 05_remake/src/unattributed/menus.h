// SUBSYSTEM: menus
// Declarations for src/unattributed/menus.cpp.
#pragma once

namespace recoil {

// 0x00401020
int __fastcall VMethod_RetNop4(int ecx, int edx, int arg1);
// 0x00406a00
int __fastcall Cheat_ContainsUpper(int ecx, int edx);
// 0x00406cf0
int __fastcall Cheat_ClearInfiniteLives(int ecx, int edx);
// 0x00406ed0
int __fastcall ScreenHolder_Ctor_004ccd28(int ecx, int edx);
// 0x00407130
int __fastcall VMethod_ReturnTrue_00407130(int ecx, int edx);
// 0x00407e30
int __fastcall ControlFlags_SetBit0(int ecx, int edx);
// 0x00407e50
int __fastcall ControlFlags_GetBit0(int ecx, int edx);
// 0x00407e60
int __fastcall ControlFlags_SetBit1(int ecx, int edx);
// 0x00407e80
int __fastcall ControlFlags_GetBit1(int ecx, int edx);
// 0x00407e90
int __fastcall ControlFlags_SetBit2(int ecx, int edx);
// 0x00407eb0
int __fastcall ControlFlags_GetBit2(int ecx, int edx);
// 0x00407ef0
int __fastcall ControlFlags_GetBit3Mode(int ecx, int edx);
// 0x00407f80
int __fastcall Setting_Get_EffectsLevel(int ecx, int edx);
// 0x00408030
int __fastcall Setting_Get_004e5d10(int ecx, int edx);
// 0x00408060
int __fastcall Setting_Get_MuteSound(int ecx, int edx);
// 0x00408090
int __fastcall Setting_GetFloat_004e5d44(int ecx, int edx);
// 0x004080c0
int __fastcall Settings_StoreSoundLOD(int ecx, int edx);
// 0x004080d0
int __fastcall Setting_Get_SoundLOD(int ecx, int edx);
// 0x004080e0
int __fastcall Settings_StoreTextureMemory(int ecx, int edx);
// 0x00408100
int __fastcall Setting_Get_TextureMemory(int ecx, int edx);
// 0x00408190
int __fastcall Setting_Get_004e5d4c(int ecx, int edx);
// 0x004081f0
int __fastcall Setting_Get_GfxFlags(int ecx, int edx);
// 0x00408210
int __fastcall Settings_StoreCDAudio(int ecx, int edx);
// 0x00408220
int __fastcall Setting_Get_CDAudio(int ecx, int edx);
// 0x00408260
int __fastcall Settings_GetNetworkFlag(int ecx, int edx);
// 0x00408270
int __fastcall Setting_Get_004e5d90(int ecx, int edx);
// 0x00408310
int __fastcall Settings_GetHWCardFlag(int ecx, int edx);
// 0x00408360
int __fastcall Setting_Get_HUDType(int ecx, int edx);
// 0x00408380
int __fastcall Setting_Get_004e5d6c(int ecx, int edx);
// 0x00408390
int __fastcall Settings_StoreJoystickEnabled(int ecx, int edx);
// 0x004083c0
int __fastcall Setting_Get_004e5d60(int ecx, int edx);
// 0x00408430
int __fastcall Rect_ClampPointF(int ecx, int edx);
// 0x004084e0
int __fastcall Setting_GetObjectDetailScale(int ecx, int edx);
// 0x004085a0
int __fastcall Setting_Get_ViewportRect(int ecx, int edx);
// 0x00408650
int __fastcall Setting_Get_ScreenRect(int ecx, int edx);
// 0x004086b0
int __fastcall Setting_Get_DisplayMode(int ecx, int edx);
// 0x00408d60
int __fastcall ControlsHost_Ctor(int ecx, int edx);
// 0x00409990
int __fastcall CreditsScreen_Ctor(int ecx, int edx);
// 0x0040bc60
int __fastcall CommandsScreen_Ctor(int ecx, int edx);
// 0x0040bdc0
int __fastcall PtrVector_ClearEnd(int ecx, int edx);
// 0x0040be60
int __fastcall PtrCopyRange(int ecx, int edx, int arg1);
// 0x0040bf00
int __fastcall FreeOwnedPointer(int ecx, int edx);
// 0x0040d0b0
int __fastcall ScreenHolder_Ctor_004ce230(int ecx, int edx);
// 0x0040d220
int __fastcall ScoreRow_Before(int ecx, int edx);
// 0x0040d9d0
int __fastcall WidgetContainer_SetField4(int ecx, int edx, int arg1);
// 0x0040dac0
int __fastcall MenuImage_Ctor_4ce488(int ecx, int edx);
// 0x0040e010
int __fastcall ListLabel_SetColours(int ecx, int edx, int arg1, int arg2);
// 0x0040e040
int __fastcall ListLabel_SetShadow(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0040ef00
int __fastcall HudLabel_SetTriple(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0040f9e0
int __fastcall ListLabel_SetColour(int ecx, int edx, int arg1);
// 0x00414b60
int __fastcall Game_CanLoad(int ecx, int edx);
// 0x00414b90
int __fastcall Game_CanSave(int ecx, int edx);
// 0x00415170
int __fastcall MainMenuScreen_Ctor(int ecx, int edx);
// 0x00415670
int __fastcall SetGlobal_004edc68(int ecx, int edx);
// 0x00415850
int __fastcall GlobalObj_Ctor_004cef88(int ecx, int edx);
// 0x004159d0
int __fastcall VMethod_ReturnFalse8_004159d0(int ecx, int edx, int arg1, int arg2);
// 0x0041aba0
int __fastcall MPNewGameScreen_Ctor(int ecx, int edx);
// 0x0041c070
int __fastcall NetExitHost_Show(int ecx, int edx);
// 0x0041c080
int __fastcall NetExitHost_Call0(int ecx, int edx);
// 0x0041c0a0
int __fastcall NetExitHost_Release(int ecx, int edx);
// 0x0041c4e0
int __fastcall NewGamePanel_SyncIntensity(int ecx, int edx);
// 0x0041c5f0
int __fastcall StaticInit_004f32c8(int ecx, int edx);
// 0x0041eb30
int __fastcall MenuWidget_Ctor_4d05c0(int ecx, int edx);
// 0x0041eb90
int __fastcall MenuWidget_Ctor_4d0638(int ecx, int edx);
// 0x0042a9d0
int __fastcall PtrVector_Size(int ecx, int edx);
// 0x0042ee40
int __fastcall SetField4(int ecx, int edx, int arg1);
// 0x00431b70
int __fastcall MenuWrap_SetBaseVtbl(int ecx, int edx);
// 0x00434660
int __fastcall SaveRecord_IsNewer(int ecx, int edx);
// 0x004353f0
int __fastcall SaveLoadDialog_SelectIndex(int ecx, int edx, int arg1);
// 0x00435c80
int __fastcall SaveLoadScreen_Ctor(int ecx, int edx);
// 0x00443140
int __fastcall ScreenManager_GetCurrent(int ecx, int edx);
// 0x00443700
int __fastcall DequeIterator_Set(int ecx, int edx, int arg1, int arg2);
// 0x0045e0f0
int __fastcall SetGlobal_00539ea0(int ecx, int edx);
// 0x00462360
int __fastcall MciDevice_Dtor(int ecx, int edx);
// 0x0046f260
int __fastcall BitmapFont_MeasureString(int ecx, int edx, int arg1, int arg2);
// 0x00489f70
int __fastcall GetGlobal_0056aa44(int ecx, int edx);
// 0x004936d0
int __fastcall Draw_FillConvexPolygon2D(int ecx, int edx, int arg1);
// 0x00498bd0
int __fastcall Draw_LineViaHook(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00498c00
int __fastcall Draw_PolylineViaHook(int ecx, int edx, int arg1, int arg2);
// 0x00499020
int __fastcall Draw_CirclePlot8(int ecx, int edx, int arg1);
// 0x004a59e0
int __fastcall CDCheck_FindDiscDrive(int ecx, int edx, int arg1);
// 0x004a5b60
int __fastcall GetMessageByID(int ecx, int edx);
// 0x00406ea0
int __fastcall StaticInit_004e5ce8(int ecx, int edx);
// 0x00407f30
int __fastcall Settings_ApplyEffectsLevel(int ecx, int edx);
// 0x004089c0
int __fastcall Mouse_AdjustForPixelDoubling(int ecx, int edx);
// 0x00408d30
int __fastcall StaticInit_004e5dd0(int ecx, int edx);
// 0x00409960
int __fastcall CreditsScreen_StaticInit(int ecx, int edx);
// 0x0040bc30
int __fastcall CommandsScreen_StaticInit(int ecx, int edx);
// 0x0040d080
int __fastcall StaticInit_004e5e08(int ecx, int edx);
// 0x0040ea60
int __fastcall Scoreboard_IsLocalPlayerFirst(int ecx, int edx);
// 0x00414710
int __fastcall ScoreRows_QuickSort(int ecx, int edx, int arg1);
// 0x00414930
int __fastcall ScoreRows_InsertBackward(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12, int arg13, int arg14, int arg15, int arg16, int arg17, int arg18, int arg19, int arg20);
// 0x00414980
int __fastcall ScoreRows_InsertionSort(int ecx, int edx, int arg1);
// 0x00415110
int __fastcall MainMenuScreen_StaticInit(int ecx, int edx);
// 0x00415820
int __fastcall StaticInit_004edc48(int ecx, int edx);
// 0x0041ab70
int __fastcall MPNewGameScreen_StaticInit(int ecx, int edx);
// 0x00435a40
int __fastcall SaveLoadScreen_StaticInit(int ecx, int edx);
// 0x004362f0
int __fastcall SaveRecordSort_Quick(int ecx, int edx, int arg1);
// 0x00436530
int __fastcall SaveRecordSort_InsertOne(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12, int arg13, int arg14, int arg15, int arg16, int arg17, int arg18, int arg19, int arg20, int arg21, int arg22, int arg23, int arg24, int arg25, int arg26, int arg27, int arg28, int arg29, int arg30, int arg31, int arg32, int arg33, int arg34, int arg35, int arg36, int arg37, int arg38, int arg39, int arg40, int arg41, int arg42, int arg43, int arg44, int arg45, int arg46, int arg47, int arg48, int arg49, int arg50, int arg51, int arg52, int arg53, int arg54, int arg55, int arg56, int arg57, int arg58, int arg59, int arg60, int arg61, int arg62, int arg63, int arg64, int arg65, int arg66, int arg67, int arg68, int arg69, int arg70, int arg71, int arg72, int arg73, int arg74, int arg75, int arg76, int arg77, int arg78, int arg79, int arg80);
// 0x00436580
int __fastcall SaveRecordSort_Partition(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12, int arg13, int arg14, int arg15, int arg16, int arg17, int arg18, int arg19, int arg20, int arg21, int arg22, int arg23, int arg24, int arg25, int arg26, int arg27, int arg28, int arg29, int arg30, int arg31, int arg32, int arg33, int arg34, int arg35, int arg36, int arg37, int arg38, int arg39, int arg40, int arg41, int arg42, int arg43, int arg44, int arg45, int arg46, int arg47, int arg48, int arg49, int arg50, int arg51, int arg52, int arg53, int arg54, int arg55, int arg56, int arg57, int arg58, int arg59, int arg60, int arg61, int arg62, int arg63, int arg64, int arg65, int arg66, int arg67, int arg68, int arg69, int arg70, int arg71, int arg72, int arg73, int arg74, int arg75, int arg76, int arg77, int arg78, int arg79, int arg80);
// 0x00498fb0
int __fastcall Draw_CircleMidpoint(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004a5bf0
int __fastcall Message_GetText(int ecx, int edx);
// 0x0040e140
int __fastcall Scoreboard_Refresh(int ecx, int edx);
// 0x004143a0
int __fastcall Thunk_0040ea60_OnMember34(int ecx, int edx);
// 0x00408070
int __fastcall Settings_ApplySoundVolume(int ecx, int edx, int arg1);
// 0x00408fa0
int __fastcall ScreenHost_CloseModalKeep(int ecx, int edx, int arg1);
// 0x00415370
int __fastcall MainMenuScreen_OnActivate(int ecx, int edx, int arg1);
// 0x00471780
int __fastcall InputMgr_SetMouseButtonBinding(int ecx, int edx);
// 0x0048e380
int __fastcall Blur16_Full(int ecx, int edx);
// 0x0048e670
int __fastcall Blur16_Vertical(int ecx, int edx);
// 0x0048e870
int __fastcall Blur16_Horizontal(int ecx, int edx);
// 0x00415630
int __fastcall StopGlobalVoice_004edc6c(int ecx, int edx);
// 0x0048d6d0
int __fastcall Screen_SetFadeFill(int ecx, int edx, int arg1, int arg2);
// 0x00409470
int __fastcall CreditsPanel_LayoutScroll(int ecx, int edx, int arg1);
// 0x0040be00
int __fastcall OwnedStringPtrs_DestroyRange(int ecx, int edx, int arg1, int arg2);
// 0x0040bf20
int __fastcall DeleteOwnedString(int ecx, int edx, int arg1);
// 0x0040bf50
int __fastcall OwnedString_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0040c1d0
int __fastcall CommandsDialog_ClearStringList(int ecx, int edx);
// 0x0040e910
int __fastcall NetScoreboard_ComputeLayout(int ecx, int edx, int arg1);
// 0x0040ff80
int __fastcall Hud_SetLayoutRects(int ecx, int edx);
// 0x004159e0
int __fastcall Screen_RunModalLoop(int ecx, int edx);
// 0x00415a80
int __fastcall ScalarDeletingDtor_00415a80(int ecx, int edx, int arg1);
// 0x00435fd0
int __fastcall SaveRecordVector_InsertN(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00443690
int __fastcall Deque_GrowMap(int ecx, int edx, int arg1);
// 0x00471c50
int __fastcall Input_ResetAll(int ecx, int edx);
// 0x0048ea00
int __fastcall Blit_DispatchByMode(int ecx, int edx);
// 0x004bc570
int __fastcall WidgetRoot_DispatchMouseAndTick(int ecx, int edx, int arg1);
// 0x0040b980
int __fastcall CommandsDialog_ScrollAllColumns(int ecx, int edx, int arg1);
// 0x0040eab0
int __fastcall HudComposite_Refresh(int ecx, int edx, int arg1);
// 0x0040ff50
int __fastcall Hud_ResetMessageState(int ecx, int edx);
// 0x004355e0
int __fastcall SaveLoadDialog_EnumerateSaves(int ecx, int edx);
// 0x00443160
int __fastcall ScreenManager_QueuePush(int ecx, int edx, int arg1, int arg2);
// 0x00443310
int __fastcall ScreenManager_QueueReplace(int ecx, int edx, int arg1, int arg2);
// 0x004434b0
int __fastcall ScreenManager_QueuePop(int ecx, int edx, int arg1);
// 0x00407110
int __fastcall ScreenHolder_Close_004e5ce8(int ecx, int edx);
// 0x00408ff0
int __fastcall Screen_Enter_004e5dd0(int ecx, int edx);
// 0x00409380
int __fastcall CreditsPanel_Tick(int ecx, int edx, int arg1);
// 0x00409b00
int __fastcall Screen_Enter_004e5de0(int ecx, int edx);
// 0x0040b630
int __fastcall ScreenList_Scroll(int ecx, int edx, int arg1);
// 0x0040bda0
int __fastcall Screen_Enter_004e5df0(int ecx, int edx);
// 0x0040d1c0
int __fastcall Screen_Enter_004e5e08(int ecx, int edx);
// 0x004159b0
int __fastcall Screen_Enter_004edc48(int ecx, int edx);
// 0x0041ad80
int __fastcall Screen_Enter_004f32a0_WithArg(int ecx, int edx);
// 0x0041c500
int __fastcall NewGamePanel_OnStart(int ecx, int edx);
// 0x0041c6c0
int __fastcall Screen_Enter_004f32c8(int ecx, int edx);
// 0x00434ee0
int __fastcall SaveLoadDialog_FillList(int ecx, int edx);
// 0x00435f50
int __fastcall SaveGameScreen_Open(int ecx, int edx);
// 0x00435f80
int __fastcall LoadGameScreen_Open(int ecx, int edx);
// 0x0040a170
int __fastcall ListLabel_CopyRange(int ecx, int edx, int arg1);
// 0x0040a1e0
int __fastcall TextPanelLabel_Assign(int ecx, int edx, int arg1);
// 0x004624f0
int __fastcall MciDevice_StopAndClose(int ecx, int edx);
// 0x0048f500
int __fastcall Draw_ImageAt(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00408f50
int __fastcall ScreenHost_CloseModal(int ecx, int edx, int arg1);
// 0x004099a0
int __fastcall Panel_OnShow_004099a0(int ecx, int edx, int arg1);
// 0x00409ad0
int __fastcall CreditsScreen_Destroy(int ecx, int edx);
// 0x00415960
int __fastcall ScreenHost_CloseCurrent(int ecx, int edx);
// 0x0041ad20
int __fastcall MPNewGameScreen_Destroy(int ecx, int edx);
// 0x00401000
int __fastcall Dialog67_Ctor(int ecx, int edx, int arg1);
// 0x00407fa0
int __fastcall Settings_ApplyObjectLOD(int ecx, int edx);
// 0x0040d1f0
int __fastcall StaticInit_004ed714(int ecx, int edx);
// 0x0041af50
int __fastcall NameDlg_DoDataExchange(int ecx, int edx, int arg1);
// 0x0041b680
int __fastcall NameDlg_OnDestroy(int ecx, int edx);
// 0x0041b6a0
int __fastcall NameDlg_ValidateName(int ecx, int edx);
// 0x0041b8d0
int __fastcall MfcWrap_ScalarDeletingDtor_004c5c66(int ecx, int edx, int arg1);
// 0x0041b8f0
int __fastcall MfcWrap_ScalarDeletingDtor_004c5c60(int ecx, int edx, int arg1);
// 0x0041b910
int __fastcall MfcWrap_ScalarDeletingDtor_004c5c5a(int ecx, int edx, int arg1);
// 0x0041b930
int __fastcall MfcWrap_ScalarDeletingDtor_004c5c6c(int ecx, int edx, int arg1);
// 0x0041c0c0
int __fastcall NetSetupDialog_OnDestroy(int ecx, int edx);
// 0x0041c990
int __fastcall StaticInit_LevelNames(int ecx, int edx);
// 0x0041ca30
int __fastcall NetSetupDialog_OnInitDialog(int ecx, int edx);
// 0x0041cb50
int __fastcall DialogCombo_OnDestroy(int ecx, int edx);
// 0x0041cb90
int __fastcall DialogCombo_OnSelChange(int ecx, int edx);
// 0x0041cbf0
int __fastcall MfcWrap_ScalarDeletingDtor_004c5cc6(int ecx, int edx, int arg1);
// 0x004a56d0
int __fastcall Timer_UpdateFrame(int ecx, int edx);
// 0x00435e80
int __fastcall SaveLoadScreen_Tick(int ecx, int edx);
// 0x004082d0
int __fastcall Settings_ApplyHUDType(int ecx, int edx);
// 0x004153d0
int __fastcall MenuScreen_OnLeave_ApplySettings_004153d0(int ecx, int edx);
// 0x00401030
int __fastcall Slot_ReturnConstAddr_004cc718_00401030(int ecx, int edx);
// 0x00404b30
int __fastcall App_SetGlobal_004da24c_00404b30(int ecx, int edx);
// 0x0040d210
int __fastcall AtexitStub_CString_Dtor_Thunk_0040d210(int ecx, int edx);
// 0x0040f2d0
int __fastcall Menus_Slot_Call4b3d00_0040f2d0(int ecx, int edx);
// 0x0040fbb0
int __fastcall Menus_Slot_0040fbb0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00411710
int __fastcall Stub_Ret_00411710(int ecx, int edx);
// 0x0041afd0
int __fastcall Slot_ReturnConstAddr_004cfbd0_0041afd0(int ecx, int edx);
// 0x0041c970
int __fastcall Slot_ReturnConstAddr_004d03d0_0041c970(int ecx, int edx);
// 0x0040d200
int __fastcall StaticInit_RegisterAtexit_0040d210(int ecx, int edx);
// 0x004716b0
int __fastcall Thunk_00470820(int ecx, int edx);
// 0x00471950
int __fastcall InputContext_Pop(int ecx, int edx);
// 0x0040bd60
int __fastcall CommandsScreen_Destroy(int ecx, int edx);
// 0x00419940
int __fastcall MPExitScreen_OnLeave_00419940(int ecx, int edx);
// 0x00401040
int __fastcall Slot_EnableWindow_ThisHwnd_0_00401040(int ecx, int edx);
// 0x00401050
int __fastcall Slot_EnableWindow_ThisHwnd_1_00401050(int ecx, int edx);
// 0x00404620
int __fastcall Slot_CallVirtual60_Arg0_Ret1_00404620(int ecx, int edx, int arg1);
// 0x0041c880
int __fastcall Dlg_DoDataExchange_0041c880(int ecx, int edx, int arg1);
// 0x0041ca10
int __fastcall Menus_DestroyCStringArray_004f32d8(int ecx, int edx);
// 0x0041ca00
int __fastcall StaticInit_RegisterAtexit_0041ca10(int ecx, int edx);
// 0x00404bb0
int __fastcall Cb_IsIntGlobalAtLeastFloatField4_00404bb0(int ecx, int edx, int arg1);
// 0x004069f0
int __fastcall Wnd_OnCreate_ReturnMinus1IfDefaultFails_004069f0(int ecx, int edx, int arg1);
// 0x0040d1e0
int __fastcall StaticInitWrapper_0040d1e0(int ecx, int edx);
// 0x0041c980
int __fastcall StaticInitWrapper_0041c980(int ecx, int edx);
// 0x004081a0
int __fastcall Settings_ApplyGfxFlags(int ecx, int edx);
// 0x00404280
int __fastcall Menus_RunScreenLoop_00404280(int ecx, int edx);
// 0x00403c10
int __fastcall LoadingRing_Ctor(int ecx, int edx);
// 0x00403e20
int __fastcall ListWidget_Dtor(int ecx, int edx);
// 0x00406e90
int __fastcall StaticInitWrapper_00406e90(int ecx, int edx);
// 0x00406eb0
int __fastcall StaticInit_RegisterAtexit_00406ec0(int ecx, int edx);
// 0x00406ec0
int __fastcall AtexitStub_ScreenHolder_Dtor_00406ec0(int ecx, int edx);
// 0x00406ee0
int __fastcall ScreenHolder_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00406f00
int __fastcall ScreenHolder_Dtor(int ecx, int edx);
// 0x00407ec0
int __fastcall ControlFlags_SetBit3AndCamera(int ecx, int edx);
// 0x00408050
int __fastcall Settings_ApplyMuteSound(int ecx, int edx);
// 0x00408d20
int __fastcall StaticInitWrapper_00408d20(int ecx, int edx);
// 0x00408d40
int __fastcall StaticInit_RegisterAtexit_00408d50(int ecx, int edx);
// 0x00408d50
int __fastcall AtexitStub_ControlsHost_Dtor_00408d50(int ecx, int edx);
// 0x00408d70
int __fastcall ControlsHost_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00408d90
int __fastcall ControlsHost_Dtor(int ecx, int edx);
// 0x00408ec0
int __fastcall ControlsHost_CloseAndApply(int ecx, int edx);
// 0x004091e0
int __fastcall TextPanel_Dtor(int ecx, int edx);
// 0x00409910
int __fastcall ListLabelVector_Free(int ecx, int edx);
// 0x00409950
int __fastcall StaticInitWrapper_00409950(int ecx, int edx);
// 0x00409970
int __fastcall CreditsScreen_StaticAtexit(int ecx, int edx);
// 0x00409980
int __fastcall AtexitStub_CreditsScreen_Dtor_00409980(int ecx, int edx);
// 0x004099d0
int __fastcall CreditsScreen_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x004099f0
int __fastcall CreditsScreen_Dtor(int ecx, int edx);
// 0x00409b20
int __fastcall CreditsSection_Dtor(int ecx, int edx);
// 0x00409b60
int __fastcall ListLabel_DestroyRange(int ecx, int edx, int arg1, int arg2);
// 0x00409b90
int __fastcall ListLabelVector_InsertN(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00409ef0
int __fastcall ListLabel_DtorThunk(int ecx, int edx, int arg1);
// 0x00409f00
int __fastcall GroupVector_InsertN(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0040a210
int __fastcall TextPanelLabel_CopyCtor(int ecx, int edx, int arg1);
// 0x0040a240
int __fastcall TextGroup_CopyCtor(int ecx, int edx, int arg1);
// 0x0040a300
int __fastcall TextGroupVector_Assign(int ecx, int edx, int arg1);
// 0x0040a940
int __fastcall OptionListScreen_Dtor_a940(int ecx, int edx);
// 0x0040aa30
int __fastcall OptionListScreen_Dtor_aa30(int ecx, int edx);
// 0x0040ab20
int __fastcall OptionListScreen_Dtor_ab20(int ecx, int edx);
// 0x0040ac10
int __fastcall OptionListScreen_Dtor_ac10(int ecx, int edx);
// 0x0040ad00
int __fastcall OptionListScreen_Dtor_ad00(int ecx, int edx);
// 0x0040b3e0
int __fastcall Screen_LeaveAndResetSelection(int ecx, int edx, int arg1, int arg2);
// 0x0040b460
int __fastcall CommandsDialog_OnCaptureKey(int ecx, int edx, int arg1, int arg2);
// 0x0040b4e0
int __fastcall CommandsDialog_OnCaptureJoystickButton(int ecx, int edx, int arg1, int arg2);
// 0x0040b560
int __fastcall CommandsDialog_OnCaptureMouseButton(int ecx, int edx, int arg1, int arg2);
// 0x0040b5e0
int __fastcall ScreenList_StepWrap(int ecx, int edx, int arg1);
// 0x0040b680
int __fastcall CommandsDialog_RefreshBindingTable(int ecx, int edx, int arg1);
// 0x0040bc20
int __fastcall StaticInitWrapper_0040bc20(int ecx, int edx);
// 0x0040bc40
int __fastcall CommandsScreen_StaticAtexit(int ecx, int edx);
// 0x0040bc50
int __fastcall AtexitStub_CommandsScreen_Dtor_0040bc50(int ecx, int edx);
// 0x0040bc70
int __fastcall CommandsScreen_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x0040bc90
int __fastcall CommandsScreen_Dtor(int ecx, int edx);
// 0x0040bf80
int __fastcall CommandsDialog_AddString(int ecx, int edx, int arg1, int arg2);
// 0x0040c280
int __fastcall OptionListScreen_Dtor_NoSEH(int ecx, int edx);
// 0x0040d070
int __fastcall StaticInitWrapper_0040d070(int ecx, int edx);
// 0x0040d090
int __fastcall StaticInit_RegisterAtexit_0040d0a0(int ecx, int edx);
// 0x0040d0a0
int __fastcall AtexitStub_ScreenHolder_004ce230_Dtor_0040d0a0(int ecx, int edx);
// 0x0040d0c0
int __fastcall ScreenHolder_004ce230_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0040d0e0
int __fastcall ScreenHolder_004ce230_Dtor(int ecx, int edx);
// 0x0040d260
int __fastcall StaticInitWrapper_0040d260(int ecx, int edx);
// 0x0040d270
int __fastcall StaticInit_004eda68(int ecx, int edx);
// 0x0040d280
int __fastcall StaticInit_RegisterAtexit_0040d290(int ecx, int edx);
// 0x0040d290
int __fastcall AtexitStub_StaticObj_Dtor_ImageWidget_0040d290(int ecx, int edx);
// 0x0040d2a0
int __fastcall StaticObj_Dtor_ImageWidget_0040d2a0(int ecx, int edx);
// 0x0040d2f0
int __fastcall StaticInitWrapper_0040d2f0(int ecx, int edx);
// 0x0040d300
int __fastcall StaticInit_004ed718(int ecx, int edx);
// 0x0040d310
int __fastcall StaticInit_RegisterAtexit_0040d320(int ecx, int edx);
// 0x0040d320
int __fastcall AtexitStub_StaticObj_Dtor_4Images_0040d320(int ecx, int edx);
// 0x0040d330
int __fastcall StaticObj_Dtor_4Images(int ecx, int edx);
// 0x0040d3b0
int __fastcall StaticObj_Dtor_1Image(int ecx, int edx);
// 0x0040d400
int __fastcall StaticInitWrapper_0040d400(int ecx, int edx);
// 0x0040d410
int __fastcall StaticInit_004e5ed0(int ecx, int edx);
// 0x0040d420
int __fastcall StaticInit_RegisterAtexit_0040d430(int ecx, int edx);
// 0x0040d430
int __fastcall AtexitStub_HudPanel_Dtor_0040d430(int ecx, int edx);
// 0x0040d440
int __fastcall HudPanel_Dtor(int ecx, int edx);
// 0x0040d590
int __fastcall Screen_Dtor_ImageLabelImage(int ecx, int edx);
// 0x0040d600
int __fastcall Thunk_This3C_ToDtor(int ecx, int edx);
// 0x0040d610
int __fastcall Screen_Dtor_3Images(int ecx, int edx);
// 0x0040d660
int __fastcall MenuImageLabelImage_Dtor(int ecx, int edx);
// 0x0040d6e0
int __fastcall MenuImagePair_Dtor(int ecx, int edx);
// 0x0040d740
int __fastcall ArrayDtor_0040d740(int ecx, int edx);
// 0x0040d760
int __fastcall ArrayDtor_0040d760(int ecx, int edx);
// 0x0040d780
int __fastcall Screen_Dtor_2Images(int ecx, int edx);
// 0x0040d7e0
int __fastcall HudPanel_Ctor_4ce278(int ecx, int edx);
// 0x0040d9e0
int __fastcall MenuTextField_Ctor_4ce398(int ecx, int edx);
// 0x0040da00
int __fastcall MenuImageLabelImage_Ctor_4ce410(int ecx, int edx);
// 0x0040db00
int __fastcall ArrayDtor_0040db00(int ecx, int edx);
// 0x0040db20
int __fastcall MenuImagePair_Ctor_4ce500(int ecx, int edx);
// 0x0040dbf0
int __fastcall HudLabel_Ctor_4ce578(int ecx, int edx);
// 0x0040dcd0
int __fastcall NetScoreboard_Ctor(int ecx, int edx);
// 0x0040e070
int __fastcall NetScoreboard_Dtor(int ecx, int edx);
// 0x0040ed80
int __fastcall HudLabel_Ctor_4ce6b0(int ecx, int edx);
// 0x0040ef60
int __fastcall HudLabel_Ctor_4ce7d8(int ecx, int edx);
// 0x0040f200
int __fastcall MenuImageTriple_Ctor_4ce870(int ecx, int edx);
// 0x0040f4c0
int __fastcall Hud_InitWidgets(int ecx, int edx);
// 0x0040fa40
int __fastcall Screen_Dtor_OwnsObject34(int ecx, int edx);
// 0x0040fab0
int __fastcall Menus_Slot_Call40fac0_0040fab0(int ecx, int edx);
// 0x0040fac0
int __fastcall HudLabel_Ctor(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0040fb70
int __fastcall MenuTextField_Ctor_4ce320(int ecx, int edx);
// 0x00415100
int __fastcall StaticInitWrapper_00415100(int ecx, int edx);
// 0x00415120
int __fastcall MainMenuScreen_StaticAtexit(int ecx, int edx);
// 0x00415130
int __fastcall AtexitStub_MainMenuScreen_Dtor_00415130(int ecx, int edx);
// 0x00415190
int __fastcall MainMenuScreen_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x004151b0
int __fastcall MainMenuScreen_Dtor(int ecx, int edx);
// 0x00415810
int __fastcall StaticInitWrapper_00415810(int ecx, int edx);
// 0x00415830
int __fastcall StaticInit_RegisterAtexit_00415840(int ecx, int edx);
// 0x00415840
int __fastcall AtexitStub_GlobalObj_Dtor_004cef88_00415840(int ecx, int edx);
// 0x00415860
int __fastcall GlobalObj_ScalarDeletingDtor_00415880(int ecx, int edx, int arg1);
// 0x00415880
int __fastcall GlobalObj_Dtor_004cef88(int ecx, int edx);
// 0x00419990
int __fastcall MPExitScreen_Tick_00419990(int ecx, int edx);
// 0x0041a190
int __fastcall MenuEditField_Ctor(int ecx, int edx, int arg1);
// 0x0041a200
int __fastcall MenuIntEditField_Ctor(int ecx, int edx, int arg1);
// 0x0041ab60
int __fastcall StaticInitWrapper_0041ab60(int ecx, int edx);
// 0x0041ab80
int __fastcall MPNewGameScreen_StaticAtexit(int ecx, int edx);
// 0x0041ab90
int __fastcall AtexitStub_MPNewGameScreen_Dtor_0041ab90(int ecx, int edx);
// 0x0041abc0
int __fastcall MPNewGameScreen_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x0041abe0
int __fastcall MPNewGameScreen_Dtor(int ecx, int edx);
// 0x0041ada0
int __fastcall NameDlg_ctor(int ecx, int edx, int arg1);
// 0x0041ae90
int __fastcall NameDlg_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x0041aeb0
int __fastcall NameDlg_dtor(int ecx, int edx);
// 0x0041afe0
int __fastcall NetConnectDlg_OnInitDialog_0041afe0(int ecx, int edx);
// 0x0041b150
int __fastcall NameDlg_RefreshSessionList(int ecx, int edx);
// 0x0041b510
int __fastcall NameDlg_OnOK(int ecx, int edx);
// 0x0041b5a0
int __fastcall Dlg_OnOKConnectModem_0041b5a0(int ecx, int edx);
// 0x0041b660
int __fastcall NameDlg_OnTimer(int ecx, int edx, int arg1);
// 0x0041bd20
int __fastcall Cheat_RevivePlayerAndApplyPickup(int ecx, int edx);
// 0x0041c130
int __fastcall Panel_OnComboSelect(int ecx, int edx);
// 0x0041c170
int __fastcall AiDebugPanel_SetParamLabels(int ecx, int edx);
// 0x0041c5e0
int __fastcall StaticInitWrapper_0041c5e0(int ecx, int edx);
// 0x0041c610
int __fastcall ScreenHolder32c8_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0041c630
int __fastcall ScreenHolder32c8_Dtor(int ecx, int edx);
// 0x0041c6a0
int __fastcall StaticInit_RegisterAtexit_0041c6b0(int ecx, int edx);
// 0x0041c6b0
int __fastcall AtexitStub_ScreenHolder32c8_Dtor_0041c6b0(int ecx, int edx);
// 0x0041c6e0
int __fastcall NetSetupDialog_Ctor(int ecx, int edx, int arg1);
// 0x0041c7d0
int __fastcall Panel_0041c7f0_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0041c7f0
int __fastcall NetSetupDialog_Dtor(int ecx, int edx);
// 0x0042a550
int __fastcall ControlsScreen_BuildBindingList(int ecx, int edx);
// 0x00431b80
int __fastcall MenuWrap_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00431ba0
int __fastcall MenuWrap_Dtor(int ecx, int edx);
// 0x00434920
int __fastcall ListLabel_Subclass_Ctor_00434920(int ecx, int edx);
// 0x00434dc0
int __fastcall Thunk_00435a70(int ecx, int edx);
// 0x00435a30
int __fastcall StaticInitWrapper_00435a30(int ecx, int edx);
// 0x00435a50
int __fastcall SaveLoadScreen_StaticAtexit(int ecx, int edx);
// 0x00435a60
int __fastcall AtexitStub_SaveLoadScreen_Dtor_00435a60(int ecx, int edx);
// 0x00435ca0
int __fastcall SaveLoadScreen_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00435cc0
int __fastcall SaveLoadScreen_Dtor(int ecx, int edx);
// 0x00435ed0
int __fastcall SaveLoadScreen_Destroy(int ecx, int edx);
// 0x00471860
int __fastcall InputContext_Push(int ecx, int edx);
// 0x004c8238
int __fastcall EH_Unwind_BriefingScreen_Ctor_1(int ecx, int edx);
// 0x004c8246
int __fastcall EH_Unwind_BriefingScreen_Ctor_2(int ecx, int edx);
// 0x004c8254
int __fastcall EH_Unwind_BriefingScreen_Ctor_3(int ecx, int edx);
// 0x004c8262
int __fastcall EH_Unwind_BriefingScreen_Ctor_4(int ecx, int edx);
// 0x004c8270
int __fastcall EH_Unwind_BriefingScreen_Ctor_5(int ecx, int edx);
// 0x004c827e
int __fastcall EH_Unwind_BriefingScreen_Ctor_6(int ecx, int edx);
// 0x004c8286
int __fastcall EH_Unwind_BriefingScreen_Ctor_7(int ecx, int edx);
// 0x004c8294
int __fastcall EH_Unwind_BriefingScreen_Ctor_8(int ecx, int edx);
// 0x004c82a2
int __fastcall EH_Unwind_BriefingScreen_Ctor_9(int ecx, int edx);
// 0x004c82b0
int __fastcall EH_Unwind_BriefingScreen_Ctor_10(int ecx, int edx);
// 0x004c82e0
int __fastcall EH_Unwind_LoadingRing_Ctor_0(int ecx, int edx);
// 0x004c8300
int __fastcall EH_Unwind_ListWidget_Dtor_0(int ecx, int edx);
// 0x004c8458
int __fastcall EH_Unwind_CheatCodeScreen_Ctor_1(int ecx, int edx);
// 0x004c8466
int __fastcall EH_Unwind_CheatCodeScreen_Ctor_2(int ecx, int edx);
// 0x004c846e
int __fastcall EH_Unwind_CheatCodeScreen_Ctor_3(int ecx, int edx);
// 0x004c8498
int __fastcall EH_Unwind_CheatScreen_Dtor_1(int ecx, int edx);
// 0x004c84b0
int __fastcall EH_Unwind_ScreenHolder_Dtor_0(int ecx, int edx);
// 0x004c84d0
int __fastcall EH_Unwind_CheatScreen_Open_0(int ecx, int edx);
// 0x004c84f0
int __fastcall EH_Unwind_CheatScreen_CloseAndApply_0(int ecx, int edx);
// 0x004c8518
int __fastcall EH_Unwind_ControlsDialog_Ctor_1(int ecx, int edx);
// 0x004c8526
int __fastcall EH_Unwind_ControlsDialog_Ctor_2(int ecx, int edx);
// 0x004c8534
int __fastcall EH_Unwind_ControlsDialog_Ctor_3(int ecx, int edx);
// 0x004c8542
int __fastcall EH_Unwind_ControlsDialog_Ctor_4(int ecx, int edx);
// 0x004c8550
int __fastcall EH_Unwind_ControlsDialog_Ctor_5(int ecx, int edx);
// 0x004c855e
int __fastcall EH_Unwind_ControlsDialog_Ctor_6(int ecx, int edx);
// 0x004c856c
int __fastcall EH_Unwind_ControlsDialog_Ctor_7(int ecx, int edx);
// 0x004c8598
int __fastcall EH_Unwind_ControlsDialog_Dtor_1(int ecx, int edx);
// 0x004c85a6
int __fastcall EH_Unwind_ControlsDialog_Dtor_2(int ecx, int edx);
// 0x004c85b4
int __fastcall EH_Unwind_ControlsDialog_Dtor_3(int ecx, int edx);
// 0x004c85c2
int __fastcall EH_Unwind_ControlsDialog_Dtor_4(int ecx, int edx);
// 0x004c85d0
int __fastcall EH_Unwind_ControlsDialog_Dtor_5(int ecx, int edx);
// 0x004c85de
int __fastcall EH_Unwind_ControlsDialog_Dtor_6(int ecx, int edx);
// 0x004c8600
int __fastcall EH_Unwind_ControlsHost_Dtor_0(int ecx, int edx);
// 0x004c8620
int __fastcall EH_Unwind_ControlsHost_Open_0(int ecx, int edx);
// 0x004c8648
int __fastcall EH_Unwind_CreditsPanel_Ctor_1(int ecx, int edx);
// 0x004c8656
int __fastcall EH_Unwind_CreditsPanel_Ctor_2(int ecx, int edx);
// 0x004c8664
int __fastcall EH_Unwind_CreditsPanel_Ctor_3(int ecx, int edx);
// 0x004c8680
int __fastcall EH_Unwind_TextPanel_Dtor_0(int ecx, int edx);
// 0x004c86a8
int __fastcall EH_Unwind_CreditsPanel_Dtor_1(int ecx, int edx);
// 0x004c86b6
int __fastcall EH_Unwind_CreditsPanel_Dtor_2(int ecx, int edx);
// 0x004c86c4
int __fastcall EH_Unwind_CreditsPanel_Dtor_3(int ecx, int edx);
// 0x004c8700
int __fastcall EH_Unwind_CreditsScreen_Dtor_0(int ecx, int edx);
// 0x004c8720
int __fastcall EH_Unwind_CreditsScreen_Create_0(int ecx, int edx);
// 0x004c8748
int __fastcall EH_Unwind_CommandsDialog_Ctor_1(int ecx, int edx);
// 0x004c8756
int __fastcall EH_Unwind_CommandsDialog_Ctor_2(int ecx, int edx);
// 0x004c8764
int __fastcall EH_Unwind_CommandsDialog_Ctor_3(int ecx, int edx);
// 0x004c8772
int __fastcall EH_Unwind_CommandsDialog_Ctor_4(int ecx, int edx);
// 0x004c8780
int __fastcall EH_Unwind_CommandsDialog_Ctor_5(int ecx, int edx);
// 0x004c878e
int __fastcall EH_Unwind_CommandsDialog_Ctor_6(int ecx, int edx);
// 0x004c879c
int __fastcall EH_Unwind_CommandsDialog_Ctor_7(int ecx, int edx);
// 0x004c87aa
int __fastcall EH_Unwind_CommandsDialog_Ctor_8(int ecx, int edx);
// 0x004c87b8
int __fastcall EH_Unwind_CommandsDialog_Ctor_9(int ecx, int edx);
// 0x004c87c6
int __fastcall EH_Unwind_CommandsDialog_Ctor_10(int ecx, int edx);
// 0x004c87d4
int __fastcall EH_Unwind_CommandsDialog_Ctor_11(int ecx, int edx);
// 0x004c87e2
int __fastcall EH_Unwind_CommandsDialog_Ctor_12(int ecx, int edx);
// 0x004c87f0
int __fastcall EH_Unwind_CommandsDialog_Ctor_13(int ecx, int edx);
// 0x004c87fe
int __fastcall EH_Unwind_CommandsDialog_Ctor_14(int ecx, int edx);
// 0x004c8820
int __fastcall EH_Unwind_OptionListScreen_Dtor_a940_0(int ecx, int edx);
// 0x004c8828
int __fastcall EH_Unwind_OptionListScreen_Dtor_a940_1(int ecx, int edx);
// 0x004c8836
int __fastcall EH_Unwind_OptionListScreen_Dtor_a940_2(int ecx, int edx);
// 0x004c8850
int __fastcall EH_Unwind_OptionListScreen_Dtor_aa30_0(int ecx, int edx);
// 0x004c8858
int __fastcall EH_Unwind_OptionListScreen_Dtor_aa30_1(int ecx, int edx);
// 0x004c8866
int __fastcall EH_Unwind_OptionListScreen_Dtor_aa30_2(int ecx, int edx);
// 0x004c8880
int __fastcall EH_Unwind_OptionListScreen_Dtor_ab20_0(int ecx, int edx);
// 0x004c8888
int __fastcall EH_Unwind_OptionListScreen_Dtor_ab20_1(int ecx, int edx);
// 0x004c8896
int __fastcall EH_Unwind_OptionListScreen_Dtor_ab20_2(int ecx, int edx);
// 0x004c88b0
int __fastcall EH_Unwind_OptionListScreen_Dtor_ac10_0(int ecx, int edx);
// 0x004c88b8
int __fastcall EH_Unwind_OptionListScreen_Dtor_ac10_1(int ecx, int edx);
// 0x004c88c6
int __fastcall EH_Unwind_OptionListScreen_Dtor_ac10_2(int ecx, int edx);
// 0x004c88e0
int __fastcall EH_Unwind_OptionListScreen_Dtor_ad00_0(int ecx, int edx);
// 0x004c88e8
int __fastcall EH_Unwind_OptionListScreen_Dtor_ad00_1(int ecx, int edx);
// 0x004c88f6
int __fastcall EH_Unwind_OptionListScreen_Dtor_ad00_2(int ecx, int edx);
// 0x004c8918
int __fastcall EH_Unwind_CommandsDialog_Dtor_1(int ecx, int edx);
// 0x004c8926
int __fastcall EH_Unwind_CommandsDialog_Dtor_2(int ecx, int edx);
// 0x004c8934
int __fastcall EH_Unwind_CommandsDialog_Dtor_3(int ecx, int edx);
// 0x004c8942
int __fastcall EH_Unwind_CommandsDialog_Dtor_4(int ecx, int edx);
// 0x004c8950
int __fastcall EH_Unwind_CommandsDialog_Dtor_5(int ecx, int edx);
// 0x004c895e
int __fastcall EH_Unwind_CommandsDialog_Dtor_6(int ecx, int edx);
// 0x004c896c
int __fastcall EH_Unwind_CommandsDialog_Dtor_7(int ecx, int edx);
// 0x004c897a
int __fastcall EH_Unwind_CommandsDialog_Dtor_8(int ecx, int edx);
// 0x004c8988
int __fastcall EH_Unwind_CommandsDialog_Dtor_9(int ecx, int edx);
// 0x004c8996
int __fastcall EH_Unwind_CommandsDialog_Dtor_10(int ecx, int edx);
// 0x004c89a4
int __fastcall EH_Unwind_CommandsDialog_Dtor_11(int ecx, int edx);
// 0x004c89b2
int __fastcall EH_Unwind_CommandsDialog_Dtor_12(int ecx, int edx);
// 0x004c89c0
int __fastcall EH_Unwind_CommandsDialog_Dtor_13(int ecx, int edx);
// 0x004c89ce
int __fastcall EH_Unwind_CommandsDialog_Dtor_14(int ecx, int edx);
// 0x004c89d6
int __fastcall EH_Unwind_CommandsDialog_Dtor_15(int ecx, int edx);
// 0x004c89e4
int __fastcall EH_Unwind_CommandsDialog_Dtor_16(int ecx, int edx);
// 0x004c89f2
int __fastcall EH_Unwind_CommandsDialog_Dtor_17(int ecx, int edx);
// 0x004c89fa
int __fastcall EH_Unwind_CommandsDialog_Dtor_18(int ecx, int edx);
// 0x004c8a08
int __fastcall EH_Unwind_CommandsDialog_Dtor_19(int ecx, int edx);
// 0x004c8a16
int __fastcall EH_Unwind_CommandsDialog_Dtor_20(int ecx, int edx);
// 0x004c8a1e
int __fastcall EH_Unwind_CommandsDialog_Dtor_21(int ecx, int edx);
// 0x004c8a2c
int __fastcall EH_Unwind_CommandsDialog_Dtor_22(int ecx, int edx);
// 0x004c8a3a
int __fastcall EH_Unwind_CommandsDialog_Dtor_23(int ecx, int edx);
// 0x004c8a42
int __fastcall EH_Unwind_CommandsDialog_Dtor_24(int ecx, int edx);
// 0x004c8a50
int __fastcall EH_Unwind_CommandsDialog_Dtor_25(int ecx, int edx);
// 0x004c8a5e
int __fastcall EH_Unwind_CommandsDialog_Dtor_26(int ecx, int edx);
// 0x004c8a66
int __fastcall EH_Unwind_CommandsDialog_Dtor_27(int ecx, int edx);
// 0x004c8a74
int __fastcall EH_Unwind_CommandsDialog_Dtor_28(int ecx, int edx);
// 0x004c8a90
int __fastcall EH_Unwind_CommandsScreen_Dtor_0(int ecx, int edx);
// 0x004c8ab0
int __fastcall EH_Unwind_CommandsScreen_Create_0(int ecx, int edx);
// 0x004c8ad0
int __fastcall EH_Unwind_CommandsDialog_AddString_0(int ecx, int edx);
// 0x004c8af0
int __fastcall EH_Unwind_OptionListScreen_Dtor_NoSEH_0(int ecx, int edx);
// 0x004c8af8
int __fastcall EH_Unwind_OptionListScreen_Dtor_NoSEH_1(int ecx, int edx);
// 0x004c8b06
int __fastcall EH_Unwind_OptionListScreen_Dtor_NoSEH_2(int ecx, int edx);
// 0x004c8b28
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_1(int ecx, int edx);
// 0x004c8b36
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_2(int ecx, int edx);
// 0x004c8b44
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_3(int ecx, int edx);
// 0x004c8b52
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_4(int ecx, int edx);
// 0x004c8b60
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_5(int ecx, int edx);
// 0x004c8b6e
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_6(int ecx, int edx);
// 0x004c8b7c
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_7(int ecx, int edx);
// 0x004c8b8a
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_8(int ecx, int edx);
// 0x004c8b98
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_9(int ecx, int edx);
// 0x004c8ba6
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_10(int ecx, int edx);
// 0x004c8bb4
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_11(int ecx, int edx);
// 0x004c8bc2
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_12(int ecx, int edx);
// 0x004c8bd0
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_13(int ecx, int edx);
// 0x004c8bf8
int __fastcall EH_Unwind_OptionsPanel_Dtor_1(int ecx, int edx);
// 0x004c8c06
int __fastcall EH_Unwind_OptionsPanel_Dtor_2(int ecx, int edx);
// 0x004c8c14
int __fastcall EH_Unwind_OptionsPanel_Dtor_3(int ecx, int edx);
// 0x004c8c22
int __fastcall EH_Unwind_OptionsPanel_Dtor_4(int ecx, int edx);
// 0x004c8c30
int __fastcall EH_Unwind_OptionsPanel_Dtor_5(int ecx, int edx);
// 0x004c8c3e
int __fastcall EH_Unwind_OptionsPanel_Dtor_6(int ecx, int edx);
// 0x004c8c4c
int __fastcall EH_Unwind_OptionsPanel_Dtor_7(int ecx, int edx);
// 0x004c8c5a
int __fastcall EH_Unwind_OptionsPanel_Dtor_8(int ecx, int edx);
// 0x004c8c68
int __fastcall EH_Unwind_OptionsPanel_Dtor_9(int ecx, int edx);
// 0x004c8c76
int __fastcall EH_Unwind_OptionsPanel_Dtor_10(int ecx, int edx);
// 0x004c8c84
int __fastcall EH_Unwind_OptionsPanel_Dtor_11(int ecx, int edx);
// 0x004c8c92
int __fastcall EH_Unwind_OptionsPanel_Dtor_12(int ecx, int edx);
// 0x004c8cb0
int __fastcall EH_Unwind_ScreenHolder_004ce230_Dtor_0(int ecx, int edx);
// 0x004c8cd0
int __fastcall EH_Unwind_OptionsScreen_Create_0(int ecx, int edx);
// 0x004c8cf0
int __fastcall EH_Unwind_StaticObj_Dtor_ImageWidget_0(int ecx, int edx);
// 0x004c8d10
int __fastcall EH_Unwind_StaticObj_Dtor_4Images_0(int ecx, int edx);
// 0x004c8d18
int __fastcall EH_Unwind_StaticObj_Dtor_4Images_1(int ecx, int edx);
// 0x004c8d26
int __fastcall EH_Unwind_StaticObj_Dtor_4Images_2(int ecx, int edx);
// 0x004c8d34
int __fastcall EH_Unwind_StaticObj_Dtor_4Images_3(int ecx, int edx);
// 0x004c8d50
int __fastcall EH_Unwind_StaticObj_Dtor_1Image_0(int ecx, int edx);
// 0x004c8d70
int __fastcall EH_Unwind_HudPanel_Dtor_0(int ecx, int edx);
// 0x004c8d78
int __fastcall EH_Unwind_HudPanel_Dtor_1(int ecx, int edx);
// 0x004c8d83
int __fastcall EH_Unwind_HudPanel_Dtor_2(int ecx, int edx);
// 0x004c8d91
int __fastcall EH_Unwind_HudPanel_Dtor_3(int ecx, int edx);
// 0x004c8d9f
int __fastcall EH_Unwind_HudPanel_Dtor_4(int ecx, int edx);
// 0x004c8dad
int __fastcall EH_Unwind_HudPanel_Dtor_5(int ecx, int edx);
// 0x004c8dbb
int __fastcall EH_Unwind_HudPanel_Dtor_6(int ecx, int edx);
// 0x004c8dc9
int __fastcall EH_Unwind_HudPanel_Dtor_7(int ecx, int edx);
// 0x004c8dd7
int __fastcall EH_Unwind_HudPanel_Dtor_8(int ecx, int edx);
// 0x004c8de2
int __fastcall EH_Unwind_HudPanel_Dtor_9(int ecx, int edx);
// 0x004c8ded
int __fastcall EH_Unwind_HudPanel_Dtor_10(int ecx, int edx);
// 0x004c8dfb
int __fastcall EH_Unwind_HudPanel_Dtor_11(int ecx, int edx);
// 0x004c8e09
int __fastcall EH_Unwind_HudPanel_Dtor_12(int ecx, int edx);
// 0x004c8e17
int __fastcall EH_Unwind_HudPanel_Dtor_13(int ecx, int edx);
// 0x004c8e30
int __fastcall EH_Unwind_Screen_Dtor_ImageLabelImage_0(int ecx, int edx);
// 0x004c8e38
int __fastcall EH_Unwind_Screen_Dtor_ImageLabelImage_1(int ecx, int edx);
// 0x004c8e50
int __fastcall EH_Unwind_Screen_Dtor_3Images_0(int ecx, int edx);
// 0x004c8e70
int __fastcall EH_Unwind_MenuImageLabelImage_Dtor_0(int ecx, int edx);
// 0x004c8e7b
int __fastcall EH_Unwind_MenuImageLabelImage_Dtor_1(int ecx, int edx);
// 0x004c8e89
int __fastcall EH_Unwind_MenuImageLabelImage_Dtor_2(int ecx, int edx);
// 0x004c8e97
int __fastcall EH_Unwind_MenuImageLabelImage_Dtor_3(int ecx, int edx);
// 0x004c8eb0
int __fastcall EH_Unwind_MenuImagePair_Dtor_0(int ecx, int edx);
// 0x004c8ed0
int __fastcall EH_Unwind_Screen_Dtor_2Images_0(int ecx, int edx);
// 0x004c8ed8
int __fastcall EH_Unwind_Screen_Dtor_2Images_1(int ecx, int edx);
// 0x004c8ef0
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_0(int ecx, int edx);
// 0x004c8ef8
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_1(int ecx, int edx);
// 0x004c8f03
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_2(int ecx, int edx);
// 0x004c8f11
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_3(int ecx, int edx);
// 0x004c8f1f
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_4(int ecx, int edx);
// 0x004c8f2a
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_5(int ecx, int edx);
// 0x004c8f38
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_6(int ecx, int edx);
// 0x004c8f46
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_7(int ecx, int edx);
// 0x004c8f54
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_8(int ecx, int edx);
// 0x004c8f62
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_9(int ecx, int edx);
// 0x004c8f6d
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_10(int ecx, int edx);
// 0x004c8f7b
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_11(int ecx, int edx);
// 0x004c8f89
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_12(int ecx, int edx);
// 0x004c8f97
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_13(int ecx, int edx);
// 0x004c8fa5
int __fastcall EH_Unwind_HudPanel_Ctor_4ce278_14(int ecx, int edx);
// 0x004c8fc0
int __fastcall EH_Unwind_MenuImageLabelImage_Ctor_4ce410_0(int ecx, int edx);
// 0x004c8fc8
int __fastcall EH_Unwind_MenuImageLabelImage_Ctor_4ce410_1(int ecx, int edx);
// 0x004c8fe0
int __fastcall EH_Unwind_MenuImagePair_Ctor_4ce500_0(int ecx, int edx);
// 0x004c8fe8
int __fastcall EH_Unwind_MenuImagePair_Ctor_4ce500_1(int ecx, int edx);
// 0x004c9000
int __fastcall EH_Unwind_HudLabel_Ctor_4ce578_0(int ecx, int edx);
// 0x004c9008
int __fastcall EH_Unwind_HudLabel_Ctor_4ce578_1(int ecx, int edx);
// 0x004c9020
int __fastcall EH_Unwind_NetScoreboard_Ctor_0(int ecx, int edx);
// 0x004c9028
int __fastcall EH_Unwind_NetScoreboard_Ctor_1(int ecx, int edx);
// 0x004c9033
int __fastcall EH_Unwind_NetScoreboard_Ctor_2(int ecx, int edx);
// 0x004c903e
int __fastcall EH_Unwind_NetScoreboard_Ctor_3(int ecx, int edx);
// 0x004c9046
int __fastcall EH_Unwind_NetScoreboard_Ctor_4(int ecx, int edx);
// 0x004c9051
int __fastcall EH_Unwind_NetScoreboard_Ctor_5(int ecx, int edx);
// 0x004c9070
int __fastcall EH_Unwind_NetScoreboard_Dtor_0(int ecx, int edx);
// 0x004c9078
int __fastcall EH_Unwind_NetScoreboard_Dtor_1(int ecx, int edx);
// 0x004c9090
int __fastcall EH_Unwind_HudLabel_Ctor_4ce6b0_0(int ecx, int edx);
// 0x004c9098
int __fastcall EH_Unwind_HudLabel_Ctor_4ce6b0_1(int ecx, int edx);
// 0x004c90b0
int __fastcall EH_Unwind_HudLabel_Ctor_4ce7d8_0(int ecx, int edx);
// 0x004c90b8
int __fastcall EH_Unwind_HudLabel_Ctor_4ce7d8_1(int ecx, int edx);
// 0x004c90d0
int __fastcall EH_Unwind_MenuImageTriple_Ctor_4ce870_0(int ecx, int edx);
// 0x004c90d8
int __fastcall EH_Unwind_MenuImageTriple_Ctor_4ce870_1(int ecx, int edx);
// 0x004c9100
int __fastcall EH_Unwind_Hud_InitWidgets_0(int ecx, int edx);
// 0x004c910b
int __fastcall EH_Unwind_Hud_InitWidgets_1(int ecx, int edx);
// 0x004c9116
int __fastcall EH_Unwind_Hud_InitWidgets_2(int ecx, int edx);
// 0x004c911e
int __fastcall EH_Unwind_Hud_InitWidgets_3(int ecx, int edx);
// 0x004c9137
int __fastcall EH_Unwind_Hud_InitWidgets_4(int ecx, int edx);
// 0x004c9142
int __fastcall EH_Unwind_Hud_InitWidgets_5(int ecx, int edx);
// 0x004c914d
int __fastcall EH_Unwind_Hud_InitWidgets_6(int ecx, int edx);
// 0x004c9155
int __fastcall EH_Unwind_Hud_InitWidgets_7(int ecx, int edx);
// 0x004c9163
int __fastcall EH_Unwind_Hud_InitWidgets_8(int ecx, int edx);
// 0x004c916e
int __fastcall EH_Unwind_Hud_InitWidgets_9(int ecx, int edx);
// 0x004c9179
int __fastcall EH_Unwind_Hud_InitWidgets_10(int ecx, int edx);
// 0x004c9184
int __fastcall EH_Unwind_Hud_InitWidgets_11(int ecx, int edx);
// 0x004c918c
int __fastcall EH_Unwind_Hud_InitWidgets_12(int ecx, int edx);
// 0x004c9197
int __fastcall EH_Unwind_Hud_InitWidgets_13(int ecx, int edx);
// 0x004c91a2
int __fastcall EH_Unwind_Hud_InitWidgets_14(int ecx, int edx);
// 0x004c91aa
int __fastcall EH_Unwind_Hud_InitWidgets_15(int ecx, int edx);
// 0x004c91b5
int __fastcall EH_Unwind_Hud_InitWidgets_16(int ecx, int edx);
// 0x004c91bd
int __fastcall EH_Unwind_Hud_InitWidgets_17(int ecx, int edx);
// 0x004c91c8
int __fastcall EH_Unwind_Hud_InitWidgets_18(int ecx, int edx);
// 0x004c91d0
int __fastcall EH_Unwind_Hud_InitWidgets_19(int ecx, int edx);
// 0x004c91db
int __fastcall EH_Unwind_Hud_InitWidgets_20(int ecx, int edx);
// 0x004c91f0
int __fastcall EH_Unwind_Screen_Dtor_OwnsObject34_0(int ecx, int edx);
// 0x004c9210
int __fastcall EH_Unwind_HudLabel_Ctor_0(int ecx, int edx);
// 0x004c9328
int __fastcall EH_Unwind_MainMenu_Ctor_1(int ecx, int edx);
// 0x004c9336
int __fastcall EH_Unwind_MainMenu_Ctor_2(int ecx, int edx);
// 0x004c9344
int __fastcall EH_Unwind_MainMenu_Ctor_3(int ecx, int edx);
// 0x004c9352
int __fastcall EH_Unwind_MainMenu_Ctor_4(int ecx, int edx);
// 0x004c9360
int __fastcall EH_Unwind_MainMenu_Ctor_5(int ecx, int edx);
// 0x004c936e
int __fastcall EH_Unwind_MainMenu_Ctor_6(int ecx, int edx);
// 0x004c937c
int __fastcall EH_Unwind_MainMenu_Ctor_7(int ecx, int edx);
// 0x004c938a
int __fastcall EH_Unwind_MainMenu_Ctor_8(int ecx, int edx);
// 0x004c93b8
int __fastcall EH_Unwind_MainMenu_Dtor_1(int ecx, int edx);
// 0x004c93c6
int __fastcall EH_Unwind_MainMenu_Dtor_2(int ecx, int edx);
// 0x004c93d4
int __fastcall EH_Unwind_MainMenu_Dtor_3(int ecx, int edx);
// 0x004c93e2
int __fastcall EH_Unwind_MainMenu_Dtor_4(int ecx, int edx);
// 0x004c93f0
int __fastcall EH_Unwind_MainMenu_Dtor_5(int ecx, int edx);
// 0x004c93fe
int __fastcall EH_Unwind_MainMenu_Dtor_6(int ecx, int edx);
// 0x004c940c
int __fastcall EH_Unwind_MainMenu_Dtor_7(int ecx, int edx);
// 0x004c9430
int __fastcall EH_Unwind_MainMenuScreen_Dtor_0(int ecx, int edx);
// 0x004c9450
int __fastcall EH_Unwind_MainMenuScreen_Create_0(int ecx, int edx);
// 0x004c9458
int __fastcall EH_Unwind_MainMenuScreen_Create_1(int ecx, int edx);
// 0x004c9478
int __fastcall EH_Unwind_ConfirmQuitScreen_Ctor_1(int ecx, int edx);
// 0x004c9486
int __fastcall EH_Unwind_ConfirmQuitScreen_Ctor_2(int ecx, int edx);
// 0x004c94a8
int __fastcall EH_Unwind_ScreenHost_Dtor_1(int ecx, int edx);
// 0x004c94c0
int __fastcall EH_Unwind_GlobalObj_Dtor_0(int ecx, int edx);
// 0x004c94e0
int __fastcall EH_Unwind_ScreenHost_OpenConfirmQuit_0(int ecx, int edx);
// 0x004c95f0
int __fastcall EH_Unwind_MPExitScreen_Create_0(int ecx, int edx);
// 0x004c9603
int __fastcall EH_Unwind_MPExitScreen_Create_2(int ecx, int edx);
// 0x004c9648
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_1(int ecx, int edx);
// 0x004c9656
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_2(int ecx, int edx);
// 0x004c9664
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_3(int ecx, int edx);
// 0x004c9672
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_4(int ecx, int edx);
// 0x004c9680
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_5(int ecx, int edx);
// 0x004c968e
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_6(int ecx, int edx);
// 0x004c969c
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_7(int ecx, int edx);
// 0x004c96aa
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_8(int ecx, int edx);
// 0x004c96b8
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_9(int ecx, int edx);
// 0x004c96c6
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_10(int ecx, int edx);
// 0x004c96d4
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_11(int ecx, int edx);
// 0x004c96e2
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_12(int ecx, int edx);
// 0x004c96f0
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_13(int ecx, int edx);
// 0x004c96fe
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_14(int ecx, int edx);
// 0x004c970c
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_15(int ecx, int edx);
// 0x004c971a
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_16(int ecx, int edx);
// 0x004c9728
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_17(int ecx, int edx);
// 0x004c9736
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_18(int ecx, int edx);
// 0x004c9744
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_19(int ecx, int edx);
// 0x004c9760
int __fastcall EH_Unwind_MenuEditField_Ctor_0(int ecx, int edx);
// 0x004c9780
int __fastcall EH_Unwind_MenuIntEditField_Ctor_0(int ecx, int edx);
// 0x004c97a8
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_1(int ecx, int edx);
// 0x004c97b6
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_2(int ecx, int edx);
// 0x004c97c4
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_3(int ecx, int edx);
// 0x004c97d2
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_4(int ecx, int edx);
// 0x004c97e0
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_5(int ecx, int edx);
// 0x004c97ee
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_6(int ecx, int edx);
// 0x004c97fc
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_7(int ecx, int edx);
// 0x004c980a
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_8(int ecx, int edx);
// 0x004c9818
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_9(int ecx, int edx);
// 0x004c9826
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_10(int ecx, int edx);
// 0x004c9834
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_11(int ecx, int edx);
// 0x004c9842
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_12(int ecx, int edx);
// 0x004c9850
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_13(int ecx, int edx);
// 0x004c985e
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_14(int ecx, int edx);
// 0x004c986c
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_15(int ecx, int edx);
// 0x004c987a
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_16(int ecx, int edx);
// 0x004c9888
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_17(int ecx, int edx);
// 0x004c9896
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_18(int ecx, int edx);
// 0x004c98b0
int __fastcall EH_Unwind_MPNewGameScreen_Dtor_0(int ecx, int edx);
// 0x004c98d0
int __fastcall EH_Unwind_MPNewGameScreen_Create_0(int ecx, int edx);
// 0x004c98f0
int __fastcall EH_Unwind_NameDlg_ctor_0(int ecx, int edx);
// 0x004c98f8
int __fastcall EH_Unwind_NameDlg_ctor_1(int ecx, int edx);
// 0x004c9903
int __fastcall EH_Unwind_NameDlg_ctor_2(int ecx, int edx);
// 0x004c9911
int __fastcall EH_Unwind_NameDlg_ctor_3(int ecx, int edx);
// 0x004c991f
int __fastcall EH_Unwind_NameDlg_ctor_4(int ecx, int edx);
// 0x004c992d
int __fastcall EH_Unwind_NameDlg_ctor_5(int ecx, int edx);
// 0x004c993b
int __fastcall EH_Unwind_NameDlg_ctor_6(int ecx, int edx);
// 0x004c9960
int __fastcall EH_Unwind_NameDlg_dtor_0(int ecx, int edx);
// 0x004c9968
int __fastcall EH_Unwind_NameDlg_dtor_1(int ecx, int edx);
// 0x004c9973
int __fastcall EH_Unwind_NameDlg_dtor_2(int ecx, int edx);
// 0x004c9981
int __fastcall EH_Unwind_NameDlg_dtor_3(int ecx, int edx);
// 0x004c998f
int __fastcall EH_Unwind_NameDlg_dtor_4(int ecx, int edx);
// 0x004c999d
int __fastcall EH_Unwind_NameDlg_dtor_5(int ecx, int edx);
// 0x004c99c0
int __fastcall EH_Unwind_NameDlg_RefreshSessionList_0(int ecx, int edx);
// 0x004c99e8
int __fastcall EH_Unwind_NetExitPanel_BuildWidgets_1(int ecx, int edx);
// 0x004c99f6
int __fastcall EH_Unwind_NetExitPanel_BuildWidgets_2(int ecx, int edx);
// 0x004c9a18
int __fastcall EH_Unwind_Panel_0041beb0_Dtor_1(int ecx, int edx);
// 0x004c9a30
int __fastcall EH_Unwind_NetExitDialog_InitOnGameLoad_0(int ecx, int edx);
// 0x004c9a50
int __fastcall EH_Unwind_AiDebugPanel_SetParamLabels_0(int ecx, int edx);
// 0x004c9a58
int __fastcall EH_Unwind_AiDebugPanel_SetParamLabels_1(int ecx, int edx);
// 0x004c9a78
int __fastcall EH_Unwind_NewGamePanel_Ctor_1(int ecx, int edx);
// 0x004c9a86
int __fastcall EH_Unwind_NewGamePanel_Ctor_2(int ecx, int edx);
// 0x004c9a94
int __fastcall EH_Unwind_NewGamePanel_Ctor_3(int ecx, int edx);
// 0x004c9aa2
int __fastcall EH_Unwind_NewGamePanel_Ctor_4(int ecx, int edx);
// 0x004c9ac8
int __fastcall EH_Unwind_NewGamePanel_Dtor_1(int ecx, int edx);
// 0x004c9ad6
int __fastcall EH_Unwind_NewGamePanel_Dtor_2(int ecx, int edx);
// 0x004c9ae4
int __fastcall EH_Unwind_NewGamePanel_Dtor_3(int ecx, int edx);
// 0x004c9b00
int __fastcall EH_Unwind_NewGameHost_Open_0(int ecx, int edx);
// 0x004c9b20
int __fastcall EH_Unwind_ScreenHolder32c8_Dtor_0(int ecx, int edx);
// 0x004c9b40
int __fastcall EH_Unwind_NetSetupDialog_Ctor_0(int ecx, int edx);
// 0x004c9b48
int __fastcall EH_Unwind_NetSetupDialog_Ctor_1(int ecx, int edx);
// 0x004c9b53
int __fastcall EH_Unwind_NetSetupDialog_Ctor_2(int ecx, int edx);
// 0x004c9b61
int __fastcall EH_Unwind_NetSetupDialog_Ctor_3(int ecx, int edx);
// 0x004c9b6f
int __fastcall EH_Unwind_NetSetupDialog_Ctor_4(int ecx, int edx);
// 0x004c9b7d
int __fastcall EH_Unwind_NetSetupDialog_Ctor_5(int ecx, int edx);
// 0x004c9ba0
int __fastcall EH_Unwind_NetSetupDialog_Dtor_0(int ecx, int edx);
// 0x004c9ba8
int __fastcall EH_Unwind_NetSetupDialog_Dtor_1(int ecx, int edx);
// 0x004c9bb3
int __fastcall EH_Unwind_NetSetupDialog_Dtor_2(int ecx, int edx);
// 0x004c9bc1
int __fastcall EH_Unwind_NetSetupDialog_Dtor_3(int ecx, int edx);
// 0x004c9bcf
int __fastcall EH_Unwind_NetSetupDialog_Dtor_4(int ecx, int edx);
// 0x004ca120
int __fastcall EH_Unwind_MenuWrap_Dtor_0(int ecx, int edx);
// 0x004ca168
int __fastcall EH_Unwind_SaveGameDialog_Ctor_1(int ecx, int edx);
// 0x004ca176
int __fastcall EH_Unwind_SaveGameDialog_Ctor_2(int ecx, int edx);
// 0x004ca184
int __fastcall EH_Unwind_SaveGameDialog_Ctor_3(int ecx, int edx);
// 0x004ca192
int __fastcall EH_Unwind_SaveGameDialog_Ctor_4(int ecx, int edx);
// 0x004ca1a0
int __fastcall EH_Unwind_SaveGameDialog_Ctor_5(int ecx, int edx);
// 0x004ca1a8
int __fastcall EH_Unwind_SaveGameDialog_Ctor_6(int ecx, int edx);
// 0x004ca1be
int __fastcall EH_Unwind_SaveGameDialog_Ctor_8(int ecx, int edx);
// 0x004ca1e8
int __fastcall EH_Unwind_SaveLoadDialogBase_Dtor_1(int ecx, int edx);
// 0x004ca1f6
int __fastcall EH_Unwind_SaveLoadDialogBase_Dtor_2(int ecx, int edx);
// 0x004ca204
int __fastcall EH_Unwind_SaveLoadDialogBase_Dtor_3(int ecx, int edx);
// 0x004ca212
int __fastcall EH_Unwind_SaveLoadDialogBase_Dtor_4(int ecx, int edx);
// 0x004ca220
int __fastcall EH_Unwind_SaveLoadDialogBase_Dtor_5(int ecx, int edx);
// 0x004ca250
int __fastcall EH_Unwind_SaveGameDialog_Dtor_2(int ecx, int edx);
// 0x004ca25e
int __fastcall EH_Unwind_SaveGameDialog_Dtor_3(int ecx, int edx);
// 0x004ca26c
int __fastcall EH_Unwind_SaveGameDialog_Dtor_4(int ecx, int edx);
// 0x004ca27a
int __fastcall EH_Unwind_SaveGameDialog_Dtor_5(int ecx, int edx);
// 0x004ca288
int __fastcall EH_Unwind_SaveGameDialog_Dtor_6(int ecx, int edx);
// 0x004ca2a8
int __fastcall EH_Unwind_LoadGameDialog_Ctor_1(int ecx, int edx);
// 0x004ca2b6
int __fastcall EH_Unwind_LoadGameDialog_Ctor_2(int ecx, int edx);
// 0x004ca2c4
int __fastcall EH_Unwind_LoadGameDialog_Ctor_3(int ecx, int edx);
// 0x004ca2d2
int __fastcall EH_Unwind_LoadGameDialog_Ctor_4(int ecx, int edx);
// 0x004ca2e0
int __fastcall EH_Unwind_LoadGameDialog_Ctor_5(int ecx, int edx);
// 0x004ca2e8
int __fastcall EH_Unwind_LoadGameDialog_Ctor_6(int ecx, int edx);
// 0x004ca2fe
int __fastcall EH_Unwind_LoadGameDialog_Ctor_8(int ecx, int edx);
// 0x004ca330
int __fastcall EH_Unwind_LoadGameDialog_Dtor_2(int ecx, int edx);
// 0x004ca33e
int __fastcall EH_Unwind_LoadGameDialog_Dtor_3(int ecx, int edx);
// 0x004ca34c
int __fastcall EH_Unwind_LoadGameDialog_Dtor_4(int ecx, int edx);
// 0x004ca35a
int __fastcall EH_Unwind_LoadGameDialog_Dtor_5(int ecx, int edx);
// 0x004ca368
int __fastcall EH_Unwind_LoadGameDialog_Dtor_6(int ecx, int edx);
// 0x004ca380
int __fastcall EH_Unwind_SaveLoadScreen_Dtor_0(int ecx, int edx);
// 0x004ca3a0
int __fastcall EH_Unwind_SaveLoadScreen_Create_0(int ecx, int edx);
// 0x004ca3a8
int __fastcall EH_Unwind_SaveLoadScreen_Create_1(int ecx, int edx);
// 0x004ca3b3
int __fastcall EH_Unwind_SaveLoadScreen_Create_2(int ecx, int edx);
// 0x004cb020
int __fastcall EH_Unwind_InputContext_Push_0(int ecx, int edx);
// 0x004c82e8
int __fastcall EH_Handler_LoadingRing_Ctor(int ecx, int edx);

// 0x004c8308
int __fastcall EH_Handler_ListWidget_Dtor(int ecx, int edx);

// 0x004c84b8
int __fastcall EH_Handler_ScreenHolder_Dtor(int ecx, int edx);

// 0x004c84db
int __fastcall EH_Handler_CheatScreen_Open(int ecx, int edx);

// 0x004c84f8
int __fastcall EH_Handler_CheatScreen_CloseAndApply(int ecx, int edx);

// 0x004c8608
int __fastcall EH_Handler_ControlsHost_Dtor(int ecx, int edx);

// 0x004c862b
int __fastcall EH_Handler_ControlsHost_Open(int ecx, int edx);

// 0x004c8688
int __fastcall EH_Handler_TextPanel_Dtor(int ecx, int edx);

// 0x004c8708
int __fastcall EH_Handler_CreditsScreen_Dtor(int ecx, int edx);

// 0x004c872b
int __fastcall EH_Handler_CreditsScreen_Create(int ecx, int edx);

// 0x004c8844
int __fastcall EH_Handler_OptionListScreen_Dtor_a940(int ecx, int edx);

// 0x004c8874
int __fastcall EH_Handler_OptionListScreen_Dtor_aa30(int ecx, int edx);

// 0x004c88a4
int __fastcall EH_Handler_OptionListScreen_Dtor_ab20(int ecx, int edx);

// 0x004c88d4
int __fastcall EH_Handler_OptionListScreen_Dtor_ac10(int ecx, int edx);

// 0x004c8904
int __fastcall EH_Handler_OptionListScreen_Dtor_ad00(int ecx, int edx);

// 0x004c8a98
int __fastcall EH_Handler_CommandsScreen_Dtor(int ecx, int edx);

// 0x004c8abb
int __fastcall EH_Handler_CommandsScreen_Create(int ecx, int edx);

// 0x004c8adb
int __fastcall EH_Handler_CommandsDialog_AddString(int ecx, int edx);

// 0x004c8b14
int __fastcall EH_Handler_OptionListScreen_Dtor_NoSEH(int ecx, int edx);

// 0x004c8cb8
int __fastcall EH_Handler_ScreenHolder_004ce230_Dtor(int ecx, int edx);

// 0x004c8cdb
int __fastcall EH_Handler_OptionsScreen_Create(int ecx, int edx);

// 0x004c8cf8
int __fastcall EH_Handler_StaticObj_Dtor_ImageWidget(int ecx, int edx);

// 0x004c8d3c
int __fastcall EH_Handler_StaticObj_Dtor_4Images(int ecx, int edx);

// 0x004c8d58
int __fastcall EH_Handler_StaticObj_Dtor_1Image(int ecx, int edx);

// 0x004c8e1f
int __fastcall EH_Handler_HudPanel_Dtor(int ecx, int edx);

// 0x004c8e46
int __fastcall EH_Handler_Screen_Dtor_ImageLabelImage(int ecx, int edx);

// 0x004c8e58
int __fastcall EH_Handler_Screen_Dtor_3Images(int ecx, int edx);

// 0x004c8ea5
int __fastcall EH_Handler_MenuImageLabelImage_Dtor(int ecx, int edx);

// 0x004c8ebb
int __fastcall EH_Handler_MenuImagePair_Dtor(int ecx, int edx);

// 0x004c8ee3
int __fastcall EH_Handler_Screen_Dtor_2Images(int ecx, int edx);

// 0x004c8fb3
int __fastcall EH_Handler_HudPanel_Ctor_4ce278(int ecx, int edx);

// 0x004c8fd6
int __fastcall EH_Handler_MenuImageLabelImage_Ctor_4ce410(int ecx, int edx);

// 0x004c8ff3
int __fastcall EH_Handler_MenuImagePair_Ctor_4ce500(int ecx, int edx);

// 0x004c9010
int __fastcall EH_Handler_HudLabel_Ctor_4ce578(int ecx, int edx);

// 0x004c9059
int __fastcall EH_Handler_NetScoreboard_Ctor(int ecx, int edx);

// 0x004c9083
int __fastcall EH_Handler_NetScoreboard_Dtor(int ecx, int edx);

// 0x004c90a0
int __fastcall EH_Handler_HudLabel_Ctor_4ce6b0(int ecx, int edx);

// 0x004c90c0
int __fastcall EH_Handler_HudLabel_Ctor_4ce7d8(int ecx, int edx);

// 0x004c90f1
int __fastcall EH_Handler_MenuImageTriple_Ctor_4ce870(int ecx, int edx);

// 0x004c91e6
int __fastcall EH_Handler_Hud_InitWidgets(int ecx, int edx);

// 0x004c91f8
int __fastcall EH_Handler_Screen_Dtor_OwnsObject34(int ecx, int edx);

// 0x004c9218
int __fastcall EH_Handler_HudLabel_Ctor(int ecx, int edx);

// 0x004c9438
int __fastcall EH_Handler_MainMenuScreen_Dtor(int ecx, int edx);

// 0x004c9463
int __fastcall EH_Handler_MainMenuScreen_Create(int ecx, int edx);

// 0x004c94c8
int __fastcall EH_Handler_GlobalObj_Dtor(int ecx, int edx);

// 0x004c94eb
int __fastcall EH_Handler_ScreenHost_OpenConfirmQuit(int ecx, int edx);

// 0x004c9768
int __fastcall EH_Handler_MenuEditField_Ctor(int ecx, int edx);

// 0x004c9788
int __fastcall EH_Handler_MenuIntEditField_Ctor(int ecx, int edx);

// 0x004c98b8
int __fastcall EH_Handler_MPNewGameScreen_Dtor(int ecx, int edx);

// 0x004c98db
int __fastcall EH_Handler_MPNewGameScreen_Create(int ecx, int edx);

// 0x004c9949
int __fastcall EH_Handler_NameDlg_ctor(int ecx, int edx);

// 0x004c99ab
int __fastcall EH_Handler_NameDlg_dtor(int ecx, int edx);

// 0x004c99cb
int __fastcall EH_Handler_NameDlg_RefreshSessionList(int ecx, int edx);

// 0x004c9a3b
int __fastcall EH_Handler_NetExitDialog_InitOnGameLoad(int ecx, int edx);

// 0x004c9a60
int __fastcall EH_Handler_AiDebugPanel_SetParamLabels(int ecx, int edx);

// 0x004c9b0b
int __fastcall EH_Handler_NewGameHost_Open(int ecx, int edx);

// 0x004c9b28
int __fastcall EH_Handler_ScreenHolder32c8_Dtor(int ecx, int edx);

// 0x004c9b8b
int __fastcall EH_Handler_NetSetupDialog_Ctor(int ecx, int edx);

// 0x004c9bdd
int __fastcall EH_Handler_NetSetupDialog_Dtor(int ecx, int edx);

// 0x004ca128
int __fastcall EH_Handler_MenuWrap_Dtor(int ecx, int edx);

// 0x004ca388
int __fastcall EH_Handler_SaveLoadScreen_Dtor(int ecx, int edx);

// 0x004ca3be
int __fastcall EH_Handler_SaveLoadScreen_Create(int ecx, int edx);

// 0x004cb02b
int __fastcall EH_Handler_InputContext_Push(int ecx, int edx);

// 0x0041cc10
int __fastcall Thunk_0041cc20_0041cc10(int ecx, int edx);
// 0x0041cc40
int __fastcall Thunk_0041cc50_0041cc40(int ecx, int edx);
// 0x0041cc70
int __fastcall Thunk_0041cc80_0041cc70(int ecx, int edx);
// 0x00431bf0
int __fastcall Thunk_00431c00_00431bf0(int ecx, int edx);
// 0x00431c20
int __fastcall Thunk_00431c30_00431c20(int ecx, int edx);
// 0x0041cc20
int __fastcall StaticInit_ZeroList_004f3318(int ecx, int edx);
// 0x0041cc50
int __fastcall StaticInit_ZeroList_004f32f8(int ecx, int edx);
// 0x0041cc80
int __fastcall StaticInit_ZeroList_004f3308(int ecx, int edx);
// 0x00431c00
int __fastcall StaticInit_ZeroList_004f3f78(int ecx, int edx);
// 0x00431c30
int __fastcall StaticInit_ZeroList_004f3f10(int ecx, int edx);
// 0x00435a70
int __fastcall LoadGameScreen_Load(int ecx, int edx);
// 0x00403930
int __fastcall BriefingScreen_Ctor(int ecx, int edx, int arg1);
// 0x00403d90
int __fastcall BriefingScreen_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00406af0
int __fastcall Cheat_ProcessEnteredCode(int ecx, int edx);
// 0x00406d20
int __fastcall CheatCodeScreen_Ctor(int ecx, int edx);
// 0x00406e10
int __fastcall CheatScreen_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00406e30
int __fastcall CheatScreen_Dtor(int ecx, int edx);
// 0x00406f60
int __fastcall CheatScreen_Open(int ecx, int edx);
// 0x00407010
int __fastcall CheatScreen_CloseAndApply(int ecx, int edx);
// 0x00408a30
int __fastcall ControlsDialog_Ctor(int ecx, int edx);
// 0x00408c40
int __fastcall ControlsDialog_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00408c70
int __fastcall ControlsDialog_Dtor(int ecx, int edx);
// 0x00408df0
int __fastcall ControlsHost_Open(int ecx, int edx);
// 0x00409040
int __fastcall CreditsPanel_Ctor(int ecx, int edx);
// 0x004091c0
int __fastcall CreditsPanel_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x004092a0
int __fastcall CreditsPanel_Dtor(int ecx, int edx);
// 0x00409a60
int __fastcall CreditsScreen_Create(int ecx, int edx);
// 0x0040a5b0
int __fastcall CommandsDialog_Ctor(int ecx, int edx);
// 0x0040a920
int __fastcall CommandsDialog_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x0040adf0
int __fastcall CommandsDialog_Dtor(int ecx, int edx);
// 0x0040bcf0
int __fastcall CommandsScreen_Create(int ecx, int edx);
// 0x0040c720
int __fastcall OptionsPanel_BuildWidgets(int ecx, int edx);
// 0x0040cf00
int __fastcall Panel_0040cf60_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0040cf60
int __fastcall OptionsPanel_Dtor(int ecx, int edx);
// 0x0040d150
int __fastcall OptionsScreen_Create(int ecx, int edx);
// 0x00414bc0
int __fastcall MainMenu_Ctor(int ecx, int edx, int arg1);
// 0x00415020
int __fastcall MainMenu_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00415040
int __fastcall MainMenu_Dtor(int ecx, int edx);
// 0x00415220
int __fastcall MainMenuScreen_Create(int ecx, int edx);
// 0x00415680
int __fastcall ConfirmQuitScreen_Ctor(int ecx, int edx);
// 0x00415790
int __fastcall GlobalObj_ScalarDeletingDtor_004157b0(int ecx, int edx, int arg1);
// 0x004157b0
int __fastcall ScreenHost_Dtor(int ecx, int edx);
// 0x004158f0
int __fastcall ScreenHost_OpenConfirmQuit(int ecx, int edx);
// 0x00419500
int __fastcall MPExitPanel_BuildWidgets(int ecx, int edx);
// 0x00419740
int __fastcall MPExitScreen_Create(int ecx, int edx);
// 0x004198d0
int __fastcall MPExitScreen_OnEnter_004198d0(int ecx, int edx);
// 0x00419aa0
int __fastcall MPNewGamePanel_BuildWidgets(int ecx, int edx, int arg1);
// 0x0041a3d0
int __fastcall MPNewGamePanel_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x0041a400
int __fastcall MPNewGamePanel_Dtor(int ecx, int edx);
// 0x0041ac50
int __fastcall MPNewGameScreen_Create(int ecx, int edx);
// 0x0041bd80
int __fastcall NetExitPanel_BuildWidgets(int ecx, int edx);
// 0x0041be90
int __fastcall Panel_0041beb0_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0041beb0
int __fastcall Panel_0041beb0_Dtor(int ecx, int edx);
// 0x0041c000
int __fastcall NetExitDialog_InitOnGameLoad(int ecx, int edx);
// 0x0041c290
int __fastcall NewGamePanel_Ctor(int ecx, int edx);
// 0x0041c3e0
int __fastcall NewGamePanel_ScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x0041c400
int __fastcall NewGamePanel_Dtor(int ecx, int edx);
// 0x0041c560
int __fastcall NewGameHost_Open(int ecx, int edx);
// 0x00434680
int __fastcall SaveGameDialog_Ctor(int ecx, int edx);
// 0x00434970
int __fastcall Thunk_00435240(int ecx, int edx);
// 0x00434980
int __fastcall SaveGameDialog_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x004349a0
int __fastcall SaveLoadDialogBase_Dtor(int ecx, int edx);
// 0x00434a80
int __fastcall SaveGameDialog_Dtor(int ecx, int edx);
// 0x00434b70
int __fastcall SaveLoadDialogBase_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00434b90
int __fastcall LoadGameDialog_Ctor(int ecx, int edx);
// 0x00434dd0
int __fastcall LoadGameDialog_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00434df0
int __fastcall LoadGameDialog_Dtor(int ecx, int edx);
// 0x00434fb0
int __fastcall SaveLoadDialog_DeleteSelected(int ecx, int edx, int arg1);
// 0x00435240
int __fastcall SaveGameScreen_Save(int ecx, int edx);
// 0x00435d20
int __fastcall SaveLoadScreen_Create(int ecx, int edx);
// 0x00438350
int __fastcall MessageBox_RunModal(int ecx, int edx, int arg1);
// 0x00463d50
int __fastcall AviPlayer_Open(int ecx, int edx, int arg1, int arg2);
// 0x00463dd0
int __fastcall AviPlayer_Close(int ecx, int edx);
// 0x004c8230
int __fastcall EH_Unwind_BriefingScreen_Ctor_0(int ecx, int edx);
// 0x004c8450
int __fastcall EH_Unwind_CheatCodeScreen_Ctor_0(int ecx, int edx);
// 0x004c8490
int __fastcall EH_Unwind_CheatScreen_Dtor_0(int ecx, int edx);
// 0x004c8510
int __fastcall EH_Unwind_ControlsDialog_Ctor_0(int ecx, int edx);
// 0x004c8590
int __fastcall EH_Unwind_ControlsDialog_Dtor_0(int ecx, int edx);
// 0x004c8640
int __fastcall EH_Unwind_CreditsPanel_Ctor_0(int ecx, int edx);
// 0x004c86a0
int __fastcall EH_Unwind_CreditsPanel_Dtor_0(int ecx, int edx);
// 0x004c8740
int __fastcall EH_Unwind_CommandsDialog_Ctor_0(int ecx, int edx);
// 0x004c8910
int __fastcall EH_Unwind_CommandsDialog_Dtor_0(int ecx, int edx);
// 0x004c8b20
int __fastcall EH_Unwind_OptionsPanel_BuildWidgets_0(int ecx, int edx);
// 0x004c8bf0
int __fastcall EH_Unwind_OptionsPanel_Dtor_0(int ecx, int edx);
// 0x004c9320
int __fastcall EH_Unwind_MainMenu_Ctor_0(int ecx, int edx);
// 0x004c93b0
int __fastcall EH_Unwind_MainMenu_Dtor_0(int ecx, int edx);
// 0x004c9470
int __fastcall EH_Unwind_ConfirmQuitScreen_Ctor_0(int ecx, int edx);
// 0x004c94a0
int __fastcall EH_Unwind_ScreenHost_Dtor_0(int ecx, int edx);
// 0x004c95fb
int __fastcall EH_Unwind_MPExitScreen_Create_1(int ecx, int edx);
// 0x004c9640
int __fastcall EH_Unwind_MPNewGamePanel_BuildWidgets_0(int ecx, int edx);
// 0x004c97a0
int __fastcall EH_Unwind_MPNewGamePanel_Dtor_0(int ecx, int edx);
// 0x004c99e0
int __fastcall EH_Unwind_NetExitPanel_BuildWidgets_0(int ecx, int edx);
// 0x004c9a10
int __fastcall EH_Unwind_Panel_0041beb0_Dtor_0(int ecx, int edx);
// 0x004c9a70
int __fastcall EH_Unwind_NewGamePanel_Ctor_0(int ecx, int edx);
// 0x004c9ac0
int __fastcall EH_Unwind_NewGamePanel_Dtor_0(int ecx, int edx);
// 0x004ca160
int __fastcall EH_Unwind_SaveGameDialog_Ctor_0(int ecx, int edx);
// 0x004ca1b6
int __fastcall EH_Unwind_SaveGameDialog_Ctor_7(int ecx, int edx);
// 0x004ca1e0
int __fastcall EH_Unwind_SaveLoadDialogBase_Dtor_0(int ecx, int edx);
// 0x004ca240
int __fastcall EH_Unwind_SaveGameDialog_Dtor_0(int ecx, int edx);
// 0x004ca248
int __fastcall EH_Unwind_SaveGameDialog_Dtor_1(int ecx, int edx);
// 0x004ca2a0
int __fastcall EH_Unwind_LoadGameDialog_Ctor_0(int ecx, int edx);
// 0x004ca2f6
int __fastcall EH_Unwind_LoadGameDialog_Ctor_7(int ecx, int edx);
// 0x004ca320
int __fastcall EH_Unwind_LoadGameDialog_Dtor_0(int ecx, int edx);
// 0x004ca328
int __fastcall EH_Unwind_LoadGameDialog_Dtor_1(int ecx, int edx);
// 0x004ca3f0
int __fastcall EH_Unwind_MessageBox_RunModal_0(int ecx, int edx);
// 0x004c82c8
int __fastcall EH_Handler_BriefingScreen_Ctor(int ecx, int edx);

// 0x004c847c
int __fastcall EH_Handler_CheatCodeScreen_Ctor(int ecx, int edx);

// 0x004c84a6
int __fastcall EH_Handler_CheatScreen_Dtor(int ecx, int edx);

// 0x004c857a
int __fastcall EH_Handler_ControlsDialog_Ctor(int ecx, int edx);

// 0x004c85ec
int __fastcall EH_Handler_ControlsDialog_Dtor(int ecx, int edx);

// 0x004c8672
int __fastcall EH_Handler_CreditsPanel_Ctor(int ecx, int edx);

// 0x004c86cc
int __fastcall EH_Handler_CreditsPanel_Dtor(int ecx, int edx);

// 0x004c880c
int __fastcall EH_Handler_CommandsDialog_Ctor(int ecx, int edx);

// 0x004c8a82
int __fastcall EH_Handler_CommandsDialog_Dtor(int ecx, int edx);

// 0x004c8bde
int __fastcall EH_Handler_OptionsPanel_BuildWidgets(int ecx, int edx);

// 0x004c8ca0
int __fastcall EH_Handler_OptionsPanel_Dtor(int ecx, int edx);

// 0x004c9398
int __fastcall EH_Handler_MainMenu_Ctor(int ecx, int edx);

// 0x004c941a
int __fastcall EH_Handler_MainMenu_Dtor(int ecx, int edx);

// 0x004c9494
int __fastcall EH_Handler_ConfirmQuitScreen_Ctor(int ecx, int edx);

// 0x004c94b6
int __fastcall EH_Handler_ScreenHost_Dtor(int ecx, int edx);

// 0x004c9611
int __fastcall EH_Handler_MPExitScreen_Create(int ecx, int edx);

// 0x004c9752
int __fastcall EH_Handler_MPNewGamePanel_BuildWidgets(int ecx, int edx);

// 0x004c98a4
int __fastcall EH_Handler_MPNewGamePanel_Dtor(int ecx, int edx);

// 0x004c9a04
int __fastcall EH_Handler_NetExitPanel_BuildWidgets(int ecx, int edx);

// 0x004c9a26
int __fastcall EH_Handler_Panel_0041beb0_Dtor(int ecx, int edx);

// 0x004c9ab0
int __fastcall EH_Handler_NewGamePanel_Ctor(int ecx, int edx);

// 0x004c9af2
int __fastcall EH_Handler_NewGamePanel_Dtor(int ecx, int edx);

// 0x004ca1cc
int __fastcall EH_Handler_SaveGameDialog_Ctor(int ecx, int edx);

// 0x004ca22e
int __fastcall EH_Handler_SaveLoadDialogBase_Dtor(int ecx, int edx);

// 0x004ca296
int __fastcall EH_Handler_SaveGameDialog_Dtor(int ecx, int edx);

// 0x004ca30c
int __fastcall EH_Handler_LoadGameDialog_Ctor(int ecx, int edx);

// 0x004ca376
int __fastcall EH_Handler_LoadGameDialog_Dtor(int ecx, int edx);

// 0x004ca3fb
int __fastcall EH_Handler_MessageBox_RunModal(int ecx, int edx);

// 0x0041b780
int __fastcall Menus_OpenHelpDocs_IndexHtml_0041b780(int ecx, int edx);
// 0x0041b2f0
int __fastcall NetConnectDlg_SelectProvider_0041b2f0(int ecx, int edx);
}  // namespace recoil
