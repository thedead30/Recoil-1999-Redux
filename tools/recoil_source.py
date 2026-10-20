"""recoil_source.py - read the player's own copy of Recoil, whatever form it comes in.

Supported inputs (auto-detected):
  * an installed game folder (contains Recoil.exe and zbd\\)
  * a .zip of an installed folder
  * a CD folder / mounted CD (InstallShield 5 installer: data1.cab)
  * disc images: .iso (2048-byte sectors), .bin raw (2352-byte sectors, Mode 1 or Mode 2, with or without .cue),
    .nrg (Nero, NER5/NERO footer), .cue + .bin
The game files are read straight from the installer cabinet when the input is a CD (nothing is installed).
CD audio tracks (the game's music) are found in .nrg / .bin(+cue) images and can be saved as WAV files.

Nothing here contains or ships Recoil data: every byte comes from the copy the user points at.
usage: python tools/recoil_source.py SOURCE [--list] [--extract DIR] [--music DIR]
"""
import io, os, re, sys, struct, zlib, zipfile, hashlib, wave

# ---- known Recoil.exe builds (SHA-256 of the file) ----
KNOWN_EXE = {
    '22423e0bb090b6be6e10bca10a7c2d851da7e3708c30d27687369d40f93987d0':
        ('Recoil 1999-01-29 build (Recoil Classic / patched)', True),
    '98f306c21ceab1cebdd492a93766118460293cf1da0f31f89aa030a281786c2f':
        ('Recoil 1998-11-04 build (original 1998 CD)', False),
    '20eb9378af7be02af6e97689bde1703ba710bf2eb132c32254c8fc152158d50b':
        ('Recoil 1998-11-04 build, modified copy (same size as the 1998 CD exe, different bytes - likely a no-CD patch)', False),
}
SUPPORTED_NOTE = ('this build tool only accepts the version the remake was made from: the 1999-01-29 build (as on the '
                  '"Recoil Classic" CD). The 1998-11-04 release differs in its program code and in its game data '
                  '(35 data files differ and missions 10-13 are packaged differently), so it cannot be used.')


def exe_version(data):
    h = hashlib.sha256(data).hexdigest()
    name, ok = KNOWN_EXE.get(h, ('unknown build', False))
    return h, name, ok


# =====================================================================================================
# Sector-level disc readers
# =====================================================================================================
SYNC = b'\x00' + b'\xff' * 10 + b'\x00'


class SectorImage:
    """2048-byte user-data sectors of a data track inside an image file."""
    def __init__(self, path, start, sector_size, data_offset, n_sectors=None):
        self.f = open(path, 'rb'); self.start = start; self.ss = sector_size; self.do = data_offset
        self.n = n_sectors

    def read(self, lba, count=1):
        out = bytearray()
        for k in range(count):
            self.f.seek(self.start + (lba + k) * self.ss + self.do)
            out += self.f.read(2048)
        return bytes(out)


class AudioTrack:
    def __init__(self, number, path, start, length):
        self.number, self.path, self.start, self.length = number, path, start, length   # bytes of 2352-byte CDDA

    @property
    def seconds(self): return self.length / 2352 / 75

    def save_wav(self, out):
        with open(self.path, 'rb') as f, wave.open(out, 'wb') as w:
            w.setnchannels(2); w.setsampwidth(2); w.setframerate(44100)
            f.seek(self.start); left = self.length
            while left > 0:
                b = f.read(min(left, 2352 * 75 * 10)); w.writeframes(b); left -= len(b)
                if not b: break


def raw_bin_layout(path, start=0):
    """raw 2352 image: data track mode + where its data sectors end (for images without a cue sheet)"""
    f = open(path, 'rb'); f.seek(start); h = f.read(16)
    if h[:12] != SYNC: return None
    mode = h[15]
    total = (os.path.getsize(path) - start) // 2352

    def is_data(s):
        f.seek(start + s * 2352); return f.read(12) == SYNC
    end = total
    if not is_data(total - 1):
        lo, hi = 0, total - 1
        while lo < hi - 1:
            m = (lo + hi) // 2
            lo, hi = (m, hi) if is_data(m) else (lo, m)
        end = hi
    return mode, end, total


