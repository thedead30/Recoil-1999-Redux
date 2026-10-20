"""port_batch.py - generate instruction-level ports for a batch of original functions (Stage 2 tooling).

Resolves every absolute reference a listing makes, or refuses the function:
  - calls to functions already ported (ledger remake_symbol) -> that symbol;
  - IAT slots (from Recoil.exe's import table) -> g_Iat_<name>_<slot> (must be declared in src/platform/*.h);
  - .rdata constants read as float/double -> generated CONFIRMED-BINARY constants from the exe bytes;
  - .data strings used as immediates -> generated CONFIRMED-DATA char arrays from the exe bytes;
  - anything else (globals, jump tables outside switches, code addresses) -> must be given in --globals, else the
    function is SKIPPED and reported.
usage: python tools/asm_port/port_batch.py LISTING ADDR[,ADDR...] --cpp OUT.cpp --header OUT.h --subsystem NAME
                                           [--globals map.json] [--file-comment TEXT] [--includes a.h,b.h]
Appends to OUT.cpp / OUT.h when they exist (functions already present are refused).
"""
import argparse, csv, glob, io, json, os, re, struct, subprocess, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
EXE = os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe')
CONVERTER = os.path.join(ROOT, 'tools', 'asm_port', 'ghidra2masm.py')


class ImageNoExe:
    """Cloud sessions have no Recoil.exe (03_re/CLOUD.md): the section table comes from the committed data image's
    constants (src/platform/image/original_data.h) and the import slots from their src/platform declarations. Fails
    closed: an import-table address with no declaration, a thunk or a switch table (both need .text bytes) leaves
    the function unresolved, so it is skipped and reported, never guessed."""
    def __init__(self):
        src = os.path.join(ROOT, '05_remake', 'src', 'platform', 'image')
        h = io.open(os.path.join(src, 'original_data.h'), encoding='utf-8').read()
        k = {n: int(v, 16) for n, v in re.findall(r'constexpr std::uint32_t (k\w+) = (0x[0-9a-f]+);', h)}
        self.b = None
        self.ranges = [('.rdata', k['kRDataVa'], k['kRDataSize']), ('.data', k['kDataVa'], k['kDataSize'])]
        self.iat = {}
        for hp in glob.glob(os.path.join(ROOT, '05_remake', 'src', 'platform', '*.h')):
            for nm, va in re.findall(r'\bg_Iat_(\w+)_(004cc[0-9a-f]{3})\b', io.open(hp, encoding='utf-8').read()):
                self.iat[int(va, 16)] = nm
        # the mirror zeroes the import table: its leading zero run bounds where undeclared slots may be
        cpp = io.open(os.path.join(src, 'original_data.cpp'), encoding='utf-8').read()
        i = cpp.index('g_RData_004cc000[kRDataAlloc] = {') + len('g_RData_004cc000[kRDataAlloc] = {')
        m = re.search(r'[1-9]', cpp[i:])
        self.iat_hi = k['kRDataVa'] + cpp[i:i + m.start()].count(',')

    def section(self, va):
        for name, lo, n in self.ranges:
            if lo <= va < lo + n:
                return name, None
        return None, None


class Image:
    def __init__(self):
        self.b = open(EXE, 'rb').read()
        b = self.b
        pe = struct.unpack_from('<I', b, 0x3c)[0]
        nsec = struct.unpack_from('<H', b, pe + 6)[0]
        opt = struct.unpack_from('<H', b, pe + 20)[0]
        self.secs = [(b[pe + 24 + opt + i * 40:pe + 24 + opt + i * 40 + 8].rstrip(b'\0').decode(),) +
                     struct.unpack_from('<IIII', b, pe + 24 + opt + i * 40 + 8) for i in range(nsec)]
        self.iat = {}
        d = self.foff_rva(struct.unpack_from('<I', b, pe + 24 + 104)[0])
        while True:
            oft, ts, fc, name, ft = struct.unpack_from('<IIIII', b, d)
            if not name:
                break
            t = self.foff_rva(oft or ft)
            dll = b[self.foff_rva(name):b.index(b'\0', self.foff_rva(name))].decode().split('.')[0].upper()
            k = 0
            while True:
                v = struct.unpack_from('<I', b, t + 4 * k)[0]
                if not v:
                    break
                if not v & 0x80000000:
                    o = self.foff_rva(v) + 2
                    self.iat[0x400000 + ft + 4 * k] = b[o:b.index(b'\0', o)].decode()
                else:  # by ordinal (MFC42): <DLL>_<ordinal>
                    self.iat[0x400000 + ft + 4 * k] = '%s_%d' % (dll, v & 0xffff)
                k += 1
            d += 20

    def foff_rva(self, rva):
        for name, vs, v, rs, ro in self.secs:
            if v <= rva < v + rs:
                return rva - v + ro

    def section(self, va):
        rva = va - 0x400000
        for name, vs, v, rs, ro in self.secs:
            if v <= rva < v + max(vs, rs):
                return name, (rva - v + ro) if rva - v < rs else None
        return None, None


