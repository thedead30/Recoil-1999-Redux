// SUBSYSTEM: asset_io
// Declarations for src/GameZRecoil/zReader/zreader.cpp.
#pragma once

namespace recoil {

// 0x0048c950
int __fastcall Container_CreateList(int ecx, int edx);
// 0x0048c9a0
int __fastcall Container_LinkNodePair(int ecx, int edx, int arg1);
// 0x0048cb00
int __fastcall Container_FindNodeByPayload(int ecx, int edx);
// 0x0048cb30
int __fastcall Container_GetNth(int ecx, int edx);
// 0x0048cbd0
int __fastcall Container_FindFirstByPredicate(int ecx, int edx, int arg1);
// 0x0048cc20
int __fastcall CircularList_FindByKey(int ecx, int edx);
// 0x0048cc60
int __fastcall Container_Count(int ecx, int edx);
// 0x0048cda0
int __fastcall ConfigTree_AllocChildArray(int ecx, int edx);
// 0x0048c820
int __fastcall Container_NodeFree_ToFreeList(int ecx, int edx);
// 0x0048cc50
int __fastcall Container_FindFirstByPredicateThunk(int ecx, int edx, int arg1);
// 0x0048c800
int __fastcall Container_GrowFreeList(int ecx, int edx);
// 0x0048cae0
int __fastcall Container_FreeNode(int ecx, int edx);
// 0x0048ce60
int __fastcall ConfigTree_FreeNode(int ecx, int edx);
// 0x0048cec0
int __fastcall ConfigTree_FindChildByName(int ecx, int edx, int arg1);
// 0x0048d080
int __fastcall ConfigTree_ReadNode(int ecx, int edx);
// 0x0048c7d0
int __fastcall Container_InitNodePool(int ecx, int edx);
// 0x0048c8e0
int __fastcall Container_NodeAlloc_FromFreeList(int ecx, int edx);
// 0x0048ca70
int __fastcall Container_ListRemoveMatching(int ecx, int edx);
// 0x0048cb70
int __fastcall Container_ListPopCursor(int ecx, int edx);
// 0x0048ce40
int __fastcall ConfigTree_Destroy(int ecx, int edx);
// 0x0048cf70
int __fastcall ConfigTree_FindChild(int ecx, int edx);
// 0x0048c970
int __fastcall Container_DestroyList(int ecx, int edx);
// 0x0048ca10
int __fastcall Container_AllocNodeWithPayload(int ecx, int edx);
// 0x0048cf80
int __fastcall ConfigTree_GetStringValue(int ecx, int edx);
// 0x0048cfb0
int __fastcall ConfigTree_GetFloatValue(int ecx, int edx, int arg1);
// 0x0048d030
int __fastcall ConfigTree_GetIntValue(int ecx, int edx, int arg1);
// 0x0048c890
int __fastcall Container_ShutdownNodePool(int ecx, int edx);
// 0x0048c9c0
int __fastcall Container_ListInsertAfterCursor(int ecx, int edx);
// 0x0048ca30
int __fastcall Container_ListAppend(int ecx, int edx);
// 0x0048d2c0
int __fastcall Reader_CloseAll(int ecx, int edx);
// 0x0048cd10
int __fastcall Archive_Shutdown(int ecx, int edx);
// 0x0048cca0
int __fastcall SearchPath_InitOrReset(int ecx, int edx);
// 0x0048cce0
int __fastcall SearchPath_InitOrAdd(int ecx, int edx);
// 0x0048cd40
int __fastcall Reader_OpenWithSearch(int ecx, int edx);
// 0x0048cc70
int __fastcall Archive_InitRegistry(int ecx, int edx);
// 0x0048d1c0
int __fastcall Zar_FindEntryInArchives(int ecx, int edx);
// 0x0048cdc0
int __fastcall ConfigTree_ParseFileByBasename(int ecx, int edx, int arg1);
// 0x004cb100
int __fastcall EH_Unwind_Zar_OpenAndRegisterArchive_0(int ecx, int edx);
// 0x004cb10b
int __fastcall EH_Handler_Zar_OpenAndRegisterArchive(int ecx, int edx);
// 0x0048d210
int __fastcall Zar_OpenAndRegisterArchive(int ecx, int edx);
}  // namespace recoil
