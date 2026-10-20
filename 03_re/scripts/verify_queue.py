"""verify_queue.py - ported functions still waiting for local verification (cloud-written or otherwise).

usage: python 03_re/scripts/verify_queue.py [--all]

Lists ledger rows whose remake_verification starts with IMPLEMENTED-UNVERIFIED, grouped by remake file, with the test
named in the note when there is one. A local (Windows) session works this queue: build, run the named test, mutation
check, then record VERIFIED-ORACLE with remake_set.py (see 03_re/CLOUD.md). Default: cloud-tagged rows only.
"""
import csv, os, re, sys, collections

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
rows = [r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))
        if r.get('remake_verification', '').startswith('IMPLEMENTED-UNVERIFIED')]
if '--all' not in sys.argv:
    rows = [r for r in rows if 'cloud' in r['remake_verification'].lower()]
by = collections.defaultdict(list)
for r in rows:
    by[r['remake_file']].append(r)
print('%d function(s) waiting for local verification' % len(rows))
for f, rs in sorted(by.items()):
    print(f)
    for r in rs:
        t = re.search(r'\b(native_\w+|test_\w+\.cpp)', r['remake_verification'])
        print('  %s %-40s %s' % (r['address'], r['ghidra_name'], t.group(1) if t else '(no test named)'))
