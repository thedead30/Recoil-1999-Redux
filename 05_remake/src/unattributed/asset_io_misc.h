// SUBSYSTEM: asset_io
// Declarations for src/unattributed/asset_io_misc.cpp.
#pragma once

namespace recoil {

// 0x00407190
int __fastcall NameTable_LookupValue(int ecx, int edx);
// 0x004815a0
int __fastcall Model_FromIndex(int ecx, int edx);
// 0x004a62f0
int __fastcall Zar_GrowTocBuffer(int ecx, int edx, int arg1);
// 0x004a6330
int __fastcall ZarIndex_Free(int ecx, int edx);
// 0x004a6360
int __fastcall Zar_WriteFooterAndClose(int ecx, int edx);
// 0x004a62b0
int __fastcall ZarArchive_Close(int ecx, int edx);
// 0x004a63f0
int __fastcall Zar_ParseFooterAndTOC(int ecx, int edx);
// 0x004a64d0
int __fastcall Zar_WriteEntry(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004a5f20
int __fastcall SearchPath_EntryLacksFile(int ecx, int edx);
// 0x004071f0
int __fastcall ConfigValue_ToInt(int ecx, int edx);
// 0x004791c0
int __fastcall Model_ScrollUVs(int ecx, int edx, int arg1, int arg2);
// 0x004cb9b0
int __fastcall EH_Unwind_ZbdResource_Construct_0(int ecx, int edx);
// 0x004cb9be
int __fastcall EH_Handler_ZbdResource_Construct(int ecx, int edx);
// 0x004c0d20
int __fastcall ZbdResource_Construct(int ecx, int edx, int a1, int a2);
}  // namespace recoil
