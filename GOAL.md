# GOAL

## What this project is

A **1:1 remake of Recoil (1999)** — Zipper Interactive's tank combat game on the GameZ
engine — running natively on Windows 11.

Not a port. Not a game "inspired by" Recoil. A reimplementation whose behaviour is
derived from the original binary, function by function, with every value traceable to
the address or data file it came from.

## What "1:1" means here, precisely

The remake is correct when, given the same inputs, it produces the same **behaviour** as
`Recoil.exe` — not merely the same *look*. Physics, AI timing, damage formulas, mission
scripting, trigger logic and input handling are held to the same bar as geometry and
textures. A tank that looks right and drives wrong is a failure.

The test of success is not "does it feel like Recoil". It is: **can every behaviour in
the remake be traced to a line in the original?**

## Scope and order

- **Map 1 first, to 100%.** Every bug, every trigger, every enemy behaviour. Only then
  map 2. Other maps are touched only for cheap generalisation checks.
- **Within map 1:** terrain/geometry/textures → tank movement/controls/animation →
  enemies, destructibles, triggered events.
- The 13 levels are **maps**; each map contains missions. Use that vocabulary.
- Maps 2–13 are then intended to be largely *generated* by the same tools and scripts
  that closed map 1. If closing map 2 requires new thinking, map 1 was not really closed.

## Longer-term intent

- Moddable, with upscaled assets as an option.
- Multiplayer is a stretch goal, not a Stage 2 concern.
- An installer comes last.
- Savegame format: documented near-perfectly. Loading *original* saves is explicitly
  **not** required.

## Two stages, and the gate between them

**Stage 1 — reverse engineering.** Produce a complete, machine-checkable record of how
the original works. Done when **all 3,330 engine functions** in `Recoil.exe` are 100%
CONFIRMED.

*Scope widened 2026-09-09* from "map 1's reachable set" to the whole binary. The reachable
set was a computed guess that wrongly excluded the entire render subsystem and most of
input, and 1,587 of its exclusions were unnamed functions nobody could judge. Reading
everything removes the boundary entirely — see `STAGE1.md`.

**Stage 2 — translation.** Write the remake against that record, function by function,
mirroring the original's structure so correspondence can always be checked.

**The gate:** no Stage 2 code until Stage 1's number reaches 100% of 3,330. Enforced by
`re_status.py`, not by intention.

## Why the gate exists — the previous attempt

The first attempt reached 34,326 lines of C++ with Stage 1 roughly 11% started. The
result: good-looking tank and terrain, but enemies that barely worked, destructibles and
triggered events that did not work at all, and weapons that behaved wrong.

The mechanism of that failure is worth stating plainly, because it was structural rather
than careless. The remake collapsed into a single 1,888-line `App::Tick()` while the
original is 4,487 separate functions. Once the code no longer mirrored the binary, the
only available question was *"does this feel right?"* — and feel-tuning is invention with
extra steps. Provenance discipline then decayed to 43 tags across 1,180 constants, and
verification tags to zero.

**So: the remake's file and function layout mirrors the original's. Always.** That single
constraint is what keeps "is this correct?" answerable.

## Definition of done — map 1

- `re_status.py` reports **3,330 / 3,330 CONFIRMED** and the gate OPEN.
- Every constant in `05_remake/src/` carries a provenance tag; the untagged count is 0.
- Every behaviour-bearing function has an oracle check against TTD or the reference frames.
- A full playthrough of map 1 matches the reference capture: same triggers fire, same
  enemies behave the same way, same weapons produce the same effects.

## How the work is divided (2026-09-10)
Stage 1 is deliberately splittable now: `03_re/scripts/harvest.py` turns the expensive,
mechanical part of a pass — cdb round-trips, decompilation reading, trap-hunting — into a
structured evidence packet that any tier can produce, leaving only judgement to be spent on
naming fields and advancing ledger rows. `03_re/WORKFLOW.md` holds the split and its limits.

**This does not change the gate.** A cheaper tier may harvest evidence and propose `INFERRED`
readings; it may not write to `04_spec/` or move a row to `CONFIRMED`. The record *is* the
Stage 2 input, so a wrong entry is worse than a missing one — nothing downstream will question
it.
