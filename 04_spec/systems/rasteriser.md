# Rasteriser (software back end)

Subsystem `rasteriser` in `03_re/ledger/subsystem_registry.csv`. **Membership OPEN.** Seeded from the direct callees of the render_frame back-end flushes (`0x0049a2b0`, `0x0049a490`, `0x0049a920`, `0x00490590`, `0x0048d7a0`). Every statement below is CONFIRMED-BINARY from bytes read with cdb `uf` unless tagged otherwise. Per-function detail is in the ledger notes.

## Span buffer

A span record is 0x18 bytes (CONFIRMED-BINARY: `0x00491da0` advances the allocator by 0x18):

| Offset | Type | Meaning |
|---|---|---|
| +0x00 | ptr | next span in the row (x-sorted) |
| +0x04 | int | x0 |
| +0x08 | int | x1 (inclusive) |
| +0x0c | float | z at x0 (inverse depth: larger = nearer) |
| +0x10 | float | z at x1 |
| +0x14 | float | dz/dx |

- The allocator cursor is `[0x0057dae0]`. The row heads are the array at `[0x0057dae4]`, one pointer per row, `[0x0057dafc]` rows. `0x00490590` zeroes the rows and resets the cursor to `[0x0057dae8]` each frame.
- The treatment of z as inverse depth is CONFIRMED-BINARY. `0x0049afb0` computes distance as 1.0 / item+8, and `0x00498c40` copies that same field into both z slots of the item span.

## Depth tolerance

`0x004904d0` zRndr_SetPerspectiveInverseZTolerance(t) sets three globals:
- tol `[0x0057db04]` = t;
- k `[0x0057db08]` = t + 1.0;
- ik `[0x0057db0c]` = 1/k, or 0 when k = 0.

The default is t = 1.0e-4 (float `0x38d1b717`, from zRndrInit `0x0048fd80`). The script parser `0x004c20a0` can override it through the keyword `SetPerspectiveInverseZTolerance`. The parser matches only the first 20 characters of the keyword, so any keyword starting with those 20 characters triggers it (quirk).

## Visibility of billboard items

- `0x00498c40` Raster_ItemCoverageTest(item) writes a one-row span for the item at the cursor: x0 = x1 = ftol(item.x) and z0 = z1 = item+8. It then asks `0x00491dd0` whether the span is visible on row ftol(item.y).
- `0x00491dd0` Span_TestVisible walks the row. It reports visible as soon as any part of the item's span is not covered by a nearer span. As it goes, it clips the span's x0 in place, past each covering span. The cheap bounding test (minimum times k against maximum) runs first. `0x004907c0` is the exact test.
- `0x004907c0` Span_FrontTest has the following steps (full formulas in the ledger):
  1. special cases for one-pixel spans;
  2. two symmetric edge-sign tests with tolerance tol;
  3. as a fallback, a compare of depth sums at the two ends of the overlap.
- **Quirk to preserve:** three of `0x004907c0`'s exits compare floats as signed 32-bit integers. It stores the float and then does an integer compare, which agrees with a float compare only while both values are non-negative.

## Screen tint (called from `0x0048d7a0`)

- Channel values come from `0x004a6bb0`, which reads `[0x00632164]`, `[0x00632168]` and `[0x0063216c]`. Reading these as the pixel format's channel masks is INFERRED.
- Frame weight k = `[0x0056b19c]`; the tint terms are pre-scaled.

| Address | Format | Path | Pixels per step |
|---|---|---|---|
| `0x0048d450` | 555 | integer | 2 per dword, (width>>1)+1 steps: writes one dword past the row (quirk) |
| `0x0048d4b0` | 565 | integer | same as above |
| `0x0048d510` | 555 | MMX | 4 per qword, width>>2 steps: leaves width&3 pixels untouched and reads one qword past the row |
| `0x0048d5f0` | 565 | MMX | same as above |

- The MMX path is chosen when byte `[[0x00632110]] & 4` is set. Reading that bit as MMX support is INFERRED from the instruction set of the two blitters it selects.
- 555 is chosen when `[0x0057de38]` == 5.

