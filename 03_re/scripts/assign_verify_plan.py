"""assign_verify_plan.py - fill the ledger columns verify_plan / verify_plan_reason (TODO item F1).

Plans:
  ORACLE          targeted TTD trace or state-tagged capture of the original (H1); gameplay
                  behaviour and anything visual (visuals are accepted only by machine frame
                  comparison against captures, per the Stage 2 scope decisions)
  EMULATE         Ghidra emulate_function on the original bytes vs the port (pure functions)
  UNIT            unit test against real asset files or round-trips
  STATIC-ACCEPTED no dynamic check planned; the reason says why (trivial, platform, out of scope)

Rules are a harvest default by subsystem and name pattern; a row whose plan was set by hand
(reason not starting with "default:") is never overwritten. Rows already VERIFIED-ORACLE or
ORACLE-TTD get ORACLE with reason "done: <verification>".
usage: python 03_re/scripts/assign_verify_plan.py
"""
import csv
import io
import os
import re

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")

BY_SUB = {
    "ORACLE": ("vehicle weapon player mission collision turret pickup camera ai_net declient zeffect "
               "hud menus ui_widgets mapscreen render_frame zvideo scene_render scene_update object3d "
               "cls_world class_api app input sound avi",
               "gameplay/timing or visual behaviour: targeted trace or state-tagged capture + machine frame compare"),
    "EMULATE": ("math3d transform geometry rasteriser poly_shade view",
                "pure computation / pixel operation: emulate the original bytes and compare outputs"),
    "UNIT": ("asset_io texture script settings savegame zclass_nodes",
             "file/data handling: unit test against the shipped asset files and round-trips"),
    "STATIC-ACCEPTED": ("net znetwork sysinfo",
                        "out of v1 scope (multiplayer) or PLATFORM host probing replaced by the remake"),
}
TRIVIAL = re.compile(r"^(Stub_|Thunk_|Wrap_|OperatorDelete|Nullsub|Ret)|_Thunk|ScalarDeletingDtor|_GetGlobal_|_SetGlobal_|^Setting_Get_")


def main():
    sub = {}
    for r in csv.DictReader(open(os.path.join(ROOT, "03_re", "ledger", "subsystems.csv"), encoding="utf-8")):
        sub.setdefault(r["address"], r["subsystem"])
    plan_of = {}
    for plan, (subs, why) in BY_SUB.items():
        for s in subs.split():
            plan_of[s] = (plan, why)
    rows = list(csv.reader(io.open(LEDGER, encoding="utf-8", newline="")))
    hdr = rows[0]
    for col in ("verify_plan", "verify_plan_reason"):
        if col not in hdr:
            hdr.append(col)
    ip, ir = hdr.index("verify_plan"), hdr.index("verify_plan_reason")
    ic, iv, iname = hdr.index("class"), hdr.index("verification"), hdr.index("ghidra_name")
    n = 0
    for r in rows[1:]:
        if not r:
            continue
        r += [""] * (len(hdr) - len(r))
        if r[ic] != "ENGINE":
            continue
        if r[ir] and not r[ir].startswith("default:") and not r[ir].startswith("done:"):
            continue
        if r[iv] in ("VERIFIED-ORACLE", "ORACLE-TTD"):
            r[ip], r[ir] = "ORACLE", "done: " + r[iv]
        elif TRIVIAL.search(r[iname]):
            r[ip], r[ir] = "STATIC-ACCEPTED", "default: trivial accessor/thunk/stub - bytes read, no behaviour to test"
        elif r[0] in sub and sub[r[0]] in plan_of:
            p, why = plan_of[sub[r[0]]]
            r[ip], r[ir] = p, "default: %s (%s)" % (why, sub[r[0]])
        else:
            r[ip], r[ir] = "", ""
        n += 1
    csv.writer(io.open(LEDGER, "w", encoding="utf-8", newline=""), lineterminator="\n").writerows(rows)
    print("plans assigned/refreshed for", n, "engine rows")


if __name__ == "__main__":
    main()
