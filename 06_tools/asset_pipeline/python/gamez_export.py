"""
gamez_export.py — geometry + scene-node export for one mission's gamez.zbd.

This is NOT a new parser. Every byte offset, section order, and struct-layout
fact used below is copied from the RE-phase tools this project already
validated:

  - tools/scripts/parse_gamez.py         (byte-perfect header/material/model3d/
                                           node-buffer walk, full-coverage
                                           validated across all 13,036 real
                                           models in all 13 missions)
  - tools/scripts/export_gamez_obj.py    (the same walk, but *capturing*
                                           vertex/polygon/UV data instead of
                                           discarding it, proven correct via
                                           OBJ export + sanity checks)
  - tools/scripts/validate_gamez_full.py (full-coverage version of the above)

This script's only new work is (a) capturing ALL models in the mission
instead of the first N, (b) also capturing the Object3DBlock fields that
recoil_confirmed_logic.md has confirmed (name/type/scale/localTransform/
bbox/activeFlags/childCount), and (c) writing the result out as glTF 2.0
(geometry) and a plain JSON node list (scene) instead of OBJ-only.

Struct facts relied on (see recoil_confirmed_logic.md for citations):
  - gamez.zbd header: 9x uint32, magic 0x02971222, const 0xf, texCount at [2]
  - texture directory: texCount * 0x24 bytes (opaque here - names not decoded,
    not confirmed field-by-field, NOT guessed)
  - material buffer: header (matCount, field2, field3, matStartIdx) + matCount
    * 0x2c records, linked-list walk for cycling data (flag_byte & 4)
  - model3d buffer: header (modelCount, modelField2, modelField3) + records,
    each record 0x58 bytes with vertCount/normCount/morphCount/pointCount at
    +0x10 and polyCount at +0xc; polygon record is 0x1c bytes, flags & 0xff =
    index count, UV block present iff mat_array[matIndex].flagByte & 1
  - node buffer: GamezNode fixed record 0xc4 bytes (name char[0x24] at +0x00,
    activeFlags at +0x24, nodeType uint32 at +0x34, childCount int32 at +0x5c,
    bboxMin at +0x74, bboxMax at +0x80), nodeCap = header[6]
  - Object3DBlock (node type 5), 0x90 bytes: flags at +0x00, scale[3] at
    +0x24, localTransform (3x3 rotation + translate, 12 floats) at +0x30,
    worldTransform at +0x60 (present in the file but NOT trusted here - the
    confirmed doc's own wording, "written by concatenation, read as parent by
    children", reads as a runtime-computed field; only localTransform is
    exported)

Mesh/hierarchy/material-texture linkage (closed 2026-09-02, see
recoil_confirmed_logic.md's GamezNode/MaterialRecord/TextureDirEntry
sections and recoil_re_log.md's "gamez.zbd mesh/hierarchy/material-texture
linkage" entry for the full GhidraMCP decompile+disassembly evidence and the
real-data range checks - not re-derived here, just applied):

  - node+0x3c ("meshPointer"/meshIndex): ON DISK this is a plain 0-based
    index into the model3d array (-1 = no mesh), confirmed via the node
    loader's resolver call (FUN_004815a0: idx<0 ? 0 : modelArrayBase +
    idx*0x58 - 0x58 matches the model3d record stride). Exported per node
    as "mesh_index" (int, matching a model3d "model_index"/glTF mesh index
    from this SAME mission's geometry export) or null when the raw value is
    negative (no mesh).
  - node+0x5c/+0x60 (childCount/childArray): ON DISK the childCount raw
    dword entries immediately following each node's variable-length tail
    are plain node indices (0-based, into this same flat node array),
    confirmed via FUN_00454bf0's resolver call (FUN_004543d0: idx<0 ? 0 :
    nodeArrayBase + idx*0xc4 - 0xc4 matches the GamezNode record stride).
    Exported per node as "children" (list of int node indices) whenever
    child_count > 0.
  - material+0x10 (texIndex): ON DISK this is a plain 0-based index into
    the texture directory, valid only when the material's flag byte
    (+0x1) has bit 0 set (the same bit already used for the per-polygon
    UV-presence test) - confirmed via the material loader's resolver call
    (FUN_0046d340: idx==-1 ? NULL : texDirBase + idx*0x24 - 0x24 matches
    the confirmed texture-directory record stride). The texture
    directory's own +0x08 field is a null-terminated ASCII base name,
    hex-dump-confirmed to match this pipeline's own mech3ax-extracted PNG
    basenames exactly (e.g. "lflare1.png"). Exported as a separate
    materials list: material_index, textured (bool), texture_index,
    texture_name (base name, no extension - a name can resolve to up to 5
    files across texture2/4/6+rtexture2/4, so the consumer picks a tier).
"""
import json
import os
import struct
import base64

