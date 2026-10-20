# Vehicle instance (`inst = [node+4]`) - field map

Consolidated 2026-09-24 (TODO item E1) from `systems/vehicle.md` section 1, `systems/player.md`
and the ledger notes of the functions named. Every row is `CONFIRMED-BINARY` from the function
cited unless marked. The block is **0x10c4 bytes** and hangs off a 0x28-byte list entry in
`[0x004f3a7c]` (`player.md`).

- `node` = list cell: `[+0]` next, `[+4]` inst, `[+8]` class entry.
- `cfg` = `[[node+8]+4]` is the class config; its map is in
  [`VehicleClassConfig.md`](VehicleClassConfig.md).
- Byte re-read for this file: `Player_ResetVehiclePhysicsState` `0x00421790` (rows marked ✓).

## Mode, state and flags
| offset | type | meaning | source |
|---|---|---|---|
| +0x08 | int | previous movement type, written on every mode change | mode transitions (`vehicle.md` 7) |
| +0x0c | float | gravity magnitude; reset to nominal `[0x004f3ac8]` ✓ | `0x0042bf90`, `0x00421790` |
| +0x10 | int | **airborne** (1 = no ground contact) | `Vehicle_TrackContactDispatch` `0x0042c0d0` |
| +0x18 | int | controls disabled (effect slot `+0x5c4` < -0.5) | dispatch, health tick |
| +0x1c | int | instant-kill request | `vehicle.md` 8 |
| +0x20 | int | **skid** flag | `Vehicle_StartSkid` / `Vehicle_EndSkid` |
| +0x24 / +0x28 / +0x2c | int | amphib / hover / sub mode available | mode transitions |
| +0x30 / +0x38 | int | flags set with a player message and sound | `vehicle.md` 7 |
| +0x34 / +0x3c | int | surface type 1 / 4 seen | contact gather |
| +0x40 | int | player forced to TRACK | `vehicle.md` 7 |
| +0x48..+0x5c | - | damage-over-time state | `vehicle.md` 8 |
| +0x60 | int | runtime state: 0/1 alive, 2 AI, 3 special tick, 4 dead/inert, 5 player dead | `movement_types.md` |
| +0x64 | float | mode-change cooldown expiry (time); reset to 0 ✓ | mode transitions, `0x00421790` |

## Controls and motion
| offset | type | meaning | source |
|---|---|---|---|
| +0x68..+0x74 | float x4 | control inputs (throttle, steer, vertical, pitch gates) | dispatch |
| +0x7c / +0x80 / +0x84 / +0x88 | float | signed demands: throttle / steer / vertical / pitch | integrators |
| +0x90 / +0x94 / +0x98 | float | pitch / yaw / roll **rate** | `0x0042bf90` |
| +0xa4..+0xac | float x3 | **world** velocity | `velocity_frames.md` |
| +0xb0..+0xb8 | float x3 | **body** velocity: lateral, vertical, forward | `velocity_frames.md` |
| +0xc8 / +0xcc | float | speed cap / yaw-rate cap (cached from `cfg+0xac` / `cfg+0xb4`) | throttle, steer |
| +0xf4.. | float x3 per point | current collision-point positions | `Vehicle_TestStationaryPoints` `0x00424ed0` |
| +0x1a8 + i*0xc | float x3 | previous collision-point positions; reset to `cfg+0x100+i*0xc` + origin for `i < cfg+0x214` ✓ | `0x00421790` |
| +0x25c | ptr | platform being ridden (0 = none) | AMPHIB, body transform |
| +0x260..+0x28c | float x12 | orientation matrix (rows +0x260, +0x26c, +0x278) and translation +0x284..+0x28c | movers, `Vehicle_BlockMotion` |
| +0x314..+0x31c | float x3 | saved origin (block-motion restore); +0x318 is also previous-frame Y | `Vehicle_BlockMotion`, `Vehicle_TrackVerticalVelocity` |
| +0x380 / +0x38c / +0x398 | float x3 | horizontal forward / forward / up | `Vehicle_ExtractAxesFromMatrix` `0x004294d0` |
| +0x3a4 | float x3 | previous horizontal forward | movers |
| +0x3b4 | float | local yaw on a platform | AMPHIB `0x004279f0` |
| +0x3bc / +0x3c0 / +0x3c4 | float | pitch / yaw / roll | movers |
| +0x3c8 / +0x3f8 | - | stored pose, copied when `+0x594` is set | `vehicle.md` |
| +0x3d8 | float | saved yaw (block-motion restore) | `Vehicle_BlockMotion` |
| +0x3ec..+0x3f4 | float x3 | **world position** | movers |
| +0x404 / +0x410 | float x3 | bounds offset / bounds = pos + offset | movers |

