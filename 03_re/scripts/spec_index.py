"""spec_index.py - append a grouped function index for the undocumented members of a subsystem
to its narrative spec, with descriptions condensed from the ledger notes.

Used for boilerplate-heavy subsystems (UI, front end) where a per-function narrative adds
nothing. Groups by name prefix (text before the first '_'). The section is labelled as
condensed from the ledger so it is never mistaken for a hand-written narrative.

usage: python 03_re/scripts/spec_index.py SUBSYSTEM SPECFILE [--title TEXT] [--note TEXT]
"""
import collections
import csv
import io
import os
import re
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def main():
    sub, spec = sys.argv[1], sys.argv[2]
    title = "Function index (added 2026-09-24, condensed from ledger notes)"
    note = ""
    if "--title" in sys.argv:
        title = sys.argv[sys.argv.index("--title") + 1]
    if "--note" in sys.argv:
        note = sys.argv[sys.argv.index("--note") + 1]
    led = {r["address"].lower(): r for r in csv.DictReader(
        open(os.path.join(ROOT, "03_re", "ledger", "functions.csv"), encoding="utf-8"))}
    out = subprocess.run([sys.executable, os.path.join(ROOT, "03_re", "scripts", "spec_gap.py"),
                          "--missing", sub], capture_output=True, text=True).stdout
    miss = [l.split()[0].lower() for l in out.splitlines() if l.strip()]
    if not miss:
        print("nothing missing for", sub)
        return 0
    groups = collections.defaultdict(list)
    for a in miss:
        n = led[a]["ghidra_name"]
        groups[n.split("_")[0] if "_" in n and not n.startswith("FUN_") else "(unnamed)"].append(a)

    def desc(r):
        d = r["spec_ref"] if r["spec_ref"] and not r["spec_ref"].endswith(".md") else r["notes"]
        d = re.sub(r"\s+", " ", d).replace("|", "/")
        return d[:110] + "..." if len(d) > 110 else d

    lines = ["## " + title, "",
             "Descriptions are condensed from the ledger notes; full text is in "
             "[`../reference/%s.md`](../reference/%s.md)." % (sub, sub)]
    if note:
        lines.append(note)
    lines.append("")
    for g in sorted(groups, key=lambda k: (-len(groups[k]), k)):
        lines.append("### " + g)
        for a in sorted(groups[g], key=lambda x: led[x]["ghidra_name"]):
            lines.append("- `%s` `%s`: %s" % (led[a]["ghidra_name"], a, desc(led[a])))
        lines.append("")
    p = os.path.join(ROOT, spec)
    t = io.open(p, encoding="utf-8").read() if os.path.exists(p) else "# %s\n" % sub
    block = "\n".join(lines)
    for m in ("## Still open", "## Open", "## Not yet established"):
        if m in t:
            t = t.replace(m, block + "\n" + m, 1)
            break
    else:
        t = t.rstrip("\n") + "\n\n" + block + "\n"
    io.open(p, "w", encoding="utf-8", newline="").write(t)
    print(sub, "indexed", len(miss), "in", len(groups), "groups")
    return 0


if __name__ == "__main__":
    sys.exit(main())
