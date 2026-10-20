"""verify_batch.py - run the tests of the functions waiting for verification and report one line per function.

usage: python tools/verify_batch.py [--all] [--only SUBSTR] [--no-build] [--jobs N]

Takes the queue from the ledger (rows whose remake_verification starts with IMPLEMENTED-UNVERIFIED; default only the
"cloud:" ones, --all for every row), finds each function's test (a native_* test name in the note, or the TESTs of a
tests/test_*.cpp file it names), builds once, runs just those tests with tools/run_tests.py and parses the log:
  arena fuzz tests  - the per-function line "<name> calls N abnormal M ok|DIFF ..." (and a crash inside its shard);
  structured tests  - the test's own ok / FAIL line, applied to every function that names it.
Writes 03_re/staging/verify/verify_results.csv (address, name, test, result, detail) and the raw log to
03_re/staging/verify/verify_run.log, and prints counts plus the first failures. It never edits the ledger: promoting a
function to VERIFIED-ORACLE stays a reviewed step (mutation check first, then 03_re/scripts/remake_set.py).
A pass here is necessary, not sufficient: the mutation pass (tools/asm_port/mutants.json) comes next.
"""
import argparse, csv, glob, io, os, re, subprocess, sys, collections

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
STAGING = os.path.join(ROOT, '03_re', 'staging', 'verify')
EXE = os.path.join(ROOT, '05_remake', 'build', 'Release', 'recoil_tests.exe')


def queue(all_rows, only):
    rows = [r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))
            if r.get('remake_verification', '').startswith('IMPLEMENTED-UNVERIFIED')]
    if not all_rows:
        rows = [r for r in rows if 'cloud' in r['remake_verification'].lower()]
    if only:
        rows = [r for r in rows if only.lower() in (r['ghidra_name'] + r['remake_file']).lower()]
    return rows


def tests_in_file(path, cache={}):
    if path not in cache:
        full = os.path.join(ROOT, '05_remake', 'tests', os.path.basename(path))
        cache[path] = re.findall(r'^TEST\((\w+)\)', io.open(full, encoding='utf-8').read(), re.M) if os.path.exists(full) else []
    return cache[path]


def tests_for(row):
    note = row['remake_verification']
    names = re.findall(r'\b(native_\w+)', note)
    for f in re.findall(r'\b(test_\w+\.cpp)', note):
        names += tests_in_file(f)
    return sorted(set(names))


def build():
    r = subprocess.run(['cmd', '/c', r'05_remake\build.bat', 'Release', 'zz_build_only_no_such_test'], cwd=ROOT,
                       capture_output=True, text=True, errors='replace')
    errs = [l for l in r.stdout.splitlines() if ' error ' in l or 'FAILED:' in l]
    if errs or not os.path.exists(EXE):
        sys.exit('build failed:\n' + '\n'.join(errs[:20]))