## Contact, zone and controller lists
| offset | type | meaning | source |
|---|---|---|---|
| +0x434 / +0x520 | float x3 | forward vector copies (view) | `vehicle.md` 10 |
| +0x444 | int | light / collision zone | `Vehicle_SnapToGround`, `Pickup_HasLineOfSight` |
| +0x448 | int | reset to 0 on respawn ✓ | `0x00421790` |
| +0x44c (+0x458 height) | record | point-0 ground hit record | `Vehicle_GatherGroundContacts` `0x00428d60` |
| +0x478 | list | vehicle-vehicle contacts | `Vehicle_ResolveVehicleContact` `0x00424ac0` |
| +0x488 | list | collide-response list | `VehicleCtrl_HandleList488` `0x00424110` |
| +0x498 | list | controller list (handler `0x00424010`) | `VehicleCtrl_HandleList498` `0x00423fc0` |
| +0x4a8 | list | pickups touched | `VehicleCtrl_CollectPickups` `0x00424210` |
| +0x4b0 / +0x4c0 | list | controller lists (+0x4c0 → `Race_OnCheckpointHit`) | `Vehicle_UpdateControllers` `0x00423460` |
| +0x4c8 (count +0x4d0) | list | speed-probability list | `VehicleCtrl_HandleList4c8` `0x00424d00` |
| +0x4d4 | int | swept this frame | `Vehicle_SweepCollisionPoints` `0x004236b0` |
| +0x4d8..+0x4e8 | int x5 | controller-ran flags | `Vehicle_UpdateControllers` |
| +0x4ec | int | block-motion override | `Vehicle_BlockMotion` `0x00425770` |
| +0x538 | float x3 | cockpit eye offset | `camera.md` (cockpit) |
| +0x574 | float x3 | flattened vector | `vehicle.md` 10 |
| +0x594 | int | use stored pose | `vehicle.md` |
| +0x5c4 | float | effect slot read by the controls-disabled test | `vehicle.md` 8 |

## Weapons, health, save, race
| offset | type | meaning | source |
|---|---|---|---|
| +0x5ec + i*0xac (i < 10) | record | weapon slot: id, two fire-group sub-entries (stride 0x54) | `Player_FillSaveRecordFields` `0x0041f010` |
| +0x624 + i*0xac | record | the same slots as addressed by the HUD | `Hud_RefreshFromPlayer` `0x004231b0` |
| +0xd58 / +0xd5c | int | reset to 0 on respawn ✓ | `0x00421790` |
| +0xed0 | ptr | name-registry node (freed at `+0x40`) | `Player_Destroy` `0x0041fd20` |
| +0xf34 | float | **hit points** (max = class `+0x398`) | `Vehicle_AdjustHealth`, `Hud_RefreshFromPlayer` |
| +0xf70 | int | AI type id / patrol index | `Vehicle_SpawnInstanceFromConfig` `0x00421ab0` |
| +0xf84..+0xf8c, +0xfb0..+0xfcc, +0xfd4..+0xfe0 | - | saved in the 0x80-byte list save record | `Vehicle_BuildListSaveRecord` `0x0041f6a0` |
| +0x1018 + idx*4 | int | race checkpoint hit flags (lap valid if all from +0x101c set) | `Race_OnCheckpointHit` `0x00425150` |

## Not established
- The meanings of `+0x448`, `+0xd58`, `+0xd5c` beyond "reset on respawn".
- The contents of the controller lists `+0x498`, `+0x4b0` (named "not yet identified" in
  `Vehicle_ClearControllerLists` `0x00423530`).
- Gaps between the rows are unmapped. `PlayerEntry_Construct` `0x004383e0` zero-fills the whole
  0x10c4 block (bytes re-read, see [`PlayerEntry.md`](PlayerEntry.md)), so an unmapped field
  starts at 0 unless an initialiser writes it.
