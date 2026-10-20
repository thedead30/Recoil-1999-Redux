# player - player vehicle (Battlesport player.cpp) (spec)

Gate OPEN 2026-09-24, 67 members. Key facts: player vehicle name bft_00, entries in list 0x004f3a7c (0x28 bytes + 0x10c4 vehicle block); config vehicle.zrd/_easy/_hard; 12 collide%02d points and 4 support%02d points per model; sweep collision classifies hits into 6 lists (+0x4b8 checkpoints, +0x4a8 pickups, +0x478 vehicles, +0x4c8 state objects, +0x488 normal y < -0.9, +0x498 -0.9..0.71); surface hit response restitution class+0x254, yaw kick class+0x258, min speed 10.0; motion block velocity = normal*20.0; vehicle ram k = class+0x21c*other+0x220, AI damage 1.1k above health ratio 0.2; mode transitions 1..5 fly/sub/track/hover/amphib with 1.0 s cooldown, sub dive -3.0 speed, -4.1 height; pitch clamp +-30 deg on ground snap; race laps via checkpoint flags v+0x1018; save records 0x140 player and 0x80 per vehicle.

All rows CONFIRMED-BINARY at the cited address (bytes read; method in ledger).

| addr | name | notes |
|---|---|---|
| 0x0041eb00 | Player_StaticInit_Construct_004f3778 | ECX=0x004f3778; jmp 0x0041eb30 (constructor). |
| 0x0041eb10 | Player_StaticInit_RegisterDtor_0041eb20 | atexit-style 0x004c60e0(0x0041eb20). |
| 0x0041eb60 | Player_StaticInit_Construct_004f3650 | ECX=0x004f3650; jmp 0x0041eb90 (constructor). |
| 0x0041eb70 | Player_StaticInit_RegisterDtor_0041eb80 | atexit-style 0x004c60e0(0x0041eb80). |
| 0x0041ec40 | Player_StaticInit_Construct_004f37b0 | 0x004ba740(0,0,0) with ECX=0x004f37b0 (widget ctor). |
| 0x0041ec60 | Player_StaticInit_RegisterDtor_0041ec70 | atexit-style 0x004c60e0(0x0041ec70). |
| 0x0041ec90 | Player_StaticInit_Construct_004f33a8 | 0x004ba740(0,0,0) with ECX=0x004f33a8 (widget ctor). |
| 0x0041ecb0 | Player_StaticInit_RegisterDtor_0041ecc0 | atexit-style 0x004c60e0(0x0041ecc0). |
| 0x0041ecd0 | Player_RecordNodeState | ECX=node. Builds entry {node, a = 0x00448140(node), b = 0x004481f0(node), c = 0x00448270(node)} and appends it to vector [0x004f3a5c..0x004f3a60) cap [0x004f3a64] (16-byte elements); when full grows to max(2n,1) via oper |
| 0x0041ef40 | Player_ResetGlobals_004f3a58 | [0x004f3a58] (byte) = high byte of ECX (push ecx; mov al,[esp+3]); zeroes [0x004f3a5c], [0x004f3a60], [0x004f3a64]. |
| 0x0041ef60 | Player_StaticInit_RegisterDtor_0041ef70 | atexit-style 0x004c60e0(0x0041ef70). |
| 0x0041efa0 | Player_ApplyNodeStates | For entries [0x004f3a5c .. 0x004f3a60) stride 0x10 {node, a, b, c}: a -> 0x00448100(node, 1), b -> 0x004481b0(node, 1), c -> 0x00448230(node, 1). |
| 0x0041f010 | Player_FillSaveRecordFields | ECX=record. Player vehicle v = [[0x004f36a4]+4], camera obj = [[[0x004f36a4]+8]+4]. rec[0]=0x140 (size). 10 weapon slots from v+0x5ec (stride 0xac): slot id and two sub-entries (stride 0x54) each (bit2 of flags, value).  |
| 0x0041f1d0 | Player_RestoreSaveRecord | ECX=record (see Player_FillSaveRecordFields). Size tag must be 0x140 (full) or 0x124 (older, no trailing block); else logs Player save data structure has b... (player.cpp line 0xd1) and returns. Player vehicle v=[[0x004f |
| 0x0041f5b0 | Player_RegisterSaveHandlers | [0x004f36a0] = 0. Registers save handlers via 0x004bffe0 (ECX=name, EDX=writer, stack reader, order, 0): name 0x004dc2d4 writer 0x0041f6a0 reader 0x0041f850 order 100; name 0x004dacd8 writer 0x0041f5f0 (Player_BuildSaveR |
| 0x0041f5f0 | Player_BuildSaveRecord | Fills a 0x140-byte record (Player_FillSaveRecordFields), stores [0x004f3718] at record+0x120, writes it via 0x004c0010(ECX=file, EDX=record name = player vehicle [[0x004f36a4]+4]+0xed0, 0x140). |
| 0x0041f6a0 | Vehicle_BuildListSaveRecord | For each player entry in [0x004f3a7c] (next +0; stops on write failure): 0x80-byte record {size 0x80, v+0x3bc..+0x3c4 (3), v+0x3ec..+0x3f4 origin (3), v+0xf70, v+0xf84..+0xf8c (3), v+0xfb0..+0xfcc (8), v+0xfd4..+0xfe0 (4 |
| 0x0041fb80 | Players_FreeAll | Destroys players: while list [0x004f3a7c] non-empty 0x0041fd20; frees linked lists [0x004f3344] (next +8), [0x004f3a7c] (0x00438430 per node), [0x004f3a6c] (each node's sub-list at +0x3ac freed and its fields +0x3a8..+0x |
| 0x0041fd20 | Player_Destroy | ECX=player entry (vehicle v=[ECX+4]). 0x00438b60, 0x004b2630. Frees the vehicle's name-registry node [v+0xed0]+0x40 from list [0x004f3344] (next +8, tail [0x004f3348], count [0x004f334c]). Unlinks the entry from player l |
| 0x0041fe40 | Player_AivConfigName | Returns string aiv.zrd (0x004dc32c). |
| 0x0041fe50 | Player_VehicleConfigForDifficulty | Difficulty Settings_GetGameIntensity(): 0 -> vehicle_easy.zrd (0x004dc334), 2 -> vehicle_hard.zrd (0x004dc348), else vehicle.zrd (0x004dc35c); falls back to vehicle.zrd when Reader_OpenWithSearch(name, EDX) fails. |
| 0x0041fe90 | Player_InitPhysicsGlobalsAndVehicleClasses | Registers HUD/UI objects, creates player scene objects, then reads physics/camera globals from the player config with defaults (all defaults read as immediates in bytes): camera_zone z>0 -> [0x004f36f0]=z, [0x004f36f4]=1 |
| 0x00420be0 | Movers_LoadFromConfig | Opens movers.zrd (0x004dc4d4); for each listed node name found (type 6 lookup): Node_OrFlags28Recursive(1) and Node_SetOwnerAndFlagsRecursive(owner=node, 0x200000); remembers last in [0x004f3ab4]. Closes config. |
| 0x00420c60 | Movers_FlagNumberedNodes | SEH frame, local CString. For i = 1..[0x004f312c]: formats node name (fmt 0x004dc4e0 with i) and looks it up (type 6); if found: Node_OrFlags28Recursive(2), Node_OrFlags24Recursive(0x40000), Node_SetOwnerAndFlagsRecursiv |
| 0x00420d10 | Vehicle_ResolveClassConfigAndNetAssignment | ECX=vehicle entry, arg class name. Finds master common data for the class in list [0x004f3a6c] (missing: Cannot find Master Common Data for %s, player.cpp line 0x46d). Assigns player index [0x004f3a94]++ (index 1 becomes |
| 0x00421470 | Vehicle_BindModelParts | ECX=player entry, EDX=vehicle part record; args (unit name, model name). 0x0042aa40 twice. Finds master model data in list [0x004f368c] (next +0) whose class name (+4) matches the vehicle class and model name (+0x54) mat |
| 0x00421790 | Player_ResetVehiclePhysicsState | ECX=player entry; v=[ECX+4], class c=[[ECX+8]+4]. 0x0042aa40 twice; v+0x448=0, v+0x64=0, v+0xc (gravity) = nom_gravity [0x004f3ac8], v+0xd58=0, v+0xd5c=0. For i < c+0x214 point count: v+0x1a8+i*0xc (3 floats) = class poi |
| 0x00421830 | Vehicle_SnapToGround | ECX=vehicle entry, EDX=align flag; v=[ECX+4]. Light zone: 0x00476320, [0x0057da28] = v+0x444; 0x00448330, 0x00448100. Casts a vertical probe from (x v+0x3ec, 500.0, z v+0x3f4) (0x00443d20) and picks ground with Terrain_F |
| 0x00421a40 | Player_InstantiateVehicleModel | ECX=node name, EDX=target. Looks up template (NodeRegistry_LookupByName type 6); creates scene object 0x00452500 (network flag passed); attaches it under [0x004f36b8] (0x004510e0) and names/links it (0x00447dc0); on succ |
| 0x00421ab0 | Vehicle_SpawnInstanceFromConfig | ECX=spawn position (nullable), EDX=aiv type_id (stored at v+0xf70 = net patrol index per existing plate comment), args (yaw deg, class key, instance name). Name == bft_00 (0x004dc52c) marks the local player vehicle. Unle |
| 0x00421d60 | Node_AndFlags28Recursive | node+0x28 &= EDX; recurses children (+0x60, count +0x5c). |
| 0x00421da0 | Node_OrFlags28Recursive | node+0x28 /= EDX; recurses children (+0x60, count +0x5c). |
| 0x00421de0 | Node_OrFlags24Recursive | node+0x24 /= EDX; recurses children (+0x60, count +0x5c). |
| 0x00421e20 | Path_GetDirectoryOfFound | Resolves file via Reader_OpenWithSearch-style lookup 0x0048cd40, _fullpath (0x104), _splitpath, and writes drive+dir (%s%s, 0x004dc6c8) into EDX. |
| 0x00421ea0 | Player_SpawnVehicleGetList | Vehicle_SpawnInstanceFromConfig(ECX_in, EDX=0, push EDX_in, arg1, arg2 order per bytes); returns [0x004f3a80] (player list tail) when the spawn succeeded, else 0. ret 8. |
| 0x00421ed0 | VehicleClass_ReadCollidePoints | ECX=player entry, EDX=model node; class c=[[ECX+8]+4]. For i = 0..11 finds child collide%02d (0x004dc6d0) (missing -> 0), reads its position (0x0044e270) and deactivates it. Stores the 12 points (3 floats each) into c+0x |
| 0x004220f0 | Vehicle_ReadSupportPoints | ECX=player entry, EDX=model node. For i = 0..3 finds child support%02d (0x004dc6dc) under EDX (0x00452770); missing -> 0. Reads its position (0x0044e270) into vehicle [[ECX+8]+4]+0x1b4 + i*0xc (3 floats) and deactivates  |
| 0x00422170 | VehicleClass_LoadCommonMode | ECX=vehicle class, EDX=config node, arg class name. Reads section common_mode (0x004dc388): nanite -> +0x2d8; sounds/{weapon_up +0x2e4, weapon_select +0x2ec, pinging +0x2f0} via 0x004a0990; activation +0x2f4; not_pursuit |
| 0x004226d0 | Vehicle_LoadClassPhysicsConfig | recoil_re_log.md:2172;recoil_re_log.md:2270;recoil_re_log.md:5860;recoil_re_log.md:5923 |
| 0x00423150 | Name_StripNumericSuffix | Copies ECX string into EDX up to (not including) the first '_' that is followed by a digit (isdigit); EDX NUL-terminated. |
| 0x004231b0 | Hud_RefreshFromPlayer | ECX=player entry, v = [ECX+4]. Hud_SetHealthBar(v+0xf34 / class max [[[0x004f3a88]+4]+4]+0x398); Hud_ForwardToGlobal_004e62f0. For 10 weapon slots (v+0x624, stride 0xac) and its two fire groups (flags bit2 at slot-8 and  |
| 0x00423380 | Vehicle_IsMovementType9BCD | Returns true for ECX in {9, 0xb, 0xc, 0xd}. Meaning of this type group not established (INFERRED name avoided). |
| 0x004233b0 | Array10_CopyRange | stdcall(first, last, dest) ret 0xc: copies 16-byte elements [first,last) to dest (skips writes when dest null); returns dest end. |
| 0x00423400 | Array10_FillCopy | stdcall(dst, count, src) ret 0xc: for count 16-byte elements copies 4 dwords from src into each non-null dst slot. |
| 0x00423460 | Vehicle_UpdateControllers | ECX=vehicle entry, v=[ECX+4]. Clears active flags v+0x4d8..+0x4e8; 0x00423530; 0x004236b0. If v+0x4d4 == 0 runs each present controller and sets its flag: +0x4c0 -> 0x00425150(EDX=[0x004f3a98]), flag +0x4e8; +0x4b0 -> 0x |
| 0x00423530 | Vehicle_ClearControllerLists | ECX=vehicle entry, v=[ECX+4]. Frees six per-controller linked lists (contents not yet identified) (nodes next +0x44, operator delete) and zeroes each 4-dword header: +0x474..+0x480, +0x484..+0x490, +0x494..+0x4a0, +0x4a4 |
| 0x004236b0 | Vehicle_SweepCollisionPoints | ECX=vehicle entry; v=[ECX+4], class c=[[ECX+8]+4]. Speed^2 = /v+0xb0..+0xb8/^2; v+0x4d4 = 1. Builds a 12-entry active mask per collision point: player vehicle with /v+0x94/ > 0.0, or AI with /v+0x94/*3.3 > /v+0xb8/ -> po |
| 0x00423b10 | Vehicle_TestSweepSegments | Large stack frame (0x004c6100 probe). ECX=vehicle entry, EDX=segment array (0x18 bytes each: start, end), stack count and per-segment point indices. Temporarily disables the vehicle's own collision (0x004481b0), sets lig |
| 0x00423c20 | Vehicle_ClassifySweepHit | ECX=vehicle entry, EDX=hit list {count, hits 0x28 bytes: +8 normal y, +0x24 surface, +0x28 object}, args (segment start, end, point index). For each hit allocates a 0x48-byte node (operator new 0x48; 10 hit dwords, then  |
| 0x00423fc0 | VehicleCtrl_HandleList498 | v=[ECX+4]: walks list v+0x498 (next +0x44) calling 0x00424010 per node, then 0x00424270(EDX=last), v+0x4e4 = 1. |
| 0x00424010 | ListNode_PickByXZDot | ECX, EDX = two list nodes {+0 normal vec3, +0xc point, +0x34 point}. For each computes d = (+0x34) - (+0xc) and the XZ dot of d with the node's +0 vector; returns ECX when ECX's value is smaller than EDX's (FCOMPP on the |
| 0x00424110 | VehicleCtrl_HandleList488 | ECX=vehicle entry, v=[ECX+4]. 0x004248e0(EDX=v+0x488 list); if v+0x60 state == 1: 0x004385f0(4, 1.0f) on the entry; then 0x00424270(EDX=v+0x488). |
| 0x00424150 | Pickup_HasLineOfSight | Local player light zone [0x0057da28] = v+0x444; temporarily disables collision on the pickup (0x004481b0, 0x00443c50/0x00443c60) and runs Collision_GridDDATraversal from pickup (+0xc, +0x10 + 1.0, +0x14) to the vehicle o |
| 0x00424210 | VehicleCtrl_CollectPickups | Only for the local player [0x004f3a88] with state v+0x60 not 4/5: for each node in list v+0x4a8 (next +0x44) where 0x00424150(node) passes, calls Pickup_OnTouch(ECX=node+0x24 object, EDX=player entry). |
| 0x00424270 | Vehicle_ResolveSurfaceHit | ECX=vehicle entry, EDX=hit node (normal +0..+8, hit point +0xc.., segment start +0x28.., end +0x34..); class c. Orients the normal against the segment direction (flip by -1.0). If the normal has a horizontal component: s |
| 0x004248e0 | VehicleCtrl_HandleList488_Collide | v=[ECX+4], EDX = list v+0x488 head. If v+0x10 set and rising (v+0xa8 > 0): flips vertical velocity (v+0xa8 and v+0xb4 = -old), resets height v+0x3f0 to saved v+0x318 minus 0.2 per list node, matrix translation y updated; |
| 0x00424ac0 | Vehicle_ResolveVehicleContact | ECX=vehicle entry; first node of list v+0x478 copied (10 dwords); other vehicle = node object owner ([node+0x40]+4 ...). k = class +0x21c * other class +0x220. Adds k * own velocity (v+0xa4, 0, v+0xac; vertical zeroed) t |
| 0x00424bf0 | Vec3_EnforceMinLength | ECX=vec3. If /v/^2 < 0.01 and non-zero: scales v to length 0.2 ([0x004dc96c] / approx sqrt(/v/^2) via exponent-halving trick + 1e-8), returns 1; zero vector or already long enough returns 0. |
| 0x00424c90 | Segment_ClipToHit | ECX=end point, EDX=start point: delta = end - start; if 0x00424bf0(&delta) reports a hit (delta shortened), end = start + delta. |
| 0x00424d00 | VehicleCtrl_HandleList4c8 | v=[ECX+4], class c=[[ECX+8]+4]. For each node of list v+0x4c8 (next +0x44): p = 0x004b2880((/v+0xb0..+0xb8/^2 * 5.0) / (c+0xac)^2) (constants 5.0 @0x004d074c); if p > 0.0 the node is unlinked from the list (count +0x4d0, |
| 0x00424ed0 | Vehicle_TestStationaryPoints | ECX=vehicle entry, v=[ECX+4]. Clears controller lists (Vehicle_ClearControllerLists); builds 6 segments from the first 6 collision points (previous pos v+0x1a8.. to current v+0xf4..) with indices 0..5 and runs Vehicle_Te |
| 0x00425060 | Node_GetNumericSuffixIfFlagged | If node +0x24 has 0x200000 and name [+0x40] byte +0x28 has bit 1: takes the name's right-hand part (CString::Right) and returns atol of it clamped to >= 0; else 0. SEH frame. |
| 0x00425150 | Race_OnCheckpointHit | ECX=vehicle entry, EDX=checkpoint index; v=[ECX+4]. Sets hit flag v+0x1018+idx*4 if unset. When idx == checkpoint count [0x004f312c] (finish): lap valid only if all flags from v+0x101c were set; clears them. Valid lap: v |
| 0x004251f0 | Vehicle_TestFootprintAtHeight | ECX=vehicle entry, arg height offset (ret 4). Clears controller lists; takes 4 class collide points (c+0x100, c+0x118, c+0x124, c+0x13c) raised by the offset, transforms them by the vehicle matrix (v+0x260..+0x28c) into  |
| 0x00425770 | Vehicle_BlockMotion | v=[ECX+4]. If v+0x4ec == 0: restores origin v+0x3ec..+0x3f4 from saved v+0x314..+0x31c and yaw v+0x3c0 from v+0x3d8, rebuilds matrix 0x00474260(pitch, yaw, roll) and v+0x284..+0x28c translation. Takes the first node of l |
| 0x00425920 | Player_RegisterResources | SEH frame. Registers resource-group entries via 0x004717c0 (10 calls, one skipped when Settings_GetHWCardFlag is set); allocates a 0x1c-byte object (0x0042f9f0) into [0x004f36b4] (0 on failure). |
| 0x00425a20 | Vehicle_ReadPlayerControlInput | recoil_re_log.md:2119;recoil_re_log.md:2270;recoil_confirmed_logic.md:923;recoil_confirmed_logic.md:996 |

## Function index additions (2026-09-24)
- **`Player_ResetForDeathAnim` `0x0041bb30`:**
  - resets the node and starts the death callback (`0x00438020(cb, 0, 1.0, 5.0)`);
  - zeroes a named sub-node's transform;
  - stops damage over time and clears the fire state.
- **`Player_RespawnReset` `0x0041bbf0`:**
  - stops damage over time and clears the fire state;
  - sets hp = `cls+0x398` (full);
  - resets the node and starts the callback (`1.0, 1.0`);
  - zeroes `+0x30..+0x3c`;
  - switches to TRACK mode (`Vehicle_TransitionToTrackMode`).
- **`Player_ApplyHealthPickup` `0x0043b660`:**
  - `EDX == 0` refills to class maximum (messages 0x902 and 0x246, 5 s each);
  - otherwise adds the amount.
- **`NameRegistry_Add` `0x00438920`** appends `{a, b}` to list `[0x004f3344]`. The HUD target
  markers walk this list (`Hud_UpdateTargetMarkers`).

