"""asset_check_gamez.py - parse every shipped gamez.zbd to EOF from the loader's own layout (KG-07).

Layout from the bytes (CONFIRMED-BINARY, 2026-09-27):
  GameZ_ReadHeader 0x004556a0: 0x24-byte header, [0] magic 0x02971222, [1] version 0xF.
  GameZ_LoadZbd 0x00455520: fseek [3] -> texture directory (0x0046d420), [4] -> materials (Material_ReadSection),
    [5] -> models (0x00481fa0), [8] -> node buffer (GameZ_ReadNodeBuffer 0x00455350 with count [6]);
    [7] -> 0x004de4c8 = the node free-list head (the allocator 0x004478c0 pops that index; next = low 24 bits of
    node +0xC0). [2] is not used by the loader.
  GameZ_ReadNodeBuffer: [6] nodes of 0xC4 bytes in one fread; then GameZ_ReadNodeData per node, in order; the runtime
    has-data flag 0x1000000 at +0xC0 is SET BY THE LOADER (ReadNodeData > 0) - it is never in the file.
  GameZ_ReadNodeData 0x00454c60, by class [+0x34]: 0 none; 1 camera 0x1E8; 2 world 0xAC + index lists of [0x90] and
    [0x9C] dwords + [0x7C] x [0x78] area partitions of 0x40 bytes, each followed by (short +0x3A) dwords; 3 window
    0xF8; 4 display 0x1C; 5 object3d 0x90; 6 lod 0x50; 9 light 0xE4 + [0xDC] dwords; 10 sound 0x94 + [0x8C] dwords;
    any other class is an error. Then [+0x54] parent and [+0x5C] child dwords (GameZ_ReadNodeIndexList 0x00454bf0).
Checks per file: header, section offsets ascending, node section parsed exactly to EOF, free list walk (length,
no cycle, every listed node empty), used + free = count. Prints one line per file; exit 1 on any failure.
"""
import glob, os, struct, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DATA = {1: 0x1E8, 3: 0xF8, 4: 0x1C, 5: 0x90, 6: 0x50}


def check(path):
    b = open(path, 'rb').read()
    h = struct.unpack_from('<9I', b, 0)
    errs = []
    if h[0] != 0x02971222 or h[1] != 0xF:
        return ['bad header %08x %x' % (h[0], h[1])], {}
    tex, mat, mdl, count, free_head, node_off = h[3], h[4], h[5], h[6], h[7], h[8]
    if not (0x24 <= tex < mat < mdl < node_off <= len(b)):
        errs.append('section offsets not ascending: %x %x %x %x' % (tex, mat, mdl, node_off))
    nodes = b[node_off:node_off + 0xC4 * count]
    if len(nodes) != 0xC4 * count:
        return errs + ['node array short'], {}
    pos = node_off + 0xC4 * count
    classes = {}
    used = 0

    def dwords(n):
        nonlocal pos
        pos += 4 * n

    for i in range(count):
        n = nodes[0xC4 * i:0xC4 * (i + 1)]
        cls = struct.unpack_from('<I', n, 0x34)[0]
        classes[cls] = classes.get(cls, 0) + 1
        if cls in DATA:
            pos += DATA[cls]
        elif cls == 2:
            w = b[pos:pos + 0xAC]
            pos += 0xAC
            n90, n9c, n78, n7c = (struct.unpack_from('<i', w, o)[0] for o in (0x90, 0x9C, 0x78, 0x7C))
            if n90 > 0: dwords(n90)
            if n9c > 0: dwords(n9c)
            for _ in range(max(n7c, 0) * max(n78, 0)):
                part = b[pos:pos + 0x40]
                pos += 0x40
                k = struct.unpack_from('<h', part, 0x3A)[0]
                if k > 0: dwords(k)
        elif cls == 9:
            k = struct.unpack_from('<i', b, pos + 0xDC)[0]
            pos += 0xE4
            if k > 0: dwords(k)
        elif cls == 10:
            k = struct.unpack_from('<i', b, pos + 0x8C)[0]
            pos += 0x94
            if k > 0: dwords(k)
        elif cls != 0:
            errs.append('node %d: unknown class %d' % (i, cls))
            break
        parents, children = struct.unpack_from('<ii', n, 0x54)[0], struct.unpack_from('<i', n, 0x5C)[0]
        if parents > 0: dwords(parents)
        if children > 0: dwords(children)
        if cls != 0:
            used += 1
        if struct.unpack_from('<I', n, 0xC0)[0] & 0x1000000:
            errs.append('node %d: has-data flag 0x1000000 present in the file' % i)
    if pos != len(b):
        errs.append('node data ends at %x, file is %x bytes' % (pos, len(b)))
    # free list: head [7], next = low 24 bits of +0xC0
    seen, cur = set(), free_head
    while cur < count and cur not in seen:
        seen.add(cur)
        n = nodes[0xC4 * cur:0xC4 * (cur + 1)]
        if struct.unpack_from('<I', n, 0x34)[0] != 0:
            errs.append('free-list node %d is not empty (class %d)' % (cur, struct.unpack_from('<I', n, 0x34)[0]))
            break
        cur = struct.unpack_from('<I', n, 0xC0)[0] & 0xFFFFFF
    if cur in seen:
        errs.append('free list cycles at %d' % cur)
    info = {'count': count, 'used': used, 'free': len(seen), 'end': cur, 'classes': classes, 'word2': h[2]}
    if used + len(seen) != count:
        errs.append('used %d + free %d != count %d' % (used, len(seen), count))
    return errs, info


def main():
    files = sorted(glob.glob(os.path.join(ROOT, '00_original', 'game_install', 'zbd', 'm*', 'gamez.zbd')))
    bad = 0
    for f in files:
        errs, info = check(f)
        rel = os.path.relpath(f, os.path.join(ROOT, '00_original', 'game_install'))
        print('%-20s %s %s' % (rel, 'OK ' if not errs else 'BAD', info if not errs else '; '.join(errs)))
        bad += bool(errs)
    print('%d files, %d bad' % (len(files), bad))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
