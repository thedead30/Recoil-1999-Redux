"""prove_identity.py - VERIFIED-IDENTICAL: prove an asm port is the original function, instruction for instruction.

A port is IDENTICAL when, over the WHOLE original body (up to the next ledger function) and the port body:
  1. every instruction pair has the same mnemonic and operands (bytediff.sig), except one accepted form: the original
     calls an import stub (FF 25 slot = jmp [slot]) and the port calls the same slot variable (g_Iat_*_<slot>) directly;
  2. every branch inside the function lands on the instruction with the same index on both sides;
  3. every call/jmp out of the function goes to the function the ledger names at the original target (original jump
     stubs E9 rel32 are followed), and the port symbol is that function;
  4. every operand value inside the original's data range (0x004cc000..0x0057ffff) maps back, through the port's data
     mirror (g_Data_004da000 / g_RData_004cc000) or the separately placed globals (kBlocks in original_data.cpp), to the
     same original address - an unmappable address FAILS; code addresses used as values (function pointers) must be the
     port symbol of the same ledger function;
  5. the port has no extra instructions after the original's last one (int3/nop padding excepted).
Switch tables rewritten as compare chains, hand-written C++ and anything else are NOT identical (they need tests).
The claim is per function and conditional on its callees and data: that is what the ledger tag says.
usage: python tools/prove_identity.py [NAME ...] [--all] [--out FILE]
"""
import os, re, sys, csv, struct, bisect
sys.path.insert(0, os.path.dirname(__file__))
import bytediff  # noqa: E402
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_OP_IMM, CS_OP_MEM  # noqa: E402

ROOT = bytediff.ROOT
EXE = os.environ.get('RECOIL_PROVE_EXE') or os.path.join(ROOT, '05_remake', 'build', 'Port', 'recoil_boot.exe')   # the build to prove (its .map beside it)
MAP = EXE[:-4] + '.map'
LEDGER = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')
DATA_LO, DATA_HI = 0x004cc000, 0x00580000
CODE_LO, CODE_HI = 0x00401000, 0x004cc000

led = [(int(r[0], 16), r[1]) for r in csv.reader(open(LEDGER, encoding='utf-8', errors='replace')) if r and r[0].startswith('0x')]
led.sort(); starts = [a for a, _ in led]; name_of = dict(led); va_of = {n: a for a, n in led}
_hdr = next(csv.reader(open(LEDGER, encoding='utf-8', errors='replace')))
_rs = _hdr.index('remake_symbol')
remake_sym = {}
for _r in csv.reader(open(LEDGER, encoding='utf-8', errors='replace')):
    if _r and _r[0].startswith('0x') and len(_r) > _rs and _r[_rs]: remake_sym.setdefault(_r[_rs], set()).add(int(_r[0], 16))
syms = {}; sym_addr = {}; names_at = {}   # identical-code folding: one address, several names
for l in open(MAP, errors='replace'):
    m = re.match(r'\s*000[1-9]:[0-9a-f]+\s+(\S+)\s+([0-9a-f]{8})', l)
    if not m: continue
    raw, a = m.group(1), int(m.group(2), 16)
    n = re.match(r'\?(\w+)@recoil@@', raw)
    name = n.group(1) if n else raw.lstrip('_')
    syms.setdefault(a, name); sym_addr.setdefault(name, a); names_at.setdefault(a, set()).add(name)
pstarts = sorted(syms)
dbase = sym_addr.get('g_Data_004da000'); rbase = sym_addr.get('g_RData_004cc000')
blocks = []   # (port address, size, original va) of separately placed globals
src = open(os.path.join(ROOT, '05_remake', 'src', 'platform', 'image', 'original_data.cpp'), encoding='utf-8', errors='replace').read()
for va, size, g in re.findall(r'\{0x([0-9a-f]{8}), 0x([0-9a-f]+), \(void\*\)&(\w+)\}', src):
    if g in sym_addr: blocks.append((sym_addr[g], int(size, 16), int(va, 16)))