## Span-function pointers

- The defaults are `[0x006320a4]` = `0x00490ae0` and `[0x006320a8]` = `0x00491840`.
- The poly flush `0x0049a490` swaps in `0x004912a0` and `0x00491da0` (Span_AllocOpen, which always allocates a fresh span and reports it visible). It restores the defaults afterwards.

## Not yet read (members pending)

- span insertion: `0x00490ae0`, `0x00491840`, `0x004912a0`;
- clip-polygon apply: `0x004927d0`;
- draw primitives: `0x00492000`, `0x00492f00`, `0x00493df0`, `0x00494af0`, `0x00495850`, `0x004969d0`, `0x00497ac0`.

## Clip-polygon fill (`0x004927d0` zRndr_FillClipPoly) - CONFIRMED-BINARY

- **Fixed point:** `fx(v)` = the low dword of (1.5·2^52 − v·(−65536.0)), which is round(v·65536) in 16.16.
- **Vertex cleanup:** a vertex whose x and y both equal the previous kept vertex is dropped, as is a closing copy of the first vertex. A polygon left with fewer than 3 vertices is skipped.
- **Row range:** from `(fx(top.y)+0x7fff)>>16` to `(fx(bottom.y)−0x8041)>>16` inclusive. The `0x8041` bias is exact: a quirk to keep.
- **Row limits:** each row runs from `(min(xa,xb)+0x7fff)>>16` to `(max(xa,xb)−0x8001)>>16`. Both edges step by their 16.16 dx/dy after the limits are taken. Edge x values are evaluated at row + 0.5.
- **Emitted span:** each non-empty row becomes a span whose z is constant, taken from the first input vertex's z. It is inserted through `[0x006320a4]` (ECX = vertex buffer, EDX = row, stack = &count); outside the poly flush that pointer is `0x00490ae0`.
- **Unused flag:** `[0x0057dac8]` makes no difference to the output here.

## Occluding span insert (`0x00490ae0` Span_InsertOccluding) - CONFIRMED-BINARY

This is the default `[0x006320a4]`. It inserts the cursor span into its row's x-sorted list:
- **Occlusion test:** at each overlap it asks whether the new span is in front. The cheap bounding test runs first and `0x004907c0` is the exact test.
- **Quirk:** this bounding test compares the float bit patterns as signed integers, whereas `0x00491dd0`'s uses FPU compares.
- **Behind:** the new span is trimmed, split around the existing span, or dropped when fully hidden.
- **In front:** the existing span is trimmed, split into left and right parts, or unlinked when exactly covered, and z at its new ends is re-derived from z0/z1 and dz.
- **Output:** each visible piece is appended to the caller's array. A piece that starts right after the previous piece is merged into it, which copies x1, z1 and next.
- **Row head:** replaced when the newly placed span starts at or before it. The end-of-list and split-existing-span paths skip this update.
- Every case is listed in the ledger note.

## Visible-piece clip (`0x00491840` Span_ClipVisible) - CONFIRMED-BINARY

This is the default `[0x006320a8]`:
- **Read-only against the row:** it clips the cursor span against the row's existing spans but never modifies the row list. Each existing span is compared through a local copy.
- **Output:** the visible pieces go into the caller's array. Pieces that start right after the previous piece are merged into it.
- **Occlusion test:** it uses FPU compares for the bounding test, then `0x004907c0`.
- **Quirk (cursor not advanced):** several exits put the span in the output array without advancing the cursor, so the next allocation reuses that record. Consumers must use the output before allocating again.
- During the poly flush `0x0049a490`, this entry is replaced by `0x00491da0`, which reports the whole span as visible.

## Overwriting span insert (`0x004912a0` Span_InsertOverwrite) - CONFIRMED-BINARY

This is the `[0x006320a4]` used during the poly flush:
- **Same logic as `0x00490ae0`:** the same insertion cases, splits, output merging and row-head rule.
- **No depth test:** the new span always wins. Existing spans it overlaps are trimmed, split, or unlinked when exactly covered.

