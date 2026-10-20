"""
Full-coverage validation pass over gamez.zbd model3d geometry.

Extends export_gamez_obj.py's approach (same section read order/offsets)
to walk EVERY model in EVERY real mission gamez.zbd (m1-m13), not just the
first N. For each model it checks:
  - no exceptions/EOF during parsing
  - zero out-of-bounds face-vertex indices
  - non-degenerate bounding box (not all-zero, not absurdly large/tiny)
  - non-zero vertex/polygon counts

To keep memory bounded, per-model vertex/poly data is discarded after its
checks run (not accumulated). Only the single largest model (by vertex
count) per mission is written out to an OBJ file for later visual
inspection, plus any model that fails/warns.

Usage:
    python validate_gamez_full.py
"""
import struct
import os

MISSIONS = [f"m{i}" for i in range(1, 14)]
BASE = os.path.join(os.environ.get("RECOIL_GAME_DIR", "game"), "zbd")
OUT_DIR = os.path.join("_export_out", "full_coverage")


def write_obj(path, model_index, verts, polys):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as out:
        out.write(f"o model_{model_index}\n")
        for (x, y, z) in verts:
            out.write(f"v {x} {y} {z}\n")
        vt_cursor = 0
        any_uv = any(p["uv"] is not None for p in polys)
        if any_uv:
            for p in polys:
                if p["uv"] is not None:
                    for (u, v) in p["uv"]:
                        out.write(f"vt {u} {v}\n")
        for p in polys:
            indices = p["idx"]
            uvs = p["uv"]
            face_terms = []
            for k, vi in enumerate(indices):
                v_out = vi + 1
                if uvs is not None:
                    vt_out = vt_cursor + k + 1
                    face_terms.append(f"{v_out}/{vt_out}")
                else:
                    face_terms.append(f"{v_out}")
            out.write("f " + " ".join(face_terms) + "\n")
            if uvs is not None:
                vt_cursor += len(indices)


