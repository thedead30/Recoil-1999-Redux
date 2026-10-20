# STAGE 1 — Reverse Engineering. The protocol, and the definition of done.

## SCOPE CHANGED 2026-09-09 — the whole binary, not a map-1 subset

Stage 1 is finished when **every engine function in `Recoil.exe` has a CONFIRMED ledger
row** — all **3,330** (4,309 total minus 837 library/CRT/MFC and 142 thunks; count as of 2026-09-25, after G1 added 94 vtable-only functions). Not when it
feels understood. Not when the log is long. When the number is 100%.

**Why the map-1 subset was abandoned.** The reachable set was computed by walking the
static call graph from curated roots. It failed twice at subsystem scale: the entire
**render** subsystem (32 named functions) and most of **input** (11, including
`Input_PollAllDevices` and `Input_Keyboard_PollAndTranslate`) were marked unreachable,
because both are entered through installed function-pointer tables a static walk cannot
follow. Worse, **1,587 of the 1,674 excluded functions were unnamed `FUN_`** — unjudgeable
without reading them. The exclusion was never a judgement; it was a guess already proven
wrong twice.

One simple rule replaces it: **read everything.** Nothing to adjudicate, no boundary to
maintain, no risk of a gate reading OPEN while a subsystem sits unread — and maps 2-13 need
the same coverage eventually.

Current: **3,330 / 3,330 CONFIRMED** (from `re_status.py`, 2026-09-25).

---

## THE PROMPT — paste this to resume Stage 1 in any session

> Continue Stage 1 on Recoil. Run `python 03_re/scripts/re_status.py` first and tell me
> the numbers before anything else. Then work the Stage 1 queue in `ROADMAP.md` in
> priority order, following the per-function protocol in `STAGE1.md`.
>
> Do not write any code in `05_remake/`. The gate is closed and you are not to open it.
> If you find yourself wanting to implement something, that is the signal to add a
> ledger row, not a source file.
>
> Work autonomously — do not stop to ask "shall I continue?". Keep looping: pick the
> next function, decompile it, write its record, update its ledger row, commit. Batch
> your reporting: tell me at the end of a work unit what moved, with tags, and lead with
> whatever failed or could not be verified.
>
> When you hit something you cannot recover, log the failed attempt (R3) and move on to
> the next function rather than inventing. A blocked function with a documented attempt
> is progress. An invented one is debt.

---

## Start every function with `harvest.py`. Not with the decompilation.

```
python 03_re/scripts/harvest.py 0x004xxxxx          (--no-exec to skip trace probing)
python 03_re/scripts/harvest.py --batch 0xA 0xB 0xC
```

It writes `03_re/harvest/<address>.md`: ledger identity, source-file attribution, **execution
across traces with positions**, every `ftol` call and approximation magic constant **with
addresses**, argument-less calls, `= 0.0` dead stores before them, the raw decompilation, and
an explicit list of what the packet does **not** establish.

**Two reasons this is the first step, both learned the hard way:**

1. **It probes execution before you invest.** `FUN_0048daf0` had the densest concentration of
   approximation sites in the binary and was top of the queue; it does not execute in any
   trace. Probing first saved the pass.
2. **It flags where the decompiler is lying.** Believing an argument-less call once produced a
   model that transformed the **origin** instead of a real position. Four separate math
   approximations hid behind `ftol()`.

The packet emits **facts and flags, never conclusions** — "fast-sqrt constant at `0x00487d51`",
not "this computes a distance". Naming anything in it is judgement work; see
`03_re/WORKFLOW.md` for which tier may do what.

## Definition of CONFIRMED — the bar a row must clear

A ledger row moves to `CONFIRMED` only when **all five** hold. Anything less stays at
`DECOMPILED` and is honestly reported as such.

1. **A record file exists** at `03_re/decomp/<address>_<name>.md` using the schema below.
2. **Every numeric constant** in the function is extracted with the address it was read
   from. No constant left as "some value".
3. **Every struct offset touched** is named, or explicitly listed as unknown. `+0x68` with
   no name is an open question, not a finished function.
4. **Callers and callees reconciled** against the ledger — the call graph in the record
   matches `functions.csv`.
5. **Verification appropriate to its class:**
   - **Behaviour-bearing** (physics, AI, weapons, damage, scripting, input, mission
     state): requires an **oracle check** — a TTD trace observation, a Ghidra
     `emulate_function` run, or an x64dbg breakpoint showing real register/memory values
     that match the static reading. Record which, and the observed values.
   - **Plumbing** (allocators, list walks, string helpers, file readers): a second
     independent read is sufficient. Mark `verification=STATIC-2X` in the ledger so the
     weaker bar is visible and auditable, never hidden.

