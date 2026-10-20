"""gen_dict_tests.py - extra arena-fuzz tests that draw arena words from the constants a function compares against.

Surviving mutants like `cmp dword ptr [eax + 0xf38], 0x75bcd15 -> ...0x75bcd16` mean no test call ever reached the branch: plain random arena words never hit
a magic value. This writes tests/test_fuzz_dict_<listing><n>.cpp (native_fuzz_dict_<listing><n>_match_original) for the functions that have a SURVIVED mutant
in 03_re/staging/verify/mutation_results.csv (other than ret-imm changes, which need the ESP check instead), with af::dictionary() filled per function by
tools/asm_port/gen_fuzz_test.py --dictionary. Existing files are never overwritten. It only writes tests: linking them into the ledger and the verdict
stay with link_dict_tests / verify_batch / promote_ready.
usage: python tools/gen_dict_tests.py [--chunk 8] [--all] [--dry]
  --all: every ported function with a listing, not only the ones with a surviving mutant
"""
import argparse, csv, glob, io, os, re, subprocess, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--chunk', type=int, default=8)
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--dry', action='store_true')
    ap.add_argument('--skip-names', default='', help='comma list of function names (or a file holding one) whose dictionary test differs / crashes on the clean build (arena artifacts)')
    a = ap.parse_args()
    skip_names = a.skip_names
    if skip_names and os.path.exists(skip_names):
        skip_names = io.open(skip_names, encoding='utf-8').read()
    skip = {x.strip() for x in skip_names.replace('\n', ',').split(',') if x.strip()}
    led = {r['ghidra_name']: r for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))}
    names = set()
    for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re/staging/verify/mutation_results.csv'), encoding='utf-8')):
        if r['outcome'] == 'SURVIVED' and not r['regex'].startswith('ret'):
            names.add(r['name'])
    if a.all:
        names = {n for n, r in led.items() if r['remake_symbol'] and r['remake_file'].endswith('.cpp')}
    names -= skip
    where = {}
    for listing in sorted(glob.glob(os.path.join(ROOT, '03_re/listings/unported/*.txt'))):
        for m in re.finditer(r'^### ([0-9a-f]{8})', io.open(listing, encoding='utf-8').read(), re.M):
            where.setdefault('0x' + m.group(1), listing)
    groups = {}
    for n in sorted(names):
        r = led.get(n)
        if not r or r['address'] not in where or not r['remake_file'].endswith('.cpp'):
            continue
        groups.setdefault((where[r['address']], r['remake_file']), []).append(r['address'])
    made = 0
    for (listing, cpp), addrs in sorted(groups.items()):
        s = os.path.basename(listing)[:-4]
        hdr = cpp.replace('05_remake/src/', '').replace('src/', '', 1)[:-4] + '.h'
        addrs.sort()
        for i in range(0, len(addrs), a.chunk):
            k = 1
            while os.path.exists(os.path.join(ROOT, '05_remake/tests/test_fuzz_dict_%s%d.cpp' % (s, k))):
                k += 1
            name = 'dict_%s%d' % (s, k)
            chunk = addrs[i:i + a.chunk]
            cmd = [sys.executable, 'tools/asm_port/gen_fuzz_test.py', os.path.relpath(listing, ROOT), ','.join(x[2:] for x in chunk), '--name', name, '--include', hdr, '--dictionary']
            if a.dry:
                print(' '.join(cmd))
                continue
            r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
            print('%s: %d functions%s' % (name, len(chunk), '' if r.returncode == 0 else ' FAILED ' + r.stderr[-200:]), flush=True)
            made += r.returncode == 0
    print('%d tests written for %d functions with constants to seed' % (made, sum(len(v) for v in groups.values())))


if __name__ == '__main__':
    main()
