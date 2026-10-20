#!/usr/bin/env python3
"""Make an ARTISTIC, high-detail version of the exported `bft` tank by driving real geometry
from the game's own textures.

THIS IS NOT A FIDELITY ARTEFACT. The 1:1 reference model is `reference/bft_model/`, which is
exactly what the original ships. This script deliberately ADDS geometry the original never had,
because a 438-triangle 1999 model reads as flat when you look at it up close or print it. Keep
the two apart: nothing here should ever be used as evidence about the original.

What it does, and why it counts as "using the textures":

  1. SUBDIVIDE each triangle to a target edge length, interpolating UVs linearly.
  2. SAMPLE that triangle's own texture at each new vertex's UV.
  3. DISPLACE the vertex along the surface normal by the sampled luminance.

So the track links, hull panel lines, vents and greebles that were painted into the textures
become actual three-dimensional relief. The tread texture in particular (`tanktrak`, a 64x16
strip of repeating link plates) turns into real raised links.

Per-material amplitudes are chosen by what the surface IS -- treads get deep, sharply quantised
relief; hull plating gets a shallow panel-line etch; light lenses and the translucent shadow
material get none. That table is the artistic judgement in this script; everything else is
mechanical.

Usage:
    python detail_bft.py [--src DIR] [--out DIR] [--quality low|med|high]
"""

import argparse
import math
import os
import shutil
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
REMAKE = os.path.abspath(os.path.join(HERE, "..", ".."))


# ---------------------------------------------------------------- PNG reader (no PIL dependency)

