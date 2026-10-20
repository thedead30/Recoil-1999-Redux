# Object polygon shading (`poly_shade`) — first pass (PARTIAL)

**Scope:** the per-object shaders `0x00477b30` (hardware; Recoil18 10,676 calls, Recoil21 1,173) and `0x00476cf0` (software twin; 0 hits in both traces, which are hardware-render sessions, so this is provisional), plus the callees only they call. The draw-mode slots `0x0056bc50`–`0x0056bc68` are in `render.md`.

Status: **complete — all 23 members `CONFIRMED-BINARY`, membership CLOSED, gate OPEN (2026-09-16).** Both shaders were confirmed from full byte traces (1,387 and 1,105 instructions).

**Correction notice (2026-09-16).** Four claims written about the shaders during a model switch were re-verified against the bytes and found wrong; they are fixed here and in the ledger. They were: three stack args (both shaders actually take **none** — plain `ret`); a 2D signed-area cull in the HW shader (it is **SW-only**); object-relative mesh gate offsets (they are **mesh-relative**); and installation via `0x004a77a0` (the real entry path is below).

## Confirmed helpers — CONFIRMED-BINARY

| Address | What it does |
|---|---|
| `0x004803b0` | Screen-rectangle trivial reject: returns 0 when **all** projected vertices (`0x0057c8c4`, stride 12) lie beyond one enabled edge. Flag bit 0: all x < R[1]; bit 1: all x ≥ R[4]; bit 2: all y < R[2]; bit 3: all y ≥ R[5]. R[3] is unused. |
| `0x00489920` | Distance/height fade (SW only). d = fast-sqrt(x² + z²) of the first vertex. d ≤ `[0x0057d944]` → returns 0 and writes nothing. c = (d − `[0x0057d944]`) × `[0x0057d94c]` below `[0x0057d948]`, else 1.0. Height h from `0x00473fc0`: h ≥ `[0x0057d950]` → c = 0; h > `[0x0057d954]` → c ×= (`[0x0057d950]` − h) × `[0x0057d958]`. Clamped to 0..1; returns c > 0.005. |
| `0x00481140` | Material frame cycle, advanced once per frame stamp `[0x0056bbd8]`. Picks frame `table[trunc(pos) mod count]` **before** advancing; then pos += speed × frame time `[0x0056b424]`. One-shot cycles hold on the last frame. Negative positions wrap by count × trunc(\|speed\|). **Quirk:** that wrap adds nothing when \|speed\| < 1, so the position can reach −1 or below, and the index would then be negative (derived; whether any data has negative speeds is unknown). |
| `0x00479020` | Animated point: transform (or copy), near-plane test against `[0x00576224]`, project, screen-bounds test (inclusive). In HW mode it scales the size by (short `[P+0x18]` × `[0x0057d978]` + 1), draws immediately through `[0x0056bc68]`, **and** also appends to the point queue. In SW mode it only appends. |
| `0x0049a830` | Point-queue append: 650 records of 0x14 bytes at `0x0062ea04`, count `[0x0062ea00]`; the append is silently dropped when the queue is full. z is rewritten in the caller's vertex as z × `[0x0057dac4]` + `[0x0057dac0]` (quirk: it modifies the caller's data). |
| `0x00499930` | Texture blend index: `[0x004e21fc]` = texture-manager record index × 32 + clamp(trunc(s × 0.125), 0, 31); −1 when there is no record. |
| `0x00499990` | Builds the record {0,0,0,a,b,c,0,1.0} from 3 dwords and calls `0x00499930`. |
| `0x00499a00` | `[0x004e2200]` = texture-manager record index, or −1. It is read by the fan filler `0x00495850`. |
| `0x00490330` | Stores 255.0 to `[ECX]`. |

### Per-vertex lighting — CONFIRMED-BINARY (`0x00487f10` HW, `0x00488d60` SW; bytes traced 2026-09-16)

**Light data D fields read** (offsets are byte-confirmed; the names are INFERRED):
- +0x80: position;
- +0x98: direction L;
- +0xA4: scale A4;
- +0xA8: floor A8;
- +0xAC: colour;
- +0xB8 / +0xBC / +0xC4: type flags;
- +0xCC / +0xD0: ranges;
- +0xD4: squared radius.

