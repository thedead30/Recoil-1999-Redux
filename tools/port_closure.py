"""port_closure.py - port every unported function whose references close over (ported functions + this same set), in one run.

port_loop.py ports bottom-up: a function waits until each callee is ported. Mutual recursion, and callers of a function that
is itself only waiting on its caller, never become ready that way (2026-09-30: 22 ready vs 394 in a closed set). This tool
computes the greatest set S of unported functions (listings in 03_re/listings/unported/) such that every reference of every
member is resolved - itself, outside .text, a declared import slot, a ported function, or another member of S - and ports S
together: port_batch.py per target file with --pending naming all of S, the EH handlers of S (tools/gen_eh_frames.py emit),
an #include fix-up pass (a member may call one generated later into another file), the data image, one build.
A build error drops the functions whose generated code the compiler names, recomputes S without them, and retries.

It does not verify anything: every row gets IMPLEMENTED-UNVERIFIED like port_loop. Multiplayer (net / znetwork) is skipped
unless --net. It never commits.
usage: python tools/port_closure.py [--net] [--dry] [--max-tries N]
"""
import collections, csv, glob, io, json, os, re, struct, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
SRC = os.path.join(ROOT, '05_remake', 'src')
EH = os.path.join(ROOT, '03_re', 'staging', 'eh', 'eh_frames.json')
SKIPPED = os.path.join(ROOT, '03_re', 'staging', 'verify', 'port_skipped.txt')
b = open(os.path.join(ROOT, '00_original', 'game_install', 'Recoil.exe'), 'rb').read()


def run(cmd):
    return subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, errors='replace')


def text(va):
    return b[va - 0x401000 + 0x400: va - 0x401000 + 0x406]


def load():
    L = {}
    for p in glob.glob(os.path.join(ROOT, '03_re', 'listings', 'unported', '*.txt')):
        s = os.path.basename(p)[:-4]
        for x in io.open(p, encoding='utf-8').read().split('### '):
            if x[:8].strip():
                L[x[:8]] = (s, x, p)
    led = {r['address'].lower(): r for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))}
    slots = set()
    for h in glob.glob(os.path.join(SRC, 'platform', '*.h')):
        slots |= set('0x' + s for s in re.findall(r'g_Iat_\w+_([0-9a-f]{8});', io.open(h, encoding='utf-8').read()))
    eh = json.load(io.open(EH, encoding='utf-8')) if os.path.exists(EH) else {}
    return L, led, slots, eh


def graph(L, led, slots, eh):
    """deps[a] = set of member candidates a needs; hard[a] = unresolvable references."""
    tds = os.path.join(ROOT, '03_re', 'ledger', 'text_data_symbols.csv')
    text_data = {r['address'].lower() for r in csv.DictReader(io.open(tds, encoding='utf-8'))} if os.path.exists(tds) else set()

    def resolve(v, depth=0):
        c8 = '0x%08x' % v
        if c8 in slots or c8 in text_data:  # an import slot, or library data in .text mapped to a platform symbol
            return 'ok', None
        cr = led.get(c8)
        if cr and cr['remake_symbol']:
            return 'ok', None
        if '%08x' % v in L:
            return 'fn', '%08x' % v
        if c8 in eh and 'refused' not in eh[c8] and cr:
            return 'fn', 'eh:' + c8
        op = text(v)
        if depth < 4 and op[:1] == b'\xe9':
            return resolve(v + 5 + struct.unpack_from('<i', op, 1)[0], depth + 1)
        if op[:2] == b'\xff\x25':
            s = '0x%08x' % struct.unpack_from('<I', op, 2)[0]
            return ('ok', None) if s in slots else ('bad', 'import slot ' + s)
        return 'bad', '%s %s' % (c8, (cr or {}).get('ghidra_name', 'no ledger row'))
    deps, hard = {}, {}
    for a, (s, body, _) in L.items():
        r = led.get('0x' + a)
        if not r or r['remake_symbol']:
            continue
        own = {int(l.split()[0], 16) for l in body.splitlines()[1:] if re.match(r'[0-9a-f]+\s', l.strip())}
        ds = {int(x, 16) for x in re.findall(r'\b(?:CALL|JMP)\s+(?:dword ptr \[)?0x([0-9a-f]+)', body)}
        ds |= {int(x, 16) for x in re.findall(r'(?:PUSH|MOV (?:\w+|(?:dword ptr )?\[[^\]]*\]),)\s*0x(4[0-9a-c][0-9a-f]{4})\b', body)}
        fn, bad = set(), []
        for v in ds:
            if v in own or not (0x401000 <= v < 0x4cc000):
                continue
            k, x = resolve(v)
            if k == 'fn':
                fn.add(x)
            elif k == 'bad':
                bad.append(x)
        deps[a], hard[a] = fn, bad
    for h, fr in eh.items():
        if 'refused' not in fr and h in led and not led[h]['remake_symbol']:
            deps['eh:' + h] = {f[2:] for f in fr['funclets'] if not led.get(f, {}).get('remake_symbol')}
            hard['eh:' + h] = [f for f in fr['funclets'] if f[2:] not in L and not led.get(f, {}).get('remake_symbol')]
    return deps, hard


