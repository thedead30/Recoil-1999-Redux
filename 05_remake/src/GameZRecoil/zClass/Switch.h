// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/Switch.cpp.
#pragma once

namespace recoil {

// 0x00452920
int __fastcall Switch_AttachChild(int ecx, int edx);
// 0x00452970
int __fastcall Switch_DetachChild(int ecx, int edx);
// 0x00452770
int __fastcall Node_FindChildByNameRecursive(int ecx, int edx);
// 0x00452810
int __fastcall gwNode_VisitTreeUntil1(int ecx, int edx);
// 0x00452860
int __fastcall Node_UpdateLightsRecursive(int ecx, int edx);
// 0x004528b0
int __fastcall ClassNode_ReleaseRecursive(int ecx, int edx);
// 0x004528e0
int __fastcall ClassNode_ReleaseModelsRecursive(int ecx, int edx);
// 0x004528a0
int __fastcall ClassNode_Release(int ecx, int edx);
// 0x004527f0
int __fastcall gwNode_IsClassType1Visible(int ecx, int edx);
}  // namespace recoil
