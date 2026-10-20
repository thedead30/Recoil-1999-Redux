# Recoil Redux

**Recoil** (Zipper Interactive, 1999), the hover-tank combat game, rebuilt for modern Windows, with optional
improvements: 1920×1080 widescreen, a HUD that fits the wider screen, high-resolution textures, HD cutscenes,
smoother rendering and built-in tools for reporting problems.

Redux is built on **Recoil Classic**, an exact rebuild of the original program. Every function of the original
`Recoil.exe` was reverse-engineered and rebuilt, and the result is checked against the original by machine:

- **4,678 of 4,716 ported functions are proven byte-identical** to the original's code (`tools/prove_identity.py`).
- The rest are checked by running them side by side with the original on the same inputs (oracle tests).
- Every game constant in the source names where it came from (an address in the original, or a data file).

## The Redux rule

**Every Redux feature is a switch, and with every switch off, Redux is Recoil Classic**, still byte-identical to the
original. The switches live in `redux.ini` in the install folder. Setup writes one for you; delete it to play the
exact original. Each change to original behaviour has an `RX-nn` row in `03_re/ledger/redux_backlog.csv`, with the
original functions it touches and how it was tested.

Redux changes the game in only two ways:

- **Stand-ins:** a few original functions are replaced by Redux versions that do the same steps plus the
  improvement. The code for them is in `05_remake/redux/`.
- **Trampolines:** a short hook runs Redux code before or after an original function, which itself is unchanged.

When a switch is off, its hook is never installed.

## You need your own copy of Recoil

This repository and its downloads contain **no part of the game**: no executable, data, maps, sounds, videos or
music. Redux reads all of them from your copy when you install it.

The supported version is the **1999-01-29 build**, as on the "Recoil Classic" CD. The 1998 release is not supported:
its program code and 35 of its data files differ.

## Installing (players)

Download **RecoilRedux-Player-1.0.zip** from the Releases page, unzip it and run `Setup.exe`. Setup needs nothing else
installed: no Python, no runtimes. Everything is on one page:

1. **dgVoodoo2.** It is free and lets old DirectDraw / Direct3D games run on modern Windows. Setup opens its download
   page; download the zip and select it. Tested with 2.8.7.3 and 2.8.7.5. Setup configures it for Redux:
   - 2 GB of video memory;
   - 16× anisotropic filtering;
   - 4× anti-aliasing;
   - mipmapping.
2. **Your copy of Recoil and an install folder.** The copy can be:
   - the installed game folder;
   - a zip of it;
   - the CD (or a copy of it);
   - a disc image (`.iso`, `.bin`/`.cue`, `.nrg`). Only a full disc image (`.nrg` or `.bin`+`.cue`) includes the CD
     music.
3. **Textures.** Pick one of three:
   - **Original:** the game's own textures.
   - **Generate HD textures (Real-ESRGAN):** Setup upscales the textures of all 13 maps from your own copy on your
     PC. On a modern PC this takes about 15 minutes. It needs a graphics card with Vulkan; without one, Setup makes a
     smooth enlargement instead (about 1.5 minutes). Real-ESRGAN is included in the download (see its notice in
     `payload\esrgan`).
   - **Asset pack:** a zip of ready-made HD textures and videos, selected with Browse. The Browse box is only
     available when this option is chosen.

   Setup shows an example picture of all three.
4. Click **Install**, then start *Recoil Redux* from the desktop shortcut. Setup also makes the mission maps from your
   copy (about 10 seconds).

## What's in version 1 (1.01: the mission map)

Each feature below lists its `redux.ini` switch.

### Display

#### 1920×1080 widescreen (RX-03)

```
[Display]
Resolution=1920x1080
```

- The game renders at 1920×1080. **A wider screen shows more of the world to the sides; the picture is not
  stretched.**
