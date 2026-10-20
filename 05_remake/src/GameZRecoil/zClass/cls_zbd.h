// SUBSYSTEM: zclass_nodes
// Declarations for src/GameZRecoil/zClass/cls_zbd.cpp.
#pragma once

namespace recoil {

// 0x004556a0
int __fastcall GameZ_OpenAndCheckHeader(int ecx, int edx);
// 0x004543f0
int __fastcall GameZ_WriteNodePointerList(int ecx, int edx, int arg1);
// 0x00454bf0
int __fastcall GameZ_ReadNodeIndexList(int ecx, int edx, int arg1);
// 0x004557a0
int __fastcall GameZ_ReloadNodeModel(int ecx, int edx, int arg1, int arg2);
// 0x00455730
int __fastcall GameZ_LoadWorldFile(int ecx, int edx);
// 0x004544b0
int __fastcall GameZ_WriteNodeData(int ecx, int edx);
// 0x00454890
int __fastcall GameZ_WriteNodes(int ecx, int edx);
// 0x00454a50
int __fastcall GameZ_SaveZbd(int ecx, int edx);
// 0x00454c60
int __fastcall GameZ_ReadNodeData(int ecx, int edx);
// 0x00455350
int __fastcall GameZ_ReadNodeBuffer(int ecx, int edx);
// 0x00455520
int __fastcall GameZ_LoadZbd(int ecx, int edx);
}  // namespace recoil
