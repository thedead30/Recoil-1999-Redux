"""port_ready.py - which unported functions in a listing can be ported now (bottom-up), and what blocks the rest.

usage: python 03_re/scripts/port_ready.py LISTING [LISTING ...] [--blockers N]

A function is ready when every call / jump / pushed code address it references is inside itself, outside the code
range, an import slot the platform layer declares (g_Iat_*), or a function the ledger already records a remake symbol
for. Thunks (a JMP or JMP [slot] at the target) are followed through the original binary when it is present
(00_original/game_install/Recoil.exe - local sessions only); without it a thunk counts as a blocker, so a cloud
session may see a few more blockers than a local one, never fewer.
Listings: 03_re/listings/*.txt, 03_re/listings/unported/<subsystem>.txt (DumpListingsFile.java format).
"""
import csv, glob, io, os, re, struct, sys, collections

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
args = [a for a in sys.argv[1:] if not a.startswith('--')]
nblock = int(sys.argv[sys.argv.index('--blockers') + 1]) if '--blockers' in sys.argv else 25
if not args:
    sys.exit(__doc__)
L = {}
for path in args:
    if path.isdigit() and '--blockers' in sys.argv and sys.argv[sys.argv.index('--blockers') + 1] == path:
        continue
    for x in io.open(path, encoding='utf-8').read().split('### '):
        if x[:8].strip():
            L[x[:8]] = x
led = {r['address'].lower(): r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))}
sub = {m['address'].lower(): m['subsystem'] for m in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/subsystems.csv'), encoding='utf-8'))}
slots = set()
for h in glob.glob(os.path.join(ROOT, '05_remake/src/platform/*.h')):
    slots |= set('0x' + s for s in re.findall(r'g_Iat_\w+_([0-9a-f]{8});', io.open(h, encoding='utf-8').read()))
exe = os.path.join(ROOT, '00_original/game_install/Recoil.exe')
b = open(exe, 'rb').read() if os.path.exists(exe) else None


def text(va):
    return b[va - 0x401000 + 0x400: va - 0x401000 + 0x406] if b else b''


def ok(v, depth=0):
    c8 = '0x%08x' % v
    if c8 in slots:
        return True
    cr = led.get(c8)
    if cr and cr['remake_symbol']:
        return True
    op = text(v)
    if depth < 4 and op[:1] == b'\xe9':
        return ok(v + 5 + struct.unpack_from('<i', op, 1)[0], depth + 1)
    if op[:2] == b'\xff\x25':
        return '0x%08x' % struct.unpack_from('<I', op, 2)[0] in slots
    return False


ready, need = [], collections.defaultdict(set)
for a, body in sorted(L.items()):
    r = led.get('0x' + a)
    if not r or r['remake_symbol']:
        continue
    own = {int(l.split()[0], 16) for l in body.splitlines()[1:] if l.strip()}
    deps = {int(x, 16) for x in re.findall(r'\b(?:CALL|JMP)\s+(?:dword ptr \[)?0x([0-9a-f]+)', body)}
    # a code address stored into memory (MOV dword ptr [0x0057d9e0],0x476cf0 - a callback / function-pointer install) is a reference too
    deps |= {int(x, 16) for x in re.findall(r'(?:PUSH|MOV (?:\w+|(?:dword ptr )?\[[^\]]*\]),)\s*0x(4[0-9a-c][0-9a-f]{4})\b', body)}
    bad = [v for v in deps if not (v in own or not (0x401000 <= v < 0x4cc000) or ok(v))]
    if not bad:
        ready.append('0x%s %s' % (a, r['ghidra_name']))
    for v in bad:
        need['0x%08x' % v].add(a)
print('ready (%d):' % len(ready))
for x in ready:
    print('  ' + x)
print('top blockers (callee, subsystem, in these listings?, unported callers):')
for c8, users in sorted(need.items(), key=lambda kv: -len(kv[1]))[:nblock]:
    cr = led.get(c8, {})
    print('  %s %s subsys=%s %s used by %d' % (c8, cr.get('ghidra_name', '?'), sub.get(c8, '-'),
                                              'here' if c8[2:] in L else 'OUTSIDE', len(users)))
if b is None:
    print('(Recoil.exe not present: thunks not followed)')
