# Weapon module (`0x004ae380`–`0x004b2960`)

**Scope:** projectiles, beams, impacts, damage, weapon lights and target tagging. The subsystem
gate is **OPEN** (61/61 CONFIRMED, membership CLOSED, callees covered — see
`03_re/ledger/subsystem_registry.csv`). Every statement is summarised from CONFIRMED ledger rows;
the row notes in `03_re/ledger/functions.csv` carry the offsets. Most rows were read from the
Ghidra decompile, a few from raw bytes / disassembly — the row says which. Mount record layout:
`04_spec/structs/WeaponMountEntry.md`, `weapon_two_struct_split.md`.

Tag on every value below: `CONFIRMED-BINARY` unless marked otherwise.

## Globals
| address | meaning |
|---|---|
| `0x00778928` / `0x00778924` | mount array base / count, records 0x164 bytes (0x59 dwords) |
| `0x00778934` | free list of projectile records (link +0) |
| `0x00778938` | free list of weapon lights (link node+0x40) — **exactly 8** created |
| `0x0077896c` | pending damage events, 0x44-byte records at `0x00778970`, **cap 64** |
| `0x00778940..54` | last impact position + hit point (written while `[0x00778968] == 1`) |
| `0x00779aac` | one-shot damage multiplier: copied into the next projectile, then reset to 1.0 |
| `0x00779a7c`, `0x00779a98` | 30.0 (radius cap; height ceiling for terrain-following) |
| `0x0056bca8` / `0x0056bcac` | dt / time copied from `0x0056b42c` / `0x0056b430` each tick (roles `INFERRED` from use) |

## Tick — `Weapon_ProjectileUpdateTick` `0x004af060`
1. Drains the pending damage queue **from the top (LIFO)**.
2. For each live mount: projectile branch (mount flag 2 clear) or beam branch (flag 2 set).

**Projectiles** (list `[mount+0x58]`)
- Homing (flag 0x4000 with a target): no turning until age reaches `[+0x4C]`; then turn rate
  `[+0x28] * ramp * dt`, ramp = (age − `[+0x4C]`) / `[+0x48]`, capped by the actual angle to target.
- Launch blend: for the first `[+0x48]` s velocity blends from the launch vector into dir·speed.
- Acceleration `[+0x30]` up to max speed; gravity `[+0x44]`·dt on Y.
- Range `[+0x1C]` on accumulated distance; expiry → impact (homing, unless flag 0x40).
- Proximity detonation within `[+0x3C]` of the target.
- Mines (speed 0, flag 0x2000): spin at dt·3.4906585 rad (200°/s) until a target is in radius.
- Collision: segment raycast old→new. Non-bouncing: snap to hit, impact, radius damage if `[+0x34] > 0`.
  Bouncing (0x2000): each bounce speed ×`[0x004e42e8]` (0.25) and a second factor ×`[0x004e42e4]`
  (0.5) — **EXE initial values of data globals**; bounce sound volume (speed/max + 1)·0.5 when
  speed > 0.2; at ≤ 0.2 the projectile rests.

**Beams**
- Continuous (flag 0x10000 clear): length eases toward `[+0x1C]`; pulse phase += dt·15.707963
  (2.5 Hz); damage `[+0x40]`·dt·pulse where pulse = (cos + 1)/2 with a **floor** of 0.25 inline.
  `Weapon_BeamDamageTick` `0x004b0e20` uses the same formula with a **cap** of 0.25 — the two
  disagree; reproduce both as found.
- Chain (flag 0x10000): up to **8** targets, sorted along the beam direction, segment lengths ×0.4,
  random jitter 0.1–0.2 of length. Beam instances hold ≤ 8 "BeamReflect%d" nodes (1 with flag 0x800).

## Firing — `Weapon_FireProjectile` `0x004ae660`
- Fire-gate hook `[0x0056bc9c]` (only when `[0x0077893c]`) can veto; empty pool → no shot.
- Lifetime/velocity: `[+0x30] == 0` and not 0x400 → speed = `[+0x24]`; else 0.0001 and caller
  velocity (0x800000 adds |v| and backs the start off by |v|).
