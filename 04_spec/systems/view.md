# View and projection

Subsystem `view` (registry: `03_re/ledger/subsystem_registry.csv`). The state that turns world
space into screen space: the view matrix, the camera matrix, and the projection constants.
Evidence tags per item; addresses are `Recoil.exe`.

## Projection setup — `FUN_00474400` (107 calls `Recoil18` / 16 `Recoil21`)
Called once per frame from `0x00479ce0`. Takes eight parameters and derives the rest:

| global | value written | measured (`Recoil18` `C2F0:F8F`) |
|---|---|---|
| `0x00566430`, `0x00566434` | `p1`, `p2` | 0, 0 |
| `0x00566848`, `0x0056684c` | `p3`, `p4` | 320, 200 |
| `0x00566858`, `0x0056685c` | `p5`, `p6` | 1.376382, 2.414213 |
| `0x00566838`, `0x0056683c` | `1/p5`, `1/p6` | |
| `0x00566850`, `0x00566854` | `p5*p3`, `p6*p4` | 440.442, 482.843 |
| `0x00566860`, `0x00566864` | `1/(p5*p3)`, `1/(p6*p4)` | 0.00227045, 0.00207107 |
| `0x005761e0`, `0x005761e4` | `p3+p1`, `p4+p2` - the screen centre | 320, 200 |
| `0x00566840`, `0x00566844` | `p7`, `p8` | 1.0, 960.0 |

**Field of view - `VERIFIED-ORACLE` (the parameters themselves were read):**
`p5 = 1.376382 = cot 36.0000 deg` and `p6 = 2.414213 = cot 22.5000 deg`, so the projection is
**72 degrees horizontal by 45 degrees vertical**, over a 640 x 400 screen (half-size 320 x 200).
This supersedes the `INFERRED` FOV note in `transform_stack.md`, which was worked backwards from
the scale factors.

**`p7` and `p8` are the camera's near and far planes** (`CONFIRMED-BINARY`, 2026-09-15): the caller
`View_SetupFromCamera` `0x00479ce0` passes camera data `+0xb0` / `+0xb4` (near raised to at least 1.0),
with `p5`/`p6` = camera `+0x1d4`/`+0x1d8` (cotangents of the half FOV) and `p3`/`p4` = half the viewport.
The measured 1.0 / 960.0 are that camera's near/far (`camera.md`). This replaces the earlier `INFERRED` note.

## Camera and view matrices — `FUN_00473e60`
Measured end to end; full chain in `transform_stack.md`. From the camera object's world matrix
(copied to `obj+0x44` by `Camera.c`'s `0x0044abf0`):
- **B** (`0x00566920`) = that matrix with **every element of rows 1 and 2 negated**;
- **A** (`0x005668e8`) = inverse of B = the **view matrix**, loaded by transform's `0x00472f90`.

The row sign flip is a fixed axis-convention change; a remake must reproduce it.

## Screen to world — lives in `transform`
`FUN_00474c20` / `FUN_00474bc0` (mouse raycast) read the screen centre and the `1/(p*half)`
scales above; they are `transform` members because they use the matrix stack. Verified there.

## Members (provisional list; see the registry for status)
`0x00472ed0` (returns `0x00566850/54` in `EAX:EDX`; Ghidra typed it `void` and showed an empty
body), `0x00473e60`, `0x00473fc0`, `0x004743e0`, `0x00474400`, `0x00474b20`, `0x00474b70`,
`0x00474fc0`, `0x004753e0`.

---

# All members CONFIRMED - gate OPEN (2026-09-15)

| addr | operation | evidence |
|---|---|---|
| `0x00474400` | projection setup (table above) | ORACLE: all 12 outputs + centre, rel.err 2.8e-8 |
| `0x00473e60` | camera matrix B and view matrix A = inverse(B) | ORACLE, chain measured end to end |
| `0x00474b70` | world -> screen: `sx = x.(p5 p3)/z + cx`, `sy = y.(p6 p4)/z + cy`, `z' = p7/z` | ORACLE 1.7e-5; 55k calls |
| `0x00474b20` | the same with `z' = 1/z` | ORACLE 1.9e-5 |
| `0x00473fc0` | `x.B1 + y.B4 + z.B7 + B10` per point - the Y component of `p.B` | ORACLE 9.8e-7 |
| `0x004743e0` | stores two values at `0x00566918/1c` | ORACLE exact |
| `0x00472ed0` | returns `(p5 p3, p6 p4)` in `EAX:EDX` | bytes; never runs |
| `0x00474fc0` | fog/falloff lookup, table built once | bytes + file constants; never runs |
| `0x004753e0` | triangle plane + screen-gradient setup (`INFERRED` texture gradients) | body; never runs |

**The field of view appears twice, independently.** The projection parameters are
`cot 36.0000` / `cot 22.5000`, and `0x004743e0` stores **1.2566 / 0.7854** - 72 and 45 degrees in
radians. Two separate code paths agree on 72 x 45 degrees.

**Fog/falloff table** (`0x00474fc0`): gated by `0x004e0e8c`, file-initialised to 1 - so that word
means "table not built yet". Builds `table[i] = exp(-i / 51)` for `i = 0..255` at `0x00566438`
using x87 `f2xm1`/`fscale`, sets `0x005669d0 = 51.0`, clears the flag. Lookup: `x > 5.0 -> 0.0`,
`x < 0.0 -> 1.0`, else `table[trunc(x * 51)]`. Constants from the file: step
`0x004d29a0 = 0.019607844` (float 1/51), bound `0x004d29a4 = 5.0`. The quantisation to 1/51 is
part of the behaviour; a remake should use the table, not `exp()`.

## Function index additions (2026-09-24)
- **Rectangles.** `Rect_SetOrigin` `0x004083d0` and `Rect_SetSize` `0x00408400` keep an
  inclusive `right-1` / `bottom-1` pair at `[6]/[7]`.
- **`Viewport_SetCamera` `0x00408480`:** `fov_x = vp.w * fov_y / vp.h`. The horizontal field of
  view follows the viewport aspect; this matters for the v2 widescreen work.
- **Attaching views.** `Viewport0_AttachView` `0x00408570` (a window class) and
  `Viewport1_AttachView` `0x004085b0` (a display class) apply size and origin.

