// SUBSYSTEM: script
// Declarations for src/GameZRecoil/zInterp/zinterp_parse.cpp.
#pragma once

namespace recoil {

// 0x004c1160
int __fastcall Script_ReadLine(int ecx, int edx, int arg1, int arg2);
// 0x004c13c0
int __fastcall Script_Tokenize(int ecx, int edx, int arg1);
// 0x004c15f0
int __fastcall Script_GetVar(int ecx, int edx, int arg1, int arg2);
// 0x004c1670
int __fastcall Script_FreeStringPairs(int ecx, int edx);
// 0x004c16c0
int __fastcall Script_FreeLabels(int ecx, int edx);
// 0x004c1870
int __fastcall Script_DumpArgs(int ecx, int edx);
// 0x004c18c0
int __fastcall Script_PushFrame(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004c1940
int __fastcall Script_PopFrame(int ecx, int edx);
// 0x004c1960
int __fastcall Script_FreeFrames(int ecx, int edx);
// 0x004c1a40
int __fastcall Script_FindLabel(int ecx, int edx, int arg1);
// 0x004c1b20
int __fastcall Script_IncLine(int ecx, int edx);
// 0x004c1b30
int __fastcall Script_CallErrorHook(int ecx, int edx);
// 0x004c5480
int __fastcall Script_Argv0Matches(int ecx, int edx, int arg1, int arg2);
// 0x004c54b0
int __fastcall Script_Argv0Equals(int ecx, int edx, int arg1);
// 0x004c5510
int __fastcall Script_Argv0(int ecx, int edx);
// 0x004c5520
int __fastcall Script_Fail(int ecx, int edx);
// 0x004c5550
int __fastcall ScriptCache_OpenIndex(int ecx, int edx, int arg1);
// 0x004c5740
int __fastcall ScriptCache_OpenCompiled(int ecx, int edx, int arg1);
// 0x004c58c0
int __fastcall Script_NodeGetModelIfAny(int ecx, int edx, int arg1);
// 0x004c1250
int __fastcall Script_ExpandVars(int ecx, int edx, int arg1);
// 0x004c1710
int __fastcall Script_VarIsTrue(int ecx, int edx, int arg1);
// 0x004c1780
int __fastcall Script_SetVar(int ecx, int edx, int arg1, int arg2);
// 0x004c1ab0
int __fastcall Script_DumpValue(int ecx, int edx, int arg1);
// 0x004c2030
int __fastcall Script_DumpTree(int ecx, int edx, int arg1, int arg2);
// 0x004c5820
int __fastcall Script_CheckArgs(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004c1990
int __fastcall Script_NextArg(int ecx, int edx);
// 0x004c1b50
int __fastcall Script_EvalCondition(int ecx, int edx);
// 0x004c19c0
int __fastcall Script_ArgBool(int ecx, int edx);
// 0x004c1a00
int __fastcall Script_ArgFloat(int ecx, int edx);
// 0x004c1a20
int __fastcall Script_ArgInt(int ecx, int edx);
// 0x004c0f70
int __fastcall Script_Reset(int ecx, int edx);
// 0x004c58e0
int __fastcall Script_SetNodeTextureScroll(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c20a0
int __fastcall Script_DispatchCommand(int ecx, int edx, int arg1);
// 0x004c0e50
int __fastcall Script_Dtor(int ecx, int edx);
// 0x004c1020
int __fastcall Script_RunStream(int ecx, int edx, int arg1, int arg2);
// 0x004c1090
int __fastcall Script_ExecuteLine(int ecx, int edx, int arg1);
// 0x004c1500
int __fastcall Script_RunFile(int ecx, int edx, int arg1);
// 0x004c1c50
int __fastcall Script_BuiltinDirectives(int ecx, int edx, int arg1);
// 0x004cb9d0
int __fastcall EH_Unwind_Script_Dtor_0(int ecx, int edx);
// 0x004cb9de
int __fastcall EH_Handler_Script_Dtor(int ecx, int edx);

}  // namespace recoil