def closure(L, led, deps, hard, net, drop):
    names = collections.Counter(r['ghidra_name'] for r in led.values())
    def ok(a):
        if a.startswith('eh:'):
            return not hard[a]
        s = L[a][0]
        nm = led['0x' + a]['ghidra_name']
        return (not hard[a] and a not in drop and (net or s not in ('net', 'znetwork')) and not nm.startswith('FUN_')
                and names[nm] == 1)
    S = {a for a in deps if ok(a)}
    while True:
        rm = {a for a in S if not deps[a] <= S}
        if not rm:
            return S
        S -= rm


def target(row, subsys):
    o = row.get('orig_file', '').replace('\\', '/').strip()
    rel = re.sub(r'\.(c|cpp)$', '', o) if o else 'unattributed/' + subsys
    return '05_remake/src/%s.cpp' % rel, '05_remake/src/%s.h' % rel


def fix_includes(cpps):
    """add the #include of every header that declares a symbol a generated file names (port_batch's own rule, run after all
    files exist)."""
    decl = {}
    for hp in sorted(glob.glob(os.path.join(SRC, '**', '*.h'), recursive=True)):
        rel = os.path.relpath(hp, SRC).replace('\\', '/')
        for s in re.findall(r'\b(?:int|void)\s+__fastcall\s+(\w+)\s*\(', io.open(hp, encoding='utf-8').read()):
            decl.setdefault(s, rel)
    n = 0
    for cpp in cpps:
        fp = os.path.join(ROOT, cpp)
        t = io.open(fp, encoding='utf-8').read()
        own = os.path.relpath(fp[:-4] + '.h', SRC).replace('\\', '/')
        used = set(re.findall(r'\b(?:call|jmp) (\w+)', t)) | set(re.findall(r'\boffset (\w+)', t)) | set(re.findall(r'&(\w+)\)', t))
        for s in sorted(used):
            inc = decl.get(s)
            if inc and inc != own and ('#include "%s"' % inc) not in t:
                t = t.replace('\n\nnamespace recoil {', '\n#include "%s"\n\nnamespace recoil {' % inc, 1)
                n += 1
        io.open(fp, 'w', encoding='utf-8', newline='').write(t)
    return n


def build():
    r = run(['cmd', '/c', os.path.join(ROOT, 'tools', 'build_only.bat')])
    errs = [l for l in r.stdout.splitlines() if ' error ' in l or 'error LNK' in l or ': fatal error' in l]
    return r.returncode == 0 and not errs, errs


