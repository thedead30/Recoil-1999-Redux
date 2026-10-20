# 0x00429750  (Ghidra: FUN_00429750)  — steering integrator
Class: behaviour-bearing
Status: **CONFIRMED**
Verification: **VERIFIED-ORACLE** (traces `Recoil16` + `Recoil18`: acceleration term, constants, reversal snap and decay curve all confirmed; **the flat clamp is still NOT exercised** - see the end)

## Signature
`void __fastcall FUN_00429750(int node)`. Same cell layout as the throttle integrator.

## Formula (as written in the binary)
```
if (inst[+0x6c] == 0.0) {                       // no steer input -> decay
    inst[+0x94] *= expApprox(...)
    return
}
if (sign(inst[+0x80]) != sign(inst[+0x94]))     // INSTANT reversal, not a ramp
    inst[+0x94] = 0
inst[+0x94] += classConfig[+0xb0] * dt * inst[+0x80]   // YAW ACCELERATION
clamp(inst[+0x94], -inst[+0xcc], +inst[+0xcc])         // YAW RATE CAP
```

## Constants / fields
| field | meaning | tag |
|---|---|---|
| classConfig +0xb0 | **yaw acceleration** (`turns[0]`) | `CONFIRMED-BINARY` |
| instance +0xcc | **yaw rate cap** (`turns[1]`) | `CONFIRMED-BINARY` |
| instance +0x6c | steer input | `CONFIRMED-BINARY` |
| instance +0x80 | signed steer demand | `CONFIRMED-BINARY` |
| instance +0x94 | yaw rate | `CONFIRMED-BINARY` |

## Behavioural note for the remake
The sign-mismatch branch sets yaw rate to **exactly zero**, so a steering reversal loses
all accumulated rate in one tick rather than decelerating through it. This is a visible
handling characteristic and must not be smoothed.

## Open questions
- Same `expApprox` curve as the throttle path.

---

## ORACLE VERIFICATION - 2026-09-09, `VERIFIED-ORACLE`

Trace `Recoil16` is the first recording with the player steering. Player node
`0x19d0a398`, instance `0x19d6b7d8` (resolved via `DAT_004f3a88`; cross-checked because
`0x19d6b7d8 + 0xb8 = 0x19d6b890`, the velocity address seen in an independent run).

### Constants, read live off the player
| field | address | value | claim | |
|---|---|---|---|---|
| `cfg+0xb0` | | **2.5** | `turns[0]` yaw acceleration | ok |
| `cfg+0xb4` | | **3.5** | `turns[1]` yaw rate cap | ok |
| `inst+0xcc` | | **3.5** | the instance's copy of the cap | ok, **identical to cfg+0xb4** |
| `inst+0x6c` | | -0.989583 | steer gate | ok |
| `inst+0x80` | `0x19d6b858` | -0.989583 | signed steer demand | ok |
| `inst+0x94` | `0x19d6b86c` | -0.000122159 | yaw rate | ok |
| `dt` | `0x004f3ac4` | 0.125 | clamped frame delta | ok |

`inst+0xcc` = `cfg+0xb4` = 3.5 mirrors the throttle side exactly, where
`inst+0xc8` = `cfg+0xac` = 65.0. Both integrators cache their cap on the instance.

### The acceleration term - confirmed to float epsilon
The player calls this function **exactly once** in the trace, so the whole of its effect is
a single memory write. Asking the trace for every write to the yaw-rate field:

```
dx -r2 @$cursession.TTD.Memory(0x19d6b86c, 0x19d6b870, "w")
   .Select(m => new { Pos = m.TimeStart, PC = m.Address, Val = m.Value })
```
```
[0x0]  Pos: 3BBC:11E3   Val: 0xbe9e6558
```

| | |
|---|---|
| observed write | **-0.309366941** |
| predicted `cfg[+0xb0]*dt*demand + yawRate` = `2.5*0.125*(-0.989583) + (-0.000122159)` | **-0.309366847** |
| difference | **9.5e-08** - float rounding |

One call, one write, and it is the value the formula predicts.

