"""make_asset_pack.py - the Recoil Redux asset pack zip for Setup's step 3 (Redux v1, user 2026-10-17).

  dist/RecoilRedux-AssetPack-HD-<ver>.zip   pack.txt + redux_textures/*.png (incl. m<N>/ mission versions) + redux_video/*.mp4

Stored, not compressed (PNG and MP4 are compressed already; Setup's zip reader takes stored entries up to 4 GB). Only .png
under redux_textures and .mp4 under redux_video are packed - no screenshots, saves, logs or notes. Every file name and
pack.txt are checked for personal data (this machine's user folder, e-mail addresses) before writing.
The pack is made from the player's-own-copy-derived AI upscales (EA content): it is for the user's own installs, not for
the public release.
usage: python tools/make_asset_pack.py [--version V] [--from GAME_FOLDER]
"""
import datetime, os, re, sys, zipfile

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
DIST = os.path.join(ROOT, 'dist')
PACK_TXT = ('Recoil Redux HD asset pack {ver}\r\n'
            'High-resolution textures for all 13 maps and HD versions of the cutscene videos.\r\n')


def main():
    a = sys.argv[1:]
    ver = a[a.index('--version') + 1] if '--version' in a else datetime.date.today().strftime('%Y.%m.%d')
    src = a[a.index('--from') + 1] if '--from' in a else os.path.join(ROOT, '05_remake', 'build', 'game_sandbox')
    files = []
    for top, ext in (('redux_textures', '.png'), ('redux_video', '.mp4')):
        for dp, _, fns in os.walk(os.path.join(src, top)):
            for fn in sorted(fns):
                if fn.lower().endswith(ext):
                    full = os.path.join(dp, fn)
                    files.append((full, os.path.relpath(full, src).replace(os.sep, '/')))
    user = os.path.basename(os.environ.get('USERPROFILE', '')).lower()
    bad = [r for _, r in files if (user and user in r.lower()) or re.search(r'@|\.com', r)]
    if bad:
        sys.exit('NOT WRITTEN - personal data in file names: %s' % bad[:5])
    n_tex = sum(r.startswith('redux_textures/') for _, r in files)
    n_vid = len(files) - n_tex
    os.makedirs(DIST, exist_ok=True)
    out = os.path.join(DIST, 'RecoilRedux-AssetPack-HD-%s.zip' % ver)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_STORED) as z:
        z.writestr('pack.txt', PACK_TXT.format(ver=ver))
        for full, rel in files:
            z.write(full, rel)
    size = os.path.getsize(out)
    if size >= 0xFFFFFFFF:
        os.remove(out)
        sys.exit('NOT WRITTEN - the pack is over 4 GB, which Setup cannot read')
    print('wrote %s: %d textures, %d videos, %.2f GB' % (os.path.relpath(out, ROOT), n_tex, n_vid, size / 1e9))
    return 0


if __name__ == '__main__':
    sys.exit(main())
