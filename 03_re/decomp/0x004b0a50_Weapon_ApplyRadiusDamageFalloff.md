# 0x004b0a50  Weapon_ApplyRadiusDamageFalloff
Class: behaviour-bearing
Status: DECOMPILED
Verification: NONE

## Signature
`__fastcall`-ish with three implicit registers Ghidra surfaces as
`in_ECX` (weapon/mount record), `in_EDX` (projectile), and explicit
`param_1` (candidate-target array), `param_2` (owner id, to skip self-damage).

## The splash damage formula — as actually written
```
for each candidate target in param_1:
    if (target[+0xbc] == param_2) continue           // never damage the firer

    if (ECX[+0x54] & 0x2000)                          // "no falloff" flag
        d = ECX[+0x40]
    else
        d = (1.0 - target.distance / ECX[+0x38]) * ECX[+0x40]

    d *= EDX[+0x80]                                   // projectile damage scale

    if (!(ECX[+0x54] & 0x1000) && queueCount < 0x40)
        queue the hit into DAT_00778970[queueCount]    // 0x44-byte records, cap 64
    else
        Weapon_ApplyDamageToTarget(EDX+0x44, target, d)
```

| field | meaning | tag |
|---|---|---|
| ECX +0x38 | **blast radius** (falloff denominator) | `CONFIRMED-BINARY` |
| ECX +0x40 | **base damage** | `CONFIRMED-BINARY` |
| ECX +0x54 bit 0x2000 | **no-falloff flag** — flat damage at any range | `CONFIRMED-BINARY` |
| ECX +0x54 bit 0x1000 | **apply-immediately flag** — bypasses the queue | `CONFIRMED-BINARY` |
| EDX +0x80 | projectile **damage scale** multiplier | `CONFIRMED-BINARY` |
| target +0xbc | owner id, compared to skip self-damage | `CONFIRMED-BINARY` |
| `DAT_0077896c` | pending-damage queue count, **cap 0x40 = 64** | `CONFIRMED-BINARY` |
| `DAT_00778970` | queue base; stride **0x44 = 68 bytes** per record | `CONFIRMED-BINARY` |

Note the falloff is **not clamped**. Beyond `ECX[+0x38]` the term goes negative, which would
*heal*. Either the caller (`Collision_FindTargetsInRadius`) guarantees distance <= radius, or
negative damage is reachable. **Unresolved — must be checked before this is translated.**

## CORRECTION to the prior record — R9
A Ghidra comment left by the previous attempt (dated 2026-08-31) states:

> `falloff = (flags & 0x2000) ? 1.0 : (1.0 - distance/blastRadius) * baseDamage`

**That is wrong.** In the flag-set branch the value is `ECX[+0x40]` (base damage), **not
`1.0`**. Base damage is a factor in *both* branches; the flag only removes the distance
term. Reading it as written would make every no-falloff weapon do `1.0 x projectileScale`
damage instead of its real base damage.

The rest of that comment (queue globals, cap 64, call chain) matches the code.

## CONFLICT — RESOLVED 2026-09-09

**They are two different structs.** See `04_spec/structs/weapon_two_struct_split.md`.

`ECX` here is the shared **`WeaponMountEntry`** (356 bytes; `DAT_00778928` base,
`DAT_00778924` count, stride `0x59` ints = `0x164` bytes — proven by
`Weapon_ProjectileUpdateTick`'s array bound matching the loader's `calloc`). Its `+0x40` is
**base damage**.

The vehicle's `+0x5e4`/`+0x5e8` point at a per-vehicle **weapon slot** whose `[0]` is a
*pointer to* the `WeaponMountEntry` — visible in `Vehicle_ReadPlayerControlInput` as
`*piVar5 + 0x54`, dereferencing before the offset. The slot's own `+0x40` is the
**next-fire timestamp** and `+0x44` the refire interval.

Both original readings were correct; the error was assuming one struct.

### Superseded — the original conflict note

`ECX[+0x40]` is read here as a **float base damage**.

But in `Vehicle_ReadPlayerControlInput` (`0x00425a20`) the *weapon mount* pointer's `+0x40`
is used as a **next-allowed-fire timestamp**, set to `mount[+0x44] + gameTime`. The same
offset cannot be both.

Three possibilities, none yet eliminated:
1. `ECX` here is a **weapon definition / config record**, a different struct from the mount
   instance that `+0x5e4`/`+0x5e8` point at.
2. `ECX` here **is** the mount and one of the two readings is wrong.
3. They are the same struct and the field is genuinely overloaded by state.

`recoil_confirmed_logic.md` records `WeaponMountEntry` as 356 bytes (`0x164`), which admits
both offsets, so size does not settle it.

**Resolution path:** read `Weapon_LoadMountArrayFromConfig` (`0x004b1190`) to see which
struct is built and what it writes at `+0x38`/`+0x40`/`+0x44`/`+0x54`, and check what
`Weapon_ProjectileUpdateTick` passes in ECX. Until then **neither reading may be used in
the remake.**

## Open questions
- The unclamped negative-damage case above.
- What the 0x44-byte queue record holds (10 dwords copied from the candidate, plus damage
  at +0x3c of the record) and who drains it.
- `param_1` candidate array layout: stride 10 dwords, `[7]` is distance, `[9]` is a pointer
  whose `+0xbc` is the owner id.
