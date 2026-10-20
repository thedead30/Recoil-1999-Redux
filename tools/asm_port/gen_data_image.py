"""gen_data_image.py - generate the port's mirror of the original's data (Stage 2 decision D7, STAGE2.md section 5).

Writes 05_remake/src/platform/image/original_data.h/.cpp:
  g_RData_004cc000  - the original .rdata (import slots zeroed: code reaches them through src/platform/*.h)
  g_Data_004da000   - the original .data + .bss (file bytes, then zero)
  a relocation table for every initialised word whose value is an address inside the original image:
    -> into a named data block (03_re/ledger/data_blocks.csv): that symbol + offset
    -> into .rdata / .data: the mirror + offset
    -> into .text at a ported function's entry (ledger remake_symbol): that function
    -> anything else in .text / .rsrc: ImageData_Unported (a trap that stops the program loudly)
  applied once, at static-initialisation time, before any game code runs.
Words inside named data blocks are skipped (those blocks have their own definitions).
Named blocks are also cut out of the mirror's meaning: the generator in port_batch.py maps their addresses to
the named symbols, never to the mirror.
Without Recoil.exe (cloud sessions) only the parts the ledger's ported set decides are refreshed: see
refresh_without_exe().
usage: python tools/asm_port/gen_data_image.py [--check] [--compiled]   (default: the runtime image; --compiled builds the bytes in, local use only)
"""
import csv, glob, io, os, re, struct, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
EXE = os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe')
OUT = os.path.join(ROOT, '05_remake', 'src', 'platform', 'image')
BASE = 0x400000
# Build-time use (public build): the data image is generated from the player's own copy of the game, never shipped.
#   --source PATH   any form tools/recoil_source.py reads (installed folder, zip, CD folder, .iso/.bin/.cue/.nrg)
#   --exe PATH      a Recoil.exe file
#   --out DIR       where original_data.cpp/.h are written (default 05_remake/src/platform/image)
# Only the supported build is accepted (tools/recoil_source.py KNOWN_EXE).
EXE_BYTES = None
if '--out' in sys.argv: OUT = os.path.abspath(sys.argv[sys.argv.index('--out') + 1])
if '--exe' in sys.argv: EXE = os.path.abspath(sys.argv[sys.argv.index('--exe') + 1])
if '--source' in sys.argv:
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    import recoil_source
    _src = recoil_source.Source(sys.argv[sys.argv.index('--source') + 1])
    if not _src.game: sys.exit('no Recoil game files found in that source')
    EXE_BYTES = _src.game.read('Recoil.exe')


def _exe_bytes():
    sys.path.insert(0, os.path.join(ROOT, 'tools'))
    import recoil_source
    b = EXE_BYTES if EXE_BYTES is not None else open(EXE, 'rb').read()
    h, name, ok = recoil_source.exe_version(b)
    if not ok:
        sys.exit('Recoil.exe is the %s (sha256 %s): not supported - %s' % (name, h, recoil_source.SUPPORTED_NOTE))
    return b


