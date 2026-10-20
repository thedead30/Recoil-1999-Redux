# Lockstep stream format (L4)

`STAGE2.md` section 4, `TODO_STAGE2.md` P0.7. A lockstep stream holds, frame by frame, everything the
ORIGINAL game received from outside its own code. Replayed into the headless remake at the same
seams, it must reproduce the original's state frame by frame.

## The seams: every point where the outside world enters the game

Each seam is a layer-D call site. Addresses were found by listing the calls in the poll functions
(`CONFIRMED-BINARY`, Ghidra listing 2026-09-25).

| seam | call site | what is recorded (after the call returns) |
|---|---|---|
| **timer** | `0x004a56d3` in the frame timer `0x004a56d0`: `CALL 0x004a59d0` (GetTickCount) | EAX: the millisecond tick count. Everything time-related (dt, clock, clamp to [0.005, 0.125]) is computed by game code from this value |
| **keyboard** | `0x0046f6c0` in `Input_Keyboard_PollAndTranslate` `0x0046f690`: `CALL [ECX+0x28]` (IDirectInputDevice::GetDeviceData) | HRESULT (EAX); the in/out element count; the returned `DIDEVICEOBJECTDATA` records (0x10 bytes each, up to 0x80) |
| **mouse** | `0x00470419` in `Input_Mouse_PollAndTranslate` `0x004703c0`: `CALL [ECX+0x24]` (GetDeviceState, 0x10 bytes); preceded by `Poll` `[EAX+0x64]` at `0x0047040c` | HRESULT; the 0x10-byte `DIMOUSESTATE` |
| **joystick** | `0x004722ef` in `Input_Joystick_Poll` `0x004722c0`: `CALL [ECX+0x24]` (GetDeviceState, 0x110 bytes); `Poll` at `0x004722df` | HRESULT; the 0x110-byte state |
| **time (random seed)** | `0x0045594e` in `0x004558f0` and `0x0045e163` in `0x0045e100`: `CALL [0x004cc48c]` (time), result passed to `srand` | EAX: the time value (`04_spec/formulas/crt_rand.md`) |

Not seams (they are the game's own code and are replayed, not injected):
- the CRT `rand()` state;
- dt and the clock;
- edge states (pressed/held/released);
- the cursor position.

These are **compared**, not fed in.

## Record layout (JSON lines, one object per frame)

```json
{"frame": 0,
 "timer": [123456],
 "kbd":   [{"hr": 0, "events": [["0000001e", "00000080", "0001e240", "00000007"]]}],
 "mouse": [{"hr": 0, "state": "0000000000000000000000000000000000"}],
 "joy":   [],
 "time":  [],
 "check": {"0x004f3ac4": "3cf5c28f", "player_inst+0x3ec": ["...", "...", "..."]}}
```

- **`frame`** is the index of the frame-timer call; a frame is the span between two `timer` reads.
- **Seam lists** keep every call of that seam inside the frame, in call order. The replay pops them
  in the same order; a mismatch in count or order is a desync by itself.
- **`check`** holds the state sampled at the end of the frame from the original (addresses from
  the spec: dt `0x004f3ac4`, player instance fields from `04_spec/structs/VehicleInstance.md`...),
  compared field by field with the remake's layout-faithful structs.

## Producing a stream

A stream is extracted from a TTD trace (`tools/trace_vectors` technique: breakpoints on the call
sites above, read the output buffers at the next instruction). A trace must contain real play
(**KG-11**) for the stream to test gameplay.
