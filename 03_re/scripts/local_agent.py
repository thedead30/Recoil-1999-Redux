#!/usr/bin/env python
"""Local-model runner: loops a work queue, one function per iteration.

The model NEVER touches the filesystem. This script does all I/O and writes only
to 03_re/staging/, so the write prohibitions cannot be violated by anything the
model emits. The model's only job is to turn an evidence packet into prose.

  python 03_re/scripts/local_agent.py            # work the queue until empty
  python 03_re/scripts/local_agent.py --once     # one item, for testing
"""
import argparse, io, json, os, re, subprocess, sys, time
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
STAGING = os.path.join(ROOT, "03_re", "staging")
HARVEST = os.path.join(ROOT, "03_re", "harvest")
QUEUE = os.path.join(STAGING, "QUEUE.txt")
LOG = os.path.join(STAGING, "PROGRESS.log")

ENDPOINT = os.environ.get("LOCAL_LLM", "http://127.0.0.1:8888/v1/chat/completions")
API_KEY = os.environ.get("LOCAL_LLM_KEY", "")   # set LOCAL_LLM_KEY; never commit a key
MODEL = os.environ.get("LOCAL_LLM_MODEL", "unsloth/Qwen3.8-Flash-Next-GGUF")

# Re-sent on EVERY request. The endpoint is stateless and a 4-bit model's rule
# adherence decays over a long conversation, so this is never sent "once".
SYSTEM = """/no_think You do mechanical evidence collection for a reverse-engineering project on Recoil (1999). You produce checkable facts, never conclusions. A more capable model reviews your work later and makes every judgement call.

HARD RULES:
1. Never state a hex value that is not in the material you were given. If asked for something absent, write exactly: NOT IN PROVIDED MATERIAL. Fabricated hex is the single most damaging thing you can produce, because it looks correct.
2. Never write dead, unused, or never-executes. A zero execution count is provisional: phrase it as "0 in trace X". Functions that run only at start-up read 0 everywhere because every trace attaches to an already-running game.
3. Never name a struct field, flag bit, or enum value. A candidate reading must be tagged INFERRED and carry at least one alternative you cannot rule out.
4. Never write "confirmed" about a finding.
5. You have no Ghidra and no internet. Work only from the packet given to you.

OUTPUT FORMAT - exactly these sections:
# 0x<address>
## What this function does
(offsets and registers only, no names)
## Execution counts
## Numbers observed
(each with the offset or register it came from)
## Questions for review
(MUST be non-empty and name something real you could not settle; reporting total success on an unfamiliar binary is not care, it is agreeableness)"""

TEMPLATE = """Produce the staging file for %s.

--- evidence packet, verbatim ---
%s
--- end packet ---"""


def _loaded_quant():
    """The server serves several quants under one model id, so record which
    one produced each file."""
    try:
        req = urllib.request.Request(ENDPOINT.replace("/chat/completions", "/models"),
                                     headers={"Authorization": "Bearer " + API_KEY})
        with urllib.request.urlopen(req, timeout=15) as r:
            for m in json.loads(r.read().decode("utf-8"))["data"]:
                if m.get("loaded") and m["id"] == MODEL:
                    return m.get("quant") or "unknown-quant"
    except Exception:                                             # noqa: BLE001
        pass
    return "unknown-quant"


QUANT = _loaded_quant()


def log(msg):
    line = "%s  %s" % (time.strftime("%Y-%m-%d %H:%M:%S"), msg)
    print(line)
    with io.open(LOG, "a", encoding="utf-8") as f:
        f.write(line + "\n")


def call_model(user, max_tokens=3700):
    body = json.dumps({
        "model": MODEL, "temperature": 0.1, "max_tokens": max_tokens,
        "chat_template_kwargs": {"enable_thinking": False},
        "messages": [{"role": "system", "content": SYSTEM},
                     {"role": "user", "content": user}],
    }).encode("utf-8")
    req = urllib.request.Request(ENDPOINT, data=body, headers={
        "Content-Type": "application/json", "Authorization": "Bearer " + API_KEY})
    with urllib.request.urlopen(req, timeout=900) as r:
        d = json.loads(r.read().decode("utf-8"))
    ch = d["choices"][0]
    return (ch["message"].get("content") or ""), ch.get("finish_reason")


