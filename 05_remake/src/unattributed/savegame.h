// SUBSYSTEM: savegame
// Declarations for src/unattributed/savegame.cpp.
#pragma once

namespace recoil {

// 0x004a65d0
int __fastcall ZarArchive_FindEntry(int ecx, int edx, int arg1);
// 0x00403db0
int __fastcall List_DestroyNodes(int ecx, int edx);
// 0x004c0630
int __fastcall SaveGame_WriteNamedEntry(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c0010
int __fastcall SaveGame_WriteRecord(int ecx, int edx, int arg1, int arg2);
// 0x004a6270
int __fastcall ZarWriter_Create(int ecx, int edx, int arg1);
// 0x004a6670
int __fastcall ZarArchive_ReadEntry(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004c0260
int __fastcall SaveHandler_Less(int ecx, int edx);
// 0x004c0620
int __fastcall SaveGame_SetDirty30(int ecx, int edx);
// 0x004c06a0
int __fastcall SaveHandler_CallSave(int ecx, int edx, int arg1);
// 0x004c06c0
int __fastcall SaveHandler_CallLoad(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c0700
int __fastcall SaveGame_FlushTempFileToEntry(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004c0780
int __fastcall TempFile_FromBuffer(int ecx, int edx, int arg1, int arg2);
// 0x004c07c0
int __fastcall TempFile_RemoveAll(int ecx, int edx, int arg1);
// 0x004c0b60
int __fastcall List_Begin(int ecx, int edx, int arg1);
// 0x004c0ba0
int __fastcall List_Swap(int ecx, int edx, int arg1);
// 0x004c0ce0
int __fastcall List_Splice(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c0070
int __fastcall SaveGame_FinishIfActive(int ecx, int edx);
// 0x004c00a0
int __fastcall SaveGame_CloseTempRecord(int ecx, int edx, int arg1);
// 0x004c00c0
int __fastcall SaveSystem_WriteToTempFile(int ecx, int edx);
// 0x004c00e0
int __fastcall SaveSystem_RemoveTempFiles(int ecx, int edx);
// 0x004c0bd0
int __fastcall List_MergeByLess(int ecx, int edx, int arg1);
// 0x004c0080
int __fastcall SaveGame_OpenTempRecord(int ecx, int edx);
// 0x004c0400
int __fastcall SaveManager_Read(int ecx, int edx, int arg1);
// 0x004c0050
int __fastcall SaveGame_LoadFromFile(int ecx, int edx);
// 0x004c0b70
int __fastcall List_CtorEmptyWithSentinel_004c0b70(int ecx, int edx);
// 0x004c0030
int __fastcall SaveGame_SaveToFile(int ecx, int edx);
// 0x004c0100
int __fastcall SaveSystem_Create(int ecx, int edx);
// 0x004c0180
int __fastcall SaveSystem_Destroy(int ecx, int edx);
// 0x004c01b0
int __fastcall SaveSystem_Dtor(int ecx, int edx);
// 0x004c0370
int __fastcall SaveManager_Write(int ecx, int edx, int arg1);
// 0x004c07d0
int __fastcall SaveHandlerList_Sort(int ecx, int edx);
// 0x004cb940
int __fastcall EH_Unwind_SaveSystem_Create_0(int ecx, int edx);
// 0x004cb94b
int __fastcall EH_Unwind_SaveSystem_Create_1(int ecx, int edx);
// 0x004cb960
int __fastcall EH_Unwind_SaveSystem_Dtor_0(int ecx, int edx);
// 0x004cb980
int __fastcall EH_Unwind_SaveHandlerList_Sort_0(int ecx, int edx);
// 0x004cb98b
int __fastcall EH_Unwind_SaveHandlerList_Sort_1(int ecx, int edx);
// 0x004cb953
int __fastcall EH_Handler_SaveSystem_Create(int ecx, int edx);

// 0x004cb968
int __fastcall EH_Handler_SaveSystem_Dtor(int ecx, int edx);

// 0x004cb9a1
int __fastcall EH_Handler_SaveHandlerList_Sort(int ecx, int edx);

}  // namespace recoil
