# Sound (`sound`) — first pass

**Scope:** the sound engine — voice/resource playback API, device and backend init, the 3D listener, and the stream-cue state machine. Registered 2026-09-16 as 49 members, **membership OPEN**. The scene-graph sound *nodes* (`World_AddSound` `0x00451590`, `gwSoundNodeSetActive` `0x00452c60`, `gwSoundNodeProcessPlay` `0x00452dc0`, `Render_ProcessSoundNode` `0x0044af60`) are CONFIRMED **clients** of this API living in `cls_world`/`scene_render`; they are not members.

**Original source module:** error reports embed `D:\Proj\GameZRecoil\zSound\zsnd_play.cpp` (string at `0x004e2208`), so the authors' own module name for this code is `zSound`.

## Runtime state — CONFIRMED-BINARY

| Global | Meaning |
|---|---|
| `[0x0056b2a0]` | initialised flag; set by `zSndInit`, tested by every playback entry |
| `[0x0056b2a4]` | playback gate; **`zSndInit` sets it to 0**, so something else must enable sound |
| `[0x0056b2a8]` | `zSndInit`'s ECX argument — an HWND-shaped value (`0x00080ae6` in-trace) |
| `[0x0056b2ac]` | **audio API mode**: 0 = DirectSound hardware buffer, 1 = A3D |
| `[0x0056b3b0]` / `[0x0056b3b8]` / `[0x0056b3c4]` | resolved setting nodes for `SoundLOD` / `MuteSound` / `SoundVolume` |
| `[0x0056b3bc]` | the MuteSound value read at init |
| `[0x0056b3c0]` | SoundVolume fallback default = 1.0f |
| `[0x0056b3d0]` / `[0x0056b3d4]` | the currently tracked resource / voice (set by the DirectSound start path) |

**VERIFIED-ORACLE** (TTD Recoil21, lifetime `[71:0 – B08D:81]`): `[0x0056b2a0] = 1`, `[0x0056b2a4] = 1` (gate open), `[0x0056b2a8] = 0x00080ae6`, and `[0x0056b2ac] = 0` with **zero writes and 196 reads** in-trace — so the API mode is fixed before the trace window and **DirectSound (mode 0) is the live path**; A3D never runs. `Sound_StartVoice` executed 28 times. All three setting lookups resolved to real config nodes, not the built-in fallbacks.

## Entry points and dispatch — CONFIRMED-BINARY

Every playback entry is gated on `[0x0056b2a4] != 0 && [0x0056b2a0] != 0 && ECX != 0`, returning 0 (or −1) otherwise.

- **`Sound_PlayResourceAuto` `0x0049f960`** (25 callers) — dispatches on the resource kind `[R+0]`: **1 = stream** → `Sound_StartStreamResourceSimple`; anything else → `Sound_PlayResourceSimple`.
- **`Sound_PlayResourceSimple` `0x0049fcf0`** — stream kind tails to `Sound_StartStreamResourceAtPosition`; otherwise a sampled voice, but **only when bit 3 of byte `[R+0xC]` is set**, else it returns 0 having done nothing. Volume = `[R+0x10] × *[0x0056b3c4] × a1` (resource gain × SoundVolume setting × caller gain).
- **`Sound_StartVoice` `0x0049fa10`** and **`Sound_AllocateVoiceSlot` `0x0049f6d0`** both branch on `[0x0056b2ac]`: mode 0 → the DirectSound path, mode 1 → the A3D path, **any other value does nothing at all**. This pairing is what identifies the selector.

## The volume curve — CONFIRMED-BINARY, and a required quirk

`Sound_VolumeToAttenuation` `0x0049f9a0` converts a linear volume to the integer attenuation handed to `IDirectSoundBuffer::SetVolume`:

```
v >= 1.0        ->      0          (no attenuation)
v <= 2^-10      ->  -10000         (0xFFFFD8F0 = DSBVOLUME_MIN, silence)
otherwise       ->  ftol(1000 * log2(v) - 0.5)
```