def validate_mission(mission):
    path = os.path.join(BASE, mission, "gamez.zbd")
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

    result = {
        "mission": mission,
        "file_size": len(data),
        "model_count_field2": None,
        "models_processed": 0,
        "total_verts": 0,
        "total_polys": 0,
        "exceptions": [],       # (model_index, message)
        "oob_violations": [],   # (model_index, oob_count, examples, nv)
        "bbox_anomalies": [],   # (model_index, kind, detail)
        "zero_geometry": [],    # (model_index, kind)
        "largest_model": None,  # (model_index, vertCount, verts, polys)
    }

    # --- Header ---
    header = struct.unpack("<9I", read(0x24))
    magic, const = header[0], header[1]
    assert magic == 0x02971222, f"bad magic 0x{magic:x}"
    assert const == 0xf, f"bad const 0x{const:x}"
    texCount = header[2]

    # --- Section 1: texture directory ---
    read(texCount * 0x24)

    # --- Section 2: material buffer ---
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

    # --- Section 3: model3d buffer ---
    modelCount, modelField2, modelField3 = struct.unpack("<3I", read(0xc))
    result["model_count_field2"] = modelField2
    model_array = read(modelCount * 0x58)

    largest = None  # (vertCount, model_index, verts, polys)

    for mi in range(modelField2):
        rec = model_array[mi * 0x58:(mi + 1) * 0x58]
        vertCount, normCount, morphCount, pointCount = struct.unpack_from("<4I", rec, 0x10)
        polyCount = struct.unpack_from("<I", rec, 0xc)[0]

        try:
            verts = []
            polys = []

            if vertCount > 0:
                vraw = read(vertCount * 0xc)
                for vi in range(vertCount):
                    x, y, z = struct.unpack_from("<3f", vraw, vi * 0xc)
                    verts.append((x, y, z))
            if normCount > 0:
                read(normCount * 0xc)
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
                        vindices = list(struct.unpack_from(f"<{idxCount}i", iraw, 0))
                        if flags & 0x200:
                            read(idxCount * 4)

                    uv_present = False
                    if matIndex >= 0 and matIndex < matCount:
                        mflag = mat_array[matIndex * 0x2c + 1]
                        uv_present = (mflag & 1) != 0

                    uvs = None
                    if uv_present:
                        uvraw = read(idxCount * 8)
                        uvs = [struct.unpack_from("<2f", uvraw, k * 8) for k in range(idxCount)]

                    if vindices:
                        polys.append({"idx": vindices, "uv": uvs, "matIndex": matIndex})
        except Exception as e:
            result["exceptions"].append((mi, str(e)))
            # Can't reliably continue parsing this mission's stream after
            # a raw read failure (position is corrupted) - abort mission.
            result["aborted_at"] = mi
            return result

        nv = len(verts)
        npoly = len(polys)
        result["models_processed"] += 1
        result["total_verts"] += nv
        result["total_polys"] += npoly

        if nv == 0 or npoly == 0:
            result["zero_geometry"].append((mi, f"vertCount={nv} polyCount={npoly}"))

        if nv > 0:
            xs = [v[0] for v in verts]
            ys = [v[1] for v in verts]
            zs = [v[2] for v in verts]
            bbmin = (min(xs), min(ys), min(zs))
            bbmax = (max(xs), max(ys), max(zs))
            size = (bbmax[0] - bbmin[0], bbmax[1] - bbmin[1], bbmax[2] - bbmin[2])
            max_dim = max(size)
            if bbmin == bbmax == (0.0, 0.0, 0.0):
                result["bbox_anomalies"].append((mi, "all-zero (origin)", None))
            elif max_dim == 0:
                result["bbox_anomalies"].append((mi, "zero-volume", size))
            elif max_dim > 1_000_000:
                result["bbox_anomalies"].append((mi, "implausibly-large", max_dim))
            elif max_dim < 1e-4:
                result["bbox_anomalies"].append((mi, "implausibly-tiny", max_dim))

        # OOB face-vertex index check
        oob_count = 0
        oob_examples = []
        for p in polys:
            for vi in p["idx"]:
                if vi < 0 or vi >= nv:
                    oob_count += 1
                    if len(oob_examples) < 10:
                        oob_examples.append(vi)
        if oob_count > 0:
            result["oob_violations"].append((mi, oob_count, oob_examples, nv))

        if largest is None or nv > largest[0]:
            largest = (nv, mi, verts, polys)

    if largest is not None:
        result["largest_model"] = largest

    result["final_pos"] = pos
    result["bytes_remaining"] = len(data) - pos

    return result


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    all_results = []
    grand_total_models = 0
    grand_total_verts = 0
    grand_total_polys = 0
    any_exceptions = False
    any_oob = False
    any_bbox = False

    for mission in MISSIONS:
        print(f"=== {mission} ===")
        r = validate_mission(mission)
        all_results.append(r)
        print(f"  file_size={r['file_size']} modelCount(field2)={r['model_count_field2']} "
              f"processed={r['models_processed']}")
        if r.get("aborted_at") is not None:
            print(f"  ABORTED at model {r['aborted_at']} due to exception")
            any_exceptions = True
        if r["exceptions"]:
            any_exceptions = True
            for mi, msg in r["exceptions"]:
                print(f"  EXCEPTION model {mi}: {msg}")
        if r["oob_violations"]:
            any_oob = True
            for mi, cnt, examples, nv in r["oob_violations"]:
                print(f"  OOB model {mi}: {cnt} violations, examples={examples}, validRange=[0,{nv-1}]")
        if r["bbox_anomalies"]:
            any_bbox = True
            for mi, kind, detail in r["bbox_anomalies"]:
                print(f"  BBOX-ANOMALY model {mi}: {kind} detail={detail}")
        if r["zero_geometry"]:
            for mi, detail in r["zero_geometry"]:
                print(f"  ZERO-GEOMETRY model {mi}: {detail}")

        grand_total_models += r["models_processed"]
        grand_total_verts += r["total_verts"]
        grand_total_polys += r["total_polys"]

        # Dump the largest model in this mission as a sample OBJ.
        if r["largest_model"] is not None:
            nv, mi, verts, polys = r["largest_model"]
            obj_path = os.path.join(OUT_DIR, f"{mission}_largest_model{mi}_{nv}verts.obj")
            write_obj(obj_path, mi, verts, polys)
            print(f"  largest model: index={mi} verts={nv} polys={len(polys)} -> {obj_path}")

        # Dump any bbox-anomalous or OOB models too, for inspection (skip if huge list).
        flagged = set(mi for mi, *_ in r["oob_violations"]) | set(mi for mi, *_ in r["bbox_anomalies"])
        if flagged and len(flagged) <= 20:
            # Re-parse just to grab these models' geometry would require another
            # pass; largest_model capture already covers the common "largest is
            # also anomalous" case. For now we just note the indices; if this
            # list is non-empty we can do a targeted re-export.
            print(f"  flagged models (for potential re-export): {sorted(flagged)}")

        print(f"  totals this mission: verts={r['total_verts']} polys={r['total_polys']}")
        print()

    print("=" * 70)
    print("GRAND TOTAL SUMMARY")
    print("=" * 70)
    print(f"missions processed: {len(MISSIONS)}")
    print(f"total models processed: {grand_total_models}")
    print(f"total vertices: {grand_total_verts}")
    print(f"total polygons: {grand_total_polys}")
    print(f"any exceptions/aborts: {any_exceptions}")
    print(f"any OOB index violations: {any_oob}")
    print(f"any bbox anomalies: {any_bbox}")
    print()
    for r in all_results:
        status = "OK"
        if r.get("aborted_at") is not None or r["exceptions"]:
            status = "EXCEPTION"
        elif r["oob_violations"]:
            status = "OOB-VIOLATIONS"
        elif r["bbox_anomalies"]:
            status = "BBOX-ANOMALY"
        print(f"  {r['mission']:4s} models={r['models_processed']:5d} "
              f"verts={r['total_verts']:8d} polys={r['total_polys']:8d} "
              f"bytes_remaining_after_models={r.get('bytes_remaining', 'N/A')} status={status}")


if __name__ == "__main__":
    main()
