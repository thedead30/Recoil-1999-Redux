// SUBSYSTEM: mission
// Declarations for src/Battlesport/mission.cpp.
#pragma once

namespace recoil {

// 0x00417430
int __fastcall Mission_BuildSaveRecord(int ecx, int edx, int arg1);
// 0x004176b0
int __fastcall Mission_BuildLatePhaseFlag(int ecx, int edx);
// 0x00417800
int __fastcall Mission_GetOutcome(int ecx, int edx);
// 0x00417f60
int __fastcall MissionObjectives_Reset(int ecx, int edx);
// 0x004186f0
int __fastcall MissionObjectives_GetSlot(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00419470
int __fastcall Mission_SetRaceTiming(int ecx, int edx, int arg1, int arg2);
// 0x00417300
int __fastcall MapScreen_SetMarkerLabelById(int ecx, int edx, int arg1, int arg2);
// 0x00417ee0
int __fastcall MissionObjectives_Clear(int ecx, int edx);
// 0x00417f90
int __fastcall Mission_LoadObjectivesArray(int ecx, int edx, int arg1);
// 0x00418620
int __fastcall Mission_ShowObjectiveBriefing(int ecx, int edx, int arg1);
// 0x00418760
int __fastcall Mission_ShowReviewScreen(int ecx, int edx, int arg1);
// 0x00418940
int __fastcall Mission_ShowWeaponInfo(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004172c0
int __fastcall MapScreen_SetMarkerStateById(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00418c30
int __fastcall MissionObjectives_FindFirstClear(int ecx, int edx);
// 0x00419010
int __fastcall Mission_Start(int ecx, int edx, int arg1);
// 0x004176f0
int __fastcall Mission_ResetState(int ecx, int edx);
// 0x00417770
int __fastcall Mission_SetOutcomeWithParam(int ecx, int edx, int arg1, int arg2);
// 0x004177a0
int __fastcall Mission_SetOutcome(int ecx, int edx, int arg1);
// 0x004177d0
int __fastcall Mission_SetOutcomeText(int ecx, int edx, int arg1);
// 0x00418fb0
int __fastcall Mission_AdvanceToNext(int ecx, int edx);
// 0x00418230
int __fastcall Mission_LoadObjectiveTriggers(int ecx, int edx, int arg1);
// 0x00417260
int __fastcall Mission_LoadMapScreen(int ecx, int edx, int arg1);
// 0x00417360
int __fastcall Mission_StaticInit_Construct(int ecx, int edx);
// 0x00417370
int __fastcall Mission_StaticInit_RegisterDtor(int ecx, int edx);
// 0x00417390
int __fastcall Mission_Construct(int ecx, int edx);
// 0x00417810
int __fastcall Mission_Load(int ecx, int edx);
// 0x004184e0
int __fastcall Mission_ObjectiveStateMachine(int ecx, int edx);
// 0x00418730
int __fastcall Mission_OnReviewConfirm(int ecx, int edx);
// 0x004188f0
int __fastcall Mission_OnReviewContinue(int ecx, int edx);
// 0x00418c70
int __fastcall Mission_BeginPlay(int ecx, int edx);
// 0x00418d40
int __fastcall Mission_TickAndCheckObjectiveCompletion(int ecx, int edx);
// 0x00419050
int __fastcall Mission_LoadWeather(int ecx, int edx, int arg1);
// 0x004192d0
int __fastcall Mission_StopConfiguredAnims(int ecx, int edx, int arg1, int arg2);
// 0x004193c0
int __fastcall Mission_LoadRaceCheckpoints(int ecx, int edx);
// 0x00419490
int __fastcall Mission_Destruct(int ecx, int edx);
// 0x004c9520
int __fastcall EH_Unwind_Mission_Construct_0(int ecx, int edx);
// 0x004c9528
int __fastcall EH_Unwind_Mission_Construct_1(int ecx, int edx);
// 0x004c9536
int __fastcall EH_Unwind_Mission_Construct_2(int ecx, int edx);
// 0x004c9544
int __fastcall EH_Unwind_Mission_Construct_3(int ecx, int edx);
// 0x004c9560
int __fastcall EH_Unwind_Mission_Load_0(int ecx, int edx);
// 0x004c9580
int __fastcall EH_Unwind_Mission_LoadWeather_0(int ecx, int edx);
// 0x004c958b
int __fastcall EH_Unwind_Mission_LoadWeather_1(int ecx, int edx);
// 0x004c95a0
int __fastcall EH_Unwind_Mission_LoadRaceCheckpoints_0(int ecx, int edx);
// 0x004c95c0
int __fastcall EH_Unwind_Mission_Destruct_0(int ecx, int edx);
// 0x004c95c8
int __fastcall EH_Unwind_Mission_Destruct_1(int ecx, int edx);
// 0x004c95d6
int __fastcall EH_Unwind_Mission_Destruct_2(int ecx, int edx);
// 0x004c9552
int __fastcall EH_Handler_Mission_Construct(int ecx, int edx);

// 0x004c9568
int __fastcall EH_Handler_Mission_Load(int ecx, int edx);

// 0x004c9596
int __fastcall EH_Handler_Mission_LoadWeather(int ecx, int edx);

// 0x004c95a8
int __fastcall EH_Handler_Mission_LoadRaceCheckpoints(int ecx, int edx);

// 0x004c95e4
int __fastcall EH_Handler_Mission_Destruct(int ecx, int edx);

// 0x004174f0
int __fastcall Mission_RestoreSaveRecord(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00417640
int __fastcall Mission_RegisterSaveHandlers(int ecx, int edx);
// 0x00417a00
int __fastcall Mission_InitGameplay(int ecx, int edx);
// 0x00417d40
int __fastcall Mission_Unload(int ecx, int edx);
}  // namespace recoil
