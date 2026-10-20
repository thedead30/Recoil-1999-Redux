"""llm_pipeline.py - unattended verification helpers driven by the local LLM (03_re/scripts/local_agent.py endpoint).

usage: python tools/llm_pipeline.py mutants [--limit N]   # LLM proposes 2 mutants per PASS function -> mutants.json
       python tools/llm_pipeline.py mutate  [--limit N]   # apply each mutant, rebuild, run its test -> mutation_results.csv
       python tools/llm_pipeline.py triage  [--limit N]   # LLM reads each failing test + log -> staging/triage/<test>.md
       python tools/llm_pipeline.py report                # counts, and the functions ready to promote

Inputs: 03_re/staging/verify/verify_results.csv and verify_run.log (tools/verify_batch.py). Every stage is resumable (it
skips work already recorded) and writes only under 03_re/staging/ and tools/asm_port/mutants.json; source files are
touched only by `mutate`, which always restores them from a backup. Nothing here edits the ledger: a function whose
test passed and whose mutants were all killed is listed by `report` for promotion with remake_set.py.
The model is never trusted: a proposed mutant is kept only if its regex matches exactly once inside the function's
own body and changes it; triage notes are advice for a reviewer, never applied automatically.
"""
import csv, io, json, os, re, shutil, subprocess, sys, time, urllib.request

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, '03_re', 'scripts'))
import local_agent as la  # endpoint, model and key only

ST = os.path.join(ROOT, '03_re', 'staging', 'verify')
TAG = os.environ.get('LLM_TAG', '')  # a model comparison writes to its own files: mutants_<tag>.json etc.
MUT = os.path.join(ROOT, 'tools', 'asm_port', 'mutants%s.json' % ('_' + TAG if TAG else ''))
MRES = os.path.join(ST, 'mutation_results%s.csv' % ('_' + TAG if TAG else ''))
SYS_MUT = ("/no_think You write mutation tests for x86 MSVC inline-asm ports. Given one function's __asm body, "
           "propose exactly 2 mutants that change behaviour a test should notice: invert a conditional jump, change "
           "a compared constant or flag mask, drop a store, or change a memory offset. Reply ONLY with JSON: "
           '[{"regex": "...", "replacement": "...", "reason": "..."}, ...]. The regex is a Python regex that must '
           "match exactly one place in the body; escape brackets and plus signs; keep it short (one instruction).")
SYS_TRI = ("/no_think You triage failing native tests of a game remake. The test compares a ported function with "
           "the original binary. Given the test source and its log lines, say whether the failure is most likely a "
           "TEST bug (buffer too small, bad setup, invalid input the game never produces, uninitialised memory), a "
           "PORT bug, or HARNESS/flaky. Reply in under 250 words: first line 'CAUSE: TEST|PORT|HARNESS', then the "
           "evidence (quote log lines and test lines), then the smallest fix. Never invent addresses or offsets.")


THINK = '--think' in sys.argv  # reasoning mode: slower, for triage / naming / test writing


def model(system, user, max_tokens=1500):
    if THINK:
        system = system.replace('/no_think ', '')
        max_tokens = max(max_tokens * 6, 16000)  # room for the reasoning before the answer
    body = json.dumps({"model": la.MODEL, "temperature": 0.6 if THINK else 0.1, "max_tokens": max_tokens,
                       "chat_template_kwargs": {"enable_thinking": THINK},
                       "messages": [{"role": "system", "content": system}, {"role": "user", "content": user}]}).encode()
    req = urllib.request.Request(la.ENDPOINT, data=body, headers={"Content-Type": "application/json",
                                                                  "Authorization": "Bearer " + la.API_KEY})
    with urllib.request.urlopen(req, timeout=1800) as r:
        msg = json.loads(r.read().decode())["choices"][0]["message"]
    text = msg.get("content") or ""
    return re.sub(r"<think>.*?</think>", "", text, flags=re.S).strip()  # reasoning never reaches the parsers


def results():
    return list(csv.DictReader(open(os.path.join(ST, 'verify_results.csv'), encoding='utf-8')))


def ledger():
    return {r['address']: r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))}


