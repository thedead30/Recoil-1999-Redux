"""file_attrib.py - TODO_STAGE2 P0.4 (T7): attribute each engine function to one of the original's
source files (the 64 "D:\\Proj\\..." paths in assert/report strings).

Evidence levels written to the ledger column orig_file_evidence:
  ANCHOR   - the function references that file's path string (assert/report call). CONFIRMED-BINARY.
  RANGE    - the function lies between two anchors of the SAME file in address order. MSVC links an
             object file's functions contiguously, so this is INFERRED (alternative: a small file
             with no asserts linked in between - possible but it would break a same-file run).
  SUBSYS   - grown outward from an attributed neighbour of the same subsystem, where that file's anchors
             include that subsystem and only one neighbouring file qualifies. INFERRED (weaker).
  (empty)  - between anchors of different files, before the first or after the last anchor:
             unattributed; the boundary is unknown.
Library/thunk rows are skipped.

Input: an xref CSV (file,func) from the Ghidra script run in P0.4 (strings containing "\\Proj\\").
usage: python 03_re/scripts/file_attrib.py XREF_CSV
"""
import collections
import csv
import io
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")


def short(p):
    p = p.replace("zD:", "D:")
    i = p.find("\\Proj\\")
    return p[i + 6:] if i >= 0 else p


def main():
    anchors = collections.defaultdict(set)
    for r in csv.DictReader(open(sys.argv[1], encoding="utf-8", errors="replace")):
        anchors[r["func"].lower()].add(short(r["file"]))
    rows = list(csv.reader(io.open(LEDGER, encoding="utf-8", newline="")))
    hdr = rows[0]
    for col in ("orig_file", "orig_file_evidence"):
        if col not in hdr:
            hdr.append(col)
    for r in rows[1:]:
        if r:
            r += [""] * (len(hdr) - len(r))
    ia, ic, iof, iev = hdr.index("address"), hdr.index("class"), hdr.index("orig_file"), hdr.index("orig_file_evidence")
    eng = sorted((r for r in rows[1:] if r and r[ic] == "ENGINE"), key=lambda r: int(r[ia], 16))
    multi = 0
    for r in eng:
        f = anchors.get(r[ia].lower())
        r[iof], r[iev] = "", ""
        if f:
            if len(f) > 1:
                multi += 1
            r[iof], r[iev] = sorted(f)[0] if len(f) == 1 else " | ".join(sorted(f)), "ANCHOR"
    # RANGE fill between same-file anchors
    idx = [i for i, r in enumerate(eng) if r[iev] == "ANCHOR" and " | " not in r[iof]]
    for a, b in zip(idx, idx[1:]):
        if eng[a][iof] == eng[b][iof]:
            for k in range(a + 1, b):
                if not eng[k][iof]:
                    eng[k][iof], eng[k][iev] = eng[a][iof], "RANGE"
    # SUBSYS pass: in a gap between anchors of different files (or at either end), a function whose
    # subsystem matches the subsystem of exactly one neighbouring file's anchors, and whose run up to that
    # anchor is unbroken by other subsystems, takes that file. INFERRED, weaker than RANGE.
    sub = {}
    for m in csv.DictReader(open(os.path.join(ROOT, "03_re", "ledger", "subsystems.csv"), encoding="utf-8")):
        sub.setdefault(m["address"].lower(), m["subsystem"])
    file_subs = collections.defaultdict(set)
    for r in eng:
        if r[iev] == "ANCHOR":
            file_subs[r[iof]].add(sub.get(r[ia].lower()))
    changed = True
    while changed:
        changed = False
        for k, r in enumerate(eng):
            if r[iof]:
                continue
            s = sub.get(r[ia].lower())
            cand = set()
            for nb in (k - 1, k + 1):
                if 0 <= nb < len(eng) and eng[nb][iof] and " | " not in eng[nb][iof] \
                        and sub.get(eng[nb][ia].lower()) == s and s in file_subs[eng[nb][iof]]:
                    cand.add(eng[nb][iof])
            if len(cand) == 1:
                r[iof], r[iev] = cand.pop(), "SUBSYS"
                changed = True
    csv.writer(io.open(LEDGER, "w", encoding="utf-8", newline=""), lineterminator="\n").writerows(rows)
    c = collections.Counter(r[iev] or "UNATTRIBUTED" for r in eng)
    files = collections.Counter(r[iof] for r in eng if r[iof])
    print("engine:", len(eng), dict(c), "| files used:", len(files), "| multi-file anchors:", multi)


if __name__ == "__main__":
    main()
