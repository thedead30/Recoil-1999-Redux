#!/usr/bin/env python3
"""
stack_arg_check.py - catch stack parameters the decompiler hid.

Ghidra sometimes renders a function as `f(void)` when it really pops stack arguments
with `ret N`. FUN_00474010 was confirmed on that incomplete reading (2026-09-15) and
had to be demoted: its decompilation showed no parameters, the bytes end `ret 4`.

For each function this compares the largest `ret N` in its disassembly (N / 4 stack
params) with the parameter count of its decompiled signature in 03_re/decomp_raw/.
Popped > declared means hidden stack arguments. It cannot see hidden REGISTER
arguments (ECX/EDX) - harvest.py's argument-less-call flag covers those.

  python 03_re/scripts/stack_arg_check.py                 # every CONFIRMED function
  python 03_re/scripts/stack_arg_check.py 0x00474010 ...  # specific addresses

Exit 1 if any function is flagged or could not be checked.
"""
import csv, glob, io, os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")
DECOMP = os.path.join(ROOT, "03_re", "decomp_raw")
VERIFY = os.path.join(ROOT, "03_re", "scripts", "ttd_verify.py")


def declared_params(addr):
    fs = glob.glob(os.path.join(DECOMP, "0x%s_*.c" % addr))
    if not fs:
        return None
    text = io.open(fs[0], encoding="utf-8", errors="replace").read()
    m = re.search(r"\n[^\n(]*\b\w+\s*\(([^)]*)\)\s*\n\{", text)
    if not m:
        return None
    inner = m.group(1).strip()
    return 0 if inner in ("", "void") else inner.count(",") + 1


def ret_sizes(addrs, chunk=40):
    """Every `ret` / `ret N` found by `uf` for each address. Chunked so one long cdb
    output cannot truncate the tail of the list."""
    found = {}
    for i in range(0, len(addrs), chunk):
        part = addrs[i:i + chunk]
        cmds = "".join(".echo ===F_%s===; uf 0x%s;" % (a, a) for a in part)
        p = subprocess.run([sys.executable, VERIFY, "--trace", "Recoil18", "--timeout", "1500",
                            "--cmds", cmds], capture_output=True, text=True, timeout=1600)
        cur = None
        for ln in (p.stdout + p.stderr).splitlines():
            m = re.match(r"^===F_([0-9a-f]{8})===", ln)
            if m:
                cur = m.group(1)
                found[cur] = set()
                continue
            m = re.match(r"^[0-9a-f]{8} [0-9a-f]+\s+ret\s*([0-9a-fA-F]+h?)?\s*$", ln.strip())
            if cur and m:
                n = m.group(1)
                found[cur].add(int(n.rstrip("h"), 16) if n else 0)
    return found


def main():
    args = [a.lower().replace("0x", "") for a in sys.argv[1:]]
    if not args:
        args = sorted(r["address"][2:].lower() for r in csv.DictReader(io.open(LEDGER, encoding="utf-8"))
                      if r["status"] == "CONFIRMED")
    rets = ret_sizes(args)
    flagged, unchecked, ok = [], [], 0
    for a in args:
        pc, rs = declared_params(a), rets.get(a)
        if pc is None or not rs:
            unchecked.append(a)
        elif max(rs) // 4 > pc:
            flagged.append((a, pc, sorted(rs)))
        else:
            ok += 1
    print("checked %d: consistent %d, HIDDEN STACK ARGS %d, unchecked %d" % (len(args), ok, len(flagged), len(unchecked)))
    for a, pc, rs in flagged:
        print("  HIDDEN  0x%s  declared %d params, ret pops %s bytes" % (a, pc, rs))
    for a in unchecked:
        print("  UNCHECKED  0x%s" % a)
    return 1 if (flagged or unchecked) else 0


if __name__ == "__main__":
    sys.exit(main())
