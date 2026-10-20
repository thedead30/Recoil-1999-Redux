// SUBSYSTEM: view
// View/projection state (original globals). Only the data is defined here so far: the writers
// (projection setup 0x00474400 and the other view members) are ported in phase P4; transform reads this
// state now. Spec: 04_spec/systems/view.md, globals 04_spec/globals/view.md
#pragma once

namespace recoil {

// 0x00566838..0x00566864: projection block, 12 floats written by 0x00474400 (view.md). Words 10 and 11
// (0x00566860 / 0x00566864) are the screen-to-view scales kx, ky read by 0x00474bc0.
extern float g_ViewParams_00566838[12];
// 0x005761e0 / 0x005761e4: screen centre (p3 + p1, p4 + p2), measured (320, 200) in Recoil18.
extern float g_ScreenCentre_005761e0[2];
// 0x005669d8..0x005669e0: default angles used by 0x00473060 when the parent level is the identity.
extern float g_DefaultAngles_005669d8[3];

// 0x004083d0
int __fastcall Rect_SetOrigin(int ecx, int edx, int arg1);
// 0x00408400
int __fastcall Rect_SetSize(int ecx, int edx, int arg1);
// 0x00408570
int __fastcall Viewport0_AttachView(int ecx, int edx);
// 0x004085b0
int __fastcall Viewport1_AttachView(int ecx, int edx);
// 0x00408500
int __fastcall Viewport0_SetSizeF(int ecx, int edx);
// 0x00408530
int __fastcall Viewport0_SetOrigin(int ecx, int edx);
// 0x004085e0
int __fastcall Viewport1_SetOrigin(int ecx, int edx);
// 0x00408620
int __fastcall Viewport1_SetSize(int ecx, int edx);
// 0x00408480
int __fastcall Viewport_SetCamera(int ecx, int edx);
// 0x00472ed0
int __fastcall View_GetProjectionScale(int ecx, int edx);
// 0x00473e60
int __fastcall View_SetCameraMatrix(int ecx, int edx);
// 0x00473fc0
int __fastcall View_TransformPointsCameraY(int ecx, int edx, int arg1);
// 0x004743e0
int __fastcall View_SetFieldOfView(int ecx, int edx, int arg1, int arg2);
// 0x00474400
int __fastcall View_SetProjection(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8);
// 0x00474b20
int __fastcall View_ProjectPointsInvZ(int ecx, int edx, int arg1);
// 0x00474b70
int __fastcall View_ProjectPoints(int ecx, int edx, int arg1);
// 0x00474fc0
int __fastcall View_ExpFalloffLookup(int ecx, int edx, int arg1);
// 0x00479ce0
int __fastcall View_SetupFromCamera(int ecx, int edx);
// 0x004753e0
int __fastcall Tri_SetupTextureGradients(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
}  // namespace recoil
