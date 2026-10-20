// SUBSYSTEM: object3d
// Declarations for src/GameZRecoil/zClass/Object3d.cpp.
#pragma once

namespace recoil {

// 0x0044dbb0
int __fastcall Object3D_SetFlag4(int ecx, int edx);
// 0x0044dc30
int __fastcall Object3D_SetClampedValueAndColour(int ecx, int edx, int arg1);
// 0x0044dd90
int __fastcall Object3D_SetValue4(int ecx, int edx, int arg1);
// 0x0044de10
int __fastcall Object3D_GetValue4(int ecx, int edx);
// 0x0044de80
int __fastcall Object3D_SetFlag2(int ecx, int edx);
// 0x0044dfd0
int __fastcall Object3D_GetScale(int ecx, int edx, int arg1, int arg2);
// 0x0044e110
int __fastcall Object3D_GetRotation(int ecx, int edx, int arg1, int arg2);
// 0x0044e270
int __fastcall Object3D_GetPosition(int ecx, int edx, int arg1, int arg2);
// 0x0044e5b0
int __fastcall Object3D_GetLocalMatrix(int ecx, int edx);
// 0x0044d990
int __fastcall gwNodeMarkSubtreeDirty(int ecx, int edx);
// 0x0044d9e0
int __fastcall Object3D_ResetTransform(int ecx, int edx);
// 0x0044df00
int __fastcall Object3D_SetScale(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044e030
int __fastcall Object3D_SetRotation(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044e170
int __fastcall Object3D_AddRotation(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044e300
int __fastcall Object3D_SetPosition(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044e3d0
int __fastcall Object3D_AddPosition(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044e4f0
int __fastcall Object3D_SetLocalMatrix(int ecx, int edx);
// 0x0044db10
int __fastcall Object3D_AttachChild(int ecx, int edx);
// 0x0044db60
int __fastcall Object3D_DetachChild(int ecx, int edx);
// 0x0044daa0
int __fastcall Object3D_Create(int ecx, int edx);
}  // namespace recoil
