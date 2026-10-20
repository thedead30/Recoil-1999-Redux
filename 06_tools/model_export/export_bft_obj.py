#!/usr/bin/env python3
"""Export the REAL 'bft' player tank (default loadout: standard cannon) as a standalone
OBJ + MTL + PNG textures, straight from the exported mission-1 asset data.

This is not a hand-assembled model. Every vertex, UV, material assignment and part
transform comes from the real extracted gamez.zbd data, resolved by the rules in
ASSET_RESOLUTION.md:

  node -> mesh        : extras.model_index map (sec. 2), NOT the glTF array position
  mesh -> material    : extras.per_triangle_material_index (sec. 3), per TRIANGLE
  material -> texture : m1_materials.json, highest tier first (sec. 4.1)
  node -> world xform : compose scale/rotation/translation down the parent chain (sec. 5)

Exclusions (each deliberate, matching the runtime renderer):
  - 'skin'          : the nanite-regeneration wave effect, not tank geometry
  - 'track_shadow'  : a ground decal needing alpha blending, not part of the model
  - non-default weapon meshes under the gun subtree (mutually alternate loadouts).
    The default loadout kept here is the standard cannon plus its scroll/mount parts.
    Pass --all-weapons to include every alternate anyway.

Usage:
    python export_bft_obj.py [--out DIR] [--all-weapons] [--node NAME]
"""

import argparse
import json
import os
import shutil
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REMAKE = os.path.abspath(os.path.join(HERE, "..", ".."))
def _paths(mission):
    geom = os.path.join(REMAKE, "tools", "asset_pipeline", "output", mission, "geometry")
    texroot = os.path.join(REMAKE, "tools", "asset_pipeline", "output", mission, "textures")
    return (geom, texroot,
            os.path.join(geom, mission + "_gamez.gltf"),
            os.path.join(geom, mission + "_gamez.bin"),
            os.path.join(geom, mission + "_scene_nodes.json"),
            os.path.join(geom, mission + "_materials.json"))


# Defaults; --mission rebinds these. Kept as module globals because find_texture() reads TEXROOT.
GEOM, TEXROOT, GLTF, BIN, NODES, MATS = _paths("m1")

# ASSET_RESOLUTION.md sec. 4.1 - prefer highest detail first.
TEX_TIERS = ["texture6", "texture4", "texture2", "rtexture4", "rtexture2"]

# Nodes excluded from the exported model (see module docstring).
#
# "horizon" is the SKY SHELL, not world geometry: a 5120 x 977.7 x 5120 box (larger than the
# 4096 x 4352 map) whose three materials are m1sky1, RGB(8,24,34) night sky and RGB(169,193,163)
# horizon haze -- all three visible in real gameplay as the sky and the green horizon band. It
# is the only node in mission 1 whose active_flags (0x84) lack BOTH bit 0x08 and bit 0x10,
# which 1,604 and 1,775 other mesh-bearing nodes respectively carry. Drawn as ordinary
# depth-tested geometry it sits ~30 units in front of the camera and occludes the entire scene,
# so a static export must leave it out; a runtime must draw it first, behind everything, in the
# usual skybox manner. GAP: exactly how the original positions it per frame is not confirmed.
#
# "destroyed" is the WRECKED state of a destructible object. Map 1 pairs it with a "healthy"
# sibling under a common parent — 89 such pairs — and the two are mutually exclusive: the engine
# shows one or the other according to the entity's live health. `active_flags` does NOT choose
# between them (only 6 of the 89 pairs differ), so the choice is runtime state, not static data.
# For a static export the correct state is mission start, i.e. everything intact.
#
# This matters a lot: 471 mesh-bearing nodes hang under "destroyed" subtrees in map 1, about 29%
# of everything being drawn. Rendering both put wrecked geometry inside intact geometry — it
# showed up in the fidelity comparison as a destroyed pop-up turret (ptur_d02/ptur_d03) drawn on
# top of the intact one (ptur01..ptur09).
#
# "bft_00" is a STATIC placed copy of the player's tank sitting at the mission spawn. The player's
# own vehicle is drawn from the "bft" template at its live transform, so leaving bft_00 in a world
# export puts a second motionless tank in the same spot -- and, worse, puts the tank's own hull
# into the ground-collision soup, so the player spawns resting on a copy of itself 2.5 units up.
EXCLUDE_NAMES = {"skin", "track_shadow", "horizon", "destroyed", "bft_00"}

