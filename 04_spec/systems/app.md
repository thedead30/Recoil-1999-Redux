# App - process start, engine bring-up, main loop, screens

Narrative spec for subsystem `app` (membership CLOSED, all members CONFIRMED). Per-function
detail is in [`../reference/app.md`](../reference/app.md). Timing is in
[`frame_timing.md`](frame_timing.md), and the screen stack in [`menus.md`](menus.md)
(`ScreenManager_*`).

**v1 scope.** Everything here is single player. The Windows menu bar, the About box and the
online and multiplayer dialogs (section 7) are **out of scope** for v1, by the user's decision
of 2026-09-24. They are listed so the spec is complete. Tags: `CONFIRMED-BINARY` unless marked;
Win32, MFC and COM behaviour is `PLATFORM`.

## 1. Process start
1. `entry` `0x004c6140` is the CRT start: SEH frame, `Crt_SetFpuControl`, `__getmainargs`,
   static initialisers (`initterm` ×2), then `WinMain_Thunk` `0x004c81c0` →
   `AfxWinMain`.
2. **`Crt_SetFpuControl` `0x004c6350`: `_controlfp(0x10000, 0x30000)`**, bytes
   `68 00000300 68 00000100 e8..` re-read 2026-09-24. This sets the x87 **precision control to
   53 bits** (`_PC_53`, mask `_MCW_PC`) before any game code runs. So the game's float maths
   rounds intermediates to double precision, not 80-bit extended. This is load-bearing for the
   numeric-fidelity decision in planning; see "Open".
3. **Static application object.**
   - `StaticInit_004f3ca8` `0x0042de30` builds it at `0x004f3ca8` (`MainApp_Ctor` `0x0042dfa0`)
     and registers its destructor (`StaticInit_RegisterAtexit_0042de50` `0x0042de40`).
   - `MainApp_Ctor` runs `RecoilApp_Constructor` `0x00442c70` (a `CWinApp`) and builds three
     sequence members (`SeqScreenA_Ctor` `0x0042eb70`, a Seq at +0x68, `SeqScreenB_Ctor`
     `0x0042ed30`) plus `FrameState_Ctor_004d0b90` `0x0042eea0`.
   - `Afx_SetMbcsState` `0x004c81d8` is MFC start-up.
4. **`RecoilApp_BootstrapAndCDCheck` `0x0042e5da`**: every branch was re-read from bytes on
   2026-09-24 (`0x0042e5da..0x0042e91a`). Ghidra enters this body mid-function, with
   `ESI = this` and `EDI = 0`.
   1. Set `[0x004f3ed0] = 1`, then call `RecoilApp_InitInstance` `0x004429d0` (3D controls, main
      window, engine registration, show). Then `[this+0x148] = 0` and
      `[this+0xc4] = this+0x1a0`.
   2. **`Messages_LoadDll("MESSAGES.DLL")`** `0x004a5ad0`. If it fails:
      - `sprintf "Exit at %s:%d\n"` with `D:\Proj\Battlesport\RecoilApp.cpp` and line 392
        (0x188), then `OutputDebugStringA`;
      - `FormatMessageA(0x1100, 0, GetLastError(), 0x400, ...)`, append `"\n\n"` and
        `"MESSAGES.DLL"`, then `LocalFree`;
      - `Render_FlipToGDISurface`, `MessageBeep(0x40)`, `MessageBoxA(0, text, "\x8e", 0x40)`,
        `ExitProcess(0)`.
   3. **CD check loop.**
      - Load the text of message 0x83 (`GetMessageByID`, 0x100 bytes).
      - Call `CDCheck_FindDiscDrive(5, "video\intro_01.avi", 0)`; non-zero passes.
      - Otherwise call `MessageBeep(0x30)` and
        `MessageBoxA([0x004f3eec], msg 0x83, caption = msg 0x901, 0x31 = MB_OKCANCEL|MB_ICONEXCLAMATION)`.
      - **OK** retries the check. **Cancel** calls `ExitProcess(0)`.
   4. **DirectX version.** `RecoilApp_ProbeDirectXCapabilities` returns a version.
      - If the version is below `0x600`: `MessageBeep(0x10)`, then a box with text msg 0x16,
        caption msg 0x14 and type 0x10, then `ExitProcess(0)`.
      - This is `PLATFORM`; the remake drops it.
   5. **Core set-up, in this order:**
      - `Stub_Ret`, `SaveSystem_Create`, `Container_InitNodePool(0x200)`;
      - `StringSet_AddSemicolonTokens(0, "zbd")`, `Archive_InitRegistry("..\data\common\zrdr")`;
      - `strncpy` msg 0x900 and msg 0x901 (0x100 bytes each);
      - `Settings_InitPathsAndAutoLoad(.., .., Version_GetString())`;
      - `Input_CreateBindings(0x2f)`.
   6. **User settings.** Call `RecoilApp_LoadUserSettings`.
      - If it fails: `Zar_OpenAndRegisterArchive("zbd\zrdr.zbd", 1)` and **try again**.
        **Correction:** the old note had the archive opened unconditionally. It is opened only
        on this fallback.
      - If the second attempt also fails: `MessageBeep(0x10)`,
        `MessageBoxA(hwnd, msg 0x1e, caption msg 0x901, 0x10)`, `ExitProcess(0)`.
   7. `Settings_ApplyVideoModePreset(Setting_Get_DisplayMode())`, then
      `MainWnd_EnableVideoModes` and `MainWnd_InitDisplayModeList` on `RecoilApp_GetField20()`.
      Return 1.