## Flat polygon (`0x00492000` Raster_FillFlatPoly) - CONFIRMED-BINARY

- **Arguments:** ECX = screen vertices, EDX = three points defining the depth plane. There are **two** stack arguments, the vertex count and the pixel value (`ret 8`); the decompiler shows one.
- **Depth plane:** dz/dx and dz/dy come from the three points. They are 0 when the determinant is 0. z at the origin is taken at pixel centres (0.5, float `0x004d2df0`).
- **Scan conversion:** the same as the clip-polygon fill, except that duplicate vertices are not removed.
- **Span depth:** each row's span has z0/z1 = plane depth at its ends × `[0x0057dac4]` + `[0x0057dac0]`. dz/dx is stored **unscaled** (quirk).
- **Drawing:** the span is inserted with `[0x006320a4]`. Each visible piece is filled by `[0x006320b0]` (default `0x004997d0`) with ECX = pixel value, EDX = pixel count, and the destination address in `[0x0056b270]`.
- **Destination address:** x0·`[0x00632060]` + y·`[0x0063205c]` + `[0x00632050]` (bytes per pixel, pitch, framebuffer base).

## Blended flat polygon (`0x00492f00` Raster_FillFlatPolyBlend) - CONFIRMED-BINARY

- **Same as `0x00492000`:** gradients, scan conversion and span depth are identical.
- **Arguments:** three stack arguments (n, level, pixel value); `ret 0xc`.
- **Span path:** spans go through `[0x006320a8]` (default: the non-inserting clipper). A translucent polygon is drawn where visible but never hides anything drawn after it.
- **Filling:** each piece is filled by `[0x006320b4]` (ECX = pixel value, EDX = level, stack = pixel count). `0x00499810` is installed there in 555 mode.
- **Level source:** the sorted flush passes `R+0x378`; the poly flush passes ftol(v).

## Textured polygon (`0x00493df0` Raster_FillTexturedPoly) - CONFIRMED-BINARY

- **Arguments:** four stack arguments (plane, UV block, n, shade index); `ret 0x10`.
- **Pre-multiplied texture coordinates:** u and v are scaled by the texture size (signed 16-bit words at tex+4/+6) and multiplied by vertex z before the plane gradients are built.
- **Detail-level choice:** a texture detail level can be chosen by `0x00499130` when `[p+0x20]` is set.
- **Perspective:** once per visible piece, not per pixel. True u and v are computed at both ends of the piece from the unscaled plane depth, then interpolated linearly in 12.20 fixed point.
- **Quirk:** the step is (u1−u0)/n, not /(n−1).
- **Fill routine:** chosen from `0x006320c8`/`cc`/`d0`/`d4` according to fields of the texture level; a shade-table offset of (shade+1)·0x200 is used when a shade table exists.

## Blended textured polygon (`0x00494af0` Raster_FillTexturedPolyBlend) - CONFIRMED-BINARY

This is `0x00493df0` with five stack arguments (a float level v and a shade index), and these differences:
- **Fill table:** its own fill routines, `0x006320e0`–`0x006320ec`.
- **Level global:** `[0x0057da48]` = round(v·255). When the texture has a shade table it holds the raw float bits of v instead (quirk).
- **Per-pixel steps:** computed as trunc(round(Δ·2^20)/n) by integer division, not the float 2^20/n multiply. Both still divide by n.
- **Pixel-centre offsets:** computed as (x+0.5)−P0.x.

## Framebuffer and perspective-subdivision settings - CONFIRMED-BINARY

- **`0x00490340`** sets the framebuffer base, clip rectangle, bytes per pixel (bpp >> 3) and pitch.
- **`0x00490430`** sets a fixed sub-span length: the largest power of two not above the request, and never below 8. The default is 32.
- **`0x00490480`** sets the adaptive minimum, maximum and factor. A minimum of 0 means fixed mode, which is the default.
- **`0x004904a0`** sets `[0x0057de68]` = 1/t, ignoring t = 0. Its readers are not yet traced.
- **Adaptive formula (read in `0x004969d0`, which is not yet confirmed):** L = clamp(round(|minz · factor / dz/dx|), min, max), and L = max when dz/dx = 0.

