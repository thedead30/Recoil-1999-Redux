# A guide to the original Recoil.exe

This is a guided tour of the original program, for anyone starting to read this project. Every fact here is
backed by the research ledger (`03_re/ledger/`) and the written specs (`04_spec/`). Follow the links for the
detail and the evidence.

Addresses are virtual addresses in the supported build (1999-01-29, SHA-256 `22423e0b…87d0`). They are the
same addresses Ghidra or x64dbg show when they load that file.

## 1. What the file is

- A 32-bit Windows program built with **Microsoft Visual C++ 5.0** (linker 5.10, January 1999) and **MFC 4.2**.
- Its floating point is x87, and the game sets it to **53-bit precision** before any game code runs
  (`Crt_SetFpuControl` `0x004c6350`). The remake keeps this, so its arithmetic rounds exactly like the original's.
- It contains **4,871 functions** (ledger `functions.csv`). 3,330 are Recoil's own code: the game and its engine.
  The rest are the C runtime, MFC, and compiler-made helpers (thunks, exception-handling funclets).
- It came from **two source projects**, which the program itself names in its assert and error messages (64 file
  names in all):
  - **`Battlesport\`**: the game. `RecoilApp.cpp`, `player.cpp`, `mission.cpp`, `hud.cpp`, `map.cpp`,
    `pickup.cpp`, `turret.cpp`, `ai_net.cpp`, `Briefing.cpp`.
  - **`GameZRecoil\`**: the **GameZ engine**: `zClass` (the world and its objects), `zVideo`, `zRndr`
    (rendering), `zSound`, `zInterp` (the scripting language), `zGeometry`, `zMath`, `zEffect`, `zModel`,
    `zDEClient`, `zNetwork`.

  The remake's `05_remake/src/` uses the same folders and file names. The ledger column `orig_file` says which
  original file each function came from, and `orig_file_evidence` says how we know.

## 2. Start-up, step by step

Spec: [`04_spec/systems/app.md`](04_spec/systems/app.md).

1. **`entry` `0x004c6140`**: the C runtime start. It sets the FPU precision, reads the command line, runs the
   static constructors, then calls `WinMain_Thunk` `0x004c81c0` → `AfxWinMain`.
2. **The application object** is a static MFC `CWinApp` at `0x004f3ca8`, built by `MainApp_Ctor` `0x0042dfa0`.
3. **`RecoilApp_BootstrapAndCDCheck` `0x0042e5da`**:
   - creates the main window (`RecoilApp_InitInstance` `0x004429d0`);
   - loads `MESSAGES.DLL`, which holds all the game's text;
   - checks for the CD;
   - brings up the engine: video, sound, input, and the asset archives.
4. **The launcher window**: the original's own window, with the Game / Options / Help menus. **Game > Start**
   pushes the first screen.

## 3. The main loop and the frame

- **`RecoilApp_Run` `0x00442d00`** loops forever:
  1. pump Windows messages;
  2. receive network messages (does nothing in single player);
  3. call the current screen's frame function;
  4. apply at most one queued screen command: push, pop or replace, on a screen stack at most 16 deep.

  Every menu, briefing, map screen and the game itself is a *screen* on this stack
  ([`menus.md`](04_spec/systems/menus.md)).
- **`App_RenderFrameAndPresent` `0x0042f280`** is the gameplay frame. In order:
  1. frame timer;
  2. input poll;
  3. run every scripted node;
  4. sound update;
  5. render the world through the camera list;
  6. HUD;
  7. objectives check;
  8. present the frame.
- **Time:** `0x004a56d0` is the only place the frame time-step is written. It is clamped to **at most 0.125 s**,
  and the vehicle code clamps it to **at least 0.005 s**. So the simulation always runs between 8 Hz and 200 Hz,
  however fast or slow the PC is ([`frame_timing.md`](04_spec/systems/frame_timing.md)).

## 4. How the game's data is organised

- **`.zbd` archives** (the "Zar" container) hold the worlds, models, textures, sounds and scripts:
  - `zbd\m1..m13\` - one folder per map;
  - `zbd\image.zbd`, `interp.zbd`, `zrdr.zbd` - shared archives;
  - `soundsH/M/L.zbd` - the sound banks in high, medium and low quality. If the chosen one is missing, the game
    falls back to the next.

  See [`asset_io.md`](04_spec/systems/asset_io.md).
- **`.zrd` config trees** (the "reader" format): nested lists of named values. They hold vehicle physics, weapons,
  AI and settings.
- **`gamez.zbd`** for each map: the saved **world graph**. A tree of nodes (objects, cameras, lights, triggers), each
  with a class, flags and a transform ([`zclass_nodes.md`](04_spec/systems/zclass_nodes.md),
  [`cls_world.md`](04_spec/systems/cls_world.md)).
- **zInterp scripts** build each mission's world and wire up its events ([`script.md`](04_spec/systems/script.md)).
  Missions, objectives and triggers are in [`mission.md`](04_spec/systems/mission.md) and
  [`objective_triggers.md`](04_spec/systems/objective_triggers.md).
- **Save games**: [`savegame.md`](04_spec/systems/savegame.md).
- **Video**: AVI files (8 in Indeo 5, 3 in Cinepak), played through Video for Windows
  ([`avi.md`](04_spec/systems/avi.md)). Windows 11 has no Indeo 5 decoder, so the remake still needs its own (`KG-18`).
- **Music**: ordinary CD-audio tracks 2 to 5, played through the MCI `cdaudio` device.

The parsers in `06_tools/scripts/` (`parse_gamez.py`, `parse_zmap.py`, `parse_anim.py`, `parse_savegame.py`) read
these formats. They are a good way to explore your own copy of the game.

## 5. The subsystems

The ledger groups every function into one of about 40 subsystems (`03_re/ledger/subsystems.csv`, with rules and
boundaries in `subsystem_registry.csv`). Each subsystem has two write-ups:
- a narrative spec in `04_spec/systems/<name>.md`;
- a per-function reference in `04_spec/reference/<name>.md`.

| Area | Subsystems |
|---|---|
| Program and screens | `app`, `menus`, `ui_widgets`, `settings`, `sysinfo`, `mapscreen`, `savegame` |
| Gameplay | `player`, `vehicle`, `weapon`, `turret`, `pickup`, `hud`, `mission`, `ai_net` |
| World and objects | `cls_world`, `zclass_nodes`, `class_api`, `object3d`, `scene_update`, `script`, `declient` |
| Physics and maths | `collision`, `geometry`, `math3d`, `transform`, `view`, `camera` |
| Rendering | `render_frame`, `scene_render`, `rasteriser`, `poly_shade`, `texture`, `zvideo`, `zeffect` |
| Sound and video | `sound`, `avi` |
| Input | `input` |
| Files | `asset_io` |
| Multiplayer | `net`, `znetwork` (not in the first release of the remake) |

Good places to start reading:
- [`damage_and_destructibles.md`](04_spec/systems/damage_and_destructibles.md): how shooting a generator opens a
  gate, traced function by function.
- [`movement_types.md`](04_spec/systems/movement_types.md): the six movement models a vehicle can use (basic,
  fly, sub, track, hover, amphibious).
- [`ai_behaviour.md`](04_spec/systems/ai_behaviour.md): how enemies choose what to do.
- [`frame_timing.md`](04_spec/systems/frame_timing.md): a short read that explains a lot.

## 6. Reading a function

1. Look it up in `03_re/ledger/functions.csv` by address or name. Its row gives:
   - its status;
   - its original source file;
   - how it was verified;
   - where it lives in the remake (`remake_file`, `remake_symbol`).
2. Read its entry in `04_spec/reference/<subsystem>.md`.
3. Open its port in `05_remake/src/`. The header comment gives the address, the calling convention and a one-line
   summary. The body is the original's machine code as MSVC inline assembly, so it can be proven identical with
   `tools/prove_identity.py`. Structure fields appear as offsets (`[ecx + 0x5a0]`); their meanings are in
   `04_spec/structs/` and the narrative specs.
4. To see it run: load your own `Recoil.exe` in Ghidra or x64dbg at the same address. Or run its oracle test, which
   calls the original and the remake on the same inputs and compares them.

## 7. Known bugs in the original

The original has bugs, found while porting and testing it: file reads whose results are never checked, stack
values used without being set, a clipping routine that can loop forever. The remake keeps all of them on purpose,
because a faithful remake includes its faults. They are listed in
[`ORIGINAL_DEFECTS.md`](ORIGINAL_DEFECTS.md), generated from the `original-defect` rows of
`03_re/ledger/known_gaps.csv`.
