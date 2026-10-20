#!/usr/bin/env python3
"""
build_ledger.py — regenerate the function correspondence ledger from Ghidra dumps.

The ledger is the single source of truth for Stage 1 progress. Every function in
Recoil.exe gets exactly one row. Status is never edited by hand in a way that
cannot be reproduced: evidence columns cite an address, a doc line, or a file.

Inputs (dumped from GhidraMCP, paths passed as args):
  --functions  JSON {result: "<json>"} from list_functions_enhanced
  --callgraph  JSON {result: "<json>"} from get_full_call_graph (format=edges)
  --mine       one or more legacy .md docs to mine for prior evidence addresses

Output:
  ledger/functions.csv
"""
import argparse, csv, json, os, re, sys
from collections import defaultdict

LIB_PAT = re.compile(
    r'^(_|__|\?|@|Ordinal|std::|operator|Catch@|Unwind@|FID_conflict)'
    r'|::'
    r'|^(malloc|free|memcpy|memset|strlen|strcpy|sprintf|printf|fopen|fclose)$'
)
AUTO_PAT = re.compile(r'^(FUN_|thunk_FUN_|LAB_|SUB_|UndefinedFunction_)')
ADDR_PAT = re.compile(r'0x([0-9a-fA-F]{6,8})\b')


def load_result(path):
    with open(path, encoding='utf-8') as fh:
        return json.loads(json.load(fh)["result"])


def norm(a):
    return a.lower().lstrip("0x").rjust(8, "0")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--functions", required=True)
    ap.add_argument("--callgraph", required=True)
    ap.add_argument("--mine", nargs="*", default=[])
    ap.add_argument("--out", required=True)
    a = ap.parse_args()

    fobj = load_result(a.functions)
    funcs = fobj["functions"] if isinstance(fobj, dict) and "functions" in fobj else fobj

    edges = load_result(a.callgraph)["edges"]
    callers = defaultdict(set)
    callees = defaultdict(set)
    for e in edges:
        ca = e["caller"].rsplit("@", 1)[-1]
        ce = e["callee"].rsplit("@", 1)[-1]
        callees[norm(ca)].add(norm(ce))
        callers[norm(ce)].add(norm(ca))

    # Mine legacy docs: an address mentioned in the prior RE record is *evidence
    # that someone looked*, not proof it is confirmed. It becomes PRIOR-EVIDENCE
    # and must be re-verified before it can reach CONFIRMED.
    mentions = defaultdict(list)
    for doc in a.mine:
        if not os.path.exists(doc):
            continue
        base = os.path.basename(doc)
        with open(doc, encoding='utf-8', errors='replace') as fh:
            for ln, line in enumerate(fh, 1):
                for m in ADDR_PAT.finditer(line):
                    v = norm(m.group(1))
                    if v.startswith("004"):          # inside Recoil.exe image
                        mentions[v].append(f"{base}:{ln}")

    rows = []
    for f in funcs:
        addr = norm(f["address"])
        name = f.get("name", "")
        thunk = bool(f.get("isThunk"))
        ext = bool(f.get("isExternal"))

        if ext or LIB_PAT.search(name):
            cls, status = "LIBRARY", "EXCLUDED"
        elif thunk:
            cls, status = "THUNK", "EXCLUDED"
        elif AUTO_PAT.match(name):
            cls, status = "ENGINE", "UNTOUCHED"
        else:
            cls, status = "ENGINE", "NAMED"

        ev = mentions.get(addr, [])
        if status in ("UNTOUCHED", "NAMED") and ev:
            status = "PRIOR-EVIDENCE"

        rows.append({
            "address": "0x" + addr,
            "ghidra_name": name,
            "class": cls,
            "status": status,
            "map1_reachable": "UNKNOWN",
            "num_callers": len(callers.get(addr, ())),
            "num_callees": len(callees.get(addr, ())),
            "spec_ref": "",
            "remake_file": "",
            "remake_symbol": "",
            "verification": "",
            "prior_evidence": ";".join(ev[:4]),
            "notes": "",
        })

    rows.sort(key=lambda r: r["address"])
    os.makedirs(os.path.dirname(a.out), exist_ok=True)
    with open(a.out, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)
    print(f"wrote {len(rows)} rows -> {a.out}")


if __name__ == "__main__":
    main()
