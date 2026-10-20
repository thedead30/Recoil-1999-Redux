"""
Standalone validation exporter for gamez.zbd model3d geometry.

Purpose: prove that the byte-perfect section parsing in parse_gamez.py
actually yields *coherent* 3D geometry (real vertex positions, in-range
face indices, plausible bounding box) - not just correctly-counted bytes.

Reuses the exact same read order / offsets as parse_gamez.py for the
header, texture directory, and material sections (needed to correctly
reach the model3d section and to resolve the per-polygon UV-presence
flag), then re-implements the model3d section but *captures* vertex
floats and polygon index/UV data instead of just skipping over them,
and writes the first N models out to a Wavefront OBJ file.

Usage:
    python export_gamez_obj.py [mission] [num_models] [out_path]

    mission     - e.g. "m1" (default: m1)
    num_models  - how many of the first models to export (default: 3)
    out_path    - output .obj path (default: <scratchpad>/m1_first3.obj)
"""
import struct
import sys
import os


def parse_and_export(path, num_models, out_path):
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

    # --- Header (identical to parse_gamez.py) ---
    header = struct.unpack("<9I", read(0x24))
    magic, const = header[0], header[1]
    assert magic == 0x02971222, f"bad magic 0x{magic:x}"
    assert const == 0xf, f"bad const 0x{const:x}"
    texCount = header[2]
    print(f"=== {path} ===")
    print(f"header={header}")
    print(f"texCount={texCount}")

    # --- Section 1: texture directory (skip, not needed for geometry) ---
    read(texCount * 0x24)

    # --- Section 2: material buffer (need mat_array + matCount for the
    # per-polygon UV-presence flag check later) ---
    matCount, matField2, matField3, matStartIdx = struct.unpack("<4I", read(0x10))
    matStartIdx = struct.unpack("<i", struct.pack("<I", matStartIdx))[0]
    mat_array = read(matCount * 0x2c)
    idx = matStartIdx
    visited = set()
    while idx >= 0:
        if idx in visited or idx >= matCount:
            break
        visited.add(idx)
        mat = mat_array[idx * 0x2c:(idx + 1) * 0x2c]
        flag_byte = mat[1]
        if flag_byte & 4:
            cycle_header = read(0x1c)
            cycleCount = struct.unpack_from("<I", cycle_header, 0x10)[0]
            read(cycleCount * 4)
        next_idx = struct.unpack_from("<h", mat, 0x2a)[0]
        idx = next_idx
    print(f"material buffer parsed: matCount={matCount} (needed only for UV-presence flag)")

    # --- Section 3: model3d buffer ---
    modelCount, modelField2, modelField3 = struct.unpack("<3I", read(0xc))
    print(f"model3d header: count={modelCount} field2={modelField2} field3={modelField3}")
    model_array = read(modelCount * 0x58)

    exported_models = []  # list of dicts: {verts:[(x,y,z)], polys:[{idx:[...], uv:[(u,v),...] or None}]}

    n_to_export = min(num_models, modelField2)
    for mi in range(modelField2):
        rec = model_array[mi * 0x58:(mi + 1) * 0x58]
        vertCount, normCount, morphCount, pointCount = struct.unpack_from("<4I", rec, 0x10)
        polyCount = struct.unpack_from("<I", rec, 0xc)[0]

        capture = mi < n_to_export
        verts = []
        polys = []

        if vertCount > 0:
            vraw = read(vertCount * 0xc)
            if capture:
                for vi in range(vertCount):
                    x, y, z = struct.unpack_from("<3f", vraw, vi * 0xc)
                    verts.append((x, y, z))
        if normCount > 0:
            read(normCount * 0xc)  # not needed for OBJ v/vt/f-only export
        if morphCount > 0:
            read(morphCount * 0xc)
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
                if idxCount != 0:
                    iraw = read(idxCount * 4)
                    if capture:
                        vindices = list(struct.unpack_from(f"<{idxCount}i", iraw, 0))
                    if flags & 0x200:
                        read(idxCount * 4)  # secondary index array (e.g. normal idx) - not needed for OBJ

                uv_present = False
                if matIndex >= 0 and matIndex < matCount:
                    mflag = mat_array[matIndex * 0x2c + 1]
                    uv_present = (mflag & 1) != 0

                uvs = None
                if uv_present:
                    uvraw = read(idxCount * 8)
                    if capture:
                        uvs = [struct.unpack_from("<2f", uvraw, k * 8) for k in range(idxCount)]

                if capture and vindices:
                    polys.append({"idx": vindices, "uv": uvs, "matIndex": matIndex})

        if capture:
            exported_models.append({
                "model_index": mi,
                "vertCount": vertCount,
                "polyCount": polyCount,
                "verts": verts,
                "polys": polys,
            })
        else:
            # We've captured everything we need; no need to keep parsing
            # the rest of the model3d/node sections for this validation.
            break

    # --- Write OBJ ---
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    global_vidx = 0  # OBJ v/vt indices are 1-based and global across the file
    global_vtidx = 0
    with open(out_path, "w") as out:
        out.write(f"# exported from {path}\n")
        out.write(f"# models: {[m['model_index'] for m in exported_models]}\n")
        for m in exported_models:
            out.write(f"o model_{m['model_index']}\n")
            for (x, y, z) in m["verts"]:
                out.write(f"v {x} {y} {z}\n")
            vt_base = global_vtidx
            any_uv = any(p["uv"] is not None for p in m["polys"])
            if any_uv:
                for p in m["polys"]:
                    if p["uv"] is not None:
                        for (u, v) in p["uv"]:
                            out.write(f"vt {u} {v}\n")
                            global_vtidx += 1
            vt_cursor = vt_base
            for p in m["polys"]:
                indices = p["idx"]
                uvs = p["uv"]
                face_terms = []
                for k, vi in enumerate(indices):
                    v_out = global_vidx + vi + 1  # 1-based, offset into this model's verts
                    if uvs is not None:
                        vt_out = vt_cursor + k + 1
                        face_terms.append(f"{v_out}/{vt_out}")
                    else:
                        face_terms.append(f"{v_out}")
                out.write("f " + " ".join(face_terms) + "\n")
                if uvs is not None:
                    vt_cursor += len(indices)
            global_vidx += len(m["verts"])

    print(f"OBJ written: {out_path}")

    # --- Sanity checks ---
    print()
    print("=== SANITY CHECKS ===")
    all_ok = True
    for m in exported_models:
        mi = m["model_index"]
        nv = len(m["verts"])
        np_ = len(m["polys"])
        print(f"-- model {mi}: vertCount={m['vertCount']} (captured {nv}), polyCount={m['polyCount']} (captured {np_})")

        if nv == 0:
            print(f"   FAIL: zero vertices")
            all_ok = False
            continue
        if nv > 200000:
            print(f"   WARNING: absurdly large vertex count ({nv})")

        # bounding box
        xs = [v[0] for v in m["verts"]]
        ys = [v[1] for v in m["verts"]]
        zs = [v[2] for v in m["verts"]]
        bbmin = (min(xs), min(ys), min(zs))
        bbmax = (max(xs), max(ys), max(zs))
        size = (bbmax[0] - bbmin[0], bbmax[1] - bbmin[1], bbmax[2] - bbmin[2])
        print(f"   bbox min={bbmin} max={bbmax} size={size}")
        if bbmin == bbmax == (0.0, 0.0, 0.0):
            print(f"   FAIL: all vertices at origin")
            all_ok = False
        max_dim = max(size)
        if max_dim == 0:
            print(f"   FAIL: degenerate (zero-volume) bounding box")
            all_ok = False
        elif max_dim > 1_000_000:
            print(f"   WARNING: bounding box implausibly large (max dim {max_dim})")
        elif max_dim < 1e-4:
            print(f"   WARNING: bounding box implausibly tiny (max dim {max_dim})")

        # face index range check
        oob_count = 0
        oob_examples = []
        max_idx_seen = -1
        min_idx_seen = None
        for p in m["polys"]:
            for vi in p["idx"]:
                if min_idx_seen is None or vi < min_idx_seen:
                    min_idx_seen = vi
                if vi > max_idx_seen:
                    max_idx_seen = vi
                if vi < 0 or vi >= nv:
                    oob_count += 1
                    if len(oob_examples) < 10:
                        oob_examples.append(vi)
        print(f"   face vertex-index range seen: [{min_idx_seen}, {max_idx_seen}] against valid range [0, {nv - 1}]")
        if oob_count > 0:
            print(f"   FAIL: {oob_count} out-of-bounds face vertex indices, e.g. {oob_examples}")
            all_ok = False
        else:
            print(f"   OK: all face vertex indices in bounds")

        if np_ == 0:
            print(f"   FAIL: zero polygons captured")
            all_ok = False

    print()
    print("=== OVERALL ===")
    print("ALL SANITY CHECKS PASSED" if all_ok else "ONE OR MORE SANITY CHECKS FAILED")
    return all_ok, exported_models, out_path


if __name__ == "__main__":
    mission = sys.argv[1] if len(sys.argv) > 1 else "m1"
    num_models = int(sys.argv[2]) if len(sys.argv) > 2 else 3
    out_path = sys.argv[3] if len(sys.argv) > 3 else os.path.join("_export_out", "%s_first%d.obj" % (mission, num_models))
    gamez_path = os.path.join(os.environ.get("RECOIL_GAME_DIR", "game"), "zbd", mission, "gamez.zbd")
    parse_and_export(gamez_path, num_models, out_path)
