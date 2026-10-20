"""check_ret_cleanup.py - compare the stack bytes each function pops on return (RET n) between the original and the port.

A replacement that pops a different number of argument bytes than the original shifts its caller's ESP, so every
ESP-relative local of an asm-ported caller moves (KG-51: a 12-byte shift). For each ledger function with a port symbol:
set of RET immediates in the original body (up to the next ledger function) vs in the port body (up to the next map
symbol). Prints mismatches only.
"""
import os, re, sys, csv
sys.path.insert(0, os.path.dirname(__file__))
import bytediff  # noqa: E402
from capstone import Cs, CS_ARCH_X86, CS_MODE_32  # noqa: E402

ROOT = bytediff.ROOT
exe = os.path.join(ROOT, '05_remake', 'build', 'Port', 'recoil_boot.exe')
led = [(int(r[0], 16), r[1]) for r in csv.reader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8', errors='replace')) if r and r[0].startswith('0x')]
led.sort(); starts = [a for a, _ in led]
psyms = []
for l in open(exe[:-4] + '.map', errors='replace'):
    m = re.match(r'\s*0001:[0-9a-f]+\s+(\S+)\s+([0-9a-f]{8})', l)
    if m: psyms.append((int(m.group(2), 16), m.group(1)))
psyms.sort(); pstarts = [a for a, _ in psyms]
pby = {}
for a, raw in psyms:
    n = re.match(r'\?(\w+)@recoil@@', raw)
    if n: pby.setdefault(n.group(1), a)
oread, _, _ = bytediff.load_pe(bytediff.ORIG); pread, _, _ = bytediff.load_pe(exe)
md = Cs(CS_ARCH_X86, CS_MODE_32)
import bisect
def rets(read, a, end):
    s = set()
    try:
        for i in md.disasm(read(a, min(end - a, 0x4000)), a):
            if i.mnemonic == 'ret': s.add(int(i.op_str, 16) if i.op_str else 0)
    except Exception: pass
    return s
bad = 0; n = 0
for k, (va, name) in enumerate(led):
    pa = pby.get(name)
    if not pa or va < 0x401000 or va >= 0x4cb000: continue
    oend = starts[k + 1] if k + 1 < len(starts) else va + 0x400
    pend = pstarts[bisect.bisect_right(pstarts, pa)] if bisect.bisect_right(pstarts, pa) < len(pstarts) else pa + 0x400
    o = rets(oread, va, oend); p = rets(pread, pa, pend)
    if not o or not p: continue
    n += 1
    if o != p:
        bad += 1; print('%08x %-44s original ret %s   port ret %s' % (va, name, sorted(o), sorted(p)))
print('%d functions compared, %d mismatched' % (n, bad))
