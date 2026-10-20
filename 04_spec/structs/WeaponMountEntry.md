# `WeaponMountEntry` — 356 bytes (`0x164`) — config key → offset map

Recovered from `Weapon_LoadMountArrayFromConfig` (`0x004b1190`) after setting
`ConfigTree_FindChildByName_Vec3`'s prototype (`int __fastcall (int *node, char *name)` @
`0x0048cf70`). Before that fix every lookup rendered as `ConfigTree_FindChildByName_Vec3()`
with no visible key name.

Array base `DAT_00778928`, count `DAT_00778924`, stride `0x59` dwords = `0x164` bytes.
Count is `(childCount - 1) / 2` — the config alternates name/body pairs.

All rows `CONFIRMED-BINARY` from `0x004b1190` unless noted.

| idx | offset | ZWEP key | notes |
|---|---|---|---|
| [0] | 0x00 | — | weapon id string (`wep_N.M`) |
| [1] | 0x04 | `DESC` | falls back to the id |
| [2] | 0x08 | (name) | `_strdup`'d |
| [3] | 0x0c | `MILITARY_NAME` | `_strdup`'d |
| [4] | 0x10 | — | `(short)` sequential index |
| **[5]** | **0x14** | **`FIRE_RATE`** | **stored as `1.0 / FIRE_RATE`** — the refire interval |
| [6] | 0x18 | `AMMO_LIMIT` | default `0x42480000` = **50.0** |
| [7] | 0x1c | `RANGE` | default `0x43fa0000` = **500.0** |
| [8] | 0x20 | — | `RANGE²` |
| [9] | 0x24 | `VELOCITY` | default `0x42f00000` = **120.0**; overwritten for ballistic weapons (below) |
| [0xa] | 0x28 | `TURN_RATE` | default `0x3e23d70a` = **0.16** |
| [0xb] | 0x2c | `PITCH_RATE` | default **0.16** |
| [0xc] | 0x30 | `ACCELERATION` | |
| **[0xd]** | **0x34** | **`IMPACT_PROXIMITY`** | the radius passed to `Collision_FindTargetsInRadius` |
| **[0xe]** | **0x38** | **`IMPACT_PROXIMITY²`** | the falloff denominator |
| [0xf] | 0x3c | `DETONATION_DISTANCE²` | stored squared |
| **[0x10]** | **0x40** | **`DAMAGE`** | default `0x3dcccccd` = **0.1** |
| [0x11] | 0x44 | `GRAVITY` | |
| [0x12] | 0x48 | `LOCK_ON` | also sets flag `0x4000` |
| [0x13] | 0x4c | `TURN_SUSPEND_TIME` | |
| [0x14] | 0x50 | — | default `0x3f800000` = **1.0** |
| **[0x15]** | **0x54** | **flags** | see below |
| [0x17] | 0x5c | `FLYOUT_HEALTH` | also sets flag `0x4` |
| [0x31] | 0xc4 | — | a node handle; gates `gwNodeSetActive` |
| [0x3e] | 0xf8 | — | pointer to `DAT_004e0fc8 * 0x4c` impact-variant records |
| [0x40] | 0x100 | — | `DAMAGE_ANIM_ON_HEALTH` entry count |
| [0x41]+ | 0x104+ | — | pairs: `[healthFraction, animHandle]`, stride 2 dwords |
| [0x49] | 0x124 | `KILL_ANIMATION` | |
| [0x4a] | 0x128 | `IMPACT_TYPE` | |
| [0x4b] | 0x12c | `CRATER` / `QUICKSAND` | base value |
| [0x4c] | 0x130 | `CRATER` / `QUICKSAND` | range = `[+0x14] − [+0xc]` |
| [0x4e]–[0x53] | 0x138–0x14c | `FREEZE` / `DESIGNATE` | six-value block |
| [0x54] | 0x150 | — | `VELOCITY² / (2 × GRAVITY)` — ballistic apex |
| [0x58] | 0x160 | — | zeroed at load |

## Flag bits at `+0x54` — CONFIRMED
| bit | key / meaning |
|---|---|
| `0x1` | set alongside `MINE` |
| `0x2` | **`BEAM`** present — `[9]` holds `1/BEAM` instead of `VELOCITY`, and the `VELOCITY` key is then skipped entirely |
| `0x4` | `FLYOUT_HEALTH` present |
| `0x8` | `CRATER` non-zero |
| `0x40` | `EXPIRES` |
| `0x80` | `FIXED_ROTATE` |
| `0x400` | `INSTANT` |
| `0x800` | `DESIGNATE` |
| `0x1000` | `CATCHES_FIRE` — **also the "bypass the damage queue" bit** in `0x4b0a50` |
| `0x2000` | **`MINE`** — and this is **the "no falloff" bit** in `0x4b0a50`: mines do flat damage at any range |
| `0x4000` | `LOCK_ON` |
| `0x8000` | `LOCK_ON_LEAD` |
| `0x10000` | `MULTI_TARGET` |
| `0x20000` | `QUICKSAND` non-zero |
| `0x40000` | `RELOAD` |
| `0x80000` | `REMOTE_DETONATE` |
| `0x100000` | `TETHER_GUIDED` |
| `0x200000` | `FREEZE` / `DESIGNATE` present |
| `0x400000` | `ANIMATION_ALWAYS` |
| `0x800000` | `RELATIVE_SPEED` |

