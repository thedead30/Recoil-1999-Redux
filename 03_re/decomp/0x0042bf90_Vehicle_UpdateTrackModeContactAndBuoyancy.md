# 0x0042bf90  Vehicle_UpdateTrackModeContactAndBuoyancy
Class: behaviour-bearing
Status: DECOMPILED
Verification: NONE

## What it does
Called by the TRACK mover (`0x00426770`) every frame. Integrates pitch and roll rates into
the orientation, applies **gravity** to vertical velocity, integrates vertical position,
then runs the ground-contact and water-buoyancy chain.

Ghidra's decompile of the prologue is poor (a `BADSPACEBASE` stack-clearing loop of 0xbcd
dwords and `extraout_ECX`/`extraout_EDX` for the arguments) — the function is `__fastcall`
with the vehicle node in ECX. **The prologue should be re-read in disassembly before this
row advances to CONFIRMED**; the body below is clear regardless.

## Gravity — CONFIRMED
```
inst[+0xa8] -= inst[+0xc] * dt         // vertical velocity -= g * dt
inst[+0x3f0] += inst[+0xa8] * dt       // vertical position += v * dt
inst[+0x288]  = inst[+0x3f0]           // position mirror
```
| field | meaning | tag |
|---|---|---|
| inst +0xc | **gravity magnitude**, per-instance | `CONFIRMED-BINARY` |
| inst +0xa8 | world-space **vertical velocity** (Y) | `CONFIRMED-BINARY` |
| inst +0x3f0 | world-space **vertical position** (Y) | `CONFIRMED-BINARY` |

This is plain semi-implicit Euler: velocity first, then position from the *new* velocity.
The remake must use that ordering, not the reverse — with a 5 ms floor on dt the difference
is small per step but accumulates over a fall.

The old record claimed `nom_gravity = 40.0`. That is a **class-config value** that lands in
`inst[+0xc]`; this function confirms the *mechanism* and the field, not the magnitude. The
40.0 remains `PRIOR-EVIDENCE` until read from the config loader or observed at runtime.

## Angular integration — CONFIRMED
```
inst[+0x3bc] += inst[+0x90] * dt       // pitch += pitchRate * dt
inst[+0x3c4] += inst[+0x98] * dt       // roll  += rollRate  * dt
FUN_00474260(pitch, yaw, roll)         // rebuild the orientation matrix
```
| field | meaning |
|---|---|
| inst +0x90 | **pitch rate** |
| inst +0x98 | **roll rate** |

Together with yaw rate at `+0x94` (from the steer integrator) this completes the angular
velocity triple: **+0x90 pitch, +0x94 yaw, +0x98 roll**, matching the Euler triple at
**+0x3bc pitch, +0x3c0 yaw, +0x3c4 roll**. `CONFIRMED-BINARY`

This also answers an open question from the throttle record: `inst[+0x90]` appearing in the
chassis-lean pitch term is the **pitch rate**, not an acceleration.

## Contact / buoyancy chain
```
FUN_0042cf90()                                        // unread
if (Vehicle_UpdateWaterBuoyancyAndAutoTransition()) {
    FUN_0042c0d0(); FUN_0042da40();
    if (node == DAT_004f3a88 && inst[+0x10] == 0)      // player AND grounded
        FUN_0042d320();
    FUN_0042c2e0();
}
```
The whole contact block only runs when the buoyancy function returns non-zero, and
`FUN_0042d320` is **player-only and grounded-only** — a candidate for camera shake, engine
audio or track dust.

## Open questions
- Re-read the prologue in disassembly; Ghidra's `extraout_*` output is not trustworthy for
  the argument set.
- `inst[+0xc]` gravity: which class-config key populates it, and is 40.0 right?
- `FUN_0042cf90`, `FUN_0042c0d0`, `FUN_0042da40`, `FUN_0042d320`, `FUN_0042c2e0` all unread —
  this is where actual ground contact and suspension live, so they are the next targets in
  the physics chain.
- `DAT_004f3bc8` is written from the second (EDX) argument — what is it?
