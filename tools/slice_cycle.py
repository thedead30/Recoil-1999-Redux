"""slice_cycle.py - run the mutate stage in slices and port in between (about 0 tokens, safe to leave running).

The mutate stage owns the source tree (it plants each mutant in 05_remake/src, rebuilds build/Release and reruns the tests), so
port_loop and every source edit must wait for it. This alternates the two:
  1. python tools/llm_pipeline.py mutate --limit N        (N pending mutants; resumable, skips those already run)
  2. commit tools/asm_port/mutants.json + 03_re/staging   (the pipeline's own outputs; never while it runs)
  3. python tools/promote_ready.py                        (test PASS + every mutant killed -> VERIFIED-ORACLE) and commit
  4. python tools/port_loop.py                            (bottom-up ports; commits and logs its own rounds)
  ... repeated --cycles times. Every commit gets a row in 03_re/ledger/work_log.csv (tools/worklog.py).
It stops, without touching anything, if the tree is not clean after a step: a mutate run that was killed can leave a mutant in
05_remake/src, and that must be looked at by a person (git diff 05_remake/src), never auto-reverted here.
usage: python tools/slice_cycle.py [--slice 100] [--cycles 3] [--model sonnet] [--dry]
"""
import argparse, os, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
PIPELINE_OWNED = ('tools/asm_port/mutants.json',)


def sh(args, check=False):
    print('$ ' + ' '.join(args), flush=True)
    r = subprocess.run(args, cwd=ROOT, text=True, errors='replace')
    if check and r.returncode:
        sys.exit('step failed (%d): %s' % (r.returncode, ' '.join(args)))
    return r.returncode


def out(args):
    return subprocess.run(args, cwd=ROOT, capture_output=True, text=True, errors='replace').stdout


def dirty_source():
    """Tracked changes in the code or the ledger other than the pipeline's outputs and the work log."""
    lines = out(['git', 'status', '--porcelain', '--untracked-files=no', '--', '05_remake', 'tools', '03_re/ledger',
                 ':!03_re/ledger/work_log.csv']).splitlines()
    return [l for l in lines if l[3:] not in PIPELINE_OWNED]


def commit(paths, msg, model, note, item):
    sh(['git', 'add'] + paths)
    if subprocess.run(['git', 'diff', '--cached', '--quiet'], cwd=ROOT).returncode == 0:
        return False
    sh(['git', 'commit', '-q', '-m', msg + '\n\nCo-Authored-By: ' + os.environ.get('SLICE_COAUTHOR', 'Claude Sonnet 5.5 <noreply@anthropic.com>')], check=True)
    sh([sys.executable, 'tools/worklog.py', 'add', model, note, '--item', item])
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--slice', type=int, default=100)
    ap.add_argument('--cycles', type=int, default=3)
    ap.add_argument('--model', default='sonnet')
    ap.add_argument('--dry', action='store_true')
    ap.add_argument('--no-port', action='store_true', help='skip port_loop (porting runs elsewhere, e.g. tools/port_closure.py in another worktree)')
    a = ap.parse_args()
    for k in range(1, a.cycles + 1):
        print('=== cycle %d of %d ===' % (k, a.cycles), flush=True)
        if a.dry:
            for s in ('llm_pipeline.py mutate --limit %d' % a.slice, 'commit mutants.json + staging', 'promote_ready.py + commit', 'port_loop.py'):
                print('would run:', s)
            continue
        bad = dirty_source()
        if bad:
            sys.exit('tree not clean before cycle %d - look at it first (a killed mutate run can leave a mutant):\n%s' % (k, '\n'.join(bad)))
        env_tag = os.environ.get('LOCAL_LLM_MODEL')
        if not env_tag:
            os.environ['LOCAL_LLM_MODEL'] = 'unsloth/Qwen3-Coder-Next-GGUF'
        sh([sys.executable, 'tools/llm_pipeline.py', 'mutate', '--limit', str(a.slice)])
        bad = dirty_source()
        if bad:
            sys.exit('mutate left changes in the tree - NOT reverting; inspect with git diff 05_remake/src:\n%s' % '\n'.join(bad))
        commit(['tools/asm_port/mutants.json', '03_re/staging'], 'Mutation slice %d (%d mutants max): results and report' % (k, a.slice),
               a.model, 'slice_cycle %d: mutate slice of %d, outputs committed' % (k, a.slice), 'B. slice_cycle')
        sh([sys.executable, 'tools/promote_ready.py'])
        sh([sys.executable, '03_re/scripts/re_status.py'])
        commit(['03_re/ledger', 'ROADMAP.md'], 'Promote pipeline round (slice %d): functions VERIFIED-ORACLE' % k,
               a.model, 'slice_cycle %d: promote_ready' % k, 'B. slice_cycle')
        if a.no_port:
            continue
        bad = dirty_source()
        if bad:
            sys.exit('tree not clean before port_loop:\n%s' % '\n'.join(bad))
        sh([sys.executable, 'tools/port_loop.py'])
    print('slice_cycle finished %d cycle(s)' % a.cycles)


if __name__ == '__main__':
    main()
