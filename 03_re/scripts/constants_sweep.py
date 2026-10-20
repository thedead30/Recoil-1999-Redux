#!/usr/bin/env python3
"""
constants_sweep.py - inventory every hardcoded float in the decompiled corpus.

Recoil is data-driven: the binary holds formulas, the .zbd config tree holds tuning.
This sweep is how we know that (115 distinct literals across 1,287 functions) and how we
find the ones that ARE baked in, so each can be attributed to a function and tagged.

Reads  03_re/decomp_raw/*.c   (produced by BulkDecompile.java)
Writes 03_re/ledger/_constants.tsv
"""
import collections
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CORPUS = os.path.join(ROOT, "03_re", "decomp_raw")
OUT = os.path.join(ROOT, "03_re", "ledger", "_constants.tsv")

# Structural values that carry no game semantics.
TRIVIAL = {"0.0", "1.0", "-1.0", "2.0", "0.5", "-2.0", "-0.0"}
FLOAT = re.compile(r"(?<![\w.])(-?\d+\.\d+(?:e-?\d+)?)(?![\w.])")

# Values that are mathematics, not tuning. Keeping them separate stops them
# cluttering the list of things that still need attributing.
MATH = {
    "6.2831855": "2*pi",
    "-6.2831855": "-2*pi",
    "3.1415927": "pi",
    "-3.1415927": "-pi",
    "1.5707964": "pi/2",
    "0.017453292": "deg->rad (pi/180)",
    "57.295776": "rad->deg (180/pi)",
    "0.003921569": "1/255",
    "255.0": "byte colour range",
    "-65536.0": "2^16 fixed point",
    "65536.0": "2^16 fixed point",
    "0.6667": "2/3",
    "-0.5236": "-pi/6 (-30 deg)",
    "0.5236": "pi/6 (30 deg)",
}


def main():
    if not os.path.isdir(CORPUS):
        sys.exit("no corpus at %s - run BulkDecompile.java first" % CORPUS)
    counts = collections.Counter()
    where = collections.defaultdict(set)
    files = 0
    for fn in sorted(os.listdir(CORPUS)):
        if not fn.endswith(".c"):
            continue
        files += 1
        path = os.path.join(CORPUS, fn)
        txt = open(path, encoding="utf-8", errors="replace").read()
        addr = fn.split("_")[0]
        for m in FLOAT.finditer(txt):
            v = m.group(1)
            if v in TRIVIAL:
                continue
            counts[v] += 1
            where[v].add(addr)

    rows = sorted(counts.items(), key=lambda kv: (-kv[1], kv[0]))
    with open(OUT, "w", encoding="utf-8") as fh:
        fh.write("value\toccurrences\tdistinct_functions\tkind\tfunctions\n")
        for v, c in rows:
            kind = MATH.get(v, "UNATTRIBUTED")
            fh.write("%s\t%d\t%d\t%s\t%s\n"
                     % (v, c, len(where[v]), kind, ",".join(sorted(where[v])[:12])))

    math_n = sum(1 for v, _ in rows if v in MATH)
    print("files scanned         : %d" % files)
    print("distinct float values : %d" % len(rows))
    print("  mathematical        : %d" % math_n)
    print("  needing attribution : %d" % (len(rows) - math_n))
    print("wrote %s" % OUT)


if __name__ == "__main__":
    main()