- The game's own 3D renderer does this once its resolution settings are extended. Redux replaces the two functions
  that turn the Options-menu video mode into sizes: `Settings_ApplyVideoModePreset` `0x00408720` and
  `Render_ApplyResolutionPreset` `0x004a7990`.
- The crosshair, radar, score, timer and weapon icons move to the right places on their own.
- Other sizes work in `redux.ini`, but v1 is made and tested for 1920×1080. The wide HUD has a known fault above
  2048 pixels wide.

#### Wide HUD (RX-04)

```
[Display]
WideHUD=1
```

At 640×480 the top strip and the weapon panel fill the screen width; on a wider screen they were fixed 640-pixel
images. Redux rebuilds them **from your own copy's images** when the game loads them (an `Image_Load` `0x0046d900`
stand-in). No artwork is shipped.

- **Top strip:** the RECOIL logo stays on the left. The score box sits under the score (centred) and the timer box
  under the timer (right edge), with the strip's own metal in between.
- **Weapon bar:** runs the full width. Its rounded ends sit at the panel's edges, and the weapon slots stay centred
  with the weapon icons.
- **Indicator squares:** stay at the panel's top-right corner beside the health gauge, where the game lights them
  for pickups. Redux moves the game's indicator strip there too (`HudTriple_LayoutFromConfig` `0x0040f2e0`).

### Graphics

#### High-resolution textures (RX-10)

```
[Textures]
Pack=redux_textures
```

- When the game loads a texture, Redux looks for `<name>.png` in the pack folder. If the file is there, it
  replaces the original pixels.
  - The PNG is read with Windows' own image decoder (WIC).
  - It is converted to the game's 16-bit format (565 or 555, as the original).
  - Its transparency becomes the texture's alpha plane.
- Textures missing from the pack stay original. The game's renderer is unchanged.
- The pack is either generated by Setup from your copy, or installed from an asset pack zip.

#### Smooth textures at a distance: mipmaps (RX-37)

```
[Graphics]
Mipmaps=1
```

The original gives every texture a single full-size level, and it sets the minification filter to plain linear. So
distant surfaces skip between texels as you move: textures shimmer, most visibly with sharp HD textures.

Redux gives every texture a mip chain, each level a 2×2 average of the one above that keeps colour-key 0 transparent.
It then sets the device to trilinear filtering (`D3DFILTER_LINEARMIPLINEAR`). Three functions are hooked:

- `DDraw_CreateTextureSurface` `0x004a83d0` (stand-in);
- `Texture_UploadPixels` `0x004a8500` (trampoline);
- `Render_CreateD3DDevice` `0x004a9c20` (trampoline).

#### Top-tier graphics by default (RX-34)

```
[Graphics]
MaxDetail=1
```

In 1999 the game picked its detail levels from a hardware test, and that test rates modern PCs as slow: its CPU speed
probe returns 0 on non-Intel processors.

Redux takes the top entry of each option in `detail.zrd` instead: effects HIGH, object detail HIGH, texture memory
MAX, sound HIGH, perspective correction on. This replaces `Settings_GetHardwarePresetOrDefault` `0x00407680`. The
HUD keeps the full HUD; the top HUD tier is the minimal HUD.

#### Lighting starts off (RX-44)

```
[Graphics]
Lighting=0
```

With the game's lighting option on, scenes are much darker (about 28 %). Redux starts with it off; you can still turn
it on in the Options menu.

#### Vehicle shadows no longer flicker (RX-41)

```
[Graphics]
ShadowFix=1
```

With lighting off, an enemy's shadow vanished for a few frames each time it fired: the muzzle flash is a light, and
the shadow was shaded with it.

While the game shades a shadow node (`*shad*`: `shadow`, `track_shadow`), Redux sets the active light count to 0 and
restores it afterwards. The hook is a trampoline on `Render_ShadeAndQueueObjectPolygons` `0x00477b30`.

#### HD cutscene videos (RX-36)

