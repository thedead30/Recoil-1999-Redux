"""triage_gaps.py - classify ledger_gaps.py candidates without launching a debugger.
For each candidate: first bytes, a prologue verdict, direct E8/E9 caller count, absolute-pointer
reference count (e.g. vtables, atexit, callbacks), and whether it lies in a registered subsystem's
address span. Writes 03_re/ledger/_gap_candidates.csv and prints a summary."""
import csv, io, re, struct, subprocess, sys, collections
d = open('00_original/game_install/Recoil.exe', 'rb').read()
pe = struct.unpack_from('<I', d, 0x3C)[0]; ns = struct.unpack_from('<H', d, pe + 6)[0]
oh = struct.unpack_from('<H', d, pe + 20)[0]; base = struct.unpack_from('<I', d, pe + 52)[0]
for i in range(ns):
    o = pe + 24 + oh + 40 * i
    if d[o:o + 5] == b'.text':
        v0 = base + struct.unpack_from('<I', d, o + 12)[0]; ro = struct.unpack_from('<I', d, o + 20)[0]
        rs = struct.unpack_from('<I', d, o + 16)[0]
fo = lambda va: ro + va - v0
out = subprocess.run([sys.executable, '03_re/scripts/ledger_gaps.py'], capture_output=True, text=True).stdout.split('\n')
cands = [int(x, 16) for x in out[1].split()] if len(out) > 1 else []
# ledger_gaps prints only 60; rerun without truncation
src = open('03_re/scripts/ledger_gaps.py').read().replace("out[:60]", "out")
ns_ = {}
sys_argv = sys.argv; sys.argv = ['x']
exec(compile(src, 'ledger_gaps', 'exec'), ns_)
sys.argv = sys_argv
cands = ns_['out']
# caller scan: one pass over .text building target -> count
calls = collections.Counter()
for f in range(ro, ro + rs - 5):
    if d[f] in (0xE8, 0xE9):
        calls[v0 + f - ro + 5 + struct.unpack_from('<i', d, f + 1)[0]] += 1
ptrs = collections.Counter()
cs = set(cands)
for f in range(0, len(d) - 4, 1):
    v = struct.unpack_from('<I', d, f)[0]
    if v in cs:
        ptrs[v] += 1
PRO = [b'\x55\x8b\xec', b'\x83\xec', b'\x81\xec', b'\x56', b'\x53', b'\x57', b'\x51', b'\x55', b'\x8b', b'\xa1', b'\x6a', b'\x68', b'\xb8', b'\xd9', b'\x33', b'\xe9', b'\xc7']
spans = collections.defaultdict(list)
for r in csv.DictReader(io.open('03_re/ledger/subsystems.csv', encoding='utf-8')):
    spans[r['subsystem']].append(int(r['address'], 16))
span = {k: (min(v), max(v)) for k, v in spans.items()}
rows = []
for c in cands:
    b = d[fo(c):fo(c) + 6]
    pro = any(b.startswith(p) for p in PRO)
    insp = [k for k, (lo, hi) in span.items() if lo <= c <= hi]
    rows.append(['0x%08x' % c, b.hex(), 'likely' if pro else 'check', calls[c], ptrs[c], ';'.join(insp)])
with io.open('03_re/ledger/_gap_candidates.csv', 'w', encoding='utf-8', newline='') as fh:
    w = csv.writer(fh, lineterminator='\n'); w.writerow(['address', 'bytes', 'prologue', 'direct_calls', 'abs_refs', 'in_span'])
    w.writerows(rows)
n = len(rows)
lik = sum(1 for r in rows if r[2] == 'likely')
ref = sum(1 for r in rows if r[3] or r[4])
ins = collections.Counter(s for r in rows for s in r[5].split(';') if s)
print('candidates %d, prologue-likely %d, referenced (call or pointer) %d' % (n, lik, ref))
print('in subsystem spans:', dict(ins))
print('unreferenced+no-prologue (probably data/padding):', sum(1 for r in rows if r[2] == 'check' and not r[3] and not r[4]))