### Why there is only one sample
Repeated `g` after the first hit lands at `4A76:2165`, which is the **end of the trace**
(lifetime `[6C:0, 4A76:2164]`). Readings taken there are the post-recording state, not
further frames - an easy way to manufacture a false "the value never changes" result. The
first hit is at `3BBC:11C3`; the write follows at `3BBC:11E3`.

**Positional discipline:** always print `Time Travel Position` alongside a sample. A
breakpoint that stops responding looks identical to a value that stopped changing.

### The asymmetry with the throttle integrator - both CONFIRMED-BINARY
| | throttle `0x429870` | steering `0x429750` |
|---|---|---|
| cap | `fabs(demand) * inst[+0xc8]` - **proportional** | `inst[+0xcc]` - **flat** |
| accel sign | `v -= accel*dt*demand` | `yaw += accel*dt*demand` |
| reversal | damped via `expApprox` | **snapped to exactly 0** |

These two functions are adjacent, similar, and behave differently in three ways. A remake
that factors them into one shared helper will be wrong on at least two counts. Half stick
gives half top speed but **full** turn rate.

## NOT verified by this run
- **The flat clamp.** Yaw rate reached only -0.309 against a cap of 3.5, so the clamp never
  engaged. The value 3.5 is confirmed from live memory and the branch is confirmed from the
  binary, but the clamping behaviour itself is unobserved.
- **The reversal snap.** Demand and yaw rate had the same sign throughout.
- **The decay path** (`inst[+0x6c] == 0`) and its `expApprox` curve - still unidentified,
  shared with the throttle integrator.

A trace with sustained hard turning in both directions would close all three.

---

## SECOND ORACLE PASS - 2026-09-09, trace `Recoil18`

`Recoil18` (2.6 GB) contains sustained hard turning in both directions. Player node
`0x19d44b28`, instance `0x19da62a0`, yaw rate at `0x19da6334`. **125 writes** to the yaw
field across the trace (100 returned by the query); the first 30 in order:

```
+0.548507 +0.861007 +1.124679 +1.359054 +1.612961     <- ramp up, varying dt
+0.370166 +0.084951                                    <- DECAY, stick released
+0.397451 +0.709951 +1.022451 +1.334951 +1.647451
+1.959951 +2.272451 +2.584951                          <- ramp, +0.312500 exactly
 0.000000                                              <- REVERSAL SNAP
-0.312500 -0.625000 -0.937500 -1.250000 -1.562500 -1.875000
 0.000000                                              <- REVERSAL SNAP
+0.312500 +0.625000 +0.937500 +1.250000 +1.562500 +1.875000
 0.000000                                              <- REVERSAL SNAP
```

### The reversal snap - now CONFIRMED
**15 writes of exactly `0.000000`**, each immediately preceding a sign change, and **zero
sign changes without one**. The previous pass listed this as unexercised. It is now
observed 15 times: the yaw rate is discarded in a single tick on reversal rather than
decelerating through zero. `VERIFIED-ORACLE`

### The acceleration term at FULL deflection
Every ramp step in the settled section is **exactly `0.312500`** =
`cfg[+0xb0] * dt * demand` = `2.5 * 0.125 * 1.0`. The earlier verification used partial
stick (-0.989583); this confirms the same term at full deflection, repeatedly, in both
directions.

### The decay path - now CONFIRMED, and it identified `expApprox`
The two frames after the stick is released fall by a **constant ratio of 0.229494**, which
is `expApprox(-turn_damping * dt)` with `turn_damping` = **12.0** (read live from
`cfg+0xb8`). This identified the shared `expApprox` helper across all three integrators -
see `04_spec/systems/expapprox.md`. `VERIFIED-ORACLE`

### The flat clamp is STILL not exercised
Maximum |yaw rate| observed is **2.584951**, against a cap of **3.5**. The reversal snap
keeps resetting the ramp before it can reach the cap: at `0.3125` per frame it needs about
**11 consecutive frames** of same-direction full stick, and the recording never holds one
direction that long. The cap value is confirmed from live memory and the branch from the
binary, but the clamping *behaviour* remains unobserved.

To close it: hold **one** direction of full lock, without reversing, for a solid three
seconds of recorded time.
