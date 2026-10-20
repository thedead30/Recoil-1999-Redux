# Contributing

The aim is a remake that behaves **exactly** like the original, and every claim about it can be checked by a
machine. A change is welcome when it comes with that check.

## The rules

These come from `CLAUDE.md`, where each rule names the machine check that enforces it.

1. **Every game constant names its source.** Tag it `CONFIRMED-BINARY` (with the address), `CONFIRMED-DATA`
   (with the file and key), `MEASURED-ASSET`, `INFERRED` (list the alternatives you considered), `INVENTED` (say what
   would settle it) or `PLATFORM` (a Windows/API value, not Recoil's).
2. **No code for a function until the ledger marks it CONFIRMED** (`03_re/ledger/functions.csv`).
3. **INVENTED needs a recorded failed attempt to find the real value** (a record in `03_re/decomp/`).
4. **A claim that something is verified names its method:**
   - `VERIFIED-IDENTICAL` - proven byte-identical
   - `VERIFIED-ORACLE` - matches the original run on the same inputs
   - `VERIFIED-UNIT` - a named test passes
   - `VERIFIED-PLAYPATH` - checked by real keystrokes in the game
   - `VERIFIED-VISUAL` - checked against captured frames
   - `IMPLEMENTED-UNVERIFIED` - done but not checked (a perfectly acceptable thing to report)

   A green build, or "the code path runs", does not count as verification.
5. **One item, one status.** No "all of these verified" across items with different evidence.

Check the state of the whole project with:

```
python 03_re/scripts/re_status.py          (or status.bat)
```

Every number under DRIFT TRIPWIRES must be 0, and the verdict must be CLEAN.

## Checking a change

| Check | Command | What it proves |
|---|---|---|
| Identity | `python tools/prove_identity.py --all --out 03_re/staging/verify/prove_identity.txt` | Each ported function compiles to the original's code, allowing only for relocation, constants and string addresses. Re-run it after every build; `re_status` flags proofs older than the build. |
| Tests | `05_remake\build.bat` | Unit tests and oracle tests, which run the original function and the remake on the same inputs. The `native_fuzz_*` tests feed random inputs, so a few shards can fail differently from run to run; compare the list of failing tests before and after your change rather than expecting zero. |
| Data image | `python tools/asm_port/gen_data_image.py --check` | The data image matches the ledger. Needs your `Recoil.exe`. |
| Release | `python tools/make_release.py --check --exe <Recoil.exe>` | The source contains no game bytes, no binary game files and no personal paths. |

The verification tools need `pip install capstone pefile minidump`. They look for your `Recoil.exe` at
`00_original\game_install\Recoil.exe` and prove the build in `05_remake\build\Port`. To use other locations, set
`RECOIL_ORIGINAL_EXE` (your `Recoil.exe`) and `RECOIL_PROVE_EXE` (the `recoil_boot.exe` to prove; its `.map` file
must be beside it).

## Known gaps

Open problems are listed as rows in `03_re/ledger/known_gaps.csv`. Add a row there rather than writing about a gap in
prose. Bugs in the original game get a row of type `original-defect`; `ORIGINAL_DEFECTS.md` is generated from those
rows.

## Never

- **Never commit anything from the game:** no executable, data, maps, sounds, videos, screenshots, or pseudo-C or
  disassembly of the original. `tools/make_release.py --check` scans for this.
- **Never run a development build inside your real Recoil install.** The game writes files into its own folder.
