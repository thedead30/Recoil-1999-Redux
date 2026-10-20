// SUBSYSTEM: poly_shade
// Declarations for src/unattributed/poly_shade.cpp.
#pragma once

namespace recoil {

// 0x0047a200
int __fastcall Poly_ClipNearZ(int ecx, int edx);
// 0x0047a4e0
int __fastcall Poly_ClipNearZ_3Attr(int ecx, int edx);
// 0x0047aa80
int __fastcall Poly_ClipNearZ_UV(int ecx, int edx);
// 0x0047af60
int __fastcall Poly_ClipNearZ_UV_1Attr(int ecx, int edx);
// 0x0047b540
int __fastcall Poly_ClipScreenEdges(int ecx, int edx);
// 0x0047bd30
int __fastcall Poly_ClipScreenEdges_3Attr(int ecx, int edx);
// 0x0047cdc0
int __fastcall Poly_ClipScreenEdges_XY(int ecx, int edx);
// 0x0047d3f0
int __fastcall Poly_ClipScreenEdges_UV(int ecx, int edx);
// 0x0047dfb0
int __fastcall Poly_ClipScreenEdges_XY_1Attr(int ecx, int edx);
// 0x0047e900
int __fastcall Poly_ClipNearZ_UV_3Attr(int ecx, int edx);
// 0x0047efd0
int __fastcall Poly_ClipScreenEdges_UV_3Attr(int ecx, int edx);
// 0x00490330
int __fastcall Store255f(int ecx, int edx);
// 0x004894f0
int __fastcall Shade_LinearFalloff(int ecx, int edx, int arg1);
// 0x00479020
int __fastcall Shade_EmitPointSprite(int ecx, int edx, int arg1);
// 0x00487f10
int __fastcall Shade_LightPolygonVertices_HW(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00488d60
int __fastcall Shade_LightPolygon_SW(int ecx, int edx, int arg1, int arg2);
// 0x004896d0
int __fastcall Shade_DistanceFadePerVertex_SW(int ecx, int edx);
// 0x00489920
int __fastcall Shade_DistanceFade(int ecx, int edx);
// 0x00489a90
int __fastcall Shade_DistanceFadePerVertex(int ecx, int edx);
// 0x00476a50
int __fastcall Shade_SetupObjectLighting(int ecx, int edx, int arg1, int arg2);
// 0x00477b30
int __fastcall Render_ShadeAndQueueObjectPolygons(int ecx, int edx);
// 0x00476cf0
int __fastcall Render_ShadeAndQueueObjectPolygons_SW(int ecx, int edx);
}  // namespace recoil
