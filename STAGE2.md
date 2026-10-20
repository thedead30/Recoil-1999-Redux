# STAGE 2 - Translation. The strategy.

**Status: AGREED 2026-09-25.** You accepted decisions D1-D6 in section 9 as recommended, and
added that movies and CD music may be converted to other file formats. Nothing in `05_remake/` is
written until P0 is complete.

Open gaps are tracked in [`03_re/ledger/known_gaps.csv`](03_re/ledger/known_gaps.csv), counted by
`re_status.py`. Every phase below names the gaps that must be closed before it starts.

---

## 1. What Stage 2 is

Stage 2 writes the remake **against the Stage 1 record, function by function, mirroring the
original's structure**. It is a translation, not a re-design.

The one rule that decides everything else comes from `GOAL.md`: **the remake's file and function
layout mirrors the original's.** When the code mirrors the binary, "is this correct?" stays
answerable by comparison. When it stops mirroring, the only question left is "does it feel right?".
That is how the previous attempt failed.

A function is finished when its **verification has run and passed**, not when it compiles or plays
well. "It feels right" is never evidence.

## 2. Where we start

| | |
|---|---|
| Engine functions CONFIRMED | 3,330 / 3,330 |
| In v1 scope | 3,182 (148 multiplayer functions in `net` / `znetwork` are out) |
| Functions with a verification plan | 3,330 (ORACLE 2,372 · UNIT 415 · EMULATE 241 · STATIC-ACCEPTED 302) |
| Oracle-verified so far | about 100 |
| Narrative spec coverage | 3,331 / 3,331 |
| Known gaps open | 24 (12 of them reverse-engineering gaps; see the CSV) |
| Reference data | TTD traces (21 GB), 4,018 reference frames, 1 state-tagged combat capture run (1,013 frames) |

**Two facts from the binary shape the plan:**
- The original was built with **MSVC 5.0** (linker 5.10, January 1999). Its floating point is x87,
  running at 53-bit precision during gameplay (`VERIFIED-ORACLE`, fpcw `0x027F`).