- The `-0.5` before the truncating `ftol` makes it **round to nearest** for the negative results.
- **1000·log₂(v) = 3321.9·log₁₀(v)**, but a true hundredth-of-a-decibel attenuation would be **2000·log₁₀(v)**. Recoil's curve is therefore about **1.66× steeper than real decibels**. This is not an approximation of dB — it is a different curve, and a remake using the textbook formula will get every volume wrong.
- The clamp threshold is not arbitrary: 2⁻¹⁰ is exactly where this curve reaches −10000.

## The DirectSound interface — CONFIRMED-BINARY

The voice object at `[R+8]` is an `IDirectSoundBuffer`. The vtable slots are identified by their use pattern across `0x0049fbb0` and `0x0049fda0`:

| Slot | Method | Evidence |
|---|---|---|
| `+0x24` | `GetStatus` | takes an out-pointer; result tested against 2 |
| `+0x30` | `Play` | called with `(0, 0, flags)` last in the start sequence |
| `+0x34` | `SetCurrentPosition` | single value, before `Play` |
| `+0x3C` | `SetVolume` | fed the attenuation from `0x0049f9a0`, or −10000 when muted |
| `+0x48` | `Stop` | last call in the stop sequence |
| `+0x50` | `Restore` | called only when status == 2 (`DSBSTATUS_BUFFERLOST`) |

That `GetStatus → if BUFFERLOST then Restore → Stop` ordering is textbook DirectSound, and it is what pins the interface identity.

**Start sequence** (`Sound_StartVoice_DirectSound` `0x0049fbb0`, the live path — 78/28 trace hits):
`GetStatus` → `Restore` if the buffer was lost → `SetVolume` (or −10000 when the mute check `0x004a07a0` returns nonzero) → `SetCurrentPosition` → `Play(0, 0, looping)`, where **the LOOPING flag is bit 0 of byte `[R+0xC]`**.

**Stop sequence** (`Sound_StopOrRestoreVoice` `0x0049fda0`): `GetStatus` → `Restore` if lost → `Stop`. Every failure is reported through `0x004a4330` with the `zsnd_play.cpp` path and a line number.

**Asymmetry worth reproducing:** the start path registers `[0x0056b3d0] = R` and `[0x0056b3d4] = voice`, but the stop path clears `[0x0056b3d0]` and `[0x0056b3d8]` — a different second global.

## The `IDirectSound` device vtable — `VERIFIED-ORACLE`

`[0x0056b2b0]` holds the device. In Recoil21 it is the live object `0x029f0720`, whose vtable
`0x73961be4` resolves **by symbol** to `DSOUND!CImpDirectSound<CDirectSound>`:

| Slot | Method |
|---|---|
| `+0x00` | `QueryInterface` |
| `+0x04` | `AddRef` |
| `+0x08` | `Release` |
| `+0x0C` | `CreateSoundBuffer` |
| `+0x10` | `GetCaps` |
| `+0x14` | **`DuplicateSoundBuffer`** |
| `+0x18` | `SetCooperativeLevel` |
| `+0x1C` | `Compact` |

This is the standard `IDirectSound` layout, confirmed against loaded symbols rather than
inferred from use — the strongest evidence class available here. **0 writes** to
`[0x0056b2b0]` in-trace, so the device is created before the trace window opens.

## Voice duplication and the polyphony cap — `CONFIRMED-BINARY`

`Sound_DuplicateHardwareBuffer` (`0x0049f830`) supplies a playable buffer for a resource:

1. If the inline slot `[v+0x44]` is free and the source buffer's `GetStatus` shows
   `DSBSTATUS_PLAYING` (bit 0) **clear**, it reuses the original buffer — no duplicate.
2. Otherwise it scans the duplicate array at `[v+0x84]` (count `[v+0x80]`) for an idle entry.
3. Failing that, and **only while fewer than 5 exist**, it allocates a `0x3C` voice record
   and calls `DuplicateSoundBuffer`, growing the array by `realloc`.

