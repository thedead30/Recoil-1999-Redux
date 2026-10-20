# mission - mission flow (Battlesport mission.cpp) (spec)

Gate OPEN 2026-09-23, 39 members (Mission class on map-screen base, global 0x004f0cc0). Key facts: load order Mission_Load 0x00417810 (support\initm%d.gw, m%d.gs / m%d_zbd.gs, nodes world1/camera1/window1/display) then Mission_InitGameplay 0x00417a00; objectives OBJECTIVE1..10 in slots of 0x31c (READ_TIME/REVIEW_DELAY default 4.0, FINAL_MISSION, AUTOPLAY, ACTIVE/INACTIVE node paths); state machine phases 0x64/0x67/0x68/0x69/0x6b (0x004184e0); 0x5c save record; weather MISSION%d SNOW/RAIN; review screen msgs 0x116-0x118. Player physics defaults (callee 0x0041fe90): nom_gravity 28.0, wat = nom/3, qsd = nom/6, max_slope 0.707, qsand_sink 0.9, lava_sink 0.6.

All rows CONFIRMED-BINARY at the cited address (bytes read; method in ledger).

| addr | name | notes |
|---|---|---|
| 0x00417260 | Mission_LoadMapScreen | sprintf .\maps\m%d.zmap [0x004daf84] with arg; result = 0x004169d0(path). Loads sounds snd_mapOn -> +0x5c, snd_mapOff -> +0x60, snd_mapClick -> +0x64 via 0x004a0990. Returns map result; ret 4. |
| 0x004172c0 | MapScreen_SetMarkerStateById | Walks marker list head +0x40 (next at +0); for each node with id node[4]==arg1: 0x00415b70(arg3) and 0x00415ae0(arg2) with ECX=node. Returns 1; ret 0xc. |
| 0x00417300 | MapScreen_SetMarkerLabelById | Walks marker list +0x40; for id match: 0x00415b70(arg2) then swaps the 16-bit halves of node+0x20 (rol 16). Returns 1; ret 8. |
| 0x00417360 | Mission_StaticInit_Construct | mov ecx,0x004f0cc0 (global Mission instance); jmp Mission_Construct 0x00417390. |
| 0x00417370 | Mission_StaticInit_RegisterDtor | atexit-style: 0x004c60e0(0x00417380) registering the global Mission destructor thunk (0x00417380 jmps to Mission_Destruct 0x00419490). |
| 0x00417390 | Mission_Construct | SEH frame. Base ctor 0x00416650 (map-screen base), constructs CStrings +0xe4/+0xe8/+0xec, +0x2478=0 (weather), +0xd4=1.0f, +0x2468=0, +0x2484=0, +0x25cc=0, then Mission_ResetState. Returns this. |
| 0x00417430 | Mission_BuildSaveRecord | Builds 0x5c-byte record: +0xdc, +0xe0, +0x108, +0x114, +0x124, +0x134, +0x138, +0x2454..+0x2464, the 10 objective flags (this+0x53c stride 0x31c), and 0x00407f20() result; writes via 0x004c0010 with EDX=name 0x004daf9c,  |
| 0x004174f0 | Mission_RestoreSaveRecord | Load reader for 0x5c record (param_3=data). +0x25cc=1, +0x138=d[2], Settings_StoreGameIntensity. If current mission +0xdc==0: Mission_SetOutcomeWithParam(d[0],d[1]), 0x004c0070, return. If d[0] differs: 0x00417d40, 0x004 |
| 0x00417640 | Mission_RegisterSaveHandlers | Registers two save-record handlers via 0x004bffe0 (ECX=name, EDX=writer, stack: reader, order, this): name 0x004dafc4 writer 0x00417680 reader 0x00417690 order 0; name 0x004dafb8 writer 0x004176b0 (Mission_BuildLatePhase |
| 0x004176b0 | Mission_BuildLatePhaseFlag | recoil_re_log.md:1448;recoil_re_log.md:1473;recoil_re_log.md:1476;recoil_confirmed_logic.md:783 |
| 0x004176f0 | Mission_ResetState | Zeroes +0xd8 and +0xdc (outcome), empties CStrings +0xe4/+0xe8, +0xf0=0, +0xe0=1, objective count +0x12c=0. If widget +0x2478: calls its vtable+0x60(0), removes from UI list via 0x004bc860 (ECX=0x0056bd58), deletes via v |
| 0x00417770 | Mission_SetOutcomeWithParam | ECX+0xe0 = arg2, ECX+0xdc = arg1; if arg1 nonzero empties CString +0xe8. Returns 1; ret 8. |
| 0x004177a0 | Mission_SetOutcome | ECX+0xdc = arg; if nonzero empties CString at ECX+0xe8 (reason text). Returns 1; ret 4. |
| 0x004177d0 | Mission_SetOutcomeText | If arg non-null assigns CString +0xe8 = arg, else empties it. Returns 1; ret 4. |
| 0x00417800 | Mission_GetOutcome | bytes read 0x00417800 len 7 |
| 0x00417810 | Mission_Load | SEH frame + local CString. 0x0046eba0; if mission id +0xdc==0 sets 1. +0x2468 = 0x004193c0(). Formats support/initm%d.gw [0x004db034] and runs it via 0x004c1500; 0x00451900; 0x00475e70. If script-name CString +0xe8 empty |
| 0x00417a00 | Mission_InitGameplay | Zeroes stats +0x2454..+0x2464, [0x004f3764], [0x00779aa0]; +0x2480 = -1.0f. 0x00417260(mission id); +0x4c = world node +0xf0. 0x004717c0 calls (4 extra in single player), 0x0041ccf0. Staged with progress (0x004a5bf0+0x00 |
| 0x00417d40 | Mission_Unload | If loaded flag +0xd8==0 returns 1. Otherwise tears down the level in fixed order, logging stage labels via 0x00414180 (ECX=label strings 0x004db15c, 0x004db14c, 0x004db138, 0x004db128, 0x004db118, 0x004db108, 0x004db0f4, |
| 0x00417ee0 | MissionObjectives_Clear | If not network game (Settings_GetNetworkFlag==0): resets each objective slot (MissionObjectives_Reset on this+0x53c+i*0x31c for i<count +0x12c), then +0x114=-1, +0x124=0, +0x12c=0, +0x138=0. If +0x120 non-null frees via  |
| 0x00417f60 | MissionObjectives_Reset | ECX=objective block: clears bytes +0x14, +0x114, +0x214 and dword +0; if +0x10 non-null frees via 0x0046d5a0 and zeroes it. |
| 0x00417f90 | Mission_LoadObjectivesArray | Opens detail config (fail: Failed to read, mission.cpp line 0x2c7, returns 1); keeps handle at +0x120. Loads image dir ../data/m%d/images/ [0x004db21c] via 0x0046ebd0. Defaults: +0x104 read time 4.0f, +0x11c review delay |
| 0x00418230 | Mission_LoadObjectiveTriggers | From config handle +0x120: REVIEW_SOUND -> sound via 0x004a0990 at +0x118. Single player: for OBJECTIVE%d (from 1, slots stride 0x31c starting this+0x31c): ACTIVE node name -> NodeRegistry_LookupByName, descending path c |
| 0x004184e0 | Mission_ObjectiveStateMachine | Phase +0x108. If not 0x6b/0x67/100: plays sound (1.0f); outcome +0xd0 != 1 -> Mission_ShowObjectiveBriefing(1); else (if +0x10c then 0x0049fec0) Mission_ShowObjectiveBriefing(0). Otherwise: 0x00411760; if mission +0xdc== |
| 0x00418620 | Mission_ShowObjectiveBriefing | +0x108=0x65. arg==0: outcome +0xd0=0, 0x00411a20, 0x004a1090([0x004f0d94]), 0x004a10d0(ECX=1). arg!=0: +0xd0=1; if current index +0x124 < count +0x12c calls 0x00411900 (ECX=slot+0x54c value, EDX=slot+0x550, push slot+0x6 |
| 0x004186f0 | MissionObjectives_GetSlot | Slot = this + idx*0x31c: out1 = slot+0x550, out2 = slot+0x650, out3 = dword slot+0x54c. Returns 1; ret 0x10. |
| 0x00418730 | Mission_OnReviewConfirm | Plays sound with ECX=[this+0x118], volume 1.0f (0x3f800000) via Sound_PlayResourceAuto; then Mission_ShowReviewScreen(this+0xd0 != 2). |
| 0x00418760 | Mission_ShowReviewScreen | arg==0: outcome +0xd0=0, 0x00411a20, return. Else +0xd0=2; builds three lines via GetMessageByID (buf 0x40): msg 0x116 (+0x138, objective count +0x12c, percent = ftol(([0x00779aa0] / +0x245c if +0x245c>0 else 1.0f@0x004c |
| 0x004188f0 | Mission_OnReviewContinue | Plays sound ECX=[this+0x118] vol 1.0f; arg0 = 0 if outcome +0xd0 is 3 or 4 else 1; calls 0x00418940(arg0, 0, **([[0x004f3a88]+4]+0x5e4)). |
| 0x00418940 | Mission_ShowWeaponInfo | arg1==0: outcome +0xd0=0, 0x00411a20. Else builds weapon info text for weapon def param_3: starts with Features: [0x004db3ec], appends by flags dword def+0x54: bit19 Remote, bit21 Thermal, bit16 Multi, bit20 Tether else  |
| 0x00418c30 | MissionObjectives_FindFirstClear | Scans objective slots (count +0x12c, flag at +0x53c, stride 0x31c) for first slot whose flag is 0; if found calls 0x004172c0(idx,1,string 0x004daf98). Returns index (== count if none). |
| 0x00418c70 | Mission_BeginPlay | +0x100=0; 0x00411760. Network game: 0x00410ed0, if 0x00408340() then 0x00408360/0x004136b0(ECX=result)/0x00410e90; 0x004138d0(5.0f) on HUD object [[0x004f3a88]+4]+0x5e4; 0x0040eca0(ECX=1); 0x00413630; Input_Mouse_CursorR |
| 0x00418d40 | Mission_TickAndCheckObjectiveCompletion | count=4, base +0x53c, stride 0x31c, frcoff/nb_force node names and flag states all verified live |
| 0x00418fb0 | Mission_AdvanceToNext | If +0x247c==0: Player_FillSaveRecordFields(ECX=this+0x2488) using player [[0x004f36a4]+4]; +0x25c8 = player+0xf38; +0x2484=1; returns Mission_Start(+0xdc + 1). Else sets [0x004e5dec]=1, returns 0. |
| 0x00419010 | Mission_Start | Clears [0x004f3ea8]; calls 0x0042ee40(arg) with ECX=0x004f3e80; then 0x00443160(0x004f3e80,0) with ECX=0x004f3ca8. Returns 1; ret 4. |
| 0x00419050 | Mission_LoadWeather | Opens detail config (fail: Failed to read, mission.cpp line 0x5f6). Finds section MISSION%d [0x004db454] by mission id +0xdc; PARTICLES count (default 100) ; TYPE compared to SNOW -> new 0x98 object via 0x004be280(count) |
| 0x004192d0 | Mission_StopConfiguredAnims | Two args (ret 8). If not network game: opens config via Settings_OpenDetailPresetFile (fail: log Failed to read, mission.cpp line 0x646); finds child list; for each child name (+0xc) resolves anim via 0x0045ff10 and, if  |
| 0x004193c0 | Mission_LoadRaceCheckpoints | Formats ..\data\m%d\zrdr [0x004db478] with mission id, opens race.zrd [0x004db46c] via Settings_OpenDetailPresetFile; if cp_count [0x004db460] present: +0x2474 = 20.0f (0x41a00000), +0x246c = count. Closes 0x0048ce40. Re |
| 0x00419470 | Mission_SetRaceTiming | +0x2470 = arg2, +0x2474 = arg1; ret 8. |
| 0x00419490 | Mission_Destruct | SEH frame. Destroys CStrings +0xec, +0xe8, +0xe4 then base dtor 0x004167a0 (via thunk). |

## Function index additions (2026-09-24)
- **Mission HUD.** `MissionHud_Reset` `0x00419650` resets the HUD composite and the message
  panel, and frees the image `+0xabe4`.
- **Panel destructors.** `MissionPanel_Dtor` `0x00419870` (two buttons + screen base) and
  `MissionPanel_scalar_deleting_dtor` `0x00419850`.
- **`Mission_SetDataSearchPaths` `0x0042ecb0`** (mission id) sets the search paths for `zbd`,
  textures (`..\data\common\textures;..\data\common\effects\...`) and readers
  (`..\data\common\zrdr`).
- **`Global_Set_0057d9a0` `0x00479cb0`** sets the UV-aware collision switch
  (`Collision_UVQueryEnabled`).