oread, _, _ = bytediff.load_pe(bytediff.ORIG); pread, plo, phi = bytediff.load_pe(EXE)
olo, ohi = DATA_LO, DATA_HI
md = Cs(CS_ARCH_X86, CS_MODE_32); md.detail = True

def port_to_va(x):
    if dbase and dbase <= x < dbase + 0xb0000: return x - dbase + 0x4da000
    if rbase and rbase <= x < rbase + 0xe000: return x - rbase + 0x4cc000
    for a, n, va in blocks:
        if a <= x < a + n: return va + (x - a)
    return None

def follow_stub(t):
    for _ in range(4):
        b = oread(t, 5)
        if b[:1] == b'\xe9': t = (t + 5 + struct.unpack('<i', b[1:5])[0]) & 0xffffffff
        else: break
    return t

WRITTEN = None
def written_set():
    """data addresses some original instruction writes through a direct [disp] operand"""
    global WRITTEN
    if WRITTEN is None:
        from capstone import CS_AC_WRITE
        WRITTEN = set()
        for a, b in zip(starts, starts[1:] + [CODE_HI]):
            if not (CODE_LO <= a < CODE_HI): continue
            for i in md.disasm(oread(a, min(b - a, 0x8000)), a):
                for o in i.operands:
                    if o.type == CS_OP_MEM and (o.access & CS_AC_WRITE) and not o.mem.base and not o.mem.index:
                        d = o.mem.disp & 0xffffffff
                        if DATA_LO <= d < DATA_HI: WRITTEN.update(range(d, d + max(o.size, 1)))
    return WRITTEN

def code_eq(ov, pv):
    """a code pointer stored as data: the port symbol is the port of the same ledger function (stubs followed)"""
    if ov == 0 and pv == 0: return True
    want = (ov, follow_stub(ov))
    return any(va_of.get(g) in want or (remake_sym.get(g, set()) & set(want)) for g in names_at.get(pv, ()))

def u32(read, a): return struct.unpack('<I', read(a, 4))[0]

def eh_equal(ov, pv):
    """MSVC FuncInfo (magic 0x19930520) and everything it points to, field by field"""
    if u32(oread, ov) != 0x19930520 or u32(pread, pv) != 0x19930520: return False
    ms, ps = u32(oread, ov + 4), u32(pread, pv + 4)
    if ms != ps: return False
    ou, pu = u32(oread, ov + 8), u32(pread, pv + 8)
    for k in range(ms):
        if u32(oread, ou + 8 * k) != u32(pread, pu + 8 * k): return False
        if not code_eq(u32(oread, ou + 8 * k + 4), u32(pread, pu + 8 * k + 4)): return False
    nt = u32(oread, ov + 12)
    if nt != u32(pread, pv + 12): return False
    ot_, pt_ = u32(oread, ov + 16), u32(pread, pv + 16)
    for k in range(nt):
        oe, pe = ot_ + 20 * k, pt_ + 20 * k
        for f in (0, 4, 8, 12):
            if u32(oread, oe + f) != u32(pread, pe + f): return False
        oh, ph = u32(oread, oe + 16), u32(pread, pe + 16)
        for c in range(u32(oread, oe + 12)):
            a, b = oh + 16 * c, ph + 16 * c
            if u32(oread, a) != u32(pread, b) or u32(oread, a + 8) != u32(pread, b + 8): return False
            otd, ptd = u32(oread, a + 4), u32(pread, b + 4)
            if not ((otd == 0 and ptd == 0) or port_to_va(ptd) == otd): return False
            if not code_eq(u32(oread, a + 12), u32(pread, b + 12)): return False
    return True