If a `redux_video` folder exists, Redux plays `redux_video\<name>.mp4` in place of `video\<name>.avi`. The asset pack
contains these videos.

- **How:** Redux wraps the game's Video for Windows calls and decodes each MP4 frame with Windows' Media Foundation
  into the game's 16-bit frame.
- **What stays the game's own:** timing, skipping, drawing and the AVI's sound.
- **Windows 11:** this also fixes the cutscenes there. 8 of the 11 original videos are Indeo 5, which Windows 11
  cannot decode.

### Mission map (RX-47, new in 1.01)

```
[Display]
Map=1
```

Press **M** (the game's Map key) in missions 1-6 for a new map, drawn over the game at three quarters of the screen
and slightly see-through, so the game keeps running behind it:

- **The level from above**, rendered by Setup from your own copy's 3D geometry and textures (`redux_map\m<N>.png`, about
  10 seconds for all six maps). The game's original map outlines show faintly on top.
- **Mission objectives**, numbered and named, at the places the game's own tour points mark for them. A marker
  disappears once its objective is complete (the objective flags at `0x004f11fc`).
- **Weapons** (orange diamonds) and **tank upgrades** (blue hexagons), shown from the start so none is missed, read live
  from the game's pickup list (`0x004f3318`): a collected pickup disappears.
- **Secrets** (purple star) named by the mission scripts.
- **You**: a white arrow at your tank's position, pointing where you are heading.

Redux draws it only while the game's map is open (map object `0x004f0cc0` `+0x68`) and closes it for the pause menu;
nothing runs while the map is closed.

### Launcher and settings

#### Simpler start-up menu (RX-42)

```
[Launcher]
TrimMenu=1
```

The start-up window's menu shows only what Redux v1 supports.

- **Removed:** Multi Player Start, Westwood Online, the software renderer, A3D, joystick, and the old resolution
  entries.
- **Resolution entry:** the remaining one is renamed to the `redux.ini` resolution and stays checked.

#### Settings are kept between runs (RX-44)

```
[Launcher]
KeepSettings=1
```

The original saved its options to `HKEY_CURRENT_USER\SOFTWARE\Zipper\RECOIL\1.0`. But it loaded them only if a key
in `HKEY_LOCAL_MACHINE` also existed, and only the 1999 administrator installer made that key. So nothing you changed
was ever loaded again.

Redux sends the game's `HKEY_LOCAL_MACHINE` registry opens to `HKEY_CURRENT_USER`, so the options you set load next
time (`Settings_LoadFromRegistry` `0x004b2960`).

#### Neutral pilot name (RX-44)

```
[Launcher]
DefaultName=User
```

A new game offers "User" as the pilot name instead of your Windows user name.

### Problem reports

These help players send precise reports, so textures and bugs can be fixed.

| Key | What it saves |
|---|---|
| **F12** | a screenshot, the game settings and your note |
| **F11** (hold) | a burst of screenshots |

Everything is saved to `Pictures\Recoil Redux Screenshots\Bug Reports`; `[Tools] ScreenshotFolder=` changes the
folder.

#### Screenshot key (RX-35)

```
[Tools]
ScreenshotKey=F12
```

F12 saves the screen as a PNG, plus a `.txt` with every game setting.

#### Note box (RX-38)

```
[Tools]
ScreenshotNote=1
```

After F12 the game pauses with its own Escape menu, and a box appears for a note about the problem.

- The box is prefilled with the map number and the tank's position and heading.
- Type the note, then press **Enter** to save it into the `.txt`, or **Esc** to skip.
- Resume the game from the pause menu.

#### Burst capture (RX-39)

```
[Tools]
RapidKey=F11
```

Hold F11 to capture 10 screenshots a second, for at least 3 seconds and up to 20. They go into a `burst_<date>_<time>`
folder. Use this for problems too quick to catch with one screenshot.

#### Texture names under the reticle (RX-46)

