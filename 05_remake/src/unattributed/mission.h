// SUBSYSTEM: mission
// Declarations for src/unattributed/mission.cpp.
#pragma once

namespace recoil {

// 0x0042ecb0
int __fastcall Mission_SetDataSearchPaths(int ecx, int edx);
// 0x00479cb0
int __fastcall Global_Set_0057d9a0(int ecx, int edx);
// 0x00419650
int __fastcall MissionHud_Reset(int ecx, int edx);
// 0x00417680
int __fastcall Mission_BuildSaveRecord_Slot_00417680(int ecx, int edx);
// 0x00419380
int __fastcall Mission_HandleMode_00419380(int ecx, int edx);
// 0x00419690
int __fastcall Screen_TickAndPresent_00419690(int ecx, int edx, int arg1);
// 0x00417350
int __fastcall StaticInitWrapper_00417350(int ecx, int edx);
// 0x00417380
int __fastcall AtexitStub_Mission_Destruct_00417380(int ecx, int edx);
// 0x004176d0
int __fastcall Mission_LoadStartAnims_004176d0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004c9628
int __fastcall EH_Unwind_MissionPanel_Dtor_1(int ecx, int edx);
// 0x00417ca0
int __fastcall Mission_HandleCommand_00417ca0(int ecx, int edx);
// 0x00419850
int __fastcall MissionPanel_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x00419870
int __fastcall MissionPanel_Dtor(int ecx, int edx);
// 0x004c9620
int __fastcall EH_Unwind_MissionPanel_Dtor_0(int ecx, int edx);
// 0x004c9636
int __fastcall EH_Handler_MissionPanel_Dtor(int ecx, int edx);

// 0x00417690
int __fastcall Mission_Slot_Forward_004174f0_00417690(int ecx, int edx, int arg1, int arg2, int arg3);
}  // namespace recoil
