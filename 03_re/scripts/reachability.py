#!/usr/bin/env python3
"""
reachability.py - compute map 1's reachable function set.

SOUNDNESS NOTE. Recoil.exe is C++ with vtable dispatch: hundreds of engine functions
have no static caller because they are reached through function pointers. A forward walk
over direct call edges alone therefore UNDER-counts, and would open the Stage 2 gate
while whole subsystems (Vehicle_MainUpdateTick, Weapon_ProjectileUpdateTick) were still
unread.

For a gate, under-counting is the dangerous error and over-counting is merely slow. So
this computes two figures and the gate uses the larger:

  LOWER  direct call edges only
  UPPER  direct edges + indirect edges recovered from DATA references, where an orphan is
         pulled in if the function that installs its pointer (or constructs its class) is
         itself reachable, iterated to a fixpoint.

Neither is exact. Exact requires replaying the TTD traces in 01_evidence/ttd_tools.

Inputs, both regenerable from Ghidra and committed to the repo:
  03_re/ledger/_callgraph.tsv          (DumpCallGraph.java)
  03_re/ledger/_orphan_installers.json (DumpIndirectEdges.java -> build_installers step)
"""
import collections
import csv
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")

# Gameplay roots. Each is a named function whose role in a map-1 session is evident from
# its name and call profile. Menu, multiplayer, settings and installer roots are
# deliberately excluded - they are not map-1 behaviour.
ROOTS = {
    "0x0042bb30": "Game_HandleStateEvent",
    "0x00426390": "Vehicle_MainUpdateTick",
    "0x004af060": "Weapon_ProjectileUpdateTick",
    "0x00436e40": "Turret_UpdateAITick",
    "0x00418d40": "Mission_TickAndCheckObjectiveCompletion",
    "0x004184e0": "Mission_ObjectiveStateMachine",
    "0x00417f90": "Mission_LoadObjectivesArray",
    "0x00418230": "Mission_LoadObjectiveTriggers",
    "0x0041fe90": "Player_InitPhysicsGlobalsAndVehicleClasses",
    "0x00442a50": "Engine_InitSubsystems",
    "0x004b1190": "Weapon_LoadMountArrayFromConfig",
    "0x00437ac0": "Turret_SpawnAllFromConfig",
    "0x00421ab0": "Vehicle_SpawnInstanceFromConfig",
    "0x00420d10": "Vehicle_ResolveClassConfigAndNetAssignment",
    "0x00402fd0": "NetGraph_LoadAllForMission",
    "0x0045efb0": "Anim_LoadFile",
    "0x00445650": "Collision_TestNodeHierarchy",
    "0x00444e90": "Collision_GridDDATraversal",
    "0x0043bcc0": "Vehicle_TakeDamage",
    "0x0043b870": "AIVehicle_TakeDamage",
    "0x004b26f0": "Weapon_ApplyDamageToTarget",
    "0x00447c60": "gwNodeSetActive",
    # --- added 2026-09-09 -------------------------------------------------
    # The render subsystem was entirely absent from the first computation: it is
    # reached through an installed DRAW DISPATCH TABLE (function pointers), which
    # a static call-graph walk cannot follow. The game plainly renders map 1, so
    # their exclusion was a defect in the reachable set, not a fact about the game.
    # Same for the audio and input entry points.
    "0x0044d600": "Render_CameraFrame_HW",
    "0x0044d3a0": "Render_CameraFrame_SW",
    "0x0044c0e0": "Render_DispatchNodeByType",
    "0x0044ce70": "Render_CullAndQueueSceneObjects",
    "0x0044f630": "Render_DispatchCameraList",
    "0x00477b30": "Render_ShadeAndQueueObjectPolygons",
    "0x00476cf0": "Render_ShadeAndQueueObjectPolygons_SW",
    "0x004a77a0": "Render_InstallDrawDispatchTable",
    "0x004a7530": "Render_BringUpDirectDraw",
    "0x004a88f0": "Render_CreatePrimarySurfacesAndZBuffer",
    "0x004a9c20": "Render_CreateD3DDevice",
    "0x004ad250": "Render_FlushMainDrawQueue",
    "0x004ace30": "Render_FlushSortedDrawQueue",
    "0x004ad120": "Render_FlushQuadDrawQueue",
    "0x0044b140": "Render_ProcessLightNode",
    "0x0044b300": "Render_ProcessObject3DNode",
    "0x0044b8c0": "Render_ProcessLodDistanceNode",
    "0x0044ada0": "Render_ProcessCameraNode",
    "0x0044af60": "Render_ProcessSoundNode",
    "0x00478c70": "Render_TestBoundSphereAgainstFrustum",
}

TAB = chr(9)
NL = chr(10)


def norm(a):
    return "0x" + a.lower().replace("0x", "").rjust(8, "0")


def load(cg_path):
    """Read the ledger and the call graph TSV dumped by DumpCallGraph.java."""
    rows = list(csv.DictReader(open(LEDGER, encoding="utf-8")))
    by_addr = {r["address"]: r for r in rows}
    out = collections.defaultdict(set)
    with open(cg_path, encoding="utf-8") as fh:
        next(fh)
        for line in fh:
            parts = line.rstrip(NL).split(TAB)
            if len(parts) < 2:
                continue
            out[norm(parts[0])].add(norm(parts[1]))
    return rows, by_addr, out


def closure(roots, out, universe):
    seen, stack = set(), list(roots)
    while stack:
        n = stack.pop()
        if n in seen or n not in universe:
            continue
        seen.add(n)
        stack.extend(out.get(n, ()))
    return seen


def main():
    cg = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        ROOT, "03_re", "ledger", "_callgraph.tsv")
    rows, by_addr, out = load(cg)
    eng = {r["address"] for r in rows if r["class"] == "ENGINE"}

    missing = [a for a in ROOTS if a not in by_addr]
    if missing:
        print("WARNING roots not in ledger:", missing)

    lower = closure(set(ROOTS) & eng, out, eng)
    print("LOWER (direct edges only) : %d / %d engine  (%.1f%%)"
          % (len(lower), len(eng), 100.0 * len(lower) / len(eng)))

    upper = set(lower)
    inst_path = os.path.join(ROOT, "03_re", "ledger", "_orphan_installers.json")
    if os.path.exists(inst_path):
        inst = json.load(open(inst_path, encoding="utf-8"))
        added = 0
        for _ in range(24):
            grew = False
            for orphan, installers in inst.items():
                o = norm(orphan)
                if o in upper or o not in eng:
                    continue
                if any(norm(i) in upper for i in installers):
                    before = len(upper)
                    upper |= closure({o}, out, eng)
                    added += len(upper) - before
                    grew = True
            if not grew:
                break
        print("UPPER (+ indirect edges)  : %d / %d engine  (%.1f%%)   [+%d via %d orphans]"
              % (len(upper), len(eng), 100.0 * len(upper) / len(eng), added, len(inst)))
    else:
        print("UPPER: skipped - no _orphan_installers.json")

    for r in rows:
        if r["class"] != "ENGINE":
            r["map1_reachable"] = ""
        else:
            r["map1_reachable"] = "YES" if r["address"] in upper else "NO"
    with open(LEDGER, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    print("ledger updated: %d marked YES"
          % sum(1 for r in rows if r["map1_reachable"] == "YES"))


if __name__ == "__main__":
    main()
