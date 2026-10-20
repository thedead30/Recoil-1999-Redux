// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/Seq.cpp.
#pragma once

namespace recoil {

// 0x00453ee0
int __fastcall Seq_Create(int ecx, int edx);
// 0x00453f40
int __fastcall Seq_InsertEntry(int ecx, int edx, int arg1, int arg2);
// 0x00454000
int __fastcall Seq_DetachChild(int ecx, int edx);
// 0x004540c0
int __fastcall Seq_SetField0(int ecx, int edx);
// 0x00454100
int __fastcall Seq_SetField4(int ecx, int edx);
// 0x00454140
int __fastcall Seq_SetField8(int ecx, int edx);
// 0x00454180
int __fastcall Seq_SetFieldC(int ecx, int edx);
// 0x004541c0
int __fastcall Seq_AdvanceKeyframes(int ecx, int edx);
// 0x004542a0
int __fastcall NodeType6_Create(int ecx, int edx);
// 0x00454320
int __fastcall Class6_DetachChild(int ecx, int edx);
// 0x00454330
int __fastcall Node_SetClassDataField0_NoCheck(int ecx, int edx);
// 0x00454340
int __fastcall Node_SetClassDataRangeCheck(int ecx, int edx, int arg1);
// 0x00454360
int __fastcall GameZ_ClearWorldPath(int ecx, int edx);
// 0x00454370
int __fastcall ClsRecord_ToIndex(int ecx, int edx);
// 0x004543a0
int __fastcall ClsRecord_ActiveIndex(int ecx, int edx);
}  // namespace recoil
