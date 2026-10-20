#!/usr/bin/env python3
"""
re_status.py - the one command that answers "how is this project actually going".

Run at the start of every session and before every report. It prints numbers, not
prose, because prose is what drifted last time. If any DRIFT line is non-zero the
project has drifted, regardless of what anyone claims in chat.

Exit code 0 = clean, 1 = drifted, 2 = no ledger. Wire it into the build.
"""
import collections
import csv, os, re, sys, collections

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEDGER = os.path.join(ROOT, "03_re", "ledger", "functions.csv")
SUBSYS = os.path.join(ROOT, "03_re", "ledger", "subsystems.csv")
REGISTRY = os.path.join(ROOT, "03_re", "ledger", "subsystem_registry.csv")
CALLGRAPH = os.path.join(ROOT, "03_re", "ledger", "_callgraph.tsv")
SRC = os.path.join(ROOT, "05_remake", "src")

# PLATFORM is the sixth provenance tag and is NOT a loophole: it means the value
# comes from the host API (a D3D enum, a WAV sample rate, a Win32 flag) and never
# from Recoil. A game-behaviour value tagged PLATFORM is a lie, and reviewable as one.
PROV = ("CONFIRMED-BINARY", "CONFIRMED-DATA", "MEASURED-ASSET",
        "INFERRED", "INVENTED", "PLATFORM")
VERIF = ("VERIFIED-PLAYPATH", "VERIFIED-VISUAL", "VERIFIED-ORACLE",
         "VERIFIED-UNIT", "VERIFIED-IDENTICAL", "IMPLEMENTED-UNVERIFIED")
# VERIFIED-IDENTICAL (2026-10-07, user decision): tools/prove_identity.py proved the asm port instruction-identical to
# the original incl. call targets and data addresses. Only valid while the current proof report lists the function
# as IDENTICAL and is newer than the build it checked: enforced by the tripwire "IDENTICAL tags without a current proof".

# A numeric literal being ASSIGNED or used as an aggregate initialiser.
# Comparisons (>=, ==, <) are control flow, not constants-of-record.
CONST = re.compile(r"(?<![=!<>+\-*/%&|^])=\s*-?\d+\.?\d*[fu]?\s*[;,)}]"
                   r"|(?<![=!<>+\-*/%&|^])=\s*-?0[xX][0-9a-fA-F]+[uUlL]*\s*[;,)}]"   # hex (P0.5: was missed)
                   r"|\{\s*-?\d+\.\d+f?\s*[,}]")
# Structural values carrying no game semantics.
EXEMPT = re.compile(r"=\s*-?(0|1|2|0\.0f?|1\.0f?|-1|0x0|0xff|255)\s*[;,)}]")
# Directories whose numbers are scaffolding, not shipped behaviour.
SKIP_DIRS = {"tests", "test", "demo"}
SRC_EXT = (".cpp", ".h", ".hpp", ".cc")


def bar(pct, w=32):
    n = int(round(pct * w / 100.0))
    return "#" * n + "." * (w - n)


def ledger_stats():
    if not os.path.exists(LEDGER):
        return None
    rows = list(csv.DictReader(open(LEDGER, encoding="utf-8")))
    eng = [r for r in rows if r["class"] == "ENGINE"]
    m1 = [r for r in eng if r["map1_reachable"] == "YES"]
    return rows, eng, m1


