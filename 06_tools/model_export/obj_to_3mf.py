#!/usr/bin/env python3
"""Convert an exported OBJ+MTL+textures model into a 3MF package.

3MF is an OPC (zip) container:

    [Content_Types].xml
    _rels/.rels                     -> points at the model part
    3D/3dmodel.model                -> the XML mesh + resources
    3D/_rels/3dmodel.model.rels     -> one relationship per texture part
    3D/Textures/*.png

Textures use the 3MF **Materials and Properties** extension (`texture2d` + `texture2dgroup`),
which Windows 3D Viewer / 3D Builder understand. The extension is deliberately NOT listed in
`requiredextensions`, so a viewer that only speaks core 3MF still loads the geometry and the
per-material `displaycolor` instead of refusing the file.

Vertex positions are welded (per-corner UVs are unaffected -- in 3MF the texture coordinates live
in a separate indexed group, not on the vertex, so welding costs nothing here, unlike in OBJ).

Two honest caveats, both inherent to the source data rather than this converter:

  * The mesh is **not manifold**. The original renders with `CULLMODE = NONE` and uses
    single-sided polygons, so there is no closed solid to recover. Viewers display it fine;
    a slicer will want to run its repair pass before printing.
  * Units are declared as **metre**, 1 unit = 1 m, matching the game's own world scale. For
    printing, scale in the slicer, or re-run with `--unit millimeter --scale 25`.

Usage:
    python obj_to_3mf.py --obj ../../reference/bft_model/bft.obj \
                         --out ../../reference/bft_model/bft.3mf
"""

import argparse
import os
import sys
import zipfile
from xml.sax.saxutils import escape

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from render_turntable import load_obj  # same OBJ/MTL reader, so the two never diverge

