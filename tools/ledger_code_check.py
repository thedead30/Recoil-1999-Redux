"""ledger_code_check.py - every ledger row that claims a port must have that function in the source.

usage: python tools/ledger_code_check.py [--fix]

A row claims a port when remake_file / remake_symbol is set. The claim holds when some .cpp under 05_remake/src defines
the symbol (`__fastcall SYMBOL(` or a plain C++ definition `SYMBOL(` at line start). Rows whose code is missing are
listed; --fix clears remake_file / remake_symbol / remake_verification on them so port_loop.py ports them again.
(Found 2026-09-28: a tree-wide revert had removed code whose ledger rows survived, and callers then failed to compile.)
"""
import csv, glob, io, os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
src = ''.join(io.open(p, encoding='utf-8', errors='replace').read()
              for p in glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.cpp'), recursive=True))
defined = (set(re.findall(r'\b(?:__fastcall|__cdecl|__stdcall|__thiscall|WINAPI)\s+(\w+)\s*\(', src))
           | set(re.findall(r'__declspec\(naked\)\s+[\w\s\*]+?\b(\w+)\s*\(', src))
           | set(re.findall(r'^[\w:<>*& ]+?\b(\w+)\s*\([^;]*\)\s*(?:\{.*)?$', src, re.M)))
p = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')
rows = list(csv.reader(io.open(p, encoding='utf-8', newline='')))
h = rows[0]
fi, si, vi = h.index('remake_file'), h.index('remake_symbol'), h.index('remake_verification')
bad = []
for r in rows[1:]:
    if not (r[fi] or r[si]):
        continue
    sym = r[si].split('::')[-1] or r[1]
    if sym not in defined:
        bad.append(r)
print('%d ledger row(s) claim a port whose code is missing' % len(bad))
for r in bad[:15]:
    print('  %s %-36s %s' % (r[0], r[si] or r[1], r[fi]))
if bad and '--fix' in sys.argv:
    for r in bad:
        r[fi] = r[si] = r[vi] = ''
    csv.writer(io.open(p, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows(rows)
    print('cleared %d row(s); they will be ported again' % len(bad))
sys.exit(1 if bad and '--fix' not in sys.argv else 0)