- Muzzle effect random roll ±π/2 when `[+0x60]` bit 1.
- **Ends by writing 0 to `[W+0xa4]`, `[W+0xa8]`, `[W+0xf4]`** (bytes read). These are checked
  against −1 on entry; that EDI is the weapon is taken from the decompile, not traced.

## Damage
- `Weapon_ApplyDamageToTarget` `0x004b26f0`: target tag kind 1 → `health -= damage`, **no clamp,
  no callback**; other kinds → the kind's handler (slot +4) after hook `[0x00779a9c]`.
- `Weapon_ApplyRadiusDamageFalloff` `0x004b0a50`: dmg = (1 − entry[7] / `[W+0x38]`)·`[W+0x40]`
  (flat with 0x2000), ×`[proj+0x80]`. **Not clamped.** entry[7] comes from
  `Collision_FindTargetsInRadius` and is the **squared** distance to the target's bounding-sphere
  surface — squared vs linear units (`[W+0x38]` storage form not established). Targets are gathered
  with radius `[W+0x34]` (X/Z grid cells, 32-hit cap).
- Queued (64 max) unless flag 0x1000 or full.
- Radius effects: radius = `[W+0x12C]` + rand·`[W+0x130]` » 15, else min(`[W+0x34]`·0.5, 30.0).

## Impacts and config
- Impact entries 0x4C bytes, loaded from EFFECT / MODEL / ANIMATION* / RANDOM_ROTATE / SOUND /
  BOUNCE_SOUND. **No bounds check**: a 5th SOUND id overwrites the BOUNCE_SOUND count; a 7th
  BOUNCE_SOUND id overruns into the next entry. Random pick = (rand·count) » 15.
- With flag 0x20000 and a secondary hit, impact returns early and skips impact sound/effects.
- Scorch decals are blended into texture pixels (`Decal_StampOntoTexture` `0x00479660`); alpha
  cutoff >3 on 565 targets, >7 on 555.

## Lights
- Pool of 8, falloff 0.1/0.2 while pooled; new lights default white, near 32, far 64.
- Light slots ease toward ±1 at `[weapon+0x50]`·dt and decay by 0x00474fc0(dt·0.75) after
  `[+0x140]`; colour resets from `[+0x144..0x14C]` when the value changes sign.

## AI helper
`Weapon_ComputeAimPitch` `0x004b0530`: dy/dist + 0.24·dist/range for gravity weapons (fast-sqrt
distance), −1 when out of range without 0x2000.

## Player loadout (per vehicle entry)
- **Entry and mounts.** `PlayerEntry_Construct` `0x004383e0` allocates the 0x10c4-byte vehicle
  state block. `PlayerEntry_AddMount` `0x004384e0` appends 0xc4-byte mount records (list
  `+0x14`, count `+0x1c`; the first is also at `+0x8`).
- **`PlayerEntry_InitWeaponMounts` `0x00438ba0`:**
  - builds each mount's weapon from the class weapon list;
  - gates each weapon by level with `Weapon_IsAvailableForLevel` `0x0043ca90`;
  - finds the mount node and its `%s_L` / `%s_R` parts, and starts their `%sSCROLL` texture
    scroll at 2.0;
  - creates the firing object (`Weapon_CreateBeamInstance`) and selects the initial primary
    and secondary groups;
  - registers the mine save handler (`0x004bffe0(0x0043cdf0, 1000, 0)`).
- **Availability by level** (`0x0043ca90`): single player allows a weapon when its minimum level
  is not 0 and is ≤ the current level. Network games use a fixed table.
- **Kill verb.** `Weapon_MountInit_LoadKillVerb` `0x0043ca20` reads up to 19 chars (default from
  the message table).
- **Gun points** `Vehicle_BindGunPoints` `0x004390d0` (disassembly re-read 2026-09-24): the node
  `gun` is stored in `+0xee0` and its translation in `+0xea8`. Under it:

  | node | stored at | barrel index |
  |---|---|---|
  | `fpnt_c` | `+0xd80` | 0 (centre) |
  | `fpnt_r` | `+0xd8c` | 1 |
  | `fpnt_l` | `+0xd98` | 2 |

  **Correction:** the ledger note had these three offsets 4 bytes high.
