"""gap_digest.py - a compact, evidence-only digest of large gap functions from a listing dump (no decompile reading).

For each function in the listing (DumpListingsFile format) prints: size, the ledger names of the functions it CALLs / JMPs to (hex when unnamed),
the imports it calls through platform slots (DLL names from platform/iat_*.h), the printable strings whose addresses it references (read from
the executable), its data globals, and its switch / vtable-call shapes. Enough to name a function honestly; the note must still say that only the
digest (calls / strings / imports) was read.
usage: python tools/gap_digest.py LISTING [ADDR ...]
"""
import csv, glob, io, os, re, struct, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def main():
    exe = open(os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe'), 'rb').read()
    pe = struct.unpack_from('<I', exe, 0x3c)[0]
    ns = struct.unpack_from('<H', exe, pe + 6)[0]
    oh = struct.unpack_from('<H', exe, pe + 20)[0]
    secs = []
    for i in range(ns):
        o = pe + 24 + oh + 40 * i
        secs.append((exe[o:o + 8].rstrip(b'\0').decode(), struct.unpack_from('<I', exe, o + 12)[0] + 0x400000,
                     struct.unpack_from('<I', exe, o + 20)[0], struct.unpack_from('<I', exe, o + 16)[0]))

    def fo(va):
        for n, sva, ro, rs in secs:
            if sva <= va < sva + rs:
                return ro + va - sva

    def cstr(va):
        o = fo(va)
        if o is None:
            return None
        e = exe.find(b'\0', o, o + 200)
        if e < 0 or e - o < 4:
            return None
        s = exe[o:e]
        return s.decode('latin1') if all(32 <= c < 127 or c in (9, 10, 13) for c in s) else None

    led = {r['address']: r['ghidra_name'] for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))}
    slots = {}
    for f in glob.glob(os.path.join(ROOT, '05_remake/src/platform/iat_*.h')) + [os.path.join(ROOT, '05_remake/src/platform/advapi.h')]:
        if os.path.exists(f):
            for nm, va in re.findall(r'g_Iat_(\w+?)_(004cc[0-9a-f]{3})\b', open(f, encoding='utf-8').read()):
                slots[int(va, 16)] = nm
    want = {('0x%08x' % int(a, 16)) for a in sys.argv[2:]}
    for chunk in io.open(sys.argv[1], encoding='utf-8', errors='replace').read().replace('\r\n', '\n').split('### ')[1:]:
        lines = chunk.strip().splitlines()
        addr = '0x' + lines[0].split()[0]
        if want and addr not in want:
            continue
        ins = [l.split(' ', 1)[1] for l in lines[1:]]
        calls, imps, strs, glob_ = [], [], [], []
        vcalls = 0
        for t in ins:
            m = re.match(r'(?:CALL|JMP) 0x([0-9a-f]{6,8})', t)
            if m:
                a = '0x%08x' % int(m.group(1), 16)
                nm = led.get(a, a)
                if nm not in calls:
                    calls.append(nm)
            m = re.search(r'(?:CALL|JMP) dword ptr \[0x(004cc[0-9a-f]{3})\]', t)
            if m and int(m.group(1), 16) in slots:
                n = slots[int(m.group(1), 16)]
                if n not in imps:
                    imps.append(n)
            if re.search(r'CALL dword ptr \[E\w\w( \+ 0x[0-9a-f]+)?\]', t):
                vcalls += 1
            for m in re.finditer(r'0x([0-9a-f]{6,8})\b', t):
                v = int(m.group(1), 16)
                if 0x4cd000 <= v < 0x7c9000:
                    s = cstr(v)
                    if s and s not in strs and len(strs) < 8:
                        strs.append(s)
                    elif not s and v >= 0x4da000 and ('0x%08x' % v) not in glob_ and len(glob_) < 8 and not t.startswith(('CALL', 'JMP')):
                        glob_.append('0x%08x' % v)
        print('%s %s: %d instr | calls: %s | imports: %s | vcalls %d | strings: %s | globals: %s' % (
            addr, lines[0].split(' ', 1)[1], len(ins), ', '.join(calls[:14]) or '-', ', '.join(imps[:8]) or '-', vcalls,
            ' | '.join(repr(s[:60]) for s in strs) or '-', ' '.join(glob_[:6]) or '-'))


if __name__ == '__main__':
    main()
