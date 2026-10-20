# The engine's math approximations — a complete site inventory
`CONFIRMED-BINARY` 2026-09-10. Cross-cutting concern; read this before implementing **any**
numeric code in Stage 2.

## Why this file exists
Four separate findings, made independently over many passes, turned out to be the same thing:
the engine substitutes cheap approximations for standard math functions, and in every case
**the textbook implementation is the wrong one**.

| approximation | where found | error |
|---|---|---|
| `expApprox` — Schraudolph exp via `ftol` + `0x3f800000` | steering/throttle damping | ~2.9% vs `expf`, one-sided |
| volume curve `ftol(1000 * log2(v) - 0.5)` | `FUN_0049f9a0` | **1.66x steeper** than true decibels |
| RGB565 expansion by plain shift, no bit replication | `Render_QueuePolygon_Untextured` | white = `(248,252,248)` not `(255,255,255)` |
| fast sqrt `(i>>1) + 0x1fc00000` | 3D audio distance | up to **+6.06%**, always over |

Rather than keep discovering these one at a time, the binary was swept for the characteristic
magic constants. The result is below.

## Fast square root — `(i >> 1) + 0x1fc00000`
**36 sites in 31 functions.** The constant has essentially no other use, so these are
unambiguous.

| address | function | status | sites |
|---|---|---|---|
| `0x004024a0` | `FUN_004024a0` | UNTOUCHED | 1 |
| `0x00410160` | `FUN_00410160` | UNTOUCHED | 1 |
| `0x00416f10` | `FUN_00416f10` | UNTOUCHED | 1 |
| `0x00424bf0` | `FUN_00424bf0` | UNTOUCHED | 1 |
| `0x004374a0` | `FUN_004374a0` | UNTOUCHED | 1 |
| `0x0043a4f0` | `FUN_0043a4f0` | UNTOUCHED | 1 |
| `0x0043b500` | `FUN_0043b500` | UNTOUCHED | 2 |
| **`0x0044b8c0`** | **`Render_ProcessLodDistanceNode`** | PRIOR-EVIDENCE | **2** |
| `0x00450c60` | `FUN_00450c60` | UNTOUCHED | 1 |
| `0x00452250` | `FUN_00452250` | UNTOUCHED | 1 |
| `0x004525d0` | `FUN_004525d0` | UNTOUCHED | 1 |
| **`0x00452650`** | **`Render_ComputeBoundingSphereFromCorners`** | NAMED | 1 |
| `0x00458c10` | `Anim_PlaceAndScaleAlongSegment_Static` | NAMED | 1 |
| `0x00458ce0` | `Anim_PlaceAndScaleAlongSegment_Animated` | NAMED | 1 |
| `0x0045c920` | `AnimEvent_ScreenShakeRect` | NAMED | 1 |
| `0x0046c3a0` | `FUN_0046c3a0` | UNTOUCHED | 1 |
| `0x00472a10` | `FUN_00472a10` | UNTOUCHED | 1 |
| `0x00472cc0` | `FUN_00472cc0` | UNTOUCHED | 1 |
| `0x00475210` | `FUN_00475210` | UNTOUCHED | 1 |
| `0x00483ad0` | `FUN_00483ad0` | UNTOUCHED | 1 |
| **`0x00487c50`** | **`FUN_00487c50`** (lighting intensity producer) | UNTOUCHED | 1 |
| `0x00487f10` | `FUN_00487f10` | UNTOUCHED | 1 |
| `0x00488d60` | `FUN_00488d60` | UNTOUCHED | 1 |
| `0x00489540` | `FUN_00489540` | UNTOUCHED | 1 |
| `0x004896d0` | `FUN_004896d0` | UNTOUCHED | 1 |
| `0x00489920` | `FUN_00489920` | UNTOUCHED | 1 |
| `0x00489a90` | `FUN_00489a90` | UNTOUCHED | 1 |
| `0x0048daf0` | `FUN_0048daf0` | UNTOUCHED | **4** |
| `0x004a2b40` | `FUN_004a2b40` (3D audio) | DECOMPILED | 1 |
| `0x004b0530` | `FUN_004b0530` | UNTOUCHED | 1 |
| `0x004b1190` | `Weapon_LoadMountArrayFromConfig` | **CONFIRMED** | 1 |