- **Slot sounds.** Handle array `+0x10b4`: `PlayerEntry_SetSlotSound` `0x004385a0`, `PlayerEntry_EnsureSlotSound` `0x00438630` (only
  if the class has a resource at `+0x2e4+i*4`) and `PlayerEntry_StopSlotSound` `0x00438660`. Slot 3 is the skid sound
  and slot 5 the landing thud (`vehicle.md`).

## Selecting weapons
- **Primary group.** `Player_SelectPrimaryGroup` `0x00439540` stores the group in `+0x5e4` and
  resets the barrel index `+0xd44 = 0`. It sets the change state `+0x5e0` (0x10 direct, 4 when a
  change is pending, with slot sound 0) and `+0xd4c = group[4]*100 + group[8]`.
- **Secondary group.** `Player_SelectSecondaryGroup` `0x00439600` stores it in `+0x5e8`, sets
  barrel index `+0xd48 = 2`, swaps which group's nodes are active, and sets
  `+0xd50 = group[4]*100 + group[8]`.
- **Out of secondary ammo.** `Player_SelectSecondaryAfterPickup` `0x00439460` tries `+0x6f0` or
  `+0x69c`. It needs owned bit 2 and ammo > 0, else shows message 0x916 or 0x917 for 5 s.
- **`Weapon_AutoSelectNextUsable` `0x0043c660`:**
  - picks the first slot with flag bit 2, usable by the class and with ammo > 0;
  - search order is current slot, then downward to 2, then upward to 9;
  - usable means `Weapon_IsUsableByClass` `0x0043c630`: SUB mode refuses weapons with flag
    0x1000 or 2.

## Firing from the vehicle
- **Primary, `Weapon_FirePrimary` `0x0043c190`.** Computes the muzzle, then:
  - drops the request unless continuous;
  - with no ammo plays the empty click (player);
  - spawns by mode: `+0xd60 == 1` → `Weapon_SpawnProjectileFrom`; flag bit 1 → beam
    (`Weapon_StartBeam`); homing → `0x0043c430`; else `Weapon_SpawnPrimaryProjectile`.
  - Recoil is 1.5 with flag bit 0. Force feedback is `w+0x40 * 0.015151516`.
  - Ammo drops by 1 unless it equals the sentinel `0x4ceb79a3` (≈1.2345679e8, "infinite").
  - The player's shot counter `[0x004f311c]` is incremented.
- **Secondary, `Weapon_FireSecondary` `0x0043a400`:** the same shape using `+0x5e8`, muzzle
  `+0xdc8`; when ammo runs out it calls `Player_SelectSecondaryAfterPickup`.
- **Muzzle point `Weapon_ComputePrimaryMuzzle` `0x0043aa30`** (disassembly re-read 2026-09-24):
  - **No gun rig:** muzzle = position + (0, 1.0, 0) (`y - (-1.0)` at `0x004d17c4`) and aim =
    forward.
  - **With a rig:** muzzle = mount matrix (`Vehicle_ComposeLocalMatrix`) × gun point:

  | index `+0xd44` | point | flash timer slot | next index |
  |---|---|---|---|
  | 0 | `fpnt_c`, or the matrix origin when `w+0x28` | `+0xf18` (`+0xf1c = w+0xc`) | 0 (no advance is written) |
  | 1 | `fpnt_r` | `+0xf10` (`+0xf14 = w+0x10`) | 2 |
  | 2 | `fpnt_l` | `+0xf08` (`+0xf0c = w+0xc`) | 1 |

  So the primary fires from the centre after selection. The secondary starts at index 2
  (`fpnt_l`) and alternates left and right.
- **Secondary muzzle.** `Weapon_ComputeSecondaryMuzzle` `0x0043acf0` is the same with `+0xd48`
  and `+0xdc8`; it re-binds gun points when `+0x5c0` is set.
- **Matrices.** `Weapon_ComposeMountMatrix` `0x0043b1b0` composes parent × node matrices into
  `+0x320`. `Weapon_ComputeMuzzleWorldPos` `0x0043b3e0` transforms the rig point by `+0x320`.
