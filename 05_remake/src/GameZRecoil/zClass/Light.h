// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/Light.cpp.
#pragma once

namespace recoil {

// 0x00453110
int __fastcall Light_Destroy(int ecx, int edx);
// 0x004531c0
int __fastcall Light_DetachChild(int ecx, int edx);
// 0x00453200
int __fastcall Light_SetFieldA8(int ecx, int edx, int arg1);
// 0x00453250
int __fastcall Light_SetFieldA4(int ecx, int edx, int arg1);
// 0x004532a0
int __fastcall Light_SetFieldB8(int ecx, int edx);
// 0x004532f0
int __fastcall Light_SetModeBC(int ecx, int edx);
// 0x00453350
int __fastcall Light_SetModeC0(int ecx, int edx);
// 0x004533b0
int __fastcall Light_SetFieldC4(int ecx, int edx);
// 0x00453400
int __fastcall Light_SetFalloffRange(int ecx, int edx, int arg1, int arg2);
// 0x00453500
int __fastcall Light_GetFalloffRange(int ecx, int edx, int arg1);
// 0x00453560
int __fastcall Light_SetVec14(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004535c0
int __fastcall Light_SetVec08(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00453a40
int __fastcall Light_GetColor(int ecx, int edx, int arg1, int arg2);
// 0x00453aa0
int __fastcall Light_SetColor(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00453620
int __fastcall Camera_UpdateFromAttachedNode(int ecx, int edx);
// 0x00453880
int __fastcall Light_TransformToViewSpace(int ecx, int edx);
// 0x00452fd0
int __fastcall Light_Create(int ecx, int edx);
}  // namespace recoil
