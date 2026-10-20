"""remake_set.py - record Stage 2 results in the ledger (remake_file, remake_symbol, remake_verification).

usage: python 03_re/scripts/remake_set.py ADDR FILE SYMBOL "VERIFICATION TAG (evidence)" [ADDR FILE SYMBOL TAG ...]
The verification text must start with a CLAUDE.md tag (VERIFIED-* or IMPLEMENTED-UNVERIFIED).
"""
import csv
import io
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")
TAGS = ("VERIFIED-PLAYPATH", "VERIFIED-VISUAL", "VERIFIED-ORACLE", "VERIFIED-UNIT", "IMPLEMENTED-UNVERIFIED")


def main():
    args = sys.argv[1:]
    if not args or len(args) % 4:
        sys.exit(__doc__)
    want = {}
    for i in range(0, len(args), 4):
        a, f, s, v = args[i:i + 4]
        if not v.startswith(TAGS):
            sys.exit("verification must start with one of %s: %r" % (TAGS, v))
        want[a.lower()] = (f, s, v)
    rows = list(csv.reader(io.open(LEDGER, encoding="utf-8", newline="")))
    h = rows[0]
    ia, ist = h.index("address"), h.index("status")
    done = set()
    for r in rows[1:]:
        if r and r[ia].lower() in want:
            if r[ist] != "CONFIRMED":
                sys.exit("%s is not CONFIRMED - no code for unconfirmed functions (CLAUDE.md rule 2)" % r[ia])
            r[h.index("remake_file")], r[h.index("remake_symbol")], r[h.index("remake_verification")] = want[r[ia].lower()]
            done.add(r[ia].lower())
    missing = set(want) - done
    if missing:
        sys.exit("not in ledger: %s" % sorted(missing))
    csv.writer(io.open(LEDGER, "w", encoding="utf-8", newline=""), lineterminator="\n").writerows(rows)
    print("recorded", len(done))


if __name__ == "__main__":
    main()
