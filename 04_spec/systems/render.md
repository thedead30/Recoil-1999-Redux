# Render

**Status (2026-09-17):** the hardware path is specified end to end — bring-up, surfaces, texture
bindings, per-polygon mode choice, queue routing, flush order, fog and fade, sprites and flares.
Each section carries its own tag. What is still open is listed in **"Still open"** near the top;
older "NOT established" notes below have been folded into it. The software rasteriser and polygon
shaders are specified separately in `rasteriser.md` and `poly_shade.md`.

## Still open
- ~~**Lighting source link**~~ — **answered 2026-09-17 (disassembly at `0x00488c71`–`0x00488c95`):** inside `Shade_LightPolygonVertices_HW` (`0x00487f10`) the light colour globals are loaded with `HwBlend_SetColourB` (`0x004a7250`) from **the chosen light slot's data `+0xAC`** (the light colour, `[0x0057d428 + i*4] + 0xAC`), or from the default colour `0x0057d968` when no light index was selected; polygon flag bit 0 chooses this setter pair (`0x004a7250`/`0x004a73a0`) versus the other (`0x004a7220`/`0x004a7330`). A second call at `0x00488cd0` reloads it after the pass. The blend-preset builder `0x0049b5a0` also calls it with its own colour. **Light selection (disassembly `0x004881cf`–`0x00488749`):** the index starts at −1 and, while looping over the light slots in order, is overwritten with the current slot whenever that light's summed intensity for the polygon exceeds **1/255** (double at `0x004d2b30`), counting such lights. So the colour used is the **LAST qualifying light in slot order, not the strongest** — a polygon lit by a bright light and a faint later one takes the faint light's colour. Reproduce, do not "fix".
- ~~**What the gradient mode's second vertex array holds**~~ — **answered 2026-09-17:** it is the mesh's **deduplicated normal array** (`Model_AddUniqueNormal` `0x00482a10`: normals within 0.0001 are shared, 1024 max, warning above 90%). Polygon bit `0x200` means per-vertex normals, so the "gradient" draw mode is **smooth (per-vertex-normal) lighting**, and the plain lit mode is face-normal lighting. The per-vertex blend passed to slot 5 is derived from those normals.
- **Which weapon triggers the magnifier inset** — **premise corrected 2026-09-17 (decomp of `Weapon_UpdateDeployAnimState` `0x00439ba0` and `Vehicle_SetCameraViewState` `0x00405c90`):** the "7" is NOT a weapon type. `[vehicle+0x58c]` is the **camera view state** (its only writer is `Vehicle_SetCameraViewState`, which keeps the previous one in `+0x590`). View **7** attaches the camera to a node at scale 0.5 and calls `[0x004f3650]`'s method `+0x60`(1); view **8** leaves view 7 (restores the previous view, calls `+0x60`(0), `0x00476320`, restores `[0x0057da28]`). **Bytes read (2026-09-17):** `0x00410e90` ends `mov [0x0057da2c],1` (inset ON) and `0x00410ed0` writes `mov [0x0057da2c],0` at `0x00410f9b` (inset OFF). In `Weapon_UpdateDeployAnimState` (bytes `0x0043a0c7`–`0x0043a15f`): when not in view 7 it calls `0x00410ed0` (inset OFF) then requests view **7** (`mov ecx,7` at `0x0043a130`) and sets deploy state 0x80; when already in view 7 it calls `0x00410e90` (inset ON) and requests view 8 (exit), state 0x100. So the inset is switched **off while the deploy view 7 is active and back on when it ends** — the opposite of the earlier reading. The flag is also set to 1 at `0x00411c7b` and cleared at `0x004119cf`. States 2→7 are refused as a direct change, and entering anything but 8 from 7 first exits 7. **Deploy start (decomp of `0x0043c430`, 2026-09-17):** after `Weapon_FireProjectile` succeeds, a weapon whose mount flags `[+0x54]` include **0x100000** (the terrain-following projectile flag in `weapon.md`) sets deploy state 0x100 and, for the player vehicle (`[0x004f3a88]`), `[+0x5ac] = 1`; the deploy state machine then enters view 7. So view 7 is the **camera riding a player-fired 0x100000 projectile**, and the magnifier inset is hidden while it lasts. Which ZWEP entry carries flag 0x100000 is not established (the flags word is not in `weapons_table.md`). Previously open: which game action requests view 7 (callers of `0x00405c90` at `0x0041bd46`, `0x00426117`, `0x004295ab`, `0x00429711` …). `CONFIRMED-BINARY` for the state logic (decompile read).
- ~~**The two sign-flipped camera fields**~~ — **answered 2026-09-17 (disassembly `0x00453825`–`0x0045386c`):** `Camera_UpdateFromAttachedNode` (`0x00453620`) writes **three** fields `[cam+0x8C..0x94]` = −(row 2 of the node-to-world 4×3 matrix), which is the camera's **world forward direction** (the node's −Z axis). `CONFIRMED-BINARY`.
- **Whether the render-size z-buffer vs surface-size viewport mismatch** in the pixel-doubled
  presets is on a live path (map 1 was not pixel-doubled).
