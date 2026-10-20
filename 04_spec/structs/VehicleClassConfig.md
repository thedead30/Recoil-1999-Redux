# Vehicle class config — key → offset map

Recovered from `Vehicle_LoadClassPhysicsConfig` (`0x004226d0`) after the
`ConfigTree_FindChildByName_Vec3` prototype fix made key names visible. All rows
`CONFIRMED-BINARY` from that function.

This is the struct the movement integrators read. The player vehicle `bft` is class
`"track"` → movement **type 3** (`04_spec/systems/movement_types.md`).

| key | offsets written | count | meaning |
|---|---|---|---|
| `platform` | 0x218 | 1 | |
| `collision` | 0x214 | 1 | |
| `collision_d` | 0x254, 0x258 | **2** | corrected 2026-09-24 |
| **`rates`** | **0xa8, 0xac** | 2 | **[0] = forward ACCELERATION, [1] = SPEED CAP MULTIPLIER** (see note) |
| **`turns`** | **0xb0, 0xb4** | 2 | **[0] = yaw ACCELERATION, [1] = YAW RATE CAP (flat, see note)** |
| `turn_damping` | 0xb8 | 1 | |
| `rate_damping` | 0xbc, 0xc0 | 2 | |
| `gun_pitch` | 0xc4, 0xc8 | **2** | corrected 2026-09-24, see the loader re-read below |
| `friction` | 0xcc, 0xd0, 0xd4 | 3 | |
| `stopping` | 0xd0, 0xd8 | 2 | **overlaps `friction[1]` at 0xd0** — load order matters |
| `chas_smooth` | 0xdc | 1 | the lean smoothing factor |
| **`chas_pitch`** | **0xe0, 0xe4, 0xe8** | **3** | [0],[1] response coefficients, **[2] = clamp** |
| **`chas_roll`** | **0xec, 0xf0, 0xf4** | **3** | [0],[1] response coefficients, **[2] = clamp** |
| `quicksand_slowdown` | 0xf8 | 1 | |
| `lava_slowdown` | 0xfc | 1 | |
| `a_damping` | 0x224 | 1 | |
| `mode_alt` | 0x228 | 1 | |
| `alt_control` | 0x22c, 0x230, 0x234 | 3 | |
| `amphib_wave` | 0x238–0x250 | **7** | all three `*_wave` keys write the **same** seven slots, so only the last one present survives (load order amphib, hover, sub). Corrected 2026-09-24: an earlier row said six |
| `hover_wave` | 0x238–0x250 | 7 | `[0]` roll base frequency, `[1]` roll frequency per unit forward speed, `[2]` roll amplitude; `[3..5]` the same for pitch; `[6]` **turn lean** = yaw rate × forward speed × `[6]` (`Vehicle_ApplyBodyWobble` `0x00429240`, `Vehicle_HoverSuspension` `0x00427440`, `Vehicle_UpdateSubModePhysics`) |
| `sub_wave` | 0x238–0x250 | 7 | |
| `mass` | 0x21c (+ derived 0x220) | 1 | default **1.0** when absent; the loader then stores `0x220 = 1.0 / mass`. Contact push `k = self.mass × other.invMass` (`Vehicle_ResolveVehicleContact` `0x00424ac0`); knockback `amount × invMass`. Bytes `0x00422b11..0x00422b4c`, key string `mass` at `0x004dc898`, constant 1.0 at `0x004d06bc` |
| `engine` | 0x29c | 1 | sound |
| `external` | 0x2a0 | 1 | sound |
| `collide` | 0x2ac | 1 | sound |
| `pitch_scale` | 0x2b4 | 1 | sound |
| `volume_scale` | 0x2b8 | 1 | sound |
| `sounds` | — | — | a subtree, not a scalar |
| `t2a_anims` `a2t_anims` `t2h_anims` `h2t_anims` `a2h_anims` `h2a_anims` `a2s_anims` `s2a_anims` | — | — | mode-transition animation sets (track/amphib/hover/sub pairs) |

## `chas_roll` is a TRIPLE — dispute settled
`CLAUDE.md` records that the old project log described `chas_roll` as a **pair**, and that a
2026-09-08 pass found that wrong. **This confirms the correction:** `chas_roll` writes three
offsets (`0xec, 0xf0, 0xf4`), exactly like `chas_pitch`.

