# Script - the zInterp world-builder interpreter

Narrative spec for subsystem `script` (membership CLOSED, all members CONFIRMED). Per-function
detail is in [`../reference/script.md`](../reference/script.md).

**What it is.** A line-oriented command language that **builds and configures the scene**:
worlds, cameras, lights, models, materials, LODs, sequences, fog, textures and renderer
tolerances. `Script_DispatchCommand` `0x004c20a0` recognises about 160 commands. The full
command list is in [`../../03_re/decomp/0x004c20a0_interp_commands.txt`](../../03_re/decomp/0x004c20a0_interp_commands.txt)
and the command-to-handler table in
[`0x004c20a0_interp_bindings.csv`](../../03_re/decomp/0x004c20a0_interp_bindings.csv).

**Correction.** The subsystem registry described this as "`.gs` mission scripts". The command
set is scene construction (e.g. `NewWorld`, `CameraSetFOV`, `LightSetColor`, `ModelPolygonVertex`,
`WorldSetFogRange`); mission logic lives in `mission.md` and `objective_triggers.md`. The registry
description is fixed in item G2.

Tag: `CONFIRMED-BINARY` unless marked.

## 1. Interpreter object
- **Lifetime.** `Script_Dtor` `0x004c0e50` frees buffers and lists. `Script_Reset` `0x004c0f70`
  frees variables, labels, frames and the string set, and clears state.
- **Variables** are `{name, value}` string pairs at `+0x60` (count `+0x64`):
  - `Script_SetVar` `0x004c1780` adds or replaces;
  - `Script_GetVar` `0x004c15f0` looks up by `strcmp`;
  - `Script_VarIsTrue` `0x004c1710` is true only when the value is the exact string `"TRUE"`;
  - `Script_FreeStringPairs` `0x004c1670` frees them.
- **Labels** (stride 0xc at `+0x68`): `Script_FindLabel` `0x004c1a40` and `Script_FreeLabels`
  `0x004c16c0`.
- **Frame stack** for nested blocks (`+0x9c`, depth `+0xa0`): `Script_PushFrame` `0x004c18c0`,
  `Script_PopFrame` `0x004c1940`, `Script_FreeFrames` `0x004c1960`.
- **Line counter.** `Script_IncLine` `0x004c1b20`.

## 2. Reading and running
- **`Script_RunStream` `0x004c1020` (stream, ctx)** loops `Script_ReadLine` → `Script_ExecuteLine`
  until either returns 0.
- **`Script_ReadLine` `0x004c1160`:**
  - Text mode: `fgetc` up to the newline.
  - Compiled mode: reads one pre-tokenised block (argc plus a blob of NUL-terminated tokens).
- **`Script_ExecuteLine` `0x004c1090`:**
  1. Unless a skip is active (`+0x25`), it tokenises the line.
  2. If there is a command and `Script_BuiltinDirectives` accepts it, it runs the pre-hook
     (`vfunc+0`), then `Script_DispatchCommand`, then the post-hook (`vfunc+4`).
- **`Script_Tokenize` `0x004c13c0`:**
  - strips a `#` comment;
  - splits on `", \t\n"` into argv (`+0x20`);
  - is a no-op in compiled mode.
- **Built-in directives** `Script_BuiltinDirectives` `0x004c1c50`:
  - conditionals: `ifdef`, `ifndef` and `endif` (skip frames);
  - `set name value` and `source file`;
  - `mkdir` (failure → `"%s %s FAILED (errno == %d)"`) and `Quit`.
- **`Script_EvalCondition` `0x004c1b50`:**
  - one operand → `Script_VarIsTrue`;
  - otherwise it folds left to right with `||` and `&&`;
  - an unknown operator stops the evaluation.
- **Arguments:**
  - `Script_NextArg` `0x004c1990` takes the next token and expands variables;
  - `Script_ExpandVars` `0x004c1250` substitutes `%name%`; unknown variables are dropped;
  - typed readers: `Script_ArgBool` `0x004c19c0` (`on` or `true`, case-insensitive),
    `Script_ArgFloat` `0x004c1a00` (`atof`), `Script_ArgInt` `0x004c1a20` (`atoi`);
  - `Script_CheckArgs` `0x004c5820` checks the minimum argument count and the current node type,
    else `Script_Fail`;
  - `argv[0]` helpers: `Script_Argv0` `0x004c5510`, `Script_Argv0Equals` `0x004c54b0`,
    `Script_Argv0Matches` `0x004c5480`.
- **Errors and diagnostics:**
  - `Script_Fail` `0x004c5520` sets the error flag `+0x10` and calls the error hook `+0x70`;
  - `Script_CallErrorHook` `0x004c1b30`;
  - `Script_DumpArgs` `0x004c1870`, `Script_DumpValue` `0x004c1ab0`, `Script_DumpTree`
    `0x004c2030` (indented node tree).