MAGIC = 0x02971222
MAGIC_CONST = 0xF

NODE_TYPE_NAMES = {
    0: "empty", 1: "camera", 2: "world", 3: "window", 4: "display",
    5: "object3d", 6: "lod", 7: "unused7", 8: "unused8", 9: "light", 10: "sound",
}


def _cstr(b):
    nul = b.find(b"\x00")
    return (b[:nul] if nul >= 0 else b).decode("ascii", errors="replace")


def parse_gamez_full(path):
    with open(path, "rb") as f:
        data = f.read()
    pos = 0

    def read(n):
        nonlocal pos
        chunk = data[pos:pos + n]
        if len(chunk) != n:
            raise EOFError(f"wanted {n} bytes at {pos}, got {len(chunk)} (file size {len(data)})")
        pos += n
        return chunk

    # --- header (parse_gamez.py) ---
    header = struct.unpack("<9I", read(0x24))
    assert header[0] == MAGIC, f"bad magic 0x{header[0]:x}"
    assert header[1] == MAGIC_CONST, f"bad const 0x{header[1]:x}"
    texCount = header[2]

    # --- texture directory (TextureDirEntry, recoil_confirmed_logic.md: name
    # is a null-terminated ASCII string at +0x08, hex-dump-confirmed against
    # this pipeline's own mech3ax-extracted PNG basenames) ---
    tex_dir_raw = read(texCount * 0x24)
    texture_names = []
    for ti in range(texCount):
        rec = tex_dir_raw[ti * 0x24:(ti + 1) * 0x24]
        texture_names.append(_cstr(rec[0x08:0x18]))

    # --- material buffer (export_gamez_obj.py), extended to capture the
    # confirmed texIndex field (+0x10, valid when flags bit0 is set) ---
    matCount, matField2, matField3, matStartIdx = struct.unpack("<4I", read(0x10))
    matStartIdx = struct.unpack("<i", struct.pack("<I", matStartIdx))[0]
    mat_array = read(matCount * 0x2c)
    idx = matStartIdx
    visited = set()
    materials = []
    while idx >= 0:
        if idx in visited or idx >= matCount:
            break
        visited.add(idx)
        mat = mat_array[idx * 0x2c:(idx + 1) * 0x2c]
        flags = mat[1]
        textured = (flags & 1) != 0
        tex_index = None
        tex_name = None
        if textured:
            raw_tex_index = struct.unpack_from("<i", mat, 0x10)[0]
            if 0 <= raw_tex_index < texCount:
                tex_index = raw_tex_index
                tex_name = texture_names[raw_tex_index]
        # Real per-material face colour: MaterialRecord+0x04/+0x08/+0x0c, three floats in a
        # 0..255 range (NOT 0..1) — confirmed 2026-09-04, see recoil_re_log.md's "MaterialRecord
        # colour field" entry and the MaterialRecord struct in recoil_confirmed_logic.md. Every
        # TEXTURED material carries (255,255,255) (a neutral tint, so modulating a sampled texel
        # by it is a no-op); UNTEXTURED materials carry the real colour their polygons must be
        # drawn in. Exported so a runtime consumer never needs a baked colour table.
        color_r, color_g, color_b = struct.unpack_from("<3f", mat, 0x04)
        # MaterialRecord+0x00 is the per-material ALPHA, 0..255 (confirmed 2026-09-04).
        #
        # Evidence, measured across mission 1's 512 live materials: it is 255 for ALL 438
        # textured materials with no variation whatsoever, and varies ONLY on untextured ones
        # (35 at 255; the rest at 0, 76, 84, 102, 127 and 153). That is exactly the shape of an
        # alpha channel used as a fallback: a textured material takes its transparency from the
        # texture (202 of 576 mission-1 textures carry real alpha), so this field sits at a
        # neutral 255 there, while an untextured polygon has nowhere else to store it.
        #
        # Corroborated visually: material 38 -- a bright orange (243,97,0) untextured quad sitting
        # among the palms and burnt textures -- reads alpha 0 here, and is indeed completely
        # ABSENT from real gameplay footage of that spot. Seven untextured materials read 0
        # (38, 122, 126, 159, 210, 224, 231); drawing them opaque puts solid coloured slabs into
        # the scene that the original never shows.
        alpha = mat[0]
        materials.append({
            "material_index": idx,
            "textured": textured,
            "texture_index": tex_index,
            "texture_name": tex_name,
            "color_r": color_r,
            "color_g": color_g,
            "color_b": color_b,
            "alpha": alpha,
        })
        if flags & 4:
            cycle_header = read(0x1c)
            cycleCount = struct.unpack_from("<I", cycle_header, 0x10)[0]
            read(cycleCount * 4)
        idx = struct.unpack_from("<h", mat, 0x2a)[0]
    materials.sort(key=lambda m: m["material_index"])

    # --- model3d buffer: capture EVERY model (export_gamez_obj.py, capture=True always) ---
    modelCount, modelField2, modelField3 = struct.unpack("<3I", read(0xc))
    model_array = read(modelCount * 0x58)

    models = []
    for mi in range(modelField2):
        rec = model_array[mi * 0x58:(mi + 1) * 0x58]
        vertCount, normCount, morphCount, pointCount = struct.unpack_from("<4I", rec, 0x10)
        polyCount = struct.unpack_from("<I", rec, 0xc)[0]

        verts = []
        polys = []

        if vertCount > 0:
            vraw = read(vertCount * 0xc)
            for vi in range(vertCount):
                verts.append(struct.unpack_from("<3f", vraw, vi * 0xc))
        # Normals and morphs were previously read-and-DISCARDED here. Both are real,
        # substantial per-vertex data (2026-09-04): every one of the 6 bft meshes that has a
        # morph set has morphCount == vertCount, and every single morph vertex differs from its
        # base vertex (e.g. rtracks base spans X 0.000..1.150 while its morph set spans
        # X -1.500..-0.350). Given the real parent node names "right_morphs"/"left_morphs" and
        # the confirmed transforming-vehicle modes (track/hover/amphib/sub), these are almost
        # certainly the per-mode deformation targets. Captured now so no consumer has to
        # re-parse the .zbd to reach them; what they MEAN is still an open RE question and is
        # deliberately not interpreted here.
        norms = []
        if normCount > 0:
            nraw = read(normCount * 0xc)
            for ni in range(normCount):
                norms.append(struct.unpack_from("<3f", nraw, ni * 0xc))
        morphs = []
        if morphCount > 0:
            mraw = read(morphCount * 0xc)
            for mi2 in range(morphCount):
                morphs.append(struct.unpack_from("<3f", mraw, mi2 * 0xc))
        if pointCount > 0:
            points = read(pointCount * 0x4c)
            for pi in range(pointCount):
                prec = points[pi * 0x4c:(pi + 1) * 0x4c]
                subCount = struct.unpack_from("<I", prec, 0xc)[0]
                if subCount > 0:
                    read(subCount * 0xc)
        if polyCount > 0:
            polyrecs = read(polyCount * 0x1c)
            for pi in range(polyCount):
                prec = polyrecs[pi * 0x1c:(pi + 1) * 0x1c]
                flags = struct.unpack_from("<I", prec, 0)[0]
                idxCount = flags & 0xff
                matIndex = struct.unpack_from("<i", prec, 0x14)[0]

                vindices = None
                nindices = None
                if idxCount != 0:
                    iraw = read(idxCount * 4)
                    vindices = list(struct.unpack_from(f"<{idxCount}i", iraw, 0))
                    if flags & 0x200:
                        # SECOND per-corner index array, same length as the vertex indices.
                        # Previously read and discarded. It indexes the model's NORMAL array:
                        # normals are neither per-vertex nor per-polygon (measured across all 101
                        # mission-1 meshes that have them, normCount equals vertCount on only 5
                        # and polyCount on only 11), so they need their own indirection, exactly
                        # like OBJ's separate v/vn indices.
                        nraw = read(idxCount * 4)
                        nindices = list(struct.unpack_from(f"<{idxCount}i", nraw, 0))

                uv_present = 0 <= matIndex < matCount and (mat_array[matIndex * 0x2c + 1] & 1) != 0
                uvs = None
                if uv_present:
                    uvraw = read(idxCount * 8)
                    uvs = [struct.unpack_from("<2f", uvraw, k * 8) for k in range(idxCount)]

                if vindices:
                    polys.append({"idx": vindices, "uv": uvs, "matIndex": matIndex,
                                  "nidx": nindices, "flags": flags})

        models.append({
            "model_index": mi,
            "vertCount": vertCount,
            "polyCount": polyCount,
            "verts": verts,
            "norms": norms,     # real per-vertex normals; [] when normCount == 0 (the bft body
                                 # meshes genuinely have none — see recoil_re_log.md 2026-09-04)
            "morphs": morphs,   # real second per-vertex position set; [] when morphCount == 0
            "polys": polys,
        })

    # --- node buffer (parse_gamez.py's self-verifying walk, extended to capture object3d fields) ---
    nodeCap = header[6]
    nodeCount_header = header[7]
    nodes_raw = read(nodeCap * 0xc4)

    scene_nodes = []
    world_grid = None  # populated when the type-2 "world" node is reached (see below)
    ni = 0
    while pos < len(data) and ni < nodeCap:
        rec = nodes_raw[ni * 0xc4:(ni + 1) * 0xc4]
        ntype = struct.unpack_from("<I", rec, 0x34)[0]
        name = _cstr(rec[0x00:0x24])
        activeFlags = rec[0x24]
        rawMeshIndex = struct.unpack_from("<i", rec, 0x3c)[0]
        childCount = struct.unpack_from("<i", rec, 0x5c)[0]
        bboxMin = struct.unpack_from("<3f", rec, 0x74)
        bboxMax = struct.unpack_from("<3f", rec, 0x80)

        entry = {
            "index": ni,
            "name": name,
            "type": ntype,
            "type_name": NODE_TYPE_NAMES.get(ntype, f"unknown_{ntype}"),
            "active_flags": activeFlags,
            # confirmed 2026-09-02 (recoil_confirmed_logic.md GamezNode.meshIndex):
            # raw on-disk index into the model3d array, -1 = no mesh. Matches a
            # model3d "model_index" / this mission's glTF mesh index directly.
            "mesh_index": rawMeshIndex if rawMeshIndex >= 0 else None,
            "child_count": childCount,
            "bbox_min": list(bboxMin),
            "bbox_max": list(bboxMax),
        }

        if ntype == 0:
            pass
        elif ntype == 1:
            read(0x1e8)
        elif ntype == 2:
            world = read(0xac)
            # Capture the world node's real AREA-PARTITION GRID (2026-09-04). Previously the cell
            # records were read purely to keep the byte stream aligned and then thrown away. It is
            # the confirmed spatial partition engine/collision's SpatialGrid models, so exporting it
            # means the runtime uses the game's OWN partition instead of inventing one. Measured for
            # m1: rows=17, cols=16 (272 cells), cell pitch 256 world units, covering 4096 x 4352.
            world_grid = {
                "rows": struct.unpack_from("<i", world, 0x7c)[0],
                "cols": struct.unpack_from("<i", world, 0x78)[0],
                "cells": [],
            }
            cnt1 = struct.unpack_from("<i", world, 0x90)[0]
            if cnt1 > 0:
                read(cnt1 * 4)
            cnt2 = struct.unpack_from("<i", world, 0x9c)[0]
            if cnt2 > 0:
                read(cnt2 * 4)
            rows = struct.unpack_from("<i", world, 0x7c)[0]
            cols = struct.unpack_from("<i", world, 0x78)[0]
            for _r in range(max(rows, 0)):
                for _c in range(max(cols, 0)):
                    cell = read(0x40)
                    _cellf = struct.unpack_from("<10f", cell, 0)
                    cellSub = struct.unpack_from("<h", cell, 0x3a)[0]
                    if cellSub > 0:
                        _members = list(
                            struct.unpack_from("<%di" % cellSub, read(cellSub * 4), 0))
                    else:
                        _members = []
                    world_grid["cells"].append({
                        "row": _r,
                        "col": _c,
                        # floats +0x08..+0x1c: the cell's real world-space bounds (x/z extents and
                        # the shared y range) — measured to advance by exactly 256 units per column
                        # for m1, i.e. the real partition pitch.
                        "bounds": [round(float(x), 3) for x in _cellf[2:8]],
                        "member_node_indices": _members,
                    })
        elif ntype == 3:
            read(0xf8)
        elif ntype == 4:
            read(0x1c)
        elif ntype == 5:  # object3d - capture confirmed fields
            block = read(0x90)
            scale = struct.unpack_from("<3f", block, 0x24)
            localTransform = struct.unpack_from("<12f", block, 0x30)
            entry["object3d"] = {
                "flags": struct.unpack_from("<I", block, 0x00)[0],
                "scale": list(scale),
                "local_rotation_3x3": list(localTransform[0:9]),
                "local_translation": list(localTransform[9:12]),
            }
        elif ntype == 6:
            read(0x50)
        elif ntype == 9:
            light = read(0xe4)
            cnt = struct.unpack_from("<i", light, 0xdc)[0]
            if cnt > 0:
                read(cnt * 4)
        elif ntype == 10:
            snd = read(0x94)
            cnt = struct.unpack_from("<i", snd, 0x8c)[0]
            if cnt > 0:
                read(cnt * 4)
        else:
            raise ValueError(f"node[{ni}]: unrecognized node type {ntype} (pos={pos})")

        cnt54 = struct.unpack_from("<i", rec, 0x54)[0]
        if cnt54 >= 1:
            read(cnt54 * 4)
        cnt5c = struct.unpack_from("<i", rec, 0x5c)[0]
        if cnt5c > 0:
            # confirmed 2026-09-02 (recoil_confirmed_logic.md GamezNode.childArray):
            # raw dwords here are plain node indices into this same flat node
            # array (parent -> children edge list), not opaque/unresolved data.
            child_raw = read(cnt5c * 4)
            entry["children"] = list(struct.unpack_from(f"<{cnt5c}i", child_raw, 0))

        scene_nodes.append(entry)
        ni += 1

    return {
        "header": list(header),
        "tex_count": texCount,
        "texture_names": texture_names,
        "mat_count": matCount,
        "materials": materials,
        "model_count_declared": modelCount,
        "model_count_real": modelField2,
        "models": models,
        "node_capacity": nodeCap,
        "node_count_header": nodeCount_header,
        "node_count_actual": len(scene_nodes),
        "scene_nodes": scene_nodes,
        "world_grid": world_grid,
        "bytes_total": len(data),
        "bytes_consumed": pos,
    }


