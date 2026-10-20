// SUBSYSTEM: asset_io
// Declarations for src/GameZRecoil/zModel/gmod_const.cpp.
#pragma once

namespace recoil {

// 0x00481bc0
int __fastcall Model_ReadThreeDwords(int ecx, int edx, int arg1, int arg2);
// 0x00482080
int __fastcall Model_Alloc(int ecx, int edx);
// 0x00482160
int __fastcall Model_ReleaseContents(int ecx, int edx);
// 0x004826a0
int __fastcall SetDword0_FromEDX(int ecx, int edx);
// 0x004826b0
int __fastcall Record_SetFlagBit1(int ecx, int edx);
// 0x004826d0
int __fastcall Record_SetFlagBit0(int ecx, int edx);
// 0x004826f0
int __fastcall Model_AddRef(int ecx, int edx);
// 0x00482700
int __fastcall Model_Release(int ecx, int edx);
// 0x00482710
int __fastcall GetField8(int ecx, int edx);
// 0x00482c60
int __fastcall Vector_UnitCross(int ecx, int edx, int arg1, int arg2);
// 0x00482e30
int __fastcall Polygon_NewellPlane(int ecx, int edx, int arg1);
// 0x00483b80
int __fastcall Model_ComputeBounds(int ecx, int edx);
// 0x00483f80
int __fastcall Model_BuildVertexWeightTable(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00484140
int __fastcall ModelInstance_SetAllPartsField4(int ecx, int edx);
// 0x00484170
int __fastcall ModelInstance_SetAllPartsBit8(int ecx, int edx);
// 0x00484230
int __fastcall ModelInstance_ResetAnimFrame(int ecx, int edx);
// 0x004842b0
int __fastcall ModelInstance_SetCycleFrame(int ecx, int edx);
// 0x00484350
int __fastcall ModelInstance_ResetNonCycleParts(int ecx, int edx);
// 0x00484860
int __fastcall Triangle_SolveGradient2D(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9);
// 0x004820f0
int __fastcall Model_Free(int ecx, int edx);
// 0x00482b40
int __fastcall Polygon_RemoveCollinearVertices(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00482db0
int __fastcall Polygon_IsPlanar(int ecx, int edx);
// 0x00483a60
int __fastcall Model_NeedsSpecialDraw(int ecx, int edx);
// 0x00483e60
int __fastcall Model_ComputeSymmetricBounds(int ecx, int edx);
// 0x004841b0
int __fastcall ModelInstance_SetCyclePartsFlag0x200(int ecx, int edx);
// 0x004842f0
int __fastcall ModelInstance_Call_00481220(int ecx, int edx);
// 0x00484310
int __fastcall ModelInstance_Call_00481260(int ecx, int edx, int arg1);
// 0x00484330
int __fastcall ModelInstance_Call_00481100(int ecx, int edx);
// 0x004843b0
int __fastcall Polygon_FixupUVs(int ecx, int edx);
// 0x004815c0
int __fastcall Model_WriteSection(int ecx, int edx);
// 0x00483ad0
int __fastcall Model_ComputeBoundsCentreRadius(int ecx, int edx);
// 0x00484250
int __fastcall ModelInstance_Call481050(int ecx, int edx);
// 0x00483510
int __fastcall UV_QuantizeAndRebase(int ecx, int edx);
// 0x00482270
int __fastcall Model_Clone(int ecx, int edx, int arg1);
// 0x00482720
int __fastcall Model_AddVertexDedup(int ecx, int edx);
// 0x00482860
int __fastcall Model_AddVertexWithOffset(int ecx, int edx, int arg1);
// 0x00482a10
int __fastcall Model_AddUniqueNormal(int ecx, int edx);
// 0x00482fe0
int __fastcall Polygon_FanTriangulate(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10);
// 0x00483240
int __fastcall Polygon_SplitFan(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);
// 0x00483650
int __fastcall Model_AddPolygon(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10);
// 0x00483610
int __fastcall Model_Call483650_Arg2Zero(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9);
// 0x00481c50
int __fastcall Model_ReadContents(int ecx, int edx);
// 0x00481aa0
int __fastcall Model_ReadOne(int ecx, int edx);
// 0x00481fa0
int __fastcall Model_ReadSection(int ecx, int edx);
// 0x004841f0
int __fastcall ModelInstance_Call480f80OnCycleParts(int ecx, int edx);
}  // namespace recoil