def data_equal(ov, pv, size, is_load):
    """a reference to a generated copy of original data: same contents.
    EH FuncInfo (starts with 0x19930520) must pass eh_equal - no fallback. Byte comparison of a constant only for a
    value the instruction LOADS from memory (is_load), over exactly the operand size; an address used as a number
    (immediate) is accepted only as an EH table or a whole constant string."""
    try:
        if u32(oread, ov) == 0x19930520:
            return 'eh-table' if eh_equal(ov, pv) else None
    except Exception:
        return None
    try:
        if is_load and ov < 0x004da000 and size in (1, 2, 4, 8, 10):   # .rdata: read-only constants
            if oread(ov, size) == pread(pv, size): return 'rdata-const'
        sym = syms.get(pv, '')
        if 'kStr_' in sym or sym.startswith('kStr'):
            ob = oread(ov, 512); pb = pread(pv, 512)
            z = ob.find(b'\0')
            if z >= 0 and ob[:z + 1] == pb[:z + 1] and not (set(range(ov, ov + z + 1)) & written_set()): return 'const-string'
    except Exception: pass
    return None

SWITCH_JMP = re.compile(r'^dword ptr \[(\w+)\*4 \+ (0x[0-9a-f]+)\]$')
BYTE_IDX = re.compile(r'^(\w+), byte ptr \[(\w+) \+ (0x[0-9a-f]+)\]$')

PAD = {('nop', ''), ('int3', ''), ('mov', 'edi, edi'), ('lea', 'esi, [esi]'), ('lea', 'edi, [edi]'), ('lea', 'esp, [esp]')}
def is_pad(x):
    return (x.mnemonic, x.op_str) in PAD or (x.mnemonic == 'lea' and re.match(r'^(\w+), \[\1( \+ 0)?\]$', x.op_str))

def ln_is_xor(o, i):
    return o[i].mnemonic == 'xor' and len(set(o[i].op_str.split(', '))) == 1

def switch_unit(o, i):
    """original switch dispatch starting at o[i]: (length, index register, dword table, byte table or None)"""
    x = o[i]
    if x.mnemonic == 'jmp':
        m = SWITCH_JMP.match(x.op_str)
        if m: return 1, m.group(1), int(m.group(2), 16), None
    if x.mnemonic == 'xor' and i + 2 < len(o) and o[i + 1].mnemonic == 'mov' and o[i + 2].mnemonic == 'jmp':
        mb = BYTE_IDX.match(o[i + 1].op_str); mj = SWITCH_JMP.match(o[i + 2].op_str)
        if mb and mj: return 3, mb.group(2), int(mj.group(2), 16), int(mb.group(3), 16)
    if x.mnemonic == 'mov' and i + 1 < len(o) and o[i + 1].mnemonic == 'jmp':
        mb = BYTE_IDX.match(x.op_str); mj = SWITCH_JMP.match(o[i + 1].op_str)
        if mb and mj: return 2, mb.group(2), int(mj.group(2), 16), int(mb.group(3), 16)
    return None

def port_chain(p, j, reg):
    """port compare chain at p[j]: ({value: target}, default target or None, length)"""
    cases = {}; k = j
    while k + 1 < len(p) and p[k].mnemonic == 'cmp' and p[k + 1].mnemonic == 'je':
        ops = p[k].op_str.split(', ')
        if ops[0] != reg or not p[k].operands or p[k].operands[1].type != CS_OP_IMM: break
        cases[p[k].operands[1].imm & 0xffffffff] = p[k + 1].operands[0].imm & 0xffffffff
        k += 2
    if not cases: return None
    default = None
    if k < len(p) and p[k].mnemonic == 'int3': k += 1
    elif k < len(p) and p[k].mnemonic == 'jmp' and p[k].operands[0].type == CS_OP_IMM:
        default = p[k].operands[0].imm & 0xffffffff; k += 1
    return cases, default, k - j

