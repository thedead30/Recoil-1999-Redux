"""check_void_returns.py - find ported functions declared `void` whose original leaves a value in EAX that callers use.

KG-51: the C++ zTransformStore was `void`, but the original returns EAX = out and gwNodeBuildNodeToAncestorMatrix copies
from it. Asm-ported callers keep the original's register contract, so any C++/void replacement of a function whose
EAX is read after the call is a silent wrong-value bug.
Method: scan the original .text for `call F`; after each call, follow straight-line code (stop at jumps/calls/ret) and
note whether EAX/AX/AL is read before it is written. Functions with at least one such call site are 'EAX-used'. Then
look up the port's declaration (05_remake/src/**/*.h|*.cpp) and report the ones declared void.
"""
import os, re, sys, csv, glob
sys.path.insert(0, os.path.dirname(__file__))
import bytediff  # noqa: E402
from capstone import Cs, CS_ARCH_X86, CS_MODE_32  # noqa: E402
from capstone.x86 import X86_REG_EAX, X86_REG_AX, X86_REG_AL, X86_REG_AH  # noqa: E402

ROOT = bytediff.ROOT
EAXS = {X86_REG_EAX, X86_REG_AX, X86_REG_AL, X86_REG_AH}
read, lo, hi = bytediff.load_pe(bytediff.ORIG)
md = Cs(CS_ARCH_X86, CS_MODE_32); md.detail = True
led = {int(r[0], 16): r[1] for r in csv.reader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8', errors='replace')) if r and r[0].startswith('0x')}
starts = sorted(a for a in led if 0x401000 <= a < 0x4cc000)
ins = []
for a, b in zip(starts, starts[1:] + [0x4cc000]):
    ins += list(md.disasm(read(a, min(b - a, 0x8000)), a)) + [None]   # None: function boundary
used = {}
for k, i in enumerate(ins):
    if i is None: continue
    if i.mnemonic != 'call' or not i.op_str.startswith('0x'): continue
    t = int(i.op_str, 16)
    if t not in led: continue
    for j in ins[k + 1:k + 8]:
        if j is None: break
        if j.mnemonic in ('xor', 'sub') and len(set(j.op_str.split(', '))) == 1 and 'ax' in j.op_str: break   # zeroing is a write
        r_, w_ = j.regs_access()
        if set(r_) & EAXS:
            used.setdefault(t, []).append((i.address, '%s %s' % (j.mnemonic, j.op_str))); break
        if set(w_) & EAXS or j.mnemonic.startswith(('j', 'call', 'ret')): break
decl = {}
for path in glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.h'), recursive=True) + glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.cpp'), recursive=True):
    for m in re.finditer(r'^\s*(?:__declspec\(naked\)\s+)?([\w:\*\s]+?)\s+(?:__\w+\s+)?(\w+)\s*\([^;{]*\)\s*[;{]', open(path, encoding='utf-8', errors='replace').read(), re.M):
        parts = m.group(1).split()
        if not parts: continue
        ret, name = parts[-1], m.group(2)
        naked = '__declspec(naked)' in m.group(0)
        decl.setdefault(name, []).append((ret, naked, os.path.relpath(path, ROOT)))
n = 0
for t, sites in sorted(used.items()):
    name = led[t]
    ds = decl.get(name, [])
    if any(nk for _, nk, _ in ds): continue          # implemented as a naked asm port: EAX as the original
    for ret, naked, path in ds:
        if ret == 'void':
            n += 1
            print('%08x %-40s void in %s; EAX read after %d call(s), e.g. %08x: %s' % (t, name, path, len(sites), sites[0][0], sites[0][1]))
            break
print('%d EAX-used functions checked, %d declared void (non-naked) in the port' % (len(used), n))
