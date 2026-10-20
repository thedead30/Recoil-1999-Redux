# Movement type enum — CONFIRMED

The vehicle class config's field `+0xa4` selects the movement model. Values recovered
from the debug-HUD switch in `FUN_0042aa50` cross-referenced against the literal string
table at `0x004dca40`.

| value | name | tag |
|---|---|---|
| 0 | `BASIC`  | `CONFIRMED-BINARY` 0x004dca68 |
| 1 | `FLY`    | `CONFIRMED-BINARY` 0x004dca48 |
| 2 | `SUB`    | `CONFIRMED-BINARY` 0x004dca4c |
| 3 | `TRACK`  | `CONFIRMED-BINARY` 0x004dca60 |
| 4 | `HOVER`  | `CONFIRMED-BINARY` 0x004dca58 |
| 5 | `AMPHIB` | `CONFIRMED-BINARY` 0x004dca50 |
| other | `UNKNOWN` | `CONFIRMED-BINARY` 0x004dca40 |

Raw bytes at `0x004dca40` (48 read):
```
55 4E 4B 4E 4F 57 4E 00  46 4C 59 00 53 55 42 00   UNKNOWN. FLY.SUB.
41 4D 50 48 49 42 00 00  48 4F 56 45 52 00 00 00   AMPHIB.. HOVER...
54 52 41 43 4B 00 00 00  42 41 53 49 43 00 00 00   TRACK... BASIC...
```

## Why this one mattered

The previous attempt built the player tank on movement **Type 0**. The player vehicle
class `bft` is `"track"`, which this confirms is **Type 3**. Every drive test in that
attempt therefore ran the wrong integrator. The old record had stated this correctly and
was not followed — see `99_attic/old_docs/recoil_confirmed_logic.md`.

This entry **upgrades that claim from PRIOR-EVIDENCE to CONFIRMED-BINARY** (R9): it is now
read directly from the binary's own string table and switch, not inherited from the log.

## Separate field — do not confuse

The vehicle **instance** field `+0x60` is runtime *state*, not the movement type. From the
same function's debug formatting:

| +0x60 | meaning | evidence |
|---|---|---|
| 0, 1 | alive, running dynamics (`"%s using %s dynamics"`) | `CONFIRMED-BINARY` FUN_0042aa50 |
| 2 | AI-driven — formats a goal node (`"%s is in mode %d and had goal node..."`) | `CONFIRMED-BINARY` |
| 4 | **DEAD** (`"%s is DEAD!"`) | `CONFIRMED-BINARY` |
| 3, 5, 6 | distinct branches in `Vehicle_MainUpdateTick`, meaning not yet recovered | open |
