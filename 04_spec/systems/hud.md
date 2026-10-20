# hud - in-game HUD (Battlesport hud.cpp) (spec)

Gate OPEN 2026-09-23, 88 members. Key facts: popup slide states 1/2/3 (Hud_PopupTick 0x00411ac0); loading checkpoint table 19 stages scaled by 0.018621974 (0x00414210); health bar colour switch at 0.25; radar open 3.0 s; target markers max 32 with edge arrows; lock-on radius [0x004ea5cc]; timer h:m:s split; scoreboard max 8 x 0x50; config sections FONTS..MODES (Hud_Init 0x00410160).

All rows CONFIRMED-BINARY at the cited address (bytes read; method in ledger).

| addr | name | notes |
|---|---|---|
| 0x0040e590 | Scoreboard_AddEntry | Builds 0x50-byte entry {id=arg[0], +8 = arg[2], name strncpy 0x3f from arg+0x24}. Vector {begin +0x80, end +0x84, cap +0x88} (accessed via base register). If full: grows to max(2*n,1) (operator new n*0x50), copies old en |
| 0x0040e800 | Scoreboard_UpdateEntry | Entries [+0x80,+0x84) stride 0x50, at most 8. Finds entry whose id [0] == arg[0]: entry+0x4c = arg[2], +0x44 = arg[3], +0x48 = arg[4] when [0x004f3128] nonzero else -1; then Scoreboard_Refresh (0x0040e140). Also refreshe |
| 0x0040e880 | Scoreboard_RemoveEntry | Entries [+0x80,+0x84) stride 0x50, max 8. On id match (entry[0] == arg[0]) shifts later entries down one (20-dword copies) and end -= 0x50; always ends with Scoreboard_Refresh. ret 4. |
| 0x0040eae0 | HudPanel_ForwardUpdate | [0x004ed4e0] vtable+0x24(arg); ret 4. |
| 0x0040eb00 | HudItem_LayoutFromConfig | On item [0x004e6dac]: places image with ConfigValue_PlaceImage at ([0x004e61f0],[0x004e61f4]) into item+0xc; reads its position (sub +0x1c vtable+0x64/+0x68) and places text widget (ConfigValue_PlaceTextWidget); text box |
| 0x0040ec90 | HudItem_NopOnSub | ECX=[0x004e6dac]+0x1c; tail-jumps Hud_Nop (0x00413eb0, ret only). |
| 0x0040eca0 | HudTimer_SetPaused | [[0x004ea654]+0x2a8] = (ECX == 0). |
| 0x0040ecc0 | HudTimer_SetTime | [[0x004ea654]+0x2a4] = float arg; ret 4. |
| 0x0040ece0 | HudTimer_SetLimit | [[0x004ea654]+0x2ac] = ftol(float arg2); then 0x0040ee60(arg1) with ECX=[0x004ea654]; ret 8. |
| 0x0040ed10 | HudTimer_GetTime | Returns float [[0x004ea654]+0x2a4] on FPU. |
| 0x0040ee60 | HudTimer_SetSeconds | this+0x2a4 = t. h = floor(t * (1/3600) @0x004ce744); rem = t - h*3600.0 (@0x004ce748); m = floor(rem * (1/60) @0x004ce74c); s = floor(rem - m*60.0 (@0x004ce750)); HudLabel_SetTriple(h, m, s) (0x0040ef00). ret 4. |
| 0x0040f070 | HudModeIcon_InitFromConfig | Config value arg (type 4 else 0): loads 3 images (0x0046d900) from data +0xc/+0x14/+0x1c into +0xbc/+0xc0/+0xc4; offsets +0xd8 = data+0x24, +0xdc = data+0x2c; HudModeIcon_Layout; image +0xbc applied (0x004b3e70); registe |
| 0x0040f0f0 | HudTriple_ReleaseImages | Frees images +0xbc, +0xc0, +0xc4 via 0x0046d5a0 and zeroes them. |
| 0x0040f130 | HudModeIcon_Layout | Moves self (vtable+0xc) to ([0x004e61f0]+x +0xd8, [0x004e61f4]+y +0xdc); rect +0xc8..+0xd4 = {x, y, x+img.w, y+img.h} with img [this+0xbc] shorts +4/+6. |
| 0x0040f1a0 | HudModeIcon_Select | Icon slots at 0x004ea660 + i*0xe0. If EDX (state) == 2: resets previously selected slot [0x004ea65c] to its image slot+0xc0 (0x004b3e70) and selects ECX. Then sets slot ECX image to slot+0xbc+EDX*4. |
| 0x0040f2e0 | HudTriple_LayoutFromConfig | Origin = screen object 0x004ed8cc vtable+0x64/+0x68, half width [0x004e61f0]/2. Reads rect from config (ConfigValue_GetRectOffset on data+8), places three images with ConfigValue_PlaceImage at label widgets 0x004e632c (d |
| 0x0040f3e0 | HudLabels_Nop3 | Hud_Nop on ECX=0x004e632c, 0x004e63e8, 0x004e64a4 (last as tail jump). |
| 0x0040f460 | HudPips_SetCount | If count +0x34 differs: clamp arg to [0,4], store, then for 3 pip widgets at +0x3c (stride 0xbc) show (vtable+0x60(1)) those with index < count else hide; then self vtable+0x20. ret 4. |
| 0x0040fb90 | HUD_BuildTimerSaveRecord | recoil_re_log.md:1458;recoil_confirmed_logic.md:793;ROADMAP.md:318;GOAL_MAP1.md:287 |
| 0x0040fbd0 | Hud_Shutdown | Releases all HUD resources: Hud_Nop x3, 8 image frees (0x0046d5a0), 0x0040f3e0, 0x0040ec90; HudScoreImages_Release per player slot 0x004eae34..<0x004ed4e0 (stride 0x44c); 0x0040f0f0 on 0x004ea660 (range 0x004ea660..0x004 |
| 0x0040fdd0 | HudPanel23_Destruct | SEH frame. Array-destructs 0x17 elements of 0x2a4 bytes at this+0x20 via 0x004c5ec0 with element dtor 0x0040bef0 (pushed imm; decomp shows thunk), then base dtor 0x004bc7b0. |
| 0x0040fe30 | HudItem_Destruct | SEH frame. Sets vtable of sub-object this+0x37c to 0x004cca10; destroys sub-object this+0xd8 via 0x004bab40; then 0x004b3d50 on this+0x1c. |
| 0x0040fe90 | HudPanel4_Destruct | SEH frame. Array-destructs 4 elements of 0x2a4 at this+0x10 with element dtor 0x004bab40, then base dtor 0x004bc7b0. |
| 0x0040fef0 | HudPanel4b_Destruct | SEH frame. Array-destructs 4 x 0x2a4 elements at this+0x10 with element dtor 0x004bab40, then base dtor 0x004bc7b0 (second 4-panel class; same body shape as 0x0040fe90, different unwind table 0x004c9298). |
| 0x00410140 | Hud_ConsumeCounter_004e61bc | If [0x004e61bc] nonzero decrements it and returns 1, else 0. |
| 0x00410160 | Hud_Init | Opens HUD config via Settings_OpenDetailPresetFile (fail: Failed to read, hud.cpp line 0x60d). Image dir ..\data\common\image via 0x0046ebd0. Reads sections (keys read from bytes): FONTS, OBJ_SUMMARY, OBJ_DESCRIPTION, ST |
| 0x00410fe0 | Hud_Tick | Per-frame HUD update (dt = [0x0056b42c]). Object [0x004e5ee8] vtable+0xc. If HUD disabled [0x004e5ed4]==0: when [0x004e6984] ticks [0x004e66f4]/[0x004e6844] (vtable+4); [0x004ea654] vtable+0x24(dt). Else if countdown act |
| 0x00411170 | Hud_WorldToRadarNDC | Projects world point to screen via Render_ProjectWorldPointToScreen (ECX in); result 0x10 (off-screen) returns 1. Else screen pt EDX: doubled when 0x00408380() (half-res); x = (x - W*0.5)/(W*0.5) with W [0x004e61d0]; y = |
| 0x00411720 | Hud_GetVec3_004e61f8 | Copies 3 dwords [0x004e61f8..0x004e6200] to ECX. |
| 0x00411740 | Hud_SetGlobal_004e6230 | bytes read 0x00411740 len 7 |
| 0x00411750 | Hud_ForwardToGlobal_004e62f0 | Pushes caller ECX as arg and calls 0x0040f460 with ECX=0x004e62f0. |
| 0x00411760 | Hud_SetRadarVisible | ECX!=0: shows objects [0x004e66f8] (vtable ptr) and embedded 0x004e66fc via vtable+0x60(1); w = ftol([0x004e6740]) - ftol(ceil(0.0)); [0x004e683c]=0, [0x004e6840]=1, [0x004e6734] = [0x004e6758] = (float)w. ECX==0: hides  |
| 0x004117f0 | Hud_RadarOpenAnimate | t [0x004e683c] += frame dt [0x0056b42c]. If t >= 3.0 (0x004ce76c): w = ftol([0x004e6740]) - ftol(ceil([0x004e6834])), done flag [0x004e6840]=0. Else w uses ceil([0x004e6834] * t * 0.333333 (0x004ce770)). [0x004e6734] = [ |
| 0x004118b0 | Hud_LayoutFromFontHeight | h = object 0x004e657c vtable+0x64 (int). a = h + 5 (float const -5.0 at 0x004ce774 subtracted), b = a + 7 (-7.0 at 0x004ce778): [0x004e6730]=[0x004e673c]=a, [0x004e6748]=[0x004e6754]=b. |
| 0x00411900 | Hud_ShowPopup | RENAMED from Hud_StartCountdown: opens the timed popup/message box (state machine ticked by Hud_PopupTick 0x00411ac0). ret 8. Only if EDX and arg1 (hold time) nonzero and [0x004e6984]==0: set text on [0x004e66f4] and [0x |
| 0x00411a20 | Hud_ClosePopup | RENAMED from Hud_StopCountdown: begins closing the popup. Only when [0x004e6984]==0. State 2 (open): [0x004e6560]=1, state 3, elapsed 0.0, hides [0x004e66f4], [0x004e6844], 0x004e6638. State 1 (opening): elapsed = slide  |
| 0x00411ac0 | Hud_PopupTick | Timed popup/message-box slide state machine. elapsed [0x004e6568] += dt [0x0056b42c]. State [0x004e6564] 1 (opening): if elapsed >= slide time [0x004e656c]: y [0x004e6894]=[0x004e68a0] = base [0x004e6888] + travel [0x004 |
| 0x00411f10 | Hud_SetHealthBar | Fraction arg clamped to [0,1] (consts 1.0 @0x004ce764, 0.0 @0x004ce75c). Colour via 0x004a6cf0(cl=0xff, dl, push 0) stored as u16 at [0x004e6dac]+0x4b0: dl=0x00 when fraction >= 0.25 (0x004ce77c), dl=0xff below. Bar sub- |
| 0x00412050 | Hud_PrintToConsole_004e6a98 | Object [0x004e6a98]: vtable+0x74(obj, string 0x004dacbc) (cdecl, 3 words cleaned) then vtable+0x78 thiscall. |
| 0x00412070 | Hud_AddTargetMarker | Adds a marker for target ECX (world object EDX) if count [0x004e6dc8] < 0x20 (cmp eax,0x20; jb), else returns 0; slot = 0x004e6dcc + n*0x1c0, count++. Projects via Render_ProjectWorldPointToScreen, moves marker (vtable+0 |
| 0x004122c0 | Hud_PickTargetMarker | Scans active markers (count [0x004e6dc8], array 0x004e6dcc stride 0x1c0) whose hidden flag +0x34 is 0; picks the one with smallest squared screen distance to crosshair ([0x004e621c],[0x004e6220]) from marker vtable+0x64/ |
| 0x004124b0 | Hud_UpdateTargetHealthBar | If ECX nonzero clears target [0x004e6af0]. When enabled [0x004e5ed4], no countdown ([0x004e6564]==0) and a target: target type +0x38->[0]: 2 (vehicle) health = [[t+4]+0xf34], max = [[[t+4]+4]+0x398]; 3 (object) health +0 |
| 0x00412620 | Hud_HideIfOwner | If [0x004e6af0] non-null and its +0x38 -> +4 equals ECX, calls vtable+0x60(0) on the embedded object at 0x004e6c6c. |
| 0x00412650 | HudPlayerSlot_SetTimerText | Slot = ECX (index, 0x44c stride from 0x004ea9e8); only if EDX == slot id [0x004ead6c + ECX*0x44c]. If value == 123456792.0f sentinel (0x004ce784) shows glyph string 0x004dae08 (byte 0xa5); else shows ceil(value) with %d  |
| 0x004126e0 | HudPlayerSlot_SetMode | Slot = 0x004ea9e8 + ECX*0x44c, mode EDX. 0x004b3e70(slot+0xbc+mode*4) on slot. Mode 0/3: slot+0xd0 = slot+0xd8, 0x004b3e70(slot+0xd4) on slot+0x390, side flag slot+0x384 = 0. Mode 5: flag = 0. Mode 1/4: slot+0xd4 = slot+ |
| 0x00412790 | HudPlayerSlot_SelectEntry | Slot base = 0x004ea9e8 + ECX*0x44c; slot+0xd0 = slot+0xd8+EDX*4 entry; calls 0x004b3e70(entry) on sub-object slot+0x390; masks byte slot+0x39c to bit 0x10 (dword write). |
| 0x004127d0 | HudPlayerSlot_Clear | Slot = 0x004ea9e8 + ECX*0x44c: 0x004b3e70(0) on slot and on slot+0x390; slot+0xe0 object vtable+0x74(obj, 0x004e5ce0); slot vtable+0x20. |
| 0x00412820 | HudPlayerSlot_SetActiveTimer | ECX=slot index, EDX=mode, arg=time (ret 4). Current active slot/mode [0x004ea9e0]/[0x004ea9e4]. Index <= 1 (cmp ebx,1): index 1 applies inlined HudPlayerSlot_SetMode logic to slot 1 (modes 0/3, 5, 1/4, 6 per bytes; decom |
| 0x00412b60 | HudContainer_Construct | SEH frame. Base ctor 0x004bc780; 0x004b3d00(0) on member this+0x30; vtable 0x004ce988; registers member via 0x004bc7c0(this+0x30); final vtable 0x004ce968. Returns this. |
| 0x00412c10 | HudRect_LoadFromConfig | Looks up key TYPEI (0x004dae0c; string as read) in config arg via ConfigTree_FindChildByName_Vec3; if found parses into this+0x10 via 0x00413a10(ECX=child+8, EDX=this+0x10, 0,0,0) and copies +0x10..+0x1c to +0x20..+0x2c. |
| 0x00412db0 | Hud_ApplyViewportRect | ECX=rect {x0,y0,x1,y1}. half = 0x00408380(); viewport origin via 0x004085e0(ECX=x0,EDX=y0), halved (sar 1) when half; 0x00408530; size w=x1-x0, h=y1-y0 (halved when half) -> 0x00408620(ECX=w,EDX=h), 0x00408500 with float |
| 0x00412ea0 | HudScorePanel_Construct | SEH frame. Base ctor 0x004bc780; members constructed via 0x004b3d00(0) at +0x30, +0xec, +0x1b4, +0x27c; intermediate vtable 0x004ce988 registering +0x30 (0x004bc7c0); final vtable 0x004ce9a8 then registers +0xec, +0x27c, |
| 0x00412f70 | HudScorePanel_LoadFromConfig | Finds key TYPEII (0x004dae14) in config arg; if found: rect via ConfigValue_GetRectOffset into this+0x10, copied to +0x20; sub-panels via 0x00413d30 from child +0x10 into this+0xec (offset 0), child +0x18 into this+0x27c |
| 0x00413080 | HudImages_Release | Frees image objects +0x1ac, +0x1b0, +0x274, +0x278 via 0x0046d5a0 (ECX=each) and zeroes the four pointers. |
| 0x004132b0 | HudMessage_LayoutRect | Builds rect for this+0x1b4: left = font [0x004e657c] vtable+0x64 + short [0x004e65b8]+4 (0 if null); top = [0x004e6578]; right = font vtable+0x68 + short [0x004e65b8]+6; 0x004b3e90(rect) on this+0x1b4; then 0x004b4180 an |
| 0x00413540 | Hud_ResetAllVisibility | 0x004bc8d0(0xe) on ECX=0x004e5ed0 and on this; masks visibility dwords to bit 0x10: this+0x288, [0x004e6588], [0x004e6708], [[0x004e66f8]+0xc], [0x004e6bbc], and for player slots 0x004eb1d0.. <0x004ed87c stride 0x44c whe |
| 0x00413700 | Hud_StaticInit_ConstructWnd | ECX=0x004e5e90; jmp CWnd::CWnd (global CWnd instance). |
| 0x00413710 | Hud_StaticInit_RegisterDtor_004e5e90 | atexit-style 0x004c60e0(0x00413720). |
| 0x00413770 | Hud_SetOverlayVisible | Object [0x004ea658] vtable+0x60(ECX!=0); when hiding also calls 0x00413630. |
| 0x004137a0 | Hud_ShowPanel_004e6db0 | Calls vtable+4 of object [0x004e6db0] with 1 if ECX nonzero else 0. |
| 0x004137c0 | Hud_ResetKeyBindings | For i in 0..0x16 (23): 0x004137f0(ECX=2, EDX=i, push 0x004e5ce0) then 0x004137f0(ECX=0, EDX=i, push 0). |
| 0x004137f0 | Hud_SetMessageLine | Line EDX in panel array [0x004e6db0]+0x20 (0x2a4 stride). Mode ECX=1: set text arg vtable+0x74 and show vtable+0x60(1). ECX=0: hide. ECX=2: if text non-empty set+show else hide. ret 4. |
| 0x004138f0 | Hud_ForwardIfActive_0056bd20 | If [[0x0056bd20]+4] nonzero, calls 0x004bd160 with ECX=[0x0056bd20], args (caller ECX, arg1); ret 4. |
| 0x00413950 | Hud_HideBothPanels | For panels [0x0056bd24] then [0x0056bd20]: 0x004bd2a0 (ECX=panel) then panel vtable+4(0). |
| 0x00413990 | ConfigValue_PlaceTextWidget | ECX=config value (type 4 required else 0; ret 0xc), EDX=widget. x = data+0x14 + arg1, y = data+0x1c + arg2, plus optional offset arg3[0]/[1]; widget vtable+0xc(x,y); then widget vtable+0x74(widget, text) with text = data |
| 0x00413a10 | ConfigValue_GetRectOffset | ECX=config value (type 4 else 0; ret 0xc). EDX rect = data +0xc/+0x14/+0x1c/+0x24; optional arg1 offset added (x to [0],[2], y to [1],[3]); optional arg2 = width ([2]-[0]), arg3 = height ([3]-[1]). Returns 1. |
| 0x00413aa0 | ConfigValue_GetRect4 | ECX=config value: if type +0 != 4 returns 0; else from data [ECX+4] copies +0xc,+0x1c,+0x14,+0x24 into EDX[0],[1],[2],[3]; returns 1. |
| 0x00413ad0 | ConfigValue_GetVec3Opt | ECX=config value; type +0 must be 4 else 0 (ret 8). From data [ECX+4]: +0xc -> *EDX, +0x14 -> *arg1, +0x1c -> *arg2, each skipped when pointer null. Returns 1. |
| 0x00413b10 | ConfigValue_SetQuadCorners | ECX=config value (type 4 else 0; ret 8), EDX=quad object. Rect from data +0xc/+0x14/+0x1c/+0x24 plus optional offset arg1; sets corners via 0x004bcf80 on EDX: 0=(l,t), 1=(l,b), 2=(r,b), 3=(r,t) as floats; optional arg2 r |
| 0x00413c10 | ConfigValue_SetQuadRectInclusive | ECX=config value (type 4 else 0; ret 0x10), EDX=quad. Rect l=+0xc, t=+0x14, r=+0x1c+1, b=+0x24+1; optional out arg4 receives it. Offsets: r += arg1, t and b += arg2, plus optional arg3 (x,y). Corners via 0x004bcf80: 0=(l |
| 0x00413d30 | ConfigValue_PlaceImage | ECX=config value (type 4 else 0), EDX=widget; 5 stack args (ret 0x14): x, y, optional offset, image, out-rect. Position = data+0x14 + x, data+0x1c + y (+ offset). If value count +4 == 6: fields +0x24 and +0x2c compared t |
| 0x00413eb0 | Hud_Nop | bytes read 0x00413eb0 len 1 (c3) |
| 0x00413ec0 | HudStatusBox_Init | ECX=status box; arg1 config value (type 4 else 0), arg2 rect. Loads 7 images via 0x0046d900 into +0xbc,+0xc0,+0xc4,+0xc8,+0xcc,+0xd8,+0xdc; position +0x388/+0x38c = data +0x44/+0x4c; HudStatusBox_Layout; +0x40 (word)=1;  |
| 0x00413ff0 | HudScoreImages_Release | Frees image objects +0xbc, +0xc0, +0xc4, +0xc8, +0xcc, +0xd8, +0xdc via 0x0046d5a0 and zeroes them. |
| 0x00414070 | HudStatusBox_Layout | Screen origin ox/oy = object 0x004ed8cc vtable+0x64/+0x68. x0 = this+0x388 + [0x004e61f0]/2, y0 = this+0x38c; frame rect {x0,y0, x0+w, y0+h} with w,h = shorts +4/+6 of image [this+0xbc]. Moves self (vtable+0xc) to (x0+ox |
| 0x00414180 | Loading_Checkpoint | Loading-progress step used between level load/unload stages. If cursor [0x004ed5c8] > max [0x004ed5c4] logs Checkpoint overflow (hud.cpp line 0x1184); else progress [0x004ed5d0] = float table [0x004ed560 + cursor*4], cur |
| 0x00414210 | Loading_InitCheckpointTable | Checkpoint count [0x004ed5c4]=0x12 (18), cursor [0x004ed5c8]=0. Raw stage times table at 0x004ed4fc (19 floats): 0.001, 0.137, 0.237, 0.34, 0.9, 9.3, 12.4, 13.4, 20.0, 26.0, 26.3, 28.7, 31.5, 34.0, 36.2, 36.4, 53.3, 53.6 |
| 0x00414300 | Hud_GetRect_004edb58 | Copies 4 dwords [0x004edb58..0x004edb64] to ECX. |
| 0x00414330 | Hud_ShowKillMessage | Name text = EDX+0x160 string (empty 0x004e5ce0 if null); if EDX null uses message 0x253 via 0x004a5bf0. sprintf fmt 0x004dae34 (three %s: ECX+0x24, text, arg+0x24) and shows via 0x004138d0(2.0f) with ECX=buffer. ret 4. |
| 0x00414390 | Hud_ForwardToPanel_0040e590 | ECX=[[0x004ed4e0]+0x34]; pushes caller ECX; calls 0x0040e590. |
| 0x004143b0 | Hud_ForwardToPanel_0040e800 | ECX=[[0x004ed4e0]+0x34]; pushes caller ECX; calls 0x0040e800. |
| 0x004143c0 | Hud_ForwardToPanel_0040e880 | ECX=[[0x004ed4e0]+0x34]; pushes caller ECX; calls 0x0040e880. |
| 0x00414670 | Array50_Count | ECX=vector {+4 begin,+8 end}; returns (end-begin)/0x50 (magic 0x66666667, sar 5) or 0 when begin null. |
| 0x004146a0 | Array50_CopyRange | stdcall(first,last,dest) ret 0xc: copies 0x50-byte elements [first,last) to dest (skipping writes when dest null), returns dest end. |
| 0x004146e0 | Array50_FillCopy | stdcall(dst,count,src) ret 0xc: for count elements of 0x50 bytes, copies 20 dwords from src into each non-null dst slot. |
| 0x00414a70 | Hud_StaticInit_Construct_004edb78 | ECX=0x004edb78; jmp 0x00414ab0 (constructor). |
| 0x00414a80 | Hud_StaticInit_RegisterDtor_004edb78 | atexit-style 0x004c60e0(0x00414a90). |
| 0x00414ab0 | InterpZbd_Construct | Base init 0x004c0d20(ECX=this, [0x004e48f4] interp.zbd name, ext .zbd 0x004dae4c); vtable = 0x004ce9c8; returns this. |

## Function index additions (2026-09-24)
- **`Loading_SetProgress` `0x00404c50`** calls the progress object's `vfunc+0x84(arg)`, then
  `Sleep(100)`. So each loading-progress update costs 100 ms, as found.
- **`ByteSet_Intersects` `0x00476370`** tests two zone sets (count-prefixed bytes):
  - it is true when the feature is off (`[0x004dd90c] == 0`), when either set is empty, when
    either contains `0xff`, or when they share an element;
  - it is used for target-marker visibility.

## Vtable-only methods (added 2026-09-25, G1; condensed from ledger)

Descriptions are condensed from the ledger notes; full text is in [`../reference/hud.md`](../reference/hud.md).

### VMethod
- `VMethod_CallSlot1_False_00412c00` `0x00412c00`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): this->vfun...
- `VMethod_CallSlot1_False_004135f0` `0x004135f0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): this->vfun...
- `VMethod_CallSlot1_True_00412bf0` `0x00412bf0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): this->vfun...
- `VMethod_ReturnFalse4_00414b50` `0x00414b50`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return 0, ...
- `VMethod_ReturnTrue_00412bd0` `0x00412bd0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return 1, ...

### HudPanel
- `HudPanel_Layout_Full_004130d0` `0x004130d0`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw): (flag) ret 4: same width/height setup a...
- `HudPanel_Layout_Split_00412c60` `0x00412c60`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw): (f) ret 4: SW: 0x00490600; [this+0x28]=...
- `HudPanel_Reset_SelectLayoutByWidth_00413340` `0x00413340`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw): 0x004bc760(!pixel-double); 0x004bc8d0(0...

### Hud
- `Hud_DrawOverlay_00413500` `0x00413500`: vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw/0x00413500_*.c): if HUD enabled [0x004e5e...
- `Hud_ForwardTo_004bc900` `0x00412be0`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x004bc900...

### Thunk
- `Thunk_00414590` `0x00414660`: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): jmp 0x0041...