def bound_before(o, i, reg):
    """the switch's bounds check: the nearest preceding 'cmp reg, N' (N = highest case)"""
    for k in range(i - 1, max(-1, i - 12), -1):
        if o[k].mnemonic == 'cmp' and o[k].op_str.split(', ')[0] == reg and o[k].operands[1].type == CS_OP_IMM:
            return o[k].operands[1].imm & 0xffffffff
    return None

def port_chain_b(p, j, reg, breg):
    """byte-table emulation: groups 'cmp reg, v / jne next / mov breg, k / jmp case' then int3.
    Returns ({v: (k, target)}, length) - the port sets the byte register exactly as the table load would."""
    cases = {}; k = j
    while k + 3 < len(p) and p[k].mnemonic == 'cmp' and p[k + 1].mnemonic == 'jne' and p[k + 2].mnemonic == 'mov' \
            and p[k + 3].mnemonic == 'jmp':
        ops = p[k].op_str.split(', '); mv = p[k + 2].op_str.split(', ')
        if ops[0] != reg or p[k].operands[1].type != CS_OP_IMM or mv[0] != breg or p[k + 2].operands[1].type != CS_OP_IMM: break
        if p[k + 3].operands[0].type != CS_OP_IMM: break
        if p[k + 1].operands[0].imm & 0xffffffff != p[k + 4].address if k + 4 < len(p) else True: break
        cases[p[k].operands[1].imm & 0xffffffff] = (p[k + 2].operands[1].imm & 0xff, p[k + 3].operands[0].imm & 0xffffffff)
        k += 4
    if not cases: return None
    if k < len(p) and p[k].mnemonic == 'int3': k += 1
    else: return None
    return cases, k - j

NORETURN_SLOTS = {0x004cc0ec, 0x004cc564}   # KERNEL32 ExitProcess, msvcrt _exit: calls never return
UNCOND = ('ret', 'jmp')

def is_noreturn_call(o, i):
    x = o[i]
    if x.mnemonic != 'call' or not x.operands: return False
    op = x.operands[0]
    if op.type == CS_OP_MEM and not op.mem.base and not op.mem.index: return (op.mem.disp & 0xffffffff) in NORETURN_SLOTS
    if op.type == CS_OP_IMM:
        t = follow_stub(op.imm & 0xffffffff); b = oread(t, 6)
        return b[:2] == b'\xff\x25' and struct.unpack('<I', b[2:6])[0] in NORETURN_SLOTS
    if op.type == 1:   # register: the nearest preceding load of that register
        reg = x.op_str
        for k in range(i - 1, max(-1, i - 30), -1):
            ops = o[k].op_str.split(', ')
            if ops and ops[0] == reg and o[k].mnemonic != 'push':
                m = re.match(r'^dword ptr \[(0x[0-9a-f]+)\]$', ops[1]) if o[k].mnemonic == 'mov' and len(ops) > 1 else None
                return bool(m) and int(m.group(1), 16) in NORETURN_SLOTS
    return False

def slot_of_stub(t):
    t = follow_stub(t); b = oread(t, 6)
    return struct.unpack('<I', b[2:6])[0] if b[:2] == b'\xff\x25' else None

def didataformat_equal(ov, pv):
    """DirectInput DIDATAFORMAT (c_dfDIKeyboard / Mouse / Joystick2): fields and every object's GUID by value"""
    try:
        of = struct.unpack('<5I', oread(ov, 20)); pf = struct.unpack('<5I', pread(pv, 20))
        if of[:4] != pf[:4] or of[0] != 24 or of[1] != 16: return False
        n = of[4]
        if n != pf[4] or n > 1024: return False
        oo, po = struct.unpack('<I', oread(ov + 20, 4))[0], struct.unpack('<I', pread(pv + 20, 4))[0]
        for k in range(n):
            a = struct.unpack('<4I', oread(oo + 16 * k, 16)); b = struct.unpack('<4I', pread(po + 16 * k, 16))
            if a[1:] != b[1:]: return False
            if (a[0] == 0) != (b[0] == 0): return False
            if a[0] and oread(a[0], 16) != pread(b[0], 16): return False
        return True
    except Exception:
        return False

