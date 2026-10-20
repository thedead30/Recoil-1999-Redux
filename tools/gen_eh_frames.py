"""gen_eh_frames.py - the compiler-generated C++ exception frames of unported functions (Stage 2 porting tool).

An MSVC 5 function with C++ unwinding pushes the address of a per-function handler stub (PUSH 0x4c9cf0; MOV EAX,FS:[0] ...).
The stub is 10 bytes, MOV EAX,<FuncInfo>; JMP <thunk to __CxxFrameHandler>, and FuncInfo (.rdata) points at an unwind map
whose actions are small funclets (destructor calls on the parent's EBP frame). None of these were Ghidra functions or ledger
rows, so port_ready counted every such push as an unresolved blocker (128 handlers / 504 funclets over ~240 functions,
2026-09-30). This tool does for them what was done by hand for EH_Handler_Zar_OpenAndRegisterArchive (zreader.cpp):

  collect            -> 03_re/staging/eh/eh_frames.json (decoded from the exe bytes) + eh_funclets_in.txt ("<addr> <name>"
                        lines for ~/ghidra_scripts/CreateDumpFunctions.java, which creates the funclets in Ghidra and dumps them)
  register DUMP.txt  -> ledger rows (ENGINE, CONFIRMED, bytes cited) for handlers and funclets, subsystems.csv member rows,
                        funclet listing chunks appended to 03_re/listings/unported/<subsystem>.txt (port_batch ports them
                        like any function); funclets and handler get the parent's orig_file so they land in its source file
  emit               -> for every registered handler whose funclets all have a remake symbol: the FuncInfo / unwind map as C++
                        data pointing at the PORT's funclets, and the naked stub (MOV EAX,offset table; JMP [__CxxFrameHandler
                        slot]) into the parent's source file, declaration into its header, remake_set IMPLEMENTED-UNVERIFIED.
                        The parent then resolves the handler through the ledger like any ported callee.

Handlers with try blocks (catch funclets return continuation addresses inside the parent) are listed, never generated.
The handler's own listing is never put in a port listing: an instruction-level port would hand __CxxFrameHandler the
ORIGINAL table, whose funclet pointers do not exist in the remake.
usage: python tools/gen_eh_frames.py collect | register DUMP.txt | emit
"""
import collections, csv, glob, io, json, os, re, struct, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
EXE = os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe')
LED = os.path.join(ROOT, '03_re', 'ledger', 'functions.csv')
SUB = os.path.join(ROOT, '03_re', 'ledger', 'subsystems.csv')
ST = os.path.join(ROOT, '03_re', 'staging', 'eh')
JS = os.path.join(ST, 'eh_frames.json')
FRAME_THUNK = 0x004c60a0      # JMP dword ptr [0x004cc5b0] (__CxxFrameHandler), CONFIRMED-BINARY: bytes ff 25 b0 c5 4c 00
MAGIC = 0x19930520            # CONFIRMED-BINARY: FuncInfo magic of MSVC 5 (every table read here carries it; others refused)
SIBLING = '0x004cb10b'        # the first hand-made handler row: its verify plan is copied

b = open(EXE, 'rb').read()
_pe = struct.unpack_from('<I', b, 0x3c)[0]
_so = _pe + 24 + struct.unpack_from('<H', b, _pe + 20)[0]
SECS = []
for _i in range(struct.unpack_from('<H', b, _pe + 6)[0]):
    _vs, _va, _rs, _ro = struct.unpack_from('<IIII', b, _so + 40 * _i + 8)
    SECS.append((_va + 0x400000, max(_vs, _rs), _ro, b[_so + 40 * _i:_so + 40 * _i + 8].rstrip(b'\0').decode()))


def off(va):
    for a, sz, ro, nm in SECS:
        if a <= va < a + sz:
            return ro + va - a
    raise ValueError('0x%08x outside the image' % va)


def u32(va):
    return struct.unpack_from('<I', b, off(va))[0]


def hexbytes(va, n):
    return ' '.join('%02x' % x for x in b[off(va):off(va) + n])


def ledger():
    return {r['address'].lower(): r for r in csv.DictReader(io.open(LED, encoding='utf-8'))}