**The prior logs do not clear this bar.** 183 rows carry `PRIOR-EVIDENCE` because an
address was mentioned somewhere in the old 7,372-line record. That means *someone looked*,
nothing more. Two such claims were already found wrong. Re-derive, then advance.

---

## Record schema — `03_re/decomp/<address>_<name>.md`

```markdown
# 0x004xxxxx  <Name>
Class: behaviour-bearing | plumbing
Status: DECOMPILED | CONFIRMED
Verification: ORACLE-TTD | ORACLE-EMULATE | ORACLE-X64DBG | STATIC-2X | NONE

## Signature
<calling convention, params with types, return>

## What it does
<3-6 sentences. Plain mechanism, no speculation. Mark any inference explicitly.>

## Constants
| value | read at | meaning | tag |
|---|---|---|---|
| 40.0f | 0x004267f3 | nom_gravity | CONFIRMED-BINARY |

## Struct offsets touched
| offset | type | name | evidence |
|---|---|---|---|

## Calls out to
| address | name | why |

## Called from
| address | name |

## Oracle evidence
<the trace/emulation/breakpoint observation, with real values. Or: why not applicable.>

## Open questions
<bullets. An empty list here on a complex function is a red flag, not a triumph.>
```

---

## Work order

**S1.0 — ~~Compute the map-1 reachable set~~ RETIRED 2026-09-09** (superseded by the scope
change above; the indirect-edge recovery it produced is still useful for understanding
dispatch, but it no longer decides scope). Original text:
Identify map-1 entry points (mission load, the frame tick, input dispatch, the objective
state machine), then walk the call graph in `functions.csv` to closure. Write `YES` into
`map1_reachable` for every function in it. This turns the Stage 2 gate from a vague
promise into a denominator. Expected size: 800–1,400 functions.

**S1.1 — Re-verify the 183 PRIOR-EVIDENCE rows.** Highest value per hour: the prior work
targeted the functions that matter, so this set is pre-prioritised. Each either advances
to CONFIRMED or gets corrected — and corrections are the most important output of this
phase, because the remake was built on some of them.

**S1.2 — Core behaviour systems, in this order.** Physics/movement → weapons/damage →
mission scripting and triggers → AI → input → collision. This order is chosen because it
matches the defect list from the last play test: the tank, then shooting, then the
gates/triggers that did not work at all, then the enemies that barely worked.

**S1.3 — Sweep the remaining reachable set.** Bulk decompilation. This is where fan-out
across subagents is worth proposing to the user, because the work is mechanical and the
output is a file on disk that can be spot-checked against Ghidra — not a conclusion that
has to be trusted.

**S1.4 — Close the gate.** 100% of the **3,330 engine functions** CONFIRMED, or explicitly
WAIVED in writing with a reason in the ledger. `re_status.py` prints `GATE: OPEN`. Only then does
Stage 2 begin.

---

## Verification — using the evidence that already exists

Two assets were collected and then barely used. They are the difference between this
attempt and the last one.

**TTD traces — 21 GB, `01_evidence/ttd_tools/game_trace`. REPLAY CONFIRMED WORKING
2026-09-09.**

```
cdbX86.exe -z <trace>.run -c "<commands>; q"
```
`cdbX86.exe` ships with the WinDbg Store package already installed — no separate install is
needed. Verified: the trace loads, modules resolve (`Recoil.exe` at `0x00400000`, plus
`DDRAW.dll`, `D3DImm.DLL`, `MSVCP50.dll`), the TTD analysers initialise, and `!tt` positions
within the trace.

**Which traces are usable — the large ones.** `Recoil02` (3.5 GB, ~653k positions),
`Recoil07`, `Recoil09` and `Recoil14` all contain real execution. The small ones
(`Recoil03`, `Recoil04`, `Recoil05`) report *"doesn't contain any recorded threads"* and are
useless — do not waste time on them.

**Provenance warning.** The traces were recorded against the `Recoil Runtime Copy` build,
while Ghidra has the `RECOIL files after setup` build loaded. Both sit at image base
`0x00400000`. They are *presumed* identical but this has **not** been byte-compared — do that
before treating a TTD observation as confirming a Ghidra address.

**4,018 reference frames** — a full combat run and a clean run of map 1, the clean run
ending with every weapon fired at least once. These are ground truth for weapon effects,
HUD state, enemy placement and terrain. Indexed in `01_evidence/frames_index/`.

**Rule:** a behaviour-bearing function verified without touching either of these should
be treated with suspicion. The evidence is sitting there; not using it was a large part
of what went wrong.

