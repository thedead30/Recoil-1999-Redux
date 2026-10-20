# zVideo - DirectDraw / Direct3D layer, surfaces, pixel formats

Narrative spec for subsystem `zvideo` (membership CLOSED, all members CONFIRMED). Per-function
detail is in [`../reference/zvideo.md`](../reference/zvideo.md). The draw-mode queues, fog and
the D3D render path are in [`render.md`](render.md), [`render_d3d.md`](render_d3d.md) and
[`render_frame.md`](render_frame.md); this file covers the rest.

**v1 role.** The remake does **not** reproduce DirectDraw or Direct3D 5. It draws through its
own modern API. This layer is specified so that three things are reproduced exactly:
- the **data** it hands to the card (surfaces, pixel formats, texture uploads, screen-rect
  fills);
- the **resolution and surface bookkeeping** that other systems read;
- the **side effects** that affect gameplay, such as the FPU state in section 1.

Tags: `CONFIRMED-BINARY` unless marked; COM and DirectX behaviour is `PLATFORM`.

## 1. FPU precision under the 3D path - `VERIFIED-ORACLE`
- **Start-up.** `Crt_SetFpuControl` `0x004c6350` sets precision control to **53 bits**
  (`app.md`).
- **Cooperative level.** `DDraw_SetCooperativeLevelAndDisplayMode` `0x004a8720` calls
  `SetCooperativeLevel` with **0x13** = `DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE`
  (bytes `6a13` re-read 2026-09-24). There is no `DDSCL_FPUSETUP` (0x800) or
  `DDSCL_FPUPRESERVE` (0x1000).
- **Oracle, 2026-09-24.**
  - Command: `ttd_verify.py --trace Recoil22 --cmds "bp 0x00426390; g; r fpcw; ..."` (hardware
    renderer).
  - Result: two separate vehicle ticks both read **`fpcw = 0x027F`**, meaning 53-bit precision,
    round-to-nearest, all exceptions masked.
  - That trace loaded `DDRAW.dll` / `D3DImm.DLL` from `Recoil Runtime Copy`, i.e. dgVoodoo2
    over `D3D11.DLL`.
- **Conclusion.** Under the setup used for every reference capture, gameplay maths runs at
  **53-bit (double) precision, round to nearest**.
  - Not established: whether the original 1999 Direct3D runtime would have switched this to
    24-bit. That does not affect the remake, whose reference is the dgVoodoo2 run.
  - For the numeric-fidelity planning decision: 53-bit x87 arithmetic matches IEEE double for
    `+ - * / sqrt`, apart from double-rounding cases when results are stored to `float`.
    `fsin`/`fcos`/`fpatan` remain x87-specific.

## 2. Bring-up and shutdown
- **Drivers.**
  - `DDraw_EnumerateGraphicsDevices` `0x004a9390` / `DDraw_EnumDriversCallback` `0x004a93d0`
    keep up to 4 drivers (`zVideo_GetDriverCount` `0x004a9900`).
  - `zVideo_SelectDriver` `0x004a7490` takes -1 for the default; `zVideo_SelectDriverByIndex`
    `0x004a8870` selects a driver record (stride 0x6EC).
  - `zVideo_GetDriverName` `0x004a7410` returns the name or `Default`, and
    `zVideo_GetDeviceGuidOrName` `0x004a9940` returns the device GUID or `GameZ`.
- **Device creation:**
  - `DDraw_CreateAndQueryInterface` `0x004a8800` and `DDraw_EnsureDeviceCreated` `0x004a7d20`;
  - `zVideo_PrepareFullscreenWindow` `0x004a6930`: no menu, `WS_EX_APPWINDOW`;
  - `DDraw_SetCooperativeLevelAndDisplayMode` `0x004a8720` and `Render_ResetDisplayMode`
    `0x004a8790`;
  - `Render_CreatePrimarySurfacesAndZBuffer` `0x004a88f0`, `DDraw_CreateSurface3` `0x004a88b0`.
- **Mode and flags:**
  - `zVideo_SetHardwareMode` `0x004a6b40` writes both `[0x00632120]` and the renderer flag
    `[0x0056bbe8]`;
  - `zVideo_SetVideoMode` `0x004a7af0`;
  - `zVideo_IsD3DDeviceCreated` `0x004a7b30`;
  - `zVideo_SetFlag0063212c` `0x004a71c0` (refused while pixel-doubled).
- **Shutdown and recovery:**
  - `zVideo_Close` `0x004a7740` (also shows the cursor again);
  - `DDraw_ReleaseDevice` `0x004a7d40`, `Render_ShutdownDirectDraw` `0x004a9300`;
  - `Render_ReleaseDeviceInterfaces` `0x004a91b0` releases the COM interfaces in a fixed order;
    `Thunk_Render_ReleaseDeviceInterfaces` `0x004a7520` jumps to it;
  - `zVideo_RestoreAllSurfaces` `0x004a90e0`, `zVideo_QueryObject28` `0x004a9160`,
    `zVideo_ProbeSurfacesLockable` `0x004a9060`.
