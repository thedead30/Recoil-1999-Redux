"""gen_fuzz_test.py - generate an arena-fuzz native L1 test (tests/arena_fuzz.h) for a batch of ported functions.

For each function: its address, port symbol (ledger remake_symbol, or ghidra_name for a batch just generated),
stack-argument bytes (from RET n in the listing) and every data-section global its listing touches (compared on
both sides through recoil::ImageData_Address, i.e. named blocks or the data image).
usage: python tools/asm_port/gen_fuzz_test.py LISTING ADDR[,ADDR...] --name NAME --include HEADER[,HEADER...]
                                              [--iters N] [--skip ADDR[,ADDR...]]
--real-floats: half of the non-pointer arena words are ordinary float values (for float code).
--skip: functions the arena cannot exercise fairly (a FILE argument, indices into heap tables); each needs its own
        structured native test (e.g. tests/test_gmod_read_native.cpp).
writes 05_remake/tests/test_fuzz_NAME.cpp
"""
import argparse, csv, io, os, re

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DATA_LO, DATA_HI = 0x004cc000, 0x0077a000  # .rdata start .. .data/.bss end (Recoil.exe section table)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('listing')
    ap.add_argument('addrs')
    ap.add_argument('--name', required=True)
    ap.add_argument('--include', required=True)
    ap.add_argument('--iters', type=int, default=1000)
    ap.add_argument('--skip', default='')
    ap.add_argument('--out-dir', default='', help='write the test here instead of 05_remake/tests (staging while a build runs)')
    ap.add_argument('--real-floats', action='store_true', help='fill half the scalar words with ordinary floats (af::real_floats)')
    ap.add_argument('--dictionary', action='store_true', help='harvest the constants each function compares / loads (+-1) into af::dictionary() for its run; functions with none are skipped')
    a = ap.parse_args()
    ledger ={r['address'].lower(): r for r in csv.DictReader(open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))}
    listing = io.open(a.listing, encoding='utf-8').read().split('### ')
    rows = []
    skip = {x.strip().lower().rjust(8, '0') for x in a.skip.split(',') if x.strip()}
    for addr in [x.strip().lower().rjust(8, '0') for x in a.addrs.split(',') if x.strip()]:
        if addr in skip:
            continue
        body = [x for x in listing if x.startswith(addr)][0]
        r = ledger['0x' + addr]
        sym = r['remake_symbol'] or r['ghidra_name']
        stack = max([int(x, 16) for x in re.findall(r'RET 0x([0-9a-f]+)', body)] or [0])
        # globals of the function (randomised) and of every callee the listing holds, transitively (kept: reset to
        # pristine for each call and compared) - a global only a callee touches must not carry state between calls
        # (Container_GrowFreeList's free list lives in Container_NodeFree_ToFreeList)
        gl, seen, todo = set(), set(), [body]
        own = None
        while todo:
            b = todo.pop()
            for c in re.findall(r'\b(?:CALL|JMP)\s+0x([0-9a-f]{6,8})', b):
                k = c.rjust(8, '0')
                hit = [x for x in listing if x.startswith(k)]
                if hit and k not in seen and k != addr:
                    seen.add(k)
                    todo.append(hit[0])
            for m in re.finditer(r'\[0x([0-9a-f]{6,8})\]|\b0x([0-9a-f]{6,8})\b', b):
                va = int(m.group(1) or m.group(2), 16)
                if 0x004cd000 <= va < DATA_HI and va >= 0x004da000:  # writable data only (.data/.bss)
                    gl.add(va & ~3)
                    if re.search(r'(double|qword) ptr \[0x%x\]' % va, b):
                        gl.add((va & ~3) + 4)
            if own is None:
                own = set(gl)  # the first body is the function's own
        dic, special = set(), set()
        if a.dictionary:
            # immediates of the function's own instructions (not [reg+0xNN] displacements, not image addresses, not tiny values a plain
            # arena word already hits): the values a compare / mask / magic check is written against, each with +-1
            for line in body.splitlines()[1:]:
                ops = line.split(' ', 2)[-1] if line.count(' ') >= 2 else ''
                stripped = re.sub(r'\[[^\]]*\]', '', ops)
                for h in re.findall(r'(?<![-\w])0x([0-9a-f]{1,8})\b', stripped):
                    v = int(h, 16)
                    if v >= 0x10 and not (0x00401000 <= v < 0x0077a000):
                        dic.update({(v - 1) & 0xffffffff, v, (v + 1) & 0xffffffff})
            if re.search(r'\bF(?:U)?COM|\bFTST|\bFXAM', body):
                # a float compare tested with `test AH,0x41` (C0|C3) has a variant that only differs for unordered operands (C2): give it NaN, +-inf and -0.0
                special = {0x7fc00000, 0x7f800000, 0xff800000, 0x80000000}
                dic.update(special)
            if not dic:
                continue
        rows.append((addr, sym, stack // 4, sorted(own), sorted(gl - own), (sorted(special) + sorted(dic - special))[:48]))
    t = ['// Arena-fuzz native L1 for a ported batch (tests/arena_fuzz.h). Generated by tools/asm_port/gen_fuzz_test.py.',
         '#include "test.h"', '#include "arena_fuzz.h"', '#include "platform/image/original_data.h"']
    heads = [h.strip() for h in a.include.split(',')]
    bad = [h for h in heads if not h or h.startswith('.') or not h.endswith('.h')]
    if bad:  # a cloud run once wrote '#include ".h"' from an empty module name
        raise SystemExit('bad --include entries: %r' % bad)
    t += ['#include "%s"' % h for h in heads]
    t += ['', '#include <cstdio>', '#include <cstdlib>', '#include <cstring>', '', 'namespace {', 'struct Entry { std::uint32_t va; void* port; int nstack; const char* name; std::vector<std::uint32_t> globals, kept, dict; };',
          'std::vector<Entry> entries()', '{', '    return {']
    for addr, sym, n, gl, kept, dic in rows:
        t.append('        {0x%s, reinterpret_cast<void*>(&recoil::%s), %d, "%s", {%s}, {%s}, {%s}},' % (addr, sym, n, sym, ', '.join('0x%08x' % g for g in gl),
                                                                                    ', '.join('0x%08x' % g for g in kept), ', '.join('0x%08x' % g for g in dic)))
    t += ['    };', '}', '}  // namespace', '',
          'TEST(native_fuzz_%s_match_original)' % a.name, '{',
          '    if (!rt::map_original()) { CHECK(false); return; }',
          '    af::real_floats() = %s;  // ordinary float values in half the scalar words' % ('true' if a.real_floats else 'false'),
          '    int failed = 0;',
          '    const char* only = std::getenv("RECOIL_ONLY");  // diagnostics: run one function by name',
          '    int shard = 0, shards = 1;  // RECOIL_SHARD=i/n: this process runs entries i, i+n, ... (tools/run_tests.py)',
          '    if (const char* s = std::getenv("RECOIL_SHARD")) std::sscanf(s, "%d/%d", &shard, &shards);',
          '    int index = 0;',
          '    for (const Entry& e : entries()) {',
          '        if (index++ % shards != shard) continue;',
          '        if (only && std::strcmp(only, e.name) != 0) continue;',
          '        std::printf(\"  run  %s\\n\", e.name); std::fflush(stdout);  // names the function if a crash escapes',
          '        std::vector<af::GlobalRange> gr;',
          '        for (std::uint32_t g : e.globals) gr.push_back({g, recoil::ImageData_Address(g), 4});',
          '        for (std::uint32_t g : e.kept) gr.push_back({g, recoil::ImageData_Address(g), 4, true});  // callee-only: pristine each call',
          '        af::dictionary() = e.dict;  // empty for a plain arena test',
          '        const af::Result r = af::run(e.va, e.port, e.nstack, %d, e.va, gr);' % a.iters,
          '        af::dictionary().clear();',
          '        std::printf("  %-40s calls %5d abnormal %5d %s%s\\n", e.name, r.calls, r.abnormal, r.bad ? "DIFF " : "ok", r.first.c_str());',
          '        if (r.bad || r.calls - r.abnormal < 50) ++failed;  // at least 50 calls that ended normally (and compared data)',
          '    }',
          '    CHECK_EQ(failed, 0);', '}', '']
    out = os.path.join(a.out_dir or os.path.join(ROOT, '05_remake', 'tests'), 'test_fuzz_%s.cpp' % a.name)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    io.open(out, 'w', encoding='utf-8', newline='').write('\n'.join(t))
    print('wrote', out, len(rows), 'functions')


if __name__ == '__main__':
    main()
