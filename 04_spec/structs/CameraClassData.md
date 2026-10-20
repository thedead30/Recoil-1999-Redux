# Camera class data (`node[+0x38]`, class type 1) - field map

Consolidated 2026-09-24 (TODO item E1) from `systems/camera.md`. That file keeps the evidence:
the `Recoil18` dump (`MEASURED`) and the setter bodies (`CONFIRMED-BINARY`). This page is the
single table.

- **Node header:** `node[+0x34]` = class type (1 = camera), `node[+0x38]` = class data.
- **Validation order:** a null node returns 5, a wrong type returns 3, null class data
  returns 5.

| offset | type | meaning | evidence |
|---|---|---|---|
| +0x10 | int | flags; bit 0x2 selects the position source for `Camera_GetWorldPosition` | getter body |
| +0x14..+0x1c | float x3 | local position | MEASURED (= `TranslateLocal` args) |
| +0x20 / +0x24 / +0x28 | float | local pitch / yaw / roll | MEASURED (= Euler extractor) |
| +0x2c..+0x34 | float x3 | world position | MEASURED (= matrix B translation) |
| +0x44..+0x70 | float x12 | world matrix (rows 1-2 are the sign-flipped rows of B) | MEASURED, `view.md` |
| +0x74..+0x7c | float x3 | negated forward axis `-(row 2)` | MEASURED |
| +0xb0 / +0xb4 | float | **near / far** (1.0 / 960.0 in the dump) | MEASURED; set by `0x0044a2f0`, read by `0x0044a380` |
| +0xd0 / +0xd4 | float | LOD multiplier `x` / `1/x^2` | `0x0044a870` / `0x0044a7f0` |
| +0xd8 / +0xdc | float | aspect scale factors | `Camera_SetAspectRatio` `0x0044a410` |
| +0xe0 / +0xe4 | float | **FOV horizontal / vertical, radians** (72 and 45 degrees in the dump) | `Camera_SetFOV` `0x0044a610`, MEASURED |
| +0xe8 / +0xec | float | FOV / aspect, **each clamped to 1.396263 rad (80 degrees)** | `0x0044a410` |
| +0xf0 / +0xf4 | float | half-angles `0.5 * (+0xe8 / +0xec)` | setters, MEASURED |
| +0xf8 | int | dirty flag (projection) | setters |
| +0x108..+0x134 | float x12 | far-plane frustum corners, camera space | MEASURED |
| +0x138 | int | dirty flag (frustum) | setters |
| +0x1d4 / +0x1d8 | float | `1 / tan(half-angle)` | `Camera_SetFOV` |
| +0x1e0 / +0x1e4 | int / 4 bytes | colour record at +0x1e4; +0x1e0 set unless the record contains 0xff | `0x0044d260` |

## Not established
Offsets not listed here were not cross-checked. `camera.md` lists only the rows it could
cross-check, and this page keeps that rule.
