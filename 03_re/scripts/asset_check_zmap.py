"""asset_check_zmap.py - MEASURED-ASSET check of the .zmap layout against every shipped map.

Layout from MapScreen_ReadFile 0x004168d0 and MapMarker_ReadFile 0x00415bd0: u32 version (must be
5), u32 [+0x24], 0x18 bounds (6 floats), then markers until a short read: 3-byte colour, u32
point count, count x 0xc points, u32 state. Passes when the parse ends exactly at end-of-file.
usage: python 03_re/scripts/asset_check_zmap.py
"""
import glob
import hashlib
import os
import struct

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def main():
    files = sorted(glob.glob(os.path.join(ROOT, "00_original", "game_install", "maps", "*.zmap")))
    ok = 0
    for p in files:
        b = open(p, "rb").read()
        ver, x = struct.unpack_from("<II", b, 0)
        o, n, pts = 0x20, 0, 0
        while o + 7 <= len(b):
            c = struct.unpack_from("<I", b, o + 3)[0]
            if o + 7 + c * 12 + 4 > len(b):
                break
            o += 7 + c * 12 + 4
            n += 1
            pts += c
        good = ver == 5 and o == len(b)
        ok += good
        print("%-9s ver %d field24 %2d markers %2d points %3d md5 %s %s" % (
            os.path.basename(p), ver, x, n, pts, hashlib.md5(b).hexdigest()[:8], "OK" if good else "MISMATCH"))
    print("%d/%d parse to end-of-file" % (ok, len(files)))


if __name__ == "__main__":
    main()
