# Damage and destructibles — how shooting a generator opens a gate

All `CONFIRMED-BINARY`. This is the chain the previous attempt reported as *"did not work
at all"*.

## The chain, end to end

```
weapon hit
  -> Weapon_ApplyDamageToTarget (0x4b26f0)
  -> Vehicle_TakeDamage (0x43bcc0) / Turret_TakeDamage (0x437d60) / AIVehicle_TakeDamage
  -> health reaches 0
  -> Vehicle_DeactivatePartsOnDeath (0x43c850)
  -> gwNodeSetActive(partNode, 0)          <- clears node flag bit 0x4
  -> Mission_TickAndCheckObjectiveCompletion sees INACTIVE node no longer active
  -> objective completes
```

Every link is now read. The load-bearing step is `gwNodeSetActive(node, 0)`: the *same*
flag bit the objective test reads (`04_spec/systems/objective_triggers.md`) and the *same*
bit the collision broadphase requires for a hit (`04_spec/systems/collision.md`).

## `Vehicle_TakeDamage` (`0x0043bcc0`)

### Damage is refused in three cases
```c
if (inst[+0x5a0] == 0 && inst[+0x60] != 4 && inst[+0x60] != 5) { ...apply damage... }
```
- `inst[+0x5a0] != 0` — an invulnerability flag
- `inst[+0x60] == 4` — inert / no AI route
- `inst[+0x60] == 5` — already dead

### On death
```c
Vehicle_DeactivatePartsOnDeath();
...
inst[+0x60] = 5;
```

## CORRECTION — `inst[+0x60]`: 4 is inert, **5 is dead**

An earlier pass concluded `+0x60 == 4` meant "not an active combat participant", covering
both death and no-route. **That was half right and is corrected here.**

| value | meaning | evidence |
|---|---|---|
| 2 | AI-driven | `Vehicle_ResolveClassConfigAndNetAssignment` |
| **4** | **inert / no AI route** — `Vehicle_DispatchMovementPhysics` skips movement entirely | assignment at spawn; movement guard |
| **5** | **DEAD** — set by `Vehicle_TakeDamage` after `DeactivatePartsOnDeath`; the TRACK mover skips weapon-deploy for it | `0x43bcc0` |

Both 4 and 5 are damage-immune, which is why they behave alike to an outside observer and
why the earlier conflation was easy to make.

### REFINED again — BOTH are death states, for different entity classes
`AIVehicle_TakeDamage` (`0x43b870`) sets `inst[+0x60] = **4**` on death, while
`Vehicle_TakeDamage` (`0x43bcc0`) sets **5**. So:

| value | set by | meaning |
|---|---|---|
| **4** | `AIVehicle_TakeDamage` on death; also at spawn when there is no AI route | **AI vehicle dead, or inert** |
| **5** | `Vehicle_TakeDamage` on death | **player / net vehicle dead** |

This reconciles the debug string: `FUN_0042aa50` prints `"%s is DEAD!"` for **4** and is
**correct** — the debug HUD is showing AI vehicles, for which 4 *is* death. My earlier note
that the string was "simply inaccurate" was itself wrong and is withdrawn.

Consequence for the remake: **the death state depends on which TakeDamage path ran.**
An AI vehicle must die to 4; the player must die to 5. Using one value for both would break
either turret targeting (which skips `!= 4`) or weapon-deploy (which skips `== 5`).

## `Vehicle_DeactivatePartsOnDeath` (`0x0043c850`)

```c
slot = inst + 0x754;
for (i = 0; i < 8; i++) {                      // 8 entries, stride 0x2b dwords = 0xac bytes
    if (slot[0] != 0) {
        gwNodeSetActive(slot[0], 0);           // <- THE DESTRUCTIBLE TRIGGER
        Object3D_SetPosition(slot[4], slot[5], slot[6]);
        Object3D_SetScale(1.0, 1.0, 1.0);
    }
    if (slot[0x15] != 0) {                     // second node slot, +0x54 within the entry
        gwNodeSetActive(slot[0x15], 0);
        Object3D_SetPosition(slot[0x19], slot[0x1a], slot[0x1b]);
        Object3D_SetScale(1.0, 1.0, 1.0);
    }
    slot += 0x2b;
}
```

Also: stops a looping sound (`inst[+0x5bc]`), calls `FUN_0043c800` and `FUN_004b22d0`,
sets `inst[+0x5e0] = 1` and clears `inst[+0xf20]` / `inst[+0xf28]`.

