import struct,csv,sys,subprocess
d=open(r'00_original/game_install/Recoil.exe','rb').read()
F={r[0]:r for r in csv.reader(open('03_re/ledger/functions.csv'))}
nm=lambda v:F.get('0x%08x'%v,[0,'?'])[1]
pe=struct.unpack_from('<I',d,0x3c)[0];n=struct.unpack_from('<H',d,pe+6)[0];so=pe+24+struct.unpack_from('<H',d,pe+20)[0]
S=[struct.unpack_from('<8xIIII',d,so+40*i) for i in range(n)]
def fl(va,dbl):
  r=va-0x400000
  for vs,vaa,rs,rp in S:
    if vaa<=r<vaa+min(vs,rs):return round(struct.unpack_from('<d' if dbl else '<f',d,rp+r-vaa)[0],6)
L=int(sys.argv[2]) if len(sys.argv)>2 else 18
for r in F.values():
  if len(r)>3 and r[0].startswith(sys.argv[1]) and r[2]=='ENGINE' and r[3]!='CONFIRMED':
    a=int(r[0],16);out=subprocess.run(['python','03_re/scripts/decomp_body.py',r[0]],capture_output=True,text=True).stdout.splitlines()
    sz=[l for l in out if 'bytes=' in l];n=int(sz[0].split('bytes=')[1]) if sz else 48
    print('==',r[0],r[5:7],'size',n)
    for l in out[3:3+L]:print(' ',l.strip()[:115])
    x=d[a-0x400C00:][:n]
    c=[nm(a+i+5+struct.unpack_from('<i',x,i+1)[0])[:24] for i in range(n-5) if x[i] in(0xe8,0xe9) and 0x401000<=a+i+5+struct.unpack_from('<i',x,i+1)[0]<0x4d0000]
    k=sorted({(hex(v),fl(v,x[i-2]==0xdc)) for i in range(2,n-4) for v in [struct.unpack_from('<I',x,i)[0]] if 0x4cc000<=v<0x4d1000 and x[i-2] in(0xd8,0xd9,0xdc)})
    print('  BYTES tail',x[n-3:n].hex(),'calls',c[:12],'fconst',k[:6],'push',[hex(struct.unpack_from('<I',x,i)[0]) for i in range(1,n-4) if x[i-1]==0x68][:6])
    if not sz:print('  RAW',x.hex())
