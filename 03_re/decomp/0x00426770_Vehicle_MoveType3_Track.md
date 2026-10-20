# 0x00426770  (Ghidra: FUN_00426770) — movement Type 3, TRACK
Class: behaviour-bearing — **the player tank's integrator**
Status: **CONFIRMED**
Verification: **ORACLE-TTD** (field identities; dynamics still untested — see the end)

## Why this function matters most
`bft` (the player vehicle) is class `"track"` = movement type **3**
(`04_spec/systems/movement_types.md`). This is therefore the function that produces every
metre the player moves. The previous attempt ran the player on Type 0 (`FUN_00428120`),
so no drive test it ever performed exercised this code.

## Dispatch — CONFIRMED
From `Vehicle_DispatchMovementPhysics`, switching on **classConfig `+0xa4`**:

| type | handler | tag |
|---|---|---|
| 0 BASIC | `FUN_00428120` | `CONFIRMED-BINARY` |
| 1 FLY | **no case — falls through, no movement update** | `CONFIRMED-BINARY` |
| 2 SUB | `Vehicle_UpdateSubModePhysics` | `CONFIRMED-BINARY` |
| 3 TRACK | `FUN_00426770` (this) | `CONFIRMED-BINARY` |
| 4 HOVER | `FUN_00427140` | `CONFIRMED-BINARY` |
| 5 AMPHIB | `FUN_004279f0` | `CONFIRMED-BINARY` |

Two guards precede the switch:
- `if (instance[+0x60] == 4) return` — no movement at all for state 4. **Amended:** `4` means
  *AI-vehicle dead **or** inert*; the player dies to **5**. See
  `04_spec/systems/damage_and_destructibles.md`. The original wording here read `4` as simply
  "DEAD", which is only half right.
- `if (instance[+0x18] != 0)` — zeroes all four control inputs `+0x68 +0x6c +0x70 +0x74`.
  So `+0x18` is a **controls-disabled flag**.

## The grounded gate — CONFIRMED
`instance[+0x10]` gates both integrators:

```
if (inst[+0x10] == 0) FUN_00429750()   // steering  — grounded only
...
if (inst[+0x10] == 0) FUN_00429870()   // throttle  — grounded only
```

When `+0x10 != 0` (airborne) steering is replaced by an exponential decay of yaw rate and
velocity is left to the ballistic path. `+0x10` is therefore the **airborne / no-contact
flag**, and `+0x14` holds its previous-frame value (compared to fire `gwNodeSetActive` only
on a transition).

## Constants
| value | meaning | tag |
|---|---|---|
| 6.2831855 | 2π — yaw wrap. `yaw` is wrapped into (−2π, 2π], **not** (−π, π] | `CONFIRMED-BINARY` |
| 2.25 | track-scroll yaw coefficient. Left uses `-2.25`, right `+2.25` | `CONFIRMED-BINARY` |
| 1.72 | track-scroll overall rate multiplier | `CONFIRMED-BINARY` |

## Integration order (as written)
```
FUN_00429560()                                  // pre-step
steer   (grounded only)                          -> inst[+0x94] yaw rate
yaw:     inst[+0x3c0] += dt * inst[+0x94]        // wrapped to +-2π
FUN_00474260(rot); FUN_004294d0()
throttle(grounded only)                          -> inst[+0xb8] forward vel
world velocity = localVel * orientation3x3(+0x260..+0x280)
position: inst[+0x3ec] += dt * inst[+0xa4]
          inst[+0x3f4] += dt * inst[+0xac]
if (type == 3) Vehicle_UpdateTrackModeContactAndBuoyancy()   // suspension/contact
Object3D_SetRotation(inst[+0x3bc], inst[+0x3c0], inst[+0x3c4])
Object3D_SetPosition(inst[+0x3ec], inst[+0x3f0], inst[+0x3f4])
```

## Chassis lean — the `chas_pitch` / `chas_roll` block, CONFIRMED
Runs only when `node[+0x84] != 0` **and** the class is type 3:

```
raw   = cfg[+0xe4]*inst[+0xb8] + cfg[+0xe0]*inst[+0x90]     // accel & something
smoothed = raw*(1-k) + k*node[+0x70]                        // k = exp decay factor
node[+0x70] = smoothed
node[+0x6c] = clamp(raw - smoothed, +-cfg[+0xe8])           // PITCH
node[+0x74] = clamp(-smoothedRoll + cfg[+0xf0]*inst[+0x94]*inst[+0xb8], +-cfg[+0xf4])  // ROLL
Object3D_SetRotation(node[+0x6c], 0, node[+0x74])
```

| classConfig | meaning | tag |
|---|---|---|
| +0xe0, +0xe4 | pitch response coefficients (`chas_pitch`) | `CONFIRMED-BINARY` |
| +0xe8 | **pitch clamp** | `CONFIRMED-BINARY` |
| +0xf0 | roll coefficient (`chas_roll`), multiplied by `yawRate * forwardVel` | `CONFIRMED-BINARY` |
| +0xf4 | **roll clamp** | `CONFIRMED-BINARY` |

