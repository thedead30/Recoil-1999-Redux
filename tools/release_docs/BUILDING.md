# Building the Recoil Remake

## What you need

Run the requirements checker first. It checks each item below and prints a download link for anything missing:

```
python tools/requirements.py
python tools/requirements.py --dgvoodoo "C:\path\to\dgVoodoo folder"   (also checks your dgVoodoo files)
python tools/requirements.py --contributor                              (also checks the verification tools' packages)
```

- **Windows 10 or 11.**
- **Python 3.10 or later.**
- **Visual Studio 2022 Build Tools**, with the "Desktop development with C++" workload. That workload includes
  MSVC, the Windows SDK, CMake and Ninja. Tested with 17.14 (MSVC 14.44, CMake 3.31.6, Ninja 1.12.1).
- **dgVoodoo2**, tested with 2.8.7.3 and 2.8.7.5. Use `DDraw.dll` and `D3DImm.dll` from the zip's `MS\x86` folder.
- **Your own copy of Recoil**, 1999-01-29 build.

The remake is built 32-bit with x87 floating point, the same as the original, so the arithmetic matches bit for bit.

## 1. Get the game files from your copy

`tools/recoil_source.py` reads your copy in any form: an installed folder, a zip, the CD or a copy of it, `.iso`,
`.bin`/`.cue` or `.nrg`. It refuses any other version of Recoil.

```
python tools/recoil_source.py "D:\Recoil.nrg"                                (show what it found)
python tools/recoil_source.py "D:\Recoil.nrg" --extract 05_remake\build\game_sandbox
python tools/recoil_source.py "D:\Recoil.nrg" --music 05_remake\build\game_sandbox\music
```

`05_remake/build/game_sandbox` is the game folder that development builds run in. It should be a copy of the game:
the game writes files into its own folder, so never point the remake at your original install.

## 2. Build

```
05_remake\build.bat                  (Release build plus the full test suite)
05_remake\build.bat Release NAME*    (run only the tests whose names start with NAME)
```

Tell CMake where your original `Recoil.exe` is. The build copies the game's menus, dialogs and icons out of it, and
the oracle tests load it:

```
cmake -S 05_remake -B 05_remake\build\Release -DRECOIL_ORIGINAL_EXE="C:\path\to\Recoil.exe"
```

By default CMake uses `00_original\game_install\Recoil.exe` beside the repository.

The data image (`05_remake/src/platform/image/original_data.cpp`) holds **no bytes of the game**. At start-up, the
remake loads the original's data tables from your `Recoil.exe`, and refuses to start if that file is not exactly the
supported build. It looks for the file in this order: the `RECOIL_ORIGINAL_EXE` environment variable, then
`original\Recoil.exe` next to the remake, then `Recoil.exe` in the game folder.

## 3. Run

```
tools\play_remake.bat
```

This runs the remake in `05_remake\build\game_sandbox`, starting directly in mission 1. Launcher switches
(environment variables):

| Variable | Effect |
|---|---|
| `RECOIL_START_MISSION=0` | normal start-up through the launcher (default 1 = straight into mission 1) |
| `RECOIL_WINDOWED=1` | windowed instead of full screen (see `tools/WINDOWED_MODE.md`) |
| `RECOIL_DDRAW_DIR=folder` | the folder holding dgVoodoo's `DDraw.dll` + `D3DImm.dll` |
| `RECOIL_NOCD=0` | require the real CD (by default the game folder stands in for the CD) |
| `RECOIL_ORIGINAL_EXE=path` | your original `Recoil.exe` |
| `RECOIL_VCD_LOG=1` | write `vcd.log`, a log of the virtual CD-audio drive that plays the music |

## 4. The player package

```
python tools/make_release.py --exe "C:\path\to\Recoil.exe"
```

This builds `05_remake\build\Player` with `RECOIL_PLAYER_RELEASE=ON`. That build uses a static C runtime, has no
console window, contains no paths from your machine, and leaves out the game's resources (Setup copies those from
the player's own copy). The script then writes both downloads to `dist\`. Before writing anything it checks that
neither download contains any Recoil data.
