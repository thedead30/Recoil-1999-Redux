"""register_data_targets.py - register the code that the data image points at but the ledger has no row for.

The boot probe stops when a data pointer (vtable slot, MFC message-map entry, callback table) reaches code the ledger does not
know - such functions were never found in Stage 1 (nothing calls them directly). Two steps:
  list                -> 03_re/staging/boot/data_targets_in.txt ("<addr> <name>" for ~/ghidra_scripts/CreateDumpFunctions.java) and
                         data_targets.json (each target with the data words that point at it, and the kind of table they sit in)
  register DUMP.txt   -> add_gap_functions.py rows: the note quotes the instructions read from the listing and names the pointing
                         data words; the subsystem is taken from the nearest ledger neighbours (INFERRED when they disagree)
Names are descriptive defaults (<Kind>_<addr>) that a later read can refine; port_batch refuses FUN_ names, not these.
usage: python tools/register_data_targets.py list | register DUMP.txt
"""
import bisect, collections, csv, io, json, os, re, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
ST = os.path.join(ROOT, '03_re', 'staging', 'boot')
JS = os.path.join(ST, 'data_targets.json')


def ledger():
    return {r['address'].lower(): r for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))}


def kind_of(va, words):
    """message-map entries are 24 bytes with the handler at +0x14; vtables are runs of consecutive code pointers"""
    return 'MsgHandler' if any(((w - 0x14) % 0x18 == 0) for w in [0]) and False else ('VSlot' if len(words) else 'DataTarget')


def list_targets():
    t = io.open(os.path.join(ROOT, '05_remake', 'src', 'platform', 'image', 'original_data.cpp'), encoding='utf-8').read()
    led = ledger()
    rel = collections.defaultdict(list)
    for va, w in re.findall(r'\{0x([0-9a-f]{8}), 0x([0-9a-f]{8}), 3\}', t):
        if '0x' + w not in led:
            rel['0x' + w].append('0x' + va)
    os.makedirs(ST, exist_ok=True)
    out = {}
    for w, vas in sorted(rel.items()):
        # an MFC AFX_MSGMAP_ENTRY is {nMessage, nCode, nID, nLastID, nSig, pfn}: pfn at +0x14 and nSig (small) at +0x10
        out[w] = {'refs': vas, 'name': 'DataTarget_%s' % w[2:]}
    json.dump(out, io.open(JS, 'w', encoding='utf-8'), indent=1)
    with io.open(os.path.join(ST, 'data_targets_in.txt'), 'w', encoding='utf-8', newline='\n') as f:
        for w, d in sorted(out.items()):
            f.write('%s %s\n' % (w, d['name']))
    print('%d code targets without a ledger row -> %s' % (len(out), JS))


def register(dump):
    tg = json.load(io.open(JS, encoding='utf-8'))
    led = ledger()
    mem = {m['address'].lower(): m['subsystem'] for m in csv.DictReader(io.open(os.path.join(ROOT, '03_re', 'ledger', 'subsystems.csv'), encoding='utf-8'))}
    keys = sorted(int(a, 16) for a in mem if a.startswith('0x'))
    chunks = {}
    for c in io.open(dump, encoding='utf-8', errors='replace').read().replace('\r\n', '\n').split('### ')[1:]:
        chunks['0x' + c[:8]] = c
    rows = {}
    for w, d in sorted(tg.items()):
        c = chunks.get(w)
        if not c:
            print('  not in dump: %s' % w)
            continue
        lines = [l.split(' ', 1)[1] for l in c.split('\n')[1:] if l.strip()]
        v = int(w, 16)
        i = bisect.bisect(keys, v)
        near = [mem['0x%08x' % k] for k in keys[max(0, i - 2):i + 2]]
        sub = collections.Counter(near).most_common(1)[0][0]
        agree = len(set(near)) == 1
        shown = '; '.join(lines[:8]) + (' ... (%d instructions)' % len(lines) if len(lines) > 8 else '')
        rows[w] = {'name': d['name'], 'subsystem': sub, 'notes': (
            '%d instructions, listing read (Ghidra function created by CreateDumpFunctions.java): %s. Reached only through data: '
            'pointed at by %s (vtable / MFC message-map / callback table words the data image relocates). Subsystem %s from the '
            'address neighbours %s%s.' % (len(lines), shown, ', '.join(d['refs'][:6]), sub, near, '' if agree else ' - INFERRED, the neighbours disagree'))}
    p = os.path.join(ST, 'data_targets_rows.json')
    json.dump(rows, io.open(p, 'w', encoding='utf-8'), indent=1)
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'add_gap_functions.py'), p, '--listing', dump, '--source',
                        'the boot probe (code reached only through data: vtable / message-map / callback words)', '--who', '2026-09-30 opus'],
                       capture_output=True, text=True)
    print(r.stdout.strip(), r.stderr.strip()[-400:])


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == 'list':
        list_targets()
    elif len(sys.argv) > 2 and sys.argv[1] == 'register':
        register(sys.argv[2])
    else:
        sys.exit(__doc__)
