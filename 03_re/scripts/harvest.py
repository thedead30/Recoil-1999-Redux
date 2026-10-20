#!/usr/bin/env python3
"""
harvest.py - produce a structured EVIDENCE PACKET for a function, with no interpretation.

Purpose
-------
This exists so the expensive part of a pass (cdb round-trips, decompilation reading,
trap-hunting) can be done mechanically and cheaply, leaving only judgement to a model.

It deliberately produces FACTS AND FLAGS, never conclusions:
  * it says "this call has no arguments in the decompiler but loads ECX first" -
    it does NOT say what the argument is;
  * it says "fast-sqrt magic constant at 0x...", not "this computes a distance";
  * it says "no hit in Recoil18 and Recoil21", never "this function is dead".

Everything it emits is checkable. Nothing it emits is a name.

Why the flags it raises are the ones it raises
----------------------------------------------
Each corresponds to a documented, repeated failure in 03_re/LOOP.md:

  ftol()                 - Ghidra swallows the FPU arithmetic. Four separate math
                           approximations were missed this way.
  argument-less call     - an unset prototype hiding register args. Believing one gave a
                           model that transformed the origin instead of a real position.
  approximation magic    - 0x1fc00000 fast sqrt, 0x3f800000-after-ftol expApprox.
  `= 0.0` then a call    - dead-store initialisers that hide a callee's writes.

Usage
-----
  python 03_re/scripts/harvest.py 0x00452ec0
  python 03_re/scripts/harvest.py 0x00452ec0 --traces Recoil18 Recoil21 Recoil22
  python 03_re/scripts/harvest.py --batch 0x00452ec0 0x00449480 0x004732f0
  python 03_re/scripts/harvest.py 0x00452ec0 --no-exec      (skip trace probing - fast)

Output goes to 03_re/harvest/<address>.md unless --stdout is given.
"""
import argparse
import bisect
import csv
import os
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EXE = os.path.join(ROOT, "00_original", "game_install", "Recoil.exe")
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")
ATTRIB = os.path.join(ROOT, "03_re", "ledger", "source_attribution.csv")
DECOMP_RAW = os.path.join(ROOT, "03_re", "decomp_raw")
OUTDIR = os.path.join(ROOT, "03_re", "harvest")
VERIFY = os.path.join(ROOT, "03_re", "scripts", "ttd_verify.py")

# Standing rule from LOOP.md: a single-trace "no hit" is provisional. Default to two,
# one of which is the largest and most varied.
DEFAULT_TRACES = ["Recoil18", "Recoil21"]

FTOL = 0x004c60a6           # the ftol thunk every FPU->int conversion goes through
MAGIC = {
    0x1fc00000: "fast sqrt  (i>>1)+0x1fc00000",
    0x5f3759df: "fast inverse sqrt",
    0x5f375a86: "fast inverse sqrt (refined)",
}


# ----------------------------------------------------------------- binary plumbing
class Image(object):
    def __init__(self, path):
        self.data = open(path, "rb").read()
        d = self.data
        pe = struct.unpack_from("<I", d, 0x3c)[0]
        nsec = struct.unpack_from("<H", d, pe + 6)[0]
        opt = struct.unpack_from("<H", d, pe + 20)[0]
        self.base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
        self.secs = []
        for i in range(nsec):
            o = pe + 24 + opt + i * 40
            vsz, va, rsz, ptr = struct.unpack_from("<IIII", d, o + 8)
            self.secs.append((va, vsz, ptr, rsz))

    def off2va(self, off):
        for va, vsz, ptr, rsz in self.secs:
            if ptr <= off < ptr + rsz:
                return self.base + va + (off - ptr)
        return None

    def va2off(self, va):
        for v, vsz, ptr, rsz in self.secs:
            if self.base + v <= va < self.base + v + rsz:
                return ptr + (va - (self.base + v))
        return None

    def read_f32(self, va):
        o = self.va2off(va)
        if o is None:
            return None
        return struct.unpack_from("<f", self.data, o)[0]


