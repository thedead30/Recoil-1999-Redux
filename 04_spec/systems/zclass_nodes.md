# zClass node types and the GameZ world file (.zbd)

**Status:** membership CLOSED, 61/61 CONFIRMED, mostly from the Ghidra decompile.
`GameZ_ReloadNodeModel` was read from disassembly; the small helpers with hidden arguments were
read from bytes. The gate is not open: 11 callees belong to the texture/material/model IO modules
(delegated). Tag: `CONFIRMED-BINARY`.

## Node class data (`[node+0x38]`, type `[node+0x34]`)
| type | class | data size | notes |
|---|---|---|---|
| 1 | camera | 0x1E8 | first 4 dwords are node references |
| 2 | world | 0xAC | node lists +0x90/+0x9C; area partition grid +0x7C columns × +0x78 cells (0x40 each, node list per cell) |
| 3 | window | 0xF8 | |
| 4 | display | 0x1C | |
| 5 | object3d | 0x90 | |
| 6 | lod | 0x50 | create defaults: [0]=1, [2]=1000.0, [3]=1000000.0, [0x11]=1 |
| 7 | sequence | 0x28 + 8/entry | entries {child, value} at +0x20, count +0x1C |
| 9 | light | 0xE4 | dirty flag +0; colour +0xAC..B4; falloff range +0xCC/+0xD0; node list +0xDC |
| 10 | sound | 0x94 | create: node bounds ±2 / 1.0 / −1.0, data +0x7C=32, +0x80=64, +0x84=4096, +0x88=0.03125; name at +8 (≤35 chars) |

Accessors report `Null node pointer` / `Null class data pointer` with the source line and return 5.

## Node records
0xC4-byte records in `[0x00539c94]` (count `[0x00539c90]`). Index ↔ pointer helpers treat
negative/null as "none". Flag `0x1000000` at `+0xC0` = record has data. The low 24 bits of `+0xC0`
hold the file offset of the node data when written.

## .zbd file
Header 0x24 bytes: magic `0x02971222`, version **15**, texture result, texture/material/model
section offsets, node count, world value `[0x004de4c8]`, node section offset.
- Load: path ≤ 46 chars is remembered in `[0x00539ca8]`. A longer path is reported (limit printed
  as 48) and not stored. Sections are read in order texture → material → model → nodes.
- Node data per type as in the table, then parent (`+0x54/+0x58`) and child (`+0x5C/+0x60`) lists.
- **Sequence (7) and type 8 nodes are not supported by the writer or reader.** The writer reports
  and writes nothing for them, not even their lists. The reader reports, returns -1 and leaves the
  record inactive.
- `GameZ_ReloadNodeModel`: re-reads one record's model from the file, optionally recursing into
  children.

## Defects to reproduce
Unchecked malloc/realloc/calloc everywhere; sequence insert index unchecked; save returns without
fclose on header write failure; sound read/write always emits a warning report.

## Function index additions (2026-09-24)
- **GameZ world file (`.zbd` scene snapshot)**, load-bearing for asset loading (TODO E4):
  - **`GameZ_OpenAndCheckHeader` `0x004556a0`:** 0x24-byte header, **magic `0x02971222`,
    version 15**. Otherwise it reports and fails.
  - **`GameZ_LoadZbd` `0x00455520`:** the path must be under 47 chars; it is stored in
    `[0x00539ca8]`.
  - `GameZ_LoadWorldFile` `0x00455730`, `GameZ_ClearWorldPath` `0x00454360`.
  - **Records** are 0xC4 bytes (array `[0x00539c94]`, count `[0x00539c90]`;
    `ClsRecord_ToIndex` `0x00454370`). Their data is per type, for example camera 0x1E8 bytes:
    `GameZ_ReadNodeData` `0x00454c60` and `GameZ_WriteNodeData` `0x004544b0`.
  - `GameZ_ReadNodeIndexList` `0x00454bf0`, `GameZ_WriteNodes` `0x00454890`, `GameZ_SaveZbd`
    `0x00454a50` (the writer is not needed by v1).
- **Copying nodes.** `ClassNode_CopyDispatch` `0x00452400` switches on the type:
  - `ClassNode_CopyCamera` `0x00451f70` and `ClassNode_CopyObject3D` `0x00452100` are
    implemented;
  - lights, sounds, animate, sequence and switch nodes refuse: `ClassNode_CopyLight/Sound/
    Animate/Sequence/Switch_Unsupported` `0x004520c0/0x004520e0/0x00452230/0x004523c0/0x004523e0`;
  - flag `0x4000000` returns the node itself;
  - `ZClassNode_CopyProperties` `0x00451bd0` copies name, active state and flags 08/10/20/40;
  - `ZClassNode_CloneModelIfSpecial` `0x00451b20`.
- **Destroying and releasing:**
  - `ZClass_DestroyNodeData` `0x0044f1d0`;
  - `ClassNode_Release` `0x004528a0`, `ClassNode_ReleaseRecursive` `0x004528b0`,
    `ClassNode_ReleaseModelsRecursive` `0x004528e0`;
  - `World_ClearAll` `0x00451a00`, `gClsShutdown` `0x004518e0`.
- **Tree walks:**
  - `gwNode_VisitTreeUntil1` `0x00452810`, `gwNode_IsClassType1Visible` `0x004527f0`;
  - `Node_UpdateLightsRecursive` `0x00452860`, `ZClassNode_UpdateModelOncePerFrame`
    `0x004c59e0`;
  - `NodeList_FindNext` `0x0044f690` (resumable search) and `NodeList_FindNextWith0044f750`
    `0x0044f740`;
  - `SceneGraph_GetRoot` `0x004518f0`.
- **Class-data accessors** (each checks for a null node or null data and returns 5, and a wrong
  class returns 3):
  - windows: `WindowClass_SetSize` / `SetOrigin` `0x0044f8b0/0x0044f9c0`,
    `Class_GetCameraFields_8_C` / `0_4` `0x0044f930/0x0044fa40`;
  - displays: `DisplayClass_SetSize` / `SetOrigin` `0x0044fe90/0x0044ff10`, `Display_SetColour`
    `0x0044ff90`;
  - lights: `Light_SetFieldA8/A4/B8/C4` `0x00453200/0x00453250/0x004532a0/0x004533b0`,
    `Light_SetModeBC` / `ModeC0` `0x004532f0/0x00453350`, `Light_SetVec14` / `Vec08`
    `0x00453560/0x004535c0`, `Light_GetColor` `0x00453a40`;
  - light falloff: `Light_SetFalloffRange` `0x00453400` (near = min, far = max; equal values log
    an error) and `Light_GetFalloffRange` `0x00453500`;
  - sounds: `Sound_SetVec30` / `GetVec30` `0x00452d00/0x00452d60`, and `FUN_00452ec0`
    `0x00452ec0` (emitter world position);
  - sequences: `Seq_InsertEntry` `0x00453f40`, `Seq_SetField0/4/8/C`
    `0x004540c0/0x00454100/0x00454140/0x00454180`;
  - type 6: `NodeType6_Create` `0x004542a0` (defaults 1000.0 and 1000000.0),
    `Node_SetClassDataField0_NoCheck` `0x00454330`, `Node_SetClassDataRangeCheck` `0x00454340`
    (stores the range squared).

## Still open
- Texture, material and model section formats (delegated modules).
- Meaning of camera/world/window/display field layouts beyond sizes.
