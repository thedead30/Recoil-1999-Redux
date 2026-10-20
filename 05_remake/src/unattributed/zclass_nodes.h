// SUBSYSTEM: zclass_nodes
// Declarations for src/unattributed/zclass_nodes.cpp.
#pragma once

namespace recoil {

// 0x0044f690
int __fastcall NodeList_FindNext(int ecx, int edx, int arg1);
// 0x004543d0
int __fastcall ClsRecord_FromIndex(int ecx, int edx);
// 0x004c59e0
int __fastcall ZClassNode_UpdateModelOncePerFrame(int ecx, int edx);
// 0x00453d20
int __fastcall Anim_InterpolateKeys(int ecx, int edx);
// 0x00452ec0
int __fastcall gwSoundNodeComputeWorldPosition(int ecx, int edx);
// 0x0044f720
int __fastcall ZClass_NameHasCurrentPrefix_0044f720(int ecx, int edx);
// 0x0044f750
int __fastcall ZClass_NameEqualsCurrent_0044f750(int ecx, int edx);
// 0x00451840
int __fastcall ZClass_Slot_ApplyNodeParams_00451840(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044f6f0
int __fastcall NodeList_FindNextByName(int ecx, int edx);
// 0x0044f740
int __fastcall NodeList_FindNextWith0044f750(int ecx, int edx);
}  // namespace recoil