- ~~**DirectX version**~~ — **settled 2026-09-17:** the interface IDs queried are, byte-read from the EXE, `{B3A6F3E0-2B43-11CF-A2DE-00AA00B93356}` IDirectDraw2 (0x004d2f98), `{6AAE1EC1-662A-11D0-889D-00AA00BBB76A}` IDirect3D2 (0x004d3048), `{6C14DB81-A733-11CE-A521-0020AF0BE560}` IDirectDrawSurface (0x004d2fb8), `{DA044E00-69B2-11D0-A1D5-00AA00B8DFBB}` IDirectDrawSurface3 (0x004d2fd8) and `{93281502-8CF8-11D0-89AB-00A0C9054129}` IDirect3DTexture2 (0x004d3108). IDirect3D2 / IDirect3DTexture2 / IDirectDrawSurface3 are the **DirectX 5** interfaces, so the render-state and light-state numbers in this file use the DX5 enums — every "DX5 enum, INFERRED" qualifier above is resolved (GUID bytes CONFIRMED-DATA; the GUID-to-interface names are PLATFORM).
- The software (SW) path at runtime: it never executed in Recoil21.

## Why render was invisible to the reachable set
Every draw function is reached **only** through `Render_InstallDrawDispatchTable`
(`0x004a77a0`), which stores them as function pointers into globals. Nothing calls them
directly, so the static call graph showed them as unreachable and the first reachable-set
computation excluded the entire subsystem. See `03_re/LOG.md`, "GATE DENOMINATOR WAS WRONG".

## The draw-mode table — `CONFIRMED-BINARY`

`Render_InstallDrawDispatchTable` writes a **contiguous, ordered block** of globals:

| address | mode | function |
|---|---|---|
| `0x0056bc50` | **0** | `Render_QueuePolygon_Untextured` |
| `0x0056bc54` | **1** | `Render_QueuePolygon_UntexturedGouraud` |
| `0x0056bc58` | **2** | `Render_QueuePolygon_UntexturedLit` |
| `0x0056bc5c` | **3** | `Render_QueuePolygon_SingleTextured` |
| `0x0056bc60` | **4** | `Render_QueuePolygon_SingleTexturedLit` |
| `0x0056bc64` | **5** | `Render_QueuePolygon_SingleTexturedLitGradient` |
| `0x0056bc68` | — | `Render_DrawPolygonImmediate` |
| `0x0056bc6c` | — | `Render_FlushSortedDrawQueue` |
| `0x0056bc70` | — | `Render_FlushMainDrawQueue` |
| `0x0056bc74` | — | `Render_FlushQuadDrawQueue` |

Contiguous 4-byte slots in a fixed order means this is indexed as an array: the shader
selects a **draw mode 0–5** and calls through `(&DAT_0056bc50)[mode]`.

**Scope of the sentence above (clarified 2026-09-16):** "every draw function is reached only
through `0x004a77a0`" covers the **draw-mode slots** in this table. It does **not** cover the
object *shaders* that call through them — `0x00477b30` (HW) and `0x00476cf0` (SW) are reached
through a separate function pointer `[0x0057d9e0]`, chosen by hardware mode at init and
invoked from five `scene_render` node processors. See `poly_shade.md`, "How the shaders are
entered".

**Six draw modes, along two axes:** textured or not × flat / gouraud / lit / lit-gradient.
That is the complete set of surface treatments the engine can emit — a remake needs all six
to reproduce map 1, and picking a single "textured and lit" path would flatten distinctions
the original makes per polygon.

### The modes chain rather than duplicate
`CONFIRMED-BINARY` from the call graph:
```
UntexturedGouraud          -> Untextured
UntexturedLit              -> Untextured
SingleTexturedLit          -> SingleTextured
SingleTexturedLitGradient  -> SingleTexturedLit -> SingleTextured
```
Each richer mode computes its extra terms and then **delegates to the simpler one** for the
actual queueing. So vertex layout and queue insertion are shared; only the colour/UV setup
differs. A remake mirroring this structure gets consistency for free.