def split_audio_by_silence(path, start, end_bytes, min_gap=150):
    """CD audio without a cue sheet: tracks are separated by digital silence (a 2 s pregap = 150 sectors of zeros)"""
    f = open(path, 'rb'); tracks = []; s = start; n_sec = (end_bytes - start) // 2352
    zero_run = 0; cur = start; k = 0
    while k < n_sec:
        f.seek(start + k * 2352); b = f.read(2352)
        if b.count(0) == 2352:
            zero_run += 1
        else:
            if zero_run >= min_gap and k - zero_run > (cur - start) // 2352:
                tracks.append((cur, start + (k - zero_run) * 2352)); cur = start + k * 2352
            elif not tracks and zero_run and cur == start:
                cur = start + k * 2352            # leading pregap
            zero_run = 0
        k += 1
    tracks.append((cur, start + (n_sec - zero_run) * 2352))
    return [(a, b) for a, b in tracks if b - a > 2352 * 75]   # drop slivers under 1 s


def parse_cue(cue_path):
    """minimal cue sheet: FILE / TRACK nn MODE1/2352|MODE2/2352|AUDIO / INDEX 01 mm:ss:ff"""
    tracks = []; cur_file = None; base = os.path.dirname(cue_path)
    for line in open(cue_path, encoding='latin-1'):
        t = line.strip().split()
        if not t: continue
        if t[0].upper() == 'FILE':
            name = line.split('"')[1] if '"' in line else t[1]; cur_file = os.path.join(base, name)
        elif t[0].upper() == 'TRACK':
            tracks.append({'n': int(t[1]), 'type': t[2].upper(), 'file': cur_file, 'index1': None})
        elif t[0].upper() == 'INDEX' and t[1] == '01':
            mm, ss, ff = map(int, t[2].split(':')); tracks[-1]['index1'] = (mm * 60 + ss) * 75 + ff
    return tracks


def nrg_tracks(path):
    """Nero image: track table from the NER5/NERO footer chunks (DAOX/DAOI)"""
    f = open(path, 'rb'); f.seek(-12, 2); tail = f.read(12)
    if tail[:4] == b'NER5': off = struct.unpack('>Q', tail[4:12])[0]; v2 = True
    else:
        f.seek(-8, 2); tail = f.read(8)
        if tail[:4] != b'NERO': return None
        off = struct.unpack('>I', tail[4:8])[0]; v2 = False
    f.seek(off); data = f.read(1 << 20); k = 0; out = []
    while k + 8 <= len(data):
        cid = data[k:k + 4]; ln = struct.unpack('>I', data[k + 4:k + 8])[0]; body = data[k + 8:k + 8 + ln]
        if cid in (b'DAOX', b'DAOI'):
            first, last = body[20], body[21]; esz = 42 if cid == b'DAOX' else 30
            for t in range(last - first + 1):
                e = body[22 + esz * t:22 + esz * (t + 1)]
                ss, mode = struct.unpack('>HH', e[12:16])
                if cid == b'DAOX': i0, i1, end = struct.unpack('>QQQ', e[18:42])
                else: i0, i1, end = struct.unpack('>III', e[18:30])
                out.append({'n': first + t, 'sector': ss, 'mode': mode, 'start': i1, 'end': end})
        if cid == b'END!': break
        k += 8 + ln
    return out


