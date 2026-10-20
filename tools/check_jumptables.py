"""check_jumptables.py - verify hand-rewritten switch tables in the asm ports (KG-51 investigation tool).

The ports replace the original's `jmp [reg*4 + TABLE]` (and byte-index `mov dl, [reg + BYTES]` + jmp) with a chain of
`cmp reg, N / je L_<va>` lines. This reads the original table entries from Recoil.exe and checks that each index maps to
the same label (L_<target va>) in the port source. Usage:
    python tools/check_jumptables.py                  # every rewritten table found in 05_remake/src
"""
import os, re, struct, sys, glob
sys.path.insert(0, os.path.dirname(__file__))
from bytediff import load_pe, ORIG  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
read, lo, hi = load_pe(ORIG)
from capstone import Cs, CS_ARCH_X86, CS_MODE_32  # noqa: E402
md = Cs(CS_ARCH_X86, CS_MODE_32)

def func_bodies():
    for path in glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.cpp'), recursive=True):
        txt = open(path, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'// (0x[0-9a-f]{8}) (\w+)[^\n]*\n(?:[^\n]*\n){0,3}?__declspec\(naked\)[^{]*\{\s*__asm\s*\{(.*?)\n    \}\n\}', txt, re.S):
            yield path, int(m.group(1), 16), m.group(2), m.group(3)

def orig_tables(va, size=0x1000):
    """(table_kind, index_reg, table_va, byte_table_va) for each switch jump in the original function."""
    code = read(va, size); out = []
    prev = None
    for i in md.disasm(code, va):
        m = re.match(r'dword ptr \[(\w+)\*4 \+ (0x[0-9a-f]+)\]', i.op_str)
        if i.mnemonic == 'jmp' and m:
            byte_tab = None
            if prev and prev.mnemonic == 'mov' and re.search(r'byte ptr \[(\w+) \+ (0x[0-9a-f]+)\]', prev.op_str):
                byte_tab = int(re.search(r'\+ (0x[0-9a-f]+)\]', prev.op_str).group(1), 16)
            out.append((i.address, int(m.group(2), 16), byte_tab))
        if i.mnemonic == 'ret' and i.address > va + 8 and out:
            pass
        prev = i
        if i.address - va > size - 16: break
    return out

bad = 0; checked = 0
for path, va, name, body in func_bodies():
    if 'unreachable: the bounds check' not in body: continue
    tabs = orig_tables(va)
    # chains in the port, in order: list of {index: label}
    chains = []
    for cm in re.finditer(r'((?:\s*cmp (\w+), (\d+)\s*\n\s*je (L_[0-9a-f]+)\s*\n)+)\s*int 3', body):
        chains.append({int(a): b for _, a, b in re.findall(r'cmp (\w+), (\d+)\s*\n\s*je (L_[0-9a-f]+)', cm.group(1))})
    for k, (jva, tva, btab) in enumerate(tabs):
        if k >= len(chains): print('%s: original switch at %08x has no port chain' % (name, jva)); bad += 1; continue
        chain = chains[k]; n = max(chain) + 1
        if btab:   # byte index table -> dword table
            idx = list(read(btab, n))
            want = {i: 'L_%x' % struct.unpack('<I', read(tva + 4 * b, 4))[0] for i, b in enumerate(idx)}
        else:
            want = {i: 'L_%x' % struct.unpack('<I', read(tva + 4 * i, 4))[0] for i in range(n)}
        diff = {i: (want[i], chain.get(i)) for i in want if want[i] != chain.get(i)}
        checked += 1
        if diff:
            bad += 1
            print('%s (%08x) switch at %08x: %s' % (name, va, jva, ', '.join('%d orig %s port %s' % (i, a, b) for i, (a, b) in sorted(diff.items()))))
print('%d switch tables checked, %d wrong' % (checked, bad))