**At most five duplicate voices per resource.** A remake without this cap will play more
simultaneous copies of a sound than the original does.

`Sound_DuplicateA3DVoice` (`0x0049f6f0`) is the structural twin for mode 1 (slots `+0xE0` /
`+0x48`, signed failure test, and it *does* report errors where the DirectSound twin fails
**silently**). A3D never runs in any trace, so its slot identities stay provisional.

> The `0x3C` voice record is the same *size* as the stream-cue record but a different
> structure. Do not conflate them.

## Sound banks and the resource stride — `CONFIRMED-BINARY`

`Sound_CreateBankAndRegisterProvider` (`0x004a09e0`) does `calloc(count, 0xB8)`, so a
**sound-resource descriptor is 0xB8 = 184 bytes**. The indexer
`Sound_Bank_GetDescriptorByIndex` (`0x004a0e90`) confirms it independently:
`idx*23` via `lea/shl/sub`, then `*8` — `idx * 184`.

Construction also **registers the bank as a provider**, appending it to the vector
`[0x0056b294]` begin / `[0x0056b298]` end / `[0x0056b29c]` capacity — a compiler-emitted
`std::vector::push_back` with doubling growth. `Sound_ResolveResourceByName` (`0x004a0990`)
searches exactly this vector, in **registration order**.

The indexer's bounds check uses a signed `jge` against the count, so it rejects
out-of-range indices but **not negative ones**.

## The software 3D model — `VERIFIED-ORACLE` (76 executions in Recoil21)

`Sound_Set3DParams_Dispatch` (`0x004a2a30`) switches on `[0x0056b2ac]`: mode 1 hands every
parameter to A3D hardware (`0x004a2a70`, vtable `+0x50` position, `+0x80` velocity, `+0xA0`
volume, `+0xB0` flag); mode 0 runs `Sound_Set3DParams_DirectSound` (`0x004a2b40`), which
**computes the whole model in software**. Mode 0 is the live path — the A3D twin never
executes in any trace.

**Listener state:** position `[0x0056b394…39C]`, orientation `[0x0056b370…378]`, velocity
`[0x0056b3a0…3a8]`. `[0x0056b36c]` enables the model (1 in-trace).

**Distance — a fast approximation, not `sqrt`:**
```
d2   = |pos - listener|²
dist = bitcast_float((bitcast_int(d2) >> 1) + 0x1FC00000)
```
The integer-halving trick, a few percent off a true square root. **A remake calling `sqrtf()`
will differ at every distance**, in both attenuation and Doppler. When `d2 == 0` the minimum
range is substituted.

**Attenuation** uses two per-resource ranges: beyond `[R+0x18]` (max) the buffer is set to
`DSBVOLUME_MIN` and the function **returns early**, skipping pan and frequency; at or beyond
`[R+0x14]` (min) an `ftol`-rounded term is added to the base gain and clamped to −10000.
Inside the min range there is no attenuation at all.

**Doppler** multiplies the buffer frequency by `1 - (Δv · dir) / (dist · c)`, where `c` is
read from `[0x004e2424]` — **the 1/345 reciprocal cached by `Sound_SetSpeedOfSound`**. The
engine never divides by the speed of sound; it multiplies by the stored reciprocal.

Errors report at `zsnd_3d.cpp` lines 352 and 356.

## Sound groups — `zsnd_grp.cpp`, and a stream *is* a group

`SoundGroup_ParseNode` (`0x004a4590`) `calloc(1, 0xB8)`s a descriptor and sets `[rec+0] = 1`
— the same tag `Sound_PlayResourceAuto` routes as "stream". `Sound_StartStreamResourceSimple`
is only a thunk into the cue-machine entry, and every field the parser writes
(`+8`, `+0x10`, `+0x14`, `+0x18`, `+0x1C`, `+0x20`, `+0x24`) is read back by the cue state
handlers and the variant picker at the same offsets. **There is one 0xB8 descriptor type.**

