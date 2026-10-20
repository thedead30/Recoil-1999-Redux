# Vehicle - movement, contact, mode changes, health

Narrative spec for subsystem `vehicle` (membership CLOSED, all members CONFIRMED). It ties the
per-function ledger notes into one model. Every function listed has a full entry in
[`../reference/vehicle.md`](../reference/vehicle.md) (generated from the ledger).

Deeper single-function records already exist and are linked, not repeated:
[`0x00426390` main tick](../../03_re/decomp/0x00426390_Vehicle_MainUpdateTick.md),
[`0x00426770` TRACK mover](../../03_re/decomp/0x00426770_Vehicle_MoveType3_Track.md),
[`0x00429750` steer](../../03_re/decomp/0x00429750_Vehicle_ApplySteer.md),
[`0x00429870` throttle](../../03_re/decomp/0x00429870_Vehicle_ApplyThrottle.md),
[`0x0042bf90` track contact](../../03_re/decomp/0x0042bf90_Vehicle_UpdateTrackModeContactAndBuoyancy.md),
[`movement_types.md`](movement_types.md), [`velocity_frames.md`](velocity_frames.md),
[`expapprox.md`](expapprox.md), [`frame_timing.md`](frame_timing.md).

**Evidence level.** Formulas below are `CONFIRMED-BINARY` (read from bytes) unless tagged
otherwise. Only the steer and throttle integrators, the timestep and the body/world velocity
round trip are `VERIFIED-ORACLE` against a running game (traces `Recoil16`-`Recoil22`). The
contact solver, the hover/amphib/sub movers, the skid model and damage are **static reads
only**: nobody has yet compared them to the running game.

## 1. Objects and notation

A vehicle is reached through a list cell `node`:

| name | expression | meaning |
|---|---|---|
| `node` | list cell, `ECX` on most calls | `[+0] next`, `[+4] inst`, `[+8] class entry` |
| `inst` | `[node+4]` | per-vehicle runtime state (offsets below are on `inst` unless marked) |
| `cfg` | `[[node+8]+4]` | per-class config (tuning values from the class file) |
| `dt` | `[0x004f3ac4]` | clamped timestep, in [0.005, 0.125] s ([frame_timing](frame_timing.md)) |
| `invDt` | `[0x004f3aac]` | `1/dt` |
| `now` | `[0x0056b428]` | game clock (seconds) |
| player | `[0x004f3a88]` | the player's `node` |

### Instance fields used across the subsystem
| offset | meaning | source |
|---|---|---|
| +0x08 | previous movement type (written on every mode change) | mode transitions |
| +0x0c | gravity magnitude (per instance) | `0x0042bf90` |
| +0x10 | **airborne** flag (1 = no ground contact) | `0x0042c0d0` sets 1 on 0 contacts, 0 otherwise |
| +0x18 | controls disabled (set while effect slot `+0x5c4` < -0.5; see section 8) | dispatch, health tick |
| +0x20 | **skid** flag | `Vehicle_StartSkid` / `Vehicle_EndSkid` |
| +0x24 / +0x28 / +0x2c | amphib / hover / sub mode available | mode transitions |
| +0x60 | runtime state: 0,1 alive; 2 AI; 3 special tick; 4 dead/inert; 5 player dead | [movement_types](movement_types.md) |
| +0x64 | mode-change cooldown expiry (time) | mode transitions |
| +0x68..+0x74 | control inputs (throttle, steer, vertical, pitch gates) | dispatch zeroes them when +0x18 |
| +0x7c / +0x80 / +0x84 / +0x88 | signed demands: throttle / steer / vertical / pitch | integrators |
| +0x90 / +0x94 / +0x98 | pitch / yaw / roll **rate** | `0x0042bf90` |
| +0xa4..+0xac | **world** velocity | [velocity_frames](velocity_frames.md) |
| +0xb0..+0xb8 | **body** velocity: lateral, vertical, forward axis | [velocity_frames](velocity_frames.md) |
| +0xc8 / +0xcc | speed cap / yaw-rate cap (cached from `cfg+0xac` / `cfg+0xb4`) | throttle, steer |
| +0x260..+0x280 | orientation matrix, rows at +0x260, +0x26c, +0x278 | movers |
| +0x318 | previous-frame world Y | `Vehicle_TrackVerticalVelocity` |
| +0x380 / +0x38c / +0x398 | horizontal forward / forward / up vectors | `Vehicle_ExtractAxesFromMatrix` |
| +0x3a4 | previous horizontal forward | movers (end of tick) |
| +0x3bc / +0x3c0 / +0x3c4 | pitch / yaw / roll angles | movers |
| +0x3ec..+0x3f4 | world position | movers |
| +0x48..+0x5c | damage-over-time state | section 8 |
| +0xf34 | hit points | `Vehicle_AdjustHealth` |