**Per light, per vertex:**
1. **Face term t:**
   - t = 1 when both B8 and BC are clear; otherwise t = N·L.
   - BC lights floor t at about 0.00402.
   - B8 lights use the per-vertex vector instead. That vector is normalised, except for the vertices ≥ 1 of full-range slots.
2. **Intensity:** I = clamp(A4·t + A8, A8, 1). It is computed only when t > 1/255 or A8 > 1e-5.
3. **BC lights invert it:** I = 1 − I.
4. **Non-full lights are attenuated by the falloff f = `0x004894f0`(dist):**
   - ordinary lights: I·f;
   - BC lights: ((1 − A8) − I)·(1 − f) + I, capped at 1 − A8.
5. **Sum** the result per vertex, **add** the ambient term `[0x0057d974]`, and **clamp** the total to 0..1.

**What the two paths output:**
- **HW:**
  - It writes the per-vertex sums unscaled into the weight arrays: `0x0057d0cc` for ordinary lights, `0x0057d1cc` for BC lights, `0x0057d2cc` for C4 lights.
  - It sets the polygon's HW colour and mode flags.
- **SW:**
  - It writes no per-vertex weights.
  - It reduces the polygon to one blend level, `ftol(clamp(max over vertices 1..n−1) × 254)`, using `0x0049b780`.
  - It picks blend preset F10 when two or more lights contribute. With exactly one, it picks E70 using that light's colour.

**Defects and quirks (reproduce as-is):**
- **Unwritten direction for vertex 0 (derived).** For a full-range slot, pass 1 writes no light vector. Vertex 0 of a B8 light then reads an unwritten stack slot as its direction. Whether that combination occurs in the data is not established.
- **Squared-radius compare.** The "affects" test in these passes compares the squared distance with +0xD4. `0x00487c50` compares the linear distance − r with +0xD0.
- **SW light count.** The SW path counts a light as lit using the cumulative per-vertex sums, which include earlier lights. The HW path uses each light's own contribution.
- **SW maximum.** The SW maximum ignores vertex 0.

### Lighting, fade and clipping helpers — CONFIRMED-BINARY (2026-09-16)

| Address | What it does |
|---|---|
| `0x00476a50` | Object lighting setup. It outputs a fade flag (`0x00489540` > 1/255), a lit flag, and a blended light colour: Σ light colour × weight, plus the ambient term `[0x0057d968..970]` weighted by `[0x0057d974]`, divided by the total weight. That colour is sent to blend preset F10 (`0x0049b350`), but only when two or more contributions exist. |
| `0x00487c50` | Per-object light influence. For each active light (max 64) it computes the distance to the object sphere (3D, fast sqrt). A light **affects** the object when dist − r < `[D+0xD0]` and is **full** when dist + r < `[D+0xCC]`. Weight = 1.0 if full, else linear falloff `0x004894f0`(dist − r), capped at `[D+0xA4]` + `[D+0xA8]` and clamped 0..1. Lights with `[D+0xBC]` set use view z as the distance and go to a second weight array `0x0057d1cc`. |
| `0x00489540` / `0x00489920` / `0x00489a90` | Distance/height fade: the sphere version (object), the first-vertex version (SW polygon) and the per-vertex version (HW polygon). They share the parameters `[0x0057d944..958]`. The HW version sets polygon flag bit 2 when the fades differ by more than 1/255, but never clears that bit when they are uniform. |
| `0x0047a200` / `0x0047aa80` | Near-plane clip (Sutherland–Hodgman, z = R[3]) of the gathered polygon; the second also carries UVs. The far test (flag 0x20) only rejects. Stack buffers hold 62 vertices. |
| `0x004a7330` | Pushes the HW colour through the hook `[0x006333d4]` (installed by `Render_InstallDrawDispatchTable`), but only when it changed (exact float compare). |

| `0x0047a4e0` / `0x0047af60` | Near-plane clip variants. The first carries three per-vertex float arrays (`0x0057d0cc`/`1cc`/`2cc`). The second carries UVs plus one array. 0 hits in both traces (provisional). |
| `0x0047b540` | Screen-edge clip of the projected polygon (`0x0057c8c4`): up to four Sutherland–Hodgman passes, keeping x ≥ R[1], x < R[7], y ≥ R[2] and y < R[8] (flag bits 1/2/4/8). Passes ping-pong through a 62-vertex stack buffer. |

