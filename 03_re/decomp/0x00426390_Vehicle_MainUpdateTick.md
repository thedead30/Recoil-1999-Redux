# 0x00426390  Vehicle_MainUpdateTick
Class: behaviour-bearing
Status: DECOMPILED
Verification: NONE  (oracle check outstanding - see Open questions)

## Signature
`void __cdecl Vehicle_MainUpdateTick(void)` — no args, no return. Operates entirely on
globals. Reached indirectly: registered as a function pointer at `0x420061` inside
`Player_InitPhysicsGlobalsAndVehicleClasses`. It has **no static caller**, which is why
the naive call-graph walk missed it.

## What it does
The per-frame vehicle update. Clamps the frame timestep, caches its reciprocal, then walks
a singly-linked list of every vehicle in the world and, per vehicle, branches on a mode
field to decide whether to read player input, run AI, run movement physics and update
weapon deploy state. Ends by calling `FUN_0042aa50` with a count of the mode-2 vehicles
seen this frame.

## Constants
| value | read at | meaning | tag |
|---|---|---|---|
| 0.005 | 0x00426390 (entry compare) | **minimum physics timestep in seconds.** `dt = max(frameDt, 0.005)`, so the sim never steps finer than 5 ms — an effective 200 Hz ceiling on physics resolution | `CONFIRMED-BINARY` |
| 1.0 | 0x004263xx | numerator of the cached reciprocal `invDt = 1.0 / dt` | `CONFIRMED-BINARY` |
| 0.01 | 0x004263xx | scale for a third cached timestep `dtScaled = dt * 0.01` | `CONFIRMED-BINARY` |

## Globals touched
| address | role | evidence |
|---|---|---|
| `DAT_0056b424` | raw frame delta time, input to the clamp | read at entry |
| `_DAT_004f3ac4` | **clamped dt** — the timestep the whole sim uses | written at entry |
| `_DAT_004f3aac` | `1.0 / dt` | written at entry |
| `_DAT_004f3abc` | `dt * 0.01` | written at entry |
| `DAT_0056b428` -> `DAT_004f3760` | copied verbatim each frame | second line |
| `DAT_004f3a7c` | **head of the vehicle linked list** | loop init |
| `DAT_004f36a4` | **the player's vehicle node** — identity compare selects the input path | `if (puVar4 == DAT_004f36a4) Vehicle_ReadPlayerControlInput()` |
| `DAT_004f3a88`, `DAT_004f3770` | two vehicles excluded from the netcode `gwNodeSetActive` call | network branch |
| `DAT_004f36b0` | non-zero enables `FUN_0043a600` after movement | movement branch |

## Struct offsets touched
Node is a list cell: `[+0x00] next`, `[+0x04] vehicle instance pointer`.
All offsets below are on the **instance** (`node[1]`).

| offset | type | name / meaning | evidence |
|---|---|---|---|
| +0x60 | int | **mode / state**. Branches: `4` and `6` share a path; `3` -> `FUN_0041b950`; `2` -> the LOD/culling path; `0` and `1` -> always run physics; `5` -> skips weapon deploy | the dispatch chain |
| +0x448 | uint | flag word; `\|= 2` unconditionally every tick | start of loop body |
| +0x510 | int | non-zero -> `FUN_00404e90()` runs (called on all three paths) | repeated guard |
| +0x5a4 | int | **primary fire trigger** state | matches prior record |
| +0x5a8 | int | **secondary fire trigger** state | matches prior record |
| +0x5b0 | int | primary fire *active* result, set from `+0x5a4 && mount->[+0x34] > 0` | written each tick |
| +0x5b4 | int | secondary fire *active* result, same shape | written each tick |
| +0x5bc | int | non-zero -> cleared and `FUN_004aefb0()` called | two sites |
| +0x5e4 | ptr | **primary weapon mount**, dereferenced at `+0x34` | fire-state calc |
| +0x5e8 | ptr | **secondary weapon mount**, dereferenced at `+0x34` | fire-state calc |
| +0xfd4 | float | threshold compared against `FUN_00472730()`'s result | mode-2 path |
| +0xfd8 | int | zero required for the early-out | mode-2 path |
| +0xfdc | int | non-zero forces the early-out branch | mode-2 path |
| +0xfe8 | int | set 1 on early-out, 0 otherwise | mode-2 path |
| +0xff8 | float | stores `FUN_00472730()`'s result (a distance) | mode-2 path |