`Vehicle_ExtractAxesFromMatrix` `0x004294d0`: forward `+0x38c = -(row2)`, up `+0x398 = row1`,
horizontal forward `+0x380` = forward with y = 0, normalised.

## 2. The per-frame pipeline

`Vehicle_MainUpdateTick` `0x00426390` (registered as a function pointer; no static caller):

1. `dt = max(frameDt, 0.005)`; caches `invDt` and `dt*0.01` (velocity dead-zone epsilon).
2. For every cell of the vehicle list `[0x004f3a7c]`:
   - player cell `[0x004f36a4]` → `Vehicle_ReadPlayerControlInput`; mode-2 cells → AI tick.
   - `Vehicle_DispatchMovementPhysics` `0x004266b0` (section 3).
   - `Vehicle_PostPhysicsTick` `0x0041b950`:
     - updates bounds `+0x410 = pos + +0x404`;
     - unless `+0x594` is set: blends rotation (`0x00472960(0.65)`); if state != 5, runs
       `0x0043a600`, `Vehicle_UpdateFireRequests`, `Weapon_UpdateDeployAnimState`,
       `Vehicle_HealthTick`, then sets `+0x594 = 1` when `Vehicle_CheckDeath` fires;
     - when `+0x594` is set it copies stored pose `+0x3f8` / `+0x3c8` instead;
     - finally pushes position and rotation to the scene object.
   - Engine sound (`Vehicle_UpdateEngineSound` `0x004386c0`) when the sound API is enabled.
3. `Vehicle_DrawDebugInfo(countOfMode2)`.

`Vehicle_UpdateFireRequests` `0x0041bab0`:
- Primary `+0x5a4`: set while fire is held, for continuous weapons (weapon flag bit 1).
- Secondary `+0x5a8`: pulses to 1 when held, the cooldown `slot+0x40 <= now` has expired and
  no weapon swap is in progress (`+0x5e0 & 0x180`). It then re-arms the cooldown to
  `now + slot+0x44`.

## 3. Movement dispatch

`Vehicle_DispatchMovementPhysics` `0x004266b0`:
- Nothing when state `+0x60 == 4`.
- `+0x18 != 0` zeroes the inputs `+0x68..+0x74`.
- Then it switches on `cfg+0xa4`:

| cfg+0xa4 | model | handler |
|---|---|---|
| 0 | BASIC | `Vehicle_UpdateBasicModePhysics` `0x00428120` |
| 1 | FLY | **no case** - no movement at all |
| 2 | SUB | `Vehicle_UpdateSubModePhysics` `0x00428520` |
| 3 | TRACK (player tank `bft`) | `0x00426770` |
| 4 | HOVER | `Vehicle_UpdateHoverModePhysics` `0x00427140` |
| 5 | AMPHIB | `Vehicle_UpdateAmphibModePhysics` `0x004279f0` |

For the player it then runs `0x00458af0` and, if available, `ForceFeedback_UpdateEffects`.

All movers except BASIC follow one skeleton:
1. auto-turn;
2. steer;
3. `yaw += yawRate*dt`, wrapped to (−2π, 2π];
4. rebuild the matrix;
5. extract axes;
6. `+0xc8 = cfg+0xac`;
7. throttle;
8. `world = body * orientation`;
9. `pos += world*dt`;
10. a model-specific ground/contact step;
11. push pose to the scene object and store previous forward and rotation.

## 4. Shared integrators

### 4.1 Steering `0x00429750`, throttle `0x00429870` - `VERIFIED-ORACLE`
The full formulas and verification are in the two decomp records. Two points matter for the
remake:
- The throttle cap is **proportional to |demand|** (half stick gives half top speed).
- A steering reversal **snaps the yaw rate to exactly 0**.

With player `bft` values: acceleration `cfg+0xa8 = 48.0`, cap `65.0`, yaw acceleration
`cfg+0xb0 = 2.5`, yaw cap `3.5`, turn damping `cfg+0xb8 = 12.0`. Each value is
`CONFIRMED-DATA` + `VERIFIED-ORACLE`.