- **Spawning.**
  - `Weapon_SpawnProjectileFrom` `0x0043c330` aims along `+0xd64` or toward the target
    `+0xec0`.
  - `Weapon_SpawnPrimaryProjectile` `0x0043c550` does the same, chosen by weapon def `+0x44`.
  - Both call `Weapon_FireProjectile` with the vehicle velocity `+0xa4`.
- **Beam.** `Weapon_StartBeam` `0x0043c2d0` starts a beam (`Weapon_ActivateProjectile`). In SUB
  mode it sets the kill flag instead.
- **Helpers.**
  - `Weapon_ResetBarrelScale` `0x0043c800` scales the mount nodes back to 1.
  - `Weapon_ClearChargeTimers` `0x0043a980` clears flash timers `+0xf08/+0xf10/+0xf18`.
  - `Weapon_DetonateMountedProjectiles` `0x0043c950` detonates four mount types.
  - `Weapon_FindMountSlotByType` `0x0043c9c0`.

## Projectile records and pools
- **Pools.** `Weapon_TakeRecordFromPool` `0x004ae530` (free list `[0x00778934]`) and
  `Weapon_PopPooledNodeOrCreate` `0x004ae4b0` / `Weapon_DetachAndPoolNode` `0x004ae4e0` (node
  pool on the mount, link `+0x40`).
- **`Weapon_SpawnProjectile` `0x004aeaa0`:**
  - puts a record at a position on list `[mount+0x58]`;
  - copies the **one-shot damage multiplier** `[0x00779aac]` into it and resets that to 1.0;
  - tags the node (`+0xbc = 1`).
- `Weapon_ActivateProjectile` `0x004aee40`:
  - plays two sounds and creates the muzzle effect, with a random roll of ±π/2 when
    `W+0x60` bit 1 is set;
  - takes the one-shot multiplier;
  - homing weapons copy and clear `[0x0077895c]/[0x00778960]`;
  - flag 0x800 takes a pooled light;
  - pushes the record on the mount list.
- **Freeing.** `Weapon_FreeProjectile` `0x004aeb50` and `Weapon_RecycleProjectileIfExpired`
  `0x004ae590` return records and nodes to their pools and reset the node transform.
  `Weapon_FreeChainAt58` `0x004aebc0` frees a whole list.
- **Detonation.**
  - `Weapon_DetonateProjectiles` `0x004aebf0` detonates all projectiles, a given owner's
    projectiles, or a synthetic one at a point. It returns the hit count and calls hook
    `[0x0056bca4]`.
  - `Weapon_ProjectileDetonate` `0x004aed00` casts down by `W+0x34*0.1`:
    - no hit → `Weapon_ProjectileImpact_ApplyDamage`;
    - else, when the path is clear, radius damage from 1.0 above.
- **Iteration and save.** `Weapon_ProjectileIterBegin/Next` `0x004b2930/0x004b2940` walk a list
  through the global `[0x0056bcb0]` (Next returns nothing). `Weapon_BuildMineSaveRecords`
  `0x0043cc70` writes each live mine as `MineData%03d`.
- `Weapon_FindMountByField10` `0x004ae450` looks up a mount by id.
- `Weapon_ClearField18_Callback` `0x004ae520`: `[EDX+0x18] = 0`.

## Impacts, sounds, beams
- **Impact.** `Weapon_ImpactFromRecord` `0x004b0980` builds an impact block and applies damage.
- **Radius effects.** `Weapon_ApplyRadiusEffectA/B` `0x004b0660/0x004b0710` use the radius rule
  above; they are gated on target bit 0x10000.
- **Clear path.** `Weapon_IsPathClear` `0x004b09d0` does a radius query; a lone hit on the
  shooter's own target is ignored.
- **Hit trace.** `Weapon_TraceHit` `0x004b0ba0` records the last impact into `0x00778940..54`
  while `[0x00778968] == 1`.
- **Chain beams.** `Weapon_SortPointsAlongDirection` `0x004b0ca0` bubble-sorts the chain-beam
  targets along the beam.
- **Effect node.** `Weapon_PlaceEffectNode` `0x004b0f70` places and orients it: pitch =
  asin(dir.y), yaw = atan2(-dir.x, -dir.z).
