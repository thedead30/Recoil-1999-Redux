"""unport.py - remove the generated port of the given functions so the porter makes it again (e.g. after a converter fix).

Deletes, in the ledger's remake_file, the block that starts at the "// 0x<addr> <name>" comment and ends at the closing brace of
that naked function, and the "// 0x<addr>" + declaration pair in the matching header; then tools/ledger_code_check.py --fix clears
the ledger rows whose code is gone. Refuses VERIFIED rows (their tests passed: nothing to redo).
usage: python tools/unport.py 0xADDR [0xADDR ...] | --from-json FILE (a list of [addr, ...] entries)
"""
import csv, io, json, os, re, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def main():
    if '--from-json' in sys.argv:
        addrs = [e[0] for e in json.load(open(sys.argv[sys.argv.index('--from-json') + 1]))]
    else:
        addrs = [a for a in sys.argv[1:] if a.startswith('0x')]
    led = {r['address']: r for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))}
    done = 0
    for a in addrs:
        r = led.get(a)
        if not r or not r['remake_file']:
            continue
        if r['remake_verification'].startswith('VERIFIED'):
            print('refused (VERIFIED): %s %s' % (a, r['ghidra_name']))
            continue
        rel = r['remake_file'] if r['remake_file'].startswith('05_remake') else '05_remake/' + r['remake_file']
        cpp = os.path.join(ROOT, rel)
        t = io.open(cpp, encoding='utf-8', newline='').read()
        nl = '\r\n' if '\r\n' in t else '\n'
        i = t.find('// %s %s ' % (a, r['ghidra_name']))
        if i < 0:
            print('no block for %s in %s' % (a, rel))
            continue
        j = t.find(nl + '}' + nl, t.find('__asm', i))
        t = t[:i] + t[j + len(nl) * 2 + 1:]
        io.open(cpp, 'w', encoding='utf-8', newline='').write(t)
        hp = cpp[:-4] + '.h'
        if os.path.exists(hp):
            h = io.open(hp, encoding='utf-8', newline='').read()
            h = re.sub(r'// %s\r?\n[^\n]*\b%s\([^\n]*\);\r?\n' % (a, re.escape(r['ghidra_name'])), '', h)
            io.open(hp, 'w', encoding='utf-8', newline='').write(h)
        done += 1
    print('removed %d port(s)' % done)
    print(subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'ledger_code_check.py'), '--fix'], capture_output=True,
                         text=True).stdout.strip().splitlines()[-1])


if __name__ == '__main__':
    main()
