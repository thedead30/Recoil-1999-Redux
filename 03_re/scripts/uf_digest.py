"""uf_digest.py - compact structural digest of a function's disassembly.

Purpose: cut the cost of the read-the-bytes step. A full `uf` listing for a
600-byte function is 150-400 lines; this emits ~15-30 lines carrying the facts
that actually decide a confirmation:

  * ret form (plain ret / ret N)  -> stack-arg count, cross-checked vs the corpus
  * call targets, named from the ledger
  * globals touched, split into read / written / address-taken
  * immediate constants, with float and known-value interpretation
  * FPU compare sites (the branch conditions that carry the semantics)
  * loop back-edges and branch count (shape, not detail)

Use it FIRST. Read the full listing only for the regions the digest or the
harvest packet flags (FTOL sites, arg-less calls, hidden stack args, unusual
opcodes). Bytes remain the evidence - this changes how they are located, not
whether they are read.

Usage:
  python 03_re/scripts/uf_digest.py 0x0049fbb0 [0x004a1420 ...]
     [--trace Recoil21] [--full ADDR] [--cache DIR]

--full ADDR also prints that one function's complete listing.
"""
import argparse, csv, io, os, re, struct, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")
EXE = os.path.join(ROOT, "00_original", "game_install", "Recoil.exe")
TTD = os.path.join(ROOT, "03_re", "scripts", "ttd_verify.py")

KNOWN = {0xFFFFD8F0: "-10000 = DSBVOLUME_MIN (DirectSound silence)",
         0x3F800000: "1.0f", 0x40000000: "2.0f", 0x437F0000: "255.0f",
         0x3F000000: "0.5f", 0x41200000: "10.0f", 0x42C80000: "100.0f"}


def load_names():
    names = {}
    try:
        for r in csv.DictReader(io.open(LEDGER, encoding="utf-8", newline="")):
            names[int(r["address"], 16)] = (r["ghidra_name"], r["status"])
    except Exception:
        pass
    return names


def load_sig_params():
    """declared parameter count per address, from the decomp_raw corpus."""
    out = {}
    d = os.path.join(ROOT, "03_re", "decomp_raw")
    if not os.path.isdir(d):
        return out
    for fn in os.listdir(d):
        m = re.match(r"0x([0-9a-fA-F]{8})_", fn)
        if not m:
            continue
        try:
            txt = io.open(os.path.join(d, fn), encoding="utf-8", errors="replace").read(4000)
        except Exception:
            continue
        sig = re.search(r"\n[A-Za-z_][\w \*]*\s+\w+\((.*?)\)\s*\n\{", txt, re.S)
        if sig:
            args = sig.group(1).strip()
            n = 0 if args in ("void", "") else len([a for a in args.split(",") if a.strip()])
            out[int(m.group(1), 16)] = n
    return out


def read_float(va):
    try:
        d = io.open(EXE, "rb").read()
    except Exception:
        return None
    pe = struct.unpack_from("<I", d, 0x3C)[0]
    ns = struct.unpack_from("<H", d, pe + 6)[0]
    oh = struct.unpack_from("<H", d, pe + 20)[0]
    base = struct.unpack_from("<I", d, pe + 52)[0]
    for i in range(ns):
        o = pe + 24 + oh + 40 * i
        va0 = base + struct.unpack_from("<I", d, o + 12)[0]
        rs = struct.unpack_from("<I", d, o + 16)[0]
        ro = struct.unpack_from("<I", d, o + 20)[0]
        if va0 <= va < va0 + rs:
            fo = ro + va - va0
            return struct.unpack_from("<f", d, fo)[0], struct.unpack_from("<I", d, fo)[0]
    return None


def disasm(addrs, trace, cache):
    cmds = []
    for a in addrs:
        cmds.append(".echo ===F_%08x===" % a)
        cmds.append("uf 0x%08x" % a)
    cmds.append(".echo ===END===")
    out = os.path.join(cache, "digest_%08x.txt" % addrs[0])
    if not os.path.exists(out):
        with open(out, "w") as fh:
            subprocess.run([sys.executable, TTD, "--trace", trace, "--timeout", "1500",
                            "--cmds", "; ".join(cmds)], stdout=fh, stderr=subprocess.STDOUT,
                           cwd=ROOT, timeout=1800)
    txt = io.open(out, errors="replace").read()
    res = {}
    for a in addrs:
        marker = "===F_%08x===" % a
        # A batched `uf` can truncate (cdb gives up, or uf walks into an adjacent
        # function and overruns). The marker for later addresses is then absent,
        # and splitting on [-1] silently returns the text of a DIFFERENT function -
        # which is how eight addresses once digested as an identical bogus
        # "1 instruction / ret 0Ch". Never guess: a missing marker is an error.
        # Tell: several addresses reporting the same ret form and instruction count.
        if txt.count(marker) < 2:      # command echo + the real output
            raise RuntimeError(
                "uf_digest: no output for 0x%08x in %s - the batch truncated.\n"
                "Delete that cache file and re-run these addresses in smaller "
                "batches (or singly). Do NOT trust any digest from this run." % (a, out))
        part = txt.split(marker)[-1].split("===")[0]
        lines = [m.group(1) + " " + m.group(2)
                 for m in (re.match(r"([0-9a-f]{8}) \S+\s+(.*)", l) for l in part.splitlines()) if m]
        # uf can also run past the requested function into its neighbours; drop
        # anything after the last ret so the digest describes ONE function.
        last_ret = max((i for i, l in enumerate(lines)
                        if re.match(r"^[0-9a-f]{8} ret", l)), default=None)
        if last_ret is not None:
            lines = lines[:last_ret + 1]
        res[a] = lines
    return res