### 4.2 Skid (lateral slip) model - `CONFIRMED-BINARY`, not yet oracle-checked
The throttle integrator starts with a branch: if skid flag `+0x20 != 0` it calls
`Vehicle_IntegrateSkid` `0x00429d30` **instead of** its normal path.

`Vehicle_ComputeLateralSlipAccel` `0x00429b40` returns a lateral acceleration:

```
if dt < 1e-7: return 0
a = -28.0 * inst[+0x264]                                   // sideways component of row0.y
    - v.z * (fwd.x*prevFwd.z - fwd.z*prevFwd.x) * invDt     // turning (heading change per dt)
if lat(+0xb0) == 0:                                        // gripping
    if |a| <= cfg+0xcc: return 0                           // grip holds
    StartSkid(); return a - sign(a)*cfg+0xcc
r = a - Sign(lat) * cfg+0xd0                               // already sliding
if throttle demand +0x7c != 0 and Sign(+0x80) == Sign(yaw +0x3c0) and sign(r) != sign(lat):
    r = 0
if not skidding and |a| > cfg+0xcc: StartSkid()
return r
```
Here `fwd` is `+0x380` and `prevFwd` is `+0x3a4`. `cfg+0xcc` is class key `friction[0]`,
used as the grip threshold. `cfg+0xd0` is `friction[1]`, used as the sliding term; it is also
`stopping[0]`, and load order decides which value survives
([`VehicleClassConfig.md`](../structs/VehicleClassConfig.md)).

`Vehicle_IntegrateSkid` `0x00429d30`:
```
body = M * world                          // body_i = row_i . world, rows at +0x260/+0x26c/+0x278
v.z -= cfg+0xa8 * demand(+0x7c) * dt
v.z  = clamp(v.z, -inst+0xc8, +inst+0xc8)   // FLAT cap while skidding (normal path is |demand|-proportional)
lat' = lat + ComputeLateralSlipAccel()*dt
if lat != 0 and sign(lat') != sign(lat): lat' = 0; EndSkid()
lat  = clamp(lat', -inst+0xc8, +inst+0xc8)
```
- `Vehicle_StartSkid` `0x00429ed0`: `+0x20 = 1` and plays sound slot 3 at 1.0.
- `Vehicle_EndSkid` `0x00429ef0`: `+0x20 = 0` and stops slot 3.

The TRACK and HOVER movers also call `EndSkid`.

**Correction (2026-09-24).** These four functions were previously named `ComputeYawAccel`,
`IntegrateYawAndSpeed`, `SetSkid` and `ClearAirborne`, and `+0x20` was read as "airborne".
Two facts contradict that reading:
- `+0xb0` is body lateral velocity (`VERIFIED-ORACLE`, [velocity_frames](velocity_frames.md)).
- `+0x20` is set together with sound slot 3 and cleared together with its stop.

The airborne flag is `+0x10`.

### 4.3 Auto-turn `Vehicle_AutoTurnToHeading` `0x00429560`
Active while `+0x1018` is set, e.g. after `Vehicle_ComputeAimYawToPoint` stores a target
direction in `+0x1000/+0x1008`. Any steer input cancels it.

When `dot(fwd, target) >= cos(dt*cfg+0xb4)`, the heading snaps: `yaw = atan2(-tx, -tz)` and the
flag and turn state clear. Otherwise it forces steer to `sign(cross)` with yaw rate
`sign * cfg+0xb4`.

## 5. TRACK (type 3) contact and suspension

The mover is described in the `0x00426770` record. Its ground step is
`Vehicle_UpdateTrackModeContactAndBuoyancy` `0x0042bf90`:

```
[0x004f3bc8] = contact point count (EDX)
pitch += pitchRate*dt; roll += rollRate*dt; rebuild matrix; extract axes
vy -= gravity(+0xc)*dt;  y += vy*dt                           // semi-implicit Euler
Vehicle_TrackContactsGather()                                  // 0x0042cf90
if Vehicle_UpdateWaterBuoyancyAndAutoTransition():            // 0x0042d5c0, see 5.6
    Vehicle_TrackContactDispatch()                             // 0x0042c0d0
    Vehicle_AlignToUpNormal()                                  // 0x0042da40
    if player and not airborne: Vehicle_ThirdContactSnap()     // 0x0042d320
    Vehicle_TrackVerticalVelocity()                            // 0x0042c2e0
```

