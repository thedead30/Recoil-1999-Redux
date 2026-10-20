"""ledger_gaps.py [LO HI] - find likely function starts missing from the ledger.
Heuristic: in .text, a byte after a ret (C3 / C2 xx xx) followed by NOP/INT3 padding up to an
address aligned to 0x10, where that aligned address is not a ledger row. Prints count and list.
Candidates must be confirmed by disassembly (padding can also precede jump tables/data)."""
import csv, io, struct, sys
d = open('00_original/game_install/Recoil.exe', 'rb').read()
pe = struct.unpack_from('<I', d, 0x3C)[0]; ns = struct.unpack_from('<H', d, pe + 6)[0]
oh = struct.unpack_from('<H', d, pe + 20)[0]; base = struct.unpack_from('<I', d, pe + 52)[0]
for i in range(ns):
    o = pe + 24 + oh + 40 * i
    if d[o:o + 5] == b'.text':
        v0 = base + struct.unpack_from('<I', d, o + 12)[0]; ro = struct.unpack_from('<I', d, o + 20)[0]
        rs = struct.unpack_from('<I', d, o + 16)[0]
L = {int(r[0], 16) for r in csv.reader(io.open('03_re/ledger/functions.csv', encoding='utf-8')) if r and r[0].startswith('0x')}
lo, hi = (int(sys.argv[1], 16), int(sys.argv[2], 16)) if len(sys.argv) > 2 else (v0, v0 + rs)
out = []
for fo in range(ro, ro + rs - 4):
    va = v0 + fo - ro
    if not (lo <= va < hi):
        continue
    b = d[fo]
    if b == 0xC3: end = fo + 1
    elif b == 0xC2 and d[fo + 2] == 0: end = fo + 3
    else: continue
    j = end
    while j < ro + rs and d[j] in (0x90, 0xCC) and (v0 + j - ro) % 0x10: j += 1
    t = v0 + j - ro
    if t % 0x10 == 0 and j > end and d[j] not in (0x90, 0xCC) and t not in L:
        out.append(t)
out = sorted(set(out))
print('candidates', len(out))
print(' '.join('0x%08x' % x for x in out[:60]))
