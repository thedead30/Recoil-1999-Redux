# pickup - pickups (Battlesport pickup.cpp) (spec)

Gate OPEN 2026-09-24, 44 members. Key facts: table base 0x004db6e8, 40 entries x 0x30 (name ptr +0x10, id +8); live list 0x004f3318 of 0x54-byte nodes (next +0x50); respawn queue 0x004f330c of {node, due time}; object names pu_%03d_%02d, type = n/100; config puppies.zrd / puppies_easy.zrd / puppies_hard.zrd by difficulty; types 0..0x21 weapon slot/group pairs, 0x22/0x23 health (refused above 0.99), 0x24 extra life max 3, 0x25..0x27 mode flags, 0x385 all weapons infinite ammo, 0x387 lives 123456789; infinite-ammo sentinel 123456792.0f; messages 0x237/0x23f/0x240; save records 0x30 bytes. Weapon select is input actions 0x0E..0x17 (Player_SelectPrimaryWeapon 0x00439260).

All rows CONFIRMED-BINARY at the cited address (bytes read; method in ledger).

| addr | name | notes |
|---|---|---|
| 0x0041ccd0 | Pickups_ReleaseLists | 0x0041e240 on ECX=0x004f3318 and 0x004f32f8, then 0x0041e270 on 0x004f3308 (tail jump). |
| 0x0041ccf0 | Pickups_Init | [0x004f3328] = ECX. Default sound via 0x004a0990. For 40 (0x28) pickup table entries at 0x004db6f0 (stride 0x30): node pu_%03d (0x004dc1b0) by entry id via NodeRegistry_LookupByName into entry+0x10; entry+0x14 = default  |
| 0x0041ceb0 | Node_ClearPickupFlagsRecursive | node+0x24 &= 0xfffbffe7 (clears bits 3, 4, 18); recurses over children array +0x60 (count +0x5c). Returns 1. |
| 0x0041cef0 | Node_SetPickupFlagsRecursive | node+0x24 = (flags & ~8) / 0x40010; recurses over children +0x60 (count +0x5c). Returns 1. |
| 0x0041cf30 | Pickup_ResolveOwnerNode | ECX = &node; if node has flag 0x40000 in +0x24, replaces *ECX with [[node+0x40]+0x24] and returns 1; else 0. |
| 0x0041cf50 | Pickup_Remove | ECX=announce flag, EDX=pickup object; list node = object+0x40. Deactivates object. If node respawn time +0x30 != 0: network+flag -> 0x00433e70; queues a 0xc respawn entry {node, due = game time [0x0056b430] + delay} on r |
| 0x0041d0c0 | Pickup_OnTouch | ECX=pickup object, EDX=touching player entry (vehicle [EDX+4]). Only for pickup-owned nodes (Pickup_ResolveOwnerNode) with a live list node (Pickup_GetField40): type = object+0x1c; if 0x0041d220 (apply to player) succeed |
| 0x0041d220 | Pickup_ApplyToPlayer | ECX=pickup type, EDX=amount, arg player entry (vehicle v=[arg+4], class c=[v+4]); ret 4. Types 0..0x21 map to weapon slot/group: type t and t+0x11 give slot = t/2+1 (1..9), odd -> secondary group (other=1), even -> prima |
| 0x0041d650 | Pickup_GiveAmmo | ECX=pickup def {[0] weapon flag, [3] default amount}, EDX=message buffer; args (player, slot, group, other group, amount) ret 0x14. Slot = v+0x5ec + slot*0xac; group entries stride 0x54 (+4 base) with def ptr [0], flags  |
| 0x0041d8a0 | PickupList_RemoveNode | ECX=pickup node, EDX=list {head +0, first +4, last +8, count +0xc}; unlinks node (next +0x50), fixing first/last/count; releases its scene object +0x24 (if object+0x58 set: 0x00451240 on it; then 0x00451a60 destroy) and  |
| 0x0041d920 | PickupList_AddNode | ECX=pickup object, EDX=position, args rot (nullable), p2, flag (ret 0xc). If flag: 0x004483f0. malloc 0x54 node: +4 = table entry whose id (entry+8, from 0x004db6f0 up to 0x004dbe70) == object+0x1c; +0 = object serial +0 |
| 0x0041da20 | Pickup_Spawn | ECX=key, EDX=position, args rotation (nullable), p3 (ret 0xc). Creates object (0x0041dab0); registers (0x0041e330); Object3D_SetPosition, optional Object3D_SetRotation; creates list node via 0x0041d920(rot, p3, 1) and co |
| 0x0041dab0 | Pickup_CreateObject | Entry = PickupTable_EntryByIndexSignedLt40(ECX); needs entry+0x18 model and an allocated scene object 0x00452500(EDX=1, 0); names it pu_%03d_%02d (0x004dc1e0) from entry+8 id and entry+0x14 counter; object+0x20 = EDX or  |
| 0x0041db40 | PickupTable_EntryByIndexSignedLt40 | Returns 0x004db6e8 + ECX*0x30 when ECX < 0x28 (signed jl), else 0. Table base 0x004db6e8 (40 entries x 0x30). |
| 0x0041db60 | Pickup_SetupObject | ECX=pickup object. Needs bounding-volume child (0x00452770), else logs Pickup %s has no bvol child (pickup.cpp line 0x152) and returns 0. n = atol(name+2): type = n/100 (fail if > 0x28), object+0x1c = type, +0x18 = seria |
| 0x0041dc30 | PickupSpawnRecord_Spawn | Spawns from record ECX via 0x0041da20 (ECX=[rec+4]+8 table key, EDX=rec+0xc position, push rec+0x18 rotation, rec+0x28); on success copies rec+0x30 into new pickup +0x30. |
| 0x0041dc60 | Pickup_SpawnWithAnim | Creates pickup object via Pickup_CreateObject (0x0041dab0, ECX=key, EDX=0); registers it (0x0041e330); resolves anim 0x0045ff10(ECX=0x004dc224 name); starts anim instance 0x0045dc70 at position EDX[0..2] with zero veloci |
| 0x0041dd60 | PickupTable_IndexOfNameOut | strcmp ECX against table names (0x004db6f8, stride 0x30, 40 entries); found: *out(arg on stack) = entry+8 value (0x004db6f0 + idx*0x30), returns 1; else *out = 0, returns 0. |
| 0x0041ddf0 | Pickups_ConfigFileForDifficulty | Difficulty Settings_GetGameIntensity(): 0 -> puppies_easy.zrd (0x004dc22c), 2 -> puppies_hard.zrd (0x004dc240), else puppies.zrd (0x004dc254); falls back to puppies.zrd if 0x0048cd40(name, EDX) fails. |
| 0x0041de30 | Player_HasWeaponForPickup | Local player vehicle [[0x004f3a88]+4]: over 10 weapon slots (flags at +0x61c, stride 0xac) returns 1 if a fire group id (-0x2c or +0x28) == ECX with bit2 set (own slot flag or +0x54 flag); else 0. |
| 0x0041de70 | Pickups_SpawnAllFromConfig | For each pickup table entry from 0x004dba18 (stride 0x30) with a name: entry+0x28 = 0x004ae3c0(name), entry+0x2c = 0. Clears live list (0x0041e240), 0x0044f6f0. Per config pass: 0x0041ddf0(0), opens config; for each chil |
| 0x0041e1a0 | PickupTable_EntryByName | idx = 0x0041e1e0(); returns 0x004db6e8 + idx*0x30 or 0 when idx < 0. |
| 0x0041e1c0 | PickupTable_EntryByIndex | Returns 0x004db6e8 + ECX*0x30 for 0 <= ECX < 0x28, else 0. |
| 0x0041e1e0 | PickupTable_IndexOfName | strcmp ECX against table entry name pointers at 0x004db6f8 (entry+0x10, stride 0x30) up to 0x004dbe78 (40 entries, null names skipped); returns index or -1. |
| 0x0041e240 | PickupList_Clear | ECX=list: removes every node (next +0x50) via 0x0041d8a0 (EDX=list). If list is the live list 0x004f3318 also clears [0x004f3330]. |
| 0x0041e270 | LinkedList_FreeAll | ECX=list {head +0, first +4, last +8, count +0xc}; frees every node (next +8) with operator delete (call 0x4c5b6a), unlinking and decrementing count, resetting head/last when empty. |
| 0x0041e2f0 | PickupList_RemoveByType | Walks live list [0x004f331c] (next +0x50): removes (0x0041d8a0, EDX=0x004f3318) nodes whose table entry [node+4]+0x28 == ECX and whose +0x24 != EDX (keep id). |
| 0x0041e330 | Pickup_SampleGroundLight | ECX=object, EDX=position. 0x00476320(ECX=0x004f3ab0); [0x0057da28] = [0x004f3ab0]. Builds a vertical probe with 0x00443d20 on world [0x004f3328] from (x, 500.0f, z) and casts it with 0x004290f0(pos.y, radius 0.5f, 1). No |
| 0x0041e430 | PickupList_AnyNear | Returns 1 if any live pickup [0x004f331c] (next +0x50) has /x (+0xc) - p.x/ < r and /z (+0x14) - p.z/ < r (p = ECX, r = float arg); else 0. ret 4. |
| 0x0041e480 | Pickup_CycleWeaponSelection | Cycles current weapon code [0x004dbe68] upward: next = cur+1, wrapping to 3 after 0x13 (cmp 0x14), skipping 0x12/0x13; picks the first code the player owns (single player: bit2 of v+0x61c + (code&1)*0x54 + (code>>1)*0xac |
| 0x0041e540 | Pickup_WeaponSlotCode | ECX in 2..9 (jump table): returns 2*ECX-3 + (EDX != 0), i.e. 1/2 for 2, 3/4 for 3 ... 15/16 for 9; other values 0. |
| 0x0041e5d0 | Engine_TimedEventQueueTick | Walks respawn queue [0x004f330c] (next +8, count [0x004f3314]): for entries whose time +4 < game time [0x0056b430] calls Pickup_Respawn on the entry, unlinks it (fixing head/tail [0x004f330c]/[0x004f3310], count) and fre |
| 0x0041e6c0 | Pickup_Respawn | ECX=pickup node: activates scene object +0x24 (gwNodeSetActive 1), 0x004481b0, 0x00448230, 0x00447dc0, Node_SetPickupFlagsRecursive; Object3D_SetPosition(+0xc..+0x14), SetRotation(+0x18..+0x20), SetScale(1,1,1); 0x0044de |
| 0x0041e780 | Pickup_BuildSaveRecords | For each live pickup [0x004f331c] (next +0x50) whose object has flag 0x40000: writes a 0x30-byte record {first flag (1 then 0), table key [node+4]+8, id, +8..+0x20 (pos, rot), +0x28, +0x30} via 0x004c0010; stops on write |
| 0x0041e840 | Pickups_ReadSaveRecord | Save reader (registered by Pickups_Init, order 300). ret 0xc. If rec[0] (first record): clears live list 0x004f3318 and zeroes each table entry +0x14 (0x004db6fc, 40 entries). Spawns pickup 0x0041da20(ECX=rec[1], EDX=rec |
| 0x0041e890 | Pickups_SyncNetworkLists | For each live pickup [0x004f331c] not in respawn list 0x004f32f8 (PickupList_ContainsSorted) calls 0x00433ea0 (net announce spawn); for each respawn-list entry [0x004f32fc] not in live list 0x004f3318 calls 0x00433e40 (n |
| 0x0041e900 | PickupList_ContainsSorted | Sorted list [EDX+4] (next +0x50): returns 1 if an entry equals *ECX, 0 once entries exceed it or list ends. |
| 0x0041e930 | PickupList_FindById | Walks list from [EDX+4] (next at +0x50) returning node whose [0] == ECX, else 0. |
| 0x0041e950 | Pickup_GetField40 | bytes read 0x0041e950 len 4 |
| 0x0041e960 | Pickups_SwapGlobal_004f3330 | Returns old [0x004f3330] and stores ECX. |
| 0x0041e970 | Pickups_GetGlobal_004f3330 | bytes read 0x0041e970 len 6 |
| 0x0041e980 | PickupTable_EntryForPrimaryWeapon | Compares the name of player ECX primary weapon ([[ [ECX+4]+0x5e4 ]]) with table names from index 0x11 (0x004dba18..0x004dbd18, stride 0x30); returns matching entry (0x004db6e8 + idx*0x30) or the table base entry when non |
| 0x0041ea00 | PickupTable_LookupByKey | RENAMED from WeaponTable_LookupByKey: table is the pickup table (0x004db6f0, 40 x 0x30; see Pickups_Init 0x0041ccf0). Scans entries from index 0x11 (key at entry+0x10 = 0x004dba40) for key == ECX; returns entry+8 dword,  |
| 0x0041ea30 | Pickup_SpawnAtNamedNode | ECX=node name, EDX=pickup key. Looks up node (NodeRegistry_LookupByName with ECX=6 type, EDX=name); if found gets world position (Anim_ResolveObjectWorldPosition) and rotation (0x0044e110) and spawns 0x0041da20(ECX=key,  |

## Function index additions (2026-09-24)
- **`PickupTable_FreeImages` `0x0041cca0`** frees the image of each pickup-table entry
  (0x004db708..0x004dbe88, stride 0x30).
- **Crate drops (`PickupCrate_*`)**, disassembly re-read 2026-09-24:
  - **`PickupCrate_TryDropIfClear` `0x00438b30`:** if `PickupCrate_CanDropAt(20.0)` →
    `Pickup_CycleWeaponSelection` → `PickupCrate_TryDrop`.
  - **`PickupCrate_CanDropAt` `0x00438a20`** refuses when the object's flag bit 4 is clear. It
    also refuses when the **player's primary weapon ammo `[mount+0x34]` equals the
    infinite-ammo sentinel 1.2345679e8**. Otherwise the answer is "no pickup within the radius".
    **Correction:** the ledger note had read this as a position field.
  - **`PickupCrate_TryDrop` `0x004389c0`:** spawns via `Pickup_SpawnWithAnim` (the network path
    is out of scope).
  - **`PickupCrate_GetWorldPos` `0x00438a70`.**

