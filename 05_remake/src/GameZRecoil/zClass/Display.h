// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/Display.cpp.
#pragma once

namespace recoil {

// 0x0044fe50
int __fastcall Display_DetachChild(int ecx, int edx);
// 0x0044fe90
int __fastcall DisplayClass_SetSize(int ecx, int edx, int arg1);
// 0x0044ff10
int __fastcall DisplayClass_SetOrigin(int ecx, int edx, int arg1);
// 0x0044ff90
int __fastcall Display_SetColour(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0044fdd0
int __fastcall DisplayClass_Create(int ecx, int edx);
}  // namespace recoil
