// SUBSYSTEM: mapscreen
// Declarations for src/Battlesport/map.cpp.
#pragma once

namespace recoil {

// 0x00415aa0
int __fastcall MapMarker_SetVtbl(int ecx, int edx);
// 0x00415ac0
int __fastcall MapMarker_Dtor(int ecx, int edx);
// 0x00415b10
int __fastcall MapMarkerList_Select(int ecx, int edx, int arg1);
// 0x00415b40
int __fastcall MapMarker_Init(int ecx, int edx);
// 0x00415b70
int __fastcall MapMarker_SetColour(int ecx, int edx, int arg1);
// 0x00415c90
int __fastcall MapMarker_GetBounds(int ecx, int edx, int arg1);
// 0x00415f40
int __fastcall Draw_Diamond(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00416240
int __fastcall Rect_OutCode(int ecx, int edx);
// 0x00416290
int __fastcall MapMarker_IsTypeGroup(int ecx, int edx);
// 0x00416390
int __fastcall Vec2_OrientTest(int ecx, int edx, int arg1);
// 0x004166e0
int __fastcall MapScreen_SetRects(int ecx, int edx, int arg1, int arg2);
// 0x00416be0
int __fastcall MapScreen_UpdateZoomAnim(int ecx, int edx);
// 0x00416c90
int __fastcall MapScreen_WorldToMap(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00416dd0
int __fastcall Draw_Cross(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00416e50
int __fastcall MapScreen_EntityOffset(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00416ef0
int __fastcall MapScreen_SetRadarRange(int ecx, int edx, int arg1);
// 0x00417220
int __fastcall MapScreen_SetFocusEntity(int ecx, int edx, int arg1);
// 0x00415ab0
int __fastcall MapMarker_Ctor(int ecx, int edx);
// 0x00415ae0
int __fastcall MapMarker_SetState(int ecx, int edx, int arg1);
// 0x00415bd0
int __fastcall MapMarker_ReadFile(int ecx, int edx, int arg1);
// 0x004162b0
int __fastcall Rect_ClipEdgeIntersect(int ecx, int edx, int arg1, int arg2);
// 0x00416660
int __fastcall MapScreen_Init(int ecx, int edx, int arg1);
// 0x004167e0
int __fastcall MapScreen_RemoveMarker(int ecx, int edx, int arg1);
// 0x00416840
int __fastcall MapScreen_AppendMarker(int ecx, int edx, int arg1);
// 0x00416d50
int __fastcall MapScreen_DrawFocusMarker(int ecx, int edx);
// 0x00416f10
int __fastcall MapScreen_DrawEntityBlip(int ecx, int edx, int arg1);
// 0x00415fb0
int __fastcall MapLine_ClipAgainstInnerRect(int ecx, int edx, int arg1);
// 0x00416650
int __fastcall MapScreen_Construct(int ecx, int edx);
// 0x00416480
int __fastcall MapMarker_DrawWorldOverlay(int ecx, int edx, int arg1);
// 0x00415d30
int __fastcall MapMarkerList_Draw(int ecx, int edx, int arg1, int arg2);
// 0x00417130
int __fastcall MapScreen_Update(int ecx, int edx);
// 0x004167a0
int __fastcall MapScreen_Destruct(int ecx, int edx);
// 0x004168d0
int __fastcall MapScreen_ReadFile(int ecx, int edx, int arg1);
// 0x004169d0
int __fastcall MapScreen_LoadFile(int ecx, int edx, int arg1);
// 0x00416a30
int __fastcall MapScreen_BeginZoomToFit(int ecx, int edx);
// 0x00416ad0
int __fastcall MapScreen_BeginZoomBack(int ecx, int edx);
// 0x00416b30
int __fastcall MapScreen_PushVisible(int ecx, int edx, int arg1);
// 0x004c9500
int __fastcall EH_Unwind_MapScreen_ReadFile_0(int ecx, int edx);
// 0x004c950b
int __fastcall EH_Handler_MapScreen_ReadFile(int ecx, int edx);

}  // namespace recoil