def subsystem_gates(rows):
    """Per-subsystem Stage 2 gates (adopted 2026-09-15, replacing one all-or-nothing
    gate over every engine function).

    A subsystem's gate opens only when ALL of these hold:
      1. its membership is CLOSED in subsystem_registry.csv - a PROVISIONAL boundary
         never opens, so a boundary cannot be left vague while code is written;
      2. every member is CONFIRMED in the ledger;
      3. every direct callee of every member is covered: itself a member, CONFIRMED,
         not an engine function (library/thunk/import), or a member of another
         subsystem whose gate is already open.
    Rule 3 is what stops a gate being opened by drawing the boundary around only the
    functions that happen to be confirmed: anything a member calls must be accounted
    for. It uses direct call edges only; vtable dispatch is not covered, and the
    report says so.

    Returns [(name, membership, n_members, n_confirmed, uncovered_callees, open)].
    """
    if not (os.path.exists(REGISTRY) and os.path.exists(SUBSYS)):
        return []
    by_addr = {r["address"].lower().replace("0x", ""): r for r in rows}
    reg = list(csv.DictReader(open(REGISTRY, encoding="utf-8")))
    members = collections.defaultdict(set)
    for m in csv.DictReader(open(SUBSYS, encoding="utf-8")):
        members[m["subsystem"]].add(m["address"].lower().replace("0x", ""))
    callees = collections.defaultdict(set)
    if os.path.exists(CALLGRAPH):
        for line in open(CALLGRAPH, encoding="utf-8").read().splitlines()[1:]:
            a, _, b = line.partition("\t")
            callees[a.strip().lower()].add(b.strip().lower())

    def covered(c):
        r = by_addr.get(c)
        return r is None or r["class"] != "ENGINE" or r["status"] == "CONFIRMED"

    # Fixed point: one subsystem may rely on another only once that one is open.
    # Opening can only shrink other subsystems' gaps, so this terminates.
    is_open = {s["subsystem"]: False for s in reg}
    result = {}
    changed = True
    while changed:
        changed = False
        for s in reg:
            name = s["subsystem"]
            mem = members.get(name, set())
            conf = sum(1 for a in mem if by_addr.get(a, {}).get("status") == "CONFIRMED")
            elsewhere = set().union(*[members[n] for n, ok in is_open.items()
                                      if ok and n != name])
            gaps = sorted({c for a in mem for c in callees.get(a, ())
                           if c not in mem and c not in elsewhere and not covered(c)})
            ok = bool(mem) and s["membership"] == "CLOSED" and conf == len(mem) and not gaps
            if ok != is_open[name]:
                is_open[name] = ok
                changed = True
            result[name] = (s["membership"], len(mem), conf, gaps, ok)
    return [(n,) + result[n] for n in sorted(result)]


def scan_source(src=None):
    src = src or SRC
    untagged, files, total_lines = [], 0, 0
    prov, verif = collections.Counter(), collections.Counter()
    if not os.path.isdir(src):
        return untagged, files, total_lines, prov, verif
    for dp, _, fns in os.walk(src):
        parts = {q.lower() for q in dp.replace("\\", "/").split("/")}
        in_test = bool(parts & SKIP_DIRS)
        for fn in fns:
            if not fn.endswith(SRC_EXT):
                continue
            files += 1
            p = os.path.join(dp, fn)
            lines = open(p, encoding="utf-8", errors="replace").read().splitlines()
            total_lines += len(lines)
            for i, line in enumerate(lines, 1):
                for t in PROV:
                    prov[t] += line.count(t)
                for t in VERIF:
                    verif[t] += line.count(t)
                if in_test:
                    continue
                s = line.split("//")[0]
                if not CONST.search(s) or EXEMPT.search(s):
                    continue
                ctx = "\n".join(lines[max(0, i - 4):i])
                if not any(t in ctx for t in PROV):
                    rel = os.path.relpath(p, os.path.dirname(src)) if src != SRC \
                          else os.path.relpath(p, ROOT)
                    untagged.append(f"{rel}:{i}: {line.strip()[:88]}")
    return untagged, files, total_lines, prov, verif


def src_outside_open_gates(gates):
    """Every remake source file must declare `SUBSYSTEM: <name>` in its first 30
    lines, and that subsystem's gate must be open. Undeclared counts as a violation:
    a file that names no subsystem is exactly how unconfirmed code would slip in."""
    open_names = {g[0] for g in gates if g[-1]}
    bad = []
    if not os.path.isdir(SRC):
        return bad
    for dp, _, fns in os.walk(SRC):
        for fn in fns:
            if not fn.endswith(SRC_EXT):
                continue
            p = os.path.join(dp, fn)
            head = "".join(open(p, encoding="utf-8", errors="replace").readlines()[:30])
            m = re.search(r"SUBSYSTEM:\s*([\w\-]+)", head)
            in_platform = "platform" in {q.lower() for q in os.path.relpath(dp, SRC).replace("\\", "/").split("/")}
            if m and m.group(1) == "platform" and in_platform:
                continue    # layer D (STAGE2.md section 3): no Recoil function, no gate; values are PLATFORM
            if not m or m.group(1) not in open_names:
                bad.append("%s  (%s)" % (os.path.relpath(p, ROOT),
                                         "no SUBSYSTEM header" if not m
                                         else "gate for '%s' is closed" % m.group(1)))
    return bad