def _fan_triangulate_corners(n):
    """Fan-triangulate a polygon of n corners, returning CORNER POSITIONS
    (0-based indices into the polygon's own idx/uv arrays), not vertex
    indices - so the caller can look up the correct per-corner UV for a
    vertex that appears more than once in the same polygon."""
    if n < 3:
        return []
    return [(0, k, k + 1) for k in range(1, n - 1)]


def build_gltf(parsed, mission, bin_path, bin_rel_uri):
    """Non-indexed TRIANGLES glTF: one mesh+node per model3d entry, fan-
    triangulated exactly as the original engine's D3DPT_TRIANGLEFAN
    submission draws each polygon (confirmed in recoil_confirmed_logic.md's
    Rendering section) - not an arbitrary triangulation choice.
    Vertex data is duplicated per-triangle-corner rather than shared/indexed,
    since UV is per-polygon-corner, not per-vertex (a shared vertex can carry
    different UVs in different polygons); documented as a known follow-up
    optimization, not a correctness gap.
    """
    buffers_out = bytearray()
    accessors = []
    buffer_views = []
    meshes = []
    nodes = []
    scene_node_indices = []

    def add_view_and_accessor(raw_bytes, count, comp_type, acc_type, min_max=None):
        offset = len(buffers_out)
        buffers_out.extend(raw_bytes)
        # pad to 4-byte alignment
        while len(buffers_out) % 4 != 0:
            buffers_out.append(0)
        bv_index = len(buffer_views)
        buffer_views.append({
            "buffer": 0,
            "byteOffset": offset,
            "byteLength": len(raw_bytes),
            "target": 34962,  # ARRAY_BUFFER
        })
        acc = {
            "bufferView": bv_index,
            "byteOffset": 0,
            "componentType": comp_type,
            "count": count,
            "type": acc_type,
        }
        if min_max:
            acc["min"], acc["max"] = min_max
        acc_index = len(accessors)
        accessors.append(acc)
        return acc_index

    exported_model_count = 0
    exported_tri_count = 0

    for m in parsed["models"]:
        verts = m["verts"]
        positions = []
        texcoords = []
        any_uv = False

        # Real per-polygon matIndex (model3d polygon record +0x14, see module
        # docstring) was already captured into p["matIndex"] by
        # parse_gamez_full() but previously discarded here - this is the fix
        # for that gap (2026-09-03, see DEVLOG.md). matIndex < 0 means "no
        # material" (confirmed: tools/scripts/parse_gamez.py's identical
        # uv-presence test treats negative matIndex as "no material -> no
        # UV"), so those polygons contribute nothing to the mesh's material
        # set. A mesh commonly spans multiple materials (e.g. m1 model_index
        # 4 spans 5: {0,3,4,5,6} across its 19 polygons - see DEVLOG.md's
        # "first real texture" entry), so this is collected as a sorted,
        # deduped list, not a single scalar.
        mesh_mat_indices = sorted({p["matIndex"] for p in m["polys"] if p["matIndex"] >= 0})

        # One entry per EMITTED triangle, in emission order, so a consumer can map triangle i of
        # this primitive straight to its real material with no re-parsing and no name lookups.
        # This is what makes generic, data-driven multi-material rendering possible: before this
        # existed, the only per-triangle material data anywhere was a hand-written table naming 8
        # specific bft parts as string literals (src/game/src/bft_material_triangles.h), which
        # covered 8 of the 386 multi-material meshes in mission 1 alone, of 13 missions.
        tri_materials = []
        # Per-corner normals, resolved through the polygon's SECOND index array (flag 0x200).
        # Confirmed 2026-09-04: that array indexes the model's normal list, not its vertex list.
        # Evidence — exactly the 101 mission-1 meshes that have normals are exactly the 101 that
        # carry the array (never one without the other), all 1,729 index values land inside
        # [0, normCount), and the highest index used is normCount-1 in every mesh sampled.
        # Normals are neither per-vertex nor per-polygon: normCount equals vertCount on only 5 of
        # those meshes and polyCount on only 11, which is why the indirection exists at all.
        model_norms = m.get("norms") or []
        normals = []
        any_normal = False
        # Morph target, when present. Every morph set in mission 1 has morphCount == vertCount
        # (12 meshes, no exceptions), so it indexes by vertex exactly like the base positions.
        model_morphs = m.get("morphs") or []
        morph_deltas = []
        has_morph = len(model_morphs) == len(verts) and len(model_morphs) > 0

        for p in m["polys"]:
            idx = p["idx"]
            uvs = p["uv"]
            nidx = p.get("nidx")
            corner_tris = _fan_triangulate_corners(len(idx))
            for (ca, cb, cc) in corner_tris:
                corner_vis = [idx[c] for c in (ca, cb, cc)]
                if any(vi < 0 or vi >= len(verts) for vi in corner_vis):
                    continue  # skip whole triangle atomically - keeps non-indexed buffer aligned to multiples of 3
                # A fan-triangulated polygon yields several triangles; each inherits its SOURCE
                # POLYGON's real matIndex (the count is therefore NOT 1:1 with polyCount).
                tri_materials.append(p["matIndex"])
                for corner_pos, vi in zip((ca, cb, cc), corner_vis):
                    positions.append(verts[vi])
                    if uvs is not None:
                        uv = uvs[corner_pos] if corner_pos < len(uvs) else (0.0, 0.0)
                        texcoords.append(uv)
                        any_uv = True
                    else:
                        texcoords.append((0.0, 0.0))
                    if model_norms and nidx is not None and corner_pos < len(nidx):
                        ni = nidx[corner_pos]
                        if 0 <= ni < len(model_norms):
                            normals.append(model_norms[ni])
                            any_normal = True
                        else:
                            normals.append((0.0, 1.0, 0.0))
                    else:
                        normals.append((0.0, 1.0, 0.0))
                    if has_morph:
                        b = verts[vi]
                        t = model_morphs[vi]
                        # glTF morph targets are DELTAS from the base position, not absolutes.
                        morph_deltas.append((t[0] - b[0], t[1] - b[1], t[2] - b[2]))

        if not positions:
            continue

        xs = [v[0] for v in positions]
        ys = [v[1] for v in positions]
        zs = [v[2] for v in positions]
        pos_min = [min(xs), min(ys), min(zs)]
        pos_max = [max(xs), max(ys), max(zs)]

        pos_bytes = b"".join(struct.pack("<3f", *v) for v in positions)
        pos_acc = add_view_and_accessor(pos_bytes, len(positions), 5126, "VEC3", (pos_min, pos_max))

        attributes = {"POSITION": pos_acc}
        if any_normal:
            nrm_bytes = b"".join(struct.pack("<3f", *n) for n in normals)
            attributes["NORMAL"] = add_view_and_accessor(nrm_bytes, len(normals), 5126, "VEC3")
        if any_uv:
            uv_bytes = b"".join(struct.pack("<2f", *uv) for uv in texcoords)
            uv_acc = add_view_and_accessor(uv_bytes, len(texcoords), 5126, "VEC2")
            attributes["TEXCOORD_0"] = uv_acc

        # Real matIndex values, keyed to this mesh's real material_index(es)
        # in <mission>_materials.json (which resolves material_index ->
        # texture_name), carried in glTF's standard "extras" object (arbitrary
        # app data, per the glTF 2.0 spec) on the primitive - the natural home
        # since glTF's own "material" field on a primitive must be an index
        # into a *glTF* materials array (a PBR material definition this
        # pipeline doesn't build), not an arbitrary app-defined index, and
        # since material assignment is per-primitive/per-geometry, not
        # per-node (m1_scene_nodes.json's nodes are geometry instances and
        # would need this duplicated per-instance for no benefit).
        # "material_indices" is always present (possibly [] if every polygon
        # had matIndex < 0); "material_index" is also set as a convenience
        # scalar iff the mesh uses exactly one material (the common case a
        # single-texture consumer like the textured-mesh render demo needs).
        primitive_extras = {"material_indices": mesh_mat_indices}
        if len(mesh_mat_indices) == 1:
            primitive_extras["material_index"] = mesh_mat_indices[0]
        # Per-triangle material assignment (see tri_materials above). Always emitted, and its
        # length is always exactly len(positions)//3.
        primitive_extras["per_triangle_material_index"] = tri_materials
        # The real model3d record index this mesh came from. A scene node's "mesh_index" field is
        # THIS value, not a position in the glTF meshes[] array (the two differ by a non-constant
        # offset because models producing no geometry are skipped). Exporting it explicitly means
        # consumers resolve node -> mesh by a real field instead of parsing the "model_<N>" name.
        primitive_extras["model_index"] = m["model_index"]
        if m.get("norms"):
            primitive_extras["source_normal_count"] = len(m["norms"])
        if m.get("morphs"):
            primitive_extras["source_morph_count"] = len(m["morphs"])

        primitive = {"attributes": attributes, "mode": 4, "extras": primitive_extras}
        if has_morph and morph_deltas:
            mxs = [d[0] for d in morph_deltas]
            mys = [d[1] for d in morph_deltas]
            mzs = [d[2] for d in morph_deltas]
            mo_bytes = b"".join(struct.pack("<3f", *d) for d in morph_deltas)
            mo_acc = add_view_and_accessor(
                mo_bytes, len(morph_deltas), 5126, "VEC3",
                ([min(mxs), min(mys), min(mzs)], [max(mxs), max(mys), max(mzs)]))
            primitive["targets"] = [{"POSITION": mo_acc}]

        mesh_index = len(meshes)
        mesh_entry = {
            "name": f"model_{m['model_index']}",
            "primitives": [primitive],
        }
        if has_morph and morph_deltas:
            # Weight 0 = base pose. The real trigger is the vehicle's transform mode
            # (track/hover/amphib/sub); nothing here claims to know how the original drives it.
            mesh_entry["weights"] = [0.0]
        meshes.append(mesh_entry)
        node_index = len(nodes)
        nodes.append({"name": f"model_{m['model_index']}", "mesh": mesh_index})
        scene_node_indices.append(node_index)

        exported_model_count += 1
        exported_tri_count += len(positions) // 3

    with open(bin_path, "wb") as f:
        f.write(bytes(buffers_out))

    gltf = {
        "asset": {"version": "2.0", "generator": "recoil-asset-pipeline/gamez_export.py"},
        "scene": 0,
        "scenes": [{"nodes": scene_node_indices}],
        "nodes": nodes,
        "meshes": meshes,
        "accessors": accessors,
        "bufferViews": buffer_views,
        "buffers": [{"uri": bin_rel_uri, "byteLength": len(buffers_out)}],
    }
    return gltf, exported_model_count, exported_tri_count