## Three flush paths
`Render_FlushMainDrawQueue`, `Render_FlushSortedDrawQueue` and `Render_FlushQuadDrawQueue`
are separate globals, so there are **three distinct queues** — a main one, a depth-sorted
one for translucent geometry (a far-to-near painter's sort) and a quad queue. Routing is
specified in "Queue routing" and the flush order in "Per-frame flush order".

## HW/SW selection
```c
DAT_006333b4 = &LAB_004a9b70;
if (DAT_00632120 != 1) DAT_006333b4 = &LAB_004a7b60;
```
A single global picks between two implementations of one slot. **Correction (2026-09-17):** the two targets are now CONFIRMED as `zVideo_FlipFrame` (`0x004a9b70`, when `[0x00632120] == 1`) and `zVideo_PresentFrame` (`0x004a7b60`) - this slot selects **flip (fullscreen) versus Blt-present**, not hardware versus software rendering. `Render_CameraFrame_HW`
(`0x44d600`) and `Render_CameraFrame_SW` (`0x44d3a0`) exist as separate functions, so the
engine has a full software fallback path. Map 1 ran the **hardware** path — see
"Map 1 ran the HARDWARE path" (VERIFIED-ORACLE).

## `Render_ProcessLightNode` (`0x0044b140`) — does NOT light anything
The render-side handler for node type 9 only clears a per-pass bit, expands the AABB,
computes a bounding sphere and frustum-culls, then recurses into children. It emits no
polygons and applies no lighting.

So **light nodes contribute nothing during the render walk**. Whatever applies lighting does
so elsewhere — most likely at shading time from a light list assembled earlier.
`Render_ProcessCameraNode` is structurally identical, consistent with both being
transform/cull-only.

**Lighting is applied at shading time, not by the node walk** (the per-vertex model is in
`poly_shade.md`). The prior record in
`99_attic/old_docs/recoil_confirmed_logic.md` describes the light node's *stored data*
(Euler angles rather than a matrix) but that is the data layout, not the shading maths.
Nothing here confirms or contradicts it.

*Partly advanced later in this file:* the global light **colour** and its source are now
confirmed (`FUN_004a7250`), but the per-vertex **shading maths** remains open.

---

## The global light colour — `FUN_004a7250`, `CONFIRMED-BINARY`

```c
_DAT_006321dc = rgb[0] * 255.0;      // R
_DAT_006321e0 = rgb[1] * 255.0;      // G
_DAT_006321e4 = rgb[2] * 255.0;      // B
```

Takes a **normalised float RGB triple** and scales it to 0–255. These three globals are read
by exactly three functions — `Render_QueuePolygon_UntexturedLit`,
`SingleTexturedLit` and `SingleTexturedLitGradient` — i.e. **all three "Lit" draw modes and
nothing else.** That confirms them as the light colour used at shading time, not a fog tint.

| value | meaning | tag |
|---|---|---|
| **255.0** | scale from normalised float to 8-bit channel | `CONFIRMED-BINARY` |

### A dominant-channel index the prior record did not note
```c
if (DAT_00632120 != 0) {                       // the same global that picks HW vs SW
    if (R < G) { if (B <= G) idx = 1; else if (B <= R) idx = 0; else idx = 2; }
    else       { if (B <= R) idx = 0;          else idx = 2; }
    DAT_00632140 = idx;                        // 0 = R, 1 = G, 2 = B
}
```
The engine computes and caches **which channel of the light colour is brightest**, in one
mode only. That is the shape of a cheap monochrome-intensity shortcut — pick the dominant
channel and shade from it rather than doing three-channel maths.

**What consumes `DAT_00632140` is not established**, so its purpose is `INFERRED`. It is
recorded because a remake doing full RGB shading in a mode where the original collapses to
one channel would produce visibly different colour on lit surfaces.

## Correction to the prior Ghidra annotation
A comment left in `Render_QueuePolygon_UntexturedLit` by the previous attempt states the
per-vertex colour is:

> `intensity[v] * lightColorConst + faceBlendFactor*255`, where `faceBlendFactor = 1 - *param_3`

Two problems with taking that as read:
1. In the current decompile the blend factor is `1.0 - *param_2`, not `param_3`. The
   parameter numbering differs, so the annotation does not match the code as it now
   decompiles.
2. The comment itself ends `"Confidence: medium-high on structure; medium on semantic
   label"` — it was never a confirmed reading.

### RECOVERED — the per-vertex colour formula. `CONFIRMED-BINARY`

```c
faceBlend = 1.0 - *param_2;                       // read ONCE per face
for each vertex v:
  if (intensity[v] > 0.003921569) {               // 1/255 threshold
      c[0] = faceBlend * base[0] + intensity[v] * lightR;   // lightR = _DAT_006321dc
      c[1] = faceBlend * base[1] + intensity[v] * lightG;
      c[2] = faceBlend * base[2] + intensity[v] * lightB;
      if (c[DAT_00632140] > 255.0) {              // if the DOMINANT channel overflows
          s = (1.0 / c[DAT_00632140]) * 255.0;
          c[1] *= s;
          c[2] *= s;
      }
      pack to ARGB;
  }
```

| value | meaning | tag |
|---|---|---|
| **0.003921569** = 1/255 | **intensity threshold** — vertices below it skip the lit path entirely | `CONFIRMED-BINARY` |
| **255.0** | overflow ceiling and rescale target | `CONFIRMED-BINARY` |

**So `DAT_00632140` is an overflow-normalisation index, not a shading shortcut.** My earlier
`INFERRED` guess that it was a "cheap monochrome-intensity shortcut" is **withdrawn** — it is
used to detect which channel will overflow first and rescale by `255 / thatChannel`.

That is a deliberate and unusual choice: **the colour is rescaled to preserve hue rather than
clamped per channel.** Per-channel clamping pushes bright colours toward white; this keeps
the ratio. A remake clamping per channel would wash out every over-bright surface.

### An asymmetry that needs disassembly before it is trusted
The rescale multiplies **`c[1]` and `c[2]` only — not `c[0]`.** With `DAT_00632140` free to
be 0, 1 or 2, that is not a consistent hue-preserving rescale.

Two readings, neither eliminated: (a) an original-game bug, in which case the remake must
reproduce it; (b) a decompiler artefact, with `c[0] *= s` folded into the `ftol()` sequence
that immediately follows. **SETTLED 2026-09-17 — reading (b), a decompiler artefact.** In `0x004ab320` the disassembly at `0x004ab4a4`–`0x004ab4c0` scales `c[0]` on the x87 stack (`fmulp st(3),st`) before `c[1]` and `c[2]` are scaled from memory; the decompiler lost it because `c[0]` was never written back. All three channels are rescaled by `255 / c[dominant]` — a true hue-preserving rescale. `CONFIRMED-BINARY`.
`INFERRED`, flagged.

The alpha path is separately confirmed: `param_5 < 0xff ? param_5 << 24 : 0xff000000` — an
8-bit alpha in the top byte, clamped.

## Which draw modes the hardware shader actually uses
`Render_ShadeAndQueueObjectPolygons` (`0x477b30`) calls through these slots:

| slot | mode | used by the HW shader? |
|---|---|---|
| `0x0056bc50` | 0 Untextured | **yes** |
| `0x0056bc54` | 1 UntexturedGouraud | **NO — never called** |
| `0x0056bc58` | 2 UntexturedLit | **yes** |
| `0x0056bc5c` | 3 SingleTextured | **yes** |
| `0x0056bc60` | 4 SingleTexturedLit | **yes** |
| `0x0056bc64` | 5 SingleTexturedLitGradient | **yes** |
| `0x0056bc68` | — DrawPolygonImmediate | **yes** |

**Mode 1 (`UntexturedGouraud`) is never reached from the hardware shader.** It is installed
in the table but unused there — presumably the software path uses it, which would be
consistent with gouraud being a software-rasteriser concern. `INFERRED`; the SW shader's
slot usage has not been checked.

`CONFIRMED-BINARY` for the five modes that *are* called.

## Direct3D bring-up — `CONFIRMED-BINARY` (2026-09-17)

**Resolution presets** (`Render_ApplyResolutionPreset` `0x004a7990`, index in ECX):

| preset | render | surface | pixel-doubled |
|---|---|---|---|
| 2 | 320×200 | 640×400 | yes |
| 3 | 320×240 | 640×480 | yes |
| 4 | 640×400 | 640×400 | no |
| 5 | 640×480 | 640×480 | no |
| 6 | 800×600 | 800×600 | no |
| 7 | 1024×768 | 1024×768 | no |

Any other index leaves the sizes untouched. Colour depth is always 16 (the 8-bit arm is dead).

**Pixel formats** (`0x004a8f80`): accepted by green mask — `0x7E0` RGB565, `0x3E0` RGB555,
`0xFF00` 24/32-bit; anything else is a fatal "Unrecognized pixel format". Textures are
registered as **ARGB4444** at bring-up (`0x004a7530`).

**Device** (`Render_CreateD3DDevice` `0x004a9c20`): 16-bit video-memory z-buffer attached to
the back surface, Direct3D device on the back surface, one viewport, a white background
material, HW and HEL caps captured.

**Default render states** (DX5 enum numbering — INFERRED from the device vtable, confirm the
DirectX version before reuse):

| state | value | meaning |
|---|---|---|
| CULLMODE | 1 | none — back faces are not culled by D3D |
| ZENABLE | 1 | on |
| **ZFUNC** | **7** | **GREATEREQUAL — a reversed depth test** |
| SHADEMODE | 1 | FLAT by default; draw modes must switch |
| TEXTUREPERSPECTIVE | 1 | on |
| TEXTUREMAG / MIN | 2 / 2 | linear / linear |
| SRCBLEND / DESTBLEND | 5 / 6 | SRCALPHA / INVSRCALPHA |
| SPECULARENABLE | 0 | off |

A remake using the default `LESSEQUAL` depth test with this engine's depth values will draw
everything back to front. Not yet established: how depth values are generated (the reason
GREATEREQUAL is correct), and whether the render-size z-buffer vs surface-size viewport
mismatch in presets 2/3 is on the live path.

## Surfaces, present and clears — `CONFIRMED-BINARY` (zvideo gap members, 2026-09-17)

**Driver enumeration** (`D3D_EnumDevicesCallback` `0x004a96b0`): a driver is skipped unless it
is hardware, RGB colour model and supports 16-bit render depth; **at most 4** drivers are kept
(a 5th is a fatal error). Each is printed to stdout (`DRIVER … OK/SKIPPED`).

**Two surface layouts** (DX5 caps numbering, `PLATFORM`):
- Windowed (`0x004a8b20`): primary with 3DDEVICE (falls back to plain primary if a lock probe
  fails), plus two offscreen surfaces of the render size; a clipper bound to the window.
- Fullscreen (`0x004a8dc0`): primary flip chain with one back buffer (PRIMARY|FLIP|COMPLEX|3DDEVICE),
  plus one offscreen surface; same clipper setup.
The offscreen surfaces are system memory unless a driver flag selects video memory.

**Present** (`zVideo_PresentFrame` `0x004a7b60`): Blt from the render surface to the primary
(DDBLT_WAIT unless no-wait). In page-lock mode it swaps the two 0x20-byte surface records instead
of copying. `zVideo_FlipFrame` (`0x004a9b70`) optionally Blts then Flips, retrying on
WASSTILLDRAWING. Both restore lost surfaces and retry.

**Clears**: colour fill with `[0x006321cc]` only when `[0x00632130]` is set; depth fill with **0**
(`0x004a81a0`/`0x004a8220`). With the GREATEREQUAL test this makes 0 the farthest depth (`INFERRED` from the two facts; depth generation still unread).

**Textures** are sized **down** to a power of two (`Math_FloorPowerOfTwo` `0x004ad680`, e.g. 300 → 256)
and converted to 1555 (no alpha plane; pixel value 0 transparent) or 4444 (with alpha).

**Fog colour** (`zVideo_ApplyFogColour` `0x004aab30`): each channel float + 0.5, truncated, packed
RGB with no clamp and no alpha byte. Palette entries are set only in 8-bit mode (dead in practice).

## Main draw queue — `CONFIRMED-BINARY` (2026-09-17)

`Render_FlushMainDrawQueue` (`0x004ad250`) disables the depth test (`ZFUNC = ALWAYS`) for the
whole flush and restores it afterwards, so **main-queue polygons are ordered by submission,
not by the z-buffer**. Entries are 0x810 bytes, one triangle fan each, with a kind 0–6:
0 translucent (alpha blend on, z-write off), 1–3 untextured flat, 4 textured flat,
5–6 textured gouraud. All state changes go through a redundant-state cache. Which draw mode
(0–5) emits which queue kind is still open.

`Render_DrawPolygonImmediate` actually draws a single **point**, not a polygon.
`Render_ProjectWorldPointToScreen` pins points behind the camera to the bottom screen edge,
spreading x linearly over ±5000 — it does not mirror them.

### Queue routing (untextured modes) — `CONFIRMED-BINARY`
| alpha | overwrite flag | destination |
|---|---|---|
| < 255 | 0 | transparent queue (max 256) |
| < 255 | 1 | main queue, kind 0 |
| 255 | 0 | drawn immediately, depth-tested |
| 255 | 1 | main queue, kind 1 (flat) / 2 (gouraud mode) |

The main queue therefore holds only *overwrite* polygons, drawn over the scene with the depth
test off. Vertex order is reversed on submission and RHW is a copy of z. Full queues drop
polygons with a logged warning. **The "gouraud" mode renders flat** in the opaque path.

### What sets the overwrite flag — `CONFIRMED-BINARY`
The HW shader pushes `[0x0057d960]` as the queue functions' overwrite argument (read at the
push sequence before `call [0x0056bc50]`, `0x00478b49`–`0x00478b7d`). That global is written
only by `RenderState_SetMode` (`0x00476080`), called from `Render_ProcessObject3DNode`
(`0x0044b300`) and `Render_ProcessLodDistanceNode` (`0x0044b8c0`): when a node's flag word
`[node+0x24]` has **bit `0x800000`** and no ancestor has already set it (guard
`[0x00539b94]`), overwrite is turned on for that node's whole subtree and cleared after it.
So overwrite is a **per-scene-node attribute** that makes a subtree draw over the scene with
the depth test off (cockpit/HUD-style geometry). `VERIFIED-ORACLE` (Recoil21): 0 writes and
value 0 — no overwrite subtree is drawn in that trace, so the runtime use is unobserved.

