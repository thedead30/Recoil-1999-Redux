"""apply_name_refinements.py - apply queued name / note refinements to the ledger (run only while port_loop is NOT running).

Input JSON: {"0x0043ff80": {"old": "OldName", "new": "NewName", "note": "text"}, ...} (keys starting with '_' are ignored).
For each entry (idempotent): 03_re/ledger/functions.csv ghidra_name old -> new and the note text is put in front of the notes; the member row in
03_re/ledger/subsystems.csv and the '### addr name' headers in 03_re/listings/**/*.txt are renamed. The Ghidra rename is done separately
(rename_function). Refuses when the row's current name is neither old nor new, and when the new name is already used by another row.
usage: python tools/apply_name_refinements.py FILE.json
"""
import csv, glob, io, json, os, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def main():
    d = {k: v for k, v in json.load(open(sys.argv[1], encoding='utf-8')).items() if not k.startswith('_')}
    led = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')
    rows = list(csv.reader(io.open(led, encoding='utf-8', newline='')))
    h = rows[0]
    ia, ih = h.index('address'), h.index('ghidra_name')
    inote = h.index('notes')
    names = {r[ih]: r[ia] for r in rows if r}
    changed = []
    for r in rows:
        if not r or r[ia] not in d:
            continue
        e = d[r[ia]]
        if r[ih] == e['new'] and e['note'] in r[inote]:
            continue
        if r[ih] not in (e['old'], e['new']):
            sys.exit('%s: current name %s is neither %s nor %s' % (r[ia], r[ih], e['old'], e['new']))
        if e['new'] in names and names[e['new']] != r[ia]:
            sys.exit('%s: the name %s is already used by %s' % (r[ia], e['new'], names[e['new']]))
        r[ih] = e['new']
        if e['note'] not in r[inote]:
            r[inote] = 'REFINED 2026-09-29: ' + e['note'] + ' | previous note: ' + r[inote]
        changed.append(r[ia])
    if changed:
        csv.writer(io.open(led, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows(rows)
    sub = os.path.join(ROOT, '03_re', 'ledger', 'subsystems.csv')
    s = io.open(sub, encoding='utf-8', newline='').read()
    for a, e in d.items():
        s = s.replace(',%s,%s,' % (a, e['old']), ',%s,%s,' % (a, e['new']))
    io.open(sub, 'w', encoding='utf-8', newline='').write(s)
    for lp in glob.glob(os.path.join(ROOT, '03_re', 'listings', '**', '*.txt'), recursive=True):
        t = io.open(lp, encoding='utf-8', newline='').read()
        u = t
        for a, e in d.items():
            u = u.replace('### %s %s' % (a[2:], e['old']), '### %s %s' % (a[2:], e['new']))
        if u != t:
            io.open(lp, 'w', encoding='utf-8', newline='').write(u)
    print('refined %d row(s): %s' % (len(changed), ' '.join(changed)))


if __name__ == '__main__':
    main()
