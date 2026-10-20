# The transform stack

`CONFIRMED-BINARY` (all five primitives read in full from their own bodies) +
`VERIFIED-ORACLE` (pointer arithmetic measured in trace `Recoil18`).

Two globals hold the stack. Neither is a pointer to a value — **both are stack pointers**
walking parallel arrays, one slot (4 bytes) per nesting level:

| global | array |
|---|---|
| `PTR_DAT_004e0e88` | **matrix pointers** — the transform at each level |
| `PTR_DAT_004e0e84` | **identity flags** — one per level |

So `*PTR_DAT_004e0e84` asks *"is the transform at the **current** level the identity"*.

A matrix is **12 floats** — a 4x3, three basis triples plus a translation. Confirmed by the
identity written in `FUN_004732f0` (`1.0` at indices 0, 4, 8) and by the 12-iteration copy loops
in `zTransformStackPushAndCopy` and `zTransformConcatenateLocal`.

## The five primitives

| address | name | callers | effect on the flag |
|---|---|---|---|
| `0x00472f30` | `zTransformStackPushRef` | 40 | push; matrix := `ECX`; flag := **0** |
| `0x00472ef0` | `zTransformStackPushAndCopy` | 21 | push; matrix := `ECX`; **inherits** parent's flag; copies parent's 12 floats in |
| `0x00472f60` | `zTransformStackPop` | 55 | pop; **writes nothing** |
| `0x004732f0` | `FUN_004732f0` (identity) | 25 | writes identity into current matrix; flag := **1** |
| `0x00474010` | `FUN_00474010` (local TRS - see correction below) | 9 | concatenates translate-rotate-scale; flag := **0** |

```c
/* 0x00472f30 */                          /* 0x00472f60 */
PTR_DAT_004e0e84 += 4;                    PTR_DAT_004e0e84 -= 4;
PTR_DAT_004e0e88 += 4;                    PTR_DAT_004e0e88 -= 4;
*PTR_DAT_004e0e88 = ECX;
*PTR_DAT_004e0e84 = 0;
```

**Push-by-reference clears the flag** because the caller supplies a matrix whose contents are
unknown — conservatively "not identity". **Push-and-copy inherits it** because the new level's
contents are byte-identical to the parent's, so the parent's answer is still correct. Both are
right, and the difference between them is the tell that the flag genuinely tracks matrix content
rather than nesting depth.

## What the flag is for
`zTransformConcatenateLocal` (`0x00473370`, 792 bytes, 15 callers):

```c
if (*PTR_DAT_004e0e84 != 0) {                 /* current level IS identity */
    dst = *PTR_DAT_004e0e88;
    for (i = 12; i != 0; i--) *dst++ = *src++; /* straight copy, no multiply */
}
else { ... full 4x3 concatenation ... }
```

Concatenating into an identity is a copy, so the flag exists to **skip a matrix multiply**. The
same optimisation appears in `FUN_00452ec0`. **One flag, two consumers, both purely
performance** — it is not a mode or a feature toggle, and a remake may ignore it entirely
without changing behaviour, only cost.

## Measurement
One pop in `Recoil18`, `C127:912` -> `C127:920`:

| | `PTR_DAT_004e0e84` | `PTR_DAT_004e0e88` | flag at the slot |
|---|---|---|---|
| before (`eip = 0x00472f60`) | `0x00566954` | `0x0056686c` | **0** |
| after | `0x00566950` | `0x00566868` | **0** |

Both pointers decrement by exactly 4, confirming the stack-pointer reading. The flag reads `0`
at **both** slots — the caller's level was not identity either at this call site.

## This retires an open question, by ruling the candidate OUT
An earlier section recorded that the flag measured `1` at `FUN_00452ec0`'s *return* while the
multiply branch demonstrably ran, and named `zTransformStackPop()` as "the obvious candidate"
for restoring it, explicitly not measured.

**`zTransformStackPop` cannot restore anything: its 34 bytes contain no store.** Nothing is
written or restored — the pop moves the pointer to the caller's slot, and the read then lands on
a **different array element** whose value has its own unrelated history.