def main():
    net, dry = '--net' in sys.argv, '--dry' in sys.argv
    tries = int(sys.argv[sys.argv.index('--max-tries') + 1]) if '--max-tries' in sys.argv else 6
    dirty = run(['git', 'status', '--porcelain', '--untracked-files=no', '--', '05_remake/src']).stdout.strip()
    if dirty and not dry:
        sys.exit('uncommitted changes in 05_remake/src - commit them first:\n' + dirty)
    drop = set()
    for attempt in range(1, tries + 1):
        L, led, slots, eh = load()
        deps, hard = graph(L, led, slots, eh)
        S = closure(L, led, deps, hard, net, drop)
        fns = sorted(a for a in S if not a.startswith('eh:'))
        hs = sorted(a for a in S if a.startswith('eh:'))
        by = collections.Counter(L[a][0] for a in fns)
        print('try %d: closed set %d functions + %d EH handlers (%s)' % (attempt, len(fns), len(hs),
              ', '.join('%s %d' % kv for kv in sorted(by.items()))), flush=True)
        if dry or not S:
            return
        pend = {'0x' + a: led['0x' + a]['ghidra_name'] for a in fns}
        for h in hs:
            pend[h[3:]] = eh[h[3:]]['handler_name']
        pf = os.path.join(ROOT, '03_re', 'staging', 'verify', 'port_closure_pending.json')
        json.dump(pend, io.open(pf, 'w', encoding='utf-8'), indent=0)
        snap = {}
        groups = collections.defaultdict(list)
        for a in fns:
            s, _, lp = L[a]
            groups[(lp, s) + target(led['0x' + a], s)].append(a)
        # snapshot every file this attempt can touch, to restore on failure
        touch = {'03_re/ledger/functions.csv', '05_remake/src/platform/image/original_data.cpp', '05_remake/src/platform/image/original_data.h'}
        for (lp, s, cpp, hdr) in groups:
            touch |= {cpp, hdr}
        for h in hs:
            touch |= set(target(led[h[3:]], eh[h[3:]]['subsystem']))
        for f in touch:
            fp = os.path.join(ROOT, f)
            snap[fp] = open(fp, 'rb').read() if os.path.exists(fp) else None
        refused = {}
        for (lp, s, cpp, hdr), grp in sorted(groups.items()):
            r = run([sys.executable, 'tools/asm_port/port_batch.py', os.path.relpath(lp, ROOT), ','.join(grp), '--cpp', cpp,
                     '--header', hdr, '--subsystem', s, '--pending', pf])
            refused.update(re.findall(r'^SKIPPED (0x[0-9a-f]{8}): (.*)$', r.stdout, re.M))
            if r.returncode:
                print('  port_batch failed for %s: %s\n  tree restored, stopping' % (cpp, r.stderr[-300:]), flush=True)
                restore(snap)
                return
        if refused:
            print('  %d refused by port_batch: %s' % (len(refused), '; '.join('%s %s' % kv for kv in list(refused.items())[:6])), flush=True)
            # a refused member breaks the closure: drop it and start over from the snapshot
            drop |= {a[2:] for a in refused}
            restore(snap)
            continue
        print('  ' + run([sys.executable, 'tools/gen_eh_frames.py', 'emit']).stdout.strip(), flush=True)
        print('  includes added: %d' % fix_includes(sorted({g[2] for g in groups} | {target(led[h[3:]], eh[h[3:]]['subsystem'])[0] for h in hs})), flush=True)
        run([sys.executable, 'tools/asm_port/gen_data_image.py'])
        ok, errs = build()
        if ok:
            retag(fns)
            st = run([sys.executable, '03_re/scripts/re_status.py']).stdout
            print('  build OK; re_status: %s' % re.search(r'VERDICT: \w+', st).group(0), flush=True)
            print('ported %d functions + %d EH handlers' % (len(fns), len(hs)))
            return
        print('  build failed (%d error lines), e.g.:\n    %s' % (len(errs), '\n    '.join(e.strip()[-200:] for e in errs[:8])), flush=True)
        bad = set()
        for e in errs:
            m = re.match(r'\s*(?:FAILED: )?(.+?\.cpp)\((\d+)\)', e.strip())
            if m:
                fp = m.group(1) if os.path.isabs(m.group(1)) else os.path.join(ROOT, '05_remake', m.group(1))
                if os.path.exists(fp):
                    lines = io.open(fp, encoding='utf-8').read().splitlines()[:int(m.group(2))]
                    for l in reversed(lines):
                        mm = re.match(r'// 0x([0-9a-f]{8}) ', l)
                        if mm:
                            bad.add(mm.group(1))
                            break
            for mm in re.finditer(r'\b(\w+)@@|"int __fastcall recoil::(\w+)\(|recoil::(\w+)', e):
                nm = next(x for x in mm.groups() if x)
                bad |= {a for a in fns if led['0x' + a]['ghidra_name'] == nm}
        restore(snap)
        if not bad:
            print('  could not attribute the errors to functions - stopping (tree restored)')
            return
        print('  dropping %d function(s): %s' % (len(bad), ', '.join(sorted(bad))[:400]), flush=True)
        drop |= {a[3:] if a.startswith('eh:') else a for a in bad}
        with open(SKIPPED, 'a') as f:
            f.write(''.join('0x%s\n' % a for a in sorted(bad) if not a.startswith('eh:')))
    print('gave up after %d tries' % tries)


TAG = 'IMPLEMENTED-UNVERIFIED (ported locally by tools/port_loop.py; no test yet)'  # port_loop's tag: gen_port_loop_tests.py picks these rows


def retag(fns):
    """port_loop's tag on every ported member except EH funclets (they run on the parent's EBP frame: no arena test can call them)."""
    led = {r['address'].lower(): r for r in csv.DictReader(io.open(os.path.join(ROOT, '03_re', 'ledger', 'functions.csv'), encoding='utf-8'))}
    args = []
    for a in fns:
        r = led['0x' + a]
        if r['remake_symbol'] and not r['ghidra_name'].startswith('EH_Unwind_'):
            args += ['0x' + a, r['remake_file'], r['remake_symbol'], TAG]
    for i in range(0, len(args), 400):
        run([sys.executable, '03_re/scripts/remake_set.py'] + args[i:i + 400])


def restore(snap):
    for fp, data in snap.items():
        if data is None:
            if os.path.exists(fp):
                os.remove(fp)
        else:
            open(fp, 'wb').write(data)


if __name__ == '__main__':
    main()
