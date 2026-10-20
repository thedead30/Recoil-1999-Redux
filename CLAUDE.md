# Recoil (1999) — 1:1 Remake

Working directory: the repository root

## Start every session with this command. Not with reading.

```
status.bat          (or: python 03_re/scripts/re_status.py)
```

It prints Stage 1 coverage, the Stage 2 gate, and the drift tripwires. It is the
orientation step. Reading prose logs is *not* orientation — the previous attempt had
a 7,372-line log that nobody, including the model that wrote it, actually read.

Then `ROADMAP.md` (board), `STAGE1.md` (protocol), `03_re/LOOP.md` (queue),
`03_re/WORKFLOW.md` (which tier does what — **scripts harvest, judgement adjudicates**).

## The five rules

Each one is enforced by a machine, not by good intentions. The previous attempt had
eleven excellent prose rules and produced 43 provenance tags against 1,180 constants.
**Rules that nothing checks have a half-life of about three days.**

**1. Every game constant carries a provenance tag.**
`CONFIRMED-BINARY` (cite address) · `CONFIRMED-DATA` (cite file+key) · `MEASURED-ASSET` ·
`INFERRED` (state the alternatives considered) · `INVENTED` (state what would close it) ·
`PLATFORM` (host API value, never from Recoil).
→ *Enforced by:* `re_status.py` untagged-constant count. Non-zero = build is dirty.

**2. No code for a function until it is CONFIRMED, nor for a subsystem until its gate opens.**
A gate opens when membership is CLOSED, every member is CONFIRMED and every callee is
covered (`03_re/ledger/subsystem_registry.csv`). Would have stopped a 1,888-line `App::Tick()`.
→ *Enforced by:* `re_status.py` "code written for unconfirmed funcs" + "code outside an open subsystem gate".

**3. INVENTED requires a logged failed recovery attempt.**
Which functions decompiled, which strings/xrefs searched, why it did not resolve.
"I did not look" is never grounds for INVENTED.
→ *Enforced by:* every INVENTED tag must name a `03_re/decomp/` record. Reviewable.

**4. Verification claims name their method.**
`VERIFIED-PLAYPATH` (real synthetic keystrokes) · `VERIFIED-VISUAL` (frames attached) ·
`VERIFIED-ORACLE` (matches original binary via TTD/x64dbg/emulate) · `VERIFIED-UNIT`
(test named) · `VERIFIED-IDENTICAL` (`tools/prove_identity.py`, re-proved after every build) · `IMPLEMENTED-UNVERIFIED` (**an acceptable thing to report; pretending
otherwise is not**).
These never count: an `RECOIL_*` env hook, a value printed to a log, "the code path
executes", a green build.

**5. One item, one status. The chat carries the same tags as the log.**
No group claims. No "all six verified" spanning items with different evidence. The user
reads the chat, not the log — a gap between them is the actual failure.

## Drift signature — what to watch for in my own output

Stop and run `re_status.py` if any of these appear:
- "verified" without a tag beside it
- a screenshot offered as proof of correctness
- a summary describing *feel* ("the tank handles better") rather than provenance
- a new file in `05_remake/src/` whose header names no address
- **`CLAUDE.md` growing.** This file getting longer is itself evidence of regression to
  prose-as-control. New findings become a ledger column or a linter check, never a new
  paragraph here.

## Ground rules

- Never guess a struct layout silently; ask for the hex range or file. Distinguish
  "this is how MW3/mech3ax does it" from "confirmed for Recoil".
- Behavioural accuracy (physics, AI timing, damage, scripting) is Core, held to the same
  bar as visual accuracy.
- Intel syntax for disassembly.
- The record is fallible. Prior-evidence claims from the old logs are *not* confirmed;
  two were already found wrong. Re-verify before advancing a status.

## Environment

Windows 11 · Ghidra 12.1.2 + GhidraMCP (connected, `Recoil.exe` loaded) · Java 21 ·
Python · Git · mech3ax · x64dbg · dgVoodoo2. All installed and working — do not reinstall.
Inline Ghidra scripting is **enabled**. `cdbX86.exe` (ships with WinDbg) replays TTD.

Evidence that exists and was barely used last time:
- **21 GB TTD traces**, `01_evidence/ttd_tools/game_trace` — replay **WORKS** (`cdbX86.exe -z`). Large ones hold real execution; small ones are empty.
- **4,018 reference frames** in `01_evidence/frames_index/` — combat run + clean run.
