# AI behaviour — NetGraph → vehicle instance

Recovered from `Vehicle_ResolveClassConfigAndNetAssignment` (`0x00420d10`), which runs at
spawn and copies the NetGraph record's behaviour fields onto the vehicle instance. All
`CONFIRMED-BINARY`.

This closes the search logged as failed in the previous pass: the consumers do **not** read
the NetGraph record directly, they read **copies on the instance**, which is why grepping
for the NetGraph offsets found nothing.

## Assignment and mode

```c
if (inst[+0xf70] == 0 || (rec = NetGraph_LookupByIndex()) == NULL)
     inst[+0x60] = 4;          // no AI route
else inst[+0x60] = 2;          // AI-driven
if (inst[+0x60] == 2) { inst[+0xf74] = rec; ...copy fields... }
```

| field | meaning |
|---|---|
| inst +0xf70 | NetGraph **index** from the spawn data |
| inst +0xf74 | **cached NetGraph record pointer** |
| inst +0x60 | **2 = AI-driven, 4 = no route assigned** |

### `+0x60 == 4` — **RESOLVED**, see the section at the end of this file.
Short version: `4` means **not an active combat participant** (inert), reached both by
dying and by having no AI route. "DEAD" is only the debug HUD's label for it.

## Fields copied — ranges are stored SQUARED

| NetGraph key | rec offset | → instance | transform | default if 0 |
|---|---|---|---|---|
| (unnamed) | +0x18 | +0xf84 | enum map `0→0, 1→2, 2→3, 3→4` | — |
| **`activate_rad`** | +0x20 | **+0xfd4** | **squared** | (not defaulted) |
| **`attack_rad`** | +0x24 | **+0xfb0** | **squared** | **1500.0** (`0x44bb8000`) |
| **`attack_dwell`** | +0x28 | +0xf94 | as-is | **10.0** (`0x41200000`) |
| **`not_pursuit_dwell`** | +0x2c | +0xf98 | as-is, only if non-zero | — |
| **`return_range`** | +0x38 | **+0xfb4** | **squared**, only if non-zero | — |
| **`hide_times[0]`** | +0x3c | +0xf9c | as-is | — |
| **`hide_times[1]`** | +0x40 | +0xfa0 | as-is | — |
| **`attack_strategy`** | +0x4c | +0xff0 | as-is (the 4-byte field) | — |

Two timers are primed to `now + 10.0`: `inst[+0xf90]` and `inst[+0xfa8]`.

**`activate_rad`, `attack_rad` and `return_range` are stored squared.** Every comparison
against them is therefore against a squared distance. A remake that compares against the
raw config value will have ranges wrong by a square — e.g. `activate_rad` 40 behaving as
1600. This is exactly the class of error that produces "enemies barely worked".

`inst[+0xf84]` is the **AI mode** — the value the debug HUD prints as
`"%s is in mode %d and had goal node..."` (`FUN_0042aa50`). Note the enum is **remapped** on
copy (`1→2, 2→3, 3→4`), so the config value and the runtime mode are *not* the same number.

## The activation check — confirmed consumer

From `Vehicle_MainUpdateTick` (`0x00426390`), the mode-2 branch:

```c
d2 = FUN_00472730();                 // a SQUARED distance
inst[+0xff8] = d2;
if ((d2 <= inst[+0xfd4] || inst[+0xfdc] != 0) && inst[+0xfd8] == 0) {
    inst[+0xfe8] = 1;
    FUN_00401060();
    goto <run the full update>;      // vehicle becomes active
}
// otherwise: LOD path, full physics skipped
```

**So `activate_rad²` (`+0xfd4`) is the wake-up radius**: an AI vehicle only runs its full
update once the measured squared distance falls within it. `inst[+0xfdc]` forces activation
regardless (a latch, once woken), and `inst[+0xfd8]` suppresses it.

| field | meaning |
|---|---|
| +0xfd4 | `activate_rad²` — wake radius |
| +0xfd8 | suppress activation |
| +0xfdc | force-active latch |
| +0xfe8 | "activated this frame" flag |
| +0xff8 | last measured squared distance |

## Not established
- `FUN_00472730` — confirmed to return the value compared against `activate_rad²`, so it is
  a squared distance, but *between what and what* is unread (presumably vehicle↔player, or
  vehicle↔camera).
