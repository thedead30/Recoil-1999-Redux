#!/usr/bin/env python3
"""Offscreen turntable renderer for an exported OBJ+MTL+textures model.

Deliberately reproduces the CONFIRMED original render state so these images mean the same
thing the in-engine ones do (ASSET_RESOLUTION.md sec. 6, from the 10 SetRenderState calls in
Render_CreateD3DDevice @ 0x4a9c20):

  ZENABLE      = TRUE            -> z-buffer, per-pixel
  ZFUNC        = LESS_EQUAL      -> depth test <=
  CULLMODE     = NONE            -> no backface culling, single-winding submission
  SPECULARENABLE = FALSE         -> no specular
  texture filter = linear        -> bilinear sampling
  face colour  = 0xFFFFFFFF      -> UNLIT. No invented lighting.

Because the real shading is unlit, a pure-texture render is flat and hard to read as 3D. So an
OPTIONAL, clearly-labelled depth-cue shade (--shade) may be applied for the human-readable
turntable; --no-shade gives the literal confirmed-unlit result. The default writes both sets.

Usage:
    python render_turntable.py --obj ../../reference/bft_model/bft.obj --out ../../reference/bft_model/renders
"""

import argparse
import math
import os
import sys

import numpy as np
from PIL import Image


# ------------------------------------------------------------------ obj/mtl

def load_mtl(path):
    mats = {}
    cur = None
    base = os.path.dirname(path)
    if not os.path.isfile(path):
        return mats
    for line in open(path, "r", encoding="utf-8"):
        p = line.split()
        if not p:
            continue
        if p[0] == "newmtl":
            cur = p[1]
            mats[cur] = {"Kd": (1.0, 1.0, 1.0), "map": None, "d": 1.0}
        elif cur is None:
            continue
        elif p[0] == "Kd":
            mats[cur]["Kd"] = (float(p[1]), float(p[2]), float(p[3]))
        elif p[0] == "d":
            mats[cur]["d"] = float(p[1])
        elif p[0] == "map_Kd":
            mats[cur]["map"] = os.path.join(base, p[1].replace("/", os.sep))
    return mats


def load_obj(path):
    verts = []
    uvs = []
    tris = []          # (i0,i1,i2, t0,t1,t2, matname)
    curmat = None
    mtl = {}
    base = os.path.dirname(path)
    for line in open(path, "r", encoding="utf-8"):
        p = line.split()
        if not p:
            continue
        k = p[0]
        if k == "v":
            verts.append((float(p[1]), float(p[2]), float(p[3])))
        elif k == "vt":
            uvs.append((float(p[1]), float(p[2])))
        elif k == "usemtl":
            curmat = p[1]
        elif k == "mtllib":
            mtl = load_mtl(os.path.join(base, p[1]))
        elif k == "f":
            idx = []
            tidx = []
            for c in p[1:4]:
                bits = c.split("/")
                idx.append(int(bits[0]) - 1)
                tidx.append(int(bits[1]) - 1 if len(bits) > 1 and bits[1] else -1)
            tris.append((idx[0], idx[1], idx[2], tidx[0], tidx[1], tidx[2], curmat))
    return np.array(verts, dtype=np.float64), np.array(uvs, dtype=np.float64) if uvs else np.zeros((0, 2)), tris, mtl


def load_texture(path, cache):
    if path in cache:
        return cache[path]
    img = None
    if path and os.path.isfile(path):
        # RGBA, not RGB: 202 of mission 1's 576 textures carry real alpha, and they are exactly
        # the foliage and effect textures (palm01-04, holobeam, tracer, smoke, splashes). Loading
        # them as RGB throws that away and draws palm fronds as opaque BLACK quads.
        im = Image.open(path).convert("RGBA")
        img = np.asarray(im, dtype=np.float64) / 255.0
    cache[path] = img
    return img


# ------------------------------------------------------------------ raster

