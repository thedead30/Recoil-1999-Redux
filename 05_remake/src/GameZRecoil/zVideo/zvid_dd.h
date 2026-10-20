// SUBSYSTEM: zvideo
// Declarations for src/GameZRecoil/zVideo/zvid_dd.cpp.
#pragma once

namespace recoil {

// 0x004a9900
int __fastcall zVideo_GetDriverCount(int ecx, int edx);
// 0x004a9910
int __fastcall zVideo_GetGlobal_00632f9c(int ecx, int edx);
// 0x004a9940
int __fastcall zVideo_GetDeviceGuidOrName(int ecx, int edx);
// 0x004a7d70
int __fastcall Render_FlipToGDISurface(int ecx, int edx);
// 0x004a88b0
int __fastcall DDraw_CreateSurface3(int ecx, int edx, int arg1, int arg2);
// 0x004a7b60
int __fastcall zVideo_BltPresent(int ecx, int edx, int arg1, int arg2);
// 0x004a7d90
int __fastcall zVideo_BltRecord0ToRecord1(int ecx, int edx);
// 0x004a7dd0
int __fastcall zVideo_BltRecord1ToRecord0(int ecx, int edx);
// 0x004a8060
int __fastcall zVideo_LockWithRestore(int ecx, int edx);
// 0x004a80c0
int __fastcall zVideo_UnlockWithRestore(int ecx, int edx);
// 0x004a8100
int __fastcall zVideo_LockWithRestore_B(int ecx, int edx);
// 0x004a8160
int __fastcall zVideo_UnlockWithRestore_B(int ecx, int edx);
// 0x004a81a0
int __fastcall zVideo_ClearDepthRect(int ecx, int edx);
// 0x004a8220
int __fastcall zVideo_ClearColourAndDepthRect(int ecx, int edx);
// 0x004a8500
int __fastcall Texture_UploadPixels(int ecx, int edx);
// 0x004a8720
int __fastcall DDraw_SetCooperativeLevelAndDisplayMode(int ecx, int edx);
// 0x004a90e0
int __fastcall zVideo_RestoreAllSurfaces(int ecx, int edx);
// 0x004a9160
int __fastcall zVideo_QueryObject28(int ecx, int edx);
// 0x004a91b0
int __fastcall Render_ReleaseDeviceInterfaces(int ecx, int edx);
// 0x004a9890
int __fastcall zVideo_SetPaletteEntries(int ecx, int edx, int arg1);
// 0x004a7fc0
int __fastcall zVideo_LockSurfaceRecord(int ecx, int edx);
// 0x004a8030
int __fastcall zVideo_UnlockSurfaceRecordIfLocked(int ecx, int edx);
// 0x004a83d0
int __fastcall DDraw_CreateTextureSurface(int ecx, int edx);
// 0x004a9300
int __fastcall Render_ShutdownDirectDraw(int ecx, int edx);
// 0x004a7d40
int __fastcall DDraw_ReleaseDevice(int ecx, int edx);
// 0x004a7e10
int __fastcall zVideo_BlitImageToBackBufferEx(int ecx, int edx, int arg1, int arg2);
// 0x004a8f80
int __fastcall Render_DetectSurfacePixelFormat(int ecx, int edx);
// 0x004a9060
int __fastcall zVideo_ProbeSurfacesLockable(int ecx, int edx);
// 0x004a96b0
int __fastcall D3D_EnumDevicesCallback(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x004a8b20
int __fastcall zVideo_CreateWindowedSurfaces(int ecx, int edx);
// 0x004a8dc0
int __fastcall zVideo_CreateFullscreenFlipChain(int ecx, int edx);
// 0x004a95e0
int __fastcall Render_EnumerateD3DDrivers(int ecx, int edx);
// 0x004a88f0
int __fastcall Render_CreatePrimarySurfacesAndZBuffer(int ecx, int edx);
// 0x004a84c0
int __fastcall Texture_CreateSurface(int ecx, int edx);
// 0x004a8650
int __fastcall Texture_ReleaseSurface(int ecx, int edx);
// 0x004a8680
int __fastcall Texture_GetDC(int ecx, int edx);
// 0x004a86f0
int __fastcall Texture_ReleaseDC(int ecx, int edx);
// 0x004a8790
int __fastcall Render_ResetDisplayMode(int ecx, int edx);
// 0x004a9920
int __fastcall Render_DriverIs3DCapable(int ecx, int edx);
// 0x004a9950
int __fastcall Render_GetDriverVideoMemory(int ecx, int edx, int arg1);
// 0x004a7d20
int __fastcall DDraw_EnsureDeviceCreated(int ecx, int edx);
// 0x004a8800
int __fastcall DDraw_CreateAndQueryInterface(int ecx, int edx);
// 0x004a8870
int __fastcall zVideo_SelectDriverByIndex(int ecx, int edx);
// 0x004a9390
int __fastcall DDraw_EnumerateGraphicsDevices(int ecx, int edx);
// 0x004a93d0
int __fastcall DDraw_EnumDriversCallback(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
}  // namespace recoil
