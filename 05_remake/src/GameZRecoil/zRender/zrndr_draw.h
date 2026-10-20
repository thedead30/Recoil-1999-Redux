// SUBSYSTEM: poly_shade
// Declarations for src/GameZRecoil/zRender/zrndr_draw.cpp.
#pragma once

namespace recoil {

// 0x00499a20
int __fastcall Queue_FlatPolygon_SW(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00499ec0
int __fastcall Queue_TexturedPolygon_SW(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8);
// 0x00499c40
int __fastcall Queue_TexturedPolygonSubdiv_SW(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
}  // namespace recoil
