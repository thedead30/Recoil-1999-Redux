"""promote_ready.py - record the functions in 03_re/staging/verify/ready_to_promote.txt as VERIFIED-ORACLE.

usage: python tools/promote_ready.py

ready_to_promote.txt is written by `python tools/llm_pipeline.py report`: functions whose native test passed and whose
every mutant was killed. Rows already VERIFIED are skipped. Uses 03_re/scripts/remake_set.py, so the ledger rules
(CLAUDE.md tag at the start of the text) are checked there.
"""
import csv, os, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
led = {r['address']: r for r in csv.DictReader(open(os.path.join(ROOT, '03_re/ledger/functions.csv'), encoding='utf-8'))}
args = []
for line in open(os.path.join(ROOT, '03_re/staging/verify/ready_to_promote.txt')):
    if not line.strip():
        continue
    a, name, tests = line.split()
    r = led[a]
    if r['remake_verification'].startswith('VERIFIED'):
        continue
    f = r['remake_file'] if r['remake_file'].startswith('05_remake') else '05_remake/' + r['remake_file']
    args += [a, f, r['remake_symbol'] or name,
             'VERIFIED-ORACLE (native L1 %s passed; local-LLM mutants all killed - tools/asm_port/mutants.json, '
             '03_re/staging/verify/mutation_results.csv)' % tests.replace(';', ', ')]
if not args:
    sys.exit('nothing new to promote')
for i in range(0, len(args), 400):
    print(subprocess.run([sys.executable, os.path.join(ROOT, '03_re/scripts/remake_set.py')] + args[i:i + 400],
                         capture_output=True, text=True).stdout.strip())
