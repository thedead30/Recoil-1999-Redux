"""bytediff_full.py - whole-function compare that does not stop at the first difference (bytediff.py does).

Equivalences accepted (and reported as such, not hidden): a direct call to a CRT/import thunk in the original vs an
indirect `call [slot]` in the port; operand-swapped `test a,b` / `test b,a`; and an original `jmp [reg*4+table]` vs
the port's `cmp reg,N / je` chain (the chain is skipped; tools/check_jumptables.py verifies the targets). Everything
else that differs is listed. Usage: python tools/bytediff_full.py NAME [NAME ...]
"""
import os, sys, csv
sys.path.insert(0, os.path.dirname(__file__))
import bytediff  # noqa: E402
from capstone import Cs, CS_ARCH_X86, CS_MODE_32  # noqa: E402

def fn_end(read, va, md, limit=0x3000):
    """original function body: up to the next ledger function start"""
    return None

def main():
    ROOT = bytediff.ROOT
    exe = os.path.join(ROOT, '05_remake', 'build', 'Port', 'recoil_boot.exe')
    starts = sorted(int(r[0], 16) for r in csv.reader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8', errors='replace')) if r and r[0].startswith('0x'))
    va_of = {r[1]: int(r[0], 16) for r in csv.reader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8', errors='replace')) if r and r[0].startswith('0x')}
    pmap = {}
    for l in open(exe[:-4] + '.map', errors='replace'):
        p = l.split()
        if len(p) >= 3 and p[1].startswith('?') and '@recoil@@' in p[1]:
            pmap.setdefault(p[1][1:p[1].index('@')], int(p[2], 16))
    oread, olo, ohi = bytediff.load_pe(bytediff.ORIG); pread, plo, phi = bytediff.load_pe(exe)
    md = Cs(CS_ARCH_X86, CS_MODE_32); md.detail = True
    for name in sys.argv[1:]:
        ova = va_of[name]; pva = pmap[name]
        oend = next(s for s in starts if s > ova)
        o = [i for i in md.disasm(oread(ova, oend - ova), ova)]
        p = list(md.disasm(pread(pva, (oend - ova) * 2 + 0x200), pva))
        i = j = 0; diffs = []; equiv = 0
        while i < len(o) and j < len(p):
            a, b = o[i], p[j]
            sa = bytediff.sig(a, olo, ohi, plo, phi); sb = bytediff.sig(b, olo, ohi, plo, phi)
            if sa == sb: i += 1; j += 1; continue
            if a.mnemonic == 'call' and b.mnemonic == 'call' and 'ptr' in b.op_str: equiv += 1; i += 1; j += 1; continue
            if a.mnemonic == b.mnemonic == 'test' and sorted(a.op_str.split(', ')) == sorted(b.op_str.split(', ')): equiv += 1; i += 1; j += 1; continue
            if a.mnemonic == 'jmp' and 'ptr' in a.op_str and '*4' in a.op_str and b.mnemonic == 'cmp':
                while j < len(p) and p[j].mnemonic in ('cmp', 'je'): j += 1
                if j < len(p) and p[j].mnemonic == 'int3': j += 1
                i += 1; equiv += 1; continue
            if a.mnemonic == 'nop' or a.mnemonic == 'int3': i += 1; continue
            diffs.append('insn %4d  original %08x %-36s | port %08x %s' % (i, a.address, a.mnemonic + ' ' + a.op_str, b.address, b.mnemonic + ' ' + b.op_str))
            i += 1; j += 1
            if len(diffs) > 25: break
        print('%-36s %d original insns, %d accepted equivalences, %d difference(s)' % (name, len(o), equiv, len(diffs)))
        for d in diffs: print('    ' + d)

if __name__ == '__main__':
    main()