`FUN_00452ec0`'s shape makes this concrete:
```c
zTransformStackPushRef();   /* new level, flag := 0        */
FUN_004732f0();             /* identity into it, flag := 1 */
/* ... concatenate the node chain; FUN_00474010 sets flag := 0 ... */
zTransformStackPop();       /* back to the CALLER's slot   */
/* return  <-- the `1` was read HERE */
```
The `1` sampled at the return was never the same storage as the `0` at the branch. There was no
contradiction to reconcile, and the passes spent reasoning about which branch ran were spent on
a question that did not exist.

**The lesson is the standing rule, again:** the flag's *value* was sampled repeatedly and
explained nothing; reading the *pointer* — and the 34 bytes of the function said to restore it —
settled it immediately.

## Status
Read in full: all five primitives above. Measured: the pop's pointer arithmetic and both slots.
**Not read:** the full concatenation branch of `zTransformConcatenateLocal` — only its
identity-copy branch is established.

---

# `zTransformConcatenateLocal` (`0x00473370`) — read in full

`CONFIRMED-BINARY` (all three paths read from disassembly) + `VERIFIED-ORACLE` (general path
hits in `Recoil18` at `C2F0:11CB` and `C2F4:1E9`).

## It takes TWO register arguments, not one
Ghidra's prototype was empty, so the packet rendered a bare `in_EDX` with no origin. It is a
real second argument, proven at the call sites:

```asm
0044b475  MOV EDX, 0x3        ; Render_ProcessObject3DNode
0044b47a  LEA ECX, [EDI+0x30]
0044b47d  CALL 0x00473370
```

Across the 23 call sites: `EDX = 3` at four (render, collision), `EDX = 1` at one, and
`MOV EDX, <reg>` at nine more — the mode is **threaded down the node walk** from a caller
(`FUN_00444890` takes it in `EDX` at `0x0044489e` and forwards it). So:

```c
void __fastcall zTransformConcatenateLocal(const float *src /*ECX*/, int mode /*EDX*/);
```

## Three paths

| condition | behaviour | floats written |
|---|---|---|
| identity flag != 0 | `REP MOVSD` 12 dwords from `src` — copy, no multiply | 12 |
| `mode == 2` | rotation only: `out3x3 = src3x3 · cur3x3` | **9** |
| otherwise (1, 3, …) | full affine concatenation | 12 |

All three end at the shared tail (`0x00473653`), which copies **12** dwords from the temp buffer
into the current level's matrix and clears the identity flag.

The general path computes, with `cur = *PTR_DAT_004e0e88`:
```
out[0..8]   = src3x3 · cur3x3
out[9..11]  = src_translation · cur3x3 + cur_translation
```
i.e. **`result = src ∘ current`** — the incoming transform applied *within* the current one,
which is what "Local" names. Read from `FLD [EAX+0x24/0x28/0x2c]` (src translation) against
`FADD [EBX+0x24/0x28/0x2c]` (current translation) at `0x004735e7`–`0x00473650`.

Note `mode == 2` is an **exact equality** (`CMP EDX,0x2`), not a bit test — `3` does *not* take
the rotation-only path. Whatever the enum means, `2` is a distinct value, not a flag bit.

## LATENT DEFECT: `mode == 2` copies three uninitialised floats
`INFERRED` as to consequence; the mechanism itself is `CONFIRMED-BINARY`.

The frame is `SUB ESP,0x3c` — a 48-byte temp buffer at `EBP-0x3c` plus three 4-byte scratch
slots at `EBP-0xc/-0x8/-0x4`, an exact fit with no slack.

In the `mode == 2` branch the stores run `FSTP [ECX]` … `FSTP [ECX+0x20]` — **nine**, ending at
`0x004734cb` — then `JMP 0x00473653`. Buffer offsets `0x24`, `0x28`, `0x2c` (`EBP-0x18`,
`EBP-0x14`, `EBP-0x10`) are **never written**, and the shared tail copies all twelve. On a fresh
frame those three slots hold whatever the previously-called function left on the stack, so the
current level's **translation would be overwritten with stale stack data**.

**Not executed in either trace checked.** Breakpoint at `0x004733b5` (the branch entry) never
hit in `Recoil18` or `Recoil21` — `g` ran to end-of-trace both times, `eip` checked (`0x0048f8fd`
/ `0x0048f8f4`), per the standing rule. Both constant-mode call sites use `1` or `3`.