## Texture span fill (`0x0049e6c0` Fill_Tex16_RightToLeft) - CONFIRMED-BINARY

- **Arguments:** ECX = u and EDX = v in 12.20 fixed point. Stack arguments: n pixels and k = 20 − log2(texture width), with k from 10 to 17 (widths 1024 down to 8).
- **Texel lookup:** texel = ((u>>20) & (width−1)) + ((v & `[0x004e21f4]`) >> k), a 16-bit value from `[0x0056b260]`.
- **Direction:** pixels are written right to left, using ESP as the destination pointer and `push word`. After each pixel, u and v advance by `[0x0056b268]`/`[0x0056b26c]`.
- **Caller contract:** the subdivided filler `0x004969d0` passes the **end** values of each sub-span and steps of (start − end)/count. The rightmost pixel therefore gets the next sub-span's start value, a one-pixel offset to keep.

## Subdivided textured polygon (`0x004969d0` Raster_FillTexturedPolySubdiv) - CONFIRMED-BINARY

- **Near-plane fallback:** if any vertex has view z < 10.0, the texture gradients come from the view-space solver `0x004753e0`, with the origin at the screen centre. Otherwise they come from the screen-space three-point solve.
- **Perspective step:** each visible piece is walked left to right in sub-spans of L pixels, with the perspective divide redone at each sub-span boundary. L is fixed (default 32) or adaptive: clamp(round(|minz·factor/dz/dx|)).
- **Adaptive depth estimate (quirk):** minz uses only the x gradient and is limited to the range 0 < z < 1000.
- **Fill:** `[0x006320b8]` or `[0x006320bc]` (with a shade table). Each call receives the sub-span's **end** u/v and a step of (start − end)/count; the fill writes right to left.
- **Edge-advance quirk:** at each row where an edge changes, each edge table advances by only one entry.

## Blended subdivided textured polygon (`0x00497ac0` Raster_FillTexturedPolySubdivBlend) - CONFIRMED-BINARY

This is `0x004969d0` with a sixth stack argument (the level) and these differences:
- **Level global:** `[0x0057da48]` = the level argument, stored raw. The sorted flush passes the float v's raw bits (quirk).
- **Fill routines:** `0x006320d8`/`dc`.
- **Span path:** the non-inserting `[0x006320a8]`.
- **Fill direction:** it passes each sub-span's **start** u/v with forward steps, the opposite of the opaque version, so its fill routines write left to right.

## Shaded textured fan triangle (`0x00495850` Raster_FillShadedTexturedFan) - CONFIRMED-BINARY

- **Per-fan cache:** the texture and depth gradients are solved once per fan (index j = 0) and kept in globals `0x0056b1f0`–`0x0056b238`. Triangles with j ≠ 0 reuse them.
- **Per-triangle colour:** a colour gradient is solved for each triangle. Colour is clamped to 0–255 at every sub-span boundary.
- **Drawing:** a subdivided perspective texture pass (right to left, end values), followed by a separate left-to-right colour pass through `[0x00632104]` (16.16 colour and step).
- **Combined fill instead:** when the texture has a shade table and no shade index is given, `0x0049f180` is used as a single combined fill, with its shade-table offset from a colour-lookup index. The index comes from `[0x004e2200]` or, when that is negative, `0x0046e960` (outside the rasteriser).
- **Depth estimate:** the minimum-depth estimate here uses both the x and y gradients, unlike `0x004969d0`.

## Texture detail-level choice (`0x00499130` Raster_PickTextureLevel) - CONFIRMED-BINARY

