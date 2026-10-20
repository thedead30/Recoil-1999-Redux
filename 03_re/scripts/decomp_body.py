"""decomp_body.py ADDR... - print decomp_raw bodies with local declarations and blank lines stripped."""
import glob, os, re, sys
for a in sys.argv[1:]:
    fs = glob.glob('03_re/decomp_raw/0x%08x_*.c' % int(a, 16))
    if not fs:
        print('== %s: no corpus file' % a); continue
    src = open(fs[0], encoding='utf-8', errors='replace').read()
    out = []
    for l in src.splitlines():
        s = l.strip()
        if not s or s.startswith('/*') or re.match(r'^(undefined\d?|int|uint|char|byte|bool|float|float10|short|ushort|code|void|UINT|MMRESULT|MCIERROR|HRESULT|size_t|tagAUXCAPSA)\s*\**\s*\w+(\s*\[\d+\])?;$', s):
            continue
        out.append(l)
    print('== ' + os.path.basename(fs[0]))
    print('\n'.join(out))
