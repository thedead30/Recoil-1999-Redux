// SUBSYSTEM: texture
// Declarations for src/GameZRecoil/zImage/zimg_fonts.cpp.
#pragma once

namespace recoil {

// 0x0046ecf0
int __fastcall Image_FreeOwnedBuffers(int ecx, int edx);
// 0x0046ecc0
int __fastcall Image_Free(int ecx, int edx);
// 0x0046d5a0
int __fastcall Image_FreeUnlessDefault(int ecx, int edx);
// 0x0046e250
int __fastcall TextureSlot_UnloadChain(int ecx, int edx);
// 0x0046e680
int __fastcall ShadeEntry_FindExact(int ecx, int edx);
// 0x0046ebd0
int __fastcall TextureManager_AddSearchPaths(int ecx, int edx);
// 0x0046efc0
int __fastcall TextureTable_GetOrDefault(int ecx, int edx);
// 0x0046e2c0
int __fastcall Path_SetExtension(int ecx, int edx);
// 0x0046ec20
int __fastcall Image_BytesPerPixel(int ecx, int edx);
// 0x0046ec00
int __fastcall Image_Alloc(int ecx, int edx);
// 0x0046ec30
int __fastcall Image_SetByte8(int ecx, int edx);
// 0x0046ec60
int __fastcall Image_SetFlags(int ecx, int edx);
// 0x0046ec90
int __fastcall Image_SetSize(int ecx, int edx, int arg1);
// 0x0046ec70
int __fastcall Image_SetPixelsAndAlpha(int ecx, int edx, int arg1);
// 0x0046e9b0
int __fastcall Texture_ResampleSquare(int ecx, int edx);
// 0x0046e380
int __fastcall TextureSlot_SetNameFromPath(int ecx, int edx);
// 0x0046e960
int __fastcall Tex_CallWithDefaultRecord(int ecx, int edx);
// 0x0046eba0
int __fastcall TextureManager_Init(int ecx, int edx);
// 0x0046ec40
int __fastcall Image_PixelDataSize(int ecx, int edx);
// 0x0046ed70
int __fastcall Image_ReadHeader(int ecx, int edx);
// 0x0046ebb0
int __fastcall TextureManager_ShutdownAll(int ecx, int edx);
// 0x0046e4e0
int __fastcall ShadeLevel_Generate(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0046e720
int __fastcall ShadeEntry_Add(int ecx, int edx);
// 0x0046e8d0
int __fastcall Image_BuildShadeLevels(int ecx, int edx);
// 0x0046eb90
int __fastcall TextureManager_Shutdown(int ecx, int edx);
// 0x0046ede0
int __fastcall Image_ReadPixels(int ecx, int edx, int arg1);
// 0x0046ef70
int __fastcall Image_CreateProcedural(int ecx, int edx);
// 0x0046e3e0
int __fastcall TextureSlot_LoadMipChain(int ecx, int edx);
// 0x0046efe0
int __fastcall Font_LoadAll(int ecx, int edx);
// 0x0046eb20
int __fastcall TextureTable_Init(int ecx, int edx);
}  // namespace recoil