---

## What Stage 1 must NOT do

- Write anything into `05_remake/src/`. The gate is closed.
- Advance a status on the strength of the old logs alone.
- Mark a function CONFIRMED with unnamed struct offsets still in it.
- Produce a session that grows the log but not the ledger. If `re_status.py` shows the
  same CONFIRMED count as it did at session start, the session produced nothing,
  whatever the log says.

---

# THE ORACLE HARNESS — working as of 2026-09-09

`03_re/scripts/ttd_verify.py` runs cdb commands against a TTD recording of the real game
and returns the output. This is what makes `VERIFIED-ORACLE` possible, and therefore what
lets any ledger row reach `CONFIRMED`.

```
python 03_re/scripts/ttd_verify.py --list
python 03_re/scripts/ttd_verify.py --trace Recoil07 --cmds "!tt 50; dd 0x00778924 L2"
```

## Binary provenance — RESOLVED
The traces were recorded from the **no-CD build** (`Recoil Runtime Copy`), Ghidra has the
clean build. They differ by **exactly one byte**:

| | |
|---|---|
| VA | `0x0042e75a` |
| clean | `75 2d` — `JNZ 0x0042e789` |
| trace | `eb 2d` — `JMP 0x0042e789` |
| context | `CALL 0x004a59e0; TEST EAX,EAX;` then the jump — the **CD check** |

Everything else is byte-identical, so **every other address is directly comparable between
Ghidra and the trace.** Do not attempt to verify `0x0042e75a` itself through TTD.

This was proven from inside the trace: `u 0x0042e75a` returns `eb 2d jmp`, confirming both
the harness and the provenance in one command.

## Which traces to use
`Recoil02` (3.6 GB), `Recoil07` (2.1 GB), `Recoil09` (1.9 GB), `Recoil14` (2.1 GB) contain
real execution. `Recoil03`, `Recoil04`, `Recoil05` are recorded but **empty**
("doesn't contain any recorded threads") and are rejected by the script.

Also present and **unchecked**, but promisingly named:
`Recoil_missionload` (977 MB), `Recoil_startup` (537 MB), `Recoil_netflag` (411 MB),
plus `Recoil06/08/10/11/12/13/15`.

## The pattern that works
```
!tt 50                      position 50% into the trace, where the game is running
dd <global> L<n>            read globals
dd poi(<ptrGlobal>)+<off>   follow a pointer and read a field
u <addr> L<n>               disassemble to confirm the build
bp <addr>; g; r             break and dump registers
```
### Two positioning rules, both learned by getting them wrong
1. At `!tt 0` the globals are uninitialised and every read is zero. Use `!tt 50`.
2. **A `!tt N` read is a snapshot of unrelated moments.** Reading two globals that should
   agree at an arbitrary position produced a false mismatch on the physics timestep — the
   process was parked in `NtWaitForSingleObject` and one global held a stale frame's value.
   **To check a relationship between globals, break at the function that consumes them**
   (`bp <addr>; g; dd ...`) so both are sampled in the same frame.

### A caveat on all TTD-derived timings
Recording under TTD slows the process enormously, so frame deltas in the traces are **not
representative of normal play**. Timing-dependent claims can be verified for their
*relationships* but not for their *typical values*.

## Worked example — the first CONFIRMED row
Claim under test (from `04_spec/structs/WeaponMountEntry.md`): the mount array's count is at
`0x00778924`, its base at `0x00778928`, `+0x40` is `DAMAGE` and `+0x14` is `1/FIRE_RATE`.

```
!tt 50; dd 0x00778924 L2; dd poi(0x00778928)+0x40 L1; dd poi(0x00778928)+0x14 L1
->  00778924  00000013 16e9a3f0
    16e9a430  3f99999a
    16e9a404  3daaaaab
```

| observed | decodes to | expected from `weapons.json` | |
|---|---|---|---|
| `0x13` | 19 | 19 weapons in `BALLISTICS` | ✓ |
| `3f99999a` | `1.2000000476837158` | `wep_1.0` RFPG `DAMAGE` = `1.2000000476837158` | ✓ **bit-exact** |
| `3daaaaab` | `0.0833333358168602` | `FIRE_RATE` 12.0, stored as `1/12` | ✓ |

Binary reading, extracted data and live runtime all agree. `Weapon_LoadMountArrayFromConfig`
(`0x004b1190`) advanced to **CONFIRMED / ORACLE-TTD**.

**This is the bar.** A row reaches `CONFIRMED` when a claim from the decompile is checked
against the trace and matches — not when the decompile merely looks convincing.