- ~~consumers of the copied fields~~ **partly found.** `return_range²` and
  `not_pursuit_dwell` are consumed by `FUN_00401710`'s leash; `attack_strategy` (+0xff0) is
  the AI state there. Still unread: `attack_rad²` (+0xfb0, only reader `FUN_00401b20`),
  `attack_dwell` (+0xf94, readers `FUN_00401c60`/`FUN_00402250`), `hide_times`
  (+0xf9c/+0xfa0 — **no readers found in the corpus at all**).
- What `attack_strategy`'s four bytes mean individually.
- `pursuit_range` (+0x30/+0x34) is **not copied** by this function — either unused, or read
  directly from the cached record at `+0xf74`.

---

## The AI state machine — `FUN_00401710` (`0x00401710`)

The per-tick AI decision function. All `CONFIRMED-BINARY`.

```c
player = DAT_004f3a88->instance;
px = player[+0x3ec]; py = player[+0x3f0]; pz = player[+0x3f4];   // world position

if (inst[+0xfd0] <= now && inst[+0xff0] != 3) FUN_00401f60();    // a periodic re-plan

d.x = px - inst[+0x3ec];
d.y = py - inst[+0x3f0];
d.z = pz - inst[+0x3f4];
d.y_for_length = 0.0;                       // LENGTH IS XZ-ONLY
dist = FUN_00402f60();                      // |d| in the XZ plane
vertical = (dist == 0) ? 0 : d.y / dist;    // vertical ratio, kept separately

// rotate the delta into the vehicle's own heading frame
right   = inst[+0x388]*d.x - inst[+0x380]*d.z;
forward = inst[+0x388]*d.z + inst[+0x380]*d.x;

if (inst[+0xffc] > 6) inst[+0xff0] = 5;     // a counter forces state 5

switch (inst[+0xff0]) {                     // AI STATE
  case 0: FUN_00401970(forward, right, dist); break;
  case 1: FUN_00401a40(forward, right, dist); break;
  case 2: FUN_00401ab0(forward, right, dist); break;
  case 3: FUN_00401180();                    break;
  case 5: FUN_00402170();                    break;   // forced-state handler
  case 6: if (inst[+0x1018] == 0) inst[+0xff0] = inst[+0xff4];  // restore saved state
}

if (classConfig[+0xa4] == 2)                 // SUB movement type
    { ... writes vertical control to inst[+0x70]/+0x74 and +0x84/+0x88 ... }

FUN_00402250(dist, flag);

// leash / disengage
if (player[+0x60] == 4 || FUN_00472670() > inst[+0xfb4]) {   // return_range squared
    FUN_00402080();
    inst[+0xf90] = inst[+0xf98] + now;       // not_pursuit_dwell timer
}
```

### `attack_strategy` is the AI STATE, at `inst[+0xff0]`

| state | handler | notes |
|---|---|---|
| 0 | `FUN_00401970` | |
| 1 | `FUN_00401a40` | reads `attack_strategy` field |
| 2 | `FUN_00401ab0` | reads `attack_strategy` field |
| 3 | `FUN_00401180` | the only state that suppresses the periodic re-plan |
| 5 | `FUN_00402170` | **forced** when `inst[+0xffc] > 6` |
| 6 | — | a *suspended* state; restores `inst[+0xff4]` when `inst[+0x1018]` clears |

So the four bytes at NetGraph `+0x4c` copied to `inst[+0xff0]` are the **initial state**, and
the field is subsequently overwritten at runtime — it is state, not a static strategy.

### Distance is XZ-only, but the vertical is kept
`d.y` is zeroed before the length is taken, so `dist` is a **horizontal** distance. The
vertical component survives as a separate ratio used only by SUB-type vehicles. AI
engagement geometry is therefore flat, matching the turret behaviour.

### The leash
`return_range²` (`inst[+0xfb4]`) is compared against `FUN_00472670()`. Exceeding it — or the
player being state 4 — triggers `FUN_00402080()` (return home) and arms the
`not_pursuit_dwell` timer at `inst[+0xf90]`.

---

## `inst[+0x60] == 4` — conflict RESOLVED

Both earlier readings were right; the label was the problem.

