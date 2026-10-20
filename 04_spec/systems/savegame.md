# Save / load container

**Status:** gate **OPEN**. 16/16 CONFIRMED and membership CLOSED. The manager wrappers,
handler call thunks and comparator were read from bytes; the rest from the Ghidra decompile.
Save list UI and file enumeration are in `menus.md`. Tag: `CONFIRMED-BINARY`; Win32 file API is
`PLATFORM`.

## Manager (`[0x0056bf70]`)
- `SaveGame_SaveToFile` `0x004c0030` / `SaveGame_LoadFromFile` `0x004c0050`: return 0 when no
  manager exists.
- Handlers sit in a `std::list` at `[mgr+4]` (count `[mgr+8]`). Each has: name `+0` of its data,
  save callback `+4`, load callback `+8`, **priority `+0xC`**, user data `+0x10`.

## Write — `0x004c0370`
1. Creates the file (CREATE_ALWAYS, exclusive).
2. Sorts handlers by ascending priority (stable list merge sort).
3. Calls each save callback in order and **stops after the first one that returns 0**.
4. Closes the file and returns the last result.

## Read — `0x004c0400`
- Opens the file as a ZAR archive (entries 0x94 bytes, name at +8).
- The entry name is `<handler>/<section> ...`: the handler is the text before `/`, the section the
  text up to the next space.
- The handler is found by exact, case-sensitive match. Unknown handlers are skipped silently.
- Entry data is read into a reusable buffer that only grows, then passed to the load callback as
  (section, data, size). Reading stops when abort flag `[mgr+0x30]` is set.
- Entry lookup inside the archive is case-insensitive (`_stricmp`); handler match is case-sensitive.

## Function index (added 2026-09-24)
- **Manager lifetime.** `SaveSystem_Create` `0x004c0100` creates the manager at `[0x0056bf70]`
  (0x34 bytes: handler list sentinel at +4, count +8, archive member +0xc).
  `SaveSystem_Destroy` / `SaveSystem_Dtor` `0x004c0180/0x004c01b0` tear it down.
- **Temporary files:**
  - `SaveGame_OpenTempRecord` `0x004c0080` returns `tmpfile()` only while a save is active;
  - `SaveGame_CloseTempRecord` `0x004c00a0` → `SaveGame_FlushTempFileToEntry` `0x004c0700`:
    size, read back, write a named entry, `_rmtmp`;
  - `SaveSystem_WriteToTempFile` `0x004c00c0` → `TempFile_FromBuffer` `0x004c0780`;
  - `SaveSystem_RemoveTempFiles` `0x004c00e0` → `TempFile_RemoveAll` `0x004c07c0`.
- **Entries.** `SaveGame_WriteNamedEntry` `0x004c0630`: the name is `"<section>/<name>"` in a
  0x50 buffer, written by `Zar_WriteEntry`. `SaveGame_SetDirty30` `0x004c0620` sets `+0x30 = 1`.
- **Handlers:**
  - `SaveHandler_CallSave` `0x004c06a0` (callback `+4`, null returns 1);
  - `SaveHandler_CallLoad` `0x004c06c0` (callback `+8`, with data, size and user data `+0x10`);
  - they are ordered by priority `+0xC` (`SaveHandler_Less` `0x004c0260`) using
    `SaveHandlerList_Sort` `0x004c07d0`, an MSVC `std::list::sort` with 15 bins, stable merge.
- **List helpers:** `List_Begin` `0x004c0b60`, `List_Swap` `0x004c0ba0`, `List_MergeByLess`
  `0x004c0bd0`, `List_Splice` `0x004c0ce0`, `List_DestroyNodes` `0x00403db0`.
- **Archive file** (`.zar` container, shared with `asset_io`):
  - `ZarWriter_Create` `0x004a6270` (`CreateFileA`, `CREATE_ALWAYS`);
  - `ZarArchive_FindEntry` `0x004a65d0`: case-insensitive, 0x94-byte entries, name at +8;
  - `ZarArchive_ReadEntry` `0x004a6670`: 0x10001 means missing; 0x10002 means the buffer is too
    small, and that return doubles as a size query.

## Defects to reproduce
Read/seek results are unchecked; buffer `new` unchecked; entry name copies of 80 may be
unterminated.

## Still open
- Where handlers register, and what each handler writes (per-system save formats).
- The ZAR writer side (how entries are written during save).
