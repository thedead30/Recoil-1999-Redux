// SUBSYSTEM: net
// Declarations for src/unattributed/net.cpp.
#pragma once

namespace recoil {

// 0x00431dd0
int __fastcall Net_StartMatch(int ecx, int edx);
// 0x004320b0
int __fastcall Net_WaitForSessions(int ecx, int edx);
// 0x004320f0
int __fastcall NetPlayers_FreeAll(int ecx, int edx);
// 0x004322a0
int __fastcall NetMatch_ResetTimers(int ecx, int edx);
// 0x00432300
int __fastcall NetMsg_SendVehicleState(int ecx, int edx);
// 0x00432830
int __fastcall PlayerList_FindById(int ecx, int edx);
// 0x00432ae0
int __fastcall NetPlayer_ApplyVehicleState(int ecx, int edx);
// 0x00432d60
int __fastcall NetPlayerTag_Update(int ecx, int edx);
// 0x00432e70
int __fastcall NetPlayers_AssignSlotsAndColours(int ecx, int edx);
// 0x00433000
int __fastcall NetMsg_SendKill(int ecx, int edx);
// 0x00433060
int __fastcall NetMsg_OnKill(int ecx, int edx);
// 0x004330f0
int __fastcall NetMsg_SendLapComplete(int ecx, int edx);
// 0x00433170
int __fastcall NetMsg_OnLapReport(int ecx, int edx);
// 0x00433200
int __fastcall Net_AllPlayersReachedLaps(int ecx, int edx);
// 0x00433250
int __fastcall NetMsg_OnPlayerStatus(int ecx, int edx);
// 0x00433310
int __fastcall NetMsg_SendPlayerStatus(int ecx, int edx);
// 0x00433390
int __fastcall NetMsg_SendMatchState(int ecx, int edx);
// 0x00433410
int __fastcall NetMsg_OnMatchState(int ecx, int edx);
// 0x004334f0
int __fastcall NetMsg_SendScoreboard(int ecx, int edx);
// 0x004335b0
int __fastcall NetMsg_OnScoreboard(int ecx, int edx);
// 0x004336f0
int __fastcall Net_GetPlayerId(int ecx, int edx);
// 0x00433710
int __fastcall Net_SetFlags3f88_3f90(int ecx, int edx);
// 0x00433730
int __fastcall Net_GetFlag_004f3f88(int ecx, int edx);
// 0x00433740
int __fastcall Net_GetFlag_004f3f90(int ecx, int edx);
// 0x00433750
int __fastcall NetMsg_SendChat(int ecx, int edx);
// 0x004337e0
int __fastcall Net_ShowChatMessage_004337e0(int ecx, int edx);
// 0x00433840
int __fastcall NetPlayer_Respawn(int ecx, int edx);
// 0x004339d0
int __fastcall NetPlayers_NearestOtherDistSq(int ecx, int edx);
// 0x00433a40
int __fastcall NetPlayer_ClearSlots2c(int ecx, int edx);
// 0x00433a50
int __fastcall NetPlayer_ApplyTeamColour(int ecx, int edx);
// 0x00433c30
int __fastcall NetMsg_SendProjectileSpawn(int ecx, int edx);
// 0x00433de0
int __fastcall NetMsg_SendObjectState(int ecx, int edx);
// 0x00433e40
int __fastcall NetMsg_SendPickupRemoved(int ecx, int edx);
// 0x00433e70
int __fastcall NetMsg_SendPickupRespawnQueued(int ecx, int edx);
// 0x00433ea0
int __fastcall NetMsg_SendPickupSpawned(int ecx, int edx);
// 0x00433f40
int __fastcall Net_Handler_00433f40(int ecx, int edx);
// 0x00434050
int __fastcall NetMsg_SendPositionEvent(int ecx, int edx, int arg1);
// 0x004340a0
int __fastcall Net_Handler_004340a0(int ecx, int edx);
// 0x004340c0
int __fastcall NetTarget_Encode(int ecx, int edx);
// 0x00434130
int __fastcall NetMsg_SendTargetEvent(int ecx, int edx);
// 0x00434230
int __fastcall Net_Handler_ReturnTrue_00434230(int ecx, int edx);
// 0x00434240
int __fastcall Net_BuildAndSendMessage_00434240(int ecx, int edx, int arg1);
// 0x00434370
int __fastcall NetMsg_SendRemoteObjectBlob(int ecx, int edx);
// 0x004343f0
int __fastcall Net_Handler_SetFlagAndCall461870_004343f0(int ecx, int edx);
// 0x00434430
int __fastcall Net_ForEachRemoteObject(int ecx, int edx);
// 0x00434460
int __fastcall NetMsg_SendEvent(int ecx, int edx, int arg1, int arg2);
// 0x00434550
int __fastcall Net_SendIfConnected(int ecx, int edx, int arg1, int arg2);
// 0x004345a0
int __fastcall PlayerList_AppendNew(int ecx, int edx, int arg1);
// 0x00434650
int __fastcall PlayerList_ForwardTo004bab40(int ecx, int edx);
// 0x00489d00
int __fastcall DPlay_InitSession(int ecx, int edx);
// 0x0048b820
int __fastcall ZNet_RegisterEntries_0048b820(int ecx, int edx);
// 0x004ca140
int __fastcall EH_Unwind_PlayerList_AppendNew_0(int ecx, int edx);
// 0x004cb040
int __fastcall EH_Unwind_DPlay_InitSession_0(int ecx, int edx);
// 0x004ca14b
int __fastcall EH_Handler_PlayerList_AppendNew(int ecx, int edx);

// 0x004cb04b
int __fastcall EH_Handler_DPlay_InitSession(int ecx, int edx);

// 0x00433ad0
int __fastcall NetMsg_OnProjectileHit(int ecx, int edx);
// 0x00433b70
int __fastcall NetProjectile_ApplyOrForward(int ecx, int edx);
// 0x00433ca0
int __fastcall NetMsg_OnObjectHit(int ecx, int edx);
// 0x00433d40
int __fastcall NetObject_ApplyOrForward(int ecx, int edx);
// 0x00431c50
int __fastcall Net_InitSession(int ecx, int edx);
// 0x004321b0
int __fastcall Resources_ReleaseGroups(int ecx, int edx);
// 0x004327e0
int __fastcall Net_Handler_004327e0(int ecx, int edx);
// 0x00432860
int __fastcall NetPlayer_AddRemote(int ecx, int edx);
// 0x00432ed0
int __fastcall Net_RemovePlayerRecord_00432ed0(int ecx, int edx);
// 0x00434190
int __fastcall Net_ApplyVehicleUpdate_00434190(int ecx, int edx);
// 0x004342d0
int __fastcall Net_Handler_004342d0(int ecx, int edx);
// 0x004344b0
int __fastcall Net_Handler_004344b0(int ecx, int edx);
}  // namespace recoil