Evidence assembled:
- `Vehicle_ResolveClassConfigAndNetAssignment` sets `4` when a vehicle has **no AI route**.
- `FUN_0042aa50`'s debug HUD prints `"%s is DEAD!"` for `4`.
- `Turret_UpdateAITick` will **not target** an entity whose `+0x60 == 4`.
- `FUN_00401710` makes an AI vehicle **disengage and return home** when the player's
  `+0x60 == 4`.
- `Vehicle_DispatchMovementPhysics` **skips movement entirely** for `4`.

**AMENDED 2026-09-09 — this was half right.** `Vehicle_TakeDamage` (`0x43bcc0`) sets
`inst[+0x60] = 5` on death, **not 4**. The correct split is:

- **4 = inert / no AI route** (movement skipped entirely)
- **5 = DEAD** (set after `DeactivatePartsOnDeath`; weapon-deploy skipped)

Both are damage-immune, which is why they look alike from outside and why the conflation was
easy. The debug HUD's `"%s is DEAD!"` string prints for **4**, and is simply inaccurate — it
was the source of the original error. **Do not treat debug strings as evidence of semantics.**

Full detail in `04_spec/systems/damage_and_destructibles.md`.

---

## AI states — what each one does

All `CONFIRMED-BINARY`. Handlers receive `(forward, right, dist)` in the vehicle's own
heading frame, where `dist` is the **horizontal** distance to the player.

### State 0 — pursue to a standoff band (`FUN_00401970`)
The only state that drives. Writes the **same control axes the player uses**
(`+0x68` throttle, `+0x6c` steer, mirrored to demands `+0x7c`/`+0x80`).

```c
if (forward <= 0.0) {                      // target is BEHIND
    throttle = 0;
    steer    = (right >= 0) ? +1 : -1;     // turn on the spot toward it
} else {
    steer = right;                          // proportional steering
    if      (dist > netgraph[+0x34]) throttle = +1.0;   // pursuit_range[1] -> close in
    else if (dist < netgraph[+0x30]) throttle = -1.0;   // pursuit_range[0] -> back off
    else                             throttle =  0.0;   // hold the band
}
```

**`pursuit_range` IS used** — and read **directly from the cached NetGraph record**
(`inst[+0xf74]`), not from a copied field. That answers the open question from the previous
pass. It is a **standoff band**: `[0]` is the minimum stand-off distance, `[1]` the maximum.
The AI reverses when the player gets too close.

Note the steering when the target is behind is a **hard ±1**, i.e. full-lock turn-in-place,
not proportional.

### States 1 and 2 — aim, then suspend (`FUN_00401a40`, `FUN_00401ab0`)
Structurally identical; they differ only in the fallback they call when
`FUN_00401420()` returns 0 (`FUN_004026d0` for state 1, `FUN_004028c0(dist)` for state 2).

```c
if (!FUN_00401420()) { <fallback>; return; }
Vehicle_ComputeAimYawToPoint();
throttle = steer = 0; demands = 0;         // stop dead
inst[+0xff4] = inst[+0xff0];               // save current state
inst[+0xff0] = 6;                          // -> SUSPENDED
```
So both are **stop-and-aim** states that park the vehicle in state 6 until something clears
`inst[+0x1018]`, at which point `FUN_00401710` restores the saved state.

### State 3 — `FUN_00401180`
The only state that **suppresses the periodic re-plan** (`FUN_00401f60`). 660 bytes, four
callers. **Not read this pass.**

### State 5 — `FUN_00402170`
Entered by force when `inst[+0xffc] > 6`. **Not read this pass.**

### State 6 — suspended
Not a handler; `FUN_00401710` restores `inst[+0xff4]` when `inst[+0x1018] == 0`.

## The attack decision — `FUN_00401b20`

```c
if (DAT_004f36ac == 0 && inst[+0xf90] < now) {      // global AI-fire enable; dwell expired
    d2 = FUN_00472670();
    if (d2 < inst[+0xfb0]) {                         // within attack_rad SQUARED
        if (FUN_00401d50(1)) {                       // line of sight
            FUN_00401c60();                          // engage
            inst[+0x448] = 1;
            if (netgraph[+0x48] != 0) FUN_00401c00();// attack_buddy -> call the linked unit
            inst[+0xffc] = 0;                        // reset the failure counter
            return 1;
        }
        inst[+0x448] = 0;                            // in range but no LOS
    }
}
return 0;
```

