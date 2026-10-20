// SUBSYSTEM: render_frame
// Declarations for src/GameZRecoil/zModel/gmod_init_render_frame.cpp.
#pragma once

namespace recoil {

// 0x00476020
int __fastcall Render_SetVertexShading(int ecx, int edx);
// 0x00476030
int __fastcall RenderState_SetSecondArrayBlendEnabled(int ecx, int edx);
// 0x00476040
int __fastcall RenderState_SetColourAndValue(int ecx, int edx, int arg1);
// 0x00476070
int __fastcall RenderState_SetFade(int ecx, int edx, int arg1);
// 0x00476080
int __fastcall RenderState_SetMode(int ecx, int edx);
// 0x00476090
int __fastcall Render_SetPerspectiveTexDeltas(int ecx, int edx, int arg1, int arg2);
// 0x004760b0
int __fastcall Render_SetInverseZTolerances(int ecx, int edx, int arg1, int arg2);
// 0x004760d0
int __fastcall Model_SetTextureScroll(int ecx, int edx, int arg1, int arg2);
// 0x00476120
int __fastcall RenderInset_SetDestinationRect(int ecx, int edx);
}  // namespace recoil