When `[0x0063209c]` is set:
- **Nearest vertex:** it takes the vertex with the largest z (inverse depth, so the nearest).
- **Texel step:** it measures how far the true texture coordinates move over one pixel in x and in y (four derivatives).
- **Choice:** it passes (signed max of the four, rounded) >> 1 to the texture manager's `0x0046e290`.
- **Quirk:** the maximum is signed, so steps that decrease u or v are never chosen.

When the flag is clear, the base texture `[p]` is used.

## Blend-to-constant span passes (`[0x00632104]`: `0x0049e200` 565, `0x0049e300` 555, `0x0049e400` 565 MMX, `0x0049e560` 555 MMX) - CONFIRMED-BINARY

These are the fan filler's second pass. They blend each pixel toward a constant colour by an amount v, 0–255 in the high half of a 16.16 value. Calling this fog is INFERRED.

- **Integer versions:**
  - v ≥ 256 → the pixel becomes the constant (`[0x00631fc8]`, or the pixel pair `[0x00631fcc]`);
  - v < 8 → the pixel is unchanged;
  - otherwise each channel is kept by w = (256−v)/8 over 32, plus a 32-entry table `[0x00631fd0]`;
  - pixel pairs share one v (quirk).
- **MMX versions:**
  - out = pixel + (K − pixel)·v/256 per channel, with K in `[0x0057da78]`–`[0x0057da88]`;
  - no thresholds, and each pixel's v is one step ahead of the integer version (quirks);
  - the unaligned head (byte address & 3 pixels, a quirk) and the tail go through the integer version.

## Blend-state record and presets - CONFIRMED-BINARY (selectors, single-pixel blend)

- **Active record:** `0x00631fb0`, 40 dwords: 3 floats (colour, INFERRED), channel values K (`+0xc`..`+0x14`), constant pixel word and pair, then a 32-entry table.
- **Presets:** three stored at `0x00631dd0`, `0x00631e70` and `0x00631f10`.
  - `0x0049b530`, `0x0049b710` and `0x0049b4c0` copy one into the active record when any of its three floats differs by at least 0.01.
  - Scene code calls these selectors, from `0x00476cf0`, `0x00487f10` and `0x00488d60`.
- **Single-pixel blend:** `0x0049b780` blends one pixel toward K by amt/256. Its low-channel term is not masked (quirk).
- **Preset builders:** `0x0049b1e0`/`0x0049b350`/`0x0049b5a0` clamp the colour to 0–1 and rebuild a preset through `0x0049e0e0`. They're seeded but not yet read.

## Blend-state preset build (`0x0049b1e0`, `0x0049b350`, `0x0049b5a0`, table `0x0049e0e0`) - CONFIRMED-BINARY

- **Clamp and change check:** each builder clamps an RGB triple to 0–1 in place. If any component changed by at least 0.01, it stores the triple and, in hardware mode (`[0x0056bbe8]`), calls its Direct3D routine.
- **Channel conversion:** it converts each component to ftol(c·255 + 0.5) and places it with the pixel-format shifts and masks `[0x0057de4c/50/54]` and `[0x0057de40/44]`.
- **Table:** `0x0049e0e0` writes the channel values, the constant pixel and pixel pair, and a 32-entry table with entry w = (31−w)·S.
- **Quirk:** the kept pixel and the constant are weighted w/32 and (31−w)/32, which sum to 31/32, so partial blends come out slightly dark.
- **Open:** nothing found so far writes the MMX blend words `0x0057da78`/`80`/`88`.

## Shaded 8-bit texture fill (`0x0049f180` Fill_Tex8Shaded_RightToLeft) - CONFIRMED-BINARY

- **Same addressing as `0x0049e6c0`:** eight width cases, writing right to left with the push-word trick.
- **Differences:** the texel is a byte, and the output pixel is looked up in a shade table at `[0x0056b264]`: entry = 256·((c >> 19) & 31) + texel.
- **Blend level:** c is the running 16.16 blend amount (`[0x0056b274]`, stepped by `[0x0056b278]`), giving 32 levels.
- **Where the table comes from:** the fan filler selects it per lighting record as index·0x4000 + 0x200 + `[t+0x18]`.

