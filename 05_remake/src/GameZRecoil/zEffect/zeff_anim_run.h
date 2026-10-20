// SUBSYSTEM: zeffect
// Declarations for src/GameZRecoil/zEffect/zeff_anim_run.cpp.
#pragma once

namespace recoil {

// 0x0045d000
int __fastcall SetGlobal_004df670(int ecx, int edx);
// 0x0045d7a0
int __fastcall Effect_ClearByte10B(int ecx, int edx);
// 0x0045e0d0
int __fastcall SetPair6C70_IfNonNull(int ecx, int edx, int arg1);
// 0x0045d240
int __fastcall AnimInstance_SaveObjectStates(int ecx, int edx);
// 0x0045db20
int __fastcall AnimInstance_CheckConditions(int ecx, int edx);
// 0x0045d6c0
int __fastcall AnimInstance_Reset(int ecx, int edx);
// 0x0045d310
int __fastcall AnimInstance_RestoreObjectStates(int ecx, int edx);
// 0x0045cc00
int __fastcall AnimInstance_RunSequence(int ecx, int edx);
// 0x0045d3d0
int __fastcall AnimInstance_Stop(int ecx, int edx);
// 0x0045d570
int __fastcall AnimInstance_Start(int ecx, int edx, int arg1);
// 0x0045d6b0
int __fastcall Wrap_0045d570_Arg1(int ecx, int edx);
// 0x0045d770
int __fastcall Effect_AdvanceTimer(int ecx, int edx);
// 0x0045d7b0
int __fastcall AnimInstance_SetTransformVelocity(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9);
// 0x0045d930
int __fastcall AnimInstance_Acquire(int ecx, int edx);
// 0x0045dc70
int __fastcall Effect_SpawnAt_Wrap_0045d7b0(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9);
// 0x0045dcb0
int __fastcall AnimInstance_SetVelocity(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0045dde0
int __fastcall Wrap_0045dcb0(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0045de00
int __fastcall AnimInstance_SetAttachWithVelocity(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0045df70
int __fastcall Wrap_0045de00(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0045df90
int __fastcall AnimInstance_SetAttachTarget(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x0045e0b0
int __fastcall Wrap_0045df90(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
}  // namespace recoil
