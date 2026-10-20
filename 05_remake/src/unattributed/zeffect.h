// SUBSYSTEM: zeffect
// Declarations for src/unattributed/zeffect.cpp.
#pragma once

namespace recoil {

// 0x00460480
int __fastcall EffectList_EntryAt(int ecx, int edx);
// 0x00462540
int __fastcall EffectCmd_SetVec1C(int ecx, int edx, int arg1);
// 0x0049aa40
int __fastcall LensFlare_SetTexture(int ecx, int edx);
// 0x00462280
int __fastcall Effect_FindTemplateByName(int ecx, int edx);
// 0x004a5b20
int __fastcall CallOptionalHook_0056b568(int ecx, int edx);
// 0x00462370
int __fastcall MciVideo_OpenAndPlay(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00460490
int __fastcall Anim_SaveActivationRecords_00460490(int ecx, int edx);
// 0x004621b0
int __fastcall Effect_UpdateScalePulse_004621b0(int ecx, int edx);
// 0x00458bb0
int __fastcall Effect_TickLifetime_00458bb0(int ecx, int edx, int arg1, int arg2);
// 0x0045d010
int __fastcall Anim_UpdateInstance_0045d010(int ecx, int edx);
// 0x0045d4c0
int __fastcall Anim_UpdateSingleChannel_0045d4c0(int ecx, int edx);
// 0x00460060
int __fastcall EffectList_ResetAndFree(int ecx, int edx);
// 0x00461040
int __fastcall Anim_LoadActivationRecord_00461040(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00461670
int __fastcall Anim_RestoreNodesFromRecord_00461670(int ecx, int edx, int arg1, int arg2, int arg3);
}  // namespace recoil