### MMX blend colour words - static finding (2026-09-16)

- **What was checked:** a raw scan of `Recoil.exe` finds exactly two references each to `0x0057da78`, `0x0057da80` and `0x0057da88`. All six are the reads inside the MMX blend passes `0x0049e400`/`0x0049e560`. No instruction writes them by absolute address, and their initial bytes are zero.
- **Consequence:** unless something writes them through a computed pointer (not yet excluded), the MMX blend path always blends toward black, whatever preset is selected. This is a candidate quirk (INFERRED until the pointer route is excluded).
- **Why the traces can't settle it:** both TTD traces show zero reads and writes of these words. The traces use the hardware renderer, so the software path never runs in them.
- **Hardware routines:** `0x004a7220`, `0x004a7250` and `0x004a7300` store colour × 255 for the hardware renderer. `0x004a7250` also records the largest channel in `[0x00632140]`.

## Fill routines, first batch - CONFIRMED-BINARY

| Address | Slot | What it does |
|---|---|---|
| `0x004997d0` | `[0x006320b0]` | solid colour, right to left (push trick) |
| `0x00499810` / `0x004998a0` | `[0x006320b4]` 555 / 565 | constant colour blended at one level; skipped below 8 (555) or below 4 (565); written outright at 252 and above; arithmetic shifts |
| `0x0049c860` / `0x0049c760` | `[0x006320d8]` 555 / 565 | 16-bit texels blended at the level `[0x0057da48]`, left to right; same thresholds; **logical** shifts on negative differences (quirk); the 555 version adds the top channel to memory before the rest |

These fills share two quirks:
- **Low channel unmasked:** in every blend fill the low-channel term is not masked.
- **Zero count:** the dec/jne loops never terminate properly on n = 0.
| `0x0049d6e0` / `0x0049d5c0` | `[0x006320dc]` 555 / 565 | the same as `[0x006320d8]`, but the texel is a byte looked up in the shade table `[0x0056b264]` |
| `0x0049c560` / `0x0049c360` | `[0x006320c8]` 555 / 565 | alpha-mapped 16-bit texels (alpha byte map `[0x0056b27c]`): alpha below 8 is transparent, 248 and above is opaque; an odd first pixel is blended by alpha/256, then **pairs share one texel and alpha** and blend with 5-bit weights that sum to 31/32 (quirks) |
| `0x0049d3b0` / `0x0049d1a0` | `[0x006320d0]` 555 / 565 | the same as `[0x006320c8]`, but the texel is a byte looked up in the shade table |
| `0x0049edc0` | `[0x006320bc]` (and `[0x006320dc]` when the bit-1 flag is clear) | 8-bit shade-table texels, right to left, no transparency |
| `0x0049b7e0` | `[0x006320c0]`/`[0x006320cc]` (and `c8`/`e0`/`e8` when the bit-1 flag is clear) | 16-bit texels, right to left, **texel 0 is transparent** |
| `0x0049bbf0` | `[0x006320d4]` (and `d0`/`e4`/`ec` when the bit-1 flag is clear) | 8-bit shade-table texels, right to left, **texel byte 0 is transparent** |
| `0x0049ec20` | `[0x006320b8]` (MMX, and `0x004b3050()` true) | 16-bit texels, **left to right**, two per MMX step (masks and shift set by `0x0049ea40`) |

### OPEN: fill direction vs caller convention (2026-09-16)

The fills write in two directions:
- **Right to left, first pixel at x0+n−1:** `0x0049e6c0`, `0x0049edc0`, `0x0049b7e0`, `0x0049bbf0`, `0x0049f180`, `0x004997d0`.
- **Left to right:** `0x0049c760`/`c860`, `0x0049d5c0`/`d6e0`, `0x0049c360`/`c560`, `0x0049d1a0`/`d3b0`, `0x0049ec20`.