def refresh_without_exe():
    """Cloud sessions have no Recoil.exe (03_re/CLOUD.md). Only the ledger's ported set can have changed since the
    committed image was generated from the exe, and it decides nothing but the relocation kinds (unported <->
    function), the #include list and kFunctions - so those are regenerated in the committed file, exactly as main()
    would write them; everything read from the exe (bytes, sections, other relocations) is kept verbatim. Refuses when
    the named blocks differ from the committed ones (that needs the exe)."""
    target = os.path.join(OUT, 'original_data.cpp')
    t = io.open(target, encoding='utf-8').read()
    blocks = [(int(r['address'], 16), int(r['size']), r['symbol'], r['header'])
              for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'data_blocks.csv'), encoding='utf-8'))]
    blockrefs = '\n'.join('    {0x%08x, 0x%x, %s},' % (a, n, '(void*)&' + sym) for a, n, sym, h in blocks)
    if '\nconst Block kBlocks[] = {\n%s\n};\n' % blockrefs not in t:
        sys.exit('gen_data_image: data_blocks.csv changed since the image was generated - run it where Recoil.exe is')
    ledger = [r for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))]
    ported = {int(r['address'], 16): r['remake_symbol'] for r in ledger if r.get('remake_symbol')}
    decl_headers = {}
    for h in glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.h'), recursive=True):
        ht = io.open(h, encoding='utf-8').read()
        rel = os.path.relpath(h, os.path.join(ROOT, '05_remake', 'src')).replace('\\', '/')
        for sym in set(ported.values()):
            if re.search(r'\b%s\s*\(' % re.escape(sym), ht):
                decl_headers.setdefault(sym, rel)
    fn_all = sorted((w, sym) for w, sym in ported.items() if sym in decl_headers)
    live = {w for w, sym in fn_all}
    changed = [0]

    def rekind(m):
        va, w, k = int(m.group(1), 16), int(m.group(2), 16), int(m.group(3))
        nk = 2 if k in (2, 3) and w in live else 3 if k == 2 else k
        changed[0] += nk != k
        return '    {0x%08x, 0x%08x, %d},' % (va, w, nk)
    i = t.index('const ImageDataRelocation g_ImageDataRelocations[] = {\n')
    j = t.index('\n};\n', i)
    t = t[:i] + re.sub(r'^    \{0x([0-9a-f]{8}), 0x([0-9a-f]{8}), (\d)\},$', rekind, t[i:j], flags=re.M) + t[j:]
    incs = '\n'.join('#include "%s"' % h for h in sorted({h for a, n, s, h in blocks} | {decl_headers[sym] for w, sym in fn_all}))
    t = re.sub(r'(#include <cstring>\n\n)[^\n]*(?:\n#include "[^"\n]+")*(\n\nnamespace recoil \{)', lambda m: m.group(1) + incs + m.group(2), t, count=1)
    fnrows = '\n'.join('    {0x%08x, reinterpret_cast<void*>(&%s)},' % (a, sym) for a, sym in fn_all)
    t = re.sub(r'(const Fn kFunctions\[\] = \{\n)(?:    \{0x[0-9a-f]{8}, [^\n]*\n)*(    \{0, nullptr\},)', lambda m: m.group(1) + fnrows + '\n' + m.group(2), t, count=1)
    if '--check' in sys.argv:
        if io.open(target, encoding='utf-8').read() != t:
            sys.exit('data image is stale: run python tools/asm_port/gen_data_image.py')
        print('data image up to date (no Recoil.exe: ported-set parts only)')
        return
    io.open(target, 'w', encoding='utf-8', newline='').write(t)
    print('no Recoil.exe: refreshed the ported-set parts of the committed image (%d relocation kinds changed, %d functions)'
          % (changed[0], len(fn_all)))


COMPILED_API = """
bool ImageData_IsRuntime() { return false; }
bool ImageData_LoadFromExe(const char*, char*, unsigned) { return true; }   // compiled-in image: nothing to load
"""


def compiled_variant(cpp):
    i = cpp.index('bool ImageData_Block(')
    return cpp[:i] + COMPILED_API.lstrip('\n') + '\n' + cpp[i:]


