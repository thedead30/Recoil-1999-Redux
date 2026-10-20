"""unconf_by_size.py SUB [N] - list unconfirmed members of SUB smallest first (address:bytes)."""
import csv, glob, io, re, sys
sub = sys.argv[1]; n = int(sys.argv[2]) if len(sys.argv) > 2 else 999
F = {r['address'].lower(): r for r in csv.DictReader(io.open('03_re/ledger/functions.csv', encoding='utf-8', newline=''))}
M = {'0x' + r['address'].lower().replace('0x', '').zfill(8) for r in csv.DictReader(io.open('03_re/ledger/subsystems.csv', encoding='utf-8')) if r['subsystem'] == sub}
out = []
for m in M:
    if F.get(m, {}).get('status') != 'CONFIRMED':
        fs = glob.glob('03_re/decomp_raw/%s_*.c' % m)
        b = int(re.search(r'bytes=(\d+)', open(fs[0], errors='replace').read()).group(1)) if fs else 99999
        out.append((b, m))
print(' '.join('%s:%d' % (m, b) for b, m in sorted(out)[:n]))