CORE_NS = "http://schemas.microsoft.com/3dmanufacturing/core/2015/02"
MAT_NS = "http://schemas.microsoft.com/3dmanufacturing/material/2015/02"
REL_MODEL = "http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"
REL_TEXTURE = "http://schemas.microsoft.com/3dmanufacturing/2013/01/3dtexture"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--obj", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--unit", default="meter",
                    choices=["micron", "millimeter", "centimeter", "inch", "foot", "meter"])
    ap.add_argument("--scale", type=float, default=1.0)
    ap.add_argument("--name", default="Recoil BFT tank")
    args = ap.parse_args()

    verts, uvs, tris, mtl = load_obj(args.obj)
    print("loaded %d verts, %d tris, %d materials" % (len(verts), len(tris), len(mtl)))

    # ---- weld positions -------------------------------------------------------------
    weld = {}
    out_verts = []
    remap = [0] * len(verts)
    for i, v in enumerate(verts):
        key = (round(float(v[0]) * 1e5), round(float(v[1]) * 1e5), round(float(v[2]) * 1e5))
        j = weld.get(key)
        if j is None:
            j = len(out_verts)
            weld[key] = j
            out_verts.append((float(v[0]), float(v[1]), float(v[2])))
        remap[i] = j
    print("welded %d -> %d vertices" % (len(verts), len(out_verts)))

    # ---- group triangles by material ------------------------------------------------
    groups = {}
    for t in tris:
        groups.setdefault(t[6], []).append(t)
    matnames = sorted(groups.keys(), key=lambda n: (n is None, n))

    # ---- allocate resource ids (must be unique across ALL resource types) ------------
    next_id = 1
    tex_id = {}        # texture file path -> texture2d id
    texgroup_id = {}   # material name -> texture2dgroup id
    texgroup_uv = {}   # material name -> list of (u, v)
    tex_rel = {}       # texture file path -> (zip path, relationship id)

    objdir = os.path.dirname(os.path.abspath(args.obj))
    for name in matnames:
        md = mtl.get(name)
        if not md or not md.get("map"):
            continue
        src = md["map"]
        if not os.path.isfile(src):
            print("  WARNING: texture missing, falling back to colour: %s" % src)
            continue
        if src not in tex_id:
            tex_id[src] = next_id
            next_id += 1
            base = os.path.basename(src)
            tex_rel[src] = ("3D/Textures/" + base, "rTex%d" % tex_id[src])

    base_id = next_id
    next_id += 1

    for name in matnames:
        md = mtl.get(name)
        if not md or not md.get("map") or md["map"] not in tex_id:
            continue
        texgroup_id[name] = next_id
        next_id += 1
        coords = []
        for (_, _, _, t0, t1, t2, _) in groups[name]:
            for t in (t0, t1, t2):
                if t >= 0 and t < len(uvs):
                    coords.append((float(uvs[t][0]), float(uvs[t][1])))
                else:
                    coords.append((0.0, 0.0))
        texgroup_uv[name] = coords

    object_id = next_id

    # ---- build 3dmodel.model --------------------------------------------------------
    L = []
    L.append('<?xml version="1.0" encoding="UTF-8"?>')
    L.append('<model unit="%s" xml:lang="en-US" xmlns="%s" xmlns:m="%s">'
             % (args.unit, CORE_NS, MAT_NS))
    L.append('<metadata name="Title">%s</metadata>' % escape(args.name))
    L.append('<metadata name="Designer">Zipper Interactive (1999) - extracted from gamez.zbd'
             '</metadata>')
    L.append('<metadata name="Description">Real extracted geometry, per-triangle materials and '
             'textures. Non-manifold: the source renders with backface culling disabled.'
             '</metadata>')
    L.append("<resources>")

    # Base materials: one per OBJ material, in `matnames` order, so pindex == position.
    L.append('<basematerials id="%d">' % base_id)
    for name in matnames:
        md = mtl.get(name, {"Kd": (0.5, 0.5, 0.5)})
        r, g, b = md["Kd"]
        L.append('<base name="%s" displaycolor="#%02X%02X%02XFF"/>'
                 % (escape(name or "mat_none"),
                    int(round(max(0.0, min(1.0, r)) * 255)),
                    int(round(max(0.0, min(1.0, g)) * 255)),
                    int(round(max(0.0, min(1.0, b)) * 255))))
    L.append("</basematerials>")

    for src, tid in sorted(tex_id.items(), key=lambda kv: kv[1]):
        L.append('<m:texture2d id="%d" path="/%s" contenttype="image/png" '
                 'tilestyleu="wrap" tilestylev="wrap" filter="linear"/>'
                 % (tid, tex_rel[src][0]))


    for name in matnames:
        gid = texgroup_id.get(name)
        if gid is None:
            continue
        L.append('<m:texture2dgroup id="%d" texid="%d">' % (gid, tex_id[mtl[name]["map"]]))
        for (u, v) in texgroup_uv[name]:
            L.append('<m:tex2coord u="%.6f" v="%.6f"/>' % (u, v))
        L.append("</m:texture2dgroup>")

    L.append('<object id="%d" type="model" pid="%d" pindex="0" name="%s">'
             % (object_id, base_id, escape(args.name)))
    L.append("<mesh>")
    L.append("<vertices>")
    s = args.scale
    for (x, y, z) in out_verts:
        L.append('<vertex x="%.6f" y="%.6f" z="%.6f"/>' % (x * s, y * s, z * s))
    L.append("</vertices>")
    L.append("<triangles>")
    degenerate = 0
    for mi, name in enumerate(matnames):
        gid = texgroup_id.get(name)
        corner = 0
        for (i0, i1, i2, _, _, _, _) in groups[name]:
            a, b, c = remap[i0], remap[i1], remap[i2]
            if a == b or b == c or a == c:
                # Welding can collapse a sliver triangle; 3MF requires three distinct indices.
                degenerate += 1
                corner += 3
                continue
            if gid is not None:
                L.append('<triangle v1="%d" v2="%d" v3="%d" pid="%d" p1="%d" p2="%d" p3="%d"/>'
                         % (a, b, c, gid, corner, corner + 1, corner + 2))
            else:
                L.append('<triangle v1="%d" v2="%d" v3="%d" pid="%d" p1="%d"/>'
                         % (a, b, c, base_id, mi))
            corner += 3
    L.append("</triangles>")
    L.append("</mesh>")
    L.append("</object>")
    L.append("</resources>")
    L.append('<build><item objectid="%d"/></build>' % object_id)
    L.append("</model>")
    model_xml = "\n".join(L)
    if degenerate:
        print("  dropped %d triangle(s) that welding made degenerate" % degenerate)

    # ---- package --------------------------------------------------------------------
    ct = ['<?xml version="1.0" encoding="UTF-8"?>',
          '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">',
          '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
          'relationships+xml"/>',
          '<Default Extension="model" ContentType="application/vnd.ms-package.'
          '3dmanufacturing-3dmodel+xml"/>',
          '<Default Extension="png" ContentType="image/png"/>',
          "</Types>"]

    rels = ['<?xml version="1.0" encoding="UTF-8"?>',
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">',
            '<Relationship Target="/3D/3dmodel.model" Id="rel0" Type="%s"/>' % REL_MODEL,
            "</Relationships>"]

    mrels = ['<?xml version="1.0" encoding="UTF-8"?>',
             '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">']
    for src, (zpath, rid) in sorted(tex_rel.items(), key=lambda kv: kv[1][0]):
        mrels.append('<Relationship Target="/%s" Id="%s" Type="%s"/>' % (zpath, rid, REL_TEXTURE))
    mrels.append("</Relationships>")

    os.makedirs(os.path.dirname(os.path.abspath(args.out)) or ".", exist_ok=True)
    with zipfile.ZipFile(args.out, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", "\n".join(ct))
        z.writestr("_rels/.rels", "\n".join(rels))
        z.writestr("3D/3dmodel.model", model_xml)
        z.writestr("3D/_rels/3dmodel.model.rels", "\n".join(mrels))
        for src, (zpath, _) in tex_rel.items():
            z.write(src, zpath)

    print("wrote %s (%.1f KB, %d textures, %d materials, unit=%s, scale=%g)"
          % (args.out, os.path.getsize(args.out) / 1024.0, len(tex_rel), len(matnames),
             args.unit, args.scale))
    return 0


if __name__ == "__main__":
    sys.exit(main())
