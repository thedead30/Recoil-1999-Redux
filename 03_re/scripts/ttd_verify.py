#!/usr/bin/env python3
"""
ttd_verify.py - run commands against a TTD trace of the real game and return the output.

This is the ORACLE. Until it existed, CONFIRMED could never rise above zero, because
STAGE1.md requires a behaviour-bearing function to be checked against the running original
before its ledger row advances.

Usage
-----
  python 03_re/scripts/ttd_verify.py --cmds "bp 0x426390; g; r; q"
  python 03_re/scripts/ttd_verify.py --trace Recoil07 --cmds "u 0x42e75a L4; q"
  python 03_re/scripts/ttd_verify.py --list

Notes
-----
* `cdbX86.exe` ships with the WinDbg Store package; no separate install is needed.
* Only the LARGE traces contain execution. Recoil03/04/05 are recorded but empty
  ("doesn't contain any recorded threads") - they are rejected here rather than
  silently producing nothing.
* The trace was recorded against the NO-CD build. It differs from the Ghidra build by
  exactly ONE byte: VA 0x0042e75a, JNZ (75) -> JMP (eb), the CD check. Every other address
  is directly comparable. Do not treat 0x0042e75a as verifiable through TTD.
"""
import argparse
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TRACES = os.path.join(ROOT, "01_evidence", "ttd_tools", "game_trace")
CDB = os.path.join(os.environ.get("LOCALAPPDATA", ""), "Microsoft", "WindowsApps",
                   "Microsoft.WinDbg_8wekyb3d8bbwe", "cdbX86.exe")

# Verified 2026-09-09 to contain real execution.
GOOD = ["Recoil02", "Recoil07", "Recoil09", "Recoil14", "Recoil16", "Recoil17", "Recoil18", "Recoil19", "Recoil20", "Recoil21", "Recoil22"]
# Verified empty - "doesn't contain any recorded threads".
EMPTY = ["Recoil03", "Recoil04", "Recoil05"]

# The single byte where the trace build differs from the Ghidra build (the no-CD patch).
PATCHED_VA = 0x0042e75a

NOISE = (
    "Debugger Extensions Gallery", "ExtensionRepository", "UseExperimentalFeature",
    "AllowNuget", "NonInteractiveNuget", "AllowParallel", "----> Repository",
    "-- Configuring repositories", ">>>>>>>>>>>>>", "*************",
    "NatVis script", "JavaScript script", "Copyright (c) Microsoft",
    "Microsoft (R) Windows Debugger", "Symbol search path", "Executable search path",
)


def clean(lines):
    return [l for l in lines if l.strip() and not any(n in l for n in NOISE)]


def run(trace, cmds, timeout=300):
    if not os.path.isfile(CDB):
        sys.exit("cdbX86.exe not found at %s" % CDB)
    if trace in EMPTY:
        sys.exit("%s is an EMPTY trace (no recorded threads). Use one of: %s"
                 % (trace, ", ".join(GOOD)))
    path = os.path.join(TRACES, trace + ".run")
    if not os.path.isfile(path):
        sys.exit("no such trace: %s" % path)
    # Always suppress first-chance guard-page violations. Recoil raises one at
    # roughly 12% of Recoil18 (eip 0x0048f8fd, inside an ordinary word-copy loop),
    # and without this `g` HALTS THERE. That halt was misread as end-of-trace across
    # several passes, turning "g stopped early" into a false "never executes".
    # 0x0048f8fd is NOT an end-of-trace marker - it is `mov word ptr [ecx+esi],dx`.
    # Prefer `TTD.Memory(a, a+1, "e").Count()` for liveness anyway; it cannot halt.
    if not cmds.lstrip().startswith("sxi gp"):
        cmds = "sxi gp; " + cmds.lstrip()
    if not cmds.rstrip().endswith("q"):
        cmds = cmds.rstrip().rstrip(";") + "; q"
    proc = subprocess.run([CDB, "-z", path, "-c", cmds],
                          capture_output=True, text=True, timeout=timeout)
    return clean((proc.stdout + proc.stderr).splitlines())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--trace", default="Recoil07",
                    help="trace name without .run (default Recoil07)")
    ap.add_argument("--cmds", help="cdb command string; 'q' is appended if absent")
    ap.add_argument("--timeout", type=int, default=300)
    ap.add_argument("--list", action="store_true", help="list traces and their usability")
    a = ap.parse_args()

    if a.list:
        for fn in sorted(os.listdir(TRACES)):
            if not fn.endswith(".run"):
                continue
            name = fn[:-4]
            mb = os.path.getsize(os.path.join(TRACES, fn)) / 1e6
            tag = "USABLE" if name in GOOD else ("EMPTY" if name in EMPTY else "unchecked")
            print("  %-12s %8.0f MB   %s" % (name, mb, tag))
        return

    if not a.cmds:
        sys.exit("--cmds is required (or use --list)")
    for line in run(a.trace, a.cmds, a.timeout):
        print(line)


if __name__ == "__main__":
    main()
