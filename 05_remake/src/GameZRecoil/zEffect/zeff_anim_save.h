// SUBSYSTEM: zeffect
// Declarations for src/GameZRecoil/zEffect/zeff_anim_save.cpp.
#pragma once

namespace recoil {

// 0x00461a90
int __fastcall EffectList_DecCount(int ecx, int edx);
// 0x00461eb0
int __fastcall SetGlobals_0053a2e4(int ecx, int edx);
// 0x00461ec0
int __fastcall AnimNode_FindInTree(int ecx, int edx);
// 0x00460ae0
int __fastcall EffectList_AppendEntry(int ecx, int edx);
// 0x00461430
int __fastcall Anim_BuildNodeSaveRecords(int ecx, int edx);
// 0x00461970
int __fastcall Anim_EmitEventRecord(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9);
// 0x00461aa0
int __fastcall Effect_QueueSpawnRecord(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00461ba0
int __fastcall Anim_EmitAttachRecord(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00461d00
int __fastcall Anim_EmitAttachVelRecord(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00460bc0
int __fastcall Anim_BuildRunningInstanceSaveRecord(int ecx, int edx, int arg1, int arg2);
// 0x00461800
int __fastcall EffectCmd_RecordSize(int ecx, int edx);
// 0x00460f80
int __fastcall Anim_BuildRunningSaveRecords(int ecx, int edx);
// 0x00462050
int __fastcall Effect_SquaredDistanceToObject(int ecx, int edx);
// 0x00461f50
int __fastcall Effect_InitSmokeDefaults(int ecx, int edx);
// 0x004606d0
int __fastcall Anim_ProcessActivationRecord(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00461840
int __fastcall Effect_ResolveNodeAndCall45d6b0(int ecx, int edx);
// 0x00461870
int __fastcall EffectCmd_Dispatch(int ecx, int edx);
// 0x00461f00
int __fastcall Effect_StartById(int ecx, int edx);
// 0x004620d0
int __fastcall EffectConfig_ResolveOrParse(int ecx, int edx);
// 0x00462130
int __fastcall EffectConfig_InstantiateEntry(int ecx, int edx);
}  // namespace recoil