### What this means
**Every distance in the engine is up to 6% too large**, in the mid range, and always in the
same direction. That is not confined to audio:

- **`Render_ProcessLodDistanceNode`** — **LOD switches happen at the wrong distances.** Models
  change detail level up to 3% further out than exact maths would put them. Directly visible
  as different pop-in points.
- **`Render_ComputeBoundingSphereFromCorners`** — bounding sphere radii are over-estimated,
  so culling is slightly conservative and objects survive culling marginally longer.
- **`FUN_00487c50`** — the per-vertex lighting intensity producer. **Lead followed and
  CONFIRMED:** the falloff parameter is `fastSqrt(distance^2) - object[+0x50]`, i.e. a radial
  distance offset by the object radius, which is why it reads negative. See
  `04_spec/systems/render_d3d.md`.
- ~~`Weapon_LoadMountArrayFromConfig` contains an approximation site its record does not
  mention~~ - **RETRACTED, this was wrong.** `04_spec/structs/WeaponMountEntry.md` documents
  it in full: the exact formula, the `0x1fc00000` bit trick, the note that it is an
  approximation and not `sqrtf`, and the instruction to reproduce it. The claimed gap was an
  artefact of the QA script (see below), not of the record.

## `expApprox` — `add eax, 0x3f800000` after `ftol`
**29 sites in 19 functions**, found by the precise instruction pattern `05 00 00 80 3F`
(not the raw constant, which is also just `1.0f` and appears 304 times).

| address | function | status | sites |
|---|---|---|---|
| `0x00405040` | `FUN_00405040` | UNTOUCHED | 4 |
| `0x004059a0` | `FUN_004059a0` | UNTOUCHED | 1 |
| `0x00425a20` | `Vehicle_ReadPlayerControlInput` | **CONFIRMED** | 1 |
| `0x00426770` | `FUN_00426770` (TRACK mover) | **CONFIRMED** | **4** |
| `0x00427440` | `FUN_00427440` | UNTOUCHED | 2 |
| `0x00427ec0` | `FUN_00427ec0` | UNTOUCHED | 1 |
| `0x00428520` | `Vehicle_UpdateSubModePhysics` | PRIOR-EVIDENCE | 1 |
| `0x004289f0` | `Vehicle_UpdateGroundClearanceAndAutoAmphib` | PRIOR-EVIDENCE | 1 |
| `0x00428c20` | `FUN_00428c20` | UNTOUCHED | 1 |
| `0x00429560` | `FUN_00429560` | PRIOR-EVIDENCE | 1 |
| `0x00429750` | steering integrator | **CONFIRMED** | 1 |
| `0x00429870` | throttle integrator | **CONFIRMED** | **4** |
| `0x0042c0d0` | `FUN_0042c0d0` | PRIOR-EVIDENCE | 1 |
| `0x0042c2e0` | `FUN_0042c2e0` (velocity frame transform) | PRIOR-EVIDENCE | 1 |
| `0x0042fdc0` | `FUN_0042fdc0` | UNTOUCHED | 1 |
| `0x004374a0` | `FUN_004374a0` | UNTOUCHED | 1 |
| `0x004386c0` | `FUN_004386c0` | UNTOUCHED | 1 |
| `0x0043a600` | `FUN_0043a600` | UNTOUCHED | 1 |
| `0x0043a900` | `FUN_0043a900` | UNTOUCHED | 1 |

Heavily concentrated in the **vehicle physics** range `0x425000-0x43a000`. Every damping term
in the movement code goes through it.

Note `0x004374a0` appears in **both** lists — it uses a fast sqrt *and* `expApprox`.

## The standing rule
**Assume every math helper is an approximation until the disassembly says otherwise.** Do not
substitute `sqrtf`, `expf`, `log10f`, or a bit-replicating colour expansion. Four independent
findings and a 50-site sweep say the engine does not use them, and in each case the difference
is behavioural, not cosmetic.

Ghidra hides all of these: `ftol()` swallows the FPU arithmetic, and the bit tricks appear as
ordinary integer arithmetic on a float. **The decompiler will never show you an
approximation.** Read the instructions.


---