## Texture handles — the draw modes' "material" argument — `CONFIRMED-BINARY` (2026-09-17)

The pointer the queue functions read `+0xC` / `+0x10` / `+0x14` / `+0x18` from is a
**28-byte D3D texture binding** (`calloc(1, 0x1C)` in `0x004aa9d0`), filled by `0x004aa0f0`
(a function missing from the Ghidra export; the storing instructions `0x004aa4c4`–`0x004aa53f`
were read):

| offset | contents | source |
|---|---|---|
| `+0x0C` | **D3D texture handle** | `IDirect3DTexture2::GetHandle` (vtable `+0x0C`) with the device `[0x006333f0]` |
| `+0x10` | TEXTUREMAPBLEND | 1 (DECAL) when the caller passes no source; otherwise 4 (MODULATEALPHA) when `[src+0x14]` ≠ 0, else 5 (DECALMASK) |
| `+0x14` / `+0x18` | ADDRESSU / ADDRESSV | 3 (CLAMP) when the corresponding flag argument is set, else 1 (WRAP) |
| `+0`, `+4`, `+8` | interfaces released by `0x004aa980` | |

Enum values are the DX5 numbering, INFERRED as elsewhere in this file. Bindings are freed by
`0x004aa980`, except the shared default `[0x006333a8]`, which is never freed.
**`0x004aa0f0` read in full (2026-09-17):** failures never return NULL — they return the shared default binding. **Paletted textures are rejected** ("Palettes not supported"). Format: screen format without alpha, 1555 with 1-bit alpha, or ARGB4444 with alpha bits. **Quirk:** the 1555 path registers a 4-bit green mask (`0x3C0`) with the format converter while the surface uses `0x3E0`. Size limits: device maximum, power-of-two when required, 8:1 aspect, square-only resampling. Which game code calls hook `[0x0056bc08]` is still open.

