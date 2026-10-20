# Input: bindings, keyboard, mouse, joystick

**Status:** membership CLOSED, 59/59 CONFIRMED; one callee (`0x004a5bf0`) is delegated (band F),
so the gate is not open yet. Offsets are in the ledger row notes. Tag: `CONFIRMED-BINARY`
unless stated; DirectInput names/codes are `PLATFORM`.

## Binding map (instance `[0x00565ea0]`)
| offset | field |
|---|---|
| +0x04 | command count |
| +0x08 | binding word per command |
| +0x0C | handler value per command |
| +0x10 | command names (80-byte buffers) |
| +0x14 | primary key -> command (2014 entries) |
| +0x1F8C | secondary key -> command (2014) |
| +0x3F04 | joystick button -> command (16) |
| +0x3F44 | mouse button -> command (4) |

Binding word bits: **0-10 primary key, 11-21 secondary key, 22-25 joystick button, 26-27 mouse
button**. Setters (`0x00470b20/b80/bf0/c60`) unbind the command previously on that input and the
input previously on that command. Rebuild (`0x00470820`) walks commands 1..count-1, later
commands win. Copy ctor names use `strncpy` 80 (may be unterminated); calloc unchecked.
`InputMgr_*` at `0x004716d0..0x00471840` are wrappers that load the instance into ECX.

## Keyboard
- `Input_Keyboard_InitialPollAndClear` `0x0046f450`: GetDeviceData (0x80 entries); INPUTLOST ->
  Acquire; other errors -> `DInput_ReportError`; clears the key state table.
- Per-key handler slots `[0x00561cd8+key*8]`, first registration wins (-1 if taken).
- Key names `Input_FormatKeyName` `0x00470f80`: `Ctrl-` (0x200), `Alt-` (0x100), `Shift-` (0x400) +
  name. Remaining-length bookkeeping subtracts the whole buffer length after each append (defect).

## Mouse — `Input_Mouse_UpdateCursorPosition` `0x004704f0` (disassembly read)
delta = ftol(raw * **1.3**) per axis; cursor += delta, clamped to [0, w-1] / [0, h-1] unless
`[0x00561cac]`; normalised pos = (cursor - origin) * scale; normalised delta = delta * scale.

## Joystick
- Acquire refcount word `[0x00561cbe]`; first acquire resets state.
- `Joystick_EnableAndSetRanges` `0x0042e170`: axis limits ±1000 (three axes), -10000/10000, and
  values 2000/3000/1500/2000 into `[0x004f3350..0x004f339c]`.
- Per axis: SetProperty RANGE, on failure read back; centre = (min+max)*0.5, scale = 2/(max-min)
  (floats, bytes read); dead zone per axis; axes 3/4 only when `[0x00565e14]` > 2 / > 3.

## Command table (`[0x004f3ae4..]`)
0x14-byte entries: CString name + dword vector; vectors grow by max(size,1).

## Module lifetime (`zIn`)
- **`zInInit` `0x00471b50` (hwnd, hinst):**
  - `DirectInputCreateA(hinst, 0x500, &[0x00561cb0])`;
  - a failure reports 0x93 and returns -1;
  - it is idempotent, guarded by `[0x00561cb4]`.
- **`zInShutdown` `0x00471c10`** shuts down, in order:
  1. the joystick (`Input_Joystick_ShutdownDevice` `0x00472280`);
  2. the keyboard (`Input_Keyboard_ShutdownDevice` `0x0046f420`);
  3. the mouse (`Input_Mouse_ShutdownDevice` `0x00470360`);
  4. then it releases DirectInput.
- **Bindings object.**
  - `Input_CreateBindings` `0x004710a0` creates it (0x3f54 bytes, `InputMap_CopyCtor`
    `0x004706c0`) at `[0x00565ea0]`. It then allocates the tables (`Bindings_Alloc`
    `0x004708f0`: the key words live in a registered settings node) and fills the name tables:
    - DirectInput `DIK_*` scancode names (`Input_InitKeyNameTable` `0x00471120`, base
      `0x00565ebc`);
    - "Button 1".."Button 8" (`Input_InitJoystickButtonNames` `0x004715e0`);
    - "Left" / "Right" / "Middle" (`Input_InitMouseButtonNames` `0x00471640`).
  - `Input_DestroyBindings` `0x00471660` frees it (`Bindings_Free` `0x00470960`,
    `InputMap_FreeArrays` `0x004707a0`).