def members():
    return {m['address'].lower(): m['subsystem'] for m in csv.DictReader(io.open(SUB, encoding='utf-8'))}


def decode(h):
    """handler 0x.. -> dict, or a string saying why it is not a plain C++ frame handler."""
    o = off(h)
    if b[o] != 0xb8 or b[o + 5] != 0xe9:
        return 'not MOV EAX,imm32; JMP rel32 (%s)' % hexbytes(h, 10)
    if h + 10 + struct.unpack_from('<i', b, o + 6)[0] != FRAME_THUNK:
        return 'jumps elsewhere than the __CxxFrameHandler thunk 0x%08x' % FRAME_THUNK
    fi = struct.unpack_from('<I', b, o + 1)[0]
    magic, nstate, umap, ntry, tmap = [u32(fi + 4 * i) for i in range(5)]
    if magic != MAGIC:
        return 'FuncInfo 0x%08x magic 0x%08x' % (fi, magic)
    unwind = [list(struct.unpack_from('<iI', b, off(umap + 8 * i))) for i in range(nstate)]
    return {'funcinfo': fi, 'nstate': nstate, 'umap': umap, 'ntry': ntry, 'tmap': tmap, 'unwind': unwind,
            'bytes': hexbytes(h, 10), 'fi_bytes': hexbytes(fi, 20)}


def own_name(led, a, default):
    """an existing ledger row keeps its name unless it is a Ghidra default (the catch funclets were named in Stage 1)"""
    r = led.get(a)
    return r['ghidra_name'] if r and not re.match(r'(Unwind@|FUN_|LAB_|Catch@|Catch_All@)', r['ghidra_name']) else default


def try_blocks(rec, d, base, led):
    """decode the try block map: each catch funclet (a handler type's address) runs on the parent's frame and returns, in EAX, the
    address where the parent continues (MOV EAX,<continuation>; RET - the MSVC 5 shape, found in the funclet's bytes). Both become
    functions of their own, like the unwind funclets, so the generated HandlerType table can point at port symbols."""
    tries, conts = [], {}
    for i in range(d['ntry']):
        lo, hi, ch, nc, ph = [u32(d['tmap'] + 20 * i + 4 * k) for k in range(5)]
        hts = []
        for j in range(nc):
            adj, ptype, disp, addr = [u32(ph + 16 * j + 4 * k) for k in range(4)]
            m = re.search(rb'\xb8(....)\xc3', b[off(addr):off(addr) + 0x400], re.S)
            if not m:
                return 'catch funclet 0x%08x: no MOV EAX,<continuation>; RET in its first 0x400 bytes' % addr
            k_ = struct.unpack('<I', m.group(1))[0]
            hts.append({'adj': adj, 'type': ptype, 'disp': disp, 'addr': addr, 'cont': k_})
            rec['funclets'].setdefault('0x%08x' % addr, own_name(led, '0x%08x' % addr, 'EH_Catch_%s_%d' % (base, len(tries) * 8 + j)))
            conts.setdefault(k_, len(conts))
        tries.append({'lo': lo, 'hi': hi, 'catch_high': ch, 'handlers_at': ph, 'handlers': hts})
    for k_, n in conts.items():
        rec['funclets'].setdefault('0x%08x' % k_, own_name(led, '0x%08x' % k_, 'EH_Cont_%s_%d' % (base, n)))
    rec['try'] = tries
    return None