It also matches the independently-derived TRACK mover reading
(`03_re/decomp/0x00426770_Vehicle_MoveType3_Track.md`), which found `+0xf0` used as the roll
coefficient and `+0xf4` as the roll clamp. Two separate derivations agreeing.
`PRIOR-EVIDENCE` → `CONFIRMED-BINARY`.

## `rates` / `turns` are acceleration+cap — confirmed a third time
This is now established from three independent directions:
1. the throttle integrator `0x00429870` reading `cfg[+0xa8]` as an accel term,
2. the steer integrator `0x00429750` reading `cfg[+0xb0]` likewise,
3. this loader writing exactly two offsets per key.

The previous attempt used `rates[0]` as **top speed**, which ran the tank at 74% of its
real speed. That error cannot recur if the remake reads this map.

## `stopping` overlaps `friction` — flagged
`friction` writes `0xcc, 0xd0, 0xd4`; `stopping` writes `0xd0, 0xd8`. **`0xd0` is written by
both.** Whichever key is processed later wins. In the loader `friction` is read before
`stopping`, so `stopping[0]` overwrites `friction[1]`.

This is either deliberate aliasing or an original-game bug. Either way **the remake must
reproduce the same load order**, or a vehicle declaring both keys will behave differently.
Do not "fix" it.

## Not yet established
- Which of `chas_pitch[0]`/`[1]` is the velocity term and which the pitch-rate term — the
  TRACK mover uses `cfg[+0xe4] * inst[+0xb8] + cfg[+0xe0] * inst[+0x90]`, so `[1]`
  multiplies forward velocity and `[0]` multiplies pitch rate. Consistent, but the
  key-order-to-index mapping inside the config array is assumed, not read.
- ~~The six `*_wave` values' meanings.~~ Closed 2026-09-24: there are seven, meanings in the table (from `Vehicle_ApplyBodyWobble`).
- ~~`rate_damping` / `turn_damping` feed the `expApprox` curve that is still unidentified.~~
  Stale (corrected 2026-09-25): the curve is identified and `VERIFIED-ORACLE`, see
  [`../systems/expapprox.md`](../systems/expapprox.md).

## Full loader re-read 2026-09-24 (item E2): corrections, defaults, missing keys
Source: `Vehicle_LoadClassPhysicsConfig` `0x004226d0..0x0042314d`, listing re-read in Ghidra
(`CONFIRMED-BINARY`). Where a default comes from a register, the value is given with the
instruction that loaded it. `EDI = 0.8f` is loaded at `0x004229df` and is callee-saved across
the `FindChild` calls.

**Corrections to the table above:**
- `gun_pitch` writes **two** values: `+0xc4` and `+0xc8`. Default `+0xc8 = 0xbe84816f`
  (-0.2588) and `+0xc4 = EDI` (0.8).
- `collision_d` writes **two** values: `+0x254` and `+0x258`. Default `(EDI = 0.8, 0.15)`.
- `sounds` is a subtree with keys **idle `+0x2a4`**, **skid `+0x2a8`**, engine `+0x29c`,
  external `+0x2a0`, collide `+0x2ac`, **land `+0x2b0`**, pitch_scale `+0x2b4` and
  volume_scale `+0x2b8`. The sound handles come from `0x004a0990`. The table above lacked
  idle, skid and land.
- `platform` = count at `+0x218` plus a point list read into `+0x1b8`.
- `collision` = count at `+0x214` plus a point list read into `+0x104`. The respawn reset reads
  the points from `+0x100` with stride 0xc; see `VehicleInstance.md`.

**Keys missing from the table above:**
- **`mode`** is copied as a string to `+0x54`, then mapped to the **movement type at `+0xa4`**:
  `track` 3, `hover` 4, `amphib` 5, `sub` 2, `fly` 1. Anything else (including no match after
  `basic`) stores `"unknown"` and 0.
- **Transition animation slots**, up to 2 animations each (`0x0045ff10`), 8 bytes per key:
  `t2a` `+0x25c`, `a2t` `+0x264`, `t2h` `+0x26c`, `h2t` `+0x274`, `s2a` `+0x27c`,
  `a2s` `+0x284`, `h2a` `+0x28c`, `a2h` `+0x294`. These explain the reads of `+0x264` in
  `Vehicle_TransitionToTrackMode` and `+0x294` in `Vehicle_TransitionToHoverMode`.
