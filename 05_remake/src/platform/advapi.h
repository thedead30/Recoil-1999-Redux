// SUBSYSTEM: platform
// The ADVAPI32.dll imports of Recoil.exe (import table 0x004cc000..0x004cc014). The original keeps its settings in
// the Windows registry (Settings_LoadFromRegistry 0x004b2960 / Settings_SaveToRegistry 0x004b2bf0); the remake
// calls the same system functions, bound here with GetProcAddress. PLATFORM.
#pragma once

namespace recoil {

// Import-address-table slots for instruction-level ports, named after the original slot address.
extern void* g_Iat_GetUserNameA_004cc000;
extern void* g_Iat_RegCloseKey_004cc004;
extern void* g_Iat_RegCreateKeyExA_004cc008;
extern void* g_Iat_RegOpenKeyExA_004cc00c;
extern void* g_Iat_RegQueryValueExA_004cc010;
extern void* g_Iat_RegSetValueExA_004cc014;

}  // namespace recoil
