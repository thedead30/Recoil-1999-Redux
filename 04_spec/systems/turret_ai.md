# Turret AI — `Turret_UpdateAITick` (`0x00436e40`)

All `CONFIRMED-BINARY` from that function. Config offsets cross-checked against
`Turret_Construct`'s key map (`04_spec/structs/config_key_maps.md`) — they agree.

## Activation guards (run order)
The tick returns immediately unless **all** hold:
1. node `turret[+0x0c]` has active flag bit `0x4` set,
2. node `turret[+0x08]` has it set,
3. `turret[+0x30]` (the **`DEACTIVATE`** node) is either null or still active.

If any fails: the muzzle-effect node `turret[+0x6c]` is deactivated via
`gwNodeSetActive(node, 0)`, the firing-sound flag `turret[+0x108]` is cleared, and
`FUN_004aefb0()` stops the sound.

Then a time gate: `if (turret[+0x16c] < now) return;` — a per-turret activation delay.

## Target selection — Manhattan distance, XZ only

```c
best = HUGE;
for (k = 0; k < 8; k++) {                      // turret[+0x78] = array of 8 candidate nodes
    n = turret[+0x78 + k*4];
    if (n == 0) break;
    if ((nodeFlags(n) & 4) == 0) continue;     // candidate must be active
    p = worldPos(n);
    d = |turret[+0x10] - p.x| + |turret[+0x18] - p.z|;   // MANHATTAN, X and Z only
    turret[+0xa4] = d;
    if (d < best) { best = d; target = p; }
}
// the player is an additional candidate, but only if:
if (Render_IsNodeTypeEnabledForRender() && playerInstance[+0x60] != 4) {   // player not DEAD
    d = |turret[+0x10] - player.x| + |turret[+0x18] - player.z|;
    if (d < best) { best = d; target = player; }
}
if (turret[+0xa0] < best) { turret[+0xa8] = 0; }   // out of DETECTION_RANGE -> no target
else                      { ... acquire ... }
```

**Three things a remake will get wrong by default:**
1. **Distance is Manhattan (`|dx| + |dz|`), not Euclidean.** A turret's effective detection
   area is a diamond, not a circle, and is up to √2× larger along the axes.
2. **Height is ignored entirely** — no `y` term. A target directly above or below at the same
   XZ is at distance 0.
3. **At most 8 candidate nodes**, scanned in array order, stopping at the first null.

`turret[+0xa0]` is **`DETECTION_RANGE`** (`Turret_Construct` writes it at index `[0x28]` =
`+0xa0`) — so `DETECTION_RANGE` is compared against a Manhattan sum, not a true distance.

## Firing

```c
if (DAT_004f3fdc != 0) {                       // a global "may fire this frame" gate
    DAT_004f3fdc = 0;  DAT_004f3fe0 = DAT_004f3fe4;
    mode = (turret[+0x150] == 1) ? 2 : 1;
    if (FUN_00401d50(mode)) {                  // line-of-sight / validity check
        turret[+0xa8] = 1;                     // has target
        turret[+0x10c] = 1;
        turret[+0x110] = targetPos;
        if (turret[+0xfc] != 0.0)
            turret[+0x100] = now + turret[+0xfc];   // FIRE_DWELL -> next-allowed time
    } else {
        if (turret[+0xfc] == 0.0)             turret[+0xa8] = 0;
        else if (turret[+0x100] <= now)       turret[+0xa8] = 0;
    }
}
```

`DAT_004f3fdc` is a **global** consumed and cleared by whichever turret runs first — turrets
therefore compete for a shared per-frame firing slot rather than each firing independently.
That is a load-bearing behaviour: it limits how many turrets can engage in one frame.

## Muzzle effect and refire
```c
if (turret[+0xec] <= now) {
    fx = turret[+0x6c];
    if (fx && !(nodeFlags(fx) & 4)) {
        gwNodeSetActive(fx, 1);
        turret[+0xec] = turret[+0x70] + now;   // effect on, for turret[+0x70] seconds
    } else {
        if (fx) gwNodeSetActive(fx, 0);
        FUN_00437730(targetPos);               // aim
        if (turret[+0xac] == 0) FUN_00437820();
        else { FUN_0045e0d0(turret); FUN_0045dde0(0,0,0); }
    }
}
```

## Field map