def collect():
    led, mem = ledger(), members()
    frames = {}
    for p in sorted(glob.glob(os.path.join(ROOT, '03_re', 'listings', 'unported', '*.txt'))):
        for chunk in io.open(p, encoding='utf-8').read().split('### ')[1:]:
            parent = '0x' + chunk[:8]
            if parent not in led or led[parent]['remake_symbol']:
                continue
            for m in re.finditer(r'^([0-9a-f]+) PUSH 0x(4c[0-9a-f]{4})\b', chunk, re.M):
                h = int(m.group(2), 16)
                if u32(h - 0) is None or b[off(h)] != 0xb8:
                    continue
                d = decode(h)
                key = '0x%08x' % h
                if key in frames:
                    frames[key]['parents'].append(parent)
                    continue
                pn = led[parent]['ghidra_name']
                base = re.sub(r'_[0-9a-f]{8}$', '', pn)
                rec = {'parents': [parent], 'parent_name': pn, 'subsystem': mem.get(parent, os.path.basename(p)[:-4]),
                       'push_at': '0x%08x' % int(m.group(1), 16), 'handler_name': 'EH_Handler_%s' % base}
                if isinstance(d, str):
                    rec['refused'] = d
                else:
                    rec.update(d)
                    fun = []
                    for _, act in d['unwind']:
                        if act and act not in fun:
                            fun.append(act)
                    rec['funclets'] = {'0x%08x' % a: 'EH_Unwind_%s_%d' % (base, i) for i, a in enumerate(fun)}
                    if d['ntry']:
                        why = try_blocks(rec, d, base, led)
                        if why:
                            rec['refused'] = why
                frames[key] = rec
    os.makedirs(ST, exist_ok=True)
    json.dump(frames, io.open(JS, 'w', encoding='utf-8'), indent=1)
    inp = os.path.join(ST, 'eh_funclets_in.txt')
    with io.open(inp, 'w', encoding='utf-8', newline='\n') as f:
        for h, r in sorted(frames.items()):
            if 'refused' not in r:
                for a, n in sorted(r['funclets'].items()):
                    if a not in led or led[a]['class'] != 'ENGINE':
                        f.write('%s %s\n' % (a, n))
    ok = [r for r in frames.values() if 'refused' not in r]
    print('%d handlers (%d plain, %d refused), %d funclets -> %s, %s' % (
        len(frames), len(ok), len(frames) - len(ok), sum(len(r['funclets']) for r in ok), JS, inp))
    for h, r in sorted(frames.items()):
        if 'refused' in r:
            print('  refused %s (%s): %s' % (h, r['parent_name'], r['refused']))