### 5.1 Contact tables (globals)
| address | meaning |
|---|---|
| `0x004f3bc8` | contact point count for this vehicle |
| `0x004f3c20` | contact points in world space, 3 floats each |
| `0x004f3bd0` | per-point active flag |
| `0x004f3bf8` | list of active point indices |
| `0x004f3c8c` | **number of active contacts** |

**Correction.** [velocity_frames](velocity_frames.md) called `0x004f3c8c` a "mode/quality flag".
It is the active contact count, written by `0x0042c0d0`.

### 5.2 Gather `Vehicle_TrackContactsGather` `0x0042cf90`
- Transforms the class's track contact points (`class model +0x1b4`, 3 floats each) by the
  orientation and position into `0x004f3c20`.
- For more than 4 or more than 6 points it adds midpoints (`Vec3_Midpoint`).
- Runs one terrain query of radius 500.0 around the vehicle (`0x004444b0`), then finds each
  point's ground height with `Terrain_FindGroundHeight` `0x004290f0`, using tolerance
  `-(vy*dt)`.

### 5.3 Classify `Vehicle_TrackContactDispatch` `0x0042c0d0`
```
thr = dt * 5.0
for each point i:
    if surface type == 3: h -= [0x004f376c];  if == 4: h -= [0x004f3698]
    active[i] = (h >= point[i].y - thr)
switch (activeCount):
  0: airborne(+0x10) = 1
     tp = max((pitch + 0.5236) * -0.7, -0.7);  tr = roll * -0.7
     k  = expApprox(-cfg+0x224 * dt)
     pitchRate = (1-k)*tp + k*pitchRate;  rollRate = (1-k)*tr + k*rollRate   // nose drops in the air
  1: Vehicle_SingleContactSnap            0x0042c520
  2: Vehicle_TwoContactSnap               0x0042c640
  3: SurfaceMask_AndThree(l0,l1,l2) ? TwoContactSnap : Vehicle_SnapToContactPlane
  >3: Vehicle_SelectTopContacts(up) then Vehicle_SnapToContactPlane           0x0042cc00, 0x0042ca40
  (every non-zero case clears airborne)
```

### 5.4 Snaps and torque
- **One contact** (`0x0042c520`): y is set on the plane through the contact
  (`Vehicle_PlaneHeightAt`), then `Vehicle_ApplyContactTorque(1.0)` with the lever arm to the
  contact.
- **Two contacts** (`0x0042c640`):
  - builds the edge between them and a plane through it and the up vector
    (`Vehicle_SetUpFromTriangle`);
  - sets y, then applies torque.
- **Three or more** (`0x0042ca40`):
  - plane through three contact points, which becomes the new up;
  - y from `Vehicle_PlaneHeightAt`, pitch and roll rates zeroed;
  - `Vehicle_StopRotation_EngineWhine` on the landing frame only (see below).
- **Top contacts** (`0x0042cc00`): keeps the points highest along the up axis, deactivates the
  others and rebuilds the active list (`Vehicle_BuildActiveContactList` `0x0042cf60`).
- **Plane height** `Vehicle_PlaneHeightAt` `0x0042cde0`: `y = (d - x*nx - z*nz) / ny`, with
  `ny == 0` replaced by 0.0001.
- `Vehicle_ApplyContactTorque(k)` `0x0042c8d0`:
  ```
  k = [0x004f3ac8] / gravity * k
  if airborne(+0x10) is still set: Vehicle_StopRotation_EngineWhine   // landing frame
                                   // (the dispatcher clears +0x10 only after the snap returns)
  pitchRate += k * arm.x;  rollRate += k * arm.z      each clamped to +-1.2
  vel.xz    += dt * gravity * 5.0 * up.xz              // slide off slopes
  ```
- `Vehicle_StopRotation_EngineWhine` `0x0042cb50` zeroes pitch and roll rates. For the player
  it plays slot 5 at volume `clamp(|vy|*0.1, 0, 1)`.
  - Both callers test `+0x10` first (bytes `8b4610 85c0 74..`) and call it only while the
    airborne flag from the previous frame is still set.
  - So it runs once per landing, and the volume scales with impact speed: a landing thud.
    The name is historical.
- `Vehicle_ThirdContactSnap` `0x0042d320` (player, grounded; section 5 dispatch):
  - Among the first `min(4, n)` points other than the two in use, it finds the deepest
    penetration.
  - If that exceeds `dt`, it re-planes through the three points and re-aligns.
  - **Correction.** The `0x0042bf90` record guessed "camera shake"; it is a contact refinement.

