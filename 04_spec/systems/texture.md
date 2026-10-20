# Texture manager (`texture`) — first pass

**Status:** subsystem registered 2026-09-17, membership OPEN. No gate — do not write code from
this file yet. Hardware binding creation is specified in `render.md` ("Texture handles").

## Image record — 0x38 bytes (`Image_Alloc` `0x0046ec00`)
| offset | field |
|---|---|
| `+0x00` | pixel count (width × height) |
| `+0x04` / `+0x06` | width / height (shorts; width also at `+0x34` as int) |
| `+0x08` | byte (set by `Image_SetByte8`) |
| `+0x09` | flags: bit 0 = 16-bit pixels (else 8-bit), bit 1 = alpha format for HW, bit 0x10 = shared palette, bit 0x20 = owns pixels `+0x10`, bit 0x40 = owns alpha `+0x14`, bit 0x80 = owns palette `+0x18` |
| `+0x0C` | clamp bits (bit 0 U, bit 1 V) — word, read by `TextureManager_LoadPending` |
| `+0x0E` | palette size (non-zero = paletted, one byte per pixel) |
| `+0x10` | pixel buffer |
| `+0x14` | alpha buffer (non-null selects ARGB4444 vs 1555 on HW) |
| `+0x18` | palette (HW rejects paletted textures) |

Pixel buffer size = pixel count if paletted, else bytes-per-pixel × pixel count.

## Search paths
Texture file search paths are a semicolon-separated string set `[0x0053d794]`, the same
mechanism as the sound config's SETS.

## Texture slots — `CONFIRMED-BINARY`
4096 slots of 0x24 bytes at `0x0053d79c` (count `[0x0053d798]`), `+0` image, `+0x1C` state,
`+0x20` next in a chain. States: **1 loaded, 2 pending first load, 3 pending reload**.
- `TextureManager_FindOrAddSlot` (`0x0046d810`) matches on the **basename without extension** —
  `a.tga` and `a.bmp`, or two `a.tga` in different folders, share one slot — and appends new slots
  with **no capacity check** (a 4097th texture overruns the table).
- Unloading a chain puts slots into state 3; the next load pass re-uploads rather than recreating
  the binding. The built-in default image `0x004e06e0` is never freed.
- Archive files are opened for a load pass and closed at its end.
- `Image_CreateProcedural` leaks its record when configuration fails.
- Images are read from a **16-byte header** (`Image_ReadHeader`); the read is unchecked.
- `Image_ApplyAlphaMask` zeroes pixels whose alpha byte is 0 (non-paletted only).
- The manager keeps **two archive lists** (`0x0053d76c` and `0x0053d774`), each of 0xA4-byte records.
- Texture archive list B is **`image.zbd`**, falling back to **`rimage.zbd`** (`0x0046da40`), opened once.
- `Path_SetExtension` finds the extension with `strchr` (first `.` after the first separator) — paths with dots in directory names get mangled.
- **Mip levels by name:** a texture named `…_1` pulls in `…_2`, `…_3`, … until one fails to load, linked through slot `+0x20`, each with a scale `base width / level width` computed in **integer** division (`TextureSlot_LoadMipChain` `0x0046e3e0`).
- Archive TOC lookup is **case-insensitive** (0x28-byte entries: name, offset `+0x20`, palette index `+0x24`); palettes are shared per archive.
- A full free leaves slots in state 3 untouched.
- **Load order:** archive list A, then list B (`image.zbd` / `rimage.zbd`), then loose files.
- **Pixel data is stored as RGB565** and converted **in place to RGB555** when the screen format
  reports 555 (`Image_ReadPixels` `0x0046ede0`).
- **Fonts:** `Font_LoadAll` (`0x0046efe0`) loads the `FONTS` list into a 20-entry table (no bound
  check) and expects exactly 95 glyphs per font, only warning otherwise.

## Archives and shade ramps — `CONFIRMED-BINARY`
- **Archive format** (`TextureArchive_Open` `0x0046dae0`): 24-byte header (version must be **1**,
  palette count, TOC count), then 0x28-byte TOC entries (name, offset `+0x20`, palette index `+0x24`),
  then 256-colour 16-bit palettes (512 bytes each). Palettes go into one global shared table.
- **Shade entries** are 8 floats `{rgbA, rgbB, weightA, weightB}`. Every paletted image gets **32
  levels** per entry: level 0 blends toward colour A, level 31 toward colour B (`ShadeLevel_Generate`
  `0x0046e4e0`) — a fog/tint ramp for the software path. Adding an entry rebuilds all palettes.
- **Archive choice by memory** (`0x0046df50`): hardware tries `rtexture{MB}.zbd` using the card's
  texture memory in megabytes, stepping down; software uses the texture-detail setting
  (1→`texture8`, 2→`texture6`, 3→`texture4`, 4→`texture2`, else `texturemax`). Final fallbacks:
  `texturemax.zbd`, then `texture.zbd`.

## Function index additions (2026-09-24)
- **Texture manager** (4096 slots of 0x24 bytes at `0x0053d79c`):
  - `TextureManager_Reset` `0x0046d550`, `TextureManager_FreeAllSlots` `0x0046d5d0`;
  - `TextureManager_ShutdownAll` `0x0046ebb0` / `TextureManager_Shutdown` `0x0046eb90`;
  - `TextureSlot_UnloadChain` `0x0046e250` follows `+0x20` while the state is 1;
  - `TextureSlot_SetNameFromPath` `0x0046e380` keeps the part after the last `\` or `/`, copied
    without a bound;
  - `TextureManager_FindSlotByName` `0x0046d4d0` is case-sensitive (member of render_frame).
- **Archives:**
  - list A (`[0x0053d76c]`): `TextureArchiveListA_FreeAll` `0x0046d6b0`,
    `Image_LoadFromArchiveListA` `0x0046dd30`;
  - list B (`[0x0053d774]`): `TextureArchiveListB_FreeAll` `0x0046d780`,
    `Image_LoadFromArchive` `0x0046d940` (opens `image.zbd` when list B is empty; case-insensitive
    table-of-contents search);
  - `TextureArchive_CloseAllFiles` `0x0046d730`. Records are 0xA4 bytes.
- **Texture table.** `TextureTable_Init` `0x0046eb20` (20 entries at `0x0056179c`) and
  `TextureTable_GetOrDefault` `0x0046efc0` (entry 0 is the fallback; no bounds check).
- **Shade entries** (8-float keys, 32 levels of 0x200 bytes each):
  - `ShadeEntry_FindExact` `0x0046e680` (exact float equality);
  - `ShadeEntry_Add` `0x0046e720` (rebuilds the affected images);
  - `Image_BuildShadeLevels` `0x0046e8d0`.
- **Image record:**
  - `Image_BytesPerPixel` `0x0046ec20`, `Image_PixelDataSize` `0x0046ec40`;
  - `Image_SetFlags` `0x0046ec60`, `Image_SetPixelsAndAlpha` `0x0046ec70`, `Image_SetSize`
    `0x0046ec90`;
  - `Image_FreeOwnedBuffers` `0x0046ecf0` frees only buffers whose ownership bits are set;
  - `Image_IsColumnTransparent` `0x0046f210` (key colour `[0x005617f4]`);
  - `Texture_ResampleSquare` `0x0046e9b0` (nearest neighbour to N×N);
  - `Image_PrepareSoftwareSampling` `0x004902b0` (log2 sizes, U/V shifts; software path).
- **Texture flag.** `Texture_SetFlag_004e073c` / `Texture_GetFlag_004e073c` `0x0046d5b0/0x0046d5c0`
  back the texture menu toggle.

