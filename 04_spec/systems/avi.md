# AVI / FMV playback (`zFMV/fmv_str.cpp`)

**Status:** gate **OPEN** — 4/4 CONFIRMED, membership CLOSED (decompile reads). Player object
open/close (`AviPlayer_Open` `0x00463d50`, `AviPlayer_Close` `0x00463dd0`) are recorded under
menus. Video for Windows names and codes are `PLATFORM`; everything else `CONFIRMED-BINARY`.

## Open video — `0x00463ef0`
- Opens the `vids` stream of the file at `[+0x38]`; decoder = `ICLocate('vidc', handler, src, dst,
  decompress)`, then `ICM_DECOMPRESS_BEGIN`.
- Output format: bit depth = current screen depth (`0x004a66e0`); BI_BITFIELDS with the screen's
  masks, or BI_RGB when 24-bit; **top-down** (height negated); rows aligned to 4 bytes.
- Frame rate `[+0xEC]` = rate / scale (integer); frame time `[+0xF0]` = rounded ms per frame.
- Every failure reports through `0x00404e80` and calls `AVIFileExit`.

## Open sound — `0x004641a0`
- `auds` stream; missing stream = silent (no error).
- Mode A (`[+0x1E0]` = 0): whole stream read into one sound buffer.
- Mode B: streaming — buffer twice the suggested chunk size, refilled in halves.

## Per frame — `AviPlayer_DecodeFrame` `0x004643a0`
- Reads one sample, `ICDecompress` into the frame surface under a critical section.
- **Loops** to frame 0 at the end.
- The first decoded frame starts the sound at volume 1.0.
- Streaming refill follows the play cursor: when it passes the midpoint, the first half is refilled;
  when it wraps, the second half.

## Defects to reproduce
- Decompress failure returns while the critical section is still held.
- `AviPlayer_FillSoundChunk` advances the read position by the second length for both halves.
- All `calloc`s are unchecked.

## Function index (added 2026-09-24, condensed from ledger notes)

Descriptions are condensed from the ledger notes; full text is in [`../reference/avi.md`](../reference/avi.md).

