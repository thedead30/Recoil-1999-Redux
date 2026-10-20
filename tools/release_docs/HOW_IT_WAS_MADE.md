# How the Recoil Remake was made

This explains how the original program was taken apart, how it was rebuilt, and how each step was checked. Use it
to understand the work or to repeat it. Everything named here is in this repository, unless it is marked as
something you generate yourself from your own copy of the game.

## The idea in one paragraph

The remake reproduces the original's **behaviour**, not just its look, and every claim about it is checked by a
machine. Each function of `Recoil.exe` was identified, named and given a ledger row (Stage 1). Then it was rebuilt
so that it compiles back to the *same machine code*, and that was proven function by function (Stage 2). Between
the rebuilt game code and modern Windows sits a thin `platform` layer. All of the game's data comes from the
player's own copy at run time.

## Ground rules, enforced by machine

An earlier attempt at this remake failed. It had good written rules, but nothing checked them, so it drifted:
34,000 lines of code, and almost no constant could be traced to the original. This attempt turned every rule into a
check that `03_re/scripts/re_status.py` runs. The project is "CLEAN" only when every check reads zero:

- **Every constant names its source** (provenance tags): `CONFIRMED-BINARY` with an address, `CONFIRMED-DATA` with a
  file and key, and so on. The checker counts untagged constants.
- **No code before evidence.** A function may only be ported once its ledger row is CONFIRMED, and a subsystem only
  once all its members are.
- **Inventing a value needs a recorded failed attempt to find the real one.**
- **"Verified" must name its method:** IDENTICAL, ORACLE, UNIT, PLAYPATH or VISUAL. "It compiles" and "it feels
  right" are not evidence.

The full rules are in [`CLAUDE.md`](CLAUDE.md), with the protocols in [`STAGE1.md`](STAGE1.md),
[`STAGE2.md`](STAGE2.md) and [`03_re/WORKFLOW.md`](03_re/WORKFLOW.md). The work was done with an AI coding assistant
(Claude) working to those rules under human direction. That is why the rules are written as instructions, and why
every one of them is checked by a machine rather than trusted.

## Stage 1: reverse engineering

**Goal:** a CONFIRMED ledger row for every engine function. That reached 3,330 of 3,330.