def export_mission_geometry(gamez_path, mission, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    parsed = parse_gamez_full(gamez_path)

    bin_name = f"{mission}_gamez.bin"
    bin_path = os.path.join(out_dir, bin_name)
    gltf, model_count, tri_count = build_gltf(parsed, mission, bin_path, bin_name)

    gltf_path = os.path.join(out_dir, f"{mission}_gamez.gltf")
    with open(gltf_path, "w") as f:
        json.dump(gltf, f)

    scene_json_path = os.path.join(out_dir, f"{mission}_scene_nodes.json")
    with open(scene_json_path, "w") as f:
        json.dump({
            "mission": mission,
            "node_capacity": parsed["node_capacity"],
            "node_count_header": parsed["node_count_header"],
            "node_count_actual": parsed["node_count_actual"],
            # The real world-node area-partition grid (2026-09-04). See the type-2 branch in
            # parse_gamez_full() for the layout; this is the game's OWN spatial partition, which
            # engine/collision's SpatialGrid should consume instead of inventing its own.
            "world_grid": parsed.get("world_grid"),
            "nodes": parsed["scene_nodes"],
        }, f, indent=1)

    mesh_linked = sum(1 for n in parsed["scene_nodes"] if n.get("mesh_index") is not None)
    nodes_with_children = sum(1 for n in parsed["scene_nodes"] if n.get("children"))

    materials_json_path = os.path.join(out_dir, f"{mission}_materials.json")
    with open(materials_json_path, "w") as f:
        json.dump({
            "mission": mission,
            "tex_count": parsed["tex_count"],
            "mat_count": parsed["mat_count"],
            "materials": parsed["materials"],
        }, f, indent=1)

    textured_materials = sum(1 for m in parsed["materials"] if m["textured"])
    resolved_textures = sum(1 for m in parsed["materials"] if m["texture_name"] is not None)

    return {
        "gltf_path": gltf_path,
        "bin_path": bin_path,
        "scene_json_path": scene_json_path,
        "materials_json_path": materials_json_path,
        "model_count_source": parsed["model_count_real"],
        "model_count_exported": model_count,
        "triangle_count": tri_count,
        "node_count": parsed["node_count_actual"],
        "nodes_with_mesh_link": mesh_linked,
        "nodes_with_children": nodes_with_children,
        "material_count": parsed["mat_count"],
        "materials_listed": len(parsed["materials"]),
        "textured_materials": textured_materials,
        "materials_resolved_to_texture": resolved_textures,
        "gltf_bytes": os.path.getsize(gltf_path),
        "bin_bytes": os.path.getsize(bin_path),
        "bytes_consumed": parsed["bytes_consumed"],
        "bytes_total": parsed["bytes_total"],
    }


if __name__ == "__main__":
    import sys
    mission = sys.argv[1] if len(sys.argv) > 1 else "m1"
    gamez_path = os.path.join(os.environ.get("RECOIL_GAME_DIR", "game"), "zbd", mission, "gamez.zbd")
    out_dir = os.path.join("output", mission, "geometry")
    result = export_mission_geometry(gamez_path, mission, out_dir)
    print(json.dumps(result, indent=2))