def sample_bilinear(tex, u, v):
    """u,v in [0,1] in OBJ convention (V=0 at the BOTTOM). Wraps: the game tiles textures.

    The image array is top-left origin, so V must be flipped back here. Getting this wrong
    double-flips: the exporter writes 1-v for OBJ, and indexing the array with that directly
    samples upside down. It is easy to miss on noisy textures and obvious on lettering --
    "CAUTION" rendered as "CVNLION" (A -> V, T -> upside-down T), which is what exposed it.
    """
    h, w, _ = tex.shape
    x = (u % 1.0) * (w - 1)
    y = ((1.0 - v) % 1.0) * (h - 1)
    x0 = np.floor(x).astype(np.int32)
    y0 = np.floor(y).astype(np.int32)
    x1 = np.clip(x0 + 1, 0, w - 1)
    y1 = np.clip(y0 + 1, 0, h - 1)
    fx = (x - x0)[..., None]
    fy = (y - y0)[..., None]
    a = tex[y0, x0] * (1 - fx) + tex[y0, x1] * fx
    b = tex[y1, x0] * (1 - fx) + tex[y1, x1] * fx
    return a * (1 - fy) + b * fy   # RGBA


def camera_basis(target, yaw_deg, pitch_deg, dist):
    """Orbit camera around `target`. yaw 0 = behind the tank, looking forward along +Z --
    the same convention as the in-engine debug orbit (reference/bft_handbuilt/README.md)."""
    yaw = math.radians(yaw_deg)
    pitch = math.radians(pitch_deg)
    eye = target + np.array([
        math.sin(yaw) * math.cos(pitch) * dist,
        math.sin(pitch) * dist,
        math.cos(yaw) * math.cos(pitch) * dist,
    ])
    fwd = target - eye
    fwd /= np.linalg.norm(fwd)
    # Straight-down / straight-up views make world-up parallel to fwd, which degenerates the
    # cross product. Fall back to the world Z axis as the reference for those.
    ref = np.array([0.0, 1.0, 0.0])
    if abs(float(fwd @ ref)) > 0.999:
        ref = np.array([0.0, 0.0, 1.0])
    # right = forward x up, NOT up x forward.
    #
    # This is not a free choice. The game's own stored camera Transform12 gives its basis rows
    # directly, and for a real captured frame row0 = (-0.9655, 0.0000, 0.2604) while
    # cross(fwd, up) reproduces it EXACTLY (dot = +1.0000) and cross(up, fwd) gives precisely the
    # negative (dot = -1.0000). Using the latter renders a horizontally MIRRORED image -- which is
    # what these turntables did until 2026-09-04, visible as reversed lettering on the tank decal.
    right = np.cross(fwd, ref)
    right /= np.linalg.norm(right)
    up = np.cross(right, fwd)
    up /= np.linalg.norm(up)
    return eye, fwd, right, up


def fit_distance(verts, target, yaw_deg, pitch_deg, fov_deg, W, H, fill=0.86, start=10.0):
    """Smallest orbit distance at which EVERY vertex projects inside `fill` of the frame.

    Projected extent is not linear in distance, so this iterates the obvious correction a few
    times; it converges in 2-3 rounds for a compact model. Without this the side views of a
    6.7-unit-deep tank get clipped at a fixed distance tuned on the narrower front view -- which
    is exactly what happened on the first turntable pass.
    """
    d = start
    f = 1.0 / math.tan(math.radians(fov_deg) * 0.5)
    aspect = W / float(H)
    for _ in range(24):
        eye, fwd, right, up = camera_basis(target, yaw_deg, pitch_deg, d)
        rel = verts - eye
        vz = rel @ fwd
        if np.min(vz) <= 0.05:
            d *= 1.5
            continue
        ndc_x = np.abs((rel @ right) * f / aspect / vz)
        ndc_y = np.abs((rel @ up) * f / vz)
        worst = max(float(np.max(ndc_x)), float(np.max(ndc_y)))
        if abs(worst - fill) < 0.005:
            break
        d *= (worst / fill) ** 0.85  # damped, since extent falls off slower than 1/d
    return d


def render(verts, uvs, tris, mtl, W, H, yaw_deg, pitch_deg, dist, target, fov_deg,
           shade, bg, texcache):
    """Orbit-camera convenience wrapper around render_from_basis()."""
    eye, fwd, right, up = camera_basis(target, yaw_deg, pitch_deg, dist)
    return render_from_basis(verts, uvs, tris, mtl, W, H, eye, fwd, right, up, fov_deg,
                             shade, bg, texcache)