### 5.5 Vertical velocity and slopes
`Vehicle_TrackVerticalVelocity` `0x0042c2e0`: vertical velocity is **differentiated from
position**, not integrated:
```
vyMeasured = (y - prevY(+0x318)) * invDt
vy = (contacts < 3) ? k*vy + (1-k)*vyMeasured, k = expApprox(-5.0*dt)
                    : vyMeasured
Vehicle_ApplySlopeForces()            // 0x0042c420
if vy > 55.0: vy = 0
if not attached (+0x25c == 0): body = M*world; if contacts >= 3: body.y = 0
```
`Vehicle_ApplySlopeForces` `0x0042c420`:
- For up to 3 active contacts whose surface normal `n` is steeper than `[0x004f3338]`
  (`n.y < threshold`):
  - `vel.xz += n.xz * dt * g * 5.0`
  - `vy += (n.y - 1) * dt * g * 5.0`

`Vehicle_AlignToUpNormal` `0x0042da40`:
1. Re-derives forward so it is perpendicular to up: `fwd.y = -(ux*fx + uz*fz)/uy`, with
   `uy == 0` replaced by 0.001.
2. Pitch and roll come from `asin`.
3. The matrix is rebuilt.

### 5.6 Water, buoyancy and automatic mode change `0x0042d5c0`
- **Platform attach.** Surface type below 4 detaches from a moving platform: it copies the
  platform matrix and recovers yaw with `fpatan`. Otherwise it attaches (`+0x25c`, `+0xed4`).
- **Auto mode.** In deep water it tries amphib mode (`+0x24` available, depth ≥ 2), then
  hover (`+0x28`).
- **Submerged** by more than 1.0:
  - sets flags `+0x30` / `+0x38`, with a message and sound for the player;
  - applies drown damage `8.0*dt` per tick through `Vehicle_TakeDamage` or
    `AIVehicle_TakeDamage`;
  - uses a depth term `depth*dt*12.0` and a constant -10.0.
- Returning 0 skips the contact solver for the frame.

## 6. The other movement models

**HOVER (4)** `0x00427140`: runs the common skeleton, then `Vehicle_HoverSuspension`
`0x00427440`.
- **Ground plane.** It samples the ground (`Vehicle_GatherGroundContacts`), takes the plane of
  the 3 lowest points and pushes along its normal by `dt*g`. That push is ×12.0 on slopes
  steeper than `[0x004f3338]`.
- **Up vector** is blended toward the normal with `k = expApprox(cfg+0x234*dt)`.
- **Clearance** is the minimum over the hover points minus `cfg+0x228`:
  - The thruster node `+0xeec` is on only while clearance ≤ 2.0.
  - Above 2.0, a rising vy is zeroed.
  - The spring is `vy = k*vy - (1-k)*cfg+0x230*clearance`, with `k = expApprox(cfg+0x22c*dt)`.
  - Penetration pushes y up by `clearance - 0.5`; on steep ground it also adds `vy += 5.0`.
- **Pitch** is clamped to ±0.5236.

**AMPHIB (5)** `0x004279f0`: supports riding a platform (`+0x25c`). Local yaw `+0x3b4` is
composed with the parent matrix, and velocity is `Δpos * invDt`. Ground step
`Vehicle_AmphibGroundSnap` `0x00427ec0`:
- **Off terrain.** If fewer samples hit terrain than the class has points:
  - the player gets `+0x40 = 1` and switches to TRACK;
  - an AI vehicle stops and waits 8.0 s.
- **Height.** y = highest sample + `cfg+0x228`. vy is measured from Δy.
- **Up vector.** From `Vehicle_ApplyBodyWobble` `0x00429240`, a sinusoidal pitch and roll
  wobble driven by speed and the clock with tuning `cfg+0x238..+0x250` (`*_wave[0..6]`). It is then blended
  toward the default up; pitch is clamped to ±0.5236.

**SUB (2)** `0x00428520`:
- Pitch input is rate-driven, with the rate clamped to `cfg+0xb4`:
  - pitch is clamped to ±0.5 and decays with `expApprox(-7*dt)` when released;
  - roll leans by `cfg+0x250*yawRate*speed`, clamped to ±0.35.
