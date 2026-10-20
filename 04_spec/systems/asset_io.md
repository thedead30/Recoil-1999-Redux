# GameZ asset sections: textures, materials, models

**Status:** membership CLOSED, 84/84 CONFIRMED (mostly from the Ghidra decompile; hidden-argument
wrappers, colour/UV constants and return forms from bytes). Tag: `CONFIRMED-BINARY`.

## Texture directory (zImage)
Entries are 0x24 bytes in a fixed array at `0x0053d79c`, **max 0x1000**, count `[0x0053d798]`.
Entry +0x20 is stored as an index in files. Index −1 ↔ null.

## Materials (0x2C bytes; array `[0x00566a1c]`, default size **2500**, max < 0x8000)
| offset | field |
|---|---|
| +0x00 | word flags: 0x100 textured(+used), 0x200 / 0x400 extra; byte+1 bit 0 textured, bit 2 cycling |
| +0x02 | 16-bit colour (untextured) |
| +0x04..+0x0C | r,g,b floats (init 255.0) |
| +0x10 | texture pointer (index in file) |
| +0x18/+0x1C | 0.5 / 0.5 at init |
| +0x24 | cycle: {loop +0, frame float +8, speed +0xC (default **15.0**), capacity +0x10, count +0x14, frames +0x18} |
| +0x28/+0x2A | short prev/next (used and free doubly-linked lists; heads `[0x004e1164]`, `[0x004e1160]`) |

- Find-or-create: a one-entry cache, then a linear search with `Material_Compare`. When the array
  is full, **the shared default material `0x005669f0` is returned**.
- Extra material names come from the detail preset config (max 100 names). The name match is a
  prefix match ending at end-of-string or a space.

## Models (0x58 bytes; array `[0x00576204]`, free list via +0x54)
Fields: flags +4 (bits 0/1/2/3/4/5), in-use +8, polygons +0xC/+0x30, vertices +0x10/+0x34,
normals +0x14/+0x38, morph +0x18/+0x40, point records (0x4C) +0x1C/+0x3C. In files +0x54 holds the
data offset. Polygon (0x1C): +0 low byte vertex count, 0x100 / 0x200 (normal indices) flags; +8
vertex indices; +0xC normal indices; +0x10 uvs; +0x14 material; +0x18.

### Adding geometry (`Model_AddPolygon` `0x00483650`)
1. Fewer than 3 vertices → error. More than 57 (0.9×64) → error.
2. Collinear vertices are removed (eps `[0x004e1390]`). If fewer than 3 remain → discarded.
3. A non-planar polygon with more than 3 vertices is fan-triangulated.
4. More than **48** vertices → split into fans of up to 4 vertices (a split piece with fewer than 3 is
   reported but still added).
5. Vertices are deduplicated within `[0x004e1398]`. Past about **922** vertices (0.9×1024) new
   vertices are refused (or added with -1 returned in the offset variant).
6. UVs are rebased by floor(min) and snapped to **1/256**. UVs of vertices beyond the first three are
   rebuilt from the plane gradient of the first three (dominant-axis projection).

## Defects to reproduce
Null-mesh write in `0x00484250`; leaked frame array in `Material_SetCycleFrameCount`; leaked polygon
copy on write error; material write continues after header errors; uninitialised bounds for empty
models; unchecked allocations throughout; `Texture_FromIndex` accepts negative indices other than −1.

