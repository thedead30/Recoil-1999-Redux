# zDEClient: world hazards (craters, quicksand) and the anim script interpreter

**Status:** membership CLOSED, 90/90 CONFIRMED. Mostly Ghidra decompile; the colour fade and the
screen shake rect were read from disassembly, and several commands from bytes. 11 callees are
delegated: the zEffect module and three UI functions. Tag: `CONFIRMED-BINARY`.

## Init — `DEClient_Init` `0x004558f0`
Reads `declient.zrd` through the detail preset file. Missing config → report, then defaults.
| hazard | flags | segments | floats |
|---|---|---|---|
| crater `[0x00539ce8]` | 0x100C | 7 | 0, 4.0, 20.0 |
| quicksand `[0x00539d34]` | 0x1008 | 7 | 40.0, 4.0, 20.0 |

- Crater materials come from a config list.
- Quicksand textures: none → untextured (reported); one → a single texture; several → a cycling
  material.
- `srand(time(0))`.
- Optional 1000 ms timer.

## Hazards
- Records: 0x34 bytes, type 1 crater / 3 quicksand, kept in a growable vector. Saved as
  `Crater%d` / `QSand%d`.
- Disc build (`0x00456010` quicksand, `0x00456c80` crater):
  - An n-segment ring, shifted inside its world partition cell with an extra 1.0 margin.
  - Rejected when the cell is full (per-cell limit in world data) or when it overlaps another
    hazard's bounds within **5.0**.
- Tessellation:
  - Crater: rim → mid ring (halfway in, lowered) → floor point one depth below. UV = position / 2r.
  - Quicksand: the same pit plus a surface disc on a second model. UV scale 4.5 / 2r.
  - The model epsilon is set to 0.005 during crater instancing.
- **Defects:** the floor UV is written through a null buffer when untextured; a failed quicksand
  surface node leaks its buffers.

## Anim script interpreter
- Commands return **2** to continue, **1** to keep running next tick, **0** to rewind.
- Objects: 0x28-byte entries at `[interp+0x114]`, node at +0x24. Index −100 or −200 selects
  special nodes (interp +0x40 / +0x74).
- Lookups by name cache their index, but the anim-node lookups (table `[0x00575dac]`, 0x134
  bytes each, loaded by `Anim_LoadFile`) test `> 0`, so **entry 0 is never used**. The same holds
  for light and sound-node indices and for the attach command.
- Timed commands share a time carry `[0x00575dd8]`. Durations are corrected for overshoot, and the
  end values are applied once the duration passes.
- Commands: attach/detach, set active, position, rotation, scale, flags/value ramp, camera params
  and optics animation, light control and colour/range fade, sound-node control, sound, message,
  colour fade (4 channels, ×255 +0.5 → 16-bit colour + alpha), screen shake/flash rect (distance
  scaled, capped at ¼ of the larger screen dimension), segment stretch between anchors,
  spawn/wait on anim nodes, physics debris.
- **Condition chain:** random table (200 entries), squared distance to the event anchor, vertical
  clearance test (default height 50.0), effects level. A `!` record is an OR alternative; a false
  chain skips the block.
- **Physics debris:** randomised launch (yaw in degrees, pitch/90), gravity, ground probe
  (candidate surfaces within 10.0 above), bounce velocity ×0.2, and a 15-second fall timeout.
  Optional resume-script, bounce sound (volume |v| / limit), spin, roll, scale and alpha.
- The interpreter clock is capped at 86400.

## Function index (hazards and records)
- **Lifetime:**
  - `DEClient_Shutdown` `0x00455e40` (tree cleared, buffers freed);
  - `DEClient_ResolveMaterial` `0x00455dd0` (material by texture, created if missing);
  - `DEClient_SetCurrentWorld` `0x00458aa0`;
  - `World_GetPartitionCell` `0x00458ac0` (`[data+0x80][row] + col*0x40`, no bounds check);
  - `DEClient_SetByteList00575dd4` `0x00458b20`;
  - `DEClient_RegisterAtExit` `0x004576a0`.
- **Quicksand:**
  - `DEClient_InstanceQuickSand` `0x00455ef0`, gated by hook `[0x00539de4]`; model epsilon 0.01
    during the build;
  - query template `QueryInit_Copy11From00539d34` `0x00455ed0`;
  - object: `DEObjectA_Create` `0x004563d0` (type 3), `DEObjectA_RebuildClip` `0x00456450`,
    `DEObjectA_TesselateQuicksand` `0x004564b0`, `DEObjectA_Free` `0x00455ea0`.
- **Crater:**
  - `DEClient_InstanceCrater` `0x00456b20`, epsilon 0.005, reached via `Query_RunWithGateHook`
    `0x00456c50` (hook `[0x00539de8]`);
  - template `QueryInit_Copy10From00539ce8` `0x00456b00`;
  - object: `DEObjectB_Create` `0x00457040` (type 1), `DEObjectB_RebuildClip` `0x004570e0`,
    `DEObjectB_TesselateCrater` `0x00457140`, `DEObjectB_Free` `0x00456ad0`.