## zVideo hook table — installed by `Render_InstallDrawDispatchTable` (`0x004a77a0`)

Besides the draw-mode slots `[0x0056bc50…74]`, the installer fills a block of function
pointers that the rest of the game calls instead of zVideo functions directly. Callers use
these pointers, so the static call graph shows no callers for the targets.

| slot | target | role |
|---|---|---|
| `[0x0056bc10]` | `0x004aa8f0` D3DTextureBinding_IsReady | surface check → bool |
| `[0x0056bc14]` | `0x004aa900` D3DTextureBinding_ReleaseSurface | |
| `[0x0056bc18]` | `0x004aa920` D3DTextureBinding_LoadFrom | colour-key from material bit 2, then `Load` into the binding's texture |
| `[0x0056bc1c]` | `0x004aa980` D3DTextureBinding_Free | never frees the default `[0x006333a8]` |
| `[0x0056bc20]` | `0x004076f0` | a bare `ret` — does nothing |
| `[0x0056bc24]` | `0x004a84c0` Texture_CreateSurface | no call site found by byte scan |
| `[0x0056bc28]` | `0x004a8650` Texture_ReleaseSurface | called from `0x00463dd0`, `0x0046ecc0` |
| `[0x0056bc2c]` | `0x004a9950` Render_GetDriverVideoMemory | a 2 MB reserve is taken off free memory in one case |
| `[0x0056bc30]` | `0x004a9a30` Render_GetDriverSecondaryMemory | |
| `[0x0056bc34]` | `0x004a9920` Render_DriverIs3DCapable | |
| `[0x0056bc38]` | `0x004a8680` Texture_GetDC | lazy surface creation |
| `[0x0056bc3c]` | `0x004a86f0` Texture_ReleaseDC | |
| `[0x0056bc40]` | `0x004aa9e0` Render_SetFogEnabled | linear fog |
| `[0x0056bc44]` | `0x004aaa30` Render_SetFogStart | cached |
| `[0x0056bc48]` | `0x004aaa60` Render_SetFogEnd | **BUG: sets fog START (pushes 5, not 6)** |
| `[0x0056bc4c]` | `0x004aaa90` Render_SetupFog | enable, colour, linear mode, start, end — bypasses the caches |