## Function index additions (2026-09-24)
### `.zbd` archive container ("Zar"), which feeds TODO E4
- **Layout from the reader:** the file ends with an 8-byte trailer `{dword 1, dword count}`,
  preceded by `count` table-of-contents entries of **0x94 bytes** each: `+0` file offset,
  `+4` size, `+8` name.
  - The parser (`Zar_ParseFooterAndTOC` `0x004a63f0`) needs the file size ≥ 8 and the first
    trailer dword = 1.
  - The writer (`Zar_WriteFooterAndClose` `0x004a6360`) writes the same layout.
  - Name lookup is case-insensitive (`ZarArchive_FindEntry`).
  - **`MEASURED-ASSET` 2026-09-24:** 17 of the 105 `.zbd` files in `00_original/game_install`
    parse with this trailer, for example `zbd\m1\zrdr.zbd` (149 entries, first `ai.zrd`).
  - **`.zbd` is not one format.** `image.zbd`, `interp.zbd`, `anim.zbd`, `gamez.zbd` and
    `[r]texture2/4.zbd` do *not* have the Zar trailer; each has its own format. `gamez.zbd`
    starts with the GameZ header `0x02971222`, version 15, which matches `zclass_nodes.md`.
  - The remaining formats are item E4. Checked so far (`MEASURED-ASSET`, 2026-09-24):
    - **Texture archives:** `texture2/4/6/8.zbd`, `rtexture2/4.zbd` and `image.zbd`, 61 of 61
      files. The layout matches `texture.md`:
      - header of 6 dwords: `0`, version **1**, palette count, entry count, `0`, `0`;
      - `count × 0x28` entries (name, data offset `+0x20`, palette index `+0x24`, -1 = none);
      - `palettes × 512` bytes of palettes.
      Data offsets are ascending and the first equals the end of the palettes in every file.
      The `rtexture*` and `image` files have **0 palettes** (direct 16-bit). The `texture*`
      files have 12 to 18.
    - **Script cache `interp.zbd`**, matching `script.md` section 3:
      - header: magic `0x08971119`, version 7, count 122;
      - `0x80`-byte entries: name (up to 0x78), time `+0x78`, data offset `+0x7c`. Offsets are
        ascending and the first equals the end of the index.
      - Entries are 94 `.gw` and 28 `.gs` compiled scripts, e.g. `support\bft1.gw` and
        `m13_zbd.gs`.
      - The compiled records are tokenised command lines, e.g.
        `{0x28, argc 2, "SetModelDirectory", "..\data\m1\models\bft"}`.
      So `.gs` and `.gw` are the scene scripts of `script.md`, and a compiled cache **ships
      with the game**.
    - **`anim.zbd`**: 13 of 13 files parse to exactly the end of the file
      (`03_re/scripts/asset_check_anim.py`), using the read order of `Anim_LoadFile`
      `0x0045efb0`:
      1. header `{magic 0x08170616, 0x1c, manifest count}`;
      2. `count × 0x54` manifest records;
      3. `0x3c` world header (node count u16 `+0xa`, trailing count `+0x10`);
      4. per node:
         - the `0x134` struct;
         - the tables counted by the bytes `+0x105..+0x10b`, then `+0x10d` (sizes 0x60, 0x28,
           0x2c, 0x2c, 0x24, 0x24, 0x30, 0x48);
         - **always** a `0x40` block into `+0xc4`;
         - a blob of size `+0x100`;
         - `+0x104` entries of `0x40` plus a buffer of size `entry+0x3c`;
      5. `trailing count × 0x24` records.
      Node counts are 173 to 819. The trailing count is **0 in every shipped file**.
    - **`.zmap`**: 12 of 12 map files parse to the end of the file
      (`03_re/scripts/asset_check_zmap.py`), using `MapScreen_ReadFile` `0x004168d0` and
      `MapMarker_ReadFile` `0x00415bd0`:
      1. header: u32 version (**5**, anything else fails with "Incorrect Map File Version"), u32
         `+0x24`, 6-float bounds;
      2. markers until a short read, each: 3-byte colour, u32 point count, `count × 0xc`
         points, u32 state.
      - **`m1`, `m7`, `m8`, `m9`, `m10`, `m11` and `m12` are byte-identical** (same MD5); those
        missions share one map. There is no `m13.zmap`; what the map screen does for mission 13
        is not established.
      - The meaning of `+0x24` (values 1 to 31) is not established; the loader does not
        interpret it.
    - **`gamez.zbd` sections** (13 of 13 files; per-type node data not parsed):
      - header words: magic, 15, texture count, texture offset (always `0x24`), material offset,
        model offset, **node capacity 16000**, world value, node offset;
      - the texture table is `count × 0x24` bytes in every file (material offset - `0x24` =
        count × 0x24);
      - the material section starts `{5000 capacity, used}`, and the model section
        `{6000 capacity, used}`;
      - the node section is 16000 records of `0xc4` bytes. The first named record's `+0xC0` equals
        exactly the end of the record array, so node data follows the records. The data offsets
        of named records ascend in 12 of 13 files; **`m6` is the exception**.
      - Not established from the files: the `0x1000000` "has data" flag of `zclass_nodes.md`
        does not appear in any file's `+0xC0` (it may be a runtime-only bit). Unnamed records hold
        small integers that look like a free-list, but the chain does not validate. The header
        world value is `[0x004de4c8]` per `zclass_nodes.md`.
        **mech3ax corroboration:** `unzbd rc gamez` reads 4199 nodes from `m1` (none empty) and
        4955 from `m6`, **287 of them `Empty`**. That fits the header value being the **head of
        the free-record list**: `m1` has no holes, so the head is the count (4199), while `m6`'s
        first hole is 309. This is `INFERRED` from two files plus mech3ax; confirm it in
        `GameZ_ReadNodeData` `0x00454c60` / the free-list code before porting.
    - **mech3ax v0.6.1 (`06_tools/mech3ax`, `unzbd rc ...`), run 2026-09-24:**

      | command | result |
      |---|---|
      | `sounds` | parses (`soundsl.zbd`, 281 entries) |
      | `interp` | parses (122 scripts, matching our count) |
      | `reader` | parses (`zrdr.zbd`, `m1\zrdr.zbd` 150 entries = 149 plus manifest, matching our parse) |
      | `messages` | parses `messages.dll` |
      | `textures` | parses (`texture2` 577 = 576 plus manifest, `image` 434, `rtexture4`) |
      | `gamez` | parses: textures, materials, meshes and nodes; `m1` has 4199 nodes (World, Window, Camera, Display, 4041 Object3d, 153 Lod, Light) |
      | `zmap` | parses; its `unk04` is our unexplained `+0x24` |
      | `anim` | **not implemented for Recoil** ("Recoil support for Anim isn't implemented yet"); our `asset_check_anim.py` is the only parser |

      mech3ax is therefore a usable **cross-check and extraction tool** for every format except
      `anim.zbd`. Its field names are its own inference, so they are not evidence for Recoil
      semantics. It agrees with our counts wherever both exist.
