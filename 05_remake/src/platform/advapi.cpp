// SUBSYSTEM: platform
// Binds the ADVAPI32.dll imports of Recoil.exe to the system advapi32.dll (see advapi.h). PLATFORM.
#include "platform/advapi.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace recoil {
namespace {
void* sym(const char* name) { return reinterpret_cast<void*>(GetProcAddress(LoadLibraryA("advapi32.dll"), name)); }
}  // namespace

void* g_Iat_GetUserNameA_004cc000 = sym("GetUserNameA");
void* g_Iat_RegCloseKey_004cc004 = sym("RegCloseKey");
void* g_Iat_RegCreateKeyExA_004cc008 = sym("RegCreateKeyExA");
void* g_Iat_RegOpenKeyExA_004cc00c = sym("RegOpenKeyExA");
void* g_Iat_RegQueryValueExA_004cc010 = sym("RegQueryValueExA");
void* g_Iat_RegSetValueExA_004cc014 = sym("RegSetValueExA");

}  // namespace recoil