def main():
    print("=" * 72)
    print("  RECOIL - RE STATUS".ljust(52) + "re_status.py")
    print("=" * 72)

    ls = ledger_stats()
    if ls is None:
        print("\n  !! NO LEDGER. Run build_ledger.py. Nothing below is meaningful.\n")
        return 2
    rows, eng, m1 = ls
    c = collections.Counter(r["status"] for r in eng)
    conf = c.get("CONFIRMED", 0)
    pct = 100.0 * conf / len(eng) if eng else 0

    print("\nSTAGE 1 - reverse engineering  (%d functions, %d engine after "
          "excluding library/thunks)" % (len(rows), len(eng)))
    print("  CONFIRMED       %5d / %d   %5.1f%%  [%s]" % (conf, len(eng), pct, bar(pct)))
    for k in ("DECOMPILED", "PRIOR-EVIDENCE", "NAMED", "UNTOUCHED", "WAIVED"):
        if c.get(k):
            print("  %-15s %5d" % (k, c[k]))


    gates = subsystem_gates(rows)
    orphans = []
    print("\n  STAGE 2 GATES  (per subsystem - 03_re/ledger/subsystem_registry.csv)")
    if not gates:
        print("  no subsystems defined - no Stage 2 code anywhere")
    for name, memb, n, nconf, gaps, ok in gates:
        why = []
        if memb != "CLOSED":
            why.append("membership %s" % memb)
        if nconf < n:
            why.append("%d unconfirmed" % (n - nconf))
        if gaps:
            why.append("%d uncovered callee(s)" % len(gaps))
        print("  %-16s %3d / %-3d confirmed   %-6s  %s" % (
            name, nconf, n, "OPEN" if ok else "CLOSED", "; ".join(why)))
    if gates:
        print("  (callee coverage uses direct call edges only; vtable dispatch not covered)")
        registered = {g[0] for g in gates}
        assigned = set()
        for m in csv.DictReader(open(SUBSYS, encoding="utf-8")):
            if m["subsystem"] in registered:
                assigned.add(m["address"].lower().replace("0x", ""))
        orphans = [r["address"] for r in eng
                   if r["address"].lower().replace("0x", "") not in assigned]
        print("  engine funcs in no subsystem       %d%s" % (
            len(orphans), ("  e.g. " + " ".join(orphans[:5])) if orphans else ""))
    # Known gaps (03_re/ledger/known_gaps.csv) - the open-items list kept since 2026-09-25
    kg = os.path.join(os.path.dirname(LEDGER), "known_gaps.csv")
    if os.path.exists(kg):
        og = [r for r in csv.DictReader(open(kg, encoding="utf-8")) if r["status"] == "OPEN"]
        kinds = collections.Counter(r["kind"] for r in og)
        print("  known gaps open                    %d  (%s)" % (
            len(og), ", ".join("%s=%d" % kv for kv in sorted(kinds.items()))))
    # G1: vtable slot targets with no ledger row (03_re/ledger/vtable_orphans.csv)
    vo = os.path.join(os.path.dirname(LEDGER), "vtable_orphans.csv")
    if os.path.exists(vo):
        vrows = [r for r in csv.DictReader(open(vo, encoding="utf-8")) if r["status"] == "NO-FUNCTION"]
        print("  vtable targets not in ledger       %d" % len(vrows))
    # F1: every engine function carries a verification plan (assign_verify_plan.py)
    plans = collections.Counter(r.get("verify_plan", "") for r in eng)
    missing = [r["address"] for r in eng if not r.get("verify_plan")]
    print("  verify_plan missing                %d%s" % (
        len(missing), ("  e.g. " + " ".join(missing[:5])) if missing else ""))
    print("  verify_plan: " + "  ".join("%s=%d" % (k, v) for k, v in sorted(plans.items()) if k))

    untagged, files, loc, prov, verif = scan_source()
    # Stage 2 progress (P0.1): translated = remake_file set; verified = remake_verification is a
    # VERIFIED-* tag. Phase per subsystem comes from subsystem_registry.csv column stage2_phase.
    sub_of = {}
    for m in csv.DictReader(open(SUBSYS, encoding="utf-8")):
        sub_of.setdefault(m["address"].lower(), m["subsystem"])
    phase_of = {r["subsystem"]: r.get("stage2_phase", "") for r in csv.DictReader(open(REGISTRY, encoding="utf-8"))}
    s2 = collections.defaultdict(lambda: [0, 0, 0])
    for r in eng:
        ph = phase_of.get(sub_of.get(r["address"].lower(), ""), "")
        if not ph or ph == "OUT":
            continue
        s2[ph][0] += 1
        s2[ph][1] += bool(r.get("remake_file"))
        s2[ph][2] += r.get("remake_verification", "").startswith("VERIFIED-")
    print("\nSTAGE 2 - remake source  (%d files, %d lines)" % (files, loc))
    if sum(prov.values()):
        print("  provenance : " + "  ".join("%s=%d" % (k, v)
                                            for k, v in prov.items() if v))
    if sum(verif.values()):
        print("  verified   : " + "  ".join("%s=%d" % (k, v)
                                            for k, v in verif.items() if v))

    if s2:
        print("  phase   in scope  translated  verified")
        for ph in sorted(s2):
            n, tr, ve = s2[ph]
            print("  %-6s  %8d  %10d  %8d" % (ph, n, tr, ve))
    impl_unconf = [r for r in rows if r["remake_file"] and r["status"] != "CONFIRMED"]
    src_closed = src_outside_open_gates(gates)
    print("\nDRIFT TRIPWIRES  (any non-zero = drifted, whatever the chat says)")
    print("  untagged constants in src/          %d" % len(untagged))
    print("  code written for unconfirmed funcs  %d" % len(impl_unconf))
    print("  code outside an open subsystem gate %d" % len(src_closed))
    print("  INVENTED values in src/             %d" % prov.get("INVENTED", 0))
    unconf = [r["address"] for r in eng if r["status"] != "CONFIRMED"]
    print("  engine funcs not CONFIRMED          %d" % len(unconf))
    print("  engine funcs in no subsystem        %d" % len(orphans))
    print("  engine funcs with no verify_plan    %d" % len(missing))
    # every commit since the tag pre-sonnet names its model and files in 03_re/ledger/work_log.csv (tools/worklog.py)
    import importlib.util as _ilu
    _spec = _ilu.spec_from_file_location("worklog", os.path.join(ROOT, "tools", "worklog.py"))
    _wl = _ilu.module_from_spec(_spec); _spec.loader.exec_module(_wl)
    unlogged = _wl.missing()
    print("  commits with no work_log row        %d" % len(unlogged))
    # VERIFIED-IDENTICAL rows need a current proof (03_re/staging/verify/prove_identity.txt, newer than the build)
    proof = os.path.join(ROOT, "03_re", "staging", "verify", "prove_identity.txt")
    exe = os.path.join(ROOT, "05_remake", "build", "Port", "recoil_boot.exe")
    ident = [r for r in rows if "VERIFIED-IDENTICAL" in r.get("remake_verification", "")]
    proven = set()
    if os.path.exists(proof) and (not os.path.exists(exe) or os.path.getmtime(proof) >= os.path.getmtime(exe)):
        proven = {l.split()[1] for l in open(proof, encoding="utf-8") if l.startswith("IDENTICAL ")}
    stale_ident = [r["ghidra_name"] for r in ident if r["ghidra_name"] not in proven]
    print("  IDENTICAL tags without a current proof %d" % len(stale_ident))

    if untagged:
        print("\n  first offenders:")
        for u in untagged[:10]:
            print("   ", u)
        if len(untagged) > 10:
            print("    ... and %d more" % (len(untagged) - 10))
    for u in src_closed[:10]:
        print("    gate:", u)

    ok = (not untagged and not impl_unconf and not src_closed and not unconf
          and not orphans and not missing and not unlogged and not stale_ident)
    print("\n" + "=" * 72)
    print("  VERDICT: " + ("CLEAN" if ok else "DRIFTED - fix before writing new code"))
    print("=" * 72)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())

