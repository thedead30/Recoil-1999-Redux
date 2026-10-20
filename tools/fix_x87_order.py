"""fix_x87_order.py - repair x87 register-register instructions whose operand order the porter flipped.

Found 2026-10-06 (playtest: camera swings, wrong projectile directions): ghidra2masm wrote e.g. `fmul st(0), st(1)` (D8 C9,
result in ST0) where the original has DC C9 (`fmul st(1), st(0)`, result in ST1). For every ported function, the compiled
port (Port build) and the original bytes are decoded and aligned by mnemonic sequence; each aligned x87 two-register
arithmetic instruction whose bytes differ is rewritten in the source to the original's operand text. Run, rebuild, run
again: the second run must report 0 fixes (bytes now equal).

usage: python tools/fix_x87_order.py [--dry]
"""
import bisect, csv, difflib, glob, io, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bytediff as B
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

ROOT = B.ROOT
X87 = {'fadd', 'fsub', 'fsubr', 'fmul', 'fdiv', 'fdivr', 'faddp', 'fsubp', 'fsubrp', 'fmulp', 'fdivp', 'fdivrp'}

def is_x87_rr(i):
    return i.mnemonic in X87 and re.fullmatch(r'st\(\d\)(, st\(\d\))?', i.op_str or '') is not None

def body_lines(src, name):
    m = re.search(r'__declspec\(naked\)[^\n(]*?\b' + re.escape(name) + r'\s*\([^)]*\)\s*\{\s*__asm\s*\{\n(.*?)\n\s*\}\s*\n\}', src, re.S)
    if not m: return None, None
    start = m.start(1); lines = []; pos = start
    for ln in m.group(1).split('\n'):
        code = ln.split(';')[0].split('//')[0].strip()
        if code and not re.fullmatch(r'[A-Za-z_]\w*:', code):
            lines.append((pos, ln))
        pos += len(ln) + 1
    return lines, m

def main():
    dry = '--dry' in sys.argv
    exe = os.path.join(ROOT, '05_remake', 'build', 'Port', 'recoil_boot.exe')
    syms = []
    for line in open(exe[:-4] + '.map', errors='replace'):
        m = re.match(r'\s*0001:[0-9a-f]{8}\s+\?(\w+)@recoil@@\S*\s+([0-9a-f]{8})', line)
        if m: syms.append((int(m.group(2), 16), m.group(1)))
    syms.sort(); starts = [s[0] for s in syms]; by_name = {n: v for v, n in syms}
    rows = [r for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))
            if r['class'] == 'ENGINE' and r['remake_file']]
    pread, _, _ = B.load_pe(exe); oread, _, _ = B.load_pe(B.ORIG)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    files = {}
    def src_of(rel):
        p = os.path.normcase(os.path.normpath(os.path.join(ROOT, rel if rel.startswith('05_remake') else os.path.join('05_remake', rel))))
        if p not in files: files[p] = io.open(p, encoding='utf-8', newline='').read()
        return p
    nfix = nfun = 0; skipped = []
    for r in rows:
        name = r['remake_symbol'].split('::')[-1]
        if name not in by_name: continue
        pva = by_name[name]; k = bisect.bisect_right(starts, pva)
        plen = (starts[k] - pva) if k < len(starts) else 0x400
        P = list(md.disasm(pread(pva, plen), pva))
        while P and P[-1].mnemonic == 'int3': P.pop()
        O = list(md.disasm(oread(int(r['address'], 16), plen + 256), int(r['address'], 16)))[:len(P) + 64]
        bad = []
        sm = difflib.SequenceMatcher(None, [i.mnemonic for i in O], [i.mnemonic for i in P], autojunk=False)
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag != 'equal': continue
            for a, b in zip(range(i1, i2), range(j1, j2)):
                if is_x87_rr(O[a]) and O[a].bytes != P[b].bytes:
                    bad.append((b, O[a]))
        if not bad: continue
        try:
            path = src_of(r['remake_file'])
        except OSError:
            skipped.append(name); continue
        lines, m = body_lines(files[path], name)
        if not lines or len(lines) != len(P):
            skipped.append('%s (source %s insns, binary %d)' % (name, len(lines) if lines else None, len(P))); continue
        s = files[path]
        for b, o in sorted(bad, reverse=True):         # edit from the end so earlier offsets stay valid
            pos, ln = lines[b]
            code = ln.strip().split()[0].lower()
            if code != P[b].mnemonic:
                skipped.append('%s insn %d (source %r vs binary %s)' % (name, b, ln.strip(), P[b].mnemonic)); continue
            indent = ln[:len(ln) - len(ln.lstrip())]
            new = '%s%s %s' % (indent, o.mnemonic, o.op_str)
            s = s[:pos] + new + s[pos + len(ln):]
            nfix += 1
        files[path] = s; nfun += 1; print('  fn %-44s %s  %d insn(s)' % (name, r['address'], len(bad)))
    if not dry:
        for p, s in files.items():
            io.open(p, 'w', encoding='utf-8', newline='').write(s)
    print('%s %d instruction(s) in %d function(s); skipped %d' % ('would fix' if dry else 'fixed', nfix, nfun, len(skipped)))
    for x in skipped[:40]: print('  skipped:', x)

if __name__ == '__main__':
    main()
