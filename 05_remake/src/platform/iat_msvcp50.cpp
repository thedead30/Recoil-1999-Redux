// SUBSYSTEM: platform
// Binds Recoil.exe's two MSVCP50.dll imports (slots 0x004cc47c / 0x004cc480, read from its import table). PLATFORM: MSVCP50.dll
// is not part of Windows 11 and the game does not ship it; msvcp60.dll (SysWOW64) exports the same two functions under the same
// decorated names with the same contract - thiscall on a 1-byte std::_Lockit object, lock / unlock one process-wide critical
// section - so the slots bind there (decision D5 binds every other import to the system DLL the same way).
#include "platform/iat_msvcp50.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace recoil {
namespace {
void* sym(const char* name) { return reinterpret_cast<void*>(GetProcAddress(LoadLibraryA("msvcp60.dll"), name)); }
}  // namespace

void* g_Iat___1_Lockit_std__QAE_XZ_004cc47c = sym("??1_Lockit@std@@QAE@XZ");
void* g_Iat___0_Lockit_std__QAE_XZ_004cc480 = sym("??0_Lockit@std@@QAE@XZ");

}  // namespace recoil
