"""link_dict_tests.py - name the dictionary test (tests/test_fuzz_dict_*.cpp) in the ledger row of every function it covers (run when port_loop / slice_cycle are NOT running).

Appends ' + native_fuzz_dict_<x>_match_original (tests/test_fuzz_dict_<x>.cpp)' to remake_verification of rows still tagged IMPLEMENTED-UNVERIFIED, so verify_batch runs
the dictionary test with the function's other tests (tools/verify_batch.tests_for collects every native_* name in the field) and llm_pipeline mutate uses it for the
function's mutants. Never touches a VERIFIED row, never adds twice. usage: python tools/link_dict_tests.py [--dry]
"""
import csv, glob, io, os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def main():
    dry = '--dry' in sys.argv
    cover = {}
    for path in sorted(glob.glob(os.path.join(ROOT, '05_remake', 'tests', 'test_fuzz_dict_*.cpp'))):
        base = os.path.basename(path)
        text = io.open(path, encoding='utf-8').read()
        m = re.search(r'TEST\((native_fuzz_dict_\w+_match_original)\)', text)
        for va in re.findall(r'^\s*\{0x([0-9a-fA-F]{8}),', text, re.M):
            cover['0x' + va.lower()] = (m.group(1), base)
    led = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')
    rows = list(csv.reader(io.open(led, encoding='utf-8', newline='')))
    h = rows[0]
    ia, iv = h.index('address'), h.index('remake_verification')
    n = 0
    for r in rows[1:]:
        if not r or not r[iv].startswith('IMPLEMENTED-UNVERIFIED') or r[ia].lower() not in cover:
            continue
        test, base = cover[r[ia].lower()]
        if test in r[iv]:
            continue
        r[iv] = r[iv].rstrip(')') + ' + %s (tests/%s) written, not run)' % (test, base) if r[iv].endswith(')') else r[iv] + ' + %s (tests/%s)' % (test, base)
        n += 1
    if n and not dry:
        csv.writer(io.open(led, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows(rows)
    print('%d row(s) linked%s' % (n, ' (dry run)' if dry else ''))


if __name__ == '__main__':
    main()
