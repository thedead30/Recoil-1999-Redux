// SUBSYSTEM: declient
// Declarations for src/GameZRecoil/zDEClient/zdec_qsand.cpp.
#pragma once

namespace recoil {

// 0x004563d0
int __fastcall DEObjectA_Create(int ecx, int edx);
// 0x00456ad0
int __fastcall DEObjectB_Free(int ecx, int edx);
// 0x00456010
int __fastcall DEObjectA_BuildQuicksandDisc(int ecx, int edx);
// 0x00456450
int __fastcall DEObjectA_RebuildClip(int ecx, int edx);
// 0x004564b0
int __fastcall DEObjectA_TesselateQuicksand(int ecx, int edx);
// 0x00455ef0
int __fastcall DEClient_InstanceQuickSand(int ecx, int edx);
}  // namespace recoil