- **Records** (0x34 bytes):
  - `DEClient_AddHazardRecord` `0x00457840` (type 1 copies 10 dwords, type 3 copies 11);
  - `DEClient_DispatchRecords` `0x00457c50`;
  - `DEClient_BuildHazardSaveRecords` `0x00457b40` (`Crater%d`, `QSand%d`);
  - `DEClient_RegisterAllEntries` `0x004575f0`, `DEClient_ResetState00539df0` `0x00457660`;
  - `Record52_CopyRange` `0x00458a30`, `Record52_FillN` `0x00458a70`.
- **Object map** (MSVC `std::map`, nil node `[0x00539e10]`, as found):
  - `DEClient_Tree_Construct` `0x00457cc0`, `_InsertUnique` `0x00457d90`, `_InsertNode`
    `0x004585a0`;
  - `_EraseRange` `0x00457e80`, `_EraseNode` `0x00457fe0`, `_EraseSubtree` `0x00458510`,
    `_Destroy` `0x004576e0`;
  - `DEClient_TreeIterator_Increment` / `_Decrement` `0x004588c0/0x00458970`;
  - `DEClient_ClearTree` `0x00457ae0`, `DEClient_DestroyAllObjects` `0x00457750`.

## Function index (anim script commands)
Return codes as above: 2 continue, 1 run again next tick, 0 rewind.

- **Flow:**
  - `Script_AdvanceRecord` `0x0045ae30`, `Script_Rewind` `0x0045c2f0`;
  - `Script_SkipToQuote` `0x0045c6b0`, `Script_CmdNop` `0x0045c6e0`;
  - `Script_CmdCallback` `0x0045c6f0` (hook `+0x6C`);
  - `Script_CmdWaitOrLoop` `0x0045c310` (count and time modes);
  - `Script_RunTimed` `0x0045b120`, `Script_ExecuteKeyframe` `0x0045ae90`;
  - local scripts: `Script_CmdStopLocalScript` `0x0045bbb0` (state 3) and
    `Script_CmdResumeLocalScript` `0x0045bb00`;
  - `AnimEvent_EvaluateConditionChain` `0x0045c3c0`.
- **Objects:**
  - `Script_CmdSetActive` `0x00459e30`, `Script_CmdSetPosition` `0x00459ce0`,
    `Script_CmdSetRotation` `0x00459ae0`, `Script_CmdSetScale` `0x00459cb0`;
  - `Script_CmdSetObjectFlags` `0x0045b210`, `Script_CmdValueRamp` `0x0045b280`;
  - `Script_CmdAttach` `0x0045b3b0` (indices must be > 0), `Script_CmdDetach` `0x0045b410`;
  - `Script_CmdSetAnimFrame` `0x0045b440`;
  - `AnimEvent_AnimateObjectTransform` `0x0045a9d0`;
  - `AnimEvent_StretchSegmentBetweenAnchors` `0x0045b4a0`.
- **Camera:**
  - `Script_CmdCamera` `0x00458eb0`, `Script_CmdCameraParams` `0x00459580`;
  - `AnimEvent_AnimateCameraOptics` `0x004596c0` (seven channels of start, end and rate).
- **Lights and sound:**
  - `Script_CmdLightControl` `0x00459080`, `AnimEvent_FadeLightColorAndRange` `0x00459280`;
  - `Script_CmdSoundNodeControl` `0x00458f70`, `Script_CmdPlaySound` `0x00458e10` (positional
    when an object is given).
- **World and screen:**
  - `Script_CmdWorldSetup` `0x00459510` (flag bits 0-3);
  - `Script_CmdColourFade` `0x0045c710`;
  - `Script_CmdShowMessage` `0x0045cbc0` (message table `[0x00575db4]`, 0x24 bytes each).
- **Anim nodes and effects** (table `[0x00575dac]`, index 0 never used):
  - `Script_CmdSpawnAnimNode` `0x0045bc60`, `Script_CmdSpawnFromAnimNode` `0x0045b8b0`;
  - `Script_CmdAnimNodeStopOrFade` `0x0045c100`, `Script_CmdAnimNodeCall45d6b0` `0x0045c1a0`;
  - `Script_CmdAnimNodeSetState` `0x0045c240` (state 2 → 6, others → 4, 5 unchanged).
- **Debris and ground:**
  - `AnimEvent_SpawnPhysicsDebris` `0x00459e70`;
  - `Ground_FindSurfaceNear` `0x0045a920`;
  - `Anim_ProximityClearanceTest` `0x0045c530`;
  - `Anim_SquaredDistanceToEventAnchor` `0x0045c640` (anchor `[0x00575dc8..d0]`);
  - `DEObject_TickTimer` `0x00458b50` (state 1 or 2: the timer counts down and fires on ≤ 0).

## Defects to reproduce (others)
- The object transform animation sets the final rotation with the object setter even for cameras.
- Index 0 is excluded as described above.
- Allocations are unchecked.

## Still open
- Config key names (not visible in the decompile).
- The zEffect functions called by the spawn and anim-node commands.