## Calls out to
| address / name | why |
|---|---|
| `Vehicle_ReadPlayerControlInput` | only when the node is `DAT_004f36a4` (the player) |
| `Vehicle_DispatchMovementPhysics` | the movement integrator dispatch |
| `Weapon_UpdateDeployAnimState` | skipped when mode == 5 |
| `gwNodeSetActive` (0x447c60) | network path only |
| `Settings_GetNetworkFlag` | gates the netcode branches |
| `RecoilApp_GetSoundAPICheckboxValue` | gates `FUN_004386c0(0\|1)` — engine-sound state |
| `FUN_0041b950` | mode 3 handler |
| `FUN_00476370`, `FUN_00472730` | visibility/LOD test and a distance measure |
| `FUN_0042aa50(count)` | end-of-frame, passed the mode-2 count |

## Oracle evidence
**None yet.** This is DECOMPILED, not CONFIRMED. Per STAGE1.md a behaviour-bearing
function needs a TTD/emulate/x64dbg observation before it advances.

## Open questions
- What are modes 0-6 *called*? Six branches are distinguished but only their numbers are
  known. `Vehicle_TransitionTo{Track,Amphib,Hover,Sub}Mode` exist and likely map onto these.
- `+0x60 == 2` is the AI/LOD path — is mode 2 "AI-driven remote vehicle"? The count passed
  to `FUN_0042aa50` suggests it is a per-frame tally of something the caller cares about.
- The 0.005 floor: is `DAT_0056b424` seconds or milliseconds? The reciprocal and the
  `* 0.01` both need a unit before any of the three can be used in the remake.
- `_DAT_004f3abc = dt * 0.01` — what consumes it? Not used in this function.

---

## ORACLE VERIFICATION — 2026-09-09, `VERIFIED-ORACLE`

```
python 03_re/scripts/ttd_verify.py --trace Recoil07 \
  --cmds "!tt 50; bp 0x00426390; g; dd 0x0056b424 L1; dd 0x004f3ac4 L1; g; dd 0x0056b424 L1; dd 0x004f3ac4 L1"
```
Two consecutive breakpoint hits at the function entry, both showing:
```
0056b424  3e000000        source frame delta = 0.125
004f3ac4  3e000000        clamped dt         = 0.125
```
`dt == max(source, 0.005)` holds. **VERIFIED.**

Sampled separately at `!tt 50`:
```
004f3ac4  3cf114b3   dt      = 0.029428815469145775
004f3aac  4207ebd4   1/dt    = 33.98030090332031   (1/0.0294288 = 33.98030074) VERIFIED
004f3abc  399a4ab0   dt*0.01 = 0.00029428815469145775                          VERIFIED (bit-exact)
```

## METHODOLOGY WARNING — learned the hard way here
Reading `0x0056b424` and `0x004f3ac4` at an arbitrary `!tt 50` position showed them
**disagreeing** (0.125 vs 0.0294), which looked like the record was wrong. It was not — at
`!tt 50` the process is parked in `NtWaitForSingleObject` on another thread and
`_DAT_004f3ac4` still holds a value from an earlier frame.

**Globals must be sampled by breaking at the function that consumes them, not at an
arbitrary trace position.** A `!tt N` read is a snapshot of unrelated moments and will
produce false mismatches.

## NEW, UNEXPLAINED — dt is exactly 0.125 = 1/8
Both breakpoint hits gave a frame delta of **precisely** `0.125`. An exact power of two
appearing twice is not a natural frame time.

Two readings, neither eliminated:
1. **An upper clamp at 0.125 exists elsewhere** and is not in this function. This function
   only implements the *lower* clamp. Many engines cap dt to avoid tunnelling.
2. The value is a fixed timestep supplied by a caller under some condition.

**Caveat on all TTD-derived frame timings:** recording under TTD slows the process
enormously, so frame deltas in the traces are **not representative of normal play** and will
sit at whatever maximum applies. The 0.005 *floor* can therefore never be exercised in a
trace — its existence is `CONFIRMED-BINARY` from the code, not observable here.

**Do not conclude the game runs at 8 fps.** Find the writer of `0x0056b424` before treating
0.125 as meaningful.

## Vehicle list structure — `VERIFIED-ORACLE` 2026-09-09
```
sxi gp; !tt 50; bp 0x00426390; g; dd 0x004f3a7c L1; dd poi(0x004f3a7c) L2; ...
->  004f3a7c  16e91718          list head
    16e91718  16e91748 16ef2670    node[0]=next, node[4]=instance
    16ef26d0  00000004             instance+0x60 = mode 4 (inert)
    16e91748  16e91898 16ef3c90    next node; its instance is the PLAYER
```
The singly-linked list with `next` at `[0]` and the instance pointer at `[4]` is confirmed
from live memory, as is `+0x60` as the mode field. The player instance `0x16ef3c90` matches
the one reached independently through `DAT_004f3a88` in the weapon verification.
