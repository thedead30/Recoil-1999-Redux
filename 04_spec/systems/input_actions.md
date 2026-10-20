# Input action IDs — CONFIRMED

Recovered by setting `Input_QueryAction`'s prototype in Ghidra
(`uint __fastcall Input_QueryAction(int actionId)` @ `0x004717d0`) and re-decompiling
`Vehicle_ReadPlayerControlInput` (`0x00425a20`). Before the prototype was set, Ghidra
rendered every call as `Input_QueryAction(void)` and the IDs were invisible — which is why
the previous attempt invented key bindings instead of reading them.

## Action IDs used by the player-control path

| ID | dec | effect | writes | tag |
|---|---|---|---|---|
| `0x01` | 1 | throttle **reverse** | inst +0x68 = −1.0 | `CONFIRMED-BINARY` |
| `0x02` | 2 | steer **one way** | inst +0x6c = +1.0 | `CONFIRMED-BINARY` |
| `0x03` | 3 | steer **other way** | inst +0x6c = −1.0 | `CONFIRMED-BINARY` |
| `0x04` | 4 | throttle **forward** | inst +0x68 = +1.0 | `CONFIRMED-BINARY` |
| `0x05` | 5 | third axis + | inst +0x70 = +1.0 | `CONFIRMED-BINARY` |
| `0x06` | 6 | third axis − | inst +0x70 = −1.0 | `CONFIRMED-BINARY` |
| `0x07` | 7 | **view reset** | `Vehicle_ResetLookOffset()` | `CONFIRMED-BINARY` |
| `0x08` | 8 | **aim-camera snap** | snapshots aim point, `Vehicle_SetCameraViewState()` | `CONFIRMED-BINARY` |
| `0x0B` | 11 | **secondary fire** | inst +0x5a8, via mount +0x5e8 | `CONFIRMED-BINARY` |
| `0x0C` | 12 | **primary fire** | inst +0x5a4, via mount +0x5e4 | `CONFIRMED-BINARY` |
| `0x0D` | 13 | latched action (also sets a netcode global) | inst +0x5b8 | `CONFIRMED-BINARY` |
| `0x25` | 37 | request **TRACK** mode (from HOVER) | `Vehicle_TransitionToTrackMode()` | `CONFIRMED-BINARY` |
| `0x26` | 38 | request **AMPHIB** mode (from HOVER) | `Vehicle_TransitionToAmphibMode(0)` | `CONFIRMED-BINARY` |
| `0x27` | 39 | request **HOVER** mode (from TRACK or AMPHIB) | `Vehicle_TransitionToHoverMode()` | `CONFIRMED-BINARY` |
| `0x28` | 40 | request **SUB** mode (from AMPHIB) | `Vehicle_TransitionToSubMode()` | `CONFIRMED-BINARY` |

## Return value convention
`Input_QueryAction` returns a bitmask. Two tests appear:
- `(result & 3) != 0` — held/active (used for continuous actions: throttle, steer, fire, mode)
- `result == 1` — a clean edge (used for one-shot actions: view reset, aim snap, `0x0D`)
- `(result & 1) != 0` — an additional sub-test inside the primary-fire alternate path

So bit 0 and bit 1 carry distinct meanings — most likely *just-pressed* and *held*. Which
is which is **not yet established**; `Input_QueryAction` itself (`0x004717d0`) has not been
read. Marked `INFERRED` until it is.

## Relationship to the prior record — R9 upgrade
The old log recorded exactly this table from an earlier pass:
> `0x4/0x1`→throttle `+0x68`, `0x2/0x3`→steer `+0x6c`, `0x5/0x6`→axis `+0x70`,
> `0x7`→view reset, `0x8`→aim-snap, `0xB`→`+0x5a8` secondary, `0xC`→`+0x5a4` primary,
> `0xD`→`+0x5b8`, `0x25/0x26/0x27/0x28`→track/amphib/hover/sub

**It matches in every entry.** This is a case where the prior record was correct and is now
independently re-derived, so it moves PRIOR-EVIDENCE → `CONFIRMED-BINARY`.

## Still missing — the physical key map
These are *action* IDs, not keys. What binds a scancode to an action is elsewhere:
`Input_QueryAction` (`0x004717d0`) and its state table. Weapon-select actions
(the old record put them at `0x0E`–`0x17`) do **not** appear in this function at all,
consistent with the old finding that selection is dispatched from `FUN_00439260` instead.

**Do not invent key bindings.** The action layer is now solid; the key layer is the next
target, and until it is read the binding is genuinely unknown.