- **`mass`** `+0x21c` / `+0x220`: see the row above.

**Defaults when a key is absent:**
| key | default |
|---|---|
| `rates` | (10.0, 30.0) |
| `friction` | (10000.0, 10.0, 0.0) |
| `stopping` | `+0xd8` = 8.0; afterwards `+0xd0` is compared with `+0xcc` and may be replaced by `0.9 × +0xcc` (`0x00422984..0x004229a3`; read the jump before porting) |
| `quicksand_slowdown` / `lava_slowdown` | 0.9 / 0.8 |
| `turns` | (0.6, 2.0) |
| `turn_damping` | 30.0 |
| `a_damping` | 8.0 |
| `alt_control` | (-10.0, EDI = 0.8, -3.0) |
| `mass` | 1.0 |
| `mode_alt` | 2.0 |
| `chas_smooth`, `chas_pitch`, `chas_roll` | 0 (EBP zeroed at `0x00422acb`) |
| `rate_damping` | both slots = EAX at `0x00422a8f` (value not established) |

## `common_mode` keys - `VehicleClass_LoadCommonMode` `0x00422170` (re-read 2026-09-24, `CONFIRMED-BINARY`)
The keys are searched inside the `common_mode` section node. `EDI` is 0 from `0x004221c7`;
a default shown as "0" is that register. `ESI = 10000.0` (`0x461c4000`) where noted.

| key | offsets | default | notes |
|---|---|---|---|
| (class name) | `+0x04` string | - | copied from the stack argument |
| (section) | `+0x54` int | - | `(argcount - 1) / 2 - 1` of the class node. **Overlap:** the physics loader copies the `mode` string to `+0x54`. Which runs last is not established; read the caller order before the port |
| `nanite` | `+0x2d8`, `+0x2e0` | 0, 0 | args 0 and 1 |
| `sounds` → `weapon_up` / `weapon_select` / `pinging` | `+0x2e4` / `+0x2ec` / `+0x2f0` | - | sound handles (`0x004a0990`) |
| `activation` | `+0x2f4` | 10000.0 | float |
| `not_pursuit_dwell` | `+0x2f8` | 3.0 | |
| `return_range` | `+0x2fc` | 62500.0 (`0x47742400`) | float |
| `start_anims` | list at `+0x300` | - | |
| `camback` | `+0x350..+0x370` (9) | (0, 4.0, 9.0, 0, 3.5, 2.25, 0, 2.25, 2.25) | |
| `aimy` | `+0x374`, `+0x378` | 3.0, 2.0 | a key the old map filed under `camback` |
| `camera_ud_swing` | `+0x37c..+0x388` | (5.5, 2.5, 0, 0) | |
| `track_switch` | `+0x38c..+0x394` | 10000.0 ×3 | |
| **`health`** | `+0x398`, derived `+0x39c = 1.0 / health` | **100.0** | **single player reads arg 0; network reads arg 1** (`Settings_GetNetworkFlag` at `0x00422510`) |
| `pickups` | `+0x3a0`, `+0x3a4` | 0, 0 | via `0x0041dd60` |
| `weapons` | list head `+0x3ac`, tail `+0x3b0`, count `+0x3b4`, declared count `+0x3b8 = argcount - 1` | - | one **0x74-byte** zeroed record per entry: `+0` next, `+4` name string, `+0x54..+0x70` = the entry's args 1..8 (arg 3 converted from int to float at `+0x5c`) |

---

## ORACLE VERIFICATION — 2026-09-09, `VERIFIED-ORACLE`

```
python 03_re/scripts/ttd_verify.py --trace Recoil07 --cmds \
 "sxi gp; !tt 50; bp 0x00429870; g; dd poi(poi(@ecx+8)+4)+0xa8 L2;
  dd poi(poi(@ecx+8)+4)+0xe0 L6; dd poi(@ecx+4)+0xc8 L1"
```
```
16ee9390  42400000 42820000                       cfg+0xa8, +0xac
16ee93c8  3f000000 bc23d70a 3dae147b 3f000000     cfg+0xe0 .. +0xec
16ee93d8  3ac49ba6 3d99999a                       cfg+0xf0, +0xf4
16ef3d58  42820000                                inst+0xc8
```