# The default loadout's weapon meshes. Names as they appear in the real bft subtree.
DEFAULT_WEAPON_MESHES = {"RFPG_R", "RFPG_L", "RFPG_RSCROLL", "RFPG_LSCROLL", "HEM"}


# ---------------------------------------------------------------- transforms

def identity():
    # Transform12 convention: 3x3 row-basis rotation + translation.
    return ([1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0], [0.0, 0.0, 0.0])


def local_of(node):
    o = node.get("object3d")
    if not o:
        return identity()
    r = list(o["local_rotation_3x3"])
    t = list(o["local_translation"])
    s = o.get("scale") or [1.0, 1.0, 1.0]
    if s != [1.0, 1.0, 1.0]:
        # Scale is real data; apply it by scaling the basis rows.
        for row in range(3):
            for col in range(3):
                r[row * 3 + col] *= s[row]
    return (r, t)


def mat_mul(parent, child):
    """world = parent o child, row-basis: v_world = v * Rc * Rp + Tc * Rp + Tp."""
    pr, pt = parent
    cr, ct = child
    out_r = [0.0] * 9
    for i in range(3):
        for j in range(3):
            acc = 0.0
            for k in range(3):
                acc += cr[i * 3 + k] * pr[k * 3 + j]
            out_r[i * 3 + j] = acc
    out_t = [0.0] * 3
    for j in range(3):
        acc = pt[j]
        for k in range(3):
            acc += ct[k] * pr[k * 3 + j]
        out_t[j] = acc
    return (out_r, out_t)


def xform_point(m, p):
    r, t = m
    return (
        p[0] * r[0] + p[1] * r[3] + p[2] * r[6] + t[0],
        p[0] * r[1] + p[1] * r[4] + p[2] * r[7] + t[1],
        p[0] * r[2] + p[1] * r[5] + p[2] * r[8] + t[2],
    )


# ---------------------------------------------------------------- gltf access

class Gltf:
    def __init__(self, gltf_path, bin_path):
        with open(gltf_path, "r", encoding="utf-8") as f:
            self.doc = json.load(f)
        with open(bin_path, "rb") as f:
            self.bin = f.read()
        # ASSET_RESOLUTION.md sec. 2: model_index -> glTF meshes[] position.
        self.by_model = {}
        for i, mesh in enumerate(self.doc["meshes"]):
            for prim in mesh["primitives"]:
                mi = prim.get("extras", {}).get("model_index")
                if mi is not None:
                    self.by_model[mi] = i

    def read_accessor(self, idx):
        acc = self.doc["accessors"][idx]
        bv = self.doc["bufferViews"][acc["bufferView"]]
        ncomp = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}[acc["type"]]
        assert acc["componentType"] == 5126, "expected float accessors"
        base = bv.get("byteOffset", 0) + acc.get("byteOffset", 0)
        n = acc["count"] * ncomp
        vals = struct.unpack_from("<%df" % n, self.bin, base)
        return [vals[i * ncomp:(i + 1) * ncomp] for i in range(acc["count"])]

    def primitive_for_model(self, model_index):
        gi = self.by_model.get(model_index)
        if gi is None:
            return None
        return self.doc["meshes"][gi]["primitives"][0]


# ---------------------------------------------------------------- textures

def find_texture(name):
    for tier in TEX_TIERS:
        p = os.path.join(TEXROOT, tier, name + ".png")
        if os.path.isfile(p):
            return p, tier
    return None, None


