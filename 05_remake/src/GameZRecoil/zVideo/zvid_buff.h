// SUBSYSTEM: zvideo
// Declarations for src/GameZRecoil/zVideo/zvid_buff.cpp.
#pragma once

namespace recoil {

// 0x004a6cf0
int __fastcall Pixel_FromRGBBytes(int ecx, int edx, int arg1);
// 0x004a6d40
int __fastcall Pixel_PackRGB(int ecx, int edx);
// 0x004a6b90
int __fastcall Pixfmt_GetFormat(int ecx, int edx, int arg1);
// 0x004a6b80
int __fastcall zVideo_SetGlobal_006321cc(int ecx, int edx);
// 0x004a7220
int __fastcall HwBlend_SetColourA(int ecx, int edx);
// 0x004a7250
int __fastcall HwBlend_SetColourB(int ecx, int edx);
// 0x004a6710
int __fastcall zVideo_GetGlobal_00632210(int ecx, int edx);
// 0x004a6720
int __fastcall Render_GetScreenWidth(int ecx, int edx);
// 0x004a6730
int __fastcall Render_GetScreenHeight(int ecx, int edx);
// 0x004a6740
int __fastcall zVideo_GetGlobal_00632208(int ecx, int edx);
// 0x004a67e0
int __fastcall zVideo_GetGlobal_00632214(int ecx, int edx);
// 0x004a67f0
int __fastcall zVideo_GetGlobal_00632230(int ecx, int edx);
// 0x004a6800
int __fastcall Render_GetSurfaceWidthCopy(int ecx, int edx);
// 0x004a6810
int __fastcall Render_GetSurfaceHeightCopy(int ecx, int edx);
// 0x004a6820
int __fastcall zVideo_GetGlobal_00632228(int ecx, int edx);
// 0x004a69c0
int __fastcall zVideo_ClampValueReturnDelta(int ecx, int edx, int arg1);
// 0x004a6b40
int __fastcall zVideo_SetHardwareMode(int ecx, int edx);
// 0x004a6b60
int __fastcall zVideo_SetGlobal_00632138(int ecx, int edx);
// 0x004a6b70
int __fastcall zVideo_SetGlobal_0063213c(int ecx, int edx);
// 0x004a6bb0
int __fastcall Pixfmt_GetChannelMasks(int ecx, int edx, int arg1);
// 0x004a6bd0
int __fastcall Pixfmt_GetChannelShifts(int ecx, int edx, int arg1);
// 0x004a6770
int __fastcall zVideo_NotifyScreenResize(int ecx, int edx);
// 0x004a67d0
int __fastcall zVideo_CallSurfaceSlot3C0_Back(int ecx, int edx);
// 0x004a6830
int __fastcall zVideo_CallSurfaceSlot3C8(int ecx, int edx);
// 0x004a6840
int __fastcall zVideo_NotifyScreenResizeFromSurface(int ecx, int edx);
// 0x004a68d0
int __fastcall zVideo_CallSurfaceSlot3C0_Front(int ecx, int edx);
// 0x004a68e0
int __fastcall zVideo_CallSurfaceSlot3C4_Record240(int ecx, int edx);
// 0x004a68f0
int __fastcall zVideo_CallSurfaceSlot3C0_Record240(int ecx, int edx);
// 0x004a6900
int __fastcall zVideo_PresentFrame(int ecx, int edx, int arg1, int arg2);
// 0x004a6bf0
int __fastcall zVideo_SetScreenPixelFormat(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004a6ca0
int __fastcall Pixel_FromColorRef(int ecx, int edx);
// 0x004a6db0
int __fastcall zVideo_SetTexturePixelFormat(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x004a7200
int __fastcall zVideo_GetSurfaceRect(int ecx, int edx);
// 0x004a7300
int __fastcall HwBlend_SetColourC(int ecx, int edx);
// 0x004a7410
int __fastcall zVideo_GetDriverName(int ecx, int edx);
// 0x004a71c0
int __fastcall zVideo_SetFlag0063212c(int ecx, int edx);
// 0x004a7330
int __fastcall HwBlend_ApplyColourIfChanged(int ecx, int edx);
// 0x004a73a0
int __fastcall HwBlend_ApplyColourBIfChanged(int ecx, int edx);
// 0x004a6e80
int __fastcall zVideo_CaptureSurfaceToImage(int ecx, int edx);
// 0x004a6fe0
int __fastcall zVideo_CopySurfaceRectToImage(int ecx, int edx, int arg1);
// 0x004a6930
int __fastcall zVideo_PrepareFullscreenWindow(int ecx, int edx);
// 0x004a7520
int __fastcall Thunk_Render_ReleaseDeviceInterfaces(int ecx, int edx);
// 0x004a69e0
int __fastcall zVideo_BlitImageToBackBuffer(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004a74d0
int __fastcall Render_BeginSceneIfIdle(int ecx, int edx);
// 0x004a74f0
int __fastcall Render_EndSceneIfLast(int ecx, int edx);
// 0x004a7490
int __fastcall zVideo_SelectDriver(int ecx, int edx);
// 0x004a7530
int __fastcall Render_BringUpDirectDraw(int ecx, int edx);
}  // namespace recoil