def load_ledger():
    rows = []
    if not os.path.isfile(LEDGER):
        return rows
    for r in csv.DictReader(open(LEDGER, encoding="utf-8")):
        a = r.get("address", "")
        if a.startswith("0x"):
            try:
                rows.append((int(a, 16), r))
            except ValueError:
                pass
    rows.sort(key=lambda t: t[0])
    return rows


def func_bounds(ledger, addr):
    """Return (start, end) using the next ledger row as the end. Approximate by design."""
    addrs = [a for a, _ in ledger]
    i = bisect.bisect_right(addrs, addr) - 1
    if i < 0:
        return addr, addr + 0x200
    start = addrs[i]
    end = addrs[i + 1] if i + 1 < len(addrs) else start + 0x400
    return start, end


def source_file_for(addr):
    if not os.path.isfile(ATTRIB):
        return None
    best = None
    for r in csv.DictReader(open(ATTRIB, encoding="utf-8")):
        try:
            a = int(r["func_addr"], 16)
        except (KeyError, ValueError):
            continue
        if a == addr:
            return "%s/%s (%s)" % (r["module"], r["source_file"], r["evidence"])
    return best


# ----------------------------------------------------------------- cdb helpers
def cdb(trace, cmds, timeout=900):
    try:
        p = subprocess.run([sys.executable, VERIFY, "--trace", trace, "--cmds", cmds],
                           capture_output=True, text=True, timeout=timeout)
        return p.stdout + p.stderr
    except subprocess.TimeoutExpired:
        return "<<timeout>>"
    except Exception as e:                                   # noqa: BLE001
        return "<<error: %s>>" % e


def probe_exec(addr, traces):
    """How many times does the function execute? Returns [(trace, count)].

    Uses TTD execute-access counting rather than `bp; g; r eip`:

        dx @$cursession.TTD.Memory(addr, addr+1, "e").Count()

    The execute-access count at the entry byte IS the call count. This is one
    command per trace with no `g`, so it is far faster than a breakpoint run, it
    yields a count rather than a yes/no, and - the reason it replaced breakpoint
    probing - it CANNOT fall into the end-of-trace trap. With `bp; g`, a function
    that never runs leaves `g` parked at the end of the trace with stale registers,
    which was misread as a result three separate times before the `eip` check
    became mandatory. There is no `eip` to misread here.

    A count of `None` means the query itself failed (timeout or cdb error) and must
    NOT be reported as zero.
    """
    out = []
    for t in traces:
        txt = cdb(t, 'dx @$cursession.TTD.Memory(0x%08x, 0x%08x, "e").Count()'
                  % (addr, addr + 1))
        m = re.search(r"Count\(\)\s*:\s*(0x[0-9a-fA-F]+|\d+)", txt)
        out.append((t, int(m.group(1), 0) if m else None))
    return out


def disasm(trace, addr, count):
    txt = cdb(trace, "u 0x%08x L%d" % (addr, count))
    lines = []
    started = False
    for ln in txt.splitlines():
        if re.match(r"^[0-9a-f]{8} ", ln):
            started = True
        if started and re.match(r"^[0-9a-f]{8} ", ln):
            lines.append(ln.rstrip())
    return lines


# ----------------------------------------------------------------- trap detection
def find_traps(img, start, end):
    """Locate the documented decompiler traps inside [start, end). Facts only."""
    traps = []
    lo, hi = img.va2off(start), img.va2off(end)
    if lo is None or hi is None:
        return traps
    blob = img.data[lo:hi]

    # 1. approximation magic constants appearing as immediates
    for mag, label in MAGIC.items():
        for m in re.finditer(re.escape(struct.pack("<I", mag)), blob):
            traps.append(("APPROXIMATION", img.off2va(lo + m.start()), label))

    # 2. expApprox signature: add eax, 0x3f800000
    for m in re.finditer(re.escape(bytes([0x05, 0x00, 0x00, 0x80, 0x3F])), blob):
        traps.append(("APPROXIMATION", img.off2va(lo + m.start()),
                      "expApprox  add eax,0x3f800000 (Schraudolph exp)"))

    # 3. calls to ftol - Ghidra shows these as ftol() and hides the FPU work
    for m in re.finditer(rb"\xE8", blob):
        off = lo + m.start()
        va = img.off2va(off)
        if va is None:
            continue
        try:
            rel = struct.unpack_from("<i", img.data, off + 1)[0]
        except struct.error:
            continue
        if va + 5 + rel == FTOL:
            traps.append(("FTOL", va, "call ftol - FPU arithmetic hidden in the decompiler"))
    return sorted(traps, key=lambda t: t[1])