`0x004bac10` (missing from the Ghidra export) renders text into a texture through the
GetDC/ReleaseDC pair using GDI (`CreateCompatibleDC`, `DrawTextA`, …).


**Fog-end bug — `CONFIRMED-BINARY`.** `Render_SetFogEnd` (`0x004aaa60`) pushes the same light-state number as `Render_SetFogStart` (5), so changing fog end at runtime moves fog start instead, and fog end stays at whatever `Render_SetupFog` last set. The bug is the repeated constant, so it holds whatever the DirectX enum numbering turns out to be. Reproduce it.

### From game texture to binding — `CONFIRMED-BINARY`
`TextureManager_LoadPending` (`0x0046de50`) walks up to 4096 texture slots (0x24 bytes each).
For each pending slot it loads the image, falling back to a loader callback and then a
**built-in default image** (missing textures never abort), then on hardware creates the D3D
binding through hook `[0x0056bc08]` with **image byte +9 bit 1 as the alpha flag** and **image
word +0xC bits 0/1 as U/V clamp**. The shared default binding `[0x006333a8]` is created blank
when zVideo opens (`0x004a75f0`).


## Map 1 ran the HARDWARE path — `VERIFIED-ORACLE` (Recoil21, 2026-09-17)
`[0x0056bbe8]` = 1 (hardware mode), `[0x00632120]` = 1, `[0x00632128]` = 0 (not pixel-doubled), and the shader pointer `[0x0057d9e0]` = `0x00477b30`. Execution counts: the HW shader `0x00477b30` ran **1173** times; the SW shader `0x00476cf0` ran **0** times. So the captured session rendered through Direct3D, and the software rasteriser is a fallback that did not run. (One trace; the reference frames in `01_evidence/frames_index/` were recorded on the same machine.)

