"""port_loop.py - unattended bottom-up porting (no tests, no verification). Local only.

usage: python tools/port_loop.py [--rounds N] [--only SUBSYSTEM[,...]] [--no-commit]

Each round: for every 03_re/listings/unported/<subsystem>.txt (net / znetwork skipped - multiplayer is out of scope),
ask port_ready.py which functions have all callees ported, port them with tools/asm_port/port_batch.py into the file
the ledger's orig_file names (05_remake/src/<orig path>.cpp, else src/unattributed/<subsystem>.cpp), regenerate the
data image, build. A subsystem batch that does not compile is reverted and its functions go to
03_re/staging/verify/port_skipped.txt (a port or converter problem for a later look). Every ported function is
recorded "IMPLEMENTED-UNVERIFIED (ported locally by tools/port_loop.py; no test yet)". After each round: re_status
must be CLEAN, then a local git commit. Stops when a round ports nothing.
"""
import collections, csv, glob, io, os, re, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
SKIP_SUBSYS = {'net', 'znetwork', '_missing'}
SKIPPED = os.path.join(ROOT, '03_re', 'staging', 'verify', 'port_skipped.txt')
TAG = 'IMPLEMENTED-UNVERIFIED (ported locally by tools/port_loop.py; no test yet)'


def run(cmd, **kw):
    return subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, errors='replace', **kw)


def ledger():
    return {r['address']: r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))}


def skipped():
    return set(open(SKIPPED).read().split()) if os.path.exists(SKIPPED) else set()


def ready(listing):
    out = run([sys.executable, '03_re/scripts/port_ready.py', listing, '--blockers', '0']).stdout
    return re.findall(r'^\s+(0x[0-9a-f]{8}) ', out, re.M)


def target(row, subsys):
    o = row.get('orig_file', '').replace('\\', '/').strip()
    rel = re.sub(r'\.(c|cpp)$', '', o) if o else 'unattributed/' + subsys
    return '05_remake/src/%s.cpp' % rel, '05_remake/src/%s.h' % rel


def build_ok():
    r = run(['cmd', '/c', os.path.join(ROOT, 'tools', 'build_only.bat')])
    return r.returncode == 0 and ' error ' not in r.stdout, [l for l in r.stdout.splitlines() if ' error ' in l][:5]