**Up to 16 nodes are deactivated per death** — 8 entries × 2 slots. Each also has its
position restored from a stored triple and its scale reset to 1.0, which matters because
the TRACK mover scales some nodes by speed while alive.

| value | meaning | tag |
|---|---|---|
| **8** | entries in the part array at `inst+0x754` | `CONFIRMED-BINARY` |
| **0xac** (172) | stride per entry | `CONFIRMED-BINARY` |
| **1.0** | scale restored on death | `CONFIRMED-BINARY` |

## `Turret_TakeDamage` (`0x00437d60`)

```c
killed = FUN_004379f0(damage, ...);
if (killed) {
    FUN_004b28e0();
    FUN_0042a9f0(turret[+0x160]);              // max health
} else {
    FUN_004b2900(turret[+0x15c] / turret[+0x160]);   // CURRENT / MAX = health FRACTION
}
```

`+0x15c` and `+0x160` are `HEALTH` current and max, matching `Turret_Construct`'s key map
(indices `[0x57]`/`[0x58]`).

The surviving branch passes a **health fraction** to `FUN_004b2900`.

### RESOLVED — this IS the `DAMAGE_ANIM_ON_HEALTH` selector. `CONFIRMED-BINARY`
`FUN_004b2900` is a one-line setter: `_DAT_00779a84 = param;`

And `Weapon_ApplyDamageToTarget` (`0x4b26f0`) brackets the virtual `TakeDamage` call with it:
```c
_DAT_00779a84 = 1.0;                          // default: undamaged
result = (*target->vtable[1])(target, damage);// the target writes its fraction via FUN_004b2900
if (DAT_00779a80 == 0) {
    n     = mount[+0x100];                    // DAMAGE_ANIM_ON_HEALTH entry count
    pairs = mount + 0x104;                    // [threshold, animHandle] pairs, stride 8
    for (i = 0; i < n; i++)
        if (_DAT_00779a84 <= pairs[i*2]) { FUN_0045dc70(hitPos...); break; }
}
```

So the chain is: **target computes `currentHealth / maxHealth` → writes it to a global →
the damage caller picks the first `DAMAGE_ANIM_ON_HEALTH` entry whose threshold is >= that
fraction → spawns that effect at the hit position.**

For RFPG that is `0.05 greenparts`, `0.15 md_impact.flt`, `1.0 sm_impact.flt`
(`CONFIRMED-DATA`, `weapons.json`) — so a nearly-dead target shows `greenparts`, a hurt one
`md_impact`, and a healthy one `sm_impact`. The **weapon** supplies the effect, keyed on the
**target's** remaining health. `INFERRED` -> `CONFIRMED-BINARY`.

## Not established
- ~~`FUN_004379f0`~~ — **read**, see below.
- ~~`FUN_004b2900`~~ — **read**, it is the health-fraction setter. `FUN_004b28e0` and
  `FUN_0042a9f0` (the death-effect entry points) are still unread.
- `AIVehicle_TakeDamage` (`0x43b870`) — still unread.
- `FUN_0043b730` (CATCHES_FIRE handler) and `FUN_0043b790` (FREEZE handler) — unread.
- Whether the part array at `inst+0x754` is the same structure as the weapon slots reached
  via `+0x5e4`/`+0x5e8`. The strides do not obviously line up and this has **not** been
  checked — do not assume they are the same array.

---

## `FUN_004379f0` — turret health subtraction, and two mechanics

```c
if (turret[+0x168] != 0)                       // ACTIVATE_ON_HIT delay
    turret[+0x16c] = now + turret[+0x168];     // <- being SHOT sets the activation timestamp
if ((turret[+0x164] == 0 || hitNode[+0x24] == turret[+0x164])   // DAMAGE_PART filter
    && (turret[+0x15c] -= damage) <= 0.0) {
    FUN_0045dde0(0,0,0);                       // death effect
    if (turret[+0x108]) { turret[+0x108] = 0; FUN_004aefb0(); }   // stop the firing sound
    return 1;                                  // destroyed
}
return 0;
```

**`DAMAGE_PART` is a weak-point mechanic.** When `turret[+0x164]` is non-zero the turret only
takes damage from hits on that specific part node — shots landing on any other part do
nothing at all. A remake applying damage on any hit would make those turrets far easier to
kill than the original. `CONFIRMED-BINARY` (config key `DAMAGE_PART` -> `[0x59]` = `+0x164`).

