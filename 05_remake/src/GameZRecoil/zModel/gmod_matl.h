// SUBSYSTEM: asset_io
// Declarations for src/GameZRecoil/zModel/gmod_matl.cpp.
#pragma once

namespace recoil {

// 0x00481530
int __fastcall Model_GetGlobalFloat_004e1398(int ecx, int edx);
// 0x00481540
int __fastcall Model_SetGlobalFloat_004e1398(int ecx, int edx, int arg1);
// 0x00481550
int __fastcall Model_SetGlobalDouble_004e1388(int ecx, int edx, int arg1);
// 0x00481560
int __fastcall Model_SetGlobalDouble_004e1390(int ecx, int edx, int arg1);
// 0x00481570
int __fastcall Model_ToIndex(int ecx, int edx);
// 0x004804c0
int __fastcall Model_SetScaleGlobals(int ecx, int edx, int arg1);
// 0x004805b0
int __fastcall Material_ToIndex(int ecx, int edx);
// 0x004805e0
int __fastcall Material_FromIndex(int ecx, int edx);
// 0x00480bf0
int __fastcall Material_SetArraySize(int ecx, int edx);
// 0x00480c40
int __fastcall Material_Init(int ecx, int edx);
// 0x00480c80
int __fastcall Material_HasTextureOrFlags(int ecx, int edx);
// 0x00480d20
int __fastcall Material_Compare(int ecx, int edx);
// 0x00480ec0
int __fastcall Model_FreeExtraBuffers(int ecx, int edx);
// 0x00480f60
int __fastcall Material_SetFlag0x200(int ecx, int edx);
// 0x00481040
int __fastcall Material_SetField20(int ecx, int edx);
// 0x00481100
int __fastcall Material_AppendCycleFrame(int ecx, int edx);
// 0x00481220
int __fastcall Material_SetCycleTextureLoop(int ecx, int edx);
// 0x00481260
int __fastcall Material_SetCycleTextureSpeed(int ecx, int edx, int arg1);
// 0x004812c0
int __fastcall Material_AllocCopy(int ecx, int edx);
// 0x00481420
int __fastcall Material_FindByTexture(int ecx, int edx);
// 0x004803b0
int __fastcall Poly_ScreenRectReject(int ecx, int edx);
// 0x004804e0
int __fastcall MaterialName_FindIndex(int ecx, int edx);
// 0x00480ae0
int __fastcall Material_InitArray(int ecx, int edx);
// 0x00480dc0
int __fastcall Material_Free(int ecx, int edx);
// 0x00481050
int __fastcall Material_SetCycleFrameCount(int ecx, int edx);
// 0x00480600
int __fastcall Material_WriteSection(int ecx, int edx);
// 0x00480d80
int __fastcall Material_FreeAllUsed(int ecx, int edx);
// 0x00480ca0
int __fastcall Material_FindOrCreate(int ecx, int edx);
// 0x00480f10
int __fastcall Material_Shutdown(int ecx, int edx);
// 0x004808c0
int __fastcall Material_ReadSection(int ecx, int edx);
// 0x00480f80
int __fastcall Material_ReleaseTextures(int ecx, int edx);
// 0x00480fd0
int __fastcall Material_ReuploadTextures(int ecx, int edx);
// 0x00481460
int __fastcall MaterialName_LoadFromConfig(int ecx, int edx);
// 0x00481140
int __fastcall Material_AdvanceCycle(int ecx, int edx);
}  // namespace recoil
