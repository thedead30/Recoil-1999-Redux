# spec_page.py SUB "title" "intro line" -> 04_spec/systems/SUB.md member table from ledger
import csv,sys
sub,title,intro=sys.argv[1],sys.argv[2],sys.argv[3]
mem=[r[1] for r in csv.reader(open('03_re/ledger/subsystems.csv',encoding='utf-8')) if r and r[0]==sub]
rows={r[0]:r for r in csv.reader(open('03_re/ledger/functions.csv',encoding='utf-8')) if r}
out=[f'# {sub} - {title} (spec)','',intro,'','All rows CONFIRMED-BINARY at the cited address (bytes read; method in ledger).','','| addr | name | notes |','|---|---|---|']
for a in sorted(mem):
    r=rows.get(a,[a,'?']); n=max((c for c in r[7:] if c),key=len,default='')
    out.append(f'| {a} | {r[1]} | {n[:220].replace("|","/")} |')
open(f'04_spec/systems/{sub}.md','w',encoding='utf-8').write('\n'.join(out)+'\n'); print(sub,len(mem))