def decomp_path(addr):
    tag = "%08x" % addr
    if not os.path.isdir(DECOMP_RAW):
        return None
    for fn in os.listdir(DECOMP_RAW):
        if tag in fn.lower():
            return os.path.join(DECOMP_RAW, fn)
    return None


def argless_calls(src):
    """Decompiler calls written with no arguments - the register-arg trap."""
    return sorted(set(re.findall(r"\b(FUN_[0-9a-f]{8}|[A-Za-z_][A-Za-z0-9_]*)\(\)", src)))


def dead_zero_inits(src):
    """`x = 0.0;` shortly before a call - initialisers that may hide a callee's writes."""
    hits = []
    lines = src.splitlines()
    for i, ln in enumerate(lines):
        if re.search(r"=\s*0\.0;", ln):
            for j in range(i + 1, min(i + 8, len(lines))):
                if re.search(r"\w+\(\);", lines[j]):
                    hits.append((ln.strip(), lines[j].strip()))
                    break
    return hits


# ----------------------------------------------------------------- packet
def harvest(addr, traces, do_exec=True, disasm_count=0):
    img = Image(EXE)
    ledger = load_ledger()
    start, end = func_bounds(ledger, addr)
    row = None
    for a, r in ledger:
        if a == addr:
            row = r
            break

    L = []
    W = L.append
    W("# EVIDENCE PACKET  0x%08x" % addr)
    W("")
    W("*Produced by `03_re/scripts/harvest.py`. **Facts and flags only - no interpretation,")
    W("no names.** Every line is checkable. Nothing here is a conclusion.*")
    W("")

    # --- identity
    W("## Identity")
    W("| field | value |")
    W("|---|---|")
    W("| address | `0x%08x` |" % addr)
    if row:
        W("| ghidra name | `%s` |" % row.get("ghidra_name", "?"))
        W("| ledger status | **%s** |" % row.get("status", "?"))
        W("| verification | %s |" % (row.get("verification") or "-"))
        W("| callers / callees | %s / %s |" % (row.get("num_callers", "?"), row.get("num_callees", "?")))
        W("| spec_ref | `%s` |" % (row.get("spec_ref") or "-"))
        W("| notes | %s |" % (row.get("notes") or "-"))
    else:
        W("| ledger | **NOT IN LEDGER** |")
    sf = source_file_for(addr)
    W("| source file | %s |" % (sf or "not attributed"))
    W("| approx extent | `0x%08x .. 0x%08x` (%d bytes, next-row estimate) |" % (start, end, end - start))
    W("")

    # --- execution
    W("## Execution")
    if not do_exec:
        W("*(skipped - `--no-exec`)*")
    else:
        W("| trace | calls |")
        W("|---|---|")
        results = probe_exec(addr, traces)
        for t, n in results:
            W("| `%s` | %s |" % (t, "**query failed**" if n is None else
                                 ("**%d**" % n if n else "0")))
        anyhit = any(n for _, n, in results if n is not None)
        failed = [t for t, n in results if n is None]
        W("")
        if failed:
            W("**Query failed for: %s.** Not a zero - re-run before concluding anything."
              % ", ".join("`%s`" % t for t in failed))
            W("")
        if anyhit:
            W("A **non-zero count in any trace is conclusive** - the function executes.")
            W("Counts are execute-accesses at the entry byte, i.e. calls. **Equal counts")
            W("across two functions in the same traces are evidence of a 1:1 call relationship.**")
        else:
            W("**No hit in %d trace(s). This is PROVISIONAL, not a negative.**" % len(results))
            W("Per `03_re/LOOP.md`, absence needs `Recoil18` plus at least one other before it")
            W("may be recorded, and even then a function running only at mission load (before")
            W("the recorder attaches) will never appear. Do not write \"dead\" from this alone.")
    W("")

    # --- traps
    W("## Flags raised")
    traps = find_traps(img, start, end)
    if not traps:
        W("None of the documented traps detected in this extent.")
    else:
        W("| kind | address | what |")
        W("|---|---|---|")
        for kind, va, what in traps:
            W("| %s | `0x%08x` | %s |" % (kind, va, what))
        W("")
        W("**Each of these means the decompiler output is incomplete at that point.**")
        W("`FTOL` hides FPU arithmetic; `APPROXIMATION` means a cheap substitute for a standard")
        W("math function is in use and the textbook version is the wrong one. See")
        W("`04_spec/systems/math_approximations.md`. **Read the disassembly at these addresses.**")
    W("")

    # --- decompilation-derived flags
    dp = decomp_path(addr)
    W("## Decompiler-output flags")
    if not dp:
        W("*No decompilation found in `03_re/decomp_raw`.*")
    else:
        src = open(dp, encoding="utf-8", errors="replace").read()
        al = argless_calls(src)
        al = [a for a in al if a not in ("ftol",)]
        if al:
            W("**Argument-less calls** - an unset prototype hiding register arguments:")
            W("")
            for a in al:
                W("- `%s()`" % a)
            W("")
            W("Confirmed failure mode: believing one of these produced a model that transformed")
            W("the origin instead of a real position. **Disassemble the call site and look for")
            W("`mov ecx,...` / `mov edx,...` immediately before it.**")
            W("")
        dz = dead_zero_inits(src)
        if dz:
            W("**`= 0.0` initialisers shortly before an argument-less call** - these may be dead")
            W("stores masking writes the callee makes through a register-passed pointer:")
            W("")
            for init, call in dz[:8]:
                W("- `%s`  ->  `%s`" % (init, call))
            W("")
        if not al and not dz:
            W("No argument-less calls or suspicious zero-initialisers detected.")
    W("")

    # --- raw material
    if dp:
        W("## Decompilation (raw)")
        W("")
        W("```c")
        W(open(dp, encoding="utf-8", errors="replace").read().rstrip())
        W("```")
        W("")

    if disasm_count and do_exec and traces:
        W("## Disassembly (first %d instructions)" % disasm_count)
        W("")
        W("```")
        for ln in disasm(traces[0], addr, disasm_count):
            W(ln)
        W("```")
        W("")

    # --- what the harvester cannot do
    W("## What this packet does NOT establish")
    W("- **What any field or flag means.** No offset here has been named.")
    W("- **Whether a `no hit` means dead code.** See the execution section.")
    W("- **What the flagged approximations compute** - only that they are present.")
    W("- **What argument-less calls receive** - only that the decompiler is hiding it.")
    W("")
    W("Naming a field, advancing a ledger row, or writing a spec entry from this packet")
    W("requires judgement that is deliberately not applied here.")
    return "\n".join(L)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("addresses", nargs="*", help="function addresses, e.g. 0x00452ec0")
    ap.add_argument("--batch", nargs="+", default=None, help="same as positional, explicit")
    ap.add_argument("--traces", nargs="+", default=DEFAULT_TRACES)
    ap.add_argument("--no-exec", action="store_true", help="skip trace probing (fast)")
    ap.add_argument("--disasm", type=int, default=0, help="also emit N instructions")
    ap.add_argument("--stdout", action="store_true")
    a = ap.parse_args()

    addrs = list(a.addresses) + list(a.batch or [])
    if not addrs:
        ap.error("give at least one address")

    if not os.path.isfile(EXE):
        sys.exit("binary not found: %s" % EXE)
    if not a.no_exec and not os.path.isfile(VERIFY):
        sys.exit("ttd_verify.py not found: %s" % VERIFY)

    if not a.stdout:
        os.makedirs(OUTDIR, exist_ok=True)

    for s in addrs:
        addr = int(s, 16)
        packet = harvest(addr, a.traces, do_exec=not a.no_exec, disasm_count=a.disasm)
        if a.stdout:
            print(packet)
        else:
            p = os.path.join(OUTDIR, "%08x.md" % addr)
            open(p, "w", encoding="utf-8").write(packet + "\n")
            print("wrote %s" % os.path.relpath(p, ROOT))


if __name__ == "__main__":
    main()