**The vehicle is the player's.** `inst+0xc8` was read at `0x16ef3d58`, so the instance is
`0x16ef3c90` — the same address reached independently through `DAT_004f3a88` in the weapon
verification. So these are the **player tank's** (`bft`) real values.

| field | raw | value | claim | |
|---|---|---|---|---|
| cfg +0xa8 | `42400000` | **48.0** | `rates[0]` = acceleration | ✓ |
| cfg +0xac | `42820000` | **65.0** | `rates[1]` = speed cap multiplier | ✓ |
| inst +0xc8 | `42820000` | **65.0** | the instance's copy of `rates[1]` | ✓ **identical** |
| cfg +0xe0 | `3f000000` | 0.5 | `chas_pitch[0]` | ✓ |
| cfg +0xe4 | `bc23d70a` | −0.01 | `chas_pitch[1]` | ✓ |
| cfg +0xe8 | `3dae147b` | **0.085** | `chas_pitch[2]` = clamp | ✓ |
| cfg +0xec | `3f000000` | 0.5 | `chas_roll[0]` | ✓ |
| cfg +0xf0 | `3ac49ba6` | 0.0015 | `chas_roll[1]` | ✓ |
| cfg +0xf4 | `3d99999a` | **0.075** | `chas_roll[2]` = clamp | ✓ |

### `chas_roll` is a TRIPLE — settled at runtime
Six consecutive floats occupy `+0xe0` through `+0xf4`, three per key. `CLAUDE.md` records
that the old log called `chas_roll` a **pair** and that a later pass found it wrong. That
correction is now confirmed from **live memory**, not just from the loader.

### The error that ran the last attempt's tank at 74% speed
`rates` reads **[48.0, 65.0]**, exactly the numbers the old log cited. The previous attempt
used `rates[0]` as top speed — driving the tank at **48 instead of 65, i.e. 73.8%**.

Three independent sources now agree: the loader writes two offsets per key, the throttle
integrator uses `+0xa8` as an acceleration term and `inst+0xc8` as a cap, and live memory
shows 48.0 and 65.0 with the cap copied to the instance. **`rates[0]` is acceleration.**

---

## `rates[1]` is a cap MULTIPLIER, not a top speed - `VERIFIED-ORACLE` 2026-09-09
The throttle integrator clamps to `fabs(throttleDemand) * inst[+0xc8]`, so 65.0 is the top
speed only at **full** stick deflection. Half stick gives half top speed.

Oracle-verified on trace `Recoil17` (the first with the tank under power): demand
-0.249444 predicted a cap of 16.2139 and the next frame's velocity read 16.2140. See
`03_re/decomp/0x00429870_Vehicle_ApplyThrottle.md`.

**Do not translate `rates[1]` as a constant top speed.** That would be the same class of
error as the previous attempt's `rates[0]`-as-top-speed bug, and would only be visible
under partial throttle - the case least likely to be tested.

## `turns` = [2.5, 3.5], and its cap is FLAT - `VERIFIED-ORACLE` 2026-09-09
Read live off the player in trace `Recoil16`: `cfg+0xb0` = **2.5** (yaw acceleration),
`cfg+0xb4` = **3.5** (yaw rate cap), and `inst+0xcc` = **3.5**, the instance's cached copy -
mirroring `inst+0xc8` = `cfg+0xac` = 65.0 on the throttle side.

**The steering cap is flat; the throttle cap is proportional to stick deflection.** The two
integrators sit next to each other and differ in three ways:

| | throttle `0x429870` | steering `0x429750` |
|---|---|---|
| cap | `fabs(demand) * inst[+0xc8]` | `inst[+0xcc]` |
| accel sign | `v -= accel*dt*demand` | `yaw += accel*dt*demand` |
| reversal | damped via `expApprox` | snapped to exactly `0` |

**Do not factor them into one shared helper.** Half stick gives half top speed but the
*full* turn rate. See `03_re/decomp/0x00429750_Vehicle_ApplySteer.md`.
