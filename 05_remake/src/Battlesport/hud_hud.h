// SUBSYSTEM: hud
// Declarations for src/Battlesport/hud_hud.cpp.
#pragma once

namespace recoil {

// 0x00410140
int __fastcall Hud_ConsumeCounter_004e61bc(int ecx, int edx);
// 0x00411720
int __fastcall Hud_GetVec3_004e61f8(int ecx, int edx);
// 0x00411740
int __fastcall Hud_SetGlobal_004e6230(int ecx, int edx);
// 0x00412bd0
int __fastcall VMethod_ReturnTrue_00412bd0(int ecx, int edx, int arg1);
// 0x00413aa0
int __fastcall ConfigValue_GetRect4(int ecx, int edx);
// 0x00413ad0
int __fastcall ConfigValue_GetVec3Opt(int ecx, int edx, int arg1, int arg2);
// 0x00413a10
int __fastcall ConfigValue_GetRectOffset(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00413eb0
int __fastcall Hud_Nop(int ecx, int edx);
// 0x00414210
int __fastcall Loading_InitCheckpointTable(int ecx, int edx);
// 0x00414300
int __fastcall Hud_GetRect_004edb58(int ecx, int edx);
// 0x004118b0
int __fastcall Hud_LayoutFromFontHeight(int ecx, int edx);
// 0x00411900
int __fastcall Hud_ShowPopup(int ecx, int edx, int arg1, int arg2);
// 0x00411a20
int __fastcall Hud_ClosePopup(int ecx, int edx);
// 0x00412050
int __fastcall Hud_PrintToConsole_004e6a98(int ecx, int edx);
// 0x004122c0
int __fastcall Hud_PickTargetMarker(int ecx, int edx);
// 0x00412620
int __fastcall Hud_HideIfOwner(int ecx, int edx);
// 0x004126e0
int __fastcall HudPlayerSlot_SetMode(int ecx, int edx);
// 0x00412790
int __fastcall HudPlayerSlot_SelectEntry(int ecx, int edx);
// 0x004127d0
int __fastcall HudPlayerSlot_Clear(int ecx, int edx);
// 0x00412be0
int __fastcall Hud_ForwardTo_004bc900(int ecx, int edx, int arg1);
// 0x00412bf0
int __fastcall VMethod_CallSlot1_True_00412bf0(int ecx, int edx);
// 0x00412c00
int __fastcall VMethod_CallSlot1_False_00412c00(int ecx, int edx);
// 0x00413080
int __fastcall HudImages_Release(int ecx, int edx);
// 0x004132b0
int __fastcall HudMessage_LayoutRect(int ecx, int edx);
// 0x00413500
int __fastcall Hud_DrawOverlay_00413500(int ecx, int edx, int arg1);
// 0x00413540
int __fastcall Hud_ResetAllVisibility(int ecx, int edx);
// 0x004135f0
int __fastcall VMethod_CallSlot1_False_004135f0(int ecx, int edx);
// 0x00413770
int __fastcall Hud_SetOverlayVisible(int ecx, int edx);
// 0x004137a0
int __fastcall Hud_ShowPanel_004e6db0(int ecx, int edx);
// 0x004137f0
int __fastcall Hud_SetMessageLine(int ecx, int edx, int arg1);
// 0x004138f0
int __fastcall Hud_ForwardIfActive_0056bd20(int ecx, int edx, int arg1);
// 0x00413950
int __fastcall Hud_HideBothPanels(int ecx, int edx);
// 0x00413990
int __fastcall ConfigValue_PlaceTextWidget(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00413d30
int __fastcall ConfigValue_PlaceImage(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00413ff0
int __fastcall HudScoreImages_Release(int ecx, int edx);
// 0x00414070
int __fastcall HudStatusBox_Layout(int ecx, int edx);
// 0x00414330
int __fastcall Hud_ShowKillMessage(int ecx, int edx, int arg1);
// 0x00411750
int __fastcall Hud_ForwardToGlobal_004e62f0(int ecx, int edx);
// 0x004137c0
int __fastcall Hud_ResetKeyBindings(int ecx, int edx);
// 0x00414180
int __fastcall Loading_Checkpoint(int ecx, int edx);
// 0x00412c10
int __fastcall HudRect_LoadFromConfig(int ecx, int edx, int arg1);
}  // namespace recoil
