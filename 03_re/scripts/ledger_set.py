"""ledger_set.py JSONFILE - apply {addr: [name, spec_ref, notes]} as CONFIRMED rows.

notes may be a string or a list of strings (joined with no separator), so long
notes can be split across lines without breaking JSON.
"""
import csv, io, json, sys

U = json.load(open(sys.argv[1], encoding='utf-8'))
p = '03_re/ledger/functions.csv'
rows = list(csv.reader(io.open(p, encoding='utf-8', newline='')))
seen = set()
for r in rows:
    if r and r[0] in U:
        name, spec, notes = U[r[0]]
        if isinstance(notes, list):
            notes = ''.join(notes)
        r[1], r[7], r[12] = name, spec, notes
        r[3] = 'CONFIRMED'
        r[10] = ''
        seen.add(r[0])
for a in sorted(set(U) - seen):
    name, spec, notes = U[a]
    if isinstance(notes, list):
        notes = ''.join(notes)
    row = [''] * len(rows[0])
    row[0], row[1], row[2], row[3], row[7], row[12] = a, name, 'ENGINE', 'CONFIRMED', spec, 'NEW ROW (missing from the Ghidra export; found by ledger_gaps.py). ' + notes
    rows.append(row); seen.add(a)
for r in rows:
    if r:
        r += [''] * (len(rows[0]) - len(r))
hdr, body = rows[0], sorted((r for r in rows[1:] if r), key=lambda r: int(r[0], 16) if r[0].startswith('0x') else -1)
rows = [hdr] + body
csv.writer(io.open(p, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows(rows)
print('confirmed', len(seen), 'missing', sorted(set(U) - seen))