## Per-polygon draw-mode choice — summary
Covered by `poly_shade.md` ("The mode argument", "Object and polygon layout"): the shader picks the draw slot from the material flags word `[material+0]` (bit `0x100` = textured) together with the fade and lighting passes. It passes `ftol(byte [material+0] × fade)` as alpha, `word [material+2]` in EDX, `[material+4]` (the texture binding) as the material argument, the vertex count, and the overwrite flag `[0x0057d960]`. HW modes 0, 2, 3, 4 and 5 are reached; mode 1 never is. **Gradient vs plain lit — `CONFIRMED-BINARY` (decompile of `0x00477b30`):** among textured lit polygons, slot 5 (SingleTexturedLitGradient) is used when `[0x0057d0c8]` is non-null, else slot 4. `[0x0057d0c8]` is set per polygon to the second vertex array scratch `0x0057cdc8` only when **all three** hold: `[0x0057d40c] != 0`, the mesh's second-array count (`[mesh+5]`, see poly_shade.md) is non-zero, and the polygon dword has **bit `0x200`** ("per-vertex data from the second array"). So the per-vertex blend array of the gradient mode **is the mesh's second vertex array**. **Branch structure:** untextured polygons use slot 0 or slot 2, textured ones slot 3, or 4/5; each also emits an extra pass through the same slot when `[0x0057da2c]` is set. **Lit vs unlit — `CONFIRMED-BINARY` (disassembly of `0x00477b30`, not the decompile):** the "lit" draw slots (2, 4, 5) are chosen when the per-polygon flag local `[ebp-10h]` is non-zero; it is zero at polygon start (`0x004782df`) and becomes non-zero in two ways:
1. **Distance fade** — `Shade_DistanceFadePerVertex` (`0x00489a90`) returns non-zero (the polygon has some faded vertex): `[ebp-10h] = 1` (`0x00478310` / `0x00478983`);
2. **Lighting** — `Shade_LightPolygonVertices_HW` (`0x00487f10`) returns non-zero: `[ebp-10h] |= 2` (`0x00478348` / `0x004789b8`).

The test is at `0x004789cc` (untextured: zero -> slot 0 at `0x00478b7d`, else slot 2 at `0x00478ab7`), and the textured branch works the same way. **So the "lit" modes carry distance fog/fade as well as lighting** — an unlit polygon in the fade band is still sent through a lit mode. A remake that uses the lit path only for lit geometry will lose the distance fade.

