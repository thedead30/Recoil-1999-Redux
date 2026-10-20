// SUBSYSTEM: zvideo
// Declarations for src/unattributed/zvideo.cpp.
#pragma once

namespace recoil {

// 0x004903f0
int __fastcall Video_GetSurfaceInfo(int ecx, int edx, int arg1, int arg2);
// 0x004a66e0
int __fastcall zVideo_GetScreenDepth(int ecx, int edx);
// 0x004c7fd0
int __fastcall ZVid_LoadPalette(int ecx, int edx);
// 0x0048d910
int __fastcall Screen_RandomDissolve(int ecx, int edx, int arg1, int arg2);
// 0x004a82f0
int __fastcall zVideo_ClearBackBufferColourAndDepthRects(int ecx, int edx);
// 0x004a7770
int __fastcall zVideo_RestoreIfMinimised_004a7770(int ecx, int edx);
// 0x004a7430
int __fastcall ZVideoTable_ElementAddr_00633e78_004a7430(int ecx, int edx);
// 0x004a7450
int __fastcall ZVideoTable_ElementAddr_00633e58_004a7450(int ecx, int edx);
// 0x004a9a30
int __fastcall Render_GetDriverSecondaryMemory(int ecx, int edx, int arg1);
// 0x004a7b40
int __fastcall DDraw_BootstrapDeviceEnumeration(int ecx, int edx);
// 0x004a6750
int __fastcall zVideo_CallDispatch_006333d0(int ecx, int edx);
// 0x004a6760
int __fastcall zVideo_CallDispatch_006333cc(int ecx, int edx);
}  // namespace recoil