| element | meaning |
|---|---|
| `DAT_004f36ac` | **global** that disables all AI attacking when non-zero |
| `inst[+0xf90]` | dwell deadline — armed with `not_pursuit_dwell` by the leash, and with `attack_dwell` elsewhere |
| `inst[+0xfb0]` | **`attack_rad²`** — the engagement gate |
| `FUN_00401d50(1)` | **the same LOS test the turret uses** (turret passes 1 or 2) |
| `netgraph[+0x48]` | **`attack_buddy`** — non-zero calls `FUN_00401c00` to alert a linked unit |
| `inst[+0x448]` | has-line-of-sight flag (also the word `Vehicle_MainUpdateTick` ORs bit 2 into) |
| `inst[+0xffc]` | **failed-engagement counter** — reset to 0 on a successful attack, and `> 6` forces state 5 |

So `inst[+0xffc]` counts consecutive ticks where the AI wanted to attack and could not.
**After six, the AI is forced into state 5** — a give-up / reposition behaviour.

## Return home — `FUN_00402080`
```c
inst[+0xf84] = inst[+0xf88];    // restore AI mode from its saved original
```
`inst[+0xf88]` holds the **original** AI mode; the leash restores it.

## Still unread
- `FUN_00401180` (state 3) and `FUN_00402170` (state 5) — the two largest handlers.
- `FUN_00401420` — the predicate states 1 and 2 branch on.
- `FUN_00401d50(mode)` — the LOS test shared with turrets; `mode` 1 vs 2 still unexplained.
- `FUN_00401c60` (engage), `FUN_00401c00` (alert buddy), `FUN_004026d0`, `FUN_004028c0`,
  `FUN_00401f60` (re-plan), `FUN_00402250`.
- What clears `inst[+0x1018]` to release state 6.
- `hide_times` (+0xf9c/+0xfa0) — **still no readers anywhere in the corpus.**

---

## Line of sight — `FUN_00401d50(mode)` (shared by turrets AND AI vehicles)

```c
player = DAT_004f3a88->instance;
DAT_0057da28 = player[+0x444];              // saved before the trace (ignore-id?)
...
if (mode == 1) { from = EDX (the AI/turret point); to   = player[+0x410..0x418]; }
else           { from = player[+0x410..0x418]; to = EDX; }
hit = Collision_GridDDATraversal(from.x, from.y, from.z, to.x, to.y, to.z);
```

**`mode` selects the ray DIRECTION, not the test.** Mode 1 traces *from the shooter to the
player*; mode 2 traces *from the player to the shooter*. Same segment, opposite order — and
a DDA grid traversal returns the **first** hit along its direction, so the two are not
interchangeable when the ray clips geometry at either end.

`player[+0x410..+0x418]` is the player's **aim/eye point** — the `position + offset` triple
established in `03_re/decomp/0x00426770_Vehicle_MoveType3_Track.md`
(`+0x410 = +0x404 + +0x3ec`). LOS is tested to that point, not to the vehicle origin.

### Return condition — RESOLVED by disassembly, `CONFIRMED-BINARY`
Ghidra showed `local_504` as uninitialised because it did not know the callee writes
through `EDX`. The disassembly (`0x00401d9b`-`0x00401e44`) shows `LEA EDX,[ESP+0xc]` is an
**output buffer** that `Collision_GridDDATraversal` fills:

```asm
LEA  EDX,[ESP+0xc]          ; out-buffer (0x504 bytes reserved)
MOV  ECX,[0x004f36b8]       ; the world/grid pointer
CALL 0x00444e90             ; Collision_GridDDATraversal -> EAX
MOV  EDI,EAX
TEST EDI,EDI
JNZ  ret1                   ; traversal non-zero            -> LOS
MOV  EAX,[ESP+0xc]          ; first dword of the out-buffer
TEST EAX,EAX
JZ   ret1                   ; buffer empty                  -> LOS
XOR  EAX,EAX ; ret 0        ; otherwise                     -> BLOCKED
```

So: **`LOS = (traversalResult != 0) || (outBuffer[0] == 0)`** — blocked only when the
traversal returns 0 *and* the out-buffer's first field is non-zero.

