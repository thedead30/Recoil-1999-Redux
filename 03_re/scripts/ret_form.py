"""ret_form.py ADDR... - static ret form from the original EXE bytes (no trace launch).
Uses the corpus 'bytes=' size: the function's last instruction is checked for C3 (ret) or C2 imm16 (ret N).
Also lists every C3 / C2 xx 00 site inside the body. Approximate for large bodies (data bytes can alias);
authoritative only when the terminal byte pattern matches."""
import glob, re, struct, sys
d = open('00_original/game_install/Recoil.exe', 'rb').read()
pe = struct.unpack_from('<I', d, 0x3C)[0]
ns = struct.unpack_from('<H', d, pe + 6)[0]; oh = struct.unpack_from('<H', d, pe + 20)[0]
base = struct.unpack_from('<I', d, pe + 52)[0]
def fo(va):
    for i in range(ns):
        o = pe + 24 + oh + 40 * i
        v0 = base + struct.unpack_from('<I', d, o + 12)[0]; rs = struct.unpack_from('<I', d, o + 16)[0]
        if v0 <= va < v0 + rs:
            return struct.unpack_from('<I', d, o + 20)[0] + va - v0
for a in sys.argv[1:]:
    va = int(a, 16)
    fs = glob.glob('03_re/decomp_raw/0x%08x_*.c' % va)
    n = int(re.search(r'bytes=(\d+)', open(fs[0], errors='replace').read()).group(1)) if fs else 0
    b = d[fo(va):fo(va) + n]
    tail = 'UNKNOWN'
    if n >= 1 and b[-1] == 0xC3:
        tail = 'ret (0 stack args)'
    elif n >= 3 and b[-3] == 0xC2:
        k = struct.unpack_from('<H', b, n - 2)[0]; tail = 'ret %#x (%d stack args)' % (k, k // 4)
    print('0x%08x bytes=%d terminal: %s' % (va, n, tail))