The callers pass values for one direction or the other:
- **Start values, forward steps:** `0x00493df0`, `0x00494af0` and `0x00497ac0`. For `0x00493df0` this was re-traced from the bytes.
- **End values, reversed steps:** `0x004969d0` and the fan filler's texture pass.

Per the installer `0x0048ff80` (decompilation; bytes pending), several slot and caller pairs mix the two conventions:
- `[0x006320cc]`/`[0x006320d4]` are always right to left, yet `0x00493df0` passes start values to them;
- `[0x006320d8]` is right to left when the bit-1 flag is clear, yet `0x00497ac0` passes start values;
- the MMX `[0x006320b8]` is left to right, yet `0x004969d0` passes end values.

Taken at face value, these would draw textures mirrored within each span. That is not yet reconciled. The possibilities are a misread direction, modes that never run in practice, or a real quirk. It has to be settled before any of these paths is coded.

## Fill-table installer (`0x0048ff80` Raster_InstallFillTable) - CONFIRMED-BINARY

- **MMX bit cleared:** it clears bit 2 of the flag dword `[[0x00632110]]` before testing it, so the MMX fill branches are unreachable from this installer.
  - In practice `[0x006320b8]` is `0x0049e6c0`, `[0x00632104]` is the integer blend pass, and `[0x00632108]` is 0.
  - The MMX fills and `0x0049e140` are never installed by it.
- **Pixel-format codes:** 5 = 555, 6 = 565.
- **Subdivision defaults:** 16–64 when bit 3 of the flag byte is set, otherwise 32–512, with factor 0.1.
- **Early exit:** it installs nothing unless there are 2 bytes per pixel.
- The full table is in the ledger note.

**Direction contradiction, narrowed:** the MMX case is dead code here. Still open:
- `[0x006320cc]`/`[0x006320d4]` (always right to left) called by `0x00493df0` with start values;
- `[0x006320d8]` = `0x0049e6c0` (right to left) called by `0x00497ac0` with start values when the bit-1 flag is clear.

**Status of the remaining mismatch (2026-09-16):**
- **What is certain:** both conventions are byte-confirmed. On the paths that pair `0x00493df0` with `[0x006320cc]`/`[0x006320d4]`, and `0x00497ac0` with `0x0049e6c0` when the bit-1 flag is clear, the texture is mirrored within each visible piece.
- **What is unknown:** how often those paths run. None of the existing traces use the software renderer.
- **For the remake:** reproduce the pairing as the binary has it, and flag it here as a known difference from what the design presumably intended.
| `0x004992b0` | `[0x006320fc]`/`[0x00632100]` | put one 16-bit pixel at (x, y) |
| `0x0049c150` | `[0x006320e8]` | 16-bit texels, texel 0 transparent; level ≥ 252 writes the texel, but **levels 4–251 write nothing visible**: the blend reloads the destination and adds zero (binary defect, reproduce as-is) |
| `0x0049c020` / `0x0049c230` | `[0x006320ec]` 565 / 555 | 8-bit shade texels, byte 0 transparent; level ≥ 252 writes shade[texel]; for levels 4–251 the blend **indexes the shade table with the destination pixel** instead of the texel, and the 555 version uses **565 masks** (binary defects) |
| `0x0049ca90` / `0x0049c970` | `[0x006320e0]` 555 / 565 | 16-bit texels blended at effective level round(alpha × v), with v read **as a float** from `[0x0057da48]`; thresholds 8 (555) / 4 (565) and 252 |
| `0x0049d950` / `0x0049d810` | `[0x006320e4]` 555 / 565 | the same, with the texel taken from the shade table |
| `0x0049ea40` | `[0x00632108]` (MMX only, unreachable) | stores the MMX texture masks and shift |

