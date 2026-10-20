"""check_port_shape.py - two mechanical checks on the asm ports (both found by playtest 2026-10-06):
  1. fall-through: a naked port whose last instruction is not ret/jmp/int3 (the original falls into the next function;
     the port would run into padding) - 0x0042eec0 ended the game after mission load.
  2. x87 operand order: compiled port vs original bytes for two-register x87 arithmetic (tools/fix_x87_order.py --dry).
Exit 1 if either finds anything. usage: python tools/check_port_shape.py"""
import glob, io, re, subprocess, sys, os
ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
OK_LAST = re.compile(r'(ret|jmp|int 3|ud2|_emit|call dword ptr \[g_iat_exitprocess)')
bad = []
for p in glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.cpp'), recursive=True):
    s = io.open(p, encoding='utf-8', errors='replace').read()
    for m in re.finditer(r'__declspec\(naked\)[^\n]*?(\w+)\s*\([^)]*\)\s*\{\s*__asm\s*\{(.*?)\n\s*\}\s*\n\}', s, re.S):
        lines = [l.split(';')[0].split('//')[0].strip() for l in m.group(2).split('\n')]
        lines = [l for l in lines if l and not l.endswith(':')]
        if lines and not OK_LAST.match(lines[-1].lower()) and 'std::abort' not in lines[-1]:
            bad.append('fall-through: %s (%s) ends with "%s"' % (m.group(1), os.path.relpath(p, ROOT), lines[-1]))
r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'fix_x87_order.py'), '--dry'], capture_output=True, text=True)
first = (r.stdout.splitlines() or [''])[0]
if not first.startswith('would fix 0 '):
    bad.append('x87 operand order: ' + first)
for b in bad: print(b)
print('check_port_shape: %d problem(s)' % len(bad))
sys.exit(1 if bad else 0)