def region(ova, oend, cuts=()):
    o = list(md.disasm(oread(ova, oend - ova), ova))
    ranges = []
    for _ in range(4):
        found = list(ranges)
        for n in range(len(o)):
            u = switch_unit(o, n)
            if not u: continue
            hi = bound_before(o, n, u[1])
            if hi is None: continue
            if u[3]:
                nb = hi + 1; ne = max(oread(u[3], nb)) + 1
                found.append((u[3], u[3] + nb))
            else:
                ne = hi + 1
            found.append((u[2], u[2] + 4 * ne))
        found = sorted(set((a, b) for a, b in found if ova <= a < oend))
        if found == ranges: break
        ranges = found
        o = []; at = ova
        for a, b in ranges + [(oend, oend)]:
            if a > at:
                seg = list(md.disasm(oread(at, a - at), at))
                while seg and is_pad(seg[-1]): seg.pop()
                o += seg
            at = max(at, b)
    if cuts:
        keep = []
        for k, x in enumerate(o):
            if any(a <= x.address < b for a, b in cuts): continue
            keep.append(x)
        o = keep
    while o and is_pad(o[-1]): o.pop()
    # alignment filler right after an unconditional exit is never executed: drop it (a branch into it still fails)
    o = [x for k, x in enumerate(o) if not (is_pad(x) and k and (o[k - 1].mnemonic in UNCOND or is_pad(o[k - 1]))
                                             and any(o[q].mnemonic in UNCOND for q in range(max(0, k - 16), k)))]
    return o

def prove(name):
    """try the ledger extent first, then extend over following ledger rows that have no port of their own
    (labels inside this function); returns (ok, why, covered interior names)"""
    ova = va_of.get(name); pva = sym_addr.get(name)
    if ova is None or pva is None: return False, 'no ledger row or port symbol'
    k = bisect.bisect_right(starts, ova)
    first = None
    for ext in range(4):
        if k + ext > len(starts): break
        oend = starts[k + ext] if k + ext < len(starts) else ova + 0x400
        interior = [name_of[a] for a in starts[k:k + ext]]
        cuts = []
        full = list(md.disasm(oread(ova, oend - ova), ova))
        main_targets = branch_targets(full, ova, oend)
        for a in starts[k:k + ext]:
            if sym_addr.get(name_of[a]) is not None:
                q = bisect.bisect_right(starts, a)
                b = starts[q] if q < len(starts) else a + 0x400
                # the row ends where this function's own branches land again (shared epilogue / continuation)
                back = [t for t in main_targets if a < t < b]
                cuts.append((a, min(back) if back else b))
        attempts = [()]
        if cuts and not cuts_entered_by_fallthrough(ova, oend, cuts): attempts.append(tuple(cuts))
        ok = False
        for cu in attempts:
            ok, why, amap = prove_region(name, ova, oend, pva, cu)
            if ok:
                if cu: why += ' (skips separately ported %s)' % ', '.join(name_of[a] for a, _ in cu)
                break
        if not ok and first is None: first = (ok, why)
        if first is None: first = (ok, why)
        if ok:
            portless = [n for n in interior if sym_addr.get(n) is None]
            cov = [n for n in portless if amap.get(va_of[n]) is not None]
            if len(cov) != len(portless): continue
            if cov: why += ' (covers interior %s)' % ', '.join(cov)
            prove.covered = cov
            return True, why
    prove.covered = []
    return first
prove.covered = []

def cuts_entered_by_fallthrough(ova, oend, cuts):
    full = list(md.disasm(oread(ova, oend - ova), ova))
    for a, b in cuts:
        before = [x for x in full if x.address < a]
        if not before or before[-1].mnemonic not in UNCOND: return True
    return False

