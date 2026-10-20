"""gen_port_loop_tests.py - arena-fuzz tests for the functions tools/port_loop.py ported without a test.

Groups the ledger rows tagged by port_loop (remake_verification starts with its TAG) by listing and remake file,
and runs tools/asm_port/gen_fuzz_test.py once per chunk of --chunk functions, writing
05_remake/tests/test_fuzz_auto_<listing><n>.cpp. Existing auto files are never overwritten (a failing or hand-fixed
test stays as it is). The tests are only written here - verify_batch / mutants / promote decide the tag.
usage: python tools/gen_port_loop_tests.py [--chunk 8] [--only SUBSYS[,SUBSYS...]] [--dry]
"""
import argparse, csv, glob, os, re, subprocess, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
TAG = 'IMPLEMENTED-UNVERIFIED (ported locally by tools/port_loop.py; no test yet)'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--chunk', type=int, default=8)
    ap.add_argument('--only', default='')
    ap.add_argument('--dry', action='store_true')
    ap.add_argument('--out-dir', default='', help='write the tests here (default 05_remake/tests)')
    a = ap.parse_args()
    only = set(filter(None, a.only.split(',')))
    rows = [r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))
            if r['remake_verification'] == TAG and r['remake_file'].endswith('.cpp')]
    want = {r['address'].lower(): r for r in rows}
    where = {}
    for listing in sorted(glob.glob(os.path.join(ROOT, '03_re/listings/unported/*.txt'))):
        for m in re.finditer(r'^### ([0-9a-f]{8})', open(listing, encoding='utf-8').read(), re.M):
            if '0x' + m.group(1) in want:
                where.setdefault('0x' + m.group(1), listing)
    groups = {}
    for addr, listing in where.items():
        s = os.path.basename(listing)[:-4]
        if only and s not in only:
            continue
        groups.setdefault((listing, want[addr]['remake_file']), []).append(addr)
    made = 0
    for (listing, cpp), addrs in sorted(groups.items()):
        s = os.path.basename(listing)[:-4]
        hdr = cpp.replace('05_remake/src/', '')[:-4] + '.h'
        addrs.sort()
        for i in range(0, len(addrs), a.chunk):
            n = 1
            while (os.path.exists(os.path.join(ROOT, '05_remake/tests/test_fuzz_auto_%s%d.cpp' % (s, n))) or os.path.exists(os.path.join(a.out_dir or ROOT, 'test_fuzz_auto_%s%d.cpp' % (s, n)))) or \
                    any(k[0] == '%s%d' % (s, n) for k in ()):
                n += 1
            name = 'auto_%s%d' % (s, n)
            chunk = addrs[i:i + a.chunk]
            cmd = [sys.executable, 'tools/asm_port/gen_fuzz_test.py', os.path.relpath(listing, ROOT), ','.join(x[2:] for x in chunk),
                   '--name', name, '--include', hdr] + (['--out-dir', a.out_dir] if a.out_dir else [])
            if a.dry:
                print(' '.join(cmd))
                continue
            r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
            print('%s: %d functions%s' % (name, len(chunk), '' if r.returncode == 0 else ' FAILED ' + r.stderr[-200:]), flush=True)
            made += r.returncode == 0
    print('%d of %d untested port_loop functions found in listings; %d tests written' % (len(where), len(want), made))


if __name__ == '__main__':
    main()