- **Open and close.** `ZarArchive_Construct` `0x004a6190`, `ZarArchive_Destruct` `0x004a61b0`,
  `Zar_OpenFile` `0x004a61d0` (`GENERIC_READ`, share read), `ZarArchive_Close` `0x004a62b0`.
- **Entries.** `ZarArchive_SeekToEntry` `0x004a6630`, `Zar_GrowTocBuffer` `0x004a62f0`,
  `ZarIndex_Free` `0x004a6330`.
- **Registry of open archives** (`[0x0056b184]`):
  - `Archive_InitRegistry` `0x0048cc70`, `Zar_OpenAndRegisterArchive` `0x0048d210`;
  - `Zar_FindEntryInArchives` `0x0048d1c0` (first archive that has the entry);
  - `Reader_CloseAll` `0x0048d2c0`, `Archive_Shutdown` `0x0048cd10`.

### Search paths and files
- **Search path.** `SearchPath_InitOrReset` / `InitOrAdd` `0x0048cca0/0x0048cce0` and
  `SearchPath_Resolve` `0x004a5e50` (splits the path and tries each search directory).
- **Opening files.** `File_OpenOnSearchPath` `0x004a5f50`, `File_Exists` `0x004a5c20` (`_access`),
  `File_GetSize` `0x004a5c50`, `File_ReadLengthPrefixedString` `0x004a6110`.
- **String sets:**
  - `StringSet_CreateAndAddTokens` `0x004a5ca0`, `StringSet_AddSemicolonTokens` `0x004a5ce0`
    (`;`-separated);
  - `StringSet_Destroy` `0x004a5cc0`, `StringSet_DestroyGlobal` `0x004a5df0`,
    `List_FreeAllPopped` `0x004a5e10`.
- **Numbered file names.** `WildcardName_InitDigitExpansion` / `NextDigitCombination`
  `0x004a5f90/0x004a6070` expand up to five `*` characters as a 0-9 odometer.
- **Shutdown.** `zUtlShutdown` `0x004a6100`.

### Config tree (`.zrd` reader data)
- **Node types:** 1 int, 2 float, 3 string, 4 list.
- **Reading and freeing:** `ConfigTree_ReadNode` `0x0048d080` (binary read), `ConfigTree_FreeNode`
  `0x0048ce60`, `ConfigTree_AllocChildArray` `0x0048cda0`.
- **Lookup:** `ConfigTree_FindChildByName` `0x0048cec0` (recursive over lists),
  `ConfigTree_GetStringValue` `0x0048cf80`, `ConfigTree_GetFloatValue` `0x0048cfb0`.
- **`ConfigTree_ParseFileByBasename` `0x0048cdc0`.**
- **Converting values:** `ConfigValue_ToInt` `0x004071f0` (int, `ftol`'d float, or a string via
  `NameTable_LookupValue` `0x00407190`).

### Containers
`Container_InitNodePool` / `ShutdownNodePool` `0x0048c7d0/0x0048c890`, `Container_ListAppend`
`0x0048ca30`, `Container_GetNth` `0x0048cb30`, `Container_Count` `0x0048cc60`,
`Container_FindFirstByPredicateThunk` `0x0048cc50`.

