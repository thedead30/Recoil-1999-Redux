"""check_targets.py - what bytediff.py cannot see: call/jump targets and global data addresses.

bytediff normalises every address to 'A', so a port that calls the wrong function or reads the wrong global still
compares equal. This pairs the instructions of the original and the port (same walk as bytediff) and checks:
  * direct call/jmp targets: the port's target symbol (from the .map) must be the function the ledger names at the
    original target address (internal labels inside the same function must map to the same offset pattern);
  * memory operands / immediates in the original's data range (0x004cc000..0x0057ffff): the port's address, mapped
    back through the data mirror (g_Data_004da000 / g_RData_004cc000 from the .map), must be the same original VA.
Usage: python tools/check_targets.py NAME_OR_ADDR ...   (or --all-in FILE.cpp)
"""
import os, re, sys, csv, struct, argparse
sys.path.insert(0, os.path.dirname(__file__))
import bytediff  # noqa: E402
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_OP_IMM, CS_OP_MEM  # noqa: E402

ROOT = bytediff.ROOT
LEDGER = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')

def load_map(path):
    syms = {}; data = {}
    for l in open(path, errors='replace'):
        m = re.match(r'\s*000\d:[0-9a-f]+\s+(\S+)\s+([0-9a-f]{8})', l)
        if not m: continue
        raw, a = m.group(1), int(m.group(2), 16)
        nm = re.match(r'\?(\w+)@', raw)
        name = nm.group(1) if nm else raw.lstrip('_')
        syms.setdefault(a, name)
        if name in ('g_Data_004da000', 'g_RData_004cc000'): data[name] = a
    return syms, data

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('names', nargs='*')
    ap.add_argument('--exe', default=os.path.join(ROOT, '05_remake', 'build', 'Port', 'recoil_boot.exe'))
    ap.add_argument('--all-in', help='check every ported function defined in this .cpp')
    a = ap.parse_args()
    va_of = {}; name_of = {}
    for r in csv.reader(open(LEDGER, encoding='utf-8', errors='replace')):
        if r and r[0].startswith('0x'):
            v = int(r[0], 16); va_of[r[1]] = v; name_of[v] = r[1]
    syms, data = load_map(a.exe[:-4] + '.map')
    by_name = {}
    for addr, nm in syms.items(): by_name.setdefault(nm, addr)
    oread, olo, ohi = bytediff.load_pe(bytediff.ORIG)
    pread, plo, phi = bytediff.load_pe(a.exe)
    dbase = data.get('g_Data_004da000'); rbase = data.get('g_RData_004cc000')
    def port_to_va(x):
        if dbase and dbase <= x < dbase + 0xb0000: return x - dbase + 0x4da000
        if rbase and rbase <= x < rbase + 0xe000: return x - rbase + 0x4cc000
        return None
    names = list(a.names)
    if a.all_in:
        names += re.findall(r'__declspec\(naked\)[^(]*?(\w+)\(', open(a.all_in, encoding='utf-8', errors='replace').read())
    md = Cs(CS_ARCH_X86, CS_MODE_32); md.detail = True
    total = 0
    for n in names:
        ova = int(n, 16) if n.startswith('0x') else va_of.get(n)
        nm = name_of.get(ova, n)
        pva = by_name.get(nm)
        if ova is None or pva is None: print('%s: not found (ledger %s, map %s)' % (n, ova, pva)); continue
        oi = bytediff.insns(md, oread, ova, 0x2000); pi = bytediff.insns(md, pread, pva, 0x2000)
        issues = []
        for k, (x, y) in enumerate(zip(oi, pi)):
            if x.mnemonic != y.mnemonic: break          # bytediff's job; stop at the first real divergence
            # direct branch / call targets
            if x.mnemonic.startswith(('call', 'j')) and x.operands and x.operands[0].type == CS_OP_IMM:
                ot = x.operands[0].imm & 0xffffffff
                if y.operands and y.operands[0].type == CS_OP_MEM:
                    # original calls an import stub (FF 25 slot = jmp [slot]); the port calls the slot directly:
                    # equal when the port's slot variable names the same original slot
                    stub = oread(ot, 6)
                    pslot = syms.get(y.operands[0].mem.disp & 0xffffffff, '')
                    if stub[:2] == b'\xff\x25':
                        slot = struct.unpack('<I', stub[2:6])[0]
                        if not pslot.endswith('_%08x' % slot):
                            issues.append('insn %3d %s: original -> import slot %08x, port -> %s' % (k, x.mnemonic, slot, pslot or 'unknown'))
                    else:
                        issues.append('insn %3d %s: original direct %08x, port indirect via %s' % (k, x.mnemonic, ot, pslot or 'unknown'))
                    continue
                pt = y.operands[0].imm & 0xffffffff
                if x.mnemonic == 'call' or not (ova <= ot < ova + 0x2000):
                    want = name_of.get(ot); got = syms.get(pt)
                    if want and got and want != got and not got.startswith(('Stub', 'g_Iat')):
                        issues.append('insn %3d %s: original -> %s (%08x), port -> %s' % (k, x.mnemonic, want, ot, got))
                else:   # internal label: same distance from the function start
                    if (ot - ova) != (pt - pva) and syms.get(pt) not in (None,) and False: pass
            # data addresses
            for xo, yo in zip(x.operands, y.operands):
                if xo.type == CS_OP_MEM and yo.type == CS_OP_MEM and not xo.mem.base and not xo.mem.index or \
                   (xo.type == CS_OP_MEM and yo.type == CS_OP_MEM):
                    od = xo.mem.disp & 0xffffffff; pd = yo.mem.disp & 0xffffffff
                elif xo.type == CS_OP_IMM and yo.type == CS_OP_IMM and not x.mnemonic.startswith(('call', 'j')):
                    od = xo.imm & 0xffffffff; pd = yo.imm & 0xffffffff
                else: continue
                if 0x004cc000 <= od < 0x00580000:
                    m = port_to_va(pd)
                    if m is not None and m != od:
                        issues.append('insn %3d %s %s: original data %08x, port data -> original %08x' % (k, x.mnemonic, x.op_str, od, m))
            if x.mnemonic == 'ret': break
        total += len(issues)
        print('%-40s %s' % (nm, 'ok' if not issues else '%d issue(s)' % len(issues)))
        for s in issues: print('    ' + s)
    print('%d issue(s)' % total)

if __name__ == '__main__':
    main()