5. **Single instance.** `App_ActivateExistingInstance` `0x0042e990`: if a `"RecoilClass"` window
   exists, it is brought to the front and the new instance exits.
6. **Logs.** `App_RedirectStdioToLogs` `0x004a5780` reopens stderr and stdout into `<name>.err`
   files, falling back to `%TEMP%\gamez.err`.

## 2. Engine bring-up and shutdown
- **`RecoilApp_PostSubsystemInit` `0x0042e220` (hwnd):**
  1. `Engine_InitSubsystems`;
  2. turret init;
  3. sound device, API mode and the sound API setting;
  4. `RecoilApp_InitVideoAndHSEDevice`;
  5. `zInInit`;
  6. display, joystick and control setup.

  If video fails it shows an error box and returns 0.
- **`Engine_InitSubsystems` `0x00442a50`** runs, in order, logging `"<name>Init: %s"`
  PASSED/FAILED:
  1. container pool;
  2. archive registry;
  3. `gModInit`;
  4. `gClsInit` (`EmptyStub_004a75e0` `0x004a75e0`, which just returns 0);
  5. `zEffInit`;
  6. `zRndrInit`;
  7. `zSndInit` (success = non-zero, the others succeed on 0);
  8. `zUtlInit` (the same empty stub);
  9. `zWepInit`;
  10. `zImgInit`;
  11. the mouse mode;
  12. `zInInit`.

  Then `Timer_Reset` `0x004a5670` (zeroes dt `0x0056b424` and t `0x0056b428`; base = tick×0.001)
  and `Global_Set_0056b564` `0x004a59a0`.
- **`RecoilApp_InitVideoAndHSEDevice` `0x0042e330`:**
  - opens video with the display mode, fullscreen and HW-API settings;
  - in software mode it also opens HSE (the software renderer, `SwRender_Init`);
  - errors print `"Error opening video ... ABORTING"` or `"Error opening HSE ... ABORTING"`;
  - on success it sets up the viewport, resets zRndr, and clears and flips twice.
- **Probe.** `RecoilApp_ProbeDirectXCapabilities` `0x0040c370` checks `DINPUT.DLL` and `DDRAW.DLL`
  with `LoadLibrary` / `GetProcAddress` (`PLATFORM`).
- **Shutdown.**
  - `App_ShutdownGame` `0x0042e430`: CD audio stop, turret, declient, HSE (software), pickup
    images, network, `Engine_ShutdownSubsystems`, video close.
  - `Engine_ShutdownSubsystems` `0x00442bc0` runs, in order: zIn, zImg, zUtl, zRndr, zEff,
    zWep, gCls, gMod, zSnd, archives, container pool.
  - `App_ExitProcess` `0x004a5980` (`_fcloseall`, `ExitProcess`) and `App_FreeLoadedLibrary`
    `0x004a5b00`.