def register(dump):
    frames = json.load(io.open(JS, encoding='utf-8'))
    chunks = {}
    for c in io.open(dump, encoding='utf-8', errors='replace').read().replace('\r\n', '\n').split('### ')[1:]:
        chunks['0x' + c[:8]] = c
    rows = list(csv.reader(io.open(LED, encoding='utf-8', newline='')))
    hdr = rows[0]
    col = {k: hdr.index(k) for k in ('verify_plan', 'verify_plan_reason', 'orig_file', 'orig_file_evidence', 'notes', 'spec_ref')}
    have = {r[0]: r for r in rows[1:] if r}
    sib = have[SIBLING]
    new, memrows, listing_add, converted = [], [], collections.defaultdict(str), []

    def row(a, name, parent, note):
        r = [''] * len(hdr)
        r[0], r[1], r[2], r[3] = a, name, 'ENGINE', 'CONFIRMED'
        pr = have[parent]
        r[col['spec_ref']] = pr[col['spec_ref']]
        r[col['notes']] = note
        r[col['verify_plan']], r[col['verify_plan_reason']] = sib[col['verify_plan']], sib[col['verify_plan_reason']]
        if pr[col['orig_file']]:
            r[col['orig_file']] = pr[col['orig_file']]
            r[col['orig_file_evidence']] = 'EH-FRAME (compiler-generated exception frame code of %s %s, which is %s)' % (
                parent, pr[1], pr[col['orig_file_evidence']] or 'attributed')
        return r
    for h, fr in sorted(frames.items()):
        if 'refused' in fr:
            continue
        missing = [a for a in fr['funclets'] if a not in chunks and (a not in have or have[a][2] != 'ENGINE')]
        if missing:
            print('  %s: funclets not in the dump: %s' % (h, ', '.join(missing)))
            continue
        parent, sub = fr['parents'][0], fr['subsystem']
        for i, (a, name) in enumerate(sorted(fr['funclets'].items())):
            if a in have and have[a][2] == 'ENGINE':
                continue
            c = chunks[a]
            lines = [l for l in c.split('\n')[1:] if l.strip()]
            end = int(lines[-1].split()[0], 16)
            states = [str(s) for s, (to, act) in enumerate(fr['unwind']) if '0x%08x' % act == a]
            catches = {'0x%08x' % ht['addr']: ht for t in fr.get('try', []) for ht in t['handlers']}
            conts = {'0x%08x' % ht['cont'] for t in fr.get('try', []) for ht in t['handlers']}
            if states:
                role = 'C++ unwind action for state(s) %s' % ','.join(states)
            elif a in catches:
                ht = catches[a]
                role = ('catch block (handler type: adjectives 0x%x, type descriptor 0x%08x, catch object at EBP%+d) that returns its '
                        'continuation 0x%08x in EAX' % (ht['adj'], ht['type'], struct.unpack('<i', struct.pack('<I', ht['disp']))[0], ht['cont']))
            else:
                role = 'catch continuation (the address a catch block returns: where the parent resumes after the catch)'
            # a continuation runs straight on into the next Ghidra function (the rest of the parent): make that fall-through explicit
            parts = lines[-1].split(' ', 2)
            if a in conts and (len(parts) < 2 or parts[1] not in ('RET', 'JMP')):
                nxt = min(int(x, 16) for x in have if x.startswith('0x') and int(x, 16) > end)
                c = c.rstrip('\n') + '\n%x JMP 0x%08x\n' % (end + 1, nxt)
                role += ('; its bytes run on into 0x%08x %s (the next Ghidra function) - the listing chunk ends with a JMP there, written by '
                         'the tool at %x (not an original instruction: the original falls through)' % (nxt, have['0x%08x' % nxt][1], end + 1))
            note = ('NEW ROW (EH funclet; tools/gen_eh_frames.py 2026-09-30). %d instructions from the Ghidra listing (function created by '
                    'CreateDumpFunctions.java), first bytes %s: %s of %s %s - FuncInfo %s (magic 0x%08x, '
                    'CONFIRMED-BINARY) unwind map 0x%08x, handler %s. Runs on the parent\'s EBP frame when an exception reaches it.'
                    % (len(lines), hexbytes(int(a, 16), min(12, end - int(a, 16) + 1)), role, parent, fr['parent_name'],
                       '0x%08x' % fr['funcinfo'], MAGIC, fr['umap'], h))
            if a in have:  # the Ghidra export's LIBRARY / EXCLUDED 'Unwind@' row becomes the engine row (as for 0x004cb100)
                note = note.replace('NEW ROW (EH funclet;', 'WAS %s/%s %s (EH funclet;' % (have[a][2], have[a][3], have[a][1]))
                r = row(a, name, parent, note)
                have[a][:] = r
                converted.append(a)
            else:
                new.append(row(a, name, parent, note))
            memrows.append((sub, a, name, 'EH funclet: unwind action of handler %s (FuncInfo 0x%08x), pushed by member %s %s at %s' % (
                h, fr['funcinfo'], parent, fr['parent_name'], fr['push_at'])))
            listing_add[sub] += '### %s %s\n' % (a[2:], name) + c.split('\n', 1)[1].rstrip('\n') + '\n\n'
        if h not in have:
            note = ('NEW ROW (EH handler; tools/gen_eh_frames.py 2026-09-30). 10 bytes, read (%s): MOV EAX,0x%08x; JMP 0x%08x (thunk -> '
                    '__CxxFrameHandler [0x004cc5b0]) - the compiler-generated C++ exception frame handler that %s %s pushes at %s. FuncInfo '
                    '0x%08x bytes (%s): magic 0x%08x, %d unwind states, unwind map 0x%08x %s, no try blocks. Port: generated table pointing '
                    'at the port\'s funclets + the same two-instruction stub (never an instruction-level port of the original table).'
                    % (fr['bytes'], fr['funcinfo'], FRAME_THUNK, parent, fr['parent_name'], fr['push_at'], fr['funcinfo'], fr['fi_bytes'],
                       MAGIC, fr['nstate'], fr['umap'], fr['unwind']))
            new.append(row(h, fr['handler_name'], parent, note))
            memrows.append((sub, h, fr['handler_name'], 'EH handler pushed by member %s %s (PUSH %s at %s)' % (
                parent, fr['parent_name'], h, fr['push_at'])))
    if not new and not converted:
        print('nothing new')
        return
    body = sorted([r for r in rows[1:] if r] + new, key=lambda r: int(r[0], 16) if r[0].startswith('0x') else -1)
    csv.writer(io.open(LED, 'w', encoding='utf-8', newline=''), lineterminator='\n').writerows([hdr] + body)
    mem = open(SUB, encoding='utf-8', newline='').read()
    nl = '\r\n' if '\r\n' in mem else '\n'
    open(SUB, 'a', encoding='utf-8', newline='').write(('' if mem.endswith(('\n', '\r\n')) else nl) + ''.join(
        '%s,%s,%s,"%s"%s' % (s, a, n, why.replace('"', "'"), nl) for s, a, n, why in memrows))
    for sub, text in listing_add.items():
        lp = os.path.join(ROOT, '03_re', 'listings', 'unported', sub + '.txt')
        raw = open(lp, encoding='utf-8', newline='').read() if os.path.exists(lp) else ''
        lnl = '\r\n' if '\r\n' in raw else '\n'
        open(lp, 'w', encoding='utf-8', newline='').write(raw + ('' if not raw or raw.endswith(lnl) else lnl) + text.replace('\n', lnl))
    print('converted %d Unwind@ rows; registered %d new rows (%d handlers, %d funclets)' % (len(converted), len(new), sum(1 for r in new if r[1].startswith('EH_Handler_')),
                                                             sum(1 for r in new if r[1].startswith('EH_Unwind_'))))