# =====================================================================================================
# ISO 9660 file system over a SectorImage
# =====================================================================================================
class Iso9660:
    def __init__(self, img):
        self.img = img; self.files = {}
        pvd = img.read(16)
        if pvd[1:6] != b'CD001': raise ValueError('no ISO 9660 volume')
        self._walk(pvd[156:156 + 34], '')

    def _walk(self, rec, prefix):
        extent, size = struct.unpack('<I', rec[2:6])[0], struct.unpack('<I', rec[10:14])[0]
        data = self.img.read(extent, (size + 2047) // 2048); k = 0
        while k < size:
            ln = data[k]
            if ln == 0: k = (k // 2048 + 1) * 2048; continue
            r = data[k:k + ln]; flags = r[25]; nl = r[32]; name = r[33:33 + nl]
            if name not in (b'\x00', b'\x01'):
                nm = name.decode('latin-1').split(';')[0].rstrip('.')
                path = prefix + nm
                if flags & 2: self._walk(r, path + '/')
                else: self.files[path.lower()] = (path, struct.unpack('<I', r[2:6])[0], struct.unpack('<I', r[10:14])[0])
            k += ln

    def read(self, path):
        _, ext, size = self.files[path.lower()]
        return self.img.read(ext, (size + 2047) // 2048)[:size]


# =====================================================================================================
# InstallShield 5 cabinet (data1.cab, "ISc(") - after the format notes of the open-source unshield project
# =====================================================================================================
class InstallShield5:
    def __init__(self, data):
        self.d = data
        sig, ver, vol, cdo, cds = struct.unpack('<5I', data[:20])
        if data[:4] != b'ISc(': raise ValueError('not an InstallShield cabinet')
        self.cdo = cdo
        ft = cdo + struct.unpack('<I', data[cdo + 0x0c:cdo + 0x10])[0]
        ndirs = struct.unpack('<I', data[cdo + 0x1c:cdo + 0x20])[0]
        nfiles = struct.unpack('<I', data[cdo + 0x28:cdo + 0x2c])[0]
        offs = struct.unpack('<%dI' % (ndirs + nfiles), data[ft:ft + 4 * (ndirs + nfiles)])
        cstr = lambda o: data[ft + o:data.index(b'\0', ft + o)].decode('latin-1')
        self.dirs = [cstr(o) for o in offs[:ndirs]]
        self.files = {}
        for o in offs[ndirs:]:
            p = ft + o
            name_off, dir_idx = struct.unpack('<II', data[p:p + 8])
            flags = struct.unpack('<H', data[p + 8:p + 10])[0]
            exp, comp = struct.unpack('<II', data[p + 10:p + 18])
            doff = struct.unpack('<I', data[p + 38:p + 42])[0]
            if flags & 8: continue                                   # invalid entry
            d = self.dirs[dir_idx] if dir_idx < len(self.dirs) else ''
            path = (d + '\\' if d else '') + cstr(name_off)
            self.files[path.lower()] = (path, flags, exp, comp, doff)

    def read(self, path):
        _, flags, exp, comp, doff = self.files[path.lower()]
        raw = self.d[doff:doff + comp]
        if flags & 2: raise ValueError('obfuscated file (not supported): ' + path)
        if not flags & 4: return raw[:exp]
        out = bytearray(); k = 0
        while k < len(raw) and len(out) < exp:              # chunks: uint16 length + raw deflate data
            n = struct.unpack('<H', raw[k:k + 2])[0]; k += 2
            out += zlib.decompress(raw[k:k + n], -15); k += n
        return bytes(out[:exp])


# =====================================================================================================
# One interface for every kind of source
# =====================================================================================================
class Source:
    """files: {lowercase path with / : display path}; read(path) -> bytes; audio: [AudioTrack]; kind: str"""
    def __init__(self, path):
        self.path = path; self.audio = []; self.kind = None; self._read = None; self.files = {}
        self._open(path)
        self.game = self._find_game()

    # -- detection --
    def _open(self, p):
        low = p.lower()
        if os.path.isdir(p):
            files = {}
            for dp, _, fns in os.walk(p):
                for fn in fns:
                    full = os.path.join(dp, fn); rel = os.path.relpath(full, p).replace('\\', '/')
                    files[rel.lower()] = rel
            self.files = files; self._read = lambda r: open(os.path.join(p, self.files[r.lower()]), 'rb').read()
            self.kind = 'folder'; return
        if zipfile.is_zipfile(p):
            z = zipfile.ZipFile(p); self.files = {n.lower(): n for n in z.namelist() if not n.endswith('/')}
            self._read = lambda r: z.read(self.files[r.lower()]); self.kind = 'zip'; return
        if low.endswith('.cue'):
            tr = parse_cue(p); data = [t for t in tr if t['type'] != 'AUDIO']
            d = data[0]; ss = 2352 if '2352' in d['type'] else 2048
            do = 0 if ss == 2048 else (24 if d['type'].startswith('MODE2') else 16)
            self._iso(SectorImage(d['file'], d['index1'] * ss, ss, do))
            files = {}
            for i, t in enumerate(tr):
                if t['type'] != 'AUDIO': continue
                nxt = tr[i + 1] if i + 1 < len(tr) and tr[i + 1]['file'] == t['file'] else None
                st = t['index1'] * 2352
                end = nxt['index1'] * 2352 if nxt else os.path.getsize(t['file'])
                self.audio.append(AudioTrack(t['n'], t['file'], st, end - st))
            self.kind = 'cue/bin'; return
        nt = nrg_tracks(p) if low.endswith('.nrg') else None
        if nt:
            d = [t for t in nt if t['mode'] != 0x0700][0]
            do = 0 if d['sector'] == 2048 else (24 if d['sector'] in (2336, 2352) and d['mode'] in (2, 3) else 16)
            self._iso(SectorImage(p, d['start'], d['sector'], do if d['sector'] != 2048 else 0))
            for t in nt:
                if t['mode'] == 0x0700: self.audio.append(AudioTrack(t['n'], p, t['start'], t['end'] - t['start']))
            self.kind = 'nrg'; return
        lay = raw_bin_layout(p)
        if lay:
            mode, end, total = lay
            self._iso(SectorImage(p, 0, 2352, 24 if mode == 2 else 16))
            cue = os.path.splitext(p)[0] + '.cue'
            if os.path.exists(cue):
                self.audio = Source(cue).audio
            elif end < total:
                for n, (a, b) in enumerate(split_audio_by_silence(p, end * 2352, total * 2352), start=2):
                    self.audio.append(AudioTrack(n, p, a, b - a))
            self.kind = 'bin (raw, %s)' % ('Mode 2' if mode == 2 else 'Mode 1'); return
        # plain .iso (2048-byte sectors)
        self._iso(SectorImage(p, 0, 2048, 0)); self.kind = 'iso'

    def _iso(self, img):
        fs = Iso9660(img)
        self.files = {k: v[0] for k, v in fs.files.items()}; self._read = fs.read

    def read(self, rel): return self._read(rel)

    # -- locate the game: an installed tree, or the InstallShield cabinet on a CD --
    def _find_game(self):
        exes = [k for k in self.files if k.endswith('recoil.exe')]
        for e in exes:
            root = e[:-len('recoil.exe')]
            if any(k.startswith(root + 'zbd/') for k in self.files):
                return Tree(self, root)
        cabs = [k for k in self.files if k.endswith('data1.cab')]
        if cabs:
            return Cabinet(InstallShield5(self.read(cabs[0])))
        return None


class Tree:
    """an installed game folder inside a Source"""
    def __init__(self, src, root): self.src, self.root = src, root; self.kind = 'installed game files'

    def list(self):
        return [self.src.files[k][len(self.root):] for k in self.src.files if k.startswith(self.root)]

    def read(self, rel): return self.src.read(self.root + rel.replace('\\', '/').lower())


class Cabinet:
    """game files inside the CD's InstallShield cabinet"""
    def __init__(self, cab): self.cab = cab; self.kind = 'InstallShield cabinet (CD installer)'

    def list(self): return [v[0].replace('\\', '/') for v in self.cab.files.values()]

    def read(self, rel):
        key = rel.replace('/', '\\').lower()
        if key in self.cab.files: return self.cab.read(key)
        for k in self.cab.files:          # cabinets may keep files under a component folder
            if k.endswith('\\' + key) or k == key: return self.cab.read(k)
        raise KeyError(rel)


def describe(path):
    s = Source(path)
    print('source  :', path)
    print('format  :', s.kind)
    if not s.game:
        print('game    : NOT FOUND (no Recoil.exe + zbd folder, no InstallShield data1.cab)'); return s
    print('game    :', s.game.kind, '(%d files)' % len(s.game.list()))
    try:
        exe = s.game.read('Recoil.exe'); h, name, ok = exe_version(exe)
        print('Recoil.exe: %s  sha256 %s..  %s' % (name, h[:16], 'SUPPORTED' if ok else 'NOT the supported build - ' + SUPPORTED_NOTE))
    except KeyError:
        print('Recoil.exe: not found in the game files')
    if s.audio:
        print('music   : %d CD audio track(s): %s' % (len(s.audio), ', '.join('track %d %d:%02d' % (t.number, t.seconds // 60, t.seconds % 60) for t in s.audio)))
    else:
        print('music   : none (only full disc images .nrg / .bin(+cue) carry the CD audio tracks)')
    return s


if __name__ == '__main__':
    a = sys.argv[1:]
    if not a: print(__doc__); sys.exit(1)
    s = describe(a[0])
    if '--list' in a and s.game:
        for n in sorted(s.game.list()): print('   ', n)
    if '--music' in a:
        out = a[a.index('--music') + 1]; os.makedirs(out, exist_ok=True)
        for t in s.audio:
            fn = os.path.join(out, 'track%02d.wav' % t.number); t.save_wav(fn); print('wrote', fn)
    if '--extract' in a and s.game:
        out = a[a.index('--extract') + 1]
        for n in s.game.list():
            dst = os.path.join(out, n.replace('/', os.sep)); os.makedirs(os.path.dirname(dst) or out, exist_ok=True)
            open(dst, 'wb').write(s.game.read(n))
        print('extracted', len(s.game.list()), 'files to', out)