- The binary names **64 of its own source files** in assert strings, in two projects:
  - `Battlesport\` (the game): `RecoilApp.cpp`, `player.cpp`, `hud.cpp`, `mission.cpp`,
    `map.cpp`, `pickup.cpp`, `turret.cpp`, `ai_net.cpp` and `Briefing.cpp`;
  - `GameZRecoil\` (the engine): `zClass\Class.c`, `Camera.c`, `Object3d.c`...; `zVideo\zvid_*.c`;
    `zSound\zsnd_*.cpp`; `zInterp\zinterp_parse.cpp`; `zRender\zrndr_draw.c`; `zMath\zmth_main.c`;
    and so on.

  The remake's source tree can therefore mirror the **original's own file names**, not a layout we
  invent. P0.4 attributed 1,571 of the 3,330 functions to 61 of those files (ledger `orig_file`). The
  other 1,759 sit in areas with no assert strings; they go in files named after their subsystem, and
  that placement is a layout choice, not a provenance claim.

## 3. Architecture - what is translated exactly, and what is replaced

| layer | what it is | how it is written | original subsystems |
|---|---|---|---|
| **A. Simulation** | game logic: vehicles, weapons, damage, AI, pickups, turrets, missions, triggers, timing | **exact translation**. No OS calls. Deterministic given (inputs, dt, RNG state) | vehicle, weapon, player, pickup, turret, ai_net, mission, collision, camera, app (tick) |
| **B. Engine data** | scene graph, node classes, assets, scripts, settings | **exact translation**. Structs are layout-faithful (section 5) | class_api, cls_world, object3d, scene_update, zclass_nodes, asset_io, texture, script, settings, declient, zeffect, savegame, math3d, transform, geometry |
| **C. Presentation logic** | what gets drawn and played: view setup, culling, polygon shading, draw-mode queues, HUD, the 2D UI pixel operations, sound voice logic | **exact translation up to the point where the original calls DirectX.** The 2D rasteriser pixel ops (blend line, colour-key blit, ripple, dissolve) are ported bit-exact | view, scene_render, poly_shade, render_frame, zvideo (data side), rasteriser (2D), hud, menus, ui_widgets, mapscreen, sound (voice logic), avi (sequencing) |
| **D. Platform** | window, GPU, audio device, input devices, files, timer, video decode | **replaced**. Every value here is tagged `PLATFORM` | the D3D5/DDraw calls, DirectSound device calls, DirectInput device calls, MCI CD audio, VFW decode, MFC shell |

**What is not translated at all:**
- the 3D software renderer (rejected by you);
- DirectPlay and Westwood Online (multiplayer, out of v1);
- the Windows menu bar and About box;
- the DirectX version probe.

**Headless first.** Layer A with layer B must run with no window, GPU or sound device. Every call
into layers C/D goes through an interface that, in test mode, **records the call sequence** instead
of executing it. That recorded sequence is itself comparable against the original's, read from TTD
(section 4, L3).

## 4. How "correct" is decided - the verification ladder

Each rung is mechanical. The tag it earns is the `CLAUDE.md` tag in brackets.

| rung | check | good for | tag |
|---|---|---|---|
| **L1 Emulate / native** | Run the **original function's bytes** on N generated inputs and compare the port **bit for bit**. Two forms: (a) Ghidra `emulate_function` vectors (`tools/emu_harness`); (b) **native**: the test runner maps `Recoil.exe` at its original addresses and calls the original function on the real x87 (`tests/native_oracle.h`). Native is the authority: Ghidra's emulator mis-models some FPU edge cases (FCOMP C3, FCHS of NaN) | pure functions: math, transform, geometry, pixel ops, parsers of in-memory data (241 EMULATE-planned) | `VERIFIED-ORACLE` |
| **L2 Unit** | Parse every shipped asset file and round-trip it; compare with the asset checkers (`asset_check_*.py`) and mech3ax counts | file formats, settings, save files (415 UNIT-planned) | `VERIFIED-UNIT` |
| **L3 Trace vectors** | From a TTD trace, record each call of a function: arguments plus the memory it reads (entry) and writes (exit). Replay those into the port and compare | everything stateful: gameplay, AI, damage (2,372 ORACLE-planned) | `VERIFIED-ORACLE` |
| **L4 Lockstep replay** | From a TTD trace, extract per frame: dt, input state, RNG state. Drive the whole headless remake with them and compare the game state **every frame** against the trace (vehicle instances, projectiles, mission state) | whole-system behaviour: the real acceptance test for gameplay | `VERIFIED-ORACLE` |
| **L5 Draw and frame compare** | (a) The original's **draw queues** at present time (vertices, texture, mode), read from TTD, against the remake's queues: exact and GPU-independent. (b) Pixel diff of rendered frames against the reference captures at matched state, within tolerance D2 | rendering | `VERIFIED-ORACLE` (a), `VERIFIED-VISUAL` (b) |
| **L6 Play path** | Scripted real keystrokes through menus and missions | front-end flows, integration | `VERIFIED-PLAYPATH` |

**The capture run you recorded cannot drive L4.** It has state snapshots but no per-frame inputs or
dt. It serves L5(b) and state sanity checks. L4 needs **TTD traces of real play** (KG-11). That is
the single most valuable thing you can record for P3.

A function may be reported `IMPLEMENTED-UNVERIFIED`. That is an acceptable state, but it never
counts toward a phase exit.

## 5. Layout-faithful data

Every struct the original keeps in memory is declared with the **original's offsets and sizes**.
Each field has a `static_assert(offsetof(...) == 0x...)` citing the struct map in
`04_spec/structs/`.

Globals live in per-module state blocks named after their address (the `04_spec/globals/` tables).

**D7 (2026-09-26) - the data image.** Globals that no module block names yet live in a byte-exact mirror of the
original `.rdata` and `.data`/`.bss` (`src/platform/image`, generated by `tools/asm_port/gen_data_image.py`), so
no array or struct extent is guessed and adjacency is exact. Initialised words that are addresses are relocated
at start-up: into the mirror, a named block (`03_re/ledger/data_blocks.csv`), a ported function, or a trap for
code not ported yet. `tests/test_data_image.cpp` checks every byte and relocation against the mapped original.
Named module blocks take precedence for their addresses (`port_batch.py` maps each address one way only).

**Why:**
- A remake object can be compared **byte for byte** with the same object read from a TTD trace
  (L3/L4). No field mapping is needed, and no field can be silently forgotten.
- Linked lists keep the original's insertion order and walk order. AI, collision and damage
  results depend on iteration order.
- Quirks survive automatically. The specs record 168 "quirk / reproduce / do not fix" notes (for
  example `stopping` overwriting `friction[1]`, the one-row vertical-clip quirk in the colour-key
  blit, and the prefix matching of script commands). The rule is: **reproduce, never fix**.

Pointers become 32-bit handles or indices where a struct is compared against trace memory. This
means a 32-bit build (D1) is also the simplest way to keep layouts identical.

## 6. Phases

Each phase has an entry gate and an exit criterion measured by a script, not by judgement. The
function counts are v1-scope ledger rows per phase.

| phase | scope | functions | entry gate | exit criterion |
|---|---|---|---|---|
| **P0 Foundations** (no game code) | decisions D1-D6; repo skeleton mirroring the 64 original files; build; tooling T1-T6; hygiene gaps | 0 | this document agreed | tools run on sample functions; KG-12, 19, 20, 21, 22, 23 closed |
| **P1 Pure core** | math3d, transform, geometry (clipping), CRT equivalents (rand, ftol, atof/atoi behaviour), expApprox | 150 | P0 done | 100% of P1 `VERIFIED-ORACLE` through L1, bit-exact |
| **P2 Engine data** | assets, archives, textures, GameZ loader, anim, zmap, settings/detail.zrd, script interpreter + cache, scene graph (class_api, cls_world, object3d, scene_update), zclass nodes, declient, zeffect, savegame | 719 | P1 done; KG-01, 07 closed | all 13 maps load; asset checkers pass; after loading map 1 the scene-graph node records equal the TTD memory image (L3) |
| **P3 Simulation, map 1** | app tick and timing, input mapping, vehicle (all movers), collision queries, weapons and projectiles, damage, player, pickups, turrets, AI net and AI, mission/objectives/triggers, camera logic | 783 | P2 done; KG-04, 05, 06, 09, 14, 15, 16, 17 closed; **TTD play traces recorded (KG-11)** | L4 lockstep on at least two traces (driving + combat): per-frame state equal (within D1 tolerance) for the whole trace; same events (hits, kills, pickups, triggers) on the same frame |
| **P4 Presentation, map 1** | view, scene_render, poly_shade, render_frame, zvideo data path + D3D backend, HUD, 2D rasteriser ops, sound logic + audio backend | 597 | P2 done (can run **in parallel with P3**); KG-13 closed | L5(a) draw queues equal at sampled frames; L5(b) pixel diff within D2 against the combat captures at lockstep-matched frames; HUD pixel-exact |
| **P5 Front end** | menus, ui_widgets, map screen, briefing, save/load screens, FMV (fmv.zrd sequences + decoder), settings screens, sysinfo equivalents | 933 | P4's 2D path done; KG-08, 18, 24 closed | every screen pixel-exact against reference frames (the 2D path is exact, so exact is expected); FMV frame-exact; L6 play path through new game → mission → save → load → quit |
| **P6 Map 1 closure, then maps 2-13** | full map-1 playthrough against reference; then the other maps using the same code and tools | - | P3-P5 done | map 1 as defined in `GOAL.md`; maps 2-13 load and replay-match without new code. If a map needs new thinking, map 1 was not really closed |
| **later (v2)** | 1080p / widescreen (same vertical FOV, 640×480 screens pillarboxed); multiplayer; installer; mods | 148 + new | v1 shipped | - |

**Order inside a phase:** bottom-up through the call graph, leaves first. Every function is then
testable with verified callees underneath it. A subsystem is "done" only when every member has a
verification tag other than `IMPLEMENTED-UNVERIFIED` (or `STATIC-ACCEPTED` with its reason).

**Why this order:**
- P1 is pure and has no gaps, so it proves the toolchain on the cheapest possible code.
- P2 has to exist before anything can be simulated or drawn.
- P3 and P4 are independent once P2 exists.
- P5 is the largest block (933 functions, mostly the UI widget library) but the least risky: it is
  deterministic 2D, and its correctness is visible pixel for pixel.

## 7. The per-function protocol (the Stage 2 loop)

1. **Pick** the next function: the lowest in the call graph, in an open phase, whose callees are
   verified.
2. **Re-read** its record: the ledger note, the spec section, the decomp record. If it is one of the
   19 decompile-only functions (KG-03) or appears in `03_re/ledger/decomp_check.csv` (KG-12:
   DROPPED-CODE or HIDDEN-REG-ARG), **translate from the listing, not the decompile**.
3. **Translate** it into the file that mirrors its original source file. Keep the ledger name.
   The header carries: address, `SUBSYSTEM:`, spec reference. Every constant is tagged (the
   `re_status` tripwires already enforce this).
4. **Verify** with its `verify_plan` rung. Write the result into the ledger columns
   `remake_file`, `remake_symbol` and `remake_verification`.
5. **Commit** with the address and the tag. Report with the same tag. One item, one status.

Tier split, as in `03_re/WORKFLOW.md`: a cheaper model may **draft** a translation when its check is
fully mechanical (L1/L2/L3). The check, not the drafter, decides. Anything verified only by L5(b)
or L6 needs judgement-tier review.

## 8. Tooling to build in P0

| id | tool | purpose |
|---|---|---|
| T1 | `05_remake` skeleton: CMake, the 64-file tree, the headless build and the platform-interface stubs | structure before code |
| T2 | `tools/emu_harness`: run an original function in the emulator on generated inputs; diff against the port | L1 |
| T3 | `tools/trace_vectors` (extends `ttd_verify.py`): per-call argument/memory vectors from TTD; per-frame dt, input and RNG streams for lockstep | L3, L4 (needs the input-state globals sourced from `input.md`) |
| T4 | `tools/lockstep`: headless runner and per-frame state differ against trace memory, using the layout-faithful structs | L4 |
| T5 | `tools/draw_diff` and `tools/frame_diff`: draw-queue compare against TTD, and pixel diff against captures | L5 |
| T6 | `decomp_check` (KG-12): flags functions whose decompile misses call targets or strings present in the listing | makes KG-03/KG-12 mechanical |
| T7 | file attribution: map every function to one of the 64 original source files (assert xrefs + contiguous address ranges: MSVC links object files in order) | the source tree mirrors the original |
| T8 | `re_status` extensions: verdict gates on unconfirmed / no-subsystem / no-plan counts (KG-19); remake functions counted by verification tag per phase | progress stays machine-measured |

## 9. Decisions (agreed 2026-09-25)

You accepted all six recommendations. For D6 you also allowed converting movies and CD music to
other formats: the movies are converted once, offline, and checked against the original's frames,
so the remake needs no Indeo/Cinepak decoder at run time.

| id | decision | recommendation | why |
|---|---|---|---|
| **D1** | numeric fidelity and build target | **32-bit (x86) build**. The simulation keeps x87 maths at 53-bit precision, with explicit `float` stores exactly where the original stores to `float`. Transcendentals (`fsin`, `fcos`, `fpatan`) go through x87 instructions | Matches the original's arithmetic and the double-rounding at `float` stores. Also keeps struct layouts identical (section 5). An x64/SSE2 build would need a tolerance in every L3/L4 comparison and could never be bit-exact |
| **D2** | pixel tolerance for 3D frames | exact for 2D screens and HUD; for 3D, compare **draw queues exactly** (L5a) and allow a small per-pixel tolerance on the frames themselves, set from measured D3D11-vs-dgVoodoo2 rasteriser differences on a test scene | GPU rasterisation differs slightly between drivers; the draw data does not |
| **D3** | graphics API | **Direct3D 11** | native on Windows 11; the fixed-function D3D5 states (Gouraud, fog, modulate, colour key, alpha blend) map onto a small shader set |
| **D4** | frame rate | keep the original's **variable dt with its clamp** [0.005, 0.125] and the frame-rate cap; for lockstep, dt is injected | the simulation is written around variable dt; a fixed tick would change behaviour |
| **D5** | random numbers | replicate the **linked CRT's `rand()`** exactly (confirm the LCG from the library bytes) with the same call order at every site (AI links, material colours, noise buffer, sound variants) | one extra or missing call desynchronises everything after it |
| **D6** | CD music and FMV | music: you supply the tracks as files, and the remake plays them where the original plays CD tracks (`track = mission % (n-2) + 2`). FMV: a bundled decoder for Indeo 5 and Cinepak (for example FFmpeg's libavcodec; licence to check), accepted by frame-exact comparison | Windows 11 has neither CD audio as the game expects nor an Indeo 5 codec (KG-18, KG-24) |

**Added 2026-09-30 (agreed with you in chat):**

| id | decision | what it means | why |
|---|---|---|---|
| **D8** | how the renderer is replaced | **Option A: our own implementation of the DirectDraw / Direct3D 5 objects the game calls, drawing with Direct3D 11 underneath** (a Recoil-only "dgVoodoo" of our own). The ported game code is not touched: it makes the same calls, in the same order, with the same values. Not option B (rewriting the ~200 render functions for D3D11) | every game function stays 1:1 and comparable with the original; the call sequence the layer receives can be recorded and compared with the original's (TTD) - the strongest render check available; B would turn those functions into "does it look right?" |
| **D9** | multiplayer code | the `net` / `znetwork` functions are **ported mechanically** like everything else, because 70 single-player functions (vehicle, player, menus, pickups...) call them (they check "is a session active?"). Multiplayer still does not work in v1: DirectPlay / Winsock stay unbound | keeps the single-player code 1:1; multiplayer as a feature stays out of v1 |
| **D10** | v2 enhancements (after v1) | planned for a v2: **1080p output, upscaled / replaced textures, better-looking bullet / laser / explosion effects**. All are **options that are off by default**, so with them off the remake is still the exact, verified v1. How: the D8 layer renders at a higher internal resolution by scaling the game's screen-space vertices (4:3 at 1440x1080 with bars needs no game change; true 16:9 widescreen needs a deliberate camera / FOV change in `view` / `scene_render` and high-res HUD / menu art - a logged v2 deviation); textures are recognised at upload by a hash of their pixels and swapped for a replacement pack; effect draws are recognised by texture and blend state and drawn with glow / bloom / additive light. New kinds of effects need changes to the effect code (`zeffect`) - also v2-only | option A keeps every enhancement in one switchable layer on top of an unchanged, verified game; nothing of v2 goes into v1 |

The D8 layer is built from the start with the three D10 hooks in mind (internal resolution scale, texture hashing at upload, effect-draw
classification), all disabled in v1.

**Also for the session:**
- Audio and input backends. I suggest DirectSound and DirectInput 8, which are still present on
  Windows 11 and closest to the original's semantics. They are `PLATFORM`, behind the layer D
  interface.
- Whether P3 and P4 run in parallel.

## 10. Risks

| risk | effect | mitigation |
|---|---|---|
| x87 differences (spills, transcendentals) | gameplay drifts over long runs | D1; L1 on every maths function; L4 catches drift early |
| order dependence (list walks, RNG calls, update order) | desync that looks like "AI is off" | layout-faithful lists; RNG stream checked in L4; call-sequence recording in headless mode |
| incomplete decompiles | silent missing behaviour | T6; the listing is the authority |
| thin reference data | P3 cannot be accepted | KG-11: targeted TTD traces before P3 exit |
| the UI widget library's size (810 functions) | P5 takes long | mechanical: pixel-exact checks let cheaper tiers draft |
| scope creep back to "feel" | the previous attempt's failure mode | phase exits are script-measured; `re_status` tripwires; no "handles better" reports |
| third-party decoder licence | FMV blocked | decide in D6 before P5 |

## 11. v1 definition of done

- Every in-scope function translated; every behaviour-bearing function carries a verification tag
  other than `IMPLEMENTED-UNVERIFIED`.
- The `re_status` tripwires are all 0: untagged constants, code for unconfirmed functions, code
  outside an open gate, INVENTED values.
- Map 1 playthrough matches the reference (the same triggers, enemies and weapon effects); maps 2-13
  load and replay-match.
- Native Windows 11, original resolution at 4:3, **no dgVoodoo2**, no multiplayer.

## 12. How progress is reported

Only from `re_status.py`: phase, functions translated per tag, gaps open, tripwires. Every chat
report carries the same tags as the ledger. A screenshot is never offered as proof, and a feeling
is never reported as a result.
