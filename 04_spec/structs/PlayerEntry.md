# Player / vehicle list entry (0x28 bytes) - field map

Consolidated 2026-09-24 (TODO item E1). This is the list cell called `node` in `vehicle.md`.
Entries live in the list at `[0x004f3a7c]`; the player's entry is `[0x004f3a88]`. They are
allocated by `Vehicle_SpawnInstanceFromConfig` `0x00421ab0` (`push 0x28`) and built by
`PlayerEntry_Construct` `0x004383e0`, whose bytes were **re-read for this file**.

## Constructor `0x004383e0` (bytes re-read, `CONFIRMED-BINARY`)
- Zeroes `+0x00, +0x08, +0x0c, +0x10, +0x14, +0x18, +0x1c, +0x20, +0x24`.
- `+0x04 = malloc(0x10c4)`, then **zero-filled** (`rep stosd`, 0x431 dwords). Every vehicle
  instance field therefore starts at 0 unless a later initialiser writes it; see
  [`VehicleInstance.md`](VehicleInstance.md).

## Fields
| offset | type | meaning | source |
|---|---|---|---|
| +0x00 | ptr | next entry | list walks (`player.md`) |
| +0x04 | ptr | vehicle instance block (0x10c4 bytes) | ctor |
| +0x08 | ptr | class entry; `[+8]+4` = class config `cfg`; cleared on destruct | `vehicle.md` 1, `PlayerEntry_Destruct` `0x00438430` |
| +0x0c | - | zeroed by ctor; use not established | ctor |
| +0x10..+0x1c | list | weapon-mount records (list at `+0x14`, count `+0x1c`), created per class mount (stride 0x50 from class `+0x58`, count class `+0x54`) | `PlayerEntry_Destruct`, `0x00421ab0` |
| +0x20 | - | zeroed by ctor; use not established | ctor |
| +0x24 | ptr | per-entry side object: `+0x308` cleared on destruct; `+0x10/+0x14` hold laps/time in network games | `PlayerEntry_Destruct`, `NetMsg_SendLapComplete` `0x004330f0` |

## Not established
- `+0x0c` and `+0x20`.
- The exact head/tail split of the mount list at `+0x10..+0x18`. Read `PlayerEntry_AddMount` `0x004384e0`
  before the port.