Roll is driven by `yawRate * forwardVel` — the tank leans into turns proportionally to how
fast it is going, which is why a stationary turn produces no roll.

## Track texture scroll — CONFIRMED
When `node[+0x88] != 0`, two `FUN_004760d0(0, rate)` calls, one per track:
```
left  = (-inst[+0xb8] - inst[+0x94] * -2.25) * 1.72
right = (-inst[+0xb8] - inst[+0x94] *  2.25) * 1.72
```
The opposing yaw sign is what makes the tracks counter-scroll when turning on the spot.

## Speed-proportional scale
`scale = |inst[+0xb8]| / inst[+0xc8]` (speed / speed-cap) fed to two `Object3D_SetScale`
calls; forced to `0,0,0` when airborne. Two nodes — likely exhaust or dust effects.

## Struct offsets added to the map
| offset | meaning |
|---|---|
| +0x10 | **airborne / no-contact flag** (gates both integrators) |
| +0x14 | previous frame's `+0x10` |
| +0x18 | controls-disabled flag |
| +0x25c | selects the towed/attached integration path |
| +0x260..+0x280 | 3x3 orientation matrix (local→world) |
| +0x284..+0x28c | position copy |
| +0x290..+0x2b0 | second 3x3 matrix (parent/attachment space) |
| +0x2b4, +0x2b8, +0x2bc | parent translation |
| +0x3bc, +0x3c0, +0x3c4 | rotation: pitch, **yaw**, roll |
| +0x3e0, +0x3e4, +0x3e8 | local offset within parent |
| +0x3ec, +0x3f0, +0x3f4 | **world position** |
| +0x404..+0x40c / +0x410..+0x418 | an offset and the position plus it (bounds centre?) |
| node +0x6c, +0x70, +0x74 | chassis pitch / smoothed lean state / roll |
| node +0x84 | chassis-lean enable |
| node +0x88 | track-scroll enable |

## Open questions
- `FUN_00429560` (pre-step) and `FUN_004294d0` unread.
- The `expApprox` damping curve (the `ftol` + `0x3f800000` bit trick) still unidentified —
  it appears in the throttle, steer and airborne paths, so it is a shared helper worth
  naming before any of the three is translated.
- `+0x25c` non-zero path uses `fpatan` and a transform stack push/pop — towed vehicles?
- `inst[+0x90]` in the pitch term — which quantity is it? Probably acceleration.
- Both branches of the `param_1 == DAT_004f3a88` test call
  `Vehicle_UpdateTrackModeContactAndBuoyancy()` identically — likely a compiler artefact of
  two different inlined argument sets, worth a disassembly check.

---

## ORACLE VERIFICATION — 2026-09-09, `VERIFIED-ORACLE` (field identities)

```
python 03_re/scripts/ttd_verify.py --trace Recoil07 --cmds \
 "sxi gp; !tt 50; bp 0x00426770; g; dd poi(@ecx+4)+0x3ec L3; dd poi(@ecx+4)+0x3bc L3;
  dd poi(@ecx+4)+0x10 L1; dd poi(@ecx+4)+0xb8 L1; dd poi(@ecx+4)+0x94 L1"
```
```
16ef407c  450aaf2f 40000000 44ace33e     +0x3ec/+0x3f0/+0x3f4  world position
16ef404c  00000000 40469705 80000000     +0x3bc/+0x3c0/+0x3c4  pitch / yaw / roll
16ef3ca0  00000000                       +0x10   airborne flag
16ef3d48  00000000                       +0xb8   forward velocity
16ef3d24  00000000                       +0x94   yaw rate
```

| field | value | |
|---|---|---|
| world position | **X 2218.95, Y 2.00, Z 1383.10** | plausible ground-level map coordinates |
| pitch / yaw / roll | 0.0 / **3.10297 rad** / −0.0 | level; yaw = **177.8°** |
| airborne `+0x10` | **0** — grounded | consistent with pitch/roll being level |
| forward velocity `+0xb8` | 0.0 | stationary |
| yaw rate `+0x94` | 0.0 | stationary |

All five field identities confirmed, and the values are mutually consistent: a stationary,
level, grounded tank.

### The decisive cross-check
The collision grid was verified **independently** in `04_spec/systems/collision.md` as
X ∈ [0, 4096], Z ∈ [0, 4352], 16 × 17 cells of 256 units. The tank's position from *this*
function falls inside it, in **cell (8, 11)** — the middle of the map.

Two separately-derived facts, from different functions and different verification runs,
agree about where the world is. That is much stronger than either alone.

### Yaw wrap
`yaw = 3.10297` sits in `(−2π, 2π]` as the record claims. It does **not** discriminate
between a ±π and a ±2π convention on its own — a value near π is consistent with both. The
±2π wrap remains `CONFIRMED-BINARY` from the code (`6.2831855` literal), not from this
observation.

### What this does NOT verify
The tank is **stationary**, so the *dynamics* are untested: that throttle drives `+0xb8`,
that steering drives `+0x94`, that the chassis-lean and track-scroll formulas produce the
claimed values. Those need a trace with the tank moving. Field identities only.
