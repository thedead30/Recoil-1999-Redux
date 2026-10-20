# 0x00429870  (Ghidra: FUN_00429870)  — throttle integrator
Class: behaviour-bearing
Status: **CONFIRMED**
Verification: **VERIFIED-ORACLE** (trace `Recoil17`, tank under power, 2026-09-09)

## Signature
`void __fastcall FUN_00429870(int node)` — `node` is a vehicle list cell.
`instance = node[+0x04]`, `classConfig = node[+0x08]->[+0x04]`.

## What it does
Integrates forward velocity from throttle input, once per tick, grounded vehicles only.
Applies a deadzone, an exponential coast//damping term, a linear acceleration term from
the class config, and a speed cap taken from the instance.

## Formula (as written in the binary)
```
if (|inst[+0xb0]| < dtEps) inst[+0xb0] = 0      // dtEps = _DAT_004f3abc = dt * 0.01
if (|inst[+0xb8]| < dtEps) inst[+0xb8] = 0
if (inst[+0x20] != 0) { FUN_00429d30(); return }        // alternate path

if (inst[+0x68] == 0.0)                                  // no throttle -> coast
    inst[+0xb8] *= expApprox(...)
else {
    if (sign(inst[+0x7c]) == sign(inst[+0xb8]))          // same direction -> damp
        inst[+0xb8] *= expApprox(...)
    inst[+0xb8] -= classConfig[+0xa8] * dt * inst[+0x7c] // ACCELERATION term
    cap = |inst[+0x7c]| * inst[+0xc8]                    // SPEED CAP
    clamp(inst[+0xb8], -cap, +cap)
}
```

## Constants / fields
| field | meaning | tag |
|---|---|---|
| `_DAT_004f3ac4` | clamped dt (see 0x00426390) | `CONFIRMED-BINARY` |
| `_DAT_004f3abc` | `dt * 0.01` — used here as a **velocity deadzone epsilon**, answering an open question from the 0x426390 record | `CONFIRMED-BINARY` |
| classConfig +0xa8 | **acceleration** (`rates[0]`) | `CONFIRMED-BINARY` |
| instance +0xc8 | **speed cap** (`rates[1]`) | `CONFIRMED-BINARY` |
| instance +0x68 | throttle input | `CONFIRMED-BINARY` |
| instance +0x7c | signed throttle demand | `CONFIRMED-BINARY` |
| instance +0xb8 | forward velocity | `CONFIRMED-BINARY` |
| instance +0xb0 | lateral velocity | `CONFIRMED-BINARY` |

## Corroboration
Independently reproduces the old record's claim that `rates` is an **acceleration/cap**
pair rather than speed/turn-rate. The old attempt used `rates[0]` as top speed and ran the
tank at 74% of its real speed. That claim is now re-derived from the binary directly.

## Open questions
- `expApprox` (`ftol` + `0x3f800000` bias) — the exact damping curve is a float-bit trick;
  the multiplier's source operand is not yet identified.
- `inst[+0x20] != 0` diverts to `FUN_00429d30` — which mode is that? **Answered 2026-09-24:**
  `+0x20` is the **skid** flag and `0x00429d30` is `Vehicle_IntegrateSkid`. While skidding
  the speed cap is flat (`inst+0xc8`), not `|demand|`-proportional. See
  `04_spec/systems/vehicle.md` 4.2.

---

## ORACLE VERIFICATION - 2026-09-09, `VERIFIED-ORACLE`

Trace `Recoil17` is the first recording in which the player tank is **under power**. Every
earlier trace had it parked, so the whole model above was static-only and untestable. The
formula as written is now reproduced against live memory.

```
python 03_re/scripts/ttd_verify.py --trace Recoil17 --cmds \
 "sxi gp; bp 0x00429870 \".if (@ecx != poi(0x004f3a88)) {gc}\";
  g; .formats poi(poi(@ecx+4)+0x7c); .formats poi(poi(@ecx+4)+0xb8);
     .formats poi(poi(@ecx+4)+0xc8); .formats poi(poi(poi(@ecx+8)+4)+0xa8);
     .formats poi(0x004f3ac4); g; ...; g; ..."
```

| frame | demand `+0x7c` | dt `_DAT_004f3ac4` | velocity `+0xb8` | cap = \|demand\| * 65 |
|---|---|---|---|---|
| P1 | -0.374444 | 0.0152133 | **24.3393** | 24.3389 |
| P2 | -0.249444 | 0.125 | 24.3393 | 16.2139 |
| P3 | -0.124444 | 0.125 | **16.2140** | 8.0889 |

Live reads also confirm `cfg+0xa8` = **48.0** and `inst+0xc8` = **65.0** on the player.

### Two independent checks, both pass
1. **The proportional clamp.** P2's demand predicts a cap of `16.2139`; P3's velocity reads
   `16.2140`. Each frame's velocity is the previous frame's computed limit.
2. **The acceleration term.** From P2:
   `24.3393 - 48.0 * 0.125 * (-0.249444) = 25.8360`, clamped to `16.2139`.
   Observed at P3: `16.2140`. Five significant figures.

### What this changes about the record
Nothing in the formula - it was read correctly from the binary. What changes is its
**status**: `cap = |demand| * rates[1]` was previously an unverified static reading, and it
is the kind of claim most likely to be silently "simplified" into a flat top speed during
translation. It is now oracle-verified and must not be simplified.

Half stick gives half top speed. `rates[1]` is a cap **multiplier**, not a top speed.

### Sign convention - settled
Demand is **negative** for forward motion; velocity is **positive** forward
(`v -= accel*dt*demand`). `Recoil16` independently caught the reverse case, velocity
`-22.03`. Both directions observed.

### dt clamp confirmed from live data
P2 and P3 both read `dt` = **0.125** exactly - the upper clamp from
`04_spec/systems/frame_timing.md`, reached because TTD recording drags the game below 8 fps.
P1 reads `0.0152133` (~66 fps), captured before the recorder attached. The clamp is real,
active, and observable.

### Not verified by this run
`expApprox` (the coast/damping multiplier) still unidentified - P1..P3 are all
same-direction-under-power frames, which take the acceleration path. The coast path
(`inst[+0x68] == 0`) and the reversal path need frames where the player releases or
reverses the stick.
