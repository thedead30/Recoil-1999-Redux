"""vtable_scan.py [--min N] [--family VTABLE] - find vtables in .rdata.

A vtable candidate is a run of >= N (default 4) consecutive dwords that all
point into .text and that is referenced (as an immediate) from .text, i.e. some
constructor writes it. Prints each vtable, slot count and slot targets.

--family VTABLE prints every vtable sharing at least 40% of its slots with the
given one (derived/base classes), plus the union of their slot targets - the
seed list for a class-hierarchy subsystem.
"""
import struct, sys

EXE = '00_original/game_install/Recoil.exe'
d = open(EXE, 'rb').read()
pe = struct.unpack_from('<I', d, 0x3c)[0]
ns = struct.unpack_from('<H', d, pe + 6)[0]
oh = struct.unpack_from('<H', d, pe + 20)[0]
secs = []
for i in range(ns):
    o = pe + 24 + oh + 40 * i
    name = d[o:o + 8].rstrip(b'\0').decode(errors='replace')
    va = 0x400000 + struct.unpack_from('<I', d, o + 12)[0]
    rs = struct.unpack_from('<I', d, o + 16)[0]
    ro = struct.unpack_from('<I', d, o + 20)[0]
    secs.append((name, va, rs, ro))
text = next(s for s in secs if s[0] == '.text')
rdata = next(s for s in secs if s[0] == '.rdata')
tlo, thi = text[1], text[1] + text[2]

args = sys.argv[1:]
mn = int(args[args.index('--min') + 1]) if '--min' in args else 4
fam = int(args[args.index('--family') + 1], 16) if '--family' in args else None

# immediates in .text (mov [reg], imm32 / push imm32 / mov reg, imm32): collect all dword values
tb = d[text[3]:text[3] + text[2]]
imm = set(struct.unpack_from('<I', tb, i)[0] for i in range(len(tb) - 3))

vts = {}
_, rva, rsz, rro = rdata
i = 0
while i < rsz - 3:
    x = struct.unpack_from('<I', d, rro + i)[0]
    if tlo <= x < thi:
        j = i
        slots = []
        while j < rsz - 3:
            y = struct.unpack_from('<I', d, rro + j)[0]
            if not (tlo <= y < thi):
                break
            slots.append(y)
            j += 4
        # a run may contain several adjacent vtables; split at referenced starts
        starts = [k for k in range(len(slots)) if (rva + i + 4 * k) in imm] or [0]
        for a, b in zip(starts, starts[1:] + [len(slots)]):
            if b - a >= mn and (rva + i + 4 * a) in imm:
                vts[rva + i + 4 * a] = slots[a:b]
        i = j
    else:
        i += 4

if fam is None:
    for v, s in sorted(vts.items()):
        print('0x%08x %3d  %s' % (v, len(s), ' '.join('%x' % x for x in s[:8])))
    print('vtables:', len(vts))
else:
    base = set(vts.get(fam, []))
    if not base:
        sys.exit('no vtable at 0x%08x' % fam)
    members = set()
    for v, s in sorted(vts.items()):
        share = len(base & set(s)) / max(1, min(len(base), len(s)))
        if share >= 0.4:
            own = sorted(set(s) - base)
            print('0x%08x %3d slots  share %.2f  own: %s' % (v, len(s), share, ' '.join('%x' % x for x in own)))
            members |= set(s)
    print('UNION', len(members), ' '.join('0x%08x' % x for x in sorted(members)))
