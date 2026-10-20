"""run_tests.py - run the remake's tests as parallel processes (one per test; fuzz tests split into shards).

Each process maps its own copy of the original image (tests/native_oracle.h), so the jobs are independent. Output
is printed per job in the registry order, so the log reads like a serial run (record steps parse the same lines).
A job that runs past --timeout is killed and reported as a hang instead of stalling the whole run.
usage: python tools/run_tests.py EXE [PATTERN...] [--jobs N] [--shards N] [--timeout SEC]
       PATTERN: a test name or a prefix ending in '*' (as recoil_tests takes); none = every test.
"""
import argparse, fnmatch, os, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor


def main():
    # test output may hold non-ASCII text (file names, messages): never let the console codepage kill the run while it prints its results
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding='utf-8', errors='replace')
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser()
    ap.add_argument('exe')
    ap.add_argument('patterns', nargs='*')
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    ap.add_argument('--shards', type=int, default=32,
                    help='processes per fuzz test (native_fuzz_*): 32 = one function per process for batches up to 32')
    ap.add_argument('--timeout', type=int, default=900, help='per job: a job past this is killed and reported HUNG')
    a = ap.parse_args()
    names = subprocess.run([a.exe, '--list'], capture_output=True, text=True, check=True).stdout.split()
    if a.patterns:
        names = [n for n in names if any(fnmatch.fnmatchcase(n, p) for p in a.patterns)]
    jobs = []  # (label, test name, shard spec)
    for n in names:
        if n.startswith('native_fuzz_') and a.shards > 1:
            jobs += [('%s [%d/%d]' % (n, i, a.shards), n, '%d/%d' % (i, a.shards)) for i in range(a.shards)]
        else:
            jobs.append((n, n, None))

    def run(job):
        label, name, shard = job
        env = dict(os.environ)
        env.pop('RECOIL_TESTS_CHILD', None)
        if shard:
            env['RECOIL_SHARD'] = shard
        t0 = time.time()
        p = subprocess.Popen([a.exe, name], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors='replace', env=env)
        try:
            out, _ = p.communicate(timeout=a.timeout)
            return label, p.returncode, out, time.time() - t0
        except subprocess.TimeoutExpired:
            # the tests run in a relaunched child (tests/test_main.cpp): kill this job's whole tree, nothing else
            subprocess.run(['taskkill', '/F', '/T', '/PID', str(p.pid)], capture_output=True)
            out, _ = p.communicate()
            return label, -1, out + 'HUNG: %s killed after %d s\n' % (label, a.timeout), time.time() - t0

    t0 = time.time()
    done = [0]

    def run_logged(job):
        r = run(job)
        done[0] += 1  # progress on stderr as each job ends, so a watcher can tell a slow run from a stalled one
        sys.stderr.write('[%d/%d] %s %s %.0f s\n' % (done[0], len(jobs), 'ok  ' if r[1] == 0 else 'FAIL', r[0], r[3]))
        sys.stderr.flush()
        return r

    with ThreadPoolExecutor(max_workers=a.jobs) as pool:
        results = list(pool.map(run_logged, jobs))
    failed = []
    for label, rc, out, secs in results:
        sys.stdout.write(out)
        if rc != 0:
            failed.append(label)
    slow = sorted(results, key=lambda r: -r[3])[:3]
    print('%d tests in %d jobs, %d failed jobs, %.0f s wall (slowest: %s)' % (
        len(names), len(jobs), len(failed), time.time() - t0, ', '.join('%s %.0f s' % (r[0], r[3]) for r in slow)))
    for f in failed:
        print('FAILED JOB', f)
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
