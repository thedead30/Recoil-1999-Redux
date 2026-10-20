// SUBSYSTEM: avi
// Declarations for src/GameZRecoil/zFMV/fmv_stream.cpp.
#pragma once

namespace recoil {

// 0x00463ef0
int __fastcall AviPlayer_OpenVideoStream(int ecx, int edx);
// 0x004641a0
int __fastcall AviPlayer_OpenSoundStream(int ecx, int edx);
// 0x004643a0
int __fastcall AviPlayer_DecodeFrame(int ecx, int edx, int arg1);
// 0x00464540
int __fastcall AviPlayer_FillSoundChunk(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