`04_spec/systems/collision.md` now establishes the traversal's return sense: it returns
`*outBuf < 1`, i.e. **true when nothing was hit**. So both clauses mean the same thing and
the test reads naturally as "clear line". `CONFIRMED-BINARY`.

### The trace temporarily disables collision on both endpoints
Before the traversal, `FUN_004481b0` is called twice with `EDX = 0` — once with the
shooter (`ECX = this`) and once with `[player + 0xed0]` — and twice again afterwards with
`EDX = 1`. So it **disables then re-enables** collision on both the shooter and the target,
so the ray cannot hit either endpoint. A remake must exclude both, not just the shooter.

Also set around the trace: `FUN_00443c50(1)` before / `FUN_00443c50(0)` after, and
`FUN_00443c60(0x40000)` — a collision filter mask pushed for the duration.

## `FUN_00401420` — the "blocked ahead" predicate (states 1 and 2)

```c
if (inst[+0x4dc] != 0 || inst[+0x4e4] != 0) {   // already in contact
    inst[+0xffc]++;                              // <- increments the FAILURE COUNTER
    return 1;
}
v = inst[+0xa4..+0xac];                          // world velocity
speed = max(|v|, 1.0);
t = speed * 0.5 - classConfig[+0x114];
probe = inst[+0x3ec..+0x3f4] + v * t;            // look-ahead point
FUN_00423b10(2, ...);                            // probe the segment
return (inst[+0x4a0] == 0 && inst[+0x480] == 0) ? 0 : 1;
```

It projects the vehicle's own velocity forward by a **speed-dependent** distance and tests
whether the path is obstructed. So **states 1 and 2 are "blocked" states**: when the way
ahead is obstructed the vehicle stops, aims, and suspends.

**This closes the loop on the give-up counter.** A vehicle already in contact increments
`inst[+0xffc]` every tick, and `FUN_00401710` forces **state 5** once it exceeds 6. So state
5 is the **unstick behaviour** for a vehicle wedged against geometry — six blocked ticks and
it changes tactics. That is a specific, observable behaviour a remake would otherwise miss
entirely, and its absence would look exactly like "enemies get stuck on walls".

## `attack_buddy` — squad alert, `FUN_00401c00`

```c
if (DAT_004f36ac != 0) return;                   // global AI-fire disable
for (n = self[+0x0c]; n != self; n = n[+0x0c]) { // CIRCULAR linked list of squad members
    if (n->instance[+0xf84] != 1) {              // AI mode != 1
        FUN_00401c60();                          // engage
        n->instance[+0xfdc] = 1;                 // FORCE-ACTIVE latch
        n->instance[+0xfe4] = now + 10.0;
    }
}
```

Vehicle node `+0x0c` is a **circular list** linking squad members — `Vehicle_ResolveClassConfigAndNetAssignment`
initialises it self-referentially (`node[+0x0c] = node`).

**Alerting a buddy sets `+0xfdc`, the force-active latch** — which bypasses the
`activate_rad²` wake test in `Vehicle_MainUpdateTick` entirely. So one unit spotting the
player wakes its whole squad **regardless of distance**, for 10 seconds.

This is a major behaviour: enemies do not wake independently by proximity alone. A remake
implementing only `activate_rad` would have squads that never coordinate.

---

## State 5 — the unstick behaviour (`FUN_00402170`)

```c
d = playerPos - selfPos;  d.y = 0;           // XZ only
right   = m388*d.x - m380*d.z;
forward = m388*d.z + m380*d.x;
steer = (forward <= 0.0) ? sign(right) : right;   // hard +-1 if the player is behind
inst[+0x6c] = steer;  inst[+0x80] = steer;
inst[+0x68] = 0;      inst[+0x7c] = 0;            // THROTTLE ZERO
```

**State 5 turns in place toward the player and does not drive.** Entered when the blocked
counter `inst[+0xffc]` exceeds 6. So a vehicle wedged against geometry stops pushing into it
and rotates on the spot — the unstick. `CONFIRMED-BINARY`

## The periodic re-plan — breadcrumb path nodes (`FUN_00401f60`)

Runs from `FUN_00401710` whenever `inst[+0xfd0] <= now` and the state is not 3.