def body_span(text, name):
    m = re.search(r'__fastcall %s\(int' % re.escape(name), text)
    if not m:
        return None
    e = text.find('\n}\n', m.start())
    return (m.start(), e) if e > 0 else None


def src_path(row):
    f = row['remake_file'].replace('\\', '/')
    return os.path.join(ROOT, f if f.startswith('05_remake') else os.path.join('05_remake', f))


def load_json(p, default):
    return json.load(open(p, encoding='utf-8')) if os.path.exists(p) else default


def stage_mutants(limit):
    led, muts = ledger(), load_json(MUT, {})
    todo = [r for r in results() if r['result'] == 'PASS' and r['name'] not in muts][:limit]
    # a tagged round (LLM_TAG=round2 ...) asks for mutants that DIFFER from the ones already in the main mutants.json
    base = load_json(os.path.join(ROOT, 'tools', 'asm_port', 'mutants.json'), {}) if TAG else {}
    for i, r in enumerate(todo):
        row = led[r['address']]
        path = src_path(row)
        text = io.open(path, encoding='utf-8').read()
        sp = body_span(text, r['name'])
        if not sp:
            muts[r['name']] = []
            continue
        body = text[sp[0]:sp[1]]
        kept = []
        for attempt in range(2):
            try:
                prior = [m['regex'] for m in base.get(r['name'], [])]
                reply = model(SYS_MUT, body[:24000] + ('\n\nAlready tested (propose DIFFERENT places): ' + json.dumps(prior) if prior else ''))
                props = json.loads(re.search(r'\[.*\]', reply, re.S).group(0))
            except Exception:
                continue
            for p in props:
                try:
                    hits = list(re.finditer(p['regex'], body))
                    changed = len(hits) == 1 and re.sub(p['regex'], lambda _m: str(p['replacement']), body, count=1) != body
                except (re.error, KeyError, TypeError):
                    continue
                if changed:
                    kept.append({'file': os.path.relpath(path, ROOT).replace('\\', '/'), 'regex': p['regex'],
                                 'replacement': p['replacement'], 'reason': str(p.get('reason', ''))[:200]})
            if len(kept) >= 2:
                break
        muts[r['name']] = kept[:2]
        json.dump(muts, open(MUT, 'w', encoding='utf-8'), indent=1)
        print('[%d/%d] %s: %d mutant(s)' % (i + 1, len(todo), r['name'], len(kept[:2])), flush=True)


def build():
    r = subprocess.run(['cmd', '/c', os.path.join(ROOT, 'tools', 'build_only.bat')], capture_output=True, text=True, errors='replace')
    return r.returncode == 0 and ' error ' not in r.stdout


def run_test(tests, fn):
    env = dict(os.environ, RECOIL_ONLY=fn) if fn else dict(os.environ)
    exe = os.path.join(ROOT, '05_remake', 'build', 'Release', 'recoil_tests.exe')
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'run_tests.py'), exe] + tests + ['--timeout', '180'], env=env,  # a mutant that hangs counts as killed after 3 min, not 15
                       cwd=os.path.join(ROOT, '05_remake'), capture_output=True, text=True, errors='replace')
    m = re.search(r'(\d+) failed jobs', r.stdout)
    return (int(m.group(1)) if m else 1) > 0, r.stdout


def stage_mutate(limit):
    muts = load_json(MUT, {})
    done = {(x['name'], x['regex']) for x in csv.DictReader(open(MRES, encoding='utf-8'))} if os.path.exists(MRES) else set()
    tests = {r['name']: r['test'].split(';') for r in results()}
    new = not os.path.exists(MRES)
    f = open(MRES, 'a', encoding='utf-8', newline='')
    w = csv.DictWriter(f, fieldnames=['name', 'regex', 'replacement', 'outcome'], lineterminator='\n')
    if new:
        w.writeheader()
    jobs = [(n, m) for n, ms in muts.items() for m in ms if (n, m['regex']) not in done][:limit]
    for i, (n, m) in enumerate(jobs):
        path = os.path.join(ROOT, m['file'])
        bak = path + '.mutbak'
        shutil.copyfile(path, bak)
        try:
            text = io.open(path, encoding='utf-8').read()
            sp = body_span(text, n)
            body = text[sp[0]:sp[1]]
            io.open(path, 'w', encoding='utf-8', newline='').write(text[:sp[0]] + re.sub(m['regex'], lambda _m: m['replacement'], body, count=1) + text[sp[1]:])
            if not build():
                outcome = 'NO-BUILD'
            else:
                failed, _ = run_test(tests[n], n)
                outcome = 'KILLED' if failed else 'SURVIVED'
        finally:
            shutil.move(bak, path)
            os.utime(path, None)
        w.writerow({'name': n, 'regex': m['regex'], 'replacement': m['replacement'], 'outcome': outcome})
        f.flush()
        print('[%d/%d] %s %s' % (i + 1, len(jobs), n, outcome), flush=True)
    f.close()
    build()


