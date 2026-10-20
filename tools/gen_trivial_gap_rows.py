"""gen_trivial_gap_rows.py - names + evidence notes for TRIVIAL gap functions (<= N instructions) from their exact instructions.

Input: a listing dump of the functions (DumpListingsFile format, from Ghidra) and --max-instr N. Output (next to --out PREFIX):
  PREFIX.json        rows for tools/add_gap_functions.py (name, subsystem, notes)
  PREFIX_names.txt   "0xADDR name" lines for the Ghidra rename script
Names come from a small pattern table over the instruction text (return-constant, return-true / zero with a stack pop, plain JMP to
another function, PUSH n / CALL x / RET forwarders ...); anything that matches no pattern is skipped and printed, to be read by hand.
The note quotes the instructions, where the address is referenced in the data sections (vtables / callback tables: byte scan of the
executable), and how the subsystem was chosen (nearest ledger function below the address that is a subsystem member). Roles that need
interpretation (e.g. "typical MFC GetRuntimeClass") are marked INFERRED.
usage: python tools/gen_trivial_gap_rows.py LISTING --max-instr 3 --out PREFIX
"""
import argparse, bisect, csv, io, json, os, re, struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('listing')
    ap.add_argument('--max-instr', type=int, default=3)
    ap.add_argument('--out', required=True)
    ap.add_argument('--style', choices=['trivial', 'decompile', 'asis'], default='trivial', help='note wording: trivial = quote the instructions; decompile = the meaning text comes from a decompile read')
    ap.add_argument('--manual', default='', help='JSON {addr: [name, meaning]}: hand-read names / meanings that override the pattern table')
    a = ap.parse_args()
    exe = open(os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe'), 'rb').read()
    pe = struct.unpack_from('<I', exe, 0x3c)[0]
    ns = struct.unpack_from('<H', exe, pe + 6)[0]
    oh = struct.unpack_from('<H', exe, pe + 20)[0]
    secs = []
    for i in range(ns):
        o = pe + 24 + oh + 40 * i
        secs.append((exe[o:o + 8].rstrip(b'\0').decode(), struct.unpack_from('<I', exe, o + 12)[0] + 0x400000,
                     struct.unpack_from('<I', exe, o + 20)[0], struct.unpack_from('<I', exe, o + 16)[0]))
    led = {r['address']: r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))}
    members = sorted((int(r['address'], 16), r['subsystem'])
                     for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/subsystems.csv'), encoding='utf-8')))
    maddr = [m[0] for m in members]

    def subsystem(va):
        i = bisect.bisect_right(maddr, va) - 1
        return members[i][1] if i >= 0 else 'app'

    def pointer_sites(va):
        pat = struct.pack('<I', va)
        sites = []
        for name, sva, ro, rs in secs:
            if name == '.text':
                continue
            blob = exe[ro:ro + rs]
            k = 0
            while len(sites) < 3:
                k = blob.find(pat, k)
                if k < 0:
                    break
                sites.append('%s 0x%08x' % (name, sva + k))
                k += 1
        return sites

    def tname(t):
        r = led.get('0x%08x' % t)
        return r['ghidra_name'] if r and not r['ghidra_name'].startswith('FUN_') else '%08x' % t

    manual = {('0x%08x' % int(k, 16)): v for k, v in json.load(open(a.manual, encoding='utf-8')).items()} if a.manual else {}
    rows, names, skipped = {}, [], []
    for chunk in io.open(a.listing, encoding='utf-8', errors='replace').read().replace('\r\n', '\n').split('### ')[1:]:
        lines = chunk.strip().splitlines()
        addr = '0x' + lines[0].split()[0]
        va = int(addr, 16)
        ins = [l.split(' ', 1)[1] for l in lines[1:]]
        if len(ins) > a.max_instr:
            continue
        txt = ' ; '.join(ins)
        name, meaning = None, ''
        m = re.fullmatch(r'MOV EAX,0x([0-9a-f]+) ; RET', txt)
        if m and int(m.group(1), 16) >= 0x4cc000:
            name = 'Slot_ReturnConstAddr_%08x_%08x' % (int(m.group(1), 16), va)
            meaning = 'Returns the constant address 0x%s in the data sections. INFERRED: the usual shape of an MFC GetRuntimeClass override (returns &classXxx).' % m.group(1)
        m = re.fullmatch(r'MOV EAX,\[0x([0-9a-f]+)\] ; RET', txt)
        if not name and m:
            name = 'Slot_ReturnGlobal_%08x_%08x' % (int(m.group(1), 16), va)
            meaning = 'Returns the dword at 0x%s (a data / import slot).' % m.group(1)
        m = re.fullmatch(r'MOV EAX,0x1 ; RET( 0x([0-9a-f]+))?', txt)
        if not name and m:
            name = 'Slot_ReturnTrue%s_%08x' % (('_Ret' + m.group(2)) if m.group(2) else '', va)
            meaning = 'Returns 1 and pops %s bytes of arguments.' % (int(m.group(2), 16) if m.group(2) else 0)
        m = re.fullmatch(r'XOR EAX,EAX ; RET( 0x([0-9a-f]+))?', txt)
        if not name and m:
            name = 'Slot_ReturnZero%s_%08x' % (('_Ret' + m.group(2)) if m.group(2) else '', va)
            meaning = 'Returns 0 and pops %s bytes of arguments.' % (int(m.group(2), 16) if m.group(2) else 0)
        m = re.fullmatch(r'JMP 0x([0-9a-f]+)', txt)
        if not name and m:
            t = int(m.group(1), 16)
            name = 'Thunk_%s_%08x' % (tname(t), va)
            meaning = 'A jump thunk to 0x%08x (%s).' % (t, tname(t))
        m = re.fullmatch(r'PUSH 0x([0-9a-f]+) ; CALL 0x([0-9a-f]+) ; RET', txt)
        if not name and m:
            t = int(m.group(2), 16)
            name = 'Slot_Call_%s_Arg%d_%08x' % (tname(t), int(m.group(1), 16), va)
            meaning = 'Calls %s (0x%08x) with the constant argument %d and returns.' % (tname(t), t, int(m.group(1), 16))
        m = re.fullmatch(r'XOR ECX,ECX ; JMP 0x([0-9a-f]+)', txt)
        if not name and m:
            t = int(m.group(1), 16)
            name = 'Slot_ZeroECX_Jmp_%s_%08x' % (tname(t), va)
            meaning = 'Sets ECX = 0 and jumps to %s (0x%08x).' % (tname(t), t)
        m = re.fullmatch(r'MOV dword ptr \[ECX \+ 0x([0-9a-f]+)\],0x([0-9a-f]+) ; RET', txt)
        if not name and m:
            name = 'Slot_SetField%s_To%s_%08x' % (m.group(1), m.group(2), va)
            meaning = 'ECX = object: stores %s at [ECX+0x%s] and returns.' % (m.group(2), m.group(1))
        if addr in manual:
            name, meaning = manual[addr]
        if not name:
            skipped.append((addr, txt))
            continue
        sub = subsystem(va)
        sites = pointer_sites(va)
        nbytes = 0
        if a.style == 'asis':  # the meaning text already says how the function was read (e.g. DIGEST ONLY / decompile read)
            rows[addr] = {'name': name, 'subsystem': sub, 'notes':
                          '%d instructions (Ghidra function created for this gap, 2026-09-29). %s Referenced as a pointer in the data sections at %s '
                          '(vtable / message-map / callback table: address-taken data, no direct caller). Subsystem chosen by address neighbourhood (nearest subsystem member below it).' % (
                              len(ins), meaning, ', '.join(sites) if sites else 'no data site found (called directly)')}
            names.append('%s %s' % (addr, name))
            continue
        if a.style == 'decompile':
            rows[addr] = {'name': name, 'subsystem': sub, 'notes':
                          'Small function (%d instructions), decompile read 2026-09-29 (Ghidra function created for this gap). %s Referenced as a pointer in the data sections at %s '
                          '(vtable / message-map / callback table: address-taken data, no direct caller). Subsystem chosen by address neighbourhood (nearest subsystem member below it).' % (
                              len(ins), meaning, ', '.join(sites) if sites else 'no data site found (called directly)')}
            names.append('%s %s' % (addr, name))
            continue
        rows[addr] = {'name': name, 'subsystem': sub, 'notes':
                      'Trivial function, instructions read 2026-09-29 (Ghidra function created for this gap): %s. %s Referenced as a pointer in the data sections at %s '
                      '(vtable / callback table: address-taken data, no direct caller). Subsystem chosen by address neighbourhood (nearest subsystem member below it).' % (
                          txt, meaning, ', '.join(sites) if sites else 'no data site found (called directly)')}
        names.append('%s %s' % (addr, name))
    json.dump(rows, open(a.out + '.json', 'w', encoding='utf-8'), indent=1)
    open(a.out + '_names.txt', 'w').write('\n'.join(names) + '\n')
    print('rows', len(rows), 'skipped (no pattern)', len(skipped))
    for s in skipped:
        print('  SKIP', s[0], s[1])


if __name__ == '__main__':
    main()