# ---------------------------------------------------------------- main

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(REMAKE, "reference", "bft_model"))
    ap.add_argument("--all-weapons", action="store_true")
    ap.add_argument("--node", default="bft")
    ap.add_argument("--mission", default="m1",
                    help="which exported mission to read (m1..m13). The resolution rules are "
                         "mission-independent; this is the switch that proves it.")
    ap.add_argument("--world", action="store_true",
                    help="export the FULL drawable set: the root subtree UNION the world_grid "
                         "members (ASSET_RESOLUTION.md sec. 2b). This is what the runtime "
                         "actually draws.")
    ap.add_argument("--grid", action="store_true",
                    help="export the STATIC WORLD instead: every node referenced by the "
                         "world_grid area-partition cells. These nodes are orphans in the "
                         "scene-graph child tree -- the grid is their only attachment point "
                         "-- which is why a root-only traversal renders no ground.")
    args = ap.parse_args()

    global GEOM, TEXROOT, GLTF, BIN, NODES, MATS
    GEOM, TEXROOT, GLTF, BIN, NODES, MATS = _paths(args.mission)
    if not os.path.isfile(NODES):
        print("mission %r has not been exported yet (%s missing) — run "
              "tools/asset_pipeline/python/build_mission_assets.py %s"
              % (args.mission, os.path.basename(NODES), args.mission), file=sys.stderr)
        return 2

    # Exporting a node BY NAME is an explicit request for that node, so it overrides the
    # standing exclusion list (which exists to keep the sky shell out of WORLD exports).
    if not args.world and not args.grid:
        EXCLUDE_NAMES.discard(args.node)

    scene = json.load(open(NODES, "r", encoding="utf-8"))
    nodes = scene["nodes"]
    by_index = {n["index"]: n for n in nodes}

    matdoc = json.load(open(MATS, "r", encoding="utf-8"))
    materials = {m["material_index"]: m for m in matdoc["materials"]}

    g = Gltf(GLTF, BIN)

    root = None
    if args.world:
        members = []
        seen = set()
        for cell in scene["world_grid"]["cells"]:
            for i in cell.get("member_node_indices") or []:
                if i not in seen:
                    seen.add(i); members.append(i)
        # Root subtree first, then any grid member not already reachable from it.
        reach = set()
        stack = [0]
        while stack:
            i = stack.pop()
            if i in reach: continue
            reach.add(i)
            stack.extend(by_index[i].get("children", []))
        extra = [i for i in members if i not in reach]
        print("root subtree reaches %d nodes; grid adds %d more (union %d)"
              % (len(reach), len(extra), len(reach) + len(extra)))
        root = {"name": "__world__", "index": -1, "mesh_index": None,
                "children": list(by_index[0].get("children", [])) + extra, "object3d": None}
    elif args.grid:
        members = []
        seen = set()
        for cell in scene["world_grid"]["cells"]:
            for i in cell.get("member_node_indices") or []:
                if i not in seen:
                    seen.add(i)
                    members.append(i)
        print("world_grid references %d distinct nodes across %d cells"
              % (len(members), len(scene["world_grid"]["cells"])))
        root = {"name": "__world_grid__", "index": -1, "mesh_index": None,
                "children": members, "object3d": None}
    else:
        for n in nodes:
            if n.get("name") == args.node:
                root = n
                break
    if root is None:
        print("node %r not found" % args.node, file=sys.stderr)
        return 2

    outdir = args.out
    texdir = os.path.join(outdir, "textures")
    os.makedirs(texdir, exist_ok=True)

    # Collected geometry, grouped by material index.
    groups = {}          # mat_index -> list of (v0,v1,v2, uv0,uv1,uv2)
    stats = {"nodes": 0, "meshed": 0, "skipped": [], "tris": 0}

    def walk(node, world):
        stats["nodes"] += 1
        name = node.get("name") or ""
        if name in EXCLUDE_NAMES:
            stats["skipped"].append((name, "excluded"))
            return
        world = mat_mul(world, local_of(node))

        mi = node.get("mesh_index")
        if mi is not None:
            prim = g.primitive_for_model(mi)
            if prim is None:
                stats["skipped"].append((name, "model %d not in gltf" % mi))
            else:
                emit(node, prim, world)

        for c in node.get("children", []):
            child = by_index.get(c)
            if child is not None:
                walk(child, world)

    def emit(node, prim, world):
        attrs = prim["attributes"]
        pos = g.read_accessor(attrs["POSITION"])
        uv = g.read_accessor(attrs["TEXCOORD_0"]) if "TEXCOORD_0" in attrs else None
        extras = prim.get("extras", {})
        pertri = extras.get("per_triangle_material_index")
        ntri = len(pos) // 3
        if not pertri:
            single = extras.get("material_index")
            if single is None:
                mset = extras.get("material_indices") or []
                single = mset[0] if len(mset) == 1 else -1
            pertri = [single] * ntri
        stats["meshed"] += 1
        for t in range(ntri):
            m = pertri[t] if t < len(pertri) else -1
            tri_p = [xform_point(world, pos[t * 3 + k]) for k in range(3)]
            tri_uv = [uv[t * 3 + k] for k in range(3)] if uv else None
            groups.setdefault(m, []).append((tri_p, tri_uv))
            stats["tris"] += 1

    # Weapon-subtree filtering: prune alternates unless --all-weapons.
    if not args.all_weapons:
        gun_kept = set()

        def mark_gun(node, in_gun):
            name = node.get("name") or ""
            if name.lower() == "gun":
                in_gun = True
            if in_gun and node.get("mesh_index") is not None:
                if name not in DEFAULT_WEAPON_MESHES:
                    EXCLUDE_NAMES.add(name)
                else:
                    gun_kept.add(name)
            for c in node.get("children", []):
                ch = by_index.get(c)
                if ch is not None:
                    mark_gun(ch, in_gun)

        mark_gun(root, False)
        print("default loadout weapon meshes kept: %s" % sorted(gun_kept))

    walk(root, identity())

    # Write OBJ + MTL.
    base = "world" if args.world else ("world_grid" if args.grid else args.node)
    obj_path = os.path.join(outdir, base + ".obj")
    mtl_path = os.path.join(outdir, base + ".mtl")

    copied = {}
    mtl_lines = ["# Recoil (1999) - real extracted '%s' materials" % base, ""]
    for m in sorted(groups.keys()):
        md = materials.get(m)
        mname = "mat_%d" % m if m >= 0 else "mat_none"
        mtl_lines.append("newmtl %s" % mname)
        mtl_lines.append("Ka 0.000 0.000 0.000")
        mtl_lines.append("Ks 0.000 0.000 0.000")   # SPECULARENABLE = FALSE
        mtl_lines.append("illum 1")
        # 'd' (dissolve) carries the real MaterialRecord+0x00 alpha. 0 means the polygon is
        # genuinely invisible in the original -- seven of mission 1's untextured materials are.
        alpha = (md.get("alpha", 255) if md else 255) / 255.0
        mtl_lines.append("d %.4f" % alpha)
        if md and md.get("textured") and md.get("texture_name"):
            src, tier = find_texture(md["texture_name"])
            mtl_lines.append("Kd 1.000 1.000 1.000")
            if src:
                dst_name = md["texture_name"] + ".png"
                if dst_name not in copied:
                    shutil.copyfile(src, os.path.join(texdir, dst_name))
                    copied[dst_name] = tier
                mtl_lines.append("map_Kd textures/%s" % dst_name)
            else:
                mtl_lines.append("# texture %r not found in any tier" % md["texture_name"])
        else:
            r = (md["color_r"] / 255.0) if md else 0.5
            gg = (md["color_g"] / 255.0) if md else 0.5
            b = (md["color_b"] / 255.0) if md else 0.5
            mtl_lines.append("Kd %.4f %.4f %.4f" % (r, gg, b))
        mtl_lines.append("")

    with open(mtl_path, "w", encoding="utf-8") as f:
        f.write("\n".join(mtl_lines))

    vlines = []
    tlines = []
    flines = []
    vi = 1
    ti = 1
    for m in sorted(groups.keys()):
        mname = "mat_%d" % m if m >= 0 else "mat_none"
        flines.append("g %s" % mname)
        flines.append("usemtl %s" % mname)
        for tri_p, tri_uv in groups[m]:
            idx = []
            for k in range(3):
                p = tri_p[k]
                vlines.append("v %.6f %.6f %.6f" % (p[0], p[1], p[2]))
                if tri_uv:
                    u, v = tri_uv[k][0], tri_uv[k][1]
                    tlines.append("vt %.6f %.6f" % (u, 1.0 - v))  # OBJ V is flipped
                    idx.append("%d/%d" % (vi, ti))
                    ti += 1
                else:
                    idx.append("%d" % vi)
                vi += 1
            flines.append("f %s %s %s" % (idx[0], idx[1], idx[2]))

    with open(obj_path, "w", encoding="utf-8") as f:
        f.write("# Recoil (1999) player tank '%s' - exported from real gamez.zbd data\n" % base)
        f.write("# %d vertices, %d triangles, %d materials\n" % (vi - 1, stats["tris"], len(groups)))
        f.write("mtllib %s.mtl\n" % base)
        f.write("\n".join(vlines))
        f.write("\n")
        f.write("\n".join(tlines))
        f.write("\n")
        f.write("\n".join(flines))
        f.write("\n")

    print("nodes visited      : %d" % stats["nodes"])
    print("mesh-bearing nodes : %d" % stats["meshed"])
    print("triangles          : %d" % stats["tris"])
    print("materials          : %d" % len(groups))
    print("textures copied    : %d (%s)" % (len(copied), ", ".join("%s<-%s" % (k, v) for k, v in sorted(copied.items()))))
    if stats["skipped"]:
        print("skipped            : %s" % ", ".join("%s(%s)" % s for s in stats["skipped"][:20]))
    print("wrote %s" % obj_path)
    print("wrote %s" % mtl_path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
