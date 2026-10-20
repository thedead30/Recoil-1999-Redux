# Per-frame camera render

Subsystem `render_frame` (membership **CLOSED**). Five functions tied by exclusive call edges:
`Render_DispatchCameraList` `0x0044f630`, the camera frames `0x0044d600` (hardware, the one that
runs) and `0x0044d3a0` (software), `Camera_UpdateFollowNodes` `0x0044d320`, and
`Render_SetFrameRateCap` `0x00449ba0`.

## Each frame (CONFIRMED, bytes)
1. For every **active camera** on list 8, run the hardware or software frame (`[0x0056bbe8]`).
2. **Adaptive level of detail**:
   - when the frame-rate cap is on, a detail value is lowered by 0.05 on a frame that meets the
     cap (0.04 s = 25 fps);
   - it is raised by 0.05 on a frame that misses it;
   - it is clamped to [0.6, 1.0] and passed to the camera's LOD scale. Every LOD distance scales
     by that value's inverse square.
   - Recoil18 `MEASURED`: cap 0.04, step 0.05, floor 0.6, start 1.0; the cap switch was off when
     the recording began.
3. Lights are selected and moved to view space, the camera and its follow nodes (sky and horizon,
   `camera.md`) are updated, the projection is set up, and sound emitters are processed.
4. The scene is rendered (`0x0044d240`, `scene_render.md`), then the back end runs through a
   function-pointer table (`0x0056bc6c..74`) and a per-item pass with a ray test from the camera.

Many callees here are unexplored (span buffer, D3D back end, projection caller, collision); they
are the gate's open items.

## Software back end and zRndr callees (CONFIRMED-BINARY, bytes read)

Constants: 0.0 = float `0x004d2e44`, 1.0 = float `0x004d2e48`, 255.0 = float `0x004d2e4c`, 32.0 = double `0x004d2d88`, 1.0 = double `0x004d2d90`. ftol = `0x004c60a6` (truncates).

- `0x00490600` zRndr_ClearClipPolys: `[0x0057de30] = 0`.
- `0x00490710` zRndr_AddClipPoly(ECX verts xyz stride 12, EDX n): ignored at 7 polys; copies all n vertices into entry `0x0057db10 + k*0x64` but stores min(n,8) at `+0x60` (n > 8 overruns into the next entry - preserve). Render frame feeds it every bit-31 record of the camera's `+0x40` table (stride 0x34, count `[cam+0xe0] & 0x7fffffff`, only when `+0xe0` bit 31 set and more than one camera is listed).
- `0x00490590` zRndr_BeginFrame: zero `[0x0057dafc]` dwords at `[0x0057dae4]`, reset `0x0057daf0`/`0x0057daf4`, `[0x0057dae0] = [0x0057dae8]`, then `0x004927d0(ECX=poly, EDX=count)` per clip poly.
- `0x0049a2b0` Backend_FlushSortedQueue (called twice): bubble sort ascending by float `R+0x334`, records `0x0057de80 + idx*0x384`, initial order reversed; dispatch per record to `0x00492f00` (no mesh) / `0x00493df0` (material bit 2, v >= 1.0) / `0x00494af0` (bit 2, v < 1.0) / `0x00497ac0` (textured). Count cleared.
- `0x0049a490` Backend_FlushPolyQueue: swaps span function pointers `0x006320a4/a8` to `0x004912a0/0x00491da0` for the flush and back to `0x00490ae0/0x00491840`; record types 0/1/2 with 255.0 opacity threshold (type 0), 1.0 threshold and x255 rescale (type 1), triangle fan through `0x00495850` (type 2). Full layout in the ledger note.
- `0x0049a920` Backend_CompactAndCollectItems(ECX start): removes items whose `0x00498c40` test fails by swapping the last one in; collects the first 64 entries with a live object and non-zero inverse depth into `0x00631cd0`.
- `0x0049b1a0` Backend_FadeKeptItems: `0x0049afb0(i)` for each collected item, only when `[0x0056b248]`; count cleared only in that case.
- `0x0048d7a0` Backend_ScreenTintOverlay: 16-bit framebuffer blend, colour weight ftol(alpha*32), frame weight ftol((1-alpha)*32), blitter chosen from 4 by format flag and mode 5.

## Function index additions (2026-09-24)
- **Renderer state setters**, mostly called from the scene script:
  - `Render_SetPerspectiveTexDeltas` `0x00476090`;
  - `Render_SetInverseZTolerances` `0x004760b0`;
  - `Render_SetSmallPolygonRejectArea` `0x00476300` (hardware also sets `[0x0056bbf4]`);
  - `RenderState_SetValue5C` `0x004762a0`, `RenderState_SetMode34` `0x004762b0`;
  - `Render_SetStrideScale` `0x004903e0`, `Render_SetViewportSizeFromRect` `0x004903c0`.
- **Models:**
  - `Model_SetTextureScroll` `0x004760d0` (u, v at `+0x24/+0x28`, flag bit 5);
  - `Model_UpdateOncePerFrame` `0x00478fc0` (stamped with frame counter `0x0056bbd8`);
  - `ModelCache_Clear` `0x00475fa0`.
- **Lights.** `ObjectLights_SetPending` `0x00476340`.
- **Projection.** `Render_ProjectToScreenScaled` `0x004766a0` (projection, then scale and
  offset).
- **Screen rects.** `Render_AddScreenRectClipPoly` `0x00490610` (halved coordinates when
  pixel-doubled).
- **Decals.** `Decal_SetImageIndex` `0x00479c50` and `Texture_IsSoftwareOnly` `0x00479cc0` (three
  decal images always take the software upload path).
- **Software path (reference only):** `SW_PlotPoint16` `0x00498cb0`,
  `Queue_TexturedPolygonSubdiv_SW` `0x00499c40`, `Backend_GetItemCount` `0x0049a8b0`.
- **zRndr lifetime.** `zRndrShutdown` `0x0048ff60`, `zRndr_ResetState` `0x0048ff70`,
  `Render_ApplyResolutionAndNotify` `0x004a66f0`.
- **`Palette_ApplyBrightness` `0x004c8070`:** 8-bit palette brightness,
  `offset = (level&0xff)*8 - 32`, clamped per channel.