## Two open questions from earlier, both now ANSWERED

### Q1 — is the splash falloff unclamped? **No. My earlier concern was wrong.**
The query radius is `+0x34` = `IMPACT_PROXIMITY`; the falloff divisor is `+0x38` =
`IMPACT_PROXIMITY²`. And the candidate record's distance field is the **squared** distance
(`Collision_FindTargetsInRadius` stores `local_5f0`, computed against `_DAT_005396c0 =
radius²`, and rejects when `radius² < distSq`).

So the real formula is:
```
damage = (1 - distSq / proximitySq) * DAMAGE * projectileScale
```
Both terms squared, and the query guarantees `distSq <= proximitySq`. **The result is
always in `[0, DAMAGE]` — never negative.** The `0x004b0a50` record's warning about
negative damage is retracted.

Note this makes falloff **quadratic in distance**, not linear — damage stays high near the
centre and drops off sharply at the edge. A remake using linear falloff would be wrong
everywhere except at the two endpoints.

### Q2 — does the slot hold `FIRE_RATE` or `1/FIRE_RATE`? **The reciprocal.**
`puVar5[5] = 1.0 / FIRE_RATE` at `+0x14`. So `FIRE_RATE` in `weapons.json` **is shots per
second** and the stored value is the interval in seconds. The `INFERRED` reading in
`weapons_table.md` is upgraded to `CONFIRMED-BINARY`.

RFPG at `FIRE_RATE 12.0` → 0.0833 s between shots. MDM at `0.5` → 2.0 s.

## Ballistic launch speed — CONFIRMED
When `GRAVITY != 0` the loader **overwrites** `VELOCITY`:
```
[0x54] = VELOCITY² / (2 * GRAVITY)                  // apex height
tmp    = 2 * GRAVITY * RANGE
[9]    = (tmp >> 1) + 0x1fc00000                    // fast sqrt bit-trick
```
i.e. launch speed becomes `sqrt(2 * GRAVITY * RANGE)` — the speed needed to reach `RANGE`
under that gravity — computed with the classic exponent-halving approximation, **not** an
exact `sqrtf`. A remake using exact `sqrt` will differ slightly; the approximation must be
reproduced to match.

## Keys recovered from raw memory
- `DAT_004e4500` = **`BEAM`** (`0x004e4500`, read directly) `CONFIRMED-BINARY`
- `DAT_004e4474` = **`MINE`** (`0x004e4474`, read directly) `CONFIRMED-BINARY`

**`BEAM` also settles a `weapons_table.md` inference.** LASERD, LASER and ARC have no
`VELOCITY` in the data because the loader *skips* `VELOCITY` when the `BEAM` flag (`0x2`) is
set. They are beam weapons by declaration, not by an absent key. `INFERRED` -> `CONFIRMED-BINARY`.

## Still unread
`DAT_004db654`, `DAT_004e4534`, `DAT_004e439c` — three key names Ghidra has not typed as
strings. Minor: they feed `[1]` (a `DESC` fallback), `[2]` (a display name) and a
`FREEZE`-shaped block.


---

## How this was oracle-verified - and what that does NOT mean
`Weapon_LoadMountArrayFromConfig` (`0x004b1190`) **does not execute in any trace**: checked in
`Recoil18` and `Recoil21`, no hit in either. It runs at **mission load**, before the recorder
attaches to the running game - the same situation as `Render_CreateD3DDevice`.

So its `VERIFIED-ORACLE` status rests on verifying the loader's **output**, not its execution:
the mount array contents were read from live memory and matched `weapons.json` (entry 14 =
`wep_8.0`, display name, `1/FIRE_RATE` = 0.5, `DAMAGE` = 44.0, and the `0x164` stride landing
exactly on an entry boundary).

That is sound - it is the only way a load-time function can be verified from an attach-time
trace - but it is a **weaker claim** than observing the function run. In particular the
ballistic block above is `CONFIRMED-BINARY` from the disassembly and **not** oracle-verified:
no trace contains a weapon being loaded, so the overwrite of `VELOCITY` has never been watched
happening. A trace that captures mission load would close it.

The guard constant is confirmed from the binary: `0x004d33f8` = **0.0**, so the test really is
`GRAVITY != 0`.