- `Vehicle_SubVerticalSpeed` `0x00428c20`:
  - vertical input gives `vy += cfg+0xa8*dt*input`, clamped to ±20.0, snapping to 0 on reversal;
  - with no input, vy decays at rate 2.0 during the mode-change cooldown, else 10.0.
- `Vehicle_UpdateGroundClearanceAndAutoAmphib` `0x004289f0`:
  - clamps against the surface and adds speed-driven bobbing (`sin` terms, `cfg+0x238..+0x24c`, plus the turn lean `cfg+0x250`);
  - drives an under-surface overlay and sound;
  - the player auto-switches to AMPHIB within 2.2 of the surface.

**BASIC (0)** `0x00428120` is a simple ground follower used by non-player classes.
- **Input.** `Vehicle_CockpitThrottle` when camera mode is 2, else `Vehicle_BasicYawInput`.
- **Ground.** `Vehicle_BasicGroundFollow`: y = max sample + `cfg+0x228`, pitch and roll forced
  to 0.

## 7. Mode changes

Entry points by movement type:

| target | function | refused when |
|---|---|---|
| TRACK | `Vehicle_TransitionToTrackMode` `0x0042ac90` | cooldown `+0x64 > now`; from SUB unless forced |
| AMPHIB | `Vehicle_TransitionToAmphibMode` `0x0042aeb0` | cooldown; `+0x24 == 0`; from SUB when EDX != 0 |
| HOVER | `Vehicle_TransitionToHoverMode` `0x0042b0f0` | cooldown; `+0x28 == 0`; from SUB unless forced |
| SUB | `Vehicle_TransitionToSubMode` `0x0042b2a0` | cooldown; `+0x2c == 0`; from TRACK unless forced |
| FLY | `Vehicle_TransitionToFlyModeUnused` `0x0042b4c0` | cooldown (5.0 s instead of 1.0; no caller path in play) |

Common steps:
1. Store the previous type in `+0x08`.
2. `Vehicle_ActivateModeConfigNode(type)` swaps the class entry and stops sound slots 0-2.
3. Set cooldown `now + 1.0`.
4. For the local player, show a 5.0 s message and select the HUD mode icon.
5. Swap effect animations `+0xf44` / `+0xf54`.

Entering SUB also:
- sets vy = −3.0 and y −= 4.1;
- picks a usable weapon;
- deactivates the copters (`Copters_ResolveAndDeactivate` looks up nodes `copter01` and
  `copter02`).

Leaving SUB reactivates the copters (`Copters_Activate`).

`Vehicle_RequestModeTransitionByEnum` `0x0042b520` maps 1-5 to these; it is used by save
restore and the network.

## 8. Health, damage, death

- **Hit points.** `inst+0xf34` in [0, `cfg+0x398`]. `Vehicle_AdjustHealth` `0x0043b5d0` adds or
  sets and clamps, then publishes the HUD ratio `cfg+0x39c*hp` to `[0x004f3754]`.
- **Player damage** `Vehicle_TakeDamage` `0x0043bcc0`:
  1. **Ignored** when invulnerable (`+0x5a0`) or when the state is 4 or 5.
  2. **Special weapon flags.**
     - Bit 12 starts damage over time (`Vehicle_StartDamageOverTime`: rate `+0x4c`, 4.0 s,
       with an effect).
     - Bit 21 goes through `Vehicle_ApplyEffectSlotHit` `0x0043b790` (disassembly re-read):
       - The weapon pushes the vehicle's effect slot `+0x5c4` via `Weapon_UpdateLightSlot`: by
         `c+0x398` when the weapon has flag bit 11, else by `c+0x39c*dmg`. Weapons with flag
         `0x200` push it negative.
       - Damage becomes **0** when the slot's current value lies in [-0.5, 0]. That includes a
         fresh slot, so the first such hit only charges the slot.
       - Here `c = [inst+4]`: `health` config values at `+0x398/+0x39c`.
       - **Correction:** this was named `Vehicle_ApplyShieldAbsorb` and described as a shield.
  3. **Disabled vehicle.** When `+0x18` is set (effect slot below -0.5, see `Vehicle_HealthTick`)
     and the weapon lacks flag 0x200, damage becomes `cfg+0x398`, i.e. full health.
  4. **Extra lives.** Health reaching 0 with extra lives `+0xf38` refills it. The value
     123456789 means infinite.
  5. **Knockback.** Damage over 5.0 knocks the vehicle back:
     `Vehicle_ApplyKnockback(k*0.005, k*0.333)` with `k = cfg+0x220*amount`. It tilts the
     vehicle and pushes body velocity away from the hit direction. Force feedback is
     `amount*0.05`.
  6. **Death.** Engine sound off, camera saved and restored, 5.0 s message, parts
     deactivated. The state becomes 5 if airborne, else `Vehicle_PlayerDied` (state 4). A
     network kill message is sent and the alarms stop.
