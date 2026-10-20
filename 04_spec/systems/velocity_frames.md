# Velocity is a VECTOR in two frames, and it round-trips every tick
`VERIFIED-ORACLE` 2026-09-09, trace `Recoil22`. This corrects a reading in which
`inst+0xb0`, `+0xb4` and `+0xb8` were treated as three loosely-related scalars.

## The two triples
| offsets | frame | meaning |
|---|---|---|
| `+0xa4, +0xa8, +0xac` | **WORLD** | velocity X, Y, Z |
| `+0xb0, +0xb4, +0xb8` | **BODY** | velocity X (lateral), Y, **Z (forward)** |

`+0xb8` is the *forward* component of the body-frame velocity - which is why the throttle
integrator treats it as "the" speed. It is not an independent scalar.

`CONFIRMED-BINARY` from `FUN_0042c2e0`, which copies world into body and then rotates by the
3x3 orientation matrix at `+0x260 .. +0x280`:
```c
body = world;                       /* +0xb0 <- +0xa4, +0xb4 <- +0xa8, +0xb8 <- +0xac */
body = M * body;                    /* M = inst[+0x260 .. +0x280] */
if (DAT_004f3c8c >= 3) body.y = 0;  /* vertical body velocity discarded */
```

## The per-tick round trip - the structural point for the remake
```
throttle/steer integrators   ->  BODY velocity   (+0xb8)
TRACK mover                  ->  WORLD velocity  = body * orientation
                             ->  position        += dt * world
Vehicle_UpdateTrackModeContactAndBuoyancy
      -> FUN_0042c2e0        ->  BODY velocity   = orientation * world   <-- RE-DERIVED
```
Body velocity is **recomputed from world velocity every frame**, after contact and gravity
have modified the world vector. A remake that keeps a single scalar `forwardSpeed` owned by
the throttle integrator will never slow on slopes, never lose speed to terrain contact, and
will drift from the original the moment the tank is not on flat ground.

## The oracle reading that settled it
At position `8FD:1F98` in `Recoil22`, the write to `+0xb8` comes from **`0x0042c3aa`**, not
from the throttle integrator. Stack:
`Vehicle_MainUpdateTick` `0x426390` -> `Vehicle_DispatchMovementPhysics` `0x4266b0` ->
`Vehicle_UpdateTrackModeContactAndBuoyancy` `0x42bf90` -> `FUN_0042c2e0`.

At that moment:
| | |
|---|---|
| world velocity `+0xa4/+0xa8/+0xac` | **(-63.8923, 0.0, -11.9488)** |
| **magnitude** | **64.999999** |
| speed cap `inst+0xc8` | **65.0** |
| throttle demand `+0x7c` | **1.0** (full deflection) |

The tank is at its cap, travelling on a heading that splits the speed between world X and Z.
The value `-11.9488` written to `+0xb8` is simply `+0xac` copied in before the rotation; the
rotation then restores `-65.000008` as the forward component. Both writes of the pair are
accounted for, and **the vector magnitude equals the cap to seven significant figures**.

## Constants recovered here
| symbol | value | meaning | tag |
|---|---|---|---|
| `_DAT_004f3aac` | **30.751** | **`1 / dt`** - confirmed against `dt` = 0.0325193 (1/dt = 30.7510) | `CONFIRMED-BINARY` + `VERIFIED-ORACLE` |
| `55.0` | 55.0 | vertical world velocity guard: `if (inst[+0xa8] > 55.0) inst[+0xa8] = 0` | `CONFIRMED-BINARY` |
| `DAT_004f3c8c` | 7 (observed) | mode/quality flag; `< 3` smooths the vertical velocity through `expApprox`, `>= 3` takes it raw and zeroes body Y | `CONFIRMED-BINARY` |

**Vertical velocity is differentiated from position, not integrated:**
`vY = (inst[+0x3f0] - inst[+0x318]) * (1/dt)`. `+0x318` is the previous frame's world Y.

## Full-deflection throttle - confirmed to the digit
`Recoil22` is the first trace at full stick. Velocity writes repeat a period-4 pattern, and
the throttle integrator's pair reads:
```
-65.000000  ->  -71.000000   (unclamped: -65.0 - 48.0*0.125*1.0 = -71.0 exactly)
            ->  -65.000000   (clamped to |demand| * 65.0 = 65.0)
```
**50 writes sit exactly at the +/-65.0 cap.** The proportional clamp is now observed at
`|demand| = 1.0`, complementing the partial-stick verification on `Recoil17`.

The decompiler shows the unclamped value being *written* before being conditionally
overwritten - and the trace shows both writes. That is why raw values above the cap appear
in memory and are not a bug.

## Still open
- ~~`FUN_0042c420`~~ answered 2026-09-24: `Vehicle_ApplySlopeForces` (see `vehicle.md` 5.5).
- ~~What `DAT_004f3c8c` selects~~ answered 2026-09-24: it is the **number of active ground
  contacts**, written by `Vehicle_TrackContactDispatch` `0x0042c0d0` (`vehicle.md` 5.1). The
  `< 3` branch (fewer than three contacts: vy smoothed with `expApprox(-5*dt)`) is still
  unexercised in any trace.
- The formula above as `body = M * world` means `body_i = row_i . world` with rows at `+0x260`,
  `+0x26c`, `+0x278`. This was re-read in `0x0042c2e0` and `0x00429d30` on 2026-09-24.
- `inst+0x318` is assumed to be previous-frame world Y from the differentiation; not
  independently confirmed.
