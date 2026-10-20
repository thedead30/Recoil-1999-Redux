// SUBSYSTEM: asset_io
// Declarations for src/GameZRecoil/zImage/zimg_texture.cpp.
#pragma once

namespace recoil {

// 0x0046d310
int __fastcall Texture_ToIndex(int ecx, int edx);
// 0x0046d340
int __fastcall Texture_FromIndex(int ecx, int edx);
// 0x0046d4c0
int __fastcall zImage_GetTable_004e0718(int ecx, int edx);
// 0x0046d360
int __fastcall Texture_WriteDirectory(int ecx, int edx);
// 0x0046d420
int __fastcall Texture_ReadDirectory(int ecx, int edx);
}  // namespace recoil
