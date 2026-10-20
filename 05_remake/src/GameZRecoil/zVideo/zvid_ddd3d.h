// SUBSYSTEM: texture
// Declarations for src/GameZRecoil/zVideo/zvid_ddd3d.cpp.
#pragma once

namespace recoil {

// 0x004aa900
int __fastcall D3DTextureBinding_ReleaseSurface(int ecx, int edx);
// 0x004aa9d0
int __fastcall D3DTextureBinding_Alloc(int ecx, int edx);
// 0x004aab30
int __fastcall zVideo_ApplyFogColour(int ecx, int edx);
// 0x004acd00
int __fastcall Render_QueueScreenRect(int ecx, int edx, int arg1, int arg2);
// 0x004ad6a0
int __fastcall zVideo_ReportError(int ecx, int edx, int arg1);
// 0x004a9b70
int __fastcall zVideo_FlipFrame(int ecx, int edx, int arg1, int arg2);
// 0x004a9c20
int __fastcall Render_CreateD3DDevice(int ecx, int edx);
// 0x004aab90
int __fastcall Render_QueuePolygon_Untextured(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004aaef0
int __fastcall Render_QueuePolygon_UntexturedGouraud(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004ab320
int __fastcall Render_QueuePolygon_UntexturedLit(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8);
// 0x004ab6d0
int __fastcall Render_QueuePolygon_SingleTextured(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004abb20
int __fastcall Render_QueuePolygon_SingleTexturedLit(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8);
// 0x004ac370
int __fastcall Render_QueuePolygon_SingleTexturedLitGradient(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8);
// 0x004acbd0
int __fastcall Render_DrawPolygonImmediate(int ecx, int edx, int arg1);
// 0x004ace30
int __fastcall Render_FlushSortedDrawQueue(int ecx, int edx);
// 0x004ad250
int __fastcall Render_FlushMainDrawQueue(int ecx, int edx);
// 0x004aa600
int __fastcall D3DTextureBinding_UploadImageToSurface(int ecx, int edx, int arg1);
// 0x004aa0f0
int __fastcall D3DTextureBinding_CreateFromImage(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004a9ac0
int __fastcall Render_BeginScene(int ecx, int edx);
// 0x004a9b40
int __fastcall Render_EndScene(int ecx, int edx);
// 0x004aa8b0
int __fastcall D3DTextureBinding_GetSize(int ecx, int edx, int arg1);
// 0x004aa8f0
int __fastcall D3DTextureBinding_IsReady(int ecx, int edx);
// 0x004aa920
int __fastcall D3DTextureBinding_LoadFrom(int ecx, int edx, int arg1);
// 0x004aaa30
int __fastcall Render_SetFogStart(int ecx, int edx, int arg1);
// 0x004aaa60
int __fastcall Render_SetFogEnd(int ecx, int edx, int arg1);
// 0x004aaa90
int __fastcall Render_SetupFog(int ecx, int edx, int arg1, int arg2, int arg3);
}  // namespace recoil
