"""add_gap_functions.py - register functions that had no Ghidra function / ledger row (Stage 1 completeness, checklist B0).

Input JSON: {"0x0041bca0": {"name": "...", "subsystem": "player", "notes": "...", "spec": "../../04_spec/systems/x.md"}, ...}
plus --listing FILE: the listing dump of those functions ("### <addr8> <name>" then "<addr> <instruction>" lines, the format of
DumpListingsFile.java) whose chunks are appended to 03_re/listings/unported/<subsystem>.txt.
For each address that is not yet a ledger row (idempotent):
  1. ledger row in 03_re/ledger/functions.csv: ENGINE, CONFIRMED, spec_ref, notes = "NEW ROW (missing from the Ghidra export; found by
     ledger_gaps.py). " + notes, and the verify plan copied from a sibling gap row (the default ORACLE plan; assign_verify_plan.py is NOT
     rerun: it re-derives plans from names for every row);
  2. membership row in 03_re/ledger/subsystems.csv in the GAP MEMBER format;
  3. the function's listing chunk appended to 03_re/listings/unported/<subsystem>.txt (so port_ready / port_loop can see it).
CONFIRMED needs evidence: every note must say what the bytes / decompile show - the tool refuses an empty or very short note.
It never renames Ghidra functions (do that with rename_function first) and never touches existing rows.
usage: python tools/add_gap_functions.py rows.json --listing dump.txt
"""
import csv, io, json, os, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
LED = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')
SUB = os.path.join(ROOT, '03_re', 'ledger', 'subsystems.csv')
SIBLING = '0x004a8220'  # a gap row whose verify plan is the harvest default
# --source TEXT: how the gap was found (default the gap_members.py sweep); --who TEXT: date + model for the membership note
SOURCE = sys.argv[sys.argv.index('--source') + 1] if '--source' in sys.argv else 'ledger_gaps.py / gap_members.py'
WHO = sys.argv[sys.argv.index('--who') + 1] if '--who' in sys.argv else '2026-09-29 sonnet'


def main():
    spec_rows = {'0x%08x' % int(k, 16): v for k, v in json.load(open(sys.argv[1], encoding='utf-8')).items()}
    listing = open(sys.argv[sys.argv.index('--listing') + 1], encoding='utf-8', errors='replace').read().replace('\r\n', '\n')
    chunks = {}
    for c in listing.split('### ')[1:]:
        chunks['0x' + c.split()[0].lstrip('0').rjust(8, '0')[-8:]] = '### ' + c.rstrip('\n') + '\n\n'
    rows = list(csv.reader(io.open(LED, encoding='utf-8', newline='')))
    hdr = rows[0]
    ip, ir = hdr.index('verify_plan'), hdr.index('verify_plan_reason')
    have = {r[0] for r in rows if r}
    sib = next(r for r in rows if r and r[0] == SIBLING)
    added, skipped = [], []
    for a, d in sorted(spec_rows.items()):
        if a in have:
            skipped.append(a)
            continue
        if len(d.get('notes', '')) < 80:
            sys.exit('%s: the note must say what the bytes show (>= 80 chars)' % a)
        if a not in chunks:
            sys.exit('%s: not in the listing dump' % a)
        row = [''] * len(hdr)
        row[0], row[1], row[2], row[3] = a, d['name'], 'ENGINE', 'CONFIRMED'
        row[7] = d.get('spec', '../../04_spec/systems/%s.md' % d['subsystem'])
        row[12] = 'NEW ROW (missing from the Ghidra export; found by %s). ' % SOURCE + d['notes']
        row[ip], row[ir] = sib[ip], sib[ir]
        rows.append(row)
        added.append(a)
    if added:
        body = sorted((r for r in rows[1:] if r), key=lambda r: int(r[0], 16) if r[0].startswith('0x') else -1)
        csv.writer(io.open(LED, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows([hdr] + body)
        mem = open(SUB, encoding='utf-8', newline='').read()
        nl = '\r\n' if '\r\n' in mem else '\n'
        add = ''.join('%s,%s,%s,"GAP MEMBER (%s: referenced by a %s member by call or address-taken; no Ghidra function existed: '
                      'read via Ghidra create_function + listing, %s)"%s' % (
                          spec_rows[a]['subsystem'], a, spec_rows[a]['name'], SOURCE, spec_rows[a]['subsystem'], WHO, nl) for a in added)
        open(SUB, 'a', encoding='utf-8', newline='').write(('' if mem.endswith(('\n', '\r\n')) else nl) + add)
        for a in added:
            d = spec_rows[a]
            lp = os.path.join(ROOT, '03_re', 'listings', 'unported', d['subsystem'] + '.txt')
            raw = open(lp, encoding='utf-8', newline='').read() if os.path.exists(lp) else ''
            lnl = '\r\n' if '\r\n' in raw else '\n'
            head = '### %s ' % a[2:]
            if head in raw:
                continue
            body = chunks[a].split('\n', 1)[1]  # the header line carries the ledger name, not Ghidra's FUN_ default
            text = ('### %s %s\n' % (a[2:], d['name']) + body).replace('\n', lnl)
            open(lp, 'w', encoding='utf-8', newline='').write(raw + ('' if not raw or raw.endswith(lnl) else lnl) + text)
    print('added %d, skipped (already rows) %d' % (len(added), len(skipped)))


if __name__ == '__main__':
    main()