- **Per-tick** `Vehicle_HealthTick` `0x004399c0`:
  - applies damage over time until `+0x58` expires;
  - applies an instant kill request `+0x1c`;
  - for the player below 25% health, loops the low-health alarm every `[0x004f3758]` s
    (`Vehicle_SetLowHealthAlarm`).
- `Vehicle_CheckDeath` `0x0043c010` returns 1 when hp ≤ 0. It also stops damage over time and
  deactivates weapon mounts (`Vehicle_DeactivatePartsOnDeath`: 8 slots at `+0x754`, stride
  0x2b dwords).

## 9. Utilities
- **Teleport.** `Vehicle_Teleport` / `Vehicle_TeleportToNode` set position, yaw and zone.
- **Stop.** `Vehicle_ZeroMotion` zeroes every velocity, rate and demand.
- **Sync from scene.** `Vehicle_SyncPlayerTransformFromObject` copies the scene object's pose
  back into the instance (state event 10).
- **Debug.** `Vehicle_DrawDebugInfo` prints `"%s using %s dynamics"`, `"%s is DEAD!"` and the
  goal-node line.
- **Helpers** (one line each in the reference):
  - `Sign_Float` `0x00426350` returns -1, 0 or 1.
  - `Vehicle_ComputeGroundContactHeight` `0x0042b8c0` projects forward onto the ground plane.
  - `Vehicle_BuildBasisFromUpForward` `0x0042b970` builds the matrix from up and forward.
  - `Vehicle_ComposeLocalMatrix` `0x0043afd0`.
  - `Vehicle_ReleaseF48` `0x0042b4a0` stops effect `+0xf48`.
  - `Vehicle_StopModeTransitionSoundSlot` `0x00438690` stops sound slot i.
  - `Vehicle_ClearFireState` `0x00439990`.
  - `Vehicle_StopAlarmSounds` `0x00439b70`.
  - `VehicleGlobals_StaticInit` / `StaticAtexit` `0x00429f20` / `0x00429f40`.

## 10. Camera view states
`Vehicle_SetCameraViewState` `0x00405c90` (state in ECX).
- **Player and fields.** The player vehicle is `V = [[0x004f36a8]+4]`. The current state is
  `[V+0x58C]` and the previous one `[V+0x590]`.
- **Ignored** when there is no player, the state is unchanged, or the switch is 2 -> 7.
- Leaving 7 for any state other than 8 first applies 8.
- **State 0** toggles: 1 -> 3, 2 -> 8, 3 -> 1.
- **State 1:** `[V+0x4F4]=0`, `[V+0x4F8]=[V+0x534]`; coming from 3 restores control bit 1; sets
  control bit 3.
- **State 2:** calls `0x004a7b20(1)` (`zVideo_SetD3DDeviceCreated`; its role here is not
  explained).
- **State 3:** saves control bit 1 to `[0x004e5cc4]` and clears it; `[V+0x508]=0`; clears
  control bit 3.
- **States 4-6:** nothing.
- **State 7** (weapon camera): the weapon node `[[[V+0x5E4]+0x28]+0xC]` attaches camera
  `[0x004f36bc]` with value 0.5 and flag 2; HUD slot 0x60(1).
- **State 8** means "return":
  - Leaving 7 undoes the state-7 steps, resets the chase camera, and refreshes `[0x0057da28]`
    from `[0x004f3ab0]`.
  - Leaving 2 calls `0x004a7b20(0)`.
  - **The resulting state is the previous state**, not 8.

`Vehicle_ResetChaseCamera` `0x00405650`:
- **Eye** = vehicle pos `[V+0x3EC]` + offset `[V+0x55C]`, looking at the vehicle pos raised by
  `[V+0x500]`.
- **Forward** = normalised (`[V+0x100C]` - eye), stored at `+0x520` and `+0x434`, and
  flattened at `+0x574`.

