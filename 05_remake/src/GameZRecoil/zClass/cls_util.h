// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/cls_util.cpp.
#pragma once

namespace recoil {

// 0x004518b0
int __fastcall ClassDb_SetNodeArraySize(int ecx, int edx);
// 0x004518f0
int __fastcall SceneGraph_GetRoot(int ecx, int edx);
// 0x00451b20
int __fastcall ZClassNode_CloneModelIfSpecial(int ecx, int edx);
// 0x004520c0
int __fastcall ClassNode_CopyLight_Unsupported(int ecx, int edx);
// 0x004520e0
int __fastcall ClassNode_CopySound_Unsupported(int ecx, int edx);
// 0x00452230
int __fastcall ClassNode_CopyAnimate_Unsupported(int ecx, int edx);
// 0x004523c0
int __fastcall ClassNode_CopySequence_Unsupported(int ecx, int edx);
// 0x004523e0
int __fastcall ClassNode_CopySwitch_Unsupported(int ecx, int edx);
// 0x00451900
int __fastcall World_AllocObjectTable(int ecx, int edx);
// 0x00451a60
int __fastcall gwNode_DestroyTree(int ecx, int edx);
// 0x00451bd0
int __fastcall ZClassNode_CopyProperties(int ecx, int edx);
// 0x00451a00
int __fastcall World_ClearAll(int ecx, int edx);
// 0x004518e0
int __fastcall gClsShutdown(int ecx, int edx);
// 0x00451f70
int __fastcall ClassNode_CopyCamera(int ecx, int edx);
// 0x00452100
int __fastcall ClassNode_CopyObject3D(int ecx, int edx);
// 0x00452250
int __fastcall ClassNode_CopyLOD(int ecx, int edx);
// 0x00452400
int __fastcall ClassNode_CopyDispatch(int ecx, int edx);
// 0x00452500
int __fastcall gwNode_CloneWithFlag(int ecx, int edx, int arg1);
// 0x00452560
int __fastcall Node_FindWithSearchParams(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
