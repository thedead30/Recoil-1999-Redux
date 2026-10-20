// SUBSYSTEM: settings
// Declarations for src/unattributed/settings.cpp. Spec: 04_spec/systems/settings.md
#pragma once

#include <cstdint>

namespace recoil {

// 0x004e5d00..0x004e5dd0: the settings block - mostly pointers to the value cells of registered settings nodes,
// filled by RecoilApp_LoadUserSettings 0x00407700 (52 dwords; .bss, zero at load).
constexpr int kSettingsBlockWords = 0x34;  // CONFIRMED-DATA: 0x004e5dd0 - 0x004e5d00 = 0xD0 bytes, the span the functions touch
extern std::uint32_t g_SettingsBlock_004e5d00[kSettingsBlockWords];

// 0x0056bcd0..0x0056bce4: settings-node list state (list head, initialised flag, three owned strings).
constexpr int kSettingsNodeStateWords = 6;  // CONFIRMED-DATA: 0x0056bcd0..0x0056bce4 inclusive, the words the functions touch
extern std::uint32_t g_SettingsNodeState_0056bcd0[kSettingsNodeStateWords];

// Register/stack shape from each listing (ECX, EDX, stack arguments); meanings in the spec lines in settings.cpp.
// 0x00407220
int __fastcall Preset_CompareOp(int ecx, int edx, int arg1);
// 0x004076f0
int __fastcall Stub_Ret(int ecx, int edx);
// 0x00407e20
int __fastcall Settings_StoreGameCtlOptions(int ecx, int edx);
// 0x00407f10
int __fastcall Settings_StoreGameIntensity(int ecx, int edx);
// 0x00407f20
int __fastcall Settings_GetGameIntensity(int ecx, int edx);
// 0x004080b0
int __fastcall RecoilApp_GetSoundAPICheckboxValue(int ecx, int edx);
// 0x00408230
int __fastcall Settings_SetNetworkFlag(int ecx, int edx);
// 0x00408240
int __fastcall Settings_StoreNetworkModem(int ecx, int edx);
// 0x00408250
int __fastcall Settings_StoreNetListen(int ecx, int edx);
// 0x00408280
int __fastcall Settings_ApplyHWCardFlag(int ecx, int edx);
// 0x00408290
int __fastcall Settings_StoreHWAPI(int ecx, int edx);
// 0x004082a0
int __fastcall Settings_StoreFullScreen(int ecx, int edx);
// 0x004082b0
int __fastcall Settings_StoreHUDFlag(int ecx, int edx);
// 0x00408300
int __fastcall Settings_Store_004e5d6c(int ecx, int edx);
// 0x00408320
int __fastcall Settings_GetHWAPI(int ecx, int edx);
// 0x00408330
int __fastcall Settings_GetFullScreen(int ecx, int edx);
// 0x00408340
int __fastcall Settings_GetSplitScreenValue(int ecx, int edx);
// 0x004083a0
int __fastcall Settings_StoreJoystickNumAxes(int ecx, int edx);
// 0x004083b0
int __fastcall Settings_StoreJoystickNumButtons(int ecx, int edx);
// 0x00408660
int __fastcall Settings_GetDisplayRect_10(int ecx, int edx);
// 0x00408670
int __fastcall Settings_GetDisplayRect_14(int ecx, int edx);
// 0x00408680
int __fastcall Settings_SetDisplayRect_20(int ecx, int edx);
// 0x00408690
int __fastcall Settings_GetDisplayRect_20(int ecx, int edx);
// 0x004086a0
int __fastcall Settings_Get_004e5d70(int ecx, int edx);
// 0x004086c0
int __fastcall Settings_GetValue_004e5d88(int ecx, int edx);
// 0x004086d0
int __fastcall Settings_GetScreenRect_14(int ecx, int edx);
// 0x00408a10
int __fastcall Settings_StoreWOLPasswordFlag(int ecx, int edx);
// 0x00408a20
int __fastcall Settings_GetWOLPasswordFlag(int ecx, int edx);

// 0x004b3380
int __fastcall Settings_FindNodeByName(int ecx, int edx);
// 0x004b2e80
int __fastcall Settings_RegisterNode(int ecx, int edx, int arg1, int arg2);
// 0x004b32c0
int __fastcall Settings_Shutdown(int ecx, int edx);
// 0x00408120
int __fastcall Settings_StorePlayerName(int ecx, int edx);
// 0x004e4668 / 0x004e466c registry path pieces
extern unsigned char g_SettingsRegText_004e466c[16];
extern unsigned char* g_SettingsRegRoot_004e4668;
// 0x004b2960 Settings_LoadFromRegistry -> 1 when HKCU and HKLM keys opened (values read into the registered nodes), else 0
int __fastcall Settings_LoadFromRegistry(int ecx, int edx);
// 0x004b2bf0 Settings_SaveToRegistry -> 1 when both keys were created/opened and the values written, else 0
int __fastcall Settings_SaveToRegistry(int ecx, int edx);
// 0x00407470
int __fastcall Preset_EvaluateCondition(int ecx, int edx);
// 0x00407680
int __fastcall Settings_GetHardwarePresetOrDefault(int ecx, int edx, int arg1);
// 0x004080a0
int __fastcall RecoilApp_ApplySoundAPISetting(int ecx, int edx);
// 0x004086e0
int __fastcall Settings_ScreenRect_SetSize(int ecx, int edx);
// 0x00408700
int __fastcall Settings_ScreenRect_SetOrigin(int ecx, int edx);
// 0x00408720
int __fastcall Settings_ApplyVideoModePreset(int ecx, int edx);
// 0x00407e00
int __fastcall Settings_ResetNetworkState(int ecx, int edx);
// 0x00407700
int __fastcall RecoilApp_LoadUserSettings(int ecx, int edx);
// 0x004b3260
int __fastcall Settings_InitPathsAndAutoLoad(int ecx, int edx, int arg1);
}  // namespace recoil
