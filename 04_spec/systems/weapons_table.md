# Weapon table — CONFIRMED-DATA

Source: `02_extracted/zrdr/zrdr_extracted/weapons.json`, key path `BALLISTICS.<id>`.
Extracted from `zrdr.zbd`; the loader (`Weapon_LoadMountArrayFromConfig` @ `0x004b1190`)
reads it as a **ZWEP** file with a version check (`VERSION` = 2) and allocates
`count * 0x164` bytes — 356 bytes per weapon record, matching the `WeaponMountEntry` size
in the prior record.

All values below are `CONFIRMED-DATA` (`weapons.json BALLISTICS.<id>.<KEY>`).

| id | NAME | DAMAGE | FIRE_RATE | RANGE | VELOCITY | IMPACT_PROX | IMPACT_TYPE |
|---|---|---|---|---|---|---|---|
| wep_1.0 | RFPG | 1.2 | 12.0 | 700 | 120 | 0 | 0 |
| wep_1.1 | ERFPG | 1.8 | 12.0 | 800 | 140 | 0 | 0 |
| wep_2.0 | HEM | 18.0 | 0.5 | 1024 | 100 | 16 | 2 |
| wep_2.1 | MDM | 24.0 | 0.5 | 1024 | 200 | 20 | 2 |
| wep_3.0 | FREON | 12.0 | 12 | 768 | 100 | 0 | — |
| wep_3.1 | NAPALM | 3.0 | 12.0 | 1024 | 100 | 4 | — |
| wep_4.0 | P-HEM | 24.0 | 1.0 | 200 | 80 | 48 | — |
| wep_4.1 | P-MDM | 28.0 | 1.0 | 200 | 80 | 32 | — |
| wep_5.0 | P-HEM | 36.0 | 1.0 | 160 | 60 | 40 | — |
| wep_5.1 | P-MDM | 39.0 | 1.0 | 160 | 60 | 40 | — |
| wep_6.0 | LASERD | 140.0 | 1.2 | 1024 | — | 0 | — |
| wep_6.1 | LASER | 22.0 | 2.0 | 512 | — | 0 | — |
| wep_7.0 | sonic | 66.0 | 0.5 | 768 | 100 | 32 | 2 |
| wep_7.1 | ARC | 65.0 | 1.5 | 256 | — | 0 | 2 |
| wep_8.0 | LO-EM | 44.0 | 2.0 | 1200 | 120 | 7 | 2 |
| wep_8.1 | LO-EM | 66.0 | 0.5 | 1050 | 80 | 6 | 2 |
| wep_9.0 | LO-EM | 140.0 | 2.0 | 1050 | 120 | 256 | 2 |
| wep_9.1 | LO-EM | 140.0 | 0.5 | 1050 | 80 | 256 | 2 |
| wep_9.2 | LO-SM | 15.0 | 1.0 | 1050 | 100 | 2 | 2 |

**Slot naming:** `wep_<slot>.<variant>`. Nine slots, each with up to two variants — matching
the prior record's finding that pressing an already-selected slot toggles its two
alternates, and that `FUN_0043c660` confirms ten slots.

## Readings that are INFERRED, not confirmed

- ~~`FIRE_RATE` is shots per second~~ **CONFIRMED-BINARY.** The loader stores
  `1.0 / FIRE_RATE` at mount `+0x14` (`0x004b1190`). RFPG 12.0 -> 0.0833 s between shots;
  MDM 0.5 -> 2.0 s.
- ~~Missing `VELOCITY` means hitscan/beam~~ **CONFIRMED-BINARY.** These weapons declare a
  **`BEAM`** key; the loader sets flag `0x2` and then *skips* `VELOCITY` entirely. The
  absent key is a consequence, not the evidence.
- **`IMPACT_TYPE` 2 vs 0** — 2 appears on everything with a non-zero `IMPACT_PROXIMITY`
  (splash), 0 on the direct-fire plasma. Likely "radius damage" vs "point damage".
  `INFERRED`; `Weapon_ProjectileImpact_ApplyDamage` (`0x004b07d0`) would settle it.

## Other keys present per weapon
`DESC`, `KILL_VERB`, `AMMO_LIMIT` (`123456789` = the no-limit sentinel, matching the prior
record's `0x4CEB79A3`), `FIRE.{ANIMATION,SOUND,RANDOM_ROTATE}`,
`FLYOUT.{MODEL,MODEL_ANIMATION,SOUND}`,
`IMPACT.{default,water,quicksand}.{ANIMATION,KILL_ANIMATION,DAMAGE_ANIM_ON_HEALTH,SOUND}`.

`DAMAGE_ANIM_ON_HEALTH` is a list of `[healthFraction, animOrPartName]` pairs — the
progressive damage visuals, keyed on remaining health fraction.

## Bearing on the +0x40 conflict
This does **not** resolve the `+0x40` conflict recorded in
`03_re/decomp/0x004b0a50_Weapon_ApplyRadiusDamageFalloff.md`. It confirms the record is 356
bytes and that both a damage and a rate value exist per weapon, but not which offsets they
land on. The loader's field writes were searched for and **not found** by offset-pattern
grep — logged as a failed attempt, not guessed.
