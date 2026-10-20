// SUBSYSTEM: mapscreen
// Declarations for src/unattributed/mapscreen.cpp.
#pragma once

namespace recoil {

// 0x00498f90
int __fastcall SW_PlotPixel_Indirect(int ecx, int edx, int arg1);
// 0x004bd6f0
int __fastcall ClipRect_Set2D(int ecx, int edx);
// 0x004bd720
int __fastcall ClipSegment_NearFar(int ecx, int edx);
// 0x004bd840
int __fastcall ClipSegment_2D(int ecx, int edx, int arg1, int arg2);
// 0x00415650
int __fastcall MapScreen_SetActiveMarker(int ecx, int edx);
// 0x00416b80
int __fastcall Slot_ScaleField0xA4_ThenSound_004cefcc_00416b80(int ecx, int edx);
// 0x00416bb0
int __fastcall Slot_ScaleField0xA4_ThenSound_004cefd0_00416bb0(int ecx, int edx);
}  // namespace recoil
