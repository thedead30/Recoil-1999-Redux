# zeffect - animation node effects (spec)

Status: gate OPEN 2026-09-23. 76/76 members CONFIRMED (bytes read); callee coverage complete.
Range 0x0045cc00..0x00462570 (between zDEClient and zFMV source paths). All provenance below is CONFIRMED-BINARY at the cited address unless noted.

## Key data (CONFIRMED-BINARY)
- Anim file: magic 0x08170616, version 0x1c; 0x54-byte records; 0x3c globals block at 0x00575da0 (Anim_LoadFile 0x0045efb0).
- Anim node table [0x00575dac], 0x134 bytes each, count u16 [0x00575daa]; sub-tables sized 0x60/0x28/0x2c/0x2c/0x24/0x24/0x30/0x48/0x40.
- Effect list [0x0053a2d8], 0x50-byte entries, starts 1000, doubles.
- Effect defs: 0x48-byte entries at [0x00575a50] (EffectDefs_LoadOnce 0x00460070).
- Velocity threshold 0.01 (double at 0x004d24c8); -99.0f sentinel at 0x004d24d0 with 0.1 tolerance (0x004d2488).
- Sequence runner 0x0045cc00 dispatches event types 1..0x28; instance states 0..6.
- Gravity -9.8 and 200-entry random table (Anim_ResetGlobals).
- Save records: Anim_%04d (n*0x44+0x58), Running_%03d (0x68 header + 0x40/0x38/0x3c entries).

## Membership note
- 0x00462370 is MCI MPEGVideo open/play: kept by address span; review against zFMV.

## Members