| Tool | Used for |
|---|---|
| **Ghidra** (with the GhidraMCP bridge) | Disassembling and decompiling the original; naming functions; listings. The scripts in `03_re/scripts/ghidra/` export listings, string ranges and indirect call edges. |
| **The ledger** (`03_re/ledger/functions.csv`, built by `03_re/scripts/build_ledger.py`) | One row per function: name, status, callers and callees, original source file, verification plan and result. Status is a number you can count, not an impression. |
| **`harvest.py`** (`03_re/scripts/`) | The first step for every function: an evidence packet with its calls, its constants, whether it runs in the recorded traces, and what is *not* yet established. |
| **Time Travel Debugging traces** (WinDbg TTD, replayed with `cdb`) | About 21 GB of recorded real play. Shows which functions actually run, with what values, in what order. `03_re/scripts/ttd_verify.py`. |
| **x64dbg and memory snapshots** | Live inspection of the original. Snapshots taken on a key press were compared with the remake's at the same moment in the game. |
| **Reference frames** | About 4,000 captured frames from the original, for visual checks. |
| **Emulation** (`tools/emu_harness/`, Ghidra's emulator) | Running single functions in isolation. |
| **Source attribution** (`file_attrib.py`) | The original's assert strings name 64 of its own source files. 1,571 functions were placed in their original files from these strings. |

Each subsystem got a written spec (`04_spec/systems/`), and every function a reference entry
(`04_spec/reference/`). These are the documentation of how Recoil works.

## Stage 2: rebuilding

**Decisions that shaped the remake** (`STAGE2.md` section 9):
- **32-bit x86, x87 floating point, MSVC.** The same kind of build as the original, so the arithmetic matches bit
  for bit.
- **Mirror the original's layout.** Same source files, same function boundaries, same names as the ledger. When
  the code mirrors the binary, "is this right?" can always be answered by comparison.
- **Instruction-level ports.** Each function is written as MSVC inline assembly converted from the Ghidra listing
  (`tools/asm_port/ghidra2masm.py`, `port_batch.py`, `port_closure.py`). Structure fields stay as offsets, and their
  meaning lives in the specs.
- **A data mirror.** The original's data sections (`.rdata`, `.data`, `.rsrc`) are kept at fixed places in the
  remake. Every address in them that points somewhere is relocated at start-up to the matching remake function or
  data (`tools/asm_port/gen_data_image.py`). These bytes are read from the player's own `Recoil.exe` when the remake
  starts.
- **A platform layer** (`05_remake/src/platform/`). Every Windows and DirectX function the original imports goes
  through a slot the remake fills in (`gen_iat_slots.py`). That is where the remake adapts to modern Windows: no-CD
  mode, the virtual CD drive for the music, windowed mode, and dgVoodoo for DirectDraw.

**How each port is checked:**

| Check | Tool | What it shows |
|---|---|---|
| **Identity** (`VERIFIED-IDENTICAL`) | `tools/prove_identity.py` | The compiled port matches the original instruction for instruction, allowing only for relocated addresses. It handles switch tables, exception-handling tables, inline copies and linker folding, and each of its rules was tested by planting a fault and checking the prover catches it. **4,678 of 4,716 functions are proven.** |
| **Oracle** (`VERIFIED-ORACLE`) | `05_remake/tests/` (`native_oracle.h`, `arena_fuzz.h`) | The test process loads the original `Recoil.exe` at its real address, runs the original function and the port on the same random inputs, and compares the results and the memory. |
| **Shape checks** | `bytediff.py`, `check_targets.py`, `check_void_returns.py`, `check_ret_cleanup.py`, `check_jumptables.py`, `port_audit.py` | Catch what a byte compare misses: wrong call targets, a lost return value, wrong stack clean-up. |
| **Play** (`VERIFIED-PLAYPATH`) | the launcher with save/compare switches | Real play, with memory snapshots compared against the original at the same moment. This is how the "turrets don't fire" bug was found (`KG-51`): a ported function returned `void` where the original returned a value in `EAX`. That one fix also cured exploding barrels and several other faults. |

Open problems are rows in `03_re/ledger/known_gaps.csv`, never paragraphs in a document. Each commit has a row in
`03_re/ledger/work_log.csv`.

## Shipping without shipping the game

- The remake's exe contains **no data from `Recoil.exe`**. At start-up it reads the data sections from the player's
  copy (`original\Recoil.exe`). It checks the file's size and SHA-256, and checks that the loaded image is
  byte-for-byte what was tested.
- The player `Setup.exe` (`05_remake/setup/`) reads the player's copy in any common form: an installed folder, a zip,
  the CD with its InstallShield cabinet, `.iso`, `.bin`/`.cue` or `.nrg`. It refuses other versions of the game. It
  turns the CD audio into WAV files, copies the original's menus, dialogs and icons into the remake, and sets up
  dgVoodoo. `tools/setup/` holds the Python reference version, and `compare_setup.py` checks that both install
  identical files.
- `tools/make_release.py` builds both downloads. It refuses to write them if the source holds any byte table from
  `Recoil.exe`, any game file, or any personal path.

## Repeating or extending the work

1. **Get a supported copy** (1999-01-29 build), and build the remake as described in [BUILDING.md](BUILDING.md).
2. **Make your own Ghidra project** from your `Recoil.exe`. The decompiled pseudo-C and the disassembly listings
   are not published here, because they are generated directly from the original program. Regenerate them
   yourself with the scripts in `03_re/scripts/ghidra/`. Use the names and comments in the ledger (`ghidra_name`,
   `notes`) to label your project.
3. **Pick a function** in the ledger. Read its spec and reference entry, then its port. Run
   `python 03_re/scripts/harvest.py <address>` for its evidence packet.
4. **Change something** in the remake: a fix, or something new for later versions (higher resolutions, better
   effects). Use [CONTRIBUTING.md](CONTRIBUTING.md): re-prove identity for the functions you did not mean to
   change, compare the test results before and after, and keep `re_status` CLEAN.

## What is not finished

- **38 functions** are not proven identical. They are verified in other ways, or listed as known gaps.
- **Multiplayer** (`net`, `znetwork`) is not in the first release.
- **The test suite has about 1,000 failing fuzz jobs.** These are known problems in the test harnesses themselves
  (`KG-66`), not in the game. Compare the failing list before and after a change rather than expecting zero.
- About 1.7 KB of the original's error messages are kept as text in the ported geometry code.
- **Open gaps:** see `03_re/ledger/known_gaps.csv`.
