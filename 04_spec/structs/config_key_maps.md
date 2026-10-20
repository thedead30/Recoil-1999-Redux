# Config key → offset maps (six loaders)

All recovered from the decompiled corpus after the `ConfigTree_FindChildByName_Vec3`
prototype fix. Every row `CONFIRMED-BINARY` from the cited function.

Vehicle physics and weapons have their own files
(`VehicleClassConfig.md`, `WeaponMountEntry.md`).

---

## AI behaviour — `NetGraph_LoadFromZrdByIndex` (`0x00403040`)

**This is the enemy AI config.** The previous attempt reported "enemies barely worked and
did not behave as they did in the original"; this is the data that drives them.

| key | offset | count | meaning (from the name; semantics not yet read) |
|---|---|---|---|
| `version` | — | | |
| `path_width` | 0x1c | 1 | patrol path corridor width |
| `activate_rad` | 0x20 | 1 | radius at which the unit wakes up |
| `attack_rad` | 0x24 | 1 | radius at which it engages |
| `attack_dwell` | 0x28 | 1 | how long it holds an attack |
| `not_pursuit_dwell` | 0x2c | 1 | dwell when not pursuing |
| `pursuit_range` | 0x30, 0x34 | 2 | a range pair (min/max, or start/break-off) |
| `return_range` | 0x38 | 1 | distance at which it returns to its route |
| `hide_times` | 0x3c, 0x40 | 2 | |
| `activate_buddy` | 0x44 | 1 | wakes a linked unit |
| `attack_buddy` | 0x48 | 1 | calls a linked unit to attack |
| `attack_strategy` | 0x4c–0x4f | 4 bytes | four **byte** fields, not dwords |
| `pursuit_params` | — | | a subtree |

`attack_strategy` writing four consecutive **byte** indices (`[0x4c]`–`[0x4f]`) is worth
noting: the prior record described an "attack_strategy enum closed from data". Four bytes
suggests four separate small values rather than one enum. **Not yet read** — flagged.

---

## Turrets — `Turret_Construct` (`0x004367a0`)

Indices are into an int array, so `[n]` = offset `n*4`.

| key | index | offset | meaning |
|---|---|---|---|
| `DETECTION_RANGE` | [0x28] | 0xa0 | acquisition range |
| `FIRE_RATE` | [0x3a] | 0xe8 | |
| `FIRE_DWELL` | [0x3f] | 0xfc | |
| `FIRE_LIMITS` | [0x3d],[0x3e] | 0xf4,0xf8 | traverse/elevation limits |
| `FIRE_ANIM` | [0x2b] | 0xac | |
| `HEALTH` | [0x57],[0x58] | 0x15c,0x160 | current + max |
| `DAMAGE_PART` | [0x59] | 0x164 | |
| `DAMAGE_MODIFIER` | [0x2c] | 0xb0 | |
| `DESTROY_ANIM` | [0x56] | 0x158 | |
| `ACTIVATE_ON_HIT` | [0x5a],[0x5b] | 0x168,0x16c | wakes when shot |
| `ALWAYS_LOOK_AT` | [0x55] | 0x154 | |
| `INTERSECT_BVOL` | [0x5c] | 0x170 | |
| `BASE_MOVES` | [10] | 0x28 | whether the base rotates |
| `MSL_LOCK` | [0x17],[0x2f],[0xb],[0xf],[7] | | missile lock parameters |
| `DEACTIVATE` | [0xc] | 0x30 | |
| `EFFECT` | [0x1b],[0x1c] | 0x6c,0x70 | |
| `PARTS` | [0x10],[0x2e],[0x2f],[0x30],[0xe] | | node names for the turret's parts |
| `SOUNDS` `START` `WEAPON` `TARGETS` | — | | subtrees |

---

## Mission objectives — `Mission_LoadObjectivesArray` (`0x00417f90`)

| key | offset | meaning |
|---|---|---|
| `READ_TIME` | 0x104 | how long the objective text displays |
| `REVIEW_DELAY` | 0x11c | delay before the review screen |
| `FINAL_MISSION` | 0x247c | marks the last mission of a map |
| `AUTOPLAY` | 0x114, 0x124, 0x138 | three fields |

## Mission triggers — `Mission_LoadObjectiveTriggers` (`0x00418230`)

| key | offset | meaning |
|---|---|---|
| `OBJECTIVE_SOUND` | 0x110, 0x124, 0x128 | three fields |
| `REVIEW_SOUND` | 0x118 | |
| `READ_SOUND` | 0x130 | |
| `ACTIVE` / `INACTIVE` | — | subtrees — **the trigger condition lists** |

`ACTIVE` and `INACTIVE` writing no scalar means they are subtrees, i.e. the actual
trigger definitions. **These are the highest-value unread items in the mission chain** —
the previous attempt's "triggered events did not work at all".

---

## Global physics + camera — `Player_InitPhysicsGlobalsAndVehicleClasses` (`0x0041fe90`)

These write module globals rather than struct offsets, so no offsets are listed. The
**key names are confirmed**; their destinations are not yet resolved.

`nom_gravity` · `wat_gravity` · `qsd_gravity` · `qsand_sink` · `lava_sink` · `max_slope` ·
`make_hot` · `make_cold` · `burning_anim` · `stealth` · `common_mode` (→ 0xa4) ·
`low_shield_snd` ·
camera: `camera_zone` · `max_cam_yaw_rate` · `mouse_push` · `fp_cam_el_rate` ·
`fp_cam_el_lim` · `underwater_cam` · `camera_elastic` · `max_cam_tether_angle`

**`nom_gravity` is confirmed as a real config key.** The prior record's claim that it equals
`40.0` remains `PRIOR-EVIDENCE` — the value has not been read from data, only the key's
existence from the binary.

Three separate gravities (`nom_`, `wat_`, `qsd_`) confirm the environment-dependent gravity
that `Vehicle_UpdateTrackModeContactAndBuoyancy` applies via `inst[+0xc]`.

---

## Vehicle instance / HUD — `FUN_00422170` (`0x00422170`)

| key | offset | count |
|---|---|---|
| `nanite` | 0x2d8, 0x2e0 | 2 |
| `weapon_up` | 0x2e4 | 1 |
| `weapon_select` | 0x2ec | 1 |
| `pinging` | 0x2f0 | 1 |
| `activation` | 0x2f4 | 1 |
| `not_pursuit_dwell` | 0x2f8 | 1 |
| `return_range` | 0x2fc | 1 |
| `camback` | [0x350]–[0x354] | 5 |
| `camera_ud_swing` | 0x37c, 0x380, 0x388 | 3 |
| `track_switch` | 0x38c, 0x390, 0x394 | 3 |
| `health` | 0x398, 0x39c | 2 |
| `pickups` | 0x3a4 | 1 |
| `weapons` | 0x3ac, 0x3b0, 0x3b4, 0x3b8 | 4 |
| `common_mode` `sounds` `start_anims` | — | subtrees |

`weapon_select` being a **config key** is notable: the previous attempt invented weapon
selection bindings. This is a per-vehicle-class value, not a keybind, but it is another
place the real data was available and unread.

---

## What this does not establish
Every row above is a **key-to-offset** mapping read from the loaders. It does **not**
establish what the values mean at runtime, their units, or their defaults. Those require
reading the consuming functions. Nothing here may be treated as behaviour.