- **Random sounds.** `Weapon_PlayRandomSoundA/B` `0x004b0fd0/0x004b1030` pick
  `k = (rand()*count) >> 15` from the SOUND / BOUNCE_SOUND lists. `Weapon_PlaySound_Thunk1/2/3` (`0x004b0600/0x004b0620/0x004b0640`)
  play at 1.0.
- **Impact config.** `Weapon_LoadImpactEntry` `0x004b1fa0` parses impact config, with the
  unchecked overflows described under "Impacts and config".
- **Beam instances.** `Weapon_CreateBeamInstance` `0x004b1ec0` creates up to 8 `BeamReflect%d`
  segment nodes (`Weapon_CreateBeamSegmentNode` `0x004b2130`); flag 0x800 forces 1 segment.

## Effect slots (lights)
A slot is `{flags, weapon, cur, target, light, timer, node}`. It is used for weapon lights and
for the per-vehicle effect slot `inst+0x5c4` (`vehicle.md` section 8).
- **`Weapon_UpdateLightSlot` `0x004b2210`** (disassembly re-read 2026-09-24):
  - `flags |= 3`, `weapon = W`;
  - `target += amount`, or `-=` when `W+0x54 & 0x200`; clamped to [-1, 1];
  - attaches a pooled light coloured from `W+0x144` when the slot has none;
  - returns **2** if `cur < -0.5`, **1** if `-0.5 <= cur <= 0`, else **0**.
- **`Weapon_TickLightSlot` `0x004b2300`** (decomp):
  - while flag bit 1 is set, `cur` eases toward `target` at `min(W+0x50*dt, 1)` and the timer is
    set to `W+0x140 + now`;
  - light falloff is `W+0x138/0x13C * |scale*cur|`, and the colour resets on a sign change;
  - after the timer expires, `cur` decays by `0x00474fc0(dt*0.75)` and the slot is released
    below 0.001;
  - returns the same 2/1/0 code.
- **Pool.** `Weapon_CreateLightPool` / `DestroyLightPool` / `TakeLightFromPool` /
  `ReturnLightToPool` / `ResetLightSlot` / `ReleaseLightSlot`
  (`0x004b2160`, `0x004b21e0`, `0x004b2520`, `0x004b2570`, `0x004b21c0`, `0x004b22d0`): exactly 8
  lights, falloff 0.1/0.2 while pooled.

## Target tags
`Weapon_TagTargetTree` `0x004b25a0` and `Weapon_TagTargetTreeAlt` `0x004b26b0` attach a 0x10-byte
tag at node `+0xbc` and propagate it down untagged children (`Weapon_PropagateTargetTag`
`0x004b25f0`); `Weapon_ClearTargetTag` `0x004b2670` removes it. `Weapon_ApplyDamageToTarget`
dispatches on this tag.

## Lifecycle
- **Init.** `Weapon_InitSubsystem` `0x004b1090` sets both flags `[0x00778964]/[0x00778968] = 1`,
  the 30.0 caps and the multiplier 1.0. It registers `Weapon_RefreshDamageAttributionCache`
  `0x004b1160` as a save handler.
- **Shutdown.** `Weapon_Shutdown` `0x004b1d90` (via `Weapon_Shutdown_Thunk` `0x004b1180`) frees
  every mount's buffers and resets the globals.
- **Accessors.**
  - `Weapon_BuildSubsystemSaveRecord` `0x004b1140` saves `[0x00779aa0]`.
  - `Weapon_SetGlobal_00779a98` `0x004b1d80` sets the terrain-following height ceiling.
  - `Weapon_GetGlobalBlock00778940` `0x004b2910` returns the last-impact block.
  - `Weapon_GetGlobal00778958` `0x004b2920` returns an unidentified value.

## Still open
- The effect slot's gameplay meaning, i.e. which weapons carry bit 21 / 0x200 and what the player
  sees, is not established. Its arithmetic is exact (`0x004b2210` re-read).
- Units of `[W+0x38]` vs squared distance (radius falloff).
- Which inline-floor vs function-cap beam pulse the game actually shows (`VERIFIED-ORACLE` needed).
- Whether `0x004e42e8`/`0x004e42e4` change at runtime.
- The null-source byte write in `Weapon_FireProjectile`.