| addr | name | notes |
|---|---|---|
| 0x0045cc00 | AnimInstance_RunSequence | ECX=instance, EDX=sequence cursor (+0x20 status, +0x21 mode, +0x24/+0x28 timers, +0x34 current event, +0x38 base, +0x3c length). Null/no event -> status 2, retu |
| 0x0045d000 | SetGlobal_004df670 | [0x004df670] = [0x0056bbd8] + 1 |
| 0x0045d240 | AnimInstance_SaveObjectStates | null -> -1; for each of the byte [ECX+0x105] object slots (0x60 bytes at [ECX+0x110]): saves the node's active bit ([node+0x24] bit 2); for object3d (type 5) sa |
| 0x0045d310 | AnimInstance_RestoreObjectStates | null -> -1; for each of the byte [ECX+0x105] saved object states (0x60 bytes at [ECX+0x110]): gwNodeSetActive(node, saved active); object3d (type 5) with the ma |
| 0x0045d3d0 | AnimInstance_Stop | null or state byte [+0x98] == 5 -> -1. Flag 0x100 of [+0x94]: 0x00451240 nonzero -> -1, else clear the flag. DEScene_DeactivateLights, DEScene_DeactivateSounds; |
| 0x0045d570 | AnimInstance_Start | ECX=instance, EDX=anim node, stack arg loop flag (ret 4). Returns -1 if null or state +0x98==5 (80 fa 05). If EDX==current +0x40 and state 2/6 calls 0x0045c040; |
| 0x0045d6b0 | Wrap_0045d570_Arg1 | 8 bytes, bytes read: push 1; call 0x0045d570; ret (ECX/EDX pass through). 17 callers; weapon uses it to release effect handles. |
| 0x0045d6c0 | AnimInstance_Reset | null -> -1. Root node 0x00449ab0 (arguments not visible) missing -> -1; node type [+0x34] 1 or 2 -> clear flag 0x100 of [ECX+0x94]; other types -> 0x004510e0 no |
| 0x0045d770 | Effect_AdvanceTimer | bytes read: no [ECX+0x40] -> return; [+0xA8] += [+0xA4] * [0x0056b424] (frame dt); result > 0 -> return, else EDX = 0 and tail-jump 0x0045d6a2 (the continuation |
| 0x0045d7a0 | Effect_ClearByte10B | [ECX+0x10B] = 0 |
| 0x0045d7b0 | AnimInstance_SetTransformVelocity | If flag 0x100: applies pos (p1..p3) and rot (p4..p6, skipped if 0x200) via 0x0044a0f0/0x00449ea0 (type1) or Object3D_SetPosition/SetRotation (type5); flag 0x80  |
| 0x0045d930 | AnimInstance_Acquire | ECX=instance, EDX=anim node. Returns 0 for null or state 6/4/5; states 2/3 with +0xa4 == -99.0 (float 0x004d24d0, tolerance 0.1 double 0x004d2488) return 0. Sta |
| 0x0045db20 | AnimInstance_CheckConditions | Iterates +0x10b condition entries (stride 0x30 at +0x128). Type 1: resolves name at +8 against anim node table [0x00575dac] (0x134 stride, count [0x00575daa]) c |
| 0x0045dc70 | Effect_SpawnAt_Wrap_0045d7b0 | 53 bytes, ret 0x24, decomp: forwards 9 stack args (position x,y,z, rotation, ...) to 0x0045d7b0. Weapon code calls it to spawn impact/muzzle effects at a positi |
| 0x0045dcb0 | AnimInstance_SetVelocity | Same attach reset as 0x0045df90; sets flag 0x80 when any /v/>0.01 (double 0.01f at 0x004d24c8, CONFIRMED-BINARY) else clears; stores +0xb4/+0xb8/+0xbc vel, zero |
| 0x0045dde0 | Wrap_0045dcb0 | 23 bytes, ret 0xC, decomp: forwards ECX/EDX and 3 stack args to 0x0045dcb0. 16 callers. |
| 0x0045de00 | AnimInstance_SetAttachWithVelocity | Attach-reset as 0x0045df90; +0x74 target, +0x78..0x80 offset (zero if null), zeroes +0x84..0x90; velocity from param_3: if null or all /c/<=0.01 (0x004d24c8) cl |
| 0x0045df70 | Wrap_0045de00 | 23 bytes, ret 0xC, decomp: forwards to 0x0045de00 with 3 stack args. |
| 0x0045df90 | AnimInstance_SetAttachTarget | Looks up instance via 0x0045d930; if flag 0x100 resets old attach (type1 0x0044a0f0/0x00449ea0, type5 Object3D_SetPosition/SetRotation; rotation skipped if 0x20 |
| 0x0045e0b0 | Wrap_0045df90 | 28 bytes, ret 0x10, decomp: forwards to 0x0045df90 with 4 stack args. |
| 0x0045e0d0 | SetPair6C70_IfNonNull | 17 bytes, bytes read: if ECX != 0, [ECX+0x6c] = EDX and [ECX+0x70] = stack arg; ret 4. |
| 0x0045e100 | Anim_ResetGlobals | AnimData_FreeAll when loaded; zeroes the zbd name and the anim globals [0x00575da0..0x00575dc4]; [0x00575dbc] = -9.8 (gravity); srand(time(0)); fills the 200-en |
| 0x0045e200 | SetGlobal_00575db8 | [0x00575db8] = ECX |
| 0x0045e210 | AnimData_SetZbdName | name longer than 128 -> report 'Animation ZBD filename too long' (zeff 0xD1) and keep the old name; else copies it to [0x0053a1c0] |
| 0x0045e270 | SetGlobal_004df730 | [0x004df730] = ECX |
| 0x0045e280 | AnimNode_FindSoundByName | (node ECX, name EDX): linear search of the 0x2C-entry sound list [ECX+0x11C] (count byte [ECX+0x108]) comparing each entry's node name pointer at +0x24; returns |
| 0x0045e300 | AnimNode_FindLightByName | as AnimNode_FindSoundByName for the light list [ECX+0x118] (count byte [ECX+0x107]) |
| 0x0045e380 | AnimNode_AddSoundEntry | existing index (0x0045e280) >= 1 -> that index. Else Sound_Create (null -> -1), 0x00447dc0 attach and Sound_SetResourceName (arguments not visible); first use a |
| 0x0045e4a0 | AnimNode_AddLightEntry | as AnimNode_AddSoundEntry for lights: existing index (AnimNode_FindLightByName) >= 1 -> it; else Light_Create + attach, first use allocates the 0x2C-entry list  |
| 0x0045e5c0 | AnimNode_ResolveTargetByName | tries Node_FindByNameRecursive twice (two different roots; operands not visible), then the light list (AnimNode_FindLightByName index > 0 -> its node at entry + |
| 0x0045e650 | Node_FindByNameRecursive | (node ECX, name EDX): returns the node whose name (its first bytes) compares equal, else searches its children (count [+0x5C]; child operands not visible) depth |
| 0x0045e6d0 | AnimNode_EnsureModel | null -> 0; flag 0x8000 of [ECX+0x94]: [ECX+0x40] = 0x00452560([0x004df728], [0x004df72c]); failure -> 0; else 0x0045ed80 and the flag is cleared; returns 1 |
| 0x0045e730 | AnimInstance_Clone | ECX=source instance (0x134 bytes), EDX=optional new name. Null -> 0. calloc(1,0x134) (push 0x134), copies source, creates/names scene object via 0x0044daa0/spri |
| 0x0045ed80 | AnimInstance_BindNode | ECX=instance, EDX=anim name. Returns 0 if null or state 5. Stores name at +0x40; copies name into +0x20 and (if +0x64 cached same) +0x44; otherwise resolves nod |
| 0x0045efb0 | Anim_LoadFile | Opens path [0x0053a1c0]; reads 0xc header: magic 0x08170616, version 0x1c (cmp [esp+x],0x1c), count -> [0x0053a248] of 0x54-byte records into malloc [0x0053a24c |
| 0x0045fb30 | Anim_InitAllOnce | Guarded by [0x00575da0]==0 (set to 1 at end). Calls Anim_LoadFile; pass 1 over anim node table [0x00575dac] (stride 0x134, from index 1 to [0x00575daa]) sets fl |
| 0x0045fd10 | AnimInstance_Free | Returns 0 if state +0x98==5. Releases +0x68 via 0x00447b60; +0x99 in {0,1,2} -> 0x004b2630; frees +0xfc (zeroes +0xfc/+0x100), per-entry +0x38 of +0xc0 array (c |
| 0x0045fe50 | AnimData_FreeAll | frees the name buffer [0x00575da4], zeroes [0x00575da8]; releases each of the [0x00575daa] anim node entries (0x0045fd10) and frees the table [0x00575dac]; free |
| 0x0045fef0 | EffectList_FreeIfLoaded | [0x00575da0] nonzero -> 0x0045fe50 (arguments not visible); return 0 |
| 0x0045ff10 | Anim_FindByName | 138 bytes, plain ret, decomp: null ECX -> 0. Linear case-sensitive inlined strcmp of ECX against the name at +0 of each 0x134-byte record in [0x00575dac] (count |
| 0x0045ffa0 | AnimNode_NextWithFlag10 | (node ECX or null): starts at the entry after ECX (index from [0x00575dac], 0x134 each) or at index 0 for null; returns the first node whose flag byte [+0x94] h |
| 0x00460010 | Effect_GetField40 | null -> 0; else [ECX+0x40] |
| 0x00460020 | EffectConfig_Reset | zeroes the loader globals [0x00575a40..0x00575a54], [0x00575a5c..0x00575a64] and calls 0x0045e100 (arguments not visible) |
| 0x00460060 | EffectList_ResetAndFree | bytes read: call 0x00460330 then tail-jump EffectList_FreeIfLoaded (ECX passed through) |
| 0x00460070 | EffectDefs_LoadOnce | Guarded by [0x00575a40]. Opens config via Settings_OpenDetailPresetFile(0) into [0x00575a48] (fail: stderr Failed to read, zEffect line 0xd8, return -1). Child  |
| 0x00460330 | EffectConfig_Release | closes the config tree [0x00575a48] (0x0048ce40); frees [0x00575a50]; drains the list [0x00575a4c] (0x0048cb70 pop, 0x00451a60 release, free) then 0x0048c970; z |
| 0x004603d0 | EffectList_Free | frees the entry array [0x0053a2d8] and zeroes the count [0x0053a2e0] |
| 0x00460400 | EffectList_Contains | linear search over the [0x0053a2e0] entries of [0x0053a2d8] (0x50 bytes): matches when dword +0x28 equals the query's and the 32-byte name at +8 compares equal  |
| 0x00460470 | EffectList_Count | return [0x0053a2e0] |
| 0x00460480 | EffectList_EntryAt | [0x0053a2d8] + index ECX * 0x50 (no bounds check) |
| 0x004606d0 | Anim_ProcessActivationRecord | EDX=record name, param_1=0x50-byte activation record (state byte at +0x54, target id +0x28). Resolves instance via 0x0045ff10. If name == Activation0000: replay |
| 0x00460ae0 | EffectList_AppendEntry | first use mallocs 1000 entries of 0x50 bytes (80000, unchecked) with count [0x0053a2e0] = 0 and capacity [0x0053a2dc] = 1000; full -> malloc double the capacity |
| 0x00460bc0 | Anim_BuildRunningInstanceSaveRecord | EDX=instance, args (index, extra). Builds 0x68-byte header: args, name 0x20, three target lookup ids (0x004543a0), offset vecs +0x78..+0x90, state +0x98, time + |
| 0x00460f80 | Anim_BuildRunningSaveRecords | for anim nodes 1..[0x00575daa]-1 (0x134 each) whose state byte +0x98 is 2 or 6, with flag 0x1000 clear or 0x2000 set, and [0x004df9b4] set: Anim_BuildRunningIns |
| 0x00461430 | Anim_BuildNodeSaveRecords | Save writer: 0x004c6100, then for anim nodes 1..[0x00575daa) (stride 0x134 in [0x00575dac]) skipping state 5 and nodes with 0x1000 set but 0x2000 clear or [0x00 |
| 0x00461800 | EffectCmd_RecordSize | by type [ECX]: 2 -> 0x38, 3 -> 0x48, 4 -> 0x4C, other -> 0x50 |
| 0x00461840 | Effect_ResolveNodeAndCall45d6b0 | bytes read: a = 0x0045ff10(ECX = this+8); b = ClsRecord_FromIndex([this+0x28]); 0x0045d6b0(ECX = a, EDX = b) |
| 0x00461870 | EffectCmd_Dispatch | node = 0x0045ff10 (arguments not visible); missing -> 0. By command type [ECX]: 1 -> 0x0045dc70 with nine dwords from +0x2C..+0x4C; 2 -> 0x0045dde0 with three;  |
| 0x00461970 | Anim_EmitEventRecord | ECX=name string, EDX=gate. Two enables: bit 0x1000 forces bit13 else global [0x004df9b4]; bit 0x400 forces bit11 else [0x004df9b8]; both need flag 0x100 clear a |
| 0x00461a90 | EffectList_DecCount | [0x0053a2e0]-- |
| 0x00461aa0 | Effect_QueueSpawnRecord | (name ECX, distance-check EDX, three values arg1..3): index = 0x004543a0(); flag 0x1000 of [ECX+0x94] -> use bit 13 as 'record it', else record when [0x004df9b4 |
| 0x00461ba0 | Anim_EmitAttachRecord | Like 0x00461970 but two name lookups via 0x004543a0 (EDX gate and arg1 gate). Enables: bit 0x1000 (test ah,0x10) else computed; bit 0x400 forces bit11. Alloc vi |
| 0x00461d00 | Anim_EmitAttachVelRecord | Three 0x004543a0 name lookups (EDX, arg1, arg3 gates), globals [0x004df9b4]/[0x004df9b8] enables with 0x1000/0x400 overrides; alloc via 0x00460ae0 type 4, strnc |
| 0x00461eb0 | SetGlobals_0053a2e4 | [0x0053a2e4] = ECX; [0x0053a2e8] = EDX << 24 |
| 0x00461ec0 | AnimNode_FindInTree | recursive search from a node (NodeRegistry lookup 0x00447f00 first; operands not visible): returns the first non-null match, otherwise recurses over the child l |
| 0x00461f00 | Effect_StartById | 73 bytes, plain ret, decomp: when [0x00575a40] != 0 and id ECX != -1: 0x004620d0 allocates an instance; if one is returned, 0x00461f50 sets it up, gwNodeSetActi |
| 0x00461f50 | Effect_InitSmokeDefaults | sets the effect defaults: [+0x14] 0.3, [+0x18] 6.0, [+0x1C] 6.0, [+0x20] 30.0, [+0x24] 3.75, [+0x28] 3.0, [+0x2C] 10.5, [+0x30] 1.0, [+0x34] 0, [+0x38] 5.0, [+0 |
| 0x00462050 | Effect_SquaredDistanceToObject | Anim_ResolveObjectWorldPosition (arguments not visible) then the squared distance from that position to the xyz at [ECX+0..8] |
| 0x004620d0 | EffectConfig_ResolveOrParse | index ECX == -1 -> 0; cached lookup 0x0048cc20 hit -> counter [0x00575a64]++, 0x0048ca70 and return it; miss -> counter [0x00575a5c]++ and parse via 0x00462130  |
| 0x00462130 | EffectConfig_InstantiateEntry | index -1 -> 0; entry = [0x00575a50] + index*0x48 with a null field +0xC -> 0; malloc(0x48) copy of the entry (unchecked); +0xC = 0x00452500([0x004dfaf0]) (0 ->  |
| 0x00462280 | Effect_FindTemplateByName | 108 bytes, plain ret, decomp: linear, case-sensitive inlined strcmp of ECX against the name pointer at +8 of each 0x48-byte template in [0x00575a50] (count [0x0 |
| 0x004622f0 | zerr_old_Report | Error-report wrapper: push 0x00575de0 (pre-formatted message buffer), push 0x23, push 'D:/Proj/GameZRecoil/zError/zerr_old.c' (0x004dfaf4), push ECX; call 0x004 |
| 0x00462310 | SetGlobals_0053a2f0 | [0x0053a2f0] = ECX, [0x0053a2f4] = EDX, [0x0053a2f8] = 0; return 0 |
| 0x00462330 | EffectCmd_InitWithName | bytes read: (this ECX, name arg1, value arg2; ret 8): [this+0x2C] = _strdup(name) (unchecked), [this+8] = value, [this] = 0; returns this |
| 0x00462370 | MciVideo_OpenAndPlay | ECX=video desc. 0x004a7d70 then MCI_OPEN (0x803, flags 0x2202) type string MPEGVideo [0x004dfb1c], device id stored ECX+4; MCI_WINDOW 0x841 flags 0x10002; if de |
| 0x00462540 | EffectCmd_SetVec1C | copies four dwords from the argument to [ECX+0x1C..0x28]; flags [ECX] /= 0x40000 |

## Function index additions (2026-09-24)
- **`CallOptionalHook_0056b568` `0x004a5b20`** calls `[0x0056b568](ECX)` when that is set, else
  returns 0. The hook is `ZLocGetID` from `MESSAGES.DLL` (`Messages_LoadDll`).