### Seq
- `Seq_AppendItem` `0x00462f10`: (item) ret 4: null -> 0; item next [+4]=0; append to list head [+0x14]/tail [+0x18] (cursor [+0x1c] = first); ...
- `Seq_Begin` `0x00462f90`: (t lo,hi double) ret 8: no first item [+0x1c] -> 0; screen setup: w=0x004a6820, h=0x004a6810(w), 0x004a6800(h)...
- `Seq_BeginAtNow` `0x004630a0`: 0x00462f90(timeGetTime()*0.001 as double) (const 0x004d2580)
- `Seq_Ctor` `0x004625e0`: (file, section, hwnd) ret 0xc: hwnd default [0x004f3eec]; [1]=hwnd, [0]=0, [4]=1, zero [5..7]; file and sectio...
- `Seq_Dtor` `0x00462630`: free [0] buffer; Seq_ResetOrClear(1) (delete all items)
- `Seq_Finish` `0x00463120`: 0x00462660(arg); ret 4
- `Seq_Load` `0x004626b0`: (file, section) ret 8: parse fmv.zrd (ConfigTree_ParseFileByBasename 0x0048cdc0) fail -> error fmv_script.cpp ...
- `Seq_Play` `0x00462f50`: (arg) ret 4: [this+0x10]=arg; Seq_BeginAtNow; while Seq_TickAtNow() repeat; Seq_Finish(0); return 1
- `Seq_ResetOrClear` `0x00462660`: (free) ret 4: free==0 -> cursor [+0x1c]=first [+0x14]; else delete every item (vfunc+0(1) scalar dtor, next +4...
- `Seq_RunBlocking` `0x00462e30`: vfunc+8(0,0) begin; while vfunc+4(0,0) != 0 repeat; vfunc+0xc() end
- `Seq_Tick` `0x00463000`: (t double) ret 8: no current item [+0x1c] -> 0; if skippable [+0x10]: key pressed (Input_Keyboard_WaitForKeyPr...
- `Seq_TickAtNow` `0x004630e0`: return 0x00463000(timeGetTime()*0.001)

### SeqAvi
- `SeqAvi_Begin` `0x00463790`: (a,b) ret 8: new(0x1e4) AviPlayer_Open(file [+8], flags [+0xc]) -> [+0x10]; zero +0x24/+0x28; Video_GetSurface...
- `SeqBlur_Ctor` `0x00463850`: (a,b) ret 8: [1]=0, vtbl 0x004d25e0, [2]=a, [3]=b
- `SeqAvi_Ctor` `0x00463570`: (dir,file,flags) ret 0xc: vtbl 0x004d25c8; path=calloc(len+0x19); sprintf fmt 0x004dd484 (dir,file); [3]=flags...
- `SeqAvi_FreePlayer` `0x00463820`: if player [this+0x10]: AviPlayer_Close (0x00463dd0) + delete; clear
- `SeqAvi_SetupScreen` `0x00463870`: zero +0x10/+0x14/+0x20/+0x24; [+0x18]=Render_GetScreenWidth, [+0x1c]=Render_GetScreenHeight; [+0x28]=0x004a680...

### SeqItem
- `SeqItem_FreeImage` `0x004633a0`: if [this+0xc]: 0x0046d5a0, clear
- `SeqItem_LoadImage` `0x00463300`: [this+0xc]=0x0046d900(a,b); ret 8
- `SeqItem_PlaySound` `0x00462e90`: (a,b) ret 8: [this+8]=0x004a0990(a,b) resource; stop previous voice [this+0xc] (0x0049fda0); if res: [this+0xc...
- `SeqItem_SetTextParams` `0x00463410`: (a,b) ret 8: [this+0x20]=0x004a6e80(); [+0x18]=a, [+0x1c]=b

### SeqFade
- `SeqFade_StepMode1` `0x004639e0`: identical to 0x00463950 but 0x0048ea00 mode EDX=1
- `SeqFade_StepMode2` `0x00463a70`: identical to 0x00463950 but 0x0048ea00 mode EDX=2
- `SeqFade_StepMode3` `0x00463950`: (a,b) ret 8: count [+8]--; n=[+0xc] steps: software path ([0x0056bbe8]==0): lock 0x004a6770, n x 0x0048ea00(EC...

### SeqItemB
- `SeqItemB_Dtor` `0x00463670`: vtbl 0x004d25c8; free [this+8]; vtbl base 0x004cee50
- `SeqItemB_FreeImage` `0x00463550`: if [this+0x20]: 0x0046d5a0, clear
- `SeqItemB_ScalarDeletingDtor` `0x00463650`: dtor 0x00463670; flag&1 -> delete; ret 4

### SeqItemC
- `SeqItemC_Draw` `0x00463cc0`: text surface via 0x004a68e0/0x004a6e80/0x004a68f0; if surface: clear (0x004a6840, 0x0048f500(0,0,0), 0x004a68d...
- `SeqItemC_Dtor` `0x00463c10`: vtbl 0x004d25f8; free [this+8]; delete sub-object [this+0xc] (dtor 0x00462360); vtbl base 0x004cee50
- `SeqItemC_ScalarDeletingDtor` `0x00463bf0`: dtor 0x00463c10; flag&1 -> delete; ret 4

### SeqImage
- `SeqImage_Ctor` `0x00463130`: (file,dur,x,y) ret 0x10: vtbl 0x004d2598, [2]=strdup(file), [4]=dur, globals 0x0053a728/2c = x,y, [5]=1 (posit...
- `SeqImage_CtorCentered` `0x004631f0`: (file,dur) ret 8: vtbl 0x004d2598, [2]=strdup(file), [4]=dur, globals 0x0053a708/0c = 0, [5]=0 (centred), Vide...

### SeqItemA
- `SeqItemA_Dtor` `0x004632a0`: vtbl 0x004d2598; SeqItem_FreeImage; free [this+8]; vtbl base 0x004cee50
- `SeqItemA_ScalarDeletingDtor` `0x004631d0`: dtor 0x004632a0; flag&1 -> delete; ret 4

### SeqText
- `SeqFade_Ctor` `0x004633c0`: (a..f) ret 0x18: [1]=0, vtbl 0x004d25b0, colour short [3]=0x004a6cf0(c), [9]=f, [4]=d, [2]=e
- `SeqMci_Ctor` `0x00463b00`: (font,dir,file) ret 0xc: vtbl 0x004d25f8; path=sprintf(0x004dd484, dir, file); [3]=new(0x30) text object 0x004...

### SeqBase
- `SeqBase_ScalarDeletingDtor` `0x00462e70`: base dtor 0x00415aa0 (vtbl reset); flag&1 -> delete; ret 4

## Vtable-only methods (added 2026-09-25, G1; condensed from ledger)

Descriptions are condensed from the ledger notes; full text is in [`../reference/avi.md`](../reference/avi.md).

### Seq
- `Seq_IsFinishedAt_00462ee0` `0x00462ee0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return t_a...
- `Seq_Present_00462f00` `0x00462f00`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): zVideo_Pre...
- `Seq_SetDuration_00462ed0` `0x00462ed0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): [this+0xc]...
- `Seq_SetFramebuffer_00463920` `0x00463920`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x0048d420...
- `Seq_Stop_00463ca0` `0x00463ca0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): if [this+0...

### SeqAvi
- `SeqAvi_ShowFrame_004636d0` `0x004636d0`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw): first call stores (a,b) at +0x18/+0x1c;...

### SeqFade
- `SeqFade_Draw_00463320` `0x00463320`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw/0x00463320_*.c): repeat 1 (+1 on HW) time...

### VMethod
- `VMethod_ReturnFalse8_00463c90` `0x00463c90`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return 0, ...

## Still open
- The pacing loop that calls DecodeFrame (who uses `[+0xF0]`).