# A false alarm, and the QA method that caused it
2026-09-10. Recorded because the failure mode is repeatable.

The sweep above flagged `Weapon_LoadMountArrayFromConfig` as a **CONFIRMED** function whose
record omitted an approximation site. **That was wrong.** The record,
`04_spec/structs/WeaponMountEntry.md`, already contained a section headed *"Ballistic launch
speed - CONFIRMED"* giving:

```
[0x54] = VELOCITY^2 / (2 * GRAVITY)              // apex height
tmp    = 2 * GRAVITY * RANGE
[9]    = (tmp >> 1) + 0x1fc00000                 // fast sqrt bit-trick
```

along with the offsets (`0x24` VELOCITY, `0x44` GRAVITY, `0x150` apex), the note that
`VELOCITY` is **overwritten**, an explicit warning that this is an approximation rather than
`sqrtf`, and the `BEAM`-flag interaction that makes `VELOCITY` absent for beam weapons.

**The record was more complete than the check that doubted it.**

## The bug in the check
The QA script resolved each row's `spec_ref` by prepending `03_re/decomp/`. But `spec_ref`
for this row is already a full relative path, `04_spec/structs/WeaponMountEntry.md`, so the
script opened `03_re/decomp/04_spec/structs/WeaponMountEntry.md`, found nothing, fell through
to a different candidate file, and reported on that instead.

**Correct handling:** `spec_ref` is sometimes a bare filename relative to `03_re/decomp/` and
sometimes a full path from the project root. Try the path as given **first**, then the
`03_re/decomp/` prefix - never only the prefix.

## What was actually new this pass
| finding | |
|---|---|
| the comparison constant `0x004d33f8` = **0.0** | confirms the record's `GRAVITY != 0` guard, read from the binary |
| **`Weapon_LoadMountArrayFromConfig` never executes in `Recoil18` or `Recoil21`** | it runs at mission load, before the recorder attaches |

That second point matters for how its `VERIFIED-ORACLE` status should be read. The function
was verified through its **output** - the mount array contents read from live memory (entry 14
= `wep_8.0`, `DAMAGE` 44.0, `1/FIRE_RATE` 0.5) - not through observing it run. That is
legitimate evidence about what the loader produced, and it is how a load-time function *can*
be verified from an attach-time trace. But it is a different claim from "the function was
observed executing correctly", and the record should say which it is.

The same applies to `Render_CreateD3DDevice` and anything else that runs before the recorder
attaches. **Load-time functions can only ever be oracle-verified through their output.**


## The error is PERIODIC in the exponent, not a flat 3%
The `+3.06%` figure is the **worst case**. The approximation is exact where the squared value
is a power of four and degrades between, so the error swings between **0% and ~3%** depending
on magnitude.

Measured example: the lighting falloff distance at `d^2 = 285.61` gives fast `16.9252` against
true `16.8999` — **+0.15%**, about 1% of that system's 2-unit falloff ramp, and negligible.

**Quote the error as 0-3% depending on magnitude.** Whether it matters is per-site: negligible
for a 2-unit lighting ramp, potentially visible for LOD switch distances.


---

# CORRECTION: the fast-sqrt worst case is +6.06%, not +3.06%
2026-09-10. The `+3.06%` figure recorded above was measured from a **sparse sample of seven
values** (1, 4, 100, 2500, 1e4, 1e5, 1e6). That was an underestimate by half.

A dense scan of `1 .. 1e8` gives a true worst case of **+6.06%**.

The error is still periodic in the exponent - zero where the squared value is an exact power
of four, worst between - but the peak is **twice** what the sparse sample suggested.

| sample | `d^2` | fast | true | error |
|---|---|---|---|---|
| lighting falloff | 285.61 | 16.9252 | 16.8999 | **+0.15%** |
| **LOD distance (live)** | **8896.78** | **98.7530** | **94.3227** | **+4.70%** |
| dense-scan peak | 3.36e7 | - | - | **+6.06%** |

**Lesson: do not characterise a periodic error from a handful of samples.** Scan densely.

**Corrected guidance:** the error ranges **0% to +6.06%** depending on magnitude, always an
over-estimate. Whether it matters is per-site, and at least one site (LOD) lands near 5%.