- **Errors.** `zVideo_ReportError` `0x004ad6a0` maps about 140 DirectDraw/Direct3D HRESULTs to
  names.

## 3. Surfaces and resolution bookkeeping
Three surface records: **0x00632200** (render size), **0x00632220** (surface size) and
**0x00632240** (third).

- **Resolution.** The pixel-doubled presets render at 320 wide (height 200 or 240) onto a
  640-wide surface.
  - `Render_GetScreenWidth` / `Height` `0x004a6720/0x004a6730` return the render size;
    `Render_GetSurfaceWidthCopy` / `HeightCopy` `0x004a6800/0x004a6810` the surface size.
  - Getters for the other globals: `zVideo_GetGlobal_00632210/08/14/30/28`
    `0x004a6710/0x004a6740/0x004a67e0/0x004a67f0/0x004a6820`, and
    `zVideo_SetGlobal_00632138/0063213c/006321cc` `0x004a6b60/0x004a6b70/0x004a6b80`.
  - `zVideo_GetSurfaceRect` `0x004a7200`, `Video_GetSurfaceInfo` `0x004903f0`,
    `zVideo_CacheClientRectScreen` `0x004a7700`, `zVideo_ClampValueReturnDelta` `0x004a69c0`.
- **Resize notifications.** `zVideo_NotifyScreenResize` `0x004a6770` (render size) and
  `zVideo_NotifyScreenResizeFromSurface` `0x004a6840` (surface size) tell the rasteriser, the
  raster buffers and the UI.
- **Installed pointers** `[0x006333c0/c4/c8]` are reached through tail thunks, each passing one
  of the three records: `zVideo_CallSurfaceSlot3C0_Back` `0x004a67d0`, `..._Front` `0x004a68d0`,
  `..._Record240` `0x004a68f0`, `zVideo_CallSurfaceSlot3C4_Record240` `0x004a68e0`,
  `zVideo_CallSurfaceSlot3C8` `0x004a6830`.
- **Lock and unlock.**
  - `zVideo_LockSurfaceRecord` `0x004a7fc0` and `zVideo_UnlockSurfaceRecordIfLocked`
    `0x004a8030`.
  - `zVideo_LockWithRestore` / `_B` `0x004a8060/0x004a8100` and `zVideo_UnlockWithRestore` /
    `_B` `0x004a80c0/0x004a8160` retry after `Restore` on `DDERR_SURFACELOST`.
- **Blits and copies:**
  - `zVideo_BltRecord0ToRecord1` / `1ToRecord0` `0x004a7d90/0x004a7dd0`;
  - `zVideo_BlitImageToBackBuffer` `0x004a69e0` and `...Ex` `0x004a7e10` (clipped image blits:
    the 2D path for menus and HUD bitmaps);
  - `zVideo_CaptureSurfaceToImage` `0x004a6e80` and `zVideo_CopySurfaceRectToImage`
    `0x004a6fe0`.

## 4. Pixel formats (16-bit)
- **Setting the formats.** `zVideo_SetScreenPixelFormat` `0x004a6bf0` and
  `zVideo_SetTexturePixelFormat` `0x004a6db0` store the masks and derive the channel shifts.
  They are read back by `Pixfmt_GetFormat` `0x004a6b90` and `Pixfmt_GetChannelShifts`
  `0x004a6bd0`.
- **Packing.**
  - `Pixel_FromColorRef` `0x004a6ca0` converts `0x00BBGGRR` →
    `(G&mG)<<sG | (R&mR)<<sR | B>>sB`.
  - `Pixel_PackRGB` `0x004a6d40` takes three `ftol`'d channels.
  - This is the exact colour quantisation that 2D fills and fades use; reproduce it bit for bit.
- **Palette.** `zVideo_SetPaletteEntries` `0x004a9890` is used only at 8-bit depth.

## 5. Textures and screen effects
- **Texture surfaces:**
  - `DDraw_CreateTextureSurface` `0x004a83d0`;
  - `Texture_UploadPixels` `0x004a8500` (16-bit source, lock with `DDLOCK_WAIT`);
  - `D3DTextureBinding_UploadImageToSurface` `0x004aa600`;
  - `D3DTextureBinding_ConvertImageToArgb16` `0x004aa6f0`, which merges the alpha plane;
  - `D3DTextureBinding_GetSize` `0x004aa8b0`.
- **Screen fills.** `Render_QueueScreenRect` `0x004acd00` queues a coloured screen rect (fades
  and flashes) and `Render_FillVertexScratchColour` `0x004accc0` fills the scratch vertices.
- **`Screen_RandomDissolve` `0x0048d910`** is a random-pixel dissolve of a rect, a no-op below
  t = 1/256.
- **D3D scene brackets.** `FUN_004a74d0` / `FUN_004a9b40` `0x004a74d0/0x004a9b40` hold the
  ref-counted scene begin/end on `IDirect3DDevice2` (see `render_d3d.md`).

## Open
- The caller-side meaning of the unnamed globals `0x00632208/10/14/28/30`.
- Which of the three surface records each installed slot function receives at run time (the
  slots are installed per driver).