**`ACTIVATE_ON_HIT` writes `turret[+0x16c]`** — the same activation-delay timestamp the
turret tick gates on (`if (turret[+0x16c] < now) return`). So shooting a dormant turret is
what starts its activation countdown. `CONFIRMED-BINARY` (config `ACTIVATE_ON_HIT` ->
`[0x5a]`/`[0x5b]` = `+0x168`/`+0x16c`).

## `NetVehicle_TakeDamage` (`0x0043b810`) — weapon-flag damage effects

```c
if (inst[+0x60] == 4) return 0;                        // inert only - NOT 5
if (mount) {
    if (mount[+0x54] & 0x1000)   FUN_0043b730(...);    // CATCHES_FIRE
    if (mount[+0x54] & 0x200000) FUN_0043b790(...);    // FREEZE / DESIGNATE
}
return inst[+0x48];
```

Two `WeaponMountEntry` flag bits get **dedicated damage-side handlers**: `CATCHES_FIRE`
(`0x1000`) and the `FREEZE`/`DESIGNATE` bit (`0x200000`). These are status effects applied
on hit, separate from the health subtraction.

Note this path checks **only** `+0x60 == 4`, not 5 — unlike `Vehicle_TakeDamage`, which
rejects both. Recorded as observed; the asymmetry is not explained.

---

## `AIVehicle_TakeDamage` (`0x0043b870`)

```c
if (inst[+0x60] == 4) return;                       // already dead / inert
...
if (inst[+0x60] == 2 && inst[+0xf84] != 1)          // AI-driven and not already engaged
    inst[+0xfdc] = 1;                                // <- FORCE-ACTIVE LATCH
...
inst[+0x60] = 4;                                     // death
```

**Being shot wakes an AI vehicle**, via the same force-active latch `attack_buddy` uses
(`04_spec/systems/ai_behaviour.md`). So there are now two confirmed ways to wake a dormant
unit regardless of `activate_rad`: a squad-mate alerting it, or being hit.

## `FUN_0043b730` — the `CATCHES_FIRE` handler

```c
inst[+0x48] = 1;                                     // burning flag (what NetVehicle_TakeDamage returns)
inst[+0x50] = FUN_004b2920();                        // damage source id
inst[+0x54] = mount;
inst[+0x4c] = damage;
inst[+0x58] = now + 4.0;                             // BURN DURATION
if (inst[+0x5c]) FUN_0045c040();                     // stop any existing burn effect
inst[+0x5c] = FUN_0045df70(inst[+0xed0], 0, 0);      // spawn the burning animation
```

| value | meaning | tag |
|---|---|---|
| **4.0** | **burn duration in seconds** | `CONFIRMED-BINARY` |

Re-igniting a burning target **replaces** the effect rather than stacking it, and refreshes
the 4-second timer.

## `FUN_0043b790` — the shield / absorb handler

```c
cfg = inst->classConfig;
if (mount[+0x54] & 0x800) {                          // DESIGNATE weapons
    FUN_004b2210(cfg[+0x398]);                       // flat drain of health[0]
    return damage;                                   // damage passes through unchanged
}
if (FUN_004b2210(cfg[+0x39c] * damage) == 1)         // scaled drain of health[1]
    return 0.0;                                      // FULLY ABSORBED
return damage;
```

`cfg[+0x398]` and `cfg[+0x39c]` are the two values of the `health` config key
(`04_spec/structs/config_key_maps.md`, `FUN_00422170`).

**Correction 2026-09-24 (disassembly of `0x0043b790` and `0x004b2210` re-read).** The
"shield" reading above was an interpretation. The bytes show a weapon **effect slot** at
`inst+0x5c4`, which is not a shield.
- `0x004b2210` (`Weapon_UpdateLightSlot`) pushes the slot's target by the amount (subtracts
  it for weapons with flag `0x200`) and clamps it to [-1, 1]. It attaches a pooled light
  coloured from the weapon.
- It returns **2** if the slot's current value is below -0.5, **1** if it lies in
  [-0.5, 0], and **0** if it is positive.
- Damage becomes 0 when the return is 1. That includes a fresh slot (current value 0), so the
  first qualifying hit only charges the slot.
- Only weapons with flag bit 21 reach this function (`Vehicle_TakeDamage`).
- The same slot drives `+0x18` (controls disabled) through `Vehicle_HealthTick` when its value
  falls below -0.5.

Renamed `Vehicle_ApplyEffectSlotHit`; full model in `vehicle.md` section 8 and `weapon.md`
"Effect slots". Not yet oracle-checked.

