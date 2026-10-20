# Camera

Subsystem `camera` (28 members, membership CLOSED): the camera API in `Camera.c`. The scene-graph
render traversal from the same file is the separate subsystem `scene_render`.

## Node and class data
A camera is a scene-graph node of **class type 1** (`node[+0x34] == 1`) whose class data is at
`node[+0x38]`. Every API function validates in the same order and returns an error code:

| check | failure return |
|---|---|
| node pointer is null | 5 (report "Null node pointer") |
| `node[+0x34] != 1` | 3 (report "Bad Class Found, Wanted 1") |
| `node[+0x38]` is null | 5 (report "Null class data pointer") |
| otherwise | 0 |

The reports go to `0x00404e80`, which is a no-op in this build (`error_reporting.md`).

## Class-data layout — MEASURED
Dumped in `Recoil18` at `C2F0:A5A` (node `0x0a26e1a8`, class data `0x08726b08`). Each row is
cross-checked against a value measured independently elsewhere; offsets without a cross-check are
not listed.

| offset | measured | meaning | cross-check |
|---|---|---|---|
| `+0x10` | 1 | flags; bit `0x2` selects the position source for `Camera_GetWorldPosition` | read from the getter's body |
| `+0x14..+0x1c` | 2190.50, 5.149, 1471.02 | local position | equals the arguments `TranslateLocal` received for this camera |
| `+0x20, +0x24` | -0.07988, -3.07182 | local pitch, yaw (roll at `+0x28` = 0) | equals the Euler extractor's output |
| `+0x2c..+0x34` | 2190.88, 5.151, 1471.13 | world position | equals the translation of camera matrix B |
| `+0x44..+0x70` | 12 floats | world matrix | rows 1-2 are the sign-flipped rows of B (see `view.md`) |
| `+0x74..+0x7c` | 0.0407, -0.0798, 0.99597 | negated forward axis, `-(row 2)` | computed from the world matrix |
| `+0xb0, +0xb4` | 1.0, 960.0 | **near, far** | equal the projection setup's `p7`, `p8` |
| `+0xd8, +0xdc` | 1, 1 | aspect scale factors | |
| `+0xe0, +0xe4` | 1.25664, 0.785398 | **field of view, horizontal / vertical, radians** (72, 45 degrees) | equal the FOV measured twice in `view.md` |
| `+0xe8, +0xec` | 1.25664, 0.785398 | FOV / aspect factor | `+0xe0 / +0xd8`, `+0xe4 / +0xdc` |
| `+0xf0, +0xf4` | 0.628319, 0.392699 | half-angles | `0.5 * (+0xe8)`, `0.5 * (+0xec)` |
| `+0x108..+0x134` | (+-697.48, +-397.65, -960) x 4 | far-plane frustum corners, camera space | `960 tan 36 = 697.48`, `960 tan 22.5 = 397.65` |

The setters raise dirty flags at `+0xf8` and `+0x138`; both read 0 here (already processed).

## Behaviour read from the setter bodies
- **`Camera_SetFOV(h, v)`** (`0x0044a610`): stores `h, v` at `+0xe0/+0xe4`, derives `+0xe8/+0xec`
  (divided by aspect), the half-angles `+0xf0/+0xf4`, and `1 / tan(half)` at `+0x1d4/+0x1d8`;
  sets both dirty flags.
- **`Camera_SetAspectRatio(a, b)`** (`0x0044a410`): stores `a, b` at `+0xd8/+0xdc`, recomputes
  `+0xe8 = +0xe0 / a`, `+0xec = +0xe4 / b`, **clamping each to `0x3fb2b021` = 1.396263 rad = 80
  degrees**, then the same half-angle and cotangent fields; sets both dirty flags.
- `0x0044a2f0` stores near/far at `+0xb0/+0xb4` and sets `+0xf8`; `0x0044a380` returns them.

`Camera_SetFOV` and `Camera_SetAspectRatio` never run in `Recoil18` or `Recoil21`; the camera's
values were set before the traces attached. Their code is read from the body; the *fields* they
write are confirmed by the dump above.

---

# All 28 API functions CONFIRMED - gate status in `re_status.py`

| addr | operation |
|---|---|
| `0x00449be0` | constructor: allocate node, class 1, class data `calloc(1, 0x1e8)`, defaults, register in list 8 |
| `0x00449c90` / `0x00449cd0` | attach / detach a child node (validate, tail-jump to `Class.c`) |
| `0x00449d20` | set or clear flags bit `0x1` |
| `0x00449da0` / `db0` / `dc0` | store `ECX` in globals `0x004ddd38` / `0x004ddd34` / `0x004ddd10` |
| `0x00449dd0` / `0x00449e80` | link a class-2 node at `data[0]` / return it |
| `0x00449e90` | `data[+4] := EDX` |
| `0x00449ea0` / `0x00449fb0` / `0x0044a060` | set / add to / get local angles `+0x20..+0x28` |
| `0x0044a0f0` / `0x0044a1a0` / `0x0044a250` | set / add to / get position (`+0x14` or `+0xa4` by flags bit `0x2`) |
| `0x0044a2f0` / `0x0044a380` | set / get near, far |
| `0x0044a410` / `0x0044a580` | set / get aspect (setter clamps to 80 degrees) |
| `0x0044a610` / `0x0044a760` | set / get field of view |
| `0x0044a870` / `0x0044a7f0` | set `+0xd0 = x, +0xd4 = 1/x^2` / get `+0xd0` (dump: 0.5 and 4) |
| `0x0044a910` / `0x0044a980` | `data[+8]` / `data[+0xc]` := `EDX` |
| `0x0044d260` | store a 4-byte record at `+0x1e4` and set `+0x1e0` unless it contains `0xff` |
| `0x00449f50` | propagate a change: mark dirty, register in list 7, dirty-mark children |

