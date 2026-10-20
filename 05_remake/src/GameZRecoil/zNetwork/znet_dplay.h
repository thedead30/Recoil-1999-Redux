// SUBSYSTEM: znetwork
// Declarations for src/GameZRecoil/zNetwork/znet_dplay.cpp.
#pragma once

namespace recoil {

// 0x00489f80
int __fastcall Net_IsSessionActive(int ecx, int edx);
// 0x00489fa0
int __fastcall NetPlayerArray_Clear(int ecx, int edx);
// 0x0048a030
int __fastcall NetPlayerList_Clear(int ecx, int edx);
// 0x0048a0d0
int __fastcall DPlay_EnumConnectionsToList(int ecx, int edx);
// 0x0048a130
int __fastcall NetPlayers_RefreshAndGetList(int ecx, int edx);
// 0x0048a140
int __fastcall DPlay_DestroyPlayer(int ecx, int edx);
// 0x0048a180
int __fastcall DPlay_SelectProvider(int ecx, int edx);
// 0x0048a220
int __fastcall DPlay_EnumSessionsSync(int ecx, int edx);
// 0x0048a2c0
int __fastcall NetPlayer_GetField38(int ecx, int edx);
// 0x0048a2e0
int __fastcall NetPlayer_GetPair30(int ecx, int edx, int arg1);
// 0x0048a310
int __fastcall Net_EnumSessions(int ecx, int edx);
// 0x0048a350
int __fastcall DPlay_CheckCaps(int ecx, int edx);
// 0x0048a410
int __fastcall DPlay_HostSession(int ecx, int edx);
// 0x0048a520
int __fastcall DPlay_JoinSession(int ecx, int edx);
// 0x0048a980
int __fastcall DPlay_DestroyLocalPlayer(int ecx, int edx);
// 0x0048a9c0
int __fastcall DPlay_CreateLocalPlayerAndJoin(int ecx, int edx);
// 0x0048acf0
int __fastcall DPlay_BroadcastNonGuaranteed(int ecx, int edx);
// 0x0048ad30
int __fastcall DPlay_BroadcastGuaranteed(int ecx, int edx);
// 0x0048ad70
int __fastcall DPlay_SendExTcp(int ecx, int edx);
// 0x0048ae10
int __fastcall DPlay_SendExGuaranteedAsync(int ecx, int edx);
// 0x0048ae70
int __fastcall Net_ReceiveMessages(int ecx, int edx);
// 0x0048afa0
int __fastcall Net_GetPlayerName(int ecx, int edx, int arg1);
// 0x0048afe0
int __fastcall DPlay_HandleSystemMessage(int ecx, int edx);
// 0x0048b3a0
int __fastcall DPlay_OnEnumConnection(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x0048b5e0
int __fastcall DPlay_OnEnumSession(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x0048b730
int __fastcall DPlay_CoCreate(int ecx, int edx);
// 0x0048b7f0
int __fastcall DPlay_Release(int ecx, int edx);
// 0x0048b860
int __fastcall NetMsg_SendPlayerSlots(int ecx, int edx);
// 0x0048b940
int __fastcall NetSlot_Allocate(int ecx, int edx);
// 0x0048b980
int __fastcall Net_GetMaxPlayers(int ecx, int edx);
// 0x0048b9a0
int __fastcall Net_GetPlayerSlot(int ecx, int edx);
// 0x0048b9d0
int __fastcall NetSession_GetFlags(int ecx, int edx);
// 0x0048b9e0
int __fastcall NetPlayer_Remove(int ecx, int edx);
// 0x0048ba60
int __fastcall NetPlayer_FindById(int ecx, int edx);
// 0x0048bab0
int __fastcall Net_GetSessionDesc(int ecx, int edx);
// 0x0048bb20
int __fastcall Net_SetSessionDesc(int ecx, int edx);
// 0x0048be70
int __fastcall DPlay_EnumSessions(int ecx, int edx);
// 0x0048bee0
int __fastcall NetSession_FreeNames(int ecx, int edx);
// 0x0048bf10
int __fastcall Array_CopyRange(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0048bf40
int __fastcall NetMsgQueue_Clear(int ecx, int edx);
// 0x0048bfb0
int __fastcall NetMsgQueue_Init(int ecx, int edx);
// 0x0048bfe0
int __fastcall StaticInit_RegisterAtexit_0048bff0(int ecx, int edx);
// 0x0048c060
int __fastcall Net_SendA_NonGuaranteed(int ecx, int edx);
// 0x0048c080
int __fastcall Net_SendB_Guaranteed(int ecx, int edx);
// 0x0048c0a0
int __fastcall NetMsgQueue_Push(int ecx, int edx, int arg1);
// 0x0048c120
int __fastcall NetMsgQueue_RemoveMatching(int ecx, int edx);
// 0x0048c200
int __fastcall NetMsgQueue_Dispatch(int ecx, int edx);
// 0x0048c250
int __fastcall DPlay_ReportError(int ecx, int edx, int arg1);
// 0x004cb060
int __fastcall EH_Unwind_DPlay_CreateLocalPlayerAndJoin_0(int ecx, int edx);
// 0x004cb080
int __fastcall EH_Unwind_DPlay_HandleSystemMessage_0(int ecx, int edx);
// 0x004cb0a0
int __fastcall EH_Unwind_DPlay_OnEnumConnection_0(int ecx, int edx);
// 0x004cb0e0
int __fastcall EH_Unwind_DPlay_ConnectForcedTcpIp_0(int ecx, int edx);
// 0x004cb06b
int __fastcall EH_Handler_DPlay_CreateLocalPlayerAndJoin(int ecx, int edx);

// 0x004cb08b
int __fastcall EH_Handler_DPlay_HandleSystemMessage(int ecx, int edx);

// 0x004cb0ab
int __fastcall EH_Handler_DPlay_OnEnumConnection(int ecx, int edx);

// 0x004cb0e8
int __fastcall EH_Handler_DPlay_ConnectForcedTcpIp(int ecx, int edx);

}  // namespace recoil