**What this does NOT establish:** whether `mode == 2` is reachable at all. Nine call sites pass a
runtime register, and the field feeding it has not been traced to its source. A no-hit in two
traces is provisional, never a negative.

**For the remake:** implement `mode == 2` as rotation-only with the translation *preserved or
zeroed deliberately* — do not reproduce the uninitialised read. Flag it if a mode-2 call is ever
observed, because the original's output there is not reproducible by construction: it depends on
unrelated stack residue.

## Status
All three paths read. General path oracle-verified. `mode == 2` read but **never observed
executing**; the meaning of the mode enum's values is **not** established — only that `2` is
special-cased and `1` and `3` are not.

---

# The `mode` argument — measured, and mode 2 is still never seen

`VERIFIED-ORACLE`, traces `Recoil18` and `Recoil21`.

## Observed values: only 1 and 3
`EDX` sampled at `0x00473370` entry, `eip` checked on every sample:

| sample set | 1 | 3 | 2 |
|---|---|---|---|
| `Recoil18`, from trace start (40) | 24 | 16 | **0** |
| `Recoil18`, from `C400:0` (25) | 14 | 11 | **0** |
| `Recoil21`, from trace start (25) | 18 | 7 | **0** |

**90 calls, two traces, two trace positions. Mode 2 never appears.**

## Only three call sites are live
Read from `[esp]` at function entry — the return address identifies the caller directly:

| return addr | actual call site | mode | where the value comes from |
|---|---|---|---|
| `0x00477d8d` (16) | `JMP` at `0x00473202`, in `FUN_004731f0` | 1 | `MOV EDX,1` at `0x004731fa`, hardcoded |
| `0x00474242` (7) | `0x0047423d`, in `FUN_00474010` | 1 | measured; not a local constant |
| `0x0044b482` (7) | `0x0044b47d`, in `Render_ProcessObject3DNode` | 3 | `MOV EDX,3` at `0x0044b475`, hardcoded |

Twenty of the twenty-three call sites do not execute in `Recoil18`. Checked directly:
a breakpoint at `0x00444961` — one of the nine sites passing a runtime register — **never hits**;
`g` ran to end-of-trace (`eip = 0x0048f8fd`), so its `EDX` was never sampled.

`FUN_004731f0` is worth naming while we are here — six instructions:
```asm
004731f0  CALL 0x00472f90
004731f5  MOV EAX, [0x004e0e88]     ; matrix stack pointer
004731fa  MOV EDX, 1                ; mode
004731ff  MOV ECX, [EAX-4]          ; the PARENT level's matrix
00473202  JMP  0x00473370           ; tail call
```
i.e. *concatenate the parent level's matrix into the current level, mode 1*.

## What the modes mean is NOT established
Only two facts constrain the enum, both `CONFIRMED-BINARY`:
- `CMP EDX,2` in `zTransformConcatenateLocal` — an **exact** test, so `3` does not take the
  rotation-only path;
- `if (1 < in_EDX)` in the node walker `FUN_00444890` (cases 5 and 6) gates a call to
  `FUN_004473e0()` — an **ordered** threshold.

So the enum is ordered with `2` sitting between two live values, and mode 2 would take *both*
the extra `FUN_004473e0` check and the defective rotation-only path. **No naming of 1, 2 or 3 is
justified by this evidence.**

## Mode 2's reachability: still open, but better bounded
Both hardcoded sources on live paths are `1` and `3`. The nine sites that pass a runtime register
are not exercised in `Recoil18`. That is not a proof of unreachability — it is a statement about
two traces of one mission — but nothing observed so far can reach the defective path.

## Gotcha: Ghidra reports tail `JMP` as `UNCONDITIONAL_CALL`
`0x00473202` is a `JMP`, not a `CALL`, yet `get_xrefs_to` lists it as `UNCONDITIONAL_CALL`. The
return address on entry is therefore the **tail-caller's** caller (`0x00477d8d`), not
`0x00473207`. Return-address attribution must be reconciled against the disassembly, or a tail
call will be mistaken for a missing xref — which is exactly what happened here before the
`JMP` was read.

---