**Correction:** `0x00494af0` storing the raw float v in `[0x0057da48]` for alpha-mapped levels is intentional, because the `[0x006320e0]`/`[0x006320e4]` fills read it as a float. The remaining level-type mismatch is the sorted flush `0x0049a2b0` passing raw float bits to `0x00497ac0`, whose `[0x006320d8]` fills read the global as an integer. That makes the level huge, so the texels are written opaque.
| `0x004992d0` | `[0x006320f0]` | solid Bresenham line, no clipping; x-major only when \|dx\| > \|dy\| strictly |
| `0x004993a0` | `[0x006320f4]` | dashed line: dash length (major+1)/d, each dash and gap one pixel longer than that (quirk), d = 0 faults |
| `0x00499500` | `[0x006320f8]` | clipped line: outcode trivial reject, float-slope clip in x then y (ftol truncation), then Bresenham; x is not re-checked after the y clip (quirk: ends can land up to a pixel outside) |

### MMX fill paths are dead code (2026-09-16, static evidence)

- **Only reference:** the MMX fills (`0x0049ea80`, `0x0049cea0`, `0x0049ddb0`, `0x0049cbb0`, `0x0049da80`, `0x0049ec20`) and helpers (`0x0049ea40`, `0x004b3050`, `0x0049e140`) are referenced only from the MMX branches of `0x0048ff80`.
- **Why they never run:** `0x0048ff80` clears the MMX bit before testing it, so none of them executes.
- **For the remake:** it needs only the integer paths.

#### The dead MMX fills - CONFIRMED-BINARY (bytes read 2026-09-16; recorded for completeness, not for porting)

| Address | Slot (installer store) | What it does |
|---|---|---|
| `0x0049ea80` | `[0x006320b8]` (`0x0049007f`/`0x004900f4`), when `0x004b3050()` is false | 16-bit texels, left to right, pixel pairs via MMX; head/tail pixels use the stack shift argument, the pairs use `[0x0057da68]` |
| `0x0049cea0` / `0x0049cbb0` | `[0x006320c8]` 555 / 565 | alpha-mapped 16-bit texels: gather into a 1024-pixel stack buffer, blend 4 pixels at a time with **no thresholds**, the last n & 3 pixels scalar with thresholds 8/4 and 252 (the two paths treat the same alpha differently) |
| `0x0049ddb0` / `0x0049da80` | `[0x006320d0]` 555 / 565 | the same, with the colour from the shade table |

The blend masks come from `0x0049e140`: `0xFC00`/`0xFFE0` keep the sign bits, so negative channel differences do not carry into the next channel.

## Function index additions and scope correction (2026-09-24)
**Scope correction.** Most of this subsystem is the software 3D back end, which is reference
only for v1. The functions below are different: they are **2D pixel operations on the 16-bit
screen buffer used by the UI and screen effects**. For example, `Raster_DrawBlendLineList` is
called from `ui_widgets` (`0x004be1fd`), and `Raster_BlitImageColorKey` is a UI image vtable
slot. They **must be reproduced exactly** in v1, because they produce the menu and HUD pixels
that the 2D machine check compares.

- **Buffers:**
  - `Raster_InitNoiseAndBackBuffer` `0x0048d340` (a `rand()` noise row, so it consumes random
    numbers) and `Raster_FreeNoiseAndBackBuffer` `0x0048d3e0`;
  - `SetGlobalRect_0056b1c4` `0x0048d420` (source buffer rect);
  - `SwRender_FreeBuffers` `0x00490780`.
- **Pixel operations:**
  - `Raster_CopyPixelClipped` `0x0048da60`;
  - `Raster_DrawBlendLineList` `0x0048ec90` and `Raster_BlendLine` `0x0048ed60` (555 and 565
    paths);
  - `Raster_BlitImageColorKey` `0x0048f560`;
  - `Raster_RippleDistort` (already above).

  Per-path details of `Raster_BlendLine` and `Raster_BlitImageColorKey` are in the ledger
  (resolved 2026-09-24).
  - **Quirk:** the blit clamps the bottom row only when `y+h-1 > H`, where x uses `>=`, so it
    can write one row past the clip.
  - **Alpha thresholds:** 555 surfaces skip at alpha ≤ 7, 565 at ≤ 3; both copy the source at
    ≥ 252.