## 3. Compiled-script cache
- **`ScriptCache_OpenIndex` `0x004c5550`:**
  - opens the cache file once;
  - the 8-byte header needs magic `0x08971119` and version `7`;
  - then it reads `count × 0x80`-byte index entries.
- **`ScriptCache_OpenCompiled` `0x004c5740` (path):**
  - finds the entry by case-insensitive name;
  - uses it only if the source file is **not newer** than the recorded time (`stat` vs entry
    `+0x78`, `difftime`);
  - then seeks to the entry's offset `+0x7c` and returns the cache stream.

  So a stale cache entry falls back to the text source.

## 3a. Dispatch rules (`CONFIRMED-BINARY`, bytes 0x004c20ad..0x004c20d3, re-read 2026-09-24)
- **Switch.** Keyed on `argv0[0] - 'A'` over 0..0x2c (`A`..`m`). The byte index table is at
  `0x004c543c` and the jump table at `0x004c53ec`. When the key is out of range, or nothing in
  the case matches, `Script_IncLine` (`[this+0x14]++`) runs. The dispatcher always returns 1.
- **Matching.** Within a case, the compares run in the order of the table and most are
  `strncmp` **prefix** matches. The order must therefore be kept: for example
  `CameraSetHorizonXZ` is tested before `CameraSetHorizon`.
- **Compare-length quirks** (read from the `push imm8` bytes). Each of these accepts a
  shortened prefix:
  - `CountCameraNodes` n = 14;
  - `Object3DSetTextureWorldTexturesPerMeter` n = 37 of 39;
  - `SetPerspectiveInverseZTolerance` n = 20.
- **Argument readers:**
  - `str` = `Script_NextArg`;
  - `bool` = `on`/`true`, case-insensitive; anything else is false;
  - `float` = `atof`, `int` = `atoi`; a missing argument reads as 0.
- **Per-command argument order and calls:**
  [`0x004c20a0_interp_args.csv`](../../03_re/decomp/0x004c20a0_interp_args.csv), 161 rows.
  **Six branches are missing from the Ghidra decompile** (`03_re/decomp_raw/0x004c20a0_*.c`):
  `VideoSetDither`, `VideoSetWireFrame`, `WorldSetFogRange`, `WorldSetFogRangeNear`,
  `WorldSetFogRangeFar` and `WorldGetFogRange`. Their rows were added from the listing
  (`0x004c4c1b..0x004c5240`). All six use **exact** `strcmp` (`Script_Argv0Equals`
  `0x004c54b0`). The fog-range commands require the current node to be a world (type 2) via
  `Script_CheckArgs` `0x004c5820`. Do not treat that decompile file as complete.
- **Commands in the shipped scripts that this dispatcher does not handle** (from
  `unzbd rc interp` over `interp.zbd`):
  - `LoadGameGen` (295 uses), `source` (218), `set` (184), `Quit` (95), `SetMission`,
    `SetModelDirectory`, `WeaponSetMaxTetherAltitude`, `GameGenSetWorld`,
    `GameGenForceFlatShading`, `SetKeyboardInput`, `GenericTerrainMorph`, `mkdir`, and a few
    tokens that look like comments (`;;;...`).
  - They are handled by `Script_BuiltinDirectives` and the game-level pre-hook (`vfunc+0`,
    section 2).
  - **Not yet itemised:** the game-level command set. Candidate code:
    `0x00451bd0/0x00451f70/0x00452100/0x00452250` reference those strings.

## 4. Command handlers kept in this subsystem
- `Script_NodeGetModelIfAny` `0x004c58c0`.
- `Script_SetNodeTextureScroll` `0x004c58e0` (`Object3DSetScroll` / `Object3DSetScrollAlways`):
  - sets the model's texture scroll (u, v);
  - "always" creates a per-frame scroll object once.
- The other handlers live in the subsystems that own the state they set. Examples:
  `Render_SetVertexShading` (render_frame), `Geom_SetBFETolerance` (geometry), `Palette_SetName`
  (texture), `ZVid_LoadPalette` (zvideo), `LensFlare_SetTexture` (zeffect), `WindowClass_*`
  (class_api).

## Open
- ~~Which files the game runs, and whether a compiled cache ships.~~ **Answered 2026-09-24,
  `MEASURED-ASSET`:** `zbd\interp.zbd` is the cache (magic `0x08971119`, v7) and holds 122
  compiled scripts: 94 `.gw` (e.g. `support\bft1.gw`) and 28 `.gs` (e.g. `m13_zbd.gs`). See
  `asset_io.md`.