def stage_triage(limit):
    log = io.open(os.path.join(ST, 'verify_run.log'), encoding='utf-8', errors='replace').read()
    out = os.path.join(ST, 'triage')
    os.makedirs(out, exist_ok=True)
    failing = sorted({t for r in results() if r['result'] == 'FAIL' for t in r['test'].split(';')
                      if re.search(r'^(FAIL %s|FAILED JOB %s)' % (t, t), log, re.M)})
    todo = [t for t in failing if not os.path.exists(os.path.join(out, t + '.md'))][:limit]
    for i, t in enumerate(todo):
        blocks = [b for b in re.split(r'\n(?=run  )', log) if b.startswith('run  ' + t)]
        lines = '\n'.join(l for b in blocks for l in b.splitlines() if re.search(r'FAIL|DIFF|crash|ESCAPED|error', l))[:6000]
        src = ''
        for p in __import__('glob').glob(os.path.join(ROOT, '05_remake', 'tests', '*.cpp')):
            s = io.open(p, encoding='utf-8', errors='replace').read()
            k = s.find('TEST(%s)' % t)
            if k >= 0:
                src = os.path.basename(p) + '\n' + s[:1500] + '\n...\n' + s[max(0, k - 4000):k + 9000]
                break
        reply = model(SYS_TRI, 'TEST: %s\n\nLOG LINES:\n%s\n\nTEST SOURCE:\n%s' % (t, lines, src[:20000]), 900)
        io.open(os.path.join(out, t + '.md'), 'w', encoding='utf-8').write('# %s\n\n%s\n\n## log lines\n```\n%s\n```\n' % (t, reply, lines[:3000]))
        print('[%d/%d] %s: %s' % (i + 1, len(todo), t, (reply.splitlines() or ['?'])[0][:60]), flush=True)


SYS_FIX = ("/no_think You fix a failing native test file of a game remake. A triage note says the TEST (not the port) "
           "is wrong. Reply ONLY with JSON: [{\"search\": \"exact text from the file\", \"replace\": \"new text\"}] - "
           "at most 4 edits, each search copied verbatim from the file and unique in it. Fix only the test (buffer "
           "sizes, setup, invalid inputs, uninitialised memory); never weaken a comparison or delete a CHECK.")


def stage_reset_nobuild(_limit):
    """Forget mutants that did not build, so `mutants` proposes new ones and `mutate` runs them."""
    muts = load_json(MUT, {})
    rows = list(csv.DictReader(open(MRES, encoding='utf-8'))) if os.path.exists(MRES) else []
    bad = {x['name'] for x in rows if x['outcome'] == 'NO-BUILD'}
    for n in bad:
        muts.pop(n, None)
    json.dump(muts, open(MUT, 'w', encoding='utf-8'), indent=1)
    with open(MRES, 'w', encoding='utf-8', newline='') as f:
        w = csv.DictWriter(f, fieldnames=['name', 'regex', 'replacement', 'outcome'], lineterminator='\n')
        w.writeheader()
        w.writerows(x for x in rows if x['name'] not in bad)
    print('reset %d function(s) with NO-BUILD mutants' % len(bad))


