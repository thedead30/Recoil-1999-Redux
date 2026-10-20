# Weapons are TWO structs, not one — conflict resolved

This resolves the `+0x40` conflict logged in
`03_re/decomp/0x004b0a50_Weapon_ApplyRadiusDamageFalloff.md`. The offset is not overloaded
and neither reading was wrong — they are **different structs**.

## 1. `WeaponMountEntry` — the shared weapon definition
A global array of **356-byte (`0x164`)** records, one per weapon type, loaded once from the
ZWEP data.

| evidence | tag |
|---|---|
| `Weapon_LoadMountArrayFromConfig` (`0x4b1190`): `calloc(1, DAT_00778924 * 0x164)` | `CONFIRMED-BINARY` |
| `Weapon_ProjectileUpdateTick` (`0x4af060`): bound is `DAT_00778928 + DAT_00778924 * 0x59`, and it strides `piVar12 += 0x59`. `0x59 ints = 0x164 bytes` — same stride, same count global | `CONFIRMED-BINARY` |
| `DAT_00778928` = array base, `DAT_00778924` = element count | `CONFIRMED-BINARY` |

Fields established so far:

| offset | meaning | evidence | tag |
|---|---|---|---|
| +0x34 | **radius-query radius** passed to `Collision_FindTargetsInRadius` | `FUN_004b09d0` | `CONFIRMED-BINARY` |
| +0x38 | **blast radius** — falloff denominator | `0x4b0a50` | `CONFIRMED-BINARY` |
| +0x40 | **base damage** | `0x4b0a50` | `CONFIRMED-BINARY` |
| +0x54 | **flags**. bit `0x2000` = no falloff · bit `0x1000` = bypass the damage queue · bit `0x4` and `0x80000` gate branches in the projectile tick · bit `0x200000` gates `FUN_00479660` | `0x4b0a50`, `0x4af060`, `0x4b26f0` | `CONFIRMED-BINARY` |

## 2. The per-vehicle weapon **slot** — runtime state
Pointed at by vehicle instance `+0x5e4` (primary) and `+0x5e8` (secondary). This is *not*
the 356-byte definition; it is per-vehicle mutable state whose **field `[0]` is a pointer to
the `WeaponMountEntry`**.

The decisive evidence, from `Vehicle_ReadPlayerControlInput` (`0x425a20`):
```c
piVar5 = (int *)piVar1[0x179];             // slot = inst[+0x5e4]
if ((*(byte *)(*piVar5 + 0x54) & 2) == 0)  // note: *piVar5 FIRST, then +0x54
```
`*piVar5` is dereferenced before `+0x54` is applied — so `slot[0]` holds a pointer, and the
`+0x54` flags live on *that* target, i.e. on the `WeaponMountEntry`. `CONFIRMED-BINARY`

| offset | meaning | tag |
|---|---|---|
| +0x00 | **pointer to the `WeaponMountEntry`** (the definition) | `CONFIRMED-BINARY` |
| +0x40 | **next-allowed-fire timestamp** (runtime) | `CONFIRMED-BINARY` |
| +0x44 | **refire interval** | `CONFIRMED-BINARY` |

Refire logic:
```c
if (slot[+0x40] <= gameTime && ...) { fire = 1; slot[+0x40] = slot[+0x44] + gameTime; }
```

There is also an embedded "no weapon" slot at instance **`+0x69c`** (`piVar1 + 0x1a7`),
compared against to reject firing when the slot is the null one. `CONFIRMED-BINARY`

## Why this matters for the remake
The definition is **shared and immutable**; the slot is **per-vehicle and mutable**. A
remake that merges them into one object would either give every vehicle of a type a shared
cooldown, or duplicate the weapon table per vehicle. Both are wrong in observable ways —
the first would make two tanks of the same type interfere with each other's rate of fire.

## Still open
- ~~Whether the stored rate is `FIRE_RATE` or its reciprocal~~ — settled for the **mount**:
  `mount[+0x14]` holds `1 / FIRE_RATE`, `CONFIRMED-BINARY` from the loader and
  `VERIFIED-ORACLE` below (`wep_8.0`, FIRE_RATE 2.0, mount reads 0.5). What the **slot**'s
  `+0x44` holds, and whether it is a copy of the mount value, is still unread.
- The slot struct's full size and its other fields.
- Which code allocates and initialises slots per vehicle (`Vehicle_SpawnInstanceFromConfig`
  is the likely site, unread).

---

## ORACLE VERIFICATION — 2026-09-09, `VERIFIED-ORACLE`

```
python 03_re/scripts/ttd_verify.py --trace Recoil07 --cmds \
 "!tt 50; bp 0x00425a20; g; dd poi(0x004f3a88)+4 L1; dd poi(poi(0x004f3a88)+4)+0x5e4 L1;
  dd poi(poi(poi(0x004f3a88)+4)+0x5e4) L1; dd 0x00778928 L1"
```
```
16e9174c  16ef3c90     node[+4]            -> vehicle instance
16ef4274  16ef47e0     inst[+0x5e4]        -> the SLOT            (0x16ef3c90+0x5e4 ✓)
16ef47e0  16e9b768     slot[0]             -> a WeaponMountEntry
00778928  16e9a3f0     mount array base
```

**The two-struct split is confirmed at runtime.** `slot[0]` = `0x16e9b768`, and:

| check | result |
|---|---|
| inside the mount array (`0x16e9a3f0` .. `0x16e9be5c`, 19 × `0x164`) | **yes** |
| offset from base | `0x1378` = 4984 |
| 4984 / `0x164` | **exactly 14.0 — an entry boundary, not an arbitrary address** |

The slot is a distinct object; its field `[0]` points at entry **14** of the shared
356-byte array. Both the split and the `0x164` stride are now runtime-verified.

### Reading entry 14 confirms four more struct fields at once
```
dd 0x16e9b768+0x40 L1  ->  42300000  = 44.0
da poi(0x16e9b768)     ->  "wep_8.0"
da poi(0x16e9b768+8)   ->  "Lock On Explosive Missile"
dd 0x16e9b768+0x14 L1  ->  3f000000  = 0.5
```

Against `weapons.json` `BALLISTICS.wep_8.0` (`CONFIRMED-DATA`):

| field | spec claim | trace | JSON | |
|---|---|---|---|---|
| `+0x00` | weapon id string | `"wep_8.0"` | key is `wep_8.0` | ✓ |
| `+0x08` | display name | `"Lock On Explosive Missile"` | `NAME` = `LO-EM` | ✓ (the abbreviation expanded) |
| `+0x14` | `1 / FIRE_RATE` | `0.5` | `FIRE_RATE` = 2.0 → 1/2 = 0.5 | ✓ |
| `+0x40` | `DAMAGE` | `44.0` | `DAMAGE` = 44.0 | ✓ |

**Array order also confirmed:** entry 14 is `wep_8.0`, which is the 15th key in
`weapons.json`'s `BALLISTICS` — so the loader preserves config order, as the record assumed.

Eight separate claims verified in two commands.