| offset | meaning | config key |
|---|---|---|
| +0x08, +0x0c | gating nodes (must be active) | `PARTS` |
| +0x10, +0x18 | turret world **x**, **z** | |
| +0x28, +0x2c | an anim/aim object and a flag | |
| +0x30 | `DEACTIVATE` node | `DEACTIVATE` |
| +0x6c | muzzle-effect node | `EFFECT` |
| +0x70 | muzzle-effect duration | `EFFECT` |
| +0x78..+0x94 | **8 candidate target nodes** | `TARGETS` |
| +0x98 | weapon `WeaponMountEntry` pointer | `WEAPON` |
| +0xa0 | **`DETECTION_RANGE`** (Manhattan) | `DETECTION_RANGE` |
| +0xa4 | last computed distance | |
| +0xa8 | **has-target flag** | |
| +0xac | selects between two fire paths | |
| +0xec | muzzle-effect / refire timestamp | |
| +0xf0, +0xf4 | a ramp: `+0xf0` accumulates `dt` toward `+0xf4`, then resets `+0xec` | |
| +0xfc | **`FIRE_DWELL`** | `FIRE_DWELL` |
| +0x100 | next-allowed-fire timestamp | |
| +0x104, +0x108 | firing-sound state | |
| +0x110 | current target position pointer | |
| +0x150 | selects LOS mode 2 vs 1 | |
| +0x154 | if set, re-aims even with no target | |
| +0x16c | **activation-delay timestamp** | |

## Two code paths — the MINE branch
The function splits on `(mountFlags >> 0xd) & 1`, i.e. **flag `0x2000` = `MINE`**
(`04_spec/structs/WeaponMountEntry.md`). A turret whose weapon is a mine runs a shorter
path that never calls the aim/fire helpers `FUN_004374a0` / `FUN_00437730`. The two paths
share the identical target-selection block, duplicated by the compiler.

## Not established
- `FUN_00401d50(mode)` — the LOS/validity test. Unread; `mode` 1 vs 2 unexplained.
- `FUN_00437730` (aim), `FUN_00437820`, `FUN_004374a0`, `FUN_00437990`, `FUN_00437430` — unread.
- `DAT_004f3fdc` / `+0xfe0` / `+0xfe4` — the shared firing-slot globals; who sets `0xfdc` back
  to non-zero each frame is not identified.
- Whether `TARGETS` in config populates `+0x78` with exactly 8 slots or fewer.

---

## Map 1 turret data — CONFIRMED-DATA

`02_extracted/zrdr_m1/ai.json` is the **turret** configuration (not the NetGraph AI data,
which lives elsewhere — noted because the filename invites the wrong assumption).

| turret | HEALTH | WEAPON | FIRE_RATE | FIRE_LIMITS | DETECTION_RANGE |
|---|---|---|---|---|---|
| `tur_1**` | 6.0 | `wep_1.0` (RFPG) | 0.1 | 0.8, 2.5 | 300.0 |
| `fcturr**` | 12.0 | `wep_6.1` (LASER) | 0.1 | 0.5, 2.0 | 300.0 |
| `tur_2**` | 8.0 | — | — | — | — |

`PARTS` is `["turret", "gun", "firepoint"]` for each — three node names, matching the
gating nodes and the muzzle point read by the tick.
`DESTROY_ANIM` is `destroy_turret`. `EFFECT` is `["lightning", 0.01]`.

Names end in `**` — a **wildcard** matching all numbered instances (`tur_1a`, `tur_1b`, …).

### Warning: turret `FIRE_RATE` is NOT weapon `FIRE_RATE`
Weapon `FIRE_RATE` is shots per second (RFPG = 12.0) and the loader stores its reciprocal.
Turret `FIRE_RATE` here is **0.1**, which as shots-per-second would be one shot every ten
seconds. `Turret_Construct` writes it to `[0x3a]` (+0xe8), a **different** field from
`FIRE_DWELL` (+0xfc), and the tick uses `+0xfc` for the refire timer.

**Do not assume the two `FIRE_RATE` keys share semantics.** What turret `+0xe8` does is
unread. `INFERRED`-free: no reading is asserted here.

### DETECTION_RANGE 300.0 is a Manhattan sum
Per the tick, `DETECTION_RANGE` is compared against `|dx| + |dz|`. So 300.0 means a diamond
of "radius" 300 along the axes — roughly 212 units along a diagonal, not 300.