def truncated(text, reason):
    """A reply that stops mid-word is not a complete reply. The server has been
    seen reporting finish_reason 'stop' on a truncated answer, so check the text
    itself rather than trusting the field."""
    if reason == "length":
        return True
    t = text.rstrip()
    return bool(t) and t[-1] not in ".)`:!?\u2014-" and not t.endswith("```")


def _values(text):
    """Every number in the text as an integer VALUE, so 0x56b2ac, DAT_0056b2ac,
    0x00000000 / 0 and 0xFFFFFFFF / -1 compare equal. A string comparison
    false-flagged 63 of the first 207 failures on leading zeros alone."""
    s = set()
    for t in re.findall(r"(?<![0-9A-Za-z])(?:0x)?([0-9a-fA-F]{2,})", text):
        try:
            s.add(int(t, 16))
        except ValueError:
            pass
    # Hex embedded in a Ghidra identifier (&stack0xffffff8c, DAT_0056b2ac, FUN_..., LAB_...,
    # PTR_...) is invisible to the pattern above; 0x004a88f0 was false-flagged that way.
    for t in re.findall(r"(?:stack0x|DAT_|FUN_|LAB_|PTR_)([0-9a-fA-F]{4,})", text):
        s.add(int(t, 16))
    for t in re.findall(r"(?<![\w.])-?\d+(?![\w.])", text):
        v = int(t)
        s.add(v)
        s.add(v & 0xffffffff)
    return s


def fabrication_check(reply, packet):
    """Mechanical, not a matter of trusting the model: every 4+ digit hex token in
    the reply must match, BY VALUE, some number in the packet. Still flags what
    it should: invented instruction addresses, and float encodings the model
    computed itself (e.g. 0x40800000 = 4.0f) that the packet never states."""
    seen = _values(packet)
    return sorted({"0x" + t for t in re.findall(r"0x([0-9a-fA-F]{4,})", reply)
                   if int(t, 16) not in seen})


def work(addr):
    out = os.path.join(STAGING, "%s.md" % addr.replace("0x", ""))
    if os.path.exists(out):
        log("SKIP %s (already staged)" % addr)
        return
    log("harvest %s" % addr)
    subprocess.run([sys.executable, os.path.join(ROOT, "03_re", "scripts", "harvest.py"), addr],
                   cwd=ROOT, capture_output=True, text=True, timeout=1800)
    pk = os.path.join(HARVEST, "%s.md" % addr.replace("0x", ""))
    if not os.path.exists(pk):
        log("FAIL %s - harvest produced no packet" % addr)
        return
    packet = io.open(pk, encoding="utf-8").read()
    if len(packet) > 40000:
        packet = packet[:40000] + "\n[TRUNCATED BY RUNNER]"

    reply, reason = call_model(TEMPLATE % (addr, packet))
    if truncated(reply, reason):
        log("retry %s (truncated reply)" % addr)
        reply, reason = call_model(TEMPLATE % (addr, packet), max_tokens=5500)

    bad = fabrication_check(reply, packet)
    header = "<!-- generated by local_agent.py (%s %s); NOT reviewed; not evidence -->\n" % (MODEL, QUANT)
    if bad:
        header += ("\n> **FABRICATION CHECK FAILED.** These hex values are not in the "
                   "packet: %s\n> Treat this entire file as unreliable.\n\n" % ", ".join(bad))
        log("WARN %s - unsourced hex: %s" % (addr, ", ".join(bad)))
    if truncated(reply, reason):
        header += "\n> **REPLY TRUNCATED even after retry. Incomplete.**\n\n"
    io.open(out, "w", encoding="utf-8").write(header + reply + "\n")
    log("wrote %s (%d chars%s)" % (out, len(reply), ", FLAGGED" if bad else ""))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--once", action="store_true")
    a = ap.parse_args()
    if not os.path.isdir(STAGING):
        os.makedirs(STAGING)
    if not os.path.exists(QUEUE):
        sys.exit("no queue at %s" % QUEUE)
    items = [l.split("#")[0].strip() for l in io.open(QUEUE, encoding="utf-8")]
    items = [i for i in items if i.startswith("0x")]
    log("queue has %d item(s)" % len(items))
    for addr in items:
        try:
            work(addr)
        except Exception as e:                                    # noqa: BLE001
            log("ERROR %s: %s" % (addr, e))
        if a.once:
            break
    log("queue complete - stopping. Do not invent further work.")


if __name__ == "__main__":
    main()