To report a texture problem, aim the reticle at it and press F12. The `.txt` then names the textures under the
reticle, nearest first, for example:

```
Textures under the reticle (960, 540), nearest first: dock_edge, ground04
```

These names are exactly what a pack file is called (`<name>.png`), so a bad texture can be regenerated directly.

The check costs nothing during play:

- **When names are noted:** while textures load. Redux wraps the game's texture-binding creator, hook `[0x0056bc08]`
  (`0x004aa0f0`), to note which name goes with which texture.
- **When polygons are tested:** only in the one frame after F12. Recorders in the draw slots `[0x0056bc5c..64]` test
  the polygons under the reticle, and the original slots are put back at the next frame.

## Not done yet

- Menus, briefings and the map screen show at 640×480 in the top-left corner (RX-05).
- The resolution is set in `redux.ini`, not in the game's Options menu (RX-17).
- Multiplayer is not in version 1.
- The rest of the plan is in `03_re/ledger/redux_backlog.csv`: better weapon effects, gamepad support and more.

## Building it yourself

See [BUILDING.md](BUILDING.md). The Redux code is in `05_remake/redux/` (`redux.cpp`, `redux_video.cpp`); the player
Setup is in `05_remake/setup/`. To change the code and check your change, see [CONTRIBUTING.md](CONTRIBUTING.md).

## Learning how it works

- [RECOIL_EXE_GUIDE.md](RECOIL_EXE_GUIDE.md): a tour of the original program. It covers start-up, the main loop,
  the data formats and the subsystems, and suggests where to start reading.
- [HOW_IT_WAS_MADE.md](HOW_IT_WAS_MADE.md): how the original was taken apart and rebuilt, the tools and checks
  used, and how to repeat or extend the work.

## How the repository is organised

| Folder | What it is |
|---|---|
| `05_remake/redux/` | **Recoil Redux**: the `redux.ini` switches and every feature. Each feature replaces original functions only when its switch is on. |
| `05_remake/setup/` | The player Setup (C++; reads folders, zips, CDs and disc images; texture generation in `upscale.cpp`). |
| `05_remake/src/` | Recoil Classic, the exact rebuild. The folders mirror the original's own source files (`GameZRecoil/`, `Battlesport/`); functions whose original file is not known are in `unattributed/`. `platform/` is the layer between the original code and modern Windows. |
| `05_remake/boot/` | The launcher, including the no-CD mode and the virtual CD drive that plays the music. |
| `05_remake/tests/` | Tests. Many run the original function and the remake side by side. |
| `03_re/ledger/` | The research ledger: one row for each of the original's functions, with its status, evidence and source file. `redux_backlog.csv` is the Redux plan and change log. |
| `04_spec/` | Written specifications of structures, formulas and systems. |
| `tools/` | The build, verification and porting tools. |
| `ORIGINAL_DEFECTS.md` | Bugs in the original game that Classic keeps on purpose. |

## Thanks

- DustTheory's [recoil-patch](https://github.com/DustTheory/recoil-patch) showed that the game's own renderer handles
  widescreen once its resolution presets are extended. Redux uses none of its code or images.
- [Real-ESRGAN](https://github.com/xinntao/Real-ESRGAN) and
  [Real-ESRGAN-ncnn-vulkan](https://github.com/xinntao/Real-ESRGAN-ncnn-vulkan) (Xintao Wang et al.) generate the HD
  textures in Setup.
- [dgVoodoo2](http://dege.freeweb.hu/) by Dege runs the game's DirectX 5 graphics on modern Windows.

## Legal

The source code is free software: you can redistribute it and/or modify it under the terms of the
**GNU General Public License, version 3** or (at your option) any later version. See [LICENSE](LICENSE).

The licence covers this project's code only, not the game. Recoil is (c) 1999 Electronic Arts / Zipper Interactive.
This is a non-commercial fan project, not affiliated with or endorsed by them. See [NOTICE](NOTICE).
