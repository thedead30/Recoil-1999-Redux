# unowned_modules.py - group unowned engine functions by source-path module string in decomp_raw.
import csv,glob,re,os,collections,sys
own={r[1] for r in csv.reader(open('03_re/ledger/subsystems.csv',encoding='utf-8')) if len(r)>1}
fn={r[0]:r for r in csv.reader(open('03_re/ledger/functions.csv',encoding='utf-8')) if r and r[0].startswith('0x')}
mods=collections.defaultdict(list);nomod=[]
for f in glob.glob('03_re/decomp_raw/*.c'):
    a=os.path.basename(f).split('_')[0]
    if a in own or a not in fn or fn[a][2]!='ENGINE': continue
    m=re.findall(r'Proj_GameZRecoil_(\w+?)_',open(f,encoding='utf-8',errors='ignore').read())
    (mods[collections.Counter(m).most_common(1)[0][0]] if m else nomod).append(a)
for k,v in sorted(mods.items(),key=lambda x:-len(x[1])):
    print(f'{k:24} {len(v):4} {min(v)}..{max(v)}')
print('no-path',len(nomod))
