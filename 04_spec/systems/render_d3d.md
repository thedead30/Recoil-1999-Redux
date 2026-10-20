

---

# LOD distance selection - `Render_ProcessLodDistanceNode` (`0x0044b8c0`)
`CONFIRMED-BINARY` + `VERIFIED-ORACLE`, trace `Recoil18`. Two fast-sqrt sites,
`0x0044bab1` and `0x0044bc3f`.

## Where the distance comes from
```asm
shl  ecx,4                          ; cameraIndex * 16
mov  eax,[0053990c + ecx]           ; the per-camera SQUARED distance
sar  eax,1
add  eax,1FC00000h                  ; fastSqrt -> linear distance
mov  [ebp-0Ch],eax
mov  edx,[edi+8]                    ; the node's LOD threshold
```

| symbol | meaning |
|---|---|
| `DAT_0053990c` | table of **squared** view distances, stride **16**, indexed by camera |
| `DAT_004ddd30` | current camera index (read live as **0**) |
| `node[+0x08]` | LOD distance threshold |
| `node[+0x38]` | pointer to the LOD record; `[1]` and `[3]` are a distance band |
| `node[+0x24]` bit `0x4` | gates processing; bit `0x80000` gates the bounding-sphere rebuild |

## Live values
| | |
|---|---|
| camera index | **0** |
| squared distance `DAT_0053990c` | **8896.78** |
| distance after fast sqrt | **98.7531** |
| true `sqrt(8896.78)` | **94.3227** |
| **error** | **+4.4303 units, +4.70%** |
| node LOD threshold `node[+0x08]` | **128.0** |

**This is the site where the approximation matters most.** A 4.4-unit error on a 94-unit
distance, measured against thresholds of order 128, means **LOD levels switch about 5% further
out than exact maths would put them**. A remake using `sqrtf` would pop models at visibly
different ranges.

It also forced a correction to `04_spec/systems/math_approximations.md`: the worst-case error
is **+6.06%**, not the `+3.06%` originally recorded from a sparse sample.

## There is an LOD CROSS-FADE
The node does not simply switch levels. A fade factor is computed, defaulting to `1.0`:

```asm
fld  [edi+20h]
fcomp [004d2344]              ; 0.01
jne  skip                     ; -> fade = 1.0
fld  [edi+14h] ; fld [ebp-4]  ; threshold
fsub st,st(1)
fld  [ebp-8]                  ; distance
fcomp st(1)
jne  skip                     ; -> fade = 1.0
fsub [ebp-8]
fld  [edi+2Ch] ; fsub [004d2338]   ; 1.0
fmulp st(1),st
fdiv st,st(1)
fsubr [004d2338]              ; 1.0 - ...
fstp [ebp-20h]                ; the fade factor
```

| constant | value |
|---|---|
| `0x004d2344` | **0.01** |
| `0x004d2338` | **1.0** |

So the code **can** blend between detail levels across a band rather than popping, with `0.01`
as the gate and `1.0` as the default. **But see the correction below: this path never executes
in any trace.**

**The exact fade curve is only partially read** - the disassembly above is transcribed but the
roles of `node[+0x14]`, `node[+0x20]` and `node[+0x2c]` are not established. Recorded as
structure, not as a formula to implement.

## Status
`0x0044b8c0` stays **PRIOR-EVIDENCE** -> **DECOMPILED**: the distance path and the fade's
existence are read and verified; the fade curve and the second sqrt site (`0x0044bc3f`) are not.


---

# CORRECTION: the LOD cross-fade EXISTS but never runs
`CONFIRMED-BINARY` + `VERIFIED-ORACLE`, traces `Recoil18` and `Recoil21`.

The previous section said models "**blend** between detail levels rather than popping". That
overstated what the evidence supports. Probing the three code paths in
`Render_ProcessLodDistanceNode` across **two** traces:

| site | what it does | `Recoil18` | `Recoil21` |
|---|---|---|---|
| `0x0044bab8` | the distance computation (first fast sqrt) | **HIT** | **HIT** |
| `0x0044bb52` | the **cross-fade** block | no hit | no hit |
| `0x0044bc2d` | the **radius clamp** (second fast sqrt) | no hit | no hit |

**Only the distance path executes.** The fade and the clamp are both gated on a comparison
against `0x004d2344` = **0.01**, and that gate is not passed by any object in either recording.

## The corrected statement
The cross-fade **machinery exists in the binary** and a remake should implement it, because
other content may enable it. But **map 1 as recorded uses hard LOD switching** - no blending
was observed. Do not describe fading as the game's behaviour; describe it as an available path.

## The radius clamp - `CONFIRMED-BINARY`, unexercised
```asm
mov  eax,[edi+34h]
mov  ecx,[edi+4]              ; a SQUARED radius
sar  eax,1                    ; (on [ebp-3Ch] = ecx)
add  eax,1FC00000h            ; fastSqrt
mov  [ebp-4],eax
fld  [ebp-8]                  ; the view distance
fcomp [ebp-4]
je   skip
mov  edx,[ebp-4]
mov  [ebp-8],edx              ; distance = max(distance, sqrt(lod[+4]))
```

So `lod[+0x04]` is a **squared radius** and the view distance is clamped to be at least that
radius - LOD never treats the camera as closer than the object's own size. Sensible, and
`CONFIRMED-BINARY` from the disassembly, but **never observed executing**.

## EDI is the LOD RECORD, not the scene node
Worth stating because it resolves an apparent contradiction. In this function `ECX` is the
scene node and `EDI` is the LOD record reached through `node[+0x38]`. So `node[+0x38]` being a
pointer and `lod[+0x38]` being a float are **different fields of different structures**, not a
conflict.

Fields of the LOD record seen so far, all from the disassembly:

| offset | use |
|---|---|
| `+0x04` | squared radius, for the minimum-distance clamp |
| `+0x08` | LOD distance threshold (read live as **128.0**) |
| `+0x14`, `+0x20`, `+0x2c` | first fade block parameters |
| `+0x34`, `+0x38` | second fade block parameters |

**No live values for these** beyond `+0x08` - the blocks that read them never execute.

## A near-miss worth recording
The first attempt to read these fields set a breakpoint at `0x0044bc2d` and dumped
`EDI`-relative floats. The values came back as `7.65e-29`, `5.01e-24` and similar. They looked
like data and could easily have been written down as field values.

They were garbage: **`eip` read `0x0048f993`, not the breakpoint address** - the breakpoint had
never hit, and `EDI` held an unrelated value. The standing rule (print the position **and check
`eip`**) is what caught it.

**Nonsensical float magnitudes - `1e-24`, `1e-29` - are a reliable tell that a pointer is
wrong.** Real engine floats sit in human ranges.

---

# CORRECTION — `Render_InstallDrawDispatchTable` (`0x004a77a0`) never executes in any trace
Its ledger row carried `VERIFIED-ORACLE`. Measured execution count at the entry byte:

| trace | count |
|---|---|
| `Recoil16` | **0** |
| `Recoil18` | **0** |
| `Recoil21` | **0** |
| `Recoil22` | **0** |

Nothing about it was ever checked against the running binary, so the verification tag is
**unearned and removed**. The body is read, so the reading stands as `CONFIRMED-BINARY`.

This is almost certainly benign in cause — it installs a dispatch table at start-up, and every
trace attaches to an already-running game, so init code falls outside the recorded window. **The
cause does not rescue the tag:** an oracle claim requires the code to have executed.

**Consequence for the remake:** the dispatch-table contents cannot be confirmed against a live
run from these traces. They are read from the binary only. To close this properly a trace must be
recorded from process start rather than by attaching.
