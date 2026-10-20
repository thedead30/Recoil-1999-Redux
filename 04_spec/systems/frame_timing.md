# Frame timing — the complete timestep model

`FUN_004a56d0` is the per-frame timer, and the only writer of `DAT_0056b424`.
All `CONFIRMED-BINARY`, with the key values `VERIFIED-ORACLE`.

```c
ticks = GetTickCount();                            // Win32, MILLISECOND resolution
now   = ticks * 0.001;                             // -> seconds
scale = _DAT_004e2fb8;                             // read BEFORE it is reset
_DAT_004e2fb8 = 1.0;                               // time scale resets to 1.0 EVERY frame
DAT_0056b42c  = now - _DAT_004e2fb0;               // raw wall-clock delta
DAT_0056b424  = scale * DAT_0056b42c;              // scaled delta
DAT_0056b430 += DAT_0056b42c;                      // REAL-time accumulator (unscaled, unclamped)
if (DAT_004e2fac != 0 && DAT_004e2fa8 < DAT_0056b424)
    DAT_0056b424 = DAT_004e2fa8;                   // <- THE UPPER CLAMP
_DAT_004e2fb0 = now;
DAT_0056b428 += DAT_0056b424;                      // GAME-time accumulator (scaled + clamped)
```

## The timestep is clamped at BOTH ends

| bound | value | where | tag |
|---|---|---|---|
| **maximum** | **0.125 s** (`DAT_004e2fa8`, enabled by `DAT_004e2fac`) | `FUN_004a56d0` | `CONFIRMED-BINARY` + `VERIFIED-ORACLE` |
| **minimum** | **0.005 s** | `Vehicle_MainUpdateTick` `0x426390` | `CONFIRMED-BINARY` |

So `dt` is confined to **[0.005, 0.125]** — an effective **8 Hz to 200 Hz** simulation band.
The upper clamp lives in the timer; the lower one in the vehicle tick. **Neither function
shows both**, which is why the earlier record could only see the floor.

### This resolves the "exact 0.125" anomaly
Verified live at a `Vehicle_MainUpdateTick` breakpoint:
```
004e2fa8  3e000000 00000001    max dt = 0.125, clamp ENABLED
004e2fb8  3f800000             time scale = 1.0
0056b42c  3eb7119a             raw delta  = 0.357...
0056b424  3e000000             final dt   = 0.125   <- CLAMPED
```
The raw wall-clock delta was **0.357 s** because TTD recording slows the process enormously.
The clamp fires, pinning `dt` at exactly `0.125` — which is why it read as a suspiciously
exact power of two on every hit. **The game was not running at 8 fps; the recording was, and
the clamp did its job.**

## Three findings a remake needs

**1. `GetTickCount()` — millisecond resolution.** `PLATFORM`. The original's `dt` is always a
whole number of milliseconds divided by 1000. A remake using a high-resolution timer will
produce `dt` values the original could never generate.

**2. There is a per-frame TIME SCALE.** `_DAT_004e2fb8` multiplies the raw delta and is
**reset to 1.0 at the top of every frame**. Anything wanting slow-motion or speed-up sets it
just before the tick, for one frame only. Who writes it is **not yet established**.

**3. Two separate clocks, and they diverge.**

| global | accumulates | scaled? | clamped? |
|---|---|---|---|
| `DAT_0056b430` | **real time** | no | no |
| `DAT_0056b428` | **game time** | yes | yes |

`DAT_0056b430` is updated *before* the clamp, so it is never clamped. Code reading one is not
interchangeable with code reading the other — and both are used widely. `DAT_0056b428` is the
one the weapon refire timer and objective deadlines compare against.

## Not established
- Who writes `_DAT_004e2fb8` (the time scale) to anything other than 1.0.
- Who sets `DAT_004e2fac` (the clamp enable) and whether it is ever off.
- `_DAT_004e2fb0` is the previous frame's timestamp; `_DAT_004e2fb4` the current.