- **Input state.** `StaticInit_00561cb0` `0x004719f0` / `StaticInit_RegisterAtexit_00471a10`
  `0x00471a00` construct it (`InputState_Ctor` `0x00471ab0`: `+0x41f4 = 8`), and
  `InputState_ClearLists` `0x00471a20` empties it.
- **Device refcounts** (first acquire resets that device):
  - keyboard: `Input_Keyboard_AcquireRef` `0x00471d20`, count `[0x00561cbc]`;
  - joystick: `Joystick_AcquireCount` / `Joystick_ReleaseCount` / `Joystick_GetAcquireCount`
    `0x00471d50/0x00471d80/0x00471dd0`, word `[0x00561cbe]`;
  - mouse: `Input_Mouse_AcquireRef` `0x00471da0`, `[0x00561cc0]`.

  `Input_PollAllDevices` `0x00471de0` polls every enabled device whose count is non-zero.
- **Suspend flag.** `Input_SetJoystickFlag2` `0x00471d10` sets bit 1 of `[0x00561cb8]`;
  `Input_ReleaseKeyboardSuspend` `0x00471cd0` clears it after a keyboard re-poll.

## Binding operations
- **`Binding_Pack` `0x00470a10`** builds the binding word:
  `((((mouse&3)<<4 | joy&0xf) << 11 | key2&0x7ff) << 11) | key1&0x7ff`. It is read back by
  `Input_GetPrimaryKeyBinding` / `Secondary` / `Joystick` / `Mouse` `0x00470a40/a60/a80/aa0`.
- **Reverse lookups:**
  - `InputMap_CommandForPrimaryKey` `0x00470ac0` (`+0x14`);
  - `InputMap_CommandForSecondaryKey` `0x00470ad0` (`+0x1F8C`);
  - `InputMap_CommandForKey` `0x00470ae0` (primary, then secondary);
  - `InputMap_CommandForJoystickButton` `0x00470b00` (`+0x3F04`);
  - `InputMap_CommandForMouseButton` `0x00470b10` (`+0x3F44`).
- **Setters:** `InputMap_SetSecondaryKeyBinding` `0x00470b80`, `SetJoystickButtonBinding`
  `0x00470bf0` and `SetMouseButtonBinding` `0x00470c60`, all shaped like the primary setter
  `0x00470b20`. `InputMap_SetCommand` `0x00470cd0` sets the name (strncpy 0x4F) plus all four
  inputs.
- **Resetting.** `Bindings_ResetAll` `0x004709d0` sets every action to `Binding_Pack(0,0)` and
  clears its handler. `Bindings_Forward_004716c0` does the same on the global instance.
- **Handlers.** `InputMap_SetCommandHandler` `0x00470df0` refuses a command with no key. It
  stores the handler and registers it on each bound key (`Input_Keyboard_SetKeyHandler`).
- **Action state.** `Input_QueryActionState` `0x00470eb0` ORs, for one command:
  - the primary and secondary key edges (`Input_Keyboard_ConsumeKeyEdge`);
  - the joystick button edge;
  - the mouse button edge.
- **Button dispatch.** `Input_Mouse_DispatchButtonEdgeHandlers` `0x00470d40` (buttons 1-3) and
  `Input_Joystick_DispatchButtonHandlers` `0x00470db0` (buttons 1-10) call the handler of the
  command bound to each newly pressed button.
- **Names.** `InputMap_CopyCommandName` `0x00470f50`, `Input_CopyKeyName_5662bc` / `_5662fc`
  `0x00471040/0x00471070` (joystick / mouse name tables; a null entry yields `""`).
- **Full names of the getters and wrappers** (one line each in the reference):
  - `Input_GetSecondaryKeyBinding` `0x00470a60`, `Input_GetJoystickButtonBinding` `0x00470a80`,
    `Input_GetMouseButtonBinding` `0x00470aa0`;
  - `InputMgr_GetJoystickButtonBinding` `0x004716f0`;
  - `InputMgr_CommandForSecondaryKey` `0x00471720`, `InputMgr_CommandForJoystickButton`
    `0x00471730`, `InputMgr_CommandForMouseButton` `0x00471740`;
  - `InputMgr_SetSecondaryKeyBinding` `0x00471760`, `InputMgr_SetJoystickButtonBinding`
    `0x00471770`.
- **`InputMgr_*` wrappers** load `[0x00565ea0]` into ECX:
  - lookups `CommandForPrimaryKey` / `SecondaryKey` / `JoystickButton` / `MouseButton`
    `0x00471710/20/30/40`;
  - setters `SetPrimaryKeyBinding` / `SecondaryKey` / `JoystickButton` / `MouseButton`
    `0x00471750/60/70/80`;
  - getters `0x004716e0/f0/0x00471700`;
  - calls `0x00471790` (SetCommand), `0x004717e0` (CopyCommandName), `0x00471800` (FormatKeyName)
    and `0x00471820`.
