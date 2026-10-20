// SUBSYSTEM: rasteriser
// Declarations for src/unattributed/rasteriser.cpp.
#pragma once

namespace recoil {

// 0x0049e0e0
int __fastcall BlendState_BuildTable(int ecx, int edx, int arg1, int arg2);
// 0x0048d420
int __fastcall SetGlobalRect_0056b1c4(int ecx, int edx, int arg1, int arg2);
// 0x0048d450
int __fastcall Tint_Row555(int ecx, int edx);
// 0x0048d4b0
int __fastcall Tint_Row565(int ecx, int edx);
// 0x0048d510
int __fastcall Tint_Row555_MMX(int ecx, int edx);
// 0x0048d5f0
int __fastcall Tint_Row565_MMX(int ecx, int edx);
// 0x0048da60
int __fastcall Raster_CopyPixelClipped(int ecx, int edx, int arg1, int arg2);
// 0x0048f560
int __fastcall Raster_BlitImageColorKey(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004907c0
int __fastcall Span_FrontTest(int ecx, int edx);
// 0x004912a0
int __fastcall Span_InsertOverwrite(int ecx, int edx, int arg1);
// 0x00491da0
int __fastcall Span_AllocOpen(int ecx, int edx, int arg1);
// 0x00492000
int __fastcall Raster_FillFlatPoly(int ecx, int edx, int arg1, int arg2);
// 0x004927d0
int __fastcall zRndr_FillClipPoly(int ecx, int edx);
// 0x00492f00
int __fastcall Raster_FillFlatPolyBlend(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004992b0
int __fastcall Raster_PutPixel16(int ecx, int edx, int arg1, int arg2);
// 0x004992d0
int __fastcall Raster_Line16(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004993a0
int __fastcall Raster_LineDashed16(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004997d0
int __fastcall Fill_Solid16_RightToLeft(int ecx, int edx);
// 0x00499810
int __fastcall Fill_BlendConst555(int ecx, int edx, int arg1);
// 0x004998a0
int __fastcall Fill_BlendConst565(int ecx, int edx, int arg1);
// 0x00499a00
int __fastcall Raster_SetFanBlendRecord(int ecx, int edx);
// 0x0049b4c0
int __fastcall BlendState_SelectPresetF10(int ecx, int edx);
// 0x0049b530
int __fastcall BlendState_SelectPresetDD0(int ecx, int edx);
// 0x0049b710
int __fastcall BlendState_SelectPresetE70(int ecx, int edx);
// 0x0049b780
int __fastcall BlendState_BlendPixel(int ecx, int edx);
// 0x0049c020
int __fastcall Fill_Tex8ShadeKeyedBlend565_Broken(int ecx, int edx, int arg1, int arg2);
// 0x0049c150
int __fastcall Fill_Tex16KeyedBlend_Broken(int ecx, int edx, int arg1, int arg2);
// 0x0049c230
int __fastcall Fill_Tex8ShadeKeyedBlend_555Slot_Broken(int ecx, int edx, int arg1, int arg2);
// 0x0049c360
int __fastcall Fill_Tex16AlphaMap565(int ecx, int edx, int arg1, int arg2);
// 0x0049c560
int __fastcall Fill_Tex16AlphaMap555(int ecx, int edx, int arg1, int arg2);
// 0x0049c760
int __fastcall Fill_Tex16Blend565_LeftToRight(int ecx, int edx, int arg1, int arg2);
// 0x0049c860
int __fastcall Fill_Tex16Blend555_LeftToRight(int ecx, int edx, int arg1, int arg2);
// 0x0049c970
int __fastcall Fill_Tex16AlphaLevelBlend565(int ecx, int edx, int arg1, int arg2);
// 0x0049ca90
int __fastcall Fill_Tex16AlphaLevelBlend555(int ecx, int edx, int arg1, int arg2);
// 0x0049cbb0
int __fastcall Fill_AlphaTex16_565_MMX(int ecx, int edx, int arg1, int arg2);
// 0x0049cea0
int __fastcall Fill_AlphaTex16_555_MMX(int ecx, int edx, int arg1, int arg2);
// 0x0049d1a0
int __fastcall Fill_Tex8ShadeAlphaMap565(int ecx, int edx, int arg1, int arg2);
// 0x0049d3b0
int __fastcall Fill_Tex8ShadeAlphaMap555(int ecx, int edx, int arg1, int arg2);
// 0x0049d5c0
int __fastcall Fill_Tex8ShadeBlend565_LeftToRight(int ecx, int edx, int arg1, int arg2);
// 0x0049d6e0
int __fastcall Fill_Tex8ShadeBlend555_LeftToRight(int ecx, int edx, int arg1, int arg2);
// 0x0049d810
int __fastcall Fill_Tex8ShadeAlphaLevelBlend565(int ecx, int edx, int arg1, int arg2);
// 0x0049d950
int __fastcall Fill_Tex8ShadeAlphaLevelBlend555(int ecx, int edx, int arg1, int arg2);
// 0x0049da80
int __fastcall Fill_AlphaShade_565_MMX(int ecx, int edx, int arg1, int arg2);
// 0x0049ddb0
int __fastcall Fill_AlphaShade_555_MMX(int ecx, int edx, int arg1, int arg2);
// 0x0049e140
int __fastcall Raster_SetBlendMasksMMX(int ecx, int edx);
// 0x0049e200
int __fastcall Span_BlendToConst565(int ecx, int edx, int arg1, int arg2);
// 0x0049e300
int __fastcall Span_BlendToConst555(int ecx, int edx, int arg1, int arg2);
// 0x0049ea40
int __fastcall Raster_SetupMMXTexState(int ecx, int edx);
// 0x0049ea80
int __fastcall Fill_Tex16_MMX_NoCpuCheck(int ecx, int edx, int arg1, int arg2);
// 0x0049ec20
int __fastcall Fill_Tex16_LeftToRight_MMX(int ecx, int edx, int arg1, int arg2);
// 0x0048d3e0
int __fastcall Raster_FreeNoiseAndBackBuffer(int ecx, int edx);
// 0x00490780
int __fastcall SwRender_FreeBuffers(int ecx, int edx);
// 0x00490ae0
int __fastcall Span_InsertOccluding(int ecx, int edx, int arg1);
// 0x00491840
int __fastcall Span_ClipVisible(int ecx, int edx, int arg1);
// 0x00491dd0
int __fastcall Span_TestVisible(int ecx, int edx);
// 0x0049e400
int __fastcall Span_BlendToConst565_MMX(int ecx, int edx, int arg1, int arg2);
// 0x0049e560
int __fastcall Span_BlendToConst555_MMX(int ecx, int edx, int arg1, int arg2);
// 0x0048d340
int __fastcall Raster_InitNoiseAndBackBuffer(int ecx, int edx);
// 0x00490520
int __fastcall SwRender_Init(int ecx, int edx);
// 0x00499130
int __fastcall Raster_PickTextureLevel(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00493df0
int __fastcall Raster_FillTexturedPoly(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00494af0
int __fastcall Raster_FillTexturedPolyBlend(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x0048daf0
int __fastcall Raster_RippleDistort(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x0048ed60
int __fastcall Raster_BlendLine(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x00498c40
int __fastcall Raster_ItemCoverageTest(int ecx, int edx);
// 0x00499500
int __fastcall Raster_LineClipped16(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00499930
int __fastcall Raster_SetTextureBlendIndex(int ecx, int edx, int arg1);
// 0x0049b1e0
int __fastcall BlendState_BuildPresetDD0(int ecx, int edx);
// 0x0049b350
int __fastcall BlendState_BuildPresetF10(int ecx, int edx);
// 0x0049b5a0
int __fastcall BlendState_BuildPresetE70(int ecx, int edx);
// 0x0048ec90
int __fastcall Raster_DrawBlendLineList(int ecx, int edx, int arg1);
// 0x00499990
int __fastcall Raster_SetTextureBlendFromRGB(int ecx, int edx, int arg1);
// 0x0049b7e0
int __fastcall Fill_Tex16Keyed_RightToLeft(int ecx, int edx, int arg1, int arg2);
// 0x0049bbf0
int __fastcall Fill_Tex8ShadeKeyed_RightToLeft(int ecx, int edx, int arg1, int arg2);
// 0x0049e6c0
int __fastcall Fill_Tex16_RightToLeft(int ecx, int edx, int arg1, int arg2);
// 0x0049edc0
int __fastcall Fill_Tex8Shade_RightToLeft(int ecx, int edx, int arg1, int arg2);
// 0x0049f180
int __fastcall Fill_Tex8Shaded_RightToLeft(int ecx, int edx, int arg1, int arg2);
// 0x0048ff80
int __fastcall Raster_InstallFillTable(int ecx, int edx);
// 0x00495850
int __fastcall Raster_FillShadedTexturedFan(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x004969d0
int __fastcall Raster_FillTexturedPolySubdiv(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00497ac0
int __fastcall Raster_FillTexturedPolySubdivBlend(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
}  // namespace recoil
