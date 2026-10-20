"""link_auto_tests.py - name the generated arena-fuzz test in the ledger row of every function it covers (run when port_loop is NOT running).

gen_port_loop_tests.py writes tests/test_fuzz_auto_<x>.cpp but leaves the ledger rows at the port_loop TAG ('no test yet'), so
verify_batch cannot find the test. This reads the entries of every auto test file ({0xVA, port, nstack, "Name", ...}) and, for each
ledger row still at the TAG, sets remake_verification to the 'written, not run' text that names the test (the form the other
657 rows already have). Idempotent; never touches a row that is already tagged with a test or VERIFIED.
usage: python tools/link_auto_tests.py [--dry]
"""
import csv, glob, io, os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
TAG = 'IMPLEMENTED-UNVERIFIED (ported locally by tools/port_loop.py; no test yet)'


def main():
    dry = '--dry' in sys.argv
    cover = {}
    for path in sorted(glob.glob(os.path.join(ROOT, '05_remake', 'tests', 'test_fuzz_auto_*.cpp'))):
        base = os.path.basename(path)
        stem = base[len('test_fuzz_'):-4]
        text = io.open(path, encoding='utf-8').read()
        for va in re.findall(r'^\s*\{0x([0-9a-fA-F]{8}),', text, re.M):
            cover.setdefault('0x' + va.lower(), (stem, base))
    led = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')
    rows = list(csv.reader(io.open(led, encoding='utf-8', newline='')))
    h = rows[0]
    ia, iv = h.index('address'), h.index('remake_verification')
    n = 0
    for r in rows[1:]:
        if r and r[iv] == TAG and r[ia].lower() in cover:
            stem, base = cover[r[ia].lower()]
            r[iv] = 'IMPLEMENTED-UNVERIFIED (port_loop: native_fuzz_%s_match_original (tests/%s) written, not run)' % (stem, base)
            n += 1
    if n and not dry:
        csv.writer(io.open(led, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows(rows)
    print('%d row(s) linked%s' % (n, ' (dry run)' if dry else ''))


if __name__ == '__main__':
    main()
