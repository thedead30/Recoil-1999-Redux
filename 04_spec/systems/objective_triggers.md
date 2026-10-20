# Objective triggers — the completion mechanism

From `Mission_TickAndCheckObjectiveCompletion` (`0x00418d40`) and
`Mission_ObjectiveStateMachine` (`0x004184e0`). All `CONFIRMED-BINARY`.

This is the system the previous attempt reported as *"the items that required
destroying/destructible environments to open gates/doors and triggered events did not work
at all"*.

## The completion test — the whole thing

```c
for (i = 0; i < mission[+0x12c]; i++) {          // 0x12c = objective count
    rec = mission + 0x53c + i * 0x31c;           // stride 796 bytes
    if (rec[0] != 0) continue;                   // already completed
    if ( (rec[2] != 0 && (nodeFlags(rec[2]) & 4) != 0)     // ACTIVE  node IS   active
      || (rec[3] != 0 && (nodeFlags(rec[3]) & 4) == 0) ) { // INACTIVE node NOT active
        rec[0] = 1;                              // completed
        mission[+0x138]++;                       // completed count
        mission[+0x108] = 0x67;                  // -> state 103
        mission[+0x114] = i;
        mission[+0x134] = mission[+0x11c] + now; // REVIEW_DELAY deadline
        ...
    }
}
```
where `nodeFlags(n)` is the byte at `n + 0x24`, and **bit `0x4` is the node's active flag** —
the same flag `gwNodeSetActive` (`0x00447c60`) writes.

**So an objective completes when its `ACTIVE` node becomes active, or its `INACTIVE` node
becomes inactive.** That is the entire destructible→gate→objective chain: shoot the
generator → its node is deactivated → the objective naming it under `INACTIVE` fires.

Only the **first** matching objective is processed per tick (`break` after a completion).

## Objective record — stride `0x31c` (796 bytes), array at mission `+0x53c`

| offset | meaning | evidence |
|---|---|---|
| +0x000 | **completed flag** (0 = pending, 1 = done) | the loop above |
| +0x004 | non-zero → the state machine runs for this objective | `case 0x6b` in the tick |
| +0x008 | **`ACTIVE` node pointer** — completes when that node IS active | the loop |
| +0x00c | **`INACTIVE` node pointer** — completes when that node is NOT active | the loop |
| +0x318 | message / text id, copied to `mission[+0x10c]` | state machine `+0x854 + i*0x31c` |

`0x854 − 0x53c = 0x318`, which is how the message offset was derived.

## Mission struct fields

| offset | meaning | source |
|---|---|---|
| +0x100 | running timer | `FUN_0040ed10()` each tick |
| +0x104 | **`READ_TIME`** | config map |
| +0x108 | **state** — see below | |
| +0x10c | current objective's message id | |
| +0x114 | index of the objective being shown | also an `AUTOPLAY` target |
| +0x11c | **`REVIEW_DELAY`** | config map |
| +0x124 | current objective index (from `FUN_00418c30`) | |
| +0x12c | **objective count** | loop bound |
| +0x134 | deadline timestamp for the current state | |
| +0x138 | **completed-objective count** | |
| +0x247c | **`FINAL_MISSION`** flag | config map |
| +0x2480 | a delay timer; when elapsed and the player is state 4 (DEAD), `FUN_00415650` runs | |
| +0x53c | objective array base | |

## State machine — `mission[+0x108]`

| value | dec | transition |
|---|---|---|
| `0x64` | 100 | initial / idle → on deadline: `0x6b` |
| `0x67` | 103 | objective just completed → on deadline: `0x6b` |
| `0x68` | 104 | review pending → on deadline: show review, → `0x69` |
| `0x69` | 105 | review shown |
| `0x6b` | 107 | active; if `rec[+0x04] != 0` runs `Mission_ObjectiveStateMachine` |

The state machine itself branches on whether the state is one of `{100, 0x67, 0x6b}`; for
any other state it plays a sound and calls `FUN_00418620(0 or 1)` depending on
`mission[+0xd0] == 1`.

## Mission end — a hardcoded 5

