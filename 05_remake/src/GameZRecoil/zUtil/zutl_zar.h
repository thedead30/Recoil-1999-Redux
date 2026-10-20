// SUBSYSTEM: asset_io
// Declarations for src/GameZRecoil/zUtil/zutl_zar.cpp.
#pragma once

namespace recoil {

// 0x004a5c20
int __fastcall File_Exists(int ecx, int edx);
// 0x004a5c50
int __fastcall File_GetSize(int ecx, int edx);
// 0x004a5f90
int __fastcall WildcardName_InitDigitExpansion(int ecx, int edx);
// 0x004a6070
int __fastcall WildcardName_NextDigitCombination(int ecx, int edx);
// 0x004a6190
int __fastcall ZarArchive_Construct(int ecx, int edx);
// 0x004a6110
int __fastcall File_ReadLengthPrefixedString(int ecx, int edx);
// 0x004a5e50
int __fastcall SearchPath_Resolve(int ecx, int edx);
// 0x004a61b0
int __fastcall ZarArchive_Destruct(int ecx, int edx);
// 0x004a5f50
int __fastcall File_OpenOnSearchPath(int ecx, int edx, int arg1);
// 0x004a61d0
int __fastcall Zar_OpenFile(int ecx, int edx, int arg1);
// 0x004a5e10
int __fastcall List_FreeAllPopped(int ecx, int edx);
// 0x004a5cc0
int __fastcall StringSet_Destroy(int ecx, int edx);
// 0x004a5da0
int __fastcall StringSet_CompareStrings(int ecx, int edx);
// 0x004a5df0
int __fastcall StringSet_DestroyGlobal(int ecx, int edx);
// 0x004a5ce0
int __fastcall StringSet_AddSemicolonTokens(int ecx, int edx);
// 0x004a6100
int __fastcall zUtlShutdown(int ecx, int edx);
// 0x004a5ca0
int __fastcall StringSet_CreateAndAddTokens(int ecx, int edx);
// 0x004a6630
int __fastcall ZarArchive_SeekToEntry(int ecx, int edx, int arg1, int arg2);
}  // namespace recoil
