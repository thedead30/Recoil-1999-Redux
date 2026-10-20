// SUBSYSTEM: hud
// Declarations for src/unattributed/hud.cpp.
#pragma once

namespace recoil {

// 0x0040eca0
int __fastcall HudTimer_SetPaused(int ecx, int edx);
// 0x0040ecc0
int __fastcall HudTimer_SetTime(int ecx, int edx, int arg1);
// 0x0040ed10
int __fastcall HudTimer_GetTime(int ecx, int edx);
// 0x00414670
int __fastcall Array50_Count(int ecx, int edx);
// 0x00414b50
int __fastcall VMethod_ReturnFalse4_00414b50(int ecx, int edx, int arg1);
// 0x0040e800
int __fastcall Scoreboard_UpdateEntry(int ecx, int edx, int arg1);
// 0x0040e880
int __fastcall Scoreboard_RemoveEntry(int ecx, int edx, int arg1);
// 0x00404c50
int __fastcall Loading_SetProgress(int ecx, int edx, int arg1);
// 0x0040eae0
int __fastcall HudPanel_ForwardUpdate(int ecx, int edx, int arg1);
// 0x0040f0f0
int __fastcall HudTriple_ReleaseImages(int ecx, int edx);
// 0x0040f130
int __fastcall HudModeIcon_Layout(int ecx, int edx);
// 0x0040f1a0
int __fastcall HudModeIcon_Select(int ecx, int edx);
// 0x0040f460
int __fastcall HudPips_SetCount(int ecx, int edx, int arg1);
// 0x004146a0
int __fastcall Array50_CopyRange(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004146e0
int __fastcall Array50_FillCopy(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00414ab0
int __fastcall InterpZbd_Construct(int ecx, int edx);
// 0x00476370
int __fastcall ByteSet_Intersects(int ecx, int edx);
// 0x0040fb90
int __fastcall HUD_BuildTimerSaveRecord(int ecx, int edx);
// 0x0040ec90
int __fastcall HudItem_NopOnSub(int ecx, int edx);
// 0x0040f3e0
int __fastcall HudLabels_Nop3(int ecx, int edx);
// 0x004143b0
int __fastcall Hud_ForwardToPanel_0040e800(int ecx, int edx);
// 0x004143c0
int __fastcall Hud_ForwardToPanel_0040e880(int ecx, int edx);
// 0x00414a70
int __fastcall Hud_StaticInit_Construct_004edb78(int ecx, int edx);
// 0x0040f2e0
int __fastcall HudTriple_LayoutFromConfig(int ecx, int edx, int arg1);
// 0x0040e590
int __fastcall Scoreboard_AddEntry(int ecx, int edx, int arg1);
// 0x0040ee60
int __fastcall HudTimer_SetSeconds(int ecx, int edx, int arg1);
// 0x0040f070
int __fastcall HudModeIcon_InitFromConfig(int ecx, int edx, int arg1);
// 0x0040ece0
int __fastcall HudTimer_SetLimit(int ecx, int edx, int arg1, int arg2);
// 0x0040eb00
int __fastcall HudItem_LayoutFromConfig(int ecx, int edx, int arg1);
// 0x00413720
int __fastcall AtexitStub_CWnd_Dtor_00413720(int ecx, int edx);
// 0x00414ad0
int __fastcall ScriptCmd_WeaponSetMaxTetherAltitude_00414ad0(int ecx, int edx, int arg1);
// 0x004136f0
int __fastcall StaticInitWrapper_004136f0(int ecx, int edx);
// 0x0040fbd0
int __fastcall Hud_Shutdown(int ecx, int edx);
// 0x0040fdd0
int __fastcall HudPanel23_Destruct(int ecx, int edx);
// 0x0040fe30
int __fastcall HudItem_Destruct(int ecx, int edx);
// 0x0040fe90
int __fastcall HudPanel4_Destruct(int ecx, int edx);
// 0x0040fef0
int __fastcall HudPanel4b_Destruct(int ecx, int edx);
// 0x00414a60
int __fastcall StaticInitWrapper_00414a60(int ecx, int edx);
// 0x00414a80
int __fastcall Hud_StaticInit_RegisterDtor_004edb78(int ecx, int edx);
// 0x00414a90
int __fastcall AtexitStub_thunk_Script_Dtor_00414a90(int ecx, int edx);
// 0x00439690
int __fastcall Hud_UpdateTargetMarkers(int ecx, int edx);
// 0x004c9230
int __fastcall EH_Unwind_HudPanel23_Destruct_0(int ecx, int edx);
// 0x004c9250
int __fastcall EH_Unwind_HudItem_Destruct_0(int ecx, int edx);
// 0x004c9270
int __fastcall EH_Unwind_HudPanel4_Destruct_0(int ecx, int edx);
// 0x004c9290
int __fastcall EH_Unwind_HudPanel4b_Destruct_0(int ecx, int edx);
// 0x004c9238
int __fastcall EH_Handler_HudPanel23_Destruct(int ecx, int edx);

// 0x004c925b
int __fastcall EH_Handler_HudItem_Destruct(int ecx, int edx);

// 0x004c9278
int __fastcall EH_Handler_HudPanel4_Destruct(int ecx, int edx);

// 0x004c9298
int __fastcall EH_Handler_HudPanel4b_Destruct(int ecx, int edx);

// 0x00414660
int __fastcall Thunk_00414590(int ecx, int edx);
// 0x00414590
int __fastcall Hud_BuildNameMessage_00414590(int ecx, int edx);
}  // namespace recoil
