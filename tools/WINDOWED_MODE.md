# Windowed mode and side-by-side testing (how it was achieved)

Status (2026-10-07): kept as a **testing tool, opt-in**. Normal play is full screen, like the original
(`Desktop\Play Recoil REMAKE.bat`). Ledger: KG-47 (windowed start), KG-55 (attempts), KG-60 (side by side), KG-61
(compare saves).

## Turning it on

| What | How |
|---|---|
| Remake in a window | `Desktop\Play Recoil REMAKE (windowed).bat`, or `set RECOIL_WINDOWED=1` before `tools\play_remake.bat` |
| Original in a window | `Desktop\Play Recoil ORIGINAL (windowed).bat` (runs the copy in `Desktop\Recoil Original Windowed`) |
| Both at once, shared keyboard/mouse | `Desktop\Recoil Side by Side.bat` -> `tools\sidebyside\sidebyside.ps1` |

## The two pieces that make a window

1. **The game must ask for windowed mode.** Recoil reads `FullScreen` from its registry settings
   (RegQueryValueExA, import slot 0x004cc010). With `RECOIL_WINDOWED=1` the remake's launcher
   (`05_remake/boot/boot_main.cpp`, `WindowedRegQuery`) answers 0 for that value only; the registry is never changed.
   The original's copy in `Recoil Original Windowed` has the game's own windowed setting.
2. **dgVoodoo must honour it.** The runtime copy of `dgVoodoo.conf` had `OutputAPI = direct3d11`, which is **not a valid
   value**; dgVoodoo then falls back to defaults, ignores `FullScreenMode = false` and calls SetFullscreenState(TRUE).
   That was the whole "always full screen" problem. The folder `Desktop\Recoil dgVoodoo Windowed` (DDraw.dll, D3DImm.dll,
   messages.dll, dgVoodoo.conf) has `OutputAPI = bestavailable` and `FullScreenMode = false`. The remake uses it via
   `RECOIL_DDRAW_DIR`; `play_remake.bat` sets that automatically only when `RECOIL_WINDOWED=1`.

## Running the original and the remake together (KG-60)

- **Second instance quits:** `App_ActivateExistingInstance` 0x0042e990 calls FindWindowA("RecoilClass") and exits if a
  window exists. Bypassed for "RecoilClass" only: remake via `RECOIL_SHARED_INPUT=1`; original via the dinput.dll wrapper
  (`tools/sidebyside/rcl_dinput.cpp`, patches import slot 0x004cc628).
- **Shared input:** DirectInput devices are switched to NONEXCLUSIVE | BACKGROUND (flags 0x0a) by patching
  IDirectInputA::CreateDevice (vtable slot 3) and IDirectInputDeviceA::SetCooperativeLevel (slot 13). Both games then
  read the same keyboard and mouse.
- **Title bars / placement:** `sidebyside.ps1` removes WS_POPUP, adds CAPTION|SYSMENU|MINIMIZEBOX, titles the windows
  "RECOIL - ORIGINAL"/"RECOIL - REMAKE", centres them on the left/main screens for ~25 s, then leaves them where dragged.
- **Keep running when not focused:** the original's wrapper subclasses the game window and swallows
  WM_ACTIVATEAPP(FALSE)/WM_ACTIVATE(WA_INACTIVE)/WM_NCACTIVATE(FALSE) and vetoes minimise (CBT hook). Works in the
  original; in the remake it crashed at 0x0040bb00, so it is off unless `RECOIL_KEEP_ACTIVE` is set.
- **Direct start of mission 1** in both (no launcher/menus): from the message pump after 20 PeekMessageA calls,
  Mission_SetDataSearchPaths(N,0) then App_StartMission(app 0x004f3ca8, 0, N, 0, 1, 1).
- **Clock:** both use a GetTickCount counted from start-up (original defect KG-54: float clock steps in 62.5 ms after
  ~6 days of Windows uptime).
- Limitation: not lockstep - the two games drift apart; that is why compare saves (KG-61) replaced live side by side.

## What was tried and failed (do not repeat)

- Skipping SetDisplayMode in the remake -> purple/black/green colours, crash in the cutscene.
- DDSCL_NORMAL cooperative level -> "Error opening video".
- DxWnd with every renderer -> "Error opening video" with system D3D. Abandoned (folder `Desktop\DxWnd` unused).
- Attaching cdb with a timeout to a game being played closed the user's game twice: never attach to a live session.
- After a killed full-screen run the desktop can stay at 640x480: reset with ChangeDisplaySettingsA(NULL, 0) (the scripts
  do this).

## Compare saves (KG-61, uses the windowed pieces)

`Desktop\Recoil Compare - Play Remake.bat` (Scroll Lock = save `cmp_<hhmmss>` + beep + snapshot) and
`Desktop\Recoil Compare - Replay in Original.bat` (`tools/sidebyside/compare_saves.ps1`): the original's wrapper presses
the launcher's Game > Start, waits 8 s after the window, runs the LoadGameScreen_Load 0x00435a70 sequence (without its
QueuePop), and writes a frame-0 memory dump when [0x004f3df0] flips to 1 (SBS_DUMP). The remake does the same with
RECOIL_LOAD_SAVE + RECOIL_DUMP. This found KG-51.
