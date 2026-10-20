// SUBSYSTEM: zvideo
// Declarations for src/GameZRecoil/zVideo/zvid_ddd3d_zvideo.cpp.
#pragma once

namespace recoil {

// 0x004accc0
int __fastcall Render_FillVertexScratchColour(int ecx, int edx, int arg1);
// 0x004ad680
int __fastcall Math_FloorPowerOfTwo(int ecx, int edx);
// 0x004aa980
int __fastcall D3DTextureBinding_Free(int ecx, int edx);
// 0x004aa9e0
int __fastcall Render_SetFogEnabled(int ecx, int edx);
// 0x004ad120
int __fastcall Render_FlushQuadDrawQueue(int ecx, int edx);
// 0x004aa6f0
int __fastcall D3DTextureBinding_ConvertImageToArgb16(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
