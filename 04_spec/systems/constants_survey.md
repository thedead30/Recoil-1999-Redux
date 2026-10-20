# Hardcoded constants survey — the binary holds formulas, the data holds numbers

Swept all 1,287 decompiled map-1 reachable functions (`03_re/decomp_raw`) for float
literals. **Only 144 distinct non-trivial values exist in the whole reachable set** — 11 of them mathematics, leaving **133 needing attribution**.

`MEASURED-ASSET` — method: `constants_sweep.py` over the full decompiled corpus,
excluding `0.0 / 1.0 / -1.0 / 2.0 / -2.0 / 0.5 / -0.0`. Numbers here are whatever that
script prints; an earlier ad-hoc grep said 115 because it excluded a different set.

## Why this matters for Stage 2

144 is a very small number for a game this size. It means **Recoil's behaviour is
data-driven**: the executable encodes the *formulas* and the `.zbd` config tree supplies the
*tuning*. Physics rates, weapon damage, AI thresholds and vehicle stats are loaded from
config, not baked in — consistent with `Vehicle_LoadClassPhysicsConfig` reading `rates`,
`turns`, `chas_pitch` etc. by key.

Consequences:
- The 1,031 already-extracted JSON files in `02_extracted/` carry most of the tuning. That
  work is more valuable than it looked.
- A remake that gets the formulas right and loads the real config should be numerically
  correct **without** hand-tuning. Conversely, hand-tuned values in the remake are a
  reliable sign a config load is missing — which is exactly what happened last time, where
  `VehicleClassRegistry` could load the real `bft` class and `main.cpp` never called it.
- Stage 1's job for constants is therefore mostly to map **config key -> struct offset ->
  formula**, not to hunt for magic numbers in code.

## The 115, categorised

**Mathematical (not game tuning) — identified:**

| value | meaning | tag |
|---|---|---|
| 6.2831855 | 2π — angle wrap | `CONFIRMED-BINARY` |
| 3.1415927 | π | `CONFIRMED-BINARY` |
| 0.017453292 | π/180 — degrees → radians | `CONFIRMED-BINARY` |
| 0.003921569 | 1/255 — byte → normalised colour | `CONFIRMED-BINARY` |
| 255.0 | byte colour range | `CONFIRMED-BINARY` |
| -65536.0 | 2^16 — fixed-point scale | `CONFIRMED-BINARY` |
| -0.5236 | −π/6 = −30° | `CONFIRMED-BINARY` |
| 0.6667 | 2/3 | `CONFIRMED-BINARY` |

**Genuine game constants found so far** (each cited to its function record):

| value | meaning | where | tag |
|---|---|---|---|
| 0.005 | physics timestep floor (200 Hz ceiling) | `0x00426390` | `CONFIRMED-BINARY` |
| 0.01 | dt scale → velocity deadzone epsilon | `0x00426390`, `0x00429870` | `CONFIRMED-BINARY` |
| 2.25 | track-scroll yaw coefficient | `0x00426770` | `CONFIRMED-BINARY` |
| 1.72 | track-scroll rate multiplier | `0x00426770` | `CONFIRMED-BINARY` |
| 10.0 | mode-2 steer-assist speed threshold | `0x00425a20` | `CONFIRMED-BINARY` |

The remaining 133 are unattributed. They are listed by frequency in the sweep and each
needs its function read before it can be tagged. **None of them may be copied into the
remake untagged** — R1 applies, and `re_status.py` enforces it.

## Reproduce
```
python 03_re/scripts/constants_sweep.py
```
