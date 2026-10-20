"""find_callers.py ADDR... - scan the EXE for E8 (call rel32) and E9 (jmp rel32) instructions whose
target is ADDR. Catches callers missing from _callgraph.tsv. Byte-pattern scan: a hit can in
principle be data aliasing; confirm with the disassembly of the reported site."""
import struct, sys
d = open('00_original/game_install/Recoil.exe', 'rb').read()
pe = struct.unpack_from('<I', d, 0x3C)[0]; ns = struct.unpack_from('<H', d, pe + 6)[0]
oh = struct.unpack_from('<H', d, pe + 20)[0]; base = struct.unpack_from('<I', d, pe + 52)[0]
for i in range(ns):
    o = pe + 24 + oh + 40 * i
    if d[o:o + 5] == b'.text':
        v0 = base + struct.unpack_from('<I', d, o + 12)[0]; ro = struct.unpack_from('<I', d, o + 20)[0]
        rs = struct.unpack_from('<I', d, o + 16)[0]
for a in sys.argv[1:]:
    t = int(a, 16); hits = []
    for fo in range(ro, ro + rs - 5):
        op = d[fo]
        if op in (0xE8, 0xE9):
            site = v0 + fo - ro
            if site + 5 + struct.unpack_from('<i', d, fo + 1)[0] == t:
                hits.append('%s@0x%08x' % ('call' if op == 0xE8 else 'jmp', site))
    print(a, len(hits), ' '.join(hits[:20]))