#### The clipper family — CONFIRMED-BINARY (bytes read 2026-09-16)

There are two kinds of clipper, in eleven variants. The two kinds are:
- **Near-plane clip:** runs on the camera-space array `0x0057c5c4` against z = R[3]. It rejects a polygon lying entirely beyond R[6] (flag 0x20).
- **Screen-edge clip:** runs on the projected array `0x0057c8c4` with four passes (flag bits 1/2/4/8). It ping-pongs through stack buffers.

Both kinds keep the vertex on the edge exactly, lerp every other carried value, and return 0 when fewer than 3 vertices remain.

| Carried data | Near-plane | Screen-edge |
|---|---|---|
| position only | `0x0047a200` | `0x0047b540` |
| position + UV | `0x0047aa80` | `0x0047d3f0` |
| position + 3 arrays (`0x0057d0cc`/`1cc`/`2cc`) | `0x0047a4e0` | `0x0047bd30` |
| position + UV + 1 array (`0x0057d0cc`) | `0x0047af60` | — |
| position + UV + 3 arrays | `0x0047e900` | `0x0047efd0` |
| x/y only (third value **not written — stale**) | — | `0x0047cdc0` |
| x/y + 1 array (third value not written) | — | `0x0047dfb0` |

**Quirk (reproduce):** the x/y-only screen clippers never write the third coordinate of the vertices they output. Whatever was already in that slot survives. It is not yet established whether any consumer reads that value on the paths that use these clippers (both have 0 hits in the traces, so the evidence is provisional).

#### Software polygon queues — CONFIRMED-BINARY (2026-09-16; 0 hits in the HW-render traces, so the runtime evidence is provisional)

**Two fixed queues, each capped at 350.** When a queue is full the polygon is dropped silently; the "Not enough MAX_…" reporter is a no-op in this build.
- **Transparent queue:** count `[0x0057de7c]`, 900-byte records at `0x0057de80`.
- **Overwrite queue:** count `[0x005cb270]`, 0x48C-byte records at `0x005cb274`.

Both record types capture the current point-queue scale and offset (`[0x0057dac0]`/`[0x0057dac4]`) and `[0x0057dac8]`.

**`0x00499a20` (flat polygons):**
- **Overwrite flag set:** the polygon goes to the overwrite queue.
- **Otherwise, alpha ≥ 255:** it is drawn immediately with `0x00492000`.
- **Otherwise (alpha < 255):** it goes to the transparent queue.

**`0x00499ec0` (textured polygons):**
- **Overwrite flag set:** the polygon goes to the overwrite queue as type 2.
- **Otherwise, material byte +9 bit 2 set:** it goes to the transparent queue with alpha 255.
- **Otherwise:** it is drawn at once as a **triangle fan** through `0x00495850`, passing fan index j so the filler can reuse its gradient cache.

**The blend index for `0x00499ec0`:** a texture-manager record index × 32 plus a 5-bit level from the first vertex value × 0.125.

**Binary defect (reproduce as-is), at `0x00499f46`–`0x00499f54` in `0x00499ec0`:**
- **What the code does:** it tests `(byte[M+9] == 0) & 2`. That value is always 0.
- **Consequence:** the branch that would call the subdivided filler `0x004969d0`, or flag the polygon, can never run. Overwrite records from this function are therefore always type 2.

**`0x004896d0` (SW per-vertex fade) writes into the shared weight array:**
- It writes 255 × fade into `0x0057d0cc`, the same array `0x00487c50` uses for light weights.
- It skips vertices whose fade is 1/255 or less, so their entries keep stale values.
- Its near test is strict (<), where the HW version `0x00489a90` uses ≤.

**Clip record — field offsets from byte-confirmed readers; meanings INFERRED:**
- **+0:** flags. Bits 1/2/4/8 select screen edges, 0x10 near-plane clipping, and 0x20 the far reject.
- **+4 / +8:** x min / y min. The rejects in `0x004803b0` and the clips in `0x0047b540` both use them.
- **+0xC:** near z.
- **+0x10 / +0x14:** the x / y max for the **reject** (`0x004803b0`).
- **+0x18:** far z.
- **+0x1C / +0x20:** the x / y max for the **clip** (`0x0047b540`).

