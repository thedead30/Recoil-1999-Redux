"""gap_members.py - for every gap candidate, find its direct (E8/E9) and absolute-pointer referrers,
map each referrer site to its containing ledger row, and report candidates referenced from a
registered subsystem member. Those are the ones that can affect a gate."""
import bisect, csv, io, struct, collections
d = open('00_original/game_install/Recoil.exe', 'rb').read()
pe = struct.unpack_from('<I', d, 0x3C)[0]; ns = struct.unpack_from('<H', d, pe + 6)[0]
oh = struct.unpack_from('<H', d, pe + 20)[0]; base = struct.unpack_from('<I', d, pe + 52)[0]
secs = []
for i in range(ns):
    o = pe + 24 + oh + 40 * i
    secs.append((d[o:o + 8].rstrip(b'\0'), base + struct.unpack_from('<I', d, o + 12)[0],
                 struct.unpack_from('<I', d, o + 20)[0], struct.unpack_from('<I', d, o + 16)[0]))
text = [s for s in secs if s[0] == b'.text'][0]
_, v0, ro, rs = text
def va(fo):
    for _, sv, sro, srs in secs:
        if sro <= fo < sro + srs: return sv + fo - sro
C = [int(r['address'], 16) for r in csv.DictReader(open('03_re/ledger/_gap_candidates.csv'))]
cs = set(C)
L = sorted(int(r[0], 16) for r in csv.reader(io.open('03_re/ledger/functions.csv', encoding='utf-8')) if r and r[0].startswith('0x'))
starts = sorted(set(L) | cs)
sub = collections.defaultdict(set)
for r in csv.DictReader(io.open('03_re/ledger/subsystems.csv', encoding='utf-8')):
    sub[int(r['address'], 16)].add(r['subsystem'])
def owner(site):
    i = bisect.bisect_right(starts, site) - 1
    return starts[i] if i >= 0 else None
refs = collections.defaultdict(set)
for f in range(ro, ro + rs - 5):
    if d[f] in (0xE8, 0xE9):
        t = v0 + f - ro + 5 + struct.unpack_from('<i', d, f + 1)[0]
        if t in cs: refs[t].add(('call', owner(v0 + f - ro)))
for f in range(ro, ro + rs - 4):
    v = struct.unpack_from('<I', d, f)[0]
    if v in cs: refs[v].add(('ptr', owner(v0 + f - ro)))
hits = 0
for c in C:
    subs = set()
    for kind, o in refs[c]:
        subs |= sub.get(o, set())
    if subs:
        hits += 1
        print('0x%08x <- %s via %s' % (c, ','.join(sorted(subs)), ' '.join('%s:0x%08x' % (k, o) for k, o in sorted(refs[c]) if sub.get(o))))
print('candidates referenced from subsystem members:', hits)
