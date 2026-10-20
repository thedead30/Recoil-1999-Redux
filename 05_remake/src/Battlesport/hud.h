// SUBSYSTEM: menus
// Declarations for src/Battlesport/hud.cpp.
#pragma once

namespace recoil {

// 0x00411eb0
int __fastcall Hud_ShowStandardWidgets(int ecx, int edx);
// 0x00413630
int __fastcall CallGlobal004e5ee8Slot18(int ecx, int edx);
// 0x004138d0
int __fastcall Hud_ShowMessage(int ecx, int edx, int arg1);
// 0x00413910
int __fastcall Hud_ClearMessagePanels(int ecx, int edx);
// 0x00410d10
int __fastcall View_SetScreenRect(int ecx, int edx, int arg1, int arg2);
// 0x00410e90
int __fastcall Inset_Enable(int ecx, int edx);
// 0x00411760
int __fastcall Hud_SetRadarVisible(int ecx, int edx);
// 0x004117f0
int __fastcall Hud_RadarOpenAnimate(int ecx, int edx);
// 0x00411f10
int __fastcall Hud_SetHealthBar(int ecx, int edx, int arg1);
// 0x004124b0
int __fastcall Hud_UpdateTargetHealthBar(int ecx, int edx);
// 0x00412650
int __fastcall HudPlayerSlot_SetTimerText(int ecx, int edx, int arg1);
// 0x00412820
int __fastcall HudPlayerSlot_SetActiveTimer(int ecx, int edx, int arg1);
// 0x00412f70
int __fastcall HudScorePanel_LoadFromConfig(int ecx, int edx, int arg1);
// 0x00413ec0
int __fastcall HudStatusBox_Init(int ecx, int edx, int arg1, int arg2);
// 0x00411ac0
int __fastcall Hud_PopupTick(int ecx, int edx);
// 0x00413340
int __fastcall HudPanel_Reset_SelectLayoutByWidth_00413340(int ecx, int edx);
// 0x00414390
int __fastcall Hud_ForwardToPanel_0040e590(int ecx, int edx);
// 0x00412db0
int __fastcall Hud_ApplyViewportRect(int ecx, int edx);
// 0x00413b10
int __fastcall ConfigValue_SetQuadCorners(int ecx, int edx, int arg1, int arg2);
// 0x00413c10
int __fastcall ConfigValue_SetQuadRectInclusive(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004134e0
int __fastcall UiScreen_Slot_Base0x4b3fb0_ThenMemberE0Slot4(int ecx, int edx);
// 0x00412c60
int __fastcall HudPanel_Layout_Split_00412c60(int ecx, int edx, int arg1);
// 0x004130d0
int __fastcall HudPanel_Layout_Full_004130d0(int ecx, int edx, int arg1);
// 0x00413700
int __fastcall Hud_StaticInit_ConstructWnd(int ecx, int edx);
// 0x00413730
int __fastcall MciDevice_DestroyGlobal(int ecx, int edx);
// 0x00410ed0
int __fastcall Inset_Disable(int ecx, int edx);
// 0x00411170
int __fastcall Hud_WorldToRadarNDC(int ecx, int edx);
// 0x00412070
int __fastcall Hud_AddTargetMarker(int ecx, int edx);
// 0x00413660
int __fastcall Hud_SetActiveObject(int ecx, int edx);
// 0x00410160
int __fastcall Hud_Init(int ecx, int edx);
// 0x004136b0
int __fastcall Hud_ApplyTypeChange(int ecx, int edx);
// 0x00410fe0
int __fastcall Hud_Tick(int ecx, int edx);
// 0x00413710
int __fastcall Hud_StaticInit_RegisterDtor_004e5e90(int ecx, int edx);
// 0x00412b60
int __fastcall HudContainer_Construct(int ecx, int edx);
// 0x00412ea0
int __fastcall HudScorePanel_Construct(int ecx, int edx);
// 0x004c92b0
int __fastcall EH_Unwind_HudContainer_Construct_0(int ecx, int edx);
// 0x004c92b8
int __fastcall EH_Unwind_HudContainer_Construct_1(int ecx, int edx);
// 0x004c92d0
int __fastcall EH_Unwind_HudScorePanel_Construct_0(int ecx, int edx);
// 0x004c92d8
int __fastcall EH_Unwind_HudScorePanel_Construct_1(int ecx, int edx);
// 0x004c92e3
int __fastcall EH_Unwind_HudScorePanel_Construct_2(int ecx, int edx);
// 0x004c92eb
int __fastcall EH_Unwind_HudScorePanel_Construct_3(int ecx, int edx);
// 0x004c92f9
int __fastcall EH_Unwind_HudScorePanel_Construct_4(int ecx, int edx);
// 0x004c9307
int __fastcall EH_Unwind_HudScorePanel_Construct_5(int ecx, int edx);
// 0x00411270
int __fastcall Input_Mouse_CursorRaycastDispatch(int ecx, int edx, int arg1, int arg2);
// 0x004c92c3
int __fastcall EH_Handler_HudContainer_Construct(int ecx, int edx);

// 0x004c9315
int __fastcall EH_Handler_HudScorePanel_Construct(int ecx, int edx);

}  // namespace recoil
