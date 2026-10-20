"""gate_gap.py SUBSYSTEM - print unconfirmed members and uncovered ENGINE callees (compact)."""
import csv, io, sys, collections
sub = sys.argv[1]
F = {r['address'].lower(): r for r in csv.DictReader(io.open('03_re/ledger/functions.csv', encoding='utf-8', newline=''))}
k = lambda a: '0x' + a.lower().replace('0x', '').zfill(8)
M = {k(r['address']) for r in csv.DictReader(io.open('03_re/ledger/subsystems.csv', encoding='utf-8')) if r['subsystem'] == sub}
ce = collections.defaultdict(set)
for line in open('03_re/ledger/_callgraph.tsv'):
    q = line.split()
    if len(q) == 2:
        try: ce['0x%08x' % int(q[0], 16)].add('0x%08x' % int(q[1], 16))
        except ValueError: pass
un = sorted(m for m in M if F.get(m, {}).get('status') != 'CONFIRMED')
ext = set().union(*[ce[m] for m in M]) - M
bad = sorted(c for c in ext if F.get(c, {}).get('status') != 'CONFIRMED' and F.get(c, {}).get('class') == 'ENGINE')
print('%s %d/%d' % (sub, len(M) - len(un), len(M)))
print('UNCONF', ' '.join(un))
print('UNCOV', ' '.join(bad))
