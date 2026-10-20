import struct
import glob
import os
from collections import OrderedDict

# Recoil savegame (.sav) format - CONFIRMED 2026-08-31 (see recoil_re_log.md).
#
# Savegames are ZAR-format archives - the SAME generic container format already
# confirmed for sounds*.zbd and zrdr.zbd (footer-anchored: data section, then a
# TOC of ZarTocEntry-shaped records, then an 8-byte footer). Confirmed via the
# actual writer functions (Zar_WriteEntry / Zar_WriteFooterAndClose, in
# GameZRecoil/zUtil/zutl_zar.cpp per embedded debug paths) - the only two
# functions in the whole binary that call WriteFile.
#
# Footer (last 8 bytes of file): u32 magic (=1), u32 entryCount.
# TOC: entryCount * 0x94 (148) bytes, immediately before the footer.
# TOC entry layout matches the already-confirmed ZarTocEntry used for
# sounds*.zbd/zrdr.zbd: u32 dataOffset, u32 dataSize, char name[68], then a
# buildPath/timestamp region (unused/zeroed for savegame entries - those fields
# are ZAR-format leftovers from the shared writer, not meaningful for saves).
# Data section (file start): each entry's raw bytes, back-to-back, exactly per
# the TOC's offset/size - no gaps, verified byte-exact across all 6 real
# samples on disk.
#
# Entry names are "Category/InstanceName" and enumerate literally everything
# the game considers save-worthy state - see recoil_re_log.md for the full
# category catalog and what each one is believed to represent. The internal
# byte layout of each category's data blob is NOT yet decoded by this parser -
# it only extracts the container structure (names, offsets, sizes, raw bytes),
# which is the part that's fully confirmed. Decoding e.g. VehicleList's 128-byte
# record or Mission/MissionData's 92 bytes is a separate, not-yet-done task.

def parse(path, verbose=True):
    with open(path, "rb") as f:
        data = f.read()
    size = len(data)
    magic, count = struct.unpack("<II", data[-8:])
    assert magic == 1, f"bad savegame footer magic {magic} (expected 1) in {path}"
    toc_size = count * 0x94
    toc_start = size - 8 - toc_size
    assert toc_start >= 0, f"TOC doesn't fit in file: {path}"

    entries = []
    running = 0
    for i in range(count):
        entry = data[toc_start + i * 0x94 : toc_start + (i + 1) * 0x94]
        data_offset, data_size = struct.unpack("<II", entry[0:8])
        name = entry[8:8 + 68].split(b"\x00")[0].decode("ascii", errors="replace")
        assert data_offset == running, (
            f"non-contiguous entry {i} ({name}) in {path}: "
            f"expected offset {running}, got {data_offset}"
        )
        payload = data[data_offset : data_offset + data_size]
        entries.append((name, data_offset, data_size, payload))
        running += data_size

    assert running == toc_start, f"data section doesn't exactly fill up to TOC start in {path}"

    if verbose:
        print(f"=== {path} ===")
        print(f"size={size} entryCount={count} toc_start={toc_start}")
        cats = OrderedDict()
        for name, _off, sz, _payload in entries:
            cat = name.split("/")[0]
            cats.setdefault(cat, []).append(sz)
        for cat, sizes in cats.items():
            print(f"  {cat}: {len(sizes)} entries, sizes {sorted(set(sizes))}")

    return entries


if __name__ == "__main__":
    for p in sorted(glob.glob(os.path.join(os.environ.get("RECOIL_GAME_DIR", "game"), "savedgames", "*.sav"))):
        try:
            parse(p, verbose=True)
        except Exception as e:
            print(f"{os.path.basename(p)}: FAILED - {e}")
        print()