| key | field | notes |
|---|---|---|
| `REPEAT` | `+0x14` (16-bit) | −1 = infinite |
| `DELAY_REPEAT` | `+0x18` | replay gap |
| `DELAY_TERMINATION` | `+0x1C` | cooldown |
| `DYNAMIC_WEIGHTS` | `+8` flag, `+0x10` factor | clamped to (0, 1] |
| `PLAY_SOLO` | `+0xC` | flag only |

Variants (`SoundGroup_ParseVariantNode`, `0x004a49b0`) are 0x18-byte records keyed
`DELAY_PLAY` → `+4`, `PLAY_COUNT` → `+0`/`+2` (defaulting to `0xFFFF` = infinite),
`WEIGHT` → `+8`, name → `+0xC`. The parser **recurses into itself**, chaining records through
`+0x14` — the same pointer the state-1 walk follows — so a variant is a *linked list of timed
events*.

**Defect:** if any variant weight is ≤ 0.0001 the parser overwrites **every** weight with
`100.0/count`, discarding the entire hand-authored distribution rather than defaulting one
entry.

## The two config formats — `SYNTAX 1` and `SYNTAX 2`

`zSnd_CreateDevice` reads `ConfigTree_GetIntValue(root, "SYNTAX")`, **defaulting to 1**, and
picks `SoundConfig_ParseSyntax1` (`0x004a1510`) or `SoundConfig_ParseSyntax2` (`0x004a1870`).
Both share a prelude — `SOUND_GROUPS` → CD open, `SETS` → the semicolon string set
`[0x0056b364]`, `SPEED_OF_SOUND` → `Sound_SetSpeedOfSound` — then walk `SOUND_PATH` as
**name/value pairs** (`(childCount - 1) / 2` banks), and finish with `CD_TRACKS` →
`Sound_BuildNameTable`. Neither executes in any trace: sound config is parsed at load.

**The real difference is how flags are encoded.**

*Syntax 1 — positional.* Trailing array slots are `strcmp`'d against the literal `"TRUE"`,
packed into `[d+0xC]` as bits 0, 2, 1 by position. **Field order in the file is load-bearing.**

*Syntax 2 — named.* Six independent key probes, each setting **or clearing** its own bit by
mere presence, so order is irrelevant:

| key | bit |
|---|---|
| `LOOPED` | 0 (`0x01`) |
| `PURGEABLE` | 1 (`0x02`) |
| `3D` | 2 (`0x04`) |
| `VOICE` | 4 (`0x10`) |
| `FREQUENCY` | 5 (`0x20`) |
| `HARDWARE` | 6 (`0x40`) |

That migration from positional to named is why two parsers exist, and why `SYNTAX` defaults
to 1 for older files. Syntax 2 additionally reads `VOLUME` into `[d+0x10]` **clamped to
[0, 1]**, a second gain at `[d+0x28]`, and `A3DDIST` for the ranges (which also sets bit 2).

### Quality tiers — and where `0xB8` comes from
Three parallel tiers, `HIGH` / `MED` / `LOW`, fill `[d+0x88…0x94]`, `[d+0x98…0xA4]` and
`[d+0xA8…0xB4]`. Each has three cases:

- **absent** → the hardcoded default for that tier, pointer null;
- **a string** → `_strdup` the filename into the pointer and **zero** the format fields;
- **otherwise** → rate/bits/channels read inline, pointer null.

A tier is therefore *either* a named file *or* an inline format — the null pointer is the
discriminator. Defaults: **44100/16/2, 22050/16/1, 11025/8/1** (a SoundLOD ladder; syntax 1
always uses these). The tiers are the last members of the record, and
`0x88 + 3 × 0x10 = 0xB8` — **exactly the `calloc` stride and the indexer's arithmetic**,
confirming the descriptor size a third independent way.

## Resource descriptor (offsets byte-confirmed; field *meanings* INFERRED)

