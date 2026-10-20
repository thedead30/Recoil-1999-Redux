# Three-tier workflow — script, cheap model, judgement

## Why this exists
Thirty passes of Stage 1 produced 30 confirmed functions and about **eight substantive
self-corrections**: an LOD cross-fade that never runs, a worst-case error figure wrong by half,
a flag bit named "start failed" that was a first-time latch, a function guessed as a listener
query that was a root walk, a false alarm about a record that was already complete.

**Every one was a judgement error. None was an execution error.** The probes, dumps, queries and
commits were never wrong; what went wrong was interpreting a plausible-looking result — naming a
field from context, trusting a sparse sample, believing a decompiler rendering.

That asymmetry is what the split exploits. The expensive-and-mechanical work is separated from
the cheap-and-dangerous work, and only the latter needs judgement.

## The three tiers

### Tier 1 — scripts. Deterministic, free, cannot hallucinate.
| task | tool |
|---|---|
| evidence packet for a function | **`03_re/scripts/harvest.py`** |
| coverage, gate, drift tripwires | `re_status.py` |
| ledger transitions with their bar enforced | `set_status.py` |
| oracle queries against a trace | `ttd_verify.py` |

**Anything mechanical belongs here, not in a model.** Most of what made passes expensive was
cdb round-trips and decompilation reading — neither needs judgement.

### Tier 2 — a cheap model. Drafting from harvested facts.
Reads packets and produces **drafts**, never records:
- summarise what a decompilation does, in terms of offsets and registers only;
- disassemble at each flagged address and transcribe the instructions;
- read named constants out of the binary and tabulate them;
- propose candidate readings **explicitly tagged `INFERRED`**, with the alternatives.

**It may not name a field, advance a ledger row, or write to `04_spec/`.**

### Tier 3 — judgement. Adjudication, per item.
- decide whether the evidence supports a claim, and at which provenance tag;
- name fields, and only after reading what writes them;
- spot that a decompiler rendering is wrong;
- decide what to investigate next;
- perform every `set_status.py` transition to `CONFIRMED`.

## The non-negotiables

**1. Nothing enters `04_spec/` or reaches `CONFIRMED` without tier 3.**
The record *is* the deliverable — Stage 2 is meant to be near-mechanical translation from it, so
a wrong entry is worse than a missing one because nothing downstream will question it.

**2. Review is per item, never batched at the end.**
Wrong interpretations compound. The vertex-0 / light-list thread took four passes to untangle,
each built on the previous framing. A hundred unreviewed claims cannot be checked against each
other — they must be re-derived from the binary, which costs the same as doing the work.

**3. A name is a claim.**
"start failed" *sounded* right beside a play call; the 24-byte callee said otherwise. **Read
what writes a field before naming it.** Guesses live in the queue with a question mark and do
not graduate.

## `harvest.py`

```
python 03_re/scripts/harvest.py 0x00452ec0                 # full packet
python 03_re/scripts/harvest.py 0x00452ec0 --no-exec       # fast, skips trace probing
python 03_re/scripts/harvest.py --batch 0xA 0xB 0xC        # several at once
python 03_re/scripts/harvest.py 0x00452ec0 --disasm 40     # add N instructions
```

Writes `03_re/harvest/<address>.md`. Each packet contains:

| section | contents |
|---|---|
| Identity | ledger row, source-file attribution, extent |
| Execution | HIT/no-hit per trace **with positions**, and the standing caveat that a no-hit is provisional |
| Flags raised | every `ftol` call and approximation magic constant **with addresses** |
| Decompiler-output flags | argument-less calls, and `= 0.0` initialisers before them |
| Decompilation | raw |
| What this does NOT establish | stated explicitly, every time |

### It emits facts and flags, never conclusions
By design it says *"fast-sqrt magic constant at `0x00487d51`"*, not *"this computes a
distance"*; *"this call has no arguments but the site loads ECX first"*, not *"the argument is
the node"*. Everything is checkable; nothing is a name.

### The flags are the failure modes, not a generic checklist
Each corresponds to a repeated, documented failure:

| flag | the failure it catches |
|---|---|
| `FTOL` | Ghidra swallows FPU arithmetic. **Four** math approximations were missed this way. |
| `APPROXIMATION` | a cheap substitute is in use and the textbook version is the wrong one. |
| argument-less call | an unset prototype hiding register args. Believing one produced a model that transformed the **origin** instead of a real position. |
| `= 0.0` before a call | dead-store initialisers masking a callee's writes through a register-passed pointer. |

Validated against known ground truth: on `0x00452ec0` it flagged both traps that cost a full
pass to find by hand; on `0x00487c50` it found the fast sqrt at `0x00487d51` exactly; on
`0x00429870` it found all four `ftol`→`expApprox` pairs.

## The loop, restructured

```
1. tier 1:  harvest.py over the queue          -> 03_re/harvest/*.md
2. tier 2:  draft from packets                 -> proposals, tagged INFERRED
3. tier 3:  adjudicate per item                -> 04_spec/, set_status.py, commit
```

Step 1 is where the cost was and needs no judgement. Step 3 is where the errors were and needs
nothing else.

## What this does not solve
Choosing **what to work on** is still judgement, and it is where several passes were saved
(probing `FUN_0048daf0` before reading it revealed dead code) and several wasted (four passes
untangling a thread that began with one wrong framing). `harvest.py` makes each item cheaper; it
does not decide which items matter.


---

## A worked example of the flag/conclusion distinction — added 2026-09-10
`harvest.py` flagged an argument-less call and a `= 0.0`-before-a-call dead store in
`FUN_00452ec0`. **Both flags were correct.** The conclusion drawn from them — that the callees
overwrite those locals through register-passed pointers — was **wrong**, and was then repeated
twice as "confirmed".

Measured at the store: the input locals are **`(0, 0, 0)`** and the output is the world
position. The zeros were real; the transform maps the origin through the node's world matrix,
whose translation column *is* the world position. The decompiler had it right.

**The lesson is the one this file already states, learned the expensive way:** the harvester
emits facts and flags, **never conclusions**. A flag says *look here*, not *something is wrong
here*. Reading `[ebp-0x18]` — one command — would have settled it at the first pass instead of
the fourth.

**Rule: do not write "confirmed" unless the specific quantity named in the claim was read.**
Corroboration from adjacent facts is not confirmation, and three rounds of it produced a
confident wrong answer.