**Naming caution:** `Camera_GetWorldPosition` returns the **local** position field (`+0x14`), not the
world position at `+0x2c`. The name predates this analysis; the behaviour is what is recorded.

Verified against the trace: the link getter, angle setter, position setter and getter, near/far
getter. The rest never run in either trace and are confirmed from their bodies, with hidden
register arguments resolved from the bytes.

# The node machinery the camera depends on (Class.c)
- **Allocator `0x004478c0`**: pool of **0xc4-byte nodes** based at `[0x00539c94]` (`0x0a26e020` in
  `Recoil18`); free list threaded through the low 24 bits of `node+0xc0`; a new node gets
  `+0x24 = 0x0108001c`, `+0x44 = 1`, `+0x30 = 0xff`, `+0x4c..+0x53 = 0xff`, the name
  `"Default_node_name"`. `INFERRED`: node index 0 (`0x0a26e020`) is the world root - it is what the
  camera links to.
- **Registry `0x0044ed90`**: doubly linked lists selected by index; a link is
  `[0] node, [1] next, [2] prev, [3] 0`. Lists seen: **6** (every allocated node), **7** (nodes
  needing an update - `node+0x24` bit 0 means "already in list 7"; ancestors are pushed at the head),
  **8** (cameras).
- **Attach / detach** keep both directions: children at `+0x5c/+0x60`, parents at `+0x54/+0x58`.

## Camera follow nodes (from `Camera_UpdateFollowNodes` `0x0044d320`, CONFIRMED)
Camera data `+8` and `+0xc` hold two nodes (set by `0x0044a910` / `0x0044a980`). Each frame, the
first is moved to the camera's world position, and the second follows the camera in **X and Z
only**, keeping its own height. `INFERRED`: a sky dome and a horizon or ground plane.

## Function index (added 2026-09-24)
- **`Camera_UpdateForVehicle` `0x00404e90`:**
  - when `[0x004e5cd8]` is set, refreshes the distance and look direction;
  - dispatches on the view state `+0x58c`: 1 `0x00405040`, 2 `Camera_Mode2_Cockpit` `0x004057d0`,
    3 `0x004059a0`, 4 `Vehicle_ResetChaseCamera`, 6 `Camera_Mode6_LookAtVehicle` `0x00405870`,
    7 `Camera_Mode7_FollowAnimObject` `0x004063f0`.
- **Follow camera** `Camera_Mode1_Follow` `0x00405040`: while the vehicle is skidding (`+0x20`), the
  direction smoother uses base rate 2.0 instead of 3.0 (`[0x004da3a0]` vs `[0x004da39c]`,
  disassembly re-read 2026-09-24), so the camera lags more in a slide.
- **Cockpit** (`0x004057d0`): position = vehicle position + `+0x538`, rotation (-1.55, 0, 0),
  direction (0, -1, 0).
- **Collision:**
  - `Camera_ResolveCollision` `0x00406110`: probes from the camera back along the direction by
    `[0x004da3a4]` (1.0 initially) and snaps to the nearest hit;
  - `Camera_SideProbePushout` `0x00405ee0`: side probes of ±2 × right, plus a vertical probe to
    +2.0 in SUB mode;
  - `Camera_ClampAboveGround` `0x00406470` (ray from 500.0);
  - `CollisionHits_FilterForCamera` `0x00406730` drops ignored or transparent nodes;
  - `CollisionHits_Nearest` `0x0042b6e0`.
- **Zones and water:**
  - `Camera_MergeZoneSet` `0x00406510` merges the hit zone ids with the player's, at most 3,
    with `0xff` as a wildcard;
  - `Camera_SubModeWaterCheck` `0x004067a0` (y -= 0.2 on a hit);
  - `Camera_UpdateOverlay4f3138` `0x00406610` (underwater overlay: ray upward +50.0).
- **View vehicle.** `Camera_SetViewVehicleToPlayer` `0x00406450` and
  `Camera_ReleaseViewVehicleIfSinglePlayer` `0x00406430`.
- **Camera API.** `FUN_00449db0` / `FUN_00449dc0` `0x00449db0/0x00449dc0`, used by
  `App_EnterGameplay` for camera flags; their bodies are in the ledger.