- `+0x00` kind (1 = stream)
- `+0x04` secondary kind, tested by the stop path
- `+0x08` voice object (`IDirectSoundBuffer*` in mode 0)
- `+0x0C` flag byte — **bit 0 = looping**, bit 3 = playable as a sample, bit 4 = gated on `[0x0056b3c8]`
- `+0x10` resource gain (float)
- `+0x30` current variant value, `+0x34` variant count, `+0x38` variant array, `+0x40` per-variant record array (stride 8)
- `+0x44` inline fallback voice slot, used when allocation fails

## The stream-cue state machine — `CONFIRMED-BINARY`

A stream cue is a scheduled *sequence* of sound events, not a single sound. It is the only
part of the sound engine that owns per-frame state.

**It is driven by the scene graph, not by calls.** `Sound_StreamCueSubsystem_LazyInit`
(`0x004a5350`) creates a node with `Object3D_Create` (`0x0044daa0`) and attaches
`gwNodeSetActionCallbackFirst` (`0x00447fe0`). That is why
`Sound_StreamCue_TickDispatch` has **zero static callers** — its address is passed as data to
the list-walk helper `0x0048cbd0`, and the walk itself is reached through the node action
callback. Any remake wiring these as direct calls will get the update order wrong.

Lazy init is guarded by four independent globals (`[0x0056b420]` node, `[0x0056b40c]`,
`[0x0056b410]` active list, `[0x0056b414]` free pool), each filled only when still zero, and
returns 0 on any allocation failure — so a partial failure **retries on the next call**
rather than leaving a half-built subsystem.

### Cue record — 0x3C bytes, pooled

| offset | meaning |
|---|---|
| `+0x04` | initial state marker, set to 1 at start |
| `+0x08` | positional flag — selects which play entry is used |
| `+0x0C`..`+0x14` | position vector (arg 2), copied when positional |
| `+0x18`..`+0x20` | play position vector (arg 3), zeroed when absent |
| `+0x24` | caller handle, passed to the play entries |
| `+0x28` | state timer (float seconds) |
| `+0x2C` | replay counter |
| `+0x30` | current event node in the selected variant list |
| `+0x34` | **state** (0–4) |
| `+0x38` | the resource `R` |

### The dispatcher — a real jump table

`Sound_StreamCue_TickDispatch` (`0x004a4c40`) is `cmp [cue+0x34],4 / ja default /
jmp dword ptr [0x004a4c98 + eax*4]`. The table has five entries:

| state | target | role |
|---|---|---|
| 0 | `0x004a4cb0` | select the first variant |
| 1 | `0x004a4ea0` | walk the variant's event list, playing each |
| 2 | `0x004a4fd0` | wait out the replay gap, then re-roll a variant |
| 3 | `0x004a5020` | post-sequence cooldown |
| 4 | inline at `0x004a4c7b` | retire |

Every arm returns 1. **`uf` truncates at the indirect `jmp`** — the table had to be walked by
hand; a 6-instruction digest for this function is a tooling artefact, not its size.

### Timing and resource fields

All four states advance `[cue+0x28]` by `[0x0056b424]`, the frame delta — the **only** time
source in the cue machine. The resource supplies the schedule:

| field | used by |
|---|---|
| `[R+0x14]` (short) | replay count; **−1 means infinite** (the counter is frozen rather than compared) |
| `[R+0x18]` | replay gap, state 2 |
| `[R+0x1C]` | cooldown; state 1 tests it `<= 0` to **skip** state 3 entirely |
| `[R+0x20]` | variant count; `< 1` retires the cue immediately |

Each replay **re-rolls** `Sound_StreamCue_PickWeightedRandomVariant` (`0x004a4d10`) rather
than repeating the selected variant — repeated ambient cues vary between loops by design.

### DEFECT — the retire path reclaims only one cue per tick

The retire arm stores the cue into the **single slot** `[0x0056b418]` *only if it is empty*,
but increments `[0x0056b41c]` **unconditionally**. `Sound_ServiceActiveStreamCues`
(`0x004a5050`) then unlinks exactly that one cue and decrements the counter once.

