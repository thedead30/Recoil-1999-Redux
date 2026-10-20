# `expApprox` - the damping curve, IDENTIFIED
`VERIFIED-ORACLE` 2026-09-09, trace `Recoil18`.

This function was an open question in three records (`0x00429750` steer, `0x00429870`
throttle, `0x00426770` TRACK mover). It is the shared exponential-decay helper that appears
in every "no input -> coast" path, disguised in the decompiler as
`(float)(ftol_result + 0x3f800000) * value`.

## What it is
A **Schraudolph fast exponential**: the float is built by integer arithmetic on the
exponent field, not computed.

```c
#define EXP_A 12102200.0f          /* CONFIRMED-BINARY: bytes 38 aa 38 4b at 0x004d07b0 and 0x004d08a4 */

static float expApprox(float x) {
    int i = (int)(EXP_A * x);       /* ftol - truncates toward zero */
    union { int i; float f; } u;
    u.i = i + 0x3f800000;           /* += bit pattern of 1.0f */
    return u.f;
}
```
`CONFIRMED-BINARY` - the `ftol` + `0x3f800000` sequence is present verbatim at all three
sites.

**Correction 2026-09-24:** this block previously gave `EXP_A` as `12102203.0f`, the textbook
value of 2^23 / ln 2. The binary stores **`12102200.0f`** (`0x4b38aa38`, read at both
`0x004d07b0` and `0x004d08a4`). Use the binary's value. The difference is tiny (about 2.5e-7
relative), but it can move the truncated integer by one ulp at some inputs. There is **no** bias-correction term (the usual Schraudolph `-60801` tuning constant
is absent), which is what makes the error one-sided.

## The measurement that identified it
The player released the stick in `Recoil18` and the yaw rate decayed across three frames at
constant `dt` = 0.125:

```
1.612961 -> 0.370166 -> 0.084951
```

| ratio | value |
|---|---|
| `0.370166 / 1.612961` | **0.229495** |
| `0.084951 / 0.370166` | **0.229494** |

A constant multiplier per frame, identical to six decimal places - so the damping is
exponential in `dt` with a per-frame factor. `turn_damping` (`cfg+0xb8`) reads **12.0** live
off the player, so the exponent is `-turn_damping * dt` = `-1.5`.

| candidate | value at x = -1.5 | error vs observed |
|---|---|---|
| true `exp(x)` | 0.223130 | **6.4e-03** |
| **`expApprox(x)` above** | **0.229495** | **6.9e-07** |

The approximation matches the game **four orders of magnitude better** than real `exp()`.
That is not a fit - it is an identification. The game is running the approximation.

## Why this matters for the remake
`expf()` is wrong here by about **2.9%** per damping step, in one direction, on every frame
in which any vehicle is coasting or its steering is decaying. It compounds: after ten frames
of coast the divergence is roughly 33%.

This is exactly the class of error the project exists to avoid - individually invisible,
never caught by a screenshot, and it makes the handling feel wrong without any single number
looking wrong. **Port the bit trick verbatim, including the truncating `ftol` and the
missing bias constant.**

## Damping constants, read live off the player (`bft`)
| key | offset | value | tag |
|---|---|---|---|
| `turn_damping` | `cfg+0xb8` | **12.0** | `CONFIRMED-DATA` + `VERIFIED-ORACLE` |
| `rate_damping[0]` | `cfg+0xbc` | **4.0** | `CONFIRMED-DATA` + `VERIFIED-ORACLE` |
| `rate_damping[1]` | `cfg+0xc0` | **1.25** | `CONFIRMED-DATA` + `VERIFIED-ORACLE` |

`turn_damping` is confirmed as the steering exponent. Which of the two `rate_damping` values
feeds the throttle coast path, and which the same-direction damping term, is **not yet
established** - the throttle decay path was not exercised in this trace.

## Rounding caveat
`ftol` truncates toward zero, so the approximation is **asymmetric about x = 0**: negative
exponents truncate upward (toward 1.0). Do not substitute `lrintf`, `floorf`, or a C++
`static_cast` under a different rounding mode - use truncation.