def read_png_luma(path):
    """Minimal PNG -> (w, h, [luma 0..1]). Handles the 8-bit RGB/RGBA/palette/grey forms the
    asset pipeline actually writes. Returns None if the file uses something exotic."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    pos = 8
    w = h = bitdepth = colortype = None
    idat = bytearray()
    palette = None
    trns = None
    while pos + 8 <= len(data):
        ln = struct.unpack(">I", data[pos:pos + 4])[0]
        typ = data[pos + 4:pos + 8]
        chunk = data[pos + 8:pos + 8 + ln]
        pos += 12 + ln
        if typ == b"IHDR":
            w, h, bitdepth, colortype = struct.unpack(">IIBB", chunk[:10])
        elif typ == b"PLTE":
            palette = chunk
        elif typ == b"tRNS":
            trns = chunk
        elif typ == b"IDAT":
            idat += chunk
        elif typ == b"IEND":
            break
    if w is None or bitdepth != 8:
        return None
    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}.get(colortype)
    if channels is None:
        return None
    raw = zlib.decompress(bytes(idat))
    stride = w * channels
    out = [0.0] * (w * h)
    prev = bytearray(stride)
    p = 0
    for y in range(h):
        ft = raw[p]
        p += 1
        line = bytearray(raw[p:p + stride])
        p += stride
        # PNG filters
        if ft == 1:
            for i in range(channels, stride):
                line[i] = (line[i] + line[i - channels]) & 0xFF
        elif ft == 2:
            for i in range(stride):
                line[i] = (line[i] + prev[i]) & 0xFF
        elif ft == 3:
            for i in range(stride):
                a = line[i - channels] if i >= channels else 0
                line[i] = (line[i] + ((a + prev[i]) >> 1)) & 0xFF
        elif ft == 4:
            for i in range(stride):
                a = line[i - channels] if i >= channels else 0
                b = prev[i]
                c = prev[i - channels] if i >= channels else 0
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 0xFF
        prev = line
        for x in range(w):
            o = x * channels
            if colortype == 3:
                idx = line[o]
                r, g, b = palette[idx * 3], palette[idx * 3 + 1], palette[idx * 3 + 2]
            elif colortype in (0, 4):
                r = g = b = line[o]
            else:
                r, g, b = line[o], line[o + 1], line[o + 2]
            out[y * w + x] = (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255.0
    return w, h, out


class Tex:
    def __init__(self, w, h, luma):
        self.w, self.h, self.l = w, h, luma
        n = len(luma)
        self.mean = sum(luma) / n if n else 0.5

    def sample(self, u, v):
        # Repeat wrap; the game's UVs tile well past [0,1] on the tread strips.
        x = int((u - math.floor(u)) * self.w) % self.w
        y = int((1.0 - (v - math.floor(v))) * self.h) % self.h
        return self.l[y * self.w + x]


# ---------------------------------------------------------------- the artistic table
#
# amplitude  : displacement in METRES at full luminance swing
# quantise   : >0 snaps luminance into N bands, so plates read as flat steps with crisp edges
#              instead of soft blobs -- what makes track links look like links
# edge       : target subdivision edge length in metres (smaller = finer)
# invert     : dark parts of the texture push OUT rather than in
DETAIL = {
    # TREADS -- the star of the show. The track band is a flat 56-triangle ribbon in the
    # original; this embosses the link plates painted into `tanktrak` into real relief, so the
    # links stand proud of the band and catch light on their own edges. 130 mm of throw on a
    # 700 mm-tall track is roughly a real link plate's proportion. Three bands, not five, so the
    # plateaus stay flat and the steps between them stay sharp.
    "tanktrak": dict(amplitude=0.130, quantise=3, edge=0.020, invert=False),
    # track covers / fenders: strong plate relief, raised bolt strips
    "tanktrks": dict(amplitude=0.050, quantise=4, edge=0.034, invert=False),
    # hull plating: real panel steps, dark recesses cut in
    "tankside": dict(amplitude=0.030, quantise=3, edge=0.045, invert=False),
    "tankback": dict(amplitude=0.042, quantise=4, edge=0.038, invert=False),
    "tanktop":  dict(amplitude=0.038, quantise=4, edge=0.038, invert=False),
    "tankbotm": dict(amplitude=0.020, quantise=0, edge=0.070, invert=False),
    "tankwing": dict(amplitude=0.035, quantise=3, edge=0.045, invert=False),
    # gun / turret greebles: crisp mechanical detail
    "tankgn01": dict(amplitude=0.035, quantise=4, edge=0.026, invert=False),
    "tankgn02": dict(amplitude=0.035, quantise=4, edge=0.026, invert=False),
    "tankgn03": dict(amplitude=0.035, quantise=4, edge=0.026, invert=False),
    "tankgn04": dict(amplitude=0.035, quantise=4, edge=0.026, invert=False),
    "tankgn05": dict(amplitude=0.035, quantise=4, edge=0.026, invert=False),
    # weapon barrels and mounts: ribbed sleeves and cooling fins
    "rfpgscl":  dict(amplitude=0.026, quantise=3, edge=0.026, invert=False),
    "mort01":   dict(amplitude=0.026, quantise=3, edge=0.026, invert=False),
    "mort02":   dict(amplitude=0.026, quantise=3, edge=0.026, invert=False),
    "mort03":   dict(amplitude=0.026, quantise=3, edge=0.026, invert=False),
    "gun_nap4": dict(amplitude=0.026, quantise=3, edge=0.026, invert=False),
    # light lenses stay glassy-smooth; a bumpy lens looks broken, not detailed.
    "tanklite": dict(amplitude=0.0, quantise=0, edge=0.060, invert=False),
}
DEFAULT_DETAIL = dict(amplitude=0.0, quantise=0, edge=0.10, invert=False)

QUALITY_SCALE = {"low": 2.2, "med": 1.0, "high": 0.65}


# ---------------------------------------------------------------- OBJ in

def load_obj(path):
    verts, uvs = [], []
    groups = []          # [(material, [ (i0,i1,i2, t0,t1,t2) ]) ]
    cur = None
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            if line.startswith("v "):
                _, x, y, z = line.split()
                verts.append((float(x), float(y), float(z)))
            elif line.startswith("vt "):
                _, u, v = line.split()
                uvs.append((float(u), float(v)))
            elif line.startswith("usemtl "):
                cur = line.split(None, 1)[1].strip()
                groups.append((cur, []))
            elif line.startswith("f "):
                parts = line.split()[1:]
                idx = []
                for p in parts:
                    a = p.split("/")
                    vi = int(a[0]) - 1
                    ti = int(a[1]) - 1 if len(a) > 1 and a[1] else -1
                    idx.append((vi, ti))
                for k in range(1, len(idx) - 1):
                    tri = (idx[0], idx[k], idx[k + 1])
                    groups[-1][1].append(tri)
    return verts, uvs, groups


def load_mtl(path):
    mats, cur = {}, None
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line.startswith("newmtl "):
                cur = line.split(None, 1)[1]
                mats[cur] = {}
            elif cur and line.startswith("map_Kd "):
                mats[cur]["map"] = line.split(None, 1)[1].strip()
            elif cur and line.startswith("Kd "):
                mats[cur]["Kd"] = line
            elif cur and line.startswith("d "):
                mats[cur]["d"] = line
    return mats


# ---------------------------------------------------------------- geometry

def sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def norm(a):
    l = math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2])
    return (a[0] / l, a[1] / l, a[2] / l) if l > 1e-12 else (0.0, 1.0, 0.0)


def dist(a, b):
    d = sub(a, b)
    return math.sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2])


def subdivide(p0, p1, p2, t0, t1, t2, target, depth_cap=5):
    """Recursive 1->4 split until the longest edge is under `target`."""
    out = []
    stack = [(p0, p1, p2, t0, t1, t2, 0)]
    while stack:
        a, b, c, ta, tb, tc, d = stack.pop()
        longest = max(dist(a, b), dist(b, c), dist(c, a))
        if longest <= target or d >= depth_cap:
            out.append((a, b, c, ta, tb, tc))
            continue
        ab = ((a[0] + b[0]) / 2, (a[1] + b[1]) / 2, (a[2] + b[2]) / 2)
        bc = ((b[0] + c[0]) / 2, (b[1] + c[1]) / 2, (b[2] + c[2]) / 2)
        ca = ((c[0] + a[0]) / 2, (c[1] + a[1]) / 2, (c[2] + a[2]) / 2)
        tab = ((ta[0] + tb[0]) / 2, (ta[1] + tb[1]) / 2)
        tbc = ((tb[0] + tc[0]) / 2, (tb[1] + tc[1]) / 2)
        tca = ((tc[0] + ta[0]) / 2, (tc[1] + ta[1]) / 2)
        stack.append((a, ab, ca, ta, tab, tca, d + 1))
        stack.append((ab, b, bc, tab, tb, tbc, d + 1))
        stack.append((ca, bc, c, tca, tbc, tc, d + 1))
        stack.append((ab, bc, ca, tab, tbc, tca, d + 1))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default=os.path.join(REMAKE, "reference", "bft_model"))
    ap.add_argument("--out", default=os.path.join(REMAKE, "reference", "bft_model_detailed"))
    ap.add_argument("--quality", choices=list(QUALITY_SCALE), default="med")
    args = ap.parse_args()

    obj_in = os.path.join(args.src, "bft.obj")
    mtl_in = os.path.join(args.src, "bft.mtl")
    if not os.path.isfile(obj_in):
        print("no source model at %s" % obj_in, file=sys.stderr)
        return 2

    verts, uvs, groups = load_obj(obj_in)
    mats = load_mtl(mtl_in)
    print("source: %d verts, %d tris, %d groups"
          % (len(verts), sum(len(g[1]) for g in groups), len(groups)))

    outdir = args.out
    texout = os.path.join(outdir, "textures")
    os.makedirs(texout, exist_ok=True)
    for fn in os.listdir(os.path.join(args.src, "textures")):
        shutil.copyfile(os.path.join(args.src, "textures", fn), os.path.join(texout, fn))

    texcache = {}

    def tex_for(matname):
        m = mats.get(matname, {})
        mp = m.get("map")
        if not mp:
            return None, None
        base = os.path.splitext(os.path.basename(mp))[0]
        if base not in texcache:
            r = read_png_luma(os.path.join(args.src, "textures", base + ".png"))
            texcache[base] = Tex(*r) if r else None
        return texcache[base], base

    scale = QUALITY_SCALE[args.quality]

    # Pass 1: subdivide, and accumulate a displacement + normal per unique POSITION so the
    # displaced mesh stays closed at UV seams (displacing per-corner would tear it open).
    acc = {}     # rounded position -> [nx,ny,nz, dispSum, count]
    tris_out = []  # (material, [(pos, uv) x3])
    KEY = 1e5

    for matname, tris in groups:
        tex, base = tex_for(matname)
        cfg = DETAIL.get(base, DEFAULT_DETAIL) if base else DEFAULT_DETAIL
        target = cfg["edge"] * scale
        amp = cfg["amplitude"]
        q = cfg["quantise"]
        for (ia, ta), (ib, tb), (ic, tc) in tris:
            p0, p1, p2 = verts[ia], verts[ib], verts[ic]
            u0 = uvs[ta] if ta >= 0 else (0.0, 0.0)
            u1 = uvs[tb] if tb >= 0 else (0.0, 0.0)
            u2 = uvs[tc] if tc >= 0 else (0.0, 0.0)
            n = norm(cross(sub(p1, p0), sub(p2, p0)))
            pieces = subdivide(p0, p1, p2, u0, u1, u2, target) if amp > 0.0 else \
                [(p0, p1, p2, u0, u1, u2)]
            for (a, b, c, ta2, tb2, tc2) in pieces:
                tris_out.append((matname, [(a, ta2), (b, tb2), (c, tc2)]))
                for pos, uv in ((a, ta2), (b, tb2), (c, tc2)):
                    k = (round(pos[0] * KEY), round(pos[1] * KEY), round(pos[2] * KEY))
                    e = acc.get(k)
                    if e is None:
                        e = [0.0, 0.0, 0.0, 0.0, 0]
                        acc[k] = e
                    e[0] += n[0]; e[1] += n[1]; e[2] += n[2]
                    d = 0.0
                    if amp > 0.0 and tex is not None:
                        lum = tex.sample(uv[0], uv[1])
                        if q > 0:
                            # Snap into q clean bands across [0,1]. The earlier
                            # floor(lum*q)/(q-1) overshot past 1 and had to be clamped, which
                            # flattened the top band and biased everything bright.
                            lum = round(lum * (q - 1)) / (q - 1) if q > 1 else round(lum)
                        d = (lum - tex.mean) * amp
                        if cfg["invert"]:
                            d = -d
                    e[3] += d
                    e[4] += 1

    print("subdivided: %d triangles, %d unique positions" % (len(tris_out), len(acc)))

    # Pass 2: emit the displaced, welded mesh.
    order = {}
    vlines, tlines, nlines = [], [], []
    uvindex = {}
    faces = {}
    for matname, corners in tris_out:
        f = []
        for pos, uv in corners:
            k = (round(pos[0] * KEY), round(pos[1] * KEY), round(pos[2] * KEY))
            e = acc[k]
            if k not in order:
                nv = norm((e[0], e[1], e[2]))
                d = e[3] / e[4]
                order[k] = len(order) + 1
                vlines.append("v %.6f %.6f %.6f"
                              % (pos[0] + nv[0] * d, pos[1] + nv[1] * d, pos[2] + nv[2] * d))
                nlines.append("vn %.6f %.6f %.6f" % nv)
            vi = order[k]
            uk = (round(uv[0] * 1e4), round(uv[1] * 1e4))
            if uk not in uvindex:
                uvindex[uk] = len(uvindex) + 1
                tlines.append("vt %.6f %.6f" % (uv[0], uv[1]))
            f.append("%d/%d/%d" % (vi, uvindex[uk], vi))
        faces.setdefault(matname, []).append("f " + " ".join(f))

    obj_path = os.path.join(outdir, "bft_detailed.obj")
    mtl_path = os.path.join(outdir, "bft_detailed.mtl")
    with open(obj_path, "w", encoding="utf-8") as f:
        f.write("# Recoil 'bft' -- ARTISTIC high-detail version.\n")
        f.write("# Geometry displaced from the game's own textures. NOT a fidelity reference:\n")
        f.write("# the 1:1 model is reference/bft_model/bft.obj.\n")
        f.write("# %d vertices, %d triangles\n" % (len(vlines), len(tris_out)))
        f.write("mtllib %s\n" % os.path.basename(mtl_path))
        f.write("\n".join(vlines) + "\n")
        f.write("\n".join(tlines) + "\n")
        f.write("\n".join(nlines) + "\n")
        for matname in faces:
            f.write("usemtl %s\n" % matname)
            f.write("\n".join(faces[matname]) + "\n")

    shutil.copyfile(mtl_in, mtl_path)
    print("wrote %s (%d verts, %d tris)" % (obj_path, len(vlines), len(tris_out)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