def _clip_near_plane(verts, uvs, tris, eye, fwd, near):
    """Split every triangle that crosses the near plane, returning new (verts, uvs, tris).

    Rejecting such a triangle outright is invisible on small props and catastrophic on large ones:
    map 1's ground tiles are 256x256 world units, so standing on one dropped the whole tile and
    left a black void under the camera. That is exactly what the fidelity comparison against real
    gameplay showed before this existed. (The C++ renderer received the same fix first; this
    rasteriser kept the old behaviour and silently under-drew every ground-level frame.)

    Sutherland-Hodgman against the single near plane, fan-triangulated. Interpolation is linear
    because clipping happens BEFORE the perspective divide, where linear is correct.
    """
    depth = (verts - eye) @ fwd
    if not (depth <= near).any():
        return verts, uvs, tris

    out_v = list(map(tuple, verts))
    out_uv = list(map(tuple, uvs)) if len(uvs) else []
    out_tris = []
    for (i0, i1, i2, t0, t1, t2, matname) in tris:
        d = (depth[i0], depth[i1], depth[i2])
        if d[0] > near and d[1] > near and d[2] > near:
            out_tris.append((i0, i1, i2, t0, t1, t2, matname))
            continue
        if d[0] <= near and d[1] <= near and d[2] <= near:
            continue                                   # entirely behind
        vi = (i0, i1, i2)
        ti = (t0, t1, t2)
        poly = []
        for k in range(3):
            j = (k + 1) % 3
            dk, dj = d[k] - near, d[j] - near
            if dk >= 0:
                poly.append((vi[k], ti[k], None))
            if (dk >= 0) != (dj >= 0):
                denom = dk - dj
                f = (dk / denom) if denom != 0.0 else 0.0
                pk = np.asarray(verts[vi[k]], dtype=np.float64)
                pj = np.asarray(verts[vi[j]], dtype=np.float64)
                out_v.append(tuple(pk + (pj - pk) * f))
                new_vi = len(out_v) - 1
                new_ti = -1
                if out_uv and ti[k] >= 0 and ti[j] >= 0:
                    uk = np.asarray(uvs[ti[k]], dtype=np.float64)
                    uj = np.asarray(uvs[ti[j]], dtype=np.float64)
                    out_uv.append(tuple(uk + (uj - uk) * f))
                    new_ti = len(out_uv) - 1
                poly.append((new_vi, new_ti, None))
        if len(poly) < 3:
            continue
        for k in range(1, len(poly) - 1):
            out_tris.append((poly[0][0], poly[k][0], poly[k + 1][0],
                             poly[0][1], poly[k][1], poly[k + 1][1], matname))

    nv = np.array(out_v, dtype=np.float64)
    nu = np.array(out_uv, dtype=np.float64) if out_uv else np.zeros((0, 2))
    return nv, nu, out_tris


