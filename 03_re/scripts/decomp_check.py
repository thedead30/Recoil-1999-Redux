"""decomp_check.py - TODO_STAGE2 P0.3 / KG-12: which decompiles are missing code the listing has.

Input: raw CSV from 06_tools/ghidra_scripts/DecompCheck.java (run over 0x401000-0x4cc000).
Filters applied here (each removes a known false-positive class, not a real miss):
  - rows that are not ENGINE functions in the ledger;
  - data refs into the import address table (the decompile names the import instead);
  - data refs to read-only non-string data (float/double constants the decompiler inlines as
    values, e.g. 0.5), so only strings and writable globals count.
A function is FLAGGED when a direct call target or a string/global reference in its listing is
absent from its decompile. Flagged CONFIRMED functions whose ledger evidence says "decomp" only
need a listing re-read before translation (STAGE2.md section 7 step 2).

kind: DROPPED-CODE = a direct call or a non-register reference is missing (code the decompile lost);
      HIDDEN-REG-ARG = only addresses/values loaded into a register are missing (mostly call
      arguments the decompile does not show because the callee has no prototype: the code is there,
      the call's argument list in the decompile is incomplete).
Output: 03_re/ledger/decomp_check.csv
usage: python 03_re/scripts/decomp_check.py RAW_CSV
"""
import csv
import io
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
EXE = os.path.join(ROOT, "00_original", "game_install", "Recoil.exe")


def sections():
    b = open(EXE, "rb").read()
    pe = struct.unpack_from("<I", b, 0x3c)[0]
    n = struct.unpack_from("<H", b, pe + 6)[0]
    opt = struct.unpack_from("<H", b, pe + 20)[0]
    ib = struct.unpack_from("<I", b, pe + 52)[0]
    iat_rva, iat_size = struct.unpack_from("<II", b, pe + 24 + 96 + 12 * 8)
    secs = []
    for i in range(n):
        name, vs, va, rs, ro = struct.unpack_from("<8sIIII", b, pe + 24 + opt + i * 40)
        ch = struct.unpack_from("<I", b, pe + 24 + opt + i * 40 + 36)[0]
        secs.append((ib + va, ib + va + max(vs, rs), bool(ch & 0x80000000), name.rstrip(b"\0").decode()))
    return b, ib, secs, (ib + iat_rva, ib + iat_rva + iat_size)


def main():
    raw = sys.argv[1]
    b, ib, secs, iat = sections()

    def writable(a):
        return any(lo <= a < hi and w for lo, hi, w, _ in secs)

    def file_off(a):
        for lo, hi, _, _ in secs:
            if lo <= a < hi:
                pass
        return None

    led = {r["address"].lower(): r for r in csv.DictReader(open(os.path.join(ROOT, "03_re", "ledger", "functions.csv"), encoding="utf-8"))}
    # string starts: printable run >= 3 followed by NUL at the address (checked against the file)
    pe = struct.unpack_from("<I", b, 0x3c)[0]
    n = struct.unpack_from("<H", b, pe + 6)[0]
    opt = struct.unpack_from("<H", b, pe + 20)[0]
    raw_secs = [struct.unpack_from("<8sIIII", b, pe + 24 + opt + i * 40) for i in range(n)]

    def fo(a):
        r = a - ib
        for _, vs, va, rs, ro in raw_secs:
            if va <= r < va + rs:
                return r - va + ro
        return None

    def is_string(a):
        o = fo(a)
        if o is None:
            return False
        s = b[o:o + 64].split(b"\0")[0]
        return len(s) >= 2 and all(32 <= c < 127 or c in (9, 10, 13) for c in s)

    out = []
    for r in csv.reader(open(raw, encoding="utf-8", errors="replace")):
        if len(r) < 9:
            continue
        a = r[0].lower()
        L = led.get(a)
        if not L or L["class"] != "ENGINE":
            continue
        mc = r[7].split()
        mr = []
        reg_only = []
        for x in r[8].split():
            regp = x.startswith("r")
            x = x.lstrip("r")
            v = int(x, 16)
            if iat[0] <= v < iat[1]:
                continue
            if not writable(v) and not is_string(v):
                continue
            (reg_only if regp else mr).append(x)
        if r[2] != "OK":
            mc = mc or ["DECOMPILE-FAILED"]
        if mc or mr or reg_only:
            ev = (L["spec_ref"] + " " + L["notes"]).lower()
            decomp_only = ("decomp" in ev) and ("bytes" not in ev) and ("listing" not in ev) and ("disassembl" not in ev)
            kind = "DROPPED-CODE" if (mc or mr) else "HIDDEN-REG-ARG"
            out.append([a, L["ghidra_name"], kind, " ".join(mc), " ".join(mr), " ".join(reg_only), "yes" if decomp_only else ""])
    w = csv.writer(open(os.path.join(ROOT, "03_re", "ledger", "decomp_check.csv"), "w", newline="", encoding="utf-8"), lineterminator="\n")
    w.writerow(["address", "name", "kind", "missing_calls", "missing_refs", "hidden_reg_args", "evidence_decomp_only"])
    w.writerows(out)
    dropped = [x for x in out if x[2] == "DROPPED-CODE"]
    print("engine functions flagged:", len(out),
          "| DROPPED-CODE:", len(dropped), "(with missing calls:", sum(1 for x in dropped if x[3]), ")",
          "| HIDDEN-REG-ARG only:", len(out) - len(dropped),
          "| DROPPED-CODE with decomp-only evidence:", sum(1 for x in dropped if x[6]))


if __name__ == "__main__":
    main()
