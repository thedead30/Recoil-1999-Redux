"""compare_setup.py - the C++ Setup (05_remake/build/Player/Setup.exe) must install exactly what the Python reference
(tools/setup/setup_core.py) installs, from the same copy of Recoil. Installs both into OUT/py and OUT/cpp and compares every file.
usage: python tools/setup/compare_setup.py --dgvoodoo ZIP --out DIR [--setup EXE] [--payload EXE] SOURCE [SOURCE...]
Exit 0 when every source gives identical installs.
"""
import hashlib, os, shutil, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))


def tree(d):
    out = {}
    for dp, _, fns in os.walk(d):
        for fn in fns:
            p = os.path.join(dp, fn)
            out[os.path.relpath(p, d).lower()] = hashlib.sha256(open(p, 'rb').read()).hexdigest()
    return out


def main():
    a = sys.argv[1:]

    def opt(name, default=None):
        if name in a:
            i = a.index(name)
            v = a[i + 1]
            del a[i:i + 2]
            return v
        return default
    dgv, out = opt('--dgvoodoo'), opt('--out')
    setup = opt('--setup', os.path.join(ROOT, '05_remake', 'build', 'Player', 'Setup.exe'))
    payload = opt('--payload', os.path.join(ROOT, '05_remake', 'build', 'Player', 'recoil_boot.exe'))
    if not (dgv and out and a):
        print(__doc__)
        return 1
    bad = 0
    for n, src in enumerate(a):
        py, cpp = os.path.join(out, '%d_py' % n), os.path.join(out, '%d_cpp' % n)
        for d in (py, cpp):
            shutil.rmtree(d, ignore_errors=True)
        common = ['--source', src, '--dgvoodoo', dgv, '--payload', payload, '--no-shortcut']
        t = time.time()
        rp = subprocess.run([sys.executable, os.path.join(HERE, 'setup_core.py')] + common + ['--dest', py], capture_output=True, text=True)
        tp = time.time() - t
        t = time.time()
        rc = subprocess.run([setup] + common + ['--dest', cpp], capture_output=True, text=True)
        tc = time.time() - t
        A, B = tree(py), tree(cpp)
        diff = sorted(k for k in set(A) | set(B) if A.get(k) != B.get(k))
        ok = (rp.returncode == 0) == (rc.returncode == 0) and not diff   # a refusal: python exits 1, Setup.exe 2
        bad += not ok
        print('%s  %s\n    python exit %d (%.0f s), C++ exit %d (%.0f s), %d files, %d differ%s' % (
            'SAME' if ok else 'DIFFERENT', src, rp.returncode, tp, rc.returncode, tc, len(A), len(diff),
            (': ' + ', '.join(diff[:8])) if diff else ''))
        if rp.returncode or rc.returncode or diff:
            print('    python: ' + (rp.stdout + rp.stderr).strip().replace('\n', '\n            '))
            print('    C++   : ' + (rc.stdout + rc.stderr).strip().replace('\n', '\n            '))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
