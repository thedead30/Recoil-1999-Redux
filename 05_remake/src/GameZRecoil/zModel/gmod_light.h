// SUBSYSTEM: render_frame
// Declarations for src/GameZRecoil/zModel/gmod_light.cpp.
#pragma once

namespace recoil {

// 0x00487900
int __fastcall Clip_BuildFootprintEdges(int ecx, int edx);
// 0x004879c0
int __fastcall Clip_PointInFootprint(int ecx, int edx, int arg1);
// 0x00487a30
int __fastcall Light_SelectActive(int ecx, int edx, int arg1);
// 0x00487c50
int __fastcall Light_ComputeObjectInfluence(int ecx, int edx, int arg1);
}  // namespace recoil