- **Rebinding in the commands screen** (disassembly re-read 2026-09-24):
  - `CommandsDialog_OnCaptureKey` `0x0040b460`:
    - Escape (scancode 1) cancels.
    - If the pressed key is not in the `+0x1F8C` table but is in `+0x14`, it takes it off there
      (`SetPrimaryKeyBinding(key, 0)`).
    - It then binds it with `SetSecondaryKeyBinding(key, selectedCommand)`.
    - It refreshes the table, leaves capture mode (`+0xcdfc = 0`) and resets input.
  - `CommandsDialog_OnCaptureJoystickButton` `0x0040b4e0` and
    `CommandsDialog_OnCaptureMouseButton` `0x0040b560` do the same for a joystick or mouse
    button (no Escape test).
  - **Correction:** these three were named `OnResetSetA/B/C`, and several wrappers were named
    `Bindings_Query_*` / `Bindings_GetKey1/2`; the byte reads above replace those names.

## Keyboard (DirectInput)
- **`Input_Keyboard_CreateDevice` `0x0046f300`:**
  - `GUID_SysKeyboard`, cooperative level 10 (`NONEXCLUSIVE|FOREGROUND`, `PLATFORM`);
  - it sets a buffer size and allocates the event buffer.
  - `Input_Keyboard_ShutdownDevice` `0x0046f420` unacquires, releases and frees.
- **`Input_Keyboard_PollAndTranslate` `0x0046f690`:**
  - reads up to 0x80 buffered events;
  - `DIERR_INPUTLOST` (`0x8007001e`) re-acquires;
  - it tracks modifiers Ctrl (0x1d/0x9d → 0x200), Shift (0x2a/0x36 → 0x400) and Alt → 0x100.
- **Per-key state** `[0x00561cd4+key*8]`, `Input_Keyboard_ConsumeKeyEdge` `0x0046f980`:
  pressed (1) becomes held (2) once read; released (bit 4) reads as 0.
- **Handler slots.**
  - `Input_Keyboard_SetKeyHandler` `0x0046f9b0`: first registration wins, and there is no
    bounds check.
  - `Input_ClearBindingSlot` `0x0046f9d0` clears a slot.
  - `Input_Keyboard_ClearHeldTable` `0x0046f9f0` clears the handler slots.
- **Blocking wait.** `Input_Keyboard_WaitForKeyPress` `0x0046fa10` reads one event at a time; it
  is used by the key-binding capture and by the AVI skip.
- **Text entry.** `Input_Keyboard_ScancodeToAscii` `0x0046fba0` uses the table built lazily by
  `Input_InitKeyNameTable` `0x0046fd20` (`0x00561848`). Shift upper-cases a-z and maps digits
  and punctuation to `!@#$%^&*()_+{}:"~|<>?`.

## Mouse (DirectInput)
- **`Input_Mouse_CreateDevice` `0x004701f0`:** `GUID_SysMouse`, re-queried to a newer interface
  (`[0x00565e78]`), `c_dfDIMouse`.
  - `Input_Mouse_SetAcquired` `0x00470310` toggles acquisition.
  - `Input_Mouse_ShutdownDevice` `0x00470360`.
  - `Input_GetMouseAvailable` `0x00470190` returns `[0x00565e74]`.
- **Polling.** `Input_Mouse_Poll` `0x004703b0` installs `Input_Mouse_PollAndTranslate`
  `0x004703c0` as `[0x004e0900]`. That function:
  - reads `GetDeviceState` (0x10 bytes);
  - keeps the previous state;
  - stores deltas at `0x00561c90/94`;
  - then updates the cursor (`Input_Mouse_UpdateCursorPosition`, factor 1.3).
- **Button edges.** `Input_GetMouseButtonEdgeState` `0x004702e0` returns 1 pressed, 2 held,
  4 released, 0 up. `Input_GetMouseButtonEdgeTable` `0x004703a0` returns the state block
  `0x00561c80`. `CopyStruct_00561c80` `0x004705f0` copies 11 dwords of it.