def declared_iat_slots():
    names = set()
    for h in glob.glob(os.path.join(ROOT, '05_remake', 'src', 'platform', '*.h')):
        names.update(re.findall(r'\b(g_Iat_\w+_004cc[0-9a-f]{3})\b', io.open(h, encoding='utf-8').read()))
    return names


def c_string(s):
    return ''.join('\\\\' if c == 0x5c else '\\"' if c == 0x22 else '\\n' if c == 0x0a else chr(c) if 32 <= c < 127
                   else '\\x%02x' % c for c in s)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('listing')
    ap.add_argument('addrs')
    ap.add_argument('--cpp', required=True)
    ap.add_argument('--header', required=True)
    ap.add_argument('--subsystem', required=True)
    ap.add_argument('--globals')
    ap.add_argument('--file-comment', default='')
    ap.add_argument('--includes', default='')
    ap.add_argument('--pending', help='JSON {addr: name} of functions ported in the same run (tools/port_closure.py): '
                    'calls to them resolve to that name; the run builds everything together')
    a = ap.parse_args()
    img = Image() if os.path.exists(EXE) else ImageNoExe()
    if img.b is None:
        print('no Recoil.exe: import slots from src/platform declarations; thunks and switch tables not followed')
    ledger = {r['address'].lower(): r for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))}
    ported = {int(k, 16): r['remake_symbol'].split('::')[-1] for k, r in ledger.items() if r.get('remake_symbol')}  # code is generated inside namespace recoil
    if a.pending:
        for k, v in json.load(open(a.pending, encoding='utf-8')).items():
            ported.setdefault(int(k, 16), v)
    slots = declared_iat_slots()
    gmap = {int(k, 16): v for k, v in json.load(open(a.globals)).items()} if a.globals else {}
    blocks, block_headers = [], {}
    for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'data_blocks.csv'), encoding='utf-8')):
        blocks.append((int(r['address'], 16), int(r['size']), r['symbol']))
        block_headers[r['symbol']] = r['header']
    # data the linker put in .text (library tables such as DirectInput's data formats): mapped to a platform symbol like a block
    tds = os.path.join(ROOT, '03_re', 'ledger', 'text_data_symbols.csv')
    for r in (csv.DictReader(open(tds, encoding='utf-8')) if os.path.exists(tds) else []):
        blocks.append((int(r['address'], 16), int(r['size'], 16), r['symbol']))
        block_headers[r['symbol']] = r['header']
    used_headers = set()
    listing = io.open(a.listing, encoding='utf-8').read().split('### ')
    addrs = [x.strip().lower().replace('0x', '').rjust(8, '0') for x in a.addrs.split(',') if x.strip()]
    batch_names = {int(x, 16): (ledger['0x' + x]['ghidra_name']) for x in addrs}
    consts, strings, code, decls, skipped = {}, {}, '', '', []
    for addr in addrs:
        body = [x for x in listing if x.startswith(addr)]
        if not body:
            skipped.append((addr, 'not in listing'))
            continue
        body = body[0]
        name = batch_names[int(addr, 16)]
        if name.startswith('FUN_'):
            skipped.append((addr, 'ledger has no name (FUN_)'))
            continue
        mp = dict(gmap)
        mp.update(ported)
        mp.update(batch_names)
        missing = []
        for va, nm in img.iat.items():
            if re.search(r'0x%08x\b' % va, body) or re.search(r'0x%x\b' % va, body):
                slot = 'g_Iat_%s_%08x' % (nm.replace('?', '_').replace('@', '_'), va)
                if slot not in slots:
                    missing.append('IAT %s (declare %s in src/platform)' % (nm, slot))
                mp[va] = slot
        # Thunks (EXCLUDED ledger rows): a call or tail jump into a compiler/linker thunk goes where the thunk goes -
        # JMP rel32 to a ported function: that function; JMP dword ptr [import slot]: through the port's slot.
        for m in re.finditer(r'\b(?:CALL|JMP|PUSH)\s+0x([0-9a-f]{6,8})\b', body):
            va = int(m.group(1), 16)
            if img.b is None or va in mp or img.section(va)[0] != '.text':
                continue
            # follow a chain of thunks (a thunk that jumps to another thunk, e.g. 0x0043f440 -> 0x004c5b70 -> MFC import), up to 4 deep
            chain, cur, resolved = [], va, None
            for _ in range(4):
                if cur in mp:
                    resolved = mp[cur]
                    break
                if img.section(cur)[0] != '.text':
                    break
                fo = img.section(cur)[1]
                op = img.b[fo:fo + 6]
                chain.append(cur)
                if op[:1] == b'\xe9':
                    cur = cur + 5 + struct.unpack_from('<i', op, 1)[0]
                elif op[:2] == b'\xff\x25':
                    sv = struct.unpack_from('<I', op, 2)[0]
                    slot = 'g_Iat_%s_%08x' % (img.iat.get(sv, '?').replace('?', '_').replace('@', '_'), sv)
                    if slot in slots:
                        resolved = 'dword ptr [%s]' % slot
                    else:
                        missing.append('thunk 0x%08x -> import slot %s (declare it in src/platform)' % (cur, slot))
                    break
                else:
                    break
            if resolved is not None:
                for c in chain:
                    mp[c] = resolved
        # Data addresses (decision D7): a named block (03_re/ledger/data_blocks.csv) or the data image mirror
        # (src/platform/image/original_data.h) - the port's copy of the original .rdata/.data/.bss.
        for m in re.finditer(r'\b0x([0-9a-f]{6,8})\b', body):
            va = int(m.group(1), 16)
            if va in mp:
                continue
            if img.b is None and 0x004cc000 <= va < img.iat_hi:
                missing.append('0x%08x may be an import slot (no Recoil.exe to tell; declare it in src/platform)' % va)
                continue
            sec = img.section(va)[0]
            blk = next(((ba, sym) for ba, n, sym in blocks if ba <= va < ba + n), None)
            if blk:
                mp[va] = blk[1] + (' + 0x%x' % (va - blk[0]) if va != blk[0] else '')
                used_headers.add(block_headers[blk[1]])
            elif sec == '.rdata':
                mp[va] = 'g_RData_004cc000 + 0x%x' % (va - 0x004cc000)
                used_headers.add('platform/image/original_data.h')
            elif sec == '.data':
                mp[va] = 'g_Data_004da000 + 0x%x' % (va - 0x004da000)
                used_headers.add('platform/image/original_data.h')
        stack = max([int(x, 16) for x in re.findall(r'RET 0x([0-9a-f]+)', body)] or [0])
        out = os.path.join(os.environ.get('TEMP', '.'), 'pb_%s.asm' % addr)
        mapfile = out + '.map'
        with open(mapfile, 'w', encoding='utf-8') as mf:
            mf.write(','.join('0x%08x=%s' % kv for kv in mp.items()))
        r = subprocess.run([sys.executable, CONVERTER, a.listing, addr, '--map', '@' + mapfile,
                            '--out', out], capture_output=True, text=True)
        if r.returncode:
            unmapped = re.findall(r'^\s+\S+ (.*)$', r.stderr, re.M) or r.stderr.strip().splitlines()[-1:]
            skipped.append((addr, 'unresolved: ' + '; '.join(unmapped[:4]) + (' ...' if len(unmapped) > 4 else '')))
            continue
        if missing:
            skipped.append((addr, '; '.join(missing)))
            continue
        params = ['int ecx', 'int edx'] + ['int arg%d' % (i + 1) for i in range(stack // 4)]
        sig = 'int __fastcall %s(%s)' % (name, ', '.join(params))
        doc = (ledger['0x' + addr]['spec_ref'] or ledger['0x' + addr]['notes']).replace('\n', ' ')[:300]
        code += ('\n// 0x%s %s - %s\n// Register/stack shape from the listing (ECX, EDX, %d stack bytes).\n' % (addr, name, doc, stack))
        code += '__declspec(naked) ' + re.sub(r' (ecx|edx|arg\d+)(?=[,)])', '', sig) + '\n{\n    __asm {\n' + io.open(out, encoding='utf-8').read() + '    }\n}\n'
        decls += '// 0x%s\n%s;\n' % (addr, sig)
    if not code:
        print('nothing generated')
    else:
        exists = os.path.exists(a.cpp)
        t = io.open(a.cpp, encoding='utf-8').read() if exists else (
            '// SUBSYSTEM: %s\n%s\n// Instruction-level ports generated by tools/asm_port/port_batch.py from the Ghidra listings.\n'
            % (a.subsystem, a.file_comment if a.file_comment.startswith('//') else '// ' + a.file_comment) + '#include "%s"\n' % os.path.relpath(a.header, os.path.join(ROOT, '05_remake', 'src')).replace('\\', '/')
            + ''.join('#include "%s"\n' % inc for inc in a.includes.split(',') if inc) + '\nnamespace recoil {\n\n}  // namespace recoil\n')
        defs = ''
        for va in sorted(consts):
            nm, kind = consts[va]
            if re.search(r'\b%s =' % nm, t):
                continue
            _, off = img.section(va)
            if kind == 'F':
                bits = struct.unpack_from('<I', img.b, off)[0]
                defs += 'const float %s = %r;  // CONFIRMED-BINARY: float 0x%08x at 0x%08x\n' % (nm, struct.unpack('<f', struct.pack('<I', bits))[0], bits, va)
            else:
                bits = struct.unpack_from('<Q', img.b, off)[0]
                defs += 'const double %s = %r;  // CONFIRMED-BINARY: double 0x%016x at 0x%08x\n' % (nm, struct.unpack('<d', struct.pack('<Q', bits))[0], bits, va)
        for va in sorted(strings):
            nm, s = strings[va]
            if not re.search(r'\b%s\[\]' % nm, t):
                defs += 'const char %s[] = "%s";  // CONFIRMED-DATA: .data string at 0x%08x\n' % (nm, c_string(s), va)
        if defs:
            code = '\nnamespace {\n' + defs + '}  // namespace\n' + code
        for addr in addrs:
            if ('// 0x%s ' % addr) in t:
                sys.exit('already ported: 0x%s' % addr)
        # headers declaring every external symbol the new code names (ported callees, named blocks, the image)
        own_header = os.path.relpath(a.header, os.path.join(ROOT, '05_remake', 'src')).replace('\\', '/')
        symbols = set(re.findall(r'\b(?:call|jmp) (\w+)', code)) | set(re.findall(r'\boffset (\w+)', code)) | set(re.findall(r'\[(g_\w+)', code))
        symbols -= {s for s in symbols if re.fullmatch(r'e?[abcd]x|e?[sd]i|e?[sb]p', s)}
        decl = {}
        for hp in sorted(glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.h'), recursive=True)):
            rel = os.path.relpath(hp, os.path.join(ROOT, '05_remake', 'src')).replace('\\', '/')
            txt = io.open(hp, encoding='utf-8').read()
            for s in symbols:
                if s not in decl and (re.search(r'\bextern\b[^;\n]*\b%s\b' % re.escape(s), txt) or re.search(r'\b%s\s*\(' % re.escape(s), txt)):
                    decl[s] = rel
        for inc in sorted(set(decl.values()) | used_headers):
            if inc != own_header and ('#include "%s"' % inc) not in t:
                t = t.replace('\n\nnamespace recoil {', '\n#include "%s"\n\nnamespace recoil {' % inc, 1)
        t = t.replace('\n}  // namespace recoil', code + '\n}  // namespace recoil')
        os.makedirs(os.path.dirname(os.path.abspath(a.cpp)), exist_ok=True)
        io.open(a.cpp, 'w', encoding='utf-8', newline='').write(t)
        h = io.open(a.header, encoding='utf-8').read() if os.path.exists(a.header) else (
            '// SUBSYSTEM: %s\n// Declarations for %s.\n#pragma once\n\nnamespace recoil {\n\n}  // namespace recoil\n'
            % (a.subsystem, os.path.relpath(a.cpp, os.path.join(ROOT, '05_remake')).replace('\\', '/')))
        h = h.replace('\n}  // namespace recoil', '\n' + decls + '}  // namespace recoil')
        io.open(a.header, 'w', encoding='utf-8', newline='').write(h)
        print('generated %d function(s) into %s' % (decls.count(';'), a.cpp))
        # record them as translated (the tests upgrade the tag once they pass): the data image and later batches
        # then know the symbol
        rel_cpp = os.path.relpath(os.path.abspath(a.cpp), os.path.join(ROOT, '05_remake')).replace('\\', '/')
        args = []
        for addr in addrs:
            m = re.search(r'// 0x%s (\w+) ' % addr, code)
            if m:
                args += ['0x' + addr, rel_cpp, m.group(1), 'IMPLEMENTED-UNVERIFIED (instruction-level port by tools/asm_port/port_batch.py; not yet compared)']
        for i in range(0, len(args), 400):  # 100 functions per call: a whole-file batch overflows the Windows command line
            subprocess.run([sys.executable, os.path.join(ROOT, '03_re', 'scripts', 'remake_set.py')] + args[i:i + 400], check=True)
    for addr, why in skipped:
        print('SKIPPED 0x%s: %s' % (addr, why))


if __name__ == '__main__':
    main()
