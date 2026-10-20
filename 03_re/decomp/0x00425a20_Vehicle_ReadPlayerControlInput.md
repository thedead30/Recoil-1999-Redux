# 0x00425a20  Vehicle_ReadPlayerControlInput
Class: behaviour-bearing
Status: DECOMPILED
Verification: NONE  (action IDs recovered 2026-09-09 by prototype fix)

## What it does
Reads mouse/joystick/keyboard, produces the four normalised control axes, clamps them to
±1, copies them into the "demand" fields the physics integrators consume, then handles
fire buttons, mode-transition requests and the aim camera.

## The control-axis pipeline — CONFIRMED, and it closes the loop with the integrators
Raw axis fields (written here), then copied to demand fields (read by the movers):

| raw | demand | consumed by | meaning |
|---|---|---|---|
| +0x68 (`[0x1a]`) | +0x7c (`[0x1f]`) | `FUN_00429870` throttle | **throttle** |
| +0x6c (`[0x1b]`) | +0x80 (`[0x20]`) | `FUN_00429750` steer | **steer** |
| +0x70 (`[0x1c]`) | +0x84 (`[0x21]`) | — | third axis (pitch/lift) |
| +0x74 (`[0x1d]`) | +0x88 (`[0x22]`) | — | fourth axis (roll/strafe) |

`CONFIRMED-BINARY`. This independently closes the chain: the input function writes exactly
the offsets the throttle and steer integrators read, which were derived separately.

## Digital vs analog — CONFIRMED
Digital input snaps to the literal float bit patterns:
- `0x3f800000` = **+1.0f**, `0xc0800000` (written as `-0x40800000`) = **−1.0f**

When `FUN_00407e50()` is non-zero the axis **ramps** instead of snapping:
```
axis += DAT_0056b424      // raw frame dt, one unit per second
axis -= DAT_0056b424
```
then all four axes are clamped to ±1.0. So held input reaches full deflection in exactly
one second of real time. `CONFIRMED-BINARY`

## Fire and mounts — cross-confirms the MainUpdateTick offsets
| field | offset | meaning |
|---|---|---|
| `[0x169]` | +0x5a4 | **primary fire** flag |
| `[0x16a]` | +0x5a8 | **secondary fire** flag |
| `[0x16b]` | +0x5ac | primary alt/charge flag |
| `[0x16e]` | +0x5b8 | a latched action flag (also sets a netcode global) |
| `[0x178]` | +0x5e0 | mount state bits; `& 0x180` selects an alternate fire path |
| `[0x179]` | +0x5e4 | **primary mount pointer** |
| `[0x17a]` | +0x5e8 | **secondary mount pointer** |

All match the offsets derived independently from `Vehicle_MainUpdateTick`. `CONFIRMED-BINARY`

## Fire rate — the refire mechanism, CONFIRMED
```
if (mount[+0x40] <= DAT_004f3760 && ...) {      // next-fire time <= now
    fireFlag = 1;
    mount[+0x40] = mount[+0x44] + DAT_004f3760;  // now + interval
}
```
| field | meaning | tag |
|---|---|---|
| mount +0x40 | **next-allowed-fire timestamp** | `CONFIRMED-BINARY` |
| mount +0x44 | **refire interval** (seconds) | `CONFIRMED-BINARY` |
| `DAT_004f3760` | current game time, copied from `DAT_0056b428` each frame in `Vehicle_MainUpdateTick` | `CONFIRMED-BINARY` |

## Mode transitions — gated on the CURRENT movement type
| current type | action leads to |
|---|---|
| 4 HOVER (and `[0xf]==0`) | `Vehicle_TransitionToTrackMode` |
| 4 HOVER (and `[0xd]!=0`) | `Vehicle_TransitionToAmphibMode(0)` |
| 3 TRACK or 5 AMPHIB | `Vehicle_TransitionToHoverMode` |
| 5 AMPHIB | `Vehicle_TransitionToSubMode` |

`CONFIRMED-BINARY`. Note these read classConfig `+0xa4` — the same movement-type field —
so the transition set available to a vehicle depends on what it currently is.

## Other
- `DAT_004f36b0 == 0` -> **early return**: a global that disables all player input.
  `Vehicle_MainUpdateTick` also tests it before calling `FUN_0043a600`.
- `DAT_004f36f0` / `DAT_004f36f4`: a **mouse deadzone** and a **sensitivity scale** — the
  steer axis is derived from mouse X beyond the deadzone, times the negated scale.
- Aim camera: on one action, snapshots `[0x3b0..0x3b2]` into `[0x403..0x405]`, calls
  `Vehicle_ComputeAimYawToPoint` and `Camera_GetWorldPosition`, then
  `Vehicle_SetCameraViewState`.

## Open questions
- ~~`Input_QueryAction()` argument invisible~~ **RESOLVED.** Setting the prototype
  (`uint __fastcall Input_QueryAction(int actionId)` @ `0x004717d0`) made every ID visible.
  Full table in `04_spec/systems/input_actions.md`; it matches the prior record exactly.
- **The physical key -> action binding is still unknown.** These are action IDs, not keys.
  `Input_QueryAction` (`0x004717d0`) itself is unread. Do not invent bindings.
- `FUN_00407e50` / `FUN_00407e80` / `FUN_00407eb0` / `FUN_004083c0` — device-present or
  mode predicates, unread.
- `piVar1[0x163] == 7` selects a different joystick mapping — device type 7?
