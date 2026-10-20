# Settings and system information

**Status:** both gates OPEN — `settings` 6/6 and `sysinfo` 30/30 CONFIRMED, memberships CLOSED
(`03_re/ledger/subsystem_registry.csv`). Summarised from CONFIRMED ledger rows (mostly decompile
reads; the small probes are byte reads). Tag: `CONFIRMED-BINARY` unless marked.

## Settings — a registry-backed variable list
- **Init** (`0x004b3260`): stores three path parts (`[0x0056bcd8]`, `[0x0056bcdc]`, `[0x0056bce0]`),
  empties the node list `[0x0056bcd0]`, sets initialised `[0x0056bcd4]`, runs system-info collection.
  It does **not** load from the registry despite the ledger name.
- **Nodes** (`Settings_RegisterNode` `0x004b2e80`, 0x20 bytes, pushed at the list head, lookup by name):

  | offset | field |
  |---|---|
  | +0x00 | buffer (types 3–7) — otherwise the value lives in the node itself |
  | +0x08 | type 0–7 |
  | +0x0C | size: forced 4 for types 0/1, 8 for type 2, caller size for 3–7 (0 → rejected) |
  | +0x10 | name |
  | +0x14 | scope: 1 = HKCU, 0 = HKLM, other = never persisted |
  | +0x18 | next |

  Registering an existing name returns the existing node unchanged.
- **Key:** `HKxx\SOFTWARE\<path1>\<path2>\<path3>` (separator string at `0x004e4678`).
- **Load** (`0x004b2960`): both HKCU and HKLM keys must open, else **nothing** loads. A value is read
  only if its stored size equals the node size exactly; the registry type is not checked.
- **Save** (`0x004b2bf0`): type 0 → REG_DWORD; types 1–7 → REG_BINARY. Aborts on the first failed
  write, leaving both keys open; an HKCU create failure closes an unopened handle.
- **Shutdown** frees names, buffers and paths.

## System information (`sysinfo`)
`SysInfo_Collect` (`0x004b30b0`) fills a block (copied out by `0x004b3090` from `0x0056bce4`):

| offset | value |
|---|---|
| +0x00 | CPU vendor string (CPUID leaf 0) |
| +0x10 | CPU family: 2/3/4 from legacy flag probes (286/386/486), CPUID family otherwise; bit 0x8000 = non-Intel |
| +0x14 | CPU speed result field +0xC — **0 on every non-Intel CPU** |
| +0x18 | flags: bit 0 MMX, bits 1–2 always 0, bit 6 video probe > 0 |
| +0x1C | total physical memory, KB |
| +0x24 | DirectSound caps field » 10 (only if DirectSound opens) |
| +0x28 | 0 |

**CPU speed** (`0x004b36f0`): no TSC → timed bit-scan loop with QueryPerformanceCounter; TSC with
no clock hint → RDTSC over ≥1000 QPC ticks at time-critical priority, repeated until three runs
agree within 3; TSC with a hint → RDTSC across a CMOS RTC seconds boundary read through **ports
0x70/0x71** (works only where user-mode port I/O is allowed — `PLATFORM`; a remake cannot and
should not reproduce this probe, only its result field).

## Defaults (added 2026-09-24, bytes re-read; caveat resolved)
- **`RecoilApp_LoadUserSettings` `0x00407700`** first zeroes the whole settings block
  (`0x004e5d00`, 0x26 dwords).
- For each preset-driven key, the value comes from the matching **`detail.zrd` hardware
  preset**. If no preset condition matches, the fallback below applies:

| key | fallback |
|---|---|
| EffectsLevel_SW / _HW | 1 / 0 |
| GfxFlags_SW: Transparency 0x2, Lighting 0x1, Perspective 0x8, GlobalLight 0x10, AllVideoBuffer | 1, 1, 1, 0, 0 |
| GfxFlags_HW (same bits) | 1, 1, 1, 1, 0 |
| ObjectLOD, TextureMemory (SW and HW), SoundLOD | 0 |
| HUDFlag_SW / _HW, HUDType_SW / _HW | 1 |
| VMode | 5 (640×480) |

- Keys without a preset call keep their saved value, else 0.
- The actual per-machine values therefore live in `detail.zrd` (asset data, item E4).

## Function index (added 2026-09-24, condensed from ledger notes)

Descriptions are condensed from the ledger notes; full text is in [`../reference/settings.md`](../reference/settings.md).

