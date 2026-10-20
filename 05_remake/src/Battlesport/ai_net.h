// SUBSYSTEM: ai_net
// Declarations for src/Battlesport/ai_net.cpp (original file Battlesport\ai_net.cpp, ledger orig_file).
// Spec: 04_spec/systems/ai_net.md
#pragma once

namespace recoil {

// 0x00402f60 Vec3_NormalizeInPlace (ECX v) -> length as float (0 length: v unchanged)
float __fastcall Vec3_NormalizeInPlace(float* v);

// 0x004016a0
int __fastcall NetNode_PickRandomLink(int ecx, int edx, int arg1, int arg2);
// 0x00401c60
int __fastcall AiVeh_EnterState1(int ecx, int edx);
// 0x00402080
int __fastcall AiVeh_CopyF88ToF84(int ecx, int edx);
// 0x00402ff0
int __fastcall NetGraph_AllocAndAppend(int ecx, int edx);
// 0x00403510
int __fastcall NetGraph_LookupByIndex(int ecx, int edx);
// 0x00403530
int __fastcall NetNode_FindById(int ecx, int edx);
// 0x00403620
int __fastcall NetEdge_Init(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x004036f0
int __fastcall NetGraph_FindNearestNode(int ecx, int edx);
// 0x00403750
int __fastcall NetGraph_LinkVehiclesOnSamePath(int ecx, int edx);
// 0x004037c0
int __fastcall NetNode_Free(int ecx, int edx);
// 0x00401580
int __fastcall AiVeh_AdvanceToLinkedNode(int ecx, int edx, int arg1, int arg2);
// 0x00401c00
int __fastcall AiList_ResetNonIdle(int ecx, int edx);
// 0x00401f60
int __fastcall AiVeh_PushReturnNode(int ecx, int edx);
// 0x00402f10
int __fastcall AiNet_ResetPlayerVehicles(int ecx, int edx);
// 0x00403550
int __fastcall NetGraph_LinkNodes(int ecx, int edx, int arg1);
// 0x00403800
int __fastcall NetGraph_Free(int ecx, int edx);
// 0x00403830
int __fastcall AiVeh_PopNegativeRouteNodes(int ecx, int edx);
// 0x00403040
int __fastcall NetGraph_LoadFromZrdByIndex(int ecx, int edx);
// 0x00403870
int __fastcall NetGraph_FreeAll(int ecx, int edx);
// 0x00402fd0
int __fastcall NetGraph_LoadAllForMission(int ecx, int edx);
// 0x00401970
int __fastcall AiVeh_KeepRangeDrive(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00402090
int __fastcall AiVeh_SteerTowardPlayerA(int ecx, int edx);
// 0x00402170
int __fastcall AiVeh_SteerTowardPlayerB(int ecx, int edx);
// 0x004024a0
int __fastcall AiVeh_LeadTargetIntercept(int ecx, int edx, int arg1);
// 0x004026d0
int __fastcall AiVeh_CircleStrafeTarget(int ecx, int edx);
// 0x004028c0
int __fastcall AiVeh_FollowOffsetTarget(int ecx, int edx, int arg1);
// 0x00402be0
int __fastcall AiVeh_DriveToNextNode(int ecx, int edx);
// 0x00402d60
int __fastcall AiVeh_ReverseToNextNode(int ecx, int edx);
// 0x00401060
int __fastcall AiVeh_StateTick(int ecx, int edx);
// 0x00401180
int __fastcall AiVeh_PatrolTick(int ecx, int edx);
// 0x00401420
int __fastcall AiVeh_ProbeAhead(int ecx, int edx);
// 0x00401710
int __fastcall AiVeh_CombatTick(int ecx, int edx);
// 0x00401a40
int __fastcall AiVeh_CircleOrAim(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00401ab0
int __fastcall AiVeh_FollowOrAim(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00401b20
int __fastcall AiVeh_TryDetectPlayer(int ecx, int edx);
// 0x00401d50
int __fastcall Collision_TraceWithPlayerPos(int ecx, int edx, int arg1);
// 0x00401e50
int __fastcall Collision_TraceWithCameraPos(int ecx, int edx, int arg1);
// 0x00402250
int __fastcall AiVeh_FireDecision(int ecx, int edx, int arg1, int arg2);
// 0x00402b70
int __fastcall AiVeh_RouteTick(int ecx, int edx);
}  // namespace recoil
