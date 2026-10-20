// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/Animate.cpp.
#pragma once

namespace recoil {

// 0x00453b10
int __fastcall Anim_Destroy(int ecx, int edx);
// 0x00453b40
int __fastcall Anim_AttachChild(int ecx, int edx);
// 0x00453b80
int __fastcall Anim_DetachChild(int ecx, int edx);
// 0x00453c90
int __fastcall Anim_AdvanceClock(int ecx, int edx, int arg1);
// 0x00453bd0
int __fastcall Anim_UpdateNode(int ecx, int edx);
}  // namespace recoil
