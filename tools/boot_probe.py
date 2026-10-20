"""boot_probe.py - run the boot probe (05_remake/build/<dir>/recoil_boot.exe) and say where it stopped.

The probe starts the remake from the ported CRT entry (05_remake/boot/boot_main.cpp). It either exits (its stderr says why: an
unported function reached through a data pointer names its caller, a fault names its address) or it is still running after
--seconds: then every thread's EIP is read with cdbX86 (non-invasive attach) and mapped to the nearest symbol of recoil_boot.map
(a ported function's name carries its original address), and the process is killed. Each stop is the next Stage 2 blocker.
usage: python tools/boot_probe.py [--dir Port] [--seconds 20]
"""
import os, re, subprocess, sys, time

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
CDB = os.path.expandvars(r'%LOCALAPPDATA%\Microsoft\WindowsApps\cdbX86.exe')


def symbols(mapfile):
    out = []
    for l in open(mapfile, encoding='latin-1'):
        m = re.match(r'\s*0001:[0-9a-f]+\s+(\S+)\s+([0-9a-f]{8})\s', l)
        if m:
            out.append((int(m.group(2), 16), m.group(1)))
    return sorted(out)


def near(syms, a):
    best = None
    for va, n in syms:
        if va > a:
            break
        best = (va, n)
    if not best:
        return '?'
    n = best[1]
    m = re.match(r'\?(\w+)@recoil@@', n)
    return '%s+0x%x' % (m.group(1) if m else n, a - best[0])


def main():
    d = sys.argv[sys.argv.index('--dir') + 1] if '--dir' in sys.argv else 'Port'
    secs = float(sys.argv[sys.argv.index('--seconds') + 1]) if '--seconds' in sys.argv else 20
    b = os.path.join(ROOT, '05_remake', 'build', d)
    exe = os.path.join(b, 'recoil_boot.exe')
    p = subprocess.Popen([exe], cwd=b, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors='replace')
    t0 = time.time()
    while p.poll() is None and time.time() - t0 < secs:
        time.sleep(0.5)
    if p.poll() is not None:
        print(p.stdout.read().strip())
        print('exit code %d (0x%08x) after %.1f s' % (p.returncode, p.returncode & 0xffffffff, time.time() - t0))
        return
    r = subprocess.run([CDB, '-pv', '-p', str(p.pid), '-c', '~*e r eip; q'], capture_output=True, text=True, errors='replace')
    syms = symbols(os.path.join(b, 'recoil_boot.map'))
    eips = [int(x, 16) for x in re.findall(r'eip=([0-9a-f]{8})', r.stdout)]
    p.kill()
    print(p.stdout.read().strip())
    print('still running after %.0f s; thread EIPs:' % secs)
    for e in eips:
        print('  %08x  %s' % (e, near(syms, e) if 0x400000 <= e < 0x10000000 else '(system DLL)'))


if __name__ == '__main__':
    main()