So when two or more cues retire in the same tick, the extras are never unlinked: they stay on
the active list and are re-dispatched in state 4 every frame, and `[0x0056b41c]` drifts upward
permanently. This is a bug in the original and must be reproduced, not fixed, for behavioural
parity. `CONFIRMED-BINARY` from the disassembly at `0x004a4c7b` and `0x004a5050`.

**Provisional:** 0 trace hits across Recoil18/21 — no stream cue is started in either trace,
so the runtime behaviour of this machine is unverified by oracle.

## Backends - which one v1 reproduces
The API mode `[0x0056b2ac]` picks the backend: **0 = DirectSound**, 1 = Aureal A3D (COM).
- `Sound_GetAPIMode` `0x004a12b0` reads the mode.
- `Sound_SetAPIModePreInit` `0x004a1290` changes it only before initialisation.
- **v1 target: mode 0**, because A3D does not exist on modern systems. A3D differences are noted
  only where the two backends behave differently.
- **DirectSound bring-up** `zSnd_CreateDeviceAndPrimaryBuffer` `0x004a1e50`:
  `DirectSoundCreate`, `DSSCL_NORMAL`, `GetCaps` (96-byte DSCAPS at `0x0056b2b8`), and a primary
  buffer with flags 0xC1.
- **A3D bring-up** `Sound_InitA3DDevice` `0x004a1d10`: `CoCreateInstance`, with three distinct
  error strings.
- `zSnd_ReleaseDevice` `0x004a1f40`.
- **Shutdown order** `zSnd_Shutdown` `0x004a13d0`: stream cues, device, CD, banks, fades, config.

## Per-frame update - `Sound_UpdateFrame` `0x0049f620` (`VERIFIED-ORACLE`, 16 runs in Recoil21)
1. Mode 1 only: A3D commit.
2. Step every fade (`SoundVoiceList_RemoveIf` `0x004a3c20` with dt, which calls `SoundFade_Step`).
3. **Marker cues for the one tracked resource** `[0x0056b3d0]`:
   - when marker time ≤ the sound clock `[0x0056b430]`, it calls the resource callback `R+0x2C`
     (set by `SoundResource_SetCallback` `0x004a1240`);
   - the index then advances **one marker per tick**, and several due markers fire on successive
     ticks. Reproduce, do not batch.
   - At the stop marker `[0x0056b3dc]` the voice stops; 999 means "no stop".

## Fades - `SoundFade_Step` `0x004a3ad0`
- **Step:** `current += sign(target - current) * dt * 2500.0`.
- **Clamp:** mode 0 to [-10000, 0] (hundredths of a dB) and applied with `SetVolume(ftol)`; mode 1
  to [0, 1].
- **Duration:** a full DirectSound fade takes **4 s**, while an A3D fade is effectively instant.
- **Quirk to reproduce:** completion is tested with exact float equality, so a target strictly
  inside the range can oscillate.
- **Finishing** optionally stops the voice. Records move to the finished list `[0x0056b404]`
  (`SoundVoiceList_PushBack` `0x004a3a80`).
- **Lists:**
  - Two `std::list`s are built statically: `SoundFadeLists_StaticInit` `0x004a3940`,
    `_RegisterStaticDtor` `0x004a39a0`, `_StaticDtor` `0x004a39b0`, `_CrtInitStub` `0x004a3930`.
  - Helpers: `SoundVoiceList_Erase` `0x004a3e50` and `SoundVoiceList_IteratorPostIncrement`
    `0x004a3e90`.
  - `SoundFade_Shutdown` `0x004a3d20` stops every active fade.

## Voices
- **Variants.** `Sound_PlayResourceVariant` `0x0049fd50` sets
  `volume = R+0x10 * gain * SoundVolume` and picks variant `i` (`[R+0x38][i]`, arg
  `[R+0x40][i*8]`). It sets the stop marker and starts the voice.
- **Gain.** `SoundVoice_SetGain` `0x004a11d0`: mode 0 stores
  `Sound_VolumeToAttenuation(master*gain)` as an int in `v+0x24`; mode 1 stores the float.