```c
if (FUN_00472670() >= 400.0) {                    // squared distance >= 400 (i.e. 20 units)
    n = calloc-ish malloc(0x30);                  // 48-byte path node, zeroed
    n[0..2] = selfPos;  n[3] = EDX;  n[10] = -1;
    n[6] = malloc(0x3c);                          // 60-byte sub-block, zeroed
    FUN_00403620(n.pos, oldNode.pos, 10.0);       // link new -> previous, radius 10.0
    inst[+0xf78] = n;                             // becomes the current goal node
    inst[+0xf80] = 0;
    inst[+0xfd0] = now + 1.0;                     // next re-plan in 1 second
}
```

**The AI drops a breadcrumb trail.** Once per second, if it has moved at least 20 units, it
allocates a path node at its current position and links it to the previous one with a
radius of 10.0. `inst[+0xf78]` is the **current goal node** — the same field
`FUN_0042aa50`'s debug string prints as `"...had goal node %d"`.

| value | meaning | tag |
|---|---|---|
| **400.0** | squared distance threshold — 20 units of travel before a new node | `CONFIRMED-BINARY` |
| **10.0** | link radius passed to `FUN_00403620` | `CONFIRMED-BINARY` |
| **1.0** | re-plan interval, seconds (`inst[+0xfd0] = now + 1.0`) | `CONFIRMED-BINARY` |

State 3 is the only state that suppresses this, which fits state 3 being a
follow-the-existing-path state rather than a pursue state. **Not confirmed — `FUN_00401180`
is still unread.**

---

## State 3 — path following (`FUN_00401180`)

The hypothesis that state 3 follows the breadcrumb path is **confirmed**. It is the only
state that suppresses the re-plan because it is *consuming* the path rather than extending it.

### Path node layout — inferred from the indexing
```c
node   = inst[+0xf78];               // current path node (48 bytes, from FUN_00401f60)
i      = inst[+0xf80];               // current waypoint index within the node
wpPos  = node[+0x0c + i*4];          // pointer to the waypoint position
wpAux  = node[+0x18 + i*4];          // a parallel array, passed to FUN_00401580
```
So a node carries **two parallel arrays** — positions at `+0x0c` and something else at
`+0x18` — indexed by `inst[+0xf80]`. `FUN_00401580(&aux, dir)` advances to the next
waypoint. `INFERRED` on the array semantics; the indexing itself is `CONFIRMED-BINARY`.

### Blocked while following
```c
if (FUN_00401420()) {                        // path ahead obstructed
    inst[+0xf8c] = inst[+0xf84];             // save AI mode
    inst[+0xf84] = 5;
    inst[+0x1018] = 1;                       // <- the flag that HOLDS state 6 suspended
    throttle = steer = 0;
    return;
}
```
This is what sets `inst[+0x1018]`, the flag `FUN_00401710` waits on to release state 6.
So the suspend/release cycle is: blocked → mode 5 + `+0x1018 = 1` → state 6 holds until
something clears it.

### Steering to the waypoint
```c
d = wpPos - selfPos;  d.y = 0;
dist    = |d|;                                        // horizontal
right   = m388*d.x - m380*d.z;
forward = m388*d.z + m380*d.x;

if (forward >= 0.0) {                                 // waypoint AHEAD
    throttle = max(1.0 - |right|, 0.25);              // slow down in turns, floor 0.25
    steer    = right;
    inst[+0xfec] = 1;                                 // "was ahead" latch
} else {                                              // waypoint BEHIND
    if (inst[+0xfec] != 0) {                          // we passed it
        FUN_00401580(...);                            // advance waypoint
        inst[+0xfec] = 0;
        FUN_00401180();                               // re-run for the new waypoint
        return;
    }
    throttle = 0;  steer = (right >= 0) ? +1 : -1;    // turn in place
}
if (classConfig[+0xa4] == 2)                          // SUB: vertical control
    inst[+0x74] = inst[+0x88] =
        ((wpPos.y - selfPos.y + classConfig[+0x228]) * _DAT_004da0c0 - inst[+0x3bc])
        * _DAT_004da0c4;

if (dist < 10.0) { FUN_00401580(...); inst[+0xfec] = 0; }   // ARRIVED -> next waypoint
```

| value | meaning | tag |
|---|---|---|
| **0.25** | **minimum throttle while turning** — the floor on `1.0 - \|steer\|` | `CONFIRMED-BINARY` |
| `1.0 - \|steer\|` | **throttle scales down with steering** — the AI slows in corners | `CONFIRMED-BINARY` |
| **10.0** | **waypoint arrival radius** | `CONFIRMED-BINARY` |

