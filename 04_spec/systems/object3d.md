# Object node (class 5)

Subsystem `object3d` (registry: membership **CLOSED**). `zClass/Object3d.c`,
`0x0044d990..0x0044e5b0`, 20 functions.

## Class data layout (from confirmed code)
| offset | meaning | evidence |
|---|---|---|
| `+0x00` | flags: bit 0 local matrix dirty, bit 3 (8), bit 4 (`0x10`) skip rebuild, bit 5 (`0x20`) world cache stale | `gwNodeUpdate`, `0x00449480`, `0x0044d990` |
| `+0x18..+0x20` | angles | `gwNodeUpdate` TRS input |
| `+0x24..+0x2c` | scale | TRS input |
| `+0x30..+0x5c` | local matrix (12 floats) | TRS output |
| `+0x54..+0x5c` | position (read into the TRS as translation) | `gwNodeUpdate` |
| `+0x60..+0x8c` | cached world matrix (translation at `+0x84`) | `0x00449480`, `0x004497b0` |

## Confirmed
- **`0x0044d9e0` reset** (bytes): angles 0, scale 1, local matrix identity; then dirty-marked and
  queued on list 7.
- **Create `0x0044daa0`**: a pool node of class 5 with 0x90 bytes of data, reset. Returns 0 if the
  reset fails.
- **Attach / detach child `0x0044db10` / `0x0044db60`**: validate, then hand over to Class.c's generic
  attach (`0x004484d0`) and detach (`0x00448660`).
- `0x0044dbb0` sets or clears data bit 4. Like the camera setters, it refuses any other class (3).
- `0x0044dc30` sets a unit-range value at `+0x14` and an optional triple at `+8..+0x10`, each clamped
  to [0, 1]. `INFERRED`: opacity and colour.

## Local TRS setters
Every setter follows the same sequence:
1. store the value(s);
2. clear data bit `0x10`;
3. clear bit 8 unless the object is still untransformed by this component (scale 1.0, angles 0.0);
4. set bit 1 (local matrix dirty);
5. dirty-mark the subtree, which sets bit `0x20` (world cache stale);
6. queue the node on list 7, where `gwNodeUpdate` rebuilds the matrix.

| function | stores | evidence |
|---|---|---|
| `SetScale` `0x0044df00` | `+0x24..+0x2c` | `VERIFIED-ORACLE`, 6/6 incl. non-uniform scale |
| `SetRotation` `0x0044e030` | `+0x18..+0x20` (angles) | `VERIFIED-ORACLE`, 6/6 |
| `AddRotation` `0x0044e170` | angles += args | bytes |
| `GetScale` / `GetRotation` | read them back | bytes |

**None of these check the class**, unlike `0x0044dbb0` / `0x0044dc30` / `0x0044dd90..0x0044de80`.
| `SetPosition` `0x0044e300` | `+0x54..+0x5c`; does **not** clear bit `0x10` | `VERIFIED-ORACLE` (whole-block diff: only the position changes) |
| `AddPosition` `0x0044e3d0` | position += args; checks the class | bytes |
| `SetLocalMatrix` `0x0044e4f0` | copies 12 floats to `+0x30`; sets bits `0x10` and 1, clears 8 | bytes |

**Bit `0x10` = "local matrix set directly"**: `SetLocalMatrix` sets it, and `gwNodeUpdate` then
does not rebuild the matrix from TRS. `SetScale` / `SetRotation` clear it (back to TRS mode);
`SetPosition` leaves it alone. `GetPosition` `0x0044e270` and `GetLocalMatrix` `0x0044e5b0` (the
hottest function in the file, 3218 calls) read back.

## Function index additions (2026-09-24)
- **Getters.** `Object3D_GetValue4` `0x0044de10` (class 5 required) and `Object3D_GetScale`
  `0x0044dfd0` (`+0x24..+0x2c`).