## 3. Main loop
**`RecoilApp_Run` `0x00442d00`** sets thread priority 2, then loops:
1. **Messages.** Pump Windows messages; a quit calls `ExitInstance` and returns.
2. **Network.** `Net_ReceiveMessages`, which is a no-op path in single player.
3. **Screen tick.** `cur = ScreenManager_GetCurrent()`:
   - if the app is inactive (`[+0x34]`), wait for a message;
   - else if no screen command is queued, call `cur->vfunc+0x10()` (the screen's frame);
   - non-zero ends the game (`PostQuitMessage`).
4. **Screen commands.** One queued command is applied per loop, from a block deque of 0x1000-byte
   blocks. The stack depth is clamped to 0..15.

   | type | action |
   |---|---|
   | 1 pop | `cur->vfunc+0x18` (exit), depth -1, `top->vfunc+0x20(arg)` |
   | 2 push | `top->vfunc+0x1c(arg)`; `new->vfunc+0xc()` ok → depth +1, `stack[depth] = new` |
   | 3 replace | `cur->vfunc+0x18`; `new->vfunc+0xc()` ok → replace, else re-enter `cur` |

**`RecoilApp_OnIdleStep` `0x00442c10`:** when the main window is not active it idles.
Otherwise it marks the app active and pushes the queued first screen.

**`App_RenderFrameAndPresent` `0x0042f280`** (arg `present`, ret 4). This is the gameplay frame.
Every path was re-read from bytes on 2026-09-24 (`0x0042f280..0x0042f5d3`).

**Correction.** The earlier note said a non-zero argument means "reset only". It does not. No
branch is taken on the argument at entry. Its only uses are the final `zVideo_PresentFrame`
call and the pixel-doubled blit hook (below). "Pixel-doubled" in this section means
`Setting_Get_004e5d6c` is set.

1. **Always:**
   - frame timer `0x004a56d0`;
   - if `[0x004f3a90]` is set and `Input_Keyboard_WaitForKeyPress(0)` fires: clear it and
     call `Wrap_0045d570_Arg1(old, 0)`;
   - `Input_PollAllDevices(1)`;
   - `[this+0xc/8/4] = ViewportRect / ScreenRect / [[0x004e5d88]]`;
   - `NodeList_RunAll`.
2. If **`[0x004e5dec]` is set, return 1**. Nothing is drawn.
3. **Before the scene:**
   - `Sound_UpdateFrame(0)`;
   - if `[0x004f3768]` and `[0x004f36c0]` are set:
     `Object3D_SetPosition([0x004f36c0], Anim_ResolveObjectWorldPosition([0x004f36bc]))`.
4. **Surface set-up:**
   - `c = Hud_ConsumeCounter_004e61bc()`, `old = zVideo_SetD3DDeviceCreated(c | IsD3DDeviceCreated())`;
   - `rect = c ? [this+4] : [this+0xc]`;
   - hardware: `0x004a6760(rect, [this+4])`; software: `zVideo_CallSurfaceSlot3C8(rect)`;
   - restore the flag with `zVideo_SetD3DDeviceCreated(old)`.
5. **Framebuffer:**
   - `zRndr_SetFramebuffer(base, [this+0xc])`. The base comes from the zVideo globals
     `0x00632208 / 0x00632210` when pixel-doubled, otherwise `0x00632228 / 0x00632230`;
     `Settings_GetDisplayRect_20` is applied between the two getters.
   - `Render_DispatchCameraList` (**world render**).
   - `GlobalCompositePanel_SetSlot(0, [this+0xc])`.
6. **HUD rect:**
   - `Hud_GetRect_004edb58` → `0x004f3c98`. When pixel-doubled, halve all four values
     (signed, toward zero).
   - If `rect.bottom > viewport.bottom`: raise `rect.top` to the viewport bottom if it is
     lower, and `slot1 = &rect`. Otherwise `slot1 = 0`.
   - `GlobalCompositePanel_SetSlot(1, slot1)`.
7. `k = Input_Keyboard_ConsumeKeyEdge(1) & 3`.
8. **Hardware path:**
   - `PointQueue_Clear`, `Mission_TickAndCheckObjectiveCompletion`, `Hud_UpdateTargetMarkers`,
     `zVideo_NotifyScreenResize`, `Manager_0056bd58_Tick(dt)`;
   - if `k`: `Slot3C0_Back`, **return 1**;
   - otherwise: `Render_SetViewportSizeFromRect([this+4])`, `Hud_Tick`, network extras,
     `Slot3C0_Back`.
9. **Software, pixel-doubled:**
   - `zVideo_NotifyScreenResize`, `Manager_0056bd58_Tick(dt)`, `Slot3C0_Back`;
   - if `present`: hook `[0x0056bbfc]([this+0xc], [this+8])`;
   - `zVideo_NotifyScreenResizeFromSurface`;
   - if `k`: `Slot3C0_Front`, `zVideo_PresentFrame(0, 0, 0, 1)`, **return 1**;
   - otherwise: `Mission_TickAndCheckObjectiveCompletion`, `Render_SetViewportSizeFromRect([this+4])`,
     **`PointQueue_FlushAll(2.0)`**, `Hud_UpdateTargetMarkers`, `Hud_Tick`, network extras,
     `Slot3C0_Front`.
10. **Software, normal:** the same as step 9 without the Back blit or the hook, and with
    **`PointQueue_FlushAll(1.0)`**.
11. **End:** if `present`: `zVideo_PresentFrame([this+4], [this+4], 0, 0)`. Return 0.

**Gameplay-relevant.**
- `Mission_TickAndCheckObjectiveCompletion` and `Hud_Tick` run **inside the render frame** and
  are skipped on a frame where `k != 0` (the key edge) or `[0x004e5dec]` is set. The remake must
  keep them in this function's order.
- On hardware, point sprites are cleared rather than drawn by this path.
## 4. Game session
- **`App_EnterGameplay` `0x0042eed0`**: every step was re-read from bytes on 2026-09-24
  (`0x0042eed0..0x0042f270`). `ESI = this`.
  1. **Hardware card only:** `SystemParametersInfoA(0x61, 1, &old, 0)`, i.e. the Win9x
     "screen saver running" trick that blocks task switching. `PLATFORM`; dropped in the remake.
  2. `Timer_Reset`, then **dt = 0.1** (`0x3dcccccd`). The network-only `NetExitDialog_InitOnGameLoad`
     runs only when the network flag is set.
  3. **Effects level.** `level = Setting_Get_EffectsLevel()`. On the software renderer, a level
     of 0 is forced to 1. Then `Settings_ApplyEffectsLevel(level)`.
  4. **Loading:**
     - `Hud_Init("hud.zrd")` and `Loading_InitCheckpointTable`;
     - checkpoint `"Loading common sounds"`, then `SoundBank_FindAndLoad("COMMON")`;
     - `Briefing_StartLoaderThread`;
     - checkpoint lines from message ids 3 (with `Version_GetString`), 5 (with
       `zVideo_GetDriverName`) and 6 (with `zVideo_GetDeviceGuidOrName`), then the text of
       message 0x10d.
  5. **Mission:**
     - `Mission_LoadObjectivesArray("objectives.zrd")`, `Player_RegisterSaveHandlers`,
       `Loader_StartIfReady`;
     - `Mission_Load`: **on failure, return at once**;
     - `Mission_InitGameplay`, `Loader_WaitForThread(1)`, `Hud_ApplyTypeChange(1)`.
  6. **Start animation.** If a pending save path `[this+0x14]` is set:
     - step the float at `[0x004f3df8]`: if it is > 0.0, subtract 5.0; otherwise set it to
       5.0 and call `Settings_ApplyMuteSound(1)`;
     - `SaveGame_LoadFromFile(path)`, `free(path)`, `[this+0x14] = 0`;
     - animation `LOAD_GAME_START`.
     Otherwise the animation is `NEW_GAME_START`. Either way, call
     `Mission_StopConfiguredAnims("StartAnims.zrd", name)`.
  7. **Rects and input:**
     - `[this+0xc] = Setting_Get_ViewportRect()`, `[this+8] = Setting_Get_ScreenRect()`,
       `[this+4] = [[0x004e5d88]]`;
     - `Input_Keyboard_InitialPollAndClear`, `Input_Mouse_RecenterCursor`;
     - hardware only: camera calls `0x00449db0` / `0x00449dc0`.
  8. **First frame.** Step the same float with 1.0 (> 0: subtract 1.0; otherwise set 1.0 and
     mute). Then `App_RenderFrameAndPresent(0)`: a full frame with the final present skipped.
     **Correction:** it is not a "reset frame" as previously written.
  9. **Hand-over:**
     - keyboard clear and mouse recentre again;
     - `[0x0056bbd8] = 0`, `[0x004f3df0] = 1`, `zVideo_SetFlag0063212c(1)`,
       `Mission_BeginPlay`;
     - if a mouse is present: `[0x00561c74] = 0`, `Input_Mouse_SetAcquired`;
     - `Input_ResetAll`, `Settings_ApplyGfxFlags(Setting_Get_GfxFlags())`;
     - `Settings_StoreJoystickEnabled(Joystick_EnableAndSetRanges(Setting_Get_004e5d60()))`;
     - control-flag bits 2, 3, 0 and 1 are each re-set from their own getter (bit 3 also
       sets the camera).
  10. **CD music.** If CD audio is on:
      `track = [mission+0xdc] % (SoundCD_GetTrackCount() - 2) + 2`, then
      `SoundCD_PlayIfPrepared(track, 5)`. Tracks 0 and 1 are never picked. The getter is
      named `Mission_GetOutcome`, but `Mission_Load` treats `+0xdc` as the mission id; see
      the open note.
  11. `Hud_ClearMessagePanels`, return 1. The network branch (`Net_IsSessionActive`, which
      also steps the counter by 5.0) is out of scope.
- **Starting a mission.** `App_StartMission` `0x0042e4d0` → `Screen_LoadArchivesThenIdle`
  `0x0042e490` (opens the sound archive when enabled), then loads by file or by index.
- **`Game_HandleStateEvent` `0x0042bb30`**: disassembly re-read 2026-09-24 (caveat resolved). It
  corrects an earlier decompile-based note that misnamed several callees.

  | event | action |
  |---|---|
  | 0 | clear `[0x004f3a90]` if the sender is the one stored |
  | 1 | `[0x004e5cd8]=1`; the camera views the player vehicle |
  | 2 | release the view vehicle (single player); inset off; mouse cursor mode 0; hide both HUD panels |
  | 10 | sync the player transform from its scene object; clear HUD message panels |
  | 11 | single player: player state `+0x60 = 6`; engine sound off |
  | 14 | single player: `[0x004f36b0]=0`, inset off, cursor mode 0, HUD timer resumes (`SetPaused(0)`), global slot 0x18; always release turrets |
  | 15 | single player: `[0x004f36b0]=1`, split-screen HUD type, inset on, cursor mode 1 centred (0.5, 0.5), show the current weapon name for 5 s, HUD timer paused, global slot 0x18. If the mission outcome is 1, `[0x004f0de4]==0` and **no shots fired** `[0x004f311c]==0`: power-up sound. Always walk turret nodes |
  | 16 | zero the player's motion |
  | 17 | teleport the player to a node |
  | 20 | `[0x004f3a90] = sender` |
  | 25 | clear extra lives, then 26 |
  | 26 | lethal damage to the player (`hp + 1.0`) |
  | 27 | 10.0 damage to the player |
  | 99 | advance to the next mission |
  | 911 | try to drop a crate if clear (radius 20.0) |
  | 912 / 913 / 914 | spawn pickup at named node `vwbus` (0x20) / `crbox` (0x24) / `drop` (0x1e) |
  | other | nothing |
- **`Game_EndSequenceTick` `0x0042f5e0`:**
  - on a won game it plays FMV `fmv_zrd` / `GRANDPRIZE` when the end timer `[0x004f3df8]`
    runs out;
  - it fades while the timer is ≤ 1.0;
  - it marks the player `+0x5a0` (invulnerable) for the duration.

## 5. Intro and attract sequences
- **Intro.** `App_StartIntroSequence` `0x0042ea20` sets the video mode, then (unless skip flag
  `[0x004f3df4]`) plays `fmv.zrd` `INTRO`.
- **Attract.** `App_StartAttractSequence` `0x0042ebf0` plays `fmv.zrd` `ATTRACT`, loading it the
  first time.
- **Video rect.** `Screen_ApplyVideoRect` `0x0042eb20` applies the screen rect from the video
  mode.
- **Destructors:**
  - `MainApp_Dtor` `0x0042de60`;
  - `SeqScreen_Dtor_A/B/C` `0x0042df10/0x0042df50/0x0042e070`;
  - `ScreenBase_ResetVtbl` `0x0042df90`;
  - scalar-deleting forms `0x0042e0b0`, `0x0042e0d0`, `0x0042e0f0`, `0x0042ebd0`,
    `0x0042ed90`.

## 6. Windows
- **Main frame window** (`MainWnd_New` `0x004301e0`, `MainWnd_Ctor` `0x00430250`,
  `App_CreateMainWindow` `0x0042e110`):
  - 0x230-byte object, paths `recoil` and `/campaigns`, error log `recoil.err`, menu `MYMENU`;
  - `MainWnd_GetTitle` `0x004306f0`: `RECOIL` or `RECOIL (3Dfx)`;
  - `MainWnd_SetWindowedChrome` `0x00430680`: window style and menu on or off;
  - destructors `MainWnd_Dtor` `0x00430610` and `MainWnd_scalar_deleting_dtor` `0x004305f0`.
- **Menu bar (out of scope for v1):**
  - video modes: `MainWnd_EnableVideoModes` `0x004308c0`, `MainWnd_InitDisplayModeList`
    `0x00431730`, and one handler per preset: `MainWnd_OnVideoMode2` `0x004309b0`,
    `MainWnd_OnVideoMode3` `0x004309d0`, `MainWnd_OnVideoMode4` `0x004309f0`,
    `MainWnd_OnVideoMode5` `0x00430a10`, `MainWnd_OnVideoMode6` `0x00430a30`,
    `MainWnd_OnVideoMode7` `0x00430a50`;
  - driver: `MainWnd_SelectDriverWindowedDefault` `0x00431610`, `MainWnd_SelectDriverAndMode`
    `0x00431680`;
  - toggles: `MainWnd_OnToggleSoundArchive` `0x00431330`, `MainWnd_OnToggleTextureFlag`
    `0x00431380`;
  - `MainWnd_OnAbout` `0x00430c30` (dialog 0x67).
- **Game frame** (window class `gamez`):
  - `GameFrame_New` `0x00443730`, `GameFrame_Ctor` `0x004437d0` (DirectDraw bring-up);
  - `GameFrame_OnPaint` `0x00443900` (StretchBlt to 640×480);
  - `GameFrame_OnDestroy` `0x00443ab0`;
  - destructors: `GameFrame_Dtor` `0x00443830`, `0x00443810`; brush destructors
    `GdiObjA/B` `0x00443b70/0x00443b90/0x00443be0/0x00443c00`.
- **Windowed mode.** `Video_IsWindowedNonGlide` `0x004a59b0` reports windowed mode, except under
  Glide.
- **CD audio.** `App_ForwardMciNotifyToScreen` `0x00443650` passes CD-audio MCI notifications to
  the current screen.

## 7. Online and multiplayer support (out of scope for v1)
- **Account and server combo dialog:**
  - `ComboDialog_RunModal` `0x00441cb0`, `ComboDialog_GetSelection` `0x00441c60`;
  - `ComboDialog_CommitEdit` `0x00442010`, `ComboDialog_OnSelChange` `0x00442080`,
    `ComboDialog_OnEditChanged` `0x00442100`;
  - `RecoilApp_SetString4b8/4bc` `0x00442180/0x004421d0`.
- **Login.** `WolLoginDialog_OnOK` `0x00441f40`.
- **Download or status dialog:**
  - `StatusDialog_Printf` `0x00442270`, `StatusDialog_CreateComObject` `0x004422a0`,
    `StatusDialog_Refresh` `0x004422f0`;
  - `StatusDialog_OnProgress` `0x004426b0` ("Bytes read %d / %d  Time left ...");
  - `App_PromptForEachListItem` `0x00442530`;
  - `Dialog9D_Ctor` `0x00442220`, `Dialog9D_ScalarDeletingDtor` `0x00442240`.
- **COM helpers:**
  - `Atl_InternalQueryInterface` `0x0042db50`;
  - `Com_QueryAndCallSlot14/18` `0x0042dc30/0x0042dcf0`;
  - `DsBuffer_Init` `0x0042dda0`, `ComPtr_Release` `0x0042de00`, `ComPtr_StopAndReset`
    `0x0042faa0`;
  - `RefCounted_Create/Release/Dtor` `0x004425c0/0x00442790/0x004427f0`, `CritSec_Delete`
    `0x00442860`;
  - `EH_Continuation_0044263e` `0x0044263e` (a fragment, not a function).

## 8. Small items
- **Runtime class and application object:** `RecoilApp_GetClassInfoPtr` `0x004428a0`,
  `RecoilApp_Dtor` `0x004428b0`, `RecoilApp_ScalarDeletingDtor` `0x004429b0`,
  `RecoilApp_GetField20` `0x00442c00` (main window).
- **Stubs and thunks:** `Stub_Ret4` `0x0042eecd`, `OperatorDelete_Thunk_0042f890` `0x0042f890`,
  `type_info_scalar_deleting_dtor` `0x004c6300`.

## Vtable-only methods (added 2026-09-25, G1; condensed from ledger)

Descriptions are condensed from the ledger notes; full text is in [`../reference/app.md`](../reference/app.md).

### AppScreen
- `AppScreen_Call415650_0042eb60` `0x0042eb60`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x00415650...
- `AppScreen_OnArg_0042eec0` `0x0042eec0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): if arg: 0x...
- `AppScreen_OnEnter_0042ee50` `0x0042ee50`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): [this+4]=0...
- `AppScreen_PlayMissionFmv_0042edb0` `0x0042edb0`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw/0x0042edb0_*.c): [this+4] = Mission_GetOu...
- `AppScreen_Shutdown_0042f9d0` `0x0042f9d0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x0048a980...
- `AppScreen_Slot10_Disable_0042eca0` `0x0042eca0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x00463120...
- `AppScreen_Slot10_Tick_0042ec80` `0x0042ec80`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): if !0x0046...
- `AppScreen_Slot8_Enable_0042eb10` `0x0042eb10`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x00463120...
- `AppScreen_Tick_0042eac0` `0x0042eac0`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw/0x0042eac0_*.c): if [0x004f3df4]: 0x00443...
- `AppScreen_Tick_0042ee70` `0x0042ee70`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): if ![this+...

### App
- `App_Callback_ImportCall_00442770` `0x00442770`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): [0x004cc0d...
- `App_Callback_Set5392a4_Off` `0x00442680`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x00442270...
- `App_Callback_Set5392a4_On` `0x00442660`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x00442270...
- `App_EnterMissionOver_0042f8e0` `0x0042f8e0`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw): checkpoint; HW: SystemParametersInfoA(0...
- `App_Forward_0042db50_004427d0` `0x004427d0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x0042db50...

### RuntimeClass
- `RuntimeClass_Get_004d0bf0` `0x00430240`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return &0x...
- `RuntimeClass_Get_004d1eb0` `0x00442260`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return &0x...
- `RuntimeClass_Get_004d20e0` `0x004437a0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return &0x...

### ScalarDeletingDtor
- `ScalarDeletingDtor_00431b50` `0x00431b50`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): vptr=0x004...

### WolPatch
- `WolPatch_OnStatus_00442720` `0x00442720`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw/0x00442720_*.c): status 2 "Connecting..."...

## Open
- ~~**Planning, numeric fidelity.**~~ **Answered 2026-09-24, `VERIFIED-ORACLE`:** under the reference
  setup (dgVoodoo2) the FPU control word at the vehicle tick is `0x027F`, i.e. 53-bit precision and
  round-to-nearest. The cooperative level (0x13) passes no FPU flags. Details are in `zvideo.md`
  section 1.
- **`Mission_GetOutcome` `0x00417800` name.** It returns `+0xdc`, which `Mission_Load` treats as
  the mission id, and `App_EnterGameplay` uses it to pick the CD track. The name may be wrong.
  Check its other callers before relying on "outcome".