def render_from_basis(verts, uvs, tris, mtl, W, H, eye, fwd, right, up, fov_deg,
                      shade, bg, texcache, bg_image=None, return_mask=False,
                      fov_x_deg=None, cy_frac=0.5):
    """`fov_x_deg` and `cy_frac` expose a GENERAL pinhole instead of the usual
    aspect-derived one.

    Needed because vertical FOV and viewport height scale the image identically, so fitting the
    original's projection by image correlation alone finds a ridge, not a peak. Horizontal scale
    depends only on the horizontal FOV, so fitting the two axes separately breaks the degeneracy.
    `cy_frac` is the principal point's height as a fraction of the image (0.5 = centred); the
    original draws its 3D view into the upper part of the frame with the HUD below, which moves
    it off centre."""
    """Rasterise from an EXPLICIT eye + orthonormal basis.

    Same rasteriser and same confirmed render state as render(); the only difference is that
    the camera is given rather than derived from an orbit. This is what lets a camera transform
    read out of the live game be reproduced exactly.
    """
    NEAR = 0.05
    # Clip slightly BEYOND the rasteriser's own near test. Clipping exactly at NEAR puts the new
    # vertices at depth == NEAR, which the `vz <= NEAR` reject below then throws away — silently
    # discarding precisely the triangles the clipper just produced, so the clip appeared to do
    # nothing and the ground under the camera stayed black.
    verts, uvs, tris = _clip_near_plane(verts, uvs, tris, eye, fwd, NEAR * 1.05)

    # `bg_image` lets a caller rasterise this geometry directly ON TOP of an already-rendered
    # background (e.g. the sky), instead of compositing afterwards by guessing which pixels were
    # painted. Guessing is what a brightness threshold does, and it is wrong: dark interiors have
    # real wall pixels below any sensible threshold, so the background bled through them and
    # produced convincing "tears" along the walls that were entirely an artifact of the tool. The
    # geometry was correct the whole time (verified 2026-09-04 by rendering world-only, which
    # matched the real frame exactly).
    if bg_image is not None:
        color = np.asarray(bg_image, dtype=np.float64) / 255.0
        if color.shape[:2] != (H, W):
            raise ValueError("bg_image is %dx%d, expected %dx%d"
                             % (color.shape[1], color.shape[0], W, H))
        color = color.copy()
    else:
        color = np.tile(np.array(bg, dtype=np.float64), (H, W, 1))
    covered = np.zeros((H, W), dtype=bool)
    depth = np.full((H, W), np.inf)

    rel = verts - eye
    vx = rel @ right
    vy = rel @ up
    vz = rel @ fwd

    f = 1.0 / math.tan(math.radians(fov_deg) * 0.5)
    if fov_x_deg is None:
        aspect = W / float(H)
        fx = f / aspect
    else:
        fx = 1.0 / math.tan(math.radians(fov_x_deg) * 0.5)
    sx = (vx * fx)
    sy = (vy * f)

    cy_px = H * cy_frac

    def project(i):
        z = vz[i]
        return (W * 0.5 * (1.0 + sx[i] / z), cy_px - H * 0.5 * (sy[i] / z), z)

    # Group triangles by material so each texture is fetched once.
    order = {}
    for t in tris:
        order.setdefault(t[6], []).append(t)

    for matname, group in order.items():
        md = mtl.get(matname, {"Kd": (0.5, 0.5, 0.5), "map": None})
        tex = load_texture(md["map"], texcache) if md["map"] else None
        kd = np.array(md["Kd"], dtype=np.float64)
        # Material alpha (MaterialRecord+0x00, exported as OBJ 'd'). 0 = never drawn.
        mat_alpha = float(md.get("d", 1.0))
        if mat_alpha <= 0.002:
            continue

        for (i0, i1, i2, t0, t1, t2, _) in group:
            # Triangles that survive here are entirely in front: geometry crossing the near
            # plane was already split by _clip_near_plane() before rasterising (see there for why
            # whole-triangle rejection is not an option on 256-unit ground tiles).
            if vz[i0] <= NEAR or vz[i1] <= NEAR or vz[i2] <= NEAR:
                continue
            p0 = project(i0)
            p1 = project(i1)
            p2 = project(i2)

            minx = max(int(math.floor(min(p0[0], p1[0], p2[0]))), 0)
            maxx = min(int(math.ceil(max(p0[0], p1[0], p2[0]))), W - 1)
            miny = max(int(math.floor(min(p0[1], p1[1], p2[1]))), 0)
            maxy = min(int(math.ceil(max(p0[1], p1[1], p2[1]))), H - 1)
            if minx > maxx or miny > maxy:
                continue

            area = (p1[0] - p0[0]) * (p2[1] - p0[1]) - (p2[0] - p0[0]) * (p1[1] - p0[1])
            if abs(area) < 1e-12:
                continue
            # CULLMODE = NONE: both windings are drawn. Normalise the sign instead of rejecting.

            xs = np.arange(minx, maxx + 1)
            ys = np.arange(miny, maxy + 1)
            px, py = np.meshgrid(xs + 0.5, ys + 0.5)

            w0 = ((p1[0] - px) * (p2[1] - py) - (p2[0] - px) * (p1[1] - py)) / area
            w1 = ((p2[0] - px) * (p0[1] - py) - (p0[0] - px) * (p2[1] - py)) / area
            w2 = 1.0 - w0 - w1
            inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
            if not inside.any():
                continue

            # Perspective-correct interpolation.
            iw0, iw1, iw2 = w0 / p0[2], w1 / p1[2], w2 / p2[2]
            wsum = iw0 + iw1 + iw2
            z = 1.0 / np.where(wsum == 0, 1e-12, wsum)

            sub = depth[miny:maxy + 1, minx:maxx + 1]
            # ZFUNC = LESS_EQUAL
            hit = inside & (z <= sub)
            if not hit.any():
                continue

            alpha = None
            if tex is not None and t0 >= 0:
                u = (uvs[t0][0] * iw0 + uvs[t1][0] * iw1 + uvs[t2][0] * iw2) * z
                v = (uvs[t0][1] * iw0 + uvs[t1][1] * iw1 + uvs[t2][1] * iw2) * z
                rgba = sample_bilinear(tex, u, v)
                rgb = rgba[..., :3] * kd  # kd is white for textured materials
                if tex.shape[2] == 4:
                    alpha = rgba[..., 3]
            else:
                rgb = np.broadcast_to(kd, px.shape + (3,)).copy()
                if mat_alpha < 0.998:
                    alpha = np.full(px.shape, mat_alpha)

            if shade:
                # LABELLED NON-ORIGINAL: a geometric depth cue only, so the flat unlit
                # result is readable as a 3D form. Not a claim about original lighting.
                e0 = np.array([verts[i1][k] - verts[i0][k] for k in range(3)])
                e1 = np.array([verts[i2][k] - verts[i0][k] for k in range(3)])
                nrm = np.cross(e0, e1)
                ln = np.linalg.norm(nrm)
                if ln > 1e-12:
                    nrm /= ln
                    ndl = abs(float(nrm @ np.array([0.35, 0.86, 0.37]) / 1.0))
                    rgb = rgb * (0.55 + 0.45 * min(ndl, 1.0))

            csub = color[miny:maxy + 1, minx:maxx + 1]
            if alpha is not None:
                # SRCALPHA / INVSRCALPHA, the confirmed original blend. Fully transparent texels
                # must not write depth either, or they punch holes in whatever is behind them.
                opaque_enough = hit & (alpha > 0.004)
                if opaque_enough.any():
                    aa = alpha[opaque_enough][:, None]
                    csub[opaque_enough] = rgb[opaque_enough] * aa + csub[opaque_enough] * (1.0 - aa)
                    solid = hit & (alpha > 0.5)
                    sub[solid] = z[solid]
                    covered[miny:maxy + 1, minx:maxx + 1][opaque_enough] = True
            else:
                csub[hit] = rgb[hit] if rgb.ndim == 3 else rgb
                sub[hit] = z[hit]
                covered[miny:maxy + 1, minx:maxx + 1][hit] = True

    out = (np.clip(color, 0, 1) * 255).astype(np.uint8)
    return (out, covered) if return_mask else out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--obj", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--width", type=int, default=900)
    ap.add_argument("--height", type=int, default=700)
    ap.add_argument("--dist", type=float, default=0.0,
                    help="fixed orbit distance; 0 (default) = auto-fit per view")
    ap.add_argument("--fill", type=float, default=0.86,
                    help="fraction of the frame the model should fill when auto-fitting")
    ap.add_argument("--fov", type=float, default=42.0)
    ap.add_argument("--frames", type=int, default=24, help="turntable frames over 360 deg")
    ap.add_argument("--pitch", type=float, default=18.0)
    ap.add_argument("--prefix", default="bft")
    ap.add_argument("--shade", dest="shade", action="store_true", default=True)
    ap.add_argument("--no-shade", dest="shade", action="store_false")
    args = ap.parse_args()

    verts, uvs, tris, mtl = load_obj(args.obj)
    print("loaded %d verts, %d tris, %d materials" % (len(verts), len(tris), len(mtl)))
    os.makedirs(args.out, exist_ok=True)
    texcache = {}
    bg = (0.13, 0.14, 0.16)
    pre = args.prefix

    # Orbit around the model's own bbox centre, not a guessed aim height -- that is what keeps
    # top and bottom views centred as well as the horizontal ones.
    lo = verts.min(axis=0)
    hi = verts.max(axis=0)
    target = (lo + hi) * 0.5
    radius = float(np.linalg.norm(hi - lo)) * 0.5
    print("bbox %s .. %s, centre %s" % (np.round(lo, 3), np.round(hi, 3), np.round(target, 3)))

    def shoot(yaw, pitch):
        d = args.dist if args.dist > 0 else fit_distance(
            verts, target, yaw, pitch, args.fov, args.width, args.height,
            fill=args.fill, start=max(radius * 3.0, 1.0))
        return render(verts, uvs, tris, mtl, args.width, args.height, yaw, pitch,
                      d, target, args.fov, args.shade, bg, texcache), d

    # Named canonical views. Orbit convention as in reference/bft_handbuilt/README.md
    # (yaw 0 = behind). Top/bottom use 89 deg rather than 90 to keep a stable up vector.
    named = [
        ("front", 180, 14), ("rear", 0, 14), ("left", 90, 14), ("right", 270, 14),
        ("top", 0, 89), ("bottom", 0, -89),
        ("threequarter", 40, 25), ("front_threequarter", 215, 22),
        ("top_threequarter", 40, 55),
    ]
    for name, yaw, pitch in named:
        img, d = shoot(yaw, pitch)
        p = os.path.join(args.out, "%s_%s.png" % (pre, name))
        Image.fromarray(img).save(p)
        print("wrote %s  (yaw %g, pitch %g, dist %.2f)" % (p, yaw, pitch, d))

    # Turntable: one distance for the whole sweep (the worst-case view's), so the model does not
    # visibly breathe in and out as it rotates.
    tdir = os.path.join(args.out, "turntable")
    os.makedirs(tdir, exist_ok=True)
    if args.dist > 0:
        turn_d = args.dist
    else:
        turn_d = max(fit_distance(verts, target, 360.0 * i / 8, args.pitch, args.fov,
                                  args.width, args.height, fill=args.fill,
                                  start=max(radius * 3.0, 1.0)) for i in range(8))
    print("turntable distance %.2f (constant)" % turn_d)
    frames = []
    for i in range(args.frames):
        yaw = 360.0 * i / args.frames
        img = render(verts, uvs, tris, mtl, args.width, args.height, yaw, args.pitch,
                     turn_d, target, args.fov, args.shade, bg, texcache)
        p = os.path.join(tdir, "turn_%02d.png" % i)
        Image.fromarray(img).save(p)
        frames.append(Image.fromarray(img))
    print("wrote %d turntable frames" % len(frames))

    if frames:
        gif = os.path.join(args.out, "%s_turntable.gif" % pre)
        small = [f.convert("RGB").resize((660, int(660 * args.height / args.width)),
                                          Image.LANCZOS).quantize(colors=128)
                 for f in frames]
        small[0].save(gif, save_all=True, append_images=small[1:], duration=110, loop=0,
                      optimize=True)
        print("wrote %s (%.1f MB)" % (gif, os.path.getsize(gif) / 1e6))

    # Contact sheet: 8 views, 4x2, each labelled. Padding is added around every tile so nothing
    # sits against an edge even at the auto-fit fill fraction.
    sheet_views = ["front", "rear", "left", "right",
                   "top", "bottom", "threequarter", "front_threequarter"]
    ims = [Image.open(os.path.join(args.out, "%s_%s.png" % (pre, n))) for n in sheet_views]
    tw, th = ims[0].size
    sw, sh = int(tw * 0.55), int(th * 0.55)
    pad = 10
    label_h = 26
    cw, ch = sw + pad * 2, sh + pad * 2 + label_h
    sheet = Image.new("RGB", (cw * 4, ch * 2), (20, 21, 24))
    try:
        from PIL import ImageDraw, ImageFont
        draw = ImageDraw.Draw(sheet)
        try:
            font = ImageFont.truetype("segoeui.ttf", 17)
        except Exception:
            font = ImageFont.load_default()
    except Exception:
        draw = None
    for i, im in enumerate(ims):
        cx, cy = (i % 4) * cw, (i // 4) * ch
        sheet.paste(im.resize((sw, sh), Image.LANCZOS), (cx + pad, cy + pad))
        if draw is not None:
            draw.text((cx + pad + 2, cy + pad + sh + 4),
                      sheet_views[i].replace("_", " "), fill=(168, 172, 178), font=font)
    sp = os.path.join(args.out, "%s_contact_sheet.png" % pre)
    sheet.save(sp)
    print("wrote", sp)
    return 0


if __name__ == "__main__":
    sys.exit(main())