### Models (gmod), materials and textures
- **Model pool** (0x58-byte records at `[0x00576204]`):
  - `ModelCache_SetArraySize` `0x00475ff0` (only once), `Model_Alloc` `0x00482080` ("model buffer
    full" error);
  - `Model_Free` `0x004820f0` refuses while in use; `Model_ReleaseContents` `0x00482160`;
  - `Model_AddRef` / `Model_Release` `0x004826f0/0x00482700` (the count at `+8`; nothing is freed
    at zero);
  - `Model_ToIndex` / `FromIndex` `0x00481570/0x004815a0`, `gModShutdown` `0x00475e60`,
    `Model_ForEachLoaded_004820f0` `0x00475f60`.
- **Reading models:** `Model_ReadSection` `0x00481fa0`, `Model_ReadOne` `0x00481aa0`,
  `Model_ReadContents` `0x00481c50`, `Model_ReadThreeDwords` `0x00481bc0`. Buffers are allocated
  unchecked.
- **Building models:**
  - `Model_AddVertexDedup` `0x00482720` (tolerance `[0x004e1398]`) and `Model_AddVertexWithOffset`
    `0x00482860`;
  - `Polygon_RemoveCollinearVertices` `0x00482b40`, `Polygon_IsPlanar` `0x00482db0`,
    `Polygon_NewellPlane` `0x00482e30`, `Vector_UnitCross` `0x00482c60`;
  - `Polygon_FanTriangulate` `0x00482fe0`, `Polygon_SplitFan` `0x00483240` (at most 4 vertices);
  - **`UV_QuantizeAndRebase` `0x00483510`: `uv = ftol((x + 1/512) * 256) / 256`**, bytes read,
    so reproduce the quantisation;
  - `Polygon_FixupUVs` `0x004843b0`, `Triangle_SolveGradient2D` `0x00484860`;
  - `Model_Call483650_Arg2Zero` `0x00483610`.
- **Other model functions:**
  - `Model_Clone` `0x00482270`, `Model_NeedsSpecialDraw` `0x00483a60`;
  - `Model_ComputeBounds` / `ComputeSymmetricBounds` `0x00483b80/0x00483e60`,
    `Model_BuildVertexWeightTable` `0x00483f80`;
  - `Model_ScrollUVs` `0x004791c0` (wrap range 0x80 on hardware, 0x800 on software);
  - `Model_FreeExtraBuffers` `0x00480ec0`;
  - global setters `Model_SetScaleGlobals` `0x004804c0`, `Model_Get/SetGlobalFloat_004e1398`
    `0x00481530/0x00481540`, `Model_SetGlobalDouble_004e1388/004e1390` `0x00481550/0x00481560`.
- **Model instance parts:**
  - `ModelInstance_SetAllPartsField4` / `Bit8` `0x00484140/0x00484170`;
  - `ModelInstance_SetCyclePartsFlag0x200` `0x004841b0`, `ModelInstance_Call480f80OnCycleParts`
    `0x004841f0`;
  - `ModelInstance_ResetAnimFrame` `0x00484230`, `ModelInstance_SetCycleFrame` `0x004842b0`
    (wraps with modulo);
  - forwarders `ModelInstance_Call_00481220/00481260/00481100` `0x004842f0/0x00484310/0x00484330`;
  - `ModelInstance_ResetNonCycleParts` `0x00484350`.
- **Materials** (0x2C-byte array `[0x00566a1c]`, default 2500 entries):
  - set-up: `Material_InitArray` `0x00480ae0`, `Material_SetArraySize` `0x00480bf0` (under
    0x8000, only once), `Material_Init` `0x00480c40` (defaults 255.0 / 0.5 / 0x7FFF);
  - lookup: `Material_FindOrCreate` `0x00480ca0`, `Material_FindByTexture` `0x00481420`;
  - freeing: `Material_Free` `0x00480dc0`, `Material_FreeAllUsed` `0x00480d80`,
    `Material_Shutdown` `0x00480f10`;
  - indices and reading: `Material_ToIndex` / `FromIndex` `0x004805b0/0x004805e0`,
    `Material_ReadSection` `0x004808c0`;
  - flags and fields: `Material_HasTextureOrFlags` `0x00480c80`, `Material_SetFlag0x200`
    `0x00480f60`, `Material_SetField20` `0x00481040`;
  - textures: `Material_ReleaseTextures` `0x00480f80`, `Material_ReuploadTextures` `0x00480fd0`;
  - cycling textures: `Material_AppendCycleFrame` `0x00481100`, `Material_SetCycleTextureLoop`
    `0x00481220`, `Material_SetCycleTextureSpeed` `0x00481260`;
  - names: `MaterialName_FindIndex` `0x004804e0` (table starting `default`),
    `MaterialName_LoadFromConfig` `0x00481460`.
- **Texture directory in the world file.** `Texture_ReadDirectory` `0x0046d420` (at most 0x1000
  entries), `Texture_WriteDirectory` `0x0046d360`, `Texture_ToIndex` `0x0046d310`,
  `zImage_GetTable_004e0718` `0x0046d4c0`.
- **Tiny helpers:** `SetDword0_FromEDX` `0x004826a0`, `Record_SetFlagBit1` `0x004826b0`,
  `GetField8` `0x00482710`.

