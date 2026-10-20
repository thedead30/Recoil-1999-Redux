"""worklog.py - the work log: one row per commit, saying which model made it, what it did and which files it touched.

Every commit since the baseline tag `pre-sonnet` must have a row in 03_re/ledger/work_log.csv (re_status.py counts the
commits without one as a drift tripwire). The file list comes from git, never from the model, so the log cannot
claim less than was touched.

usage:
  python tools/worklog.py add MODEL "what was done" [--item "checklist item"]   # logs HEAD (run right after committing)
  python tools/worklog.py missing                                              # commits since pre-sonnet with no row
  python tools/worklog.py show [--model sonnet] [--since REV]                  # rows, newest last, with their files
  python tools/worklog.py files [--model sonnet] [--since REV]                 # every file a model touched
Rolling back a model's work: `show --model sonnet` lists its commits; `git revert <hash>` undoes one (newest first).
"""
import argparse, csv, datetime, os, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
LOG = os.path.join(ROOT, '03_re', 'ledger', 'work_log.csv')
BASE = 'pre-sonnet'
FIELDS = ['when', 'commit', 'model', 'item', 'summary', 'files']


def git(*a):
    return subprocess.run(['git'] + list(a), cwd=ROOT, capture_output=True, text=True, errors='replace').stdout.strip()


def rows():
    return list(csv.DictReader(open(LOG, encoding='utf-8'))) if os.path.exists(LOG) else []


def commits_since(rev):
    out = git('rev-list', '--reverse', '%s..HEAD' % rev)
    return out.split() if out else []


def missing():
    have = {r['commit'] for r in rows()}
    if not git('rev-parse', '--verify', '--quiet', BASE):
        return []
    # the commit that only adds a work_log row for the previous one is part of that entry
    return [c for c in commits_since(BASE) if c[:10] not in have
            and git('show', '--name-only', '--format=', c).splitlines() != ['03_re/ledger/work_log.csv']]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['add', 'missing', 'show', 'files'])
    ap.add_argument('model', nargs='?')
    ap.add_argument('summary', nargs='?')
    ap.add_argument('--item', default='')
    ap.add_argument('--since', default=BASE)
    ap.add_argument('--commit', default='HEAD')
    a = ap.parse_args()
    if a.cmd == 'add':
        if not a.model or not a.summary:
            sys.exit('usage: worklog.py add MODEL "summary" [--item ITEM]')
        h = git('rev-parse', a.commit)[:10]
        if any(r['commit'] == h for r in rows()):
            sys.exit('commit %s is already logged' % h)
        files = git('show', '--name-only', '--format=', h).splitlines()
        new = not os.path.exists(LOG)
        with open(LOG, 'a', encoding='utf-8', newline='') as f:
            w = csv.DictWriter(f, fieldnames=FIELDS, lineterminator='\n')
            if new:
                w.writeheader()
            w.writerow({'when': datetime.datetime.now().strftime('%Y-%m-%d %H:%M'), 'commit': h, 'model': a.model.lower(),
                        'item': a.item, 'summary': a.summary, 'files': ';'.join(files)})
        print('logged %s (%d files) - commit 03_re/ledger/work_log.csv with the next commit' % (h, len(files)))
        return
    if a.cmd == 'missing':
        m = missing()
        for c in m:
            print(c[:10], git('log', '-1', '--format=%s', c)[:100])
        print('%d commit(s) since %s without a work_log row' % (len(m), BASE))
        return
    sel = [r for r in rows() if not a.model or r['model'] == a.model.lower()]
    since = set(c[:10] for c in commits_since(a.since)) if git('rev-parse', '--verify', '--quiet', a.since) else None
    if since is not None:
        sel = [r for r in sel if r['commit'] in since]
    if a.cmd == 'show':
        for r in sel:
            print('%s %s %-7s %s%s' % (r['when'], r['commit'], r['model'], r['summary'], ('  [' + r['item'] + ']') if r['item'] else ''))
            for f in r['files'].split(';'):
                print('      ' + f)
    else:
        for f in sorted({f for r in sel for f in r['files'].split(';') if f}):
            print(f)


if __name__ == '__main__':
    main()