def digest(a, lines, names, sigs):
    print("=" * 72)
    nm, st = names.get(a, ("?", "?"))
    print("0x%08x  %s  [%s]  %d instructions" % (a, nm, st, len(lines)))
    if not lines:
        print("  (no listing - function may not be in the trace image)")
        return
    rets = sorted({l.split(None, 1)[1].strip() for l in lines if re.match(r"^[0-9a-f]{8} ret", l)})
    stack_bytes = 0
    for r in rets:
        m = re.match(r"ret\s+([0-9A-Fa-f]+)h?", r)
        if m:
            stack_bytes = max(stack_bytes, int(m.group(1), 16))
    print("  ret forms: %s  -> %d stack arg(s)" % (", ".join(rets) or "none", stack_bytes // 4))
    if a in sigs:
        flag = "" if sigs[a] == stack_bytes // 4 else "   <-- MISMATCH vs corpus signature"
        print("  corpus signature declares %d param(s)%s" % (sigs[a], flag))
    calls = []
    for l in lines:
        m = re.search(r"call\s+.*?\(([0-9a-f]{6,8})\)", l)
        if m:
            t = int(m.group(1), 16)
            calls.append((l[:8], t, names.get(t, ("?", ""))[0]))
        elif re.search(r"call\s+dword ptr", l):
            g = re.search(r"\(([0-9a-f]{6,8})\)", l)
            if g:
                calls.append((l[:8], -1, "INDIRECT via global [0x%s]" % g.group(1)))
            else:
                op = re.search(r"call\s+dword ptr\s+(\[[^\]]+\])", l)
                calls.append((l[:8], -1, "INDIRECT vtable slot %s" % (op.group(1) if op else "?")))
    if calls:
        print("  calls (%d):" % len(calls))
        for site, t, n in calls:
            print("     %s -> %s %s" % (site, ("0x%08x" % t) if t > 0 else "        ", n))
    reads, writes, taken = set(), set(), set()
    for l in lines:
        for m in re.finditer(r"\(([0-9a-f]{6,8})\)", l):
            g = int(m.group(1), 16)
            # only .rdata/.data addresses are globals; anything below 0x004d0000 is
            # code (a branch or call target) and would otherwise pollute the list
            if g < 0x004d0000 or g > 0x700000:
                continue
            if re.search(r"(mov|fstp|fst)\s+(dword|word|byte|qword) ptr .*\(%s\)\s*\]?,"
                         % m.group(1), l):
                writes.add(g)
            elif "offset" in l:
                taken.add(g)
            else:
                reads.add(g)
    for label, s in (("written", writes), ("read", reads - writes), ("addr-taken", taken)):
        if s:
            print("  globals %s: %s" % (label, " ".join("0x%08x" % g for g in sorted(s))))
    consts = {}
    for l in lines:
        for m in re.finditer(r",\s*([0-9A-F][0-9A-Fa-f]{0,7})h\b", l):
            v = int(m.group(1), 16)
            if v > 8:
                consts[v] = consts.get(v, 0) + 1
    if consts:
        print("  immediates:")
        for v in sorted(consts):
            note = KNOWN.get(v, "")
            if not note and 0x30000000 < v < 0x50000000:
                note = "as float %g" % struct.unpack("<f", struct.pack("<I", v))[0]
            print("     0x%08X (x%d) %s" % (v, consts[v], note))
    fcmp = [l for l in lines if re.search(r"\bf(comp?|ucomp?|test)", l)]
    if fcmp:
        print("  FPU compares (%d): %s" % (len(fcmp), ", ".join(l[:8] for l in fcmp[:12])))
    back = sum(1 for l in lines
               if re.search(r"\bj\w+\s+.*?\(([0-9a-f]{8})\)", l)
               and int(re.search(r"\(([0-9a-f]{8})\)", l).group(1), 16) < int(l[:8], 16))
    branches = sum(1 for l in lines if re.match(r"^[0-9a-f]{8} j", l))
    print("  shape: %d branches, %d loop back-edge(s)" % (branches, back))
    flags = []
    if any("ftol" in l for l in lines):
        flags.append("FTOL (hidden FPU arithmetic - read those sites)")
    if any("1FC00000h" in l for l in lines):
        flags.append("fast-sqrt approximation")
    if any(re.search(r"call\s+dword ptr", l) for l in lines):
        flags.append("indirect call (vtable/hook - resolve the target)")
    if flags:
        print("  FLAGS: " + "; ".join(flags))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("addrs", nargs="+")
    ap.add_argument("--trace", default="Recoil21")
    ap.add_argument("--full", default=None)
    ap.add_argument("--cache", default=os.path.join(ROOT, "03_re", "scratch_digest"))
    a = ap.parse_args()
    os.makedirs(a.cache, exist_ok=True)
    addrs = [int(x, 16) for x in a.addrs]
    names, sigs = load_names(), load_sig_params()
    listings = disasm(addrs, a.trace, a.cache)
    for ad in addrs:
        digest(ad, listings.get(ad, []), names, sigs)
    if a.full:
        f = int(a.full, 16)
        print("\n=== FULL LISTING 0x%08x ===" % f)
        for l in listings.get(f, []):
            print(l)


if __name__ == "__main__":
    main()