def main():
    rounds = int(sys.argv[sys.argv.index('--rounds') + 1]) if '--rounds' in sys.argv else 50
    only = set(sys.argv[sys.argv.index('--only') + 1].split(',')) if '--only' in sys.argv else None
    dirty = run(['git', 'status', '--porcelain', '--untracked-files=no', '--', '05_remake', 'tools', '03_re/ledger', ':!03_re/ledger/work_log.csv']).stdout.strip()
    if dirty:
        sys.exit('uncommitted changes in 05_remake / tools / 03_re/ledger - commit them first:\n' + dirty)
    total = 0
    for rnd in range(1, rounds + 1):
        # ledger and source must agree: clear rows whose code is missing, so they are ported again
        print(run([sys.executable, 'tools/ledger_code_check.py', '--fix']).stdout.strip().splitlines()[-1], flush=True)
        led, skip, ported_round = ledger(), skipped(), []
        for listing in sorted(glob.glob(os.path.join(ROOT, '03_re/listings/unported/*.txt'))):
            s = os.path.basename(listing)[:-4]
            if s in SKIP_SUBSYS or (only and s not in only):
                continue
            # a name two ledger rows share (a misnaming) would be a second definition of the same symbol: refuse it
            names = collections.Counter(r['ghidra_name'] for r in led.values())
            dup = [a for a in ready(listing) if a in led and names[led[a]['ghidra_name']] > 1]
            if dup:
                print('round %d %s: %d refused (name shared with another ledger row: %s)' % (rnd, s, len(dup), ', '.join(led[a]['ghidra_name'] for a in dup)), flush=True)
            addrs = [a for a in ready(listing) if a not in skip and a not in dup]
            if not addrs:
                continue
            groups = {}
            for a in addrs:
                groups.setdefault(target(led[a], s), []).append(a)
            for (cpp, hdr), grp in groups.items():
                snap = {}  # the batch's own files, restored byte for byte on failure (never a tree-wide checkout)
                for f in (cpp, hdr, '03_re/ledger/functions.csv', '05_remake/src/platform/image/original_data.cpp',
                          '05_remake/src/platform/image/original_data.h'):
                    fp = os.path.join(ROOT, f)
                    snap[fp] = open(fp, 'rb').read() if os.path.exists(fp) else None
                r = run([sys.executable, 'tools/asm_port/port_batch.py', os.path.relpath(listing, ROOT), ','.join(grp),
                         '--cpp', cpp, '--header', hdr, '--subsystem', s])
                run([sys.executable, 'tools/asm_port/gen_data_image.py'])
                # port_batch exits 0 even when it skips functions (unnamed FUN_, unresolved references): read its report
                refused = dict(re.findall(r'^SKIPPED (0x[0-9a-f]{8}): (.*)$', r.stdout, re.M))
                if refused:
                    with open(SKIPPED, 'a') as f:
                        f.write('\n'.join(refused) + '\n')
                    print('round %d %s: %d refused by port_batch (%s)' % (rnd, s, len(refused), '; '.join(sorted(set(refused.values())))[:160]), flush=True)
                done = [a for a in grp if a not in refused]
                if not done:
                    continue
                ok, errs = (r.returncode == 0, [r.stderr[-300:]]) if r.returncode else build_ok()
                if ok:
                    ported_round += [(a, cpp) for a in done]
                    print('round %d %s: %d ported -> %s' % (rnd, s, len(done), cpp), flush=True)
                else:
                    # revert this batch: tracked files back to the snapshot, new untracked files removed
                    for fp, data in snap.items():
                        if data is None:
                            if os.path.exists(fp):
                                os.remove(fp)
                        else:
                            open(fp, 'wb').write(data)
                    with open(SKIPPED, 'a') as f:
                        f.write('\n'.join(grp) + '\n')
                    print('round %d %s: %d SKIPPED (%s)' % (rnd, s, len(grp), ' | '.join(e.strip()[-120:] for e in errs)), flush=True)
        if not ported_round:
            break
        led = ledger()
        args = []
        for a, cpp in ported_round:
            row = led[a]
            args += [a, cpp, row['remake_symbol'] or row['ghidra_name'], TAG]
        for i in range(0, len(args), 400):
            run([sys.executable, '03_re/scripts/remake_set.py'] + args[i:i + 400])
        run([sys.executable, '03_re/scripts/roadmap_sync.py'])
        st = run([sys.executable, '03_re/scripts/re_status.py']).stdout
        if 'VERDICT: CLEAN' not in st:
            print('re_status not CLEAN after round %d - stopping without commit' % rnd)
            print('\n'.join(l for l in st.splitlines() if l.strip())[-1500:])
            return
        total += len(ported_round)
        if '--no-commit' not in sys.argv:
            run(['git', 'add', '-A', '05_remake', '03_re', 'tools', 'ROADMAP.md', 'ORIGINAL_DEFECTS.md'])
            # the model that runs the loop: PORT_LOOP_MODEL (work log row) and PORT_LOOP_COAUTHOR (commit trailer)
            model = os.environ.get('PORT_LOOP_MODEL', 'sonnet')
            coauthor = os.environ.get('PORT_LOOP_COAUTHOR', 'Claude Sonnet 5.5 <noreply@anthropic.com>')
            run(['git', 'commit', '-q', '-m', 'Port loop round %d: %d functions ported (IMPLEMENTED-UNVERIFIED, no tests yet)\n\n'
                 'Co-Authored-By: %s' % (rnd, len(ported_round), coauthor)])
            # re_status counts commits without a work_log row as drift (tools/worklog.py): log this one now, so the next round
            # is CLEAN; the row itself goes in with the next round's commit
            run([sys.executable, 'tools/worklog.py', 'add', model, 'port_loop round %d: %d functions ported' % (rnd, len(ported_round)),
                 '--item', 'B. port_loop'])
        print('round %d done: %d ported (total %d)' % (rnd, len(ported_round), total), flush=True)
    print('port loop finished: %d functions ported' % total)


if __name__ == '__main__':
    main()