# CORRECTION (2026-09-15) — call-site liveness and mode distribution were prefix-only
The section *"The `mode` argument — measured"* above states that only three call sites
are live and that 90 sampled calls showed only modes 1 and 3. Those samples were all
taken before `g` halted on a first-chance guard-page fault ~12% into `Recoil18`, so they
describe the trace prefix, not the trace.

Trace-wide execution counts (`TTD.Memory(a, a+1, "e").Count()`, which cannot halt) show
**eleven live call sites**; the full table is in `03_re/LOOP.md` under 2026-09-15.
`0x0044b47d` alone executes 2881 times in `Recoil18`.

What survives: **the mode-2 branch `0x004733b5` executes 0 times trace-wide in both
`Recoil18` and `Recoil21`**, so the latent defect is not reached in either recording. What
does not survive: "only modes 1 and 3 occur" at the variable-register call sites, which
are now known to be live but whose `EDX` values were never sampled past the prefix.

---

# The rest of the stack API (2026-09-15)
All `CONFIRMED`. Bytes read for every entry; oracle evidence from `Recoil18` where stated.

| addr | behaviour | evidence |
|---|---|---|
| `0x00473250` | **load**: copy 12 floats from `ECX` into the current matrix; identity flag := 0 | `VERIFIED-ORACLE`: copy measured `C127:8C4`->`8DA`; its instructions write the flag array 14491 times (= its call count), 501 of them 1 -> 0 |
| `0x00473210` | **store**: copy the current matrix out to `param_1` | `VERIFIED-ORACLE` `1098B:28E` |
| `0x00473230` | return the current matrix pointer | `VERIFIED-ORACLE` `C127:BFA` |
| `0x00473240` | return the current identity flag | `VERIFIED-ORACLE` `C127:DBD` |
| `0x00472f90` | `mov ecx,0x005668e8; jmp load` - current := fixed matrix **A** | bytes |
| `0x00472fa0` | `mov ecx,0x00566920; jmp load` - current := fixed matrix **B** | bytes |
| `0x004731f0` | current := **A** (via `0x00472f90`), then concatenate the parent level's matrix into it, mode 1 | bytes; 7888/847 calls = the `0x00473202` call-site count |

**Trap, seen here twice:** Ghidra renders both loaders as an argument-less `FUN_00473250()`
and hides the constant loaded into `ECX`. Reading the bytes is the only reliable way to know
what such a wrapper loads.

## The fixed matrix pair A (`0x005668e8`) and B (`0x00566920`)
`CONFIRMED-BINARY` for the writer, `MEASURED` for the relationship (`Recoil18` snapshot).

- **Both are written only by `FUN_00473e60`** - every write to either range in `Recoil18`
  comes from an instruction inside it.
- **A is the exact inverse of B.** Rotation parts are transposes (max difference 0); B's
  rotation is orthonormal (6.6e-8); A's translation equals `-t_B . R_B^T` to 0.000.
- Snapshot: B translation (2190.88, 5.15, 1471.13); A translation (2128.92, 129.65, -1554.06).
- Convention implied (matches the concatenator): row vectors, `p' = p.R + t`, rows at indices
  0-2 / 3-5 / 6-8, translation at 9-11.

**B is the camera object's world matrix, axis-converted; A is the view matrix.** `VERIFIED-ORACLE`,
`Recoil18`, chain measured end to end:
1. `0x0044abf0` (attributed to `Camera.c`) builds a world transform: identity (`0x004732f0`), then
   the parent-chain walker `0x00449480`, then reads the current matrix.
2. It copies that matrix into its object at `EDX+0x44`. Measured at its entry (`C2F0:A5A`):
   `EDX = 0x08726b08`, so `EDX+0x44 = 0x08726b4c`.
3. It calls `FUN_00473e60` with `ECX = 0x08726b4c` (return address `0x0044acae`, `C2F0:C8C`).
   That buffer is written 1284 times in the trace, **every write by the one instruction
   `0x0044ac92`** - the copy loop in step 2.
4. `FUN_00473e60` writes **B = the input with every element of rows 1 and 2 sign-flipped**; row 0
   and the translation are copied unchanged (input vs output compared at `C2F0:C8C`->`D0B`).
   That is a fixed Y/Z axis-convention conversion between node space and camera space -
   **a remake must reproduce it exactly.** It then writes **A = inverse of B**.