- **Cursor and screen:**
  - `Input_Mouse_SetScreenSize` `0x004701a0`: centre = size/2, inverse half size.
  - `Input_Mouse_UpdateFromClientRect` `0x00470060`.
  - `Input_Mouse_SetNormalizedPos` `0x004700a0`: clamps to [-1, 1], then
    `cursor = ftol(v*scale) + centre`.
  - `Input_Mouse_SetCursorToCenter` `0x00470020` (`ClientToScreen` + `SetCursorPos`).
  - `Input_Mouse_RecenterCursor` `0x00470150` recentres both axes and zeroes the normalised
    position; `Input_Mouse_Recenter` `0x00470180` recentres x only.
- **Other:**
  - `Mouse_ResetDeltas` `0x00470610`;
  - `Input_SetMouseMode_Exchange` `0x00470670` swaps `[0x004e08fc]`;
  - `Input_Mouse_WaitForClick` `0x00470680` returns the first held button 1-3.

## Joystick (DirectInput)
- **`Input_Joystick_CreateDevice` `0x00471e40`:**
  - `EnumDevices(JOYSTICK, ATTACHEDONLY)` → `[0x00565bd0]`, `c_dfDIJoystick`.
  - `Input_Joystick_Acquire` `0x00471fb0`.
  - `Joystick_IsPresent` `0x004722b0` returns `[0x00565bcc]`.
  - `Input_Joystick_ShutdownDevice` `0x00472280`.
- **`Input_Joystick_Poll` `0x004722c0`:**
  - `Poll` + `GetDeviceState` (0x110 bytes → `0x00566310`);
  - it zeroes axes 3 and 4 when the device has fewer axes;
  - `INPUTLOST` re-acquires.
- **Button edges.** `Input_GetJoystickButtonEdgeState` `0x004723a0` uses the same 1/2/4/0 code
  as the mouse. `Joystick_ResetState` `0x00472410` clears buttons 1-10 and sets the axes to
  0xFFFF. `Input_WaitJoystickButton` `0x004723d0` blocks for a press.
- **Properties** (HRESULTs are `PLATFORM`): `Joystick_SetDeadzone` `0x004721a0`,
  `Joystick_SetAxisRange` `0x004721e0`, `Joystick_GetAxisRange` `0x00472230`.
- **Other:** `Input_Joystick_AcquireOk` `0x00472450`, `Input_Joystick_GetField565e18`
  `0x00472480`, `GetGlobalStruct_00565bd4` `0x00472390`.

## Force feedback (DirectInput effects)
- **Availability.** `ForceFeedback_IsAvailable` `0x0042fa80` requires both the setting and a
  force-feedback device.
- **Effect objects:**
  - periodic (`ForceFeedback_CreatePeriodicEffect` `0x0042ffa0`);
  - constant (`ForceFeedback_CreateConstantEffect` `0x00430070`);
  - ramp (`ForceFeedback_CreateRampEffect` `0x00430100`).

  All are DIEFFECT size 0x34, flags 0x22, duration 10000 (the ramp also uses 20000).
- **Playing effects.**
  - `ForceFeedback_PlayConstant` `0x0042fac0` stops the effect, sets the magnitude, and starts it
    (weapon fire: `w+0x40*0.015151516`).
  - `ForceFeedback_PlayDirectional` `0x0042fb50` and `ForceFeedback_PlayHitDirectional`
    `0x0042fc90` set the direction as the hit angle relative to the player's camera facing
    (`+0x520/+0x528`). Hits pass `amount*0.05`.
- **v1 note:** force feedback is `PLATFORM` behaviour and does not affect the simulation.

## Command table (config-driven)
- **Adding.** `CommandTable_AddEntry` `0x0042a070` appends to `[0x004f3ae4..0x004f3aec]`, and
  `CommandTableEntry_PushValue` `0x0042a2c0` adds a value to an entry.
- **Freeing.** `CommandTable_Clear` `0x00429f80` and `CommandTableEntry_Dtor` `0x0042a000`.
- **Reading:** `CommandTable_Count` `0x0042a480`, `CommandTable_GetFirstField` `0x0042a4a0`,
  `CommandTable_EntrySubCount` `0x0042a4b0`, `CommandTable_EntrySubItem` `0x0042a4d0`.
- **Slots.** `CommandSlot_Bind` `0x0042a500` binds a command slot `[0x004f3af0+i*4]` through
  `InputMgr_Call_00470cd0` (SetCommand); `CommandSlot_Call_004a5bf0` and `..._Plus1`
  `0x0042a4e0/0x0042a4f0` look up its message text.
- **Helper.** `Array_FillN` `0x0040c190` (the null check sits inside the loop, as found).

## Still open
- Which game command indices map to which actions (dialog config), and `0x004a5bf0`.
- Which axis offset each `Joystick_ApplyAxisRanges` call passes (ECX hidden in decompile).