def stage_testfix(limit):
    """For triage notes saying CAUSE: TEST: model proposes edits; keep them only if the test then passes."""
    import glob
    out = os.path.join(ST, 'testfix%s.csv' % ('_' + TAG if TAG else ''))
    done = {x['test'] for x in csv.DictReader(open(out, encoding='utf-8'))} if os.path.exists(out) else set()
    notes = [p for p in sorted(glob.glob(os.path.join(ST, 'triage', '*.md')))
             if 'CAUSE: TEST' in io.open(p, encoding='utf-8').read()[:400]]
    todo = [p for p in notes if os.path.basename(p)[:-3] not in done][:limit]
    new = not os.path.exists(out)
    f = open(out, 'a', encoding='utf-8', newline='')
    w = csv.DictWriter(f, fieldnames=['test', 'file', 'outcome', 'edits'], lineterminator='\n')
    if new:
        w.writeheader()
    for i, note in enumerate(todo):
        test = os.path.basename(note)[:-3]
        path = next((p for p in glob.glob(os.path.join(ROOT, '05_remake', 'tests', '*.cpp'))
                     if 'TEST(%s)' % test in io.open(p, encoding='utf-8', errors='replace').read()), None)
        outcome, edits = 'NO-FILE', 0
        if path:
            src = io.open(path, encoding='utf-8').read()
            k = src.find('TEST(%s)' % test)
            ctx = src[:1500] + '\n...\n' + src[max(0, k - 6000):k + 10000]
            try:
                props = json.loads(re.search(r'\[.*\]', model(SYS_FIX, 'TRIAGE NOTE:\n%s\n\nTEST FILE %s:\n%s' % (
                    io.open(note, encoding='utf-8').read()[:4000], os.path.basename(path), ctx), 2000), re.S).group(0))
            except Exception:
                props = []
            new_src = src
            for e in props[:4]:
                try:
                    if new_src.count(e['search']) == 1 and e['search'] != e['replace'] and 'CHECK' not in e['search'].replace(e['replace'], ''):
                        new_src = new_src.replace(e['search'], e['replace'])
                        edits += 1
                except (KeyError, TypeError, AttributeError):
                    pass
            # never accept a "fix" that removes what is tested: fewer function-table entries ({0x...}) or calls
            # commented out / tables emptied (Coder-Next 2026-10-06 passed 15 tests by deleting their entries)
            ent = lambda t: len(re.findall(r'^\s*\{0x[0-9a-fA-F]+', t, re.M))
            if edits and (ent(new_src) < ent(src) or new_src.replace(' ', '').count('return{};') > src.replace(' ', '').count('return{};')
                          or new_src.count('continue;') > src.count('continue;')):
                edits = 0; outcome = 'REJECTED-WEAKENS'
            elif edits == 0:
                outcome = 'NO-EDIT'
            if edits:
                bak = path + '.fixbak'
                shutil.copyfile(path, bak)
                io.open(path, 'w', encoding='utf-8', newline='').write(new_src)
                if not build():
                    outcome = 'NO-BUILD'
                else:
                    failed, _ = run_test([test], '')
                    outcome = 'FAILS' if failed else 'FIXED'
                if outcome == 'FIXED':
                    os.remove(bak)
                else:
                    shutil.move(bak, path)
                    os.utime(path, None)
        w.writerow({'test': test, 'file': os.path.basename(path or ''), 'outcome': outcome, 'edits': edits})
        f.flush()
        print('[%d/%d] %s %s (%d edits)' % (i + 1, len(todo), test, outcome, edits), flush=True)
    f.close()
    build()


SYS_STR = ("/no_think A mutation of a ported x86 function SURVIVED its native test (the test still passed with the bug "
           "planted). Either strengthen the test so the mutant is detected, or, if the mutation cannot change "
           "behaviour, say so. Reply ONLY with JSON: {\"equivalent\": false, \"edits\": [{\"search\": \"exact unique "
           "text from the test file\", \"replace\": \"new text\"}]} (at most 4 edits: add inputs that reach the "
           "mutated instruction, compare more state) or {\"equivalent\": true, \"reason\": \"...\"}. Never remove or "
           "weaken a CHECK.")


