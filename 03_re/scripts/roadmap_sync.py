# roadmap_sync.py - rewrites ROADMAP.md "Where things stand" table from re_status.py output.
# Every number comes from re_status.py; a line re_status no longer prints reads as 0.
import subprocess, re, datetime, sys

out = subprocess.run([sys.executable, '03_re/scripts/re_status.py'], capture_output=True, text=True).stdout


def g(p, default='0'):
    m = re.search(p, out, re.M)
    return m.group(1) if m else default


conf = g(r'CONFIRMED\s+(\d+ / \d+\s+[\d.]+%)', '?')
rows = re.findall(r'^\s{2}(\w+)\s+(\d+) / (\d+)\s+confirmed\s+(OPEN|CLOSED)[ \t]*(.*)$', out, re.M)
opn = sum(r[3] == 'OPEN' for r in rows)
closed = [f"{r[0]} ({r[1]}/{r[2]}; {r[4].strip()})" for r in rows if r[3] == 'CLOSED']
phases = re.findall(r'^\s{2}(P\d)\s+(\d+)\s+(\d+)\s+(\d+)\s*$', out, re.M)
p_txt = '; '.join(f"{p} {v}/{n} verified ({t} translated)" for p, n, t, v in phases) or 'none'
stage = '**2 - translation**' if int(g(r'engine funcs not CONFIRMED\s+(\d+)', '1')) == 0 else '**1 - reverse engineering**'
t = f"""| | |
|---|---|
| Stage | {stage} |
| Engine functions CONFIRMED | **{conf}** |
| Subsystem gates | **{opn} / {len(rows)} OPEN** |
| Closed gates | {'; '.join(closed) or 'none'} |
| Stage 2 progress | {p_txt} |
| Known gaps open | {g(r'known gaps open\s+(\d+)')} (`03_re/ledger/known_gaps.csv`) |
| Remake source | {g(r'remake source\s+\((.*?)\)', '?')} |
| Drift verdict | {g(r'VERDICT:\s+(\w+)', '?')} |
"""
s = open('ROADMAP.md', encoding='utf-8').read()
s = re.sub(r'Last updated: .*', f'Last updated: {datetime.date.today()} (auto: roadmap_sync.py)', s, 1)
s = re.sub(r'(## Where things stand\n\n)(?:\|[^\n]*\n)+', lambda m: m.group(1) + t, s, 1)
open('ROADMAP.md', 'w', encoding='utf-8').write(s)
print('ROADMAP synced:', conf, f'{opn}/{len(rows)} open;', p_txt)

# the defect log of the original game, rendered from known_gaps.csv (kind original-defect)
import os, subprocess, sys
subprocess.run([sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)), 'original_defects.py')], check=True)