## Damage subsystem — status
Closed except: `FUN_004b2210` (the absorb function itself), `FUN_004b2920`,
`FUN_004b28e0`, `FUN_0042a9f0`, `FUN_0045df70`, `FUN_0045c040` — all effect/bookkeeping
entry points, none of which change the damage arithmetic.

---

# ORACLE VERIFICATION - the turret damage chain, end to end
`VERIFIED-ORACLE` 2026-09-09, trace `Recoil21` (the first trace in which anything is
destroyed). Turret instance `0x19d98a28`, health field `+0x15c` at `0x19d98b84`.

## Static fields, read live at the first hit
| field | value | meaning |
|---|---|---|
| `+0x15c` | **6.0** | current health |
| `+0x160` | **6.0** | max health - undamaged at first hit |
| `+0x164` | **0** | `DAMAGE_PART` - no weak-point restriction on this turret |
| `+0x168` | **0.0** | `ACTIVATE_ON_HIT` disabled |
| `+0x16c` | **+INF** | activation timestamp - dormant, never auto-activates |
| `_DAT_00779a84` | **1.0** | the fraction global, at its "undamaged" default |

That last row is a direct confirmation of the bracketing in `Weapon_ApplyDamageToTarget`
(`0x4b26f0`): the caller writes `1.0` *before* the virtual `TakeDamage` call, and the target
overwrites it. Read at function entry, it still holds the default.

## The full damage sequence
Every write to the health field across the trace:

```
6.0 (initial)  ->  4.2  ->  2.4  ->  0.6  ->  -1.2
```
| | |
|---|---|
| damage per hit | **-1.800000**, three times, exactly constant |
| weapon | **`wep_1.1` "ERFPG"**, `DAMAGE` = `1.7999999523162842` = `0x3fe66666` = `1.8f` |
| scaling | **none** - the config value is applied verbatim |

`CONFIRMED-DATA` (weapons.json) + `VERIFIED-ORACLE`. Splash was not involved:
`Weapon_ApplyRadiusDamageFalloff` (`0x4b0a50`) never executes in this trace, so this is the
**direct-hit** path.

### Health goes NEGATIVE - do not clamp
The final write is **-1.2**, not `0.0`. `FUN_004379f0` tests `(health -= damage) <= 0.0` and
leaves the overshoot in place. A remake that clamps health at zero would still *kill* the
turret correctly, but any logic reading the stored value after death would differ.
`CONFIRMED-BINARY` + `VERIFIED-ORACLE`

## The health-fraction selector - confirmed exactly
Writes to `_DAT_00779a84` across the trace, distinct values: **1.0, 0.7, 0.4, 0.1**.

| health after hit | fraction written | `health / maxHealth` | |
|---|---|---|---|
| 4.2 | **0.7** | 4.2 / 6.0 = 0.7 | ok |
| 2.4 | **0.4** | 2.4 / 6.0 = 0.4 | ok |
| 0.6 | **0.1** | 0.6 / 6.0 = 0.1 | ok |
| (default before each call) | 1.0 | - | ok |

`FUN_004b2900(turret[+0x15c] / turret[+0x160])` is confirmed against live memory. The
`DAMAGE_ANIM_ON_HEALTH` thresholds `0.05 greenparts / 0.15 md_impact / 1.0 sm_impact` then
select: 0.7 and 0.4 both take `sm_impact`, and **0.1 takes `md_impact`** (0.1 <= 0.15, and
0.1 > 0.05). No observed fraction reaches `greenparts`.

## The death path executes completely
All four functions the death branch calls are hit in this trace:

| | |
|---|---|
| `0x0045dde0` | death effect (from `FUN_004379f0`) |
| `0x004b28e0` | |
| `0x0042a9f0` | called with max health |
| `0x0045dc70` | the `DAMAGE_ANIM_ON_HEALTH` effect spawn |

`Vehicle_DeactivatePartsOnDeath` (`0x43c850`) does **not** hit, and that is correct rather
than a gap - it is the *vehicle* death path. Turret death is a separate branch.

## What this does NOT cover
- **`DAMAGE_PART`**: this turret has it set to 0, so the weak-point filter was never
  exercised. A turret configured with a non-zero `DAMAGE_PART` is still needed.
- **`ACTIVATE_ON_HIT`**: 0 on this turret, so the shot-wakes-a-dormant-turret mechanic is
  unverified.
- **Splash damage**: `0x4b0a50` still has never executed with a target in range.
- **Generator destruction**: trace `Recoil20` contains firing and radius queries but **no
  damage application at all** - nothing was destroyed inside its 30-second window.