```c
if (mission[+0x247c] != 0 && mission[+0x138] == 5) {   // FINAL_MISSION && 5 completed
    ... Mission_ShowReviewScreen(1);
    mission[+0x108] = 0x68;
    mission[+0x134] = now + 60.0;
}
```

| value | meaning | tag |
|---|---|---|
| **5** | objectives required to end a `FINAL_MISSION` | `CONFIRMED-BINARY` `0x00418d40` |
| **60.0** | seconds before the review screen advances | `CONFIRMED-BINARY` `0x00418d40` |

The `5` is a literal, not config-driven. A remake reading "all objectives complete" would
be wrong on any final mission with a different objective count.

## What is NOT established
- `FUN_00418c30` (produces `+0x124`), `FUN_004172c0`, `FUN_00417300`, `FUN_00418620`,
  `FUN_00415650`, `FUN_0042bf40` — all unread.
- `mission[+0xd0]` values (1 and 4 are tested) and `+0xdc`, `+0xd4`.
- How `ACTIVE`/`INACTIVE` node **names** in the config are resolved to the node pointers at
  `rec[+0x08]`/`rec[+0x0c]` — that happens in `Mission_LoadObjectiveTriggers` (`0x00418230`),
  whose subtree walk has not been read.
- Whether an objective may name *both* `ACTIVE` and `INACTIVE` (the `||` allows it).

---

## Cross-check against map 1's real data — CONFIRMED-DATA

`02_extracted/objectives/m1_objectives.json`:

| # | name | condition | completion |
|---|---|---|---|
| 1 | `obj1fgate` | `ACTIVE: ["frcoff"]` | when node `frcoff` **becomes active** |
| 2 | `comandlw` | `INACTIVE: ["nb_force"]` | when node `nb_force` **becomes inactive** |
| 3 | `misslw` | `INACTIVE: ["rocket_gull"]` | when node `rocket_gull` **becomes inactive** |
| 4 | `helolw` | **neither key** | **cannot complete via this loop** |

Map-1 values: `READ_TIME` **8**, `REVIEW_DELAY` **4** (`CONFIRMED-DATA`).

The data matches the binary reading exactly: two objectives use `INACTIVE` (destroy
something), one uses `ACTIVE` (something switches on), and each names a single scene node.
This is the destructible→objective chain, end to end, from two independent sources.

### OBJECTIVE4 has no trigger — important
`helolw` declares neither `ACTIVE` nor `INACTIVE`, so `rec[+0x08]` and `rec[+0x0c]` are both
null and the completion test can **never** fire for it. It must complete through a
different mechanism — a script in `interp.zbd`, or a direct write to `rec[0]` from elsewhere.

**A remake that only implements this loop will leave objective 4 permanently incomplete.**
Finding what completes it is a required item, not an optional one. `FUN_004172c0` and
`FUN_00417300` (called on completion) and the interp bytecode are the places to look.

### Objective record also carries, per the data
`name`, `MSG_*_SUMMARY`, `MSG_*_OBJ`, `MSG_*_COMP` (three message ids), `READ_SOUND`
(optionally with a duration, e.g. `24.0`), `HINT_SOUND`/`HINT_SND`, `HINT_TIME`
(180 / 120 seconds), `HINT_ACTIVE`, `HINT_ZONE`.

The `HINT_*` keys do **not** appear in `Mission_LoadObjectiveTriggers`'s key list, so they
are read somewhere else — unlocated. Logged, not guessed.

---

## OBJECTIVE4 — five recovery attempts, all negative (R3 log)

What completes map 1's `helolw` is **still unknown**. Attempts made, so a later session does
not repeat them:

1. **Read `Mission_LoadObjectiveTriggers` (`0x418230`).** Confirms the layout —
   `ACTIVE` -> `mission+0x228+i*0x31c`, `INACTIVE` -> `+0x22c+i*0x31c`, resolved through
   `NodeRegistry_LookupByName`. When **neither** key is present it explicitly writes **0 to
   both**. So there is no fallback to the objective's own name. Negative.