def apply_mutant(n, m, on):
    path = os.path.join(ROOT, m['file'])
    if on:
        shutil.copyfile(path, path + '.mutbak')
        text = io.open(path, encoding='utf-8').read()
        sp = body_span(text, n)
        io.open(path, 'w', encoding='utf-8', newline='').write(text[:sp[0]] + re.sub(m['regex'], lambda _m: m['replacement'], text[sp[0]:sp[1]], count=1) + text[sp[1]:])
    elif os.path.exists(path + '.mutbak'):
        shutil.move(path + '.mutbak', path)
        os.utime(path, None)


def stage_strengthen(limit):
    import glob
    out = os.path.join(ST, 'strengthen.csv')
    done = {(x['name'], x['regex']) for x in csv.DictReader(open(out, encoding='utf-8'))} if os.path.exists(out) else set()
    muts = load_json(MUT, {})
    tests = {r['name']: r['test'].split(';') for r in results()}
    surv = [x for x in csv.DictReader(open(MRES, encoding='utf-8')) if x['outcome'] == 'SURVIVED' and (x['name'], x['regex']) not in done]
    new = not os.path.exists(out)
    f = open(out, 'a', encoding='utf-8', newline='')
    w = csv.DictWriter(f, fieldnames=['name', 'regex', 'outcome', 'note'], lineterminator='\n')
    if new:
        w.writeheader()
    for i, x in enumerate(surv[:limit]):
        n = x['name']
        m = next((mm for mm in muts.get(n, []) if mm['regex'] == x['regex']), None)
        ts = tests.get(n, [])
        path = next((p for p in glob.glob(os.path.join(ROOT, '05_remake', 'tests', '*.cpp')) if ts and 'TEST(%s)' % ts[0] in io.open(p, encoding='utf-8', errors='replace').read()), None)
        outcome, note = 'SKIP', ''
        if m and path:
            src = io.open(path, encoding='utf-8').read()
            k = src.find('TEST(%s)' % ts[0])
            ptext = io.open(os.path.join(ROOT, m['file']), encoding='utf-8').read()
            sp = body_span(ptext, n)
            try:
                reply = model(SYS_STR, 'FUNCTION %s PORT:\n%s\n\nMUTANT: replace /%s/ with "%s" (%s)\n\nTEST FILE %s:\n%s' % (
                    n, ptext[sp[0]:sp[1]][:8000], m['regex'], m['replacement'], m['reason'], os.path.basename(path),
                    src[:1500] + '\n...\n' + src[max(0, k - 5000):k + 9000]), 2000)
                d = json.loads(re.search(r'\{.*\}', reply, re.S).group(0))
            except Exception:
                d = {}
            if d.get('equivalent'):
                outcome, note = 'CLAIMS-EQUIVALENT', str(d.get('reason', ''))[:300]
            else:
                new_src, edits = src, 0
                for e in (d.get('edits') or [])[:4]:
                    try:
                        if new_src.count(e['search']) == 1 and 'CHECK' not in e['search'].replace(e['replace'], ''):
                            new_src = new_src.replace(e['search'], e['replace']); edits += 1
                    except (KeyError, TypeError, AttributeError):
                        pass
                if not edits:
                    outcome = 'NO-EDIT'
                else:
                    shutil.copyfile(path, path + '.strbak')
                    io.open(path, 'w', encoding='utf-8', newline='').write(new_src)
                    ok = build() and not run_test(ts, n)[0]
                    killed = False
                    if ok:
                        apply_mutant(n, m, True)
                        try:
                            killed = build() and run_test(ts, n)[0]
                        finally:
                            apply_mutant(n, m, False)
                    outcome = 'STRENGTHENED' if ok and killed else ('STILL-SURVIVES' if ok else 'BREAKS-TEST')
                    if outcome == 'STRENGTHENED':
                        os.remove(path + '.strbak')
                    else:
                        shutil.move(path + '.strbak', path); os.utime(path, None)
        w.writerow({'name': n, 'regex': x['regex'], 'outcome': outcome, 'note': note})
        f.flush()
        print('[%d/%d] %s %s' % (i + 1, min(limit, len(surv)), n, outcome), flush=True)
    f.close()
    build()


