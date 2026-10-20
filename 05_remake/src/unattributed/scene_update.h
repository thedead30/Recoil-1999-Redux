// SUBSYSTEM: scene_update
// Declarations for src/unattributed/scene_update.cpp.
#pragma once

namespace recoil {

// 0x0044eed0
int __fastcall NodeList_MarkNode(int ecx, int edx);
// 0x0044e630
int __fastcall NodeList_AllocLink(int ecx, int edx);
// 0x0044e690
int __fastcall NodeList_FreeLink(int ecx, int edx);
// 0x0044e6d0
int __fastcall NodeList_FreeLinkPool(int ecx, int edx);
// 0x0044ec90
int __fastcall NodeList_Count(int ecx, int edx);
// 0x0044ecb0
int __fastcall NodeList_DebugPrint(int ecx, int edx);
// 0x0044ecf0
int __fastcall NodeRegistry_LookupByName(int ecx, int edx);
// 0x0044ed50
int __fastcall NodeList_GetTail(int ecx, int edx);
// 0x0044e700
int __fastcall NodeList_FlushList(int ecx, int edx);
// 0x0044ed60
int __fastcall NodeList_DeferNode(int ecx, int edx);
// 0x0044ee10
int __fastcall NodeList_PushHead(int ecx, int edx);
// 0x0044eea0
int __fastcall NodeList_ProcessDeferred(int ecx, int edx);
// 0x0044e920
int __fastcall NodeList_FlushAll(int ecx, int edx);
// 0x0044ed90
int __fastcall NodeRegistry_Insert(int ecx, int edx);
// 0x0044eaa0
int __fastcall NodeList_RunCallbacks(int ecx, int edx);
// 0x0044ebe0
int __fastcall NodeList_RunList11(int ecx, int edx);
// 0x0044ec30
int __fastcall NodeList_RunList12(int ecx, int edx);
// 0x0044eb00
int __fastcall NodeTree_UpdatePending(int ecx, int edx);
// 0x0044eb50
int __fastcall NodeTree_UpdateWithAncestors(int ecx, int edx);
// 0x0044eba0
int __fastcall NodeList_UpdateAllPending(int ecx, int edx);
// 0x0044ec80
int __fastcall NodeList_RunClientLists(int ecx, int edx);
// 0x0044ea70
int __fastcall NodeList_RunAll(int ecx, int edx);
}  // namespace recoil