Whether the object at `0x08726b08` is *the* active camera or one of several camera objects is not
established; the function is in `Camera.c` and runs once per frame-ish (1284 / 12 = 107 builds).

---

# `FUN_00472fb0` — yaw billboard over the parent (`CONFIRMED`, `VERIFIED-ORACLE`)
Bytes read; both callers are the object-render functions (`0x00476cf0`, `0x00477b30`).
2788 calls `Recoil18` / 326 `Recoil21`.

```
yaw  = (parent level not identity) ? atan2(P[6], P[8]) : 0     ; P = parent matrix, via 0x00474de0
cur  = identity ; cur = RotY(param_1 - yaw) . cur               ; 0x004732f0, 0x00473b10
cur  = P . cur            (concat, EDX = 1)
cur.t = P.t                                                     ; parent position kept verbatim
push_ref(local) ; local = A (view) ; local = cur . A ; pop ; cur = local
```
So the result has the parent's **position**, an **absolute yaw of `param_1`** (the parent's own
yaw is cancelled), and is pre-multiplied into view space.

**Oracle check** (`Recoil18` `C596:2518`, `C59D:5B8`): recomputing the output from the inputs
with these formulas matches the measured matrix to **1.4e-6** in both samples; with the
`atan2` arguments swapped it is off by 1.0, which fixes the argument order as `atan2(m6, m8)`.

**Limit, stated plainly:** in both samples the parent rotation was the identity and the yaw 0,
so the order of the `P . cur` step and behaviour with a nonzero parent yaw are *not* exercised.
Decompiler artifacts here: the identity call shows an argument it does not take; the loads hide
their `ECX` constants.

`Transform_RotateYLocal` identity-flag branch writes `m0=c, m2=-s, m6=s, m8=c` - confirmed by
the same check. Its general branch is verified separately.

---

# `Transform_RotateYLocal` (`0x00473b10`), `FUN_00474de0`, `FUN_00473060` (2026-09-15)