**The two shader switches.** `[0x0057d40c]` (gradient/second-array blend) is set only by
`RenderState_SetSecondArrayBlendEnabled` (`0x00476030`) and cleared at render-state init.
`[0x0057da2c]` (the extra per-polygon pass) is set to 1 by `0x00410e90` and to 0 by `0x00410ed0`,
a pair of game-side enter/exit routines that also toggle several objects through their
`+0x60` method — so it is a **view mode** that redraws every polygon a second time.
`Render_ProcessObject3DNode` suppresses it while drawing the node `[0x004ddd38]` and restores it
afterwards, so that one node (most likely the player's own vehicle — `INFERRED`) is excluded.
**What the extra pass draws — `CONFIRMED-BINARY` (disassembly `0x00478b83`–`0x00478c3c`), with trace values `VERIFIED-ORACLE`:** it is a **magnified picture-in-picture inset**, not a colour mode. After the normal draw, when `[0x0057da2c]` is set, the polygon is re-rejected and re-clipped against a SECOND clip record at `0x00576258` (same functions `0x004803b0` / `0x0047b540`), each vertex is remapped `x' = x·[0x005762ac] + [0x005762b4]`, `y' = y·[0x005762b0] + [0x005762b8]`, `z' = z − (−1.0 / near)`, and the polygon is drawn again through the same slot with the same alpha, material and overwrite arguments. In Recoil21 the record is the screen box x 324–374, y 82.9–132.9 (a 50×50 region at the screen centre) and the transform is scale 2.0 with offsets −638.0 / +124.15 — so the centre region is drawn **2× magnified into a 100×100 box at x 10–110, y 290–390**. Its switching around camera view state 7 is in Still open (the inset is hidden during view 7) this is a **zoom/targeting inset**; the specific weapon name is `INFERRED`. The inset is pushed by 1/near in z. The inset rectangle and scale live in globals set elsewhere (the values above are runtime values, not constants).

**Inset geometry — `CONFIRMED-BINARY`.** `RenderInset_SetDestinationRect` (`0x00476120`) sets the
output window when the mode is entered. `RenderInset_SetSourceRect` (`0x00479f90`) is called each
update by the mouse raycast dispatcher `0x00411270` with a box **centred on the cursor**
(half-size `[0x004e6ae0]`/`[0x004e6ae4]`, 25.0 in Recoil21). It derives
`scale = destSize / sourceSize` and `offset = destOrigin − sourceOrigin × scale`. So the inset is a
**cursor-following magnifier**: a 50×50 area under the cursor shown 2× in a fixed 100×100 window.
The 2.0 factor is the ratio of the two boxes, not a constant.

## Per-frame flush order (HW) — `CONFIRMED-BINARY`
From `Render_CameraFrame_HW` (`0x0044d600`), after the scene walk and `zTransformStackPop`:
1. `[0x0056bc6c]` **sorted/transparent** flush;
2. when the frame's EDX flag is set, `Manager_0056bd58_Tick` (`0x004bef70`) with dt `[0x0056b424]`;
3. sorted flush again, then `[0x0056bc70]` **main (overwrite)** flush;
4. for each of `0x0049a9c0()` entries: `0x0049aa30`, a `Collision_GridDDATraversal` line test from the
   camera position (`[cam+0x2c..0x34]`), and `0x0049afb0` when the line is blocked or its result
   flag is clear — an **occlusion-tested per-entry pass** (lens flares are the likely use, `INFERRED`);
5. sorted, main, then `[0x0056bc74]` **quad** flush, then `0x004a74f0`.

So transparent geometry is flushed **before** overwrite geometry, and overwrite before quads, and
the effect tick's output lands in a second transparent flush.

**Draw-slot callers outside the two shaders (byte scan):** slot 3 (`0x0056bc5c`) from `0x0049aa90`
and `0x004bdee0` (SnowFX particle init/update); slot 6 immediate/point (`0x0056bc68`) from
`Shade_EmitPointSprite` `0x00479020`; the sorted flush also from `0x004bdee0`; the quad flush from
`0x00463440`. All other draw slots are reached only from the shaders.

## Distance and height fade — `CONFIRMED-BINARY`
`Shade_DistanceFadePerVertex` (`0x00489a90`) uses two ranges, set through confirmed setters that
also cache the reciprocal:

| range | globals | setter | default (render-state init `0x00475c40`) |
|---|---|---|---|
| A — horizontal distance √(x²+z²) | near `[0x0057d944]`, far `[0x0057d948]`, 1/(far−near) `[0x0057d94c]` | `0x00476190` / `0x004761e0` | **500.0 → 700.0** (1/200 = 0.005) |
| B — height | `[0x0057d950]`, `[0x0057d954]`, reciprocal `[0x0057d958]` | `0x00476220` / `0x00476260` | **300.0 / 200.0** |

Distance uses the same fast square-root approximation as the software 3D sound model. A zero-width
range leaves the old reciprocal in place (the setters skip the division). Because the lit draw
modes carry this fade (see "Lit vs unlit"), these defaults decide where geometry starts to fade in
the hardware path.

## Animated materials
`Material_AdvanceCycle` (`0x00481140`, CONFIRMED, see `poly_shade.md`) advances a material's
cycle record `[material+0x24]` at most once per frame stamp `[0x0056bbd8]`; the shaders call it for
materials with flag bit `0x400`. A null cycle pointer is reported through the no-op reporter and
ignored.

## Screen sprites
`Render_DrawScreenSprite` (`0x0049aa90`) draws a screen-aligned textured square, clipping it to the
screen or a rectangle **with UV correction** so a partly off-screen sprite shows the right part of
its texture. HW draws it at z 0.5 through slot 3; SW queues it at z 10.0.

**Point queue and its occlusion pass.** Points appended by `Queue_AppendPoint` (`0x0049a830`) go to a
650-entry queue (0x14 bytes: x, y, z, value, object). In the HW frame, points that carry an object
pointer (only the first 64 entries qualify) get a line-of-sight test from the camera and an inverse-
depth fade (`Backend_ItemDistanceFade`) — this is the occlusion-tested pass in the flush order.
`PointQueue_FlushAll` (`0x0049a8c0`) draws and empties the queue; `PointQueue_Clear` (`0x0049a910`)
empties it without drawing.

**The transparent queue is depth-sorted** (`Render_FlushSortedDrawQueue` `0x004ace30`, from its
ledger note): a stable ascending sort on vertex 0's screen z, drawn far to near — a painter's sort
keyed on one vertex, so intersecting or long transparent polygons can sort wrongly, as in the original.

**Lens flares — `CONFIRMED-BINARY`.** The occlusion-tested point pass draws lens flares:
`LensFlare_Draw` (`0x0049b020`) takes the distance-fade intensity and draws four screen sprites
at one centre with half-sizes 2s, 2s, s, 3s where s = intensity × screenWidth / 32. Visibility
comes from `World_QueryObjectsAtPoint` (`0x00443d20`), which tests a grid cell of a
**wrapping, tiled world**.