def target(row, subsys):
    o = row.get('orig_file', '').replace('\\', '/').strip()
    rel = re.sub(r'\.(c|cpp)$', '', o) if o else 'unattributed/' + subsys
    return os.path.join(ROOT, '05_remake', 'src', rel + '.cpp'), os.path.join(ROOT, '05_remake', 'src', rel + '.h')


def emit():
    frames = json.load(io.open(JS, encoding='utf-8'))
    led = ledger()
    args, done = [], 0
    for h, fr in sorted(frames.items()):
        if 'refused' in fr or h not in led or led[h]['remake_symbol']:
            continue
        syms = {a: led.get(a, {}).get('remake_symbol', '') for a in fr['funclets']}
        if not all(syms.values()):
            continue
        cpp, hdr = target(led[h], fr['subsystem'])
        funclet_hdrs = set()
        for a in fr['funclets']:
            fc = led[a]['remake_file'].replace('05_remake/src/', '').replace('src/', '')
            funclet_hdrs.add(re.sub(r'\.cpp$', '.h', fc))
        name, fi, um = fr['handler_name'], fr['funcinfo'], fr['umap']
        ent = ', '.join('{%d, %s}' % (to, 'reinterpret_cast<void*>(&%s)' % syms['0x%08x' % act].split('::')[-1] if act else 'nullptr')
                        for to, act in fr['unwind'])
        tries = fr.get('try', [])
        trycode, tryref, trydesc = '', 'nullptr', 'no try blocks'
        if tries:
            # catch blocks and their continuations are ported functions of their own (register): the handler types point at them; a
            # type descriptor is the mirrored .data copy (__CxxFrameHandler matches a thrown type to it by its decorated name)
            for t in tries:
                hts = []
                for ht in t['handlers']:
                    ty = 'nullptr'
                    if ht['type']:
                        if not (0x004da000 <= ht['type'] < 0x007c9000):
                            raise SystemExit('%s: type descriptor 0x%08x outside .data' % (h, ht['type']))
                        ty = 'g_Data_004da000 + 0x%x' % (ht['type'] - 0x004da000)
                    hts.append('{0x%xu, %s, %d, reinterpret_cast<void*>(&%s)}' % (
                        ht['adj'], ty, struct.unpack('<i', struct.pack('<I', ht['disp']))[0], syms['0x%08x' % ht['addr']].split('::')[-1]))
                trycode += 'const EhHandlerType g_EhHandlerTypes_%08x[%d] = {%s};\n' % (t['handlers_at'], len(hts), ', '.join(hts))
            trycode += 'const EhTryBlockMapEntry g_EhTryMap_%08x[%d] = {%s};\n' % (fr['tmap'], len(tries), ', '.join(
                '{%d, %d, %d, %d, const_cast<EhHandlerType*>(g_EhHandlerTypes_%08x)}' % (t['lo'], t['hi'], t['catch_high'], len(t['handlers']), t['handlers_at'])
                for t in tries))
            tryref = 'g_EhTryMap_%08x' % fr['tmap']
            trydesc = '%d try block(s) at 0x%08x %s' % (len(tries), fr['tmap'], [(t['lo'], t['hi'], t['catch_high'], [
                ('0x%08x' % x['type'], '0x%08x' % x['addr']) for x in t['handlers']]) for t in tries])
        code = ('\n// %s %s - %s\n// Generated by tools/gen_eh_frames.py from the exe bytes (%s): MOV EAX,FuncInfo 0x%08x; JMP 0x%08x (-> __CxxFrameHandler).\n'
                '// FuncInfo CONFIRMED-BINARY: magic 0x%08x, %d unwind states, unwind map 0x%08x (to-state, action) = %s, %s;\n'
                '// the port\'s table points at the port\'s funclets. Needs /SAFESEH:NO (CMakeLists.txt).\n'
                'namespace {\nconst EhUnwindMapEntry g_EhUnwindMap_%08x[%d] = {%s};\n%s'
                'const EhFuncInfo g_EhFuncInfo_%08x = {0x%08xu, %d, g_EhUnwindMap_%08x, %d, %s};\n}  // namespace\n\n'
                '__declspec(naked) int __fastcall %s(int, int)\n{\n    __asm {\n        mov eax, offset g_EhFuncInfo_%08x\n'
                '        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]\n    }\n}\n'
                % (h, name, led[h]['spec_ref'], fr['bytes'], fi, FRAME_THUNK, MAGIC, fr['nstate'], um,
                   ['(%d, 0x%08x)' % (to, act) for to, act in fr['unwind']], trydesc, um, fr['nstate'], ent, trycode, fi, MAGIC, fr['nstate'], um,
                   len(tries), tryref, name, fi))
        if tries:
            funclet_hdrs.add('platform/image/original_data.h')
        rel_h = os.path.relpath(hdr, os.path.join(ROOT, '05_remake', 'src')).replace('\\', '/')
        if os.path.exists(cpp):
            t = io.open(cpp, encoding='utf-8').read()
        else:
            t = ('// SUBSYSTEM: %s\n// Instruction-level ports generated by tools/asm_port/port_batch.py from the Ghidra listings.\n#include "%s"\n'
                 '\nnamespace recoil {\n\n}  // namespace recoil\n' % (fr['subsystem'], rel_h))
        if '// %s ' % h in t:
            continue
        for inc in sorted(funclet_hdrs | {'platform/msvc_eh.h', 'platform/iat_msvcrt.h'}):
            if inc != rel_h and '#include "%s"' % inc not in t:
                t = t.replace('\n\nnamespace recoil {', '\n#include "%s"\n\nnamespace recoil {' % inc, 1)
        i = t.rindex('\n}  // namespace recoil')
        t = t[:i] + code + t[i:]
        io.open(cpp, 'w', encoding='utf-8', newline='').write(t)
        ht = io.open(hdr, encoding='utf-8').read() if os.path.exists(hdr) else (
            '// SUBSYSTEM: %s\n// Declarations for %s.\n#pragma once\n\nnamespace recoil {\n\n}  // namespace recoil\n'
            % (fr['subsystem'], os.path.relpath(cpp, os.path.join(ROOT, '05_remake')).replace('\\', '/')))
        j = ht.rindex('\n}  // namespace recoil')
        ht = ht[:j] + '\n// %s\nint __fastcall %s(int ecx, int edx);\n' % (h, name) + ht[j:]
        io.open(hdr, 'w', encoding='utf-8', newline='').write(ht)
        args += [h, os.path.relpath(cpp, os.path.join(ROOT, '05_remake')).replace('\\', '/'), name,
                 'IMPLEMENTED-UNVERIFIED (EH handler generated by tools/gen_eh_frames.py from the exe bytes; not yet compared)']
        done += 1
    for i in range(0, len(args), 400):
        subprocess.run([sys.executable, os.path.join(ROOT, '03_re', 'scripts', 'remake_set.py')] + args[i:i + 400], check=True)
    print('emitted %d handler(s)' % done)


if __name__ == '__main__':
    cmd = sys.argv[1] if len(sys.argv) > 1 else ''
    if cmd == 'collect':
        collect()
    elif cmd == 'register':
        register(sys.argv[2])
    elif cmd == 'emit':
        emit()
    else:
        sys.exit(__doc__)
