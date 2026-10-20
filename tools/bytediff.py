"""bytediff.py - compare each ported function's compiled instructions with the original's, instruction by instruction.

The asm ports are meant to be instruction-for-instruction copies, so the compiled port and the original bytes should decode
to the same mnemonics, registers and non-address constants. Addresses differ (the port links elsewhere and reaches data
through the data-image mirror), so any operand value inside either image range is treated as "an address" and compared
only by kind. A difference in mnemonic, register, operand size or small constant is reported - that is a mis-port.

usage: python tools/bytediff.py [--exe 05_remake/build/Port/recoil_boot.exe] [--map ...] [NAME_OR_ADDR ...]
       (no names: every ported function; prints only those that differ)
"""
import argparse, bisect, csv, io, os, re, struct, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
ORIG = os.environ.get('RECOIL_ORIGINAL_EXE') or os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe')   # your Recoil.exe

def load_pe(path):
    d = open(path, 'rb').read()
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    nsec = struct.unpack_from('<H', d, pe + 6)[0]
    opt = struct.unpack_from('<H', d, pe + 20)[0]
    base = struct.unpack_from('<I', d, pe + 24 + 28)[0]
    secs = []
    for i in range(nsec):
        o = pe + 24 + opt + 40 * i
        vsz, va, rsz, raw = struct.unpack_from('<IIII', d, o + 8)
        secs.append((base + va, vsz, raw, rsz))
    def read(addr, n):
        for va, vsz, raw, rsz in secs:
            if va <= addr < va + max(vsz, rsz):
                off = raw + addr - va
                return d[off:off + n]
        return b''
    lo = base; hi = max(va + vsz for va, vsz, _, _ in secs)
    return read, lo, hi

def insns(md, read, addr, limit):
    code = read(addr, limit)
    out = []
    for i in md.disasm(code, addr):
        out.append(i)
        if i.mnemonic in ('ret', 'jmp') and len(out) > 0 and i.mnemonic == 'ret':
            pass
    return out

def sig(i, lo, hi, plo, phi):
    """Normalised instruction signature: mnemonic + operands with addresses reduced to 'A'."""
    parts = [i.mnemonic]
    for op in i.operands:
        if op.type == X86_OP_REG:
            parts.append(i.reg_name(op.reg))
        elif op.type == X86_OP_IMM:
            v = op.imm & 0xffffffff
            parts.append('A' if (lo <= v < hi or plo <= v < phi) or i.mnemonic.startswith(('j', 'call')) else hex(v))
        elif op.type == X86_OP_MEM:
            m = op.mem
            disp = m.disp & 0xffffffff
            d = 'A' if (lo <= disp < hi or plo <= disp < phi) else hex(disp)
            parts.append('[%s%s%s*%d+%s]s%d' % (i.reg_name(m.segment) + ':' if m.segment else '', i.reg_name(m.base) if m.base else '',
                                                '+' + i.reg_name(m.index) if m.index else '', m.scale, d, op.size))
    return ' '.join(parts)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=os.path.join(ROOT, '05_remake', 'build', 'Port', 'recoil_boot.exe'))
    ap.add_argument('--map', default=None)
    ap.add_argument('names', nargs='*')
    a = ap.parse_args()
    mp = a.map or a.exe[:-4] + '.map'
    syms = []
    for line in open(mp, errors='replace'):
        m = re.match(r'\s*0001:[0-9a-f]{8}\s+\?(\w+)@recoil@@\S*\s+([0-9a-f]{8})', line)
        if m: syms.append((int(m.group(2), 16), m.group(1)))
    syms.sort(); starts = [s[0] for s in syms]
    by_name = {n: va for va, n in syms}
    led = {r['remake_symbol'].split('::')[-1]: int(r['address'], 16)
           for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))
           if r['remake_symbol'] and r['class'] == 'ENGINE'}
    oread, olo, ohi = load_pe(ORIG)
    pread, plo, phi = load_pe(a.exe)
    md = Cs(CS_ARCH_X86, CS_MODE_32); md.detail = True
    todo = a.names or sorted(led, key=led.get)
    ndiff = 0
    for name in todo:
        if name.startswith('0x'):
            name = next((n for n, v in led.items() if v == int(name, 16)), name)
        if name not in by_name or name not in led: continue
        pva = by_name[name]; k = bisect.bisect_right(starts, pva)
        plen = (starts[k] - pva) if k < len(starts) else 0x400
        P = list(md.disasm(pread(pva, plen), pva))
        # trim the port at trailing int3 padding
        while P and P[-1].mnemonic == 'int3': P.pop()
        O = list(md.disasm(oread(led[name], plen + 64), led[name]))[:len(P)]
        for j, (o, p) in enumerate(zip(O, P)):
            so, sp = sig(o, olo, ohi, plo, phi), sig(p, olo, ohi, plo, phi)
            if so != sp:
                ndiff += 1
                print('%-44s 0x%08x insn %3d: original %-40s | port %s' % (name, led[name], j, o.mnemonic + ' ' + o.op_str, p.mnemonic + ' ' + p.op_str))
                break
    print('%d function(s) differ' % ndiff, file=sys.stderr)

if __name__ == '__main__':
    main()