**Two independent ways to advance a waypoint**: arriving within 10.0 units, *or* overshooting
it (the `forward` sign flipping negative after having been positive, tracked by the
`+0xfec` latch). A remake implementing only the arrival radius would leave AI vehicles
circling waypoints they had already passed.

**The throttle/steering coupling is a visible handling trait**: an AI vehicle at full lock
drives at 25% throttle, and at a gentle angle at nearly full throttle. Constant-speed AI
would look obviously wrong on the same path.

| field | meaning |
|---|---|
| +0xf78 | current path node |
| +0xf80 | current waypoint index within it |
| +0xf8c | saved AI mode (while blocked) |
| +0xfec | "waypoint was ahead" latch, for overshoot detection |
| +0x1018 | blocked flag — holds state 6 suspended |

---

## Engage — `FUN_00401c60`, and the fire duty cycle — `FUN_00402250`

Both `CONFIRMED-BINARY`. These were the last two unread functions in the AI chain and they
close the `attack_dwell` question.

### `FUN_00401c60` — entering the engaged state
```c
if (DAT_004f36ac != 0) return;               // global AI-fire disable
inst[+0xf88] = inst[+0xf84];                 // SAVE the current AI mode
inst[+0xfa8] = now;                          // fire-window start
inst[+0xfac] = inst[+0xf94] + now;           // fire-window end  <- attack_dwell
inst[+0xf84] = 1;                            // AI MODE 1 = ENGAGED
wp = pathNode[+0x0c + inst[+0xf80]*4];
inst[+0xfb8..+0xfc0] = *wp;                  // remember the waypoint being left
if (inst[+0xff0] == 2)                       // AI state 2
    inst[+0xfc4..+0xfcc] = selfPos - playerPos, with Y zeroed;
```
`inst[+0xf88]` is the field `FUN_00402080` (return home) restores from — so the save/restore
pair is now closed: engaging saves the mode, the leash puts it back.

**AI mode 1 = engaged.** This is why `attack_buddy` skips squad members already at mode 1 —
they are already fighting.

### `FUN_00402250` — the fire duty cycle
```c
slot = inst[+0x5e4];                          // weapon slot
if (inst[+0xfac] < now) {                     // window expired -> restart the cycle
    t = inst[+0xf98] + now;                   // not_pursuit_dwell  = the GAP
    inst[+0xfa8] = t;                         // next window starts
    inst[+0xfac] = inst[+0xf94] + t;          // attack_dwell       = the BURST length
}
if (inst[+0x5bc] == 0
 && slot[+0x40] < now                         // weapon refire timer elapsed
 && inst[+0xfa8] < now                        // inside the fire window
 && inst[+0x18] == 0                          // controls not disabled
 && param_2 > 0.75                            // ALIGNMENT threshold
 && param_1 < slot[+0x4c]                     // below max engagement range
 && slot[+0x48] < param_1)                    // above min engagement range
    if (FUN_00401d50(1)) { ...fire... }
```

**This is the AI's rate of fire, and it is a duty cycle, not a simple cooldown:**

| field | role | tag |
|---|---|---|
| `attack_dwell` (+0xf94) | **burst length** — how long the fire window stays open | `CONFIRMED-BINARY` |
| `not_pursuit_dwell` (+0xf98) | **gap** — delay before the next window opens | `CONFIRMED-BINARY` |
| +0xfa8 / +0xfac | window start / end timestamps | `CONFIRMED-BINARY` |
| **0.75** | **alignment threshold** — the AI will not fire unless `param_2` exceeds it | `CONFIRMED-BINARY` |
| slot +0x48 / +0x4c | **per-weapon min / max engagement range** | `CONFIRMED-BINARY` |

So `not_pursuit_dwell` is **not** only the leash timer — it is also the pause between
bursts. And the AI has a **minimum** engagement range as well as a maximum: it will not
fire at a target that is too close.

Four independent gates must all pass before an AI shot: the weapon's own refire timer, the
burst window, the alignment threshold, and the range band — then line of sight. A remake
firing on cooldown alone would produce a continuous stream rather than the original's
bursts.