## 11. Constants
| value | where | meaning | tag |
|---|---|---|---|
| 0.005 | `0x00426390` | minimum dt | `CONFIRMED-BINARY` + `VERIFIED-ORACLE` |
| 6.2831855 | movers | yaw wrap | `CONFIRMED-BINARY` |
| -28.0 | `0x004d0840` | lateral slope term in slip accel | `CONFIRMED-BINARY` |
| 1e-7 (double) | `0x004d0838` | dt guard in slip accel | `CONFIRMED-BINARY` |
| 5.0 | `0x004d0898` | contact threshold `dt*5`, slope push `g*5` | `CONFIRMED-BINARY` |
| -0.5236, -0.7 | `0x004d089c`, `0x004d08a0` | airborne nose-down target | `CONFIRMED-BINARY` |
| -5.0 | `0x004d08ac` | vertical-velocity blend rate (< 3 contacts) | `CONFIRMED-BINARY` |
| 55.0 | `0x004d08b0` | vertical velocity guard | `CONFIRMED-BINARY` |
| ±1.2 | `0x004d08b4/b8` | contact torque rate clamp | `CONFIRMED-BINARY` |
| 12102200.0 | `0x004d07b0`, `0x004d08a4` | expApprox multiplier | `CONFIRMED-BINARY` |
| 500.0 | `0x43fa0000` imm | contact terrain query radius | `CONFIRMED-BINARY` |
| -300.0 / -250.0 / 10000.9 | ground search | sentinels in `Terrain_FindGroundHeight` | `CONFIRMED-BINARY` |
| 1.0 / 5.0 | mode changes | mode cooldown / FLY cooldown | `CONFIRMED-BINARY` |
| -3.0, 4.1 | `Vehicle_TransitionToSubMode` | dive speed and drop on entering SUB | `CONFIRMED-BINARY` |
| 2.0, 10.0, ±20.0 | `0x00428c20` | SUB vertical decay rates and clamp | `CONFIRMED-BINARY` |
| 8.0 | `0x0042d5c0` | drown damage per second | `CONFIRMED-BINARY` |
| 4.0 | `0x0043b730` | damage-over-time duration | `CONFIRMED-BINARY` |
| 5.0, 0.005, 0.333, 0.05 | `0x0043bcc0` | knockback threshold and factors, force feedback | `CONFIRMED-BINARY` |
| 123456789 | `0x0043bcc0` | "infinite lives" sentinel | `CONFIRMED-BINARY` |

## 12. Open - what is not established
- **Oracle.** None of sections 4.2, 5, 6, 7 or 8 has been compared against a running game.
  Existing traces with the tank moving (`Recoil16`-`Recoil22`) exercise steering and throttle
  on flat ground. Slopes, jumps, water, mode changes, skids and damage need targeted traces.
- **Sign of "forward".** Forward is `-row2`, while body `v.z = row2 . world`. So a positive
  `v.z` points along −forward. The throttle record reads negative demand as "forward"; that
  reading has not been checked against the matrix algebra.
- **Class-file keys.** Mapped in [`VehicleClassConfig.md`](../structs/VehicleClassConfig.md):
  - `friction` = `0xcc/0xd0/0xd4` and `stopping` = `0xd0/0xd8`;
  - `a_damping` = `0x224` and `mode_alt` = `0x228`;
  - `alt_control` = `0x22c..0x234` and `*_wave` = `0x238..0x250` (**seven** values; corrected
    2026-09-24);
  - `mass` = `0x21c` (default 1.0), with `0x220 = 1/mass` derived by the loader.

  Both formerly unmapped offsets are closed (item E2, bytes `0x00422b11..0x00422b4c` and
  `0x00422b90..0x00422cb0`). The knockback `k = cfg+0x220 * amount` is therefore `amount / mass`,
  and the SUB/hover roll lean `cfg+0x250` is `*_wave[6]`.
- **Surface offsets.** The surface-type offsets `[0x004f376c]` (type 3) and `[0x004f3698]`
  (type 4), and the steepness threshold `[0x004f3338]`, are not identified.
- ~~**Branch detail.**~~ `Vehicle_GatherGroundContacts` `0x00428d60` is itemised in the ledger
  (2026-09-24). Sample points are **pre-offset by `vy*dt`** except in SUB mode. The query runs in
  the vehicle's zone with radius 500.0. Point 0's hit record is kept at `+0x44c` (height `+0x458`,
  zone `+0x444`). `+0x34` / `+0x3c` flag surface types 1 / 4.