SYS_NAME = ("/no_think You name functions of a 1999 game (Recoil, Zipper engine). Given one function's x86 listing and "
            "its ledger note, reply with ONE line: <Subsystem_VerbNoun name> | <one-sentence reason quoting the "
            "instructions that justify it>. Use names consistent with the neighbours given. If unsure, prefix '?'.")


def stage_names(limit):
    import glob
    out = os.path.join(ST, 'name_proposals.csv')
    done = {x['address'] for x in csv.DictReader(open(out, encoding='utf-8'))} if os.path.exists(out) else set()
    L = {}
    for p in glob.glob(os.path.join(ROOT, '03_re', 'listings', '**', '*.txt'), recursive=True):
        for b in io.open(p, encoding='utf-8', errors='replace').read().split('### ')[1:]:
            L.setdefault('0x' + b[:8], b)
    led = list(csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8')))
    named = [r['ghidra_name'] for r in led if not r['ghidra_name'].startswith('FUN_')]
    todo = [r for r in led if r['ghidra_name'].startswith('FUN_') and r['address'] in L and r['address'] not in done][:limit]
    new = not os.path.exists(out)
    f = open(out, 'a', encoding='utf-8', newline='')
    w = csv.DictWriter(f, fieldnames=['address', 'proposal', 'reason'], lineterminator='\n')
    if new:
        w.writeheader()
    for i, r in enumerate(todo):
        a = int(r['address'], 16)
        near = [n for n in named][:0]
        neigh = ', '.join(x['ghidra_name'] for x in led if not x['ghidra_name'].startswith('FUN_') and abs(int(x['address'], 16) - a) < 0x400)[:600]
        try:
            reply = model(SYS_NAME, 'NEIGHBOUR NAMES: %s\nLEDGER NOTE: %s\nLISTING:\n%s' % (neigh, r['notes'][:1500], L[r['address']][:12000]), 200).strip().splitlines()[0]
        except Exception:
            continue
        name, _, why = reply.partition('|')
        w.writerow({'address': r['address'], 'proposal': name.strip()[:80], 'reason': why.strip()[:300]})
        f.flush()
        print('[%d/%d] %s -> %s' % (i + 1, len(todo), r['address'], name.strip()[:60]), flush=True)
    f.close()


def stage_report():
    rs = results()
    muts = load_json(MUT, {})
    mres = list(csv.DictReader(open(MRES, encoding='utf-8'))) if os.path.exists(MRES) else []
    by = {}
    for x in mres:
        by.setdefault(x['name'], []).append(x['outcome'])
    ready = [r for r in rs if r['result'] == 'PASS' and by.get(r['name']) and all(o == 'KILLED' for o in by[r['name']])]
    surv = sorted(n for n, os_ in by.items() if 'SURVIVED' in os_)
    print('verify: PASS %d FAIL %d | mutants proposed for %d | runs %d (killed %d, survived %d, no-build %d)' % (
        sum(r['result'] == 'PASS' for r in rs), sum(r['result'] == 'FAIL' for r in rs), sum(1 for v in muts.values() if v),
        len(mres), sum(x['outcome'] == 'KILLED' for x in mres), sum(x['outcome'] == 'SURVIVED' for x in mres),
        sum(x['outcome'] == 'NO-BUILD' for x in mres)))
    io.open(os.path.join(ST, 'ready_to_promote.txt'), 'w', encoding='utf-8').write(
        '\n'.join('%s %s %s' % (r['address'], r['name'], r['test']) for r in ready) + '\n')
    print('ready to promote: %d (03_re/staging/verify/ready_to_promote.txt); functions with a surviving mutant: %d' % (len(ready), len(surv)))


if __name__ == '__main__':
    st = sys.argv[1] if len(sys.argv) > 1 else 'report'
    lim = int(sys.argv[sys.argv.index('--limit') + 1]) if '--limit' in sys.argv else 10 ** 9
    {'mutants': stage_mutants, 'mutate': stage_mutate, 'triage': stage_triage, 'reset-nobuild': stage_reset_nobuild, 'testfix': stage_testfix, 'strengthen': stage_strengthen, 'names': stage_names}.get(st, lambda _: stage_report())(lim)
    if st != 'report':
        stage_report()
