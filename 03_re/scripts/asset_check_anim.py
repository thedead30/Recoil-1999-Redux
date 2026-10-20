"""asset_check_anim.py - MEASURED-ASSET check of the anim.zbd layout against every shipped file.

Read order is the one in Anim_LoadFile 0x0045efb0 (03_re/decomp_raw/0x0045efb0_Anim_LoadFile.c).
A file passes when the sequential parse lands exactly on end-of-file.
usage: python 03_re/scripts/asset_check_anim.py
"""
import glob
import os
import struct

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
TABLES = [(0x105, 0x60), (0x106, 0x28), (0x107, 0x2c), (0x108, 0x2c), (0x109, 0x24),
          (0x10a, 0x24), (0x10b, 0x30), (0x10d, 0x48)]


def parse(p):
    b = open(p, "rb").read()
    magic, ver, cnt = struct.unpack_from("<III", b, 0)
    o = 12 + cnt * 0x54                       # manifest records
    nn = struct.unpack_from("<H", b, o + 0xa)[0]
    n2 = struct.unpack_from("<I", b, o + 0x10)[0]
    o += 0x3c                                 # world header
    for _ in range(nn):
        s = b[o:o + 0x134]
        o += 0x134                            # node struct
        for off, sz in TABLES:
            o += s[off] * sz
        blk = b[o:o + 0x40]
        o += 0x40                             # always read into node +0xc4
        bs = struct.unpack_from("<i", blk, 0x100 - 0xc4)[0]
        o += max(bs, 0)                       # blob, size = node +0x100
        for _k in range(s[0x104]):
            e = b[o:o + 0x40]
            o += 0x40 + max(struct.unpack_from("<i", e, 0x3c)[0], 0)
    o += n2 * 0x24                            # trailing records
    return magic, ver, cnt, nn, n2, o == len(b)


def main():
    files = sorted(glob.glob(os.path.join(ROOT, "00_original", "game_install", "zbd", "*", "anim.zbd")))
    ok = 0
    for p in files:
        m, v, c, nn, n2, good = parse(p)
        ok += good and m == 0x08170616 and v == 0x1c
        print("%-28s magic %08x ver %d manifest %3d nodes %4d trailing %d %s" % (
            os.path.relpath(p, ROOT), m, v, c, nn, n2, "OK" if good else "MISMATCH"))
    print("%d/%d parse to end-of-file" % (ok, len(files)))


if __name__ == "__main__":
    main()
