// SUBSYSTEM: platform
// Recoil.exe's two MSVCP50.dll imports: std::_Lockit's constructor and destructor (the STL's process-wide lock). PLATFORM
// (host DLL binding, see iat_msvcp50.cpp for why they bind to msvcp60.dll).
#pragma once

namespace recoil {

extern void* g_Iat___1_Lockit_std__QAE_XZ_004cc47c;  // ??1_Lockit@std@@QAE@XZ  std::_Lockit::~_Lockit()  (thiscall)
extern void* g_Iat___0_Lockit_std__QAE_XZ_004cc480;  // ??0_Lockit@std@@QAE@XZ  std::_Lockit::_Lockit()   (thiscall)

}  // namespace recoil
