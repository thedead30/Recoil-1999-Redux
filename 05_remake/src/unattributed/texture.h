// SUBSYSTEM: texture
// Declarations for src/unattributed/texture.cpp.
#pragma once

namespace recoil {

// 0x0046d550
int __fastcall TextureManager_Reset(int ecx, int edx);
// 0x0046d730
int __fastcall TextureArchive_CloseAllFiles(int ecx, int edx);
// 0x0046d780
int __fastcall TextureArchiveListB_FreeAll(int ecx, int edx);
// 0x004902b0
int __fastcall Image_PrepareSoftwareSampling(int ecx, int edx);
// 0x0046d5b0
int __fastcall Texture_SetFlag_004e073c(int ecx, int edx);
// 0x0046d5c0
int __fastcall Texture_GetFlag_004e073c(int ecx, int edx);
// 0x0046f210
int __fastcall Image_IsColumnTransparent(int ecx, int edx);
// 0x00479c60
int __fastcall Palette_SetName(int ecx, int edx);
// 0x0046d6b0
int __fastcall TextureArchiveListA_FreeAll(int ecx, int edx);
// 0x0046d870
int __fastcall Image_ApplyAlphaMask(int ecx, int edx);
// 0x0046d5d0
int __fastcall TextureManager_FreeAllSlots(int ecx, int edx);
// 0x0046d810
int __fastcall TextureManager_FindOrAddSlot(int ecx, int edx);
// 0x0046dae0
int __fastcall TextureArchive_Open(int ecx, int edx);
// 0x0046da40
int __fastcall TextureArchiveListB_OpenImageZbd(int ecx, int edx);
// 0x0046df50
int __fastcall TextureArchiveListA_OpenBySize(int ecx, int edx);
// 0x0046d940
int __fastcall Image_LoadFromArchive(int ecx, int edx);
// 0x0046dd30
int __fastcall Image_LoadFromArchiveListA(int ecx, int edx);
// 0x0046d900
int __fastcall Image_Load(int ecx, int edx);
// 0x0046de50
int __fastcall TextureManager_LoadPending(int ecx, int edx);
}  // namespace recoil