- **Frequency.** `SoundVoice_SetFrequencyRatio` `0x004a10e0` (disassembly re-read 2026-09-24):
  - the ratio is clamped to [0, 1];
  - **mode 0:** `SetFrequency(ftol((R+0x1C - R+0x20)*ratio + R+0x20))` Hz;
  - mode 1: the same value divided by `R+0x24`.

  This is the engine-sound pitch path (`Vehicle_UpdateEngineSound`: ratio = `cls+0x2b4*level`).
  The mode-0 formula was previously recorded as "not established".
- **Adjust and replay.** `SoundVoice_AdjustAndReplay` `0x004a0490` (mode 0 → `_DS`) and
  `SoundVoice_AdjustAndReplay_A3D` `0x004a0380` (applies only positive deltas).
- **Snapshot and list operations:**
  - `SoundVoice_Snapshot` `0x004a0300`;
  - `Sound_BuildPlayingVoiceList` `0x0049fff0` (a heap `std::list` of every playing voice);
  - `SoundList_StopPlayingVoices` `0x004a0500`;
  - `SoundList_ApplyMasterVolume` `0x004a0590`;
  - `SoundList_Destroy` `0x004a05f0`;
  - `SoundList_AllocNode` `0x004a07c0`.
- **Mute is a nesting counter**, `Sound_PushPopMute` `0x004a0670`:
  - `[0x0056b3bc]` goes +1 on push and -1 on pop, and the MuteSound setting becomes
    `count > 0`;
  - every live voice is re-applied (`DSBVOLUME_MIN` while muted);
  - the counter is not clamped: an unmatched pop drives it negative.
- **Master volume.** `Sound_ScaleMasterVolume` `0x004a10b0` multiplies it in place with no null
  check.
- **Busy flags.** `FUN_004a1250` `0x004a1250` is a test-and-set on the play result (verified in
  Recoil18). `SoundResource_ClearBusy` `0x004a1270`.
- **Power-up sound.** `Sound_PlayPowerup` `0x0042bf40` lazily loads `snd_powerup`, then plays or
  stops it.
- **Helpers.** `Sound_LoadFloatArg` `0x0049fa00` is a float identity shim. `Value_AssignDword`
  `0x0040c1c0` copies one dword, null-safe.

## Banks and files
- **Bank vector** `[0x0056b294..0x0056b29c]`:
  - `SoundBankVector_StaticInit` `0x004a0810`, `_RegisterStaticDtor` `0x004a0830`,
    `_StaticDtor` `0x004a0840`, `_CrtInitStub` `0x004a0800`;
  - `SoundBankVector_Clear` `0x004a0880`, `_At` `0x004a08d0`, `_Size` `0x004a0900`;
  - `SoundBank_FindByName` `0x004a0920` (case-sensitive).
- **`SoundBank_Load` `0x004a0c40`:**
  1. **Archive pass**, when `[0x004e2234]` is set (`SoundArchive_SetEnabled` `0x004a07f0`): it
     tries `soundsH/M/L.zbd`, starting at the SoundLOD setting and wrapping L → H. The first
     archive that opens is used.
  2. **Loose-file pass** for every descriptor still without a buffer.
  3. **Defect:** a SoundLOD outside 0-2 indexes the name table with the bank pointer.
- **Thunks.** `SoundBank_FindAndLoad` `0x004a0860` and `SoundBank_FindAndReleaseBuffers`
  `0x004a0870`.
- **Bank buffers:** `SoundBank_LoadBuffers` `0x004a0fb0`, `SoundBank_ReleaseAllBuffers`
  `0x004a0e40`, `SoundBank_Destroy` `0x004a0c00`, `SoundBank_FindLoadedDescriptorByName`
  `0x004a0ec0` (stride 0xB8).