**`Transform_RotateYLocal(theta)`** - `CONFIRMED`, `VERIFIED-ORACLE`. Row-vector convention,
`M := R_y . M`:
- identity flag set: writes `m0 = c, m2 = -s, m6 = s, m8 = c` into the identity (checked through
  `FUN_00472fb0`'s oracle test);
- flag clear: `row0 := c.row0 - s.row2`, `row2 := s.row0 + c.row2`; row 1 and translation
  unchanged; flag := 0. `Recoil18` `C2FE:21D2`, `CC9E:298`: error 8.5e-9 and 2.3e-8; the opposite
  sign convention misses by 0.14 and 0.25. **Limit:** input rotation was the identity in both
  general-branch samples, so mixing of non-trivial rows is inferred from the body.
- `RotateXLocal` / `RotateZLocal` (`0x00473970`, `0x00473cc0`) have the same shape; not yet checked.

**`FUN_00474de0`** (member of `math3d`) - `CONFIRMED`, `VERIFIED-ORACLE`: returns
`atan2(m[6], m[8])` of the matrix in `ECX` - the heading of row 2 - or `0.0` if both are 0.
Three `Recoil18` calls (`C2F0:BD0`, `CC8A:543`, `D82E:4FB`) match exactly.

`INFERRED`: the yaw passed to `FUN_00472fb0` is the **camera's heading** - the `param_1` seen
(-3.071819) equals this function's result on the camera matrix at the same moment - so
`FUN_00472fb0` is a **yaw-only billboard facing the camera**. Alternative not excluded: another
caller-supplied yaw that happened to coincide in the samples taken.

**`FUN_00473060`** - `CONFIRMED` from bytes only (0 calls in `Recoil18` and `Recoil21`). The
**full-orientation** sibling of `FUN_00472fb0`:
1. parent angles: `0x00474e10(ECX = parent matrix)` if the parent is not identity, else the three
   floats at `0x005669d8`-`0x005669e0` (0, 0, 0 throughout `Recoil18`, never written in-trace);
2. current := B (camera world, via `0x00472fa0`), then rows 1 and 2 sign-flipped back;
3. camera angles: `0x00474e10(ECX = current matrix)`;
4. each angle triple through `0x004757c0`, the two results combined by `0x004759d0`, converted to
   a matrix by `0x00475a80` (`INFERRED`: Euler -> quaternion, quaternion product,
   quaternion -> matrix; those four are `math3d` members, not yet confirmed);
5. then exactly as `FUN_00472fb0`: concat with the parent, parent translation, times view A.

---

# Initial state, from the executable file (`CONFIRMED-DATA`, `Recoil.exe` `.data`)
| address | initial value | meaning |
|---|---|---|
| `0x004e0e84` | `0x00566950` | identity-flag stack pointer - **base of the flag stack** |
| `0x004e0e88` | `0x00566868` | matrix-pointer stack pointer - **base of the pointer stack** |
| `0x004e0e8c` | `0x00000001` | adjacent word; role not established |

Both stacks grow upward by 4 bytes per push, so depth = `(pointer - base) / 4`. The slots measured
earlier (`0x00566954`, `0x0056686c`) are depth 1.

`0x005668e8` (view A), `0x00566920` (camera B) and the default angles `0x005669d8`-`0x005669e0`
all lie in the part of `.data` with no file backing (raw size `0xbc00` of virtual `0x29fac0`), so
they are **zero at load**. The default angles are never written in `Recoil18`, so they are
(0, 0, 0) - unless something writes them before a trace attaches, which no current trace can show.

---

# Point and vector transforms (2026-09-15)
All operate on arrays of 3-float points, `ECX` = array, count in `EDX` or on the stack.

| addr | operation | identity level | evidence |
|---|---|---|---|
| `0x00474710` | `EDX[i] = ECX[i] . R` - rotate, no translation, to a separate array; count = `param_1` | copies through | `VERIFIED-ORACLE` 2.8e-8 |
| `0x004747d0` | `p := p . R + t` in place; count = `EDX` | no-op | `VERIFIED-ORACLE` 3.9e-5 (float32 rounding at ~2000 units) |
| `0x00474670` | `v := R . v` in place - **the transposed rotation**, i.e. the inverse of a pure rotation; count = `EDX` | no-op | `VERIFIED-ORACLE` (Recoil21) 1.2e-8 |

The test for each discriminates the convention: the transposed (or untransposed) alternative
misses by 0.1 to 1.7.

## `FUN_00474c20` - screen point to world (mouse-cursor raycast)
Callers: two sites in `Input_Mouse_CursorRaycastDispatch` and one in `FUN_0049aa30`.
`FUN_00474bc0` computes, per point, `(x - cx)*kx/z, (y - cy)*ky/z, 1/z` from globals
`DAT_005761e0/e4` and `DAT_00566860/64`; `FUN_00474c20` then pushes a level, loads camera
matrix B and transforms the result into world space. Oracle check pending.

**Latent defect, NOT triggered in the shipped game.** `FUN_00474c20` passes `FUN_00474bc0` a
stack buffer of **12 bytes** (`ebp-0x14`..`ebp-0x9`, room for one point) but lets it write
`param_1` points. Every caller pushes a literal **1** (`0x004114a1`, `0x004114be`, `0x0049aa37`),
and a breakpoint for any other count never fired in `Recoil18`. **Remake:** fix the count at 1,
or size the buffer to the count - never reproduce the undersized buffer with a general count.

---

# Raycast verified; quaternion helpers (2026-09-15)
**`FUN_00474c20` / `FUN_00474bc0`** - `VERIFIED-ORACLE`. For screen point `(x, y, z)`:
`w = 1/z`, `v = ((x - cx)*kx*w, (y - cy)*ky*w, w)`, `world = v . R_B + t_B`. Two `Recoil18`
calls match to 1.1e-4 and 4.8e-5 (float32 rounding at ~2000-3000 units); multiplying by `z`
instead of dividing misses by 996.

Measured globals: `cx, cy` = `DAT_005761e0/e4` = **(320, 200)** - the centre of a 640 x 400
screen; `kx, ky` = `DAT_00566860/64` = **0.00227045, 0.00207107**. So `ky*200` = **0.41421** and
`kx*320` = **0.72654**. `INFERRED`: 0.41421 is tan 22.5 degrees to five digits, i.e. a **45 degree
vertical field of view**; 0.72654 is tan 36 degrees. Alternative not excluded: the values derive
from other quantities that merely produce these tangents.

**`FUN_004757c0`** - Euler to quaternion from half angles (`CONFIRMED` from body; never runs).
**`FUN_00475a80`** (`math3d`) - quaternion `(w, x, y, z)` to the 3 x 3 rotation, the standard
formula in row-vector form, translation not written (`CONFIRMED` from body; never runs).

---

# `FUN_00474e10` - Euler extraction (`math3d`, `CONFIRMED`, `VERIFIED-ORACLE`)
Read from bytes; the decompiler hid every register argument. Output to `EDX` as
`(pitch, yaw, roll)`:
```
yaw   = atan2(m6, m8)                               ; 0x00474de0
pitch = atan2(-m7, sqrt(m6^2 + m8^2))
A     = rotateY(row0, -yaw)                         ; 0x00474f40: x' = c.x + s.z, z' = c.z - s.x
B     = rotateX(A, -pitch)                          ; 0x00474ec0: y' = c.y - s.z, z' = c.z + s.y
roll  = atan2(B.y, sqrt(B.x^2 + B.z^2))
if m4 < 0.0: roll = pi - roll                       ; constants 0x004d2960 = 0.0, 0x004d2998 = 3.1415927
```
Two `Recoil18` calls match to 5.8e-9 and 6.1e-9. **Limit:** roll is 0 in every observed call (123
across `Recoil18` and `Recoil21`; a breakpoint for any matrix with `m1 != 0` never fired), so
the roll path and its quadrant correction rest on the body and the file constants.

**Update:** the field of view is now measured directly from the projection setup's parameters -
72 degrees horizontal by 45 degrees vertical - see `04_spec/systems/view.md`. The `INFERRED`
note above is superseded.


---

# CORRECTION (2026-09-15) — `FUN_00474010` is a translate-rotate-scale, not a bare rotation
Earlier in this file it is described as "Euler rotation" taking its angles in `ECX`. That was
**incomplete**. The function ends `ret 4` and reads a stack argument, and it uses `EDX`; Ghidra
renders it as `void(void)`, which hid both. Re-read from the bytes and re-verified:

```
FUN_00474010(ECX = angles[3] (radians), EDX = translation[3], [esp+4] = &scale[3])
local.R  = Rz(angles[2]) . Rx(angles[0]) . Ry(angles[1])        // same order as math3d 0x00474260
local.row[k] *= scale[k]        for each k with scale[k] != 1.0  // S.R: ROWS are scaled
local.T[k]    = translation[k]  for each k with translation[k] != 0.0 (else 0; so -0.0 -> +0.0)
zTransformConcatenateLocal(&local, mode 1);  identity flag := 0
```

| check (`VERIFIED-ORACLE`) | result |
|---|---|
| rotation order, 3 `Recoil18` calls | Rz.Rx.Ry matches to ~4e-8; the other orders miss by ~0.03 |
| translation | copied exactly into the local matrix |
| scale (0.75, 0.75, 4.25), `Recoil18` | rows scaled matches; columns scaled rejected |
| scale (2, 2, 2), `Recoil21` | rows scaled matches |

The earlier measurements (angles in radians, flag 1 -> 0) still hold. The row was demoted, which
closed the transform gate, then re-confirmed on this evidence.

## Function index additions (2026-09-24)
- **Billboards.** `FUN_00472fb0` `0x00472fb0` is a yaw billboard: `RotY(a - atan2(P[6], P[8]))`
  with the parent's translation, times the view matrix. `FUN_00473060` `0x00473060` is a
  full-orientation billboard.
- **Setting the current transform:**
  - `FUN_00473280` `0x00473280` sets the current 3×3 rotation from 9 floats;
  - `FUN_00473690` `0x00473690` scales locally (on an identity level it writes the diagonal);
  - `Transform_TranslateLocal` `0x004737e0` (`VERIFIED-ORACLE`, Recoil18).
- **Vectors.** `FUN_004745e0` `0x004745e0` rotates N vectors, `v := v.R`.
- **Screen to world.** `FUN_00474bc0` `0x00474bc0` converts screen to view space:
  `w = 1/z; x' = (x-cx)*sx*w; y' = (y-cy)*sy*w`. `FUN_00474c20` `0x00474c20` does the mouse
  raycast, screen to world.

