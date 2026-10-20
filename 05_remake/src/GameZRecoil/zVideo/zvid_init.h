// SUBSYSTEM: zvideo
// Declarations for src/GameZRecoil/zVideo/zvid_init.cpp.
#pragma once

namespace recoil {

// 0x004a7b20
int __fastcall zVideo_SetD3DDeviceCreated(int ecx, int edx);
// 0x004a7b30
int __fastcall zVideo_IsD3DDeviceCreated(int ecx, int edx);
// 0x004a7990
int __fastcall Render_ApplyResolutionPreset(int ecx, int edx);
// 0x004a7af0
int __fastcall zVideo_SetVideoMode(int ecx, int edx);
// 0x004a7700
int __fastcall zVideo_CacheClientRectScreen(int ecx, int edx);
// 0x004a7740
int __fastcall zVideo_Close(int ecx, int edx);
// 0x004a75f0
int __fastcall zVideo_Open(int ecx, int edx, int arg1, int arg2);
// 0x004a77a0
int __fastcall Render_InstallDrawDispatchTable(int ecx, int edx);
}  // namespace recoil