The record is at least 9 dwords. An earlier 7-dword reading was incomplete; it was corrected on 2026-09-16.

**Point-queue scale (SW):** before each animated point, the SW shader sets `[0x0057dac0]` = 0 and `[0x0057dac4]` = short `[P+0x18]` × `[0x0057d978]` + 1.0. This is the same factor the HW path applies to the projected size.

## The mode argument (bytes, HW shader)

At all three `ftol` sites (`0x00478a7e`, `0x00478b6c`, `0x00478c2b`) the value pushed to the draw slot is:
- `ftol(byte [material+0] × [0x0057d964])`, where `[0x0057d964]` is set by `RenderState_SetFade` `0x00476070`.

The rest of the call:
- ECX = the projected vertex array `0x0057c8c4`;
- EDX = word `[material+2]`;
- the other stack arguments are `[material+4]`, the vertex count, and the mode flags `[0x0057d960]` (set by `RenderState_SetMode` `0x00476080`).

## How the shaders are entered — CONFIRMED-BINARY (2026-09-16)

The two shaders are **not** installed in the draw-dispatch table. Each has exactly one reference in the whole image, and both store into the same function pointer `[0x0057d9e0]`:

- `0x00475c40` (render-state / `GfxFlags` init) stores the **software** shader `0x00476cf0` as the default.
- `0x00475e70` overwrites it with the **hardware** shader `0x00477b30`, but only when `[0x0056bbe8] != 0`; otherwise it sets `[0x0057d9c8] = 1` and leaves the SW default in place.

That pointer is then invoked as `mov ecx,esi; call dword ptr [0x0057d9e0]` by five `scene_render` node processors: `0x0044ada0`, `0x0044af60`, `0x0044b140`, `0x0044b300`, `0x0044b710`. So the renderer picks SW or HW **once, by hardware mode**, and every scene node goes through the chosen shader.

`0x004a77a0` is a different mechanism: it installs the draw-mode **slots** `0x0056bc50`–`0x74` that the shaders *call through* (see render.md).

**Also set by that init (`0x00475c40`), and read by the shaders:** `[0x0057624c]` = 4.0, the 2D polygon-area epsilon the SW shader culls with, and the screen bounds 320.0 / 200.0 / 319.0 / 199.0.

## Object and polygon layout — CONFIRMED-BINARY (offsets from the byte traces; field *meanings* INFERRED)

- **Mesh:** `[ECX+0x3c]`.
  - [0] = transform mode 0/1/2. Mode 2 is point-cloud only: vertices are projected and emitted as points, then it returns.
  - [3] = polygon count; [4] = vertex count; [5] = second vertex-array count; [7] = count of 0x4C-byte animated-point records.
  - [0xc] = polygons, 7 dwords each; [0xd] = vertices; [0xe] = second vertex array; [0xf] = animated points.
- **Polygon record:** 7 dwords (0x1C bytes). Dword 0's low byte is the vertex count; **bit 0x100 = two-sided** (back faces negate the plane instead of being culled); **bit 0x200** = per-vertex data from the second array. The **material pointer is at `[poly+0x14]`**.
- **Material flags word** (`word [material+0]`) — distinct from the polygon dword, which an earlier draft conflated: **bit 0x100 = textured**, **bit 0x400 = animated material** (advanced by `0x00481140`).
- **Mesh gate for the model transform** (all three **mesh**-relative, not object-relative): byte `[mesh+4] & 8`, float `[mesh+0x20] != 0.0`, and `[mesh+0x18] != 0`.
- **Back-face cull:** plane d = N·v0 with N = (v2−v1)×(v0−v1), tested against the epsilon `[0x004e0fc0]`. **Both** shaders do this; only the SW shader additionally culls on the 2D signed area against `[0x0057624c]`. A polygon with |d| ≤ epsilon is dropped even when two-sided.
- **Mode choice:** material-word bit 0x100 (textured) plus the results of the fade and lighting passes. HW modes 0/2 and 3/4/5 are reached; mode 1 never is (render.md).
