#!/usr/bin/env python3
"""set_status.py - change a ledger row's status reproducibly, never by hand-editing.

  python 03_re/scripts/set_status.py 0x00426390 DECOMPILED --record 0x00426390_Vehicle_MainUpdateTick.md
"""
import argparse, csv, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")
VALID = {"UNTOUCHED","NAMED","PRIOR-EVIDENCE","DECOMPILED","CONFIRMED","WAIVED"}

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("address"); ap.add_argument("status")
    ap.add_argument("--record", default=""); ap.add_argument("--verification", default="")
    ap.add_argument("--notes", default="")
    a = ap.parse_args()
    if a.status not in VALID:
        sys.exit(f"bad status {a.status}; valid: {sorted(VALID)}")
    if a.status == "CONFIRMED" and not a.record:
        sys.exit("CONFIRMED requires --record (STAGE1.md bar #1)")
    if a.status == "CONFIRMED" and not a.verification:
        sys.exit("CONFIRMED requires --verification (STAGE1.md bar #5)")
    rows = list(csv.DictReader(open(LEDGER, encoding="utf-8")))
    hit = 0
    for r in rows:
        if r["address"].lower() == a.address.lower():
            r["status"] = a.status
            if a.record: r["spec_ref"] = a.record
            if a.verification: r["verification"] = a.verification
            if a.notes: r["notes"] = a.notes
            hit += 1
            print(f"{r['address']} {r['ghidra_name']} -> {a.status}")
    if not hit: sys.exit(f"address {a.address} not in ledger")
    with open(LEDGER,"w",newline="",encoding="utf-8") as fh:
        w=csv.DictWriter(fh,fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)

if __name__ == "__main__":
    main()