def prove_region(name, ova, oend, pva, cuts=()):
    j = bisect.bisect_right(pstarts, pva)
    pend = pstarts[j] if j < len(pstarts) else pva + 0x400
    o = region(ova, oend, cuts)
    p = list(md.disasm(pread(pva, pend - pva), pva))
    while p and is_pad(p[-1]): p.pop()
    if not o: return False, 'empty original body', {}
    notes = set(); amap = {}; deferred = []; inline = []
    i = j = 0
    while i < len(o) and j < len(p):
        u = switch_unit(o, i)
        if u and u[3] and ln_is_xor(o, i) and p[j].mnemonic == 'xor' and p[j].op_str == o[i].op_str:
            ln, reg, tab, btab = u
            breg = o[i + 1].op_str.split(', ')[0]
            chb = port_chain_b(p, j + 1, reg, breg)
            if chb:
                cases, pl = chb
                hi = bound_before(o, i, reg)
                if hi is None: return False, 'insn %d: switch without a bounds check' % i, amap
                if sorted(cases) != list(range(hi + 1)): return False, 'insn %d: byte-table chain does not cover 0..%d' % (i, hi), amap
                for v in range(hi + 1):
                    b = oread(btab + v, 1)[0]
                    k_, pt = cases[v]
                    if k_ != b: return False, 'insn %d: case %d sets %s=%d, table byte is %d' % (i, v, breg, k_, b), amap
                    deferred.append((i, u32(oread, tab + 4 * b), pt, 'byte-switch case %d' % v))
                amap[o[i].address] = p[j].address
                i += ln; j += 1 + pl; notes.add('switch-chain'); continue
        if u:
            ln, reg, tab, btab = u
            ch = port_chain(p, j, reg)
            if not ch: return False, 'insn %d: switch without a matching port compare chain' % i, amap
            cases, default, pl = ch
            hi = bound_before(o, i, reg)
            if hi is None: return False, 'insn %d: switch without a bounds check' % i, amap
            for v in range(hi + 1):
                slot = oread(btab + v, 1)[0] if btab else v
                pt = cases.get(v, default)
                if pt is None: return False, 'insn %d: case %d missing in the port chain' % (i, v), amap
                deferred.append((i, u32(oread, tab + 4 * slot), pt, 'switch case %d' % v))
            if any(v > hi for v in cases): return False, 'insn %d: port chain has a case beyond the bound' % i, amap
            amap[o[i].address] = p[j].address
            i += ln; j += pl; notes.add('switch-chain'); continue
        x, y = o[i], p[j]
        amap[x.address] = y.address
        ok, why = pair(x, y, i, ova, oend, notes, deferred, inline, pva, pend)
        if not ok:
            # dead code after a call that never returns (ExitProcess / _exit): the port may omit it up to the next
            # instruction something branches to
            if i and is_noreturn_call(o, i - 1):
                targets = branch_targets(o, ova, oend)
                k2 = i
                while k2 < len(o) and o[k2].address not in targets: k2 += 1
                if k2 > i and k2 < len(o):
                    del amap[x.address]
                    i = k2; notes.add('noreturn-dead-code'); continue
            if i and o[i - 1].mnemonic in UNCOND:
                del amap[x.address]
                break                                       # leftover handled as tail-not-owned below
            return False, why, amap
        i += 1; j += 1
    if i < len(o) or j < len(p):
        last_o = o[i - 1] if i else None
        if i == len(o) and j == len(p) - 1 and last_o is not None and last_o.mnemonic not in UNCOND \
                and p[j].mnemonic == 'jmp' and p[j].operands[0].type == CS_OP_IMM and code_eq(oend, p[j].operands[0].imm & 0xffffffff):
            notes.add('fallthrough-jmp'); j += 1          # the original falls into the next function; the port jumps there
        elif last_o is not None and last_o.mnemonic in UNCOND:
            notes.add('tail-not-owned')                     # code beyond the last unconditional exit, reached by nothing here
        else:
            return False, 'length differs: %d of %d original / %d of %d port instructions matched' % (i, len(o), j, len(p)), amap
    # inline copies: an external original target whose port target is a label inside this port
    for n, ot, pt in inline:
        k = bisect.bisect_right(starts, ot)
        cend = starts[k] if k < len(starts) else ot + 0x400
        oc = list(md.disasm(oread(ot, cend - ot), ot))
        pj = next((q for q, yy in enumerate(p) if yy.address == pt), None)
        if pj is None: return False, 'insn %d: inline target not found' % n, amap
        q = 0
        while True:
            if q >= len(oc) or pj + q >= len(p): return False, 'insn %d: inline copy of %08x ends early' % (n, ot), amap
            amap[oc[q].address] = p[pj + q].address
            ok, why = pair(oc[q], p[pj + q], n, ot, cend, notes, deferred, inline, pva, pend)
            if not ok: return False, 'inline copy of %08x: %s' % (ot, why), amap
            if oc[q].mnemonic in UNCOND and all(d[1] < ot or d[1] <= oc[q].address for d in deferred if ot <= d[1] < cend): break
            q += 1
        notes.add('inline-copy')
    for n, ot, pt, what in deferred:
        if amap.get(ot) != pt: return False, 'insn %d: %s lands on a different instruction' % (n, what), amap
    return True, '%d instructions%s' % (len(o), (' (accepted: ' + ', '.join(sorted(notes)) + ')') if notes else ''), amap