2. **Searched the whole corpus for other code indexing by the `0x31c` stride.** Four hits
   (`0x4236b0`, `0x4248e0`, `0x425770`, `0x42be00`) are all vehicle code using `0x31c` as a
   plain instance offset — coincidental. Only `FUN_00418620` genuinely indexes objective
   records, and it reads (`+0x650+i*0x31c`), does not write the completed flag. Negative.
3. **Searched `interp.zbd` bytecode** (`02_extracted/interp/interp.json`, 122 entries,
   100 KB) for `objective` / `complete` / `helolw` / the other objective names —
   **zero occurrences**. The interp scripts do not touch objectives. Negative.
4. **Checked whether `helolw` is itself a scene node** (which would allow an implicit
   trigger). It appears in 2 extracted files, where the real trigger nodes `frcoff`,
   `nb_force`, `rocket_gull` appear in 3 — the extra one being the scene. So `helolw` is an
   objective name only, not a node. Negative.
5. **Read `FUN_00418c30`.** It returns the index of the **first incomplete** objective and
   writes it to `mission[+0x124]`; it completes nothing. Negative.

### Where it could still be
- Code **outside** the map-1 reachable set (the set is a >= bound, not exact — see LOG S1.0).
- A mission-exit or zone-entry path that marks remaining objectives complete.
- `helolw` may be **informational** and never complete — plausible, since map 1 is not a
  `FINAL_MISSION` and the 5-objective end condition cannot apply to its 4 objectives.

**Do not implement a guess.** Until this is resolved the honest remake behaviour is: three
objectives complete via the node test, the fourth does not, and that is recorded as a known
divergence rather than papered over with an invented trigger.

---

# ORACLE VERIFICATION — 2026-09-09, `VERIFIED-ORACLE`

The system the previous attempt reported as *"did not work at all"*, now verified from
binary, extracted data and live runtime simultaneously.

```
python 03_re/scripts/ttd_verify.py --trace Recoil07 --cmds \
 "sxi gp; !tt 50; bp 0x00418d40; g; r ecx; dd @ecx+0x12c L1; dd @ecx+0x53c L4; dd @ecx+0x858 L4"
```
```
ecx=004f0cc0                                          the mission object (a static global)
004f0dec  00000004                                    mission+0x12c = objective COUNT = 4
004f11fc  00000000 00000000 16805b80 00000000         record 0
004f1518  00000000 00000000 00000000 1682850c         record 1
```

| claim | check | |
|---|---|---|
| array base is `mission+0x53c` | `0x004f11fc − 0x53c = 0x004f0cc0` = `ecx` | ✓ |
| stride is `0x31c` | `0x004f1518 − 0x004f11fc = 0x31c` | ✓ **exact** |
| count at `+0x12c` | **4** — map 1 has exactly 4 objectives | ✓ |
| `rec+0x08` = `ACTIVE` node | record 0 has it, record 1 does not | ✓ |
| `rec+0x0c` = `INACTIVE` node | record 1 has it, record 0 does not | ✓ |

That pattern is exactly what `m1_objectives.json` declares: OBJECTIVE1 `obj1fgate` uses
**`ACTIVE`**, OBJECTIVE2 `comandlw` uses **`INACTIVE`**.

## The node names, read from live memory
```
da 0x16805b80  ->  "frcoff"
da 0x1682850c  ->  "nb_force"
```
`CONFIRMED-DATA` in `m1_objectives.json`: OBJECTIVE1's `ACTIVE` node is **`frcoff`**;
OBJECTIVE2's `INACTIVE` node is **`nb_force`**. Both match. The node's **name is at
`node+0x00`**, which was not previously established.

## The flag states explain why both are incomplete
```
dd 0x16805b80+0x24  ->  03080318      frcoff    bit 0x4 = 0  -> NOT active
dd 0x1682850c+0x24  ->  0108031c      nb_force  bit 0x4 = 1  -> active
```

- **OBJECTIVE1** completes when `frcoff` **becomes** active. It is not. Incomplete. ✓
- **OBJECTIVE2** completes when `nb_force` becomes **in**active. It is still active.
  Incomplete. ✓

Both `rec+0x00` completion flags read **0**, consistent with both tests. The mechanism, the
data binding, the flag bit and the resulting state all agree.

**This is the destructible→gate→objective chain confirmed end to end at runtime.**
