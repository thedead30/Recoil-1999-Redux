"""spec_gap.py - how much of each subsystem is written up in narrative 04_spec/ docs.

A member counts as documented when its address (0x%08x, any case) or its ledger
name appears in at least one hand-written 04_spec/**/*.md file. The generated
04_spec/reference/ docs are excluded. Mentioned is not the same as specified:
this is a floor on the gap, not proof of a good spec.

usage: python 03_re/scripts/spec_gap.py [--md OUT.md] [--missing SUBSYSTEM]
"""
import collections
import csv
import glob
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")
SUBSYS = os.path.join(ROOT, "03_re", "ledger", "subsystems.csv")
REGISTRY = os.path.join(ROOT, "03_re", "ledger", "subsystem_registry.csv")
SPEC = os.path.join(ROOT, "04_spec")


def load():
    fn = {r["address"].lower(): r for r in csv.DictReader(open(LEDGER, encoding="utf-8"))}
    reg = [r["subsystem"] for r in csv.DictReader(open(REGISTRY, encoding="utf-8"))]
    mem = collections.defaultdict(set)
    for m in csv.DictReader(open(SUBSYS, encoding="utf-8")):
        a = m["address"].lower()
        if a in fn and fn[a]["class"] == "ENGINE":
            mem[m["subsystem"]].add(a)
    docs = {}
    for p in glob.glob(os.path.join(SPEC, "**", "*.md"), recursive=True):
        rel = os.path.relpath(p, SPEC).replace("\\", "/")
        # reference/ is generated from the ledger by gen_spec_ref.py and mentions every
        # member by construction; SPEC_GAP.md is this script's own output. Neither is
        # narrative, so neither may count as documentation.
        if rel.startswith("reference/") or rel == "SPEC_GAP.md":
            continue
        docs[rel] = open(
            p, encoding="utf-8", errors="replace").read()
    return fn, reg, mem, docs


def documented(addr, name, docs):
    pat = re.compile(re.escape(addr), re.I)
    hits = [d for d, t in docs.items() if pat.search(t)]
    if not hits and name and not name.startswith("FUN_"):
        wn = re.compile(r"\b%s\b" % re.escape(name))
        hits = [d for d, t in docs.items() if wn.search(t)]
    return hits


def main():
    args = sys.argv[1:]
    fn, reg, mem, docs = load()
    rows = []
    for s in reg:
        m = sorted(mem.get(s, ()))
        own = [d for d in docs if os.path.splitext(os.path.basename(d))[0] == s]
        undoc = [a for a in m if not documented(a, fn[a]["ghidra_name"], docs)]
        rows.append((s, len(m), len(m) - len(undoc), undoc, own))
    if "--missing" in args:
        s = args[args.index("--missing") + 1]
        for r in rows:
            if r[0] == s:
                for a in r[3]:
                    print(a, fn[a]["ghidra_name"])
        return 0
    tot = sum(r[1] for r in rows)
    doc = sum(r[2] for r in rows)
    lines = ["| subsystem | members | documented | % | own spec file |",
             "|---|---:|---:|---:|---|"]
    for s, n, d, _, own in sorted(rows, key=lambda r: (r[2] / r[1] if r[1] else 1, -r[1])):
        lines.append("| %s | %d | %d | %d%% | %s |" % (
            s, n, d, round(100.0 * d / n) if n else 100, ", ".join(own) or "**none**"))
    lines.append("| **total** | %d | %d | %d%% | |" % (tot, doc, round(100.0 * doc / tot)))
    out = "\n".join(lines)
    print(out)
    if "--md" in args:
        open(args[args.index("--md") + 1], "w", encoding="utf-8").write(out + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
