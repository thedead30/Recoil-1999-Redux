"""
build_mission_assets.py — the real, runnable asset-conversion pipeline.

Converts ONE mission's original-format assets (.zbd, from
`RECOIL files after setup/zbd/<mission>/`) into the intermediate,
engine-ready format set decided in ARCHITECTURE.md section 6
("import once, at build/dev time... the shipped runtime does not parse
.zbd/.zmap at all"):

  geometry  -> glTF 2.0 (+ .bin)   via gamez_export.py
  scene     -> plain JSON node list via gamez_export.py
  textures  -> PNG files            via mech3ax unzbd.exe (rc textures)
  data      -> normalized JSON      via normalize_reader_json.py, fed by
                                       mech3ax unzbd.exe (rc reader)

This script is the ONLY place in the pipeline that shells out to the
original container formats (mech3ax's unzbd.exe, already confirmed working
for Recoil per CLAUDE.md's tools table) or re-runs the validated
tools/scripts/*.py parsing logic - everything downstream should consume the
output/ directory this produces, never the original .zbd files directly.

Usage:
    python build_mission_assets.py [mission] [--game-dir PATH] [--out PATH]

    mission    e.g. "m1" (default: m1)
    --game-dir defaults to "RECOIL files after setup/zbd" next to this
               project's root (see GAME_ZBD_DIR below)
    --out      defaults to tools/asset_pipeline/output/<mission>
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gamez_export import export_mission_geometry
from normalize_reader_json import normalize_reader_file

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
PIPELINE_ROOT = os.path.dirname(THIS_DIR)  # .../Recoil Remake/tools/asset_pipeline
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(PIPELINE_ROOT)))  # .../Recoil

DEFAULT_GAME_ZBD_DIR = os.path.join(PROJECT_ROOT, "RECOIL files after setup", "zbd")
DEFAULT_MECH3AX_DIR = os.path.join(
    PROJECT_ROOT, "tools", "mech3ax", "mech3ax-v0.6.1-x86_64-pc-windows-msvc"
)

# Shared (game-wide, not per-mission) structured-data files, extracted from
# the root zrdr.zbd. Source: recoil_re_log.md confirms weapons.json /
# vehicle*.json live in the shared reader archive, not the per-mission one.
SHARED_DATA_FILES = {
    "weapon_mounts": "weapons.json",
    "vehicle_classes": "vehicle.json",
    "vehicle_classes_easy": "vehicle_easy.json",
    "vehicle_classes_hard": "vehicle_hard.json",
}

# Per-mission structured-data files, extracted from <mission>/zrdr.zbd.
MISSION_DATA_FILES = {
    "ai_stats": "ai.json",
    "ai_vehicle_spawns": "aiv.json",
    "ai_vehicle_spawns_easy": "aiv_easy.json",
    "ai_vehicle_spawns_hard": "aiv_hard.json",
    "objectives": "objectives.json",
}

# Every OTHER reader file in the per-mission archive is copied out too, into data/anim_defs/.
# Most of them carry ANIMATION_DEFINITIONS — the door/gate/barrier/destructible mechanism in
# source form (RESET_STATE node states, per-sequence OBJECT_ACTIVE_STATE transitions,
# CALL_ANIMATION chaining, and the ACTIVATION/HEALTH pair that says what fires a definition).
# Map 1 alone has 104 definitions across 45 files. They were previously left in the raw
# extraction directory, so the engine could not see them at all.
#
# Copied VERBATIM, not normalised: these are read by recoil::mission::ParseAnimationDefinitions,
# which walks the archive's own flat "[key, value, key, value, ...]" convention directly. That
# convention is load-bearing here in a way it is not for the files above — an ANIMATION_LIST holds
# many entries under the SAME "ANIMATION_DEFINITION" key, so normalising to a JSON object would
# collapse them and silently lose all but the last.
ANIM_DEF_OUTPUT_SUBDIR = os.path.join("data", "anim_defs")

# The SHARED animation definitions, from the root zrdr.zbd. A mission's own ANIMATION_LIST names
# these explicitly (`..\data\common\zrdr\anim.zrd` and the enemies/*.zrd beside it), and they
# hold the effects every mission calls into: explosions, sparks, smoke, debris.
#
# They are not optional dressing. Of map 1's 204 CALL_ANIMATION targets, 120 resolve inside the
# mission archive and **195 once these are loaded** — so the common archive supplies 75 of them.
# Written to output/common/anim_defs/ rather than per-mission, since every mission shares them.
COMMON_ANIM_DEF_SUBDIR = os.path.join("common", "anim_defs")

TEXTURE_ARCHIVES = [
    "texture2.zbd", "texture4.zbd", "texture6.zbd",
    "rtexture2.zbd", "rtexture4.zbd",
]


def run_unzbd(mech3ax_dir, mode, in_path, out_zip):
    exe = os.path.join(mech3ax_dir, "unzbd.exe")
    proc = subprocess.run(
        [exe, "rc", mode, in_path, out_zip],
        capture_output=True, text=True,
    )
    return proc.returncode, proc.stdout, proc.stderr


def unzip_to(zip_path, dest_dir):
    os.makedirs(dest_dir, exist_ok=True)
    with zipfile.ZipFile(zip_path) as zf:
        zf.extractall(dest_dir)
        return zf.namelist()


def extract_textures(mech3ax_dir, mission_zbd_dir, out_textures_dir, log):
    results = {}
    for archive in TEXTURE_ARCHIVES:
        src = os.path.join(mission_zbd_dir, archive)
        if not os.path.isfile(src):
            log.append(f"textures: SKIP {archive} (not found at {src})")
            continue
        stem = archive[:-4]
        zip_path = os.path.join(out_textures_dir, f"_{stem}.zip")
        os.makedirs(out_textures_dir, exist_ok=True)
        rc, out, err = run_unzbd(mech3ax_dir, "textures", src, zip_path)
        if rc != 0:
            log.append(f"textures: FAILED {archive} rc={rc} stderr={err.strip()[:200]}")
            continue
        dest_dir = os.path.join(out_textures_dir, stem)
        names = unzip_to(zip_path, dest_dir)
        os.remove(zip_path)
        png_count = sum(1 for n in names if n.lower().endswith(".png"))
        results[stem] = {"png_count": png_count, "total_entries": len(names), "dir": dest_dir}
        log.append(f"textures: OK {archive} -> {png_count} PNGs in {dest_dir}")
    return results


def extract_reader_archive(mech3ax_dir, zbd_path, tmp_dir, wanted_files, log, label):
    os.makedirs(tmp_dir, exist_ok=True)
    zip_path = os.path.join(tmp_dir, "_reader.zip")
    rc, out, err = run_unzbd(mech3ax_dir, "reader", zbd_path, zip_path)
    if rc != 0:
        log.append(f"{label}: FAILED unzbd rc={rc} stderr={err.strip()[:300]}")
        return {}
    extract_dir = os.path.join(tmp_dir, "_extracted")
    names = unzip_to(zip_path, extract_dir)
    os.remove(zip_path)
    log.append(f"{label}: reader archive extracted, {len(names)} files total")

    found = {}
    for key, filename in wanted_files.items():
        candidates = [n for n in names if os.path.basename(n) == filename]
        if not candidates:
            log.append(f"{label}: MISSING '{filename}' for '{key}' (not present in this reader archive)")
            continue
        found[key] = os.path.join(extract_dir, candidates[0])
    return found


def extract_net_graphs(mech3ax_dir, mission_zbd_dir_tmp_extract_dir, names, log):
    """net_NN.json files (N=1..99) - variable count per mission, found by
    pattern rather than a fixed list (confirmed convention: NetGraph_
    LoadAllForMission loads net_%02d.zrd, N=1..99, per
    recoil_confirmed_logic.md)."""
    import re
    pattern = re.compile(r"^net_(\d{2})\.json$")
    matches = []
    for n in names:
        base = os.path.basename(n)
        m = pattern.match(base)
        if m:
            matches.append((int(m.group(1)), n))
    matches.sort()
    return matches


def normalize_and_write(src_path, dest_path, log, label):
    with open(src_path) as f:
        raw = json.load(f)
    normalized, warnings = normalize_reader_file(raw)
    with open(dest_path, "w") as f:
        json.dump(normalized, f, indent=1)
    for w in warnings:
        log.append(f"{label}: WARNING {w}")
    return normalized, len(warnings)


def build(mission, game_zbd_dir, mech3ax_dir, out_dir):
    log = []
    report = {"mission": mission, "log": log}

    mission_zbd_dir = os.path.join(game_zbd_dir, mission)
    if not os.path.isdir(mission_zbd_dir):
        raise SystemExit(f"mission directory not found: {mission_zbd_dir}")

    os.makedirs(out_dir, exist_ok=True)
    tmp_dir = os.path.join(out_dir, "_tmp")
    if os.path.isdir(tmp_dir):
        shutil.rmtree(tmp_dir)
    os.makedirs(tmp_dir, exist_ok=True)

    # --- 1. Geometry + scene nodes (gamez.zbd, validated parser) ---
    gamez_path = os.path.join(mission_zbd_dir, "gamez.zbd")
    geometry_dir = os.path.join(out_dir, "geometry")
    geom_result = export_mission_geometry(gamez_path, mission, geometry_dir)
    log.append(
        f"geometry: {geom_result['model_count_exported']}/{geom_result['model_count_source']} "
        f"models exported, {geom_result['triangle_count']} triangles, "
        f"{geom_result['node_count']} scene nodes "
        f"({geom_result['nodes_with_mesh_link']} mesh-linked, "
        f"{geom_result['nodes_with_children']} with children), "
        f"{geom_result['bytes_consumed']}/{geom_result['bytes_total']} bytes consumed (full coverage)"
    )
    log.append(
        f"materials: {geom_result['materials_listed']}/{geom_result['material_count']} live materials, "
        f"{geom_result['textured_materials']} textured, "
        f"{geom_result['materials_resolved_to_texture']} resolved to a texture name -> "
        f"{os.path.basename(geom_result['materials_json_path'])}"
    )
    report["geometry"] = geom_result

    # --- 2. Textures (mech3ax unzbd, confirmed-working per CLAUDE.md) ---
    textures_dir = os.path.join(out_dir, "textures")
    tex_result = extract_textures(mech3ax_dir, mission_zbd_dir, textures_dir, log)
    report["textures"] = tex_result

    # --- 3. Structured data: shared (weapons/vehicle classes) ---
    data_dir = os.path.join(out_dir, "data")
    os.makedirs(data_dir, exist_ok=True)

    shared_zbd = os.path.join(game_zbd_dir, "zrdr.zbd")
    shared_tmp = os.path.join(tmp_dir, "shared_reader")
    shared_files = extract_reader_archive(
        mech3ax_dir, shared_zbd, shared_tmp, SHARED_DATA_FILES, log, "shared-data"
    )
    data_report = {}
    for key, src_path in shared_files.items():
        dest_path = os.path.join(data_dir, f"{key}.json")
        normalized, n_warn = normalize_and_write(src_path, dest_path, log, f"shared-data/{key}")
        data_report[key] = {"path": dest_path, "warnings": n_warn}

    # --- 4. Structured data: per-mission (ai stats, spawns, objectives, net graphs) ---
    mission_zbd = os.path.join(mission_zbd_dir, "zrdr.zbd")
    mission_tmp = os.path.join(tmp_dir, "mission_reader")
    mission_files = extract_reader_archive(
        mech3ax_dir, mission_zbd, mission_tmp, MISSION_DATA_FILES, log, "mission-data"
    )
    for key, src_path in mission_files.items():
        dest_path = os.path.join(data_dir, f"{key}.json")
        normalized, n_warn = normalize_and_write(src_path, dest_path, log, f"mission-data/{key}")
        data_report[key] = {"path": dest_path, "warnings": n_warn}

    # net_NN.json (variable count) - re-list the same extracted reader archive
    mission_extract_dir = os.path.join(mission_tmp, "_extracted")
    all_names = []
    for root, _dirs, files in os.walk(mission_extract_dir):
        for fn in files:
            all_names.append(os.path.relpath(os.path.join(root, fn), mission_extract_dir))
    net_matches = extract_net_graphs(mech3ax_dir, mission_extract_dir, all_names, log)
    net_dir = os.path.join(data_dir, "net_graphs")
    os.makedirs(net_dir, exist_ok=True)
    net_report = []
    total_net_warn = 0
    for n, relname in net_matches:
        src_path = os.path.join(mission_extract_dir, relname)
        dest_path = os.path.join(net_dir, f"net_{n:02d}.json")
        normalized, n_warn = normalize_and_write(src_path, dest_path, log, f"net-graph/net_{n:02d}")
        total_net_warn += n_warn
        net_report.append(dest_path)
    log.append(f"net-graphs: {len(net_report)} patrol-graph files normalized, {total_net_warn} warnings")
    data_report["net_graphs"] = {"count": len(net_report), "dir": net_dir, "warnings": total_net_warn}

    # Animation definition files — copied VERBATIM (see ANIM_DEF_OUTPUT_SUBDIR's comment for why
    # these must NOT be normalised). Everything in the mission's reader archive that is not one of
    # the named files above and is not a net graph.
    anim_dir = os.path.join(out_dir, ANIM_DEF_OUTPUT_SUBDIR)
    os.makedirs(anim_dir, exist_ok=True)
    named = set(MISSION_DATA_FILES.values())
    net_names = {relname for _n, relname in net_matches}
    copied = 0
    for relname in all_names:
        base = os.path.basename(relname)
        if not base.endswith(".json"):
            continue
        if base in named or relname in net_names or base == "manifest.json":
            continue
        shutil.copyfile(os.path.join(mission_extract_dir, relname),
                        os.path.join(anim_dir, base))
        copied += 1
    log.append(f"anim-defs: {copied} animation-definition file(s) copied verbatim to {anim_dir}")
    data_report["anim_defs"] = {"count": copied, "dir": anim_dir}

    report["data"] = data_report

    shutil.rmtree(tmp_dir, ignore_errors=True)

    manifest_path = os.path.join(out_dir, "manifest.json")
    with open(manifest_path, "w") as f:
        json.dump(report, f, indent=1)
    return report


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mission", nargs="?", default="m1")
    ap.add_argument("--game-dir", default=DEFAULT_GAME_ZBD_DIR)
    ap.add_argument("--mech3ax-dir", default=DEFAULT_MECH3AX_DIR)
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    out_dir = args.out or os.path.join(PIPELINE_ROOT, "output", args.mission)
    report = build(args.mission, args.game_dir, args.mech3ax_dir, out_dir)

    print(f"\n=== build_mission_assets: {args.mission} ===")
    for line in report["log"]:
        print(" ", line)
    print(f"\nmanifest written to {os.path.join(out_dir, 'manifest.json')}")


if __name__ == "__main__":
    main()
