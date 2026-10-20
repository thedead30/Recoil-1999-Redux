"""trace_vectors.py - L3 test vectors from TTD traces (STAGE2.md section 4, TODO_STAGE2 P0.7).

For each call of a function in a recorded trace of the ORIGINAL game, record the inputs at entry
(registers, stack arguments, memory regions) and memory regions at return, and write them as a
vector file in the same shape the emulation harness writes (05_remake/tests/vectors/<name>.trace.json),
so the same C++ tests can replay them.

How (two cdb passes over the trace, via 03_re/scripts/ttd_verify.py):
  1. `dx @$cursession.TTD.Calls(addr)` lists every call with its TimeStart / TimeEnd positions.
  2. For each call: `!tt <start>` -> print registers, stack args, input memory; remember the output
     pointers; `!tt <end>` -> print output memory. Markers separate the cases.

usage:
  python tools/trace_vectors/trace_vectors.py --trace Recoil22 --addr 0x00472960 --name Vec3_Lerp \
      --regs ECX,EDX --stack 1 --in ECX:3,EDX:3 --out ECX:3 --max 64
  --in / --out   REG:ndwords  dwords read at [REG] (REG's entry value) at entry / at return
Values are little-endian hex dwords, the vector format of tools/emu_harness/emu.py.
"""
import argparse
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "03_re", "scripts"))
import ttd_verify  # noqa: E402


def le(h):
    return bytes.fromhex(h)[::-1].hex()


def calls(trace, addr, n):
    out = ttd_verify.run(trace, "dx -r2 @$cursession.TTD.Calls(%s).Take(%d).Select(c => new { s = c.TimeStart, e = c.TimeEnd })" % (addr, n), timeout=1800)
    pos = re.findall(r"s\s*:\s*([0-9A-F]+:[0-9A-F]+).*?e\s*:\s*([0-9A-F]+:[0-9A-F]+)", "\n".join(out), re.S | re.I)
    return pos


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--trace", required=True)
    ap.add_argument("--addr", required=True)
    ap.add_argument("--name", required=True)
    ap.add_argument("--regs", default="")
    ap.add_argument("--stack", type=int, default=0)
    ap.add_argument("--in", dest="inp", default="")
    ap.add_argument("--out", default="")
    ap.add_argument("--max", type=int, default=64)
    a = ap.parse_args()
    regs = [r.strip().lower() for r in a.regs.split(",") if r.strip()]
    mins = [(x.split(":")[0].lower(), int(x.split(":")[1])) for x in a.inp.split(",") if x]
    mouts = [(x.split(":")[0].lower(), int(x.split(":")[1])) for x in a.out.split(",") if x]

    pos = calls(a.trace, a.addr, a.max)
    if not pos:
        sys.exit("no calls of %s found in %s" % (a.addr, a.trace))
    cmds = []
    for i, (s, e) in enumerate(pos):
        cmds.append("!tt %s" % s)
        cmds.append(".printf \"CASE %d ENTRY\\n\"" % i)
        for r in regs:
            cmds.append(".printf \"R %s %%08x\\n\", @%s" % (r, r))
        for k in range(a.stack):
            cmds.append(".printf \"S s%d %%08x\\n\", dwo(@esp+%d)" % (k, 4 + 4 * k))
        for r, n in mins:
            for k in range(n):
                cmds.append(".printf \"I in_%s %%08x\\n\", dwo(@%s+%d)" % (r, r, 4 * k))
        for j, (r, n) in enumerate(mouts):
            cmds.append("r $t%d = @%s" % (j, r))
        cmds.append("!tt %s" % e)
        cmds.append(".printf \"CASE %d EXIT\\n\"" % i)
        for j, (r, n) in enumerate(mouts):
            for k in range(n):
                cmds.append(".printf \"O out_%s %%08x\\n\", dwo(@$t%d+%d)" % (r, j, 4 * k))
    # one command per line in a cdb script file (a single -c string exceeds the Windows limit)
    import tempfile
    fd, script = tempfile.mkstemp(suffix=".cdb", text=True)
    with os.fdopen(fd, "w") as f:
        f.write("\n".join(cmds) + "\n")
    try:
        text = "\n".join(ttd_verify.run(a.trace, "$$><%s" % script, timeout=3600))
    finally:
        os.remove(script)

    cases, cur = [], None
    for line in text.splitlines():
        m = re.search(r"CASE (\d+) (ENTRY|EXIT)", line)
        if m:
            if m.group(2) == "ENTRY":
                cur = {}
            elif cur is not None:
                cases.append(cur)
            continue
        m = re.match(r"\s*[RSIO] (\w+) ([0-9a-f]{8})\s*$", line)
        if m and cur is not None:
            cur.setdefault(m.group(1), []).append(le(m.group(2)))
    # the last case's EXIT lines come after the final marker
    if cur is not None and cur and (not cases or cases[-1] is not cur):
        cases.append(cur)
    dst = os.path.join(ROOT, "05_remake", "tests", "vectors", a.name + ".trace.json")
    json.dump({"function": a.name, "address": a.addr, "trace": a.trace,
               "source": "TTD trace of the original game (tools/trace_vectors/trace_vectors.py)",
               "cases": cases}, open(dst, "w"), indent=1)
    print(a.name, len(cases), "trace vectors from", len(pos), "calls ->", os.path.relpath(dst, ROOT))


if __name__ == "__main__":
    main()