- **Sound file records** (0x24 bytes):
  - `NamedRecord_Construct` `0x004a53f0`, `SoundFile_Destroy` `0x004a5440`;
  - `SoundFile_LoadFromDisk` `0x004a5540`, `SoundFile_LoadFromArchive` `0x004a5600`,
    `SoundFile_Unload` `0x004a55c0`;
  - `Wav_ParseRiffChunks` `0x004a5460`: RIFF/WAVE with `cue`, `fmt` and `data` chunks.
- **Buffers.**
  - `SoundBuffer_Create_Dispatch` `0x004a2ea0` picks the backend.
  - **`SoundBuffer_Create_DirectSound` `0x004a3180`:** DSBUFFERDESC flags come bit by bit from
    the descriptor flags:
    - always `CTRLVOLUME`;
    - 3D bit → also `CTRLPAN`;
    - FREQUENCY bit → `CTRLFREQUENCY`;
    - PURGEABLE bit → `LOCSOFTWARE`, else HARDWARE → `STATIC`;
    - bit 8 → `GLOBALFOCUS`.
  - `SoundBuffer_Create_A3D` `0x004a2ec0`.
  - `SoundBuffer_Lock` / `_Unlock` / `_GetPlayPosition` `0x004a34e0/0x004a3590/0x004a3620`.
  - `SoundResource_ReleaseBuffers` `0x004a3690`, `SoundVoiceSet_Destroy` `0x004a3910`.
- **Streams and names:**
  - `SoundStream_CreateDefaultBuffer` `0x004a3850`;
  - `Sound_StartStreamResource` `0x004a5250`: 0x3C-byte cue, state 1, optional position and
    velocity;
  - `Sound_StreamCue_RetireByPredicate` `0x004a51f0` (sets state 4);
  - `SoundStreamCue_MatchResourcePredicate` `0x004a5220`, `Container_MatchPointerPredicate`
    `0x004a51e0`, `SoundStreamCue_Shutdown` `0x004a50a0`;
  - `Sound_NameTable_Find` `0x004a44c0` and `SoundNameTable_MatchNamePredicate` `0x004a44e0`.
- **Errors.** `Sound_ReportA3DError` `0x004a3ef0` and `SoundCD_ReportMCIError` `0x004a3ea0`
  (256-byte buffer).

## CD audio (redbook music) - `zsnd_cd.cpp`
The original plays music from CD audio tracks through MCI.
- **Playback:**
  - `SoundCD_PlayTrack` `0x004a2750` (MCI_PLAY);
  - `SoundCD_Play` `0x004a2600`: mode 5 loops by replaying on the MCI notify. Single-track modes
    stop at the next track boundary.
  - `SoundCD_PlayIfPrepared` `0x004a25e0`;
  - `SoundCD_OnMciNotify` `0x004a26b0` replays in mode 5;
  - `SoundCD_Stop` `0x004a26f0`, `SoundCD_Shutdown` `0x004a24d0` (MCI_CLOSE).
- **Info and volume:** `SoundCD_GetTrackCount` `0x004a2930`, `SoundCD_HasVolumeControl`
  `0x004a27d0`, `SoundCD_GetVolume` `0x004a27f0` (aux mixer).
- **Track list:** `SoundCD_ResetTrackState` `0x004a2490`, and the `CD_TRACKS` list
  (`SoundCDTrackList_StaticInit/RegisterStaticDtor/StaticDtor/CrtInitStub`
  `0x004a2020/0x004a2050/0x004a2060/0x004a2010`).
- **v1 note:** modern PCs rarely have a CD drive. How the remake sources the music tracks (ripped
  from the user's disc) is a planning decision. The **track order and looping rules** above are
  what must be reproduced.

## Not yet established

- Device and backend bring-up (`zSnd_CreateDevice` `0x004a1420`, `0x004a1510`, `0x004a1870`, `0x004a20d0`) — the largest unread block.
- The A3D vtable (slots `+0x38`, `+0x48`, `+0xA0`, `+0xD0` seen) — mode 1 never runs in any trace, so it stays provisional.
- What sets `[0x0056b2a4]` to enable playback after `zSndInit` clears it.
