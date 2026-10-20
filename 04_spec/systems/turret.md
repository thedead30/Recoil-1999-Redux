# turret - Turret (spec)

Static turrets: config construct (0x004367a0), activation by Manhattan range, target tracking exp(-3dt), fire at dot>0.89.

All rows CONFIRMED-BINARY at the cited address (bytes read; method in ledger).

| addr | name | notes |
|---|---|---|
| 0x00436630 | Turret_InitializeDefaults | Zeroes the 0x180-byte turret fields, with defaults: +0x68 = -1.0, +0x9c = 50 (0x32), +0xa0 = 200.0, +0xb0 = 1.0, +0xe8/+0xec = 1.0, +0xcc = -1.0, +0x15c/+0x160 = 100.0 (health and max), +0x170 = 1, +0x16c = HUGE (from _H |
| 0x004367a0 | Turret_Construct | thiscall ret 0x10; reads config param_4: HEALTH, PARTS, DEACTIVATE, EFFECT(10.0f), ACTIVATE_ON_HIT, ALWAYS_LOOK_AT, WEAPON{BASE_MOVES->+0x28, DETECTION_RANGE->+0xa0, FIRE_RATE->+0xe8, FIRE_LIMITS->+0xf4, FIRE_DWELL->+0xf |
| 0x00436e00 | Turret_ReleaseSounds | Stops playing sound turret+0x104 (0x004b1f90) if set; 0x004b2630 on turret+0xc. |
| 0x00436e20 | TurretTarget_IsValid | ECX=target ref {[0] object, [2] node}: returns 1 when [0] set and node+0x24 has bit 2 (0x4), else 0. |
| 0x00436e40 | Turret_UpdateAITick | ECX=turret, arg player position. If turret base/barrel/extra nodes are inactive (bit 4 of +0x24): hides muzzle flash node +0x6c, stops continuous fire (+0x108 -> 0x004aefb0); return. Dormant until time +0x16c. Weapon fla |
| 0x00437430 | Turret_ComputeAimOrigin | Aim point +0x1c..+0x24 = base +0x10..+0x18, raised by base node height ([+0x3c]+0x28 when +0x38) and barrel node height ([+0x5c]+0x28 when +0x40); then + (+0x48) when +0xbc set, else when +0xc0 set by the midpoint of +0x |
| 0x004374a0 | Turret_TrackTarget | Direction from barrel node origin ([+0x5c]+0x24..+0x2c, transformed to world) to target (arg), normalized and brought into local space (0x00474670). Unless locked (+0x154): fire flag +0xa8 = 1 when dot(dir, current facin |
| 0x00437730 | Turret_UpdateAimVector | Picks the muzzle point: single barrel (+0xb8 < 2) uses +0x44..+0x4c; else cycles barrel index +0xb4 over +0xb8 barrels (stride 0xc from +0x44). Transforms it to world (zTransformStackPushRef, 0x004732f0, 0x00449480, 0x00 |
| 0x00437820 | Turret_Fire | If no burst sound +0x104: optional hit/lead calc 0x004b0530 when weapon def +0x98 has +0x44 speed (not -1.0 -> 0x0043b500); toggles collision off on the turret node (0x004ae4a0, 0x004481b0 when +0x28), sets [0x00779aac]  |
| 0x00437990 | Turret_TickReload | arg dt (ret 4): if clip size +0xf4 != 0.0: remaining +0xf0 -= dt; when <= 0.0: +0xa8 = 0 (stop firing), +0xf0 = +0xf4, next fire time +0xec = now [0x0056b428] + reload +0xf8. |
| 0x004379f0 | FUN_004379f0 | turret damage chain end-to-end on Recoil21; 6.0->-1.2 by 1.8 (wep_1.1 ERFPG) unscaled; fraction=cur/max |
| 0x00437aa0 | Turret_ModuleReset | [0x004f3fd8] = 0, [0x004f3fe0] = 0. |
| 0x00437ab0 | Turret_ModuleShutdown | Calls Turrets_FreeAll, returns 0. |
| 0x00437ac0 | Turret_SpawnAllFromConfig | Network game -> -1. Opens config (fail: Failed to read, turret.cpp line 0x4ce, -1); keeps it in [0x004f3fd4]. DESTROY_ANIM (0x004dd13c) anim via 0x0045ff10; second anim -> [0x004f41ec]. For each TURRET (0x004dd194) child |
| 0x00437d40 | Turrets_Release | ECX=[0x004f3fd0], EDX=0; tail-jumps 0x00447f30. |
| 0x00437d50 | Turrets_ForEachNode | ECX = turret root [0x004f3fd0], EDX = callback 0x00437ca0; tail-jumps 0x00447f30 (walk). |
| 0x00437d60 | Turret_TakeDamage | turret damage chain end-to-end on Recoil21; 6.0->-1.2 by 1.8 (wep_1.1 ERFPG) unscaled; fraction=cur/max |
| 0x00437dc0 | Turrets_FreeAll | For i < [0x004f3fd8]: 0x00436e00 on turret [0x004f3fe8+i*4], delete, clear. Count 0. If config [0x004f3fd4]: 0x0048ce40, clear. If [0x004f3fd0]: 0x00447f30(EDX=0), 0x00447b60, clear. Returns 0. |
| 0x00437e60 | Node_SetOwnerAndFlagsRecursive | node+0x40 = EDX (owner), node+0x24 /= arg; recurses children (+0x60, count +0x5c). ret 4. |
| 0x00437ea0 | Node_ApplyToMeshesRecursive | Gets node mesh via 0x00447f00 (out local); if non-null calls 0x004826d0(mesh, EDX); recurses children (+0x60, count +0x5c) with the same EDX. |
| 0x00438020 | Fader_Add | ECX=object, EDX=target, args (p1, from, to, duration). operator new 0x20 zeroed node appended to fader list [0x004f41f4]/[0x004f41f8] (next +0x1c, count [0x004f41fc]). +0 = ECX, +4 = EDX, +8 = p1; from and to clamped to  |
| 0x00438180 | TurretList_FreeAll | Frees linked list [0x004f41f4] (next at +0x1c) with operator delete; zeroes [0x004f41f0], [0x004f41f8], [0x004f41f4], [0x004f41fc]. |
| 0x004381d0 | Engine_TweenInterpolationTick | Advances each fader in list [0x004f41f4] (next +0x1c, count [0x004f41fc]): t +0x14 += rate +0x18 * real dt [0x0056b424], clamped to [0,1]; value = t, or 1 - t when mode +0xc == 1, applied via 0x0044dd90(value) on the obj |

## Function index additions (2026-09-24)
- **Turret table.** `TurretTable_Create` `0x00438a90` resolves a named node pair
  (`NamedNodePair_Resolve` `0x00438990`) into `[0x004f4210]`; if either node is missing it is
  discarded. `TurretTable_Free` `0x00438b10` frees it.
- **Global.** `Globals_Set779a84` `0x004b2900` stores `[0x00779a84]`, set from turret, vehicle
  and AI damage paths.

