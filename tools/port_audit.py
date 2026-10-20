"""port_audit.py - KG-63: run the stricter port checks over every ported function and write one report.

  * bytediff_full: whole-function instruction compare (does not stop at the first difference);
  * check_targets: call targets and global data addresses resolved on both sides.
Usage: python tools/port_audit.py [--out 03_re/staging/verify/port_audit.txt]
Lists only functions with differences / issues; equivalences bytediff_full accepts are counted, not listed.
"""
import io, os, sys, csv, re, contextlib, traceback
sys.path.insert(0, os.path.dirname(__file__))
import bytediff_full, check_targets  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
out_path = os.path.join(ROOT, '03_re', 'staging', 'verify', 'port_audit.txt')
if '--out' in sys.argv: out_path = sys.argv[sys.argv.index('--out') + 1]
led = [r[1] for r in csv.reader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8', errors='replace')) if r and r[0].startswith('0x')]
mp = os.path.join(ROOT, '05_remake', 'build', 'Port', 'recoil_boot.map')
pm = set(m.group(1) for m in (re.search(r'\?(\w+)@recoil@@', l) for l in open(mp, errors='replace')) if m)
names = [n for n in led if n in pm]

def run(mod, name):
    buf = io.StringIO(); argv = sys.argv
    try:
        sys.argv = [mod.__file__, name]
        with contextlib.redirect_stdout(buf): mod.main()
    except SystemExit: pass
    except Exception: buf.write('ERROR ' + traceback.format_exc().splitlines()[-1] + '\n')
    finally: sys.argv = argv
    return buf.getvalue()

flagged = 0
with open(out_path, 'w', encoding='utf-8') as f:
    for k, n in enumerate(names):
        a = run(bytediff_full, n); b = run(check_targets, n)
        bad_a = re.search(r', ([1-9]\d*) difference', a) or 'ERROR' in a
        bad_b = re.search(r'^([1-9]\d*) issue', b, re.M) or 'ERROR' in b
        if bad_a or bad_b:
            flagged += 1
            f.write('=== %s\n%s%s\n' % (n, a if bad_a else '', b if bad_b else ''))
            f.flush()
        if k % 200 == 0: print('%d/%d checked, %d flagged' % (k, len(names), flagged), flush=True)
    f.write('\n%d functions checked, %d flagged\n' % (len(names), flagged))
print('done: %d checked, %d flagged -> %s' % (len(names), flagged, out_path))