def runtime_variant(cpp, b, secs, iat_rva, iat_size, rsrc_offset, image_bytes):
    """the same image, with every byte of Recoil.exe replaced by a loader that reads the player's copy at start-up"""
    import hashlib, re as _re
    sha = hashlib.sha256(b).digest()
    img_sha = hashlib.sha256(image_bytes).digest()   # the exact bytes the compiled-in variant contains
    vs_r, v_r, rs_r, ro_r = secs['.rdata']; vs_d, v_d, rs_d, ro_d = secs['.data']; vs_s, v_s, rs_s, ro_s = secs['.rsrc']
    note = r"\1;   // filled from the player's Recoil.exe at start-up (ImageData_LoadFromExe)"
    cpp = _re.sub(r'(alignas\(4096\) unsigned char g_RData_004cc000\[kRDataAlloc\]) = \{\n.*?\n\};', note, cpp, count=1, flags=_re.S)
    cpp = _re.sub(r'(alignas\(4096\) unsigned char g_Data_004da000\[kDataAlloc\]) = \{\n.*?\n\};', note, cpp, count=1, flags=_re.S)
    cpp = _re.sub(r'// \.rsrc bytes \(CONFIRMED-DATA, the file\), copied into the mirror\'s tail at start-up\nconst unsigned char kRsrcBytes\[\] = \{\n.*?\n\};\n',
                  '', cpp, count=1, flags=_re.S)
    cpp = cpp.replace('    std::memcpy(g_Data_004da000 + kRsrcOffset, kRsrcBytes, sizeof kRsrcBytes);\n', '')
    cpp = cpp.replace('const bool g_relocated = relocate();\n', '')
    consts = (
        '// Where the mirrors come from in the player\'s Recoil.exe (CONFIRMED-DATA: its section table) - no game bytes here.\n'
        'constexpr std::uint32_t kExeSize = %d;              // CONFIRMED-DATA: file size of the supported build\n'
        'const unsigned char kExeSha256[32] = {%s};   // CONFIRMED-DATA: SHA-256 of the supported build\n'
        'const unsigned char kImageSha256[32] = {%s};   // SHA-256 of the image bytes the compiled-in variant holds (equality check)\n'
        'constexpr std::uint32_t kRDataFile = 0x%x, kRDataCopy = 0x%x;   // CONFIRMED-DATA: .rdata file offset / bytes copied\n'
        'constexpr std::uint32_t kIatOffset = 0x%x, kIatBytes = 0x%x;    // CONFIRMED-DATA: import slots inside .rdata (zeroed)\n'
        'constexpr std::uint32_t kDataFile = 0x%x, kDataCopy = 0x%x;     // CONFIRMED-DATA: .data file offset / raw size\n'
        'constexpr std::uint32_t kRsrcFile = 0x%x, kRsrcCopy = 0x%x;     // CONFIRMED-DATA: .rsrc file offset / raw size\n'
        % (len(b), ','.join('0x%02x' % x for x in sha), ','.join('0x%02x' % x for x in img_sha), ro_r, min(rs_r, vs_r), iat_rva - v_r, iat_size, ro_d, rs_d, ro_s, rs_s))
    i = cpp.index('constexpr std::uint32_t kRsrcOffset')
    cpp = cpp[:i] + consts + cpp[i:]
    api = r"""
bool ImageData_IsRuntime() { return true; }

bool ImageData_LoadFromExe(const char* path, char* why, unsigned why_size)
{
    auto fail = [&](const char* m) { if (why && why_size) std::snprintf(why, why_size, "%s", m); return false; };
    HANDLE f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return fail("cannot open the original Recoil.exe");
    std::vector<unsigned char> b(kExeSize + 1);
    DWORD got = 0;
    ReadFile(f, b.data(), kExeSize + 1, &got, nullptr);
    CloseHandle(f);
    if (got != kExeSize) return fail("Recoil.exe is not the supported 1999-01-29 build (wrong size)");
    unsigned char h[32] = {};
    BCRYPT_ALG_HANDLE alg = nullptr;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) return fail("SHA-256 unavailable");
    const NTSTATUS st = BCryptHash(alg, nullptr, 0, b.data(), kExeSize, h, sizeof h);
    BCryptCloseAlgorithmProvider(alg, 0);
    if (st != 0 || std::memcmp(h, kExeSha256, sizeof h) != 0) return fail("Recoil.exe is not the supported 1999-01-29 build (fingerprint differs)");
    std::memcpy(g_RData_004cc000, b.data() + kRDataFile, kRDataCopy);
    std::memset(g_RData_004cc000 + kIatOffset, 0, kIatBytes);   // import slots: reached through src/platform
    std::memcpy(g_Data_004da000, b.data() + kDataFile, kDataCopy);
    std::memcpy(g_Data_004da000 + kRsrcOffset, b.data() + kRsrcFile, kRsrcCopy);
    for (std::uint32_t i = 0; i < g_ImageDataRelocationCount; ++i)   // relocation sites hold 0 until relocate(), as compiled in
        *static_cast<std::uint32_t*>(ImageData_Address(g_ImageDataRelocations[i].va)) = 0;
    // the loaded bytes must be exactly the image a compiled-in build holds (hash of .rdata copy + .data + .rsrc)
    BCRYPT_ALG_HANDLE a2 = nullptr; BCRYPT_HASH_HANDLE hh = nullptr; unsigned char ih[32] = {};
    BCryptOpenAlgorithmProvider(&a2, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    BCryptCreateHash(a2, &hh, nullptr, 0, nullptr, 0, 0);
    BCryptHashData(hh, g_RData_004cc000, kRDataCopy, 0);
    BCryptHashData(hh, g_Data_004da000, kDataCopy, 0);
    BCryptHashData(hh, g_Data_004da000 + kRsrcOffset, kRsrcCopy, 0);
    BCryptFinishHash(hh, ih, sizeof ih, 0);
    BCryptDestroyHash(hh); BCryptCloseAlgorithmProvider(a2, 0);
    if (std::memcmp(ih, kImageSha256, sizeof ih) != 0) return fail("loaded data image differs from the reference image");
    return relocate();
}
"""
    i = cpp.index('bool ImageData_Block(')
    cpp = cpp[:i] + api.lstrip('\n') + '\n' + cpp[i:]
    cpp = cpp.replace('#include <windows.h>\n', '#include <windows.h>\n#include <bcrypt.h>\n#include <cstdio>\n#include <vector>\n', 1)
    cpp = cpp.replace('// The port\'s mirror of the original\'s data sections',
                      '// RUNTIME variant (the default): contains no bytes of Recoil.exe; ImageData_LoadFromExe reads the player\'s copy.\n'
                      '// The port\'s mirror of the original\'s data sections', 1)
    return cpp