def branch_targets(o, ova, oend):
    t = set()
    for x in o:
        if (x.mnemonic.startswith('j') or x.mnemonic.startswith('loop')) and x.operands and x.operands[0].type == CS_OP_IMM:
            v = x.operands[0].imm & 0xffffffff
            if ova <= v < oend: t.add(v)
    return t

def imm_equal(x, y):
    """same mnemonic and operands; immediates compared at the operand's width (imm8 sign-extended vs imm16/32)"""
    if x.mnemonic != y.mnemonic or len(x.operands) != len(y.operands): return False
    for xo, yo in zip(x.operands, y.operands):
        if xo.type != yo.type: return False
        if xo.type == CS_OP_IMM:
            mask = (1 << (8 * max(xo.size, 1))) - 1
            if (xo.imm & mask) != (yo.imm & mask): return False
            if DATA_LO <= (xo.imm & 0xffffffff) < DATA_HI or CODE_LO <= (xo.imm & 0xffffffff) < CODE_HI: return False
        elif xo.type == 1:
            if xo.reg != yo.reg: return False
        else:
            return False
    return True

def pair(x, y, n, ova, oend, notes, deferred, inline, pva, pend):
    if x.mnemonic == 'ret' and y.mnemonic == 'ret' and x.op_str in ('', '0') and y.op_str in ('', '0'): return True, ''
    if x.mnemonic != y.mnemonic: return False, 'insn %d: %s vs %s' % (n, x.mnemonic, y.mnemonic)
    sx = bytediff.sig(x, olo, ohi, plo, phi); sy = bytediff.sig(y, olo, ohi, plo, phi)
    branch = x.mnemonic == 'call' or x.mnemonic.startswith('j') or x.mnemonic.startswith('loop')
    if branch and x.operands and x.operands[0].type == CS_OP_IMM:
        ot = x.operands[0].imm & 0xffffffff
        if ova <= ot < oend:   # internal branch: checked against the alignment once it is complete
            if not (y.operands and y.operands[0].type == CS_OP_IMM): return False, 'insn %d: internal branch became indirect' % n
            deferred.append((n, ot, y.operands[0].imm & 0xffffffff, 'branch'))
            return True, ''
        if y.operands and y.operands[0].type == CS_OP_MEM:   # import stub (possibly behind a jump stub) vs slot
            slot = slot_of_stub(ot); slot_var = syms.get(y.operands[0].mem.disp & 0xffffffff, '')
            if slot is not None and slot_var.endswith('_%08x' % slot): return True, ''
            return False, 'insn %d: call %08x vs indirect %s' % (n, ot, slot_var or '?')
        pt = y.operands[0].imm & 0xffffffff
        if x.mnemonic == 'jmp' and pva <= pt < pend and not (ova <= ot < oend) and not code_eq(ot, pt):
            inline.append((n, ot, pt)); return True, ''      # tail jump into a copy of the original code at ot
        if not code_eq(ot, pt):
            want = name_of.get(follow_stub(ot)) or name_of.get(ot)
            return False, 'insn %d: %s -> %s (%08x) vs %s' % (n, x.mnemonic, want, ot, syms.get(pt))
        return True, ''
    if sx != sy:
        if x.mnemonic == 'test' and sorted(x.op_str.split(', ')) == sorted(y.op_str.split(', ')): return True, ''
        if imm_equal(x, y): notes.add('imm-encoding'); return True, ''
        if x.mnemonic == 'push' and x.operands and x.operands[0].type == CS_OP_IMM and y.operands[0].type == CS_OP_MEM:
            slot = slot_of_stub(x.operands[0].imm & 0xffffffff)   # push &import_stub vs push [slot]: same function
            if slot is not None and syms.get(y.operands[0].mem.disp & 0xffffffff, '').endswith('_%08x' % slot):
                notes.add('stub-pointer'); return True, ''
        return False, 'insn %d: %s %s vs %s %s' % (n, x.mnemonic, x.op_str, y.mnemonic, y.op_str)
    for xo, yo in zip(x.operands, y.operands):
        if xo.type == CS_OP_MEM: ov, pv = xo.mem.disp & 0xffffffff, yo.mem.disp & 0xffffffff
        elif xo.type == CS_OP_IMM: ov, pv = xo.imm & 0xffffffff, yo.imm & 0xffffffff
        else: continue
        if DATA_LO <= ov < DATA_HI:
            if port_to_va(pv) != ov:
                if not syms.get(pv, '').endswith('_%08x' % ov):
                    how = data_equal(ov, pv, xo.size, xo.type == CS_OP_MEM)
                    if not how: return False, 'insn %d: data %08x vs port %08x (%s)' % (n, ov, pv, syms.get(pv, 'unmapped'))
                    notes.add(how)
        elif CODE_LO <= ov < CODE_HI and xo.type == CS_OP_IMM:
            if not code_eq(ov, pv):
                if didataformat_equal(ov, pv): notes.add('dinput-format'); continue
                return False, 'insn %d: code address %08x (%s) vs %s' % (n, ov, name_of.get(ov), syms.get(pv))
            if name_of.get(ov) != syms.get(pv): notes.add('stub-pointer')
    return True, ''

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    out = sys.argv[sys.argv.index('--out') + 1] if '--out' in sys.argv else None
    if out in args: args.remove(out)
    names = args
    if '--all' in sys.argv: names = [n for _, n in led if n in sym_addr]
    ok = 0; lines = []; covered = {}
    for nme in names:
        good, why = prove(nme)
        ok += good
        lines.append('%s %s  %s' % ('IDENTICAL ' if good else 'not-proven', nme, why))
        if good:
            for c in prove.covered: covered[c] = nme
    for c, parent in covered.items():   # labels inside a proven function, with no port symbol of their own
        lines.append('IDENTICAL  %s  interior of %s (instructions covered by its proof)' % (c, parent)); ok += 1
    txt = '\n'.join(lines) + '\n%d of %d proven identical\n' % (ok, len(names) + len(covered))
    if out: open(out, 'w', encoding='utf-8').write(txt)
    print(txt if not out or len(names) < 20 else txt.splitlines()[-1])

if __name__ == '__main__':
    main()