### Settings
- `Settings_ApplyHWCardFlag` `0x00408280`: *[0x004e5d58] = ECX; [0x004e5dcc] = ECX
- `Settings_ApplyVideoModePreset` `0x00408720`: ECX=mode: *[0x004e5d30]=mode; per mode, calls in order Viewport0_SetOrigin/Viewport0_SetSizeF/ScreenRect_SetSi...
- `Settings_FindNodeByName` `0x004b3380`: No stack args; name in ECX. Walks a SINGLY-LINKED list of settings nodes from the head [0x0056bcd0], returning...
- `Settings_GetDisplayRect_10` `0x00408660`: return [*[0x004e5d84]]+0x10
- `Settings_GetDisplayRect_14` `0x00408670`: return [*[0x004e5d84]]+0x14
- `Settings_GetDisplayRect_20` `0x00408690`: return [*[0x004e5d84]]+0x20
- `Settings_GetFullScreen` `0x00408330`: return *[0x004e5d54]
- `Settings_GetHWAPI` `0x00408320`: return *[0x004e5d5c]
- `Settings_GetHardwarePresetOrDefault` `0x00407680`: (default) ECX=preset node ret 4: child by name (ConfigTree_FindChild 0x0048cf70); for each entry 1..n-1: if Pr...
- `Settings_GetScreenRect_14` `0x004086d0`: return [*[0x004e5d88]]+0x14
- `Settings_GetWOLPasswordFlag` `0x00408a20`: return *[0x004e5d94]
- `Settings_Get_004e5d70` `0x004086a0`: return *[0x004e5d70]
- `Settings_ResetNetworkState` `0x00407e00`: call 0x00429f80; Settings_SetNetworkFlag(0) (0x00408230); Settings_StoreNetworkModem(0) (0x00408240); tail jmp...
- `Settings_ScreenRect_SetSize` `0x004086e0`: Rect_SetOrigin (0x004083d0) on [*[0x004e5d88]] with (ECX, EDX)
- `Settings_ScreenRect_SetOrigin` `0x00408700`: Rect_SetSize (0x00408400) on [*[0x004e5d88]] with (ECX, EDX)
- `Settings_SetDisplayRect_20` `0x00408680`: [*[0x004e5d84]]+0x20 = ECX
- `Settings_SetNetworkFlag` `0x00408230`: *[0x004e5d74] = ECX
- `Settings_Shutdown` `0x004b32c0`: 181 bytes, decomp (prior name kept). Only when initialised: frees every node (name, and the buffer for types 3...
- `Settings_StoreFullScreen` `0x004082a0`: *[0x004e5d54] = ECX
- `Settings_StoreGameCtlOptions` `0x00407e20`: *[0x004e5d3c] = ECX
- `Settings_StoreHUDFlag` `0x004082b0`: if [0x004e5dcc]: *[0x004e5d24]=ECX else *[0x004e5d20]=ECX
- `Settings_StoreHWAPI` `0x00408290`: *[0x004e5d5c] = ECX
- `Settings_StoreJoystickNumAxes` `0x004083a0`: *[0x004e5d64] = ECX
- `Settings_StoreJoystickNumButtons` `0x004083b0`: *[0x004e5d68] = ECX
- `Settings_StoreNetListen` `0x00408250`: *[0x004e5d78] = ECX
- `Settings_StoreWOLPasswordFlag` `0x00408a10`: *[0x004e5d94] = ECX
- `Settings_Store_004e5d6c` `0x00408300`: *[0x004e5d6c] = ECX

### RecoilApp
- `RecoilApp_ApplySoundAPISetting` `0x004080a0`: *[0x004e5d34] = ECX; tail jmp 0x004a1290
- `RecoilApp_GetSoundAPICheckboxValue` `0x004080b0`: return *[0x004e5d34]
- `RecoilApp_LoadUserSettings` `0x00407700`: Settings_OpenDetailPresetFile(0) (detail.zrd) fail -> 0; 0x004b3090; registers settings nodes (Settings_Regist...

### Preset
- `Preset_CompareOp` `0x00407220`: (rhs) ECX=operator string, EDX=lhs ret 4: "==" eq, "<" lt, ">" gt, "<=" le, ">=" ge, "!=" ne, "~=" always true...
- `Preset_EvaluateCondition` `0x00407470`: config node ECX: string "DEFAULT" -> 1; list of 4 {name, op, value}: name CPU_CLASS / CPU_MHZ / VIDEO_KB / RAM...

## Still open
- The reference constant in the non-TSC speed routine (not visible in the decompile).
- What the "video probe" thunk `0x004a9910` measures.
- Which game code reads the info block fields (only `0x0040773d` copies it).
