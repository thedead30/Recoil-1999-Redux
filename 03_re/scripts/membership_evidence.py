"""membership_evidence.py SUB LO HI GLO GHI - for non-member ENGINE functions in [LO,HI), score membership
evidence from the decomp corpus: references to globals in [GLO,GHI), calls to members, callers that are members.
Prints one compact line per function."""
import csv, glob, io, os, re, sys, collections
REG = '--register' in sys.argv
if REG: sys.argv.remove('--register')
sub = sys.argv[1]; lo, hi, glo, ghi = (int(x, 16) for x in sys.argv[2:6])
F = {r['address'].lower(): r for r in csv.DictReader(io.open('03_re/ledger/functions.csv', encoding='utf-8', newline=''))}
k = lambda a: '0x' + a.lower().replace('0x', '').zfill(8)
M = {k(r['address']) for r in csv.DictReader(io.open('03_re/ledger/subsystems.csv', encoding='utf-8')) if r['subsystem'] == sub}
ce = collections.defaultdict(set); cr = collections.defaultdict(set)
for line in open('03_re/ledger/_callgraph.tsv'):
    q = line.split()
    if len(q) == 2:
        try:
            a, b = '0x%08x' % int(q[0], 16), '0x%08x' % int(q[1], 16)
            ce[a].add(b); cr[b].add(a)
        except ValueError:
            pass
for a in sorted(F):
    v = int(a, 16)
    if not (lo <= v < hi) or a in M or F[a]['class'] != 'ENGINE':
        continue
    fs = glob.glob('03_re/decomp_raw/%s_*.c' % a)
    txt = open(fs[0], encoding='utf-8', errors='replace').read() if fs else ''
    g = sorted({x for x in re.findall(r'DAT_00([0-9a-f]{6})', txt) if glo <= int(x, 16) < ghi})
    cm = len(ce[a] & M); crm = len(cr[a] & M)
    score = len(g) + cm + crm
    print('%s %-10s g=%d callsM=%d calledByM=%d callers=%d %s%s' % (
        a, F[a]['status'][:10], len(g), cm, crm, len(cr[a]),
        F[a]['ghidra_name'][:28], '  NO-EVIDENCE' if score == 0 else ''))


def register_to_fixpoint():
    """--register: append evidence-bearing non-members to subsystems.csv, repeating until no new ones."""
    import subprocess
    total = 0
    while True:
        out = subprocess.run([sys.executable, __file__] + sys.argv[1:6], capture_output=True, text=True).stdout
        new = [l for l in out.splitlines() if l.strip() and 'NO-EVIDENCE' not in l]
        if not new:
            break
        p = '03_re/ledger/subsystems.csv'
        rows = list(csv.reader(io.open(p, encoding='utf-8', newline='')))
        for l in new:
            a = l.split()[0]
            ev = ' '.join(t for t in l.split() if '=' in t)
            rows.append([sub, a, l.split()[-1], 'MEMBERSHIP SCAN (membership_evidence.py %s): %s' % (' '.join(sys.argv[2:6]), ev)])
        csv.writer(io.open(p, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows(rows)
        total += len(new)
    print('registered', total)


if REG:
    register_to_fixpoint()