def parse(log):
    """-> {test: 'ok'|'FAIL'}, {function name: (result, detail)} from the run_tests.py log."""
    test_res, fn_res = {}, {}
    per_fn = {}  # test -> True when it is a per-function runner that listed its differing functions and did not crash
    named, crashed = collections.defaultdict(set), set()  # test -> functions its log names as differing; tests that crashed
    seen = collections.defaultdict(set)  # test -> every function its log shows it running (per-function runners)
    finished = set()  # tests whose own ok / FAIL line was printed
    cur_test, cur_fn = None, None

    def put(fn, res, det):  # a function several tests run: one FAIL anywhere is its result, never overwritten by a later PASS
        if fn_res.get(fn, ('PASS',))[0] != 'FAIL':
            fn_res[fn] = (res, det)
    for line in log.splitlines():
        m = re.match(r'^run  (\w+)', line)
        if m:
            cur_test, cur_fn = m.group(1), None
            continue
        m = re.match(r'^  run  (\w+)', line)
        if m:
            cur_fn = m.group(1)
            seen[cur_test].add(cur_fn)
            continue
        m = re.match(r'^\s+(\w+)\s+(\d+) of (\d+) differ', line)  # vt_runner / ui_widgets: one independent case per function
        if m:
            # vt_runner.h prints every function it ran (0 of N for a match, since 2026-09-30): a line is the evidence it was exercised
            put(m.group(1), 'PASS' if m.group(2) == '0' else 'FAIL', '%s of %s cases differ in %s' % (m.group(2), m.group(3), cur_test))
            if m.group(2) != '0':
                named[cur_test].add(m.group(1))
            seen[cur_test].add(m.group(1))
            continue
        if re.search(r'\d+ calls, \d+ functions differ', line) and cur_test:
            per_fn.setdefault(cur_test, True)
            continue
        m = re.match(r'^\s+(\w+)\s+calls\s+\d+\s+abnormal\s+\d+\s+(ok|DIFF)(.*)$', line)
        if m:
            put(m.group(1), 'PASS' if m.group(2) == 'ok' else 'FAIL', (m.group(2) + m.group(3)).strip()[:160])
            if m.group(2) != 'ok':
                named[cur_test].add(m.group(1))
            seen[cur_test].add(m.group(1))
            continue
        if ('test child crashed' in line or 'ESCAPED FAULT' in line) and cur_test:
            test_res[cur_test] = 'FAIL'
            if cur_test in finished:
                continue  # a fault after the test's own ok / FAIL line (process teardown): its per-function lines are complete
            if cur_fn and cur_fn not in fn_res:
                fn_res[cur_fn] = ('FAIL', line.strip()[:160])
            per_fn[cur_test] = False
            crashed.add(cur_test)
            continue
        m = re.match(r'^(ok  |FAIL) (\w+)', line)
        if m:
            name = m.group(2)
            finished.add(name)
            if test_res.get(name) != 'FAIL':
                test_res[name] = 'ok' if m.group(1).startswith('ok') else 'FAIL'
            continue
        m = re.match(r'^FAILED JOB (\w+)', line)
        if m:
            test_res[m.group(1)] = 'FAIL'  # any failed job (a crash is reported by its own lines above)
    return test_res, fn_res, {t for t, v in per_fn.items() if v}, named, crashed, seen


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--only')
    ap.add_argument('--no-build', action='store_true')
    ap.add_argument('--jobs', type=int, default=0)
    ap.add_argument('--from-log', default='', help='parse these existing run logs (comma-separated) instead of running the tests')
    ap.add_argument('--exe', default=EXE, help='test executable (e.g. a separate build dir while another run owns build/Release)')
    a = ap.parse_args()
    rows = queue(a.all, a.only)
    need = {r['address']: tests_for(r) for r in rows}
    tests = sorted({t for ts in need.values() for t in ts})
    print('%d function(s), %d test(s)' % (len(rows), len(tests)))
    if a.from_log:
        log = chr(10).join(io.open(f, encoding='utf-8', errors='replace').read() for f in a.from_log.split(','))
    else:
        if not a.no_build:
            build()
        cmd = [sys.executable, os.path.join(ROOT, 'tools', 'run_tests.py'), a.exe] + tests
        if a.jobs:
            cmd += ['--jobs', str(a.jobs)]
        log = subprocess.run(cmd, cwd=os.path.join(ROOT, '05_remake'), capture_output=True, text=True, errors='replace').stdout
        os.makedirs(STAGING, exist_ok=True)
        io.open(os.path.join(STAGING, 'verify_run.log'), 'w', encoding='utf-8').write(log)
    test_res, fn_res, per_fn_tests, named, crashed, seen = parse(log)
    out, counts = [], collections.Counter()
    for r in rows:
        ts = need[r['address']]
        name = r['ghidra_name']
        if not ts:
            res, det = 'NO-TEST', 'no test named in the ledger note'
        elif name in fn_res:
            res, det = fn_res[name]
        elif all(test_res.get(t) == 'ok' or t in per_fn_tests for t in ts) and any(t in per_fn_tests for t in ts):
            res, det = 'PASS', 'its cases matched in ' + ','.join(ts) + ' (other functions of the runner differed)'
        elif all(test_res.get(t) == 'ok' for t in ts):
            res, det = 'PASS', ','.join(ts)
        elif any(test_res.get(t) == 'FAIL' for t in ts):
            res, det = 'FAIL', 'test failed: ' + ','.join(t for t in ts if test_res.get(t) == 'FAIL')
        else:
            res, det = 'NOT-RUN', 'no result for ' + ','.join(ts)
        counts[res] += 1
        out.append({'address': r['address'], 'name': name, 'test': ';'.join(ts), 'result': res, 'detail': det})
    with io.open(os.path.join(STAGING, 'verify_results.csv'), 'w', encoding='utf-8', newline='') as f:
        w = csv.DictWriter(f, fieldnames=['address', 'name', 'test', 'result', 'detail'], lineterminator='\n')
        w.writeheader()
        w.writerows(out)
    print('  '.join('%s %d' % kv for kv in sorted(counts.items())))
    for o in [o for o in out if o['result'] != 'PASS'][:25]:
        print('  %-7s %s %-38s %s' % (o['result'], o['address'], o['name'][:38], o['detail'][:90]))
    print('results: 03_re/staging/verify/verify_results.csv  log: 03_re/staging/verify/verify_run.log')


if __name__ == '__main__':
    main()