def main():
    if EXE_BYTES is None and not os.path.exists(EXE):
        return refresh_without_exe()
    b = _exe_bytes()
    pe = struct.unpack_from('<I', b, 0x3c)[0]
    nsec = struct.unpack_from('<H', b, pe + 6)[0]
    opt = struct.unpack_from('<H', b, pe + 20)[0]
    secs = {b[pe + 24 + opt + i * 40:pe + 24 + opt + i * 40 + 8].rstrip(b'\0').decode(): struct.unpack_from('<IIII', b, pe + 24 + opt + i * 40 + 8)
            for i in range(nsec)}
    iat_rva, iat_size = struct.unpack_from('<II', b, pe + 24 + 96 + 12 * 8)  # data directory 12: IAT

    def sec_of(va):
        for nm, (vs, v, rs, ro) in secs.items():
            if v <= va - BASE < v + max(vs, rs):
                return nm
        return None

    blocks = [(int(r['address'], 16), int(r['size']), r['symbol'], r['header'])
              for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'data_blocks.csv'), encoding='utf-8'))]

    def in_block(va):
        for a, n, sym, hdr in blocks:
            if a <= va < a + n:
                return a, sym
        return None

    # Recoil.exe has no relocation table (relocations stripped), so pointers are recognised by value. Text is not a
    # pointer even where four of its bytes read as an image address ("s%s\0" at 0x004e300c = 0x00732573 was
    # "relocated" into the mirror, breaking SearchPath_Resolve's format): words overlapping a string or char array
    # that Ghidra defined (03_re/ledger/data_string_ranges.csv, exported from the Ghidra project) are left as bytes.
    import bisect
    strings = sorted((int(r['start'], 16), int(r['end'], 16))
                     for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'data_string_ranges.csv'), encoding='utf-8')))
    string_starts = [s for s, e in strings]

    def in_string(va):
        k = bisect.bisect_right(string_starts, va + 3) - 1  # last string starting at or before the word's last byte
        return k >= 0 and strings[k][1] > va

    # 3-character strings that read as code addresses and that Ghidra never defined as strings (found 2026-09-30 by the boot probe:
    # the relocation turned them into trap pointers). Bytes read: AI mode names ZIG/BAC/FOL at 0x004da0fc.., unit types "fly"/"SUB"
    # at 0x004dc944 / 0x004dca4c, "IRC" 0x004dd2d8, "OFF" 0x004de4c0, key name "TAB" 0x004e0bec, "MED" 0x004e231c.
    TEXT_WORDS = {0x004da0fc, 0x004da100, 0x004da104, 0x004da10c, 0x004dc944, 0x004dca4c, 0x004dd2d8, 0x004de4c0, 0x004e0bec,
                  0x004e231c, 0x004e095c, 0x004e0a68}  # + "HEA" 0x004da104, key names "END" 0x004e095c / "ADD" 0x004e0a68

    ledger = [r for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))]
    ported = {int(r['address'], 16): r['remake_symbol'] for r in ledger if r.get('remake_symbol')}
    # which ported symbols have a declaration in a header (so their address can be taken here)
    decl_headers = {}
    for h in glob.glob(os.path.join(ROOT, '05_remake', 'src', '**', '*.h'), recursive=True):
        t = io.open(h, encoding='utf-8').read()
        rel = os.path.relpath(h, os.path.join(ROOT, '05_remake', 'src')).replace('\\', '/')
        for sym in set(ported.values()):
            if re.search(r'\b%s\s*\(' % re.escape(sym), t):
                decl_headers.setdefault(sym, rel)

    # Import stubs (PLATFORM): a data word that points at one of the original's import thunks (JMP dword ptr [slot] - e.g. the
    # vtable slots of the MFC methods a game class inherits) becomes a pointer to a generated stub that jumps through the port's
    # slot for that import. The slot is read at call time, so the stub works whatever order the static initialisers run in.
    slot_decl = {}
    for h in glob.glob(os.path.join(ROOT, '05_remake', 'src', 'platform', '*.h')):
        rel = os.path.relpath(h, os.path.join(ROOT, '05_remake', 'src')).replace('\\', '/')
        for nm, sl in re.findall(r'\b(g_Iat_\w+_(004cc[0-9a-f]{3}))\s*;', io.open(h, encoding='utf-8').read()):
            slot_decl[int(sl, 16)] = (nm, rel)
    _vs_t, v_t, _rs_t, ro_t = secs['.text']

    def import_thunk(w):
        o = ro_t + (w - BASE - v_t)
        if 0 <= o < len(b) - 6 and b[o:o + 2] == b'\xff\x25':
            return slot_decl.get(struct.unpack_from('<I', b, o + 2)[0])
        return None
    import_stubs = {}
    vs_r, v_r, rs_r, ro_r = secs['.rdata']
    vs_d, v_d, rs_d, ro_d = secs['.data']
    rdata = bytearray(b[ro_r:ro_r + min(rs_r, vs_r)])
    rdata[iat_rva - v_r:iat_rva - v_r + iat_size] = bytes(iat_size)  # import slots: reached through src/platform
    data = bytearray(b[ro_d:ro_d + rs_d])
    vs_s, v_s, rs_s, ro_s = secs['.rsrc']
    rsrc = b[ro_s:ro_s + rs_s]  # appended after .data/.bss below (the image's tail, 0x%x bytes)
    relocs, counts = [], {}
    for name, img, v, is_data in (('.rdata', rdata, v_r, False), ('.data', data, v_d, True)):
        for o in range(0, len(img) - 3, 4):
            va = BASE + v + o
            if not is_data and iat_rva <= va - BASE < iat_rva + iat_size:
                continue
            w = struct.unpack_from('<I', img, o)[0]
            if not (BASE + 0x1000 <= w < BASE + 0x3c9000):
                continue
            if in_block(va):
                continue  # the named block defines its own contents
            if in_string(va) or va in TEXT_WORDS:
                counts['string'] = counts.get('string', 0) + 1  # text that reads as an address: not relocated
                continue
            tgt = sec_of(w)
            blk = in_block(w)
            if blk:
                kind = 'block'
            elif tgt in ('.rdata', '.data'):
                kind = 'mirror'
            elif tgt == '.text' and w in ported and ported[w] in decl_headers:
                kind = 'function'
            elif tgt == '.text' and import_thunk(w):
                kind = 'function'
                import_stubs[w] = import_thunk(w)
                ported[w] = 'ImportStub_%08x' % w
                decl_headers[ported[w]] = 'platform/image/original_data.h'
            else:
                kind = 'unported'
            counts[kind] = counts.get(kind, 0) + 1
            relocs.append((va, w, kind, blk))
            struct.pack_into('<I', img, o, 0)  # the relocation writes the real value at start-up
    # Check: a relocated word inside a printable NUL-terminated run of 4+ characters is probably text Ghidra never
    # defined as a string ("MHZ\0" of "CPU_MHZ" at 0x004da684 was relocated and broke Preset_EvaluateCondition). Such
    # a word fails the generation until it is defined as a string in Ghidra (then re-export data_string_ranges.csv
    # with 03_re/scripts/ghidra/DumpStringRanges.java) or, after review, listed in TEXT_RELOC_REVIEWED as a pointer.
    TEXT_RELOC_REVIEWED = {0x004d2520}  # .rdata "?p.F" run: 8 bytes before a zero; not code-referenced as text

    def orig_byte(va):
        for vs, v, rs, ro in (secs['.rdata'], secs['.data']):
            if v <= va - BASE < v + min(vs, rs):
                return b[ro + va - BASE - v]
        return None

    def printable(c):
        return c is not None and 32 <= c < 127

    suspicious = []
    for va, w, k, blk in relocs:
        ws = [orig_byte(va + i) for i in range(4)]
        if va in TEXT_RELOC_REVIEWED or not printable(ws[0]) or not all(printable(c) or c == 0 for c in ws):
            continue
        st = va
        while printable(orig_byte(st - 1)):
            st -= 1
        en = va
        while printable(orig_byte(en)):
            en += 1
        if orig_byte(en) == 0 and en - st >= 4:
            suspicious.append((st, va, bytes(orig_byte(x) for x in range(st, en))))
    # 3-character strings fill a whole word ("r%s\0" at 0x004e07e8 = 0x00732572 was relocated into the mirror and broke
    # TextureArchiveListB_OpenImageZbd's sprintf); in .data, relocated as a data pointer, not after other text
    TEXT3_REVIEWED = {0x004dd5b0, 0x004dd5d0, 0x004dd5f0, 0x004e5b10,  # RTTI TypeDescriptors: +0 is a vftable pointer
                      0x004dc264}  # "x:O": unreferenced, not a string by position
    for va, w, k, blk in relocs:
        if va < BASE + v_d or k not in ('mirror', 'block', 'unported') or va in TEXT3_REVIEWED:
            continue
        ws = [orig_byte(va + i) for i in range(4)]
        if all(printable(c) for c in ws[:3]) and ws[3] == 0 and not printable(orig_byte(va - 1)):
            suspicious.append((va, va, bytes(ws[:3])))
    if suspicious:
        for st, va, txt in suspicious:
            print('relocated word 0x%08x inside text %r at 0x%08x' % (va, txt, st))
        sys.exit('gen_data_image: %d relocated words look like text - define the strings in Ghidra (see above)' % len(suspicious))
    headers = sorted({decl_headers[ported[w]] for va, w, k, blk in relocs if k == 'function'} | {blk and h for a, n, s, h in blocks for blk in [1]})

    def arr(bs):
        lines = []
        for i in range(0, len(bs), 32):
            lines.append('    ' + ','.join(str(x) for x in bs[i:i + 32]) + ',')
        return '\n'.join(lines)

    os.makedirs(OUT, exist_ok=True)
    io.open(os.path.join(OUT, 'original_data.h'), 'w', encoding='utf-8', newline='').write('''// SUBSYSTEM: platform
// The port's mirror of the original's data sections (decision D7, STAGE2.md section 5). Generated by
// tools/asm_port/gen_data_image.py from Recoil.exe - do not edit by hand.
#pragma once

#include <cstdint>

namespace recoil {

constexpr std::uint32_t kRDataVa = 0x%08x;   // CONFIRMED-DATA: Recoil.exe section table, .rdata virtual address
constexpr std::uint32_t kRDataSize = 0x%x;   // CONFIRMED-DATA: .rdata virtual size
constexpr std::uint32_t kDataVa = 0x%08x;    // CONFIRMED-DATA: .data virtual address
constexpr std::uint32_t kDataSize = 0x%x;    // CONFIRMED-DATA: .data virtual size (file part, then .bss)

// Each mirror is followed by a 64 KB no-access guard (PLATFORM): code that runs off the end of the original's
// data faults there, as the original faults at the end of its image, instead of overwriting the program's own data.
constexpr std::uint32_t kMirrorGuard = 0x10000;  // PLATFORM: guard size, not a game value
constexpr std::uint32_t kRDataAlloc = ((kRDataSize + 0xFFFu) & ~0xFFFu) + kMirrorGuard;  // PLATFORM: page-rounded + guard
constexpr std::uint32_t kImageTail = 0x%x;  // CONFIRMED-DATA: .rsrc end - .data start: the mirror runs to the image's end
constexpr std::uint32_t kDataAlloc = ((kImageTail + 0xFFFu) & ~0xFFFu) + kMirrorGuard;    // PLATFORM: page-rounded + guard
extern unsigned char g_RData_004cc000[kRDataAlloc];
extern unsigned char g_Data_004da000[kDataAlloc];

// Where the port keeps the original address va (named block, mirror); nullptr outside the data sections.
void* ImageData_Address(std::uint32_t va);
// Relocations applied at start-up, by kind (for tests and reports).
struct ImageDataRelocation { std::uint32_t va, target; std::uint8_t kind; };  // kind: 0 mirror, 1 block, 2 function, 3 unported
extern const ImageDataRelocation g_ImageDataRelocations[];
extern const std::uint32_t g_ImageDataRelocationCount;
// Runtime image (the default; --compiled builds the bytes in instead): the mirrors are filled from the player's own Recoil.exe at start-up.
// ImageData_LoadFromExe checks the file (SHA-256 of the supported 1999-01-29 build), copies .rdata/.data/.rsrc, applies the
// relocations; why receives a message on failure. In a compiled-in build it does nothing and returns true.
bool ImageData_IsRuntime();
bool ImageData_LoadFromExe(const char* path, char* why, unsigned why_size);
// The original address the port keeps at p (inverse of ImageData_Address; also ported functions), or 0.
std::uint32_t ImageData_VaOf(const void* p);
// The i-th named data block (a port global outside the mirror): its storage and size; false past the last.
// Test harnesses snapshot and restore these along with the mirror (tests/native_oracle.h restore_pristine).
bool ImageData_Block(unsigned i, void** at, std::uint32_t* size);
// Target of every pointer into code not yet ported: stops the program loudly.
void ImageData_Unported();

}  // namespace recoil
''' % (BASE + v_r, vs_r, BASE + v_d, vs_d, (v_s + vs_s) - v_d))
    blockrefs = '\n'.join('    {0x%08x, 0x%x, %s},' % (a, n, '(void*)&' + sym) for a, n, sym, h in blocks)
    relrows = '\n'.join('    {0x%08x, 0x%08x, %d},' % (va, w, {'mirror': 0, 'block': 1, 'function': 2, 'unported': 3}[k]) for va, w, k, blk in relocs)
    # every ported function with a declaration: relocation targets and the reverse map (ImageData_VaOf)
    fn_all = sorted((w, sym) for w, sym in ported.items() if sym in decl_headers)
    fnrows = '\n'.join('    {0x%08x, reinterpret_cast<void*>(&%s)},' % (a, sym) for a, sym in fn_all)
    cpp = '''// SUBSYSTEM: platform
// The port's mirror of the original's data sections (decision D7). Generated by tools/asm_port/gen_data_image.py
// from Recoil.exe - do not edit by hand. Initial bytes: CONFIRMED-DATA (the file); relocated words are written at
// start-up from g_ImageDataRelocations.
#include "platform/image/original_data.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <intrin.h>
#include <cstring>

%s

namespace recoil {

alignas(4096) unsigned char g_RData_004cc000[kRDataAlloc] = {
%s
};
alignas(4096) unsigned char g_Data_004da000[kDataAlloc] = {
%s
};

const ImageDataRelocation g_ImageDataRelocations[] = {
%s
};
const std::uint32_t g_ImageDataRelocationCount = sizeof g_ImageDataRelocations / sizeof g_ImageDataRelocations[0];

namespace {
struct Block { std::uint32_t va, size; void* at; };
const Block kBlocks[] = {
%s
};
// .rsrc bytes (CONFIRMED-DATA, the file), copied into the mirror's tail at start-up
const unsigned char kRsrcBytes[] = {
%s
};
constexpr std::uint32_t kRsrcOffset = 0x%x;  // CONFIRMED-DATA: .rsrc start - .data start
struct Fn { std::uint32_t va; void* at; };
const Fn kFunctions[] = {
%s
    {0, nullptr},
};

void* function_at(std::uint32_t va)
{
    for (const Fn& f : kFunctions)
        if (f.va == va) return f.at;
    return nullptr;
}

bool relocate()
{
    std::memcpy(g_Data_004da000 + kRsrcOffset, kRsrcBytes, sizeof kRsrcBytes);
    for (std::uint32_t i = 0; i < g_ImageDataRelocationCount; ++i) {
        const ImageDataRelocation& r = g_ImageDataRelocations[i];
        void* target = r.kind == 2 ? function_at(r.target) : r.kind == 3 ? reinterpret_cast<void*>(&ImageData_Unported)
                                                                          : ImageData_Address(r.target);
        auto* slot = static_cast<std::uint32_t*>(ImageData_Address(r.va));
        *slot = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target));
    }
    DWORD old = 0;
    VirtualProtect(g_RData_004cc000 + (kRDataAlloc - kMirrorGuard), kMirrorGuard, PAGE_NOACCESS, &old);
    VirtualProtect(g_Data_004da000 + (kDataAlloc - kMirrorGuard), kMirrorGuard, PAGE_NOACCESS, &old);
    return true;
}
const bool g_relocated = relocate();
}  // namespace

bool ImageData_Block(unsigned i, void** at, std::uint32_t* size)
{
    if (i >= sizeof kBlocks / sizeof kBlocks[0]) return false;
    *at = kBlocks[i].at;
    *size = kBlocks[i].size;
    return true;
}

void* ImageData_Address(std::uint32_t va)
{
    for (const Block& b : kBlocks)
        if (va >= b.va && va < b.va + b.size) return static_cast<unsigned char*>(b.at) + (va - b.va);
    if (va >= kRDataVa && va < kRDataVa + kRDataSize) return g_RData_004cc000 + (va - kRDataVa);
    if (va >= kDataVa && va < kDataVa + kDataSize) return g_Data_004da000 + (va - kDataVa);
    return nullptr;
}

std::uint32_t ImageData_VaOf(const void* p)
{
    const auto* c = static_cast<const unsigned char*>(p);
    for (const Block& b : kBlocks)
        if (c >= static_cast<const unsigned char*>(b.at) && c < static_cast<const unsigned char*>(b.at) + b.size)
            return b.va + static_cast<std::uint32_t>(c - static_cast<const unsigned char*>(b.at));
    if (c >= g_RData_004cc000 && c < g_RData_004cc000 + kRDataSize) return kRDataVa + static_cast<std::uint32_t>(c - g_RData_004cc000);
    if (c >= g_Data_004da000 && c < g_Data_004da000 + kDataSize) return kDataVa + static_cast<std::uint32_t>(c - g_Data_004da000);
    for (const Fn& f : kFunctions)
        if (f.at && f.at == p) return f.va;
    return 0;
}

void ImageData_Unported()
{
    // the caller's return address names the call site (look it up in the executable's .map): the next blocker
    std::fprintf(stderr, "reached original code that is not ported yet (through a data pointer), called from %%p\\n", _ReturnAddress());
    std::abort();
}

}  // namespace recoil
''' % ('\n'.join('#include "%s"' % h for h in sorted({h for a, n, s, h in blocks} | {decl_headers[sym] for w, sym in fn_all})),
       arr(rdata), arr(data), relrows, blockrefs, arr(rsrc), v_s - v_d, fnrows)
    if import_stubs:
        hp = os.path.join(OUT, 'original_data.h')
        ht = io.open(hp, encoding='utf-8').read()
        decls = ('// Import stubs (PLATFORM): stand-ins for the original import thunks that data points at (JMP dword ptr [slot]).\n'
                 + ''.join('void ImportStub_%08x();  // original thunk 0x%08x -> %s\n' % (w, w, nm) for w, (nm, rel) in sorted(import_stubs.items())))
        k = ht.rindex('\n}  // namespace recoil')
        io.open(hp, 'w', encoding='utf-8', newline='').write(ht[:k] + '\n' + decls + ht[k:])
        incs = ''.join('#include "%s"\n' % rel for rel in sorted({rel for nm, rel in import_stubs.values()}))
        cpp = cpp.replace('#include "platform/image/original_data.h"\n', '#include "platform/image/original_data.h"\n' + incs, 1)
        defs = ('// Import stubs (PLATFORM): each jumps through the port\'s slot of the import the original thunk jumps through.\n'
                + ''.join('__declspec(naked) void ImportStub_%08x()\n{\n    __asm { jmp dword ptr [%s] }\n}\n' % (w, nm)
                          for w, (nm, rel) in sorted(import_stubs.items())) + '\n')
        k = cpp.index('const Fn kFunctions[] = {')
        cpp = cpp[:k] + defs + cpp[k:]
    cpp = runtime_variant(cpp, b, secs, iat_rva, iat_size, v_s - v_d, bytes(rdata) + bytes(data) + bytes(rsrc)) if '--compiled' not in sys.argv else compiled_variant(cpp)   # default: runtime (no Recoil bytes); --compiled: the image built in
    target = os.path.join(OUT, 'original_data.cpp')
    if '--check' in sys.argv:
        # build-time guard: the committed image must match what the ledger generates now (a newly ported function
        # must not stay behind the unported-code trap)
        current = io.open(target, encoding='utf-8').read() if os.path.exists(target) else ''
        if current != cpp:
            sys.exit('data image is stale: run python tools/asm_port/gen_data_image.py')
        print('data image up to date (%d relocations)' % len(relocs))
        return
    io.open(target, 'w', encoding='utf-8', newline='').write(cpp)
    print('relocations', len(relocs), counts)


if __name__ == '__main__':
    main()
