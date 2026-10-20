// SUBSYSTEM: ui_widgets
// Functions of subsystem `ui_widgets` with no attributed original source file (ledger orig_file empty); their
// placement here is a layout choice, not a provenance claim (STAGE2.md section 2). Spec: 04_spec/systems/ui_widgets.md
#include "unattributed/ui_widgets.h"
#include "unattributed/texture.h"
#include "GameZRecoil/zImage/zimg_fonts.h"
#include "GameZRecoil/zReader/zreader.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"
#include "unattributed/rasteriser.h"
#include "unattributed/settings.h"
#include "GameZRecoil/zVideo/zvid_buff.h"
#include "unattributed/menus.h"
#include "unattributed/zeffect.h"
#include "platform/mfc42.h"
#include "unattributed/hud.h"
#include "platform/iat_gdi32.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_user32.h"
#include "GameZRecoil/zSound/zsnd_cd.h"
#include "unattributed/mapscreen.h"
#include "unattributed/zvideo.h"
#include "Battlesport/ai_net.h"
#include "GameZRecoil/zClass/Camera.h"
#include "unattributed/input.h"
#include "unattributed/render_frame.h"
#include "unattributed/transform.h"
#include "GameZRecoil/zVideo/zvid_ddd3d.h"
#include "GameZRecoil/zInput/zin_kbd.h"
#include "platform/iat_mfc42.h"
#include "GameZRecoil/zSound/zsnd_parm.h"
#include "Battlesport/hud.h"
#include "unattributed/sound.h"
#include "unattributed/sysinfo.h"
#include "platform/msvc_eh.h"
#include "GameZRecoil/zFMV/fmv_stream.h"
#include "unattributed/app.h"
#include "Battlesport/mission.h"
#include "GameZRecoil/zNetwork/znet_dplay.h"
#include "unattributed/net.h"

namespace recoil {

// 0x00404e80 - CONFIRMED-BINARY: the single byte c3 (RET), NOP-padded. A naked RET keeps EAX, flags and every
// register exactly as the caller left them, as the original does.
__declspec(naked) void Debug_ReportNoop()
{
    __asm ret
}

// 0x0046f130 Font_ScanGlyphs - ../../04_spec/systems/texture.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Font_ScanGlyphs(int, int)
{
    __asm {
        sub esp, 0x8
        push ebx
        push ebp
        push esi
        push edi
        mov edi, dword ptr [ecx]
        mov eax, 0xac769185
        xor esi, esi
        lea ebx, [ecx + 0x8]
        movsx ebp, word ptr [edi + 0x4]
        imul ebp
        add edx, ebp
        mov dword ptr [esp + 0x10], 0x1
        sar edx, 0x6
        mov eax, edx
        shr eax, 0x1f
        lea edx, [edx + eax*0x1 - 0x1]
        mov dword ptr [ecx + 0x4], edx
    L_46f162:
        movsx eax, word ptr [edi + 0x4]
        cmp esi, eax
        jge L_46f1ff
        mov dword ptr [ebx + 0x4], 0x0
        mov edx, esi
        movsx ecx, word ptr [edi + 0x6]
        dec ecx
        mov dword ptr [ebx + 0xc], ecx
        mov ecx, edi
        call Image_IsColumnTransparent
        test eax, eax
        jz L_46f198
    L_46f18a:
        inc esi
        mov ecx, edi
        mov edx, esi
        call Image_IsColumnTransparent
        test eax, eax
        jnz L_46f18a
    L_46f198:
        mov edx, esi
        mov ecx, edi
        mov dword ptr [esp + 0x14], esi
        call Image_IsColumnTransparent
        test eax, eax
        jnz L_46f1b7
    L_46f1a9:
        inc esi
        mov ecx, edi
        mov edx, esi
        call Image_IsColumnTransparent
        test eax, eax
        jz L_46f1a9
    L_46f1b7:
        mov edx, esi
        mov ecx, edi
        mov ebp, esi
        call Image_IsColumnTransparent
        test eax, eax
        jz L_46f1d4
    L_46f1c6:
        inc esi
        mov ecx, edi
        mov edx, esi
        call Image_IsColumnTransparent
        test eax, eax
        jnz L_46f1c6
    L_46f1d4:
        mov eax, esi
        add ebx, 0x10
        sub eax, ebp
        cdq
        sub eax, edx
        mov edx, dword ptr [esp + 0x14]
        sar eax, 0x1
        sub esi, eax
        mov eax, dword ptr [esp + 0x10]
        inc ebp
        mov dword ptr [ebx - 0x10], edx
        mov dword ptr [ebx - 0x8], ebp
        inc eax
        cmp eax, 0x5f
        mov dword ptr [esp + 0x10], eax
        jl L_46f162
    L_46f1ff:
        mov eax, dword ptr [esp + 0x10]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret
    }
}

// 0x00404ca0 Widget_Slot_CallVirtual08 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_CallVirtual08(int, int)
{
    __asm {
        mov eax, dword ptr [ecx]
        jmp dword ptr [eax + 0x8]
    }
}

// 0x00404d10 Widget_ReturnTrue - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_ReturnTrue(int, int, int, int)
{
    __asm {
        mov AL, 0x1
        ret 0x8
    }
}

// 0x00404d50 Widget_GetX - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_GetX(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x14]
        ret
    }
}

// 0x00404d60 Widget_GetY - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_GetY(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x18]
        ret
    }
}

// 0x00404d90 ImageWidget_GetX - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_GetX(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x48]
        test eax, eax
        jz L_404dbe
        mov eax, dword ptr [ecx + 0x3c]
        test eax, eax
        jz L_404daf
        movsx eax, word ptr [eax + 0x4]
        cdq
        sub eax, edx
        mov edx, eax
        mov eax, dword ptr [ecx + 0x14]
        sar edx, 0x1
        add eax, edx
        ret
    L_404daf:
        xor eax, eax
        cdq
        sub eax, edx
        mov edx, eax
        mov eax, dword ptr [ecx + 0x14]
        sar edx, 0x1
        add eax, edx
        ret
    L_404dbe:
        mov eax, dword ptr [ecx + 0x14]
        ret
    }
}

// 0x00404dd0 ImageWidget_GetY - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_GetY(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x48]
        test eax, eax
        jz L_404dfe
        mov eax, dword ptr [ecx + 0x3c]
        test eax, eax
        jz L_404def
        movsx eax, word ptr [eax + 0x6]
        cdq
        sub eax, edx
        mov edx, eax
        mov eax, dword ptr [ecx + 0x18]
        sar edx, 0x1
        add eax, edx
        ret
    L_404def:
        xor eax, eax
        cdq
        sub eax, edx
        mov edx, eax
        mov eax, dword ptr [ecx + 0x18]
        sar edx, 0x1
        add eax, edx
        ret
    L_404dfe:
        mov eax, dword ptr [ecx + 0x18]
        ret
    }
}

// 0x00407140 Widget_ReturnZero - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_ReturnZero(int, int)
{
    __asm {
        xor eax, eax
        ret
    }
}

// 0x00407150 Widget_EmptyVirtual - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_EmptyVirtual(int, int, int)
{
    __asm {
        ret 0x4
    }
}

// 0x00409550 UiScreen_Slot_ClearOwnerADS8 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_ClearOwnerADS8(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xc8]
        mov dword ptr [eax + 0xad58], 0x0
        ret
    }
}

// 0x0040bdf0 Stub_NoOp_TwoStackArgs - ../../04_spec/systems/container.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Stub_NoOp_TwoStackArgs(int, int, int, int)
{
    __asm {
        ret 0x8
    }
}

// 0x0040bea0 TextWidget_GetFont - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextWidget_GetFont(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x154]
        ret
    }
}

// 0x0040beb0 TextWidget_SetFontHandle - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall TextWidget_SetFontHandle(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [ecx + 0x154], eax
        ret 0x4
    }
}

// 0x0040bec0 ListLabel_SetFixedRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabel_SetFixedRect(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [ecx + 0x278], 0x1
        add ecx, 0x27c
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov edx, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], edx
        mov eax, dword ptr [eax + 0xc]
        mov dword ptr [ecx + 0xc], eax
        ret 0x4
    }
}

// 0x0041a290 Widget_Slot_TailVirtual8C - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_TailVirtual8C(int, int)
{
    __asm {
        mov eax, dword ptr [ecx]
        jmp dword ptr [eax + 0x8c]
    }
}

// 0x004353e0 UiScreen_Slot_TailOwnerSlot0C - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_TailOwnerSlot0C(int, int)
{
    __asm {
        mov ecx, dword ptr [ecx + 0xc8]
        mov eax, dword ptr [ecx]
        jmp dword ptr [eax + 0xc]
    }
}

// 0x004b3da0 ImageWidget_ReleaseOwnedImage - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_ReleaseOwnedImage(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0x3c]
        test ecx, ecx
        jz L_4b3db9
        mov eax, dword ptr [esi + 0x34]
        test eax, eax
        jz L_4b3db9
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0x3c], eax
    L_4b3db9:
        mov dword ptr [esi + 0x34], 0x0
        pop esi
        ret
    }
}

// 0x004b40c0 Widget_CopyCtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_CopyCtor(int, int, int)
{
    __asm {
        mov eax, ecx
        push esi
        xor ecx, ecx
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [eax + 0x8], ecx
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [eax], offset g_RData_004cc000 + 0xa10
        mov edx, dword ptr [ecx + 0xc]
        add ecx, 0x20
        mov dword ptr [eax + 0xc], edx
        mov DX, word ptr [ecx + 0x10]
        mov word ptr [eax + 0x30], DX
        mov edx, dword ptr [ecx - 0x10]
        mov dword ptr [eax + 0x10], edx
        mov edx, dword ptr [ecx - 0xc]
        mov dword ptr [eax + 0x14], edx
        mov edx, dword ptr [ecx - 0x8]
        mov dword ptr [eax + 0x18], edx
        mov edx, dword ptr [ecx - 0x4]
        mov dword ptr [eax + 0x1c], edx
        mov esi, dword ptr [ecx]
        lea edx, [eax + 0x20]
        mov dword ptr [eax + 0x20], esi
        mov esi, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], esi
        mov esi, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], esi
        mov ecx, dword ptr [ecx + 0xc]
        pop esi
        mov dword ptr [edx + 0xc], ecx
        ret 0x4
    }
}

// 0x004b4120 Widget_Assign - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_Assign(int, int, int)
{
    __asm {
        mov eax, ecx
        push esi
        xor ecx, ecx
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [eax + 0x8], ecx
        mov ecx, dword ptr [esp + 0x8]
        mov edx, dword ptr [ecx + 0xc]
        add ecx, 0x20
        mov dword ptr [eax + 0xc], edx
        mov DX, word ptr [ecx + 0x10]
        mov word ptr [eax + 0x30], DX
        mov edx, dword ptr [ecx - 0x10]
        mov dword ptr [eax + 0x10], edx
        mov edx, dword ptr [ecx - 0xc]
        mov dword ptr [eax + 0x14], edx
        mov edx, dword ptr [ecx - 0x8]
        mov dword ptr [eax + 0x18], edx
        mov edx, dword ptr [ecx - 0x4]
        mov dword ptr [eax + 0x1c], edx
        mov esi, dword ptr [ecx]
        lea edx, [eax + 0x20]
        mov dword ptr [eax + 0x20], esi
        mov esi, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], esi
        mov esi, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], esi
        mov ecx, dword ptr [ecx + 0xc]
        pop esi
        mov dword ptr [edx + 0xc], ecx
        ret 0x4
    }
}

// 0x004b4180 Widget_OrFlagsWithGlobal - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_OrFlagsWithGlobal(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0xa870]
        mov edx, dword ptr [ecx + 0xc]
        or edx, eax
        mov dword ptr [ecx + 0xc], edx
        ret
    }
}

// 0x004b41b0 Widget_SetBoundsRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_SetBoundsRect(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        test eax, eax
        jz L_4b41d1
        mov edx, dword ptr [eax]
        add ecx, 0x20
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov edx, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], edx
        mov eax, dword ptr [eax + 0xc]
        mov dword ptr [ecx + 0xc], eax
    L_4b41d1:
        ret 0x4
    }
}

// 0x004b4280 Widget_SetLifetime - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_SetLifetime(int, int, int)
{
    __asm {
        fld dword ptr [esp + 0x4]
        fcomp dword ptr [g_RData_004cc000 + 0x74a8]
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [ecx + 0x10], eax
        fnstsw AX
        test AH, 0x1
        jnz L_4b42a3
        mov eax, dword ptr [ecx + 0xc]
        or AL, 0x1
        mov dword ptr [ecx + 0xc], eax
        ret 0x4
    L_4b42a3:
        mov edx, dword ptr [ecx + 0xc]
        and edx, 0xfffffffe
        or edx, 0x10
        mov dword ptr [ecx + 0xc], edx
        ret 0x4
    }
}

// 0x004b4410 TextBuffer_GetText - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextBuffer_GetText(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        ret
    }
}

// 0x004b4420 EditField_SetCursor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_SetCursor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        or ecx, 0xffffffff
        mov edx, dword ptr [esi + 0x4]
        xor eax, eax
        mov edi, edx
        repne scasb
        mov eax, dword ptr [esp + 0xc]
        not ecx
        dec ecx
        cmp eax, ecx
        jge L_4b4445
        mov ecx, eax
        mov dword ptr [esi + 0xc], ecx
        pop edi
        pop esi
        ret 0x4
    L_4b4445:
        mov edi, edx
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        not ecx
        dec ecx
        pop edi
        mov dword ptr [esi + 0xc], ecx
        pop esi
        ret 0x4
    }
}

// 0x004b4560 ListCursor_DecNoNotify_004b4560 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): if [this+0xc]>0: --[this+0xc]. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListCursor_DecNoNotify_004b4560(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xc]
        test eax, eax
        jle L_4b456b
        dec eax
        mov dword ptr [ecx + 0xc], eax
    L_4b456b:
        ret
    }
}

// 0x004b4570 ListCursor_Next_004b4570 - vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw/0x004b4570_*.c): if [this+0xc] < strlen([this+4]) - 1: ++[this+0xc]. Subsystem from address neighbours/callees (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListCursor_Next_004b4570(int, int)
{
    __asm {
        mov edx, ecx
        push esi
        push edi
        or ecx, 0xffffffff
        mov edi, dword ptr [edx + 0x4]
        xor eax, eax
        repne scasb
        mov esi, dword ptr [edx + 0xc]
        not ecx
        dec ecx
        cmp esi, ecx
        jge L_4b458c
        inc esi
        mov dword ptr [edx + 0xc], esi
    L_4b458c:
        pop edi
        pop esi
        ret
    }
}

// 0x004b4590 EditField_OpenGap - bytes ret 8 (n,pos): len=strlen(buf); if len+n < cap: shift bytes [pos..len] right by n (from end), return 1 else 0
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall EditField_OpenGap(int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        or ecx, 0xffffffff
        mov edi, dword ptr [esi + 0x4]
        xor eax, eax
        xor edx, edx
        repne scasb
        mov edi, dword ptr [esp + 0xc]
        not ecx
        dec ecx
        lea eax, [ecx + edi*0x1]
        mov ecx, dword ptr [esi + 0x8]
        cmp eax, ecx
        jge L_4b45d3
        push ebp
        mov ebp, dword ptr [esp + 0x14]
        cmp eax, ebp
        jle L_4b45cd
    L_4b45ba:
        mov ecx, dword ptr [esi + 0x4]
        mov edx, eax
        sub edx, edi
        dec eax
        cmp eax, ebp
        mov DL, byte ptr [edx + ecx*0x1]
        mov byte ptr [eax + ecx*0x1 + 0x1], DL
        jg L_4b45ba
    L_4b45cd:
        mov edx, 0x1
        pop ebp
    L_4b45d3:
        pop edi
        mov eax, edx
        pop esi
        ret 0x8
    }
}

// 0x004b45e0 EditField_CloseGap - bytes ret 8 (n,pos): for i=pos..strlen-1: buf[i]=buf[i+n] (delete n chars at pos)
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall EditField_CloseGap(int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        or ecx, 0xffffffff
        mov edi, dword ptr [esi + 0x4]
        xor eax, eax
        repne scasb
        mov eax, dword ptr [esp + 0x10]
        not ecx
        dec ecx
        mov edi, ecx
        cmp eax, edi
        jge L_4b4612
        push ebp
        mov ebp, dword ptr [esp + 0x10]
    L_4b4600:
        mov ecx, dword ptr [esi + 0x4]
        lea edx, [eax + ecx*0x1]
        inc eax
        cmp eax, edi
        mov CL, byte ptr [ebp + edx*0x1]
        mov byte ptr [edx], CL
        jl L_4b4600
        pop ebp
    L_4b4612:
        pop edi
        mov eax, 0x1
        pop esi
        ret 0x8
    }
}

// 0x004b47a0 Widget_BaseDtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_BaseDtor(int, int)
{
    __asm {
        mov dword ptr [ecx], offset g_RData_004cc000 + 0xa10
        ret
    }
}

// 0x004b70b0 Widget_GetSubobjectCC - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_GetSubobjectCC(int, int)
{
    __asm {
        lea eax, [ecx + 0xcc]
        ret
    }
}

// 0x004b7f20 Cycler_SetIndex - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Cycler_SetIndex(int, int, int)
{
    __asm {
        mov edx, dword ptr [esp + 0x4]
        mov eax, dword ptr [ecx + 0x14c]
        push esi
        mov esi, dword ptr [ecx + 0x154]
        cmp edx, esi
        jge L_4b7f3f
        mov dword ptr [ecx + 0x14c], esi
        pop esi
        ret 0x4
    L_4b7f3f:
        mov esi, dword ptr [ecx + 0x150]
        cmp edx, esi
        jl L_4b7f54
        dec esi
        mov dword ptr [ecx + 0x14c], esi
        pop esi
        ret 0x4
    L_4b7f54:
        mov esi, dword ptr [ecx + 0x158]
        cmp edx, esi
        jl L_4b7f69
        dec esi
        mov dword ptr [ecx + 0x14c], esi
        pop esi
        ret 0x4
    L_4b7f69:
        mov dword ptr [ecx + 0x14c], edx
        pop esi
        ret 0x4
    }
}

// 0x004b7f80 Cycler_SetRange - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Cycler_SetRange(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        test eax, eax
        jl L_4b7f96
        cmp eax, dword ptr [ecx + 0x150]
        jge L_4b7f96
        mov dword ptr [ecx + 0x154], eax
    L_4b7f96:
        mov edx, dword ptr [esp + 0x8]
        cmp edx, eax
        jl L_4b7fac
        cmp edx, dword ptr [ecx + 0x150]
        jge L_4b7fac
        mov dword ptr [ecx + 0x158], edx
    L_4b7fac:
        cmp dword ptr [ecx + 0x14c], eax
        jge L_4b7fba
        mov dword ptr [ecx + 0x14c], eax
    L_4b7fba:
        cmp dword ptr [ecx + 0x14c], edx
        jl L_4b7fc9
        dec edx
        mov dword ptr [ecx + 0x14c], edx
    L_4b7fc9:
        ret 0x8
    }
}

// 0x004ba350 ScreenBase_CloseConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenBase_CloseConfig(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0xa93c]
        test ecx, ecx
        jz L_4ba362
        call ConfigTree_Destroy
    L_4ba362:
        mov dword ptr [esi + 0xa93c], 0x0
        mov dword ptr [esi + 0xa940], 0x0
        pop esi
        ret 0x4
    }
}

// 0x004ba400 ListLabel_GetFixedRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_GetFixedRect(int, int)
{
    __asm {
        lea eax, [ecx + 0x27c]
        ret
    }
}

// 0x004ba4d0 PtrVector_EraseRange - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall PtrVector_EraseRange(int, int, int, int)
{
    __asm {
        mov edx, dword ptr [esp + 0x8]
        mov eax, dword ptr [esp + 0x4]
        push esi
        push edi
        mov edi, dword ptr [ecx + 0x8]
        mov esi, eax
        cmp edx, edi
        jz L_4ba4f3
        push ebx
    L_4ba4e4:
        mov ebx, dword ptr [edx]
        add edx, 0x4
        mov dword ptr [esi], ebx
        add esi, 0x4
        cmp edx, edi
        jnz L_4ba4e4
        pop ebx
    L_4ba4f3:
        mov edx, dword ptr [ecx + 0x8]
        mov dword ptr [ecx + 0x8], esi
        pop edi
        mov dword ptr [esp + 0x8], edx
        pop esi
        ret 0x8
    }
}

// 0x004bc4e0 Widget_HitTestCircle - bytes ret 8 (x,y): (x-+0x14)^2+(y-+0x18)^2 < +0x38
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_HitTestCircle(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [ecx + 0x14]
        push esi
        mov esi, dword ptr [ecx + 0x18]
        sub eax, edx
        mov edx, dword ptr [esp + 0xc]
        sub edx, esi
        mov esi, eax
        imul esi, eax
        mov eax, edx
        imul eax, edx
        mov edx, dword ptr [ecx + 0x38]
        add esi, eax
        xor eax, eax
        cmp esi, edx
        setl AL
        pop esi
        ret 0x8
    }
}

// 0x004bc550 Widget_SetField10 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_SetField10(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [ecx + 0x10], eax
        ret 0x4
    }
}

// 0x004bc560 Widget_GetField10 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_GetField10(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x10]
        ret
    }
}

// 0x004bc760 Widget_SetGlobalDirtyMode - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_SetGlobalDirtyMode(int, int)
{
    __asm {
        neg ecx
        sbb ecx, ecx
        and ecx, 0x8
        add ecx, 0x4
        mov dword ptr [g_Data_004da000 + 0xa870], ecx
        ret
    }
}

// 0x004bc780 WidgetContainer_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall WidgetContainer_Ctor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x0
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7c58
        call dword ptr [g_RData_004cc000 + 0x7c5c]
        mov dword ptr [esi + 0x8], 0x0
        mov dword ptr [esi + 0xc], 0x0
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004bc7b0 WidgetContainer_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall WidgetContainer_Dtor(int, int)
{
    __asm {
        mov dword ptr [ecx], offset g_RData_004cc000 + 0x7c58
        ret
    }
}

// 0x004bc7c0 Widget_AppendChild - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_AppendChild(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x8]
        test eax, eax
        jz L_4bc7ea
        mov edx, dword ptr [ecx + 0xc]
        test edx, edx
        jz L_4bc7ea
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [ecx + 0xc], eax
        mov dword ptr [eax + 0x4], 0x0
        mov dword ptr [eax + 0x8], ecx
        mov eax, 0x1
        ret 0x4
    L_4bc7ea:
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [ecx + 0xc], eax
        mov dword ptr [ecx + 0x8], eax
        mov dword ptr [eax + 0x4], 0x0
        mov dword ptr [eax + 0x8], ecx
        mov eax, 0x1
        ret 0x4
    }
}

// 0x004bc810 WidgetList_FindPrev - (node,&prev) ret 8: head [ECX+8]; node==head -> prev=0 return 1; walk next +4, on match prev=predecessor return 1; else 0
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall WidgetList_FindPrev(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x8]
        test eax, eax
        jz L_4bc857
        mov edx, dword ptr [esp + 0x4]
        cmp edx, eax
        jnz L_4bc831
        mov eax, dword ptr [esp + 0x8]
        mov dword ptr [eax], 0x0
        mov eax, 0x1
        ret 0x8
    L_4bc831:
        test eax, eax
        jz L_4bc857
    L_4bc835:
        mov ecx, dword ptr [eax + 0x4]
        cmp ecx, edx
        jz L_4bc845
        mov eax, ecx
        test eax, eax
        jnz L_4bc835
        ret 0x8
    L_4bc845:
        mov ecx, dword ptr [esp + 0x8]
        test ecx, ecx
        jz L_4bc84f
        mov dword ptr [ecx], eax
    L_4bc84f:
        mov eax, 0x1
        ret 0x8
    L_4bc857:
        xor eax, eax
        ret 0x8
    }
}

// 0x004bc8d0 WidgetContainer_SetChildFlags - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall WidgetContainer_SetChildFlags(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x8]
        test eax, eax
        jz L_4bc8f9
        mov ecx, dword ptr [esp + 0x4]
    L_4bc8db:
        mov edx, dword ptr [eax + 0xc]
        not edx
        test DL, 0x10
        jz L_4bc8ea
        mov dword ptr [eax + 0xc], ecx
        jmp L_4bc8f2
    L_4bc8ea:
        mov edx, ecx
        or edx, 0x10
        mov dword ptr [eax + 0xc], edx
    L_4bc8f2:
        mov eax, dword ptr [eax + 0x4]
        test eax, eax
        jnz L_4bc8db
    L_4bc8f9:
        ret 0x4
    }
}

// 0x004bc930 ListLabel_StartBlink - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabel_StartBlink(int, int, int)
{
    __asm {
        fld dword ptr [esp + 0x4]
        fcomp dword ptr [g_RData_004cc000 + 0x7c60]
        mov edx, 0x1
        mov dword ptr [ecx + 0x2b4], edx
        fnstsw AX
        test AH, 0x41
        jnz L_4bc95c
        fld dword ptr [esp + 0x4]
        fmul dword ptr [g_RData_004cc000 + 0x7c64]
        fstp dword ptr [ecx + 0x2a8]
    L_4bc95c:
        mov eax, dword ptr [ecx + 0x2a8]
        mov dword ptr [ecx + 0x2bc], edx
        mov dword ptr [ecx + 0x2a4], eax
        ret 0x4
    }
}

// 0x004bd3d0 MessagePanel_SetColours - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall MessagePanel_SetColours(int, int, int, int)
{
    __asm {
        lea eax, [ecx + 0x7fc]
        add ecx, 0x10
        cmp eax, ecx
        jc L_4bd409
        mov edx, dword ptr [esp + 0x8]
        push edi
        push esi
        mov esi, dword ptr [esp + 0xc]
        mov edi, 0x1
    L_4bd3ec:
        mov dword ptr [eax + 0x14c], esi
        mov dword ptr [eax + 0x150], edx
        mov dword ptr [eax + 0x270], edi
        sub eax, 0x2a4
        cmp eax, ecx
        jnc L_4bd3ec
        pop esi
        pop edi
    L_4bd409:
        ret 0x8
    }
}

// 0x004bd470 DrawQueue_Remove - unlink ECX from singly linked list head 0x0056bd34 (tail 0x0056bd38, count 0x0056bd30 -1)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DrawQueue_Remove(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [g_Data_004da000 + 0x91d34]
        mov eax, esi
        xor edx, edx
        test eax, eax
        jz L_4bd4c2
    L_4bd47f:
        cmp ecx, eax
        jz L_4bd48d
        mov edx, eax
        mov eax, dword ptr [eax]
        test eax, eax
        jnz L_4bd47f
        pop esi
        ret
    L_4bd48d:
        test edx, edx
        jnz L_4bd4a5
        mov eax, dword ptr [esi]
        mov dword ptr [g_Data_004da000 + 0x91d34], eax
        mov eax, dword ptr [g_Data_004da000 + 0x91d30]
        dec eax
        mov dword ptr [g_Data_004da000 + 0x91d30], eax
        pop esi
        ret
    L_4bd4a5:
        cmp eax, dword ptr [g_Data_004da000 + 0x91d38]
        jnz L_4bd4b3
        mov dword ptr [g_Data_004da000 + 0x91d38], edx
    L_4bd4b3:
        mov ecx, dword ptr [eax]
        mov dword ptr [edx], ecx
        mov eax, dword ptr [g_Data_004da000 + 0x91d30]
        dec eax
        mov dword ptr [g_Data_004da000 + 0x91d30], eax
    L_4bd4c2:
        pop esi
        ret
    }
}

// 0x004bd800 Vec3_LerpToZ - bytes ret 4 (z): t=(z-a.z)/(b.z-a.z); a.xy += (b.xy-a.xy)*t; a.z=z
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Vec3_LerpToZ(int, int, int)
{
    __asm {
        fld dword ptr [edx + 0x8]
        fld dword ptr [esp + 0x4]
        fsub dword ptr [ecx + 0x8]
        fxch st(1)
        fsub dword ptr [ecx + 0x8]
        fld dword ptr [edx]
        fxch st(1)
        fdivp st(2), st(0)
        fsub dword ptr [ecx]
        fld dword ptr [esp + 0x4]
        fxch st(1)
        fmul st(0), st(2)
        fadd dword ptr [ecx]
        fstp dword ptr [ecx]
        fld dword ptr [edx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fstp dword ptr [ecx + 0x8]
        fmul st(0), st(1)
        fadd dword ptr [ecx + 0x4]
        fstp dword ptr [ecx + 0x4]
        fstp st(0)
        ret 0x4
    }
}

// 0x004bd9c0 Seg_ClipToX - (x) ECX=p EDX=other ret 4: p.y += (x-p.x)*(other.y-p.y)/(other.x-p.x); p.x=x
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Seg_ClipToX(int, int, int)
{
    __asm {
        fld dword ptr [edx + 0x4]
        fld dword ptr [esp + 0x4]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fld dword ptr [edx]
        fsub dword ptr [ecx]
        fxch st(2)
        fmulp st(1), st(0)
        fld dword ptr [esp + 0x4]
        fxch st(2)
        fdivp st(1), st(0)
        fadd dword ptr [ecx + 0x4]
        fxch st(1)
        fstp dword ptr [ecx]
        fstp dword ptr [ecx + 0x4]
        ret 0x4
    }
}

// 0x004bdb30 Seg_ClipToY - (y) ECX=p EDX=other ret 4: p.x += (other.x-p.x)*(y-p.y)/(other.y-p.y); p.y=y
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Seg_ClipToY(int, int, int)
{
    __asm {
        fld dword ptr [esp + 0x4]
        fld dword ptr [edx]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fld dword ptr [edx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fxch st(2)
        fmulp st(1), st(0)
        fld dword ptr [esp + 0x4]
        fxch st(2)
        fdivp st(1), st(0)
        fadd dword ptr [ecx]
        fxch st(1)
        fstp dword ptr [ecx + 0x4]
        fstp dword ptr [ecx]
        ret 0x4
    }
}

// 0x004bdee0 ScreenParticle_Respawn - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ScreenParticle_Respawn(int, int, int, int)
{
    __asm {
        mov edx, dword ptr [ecx + 0x60]
        mov eax, dword ptr [esp + 0x4]
        push ebx
        mov ebx, dword ptr [g_Iat_rand_004cc5d8]
        push esi
        mov esi, dword ptr [ecx + edx*0x4 + 0x58]
        mov edx, dword ptr [ecx + 0x64]
        lea eax, [eax + eax*0x2]
        push edi
        mov edi, dword ptr [ecx + edx*0x4 + 0x58]
        shl eax, 0x2
        add esi, eax
        add edi, eax
        call ebx
        mov dword ptr [esp + 0x10], eax
        fild dword ptr [esp + 0x10]
        fmul dword ptr [g_RData_004cc000 + 0x7e68]
        fsubr dword ptr [g_RData_004cc000 + 0x7e6c]
        fstp dword ptr [esi + 0x8]
        call ebx
        mov dword ptr [esp + 0x10], eax
        fild dword ptr [esp + 0x10]
        fmul dword ptr [g_RData_004cc000 + 0x7e70]
        fsubr dword ptr [g_RData_004cc000 + 0x7e74]
        fst dword ptr [esi]
        fld dword ptr [esi + 0x8]
        fchs
        fld st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_4bdf62
        fld dword ptr [g_RData_004cc000 + 0x7e7c]
        fxch st(1)
        fsub dword ptr [g_RData_004cc000 + 0x7e78]
        fxch st(1)
        fsub dword ptr [esi + 0x8]
        fxch st(1)
        fstp dword ptr [esi]
        fstp dword ptr [esi + 0x8]
        jmp L_4bdf64
    L_4bdf62:
        fstp st(0)
    L_4bdf64:
        call ebx
        mov dword ptr [esp + 0x10], eax
        fild dword ptr [esp + 0x10]
        fld dword ptr [esi + 0x8]
        fchs
        fxch st(1)
        fmul dword ptr [g_RData_004cc000 + 0x7e70]
        fsubr dword ptr [g_RData_004cc000 + 0x7e74]
        fxch st(1)
        fld st(1)
        fcompp
        fst dword ptr [esi + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_4bdfae
        fld dword ptr [g_RData_004cc000 + 0x7e7c]
        fxch st(1)
        fsub dword ptr [g_RData_004cc000 + 0x7e78]
        fxch st(1)
        fsub dword ptr [esi + 0x8]
        fxch st(1)
        fstp dword ptr [esi + 0x4]
        fstp dword ptr [esi + 0x8]
        jmp L_4bdfb0
    L_4bdfae:
        fstp st(0)
    L_4bdfb0:
        mov eax, dword ptr [esi]
        mov dword ptr [edi], eax
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [edi + 0x4], ecx
        mov edx, dword ptr [esi + 0x8]
        mov dword ptr [edi + 0x8], edx
        pop edi
        pop esi
        pop ebx
        ret 0x8
    }
}

// 0x004be210 Points_AllInsideIntRect - bytes ret 8 (count,rect): rect null or count<1 -> 1; for each vec3 at ECX (stride 12): x<l, x>r, y<t or y>b (fild int rect) -> 0; else 1
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Points_AllInsideIntRect(int, int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0xc]
        test esi, esi
        push edi
        jz L_4be276
        mov edi, dword ptr [esp + 0xc]
        xor edx, edx
        test edi, edi
        jle L_4be276
        fild dword ptr [esi]
        fstp dword ptr [esp + 0x10]
    L_4be22a:
        fld dword ptr [ecx]
        fcomp dword ptr [esp + 0x10]
        fnstsw AX
        test AH, 0x1
        jnz L_4be26f
        fild dword ptr [esi + 0x8]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_4be26f
        fild dword ptr [esi + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_4be26f
        fild dword ptr [esi + 0xc]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_4be26f
        inc edx
        add ecx, 0xc
        cmp edx, edi
        jl L_4be22a
        mov eax, 0x1
        pop edi
        pop esi
        ret 0x8
    L_4be26f:
        xor eax, eax
        pop edi
        pop esi
        ret 0x8
    L_4be276:
        pop edi
        mov eax, 0x1
        pop esi
        ret 0x8
    }
}

// 0x004bee00 CompositePanel_SetSlot - bytes ret 8 (i,v): if i<2: [this+0x10+i*4]=v
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall CompositePanel_SetSlot(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        cmp eax, 0x2
        jge L_4bee11
        mov edx, dword ptr [esp + 0x8]
        mov dword ptr [ecx + eax*0x4 + 0x10], edx
    L_4bee11:
        ret 0x8
    }
}

// 0x004bee20 Object_SetRect18 - ../../04_spec/systems/render.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall Object_SetRect18(int, int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [esp + 0x8]
        mov dword ptr [ecx + 0x18], eax
        mov eax, dword ptr [esp + 0xc]
        mov dword ptr [ecx + 0x1c], edx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x20], eax
        mov dword ptr [ecx + 0x24], edx
        ret 0x10
    }
}

// 0x004bf7c0 Panel_SetMode1_004bf7c0 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): [this+0xa95c]=1; [this+0xa960]=0. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Panel_SetMode1_004bf7c0(int, int)
{
    __asm {
        mov dword ptr [ecx + 0xa95c], 0x1
        mov dword ptr [ecx + 0xa960], 0x0
        ret
    }
}

// 0x004bf7e0 Panel_SetMode2_004bf7e0 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): [this+0xa95c]=2; [this+0xa960]=0. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Panel_SetMode2_004bf7e0(int, int)
{
    __asm {
        mov dword ptr [ecx + 0xa95c], 0x2
        mov dword ptr [ecx + 0xa960], 0x0
        ret
    }
}

// 0x004bfe20 AviWidget_SetColourKey - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall AviWidget_SetColourKey(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x34]
        test eax, eax
        jz L_4bfe2b
        or byte ptr [eax + 0x9], 0x2
    L_4bfe2b:
        mov AX, word ptr [esp + 0x4]
        mov word ptr [ecx + 0x3c], AX
        ret 0x4
    }
}

// 0x00404cd0 Widget_SetPosition - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_SetPosition(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [esp + 0x8]
        mov dword ptr [ecx + 0x14], eax
        mov eax, dword ptr [ecx]
        mov dword ptr [ecx + 0x18], edx
        call dword ptr [eax + 0x20]
        ret 0x8
    }
}

// 0x00404cf0 Widget_SetX - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_SetX(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0x14], eax
        call dword ptr [edx + 0x20]
        ret 0x4
    }
}

// 0x00404d00 Widget_SetY - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_SetY(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0x18], eax
        call dword ptr [edx + 0x20]
        ret 0x4
    }
}

// 0x00404d20 Widget_SetVisible - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_SetVisible(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        test eax, eax
        mov eax, dword ptr [ecx + 0xc]
        jz L_404d38
        and AL, 0xef
        mov dword ptr [ecx + 0xc], eax
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x20]
        ret 0x4
    L_404d38:
        or AL, 0x10
        mov dword ptr [ecx + 0xc], eax
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x20]
        ret 0x4
    }
}

// 0x00404e10 ImageWidget_Slot_UpdateBoundsFromImage - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_Slot_UpdateBoundsFromImage(int, int)
{
    __asm {
        sub esp, 0x10
        mov eax, dword ptr [ecx + 0x3c]
        push esi
        mov esi, dword ptr [ecx + 0x14]
        push edi
        mov edi, dword ptr [ecx + 0x18]
        mov dword ptr [esp + 0x8], esi
        test eax, eax
        mov dword ptr [esp + 0xc], edi
        jz L_404e30
        movsx edx, word ptr [eax + 0x4]
        jmp L_404e32
    L_404e30:
        xor edx, edx
    L_404e32:
        add esi, edx
        test eax, eax
        mov dword ptr [esp + 0x10], esi
        jz L_404e42
        movsx eax, word ptr [eax + 0x6]
        jmp L_404e44
    L_404e42:
        xor eax, eax
    L_404e44:
        add edi, eax
        mov eax, dword ptr [ecx]
        lea edx, [esp + 0x8]
        mov dword ptr [esp + 0x14], edi
        push edx
        call dword ptr [eax + 0x1c]
        pop edi
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x00409010 ScrollGroup_ActivateChild - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScrollGroup_ActivateChild(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [ecx + 0x14c]
        cmp eax, edx
        jge L_409034
        mov ecx, dword ptr [ecx + eax*0x4 + 0x150]
        mov eax, dword ptr [ecx]
        mov dword ptr [ecx + 0xc4], 0x1
        call dword ptr [eax + 0x78]
    L_409034:
        ret 0x4
    }
}

// 0x0040db90 UiScreen_Slot_NotifyVisibleChildren - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_NotifyVisibleChildren(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x54]
        not eax
        test AL, 0x10
        jz L_40dba5
        mov edx, dword ptr [esi + 0x48]
        lea ecx, [esi + 0x48]
        call dword ptr [edx + 0x4]
    L_40dba5:
        mov eax, dword ptr [esi + 0x110]
        not eax
        test AL, 0x10
        jz L_40dbc0
        mov edx, dword ptr [esi + 0x104]
        lea ecx, [esi + 0x104]
        call dword ptr [edx + 0x4]
    L_40dbc0:
        pop esi
        ret
    }
}

// 0x0040f400 UiScreen_Slot_Virtual08_NotifyThreeChildren - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Virtual08_NotifyThreeChildren(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x8]
        mov ecx, dword ptr [esi + 0x1c0]
        not ecx
        test CL, 0x10
        jz L_40f424
        mov edx, dword ptr [esi + 0x1b4]
        lea ecx, [esi + 0x1b4]
        call dword ptr [edx + 0x4]
    L_40f424:
        mov eax, dword ptr [esi + 0x104]
        not eax
        test AL, 0x10
        jz L_40f43f
        mov edx, dword ptr [esi + 0xf8]
        lea ecx, [esi + 0xf8]
        call dword ptr [edx + 0x4]
    L_40f43f:
        mov eax, dword ptr [esi + 0x48]
        not eax
        test AL, 0x10
        jz L_40f451
        mov edx, dword ptr [esi + 0x3c]
        lea ecx, [esi + 0x3c]
        call dword ptr [edx + 0x4]
    L_40f451:
        pop esi
        ret
    }
}

// 0x0040fa10 Widget_Slot_ForwardToMember34Slot0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_ForwardToMember34Slot0(int, int, int)
{
    __asm {
        mov ecx, dword ptr [ecx + 0x34]
        mov edx, dword ptr [esp + 0x4]
        push edx
        mov eax, dword ptr [ecx]
        call dword ptr [eax]
        ret 0x4
    }
}

// 0x004b3dd0 ImageWidget_SetPosition - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ImageWidget_SetPosition(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x48]
        push esi
        test eax, eax
        jz L_4b3e0c
        mov esi, dword ptr [ecx + 0x3c]
        test esi, esi
        jz L_4b3e0c
        movsx eax, word ptr [esi + 0x4]
        cdq
        sub eax, edx
        mov edx, dword ptr [esp + 0x8]
        sar eax, 0x1
        sub edx, eax
        mov dword ptr [ecx + 0x14], edx
        movsx eax, word ptr [esi + 0x6]
        cdq
        sub eax, edx
        mov edx, dword ptr [esp + 0xc]
        sar eax, 0x1
        sub edx, eax
        mov eax, dword ptr [ecx]
        mov dword ptr [ecx + 0x18], edx
        call dword ptr [eax + 0x20]
        pop esi
        ret 0x8
    L_4b3e0c:
        mov edx, dword ptr [esp + 0x8]
        mov eax, dword ptr [esp + 0xc]
        mov dword ptr [ecx + 0x14], edx
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0x18], eax
        call dword ptr [edx + 0x20]
        pop esi
        ret 0x8
    }
}

// 0x004b3e30 ImageWidget_SetImage - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ImageWidget_SetImage(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        test edi, edi
        jnz L_4b3e43
        xor eax, eax
        pop edi
        pop esi
        ret 0x4
    L_4b3e43:
        mov ecx, esi
        call ImageWidget_ReleaseOwnedImage
        mov ecx, edi
        call Image_Load
        test eax, eax
        mov dword ptr [esi + 0x3c], eax
        jz L_4b3e5f
        mov dword ptr [esi + 0x34], 0x1
    L_4b3e5f:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        mov eax, dword ptr [esi + 0x3c]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004b3e70 ImageWidget_SetImageNoOwn - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ImageWidget_SetImageNoOwn(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx]
        push esi
        mov esi, dword ptr [esp + 0x8]
        mov dword ptr [ecx + 0x34], 0x0
        mov dword ptr [ecx + 0x3c], esi
        call dword ptr [eax + 0x20]
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b3e90 Widget_AddDirtyRect - If widget has image +0x3c: finds first free of 4 dirty-rect slots (+0x4c, stride 0x1c); copies arg rect into slot+0xc..+0x18 and clips to the widget bounds (origin +0x14/+0x18, size from image shorts +4/+6); if non-empty: dirty count +0x38 += 1, slot type = 1 + ([0x004e4870] == 0xc), start = (left, 
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_AddDirtyRect(int, int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        push edi
        xor edi, edi
        mov eax, dword ptr [esi + 0x3c]
        test eax, eax
        jz L_4b3fa4
        xor eax, eax
        lea ecx, [esi + 0x4c]
    L_4b3ea7:
        cmp dword ptr [ecx], 0x0
        jz L_4b3eb7
        inc eax
        add ecx, 0x1c
        cmp eax, 0x4
        jl L_4b3ea7
        jmp L_4b3ec2
    L_4b3eb7:
        mov ecx, eax
        shl ecx, 0x3
        sub ecx, eax
        lea edi, [esi + ecx*0x4 + 0x4c]
    L_4b3ec2:
        test edi, edi
        jz L_4b3fa4
        mov edx, dword ptr [esp + 0x10]
        lea ebx, [edi + 0xc]
        mov eax, ebx
        mov ecx, dword ptr [edx]
        mov dword ptr [eax], ecx
        mov ecx, dword ptr [edx + 0x4]
        mov dword ptr [eax + 0x4], ecx
        mov ecx, dword ptr [edx + 0x8]
        mov dword ptr [eax + 0x8], ecx
        mov edx, dword ptr [edx + 0xc]
        mov dword ptr [eax + 0xc], edx
        mov eax, dword ptr [esi + 0x14]
        cmp dword ptr [ebx], eax
        jge L_4b3ef2
        mov dword ptr [ebx], eax
    L_4b3ef2:
        mov eax, dword ptr [esi + 0x3c]
        mov edx, dword ptr [esi + 0x14]
        movsx ecx, word ptr [eax + 0x4]
        lea eax, [ecx + edx*0x1]
        mov ecx, dword ptr [edi + 0x14]
        cmp ecx, eax
        jle L_4b3f09
        mov dword ptr [edi + 0x18], eax
    L_4b3f09:
        mov eax, dword ptr [esi + 0x18]
        mov ecx, dword ptr [edi + 0x10]
        cmp ecx, eax
        jge L_4b3f16
        mov dword ptr [edi + 0x10], eax
    L_4b3f16:
        mov eax, dword ptr [esi + 0x3c]
        mov edx, dword ptr [esi + 0x18]
        movsx ecx, word ptr [eax + 0x6]
        lea eax, [ecx + edx*0x1]
        mov ecx, dword ptr [edi + 0x18]
        cmp ecx, eax
        jle L_4b3f2d
        mov dword ptr [edi + 0x18], eax
    L_4b3f2d:
        mov eax, dword ptr [edi + 0x14]
        mov ecx, dword ptr [ebx]
        cmp eax, ecx
        jle L_4b3fa4
        mov ecx, dword ptr [edi + 0x18]
        mov eax, dword ptr [edi + 0x10]
        cmp ecx, eax
        jle L_4b3fa4
        mov edx, dword ptr [esi + 0x38]
        inc edx
        mov dword ptr [esi + 0x38], edx
        mov ecx, dword ptr [g_Data_004da000 + 0xa870]
        xor edx, edx
        cmp ecx, 0xc
        setz DL
        inc edx
        mov dword ptr [edi], edx
        mov eax, dword ptr [ebx]
        mov ecx, dword ptr [edi + 0x10]
        mov dword ptr [edi + 0x4], eax
        mov dword ptr [edi + 0x8], ecx
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x64]
        mov ecx, dword ptr [ebx]
        sub ecx, eax
        mov dword ptr [ebx], ecx
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        mov ebx, dword ptr [edi + 0x14]
        mov ecx, esi
        sub ebx, eax
        mov dword ptr [edi + 0x14], ebx
        mov edx, dword ptr [esi]
        call dword ptr [edx + 0x68]
        mov edx, dword ptr [edi + 0x10]
        mov ecx, esi
        sub edx, eax
        mov dword ptr [edi + 0x10], edx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x68]
        mov ecx, dword ptr [edi + 0x18]
        sub ecx, eax
        mov dword ptr [edi + 0x18], ecx
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
    L_4b3fa4:
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004b4030 Widget_HitTestRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_HitTestRect(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xc]
        not eax
        test AL, 0x10
        jz L_4b4069
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x2c]
        test eax, eax
        jz L_4b4069
        mov ecx, dword ptr [esp + 0x4]
        mov edx, dword ptr [eax]
        cmp ecx, edx
        jl L_4b4069
        cmp ecx, dword ptr [eax + 0x8]
        jg L_4b4069
        mov ecx, dword ptr [esp + 0x8]
        mov edx, dword ptr [eax + 0x4]
        cmp ecx, edx
        jl L_4b4069
        cmp ecx, dword ptr [eax + 0xc]
        jg L_4b4069
        mov eax, 0x1
        ret 0x8
    L_4b4069:
        xor eax, eax
        ret 0x8
    }
}

// 0x004b4190 Widget_SetField1CAndNotify - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_SetField1CAndNotify(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0x1c], eax
        mov eax, dword ptr [esp + 0x8]
        push eax
        call dword ptr [edx + 0x1c]
        ret 0x8
    }
}

// 0x004b41e0 Widget_Tick - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_Tick(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc]
        mov ecx, eax
        not ecx
        test CL, 0x10
        jz L_4b424b
        test AL, 0x2
        jnz L_4b41fc
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x4]
        jmp L_4b4221
    L_4b41fc:
        test AL, 0x4
        jz L_4b420e
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x4]
        mov eax, dword ptr [esi + 0xc]
        and AL, 0xfb
        jmp L_4b421e
    L_4b420e:
        test AL, 0x8
        jz L_4b4221
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x4]
        mov eax, dword ptr [esi + 0xc]
        and AL, 0xf7
    L_4b421e:
        mov dword ptr [esi + 0xc], eax
    L_4b4221:
        test byte ptr [esi + 0xc], 0x1
        jz L_4b4279
        fld dword ptr [esi + 0x10]
        fsub dword ptr [esp + 0x8]
        fcom qword ptr [g_RData_004cc000 + 0x74a0]
        fstp dword ptr [esi + 0x10]
        fnstsw AX
        test AH, 0x41
        jz L_4b4279
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x60]
        pop esi
        ret 0x4
    L_4b424b:
        test AL, 0x2
        jz L_4b4279
        test AL, 0x4
        jz L_4b4266
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x8]
        mov eax, dword ptr [esi + 0xc]
        and AL, 0xfb
        mov dword ptr [esi + 0xc], eax
        pop esi
        ret 0x4
    L_4b4266:
        test AL, 0x8
        jz L_4b4279
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x8]
        mov eax, dword ptr [esi + 0xc]
        and AL, 0xf7
        mov dword ptr [esi + 0xc], eax
    L_4b4279:
        pop esi
        ret 0x4
    }
}

// 0x004b42c0 Widget_GetExtentRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_GetExtentRect(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x64]
        mov edi, dword ptr [esp + 0xc]
        mov ecx, esi
        mov dword ptr [edi + 0x8], eax
        mov dword ptr [edi], eax
        mov edx, dword ptr [esi]
        call dword ptr [edx + 0x68]
        mov dword ptr [edi + 0xc], eax
        mov dword ptr [edi + 0x4], eax
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004b44e0 EditField_InsertChar - bytes ret 4 (ch): if strlen(buf +4) < cap(+8)-1: EditField_OpenGap(1, cursor +0xc); buf[cursor]=ch; cursor++
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_InsertChar(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        or ecx, 0xffffffff
        mov edi, dword ptr [esi + 0x4]
        xor eax, eax
        repne scasb
        mov eax, dword ptr [esi + 0x8]
        not ecx
        dec ecx
        dec eax
        cmp ecx, eax
        jge L_4b451f
        mov ecx, dword ptr [esi + 0xc]
        push ecx
        push 0x1
        mov ecx, esi
        call EditField_OpenGap
        mov edx, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0x4]
        mov CL, byte ptr [esp + 0xc]
        mov byte ptr [edx + eax*0x1], CL
        mov eax, dword ptr [esi + 0xc]
        inc eax
        mov dword ptr [esi + 0xc], eax
        pop edi
        pop esi
        ret 0x4
    L_4b451f:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004b4ba0 EditField_SetEnabled - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_SetEnabled(int, int, int)
{
    __asm {
        mov edx, dword ptr [esp + 0x4]
        push ebp
        push esi
        mov esi, ecx
        push edi
        xor edi, edi
        mov ebp, dword ptr [esi + 0x36c]
        mov dword ptr [esi + 0x36c], edx
        mov ecx, dword ptr [esi + 0x110]
        test ecx, ecx
        jnz L_4b4bc5
        xor eax, eax
        jmp L_4b4bd0
    L_4b4bc5:
        mov eax, dword ptr [esi + 0x114]
        sub eax, ecx
        sar eax, 0x2
    L_4b4bd0:
        xor ecx, ecx
        test eax, eax
        setz CL
        test CL, CL
        jnz L_4b4be3
        mov eax, dword ptr [esi + 0x110]
        mov edi, dword ptr [eax]
    L_4b4be3:
        test edx, edx
        jz L_4b4c16
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x260]
        lea ecx, [esi + 0x260]
        push 0x1
        call dword ptr [eax + 0x60]
        test edi, edi
        jz L_4b4c3d
        mov edx, dword ptr [edi]
        push 0x1
        mov ecx, edi
        call dword ptr [edx + 0x60]
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        ret 0x4
    L_4b4c16:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x60]
        test edi, edi
        jz L_4b4c2c
        mov edx, dword ptr [edi]
        push 0x0
        mov ecx, edi
        call dword ptr [edx + 0x60]
    L_4b4c2c:
        mov eax, dword ptr [esi + 0x260]
        lea ecx, [esi + 0x260]
        push 0x0
        call dword ptr [eax + 0x60]
    L_4b4c3d:
        pop edi
        mov eax, ebp
        pop esi
        pop ebp
        ret 0x4
    }
}

// 0x004b52f0 DeleteWidgetPtr - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall DeleteWidgetPtr(int, int, int)
{
    __asm {
        mov ecx, dword ptr [esp + 0x4]
        test ecx, ecx
        jz L_4b52fe
        mov eax, dword ptr [ecx]
        push 0x1
        call dword ptr [eax]
    L_4b52fe:
        xor eax, eax
        ret 0x4
    }
}

// 0x004b5310 Widget_MarkDirtyAndRedrawChildren - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_MarkDirtyAndRedrawChildren(int, int)
{
    __asm {
        push esi
        push edi
        mov edi, ecx
        call Widget_OrFlagsWithGlobal
        mov esi, dword ptr [edi + 0x110]
        test esi, esi
        jz L_4b533f
        cmp esi, dword ptr [edi + 0x114]
        jz L_4b533f
    L_4b532b:
        mov ecx, dword ptr [esi]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x20]
        mov eax, dword ptr [edi + 0x114]
        add esi, 0x4
        cmp esi, eax
        jnz L_4b532b
    L_4b533f:
        pop edi
        pop esi
        ret
    }
}

// 0x004b8100 Cycler_SetItemFont - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Cycler_SetItemFont(int, int, int, int)
{
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 0x8]
        push esi
        push edi
        mov edi, ecx
        cmp ebx, dword ptr [edi + 0x150]
        jl L_4b8134
        lea eax, [ebx + 0x1]
        cmp eax, 0x14
        jl L_4b811e
        mov eax, 0x14
    L_4b811e:
        mov ecx, dword ptr [edi + 0x158]
        mov dword ptr [edi + 0x150], eax
        cmp eax, ecx
        jle L_4b8134
        mov dword ptr [edi + 0x158], eax
    L_4b8134:
        cmp ebx, dword ptr [edi + 0x158]
        jg L_4b81f2
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [edi + 0xc8]
        lea eax, [eax + eax*0x8]
        mov esi, dword ptr [ecx + eax*0x4 + 0x1cec]
        lea eax, [ecx + eax*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b81f2
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [edi + ebx*0x4 + 0x168]
        push 0x2
        push 0x0
        mov edx, dword ptr [ecx]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov eax, dword ptr [edi + ebx*0x4 + 0x168]
        mov ecx, dword ptr [esi + 0xc]
        mov dword ptr [eax + 0x14c], ecx
        mov dword ptr [eax + 0x150], ecx
        mov ecx, 0x1
        mov dword ptr [eax + 0x270], ecx
        mov eax, dword ptr [edi + ebx*0x4 + 0x168]
        mov edx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], edx
        mov dword ptr [eax + 0x29c], ecx
        mov dword ptr [eax + 0x2a0], ecx
        mov eax, dword ptr [edi + ebx*0x4 + 0x168]
        mov ecx, dword ptr [esi + 0x20]
        mov dword ptr [eax + 0x144], ecx
        mov edi, dword ptr [edi + ebx*0x4 + 0x168]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [edi + 0x268], edx
        mov dword ptr [edi + 0x26c], eax
    L_4b81f2:
        pop edi
        pop esi
        pop ebx
        ret 0x8
    }
}

// 0x004b9330 ListScreen_ScrollTo - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListScreen_ScrollTo(int, int, int)
{
    __asm {
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0xc]
        push esi
        mov esi, ecx
        xor ebx, ebx
        push edi
        mov ecx, dword ptr [esi + 0x168]
        test ecx, ecx
        jle L_4b93f4
        xor edi, edi
    L_4b934c:
        lea eax, [ebx + ebp*0x1]
        sub eax, ecx
        js L_4b93b2
        mov edx, dword ptr [esi + 0x420]
        test edx, edx
        jnz L_4b9361
        xor ecx, ecx
        jmp L_4b936c
    L_4b9361:
        mov ecx, dword ptr [esi + 0x424]
        sub ecx, edx
        sar ecx, 0x2
    L_4b936c:
        cmp eax, ecx
        jnc L_4b93b2
        mov ecx, dword ptr [esi + 0x420]
        mov edx, dword ptr [esi + 0x418]
        mov ecx, dword ptr [ecx + eax*0x4]
        mov dword ptr [edi + edx*0x1 + 0x2a4], eax
        mov eax, dword ptr [esi + 0x418]
        mov ecx, dword ptr [ecx]
        add eax, edi
        push ecx
        push offset g_Data_004da000 + 0x97c
        mov edx, dword ptr [eax]
        push eax
        call dword ptr [edx + 0x74]
        mov edx, dword ptr [esi + 0x418]
        mov ecx, edi
        add ecx, edx
        add esp, 0xc
        mov edx, dword ptr [ecx]
        push 0x1
        call dword ptr [edx + 0x60]
        jmp L_4b93d0
    L_4b93b2:
        mov eax, dword ptr [esi + 0x418]
        mov ecx, edi
        add ecx, eax
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esi + 0x418]
        add ecx, edi
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x8]
    L_4b93d0:
        mov eax, dword ptr [esi + 0x418]
        mov edx, dword ptr [edi + eax*0x1]
        lea ecx, [edi + eax*0x1]
        call dword ptr [edx + 0x20]
        mov ecx, dword ptr [esi + 0x168]
        inc ebx
        add edi, 0x2ac
        cmp ebx, ecx
        jl L_4b934c
    L_4b93f4:
        test ebp, ebp
        jl L_4b943f
        mov ecx, dword ptr [esi + 0x420]
        test ecx, ecx
        jnz L_4b9406
        xor eax, eax
        jmp L_4b9411
    L_4b9406:
        mov eax, dword ptr [esi + 0x424]
        sub eax, ecx
        sar eax, 0x2
    L_4b9411:
        cmp ebp, eax
        jnc L_4b943f
        mov dword ptr [esi + 0x410], ebp
        mov edx, dword ptr [esi + 0x420]
        mov ecx, dword ptr [esi + 0x16c]
        lea eax, [esi + 0x16c]
        mov edx, dword ptr [edx + ebp*0x4]
        mov edx, dword ptr [edx]
        push edx
        push offset g_Data_004da000 + 0x97c
        push eax
        call dword ptr [ecx + 0x74]
        add esp, 0xc
    L_4b943f:
        mov ebx, dword ptr [esi + 0x168]
        mov eax, dword ptr [esi + 0x164]
        cmp ebx, eax
        jge L_4b950e
        lea eax, [ebx + ebx*0x8]
        lea eax, [ebx + eax*0x2]
        lea edi, [eax + eax*0x8]
        shl edi, 0x2
    L_4b945f:
        mov edx, dword ptr [esi + 0x168]
        lea eax, [ebx + ebp*0x1]
        sub eax, edx
        inc eax
        js L_4b94cc
        mov edx, dword ptr [esi + 0x420]
        test edx, edx
        jnz L_4b947b
        xor ecx, ecx
        jmp L_4b9486
    L_4b947b:
        mov ecx, dword ptr [esi + 0x424]
        sub ecx, edx
        sar ecx, 0x2
    L_4b9486:
        cmp eax, ecx
        jnc L_4b94cc
        mov ecx, dword ptr [esi + 0x420]
        mov edx, dword ptr [esi + 0x418]
        mov ecx, dword ptr [ecx + eax*0x4]
        mov dword ptr [edi + edx*0x1 + 0x2a4], eax
        mov eax, dword ptr [esi + 0x418]
        mov ecx, dword ptr [ecx]
        add eax, edi
        push ecx
        push offset g_Data_004da000 + 0x97c
        mov edx, dword ptr [eax]
        push eax
        call dword ptr [edx + 0x74]
        mov edx, dword ptr [esi + 0x418]
        mov ecx, edi
        add ecx, edx
        add esp, 0xc
        mov edx, dword ptr [ecx]
        push 0x1
        call dword ptr [edx + 0x60]
        jmp L_4b94ea
    L_4b94cc:
        mov eax, dword ptr [esi + 0x418]
        mov ecx, edi
        add ecx, eax
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esi + 0x418]
        add ecx, edi
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x8]
    L_4b94ea:
        mov eax, dword ptr [esi + 0x418]
        mov edx, dword ptr [edi + eax*0x1]
        lea ecx, [edi + eax*0x1]
        call dword ptr [edx + 0x20]
        mov eax, dword ptr [esi + 0x164]
        inc ebx
        add edi, 0x2ac
        cmp ebx, eax
        jl L_4b945f
    L_4b950e:
        mov dword ptr [esi + 0x430], ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    }
}

// 0x004b9520 ListItem_Slot_ForwardValueToTarget - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListItem_Slot_ForwardValueToTarget(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x2a8]
        test eax, eax
        jz L_4b953b
        mov ecx, dword ptr [ecx + 0x2a4]
        mov edx, dword ptr [eax]
        push ecx
        mov ecx, eax
        call dword ptr [edx + 0x84]
    L_4b953b:
        ret
    }
}

// 0x004ba070 ScreenBase_BindButtonImpl - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ScreenBase_BindButtonImpl(int, int, int, int)
{
    __asm {
        mov eax, edx
        push edi
        test eax, eax
        mov edi, ecx
        jz L_4ba0ad
        mov edx, offset g_Data_004da000 + 0xa838
        mov ecx, eax
        call ConfigTree_FindChild
        mov edx, dword ptr [esp + 0xc]
        mov ecx, eax
        call ConfigTree_FindChild
        test eax, eax
        jz L_4ba0ad
        push esi
        mov esi, dword ptr [esp + 0xc]
        push edi
        push eax
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x7c]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x80]
        pop esi
    L_4ba0ad:
        xor AL, AL
        pop edi
        ret 0x8
    }
}

// 0x004ba3a0 WidgetContainer_RedrawChildren - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall WidgetContainer_RedrawChildren(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [ecx + 0x8]
        test esi, esi
        jz L_4ba3b6
    L_4ba3a8:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        mov esi, dword ptr [esi + 0x4]
        test esi, esi
        jnz L_4ba3a8
    L_4ba3b6:
        pop esi
        ret
    }
}

// 0x004ba3c0 Widget_SetField14CAndRedraw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_SetField14CAndRedraw(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0x14c], eax
        call dword ptr [edx + 0x20]
        ret 0x4
    }
}

// 0x004ba3e0 Widget_ForwardSlot88 - bytes: Stub_Ret; child +0x110 vtbl+0x88()
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_ForwardSlot88(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Stub_Ret
        mov ecx, dword ptr [esi + 0x110]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x88]
        pop esi
        ret
    }
}

// 0x004bb440 ListLabel_GetText - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_GetText(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4bb455
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x90]
    L_4bb455:
        lea eax, [esi + 0x15c]
        pop esi
        ret
    }
}

// 0x004bb710 ListLabel_GetLineHeight - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_GetLineHeight(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4bb725
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x90]
    L_4bb725:
        mov eax, dword ptr [esi + 0x260]
        mov ecx, dword ptr [esi + 0x274]
        sub eax, ecx
        pop esi
        ret
    }
}

// 0x004bb980 ListWidget_BroadcastIfVisible - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListWidget_BroadcastIfVisible(int, int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jz L_4bb9db
        mov ebp, dword ptr [esp + 0x14]
        xor ebx, ebx
        xor edi, edi
    L_4bb997:
        mov eax, dword ptr [esi + 0x2ac]
        test eax, eax
        jnz L_4bb9a5
        xor edx, edx
        jmp L_4bb9be
    L_4bb9a5:
        mov ecx, dword ptr [esi + 0x2b0]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
    L_4bb9be:
        cmp ebx, edx
        jnc L_4bb9db
        mov edx, dword ptr [esi + 0x2ac]
        push ebp
        mov eax, dword ptr [edx + edi*0x1]
        lea ecx, [edx + edi*0x1]
        call dword ptr [eax + 0x24]
        inc ebx
        add edi, 0x2c0
        jmp L_4bb997
    L_4bb9db:
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    }
}

// 0x004bbaa0 ListWidget_PrintfCurrent - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListWidget_PrintfCurrent(int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        lea edx, [esp + 0xc]
        push edx
        mov edx, dword ptr [esp + 0xc]
        mov ecx, dword ptr [eax]
        push edx
        push eax
        call dword ptr [ecx + 0x88]
        add esp, 0xc
        ret
    }
}

// 0x004bbac0 ListWidget_ForwardToCurrentItem - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListWidget_ForwardToCurrentItem(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        mov eax, dword ptr [esi + 0x2a4]
        lea ecx, [eax + eax*0x4]
        lea edx, [eax + ecx*0x2]
        mov eax, dword ptr [esi + 0x2ac]
        shl edx, 0x6
        add eax, edx
        mov edx, dword ptr [esp + 0x10]
        push edx
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [eax]
        push edx
        push eax
        call dword ptr [ecx + 0x88]
        mov eax, dword ptr [esi + 0x2a4]
        mov edx, dword ptr [esi + 0x2ac]
        add esp, 0xc
        lea ecx, [eax + eax*0x4]
        push 0x1
        lea ecx, [eax + ecx*0x2]
        shl ecx, 0x6
        add ecx, edx
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x94]
        pop esi
        ret
    }
}

// 0x004bbed0 ListWidget_ClearRange - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ListWidget_ClearRange(int, int, int, int)
{
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 0xc]
        push ebp
        mov ebp, dword ptr [esp + 0xc]
        push esi
        push edi
        cmp ebp, ebx
        mov edi, ecx
        jle L_4bbee6
        mov ebp, ebx
        jmp L_4bbeec
    L_4bbee6:
        test ebp, ebp
        jge L_4bbeec
        xor ebp, ebp
    L_4bbeec:
        mov eax, dword ptr [edi + 0x2ac]
        test eax, eax
        jnz L_4bbefa
        xor edx, edx
        jmp L_4bbf13
    L_4bbefa:
        mov ecx, dword ptr [edi + 0x2b0]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bbf13:
        cmp ebx, edx
        jbe L_4bbf42
        mov eax, dword ptr [edi + 0x2ac]
        test eax, eax
        jnz L_4bbf25
        xor ebx, ebx
        jmp L_4bbf48
    L_4bbf25:
        mov ecx, dword ptr [edi + 0x2b0]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
        mov ebx, edx
        jmp L_4bbf48
    L_4bbf42:
        cmp ebp, ebx
        jle L_4bbf4a
        mov ebx, ebp
    L_4bbf48:
        cmp ebp, ebx
    L_4bbf4a:
        jge L_4bbf87
        lea edx, [ebp + ebp*0x4]
        lea esi, [ebp + edx*0x2]
        shl esi, 0x6
        sub ebx, ebp
    L_4bbf59:
        mov eax, dword ptr [edi + 0x2ac]
        push offset g_Data_004da000 + 0xbce0
        add eax, esi
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x74]
        mov ecx, dword ptr [edi + 0x2ac]
        add esp, 0x8
        add ecx, esi
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        add esi, 0x2c0
        dec ebx
        jnz L_4bbf59
    L_4bbf87:
        mov dword ptr [edi + 0x2a4], ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x8
    }
}

// 0x004bc900 Manager_BroadcastVirtual24 - ../../04_spec/systems/render.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Manager_BroadcastVirtual24(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        push esi
        test eax, eax
        jz L_4bc924
        mov esi, dword ptr [ecx + 0x8]
        test esi, esi
        jz L_4bc924
        push edi
        mov edi, dword ptr [esp + 0xc]
    L_4bc914:
        mov eax, dword ptr [esi]
        push edi
        mov ecx, esi
        call dword ptr [eax + 0x24]
        mov esi, dword ptr [esi + 0x4]
        test esi, esi
        jnz L_4bc914
        pop edi
    L_4bc924:
        pop esi
        ret 0x4
    }
}

// 0x004bcd40 Widget_SetField1CAndRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_SetField1CAndRect(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov dword ptr [ecx + 0x1c], eax
        mov eax, dword ptr [esp + 0x8]
        test eax, eax
        jz L_4bcd6b
        push esi
        mov esi, dword ptr [eax]
        lea edx, [ecx + 0x20]
        mov dword ptr [ecx + 0x20], esi
        mov esi, dword ptr [eax + 0x4]
        mov dword ptr [edx + 0x4], esi
        mov esi, dword ptr [eax + 0x8]
        mov dword ptr [edx + 0x8], esi
        mov eax, dword ptr [eax + 0xc]
        pop esi
        mov dword ptr [edx + 0xc], eax
    L_4bcd6b:
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x20]
        ret 0x8
    }
}

// 0x004bd110 WidgetArray_SetRectAll - For widgets from this+0x7fc down to this+0x10 (stride -0x2a4): vtable+0x80(a1,a2,a3,a4,0,0,2). ret 0x10.
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall WidgetArray_SetRectAll(int, int, int, int, int, int)
{
    __asm {
        push esi
        push edi
        lea esi, [ecx + 0x7fc]
        lea edi, [ecx + 0x10]
        cmp esi, edi
        jc L_4bd151
        push ebp
        mov ebp, dword ptr [esp + 0x18]
        push ebx
        mov ebx, dword ptr [esp + 0x20]
    L_4bd129:
        mov ecx, dword ptr [esp + 0x18]
        mov edx, dword ptr [esp + 0x14]
        mov eax, dword ptr [esi]
        push 0x2
        push 0x0
        push 0x0
        push ebx
        push ebp
        push ecx
        push edx
        mov ecx, esi
        call dword ptr [eax + 0x80]
        sub esi, 0x2a4
        cmp esi, edi
        jnc L_4bd129
        pop ebx
        pop ebp
    L_4bd151:
        pop edi
        pop esi
        ret 0x10
    }
}

// 0x004bd2a0 MessagePanel_Clear - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MessagePanel_Clear(int, int)
{
    __asm {
        push esi
        push edi
        lea esi, [ecx + 0x10]
        mov edi, 0x4
    L_4bd2aa:
        mov eax, dword ptr [esi]
        push offset g_Data_004da000 + 0xbce0
        push esi
        call dword ptr [eax + 0x74]
        mov edx, dword ptr [esi]
        add esp, 0x8
        mov ecx, esi
        push 0x0
        call dword ptr [edx + 0x60]
        add esi, 0x2a4
        dec edi
        jnz L_4bd2aa
        pop edi
        pop esi
        ret
    }
}

// 0x004bd410 MessagePanel_SetCentreX - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MessagePanel_SetCentreX(int, int, int)
{
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 0x8]
        push esi
        push edi
        lea esi, [ecx + 0x10]
        mov edi, 0x4
    L_4bd41f:
        mov eax, dword ptr [esi]
        push ebx
        mov ecx, esi
        call dword ptr [eax + 0x10]
        add esi, 0x2a4
        dec edi
        jnz L_4bd41f
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004bd440 MessagePanel_SetTopY - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MessagePanel_SetTopY(int, int, int)
{
    __asm {
        push ebx
        push esi
        push edi
        mov edi, dword ptr [esp + 0x10]
        lea esi, [ecx + 0x10]
        mov ebx, 0x4
    L_4bd44f:
        mov eax, dword ptr [esi]
        push edi
        mov ecx, esi
        call dword ptr [eax + 0x14]
        sub edi, 0x12
        add esi, 0x2a4
        dec ebx
        jnz L_4bd44f
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004bdb60 Widget_Slot_DrawWithStyle - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_DrawWithStyle(int, int)
{
    __asm {
        push esi
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi]
        mov esi, dword ptr [edi + 0x8]
        call dword ptr [eax + 0x8]
        test esi, esi
        jz L_4bdbb4
        mov ecx, dword ptr [esi + 0x18]
        push ebp
        test ecx, ecx
        push ebx
        jz L_4bdb89
        mov edx, dword ptr [esi + 0x24]
        mov eax, dword ptr [esi + 0x20]
        push edx
        mov edx, dword ptr [esi + 0x1c]
        push eax
        call SetGlobalRect_0056b1c4
    L_4bdb89:
        lea ebp, [esi + 0x10]
        mov ebx, 0x2
        mov esi, ebp
    L_4bdb93:
        mov eax, dword ptr [esi]
        test eax, eax
        jz L_4bdba3
        mov edx, dword ptr [edi]
        mov ecx, edi
        mov dword ptr [edi + 0x34], eax
        call dword ptr [edx + 0x74]
    L_4bdba3:
        add esi, 0x4
        dec ebx
        jnz L_4bdb93
        mov eax, dword ptr [ebp]
        pop ebx
        mov dword ptr [edi + 0x34], eax
        pop ebp
        pop edi
        pop esi
        ret
    L_4bdbb4:
        mov edx, dword ptr [edi]
        mov ecx, edi
        call dword ptr [edx + 0x74]
        pop edi
        pop esi
        ret
    }
}

// 0x004bdc00 Widget_SetRectAndParams - (a1..a7) ret 0x1c: vfunc +0xc(a1,a2); this[+0x38..+0x48] = a3..a7
// Register/stack shape from the listing (ECX, EDX, 28 stack bytes).
__declspec(naked) int __fastcall Widget_SetRectAndParams(int, int, int, int, int, int, int, int, int)
{
    __asm {
        mov edx, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 0xc]
        mov eax, dword ptr [esi]
        push ecx
        push edx
        mov ecx, esi
        call dword ptr [eax + 0xc]
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x18]
        mov dword ptr [esi + 0x38], eax
        mov eax, dword ptr [esp + 0x1c]
        mov dword ptr [esi + 0x3c], ecx
        mov ecx, dword ptr [esp + 0x20]
        mov dword ptr [esi + 0x40], edx
        mov dword ptr [esi + 0x44], eax
        mov dword ptr [esi + 0x48], ecx
        pop esi
        ret 0x1c
    }
}

// 0x004bed50 ScreenFX_SetPrimaryRect - ECX=screen fx. Stores u16 arg at +0x60, args at +0x68/+0x6c; embedded object at +0x28: calls its vtable +0x60 with 1 (this=ECX+0x28), clears +0x38, sets bit 1 in +0x34. ret 0xc.
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ScreenFX_SetPrimaryRect(int, int, int, int, int)
{
    __asm {
        mov AX, word ptr [esp + 0x4]
        mov edx, dword ptr [esp + 0x8]
        push esi
        mov word ptr [ecx + 0x60], AX
        mov eax, dword ptr [esp + 0x10]
        lea esi, [ecx + 0x28]
        mov dword ptr [ecx + 0x68], edx
        mov dword ptr [ecx + 0x6c], eax
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0xc]
        mov dword ptr [esi + 0x10], 0x0
        or AL, 0x1
        mov dword ptr [esi + 0xc], eax
        pop esi
        ret 0xc
    }
}

// 0x004bf8b0 LineWidget_SetPoint - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall LineWidget_SetPoint(int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [esp + 0xc]
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 0xc]
        push edi
        mov dword ptr [esi + eax*0x8 + 0x34], ecx
        mov dword ptr [esi + eax*0x8 + 0x38], edx
        cmp dword ptr [esi + 0xdc], eax
        jg L_4bf8d9
        lea edi, [eax + 0x1]
        mov dword ptr [esi + 0xdc], edi
    L_4bf8d9:
        test eax, eax
        jnz L_4bf8e6
        mov eax, dword ptr [esi]
        push edx
        push ecx
        mov ecx, esi
        call dword ptr [eax + 0xc]
    L_4bf8e6:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        pop edi
        pop esi
        ret 0xc
    }
}

// 0x004bff00 ImageWidget_Slot_ComputeBounds - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_Slot_ComputeBounds(int, int)
{
    __asm {
        sub esp, 0x10
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x64]
        test eax, eax
        jle L_4bff1e
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x64]
        mov dword ptr [esp + 0xc], eax
        jmp L_4bff26
    L_4bff1e:
        mov dword ptr [esp + 0xc], 0x0
    L_4bff26:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x68]
        test eax, eax
        jle L_4bff3a
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x68]
        jmp L_4bff3c
    L_4bff3a:
        xor eax, eax
    L_4bff3c:
        mov ecx, dword ptr [esi + 0x34]
        mov dword ptr [esp + 0x10], eax
        test ecx, ecx
        jz L_4bff9e
        mov edx, dword ptr [esi + 0x1c]
        test edx, edx
        jz L_4bff7c
        movsx edi, word ptr [ecx + 0x4]
        mov ebx, dword ptr [esp + 0xc]
        add edi, ebx
        movsx ebx, word ptr [edx + 0x4]
        cmp edi, ebx
        mov dword ptr [esp + 0x14], edi
        jl L_4bff68
        mov dword ptr [esp + 0x14], ebx
    L_4bff68:
        movsx ecx, word ptr [ecx + 0x6]
        add eax, ecx
        movsx ecx, word ptr [edx + 0x6]
        cmp eax, ecx
        jge L_4bff8e
        mov dword ptr [esp + 0x18], eax
        jmp L_4bff92
    L_4bff7c:
        movsx edx, word ptr [ecx + 0x4]
        add edx, dword ptr [esp + 0xc]
        mov dword ptr [esp + 0x14], edx
        movsx ecx, word ptr [ecx + 0x6]
        add ecx, eax
    L_4bff8e:
        mov dword ptr [esp + 0x18], ecx
    L_4bff92:
        mov edx, dword ptr [esi]
        lea eax, [esp + 0xc]
        push eax
        mov ecx, esi
        call dword ptr [edx + 0x1c]
    L_4bff9e:
        pop edi
        pop esi
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004bffb0 ImageWidget_SetPositionAndAnchor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall ImageWidget_SetPositionAndAnchor(int, int, int, int, int, int)
{
    __asm {
        mov edx, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 0xc]
        mov eax, dword ptr [esi]
        push ecx
        push edx
        mov ecx, esi
        call dword ptr [eax + 0xc]
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x14]
        mov dword ptr [esi + 0x34], eax
        mov dword ptr [esi + 0x38], ecx
        pop esi
        ret 0x10
    }
}

// 0x00404e60 Widget_Slot_HitTestVia004bc4e0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_HitTestVia004bc4e0(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        mov edx, dword ptr [esp + 0x4]
        push eax
        push edx
        call Widget_HitTestCircle
        neg AL
        sbb eax, eax
        neg eax
        ret 0x8
    }
}

// 0x00409410 UiScreen_Slot_BroadcastToGroups - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_BroadcastToGroups(int, int, int)
{
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 0x8]
        push ebp
        push edi
        mov ebp, ecx
        push ebx
        call Widget_Tick
        mov edi, dword ptr [ebp + 0x150]
        mov eax, dword ptr [ebp + 0x154]
        cmp edi, eax
        jz L_40945d
        push esi
    L_409430:
        mov esi, dword ptr [edi + 0x4]
        mov eax, dword ptr [edi + 0x8]
        cmp esi, eax
        jz L_40944f
    L_40943a:
        mov eax, dword ptr [esi]
        push ebx
        mov ecx, esi
        call dword ptr [eax + 0x24]
        mov eax, dword ptr [edi + 0x8]
        add esi, 0x2ac
        cmp esi, eax
        jnz L_40943a
    L_40944f:
        mov eax, dword ptr [ebp + 0x154]
        add edi, 0x10
        cmp edi, eax
        jnz L_409430
        pop esi
    L_40945d:
        pop edi
        pop ebp
        pop ebx
        ret 0x4
    }
}

// 0x0040be90 ListLabel_MarkDirty - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_MarkDirty(int, int)
{
    __asm {
        mov dword ptr [ecx + 0x270], 0x1
        jmp Widget_OrFlagsWithGlobal
    }
}

// 0x004b4070 Widget_BaseCtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Widget_BaseCtor(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push edi
        mov ecx, dword ptr [esp + 0x10]
        xor edi, edi
        mov dword ptr [esi + 0x18], ecx
        mov ecx, esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0xa10
        mov dword ptr [esi + 0x8], edi
        mov dword ptr [esi + 0x4], edi
        mov dword ptr [esi + 0x10], edi
        mov dword ptr [esi + 0x14], eax
        call dword ptr [g_RData_004cc000 + 0xa30]
        push edi
        push edi
        mov ecx, esi
        mov dword ptr [esi + 0xc], edi
        mov word ptr [esi + 0x30], DI
        call Widget_SetField1CAndNotify
        mov eax, esi
        pop edi
        pop esi
        ret 0x8
    }
}

// 0x004b43d0 TextBuffer_Assign - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall TextBuffer_Assign(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 0x8]
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [esi + 0x4]
        push eax
        push ecx
        push edx
        call dword ptr [g_Iat_strncpy_004cc5a0]
        mov eax, dword ptr [esi + 0x8]
        mov ecx, dword ptr [esi + 0x4]
        add esp, 0xc
        mov byte ptr [eax + ecx*0x1 - 0x1], 0x0
        mov edx, dword ptr [esi + 0xc]
        push edx
        mov ecx, esi
        call EditField_SetCursor
        pop esi
        ret 0x4
    }
}

// 0x004b4530 ListCursor_Prev_004b4530 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): if [this+0xc]>0: --[this+0xc]; 0x004b45e0(1, idx). Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListCursor_Prev_004b4530(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xc]
        test eax, eax
        jle L_4b4543
        dec eax
        push eax
        push 0x1
        mov dword ptr [ecx + 0xc], eax
        call EditField_CloseGap
    L_4b4543:
        ret
    }
}

// 0x004b4550 ListCursor_Refresh_004b4550 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x004b45e0(1, [this+0xc]). Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListCursor_Refresh_004b4550(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xc]
        push eax
        push 0x1
        call EditField_CloseGap
        ret
    }
}

// 0x004b4810 EditField_SetCaretShape - disassembly re-read 2026-09-24 (caveat resolved): (x, y, d, h) ret 0x10; stores +0xe8=x, +0xec=y, +0xf0=d, +0xf4=h, then 13 LineWidget_SetPoint calls building a closed I-BEAM outline: 0 (x-d,y) 1 (x+d,y) 2 (x+d,y+1) 3 (x,y+1) 4 (x,y+h-1) 5 (x+d,y+h-1) 6 (x+d,y+h) 7 (x-d,y+h) 8 (x-d,y+h-1) 9 (x,y+h-1
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall EditField_SetCaretShape(int, int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov edx, dword ptr [esp + 0xc]
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 0xc]
        push edi
        mov edi, dword ptr [esp + 0x18]
        mov dword ptr [esi + 0xe8], eax
        sub eax, edx
        push ecx
        mov dword ptr [esi + 0xec], ecx
        push eax
        push 0x0
        mov ecx, esi
        mov dword ptr [esi + 0xf0], edx
        mov dword ptr [esi + 0xf4], edi
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xf0]
        mov edi, dword ptr [esi + 0xe8]
        mov eax, dword ptr [esi + 0xec]
        add ecx, edi
        push eax
        push ecx
        push 0x1
        mov ecx, esi
        call LineWidget_SetPoint
        mov edx, dword ptr [esi + 0xec]
        mov eax, dword ptr [esi + 0xf0]
        inc edx
        mov ecx, esi
        push edx
        mov edx, dword ptr [esi + 0xe8]
        add eax, edx
        push eax
        push 0x2
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xec]
        mov edx, dword ptr [esi + 0xe8]
        inc ecx
        push ecx
        push edx
        push 0x3
        mov ecx, esi
        call LineWidget_SetPoint
        mov eax, dword ptr [esi + 0xf4]
        mov ecx, dword ptr [esi + 0xec]
        lea edx, [eax + ecx*0x1 - 0x1]
        mov eax, dword ptr [esi + 0xe8]
        push edx
        push eax
        push 0x4
        mov ecx, esi
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xf4]
        mov edx, dword ptr [esi + 0xec]
        mov edi, dword ptr [esi + 0xe8]
        lea eax, [ecx + edx*0x1 - 0x1]
        mov ecx, dword ptr [esi + 0xf0]
        add ecx, edi
        push eax
        push ecx
        push 0x5
        mov ecx, esi
        call LineWidget_SetPoint
        mov edx, dword ptr [esi + 0xf4]
        mov eax, dword ptr [esi + 0xec]
        add edx, eax
        mov eax, dword ptr [esi + 0xf0]
        push edx
        mov edx, dword ptr [esi + 0xe8]
        add eax, edx
        mov ecx, esi
        push eax
        push 0x6
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xf4]
        mov edx, dword ptr [esi + 0xec]
        mov edi, dword ptr [esi + 0xf0]
        add ecx, edx
        mov edx, dword ptr [esi + 0xe8]
        push ecx
        sub edx, edi
        mov ecx, esi
        push edx
        push 0x7
        call LineWidget_SetPoint
        mov eax, dword ptr [esi + 0xf4]
        mov ecx, dword ptr [esi + 0xec]
        lea edx, [eax + ecx*0x1 - 0x1]
        mov eax, dword ptr [esi + 0xe8]
        push edx
        mov edx, dword ptr [esi + 0xf0]
        sub eax, edx
        mov ecx, esi
        push eax
        push 0x8
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xf4]
        mov edx, dword ptr [esi + 0xec]
        lea eax, [ecx + edx*0x1 - 0x1]
        mov ecx, dword ptr [esi + 0xe8]
        push eax
        push ecx
        push 0x9
        mov ecx, esi
        call LineWidget_SetPoint
        mov edx, dword ptr [esi + 0xec]
        mov eax, dword ptr [esi + 0xe8]
        inc edx
        mov ecx, esi
        push edx
        push eax
        push 0xa
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xec]
        mov edx, dword ptr [esi + 0xe8]
        mov eax, dword ptr [esi + 0xf0]
        inc ecx
        sub edx, eax
        push ecx
        push edx
        push 0xb
        mov ecx, esi
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xe8]
        mov edi, dword ptr [esi + 0xf0]
        mov eax, dword ptr [esi + 0xec]
        sub ecx, edi
        push eax
        push ecx
        push 0xc
        mov ecx, esi
        call LineWidget_SetPoint
        pop edi
        pop esi
        ret 0x10
    }
}

// 0x004b4ed0 EditField_GetText - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EditField_GetText(int, int)
{
    __asm {
        add ecx, 0x14c
        jmp TextBuffer_GetText
    }
}

// 0x004b5350 Button_GetHitRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_GetHitRect(int, int)
{
    __asm {
        push esi
        push edi
        mov edi, ecx
        xor eax, eax
        mov ecx, dword ptr [edi + 0xc4]
        mov esi, dword ptr [edi + 0x3c]
        test ecx, ecx
        jnz L_4b5366
        pop edi
        pop esi
        ret
    L_4b5366:
        push ebp
        push ebx
        test esi, esi
        jz L_4b539f
        mov eax, dword ptr [edi + 0x18]
        mov edx, dword ptr [edi + 0x14]
        lea ecx, [edi + 0xcc]
        mov dword ptr [edi + 0xd0], eax
        mov dword ptr [ecx], edx
        movsx ebx, word ptr [esi + 0x6]
        add ebx, eax
        mov dword ptr [edi + 0xd8], ebx
        pop ebx
        movsx eax, word ptr [esi + 0x4]
        add eax, edx
        pop ebp
        mov dword ptr [edi + 0xd4], eax
        mov eax, ecx
        pop edi
        pop esi
        ret
    L_4b539f:
        mov ecx, dword ptr [edi + 0x110]
        test ecx, ecx
        jz L_4b5621
        mov ecx, dword ptr [ecx]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x68]
        mov dword ptr [edi + 0xd0], eax
        mov eax, dword ptr [edi + 0x110]
        mov ecx, dword ptr [eax]
        call ListLabel_GetLineHeight
        add eax, dword ptr [edi + 0xd0]
        mov dword ptr [edi + 0xd8], eax
        mov esi, dword ptr [edi + 0x110]
        cmp esi, dword ptr [edi + 0x114]
        jz L_4b5600
    L_4b53e5:
        mov ecx, dword ptr [esi]
        call ListLabel_GetLineHeight
        mov edx, dword ptr [edi + 0xd8]
        add edx, eax
        mov dword ptr [edi + 0xd8], edx
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x144]
        sub eax, 0x0
        jz L_4b557e
        dec eax
        jz L_4b5498
        dec eax
        jnz L_4b55ef
        mov ecx, dword ptr [edi + 0x110]
        mov ecx, dword ptr [ecx]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        mov dword ptr [edi + 0xd4], eax
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b5442
        mov eax, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [eax + 0x90]
    L_4b5442:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov ebp, dword ptr [edi + 0xcc]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        sub eax, ebx
        cmp eax, ebp
        jle L_4b548b
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b5471
        mov eax, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [eax + 0x90]
    L_4b5471:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        sub eax, ebx
        mov dword ptr [edi + 0xcc], eax
        jmp L_4b55ef
    L_4b548b:
        mov eax, ebp
        mov dword ptr [edi + 0xcc], eax
        jmp L_4b55ef
    L_4b5498:
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b54ac
        mov eax, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [eax + 0x90]
    L_4b54ac:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov ebp, dword ptr [edi + 0xcc]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        mov ecx, eax
        mov eax, ebx
        cdq
        sub eax, edx
        sar eax, 0x1
        sub ecx, eax
        cmp ecx, ebp
        jge L_4b54fe
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b54e4
        mov edx, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [edx + 0x90]
    L_4b54e4:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x64]
        mov ecx, eax
        mov eax, ebx
        cdq
        sub eax, edx
        sar eax, 0x1
        sub ecx, eax
        jmp L_4b5500
    L_4b54fe:
        mov ecx, ebp
    L_4b5500:
        mov dword ptr [edi + 0xcc], ecx
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b551c
        mov edx, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [edx + 0x90]
    L_4b551c:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov ebp, dword ptr [edi + 0xd4]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x64]
        mov ecx, eax
        mov eax, ebx
        cdq
        sub eax, edx
        sar eax, 0x1
        add ecx, eax
        cmp ecx, ebp
        jle L_4b5574
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b5554
        mov edx, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [edx + 0x90]
    L_4b5554:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x64]
        mov ecx, eax
        mov eax, ebx
        cdq
        sub eax, edx
        sar eax, 0x1
        add ecx, eax
        mov dword ptr [edi + 0xd4], ecx
        jmp L_4b55ef
    L_4b5574:
        mov ecx, ebp
        mov dword ptr [edi + 0xd4], ecx
        jmp L_4b55ef
    L_4b557e:
        mov ecx, dword ptr [edi + 0x110]
        mov ecx, dword ptr [ecx]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        mov dword ptr [edi + 0xcc], eax
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b55a7
        mov eax, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [eax + 0x90]
    L_4b55a7:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov ebp, dword ptr [edi + 0xd4]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        add eax, ebx
        cmp eax, ebp
        jle L_4b55e7
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b55d6
        mov eax, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [eax + 0x90]
    L_4b55d6:
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ebx + 0x25c]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        add eax, ebx
        jmp L_4b55e9
    L_4b55e7:
        mov eax, ebp
    L_4b55e9:
        mov dword ptr [edi + 0xd4], eax
    L_4b55ef:
        mov eax, dword ptr [edi + 0x114]
        add esi, 0x4
        cmp esi, eax
        jnz L_4b53e5
    L_4b5600:
        mov eax, dword ptr [edi + 0x110]
        mov ecx, dword ptr [eax]
        call ListLabel_GetLineHeight
        mov ecx, dword ptr [edi + 0xd8]
        sub ecx, eax
        lea eax, [edi + 0xcc]
        mov dword ptr [edi + 0xd8], ecx
    L_4b5621:
        pop ebx
        pop ebp
        pop edi
        pop esi
        ret
    }
}

// 0x004b5740 ToggleButton_Slot_RefreshVisuals - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ToggleButton_Slot_RefreshVisuals(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 0x120]
        mov eax, dword ptr [esi + 0x124]
        cmp edi, eax
        jz L_4b576a
    L_4b5754:
        mov ecx, dword ptr [edi]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x124]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b5754
    L_4b576a:
        mov edi, dword ptr [esi + 0x130]
        mov eax, dword ptr [esi + 0x134]
        cmp edi, eax
        jz L_4b5790
    L_4b577a:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x134]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b577a
    L_4b5790:
        mov eax, dword ptr [esi + 0xc4]
        mov edi, dword ptr [esi + 0x110]
        test eax, eax
        mov eax, dword ptr [esi + 0x114]
        jz L_4b57fe
        cmp edi, eax
        jz L_4b57c0
    L_4b57aa:
        mov ecx, dword ptr [edi]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b57aa
    L_4b57c0:
        mov edi, dword ptr [esi + 0x140]
        mov eax, dword ptr [esi + 0x144]
        cmp edi, eax
        jz L_4b57e6
    L_4b57d0:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x144]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b57d0
    L_4b57e6:
        mov eax, dword ptr [esi + 0xdc]
        mov ecx, esi
        push eax
        call ImageWidget_SetImageNoOwn
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        pop edi
        pop esi
        ret
    L_4b57fe:
        cmp edi, eax
        jz L_4b5818
    L_4b5802:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b5802
    L_4b5818:
        mov edi, dword ptr [esi + 0x140]
        mov eax, dword ptr [esi + 0x144]
        cmp edi, eax
        jz L_4b583e
    L_4b5828:
        mov ecx, dword ptr [edi]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x144]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b5828
    L_4b583e:
        mov ecx, dword ptr [esi + 0xe0]
        push ecx
        mov ecx, esi
        call ImageWidget_SetImageNoOwn
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        pop edi
        pop esi
        ret
    }
}

// 0x004b5860 Button_Reset - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_Reset(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0xdc]
        test eax, eax
        jz L_4b5874
        push eax
        call ImageWidget_SetImageNoOwn
    L_4b5874:
        mov eax, dword ptr [esi + 0xec]
        test eax, eax
        jz L_4b5888
        mov dword ptr [esi + 0xec], 0x0
    L_4b5888:
        mov edi, dword ptr [esi + 0x120]
        mov eax, dword ptr [esi + 0x124]
        cmp edi, eax
        jz L_4b58ae
    L_4b5898:
        mov ecx, dword ptr [edi]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x124]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b5898
    L_4b58ae:
        mov edi, dword ptr [esi + 0x130]
        mov eax, dword ptr [esi + 0x134]
        cmp edi, eax
        jz L_4b58d4
    L_4b58be:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x134]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b58be
    L_4b58d4:
        mov edi, dword ptr [esi + 0x110]
        mov eax, dword ptr [esi + 0x114]
        cmp edi, eax
        jz L_4b58fa
    L_4b58e4:
        mov ecx, dword ptr [edi]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b58e4
    L_4b58fa:
        pop edi
        pop esi
        ret
    }
}

// 0x004b70c0 ToggleImageButton_Slot_RefreshVisuals - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ToggleImageButton_Slot_RefreshVisuals(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 0x120]
        mov eax, dword ptr [esi + 0x124]
        cmp edi, eax
        jz L_4b70ea
    L_4b70d4:
        mov ecx, dword ptr [edi]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x124]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b70d4
    L_4b70ea:
        mov edi, dword ptr [esi + 0x130]
        mov eax, dword ptr [esi + 0x134]
        cmp edi, eax
        jz L_4b7110
    L_4b70fa:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x134]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b70fa
    L_4b7110:
        mov eax, dword ptr [esi + 0xc4]
        mov edi, dword ptr [esi + 0x110]
        test eax, eax
        mov eax, dword ptr [esi + 0x114]
        jz L_4b719a
        cmp edi, eax
        jz L_4b7140
    L_4b712a:
        mov ecx, dword ptr [edi]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b712a
    L_4b7140:
        mov edi, dword ptr [esi + 0x140]
        mov eax, dword ptr [esi + 0x144]
        cmp edi, eax
        jz L_4b7166
    L_4b7150:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x144]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b7150
    L_4b7166:
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jz L_4b7200
        mov eax, dword ptr [esi + 0x15c]
        test eax, eax
        jnz L_4b71f8
        mov eax, dword ptr [esi + 0x158]
        test eax, eax
        jz L_4b7200
        push eax
        mov ecx, esi
        call ImageWidget_SetImageNoOwn
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        pop edi
        pop esi
        ret
    L_4b719a:
        cmp edi, eax
        jz L_4b71b4
    L_4b719e:
        mov ecx, dword ptr [edi]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b719e
    L_4b71b4:
        mov edi, dword ptr [esi + 0x140]
        mov eax, dword ptr [esi + 0x144]
        cmp edi, eax
        jz L_4b71da
    L_4b71c4:
        mov ecx, dword ptr [edi]
        push 0x1
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x144]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b71c4
    L_4b71da:
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jz L_4b7200
        mov eax, dword ptr [esi + 0x150]
        test eax, eax
        jnz L_4b71f8
        mov eax, dword ptr [esi + 0x154]
        test eax, eax
        jz L_4b7200
    L_4b71f8:
        push eax
        mov ecx, esi
        call ImageWidget_SetImageNoOwn
    L_4b7200:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        pop edi
        pop esi
        ret
    }
}

// 0x004b72c0 Toggle_SetState - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Toggle_SetState(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push edi
        test eax, eax
        mov edi, dword ptr [esi + 0x14c]
        mov dword ptr [esi + 0x14c], eax
        jz L_4b7307
        mov eax, dword ptr [esi + 0x15c]
        test eax, eax
        jz L_4b72e8
        push eax
        call ImageWidget_SetImageNoOwn
    L_4b72e8:
        mov ecx, dword ptr [esi + 0x160]
        test ecx, ecx
        jz L_4b732a
        mov eax, dword ptr [ecx]
        push 0x1
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        mov eax, edi
        pop edi
        pop esi
        ret 0x4
    L_4b7307:
        mov eax, dword ptr [esi + 0x158]
        test eax, eax
        jz L_4b7319
        push eax
        mov ecx, esi
        call ImageWidget_SetImageNoOwn
    L_4b7319:
        mov ecx, dword ptr [esi + 0x160]
        test ecx, ecx
        jz L_4b732a
        mov edx, dword ptr [ecx]
        push 0x0
        call dword ptr [edx + 0x60]
    L_4b732a:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        mov eax, edi
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004b7e60 RadioGroup_Slot_UpdateButtons - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall RadioGroup_Slot_UpdateButtons(int, int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        xor ebx, ebx
        mov eax, dword ptr [esi + 0x150]
        test eax, eax
        jle L_4b7eca
        push edi
        lea edi, [esi + 0x1b8]
    L_4b7e77:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        mov ecx, dword ptr [edi - 0x50]
        test ecx, ecx
        jz L_4b7e9d
        cmp ebx, dword ptr [esi + 0x14c]
        jnz L_4b7e96
        mov edx, dword ptr [ecx]
        push 0x1
        call dword ptr [edx + 0x60]
        jmp L_4b7e9d
    L_4b7e96:
        mov eax, dword ptr [ecx]
        push 0x0
        call dword ptr [eax + 0x60]
    L_4b7e9d:
        mov ecx, dword ptr [edi]
        test ecx, ecx
        jz L_4b7ebb
        cmp ebx, dword ptr [esi + 0x14c]
        jnz L_4b7eb4
        mov edx, dword ptr [ecx]
        push 0x1
        call dword ptr [edx + 0x60]
        jmp L_4b7ebb
    L_4b7eb4:
        mov eax, dword ptr [ecx]
        push 0x0
        call dword ptr [eax + 0x60]
    L_4b7ebb:
        mov eax, dword ptr [esi + 0x150]
        inc ebx
        add edi, 0x4
        cmp ebx, eax
        jl L_4b7e77
        pop edi
    L_4b7eca:
        mov ecx, dword ptr [esp + 0xc]
        push ecx
        mov ecx, esi
        call Widget_Tick
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004b8a90 ScrollGroupChild_SetSelected - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScrollGroupChild_SetSelected(int, int, int)
{
    __asm {
        mov edx, dword ptr [ecx + 0xc4]
        mov eax, dword ptr [esp + 0x4]
        test edx, edx
        mov dword ptr [ecx + 0x14c], eax
        jz L_4b8ae0
        test eax, eax
        jz L_4b8abc
        mov eax, dword ptr [ecx + 0x16c]
        mov edx, dword ptr [ecx + 0x174]
        mov dword ptr [ecx + 0xdc], eax
        jmp L_4b8ace
    L_4b8abc:
        mov eax, dword ptr [ecx + 0x170]
        mov edx, dword ptr [ecx + 0x178]
        mov dword ptr [ecx + 0xdc], eax
    L_4b8ace:
        mov eax, dword ptr [ecx + 0xdc]
        mov dword ptr [ecx + 0xe4], edx
        push eax
        call ImageWidget_SetImageNoOwn
    L_4b8ae0:
        ret 0x4
    }
}

// 0x004b9320 Thunk_004b9330 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Thunk_004b9330(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push eax
        call ListScreen_ScrollTo
        ret 0x4
    }
}

// 0x004ba0c0 ScreenBase_BindButton - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ScreenBase_BindButton(int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0xc]
        mov edx, dword ptr [esp + 0x8]
        push eax
        push edx
        mov edx, dword ptr [ecx + 0xa940]
        call ScreenBase_BindButtonImpl
        and eax, 0xff
        ret 0xc
    }
}

// 0x004ba0e0 ScreenBase_BindWidget - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ScreenBase_BindWidget(int, int, int, int, int)
{
    __asm {
        sub esp, 0x20
        push ebp
        mov ebp, ecx
        push edi
        xor edi, edi
        mov ecx, dword ptr [ebp + 0xa940]
        cmp ecx, edi
        jnz L_4ba0fd
        xor eax, eax
        pop edi
        pop ebp
        add esp, 0x20
        ret 0xc
    L_4ba0fd:
        push esi
        push ebx
        mov edx, offset g_Data_004da000 + 0xa864
        call ConfigTree_FindChild
        mov ebx, eax
        cmp ebx, edi
        jz L_4ba340
        mov edx, dword ptr [esp + 0x3c]
        mov ecx, ebx
        call ConfigTree_FindChild
        mov ebx, eax
        cmp ebx, edi
        jz L_4ba340
        mov esi, dword ptr [esp + 0x38]
        mov ecx, ebp
        push esi
        call Widget_AppendChild
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, ebx
        call ConfigTree_FindChild
        cmp eax, edi
        jz L_4ba152
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        push ecx
        mov ecx, esi
        call ImageWidget_SetImage
    L_4ba152:
        mov edx, offset g_Data_004da000 + 0xa710
        mov ecx, ebx
        call ConfigTree_FindChild
        cmp eax, edi
        jz L_4ba180
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [ebp + 0xa948]
        mov edx, dword ptr [esi]
        add ecx, dword ptr [eax + 0x14]
        push ecx
        mov ecx, dword ptr [ebp + 0xa944]
        add ecx, dword ptr [eax + 0xc]
        push ecx
        mov ecx, esi
        call dword ptr [edx + 0xc]
    L_4ba180:
        mov edx, offset g_Data_004da000 + 0xa858
        mov ecx, ebx
        call ConfigTree_FindChild
        cmp eax, edi
        jz L_4ba1b5
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x10], edi
        mov dword ptr [esp + 0x14], edi
        mov ecx, esi
        mov edx, dword ptr [eax + 0xc]
        mov dword ptr [esp + 0x18], edx
        mov eax, dword ptr [eax + 0x14]
        mov edx, dword ptr [esi]
        mov dword ptr [esp + 0x1c], eax
        lea eax, [esp + 0x10]
        push eax
        call dword ptr [edx + 0x6c]
    L_4ba1b5:
        mov edx, offset g_Data_004da000 + 0xa75c
        mov ecx, ebx
        call ConfigTree_FindChild
        cmp eax, edi
        jz L_4ba24f
        mov eax, dword ptr [eax + 0x4]
        lea ecx, [eax + eax*0x8]
        mov edi, dword ptr [ebp + ecx*0x4 + 0x1cec]
        lea eax, [ebp + ecx*0x4 + 0x1cec]
        neg edi
        sbb edi, edi
        and edi, eax
        test edi, edi
        jz L_4ba24f
        mov edx, dword ptr [edi + 0x20]
        mov eax, dword ptr [esi]
        push 0x2
        mov dword ptr [esi + 0x144], edx
        mov ecx, dword ptr [edi + 0x1c]
        mov edx, dword ptr [edi + 0x8]
        push 0x0
        push 0x0
        push 0x0
        push ecx
        mov ecx, dword ptr [edi + 0x4]
        push edx
        push ecx
        mov ecx, esi
        call dword ptr [eax + 0x80]
        mov eax, dword ptr [edi + 0xc]
        mov dword ptr [esi + 0x14c], eax
        mov dword ptr [esi + 0x150], eax
        mov eax, 0x1
        mov dword ptr [esi + 0x270], eax
        mov edx, dword ptr [edi + 0x18]
        mov dword ptr [esi + 0x264], edx
        mov dword ptr [esi + 0x29c], eax
        mov dword ptr [esi + 0x2a0], eax
        mov eax, dword ptr [edi + 0x10]
        mov ecx, dword ptr [edi + 0x14]
        mov dword ptr [esi + 0x26c], eax
        mov dword ptr [esi + 0x268], ecx
    L_4ba24f:
        mov edx, offset g_Data_004da000 + 0x1428
        mov ecx, ebx
        call ConfigTree_FindChild
        test eax, eax
        jz L_4ba279
        mov eax, dword ptr [eax + 0x4]
        mov DL, byte ptr [eax + 0x1c]
        mov CL, byte ptr [eax + 0xc]
        push edx
        mov DL, byte ptr [eax + 0x14]
        call Pixel_FromRGBBytes
        and eax, 0xffff
        mov dword ptr [esi + 0x3c], eax
    L_4ba279:
        mov edx, offset g_Data_004da000 + 0xa84c
        mov ecx, ebx
        call ConfigTree_FindChild
        mov edi, eax
        test edi, edi
        jz L_4ba2c2
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x68]
        mov ecx, dword ptr [edi + 0x4]
        mov edx, dword ptr [ecx + 0x14]
        mov ecx, esi
        add eax, edx
        mov edx, dword ptr [esi]
        push eax
        call dword ptr [edx + 0x64]
        mov ecx, dword ptr [edi + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov ecx, esi
        add eax, edx
        mov edx, dword ptr [esi]
        push eax
        call dword ptr [edx + 0x68]
        push eax
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        push eax
        mov ecx, esi
        call ImageWidget_SetPositionAndAnchor
    L_4ba2c2:
        mov edx, offset g_Data_004da000 + 0xa840
        mov ecx, ebx
        call ConfigTree_FindChild
        test eax, eax
        jz L_4ba2f4
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0x14]
        mov edx, dword ptr [eax + 0xc]
        mov eax, dword ptr [esi]
        push ecx
        push edx
        mov ecx, esi
        call dword ptr [eax + 0x68]
        mov edx, dword ptr [esi]
        push eax
        mov ecx, esi
        call dword ptr [edx + 0x64]
        push eax
        mov ecx, esi
        call ImageWidget_SetPositionAndAnchor
    L_4ba2f4:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        mov edx, dword ptr [esi]
        mov ecx, esi
        mov dword ptr [esp + 0x20], eax
        call dword ptr [edx + 0x68]
        mov dword ptr [esp + 0x24], eax
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        mov edx, dword ptr [esi]
        mov ecx, esi
        mov dword ptr [esp + 0x28], eax
        call dword ptr [edx + 0x68]
        mov edx, dword ptr [ebp + 0x118]
        lea ecx, [esp + 0x20]
        mov dword ptr [esp + 0x2c], eax
        mov eax, dword ptr [esi]
        push ecx
        push edx
        mov ecx, esi
        call dword ptr [eax + 0x18]
        xor eax, eax
        mov AL, byte ptr [esi + 0xc]
        and eax, 0x10
        or AL, 0x2
        mov dword ptr [esi + 0xc], eax
    L_4ba340:
        pop ebx
        pop esi
        pop edi
        xor eax, eax
        pop ebp
        add esp, 0x20
        ret 0xc
    }
}

// 0x004bb3d0 ListWidget_HitTest - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ListWidget_HitTest(int, int, int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jz L_4bb42e
        mov ebx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esi + 0x14]
        cmp eax, ebx
        jg L_4bb42e
        mov edi, dword ptr [esp + 0x14]
        mov eax, dword ptr [esi + 0x18]
        cmp eax, edi
        jg L_4bb42e
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4bb406
        mov edx, dword ptr [esi]
        call dword ptr [edx + 0x90]
    L_4bb406:
        mov eax, dword ptr [esi + 0x25c]
        mov edx, dword ptr [esi + 0x14]
        add eax, edx
        cmp ebx, eax
        jge L_4bb42e
        mov ecx, esi
        call ListLabel_GetLineHeight
        add eax, dword ptr [esi + 0x18]
        cmp edi, eax
        jge L_4bb42e
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        ret 0x8
    L_4bb42e:
        pop edi
        pop esi
        xor eax, eax
        pop ebx
        ret 0x8
    }
}

// 0x004bb740 ListWidget_GetRect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListWidget_GetRect(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        push edi
        call Widget_GetExtentRect
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4bb762
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x90]
    L_4bb762:
        mov ecx, dword ptr [esi + 0x25c]
        mov edx, dword ptr [edi]
        add ecx, edx
        mov dword ptr [edi + 0x8], ecx
        mov ecx, esi
        call ListLabel_GetLineHeight
        add eax, dword ptr [edi + 0x4]
        mov dword ptr [edi + 0xc], eax
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004bb9f0 ListWidget_SetPosition - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ListWidget_SetPosition(int, int, int, int)
{
    __asm {
        push ecx
        mov eax, dword ptr [esp + 0x8]
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esi]
        mov dword ptr [esi + 0x18], ecx
        mov ecx, esi
        mov dword ptr [esi + 0x14], eax
        call dword ptr [edx + 0x20]
        mov ecx, esi
        call ListLabel_GetLineHeight
        xor ebx, ebx
        mov dword ptr [esp + 0x10], eax
        mov dword ptr [esp + 0x18], ebx
        mov dword ptr [esp + 0x1c], ebx
    L_4bba21:
        mov eax, dword ptr [esi + 0x2ac]
        test eax, eax
        jnz L_4bba2f
        xor edx, edx
        jmp L_4bba48
    L_4bba2f:
        mov ecx, dword ptr [esi + 0x2b0]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bba48:
        cmp dword ptr [esp + 0x18], edx
        jnc L_4bba94
        mov ecx, dword ptr [esi + 0x2ac]
        mov edx, dword ptr [esi]
        mov ebp, dword ptr [ecx + ebx*0x1]
        lea edi, [ecx + ebx*0x1]
        mov ecx, esi
        call dword ptr [edx + 0x68]
        mov ecx, dword ptr [esp + 0x1c]
        add eax, ecx
        mov ecx, esi
        push eax
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x64]
        push eax
        mov ecx, edi
        call dword ptr [ebp + 0xc]
        mov edx, dword ptr [esp + 0x18]
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esp + 0x1c]
        inc edx
        add ebx, 0x2c0
        add eax, ecx
        mov dword ptr [esp + 0x18], edx
        mov dword ptr [esp + 0x1c], eax
        jmp L_4bba21
    L_4bba94:
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x8
    }
}

// 0x004bbb20 ListWidget_Slot_AppendRow - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListWidget_Slot_AppendRow(int, int)
{
    __asm {
        push ecx
        push ebx
        push ebp
        push esi
        mov esi, ecx
        xor ebx, ebx
        push edi
        mov edx, dword ptr [esi + 0x2a4]
        inc edx
        mov dword ptr [esi + 0x2a4], edx
        mov eax, dword ptr [esi + 0x2ac]
        cmp eax, ebx
        mov edi, edx
        jnz L_4bbb46
        xor edx, edx
        jmp L_4bbb5f
    L_4bbb46:
        mov ecx, dword ptr [esi + 0x2b0]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bbb5f:
        cmp edi, edx
        jc L_4bbbcf
        mov dword ptr [esp + 0x10], ebx
    L_4bbb67:
        mov eax, dword ptr [esi + 0x2ac]
        test eax, eax
        jnz L_4bbb75
        xor edx, edx
        jmp L_4bbb8e
    L_4bbb75:
        mov ecx, dword ptr [esi + 0x2b0]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
    L_4bbb8e:
        mov eax, dword ptr [esp + 0x10]
        dec edx
        cmp eax, edx
        jnc L_4bbbc9
        mov eax, dword ptr [esi + 0x2ac]
        mov ebp, dword ptr [ebx + eax*0x1]
        lea edi, [ebx + eax*0x1]
        lea ecx, [eax + ebx*0x1 + 0x2c0]
        call ListLabel_GetText
        push eax
        mov ecx, edi
        call dword ptr [ebp + 0x8c]
        mov ecx, dword ptr [esp + 0x10]
        inc ecx
        add ebx, 0x2c0
        mov dword ptr [esp + 0x10], ecx
        jmp L_4bbb67
    L_4bbbc9:
        dec dword ptr [esi + 0x2a4]
    L_4bbbcf:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret
    }
}

// 0x004bbe90 ListWidget_ClearAll - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListWidget_ClearAll(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x2ac]
        test eax, eax
        jnz L_4bbea4
        xor edx, edx
        push edx
        push edx
        call ListWidget_ClearRange
        ret
    L_4bbea4:
        mov edx, dword ptr [ecx + 0x2b0]
        sub edx, eax
        mov eax, 0x2e8ba2e9
        imul edx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
        push edx
        push 0x0
        call ListWidget_ClearRange
        ret
    }
}

// 0x004bc510 ScreenRoot_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenRoot_Ctor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call WidgetContainer_Ctor
        mov eax, dword ptr [esp + 0x8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7c50
        mov dword ptr [esi + 0x40], eax
        mov dword ptr [esi + 0x10], 0x0
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004bc540 ScreenRoot_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScreenRoot_Dtor(int, int)
{
    __asm {
        mov dword ptr [ecx], offset g_RData_004cc000 + 0x7c50
        jmp WidgetContainer_Dtor
    }
}

// 0x004bc860 WidgetList_Remove - Finds node via 0x004bc810(node, &prev); not found -> 0. Unlinks from singly linked list (head +8, tail +0xc, next +4): updates head or prev->next, tail if node was tail; clears node +4/+8. Returns 1; ret 4.
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall WidgetList_Remove(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        lea eax, [esp + 0x8]
        push edi
        push eax
        mov edi, ecx
        push esi
        call WidgetList_FindPrev
        test eax, eax
        jz L_4bc8c5
        mov eax, dword ptr [esp + 0xc]
        xor ecx, ecx
        cmp eax, ecx
        jz L_4bc8a5
        mov edx, dword ptr [esi + 0x4]
        mov dword ptr [eax + 0x4], edx
        mov eax, dword ptr [edi + 0xc]
        cmp esi, eax
        jnz L_4bc8b5
        mov eax, dword ptr [esp + 0xc]
        mov dword ptr [edi + 0xc], eax
        mov dword ptr [esi + 0x4], ecx
        mov dword ptr [esi + 0x8], ecx
        mov eax, 0x1
        pop edi
        pop esi
        ret 0x4
    L_4bc8a5:
        mov edx, dword ptr [edi + 0xc]
        mov eax, dword ptr [esi + 0x4]
        cmp esi, edx
        mov dword ptr [edi + 0x8], eax
        jnz L_4bc8b5
        mov dword ptr [edi + 0xc], eax
    L_4bc8b5:
        mov dword ptr [esi + 0x4], ecx
        mov dword ptr [esi + 0x8], ecx
        mov eax, 0x1
        pop edi
        pop esi
        ret 0x4
    L_4bc8c5:
        pop edi
        xor eax, eax
        pop esi
        ret 0x4
    }
}

// 0x004bc980 ListLabel_StartBlinkMode1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabel_StartBlinkMode1(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        cmp dword ptr [esi + 0x2b8], 0x1
        jz L_4bc9a0
        mov eax, dword ptr [esp + 0x8]
        push eax
        call ListLabel_StartBlink
        mov dword ptr [esi + 0x2b8], 0x1
    L_4bc9a0:
        pop esi
        ret 0x4
    }
}

// 0x004bc9b0 ListLabel_StartColourAnim - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ListLabel_StartColourAnim(int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        cmp dword ptr [esi + 0x2b8], 0x2
        jz L_4bc9e0
        mov eax, dword ptr [esp + 0xc]
        push eax
        call ListLabel_StartBlink
        mov eax, dword ptr [esp + 0x8]
        mov dword ptr [esi + 0x2b8], 0x2
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b0], eax
    L_4bc9e0:
        pop esi
        ret 0x8
    }
}

// 0x004bcc80 TextWidget_Assign - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall TextWidget_Assign(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        push edi
        call Widget_Assign
        lea eax, [edi + 0x34]
        push 0x100
        lea ecx, [esi + 0x34]
        push eax
        push ecx
        call dword ptr [g_Iat_strncpy_004cc5a0]
        mov edx, dword ptr [edi + 0x134]
        add esp, 0xc
        mov dword ptr [esi + 0x134], edx
        mov eax, dword ptr [edi + 0x138]
        mov dword ptr [esi + 0x138], eax
        mov ecx, dword ptr [edi + 0x13c]
        mov dword ptr [esi + 0x13c], ecx
        mov edx, dword ptr [edi + 0x140]
        mov dword ptr [esi + 0x140], edx
        mov eax, dword ptr [edi + 0x144]
        mov dword ptr [esi + 0x144], eax
        mov eax, esi
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004bd160 MessagePanel_PushLine - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall MessagePanel_PushLine(int, int, int, int)
{
    __asm {
        push ecx
        push ebx
        mov ebx, ecx
        push esi
        push 0x1
        mov eax, dword ptr [ebx]
        mov dword ptr [esp + 0xc], ebx
        call dword ptr [eax + 0x4]
        mov ecx, dword ptr [ebx + 0x1c]
        not ecx
        test CL, 0x10
        jz L_4bd249
        push edi
        push ebp
        lea ebp, [ebx + 0x10]
        mov ecx, ebp
        call ListLabel_GetText
        mov ecx, dword ptr [esp + 0x18]
        mov esi, eax
    L_4bd190:
        mov DL, byte ptr [ecx]
        mov AL, DL
        cmp DL, byte ptr [esi]
        jnz L_4bd1b4
        test AL, AL
        jz L_4bd1b0
        mov DL, byte ptr [ecx + 0x1]
        mov AL, DL
        cmp DL, byte ptr [esi + 0x1]
        jnz L_4bd1b4
        add ecx, 0x2
        add esi, 0x2
        test AL, AL
        jnz L_4bd190
    L_4bd1b0:
        xor eax, eax
        jmp L_4bd1b9
    L_4bd1b4:
        sbb eax, eax
        sbb eax, -0x1
    L_4bd1b9:
        test eax, eax
        jz L_4bd247
        lea edi, [ebx + 0x558]
        cmp edi, ebp
        jc L_4bd247
        lea esi, [edi + 0x2a4]
    L_4bd1d1:
        mov eax, dword ptr [esi + 0xfffffd68]
        not eax
        test AL, 0x10
        jz L_4bd237
        mov edx, dword ptr [edi]
        push 0x0
        mov ecx, edi
        call dword ptr [edx + 0x60]
        fld dword ptr [esi + 0xfffffd6c]
        push ecx
        mov ecx, esi
        fstp dword ptr [esp]
        call Widget_SetLifetime
        mov ebx, dword ptr [esi]
        mov ecx, edi
        call ListLabel_GetText
        push eax
        push esi
        call dword ptr [ebx + 0x74]
        mov ecx, dword ptr [esi + 0xfffffea8]
        mov eax, dword ptr [esi + 0xfffffeac]
        mov edx, dword ptr [esi]
        add esp, 0x8
        mov dword ptr [esi + 0x14c], ecx
        mov ecx, esi
        push 0x1
        mov dword ptr [esi + 0x150], eax
        mov dword ptr [esi + 0x270], 0x1
        call dword ptr [edx + 0x60]
        mov ebx, dword ptr [esp + 0x10]
    L_4bd237:
        sub edi, 0x2a4
        sub esi, 0x2a4
        cmp edi, ebp
        jnc L_4bd1d1
    L_4bd247:
        pop ebp
        pop edi
    L_4bd249:
        mov eax, dword ptr [esp + 0x14]
        lea esi, [ebx + 0x10]
        push eax
        mov ecx, esi
        call Widget_SetLifetime
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esi]
        push edx
        push offset g_Data_004da000 + 0x97c
        push esi
        call dword ptr [ecx + 0x74]
        mov eax, dword ptr [esi]
        add esp, 0xc
        mov ecx, esi
        push 0x1
        call dword ptr [eax + 0x60]
        mov eax, esi
        pop esi
        pop ebx
        pop ecx
        ret 0x8
    }
}

// 0x004bd880 Seg_ClipAgainstXBounds - bytes ret 8 (outA,outB): both > xmax 0x004e4888 or both < xmin 0x004e4878 -> flags=1, return 0; else Seg_ClipToX on crossing ends
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Seg_ClipAgainstXBounds(int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, edx
        fld dword ptr [esi]
        fcomp dword ptr [g_Data_004da000 + 0xa888]
        fnstsw AX
        test AH, 0x41
        jnz L_4bd8bf
        fld dword ptr [edi]
        fcomp dword ptr [g_Data_004da000 + 0xa888]
        fnstsw AX
        test AH, 0x41
        jnz L_4bd8bf
        mov eax, dword ptr [esp + 0xc]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax], 0x1
        mov dword ptr [ecx], 0x1
        xor eax, eax
        pop edi
        pop esi
        ret 0x8
    L_4bd8bf:
        fld dword ptr [esi]
        fcomp dword ptr [g_Data_004da000 + 0xa878]
        fnstsw AX
        test AH, 0x1
        jz L_4bd8f8
        fld dword ptr [edi]
        fcomp dword ptr [g_Data_004da000 + 0xa878]
        fnstsw AX
        test AH, 0x1
        jz L_4bd8f8
        mov edx, dword ptr [esp + 0xc]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx], 0x1
        mov dword ptr [eax], 0x1
        xor eax, eax
        pop edi
        pop esi
        ret 0x8
    L_4bd8f8:
        fld dword ptr [esi]
        fcomp dword ptr [g_Data_004da000 + 0xa878]
        fnstsw AX
        test AH, 0x1
        jz L_4bd923
        mov ecx, dword ptr [g_Data_004da000 + 0xa878]
        mov edx, edi
        push ecx
        mov ecx, esi
        call Seg_ClipToX
        mov edx, dword ptr [esp + 0xc]
        mov dword ptr [edx], 0x1
        jmp L_4bd94b
    L_4bd923:
        fld dword ptr [esi]
        fcomp dword ptr [g_Data_004da000 + 0xa888]
        fnstsw AX
        test AH, 0x41
        jnz L_4bd94b
        mov eax, dword ptr [g_Data_004da000 + 0xa888]
        mov edx, edi
        push eax
        mov ecx, esi
        call Seg_ClipToX
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [ecx], 0x1
    L_4bd94b:
        fld dword ptr [edi]
        fcomp dword ptr [g_Data_004da000 + 0xa878]
        fnstsw AX
        test AH, 0x1
        jz L_4bd97e
        mov edx, dword ptr [g_Data_004da000 + 0xa878]
        mov ecx, edi
        push edx
        mov edx, esi
        call Seg_ClipToX
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax], 0x1
        mov eax, 0x1
        pop edi
        pop esi
        ret 0x8
    L_4bd97e:
        fld dword ptr [edi]
        fcomp dword ptr [g_Data_004da000 + 0xa888]
        fnstsw AX
        test AH, 0x41
        jnz L_4bd9a7
        mov ecx, dword ptr [g_Data_004da000 + 0xa888]
        mov edx, esi
        push ecx
        mov ecx, edi
        call Seg_ClipToX
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx], 0x1
    L_4bd9a7:
        pop edi
        mov eax, 0x1
        pop esi
        ret 0x8
    }
}

// 0x004bd9f0 Seg_ClipAgainstYBounds - bytes ret 8: same as Seg_ClipAgainstXBounds on y (0x004e488c/0x004e487c) with Seg_ClipToY
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Seg_ClipAgainstYBounds(int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, edx
        fld dword ptr [esi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa88c]
        fnstsw AX
        test AH, 0x41
        jnz L_4bda31
        fld dword ptr [edi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa88c]
        fnstsw AX
        test AH, 0x41
        jnz L_4bda31
        mov eax, dword ptr [esp + 0xc]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax], 0x1
        mov dword ptr [ecx], 0x1
        xor eax, eax
        pop edi
        pop esi
        ret 0x8
    L_4bda31:
        fld dword ptr [esi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa87c]
        fnstsw AX
        test AH, 0x1
        jz L_4bda6c
        fld dword ptr [edi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa87c]
        fnstsw AX
        test AH, 0x1
        jz L_4bda6c
        mov edx, dword ptr [esp + 0xc]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx], 0x1
        mov dword ptr [eax], 0x1
        xor eax, eax
        pop edi
        pop esi
        ret 0x8
    L_4bda6c:
        fld dword ptr [esi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa87c]
        fnstsw AX
        test AH, 0x1
        jz L_4bda98
        mov ecx, dword ptr [g_Data_004da000 + 0xa87c]
        mov edx, edi
        push ecx
        mov ecx, esi
        call Seg_ClipToY
        mov edx, dword ptr [esp + 0xc]
        mov dword ptr [edx], 0x1
        jmp L_4bdac1
    L_4bda98:
        fld dword ptr [esi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa88c]
        fnstsw AX
        test AH, 0x41
        jnz L_4bdac1
        mov eax, dword ptr [g_Data_004da000 + 0xa88c]
        mov edx, edi
        push eax
        mov ecx, esi
        call Seg_ClipToY
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [ecx], 0x1
    L_4bdac1:
        fld dword ptr [edi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa87c]
        fnstsw AX
        test AH, 0x1
        jz L_4bdaf5
        mov edx, dword ptr [g_Data_004da000 + 0xa87c]
        mov ecx, edi
        push edx
        mov edx, esi
        call Seg_ClipToY
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax], 0x1
        mov eax, 0x1
        pop edi
        pop esi
        ret 0x8
    L_4bdaf5:
        fld dword ptr [edi + 0x4]
        fcomp dword ptr [g_Data_004da000 + 0xa88c]
        fnstsw AX
        test AH, 0x41
        jnz L_4bdb1f
        mov ecx, dword ptr [g_Data_004da000 + 0xa88c]
        mov edx, esi
        push ecx
        mov ecx, edi
        call Seg_ClipToY
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx], 0x1
    L_4bdb1f:
        pop edi
        mov eax, 0x1
        pop esi
        ret 0x8
    }
}

// 0x004bed30 Manager_0056bd58_ResetTimer - ../../04_spec/systems/render.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Manager_0056bd58_ResetTimer(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push eax
        call Manager_BroadcastVirtual24
        mov dword ptr [esi + 0x1ec], 0x0
        pop esi
        ret 0x4
    }
}

// 0x004bed90 ScreenFX_AllocateRectSlot - ECX=screen fx. Slot = ECX+0x70+count*0x4c (count [+0x1ec]; incremented only while <4, so slot 4 is reused when full). Calls 0x004bdc00 with the 7 args on the slot, then slot vtable +0x60(1), clears slot+0x10, sets bit 1 in slot+0xc. ret 0x1c.
// Register/stack shape from the listing (ECX, EDX, 28 stack bytes).
__declspec(naked) int __fastcall ScreenFX_AllocateRectSlot(int, int, int, int, int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x1ec]
        push esi
        cmp eax, 0x4
        lea edx, [eax + eax*0x8]
        lea edx, [eax + edx*0x2]
        lea esi, [ecx + edx*0x4 + 0x70]
        jge L_4bedad
        inc eax
        mov dword ptr [ecx + 0x1ec], eax
    L_4bedad:
        mov eax, dword ptr [esp + 0x20]
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x18]
        push eax
        mov eax, dword ptr [esp + 0x18]
        push ecx
        mov ecx, dword ptr [esp + 0x18]
        push edx
        mov edx, dword ptr [esp + 0x18]
        push eax
        mov eax, dword ptr [esp + 0x18]
        push ecx
        push edx
        push eax
        mov ecx, esi
        call Widget_SetRectAndParams
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0xc]
        mov dword ptr [esi + 0x10], 0x0
        or AL, 0x1
        mov dword ptr [esi + 0xc], eax
        pop esi
        ret 0x1c
    }
}

// 0x004beee0 ScreenFX_QueueFade - bytes read: stores the 16-bit colour (ECX low word) and the double argument, ECX = 0x0056bd58 screen-effects object; call 0x004bed50; ret 8
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ScreenFX_QueueFade(int, int, int, int)
{
    __asm {
        push ecx
        mov eax, dword ptr [esp + 0xc]
        mov word ptr [esp], CX
        mov ecx, dword ptr [esp + 0x8]
        mov edx, dword ptr [esp]
        push eax
        push ecx
        push edx
        mov ecx, offset g_Data_004da000 + 0x91d58
        call ScreenFX_SetPrimaryRect
        pop ecx
        ret 0x8
    }
}

// 0x004bef40 GlobalCompositePanel_SetSlot - bytes: push edx, ecx; ecx=0x0056bd58; call CompositePanel_SetSlot(i=ECX,v=EDX); callers 0x0042f3c3/0x0042f44c
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GlobalCompositePanel_SetSlot(int, int)
{
    __asm {
        push edx
        push ecx
        mov ecx, offset g_Data_004da000 + 0x91d58
        call CompositePanel_SetSlot
        ret
    }
}

// 0x004bef50 SetRect_Object0056bd58 - ../../04_spec/systems/render.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall SetRect_Object0056bd58(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push eax
        mov eax, dword ptr [esp + 0x8]
        push eax
        push edx
        push ecx
        mov ecx, offset g_Data_004da000 + 0x91d58
        call Object_SetRect18
        ret 0x8
    }
}

// 0x004b4e60 EditField_SetText - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_SetText(int, int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esp + 0x10]
        lea ebx, [esi + 0x14c]
        push edi
        mov ecx, ebx
        call TextBuffer_Assign
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        not ecx
        dec ecx
        push ecx
        mov ecx, ebx
        call EditField_SetCursor
        mov ecx, ebx
        call TextBuffer_GetText
        mov edx, dword ptr [esi + 0x110]
        test edx, edx
        jnz L_4b4e9e
        xor ecx, ecx
        jmp L_4b4ea9
    L_4b4e9e:
        mov ecx, dword ptr [esi + 0x114]
        sub ecx, edx
        sar ecx, 0x2
    L_4b4ea9:
        test ecx, ecx
        jz L_4b4ebe
        mov ecx, dword ptr [esi + 0x110]
        push eax
        mov ecx, dword ptr [ecx]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x8c]
    L_4b4ebe:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x0041a2d0 IntEditField_ParseClamped - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall IntEditField_ParseClamped(int, int)
{
    __asm {
        sub esp, 0x14
        push esi
        push edi
        mov edi, ecx
        call EditField_GetText
        test eax, eax
        jz L_41a2f3
        cmp byte ptr [eax], 0x0
        jz L_41a2f3
        push eax
        call dword ptr [g_Iat_atoi_004cc5d4]
        add esp, 0x4
        mov esi, eax
        jmp L_41a2f9
    L_41a2f3:
        mov esi, dword ptr [edi + 0x374]
    L_41a2f9:
        mov ecx, dword ptr [edi + 0x374]
        cmp esi, ecx
        jge L_41a305
        mov esi, ecx
    L_41a305:
        mov edx, dword ptr [edi + 0x378]
        cmp esi, edx
        jle L_41a311
        mov esi, edx
    L_41a311:
        cmp esi, ecx
        mov eax, esi
        jge L_41a319
        mov eax, ecx
    L_41a319:
        cmp eax, edx
        jle L_41a31f
        mov eax, edx
    L_41a31f:
        push eax
        lea eax, [esp + 0xc]
        push offset g_Data_004da000 + 0xcbc
        push eax
        call dword ptr [g_Iat_sprintf_004cc5c4]
        add esp, 0xc
        lea ecx, [esp + 0x8]
        push ecx
        mov ecx, edi
        call EditField_SetText
        mov eax, esi
        pop edi
        pop esi
        add esp, 0x14
        ret
    }
}

// 0x004b3d00 ImageWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ImageWidget_Ctor(int, int, int)
{
    __asm {
        push esi
        push edi
        xor edi, edi
        mov esi, ecx
        push edi
        push edi
        call Widget_BaseCtor
        mov eax, dword ptr [esp + 0xc]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7428
        mov dword ptr [esi + 0x48], eax
        mov dword ptr [esi + 0x3c], edi
        mov dword ptr [esi + 0x34], edi
        mov dword ptr [esi + 0x44], edi
        mov word ptr [esi + 0x40], DI
        mov dword ptr [esi + 0x38], edi
        lea eax, [esi + 0x4c]
        mov ecx, 0x4
    L_4b3d32:
        mov dword ptr [eax], edi
        add eax, 0x1c
        dec ecx
        jnz L_4b3d32
        mov eax, esi
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004b87e0 Toggle_OnReleaseIfOff - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Toggle_OnReleaseIfOff(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x14c]
        test eax, eax
        jnz L_4b87ef
        jmp Button_Reset
    L_4b87ef:
        ret
    }
}

// 0x004b8af0 Cycler_GetLabel - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Cycler_GetLabel(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x158]
        test eax, eax
        jz L_4b8b01
        lea eax, [ecx + 0x15c]
        ret
    L_4b8b01:
        jmp Button_GetHitRect
    }
}

// 0x004b8cf0 ScrollGroup_Select - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScrollGroup_Select(int, int, int)
{
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 0x8]
        push esi
        push edi
        mov dword ptr [ecx + 0x178], ebx
        xor esi, esi
        lea edi, [ecx + 0x150]
    L_4b8d05:
        mov ecx, dword ptr [edi]
        test ecx, ecx
        jz L_4b8d18
        xor eax, eax
        cmp esi, ebx
        setz AL
        push eax
        call ScrollGroupChild_SetSelected
    L_4b8d18:
        inc esi
        add edi, 0x4
        cmp esi, 0xa
        jl L_4b8d05
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        ret 0x4
    }
}

// 0x004bc480 RingWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall RingWidget_Ctor(int, int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push esi
        mov esi, ecx
        push eax
        mov ecx, dword ptr [esp + 0xc]
        push ecx
        mov ecx, esi
        call Widget_BaseCtor
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7bd8
        mov edx, eax
        mov dword ptr [esi + 0x34], eax
        imul edx, eax
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esi + 0x38], edx
        mov dword ptr [esi + 0x3c], eax
        mov eax, esi
        pop esi
        ret 0x10
    }
}

// 0x004bd280 MessageOverlay_Show - bytes read: push stack arg and ECX (text); ECX = [0x0056bd24] message overlay instance; call 0x004bd160; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MessageOverlay_Show(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push eax
        push ecx
        mov ecx, dword ptr [g_Data_004da000 + 0x91d24]
        call MessagePanel_PushLine
        ret 0x4
    }
}

// 0x004bef10 ScreenFX_QueueFlashRect - bytes read: forwards five stack args plus EDX and ECX to ScreenFX_AllocateRectSlot with ECX = 0x0056bd58 screen-effects object
// Register/stack shape from the listing (ECX, EDX, 20 stack bytes).
__declspec(naked) int __fastcall ScreenFX_QueueFlashRect(int, int, int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x14]
        push eax
        mov eax, dword ptr [esp + 0x14]
        push eax
        mov eax, dword ptr [esp + 0x14]
        push eax
        mov eax, dword ptr [esp + 0x14]
        push eax
        mov eax, dword ptr [esp + 0x14]
        push eax
        push edx
        push ecx
        mov ecx, offset g_Data_004da000 + 0x91d58
        call ScreenFX_AllocateRectSlot
        ret 0x14
    }
}

// 0x004bef70 Manager_0056bd58_Tick - ../../04_spec/systems/render_frame.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Manager_0056bd58_Tick(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov ecx, offset g_Data_004da000 + 0x91d58
        push eax
        call Manager_0056bd58_ResetTimer
        ret 0x4
    }
}

// 0x0040c9c0 OptionToggle_Refresh_Bit10 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Refresh_Bit10(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_GfxFlags
        and eax, 0x10
        mov ecx, esi
        push eax
        call Toggle_SetState
        pop esi
        ret
    }
}

// 0x0040ca20 OptionToggle_Refresh_Bit08 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Refresh_Bit08(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_GfxFlags
        and eax, 0x8
        mov ecx, esi
        push eax
        call Toggle_SetState
        pop esi
        ret
    }
}

// 0x0040ca80 OptionToggle_Refresh_00408360Is2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Refresh_00408360Is2(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_HUDType
        xor ecx, ecx
        cmp eax, 0x2
        setz CL
        push ecx
        mov ecx, esi
        call Toggle_SetState
        pop esi
        ret
    }
}

// 0x0040cab0 OptionCycler_Refresh_From00408030 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Refresh_From00408030(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_004e5d10
        push eax
        mov ecx, esi
        call Cycler_SetIndex
        pop esi
        ret
    }
}

// 0x0040caf0 OptionCycler_Refresh_From00408100 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Refresh_From00408100(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_TextureMemory
        push eax
        mov ecx, esi
        call Cycler_SetIndex
        pop esi
        ret
    }
}

// 0x0040cb30 OptionCycler_Refresh_HWCardGated - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Refresh_HWCardGated(int, int)
{
    __asm {
        push esi
        push edi
        mov edi, ecx
        call Setting_Get_EffectsLevel
        mov esi, eax
        call Settings_GetHWCardFlag
        test eax, eax
        jnz L_40cb58
        test esi, esi
        jnz L_40cb4d
        mov esi, 0x1
    L_40cb4d:
        push 0x3
        push 0x1
        mov ecx, edi
        call Cycler_SetRange
    L_40cb58:
        push esi
        mov ecx, edi
        call Cycler_SetIndex
        pop edi
        pop esi
        ret
    }
}

// 0x0040cb90 OptionToggle_Refresh_Not00408060 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Refresh_Not00408060(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_MuteSound
        neg eax
        sbb eax, eax
        mov ecx, esi
        inc eax
        push eax
        call Toggle_SetState
        pop esi
        ret
    }
}

// 0x0040cbd0 OptionCycler_Refresh_From004080d0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Refresh_From004080d0(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_SoundLOD
        push eax
        mov ecx, esi
        call Cycler_SetIndex
        pop esi
        ret
    }
}

// 0x0040cc10 OptionControl_Refresh_FloatFrom00408090 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionControl_Refresh_FloatFrom00408090(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi]
        call Setting_GetFloat_004e5d44
        push ecx
        mov ecx, esi
        fstp dword ptr [esp]
        call dword ptr [edi + 0x84]
        pop edi
        pop esi
        ret
    }
}

// 0x0040cc60 OptionToggle_Refresh_From00408220 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Refresh_From00408220(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Setting_Get_CDAudio
        push eax
        mov ecx, esi
        call Toggle_SetState
        pop esi
        ret
    }
}


// 0x004bcd80 TextWidget_UpdateBoundsFromText - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextWidget_UpdateBoundsFromText(int, int)
{
    __asm {
        sub esp, 0x8
        lea eax, [esp + 0x4]
        push esi
        mov esi, ecx
        lea ecx, [esp + 0x4]
        push eax
        mov edx, dword ptr [esi + 0x134]
        push ecx
        lea ecx, [esi + 0x34]
        call BitmapFont_MeasureString
        mov edx, dword ptr [esi + 0x20]
        mov eax, dword ptr [esp + 0x4]
        mov ecx, dword ptr [esp + 0x8]
        add edx, eax
        mov eax, dword ptr [esi + 0x24]
        mov dword ptr [esi + 0x28], edx
        add eax, ecx
        mov dword ptr [esi + 0x2c], eax
        pop esi
        add esp, 0x8
        ret
    }
}

// 0x004bcdc0 TextWidget_MeasureWidth - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextWidget_MeasureWidth(int, int)
{
    __asm {
        sub esp, 0x8
        lea eax, [esp + 0x4]
        lea edx, [esp]
        add ecx, 0x34
        push eax
        push edx
        mov edx, dword ptr [ecx + 0x100]
        call BitmapFont_MeasureString
        mov eax, dword ptr [esp]
        add esp, 0x8
        ret
    }
}

// 0x004bcea0 TextWidget_HitTest - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall TextWidget_HitTest(int, int, int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        mov ebx, dword ptr [esp + 0x10]
        push edi
        mov edi, dword ptr [esp + 0x10]
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jz L_4bcec7
        cmp dword ptr [esi + 0x14], edi
        jg L_4bcec7
        cmp dword ptr [esi + 0x18], ebx
        jg L_4bcec7
        mov eax, 0x1
        jmp L_4bcec9
    L_4bcec7:
        xor eax, eax
    L_4bcec9:
        test eax, eax
        jz L_4bcf0c
        lea ecx, [esp + 0x10]
        lea edx, [esp + 0x14]
        push ecx
        push edx
        mov edx, dword ptr [esi + 0x134]
        lea ecx, [esi + 0x34]
        call BitmapFont_MeasureString
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esp + 0x14]
        add eax, edx
        cmp edi, eax
        jg L_4bcf0a
        mov ecx, dword ptr [esi + 0x18]
        mov edx, dword ptr [esp + 0x10]
        add ecx, edx
        cmp ebx, ecx
        jg L_4bcf0a
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        ret 0x8
    L_4bcf0a:
        xor eax, eax
    L_4bcf0c:
        pop edi
        pop esi
        pop ebx
        ret 0x8
    }
}

// 0x004bcff0 Widget_Slot_Virtual08_Then004936d0IfField4C - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_Virtual08_Then004936d0IfField4C(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x8]
        mov edx, dword ptr [esi + 0x130]
        test edx, edx
        jz L_4bd011
        mov ecx, dword ptr [esi + 0x134]
        push ecx
        lea ecx, [esi + 0x34]
        call Draw_FillConvexPolygon2D
    L_4bd011:
        pop esi
        ret
    }
}

// 0x004bf900 LineWidget_Slot_Draw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall LineWidget_Slot_Draw(int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi]
        call dword ptr [eax + 0x8]
        mov eax, dword ptr [edi + 0xdc]
        test eax, eax
        jz L_4bf970
        mov ecx, dword ptr [edi + 0xe4]
        test ecx, ecx
        jz L_4bf937
        mov edx, dword ptr [edi + 0xe0]
        push edx
        push ecx
        lea edx, [eax - 0x1]
        lea ecx, [edi + 0x34]
        call Draw_PolylineViaHook
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    L_4bf937:
        xor ebp, ebp
        dec eax
        test eax, eax
        lea esi, [edi + 0x34]
        lea ebx, [edi + 0x3c]
        jle L_4bf970
        sub ebx, esi
    L_4bf946:
        mov eax, dword ptr [edi + 0xe0]
        mov ecx, dword ptr [esi + ebx*0x1 + 0x4]
        mov edx, dword ptr [esi + ebx*0x1]
        push eax
        push ecx
        mov ecx, dword ptr [esi]
        push edx
        mov edx, dword ptr [esi + ebx*0x1 - 0x4]
        call Draw_LineViaHook
        mov eax, dword ptr [edi + 0xdc]
        inc ebp
        add esi, 0x8
        dec eax
        cmp ebp, eax
        jl L_4bf946
    L_4bf970:
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}

// 0x00435a10 ListScreen_Slot_ReselectIfActive - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListScreen_Slot_ReselectIfActive(int, int)
{
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [eax + 0x8]
        test ecx, ecx
        jz L_435a25
        mov eax, dword ptr [eax + 0x2a4]
        push eax
        call SaveLoadDialog_SelectIndex
    L_435a25:
        ret
    }
}

// 0x00403c80 Thunk_Widget_Slot_Virtual08_Then00498fb0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_Widget_Slot_Virtual08_Then00498fb0(int, int)
{
    __asm {
        jmp Widget_Slot_Virtual08_Then00498fb0
    }
}

// 0x004b47b0 LineWidget_Slot_TickBlink - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall LineWidget_Slot_TickBlink(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xc]
        not eax
        test AL, 0x10
        jz L_4b4808
        mov eax, dword ptr [ecx + 0xf8]
        test eax, eax
        jz L_4b4803
        fld dword ptr [ecx + 0x100]
        fsub dword ptr [esp + 0x4]
        fcom qword ptr [g_RData_004cc000 + 0x74c8]
        fstp dword ptr [ecx + 0x100]
        fnstsw AX
        test AH, 0x1
        jz L_4b47fa
        mov edx, dword ptr [ecx + 0x104]
        mov eax, dword ptr [ecx + 0xfc]
        neg edx
        mov dword ptr [ecx + 0x104], edx
        mov dword ptr [ecx + 0x100], eax
    L_4b47fa:
        cmp dword ptr [ecx + 0x104], 0x1
        jnz L_4b4808
    L_4b4803:
        call LineWidget_Slot_Draw
    L_4b4808:
        ret 0x4
    }
}

// 0x004bcdf0 TextWidget_CentreInBox - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextWidget_CentreInBox(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call TextWidget_MeasureWidth
        mov ecx, dword ptr [esi + 0x13c]
        mov edx, eax
        mov eax, dword ptr [esi + 0x140]
        sub eax, edx
        sub eax, ecx
        cdq
        sub eax, edx
        sar eax, 0x1
        add eax, ecx
        mov ecx, dword ptr [esi + 0x1c]
        test ecx, ecx
        mov dword ptr [esi + 0x14], eax
        jz L_4bce2b
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [esi + 0x20], eax
        mov dword ptr [esi + 0x24], ecx
        mov ecx, esi
        call TextWidget_UpdateBoundsFromText
    L_4bce2b:
        pop esi
        ret
    }
}

// 0x004bb540 ListLabel_Printf - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_Printf(int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push ebx
        push esi
        push edi
        test eax, eax
        jnz L_4bb567
        mov edx, dword ptr [esp + 0x10]
        mov ecx, 0x40
        lea edi, [edx + 0x34]
        rep stosd
        mov dword ptr [edx + 0x270], 0x1
        pop edi
        pop esi
        pop ebx
        ret
    L_4bb567:
        mov esi, dword ptr [esp + 0x10]
        lea ecx, [esp + 0x18]
        push ecx
        push eax
        lea edi, [esi + 0x34]
        push 0x100
        push edi
        call dword ptr [g_Iat__vsnprintf_004cc530]
        add esp, 0x10
        lea ebx, [esi + 0x15c]
        push 0x100
        push edi
        push ebx
        call dword ptr [g_Iat_strncmp_004cc5cc]
        add esp, 0xc
        test eax, eax
        jz L_4bb5cf
        mov eax, dword ptr [esi + 0x138]
        test eax, eax
        jz L_4bb5ae
        mov ecx, esi
        call TextWidget_CentreInBox
    L_4bb5ae:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        push 0x100
        push edi
        push ebx
        mov dword ptr [esi + 0x270], 0x1
        call dword ptr [g_Iat_strncpy_004cc5a0]
        add esp, 0xc
    L_4bb5cf:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x004bb5e0 ListLabel_VPrintf - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_VPrintf(int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push ebx
        push esi
        push edi
        test eax, eax
        jnz L_4bb607
        mov edx, dword ptr [esp + 0x10]
        mov ecx, 0x40
        lea edi, [edx + 0x34]
        rep stosd
        mov dword ptr [edx + 0x270], 0x1
        pop edi
        pop esi
        pop ebx
        ret
    L_4bb607:
        mov esi, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        push ecx
        push eax
        lea edi, [esi + 0x34]
        push 0x100
        push edi
        call dword ptr [g_Iat__vsnprintf_004cc530]
        add esp, 0x10
        lea ebx, [esi + 0x15c]
        push 0x100
        push edi
        push ebx
        call dword ptr [g_Iat_strncmp_004cc5cc]
        add esp, 0xc
        test eax, eax
        jz L_4bb66f
        mov eax, dword ptr [esi + 0x138]
        test eax, eax
        jz L_4bb64e
        mov ecx, esi
        call TextWidget_CentreInBox
    L_4bb64e:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        push 0x100
        push edi
        push ebx
        mov dword ptr [esi + 0x270], 0x1
        call dword ptr [g_Iat_strncpy_004cc5a0]
        add esp, 0xc
    L_4bb66f:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x004bb680 ListLabel_SetText - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabel_SetText(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push ebx
        push ebp
        push esi
        test eax, eax
        push edi
        mov esi, ecx
        jnz L_4bb6a9
        mov ecx, 0x40
        lea edi, [esi + 0x34]
        rep stosd
        mov dword ptr [esi + 0x270], 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    L_4bb6a9:
        mov ebp, dword ptr [g_Iat_strncpy_004cc5a0]
        lea edi, [esi + 0x34]
        push 0x100
        push eax
        push edi
        call ebp
        add esp, 0xc
        lea ebx, [esi + 0x15c]
        push 0x100
        push edi
        push ebx
        call dword ptr [g_Iat_strncmp_004cc5cc]
        add esp, 0xc
        test eax, eax
        jz L_4bb706
        mov eax, dword ptr [esi + 0x138]
        test eax, eax
        jz L_4bb6e9
        mov ecx, esi
        call TextWidget_CentreInBox
    L_4bb6e9:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        push 0x100
        push edi
        push ebx
        mov dword ptr [esi + 0x270], 0x1
        call ebp
        add esp, 0xc
    L_4bb706:
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    }
}

// 0x004bccf0 TextWidget_Printf - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextWidget_Printf(int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        test eax, eax
        jnz L_4bcd09
        mov edx, dword ptr [esp + 0x4]
        push edi
        mov ecx, 0x40
        lea edi, [edx + 0x34]
        rep stosd
        pop edi
        ret
    L_4bcd09:
        push esi
        mov esi, dword ptr [esp + 0x8]
        lea ecx, [esp + 0x10]
        push ecx
        lea edx, [esi + 0x34]
        push eax
        push edx
        call dword ptr [g_Iat_vsprintf_004cc4a8]
        mov eax, dword ptr [esi + 0x138]
        add esp, 0xc
        test eax, eax
        jz L_4bcd32
        mov ecx, esi
        call TextWidget_CentreInBox
    L_4bcd32:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        pop esi
        ret
    }
}

// 0x004a5b40 Call004a5bf0_If004a5b20 - ../../04_spec/structs/weapon_two_struct_split.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Call004a5bf0_If004a5b20(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call CallOptionalHook_0056b568
        test eax, eax
        jz L_4a5b55
        mov ecx, eax
        call Message_GetText
        pop esi
        ret
    L_4a5b55:
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004bfba0 AnimImageWidget_CaptureFrame - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall AnimImageWidget_CaptureFrame(int, int, int, int)
{
    __asm {
        sub esp, 0x20
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov ecx, dword ptr [esi + 0xbc]
        test ecx, ecx
        jz L_4bfc40
        mov eax, dword ptr [esi + 0x3c]
        mov edi, dword ptr [esp + 0x34]
        mov ebx, dword ptr [esp + 0x30]
        mov dword ptr [esp + 0x10], edi
        movsx edx, word ptr [eax + 0x6]
        add edx, edi
        mov dword ptr [esp + 0xc], ebx
        mov dword ptr [esp + 0x18], edx
        push ecx
        movsx eax, word ptr [eax + 0x4]
        mov ecx, dword ptr [esi + 0xc4]
        add eax, ebx
        lea edx, [esp + 0x10]
        mov dword ptr [esp + 0x18], eax
        call zVideo_CopySurfaceRectToImage
        test eax, eax
        jz L_4bfc35
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esp + 0xc]
        mov edx, dword ptr [esp + 0x18]
        sub ecx, edi
        mov dword ptr [esp + 0x20], ecx
        mov ecx, dword ptr [esp + 0x14]
        sub eax, ebx
        sub ecx, ebx
        mov dword ptr [esp + 0x1c], eax
        sub edx, edi
        mov dword ptr [esp + 0x24], ecx
        mov ecx, dword ptr [esi + 0xbc]
        lea eax, [esp + 0x1c]
        mov dword ptr [esp + 0x28], edx
        mov edx, dword ptr [esi]
        push eax
        push ecx
        mov ecx, esi
        call dword ptr [edx + 0x18]
        pop edi
        pop esi
        pop ebx
        add esp, 0x20
        ret 0x8
    L_4bfc35:
        mov edx, dword ptr [esi]
        push 0x0
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x18]
    L_4bfc40:
        pop edi
        pop esi
        pop ebx
        add esp, 0x20
        ret 0x8
    }
}

// 0x004bdbc0 Widget_Slot_Call0048d6d0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_Call0048d6d0(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x44]
        mov edx, dword ptr [ecx + 0x34]
        push eax
        mov eax, dword ptr [ecx + 0x40]
        mov CX, word ptr [ecx + 0x38]
        push eax
        call Screen_SetFadeFill
        ret
    }
}

// 0x00403cb0 RingWidget_Slot_UpdateShrink - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall RingWidget_Slot_UpdateShrink(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jz L_403d60
        mov edx, dword ptr [esi]
        push edi
        mov edi, dword ptr [esi + 0x34]
        call dword ptr [edx + 0x64]
        sub eax, edi
        mov edi, dword ptr [esi + 0x34]
        mov dword ptr [esi + 0x20], eax
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x68]
        mov edx, dword ptr [esi]
        sub eax, edi
        mov edi, dword ptr [esi + 0x34]
        mov ecx, esi
        mov dword ptr [esi + 0x24], eax
        call dword ptr [edx + 0x64]
        mov edx, dword ptr [esi]
        lea eax, [eax + edi*0x1 + 0x1]
        mov edi, dword ptr [esi + 0x34]
        mov ecx, esi
        mov dword ptr [esi + 0x28], eax
        call dword ptr [edx + 0x68]
        lea eax, [eax + edi*0x1 + 0x1]
        mov edi, dword ptr [esi + 0x34]
        cmp edi, 0x3
        mov dword ptr [esi + 0x2c], eax
        jle L_403d38
        fld dword ptr [esp + 0xc]
        fmul qword ptr [g_RData_004cc000 + 0x868]
        fcom qword ptr [g_RData_004cc000 + 0x870]
        fnstsw AX
        test AH, 0x1
        jz L_403d26
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x878]
    L_403d26:
        call dword ptr [g_Iat__ftol_004cc5ac]
        sub edi, eax
        mov ecx, edi
        mov dword ptr [esi + 0x34], edi
        imul ecx, edi
        mov dword ptr [esi + 0x38], ecx
    L_403d38:
        cmp dword ptr [esi + 0x34], 0x3
        jge L_403d4c
        mov dword ptr [esi + 0x34], 0x3
        mov dword ptr [esi + 0x38], 0x9
    L_403d4c:
        mov edx, dword ptr [esp + 0xc]
        mov ecx, esi
        push edx
        call Widget_Tick
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        pop edi
    L_403d60:
        pop esi
        ret 0x4
    }
}

// 0x00404d70 Widget_BaseScalarDeletingDtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Widget_BaseScalarDeletingDtor(int, int, int)
{
    __asm {
        mov AL, byte ptr [esp + 0x4]
        push esi
        mov esi, ecx
        test AL, 0x1
        mov dword ptr [esi], offset g_RData_004cc000 + 0xa10
        jz L_404d8a
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_404d8a:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00407170 ScalarDeletingDtor_00407170 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): vptr=0x004ccd50; if (flags&1) operator delete(this); return this. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_00407170(int, int, int)
{
    __asm {
        mov AL, byte ptr [esp + 0x4]
        push esi
        mov esi, ecx
        test AL, 0x1
        mov dword ptr [esi], offset g_RData_004cc000 + 0xd50
        jz L_40718a
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40718a:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040cd30 OptionCycler_Refresh_DisplayMode - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Refresh_DisplayMode(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Settings_GetHWCardFlag
        test eax, eax
        jz L_40cdac
        call Setting_Get_DisplayMode
        add eax, -0x2
        cmp eax, 0x5
        ja L_40ce46
        cmp eax, 0
        je L_40cd96
        cmp eax, 1
        je L_40cd6a
        cmp eax, 2
        je L_40cd80
        cmp eax, 3
        je L_40cd54
        cmp eax, 4
        je L_40ce1c
        cmp eax, 5
        je L_40ce32
        int 3  // unreachable: the bounds check above excludes other indices
    L_40cd54:
        push 0x0
        mov ecx, esi
        call Cycler_SetIndex
        push 0x1
        push 0x0
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40cd6a:
        push 0x1
        mov ecx, esi
        call Cycler_SetIndex
        push 0x2
        push 0x1
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40cd80:
        push 0x2
        mov ecx, esi
        call Cycler_SetIndex
        push 0x3
        push 0x2
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40cd96:
        push 0x3
        mov ecx, esi
        call Cycler_SetIndex
        push 0x4
        push 0x3
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40cdac:
        call Setting_Get_DisplayMode
        add eax, -0x2
        cmp eax, 0x5
        ja L_40ce46
        cmp eax, 0
        je L_40ce06
        cmp eax, 1
        je L_40cdda
        cmp eax, 2
        je L_40cdf0
        cmp eax, 3
        je L_40cdc4
        cmp eax, 4
        je L_40ce1c
        cmp eax, 5
        je L_40ce32
        int 3  // unreachable: the bounds check above excludes other indices
    L_40cdc4:
        push 0x0
        mov ecx, esi
        call Cycler_SetIndex
        push 0x2
        push 0x0
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40cdda:
        push 0x1
        mov ecx, esi
        call Cycler_SetIndex
        push 0x2
        push 0x0
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40cdf0:
        push 0x2
        mov ecx, esi
        call Cycler_SetIndex
        push 0x4
        push 0x2
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40ce06:
        push 0x3
        mov ecx, esi
        call Cycler_SetIndex
        push 0x4
        push 0x2
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40ce1c:
        push 0x4
        mov ecx, esi
        call Cycler_SetIndex
        push 0x5
        push 0x4
        mov ecx, esi
        call Cycler_SetRange
        pop esi
        ret
    L_40ce32:
        push 0x5
        mov ecx, esi
        call Cycler_SetIndex
        push 0x6
        push 0x5
        mov ecx, esi
        call Cycler_SetRange
    L_40ce46:
        pop esi
        ret
    }
}

// 0x0040ed20 ScrollingWidget_Slot_Tick - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScrollingWidget_Slot_Tick(int, int, int)
{
    __asm {
        push ecx
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x2a8]
        test eax, eax
        jnz L_40ed67
        call Settings_GetNetworkFlag
        fild dword ptr [esi + 0x2ac]
        test eax, eax
        jz L_40ed45
        fmul dword ptr [g_Data_004da000 + 0x9142c]
        jmp L_40ed4b
    L_40ed45:
        fmul dword ptr [g_Data_004da000 + 0x91424]
    L_40ed4b:
        fadd dword ptr [esi + 0x2a4]
        mov ecx, esi
        fst dword ptr [esp + 0x4]
        mov eax, dword ptr [esp + 0x4]
        fstp dword ptr [esi + 0x2a4]
        push eax
        call HudTimer_SetSeconds
    L_40ed67:
        mov ecx, dword ptr [esp + 0xc]
        push ecx
        mov ecx, esi
        call Widget_Tick
        pop esi
        pop ecx
        ret 0x4
    }
}

// 0x0041ebb0 ScalarDeletingDtor_004b47a0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b47a0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Widget_BaseDtor
        test byte ptr [esp + 0x8], 0x1
        jz L_41ebc8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_41ebc8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b4370 KeyDispatch_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall KeyDispatch_Dtor(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [ecx], offset g_RData_004cc000 + 0x74d8
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        ret
    }
}

// 0x004b4390 EditField_ResizeBuffer - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_ResizeBuffer(int, int, int)
{
    __asm {
        push ebx
        push esi
        push edi
        mov edi, dword ptr [esp + 0x10]
        mov esi, ecx
        push edi
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov ecx, dword ptr [esi + 0x4]
        add esp, 0x4
        test ecx, ecx
        mov ebx, eax
        jz L_4b43c0
        mov eax, dword ptr [esi + 0x8]
        cmp edi, eax
        jge L_4b43b4
        mov eax, edi
    L_4b43b4:
        push eax
        push ecx
        push ebx
        call dword ptr [g_Iat_strncpy_004cc5a0]
        add esp, 0xc
    L_4b43c0:
        mov dword ptr [esi + 0x8], edi
        mov dword ptr [esi + 0x4], ebx
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004b4460 KeyDispatch_OnChar - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall KeyDispatch_OnChar(int, int, int)
{
    __asm {
        mov edx, dword ptr [esp + 0x4]
        movsx eax, DL
        movsx eax, byte ptr [eax + ecx*0x1 + 0x10]
        cmp eax, 0x7
        ja L_4b44b6
        cmp eax, 0
        je L_4b4478
        cmp eax, 1
        je L_4b4480
        cmp eax, 2
        je L_4b4489
        cmp eax, 3
        je L_4b4491
        cmp eax, 4
        je L_4b4499
        cmp eax, 5
        je L_4b44a1
        cmp eax, 6
        je L_4b44a9
        cmp eax, 7
        je L_4b44b1
        int 3  // unreachable: the bounds check above excludes other indices
    L_4b4478:
        mov eax, dword ptr [ecx]
        push edx
        call dword ptr [eax]
        ret 0x4
    L_4b4480:
        mov eax, dword ptr [ecx]
        push edx
        call dword ptr [eax + 0x4]
        ret 0x4
    L_4b4489:
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0xc]
        ret 0x4
    L_4b4491:
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x8]
        ret 0x4
    L_4b4499:
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x10]
        ret 0x4
    L_4b44a1:
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x14]
        ret 0x4
    L_4b44a9:
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x18]
        ret 0x4
    L_4b44b1:
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x1c]
    L_4b44b6:
        ret 0x4
    }
}

// 0x004b86b0 Slider_SetValue - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slider_SetValue(int, int, int)
{
    __asm {
        push ecx
        push ebx
        push esi
        mov esi, ecx
        xor ebx, ebx
        push edi
        cmp dword ptr [esi + 0x150], ebx
        jz L_4b874c
        mov eax, dword ptr [esp + 0x14]
        mov edx, dword ptr [esi]
        mov dword ptr [esi + 0x14c], eax
        call dword ptr [edx + 0x20]
        mov edi, dword ptr [esi + 0x150]
        mov dword ptr [esi + 0x158], ebx
        movsx eax, word ptr [edi + 0x6]
        mov dword ptr [esi + 0x160], eax
        mov dword ptr [esi + 0x154], ebx
        movsx ecx, word ptr [edi + 0x4]
        mov dword ptr [esp + 0xc], ecx
        fild dword ptr [esp + 0xc]
        fmul dword ptr [esp + 0x14]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov ecx, dword ptr [esi + 0x16c]
        mov dword ptr [esi + 0x15c], eax
        cmp ecx, ebx
        mov dword ptr [esi + 0x164], ebx
        mov dword ptr [esi + 0x168], ebx
        jz L_4b874c
        mov dword ptr [esi + 0x174], ebx
        movsx edx, word ptr [edi + 0x6]
        mov dword ptr [esi + 0x17c], edx
        mov dword ptr [esi + 0x170], eax
        movsx ecx, word ptr [ecx + 0x4]
        mov dword ptr [esi + 0x178], ecx
        mov dword ptr [esi + 0x180], eax
        mov dword ptr [esi + 0x184], ebx
    L_4b874c:
        pop edi
        pop esi
        pop ebx
        pop ecx
        ret 0x4
    }
}

// 0x004ba470 PtrVector_Free - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall PtrVector_Free(int, int)
{
    __asm {
        push ecx
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x4]
        push eax
        mov dword ptr [esp + 0x8], eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        xor eax, eax
        mov dword ptr [esi + 0x4], eax
        mov dword ptr [esi + 0x8], eax
        mov dword ptr [esi + 0xc], eax
        pop esi
        pop ecx
        ret
    }
}

// 0x004ba510 PtrVector_InsertN - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall PtrVector_InsertN(int, int, int, int, int)
{
    __asm {
        sub esp, 0x8
        push ebx
        mov ebx, ecx
        push ebp
        mov ebp, dword ptr [esp + 0x18]
        mov ecx, dword ptr [ebx + 0x8]
        mov eax, dword ptr [ebx + 0xc]
        sub eax, ecx
        push esi
        sar eax, 0x2
        cmp eax, ebp
        push edi
        mov dword ptr [esp + 0x10], ebx
        jnc L_4ba653
        mov edx, dword ptr [ebx + 0x4]
        test edx, edx
        jnz L_4ba53f
        xor eax, eax
        jmp L_4ba546
    L_4ba53f:
        mov eax, ecx
        sub eax, edx
        sar eax, 0x2
    L_4ba546:
        cmp ebp, eax
        jnc L_4ba55b
        test edx, edx
        jnz L_4ba552
        xor eax, eax
        jmp L_4ba55d
    L_4ba552:
        mov eax, ecx
        sub eax, edx
        sar eax, 0x2
        jmp L_4ba55d
    L_4ba55b:
        mov eax, ebp
    L_4ba55d:
        test edx, edx
        jnz L_4ba565
        xor ecx, ecx
        jmp L_4ba56a
    L_4ba565:
        sub ecx, edx
        sar ecx, 0x2
    L_4ba56a:
        add eax, ecx
        test eax, eax
        mov dword ptr [esp + 0x14], eax
        jge L_4ba576
        xor eax, eax
    L_4ba576:
        lea ecx, [eax*0x4 + 0x0]
        push ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov dword ptr [esp + 0x24], eax
        mov edx, eax
        mov eax, dword ptr [ebx + 0x4]
        mov ebx, dword ptr [esp + 0x20]
        add esp, 0x4
        cmp eax, ebx
        jz L_4ba5a9
    L_4ba597:
        test edx, edx
        jz L_4ba59f
        mov ecx, dword ptr [eax]
        mov dword ptr [edx], ecx
    L_4ba59f:
        add eax, 0x4
        add edx, 0x4
        cmp eax, ebx
        jnz L_4ba597
    L_4ba5a9:
        test ebp, ebp
        mov eax, edx
        jbe L_4ba5c3
        mov esi, dword ptr [esp + 0x24]
        mov ecx, ebp
    L_4ba5b5:
        test eax, eax
        jz L_4ba5bd
        mov edi, dword ptr [esi]
        mov dword ptr [eax], edi
    L_4ba5bd:
        add eax, 0x4
        dec ecx
        jnz L_4ba5b5
    L_4ba5c3:
        mov eax, dword ptr [esp + 0x10]
        lea edi, [ebp*0x4 + 0x0]
        mov esi, dword ptr [eax + 0x8]
        lea ecx, [edx + edi*0x1]
        cmp ebx, esi
        jz L_4ba5f2
        mov eax, ecx
        sub eax, edx
        add eax, ebx
        sub eax, edi
    L_4ba5e0:
        test ecx, ecx
        jz L_4ba5e8
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
    L_4ba5e8:
        add eax, 0x4
        add ecx, 0x4
        cmp eax, esi
        jnz L_4ba5e0
    L_4ba5f2:
        mov eax, dword ptr [esp + 0x10]
        mov eax, dword ptr [eax + 0x4]
        push eax
        mov dword ptr [esp + 0x28], eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        mov esi, dword ptr [esp + 0x24]
        mov ecx, dword ptr [esp + 0x18]
        mov edx, dword ptr [esp + 0x14]
        add esp, 0x4
        lea eax, [esi + ecx*0x4]
        mov ecx, dword ptr [edx + 0x4]
        mov dword ptr [edx + 0xc], eax
        test ecx, ecx
        jnz L_4ba636
        xor eax, eax
        mov dword ptr [edx + 0x4], esi
        add ebp, eax
        lea ecx, [esi + ebp*0x4]
        mov dword ptr [edx + 0x8], ecx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_4ba636:
        mov eax, dword ptr [edx + 0x8]
        mov dword ptr [edx + 0x4], esi
        sub eax, ecx
        sar eax, 0x2
        add ebp, eax
        lea ecx, [esi + ebp*0x4]
        mov dword ptr [edx + 0x8], ecx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_4ba653:
        mov esi, dword ptr [esp + 0x1c]
        mov edx, ecx
        sub edx, esi
        sar edx, 0x2
        cmp edx, ebp
        jnc L_4ba6cd
        lea edi, [ebp*0x4 + 0x0]
        cmp esi, ecx
        lea edx, [esi + edi*0x1]
        jz L_4ba68a
        mov eax, edx
        sub eax, edi
    L_4ba674:
        test edx, edx
        jz L_4ba680
        mov ebx, dword ptr [eax]
        mov dword ptr [edx], ebx
        mov ebx, dword ptr [esp + 0x10]
    L_4ba680:
        add eax, 0x4
        add edx, 0x4
        cmp eax, ecx
        jnz L_4ba674
    L_4ba68a:
        mov eax, dword ptr [ebx + 0x8]
        mov edx, dword ptr [esp + 0x24]
        mov ecx, eax
        sub ecx, esi
        sar ecx, 0x2
        sub ebp, ecx
        mov ecx, ebp
        jz L_4ba6ac
    L_4ba69e:
        test eax, eax
        jz L_4ba6a6
        mov ebp, dword ptr [edx]
        mov dword ptr [eax], ebp
    L_4ba6a6:
        add eax, 0x4
        dec ecx
        jnz L_4ba69e
    L_4ba6ac:
        mov ecx, dword ptr [ebx + 0x8]
        mov eax, esi
        cmp esi, ecx
        jz L_4ba726
    L_4ba6b5:
        mov esi, dword ptr [edx]
        mov dword ptr [eax], esi
        add eax, 0x4
        cmp eax, ecx
        jnz L_4ba6b5
        add dword ptr [ebx + 0x8], edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_4ba6cd:
        test ebp, ebp
        jbe L_4ba729
        lea edi, [ebp*0x4 + 0x0]
        mov eax, ecx
        sub eax, edi
        mov edx, ecx
        cmp eax, ecx
        jz L_4ba6f4
    L_4ba6e2:
        test edx, edx
        jz L_4ba6ea
        mov ebp, dword ptr [eax]
        mov dword ptr [edx], ebp
    L_4ba6ea:
        add eax, 0x4
        add edx, 0x4
        cmp eax, ecx
        jnz L_4ba6e2
    L_4ba6f4:
        mov ecx, dword ptr [ebx + 0x8]
        mov eax, ecx
        sub eax, edi
        cmp esi, eax
        jz L_4ba70e
    L_4ba6ff:
        mov edx, dword ptr [eax - 0x4]
        sub eax, 0x4
        sub ecx, 0x4
        cmp eax, esi
        mov dword ptr [ecx], edx
        jnz L_4ba6ff
    L_4ba70e:
        lea ecx, [esi + edi*0x1]
        mov eax, esi
        cmp esi, ecx
        jz L_4ba726
        mov edx, dword ptr [esp + 0x24]
    L_4ba71b:
        mov esi, dword ptr [edx]
        mov dword ptr [eax], esi
        add eax, 0x4
        cmp eax, ecx
        jnz L_4ba71b
    L_4ba726:
        add dword ptr [ebx + 0x8], edi
    L_4ba729:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    }
}

// 0x004bb0c0 Color16_Lerp - bytes ret 4 (t): t<0.001 -> ECX colour; t>0.999 -> EDX colour; else per-channel lerp via 3 ftol
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Color16_Lerp(int, int, int)
{
    __asm {
        sub esp, 0xc
        fld dword ptr [esp + 0x10]
        fcomp qword ptr [g_RData_004cc000 + 0x7b20]
        push ebx
        push esi
        mov ebx, edx
        mov dword ptr [esp + 0x8], ecx
        fnstsw AX
        test AH, 0x1
        jnz L_4bb1af
        fld dword ptr [esp + 0x18]
        fcomp qword ptr [g_RData_004cc000 + 0x7b28]
        fnstsw AX
        test AH, 0x41
        jnz L_4bb0fb
        mov eax, ebx
        pop esi
        pop ebx
        add esp, 0xc
        ret 0x4
    L_4bb0fb:
        fld dword ptr [esp + 0x18]
        fsubr qword ptr [g_RData_004cc000 + 0x7b30]
        and ecx, 0xff
        mov eax, ebx
        mov dword ptr [esp + 0xc], ecx
        and eax, 0xff
        fild dword ptr [esp + 0xc]
        mov dword ptr [esp + 0xc], eax
        fmul st(0), st(1)
        fild dword ptr [esp + 0xc]
        fmul dword ptr [esp + 0x18]
        faddp st(1), st(0)
        call dword ptr [g_Iat__ftol_004cc5ac]
        xor ecx, ecx
        xor edx, edx
        mov CL, byte ptr [esp + 0x9]
        mov DL, BH
        and ecx, 0xff
        and edx, 0xff
        mov dword ptr [esp + 0xc], ecx
        mov esi, eax
        fild dword ptr [esp + 0xc]
        mov dword ptr [esp + 0xc], edx
        fmul st(0), st(1)
        fild dword ptr [esp + 0xc]
        fmul dword ptr [esp + 0x18]
        faddp st(1), st(0)
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esp + 0x8]
        shr eax, 0x10
        and eax, 0xff
        mov dword ptr [esp + 0xc], eax
        fild dword ptr [esp + 0xc]
        shr ebx, 0x10
        and ebx, 0xff
        mov dword ptr [esp + 0xc], ebx
        fmul st(0), st(1)
        fild dword ptr [esp + 0xc]
        fmul dword ptr [esp + 0x18]
        faddp st(1), st(0)
        call dword ptr [g_Iat__ftol_004cc5ac]
        xor ecx, ecx
        and esi, 0xff
        mov CH, AL
        mov CL, byte ptr [esp + 0x10]
        shl ecx, 0x8
        fstp st(0)
        or ecx, esi
    L_4bb1af:
        pop esi
        mov eax, ecx
        pop ebx
        add esp, 0xc
        ret 0x4
    }
}

// 0x004bbfa0 WidgetArray_Destroy - bytes: for each 0x2c0-byte (0xb0 dwords) element in [+4,+8): vtbl+0(0) dtor; operator delete; +4..=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall WidgetArray_Destroy(int, int)
{
    __asm {
        push ebx
        mov ebx, ecx
        push esi
        push edi
        mov edi, dword ptr [ebx + 0x8]
        mov esi, dword ptr [ebx + 0x4]
        cmp esi, edi
        jz L_4bbfc1
    L_4bbfaf:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        add esi, 0x2c0
        cmp esi, edi
        jnz L_4bbfaf
    L_4bbfc1:
        mov ecx, dword ptr [ebx + 0x4]
        push ecx
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        mov dword ptr [ebx + 0x4], 0x0
        mov dword ptr [ebx + 0x8], 0x0
        mov dword ptr [ebx + 0xc], 0x0
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x004bcf80 Quad_SetVertex - If 0 <= index < 0x15: vertex at this+0x34+index*0xc gets x,y; vertex count +0x130 = max(count, index+1); index 0 also moves widget (vtable+0xc with ftol(x), ftol(y)). Always vtable+0x20. ret 0xc.
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall Quad_SetVertex(int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        cmp eax, 0x15
        push edi
        mov esi, ecx
        jge L_4bcfd8
        cmp eax, -0x1
        jle L_4bcfd8
        mov edx, dword ptr [esp + 0x10]
        lea ecx, [eax + eax*0x2]
        lea ecx, [esi + ecx*0x4]
        mov dword ptr [ecx + 0x34], edx
        mov edx, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x38], edx
        mov edx, dword ptr [esi + 0x130]
        lea ecx, [eax + 0x1]
        cmp edx, ecx
        jge L_4bcfb9
        mov dword ptr [esi + 0x130], ecx
    L_4bcfb9:
        test eax, eax
        jnz L_4bcfd8
        fld dword ptr [esp + 0x14]
        mov edi, dword ptr [esi]
        call dword ptr [g_Iat__ftol_004cc5ac]
        fld dword ptr [esp + 0x10]
        push eax
        call dword ptr [g_Iat__ftol_004cc5ac]
        push eax
        mov ecx, esi
        call dword ptr [edi + 0xc]
    L_4bcfd8:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        pop edi
        pop esi
        ret 0xc
    }
}

// 0x004bdc40 Widget_Slot_Call0048daf0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_Call0048daf0(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x34]
        mov edx, dword ptr [ecx + 0x48]
        push eax
        mov eax, dword ptr [ecx + 0x44]
        push edx
        mov edx, dword ptr [ecx + 0x40]
        push eax
        mov eax, dword ptr [ecx + 0x3c]
        push edx
        mov edx, dword ptr [ecx + 0x38]
        push eax
        push edx
        mov edx, dword ptr [ecx + 0x18]
        mov ecx, dword ptr [ecx + 0x14]
        call Raster_RippleDistort
        ret
    }
}

// 0x004bfae0 AnimImageWidget_AllocFrameImage - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AnimImageWidget_AllocFrameImage(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0xc0]
        test eax, eax
        jz L_4bfb6c
        mov edi, dword ptr [esi + 0x3c]
        test edi, edi
        jz L_4bfb6c
        mov ecx, dword ptr [esi + 0xbc]
        test ecx, ecx
        jz L_4bfb04
        call Image_Free
    L_4bfb04:
        call Image_Alloc
        test eax, eax
        mov dword ptr [esi + 0xbc], eax
        jz L_4bfb6c
        mov CX, word ptr [edi + 0x6]
        mov DX, word ptr [edi + 0x4]
        push ecx
        mov ecx, eax
        call Image_SetSize
        mov edx, dword ptr [esi + 0xbc]
        mov eax, dword ptr [edx]
        shl eax, 0x1
        push eax
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov ecx, dword ptr [esi + 0xbc]
        add esp, 0x4
        mov edx, eax
        push 0x0
        call Image_SetPixelsAndAlpha
        mov eax, dword ptr [esi + 0xbc]
        mov CL, byte ptr [eax + 0x9]
        or CL, 0x20
        mov byte ptr [eax + 0x9], CL
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x68]
        push eax
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        push eax
        mov ecx, esi
        call AnimImageWidget_CaptureFrame
    L_4bfb6c:
        pop edi
        pop esi
        ret
    }
}

// 0x004bfb70 ImageWidget_SetPositionAndSync - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ImageWidget_SetPositionAndSync(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push esi
        mov esi, ecx
        push eax
        mov ecx, dword ptr [esp + 0xc]
        push ecx
        mov ecx, esi
        call ImageWidget_SetPosition
        mov edx, dword ptr [esi + 0x18]
        mov eax, dword ptr [esi + 0x14]
        push edx
        push eax
        mov ecx, esi
        call AnimImageWidget_CaptureFrame
        pop esi
        ret 0x8
    }
}

// 0x004c0280 NamedTable_AddIfAbsent - (name,p2..p5) ret 0x14: strcmp-scan circular list [ECX+4] (key at +8); if absent new(0x1c) node linked before head, payload {name,p2,p3,p4,p5}, count [ECX+8]++
// Register/stack shape from the listing (ECX, EDX, 20 stack bytes).
__declspec(naked) int __fastcall NamedTable_AddIfAbsent(int, int, int, int, int, int, int)
{
    __asm {
        sub esp, 0x18
        push ebx
        mov ebx, ecx
        push ebp
        push esi
        mov ebp, dword ptr [ebx + 0x4]
        push edi
        xor eax, eax
        mov edx, dword ptr [esp + 0x2c]
        mov edi, dword ptr [ebp]
        mov dword ptr [esp + 0x10], ebx
        cmp edi, ebp
        setz AL
        neg AL
        sbb eax, eax
        inc eax
        test AL, AL
        jz L_4c02ef
    L_4c02a7:
        mov esi, dword ptr [edi + 0x8]
        mov eax, edx
    L_4c02ac:
        mov BL, byte ptr [eax]
        mov CL, BL
        cmp BL, byte ptr [esi]
        jnz L_4c02d0
        test CL, CL
        jz L_4c02cc
        mov BL, byte ptr [eax + 0x1]
        mov CL, BL
        cmp BL, byte ptr [esi + 0x1]
        jnz L_4c02d0
        add eax, 0x2
        add esi, 0x2
        test CL, CL
        jnz L_4c02ac
    L_4c02cc:
        xor eax, eax
        jmp L_4c02d5
    L_4c02d0:
        sbb eax, eax
        sbb eax, -0x1
    L_4c02d5:
        test eax, eax
        jz L_4c02eb
        mov edi, dword ptr [edi]
        xor ecx, ecx
        cmp edi, ebp
        setz CL
        neg CL
        sbb ecx, ecx
        inc ecx
        test CL, CL
        jnz L_4c02a7
    L_4c02eb:
        mov ebx, dword ptr [esp + 0x10]
    L_4c02ef:
        xor eax, eax
        cmp edi, ebp
        setz AL
        test AL, AL
        jz L_4c035d
        mov ecx, dword ptr [esp + 0x30]
        mov eax, dword ptr [esp + 0x38]
        mov esi, dword ptr [ebp + 0x4]
        mov dword ptr [esp + 0x14], edx
        mov edx, dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x18], ecx
        mov ecx, dword ptr [esp + 0x3c]
        push 0x1c
        mov dword ptr [esp + 0x20], edx
        mov dword ptr [esp + 0x24], eax
        mov dword ptr [esp + 0x28], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov ecx, ebp
        test ebp, ebp
        jnz L_4c0333
        mov ecx, eax
    L_4c0333:
        mov dword ptr [eax], ecx
        mov ecx, esi
        test esi, esi
        jnz L_4c033d
        mov ecx, eax
    L_4c033d:
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [ebp + 0x4], eax
        mov edx, dword ptr [eax + 0x4]
        lea edi, [eax + 0x8]
        test edi, edi
        mov dword ptr [edx], eax
        jz L_4c035a
        mov ecx, 0x5
        lea esi, [esp + 0x14]
        rep movsd
    L_4c035a:
        inc dword ptr [ebx + 0x8]
    L_4c035d:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret 0x14
    }
}

// 0x00407100 UiScreen_Slot_004434b0_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_004434b0_0(int, int)
{
    __asm {
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        ret
    }
}

// 0x0040ba90 UiButton_Slot_0040b980 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b980(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov ecx, dword ptr [ecx + 0xc8]
        push eax
        call CommandsDialog_ScrollAllColumns
        ret 0x4
    }
}

// 0x0041a2a0 EditField_OnChar_DigitsOnly - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_OnChar_DigitsOnly(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        push edi
        push offset g_RData_004cc000 + 0x3b90
        call dword ptr [g_Iat_strchr_004cc5d0]
        add esp, 0x8
        test eax, eax
        jz L_41a2c7
        push edi
        lea ecx, [esi + 0x14c]
        call KeyDispatch_OnChar
    L_41a2c7:
        pop edi
        xor eax, eax
        pop esi
        ret 0x4
    }
}

// 0x0041be70 UiScreen_Slot_00443160_4f3e78 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_00443160_4f3e78(int, int)
{
    __asm {
        push 0x0
        push offset g_Data_004da000 + 0x19e78
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePush
        ret
    }
}

// 0x004348f0 EditField_OnChar_NameChars - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_OnChar_NameChars(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        push edi
        push offset g_RData_004cc000 + 0x5598
        call dword ptr [g_Iat_strchr_004cc5d0]
        add esp, 0x8
        test eax, eax
        jz L_434917
        push edi
        lea ecx, [esi + 0x14c]
        call KeyDispatch_OnChar
    L_434917:
        pop edi
        xor eax, eax
        pop esi
        ret 0x4
    }
}

// 0x004b42f0 TextBuffer_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall TextBuffer_Ctor(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        xor ebx, ebx
        push eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x74d8
        mov dword ptr [esi + 0xc], ebx
        mov dword ptr [esi + 0x4], ebx
        mov dword ptr [esi + 0x8], ebx
        call EditField_ResizeBuffer
        mov ebp, dword ptr [g_Iat_isprint_004cc52c]
        xor edi, edi
    L_4b4319:
        push edi
        call ebp
        add esp, 0x4
        test eax, eax
        jz L_4b4329
        mov byte ptr [edi + esi*0x1 + 0x10], BL
        jmp L_4b432e
    L_4b4329:
        mov byte ptr [edi + esi*0x1 + 0x10], 0x1
    L_4b432e:
        inc edi
        cmp edi, 0x100
        jl L_4b4319
        mov byte ptr [esi + 0x30], BL
        mov byte ptr [esi + 0x3e], BL
        mov byte ptr [esi + 0x2b], 0x2
        mov byte ptr [esi + 0x1d], 0x3
        mov byte ptr [esi + 0x18], 0x4
        mov byte ptr [esi + 0x8f], 0x5
        mov byte ptr [esi + 0x12], 0x6
        mov byte ptr [esi + 0x16], 0x7
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    }
}

// 0x004b4b50 EditField_OnChar - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_OnChar(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov AL, byte ptr [esi + 0x371]
        test AL, AL
        jz L_4b4b88
        mov edi, dword ptr [esp + 0xc]
        push edi
        push offset g_RData_004cc000 + 0x74b0
        call dword ptr [g_Iat_strchr_004cc5d0]
        add esp, 0x8
        test eax, eax
        jz L_4b4b98
        push edi
        lea ecx, [esi + 0x14c]
        call KeyDispatch_OnChar
        xor eax, eax
        pop edi
        pop esi
        ret 0x4
    L_4b4b88:
        mov eax, dword ptr [esp + 0xc]
        lea ecx, [esi + 0x14c]
        push eax
        call KeyDispatch_OnChar
    L_4b4b98:
        pop edi
        xor eax, eax
        pop esi
        ret 0x4
    }
}

// 0x004b4e40 EditField_SetMaxLength - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_SetMaxLength(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        add ecx, 0x14c
        push eax
        call EditField_ResizeBuffer
        ret 0x4
    }
}

// 0x004bfa50 ImageWidget_SetImageAndNotify - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ImageWidget_SetImageAndNotify(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push eax
        call ImageWidget_SetImage
        test eax, eax
        jz L_4bfa68
        mov ecx, esi
        call AnimImageWidget_AllocFrameImage
    L_4bfa68:
        pop esi
        ret 0x4
    }
}

// 0x004bfa70 ImageWidget_SetImageAndNotify_B - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ImageWidget_SetImageAndNotify_B(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push eax
        call ImageWidget_SetImageNoOwn
        test eax, eax
        jz L_4bfa88
        mov ecx, esi
        call AnimImageWidget_AllocFrameImage
    L_4bfa88:
        pop esi
        ret 0x4
    }
}

// 0x004bfa90 AnimImageWidget_SetEnabled - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall AnimImageWidget_SetEnabled(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        test eax, eax
        mov dword ptr [esi + 0xc0], eax
        jnz L_4bfac9
        mov ecx, dword ptr [esi + 0xbc]
        test ecx, ecx
        jz L_4bfac9
        call Image_Free
        mov eax, dword ptr [esi]
        push 0x0
        push 0x0
        mov ecx, esi
        mov dword ptr [esi + 0xbc], 0x0
        call dword ptr [eax + 0x18]
        pop esi
        ret 0x4
    L_4bfac9:
        mov eax, dword ptr [esi + 0xbc]
        test eax, eax
        jnz L_4bfada
        mov ecx, esi
        call AnimImageWidget_AllocFrameImage
    L_4bfada:
        pop esi
        ret 0x4
    }
}

// 0x004bffe0 Timer_RegisterIfEnabled - ../../04_spec/structs/weapon_two_struct_split.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall Timer_RegisterIfEnabled(int, int, int, int, int)
{
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [g_Data_004da000 + 0x91f70]
        test ecx, ecx
        jz L_4c0004
        push esi
        mov esi, dword ptr [esp + 0x10]
        push esi
        mov esi, dword ptr [esp + 0x10]
        push esi
        mov esi, dword ptr [esp + 0x10]
        push esi
        push edx
        push eax
        call NamedTable_AddIfAbsent
        pop esi
    L_4c0004:
        ret 0xc
    }
}

// 0x004ba9e0 ListLabel_Assign - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabel_Assign(int, int, int)
{
    __asm {
        sub esp, 0x3c
        push esi
        push edi
        mov edi, dword ptr [esp + 0x48]
        mov esi, ecx
        push edi
        call TextWidget_Assign
        mov dword ptr [esi + 0x148], 0x0
        mov eax, dword ptr [edi + 0x14c]
        mov dword ptr [esi + 0x14c], eax
        mov ecx, dword ptr [edi + 0x150]
        lea edx, [esp + 0x8]
        mov dword ptr [esi + 0x150], ecx
        mov eax, dword ptr [edi + 0x154]
        push edx
        push 0x3c
        push eax
        call dword ptr [g_Iat_GetObjectA_004cc090]
        test eax, eax
        jz L_4baa3c
        lea ecx, [esp + 0x8]
        push ecx
        call dword ptr [g_Iat_CreateFontIndirectA_004cc094]
        mov dword ptr [esi + 0x154], eax
    L_4baa3c:
        mov edx, dword ptr [edi + 0x158]
        lea eax, [edi + 0x15c]
        push 0x100
        lea ecx, [esi + 0x15c]
        push eax
        push ecx
        mov dword ptr [esi + 0x158], edx
        call dword ptr [g_Iat_strncpy_004cc5a0]
        mov edx, dword ptr [edi + 0x25c]
        add esp, 0xc
        mov dword ptr [esi + 0x25c], edx
        mov eax, dword ptr [edi + 0x260]
        mov dword ptr [esi + 0x260], eax
        mov ecx, dword ptr [edi + 0x264]
        mov dword ptr [esi + 0x264], ecx
        mov edx, dword ptr [edi + 0x268]
        mov dword ptr [esi + 0x268], edx
        mov eax, dword ptr [edi + 0x26c]
        mov dword ptr [esi + 0x26c], eax
        mov dword ptr [esi + 0x270], 0x1
        mov ecx, dword ptr [edi + 0x274]
        lea eax, [edi + 0x27c]
        mov dword ptr [esi + 0x274], ecx
        mov edx, dword ptr [edi + 0x278]
        mov dword ptr [esi + 0x278], edx
        mov edx, dword ptr [eax]
        lea ecx, [esi + 0x27c]
        mov dword ptr [esi + 0x27c], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov edx, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], edx
        mov eax, dword ptr [eax + 0xc]
        lea edx, [esi + 0x28c]
        mov dword ptr [ecx + 0xc], eax
        lea ecx, [edi + 0x28c]
        mov eax, dword ptr [edi + 0x28c]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov eax, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], eax
        mov ecx, dword ptr [ecx + 0xc]
        mov dword ptr [edx + 0xc], ecx
        mov edx, dword ptr [edi + 0x144]
        mov dword ptr [esi + 0x144], edx
        mov eax, dword ptr [edi + 0x29c]
        mov dword ptr [esi + 0x29c], eax
        mov ecx, dword ptr [edi + 0x2a0]
        mov dword ptr [esi + 0x2a0], ecx
        mov eax, esi
        pop edi
        pop esi
        add esp, 0x3c
        ret 0x4
    }
}

// 0x004babb0 TextWidget_SetFont - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 28 stack bytes).
__declspec(naked) int __fastcall TextWidget_SetFont(int, int, int, int, int, int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x154]
        push eax
        call dword ptr [g_Iat_DeleteObject_004cc070]
        mov ecx, dword ptr [esp + 0x8]
        mov edx, dword ptr [esp + 0x20]
        mov eax, dword ptr [esp + 0x1c]
        push ecx
        mov ecx, dword ptr [esp + 0x1c]
        push edx
        mov edx, dword ptr [esp + 0x18]
        push 0x1
        push 0x0
        push 0x4
        push eax
        mov eax, dword ptr [esp + 0x2c]
        push 0x0
        push 0x0
        push ecx
        mov ecx, dword ptr [esp + 0x30]
        push edx
        push 0x0
        push 0x0
        push eax
        neg ecx
        push ecx
        call dword ptr [g_Iat_CreateFontA_004cc088]
        mov dword ptr [esi + 0x154], eax
        mov dword ptr [esi + 0x270], 0x1
        pop esi
        ret 0x1c
    }
}

// 0x004bac10 ListLabel_Slot_RenderTextImage - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_Slot_RenderTextImage(int, int)
{
    __asm {
        sub esp, 0x64
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        or ecx, 0xffffffff
        lea ebx, [esi + 0x34]
        xor eax, eax
        mov edi, ebx
        repne scasb
        not ecx
        dec ecx
        mov dword ptr [esp + 0x18], ecx
        jnz L_4bac60
        mov dword ptr [esi + 0x298], eax
        mov dword ptr [esi + 0x294], eax
        mov dword ptr [esi + 0x290], eax
        mov dword ptr [esi + 0x28c], eax
        mov dword ptr [esi + 0x260], eax
        mov dword ptr [esi + 0x25c], eax
        mov dword ptr [esi + 0x270], eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x64
        ret
    L_4bac60:
        push 0x0
        call dword ptr [g_Iat_CreateCompatibleDC_004cc0a4]
        mov edi, eax
        test edi, edi
        mov dword ptr [esp + 0x18], edi
        jz L_4bb09c
        mov eax, dword ptr [esi + 0x154]
        push eax
        push edi
        call dword ptr [g_Iat_SelectObject_004cc06c]
        mov eax, dword ptr [esi + 0x278]
        test eax, eax
        jz L_4bacc1
        lea ebp, [esi + 0x28c]
        lea ecx, [esi + 0x27c]
        mov edx, ebp
        mov dword ptr [esp + 0x14], 0x10
        mov eax, dword ptr [ecx]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov eax, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], eax
        mov eax, 0x1
        mov ecx, dword ptr [ecx + 0xc]
        mov dword ptr [edx + 0xc], ecx
        jmp L_4bacdf
    L_4bacc1:
        lea ebp, [esi + 0x28c]
        push 0x400
        push ebp
        push -0x1
        push ebx
        push edi
        mov dword ptr [esp + 0x28], 0x0
        call dword ptr [g_Iat_DrawTextA_004cc67c]
    L_4bacdf:
        test eax, eax
        jz L_4bb0ac
        mov eax, dword ptr [esi + 0x264]
        test eax, eax
        jz L_4bad23
        mov eax, dword ptr [esi + 0x2a0]
        mov ecx, dword ptr [esi + 0x294]
        cdq
        xor eax, edx
        sub eax, edx
        mov edx, dword ptr [esi + 0x298]
        add edx, eax
        mov eax, dword ptr [esi + 0x29c]
        mov dword ptr [esi + 0x298], edx
        cdq
        xor eax, edx
        sub eax, edx
        add ecx, eax
        mov dword ptr [esi + 0x294], ecx
    L_4bad23:
        mov eax, dword ptr [esi + 0x294]
        mov edx, dword ptr [ebp]
        mov ecx, dword ptr [esi + 0x290]
        sub eax, edx
        mov edx, dword ptr [esi + 0x298]
        mov dword ptr [esi + 0x25c], eax
        sub edx, ecx
        mov ecx, dword ptr [esi + 0x148]
        test ecx, ecx
        mov dword ptr [esi + 0x260], edx
        jz L_4bad89
        movsx ebx, word ptr [ecx + 0x4]
        cmp eax, ebx
        jg L_4bad66
        movsx eax, word ptr [ecx + 0x6]
        cmp edx, eax
        jle L_4badf3
    L_4bad66:
        call Image_Free
        call Image_Alloc
        mov DL, 0x3
        mov ecx, eax
        mov dword ptr [esi + 0x148], eax
        call Image_SetFlags
        mov CX, word ptr [esi + 0x260]
        push ecx
        jmp L_4bada5
    L_4bad89:
        call Image_Alloc
        mov DL, 0x3
        mov ecx, eax
        mov dword ptr [esi + 0x148], eax
        call Image_SetFlags
        mov DX, word ptr [esi + 0x260]
        push edx
    L_4bada5:
        mov DX, word ptr [esi + 0x25c]
        mov ecx, dword ptr [esi + 0x148]
        call Image_SetSize
        mov ecx, dword ptr [esi + 0x148]
        call Image_BytesPerPixel
        imul eax, dword ptr [esi + 0x25c]
        imul eax, dword ptr [esi + 0x260]
        push eax
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov ecx, dword ptr [esi + 0x148]
        add esp, 0x4
        mov edx, eax
        push 0x0
        call Image_SetPixelsAndAlpha
        mov eax, dword ptr [esi + 0x148]
        or byte ptr [eax + 0x9], 0x20
    L_4badf3:
        mov eax, dword ptr [esi + 0x148]
        test eax, eax
        jz L_4bb095
        mov edi, eax
        mov ecx, edi
        call Image_BytesPerPixel
        mov ecx, eax
        xor eax, eax
        imul ecx, dword ptr [edi]
        mov edi, dword ptr [edi + 0x10]
        mov edx, ecx
        shr ecx, 0x2
        rep stosd
        mov ecx, edx
        lea edx, [esp + 0x10]
        and ecx, 0x3
        rep stosb
        mov ecx, dword ptr [esi + 0x148]
        call dword ptr [g_Data_004da000 + 0x91c38]
        test eax, eax
        jz L_4baf8a
        mov eax, ebp
        mov ecx, dword ptr [eax]
        mov dword ptr [esp + 0x2c], ecx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x30], edx
        mov ecx, dword ptr [eax + 0x8]
        mov dword ptr [esp + 0x34], ecx
        mov ecx, dword ptr [esp + 0x10]
        mov edx, dword ptr [eax + 0xc]
        mov eax, dword ptr [esi + 0x154]
        push eax
        push ecx
        mov dword ptr [esp + 0x40], edx
        call dword ptr [g_Iat_SelectObject_004cc06c]
        mov eax, dword ptr [esi + 0x264]
        mov ebx, dword ptr [g_Iat_SetTextColor_004cc078]
        mov edi, dword ptr [g_Iat_SetBkMode_004cc07c]
        test eax, eax
        jz L_4baf12
        mov eax, dword ptr [ebp + 0x4]
        mov edx, dword ptr [ebp]
        mov ecx, dword ptr [ebp + 0x8]
        mov dword ptr [esp + 0x20], eax
        mov eax, dword ptr [esi + 0x29c]
        mov dword ptr [esp + 0x1c], edx
        mov edx, dword ptr [ebp + 0xc]
        mov dword ptr [esp + 0x24], ecx
        test eax, eax
        mov dword ptr [esp + 0x28], edx
        jle L_4baeaf
        add dword ptr [esp + 0x1c], eax
        jmp L_4baeb3
    L_4baeaf:
        sub dword ptr [esp + 0x2c], eax
    L_4baeb3:
        mov eax, dword ptr [esi + 0x2a0]
        test eax, eax
        jle L_4baec3
        add dword ptr [esp + 0x20], eax
        jmp L_4baec7
    L_4baec3:
        sub dword ptr [esp + 0x30], eax
    L_4baec7:
        mov eax, dword ptr [esp + 0x10]
        push 0x141414
        push eax
        call ebx
        cmp dword ptr [esi + 0x268], 0x2
        jnz L_4baee9
        mov ecx, dword ptr [esp + 0x10]
        push 0x20
        push ecx
        call dword ptr [g_Iat_SetBkColor_004cc080]
    L_4baee9:
        mov edx, dword ptr [esi + 0x268]
        mov eax, dword ptr [esp + 0x10]
        push edx
        push eax
        call edi
        mov ecx, dword ptr [esp + 0x14]
        lea edx, [esp + 0x1c]
        push ecx
        push edx
        lea eax, [esi + 0x34]
        push -0x1
        push eax
        mov eax, dword ptr [esp + 0x20]
        push eax
        call dword ptr [g_Iat_DrawTextA_004cc67c]
    L_4baf12:
        mov eax, dword ptr [esi + 0x14c]
        mov ecx, dword ptr [esi + 0x150]
        cmp eax, ecx
        jz L_4baf2e
        mov ecx, dword ptr [esp + 0x10]
        push 0xffffff
        push ecx
        jmp L_4baf34
    L_4baf2e:
        mov edx, dword ptr [esp + 0x10]
        push eax
        push edx
    L_4baf34:
        call ebx
        cmp dword ptr [esi + 0x268], 0x2
        jnz L_4baf51
        mov eax, dword ptr [esi + 0x26c]
        mov ecx, dword ptr [esp + 0x10]
        push eax
        push ecx
        call dword ptr [g_Iat_SetBkColor_004cc080]
    L_4baf51:
        mov edx, dword ptr [esi + 0x268]
        mov eax, dword ptr [esp + 0x10]
        push edx
        push eax
        call edi
        mov ecx, dword ptr [esp + 0x14]
        lea edx, [esp + 0x2c]
        push ecx
        push edx
        lea eax, [esi + 0x34]
        push -0x1
        push eax
        mov eax, dword ptr [esp + 0x20]
        push eax
        call dword ptr [g_Iat_DrawTextA_004cc67c]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esi + 0x148]
        call dword ptr [g_Data_004da000 + 0x91c3c]
    L_4baf8a:
        mov edx, dword ptr [esp + 0x18]
        lea ecx, [esp + 0x3c]
        push ecx
        push edx
        call dword ptr [g_Iat_GetTextMetricsA_004cc084]
        test eax, eax
        jz L_4bb091
        mov eax, dword ptr [esi + 0x14c]
        mov ecx, dword ptr [esi + 0x150]
        cmp eax, ecx
        jz L_4bb087
        push 0xff
        or DL, 0xff
        or CL, 0xff
        call Pixel_FromRGBBytes
        mov ebx, eax
        mov eax, dword ptr [esi + 0x148]
        xor ebp, ebp
        cmp word ptr [eax + 0x6], BP
        mov edi, dword ptr [eax + 0x10]
        jle L_4bb087
    L_4bafdd:
        mov eax, dword ptr [esi + 0x264]
        test eax, eax
        jz L_4baffb
        mov eax, dword ptr [esi + 0x2a0]
        mov ecx, dword ptr [esp + 0x3c]
        mov edx, dword ptr [esp + 0x4c]
        add eax, ebp
        add ecx, edx
        jmp L_4bb007
    L_4baffb:
        mov eax, dword ptr [esp + 0x3c]
        mov ecx, dword ptr [esp + 0x4c]
        add ecx, eax
        mov eax, ebp
    L_4bb007:
        cdq
        idiv ecx
        mov eax, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [esi + 0x14c]
        sub edx, eax
        mov dword ptr [esp + 0x18], edx
        mov edx, dword ptr [esp + 0x44]
        fild dword ptr [esp + 0x18]
        sub edx, eax
        mov dword ptr [esp + 0x18], edx
        mov edx, dword ptr [esi + 0x150]
        fidiv dword ptr [esp + 0x18]
        fstp dword ptr [esp]
        call Color16_Lerp
        mov ecx, eax
        xor edx, edx
        shr ecx, 0x10
        push ecx
        mov DL, AH
        mov CL, AL
        call Pixel_FromRGBBytes
        mov edx, dword ptr [esi + 0x148]
        xor ecx, ecx
        cmp word ptr [edx + 0x4], CX
        jle L_4bb074
    L_4bb05a:
        cmp word ptr [edi], BX
        jnz L_4bb062
        mov word ptr [edi], AX
    L_4bb062:
        mov edx, dword ptr [esi + 0x148]
        add edi, 0x2
        inc ecx
        movsx edx, word ptr [edx + 0x4]
        cmp ecx, edx
        jl L_4bb05a
    L_4bb074:
        mov eax, dword ptr [esi + 0x148]
        inc ebp
        movsx ecx, word ptr [eax + 0x6]
        cmp ebp, ecx
        jl L_4bafdd
    L_4bb087:
        mov edx, dword ptr [esp + 0x4c]
        mov dword ptr [esi + 0x274], edx
    L_4bb091:
        mov edi, dword ptr [esp + 0x18]
    L_4bb095:
        push edi
        call dword ptr [g_Iat_DeleteDC_004cc08c]
    L_4bb09c:
        xor eax, eax
        pop edi
        mov dword ptr [esi + 0x270], eax
        pop esi
        pop ebp
        pop ebx
        add esp, 0x64
        ret
    L_4bb0ac:
        call dword ptr [g_Iat_GetLastError_004cc0dc]
        jmp L_4bb095
    }
}

// 0x004bb1c0 TextLabel_MeasurePrefix - bytes ret 8 (n,rect): CreateCompatibleDC; SelectObject(font +0x154); copy _strdup(text +0x34), truncate at n; DrawTextA(..,rect,DT_CALCRECT 0x400); free; DeleteDC
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall TextLabel_MeasurePrefix(int, int, int, int)
{
    __asm {
        push ecx
        push ebx
        push esi
        mov esi, ecx
        push 0x0
        mov dword ptr [esp + 0xc], 0x0
        call dword ptr [g_Iat_CreateCompatibleDC_004cc0a4]
        mov ebx, eax
        test ebx, ebx
        jz L_4bb288
        mov eax, dword ptr [esi + 0x154]
        push edi
        push ebp
        push eax
        push ebx
        call dword ptr [g_Iat_SelectObject_004cc06c]
        mov ebp, dword ptr [esp + 0x18]
        test ebp, ebp
        jle L_4bb256
        add esi, 0x34
        push esi
        call dword ptr [g_Iat__strdup_004cc5e4]
        mov esi, eax
        or ecx, 0xffffffff
        mov edi, esi
        xor eax, eax
        add esp, 0x4
        repne scasb
        not ecx
        dec ecx
        cmp ebp, ecx
        jg L_4bb239
        mov ecx, dword ptr [esp + 0x1c]
        push 0x400
        push ecx
        push -0x1
        push esi
        push ebx
        mov byte ptr [esi + ebp*0x1], AL
        call dword ptr [g_Iat_DrawTextA_004cc67c]
        test eax, eax
        jz L_4bb239
        mov dword ptr [esp + 0x10], 0x1
    L_4bb239:
        push esi
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        push ebx
        call dword ptr [g_Iat_DeleteDC_004cc08c]
        mov eax, dword ptr [esp + 0x10]
        pop ebp
        pop edi
        pop esi
        pop ebx
        pop ecx
        ret 0x8
    L_4bb256:
        mov esi, dword ptr [esp + 0x1c]
        push 0x400
        push esi
        push -0x1
        push offset g_Data_004da000 + 0x6be4
        push ebx
        call dword ptr [g_Iat_DrawTextA_004cc67c]
        test eax, eax
        jz L_4bb27f
        mov edx, dword ptr [esi]
        mov dword ptr [esp + 0x10], 0x1
        mov dword ptr [esi + 0x8], edx
    L_4bb27f:
        push ebx
        call dword ptr [g_Iat_DeleteDC_004cc08c]
        pop ebp
        pop edi
    L_4bb288:
        mov eax, dword ptr [esp + 0x8]
        pop esi
        pop ebx
        pop ecx
        ret 0x8
    }
}

// 0x004bb2a0 ListLabel_Slot_ComputeLayout - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_Slot_ComputeLayout(int, int)
{
    __asm {
        sub esp, 0x8
        xor eax, eax
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        or ecx, 0xffffffff
        lea ebx, [esi + 0x34]
        mov edi, ebx
        repne scasb
        mov eax, dword ptr [esi + 0x278]
        not ecx
        dec ecx
        test eax, eax
        mov edi, ecx
        jz L_4bb2ee
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esi + 0x284]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [esi + 0x20], eax
        add edx, eax
        mov eax, dword ptr [esi + 0x288]
        add eax, ecx
        mov dword ptr [esi + 0x24], ecx
        mov dword ptr [esi + 0x28], edx
        mov dword ptr [esi + 0x2c], eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret
    L_4bb2ee:
        push 0x0
        call dword ptr [g_Iat_CreateCompatibleDC_004cc0a4]
        mov ebp, eax
        test ebp, ebp
        jz L_4bb3bc
        mov ecx, dword ptr [esi + 0x154]
        push ecx
        push ebp
        call dword ptr [g_Iat_SelectObject_004cc06c]
        lea edx, [esp + 0x10]
        push edx
        push edi
        push ebx
        push ebp
        call dword ptr [g_Iat_GetTextExtentPoint32A_004cc074]
        test eax, eax
        jz L_4bb3b5
        mov eax, dword ptr [esi + 0x144]
        test eax, eax
        jnz L_4bb333
        mov eax, dword ptr [esi + 0x20]
        jmp L_4bb372
    L_4bb333:
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4bb347
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x90]
    L_4bb347:
        mov eax, dword ptr [esi + 0x144]
        mov ecx, dword ptr [esi + 0x25c]
        cmp eax, 0x1
        jnz L_4bb36d
        mov edi, dword ptr [esi + 0x20]
        mov eax, dword ptr [esi + 0x28]
        sub eax, edi
        cdq
        sub eax, edx
        sar eax, 0x1
        sar ecx, 0x1
        sub eax, ecx
        add eax, edi
        jmp L_4bb372
    L_4bb36d:
        mov eax, dword ptr [esi + 0x28]
        sub eax, ecx
    L_4bb372:
        mov ecx, dword ptr [esp + 0x10]
        mov edx, dword ptr [esi + 0x24]
        mov dword ptr [esi + 0x20], eax
        add ecx, eax
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esi + 0x28], ecx
        lea edi, [edx + eax*0x1]
        mov eax, dword ptr [esi + 0x264]
        test eax, eax
        mov dword ptr [esi + 0x2c], edi
        jz L_4bb3b5
        mov eax, dword ptr [esi + 0x2a0]
        cdq
        xor eax, edx
        sub eax, edx
        add eax, edi
        mov dword ptr [esi + 0x2c], eax
        mov eax, dword ptr [esi + 0x29c]
        cdq
        xor eax, edx
        sub eax, edx
        add eax, ecx
        mov dword ptr [esi + 0x28], eax
    L_4bb3b5:
        push ebp
        call dword ptr [g_Iat_DeleteDC_004cc08c]
    L_4bb3bc:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret
    }
}

// 0x0040ccc0 OptionSlider_Refresh_From004a27f0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionSlider_Refresh_From004a27f0(int, int)
{
    __asm {
        sub esp, 0xc
        push esi
        mov esi, ecx
        lea edx, [esp + 0x6]
        lea ecx, [esp + 0x8]
        call SoundCD_GetVolume
        mov ecx, dword ptr [esp + 0x8]
        mov eax, dword ptr [esi]
        and ecx, 0xffff
        mov dword ptr [esp + 0xc], ecx
        push ecx
        fild dword ptr [esp + 0x10]
        mov ecx, esi
        fmul dword ptr [g_RData_004cc000 + 0x2224]
        fstp dword ptr [esp]
        call dword ptr [eax + 0x84]
        pop esi
        add esp, 0xc
        ret
    }
}

// 0x004b4ca0 EditField_Slot_TickCaret - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_Slot_TickCaret(int, int, int)
{
    __asm {
        sub esp, 0x10
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jnz L_4b4cec
        mov ecx, dword ptr [esi + 0x110]
        push 0x0
        mov ecx, dword ptr [ecx]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x20]
        mov eax, dword ptr [esi + 0x260]
        add esi, 0x260
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x60]
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        pop esi
        add esp, 0x10
        ret 0x4
    L_4b4cec:
        mov AL, byte ptr [esi + 0x370]
        push edi
        test AL, AL
        jz L_4b4e03
        mov eax, dword ptr [esi + 0x110]
        push 0x1
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        lea ecx, [esi + 0x14c]
        call TextBuffer_GetText
        mov edx, dword ptr [esi + 0x110]
        test edx, edx
        jnz L_4b4d23
        xor ecx, ecx
        jmp L_4b4d2e
    L_4b4d23:
        mov ecx, dword ptr [esi + 0x114]
        sub ecx, edx
        sar ecx, 0x2
    L_4b4d2e:
        test ecx, ecx
        jz L_4b4d43
        mov ecx, dword ptr [esi + 0x110]
        push eax
        mov ecx, dword ptr [ecx]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x8c]
    L_4b4d43:
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        mov dword ptr [esp + 0x8], eax
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x68]
        mov dword ptr [esp + 0xc], eax
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x68]
        mov ecx, dword ptr [esi + 0x158]
        mov edx, dword ptr [esi + 0x110]
        mov dword ptr [esp + 0x14], eax
        lea eax, [esp + 0x8]
        push eax
        push ecx
        mov ecx, dword ptr [edx]
        call TextLabel_MeasurePrefix
        test eax, eax
        jz L_4b4dfa
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        mov ecx, dword ptr [ecx + 0x14c]
        mov edx, ecx
        shr edx, 0x10
        push edx
        xor edx, edx
        mov DL, CH
        call Pixel_FromRGBBytes
        mov ecx, dword ptr [esp + 0x14]
        and eax, 0xffff
        mov dword ptr [esi + 0x340], eax
        mov eax, dword ptr [esp + 0xc]
        mov edx, dword ptr [esi + 0x368]
        sub ecx, eax
        push ecx
        push edx
        push eax
        mov eax, dword ptr [esp + 0x1c]
        lea edi, [esi + 0x260]
        push eax
        mov ecx, edi
        call EditField_SetCaretShape
        mov edx, dword ptr [edi]
        push 0x1
        mov ecx, edi
        call dword ptr [edx + 0x60]
    L_4b4dfa:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        jmp L_4b4e14
    L_4b4e03:
        mov edx, dword ptr [esi + 0x260]
        lea ecx, [esi + 0x260]
        push 0x0
        call dword ptr [edx + 0x60]
    L_4b4e14:
        mov edi, dword ptr [esp + 0x1c]
        mov ecx, esi
        push edi
        call Widget_Tick
        mov eax, dword ptr [esi + 0x260]
        lea ecx, [esi + 0x260]
        push edi
        call dword ptr [eax + 0x24]
        pop edi
        pop esi
        add esp, 0x10
        ret 0x4
    }
}

// 0x004bbbe0 ListWidget_SetFontAll - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 28 stack bytes).
__declspec(naked) int __fastcall ListWidget_SetFontAll(int, int, int, int, int, int, int, int, int)
{
    __asm {
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0x24]
        push esi
        push edi
        xor ebx, ebx
        mov esi, ecx
        xor edi, edi
    L_4bbbee:
        mov eax, dword ptr [esi + 0x2ac]
        test eax, eax
        jnz L_4bbbfc
        xor edx, edx
        jmp L_4bbc15
    L_4bbbfc:
        mov ecx, dword ptr [esi + 0x2b0]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bbc15:
        cmp ebx, edx
        jnc L_4bbc51
        mov eax, dword ptr [esp + 0x28]
        push ebp
        push eax
        mov eax, dword ptr [esp + 0x2c]
        mov ecx, dword ptr [esi + 0x2ac]
        push eax
        mov eax, dword ptr [esp + 0x2c]
        add ecx, edi
        push eax
        mov eax, dword ptr [esp + 0x2c]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esp + 0x2c]
        push eax
        mov eax, dword ptr [esp + 0x2c]
        push eax
        call dword ptr [edx + 0x80]
        inc ebx
        add edi, 0x2c0
        jmp L_4bbbee
    L_4bbc51:
        mov ecx, dword ptr [esp + 0x28]
        mov edx, dword ptr [esp + 0x24]
        mov eax, dword ptr [esp + 0x20]
        push ebp
        push ecx
        mov ecx, dword ptr [esp + 0x24]
        push edx
        mov edx, dword ptr [esp + 0x24]
        push eax
        mov eax, dword ptr [esp + 0x24]
        push ecx
        push edx
        push eax
        mov ecx, esi
        call TextWidget_SetFont
        mov edi, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edi + 0x68]
        mov edx, dword ptr [esi]
        push eax
        mov ecx, esi
        call dword ptr [edx + 0x64]
        push eax
        mov ecx, esi
        call dword ptr [edi + 0xc]
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x1c
    }
}

// 0x004bc320 ListItem_CopyRange - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListItem_CopyRange(int, int, int)
{
    __asm {
        push ebx
        push esi
        mov ebx, edx
        mov esi, ecx
        push edi
        mov edi, dword ptr [esp + 0x10]
        cmp esi, ebx
        jz L_4bc389
    L_4bc32f:
        push esi
        mov ecx, edi
        call ListLabel_Assign
        mov eax, dword ptr [esi + 0x2a4]
        add esi, 0x2c0
        mov dword ptr [edi + 0x2a4], eax
        mov ecx, dword ptr [esi - 0x18]
        mov dword ptr [edi + 0x2a8], ecx
        mov edx, dword ptr [esi - 0x14]
        mov dword ptr [edi + 0x2ac], edx
        mov eax, dword ptr [esi - 0x10]
        mov dword ptr [edi + 0x2b0], eax
        mov ecx, dword ptr [esi - 0xc]
        mov dword ptr [edi + 0x2b4], ecx
        mov edx, dword ptr [esi - 0x8]
        mov dword ptr [edi + 0x2b8], edx
        mov eax, dword ptr [esi - 0x4]
        mov dword ptr [edi + 0x2bc], eax
        add edi, 0x2c0
        cmp esi, ebx
        jnz L_4bc32f
    L_4bc389:
        mov eax, edi
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004bc3a0 ListLabelAnimated_Assign - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabelAnimated_Assign(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        push edi
        call ListLabel_Assign
        mov eax, dword ptr [edi + 0x2a4]
        mov dword ptr [esi + 0x2a4], eax
        mov ecx, dword ptr [edi + 0x2a8]
        mov dword ptr [esi + 0x2a8], ecx
        mov edx, dword ptr [edi + 0x2ac]
        mov dword ptr [esi + 0x2ac], edx
        mov eax, dword ptr [edi + 0x2b0]
        mov dword ptr [esi + 0x2b0], eax
        mov ecx, dword ptr [edi + 0x2b4]
        mov dword ptr [esi + 0x2b4], ecx
        mov edx, dword ptr [edi + 0x2b8]
        mov dword ptr [esi + 0x2b8], edx
        mov eax, dword ptr [edi + 0x2bc]
        mov dword ptr [esi + 0x2bc], eax
        mov eax, esi
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00403c90 Widget_Slot_DrawImageAt24 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_DrawImageAt24(int, int)
{
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [eax + 0x1c]
        test ecx, ecx
        jz L_403caa
        lea edx, [eax + 0x20]
        mov eax, dword ptr [eax + 0x24]
        push edx
        push 0x0
        mov edx, dword ptr [edx]
        push eax
        call Draw_ImageAt
    L_403caa:
        ret
    }
}

// 0x00404cb0 Widget_Slot_DrawImageAt18 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_DrawImageAt18(int, int)
{
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [eax + 0x1c]
        test ecx, ecx
        jz L_404ccb
        lea edx, [eax + 0x20]
        push edx
        mov edx, dword ptr [eax + 0x18]
        push 0x0
        push edx
        mov edx, dword ptr [eax + 0x14]
        call Draw_ImageAt
    L_404ccb:
        ret
    }
}

// 0x004b3fb0 ImageWidget_Slot_Draw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_Slot_Draw(int, int)
{
    __asm {
        push ebx
        push esi
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x3c]
        test eax, eax
        jz L_4b4023
        mov ecx, dword ptr [edi + 0x38]
        test ecx, ecx
        jz L_4b3ff9
        lea esi, [edi + 0x4c]
        mov ebx, 0x4
    L_4b3fcb:
        cmp dword ptr [esi], 0x0
        jz L_4b3fef
        mov ecx, dword ptr [esi + 0x8]
        mov edx, dword ptr [esi + 0x4]
        lea eax, [esi + 0xc]
        push eax
        push 0x0
        push ecx
        mov ecx, dword ptr [edi + 0x3c]
        call Draw_ImageAt
        mov eax, dword ptr [esi]
        dec eax
        mov dword ptr [esi], eax
        jnz L_4b3fef
        dec dword ptr [edi + 0x38]
    L_4b3fef:
        add esi, 0x1c
        dec ebx
        jnz L_4b3fcb
        pop edi
        pop esi
        pop ebx
        ret
    L_4b3ff9:
        mov ecx, dword ptr [g_Data_004da000 + 0x91d1c]
        test ecx, ecx
        jz L_4b4007
        cmp ecx, eax
        jnz L_4b4023
    L_4b4007:
        mov edx, dword ptr [edi]
        mov ecx, edi
        call dword ptr [edx + 0x8]
        mov eax, dword ptr [edi + 0x44]
        mov ecx, dword ptr [edi + 0x18]
        mov edx, dword ptr [edi + 0x14]
        push eax
        push 0x0
        push ecx
        mov ecx, dword ptr [edi + 0x3c]
        call Draw_ImageAt
    L_4b4023:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x004b8520 ScrollbarPair_Slot_Draw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScrollbarPair_Slot_Draw(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x16c]
        test eax, eax
        jz L_4b85b6
        mov eax, dword ptr [esi + 0x150]
        test eax, eax
        jz L_4b85b6
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x8]
        mov ecx, dword ptr [esi + 0x154]
        mov edx, dword ptr [esi + 0x15c]
        lea eax, [esi + 0x154]
        cmp ecx, edx
        jz L_4b857b
        mov edx, dword ptr [esi + 0x18]
        push eax
        mov eax, dword ptr [esi + 0x168]
        mov ecx, dword ptr [esi + 0x150]
        add edx, eax
        mov eax, dword ptr [esi + 0x164]
        push 0x0
        push edx
        mov edx, dword ptr [esi + 0x14]
        add edx, eax
        call Draw_ImageAt
    L_4b857b:
        mov ecx, dword ptr [esi + 0x170]
        mov edx, dword ptr [esi + 0x178]
        lea eax, [esi + 0x170]
        cmp ecx, edx
        jz L_4b85b6
        mov edx, dword ptr [esi + 0x18]
        push eax
        mov eax, dword ptr [esi + 0x184]
        mov ecx, dword ptr [esi + 0x16c]
        add edx, eax
        mov eax, dword ptr [esi + 0x180]
        push 0x0
        push edx
        mov edx, dword ptr [esi + 0x14]
        add edx, eax
        call Draw_ImageAt
    L_4b85b6:
        pop esi
        ret
    }
}

// 0x004ba380 ScreenBase_DrawBackground - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScreenBase_DrawBackground(int, int)
{
    __asm {
        mov ecx, dword ptr [ecx + 0x114]
        test ecx, ecx
        jz L_4ba397
        push 0x0
        push 0x0
        push 0x0
        xor edx, edx
        call Draw_ImageAt
    L_4ba397:
        ret
    }
}

// 0x004bb460 ListLabel_Slot_Draw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_Slot_Draw(int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4bb477
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x90]
    L_4bb477:
        mov eax, dword ptr [esi + 0x148]
        test eax, eax
        jz L_4bb532
        mov AL, byte ptr [esi + 0x34]
        test AL, AL
        jz L_4bb52b
        mov eax, dword ptr [esi + 0x144]
        test eax, eax
        jz L_4bb505
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4bb4ae
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x90]
    L_4bb4ae:
        mov edi, dword ptr [esi + 0x28]
        mov ecx, dword ptr [esi + 0x20]
        mov eax, dword ptr [esi + 0x144]
        mov ebx, dword ptr [esi + 0x25c]
        sub edi, ecx
        cmp eax, 0x1
        jnz L_4bb4cb
        sar edi, 0x1
        sar ebx, 0x1
    L_4bb4cb:
        mov eax, dword ptr [esi + 0x14]
        mov ecx, esi
        sub eax, edi
        mov dword ptr [esi + 0x14], eax
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x8]
        mov ecx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0x18]
        sub edi, ebx
        add ecx, edi
        mov dword ptr [esi + 0x14], ecx
        mov edx, ecx
        lea ecx, [esi + 0x28c]
        push ecx
        mov ecx, dword ptr [esi + 0x148]
        push 0x0
        push eax
        call Draw_ImageAt
        add dword ptr [esi + 0x14], ebx
        pop edi
        pop esi
        pop ebx
        ret
    L_4bb505:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x8]
        mov ecx, dword ptr [esi + 0x18]
        mov edx, dword ptr [esi + 0x14]
        lea eax, [esi + 0x28c]
        push eax
        push 0x0
        push ecx
        mov ecx, dword ptr [esi + 0x148]
        call Draw_ImageAt
        pop edi
        pop esi
        pop ebx
        ret
    L_4bb52b:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x8]
    L_4bb532:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x004bfc60 Widget_Slot_DrawImageAt18_B - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_DrawImageAt18_B(int, int)
{
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [eax + 0x1c]
        test ecx, ecx
        jz L_4bfc7b
        lea edx, [eax + 0x20]
        push edx
        mov edx, dword ptr [eax + 0x18]
        push 0x0
        push edx
        mov edx, dword ptr [eax + 0x14]
        call Draw_ImageAt
    L_4bfc7b:
        ret
    }
}

// 0x004bfe90 Widget_Slot_Virtual08_Then0048f500IfField0D - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_Virtual08_Then0048f500IfField0D(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x8]
        mov ecx, dword ptr [esi + 0x34]
        test ecx, ecx
        jz L_4bfeb2
        mov DX, word ptr [esi + 0x3c]
        mov eax, dword ptr [esi + 0x18]
        push 0x0
        push edx
        mov edx, dword ptr [esi + 0x14]
        push eax
        call Draw_ImageAt
    L_4bfeb2:
        pop esi
        ret
    }
}

// 0x004bfec0 Widget_Slot_DrawImageClampedY - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_DrawImageClampedY(int, int)
{
    __asm {
        mov eax, ecx
        xor edx, edx
        push esi
        mov ecx, dword ptr [eax + 0x14]
        test ecx, ecx
        setle DL
        dec edx
        and edx, ecx
        mov ecx, dword ptr [eax + 0x18]
        mov esi, edx
        xor edx, edx
        test ecx, ecx
        setle DL
        dec edx
        and edx, ecx
        mov ecx, dword ptr [eax + 0x1c]
        test ecx, ecx
        jz L_4bfef4
        add eax, 0x20
        push eax
        push 0x0
        push edx
        mov edx, esi
        call Draw_ImageAt
    L_4bfef4:
        pop esi
        ret
    }
}

// 0x004c7f00 BitmapFont_DrawString - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall BitmapFont_DrawString(int, int, int, int)
{
    __asm {
        sub esp, 0x8
        push ebx
        push ebp
        push esi
        mov esi, dword ptr [esp + 0x18]
        mov dword ptr [esp + 0xc], ecx
        push edi
        mov ecx, dword ptr [esp + 0x20]
        mov dword ptr [esp + 0x14], edx
        mov edi, edx
        mov ebp, esi
        call TextureTable_GetOrDefault
        mov ebx, eax
        test ebx, ebx
        jnz L_4c7f37
        xor ecx, ecx
        call TextureTable_GetOrDefault
        mov ebx, eax
        test ebx, ebx
        jz L_4c7fb7
    L_4c7f37:
        mov ecx, dword ptr [ebx]
        mov dword ptr [esp + 0x1c], ecx
        movsx eax, word ptr [ecx + 0x6]
        add esi, eax
        mov eax, dword ptr [g_Data_004da000 + 0x158058]
        cmp esi, eax
        jge L_4c7fb7
        mov edx, dword ptr [esp + 0x10]
        mov AL, byte ptr [edx]
        inc edx
        test AL, AL
        mov dword ptr [esp + 0x10], edx
        jz L_4c7fb7
    L_4c7f5b:
        cmp AL, 0x20
        jnz L_4c7f64
        add edi, dword ptr [ebx + 0x4]
        jmp L_4c7fa8
    L_4c7f64:
        cmp AL, 0xd
        jz L_4c7fa8
        cmp AL, 0xa
        jnz L_4c7f78
        movsx edx, word ptr [ecx + 0x6]
        mov edi, dword ptr [esp + 0x14]
        add ebp, edx
        jmp L_4c7fa8
    L_4c7f78:
        movsx eax, AL
        sub eax, 0x21
        js L_4c7f85
        cmp eax, 0x5f
        jl L_4c7f87
    L_4c7f85:
        xor eax, eax
    L_4c7f87:
        mov ecx, dword ptr [ebx]
        mov edx, edi
        shl eax, 0x4
        lea esi, [eax + ebx*0x1 + 0x8]
        push esi
        push 0x0
        push ebp
        call Draw_ImageAt
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [esp + 0x1c]
        sub eax, edx
        add edi, eax
    L_4c7fa8:
        mov edx, dword ptr [esp + 0x10]
        mov AL, byte ptr [edx]
        inc edx
        test AL, AL
        mov dword ptr [esp + 0x10], edx
        jnz L_4c7f5b
    L_4c7fb7:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x8
    }
}

// 0x004038a0 ImageWidget_Slot_DrawWithFade - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_Slot_DrawWithFade(int, int)
{
    __asm {
        sub esp, 0x10
        push esi
        mov esi, ecx
        call ImageWidget_Slot_Draw
        fld dword ptr [esi + 0xbc]
        fcomp qword ptr [g_RData_004cc000 + 0x860]
        fnstsw AX
        test AH, 0x41
        jnz L_403923
        mov eax, dword ptr [esi]
        push edi
        mov ecx, esi
        call dword ptr [eax + 0x64]
        mov edx, dword ptr [esi]
        mov ecx, esi
        mov dword ptr [esp + 0x8], eax
        call dword ptr [edx + 0x68]
        mov dword ptr [esp + 0xc], eax
        mov eax, dword ptr [esi + 0x3c]
        test eax, eax
        jz L_4038e2
        movsx edi, word ptr [eax + 0x4]
        jmp L_4038e4
    L_4038e2:
        xor edi, edi
    L_4038e4:
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        add eax, edi
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esi + 0x3c]
        test eax, eax
        jz L_4038fe
        movsx edi, word ptr [eax + 0x6]
        jmp L_403900
    L_4038fe:
        xor edi, edi
    L_403900:
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x68]
        fld dword ptr [esi + 0xbc]
        sub esp, 0x8
        add eax, edi
        lea ecx, [esp + 0x10]
        mov dword ptr [esp + 0x1c], eax
        fstp qword ptr [esp]
        call Screen_RandomDissolve
        pop edi
    L_403923:
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x0040f040 UiScreen_Slot_Virtual20_Virtual74_004bb460 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Virtual20_Virtual74_004bb460(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x20]
        fld dword ptr [esi + 0x2a8]
        mov ecx, dword ptr [esi]
        sub esp, 0x8
        fstp qword ptr [esp]
        push offset g_Data_004da000 + 0xd0c
        push esi
        call dword ptr [ecx + 0x74]
        add esp, 0x10
        mov ecx, esi
        call ListLabel_Slot_Draw
        pop esi
        ret
    }
}

// 0x00434950 UiScreen_Slot_Call004bb460_ThenVirtual78 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call004bb460_ThenVirtual78(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ListLabel_Slot_Draw
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x78]
        pop esi
        ret
    }
}

// 0x004ba410 ListWidget_Slot_ComputeBounds - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListWidget_Slot_ComputeBounds(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call ListLabel_Slot_Draw
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        mov dword ptr [esi + 0x20], eax
        mov eax, dword ptr [esi + 0x270]
        test eax, eax
        jz L_4ba437
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x90]
    L_4ba437:
        mov eax, dword ptr [esi]
        mov edi, dword ptr [esi + 0x25c]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        mov edx, dword ptr [esi]
        add eax, edi
        mov ecx, esi
        mov dword ptr [esi + 0x28], eax
        call dword ptr [edx + 0x68]
        mov ecx, esi
        mov dword ptr [esi + 0x24], eax
        call ListLabel_GetLineHeight
        mov edi, eax
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x68]
        add edi, eax
        mov dword ptr [esi + 0x2c], edi
        pop edi
        pop esi
        ret
    }
}

// 0x004bce30 TextWidget_Slot_Draw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextWidget_Slot_Draw(int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x8]
        mov AL, byte ptr [esi + 0x34]
        lea ebx, [esi + 0x34]
        test AL, AL
        jz L_4bce9c
        mov eax, dword ptr [esi + 0x144]
        test eax, eax
        jz L_4bce87
        mov ecx, esi
        call TextWidget_MeasureWidth
        mov edi, eax
        mov eax, dword ptr [esi + 0x144]
        cmp eax, 0x1
        jnz L_4bce64
        sar edi, 0x1
    L_4bce64:
        mov ecx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0x18]
        sub ecx, edi
        mov dword ptr [esi + 0x14], ecx
        mov edx, ecx
        mov ecx, dword ptr [esi + 0x134]
        push ecx
        push eax
        mov ecx, ebx
        call BitmapFont_DrawString
        add dword ptr [esi + 0x14], edi
        pop edi
        pop esi
        pop ebx
        ret
    L_4bce87:
        mov ecx, dword ptr [esi + 0x134]
        mov edx, dword ptr [esi + 0x18]
        push ecx
        push edx
        mov edx, dword ptr [esi + 0x14]
        mov ecx, ebx
        call BitmapFont_DrawString
    L_4bce9c:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x004bd4d0 DrawItem_Render - switch type [+4]: 1 text 0x0048f500 when +0x18; 2 fill 0x00498bd0(+0x18,+0x1c,+0x20); 3 image 0x004936d0(+0x110); 4 short pair 0x004c7f00 when byte +0x1a; 5 string 0x004c7f00 when nonempty; 6 0x00498f90(+0x18); 7 clipped rect 0x004bd840 then ftol x4 and 0x00498bd0; 8 0x00498c00(+0xc0,+0xbc)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DrawItem_Render(int, int)
{
    __asm {
        sub esp, 0x20
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x4]
        dec eax
        cmp eax, 0x7
        ja L_4bd639
        cmp eax, 0
        je L_4bd53c
        cmp eax, 1
        je L_4bd57b
        cmp eax, 2
        je L_4bd561
        cmp eax, 3
        je L_4bd4ea
        cmp eax, 4
        je L_4bd510
        cmp eax, 5
        je L_4bd608
        cmp eax, 6
        je L_4bd597
        cmp eax, 7
        je L_4bd61c
        int 3  // unreachable: the bounds check above excludes other indices
    L_4bd4ea:
        mov AL, byte ptr [esi + 0x1a]
        lea ecx, [esi + 0x1a]
        test AL, AL
        jz L_4bd639
        movsx eax, word ptr [esi + 0x18]
        movsx edx, word ptr [esi + 0x14]
        push eax
        push edx
        movsx edx, word ptr [esi + 0x10]
        call BitmapFont_DrawString
        pop esi
        add esp, 0x20
        ret
    L_4bd510:
        mov ecx, dword ptr [esi + 0x1c]
        test ecx, ecx
        jz L_4bd639
        cmp byte ptr [ecx], 0x0
        jz L_4bd639
        movsx eax, word ptr [esi + 0x18]
        movsx edx, word ptr [esi + 0x14]
        push eax
        push edx
        movsx edx, word ptr [esi + 0x10]
        call BitmapFont_DrawString
        pop esi
        add esp, 0x20
        ret
    L_4bd53c:
        mov ecx, dword ptr [esi + 0x18]
        test ecx, ecx
        jz L_4bd639
        mov eax, dword ptr [esi + 0x20]
        mov DX, word ptr [esi + 0x1c]
        push eax
        mov eax, dword ptr [esi + 0x14]
        push edx
        mov edx, dword ptr [esi + 0x10]
        push eax
        call Draw_ImageAt
        pop esi
        add esp, 0x20
        ret
    L_4bd561:
        mov ecx, dword ptr [esi + 0x110]
        mov edx, dword ptr [esi + 0x10c]
        push ecx
        lea ecx, [esi + 0x10]
        call Draw_FillConvexPolygon2D
        pop esi
        add esp, 0x20
        ret
    L_4bd57b:
        mov edx, dword ptr [esi + 0x20]
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esi + 0x18]
        push edx
        mov edx, dword ptr [esi + 0x14]
        push eax
        push ecx
        mov ecx, dword ptr [esi + 0x10]
        call Draw_LineViaHook
        pop esi
        add esp, 0x20
        ret
    L_4bd597:
        fild dword ptr [esi + 0x10]
        lea edx, [esp + 0x4]
        lea eax, [esp + 0x8]
        push edx
        push eax
        fstp dword ptr [esp + 0x20]
        fild dword ptr [esi + 0x14]
        lea edx, [esp + 0x14]
        lea ecx, [esp + 0x20]
        fstp dword ptr [esp + 0x24]
        fild dword ptr [esi + 0x18]
        fstp dword ptr [esp + 0x14]
        fild dword ptr [esi + 0x1c]
        fstp dword ptr [esp + 0x18]
        call ClipSegment_2D
        test eax, eax
        jz L_4bd639
        mov ecx, dword ptr [esi + 0x20]
        fld dword ptr [esp + 0x10]
        push ecx
        call dword ptr [g_Iat__ftol_004cc5ac]
        fld dword ptr [esp + 0x10]
        push eax
        call dword ptr [g_Iat__ftol_004cc5ac]
        fld dword ptr [esp + 0x24]
        push eax
        call dword ptr [g_Iat__ftol_004cc5ac]
        fld dword ptr [esp + 0x24]
        mov esi, eax
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov ecx, eax
        mov edx, esi
        call Draw_LineViaHook
        pop esi
        add esp, 0x20
        ret
    L_4bd608:
        mov edx, dword ptr [esi + 0x18]
        mov ecx, dword ptr [esi + 0x10]
        push edx
        mov edx, dword ptr [esi + 0x14]
        call SW_PlotPixel_Indirect
        pop esi
        add esp, 0x20
        ret
    L_4bd61c:
        mov eax, dword ptr [esi + 0xbc]
        mov ecx, dword ptr [esi + 0xc0]
        mov edx, dword ptr [esi + 0xb8]
        push eax
        push ecx
        dec edx
        lea ecx, [esi + 0x10]
        call Draw_PolylineViaHook
    L_4bd639:
        pop esi
        add esp, 0x20
        ret
    }
}

// 0x004bfc50 Thunk_ImageWidget_Slot_Draw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_ImageWidget_Slot_Draw(int, int)
{
    __asm {
        jmp ImageWidget_Slot_Draw
    }
}

// 0x004bd660 UiTimers_Tick - Walks list [0x0056bd34] (next at +0). Flags +8: bit1 clear -> 0x004bd4d0; bit1 set with bit2 -> 0x004bd4d0 and clear bit2; bit3 -> 0x004bd4d0 and clear bit3. If bit0: timer +0xc -= real dt [0x0056b424]; when <= 0.0 (0x004d3d70) sets state +4 = 9 and calls 0x004bd470.
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiTimers_Tick(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [g_Data_004da000 + 0x91d34]
        test esi, esi
        jz L_4bd6e0
        push edi
        push ebp
        push ebx
        mov ebp, 0xfffffff7
        mov BL, 0x1
        mov edi, 0x9
    L_4bd67a:
        mov eax, dword ptr [esi + 0x8]
        test AL, 0x2
        jnz L_4bd68a
        mov ecx, esi
        call DrawItem_Render
        jmp L_4bd6af
    L_4bd68a:
        test AL, 0x4
        jz L_4bd69c
        mov ecx, esi
        call DrawItem_Render
        mov eax, dword ptr [esi + 0x8]
        and AL, 0xfb
        jmp L_4bd6ac
    L_4bd69c:
        test AL, 0x8
        jz L_4bd6af
        mov ecx, esi
        call DrawItem_Render
        mov eax, dword ptr [esi + 0x8]
        and eax, ebp
    L_4bd6ac:
        mov dword ptr [esi + 0x8], eax
    L_4bd6af:
        test byte ptr [esi + 0x8], BL
        jz L_4bd6d7
        fld dword ptr [esi + 0xc]
        fsub dword ptr [g_Data_004da000 + 0x91424]
        fcom qword ptr [g_RData_004cc000 + 0x7d70]
        fstp dword ptr [esi + 0xc]
        fnstsw AX
        test AH, 0x41
        jz L_4bd6d7
        mov ecx, esi
        mov dword ptr [esi + 0x4], edi
        call DrawQueue_Remove
    L_4bd6d7:
        mov esi, dword ptr [esi]
        test esi, esi
        jnz L_4bd67a
        pop ebx
        pop ebp
        pop edi
    L_4bd6e0:
        pop esi
        ret
    }
}

// The MSVC "eh vector constructor/destructor iterator" helpers, instruction-level ports (2026-10-07; they replace the
// sonnet 2026-09-29 C++ versions, which dropped the SEH frame and returned 0 instead of the original's EAX).
// CONFIRMED-BINARY: bytes 0x004c5ec0..0x004c5f30, 0x004c5f70..0x004c5fd9, 0x004c6000..0x004c606e. Each runs under an SEH frame
// (handler _except_handler3 via the import stub 0x004c6326; scope tables 0x004d4318 / 0x004d4328 / 0x004d4338 in the data
// mirror, whose filter/handler words relocate to the ported funclets DataTarget_004c5f33, DataTarget_004c5fb0,
// DataTarget_004c5fbd, DataTarget_004c606f).

// 0x004c5ec0 ArrayDtor_Eh2 (array, size, count, dtor; ret 0x10): end = array + size * count, then while (--count >= 0)
// { end -= size; dtor(end) }; the local unwind ArrayDtor_Eh2_Unwind 0x004c5f39 runs at the end (nothing to do once done).
__declspec(naked) int __stdcall ArrayDtor_Eh2(char*, unsigned, int, ElementDtor)
{
    __asm {
        push ebp
        mov ebp, esp
        push -0x1
        push offset g_RData_004cc000 + 0x8318
        push dword ptr [g_Iat__except_handler3_004cc554]
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        add esp, -0xc
        push ebx
        push esi
        push edi
        xor eax, eax
        mov dword ptr [ebp - 0x1c], eax
        mov ecx, dword ptr [ebp + 0x10]
        mov edi, dword ptr [ebp + 0xc]
        imul ecx, edi
        mov esi, dword ptr [ebp + 0x8]
        add esi, ecx
        mov dword ptr [ebp + 0x8], esi
        mov dword ptr [ebp - 0x4], eax
    L_4c5efc:
        dec dword ptr [ebp + 0x10]
        js L_4c5f0d
        sub esi, edi
        mov dword ptr [ebp + 0x8], esi
        mov ecx, esi
        call dword ptr [ebp + 0x14]
        jmp L_4c5efc
    L_4c5f0d:
        mov dword ptr [ebp - 0x1c], 0x1
        mov dword ptr [ebp - 0x4], 0xffffffff
        call ArrayDtor_Eh2_Unwind
        mov ecx, dword ptr [ebp - 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x10
    }
}

// 0x004c5f70 ArrayDtor_Eh (end, size, count, dtor; ret 0x10): while (--count >= 0) { end -= size; dtor(end) }. The filter
// (0x004c5fb0, Seh_CxxExceptionFilter 0x004c5fe0) and handler (0x004c5fbd) blocks sit inside the original body after the
// loop's jmp; they are reached only through the scope table, which relocates to their separate ports.
__declspec(naked) int __stdcall ArrayDtor_Eh(char*, unsigned, int, ElementDtor)
{
    __asm {
        push ebp
        mov ebp, esp
        push -0x1
        push offset g_RData_004cc000 + 0x8328
        push dword ptr [g_Iat__except_handler3_004cc554]
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x8
        push ebx
        push esi
        push edi
        mov dword ptr [ebp - 0x18], esp
        mov dword ptr [ebp - 0x4], 0x0
    L_4c5f9d:
        dec dword ptr [ebp + 0x10]
        js L_4c5fc0
        mov ecx, dword ptr [ebp + 0x8]
        sub ecx, dword ptr [ebp + 0xc]
        mov dword ptr [ebp + 0x8], ecx
        call dword ptr [ebp + 0x14]
        jmp L_4c5f9d
        mov eax, dword ptr [ebp - 0x14]
        push eax
        call Seh_CxxExceptionFilter_004c5fe0
        add esp, 0x4
        ret
        mov esp, dword ptr [ebp - 0x18]
    L_4c5fc0:
        mov dword ptr [ebp - 0x4], 0xffffffff
        mov ecx, dword ptr [ebp - 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x10
    }
}

// 0x004c6000 ArrayCtor_Eh (array, size, count, ctor, dtor; ret 0x14): for (i = 0; i < count; ++i) { ctor(array); array +=
// size }; the local unwind ArrayCtor_Eh_Unwind 0x004c6078 runs at the end (destroys built elements only if not completed;
// the dtor argument is read there).
__declspec(naked) int __stdcall ArrayCtor_Eh(char*, unsigned, int, ElementDtor, ElementDtor)
{
    __asm {
        push ebp
        mov ebp, esp
        push -0x1
        push offset g_RData_004cc000 + 0x8338
        push dword ptr [g_Iat__except_handler3_004cc554]
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        add esp, -0x10
        push ebx
        push esi
        push edi
        xor esi, esi
        mov dword ptr [ebp - 0x20], esi
        mov dword ptr [ebp - 0x4], esi
        mov dword ptr [ebp - 0x1c], esi
        mov ebx, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp + 0x8]
    L_4c6034:
        cmp esi, dword ptr [ebp + 0x10]
        jge L_4c6049
        mov ecx, edi
        call dword ptr [ebp + 0x14]
        add edi, ebx
        mov dword ptr [ebp + 0x8], edi
        inc esi
        mov dword ptr [ebp - 0x1c], esi
        jmp L_4c6034
    L_4c6049:
        mov dword ptr [ebp - 0x20], 0x1
        mov dword ptr [ebp - 0x4], 0xffffffff
        call ArrayCtor_Eh_Unwind
        mov ecx, dword ptr [ebp - 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x14
    }
}

// 0x004bc9f0 ListLabel_Slot_TickAnimated - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabel_Slot_TickAnimated(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc]
        mov ecx, eax
        not ecx
        test CL, 0x10
        jz L_4bcb34
        test AL, 0x1
        jz L_4bca27
        fld dword ptr [esi + 0x10]
        fsub dword ptr [esp + 0x8]
        fcom qword ptr [g_RData_004cc000 + 0x7c68]
        fstp dword ptr [esi + 0x10]
        fnstsw AX
        test AH, 0x41
        jz L_4bca27
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x60]
    L_4bca27:
        mov eax, dword ptr [esi + 0x2b4]
        test eax, eax
        jz L_4bcb2d
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jz L_4bcb2d
        fld dword ptr [esi + 0x2a4]
        fsub dword ptr [esp + 0x8]
        mov eax, dword ptr [esi + 0x2b8]
        cmp eax, 0x3
        fstp dword ptr [esp + 0x8]
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi + 0x2a4], ecx
        ja L_4bcb2d
        cmp eax, 0
        je L_4bca70
        cmp eax, 1
        je L_4bca77
        cmp eax, 2
        je L_4bcac8
        cmp eax, 3
        je L_4bcac8
        int 3  // unreachable: the bounds check above excludes other indices
    L_4bca70:
        mov ecx, esi
        call ListLabel_Slot_Draw
    L_4bca77:
        fld dword ptr [esi + 0x2a4]
        fcomp qword ptr [g_RData_004cc000 + 0x7c68]
        fnstsw AX
        test AH, 0x1
        jz L_4bcab4
        fld dword ptr [esi + 0x2a8]
        fadd dword ptr [esi + 0x2a4]
        mov edx, dword ptr [esi + 0x2bc]
        mov dword ptr [esi + 0x270], 0x1
        neg edx
        mov dword ptr [esi + 0x2bc], edx
        fstp dword ptr [esi + 0x2a4]
    L_4bcab4:
        cmp dword ptr [esi + 0x2bc], 0x1
        jnz L_4bcb34
        mov ecx, esi
        call ListLabel_Slot_Draw
        pop esi
        ret 0x4
    L_4bcac8:
        fld dword ptr [esp + 0x8]
        fcomp qword ptr [g_RData_004cc000 + 0x7c68]
        fnstsw AX
        test AH, 0x1
        jz L_4bcb2d
        mov eax, dword ptr [esi + 0x2bc]
        mov ecx, dword ptr [esi + 0x2a8]
        mov edx, dword ptr [esi + 0x2ac]
        mov dword ptr [esi + 0x2a4], ecx
        mov ecx, dword ptr [esi + 0x150]
        mov dword ptr [esi + 0x270], 0x1
        neg eax
        mov dword ptr [esi + 0x2bc], eax
        mov eax, dword ptr [esi + 0x14c]
        mov dword ptr [esi + 0x14c], edx
        mov edx, dword ptr [esi + 0x2b0]
        mov dword ptr [esi + 0x150], edx
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b0], ecx
    L_4bcb2d:
        mov ecx, esi
        call ListLabel_Slot_Draw
    L_4bcb34:
        pop esi
        ret 0x4
    }
}

// 0x004be2f0 ScreenParticleOverlay_Slot_Tick - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenParticleOverlay_Slot_Tick(int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xb4
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jz L_4be7fb
        fld dword ptr [g_Data_004da000 + 0x91f68]
        fadd dword ptr [ebp + 0x8]
        xor edi, edi
        fstp dword ptr [g_Data_004da000 + 0x91f68]
        cmp dword ptr [esi + 0x50], edi
        jz L_4be7fb
        mov eax, dword ptr [esi + 0x34]
        push ebx
        cmp eax, edi
        jz L_4be343
        mov ecx, dword ptr [eax + 0x8]
        mov edx, dword ptr [eax]
        sub ecx, edx
        mov edx, dword ptr [eax + 0xc]
        mov dword ptr [ebp - 0x24], ecx
        mov ecx, dword ptr [eax + 0x4]
        sub edx, ecx
        mov dword ptr [ebp - 0x28], edx
        jmp L_4be348
    L_4be343:
        call zVideo_GetSurfaceRect
    L_4be348:
        lea eax, [ebp - 0x54]
        lea ecx, [ebp - 0x58]
        push eax
        push ecx
        mov ecx, dword ptr [esi + 0x50]
        lea edx, [ebp - 0x5c]
        call Camera_GetWorldPosition
        mov ecx, dword ptr [esi + 0x50]
        lea edx, [ebp - 0x48]
        lea eax, [ebp - 0x4c]
        push edx
        push eax
        lea edx, [ebp - 0x50]
        call Camera_GetData20Vec
        lea ecx, [ebp - 0x38]
        lea edx, [ebp - 0x5c]
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x4], edx
        mov ebx, offset g_Data_004da000 + 0x91f48
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp - 0x8]
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fsub dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        fld dword ptr [ebp - 0x38]
        fmul dword ptr [g_RData_004cc000 + 0x7e84]
        mov ecx, dword ptr [ebp - 0x58]
        mov eax, dword ptr [ebp - 0x5c]
        mov edx, dword ptr [ebp - 0x54]
        mov dword ptr [g_Data_004da000 + 0x91f4c], ecx
        lea ecx, [ebp + 0xffffff4c]
        mov dword ptr [g_Data_004da000 + 0x91f48], eax
        fstp dword ptr [ebp - 0x38]
        fld dword ptr [ebp - 0x34]
        fmul dword ptr [g_RData_004cc000 + 0x7e84]
        mov dword ptr [g_Data_004da000 + 0x91f50], edx
        fstp dword ptr [ebp - 0x34]
        fld dword ptr [ebp - 0x30]
        fmul dword ptr [g_RData_004cc000 + 0x7e84]
        fstp dword ptr [ebp - 0x30]
        call zTransformStackPushRef
        call zTransformLoadIdentity
        fld dword ptr [ebp - 0x50]
        push ecx
        fchs
        fstp dword ptr [esp]
        call Transform_RotateXLocal
        fld dword ptr [ebp - 0x4c]
        push ecx
        fchs
        fstp dword ptr [esp]
        call Transform_RotateYLocal
        mov edx, 0x1
        lea ecx, [ebp - 0x38]
        call zTransformPoints
        call zTransformStackPop
        lea ecx, [ebp + 0xffffff4c]
        call zTransformStackPushRef
        call zTransformLoadIdentity
        mov eax, dword ptr [ebp - 0x48]
        push eax
        call Transform_RotateZLocal
        mov ecx, dword ptr [ebp - 0x4c]
        push ecx
        call Transform_RotateYLocal
        mov edx, dword ptr [ebp - 0x50]
        push edx
        call Transform_RotateXLocal
        fld dword ptr [esi + 0x70]
        fmul qword ptr [g_RData_004cc000 + 0x7e88]
        lea eax, [esi + 0x74]
        mov ecx, dword ptr [esi + 0x74]
        fld st(0)
        mov dword ptr [ebp - 0x20], ecx
        mov edx, dword ptr [eax + 0x4]
        fmul dword ptr [ebp - 0x20]
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ebp - 0x1c], edx
        mov dword ptr [ebp - 0x18], eax
        mov edx, 0x1
        fstp dword ptr [ebp - 0x20]
        fld st(0)
        fmul dword ptr [ebp - 0x1c]
        fstp dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x18]
        fstp dword ptr [ebp - 0x18]
        lea ecx, [ebp - 0x20]
        call zTransformPoints
        fld dword ptr [esi + 0x68]
        fsin
        mov edx, 0x1
        lea ecx, [ebp - 0x68]
        mov dword ptr [ebp - 0x64], 0x0
        fmul dword ptr [esi + 0x6c]
        fmul qword ptr [g_RData_004cc000 + 0x7e88]
        fstp dword ptr [ebp - 0x68]
        fld dword ptr [esi + 0x68]
        fcos
        fmul dword ptr [esi + 0x6c]
        fmul qword ptr [g_RData_004cc000 + 0x7e88]
        fstp dword ptr [ebp - 0x60]
        call zTransformPoints
        call zTransformStackPop
        lea ecx, [ebp - 0x44]
        lea edx, [ebp - 0x38]
        lea eax, [ebp - 0x20]
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0x2c], eax
        mov ebx, dword ptr [ebp - 0x2c]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp - 0x8]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        lea ecx, [ebp - 0x44]
        lea edx, [ebp - 0x44]
        lea eax, [ebp - 0x68]
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0x2c], eax
        mov ebx, dword ptr [ebp - 0x2c]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp - 0x8]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        lea ecx, [ebp - 0x44]
        mov dword ptr [ebp - 0x8], ecx
        mov ecx, dword ptr [ebp - 0x8]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x4]
        fcomp qword ptr [g_RData_004cc000 + 0x7e90]
        fnstsw AX
        test AH, 0x41
        jnz L_4be568
        lea ecx, [ebp - 0x44]
        call Vec3_NormalizeInPlace
        fstp st(0)
    L_4be568:
        mov edx, dword ptr [ebp - 0x44]
        mov eax, dword ptr [ebp - 0x40]
        mov ecx, dword ptr [ebp - 0x3c]
        mov dword ptr [ebp - 0x14], edx
        lea edx, [ebp - 0x14]
        mov dword ptr [ebp - 0x10], eax
        mov dword ptr [ebp - 0xc], ecx
        mov dword ptr [ebp - 0x8], edx
        mov ecx, dword ptr [ebp - 0x8]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x4]
        fcomp qword ptr [g_RData_004cc000 + 0x7e98]
        fnstsw AX
        test AH, 0x41
        jnz L_4be5da
        lea ecx, [ebp - 0x14]
        call Vec3_NormalizeInPlace
        fstp st(0)
        fld dword ptr [ebp - 0x14]
        fmul dword ptr [g_RData_004cc000 + 0x7ea0]
        fstp dword ptr [ebp - 0x14]
        fld dword ptr [ebp - 0x10]
        fmul dword ptr [g_RData_004cc000 + 0x7ea0]
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0xc]
        fmul dword ptr [g_RData_004cc000 + 0x7ea0]
        fstp dword ptr [ebp - 0xc]
    L_4be5da:
        mov eax, dword ptr [esi + 0x40]
        mov dword ptr [ebp - 0x6c], edi
        cmp eax, edi
        jle L_4be7e3
        fild dword ptr [ebp - 0x24]
        lea eax, [ebp - 0x44]
        lea ecx, [ebp - 0x78]
        mov dword ptr [ebp - 0x80], eax
        lea edx, [ebp - 0x14]
        fstp dword ptr [ebp - 0x4]
        fild dword ptr [ebp - 0x28]
        xor eax, eax
        mov dword ptr [ebp + 0xffffff7c], ecx
        mov dword ptr [ebp - 0x7c], edx
        mov dword ptr [ebp - 0x2c], eax
        fstp dword ptr [ebp - 0x8]
        jmp L_4be613
    L_4be610:
        mov eax, dword ptr [ebp - 0x2c]
    L_4be613:
        mov ecx, dword ptr [esi + 0x64]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        mov ecx, dword ptr [esi + 0x60]
        add edx, eax
        mov dword ptr [ebp - 0x28], edx
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        add edx, eax
        mov dword ptr [ebp - 0x24], edx
        mov ebx, dword ptr [ebp - 0x24]
        mov ecx, dword ptr [ebp - 0x80]
        mov edx, dword ptr [ebp - 0x28]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        mov ecx, dword ptr [esi + 0x60]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        add edx, eax
        mov dword ptr [ebp - 0x28], edx
        mov ebx, dword ptr [ebp - 0x28]
        mov ecx, dword ptr [ebp - 0x7c]
        mov edx, dword ptr [ebp + 0xffffff7c]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        mov ecx, dword ptr [esi + 0x60]
        fld dword ptr [g_RData_004cc000 + 0x7e7c]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        fsub dword ptr [edx + eax*0x1 + 0x8]
        fld dword ptr [g_RData_004cc000 + 0x7e7c]
        fsub dword ptr [ebp - 0x70]
        fld st(0)
        fmul dword ptr [ebp - 0x78]
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x4]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov ecx, dword ptr [esi + 0x38]
        fld st(0)
        mov dword ptr [ecx + edi*0x1], eax
        fmul dword ptr [ebp - 0x74]
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x8]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov edx, dword ptr [esi + 0x38]
        mov ebx, dword ptr [ebp - 0x2c]
        mov dword ptr [edx + edi*0x1 + 0x4], eax
        mov ecx, dword ptr [esi + 0x60]
        mov eax, dword ptr [esi + 0x38]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        add eax, edi
        mov dword ptr [ebp - 0x24], eax
        fld dword ptr [edx + ebx*0x1]
        fmul st(0), st(2)
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x4]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov ecx, eax
        mov eax, dword ptr [ebp - 0x24]
        sub ecx, dword ptr [eax]
        mov dword ptr [eax + 0x8], ecx
        mov eax, dword ptr [esi + 0x60]
        mov edx, dword ptr [esi + 0x38]
        mov ecx, dword ptr [esi + eax*0x4 + 0x58]
        add edx, edi
        mov dword ptr [ebp - 0x24], edx
        fld dword ptr [ecx + ebx*0x1 + 0x4]
        fmul st(0), st(2)
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x8]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov edx, eax
        mov eax, dword ptr [ebp - 0x24]
        sub edx, dword ptr [eax + 0x4]
        mov dword ptr [eax + 0xc], edx
        mov eax, dword ptr [esi + 0x38]
        mov CX, word ptr [esi + 0x44]
        mov word ptr [eax + edi*0x1 + 0x10], CX
        mov edx, dword ptr [esi + 0x38]
        fmul dword ptr [esi + 0x48]
        fstp dword ptr [edx + edi*0x1 + 0x14]
        mov eax, dword ptr [esi + 0x38]
        fld st(0)
        fmul dword ptr [esi + 0x4c]
        fstp dword ptr [eax + edi*0x1 + 0x18]
        mov ecx, dword ptr [esi + 0x54]
        inc ecx
        mov dword ptr [ebp - 0x28], ecx
        fild dword ptr [ebp - 0x28]
        fxch st(1)
        fmulp st(1), st(0)
        fmul qword ptr [g_RData_004cc000 + 0x7ea8]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov edx, dword ptr [esi + 0x38]
        mov dword ptr [edx + edi*0x1 + 0x1c], eax
        mov eax, dword ptr [esi + 0x64]
        mov ecx, dword ptr [esi + eax*0x4 + 0x58]
        add ecx, ebx
        fld dword ptr [ecx + 0x8]
        fabs
        fld dword ptr [ecx + 0x4]
        fabs
        fcomp
        fnstsw AX
        test AH, 0x41
        jz L_4be7b9
        fld dword ptr [ecx]
        fabs
        fcomp
        fnstsw AX
        test AH, 0x41
        fstp st(0)
        jz L_4be7bb
        fld dword ptr [ecx + 0x8]
        fcomp qword ptr [g_RData_004cc000 + 0x7e90]
        fnstsw AX
        test AH, 0x41
        jz L_4be7bb
        fld dword ptr [ecx + 0x8]
        fcomp qword ptr [g_RData_004cc000 + 0x7eb0]
        fnstsw AX
        test AH, 0x1
        jz L_4be7c8
        jmp L_4be7bb
    L_4be7b9:
        fstp st(0)
    L_4be7bb:
        mov edx, dword ptr [ebp - 0x6c]
        push 0x0
        push edx
        mov ecx, esi
        call ScreenParticle_Respawn
    L_4be7c8:
        mov eax, dword ptr [ebp - 0x6c]
        mov ecx, dword ptr [esi + 0x40]
        inc eax
        add edi, 0x20
        add ebx, 0xc
        cmp eax, ecx
        mov dword ptr [ebp - 0x6c], eax
        mov dword ptr [ebp - 0x2c], ebx
        jl L_4be610
    L_4be7e3:
        mov eax, dword ptr [ebp + 0x8]
        mov ecx, esi
        push eax
        call Widget_Tick
        mov eax, dword ptr [esi + 0x60]
        mov ecx, dword ptr [esi + 0x64]
        mov dword ptr [esi + 0x60], ecx
        mov dword ptr [esi + 0x64], eax
        pop ebx
    L_4be7fb:
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004be880 ScreenParticleOverlay_Slot_TickRespawnAll - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenParticleOverlay_Slot_TickRespawnAll(int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xb4
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0xc]
        not eax
        test AL, 0x10
        jz L_4bed1a
        fld dword ptr [g_Data_004da000 + 0x91f6c]
        fadd dword ptr [ebp + 0x8]
        xor edi, edi
        fstp dword ptr [g_Data_004da000 + 0x91f6c]
        cmp dword ptr [esi + 0x50], edi
        jz L_4bed1a
        mov eax, dword ptr [esi + 0x34]
        push ebx
        cmp eax, edi
        jz L_4be8d3
        mov ecx, dword ptr [eax + 0x8]
        mov edx, dword ptr [eax]
        sub ecx, edx
        mov edx, dword ptr [eax + 0xc]
        mov dword ptr [ebp - 0x24], ecx
        mov ecx, dword ptr [eax + 0x4]
        sub edx, ecx
        mov dword ptr [ebp - 0x6c], edx
        jmp L_4be8d8
    L_4be8d3:
        call zVideo_GetSurfaceRect
    L_4be8d8:
        lea eax, [ebp - 0x50]
        lea ecx, [ebp - 0x54]
        push eax
        push ecx
        mov ecx, dword ptr [esi + 0x50]
        lea edx, [ebp - 0x58]
        call Camera_GetWorldPosition
        mov ecx, dword ptr [esi + 0x50]
        lea edx, [ebp - 0x40]
        lea eax, [ebp - 0x44]
        push edx
        push eax
        lea edx, [ebp - 0x48]
        call Camera_GetData20Vec
        lea ecx, [ebp - 0x30]
        lea edx, [ebp - 0x58]
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x4], edx
        mov ebx, offset g_Data_004da000 + 0x91f58
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp - 0x8]
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fsub dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        fld dword ptr [ebp - 0x30]
        fmul dword ptr [g_RData_004cc000 + 0x7e84]
        mov ecx, dword ptr [ebp - 0x54]
        mov eax, dword ptr [ebp - 0x58]
        mov edx, dword ptr [ebp - 0x50]
        mov dword ptr [g_Data_004da000 + 0x91f5c], ecx
        lea ecx, [ebp + 0xffffff4c]
        mov dword ptr [g_Data_004da000 + 0x91f58], eax
        fstp dword ptr [ebp - 0x30]
        fld dword ptr [ebp - 0x2c]
        fmul dword ptr [g_RData_004cc000 + 0x7e84]
        mov dword ptr [g_Data_004da000 + 0x91f60], edx
        fstp dword ptr [ebp - 0x2c]
        fld dword ptr [ebp - 0x28]
        fmul dword ptr [g_RData_004cc000 + 0x7e84]
        fstp dword ptr [ebp - 0x28]
        call zTransformStackPushRef
        call zTransformLoadIdentity
        fld dword ptr [ebp - 0x48]
        push ecx
        fchs
        fstp dword ptr [esp]
        call Transform_RotateXLocal
        fld dword ptr [ebp - 0x44]
        push ecx
        fchs
        fstp dword ptr [esp]
        call Transform_RotateYLocal
        mov edx, 0x1
        lea ecx, [ebp - 0x30]
        call zTransformPoints
        call zTransformStackPop
        lea ecx, [ebp + 0xffffff4c]
        call zTransformStackPushRef
        call zTransformLoadIdentity
        mov eax, dword ptr [ebp - 0x40]
        push eax
        call Transform_RotateZLocal
        mov ecx, dword ptr [ebp - 0x44]
        push ecx
        call Transform_RotateYLocal
        mov edx, dword ptr [ebp - 0x48]
        push edx
        call Transform_RotateXLocal
        fld dword ptr [esi + 0x70]
        fmul qword ptr [g_RData_004cc000 + 0x7e88]
        lea eax, [esi + 0x74]
        mov ecx, dword ptr [esi + 0x74]
        fld st(0)
        mov dword ptr [ebp - 0x20], ecx
        mov edx, dword ptr [eax + 0x4]
        fmul dword ptr [ebp - 0x20]
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ebp - 0x1c], edx
        mov dword ptr [ebp - 0x18], eax
        mov edx, 0x1
        fstp dword ptr [ebp - 0x20]
        fld st(0)
        fmul dword ptr [ebp - 0x1c]
        fstp dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x18]
        fstp dword ptr [ebp - 0x18]
        lea ecx, [ebp - 0x20]
        call zTransformPoints
        fld dword ptr [esi + 0x68]
        fsin
        mov edx, 0x1
        lea ecx, [ebp - 0x64]
        mov dword ptr [ebp - 0x60], 0x0
        fmul dword ptr [esi + 0x6c]
        fmul qword ptr [g_RData_004cc000 + 0x7e88]
        fstp dword ptr [ebp - 0x64]
        fld dword ptr [esi + 0x68]
        fcos
        fmul dword ptr [esi + 0x6c]
        fmul qword ptr [g_RData_004cc000 + 0x7e88]
        fstp dword ptr [ebp - 0x5c]
        call zTransformPoints
        call zTransformStackPop
        lea ecx, [ebp - 0x3c]
        lea edx, [ebp - 0x30]
        lea eax, [ebp - 0x20]
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0x4c], eax
        mov ebx, dword ptr [ebp - 0x4c]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp - 0x8]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        lea ecx, [ebp - 0x3c]
        lea edx, [ebp - 0x3c]
        lea eax, [ebp - 0x64]
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0x4c], eax
        mov ebx, dword ptr [ebp - 0x4c]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp - 0x8]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        lea ecx, [ebp - 0x3c]
        mov dword ptr [ebp - 0x8], ecx
        mov ecx, dword ptr [ebp - 0x8]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x4]
        fcomp qword ptr [g_RData_004cc000 + 0x7e90]
        fnstsw AX
        test AH, 0x41
        jnz L_4beaf8
        lea ecx, [ebp - 0x3c]
        call Vec3_NormalizeInPlace
        fstp st(0)
    L_4beaf8:
        mov edx, dword ptr [ebp - 0x3c]
        mov eax, dword ptr [ebp - 0x38]
        mov ecx, dword ptr [ebp - 0x34]
        mov dword ptr [ebp - 0x14], edx
        lea edx, [ebp - 0x14]
        mov dword ptr [ebp - 0x10], eax
        mov dword ptr [ebp - 0xc], ecx
        mov dword ptr [ebp - 0x8], edx
        mov ecx, dword ptr [ebp - 0x8]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x4]
        fcomp qword ptr [g_RData_004cc000 + 0x7e98]
        fnstsw AX
        test AH, 0x41
        jnz L_4beb6a
        lea ecx, [ebp - 0x14]
        call Vec3_NormalizeInPlace
        fstp st(0)
        fld dword ptr [ebp - 0x14]
        fmul dword ptr [g_RData_004cc000 + 0x7ea0]
        fstp dword ptr [ebp - 0x14]
        fld dword ptr [ebp - 0x10]
        fmul dword ptr [g_RData_004cc000 + 0x7ea0]
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0xc]
        fmul dword ptr [g_RData_004cc000 + 0x7ea0]
        fstp dword ptr [ebp - 0xc]
    L_4beb6a:
        mov eax, dword ptr [esi + 0x40]
        mov dword ptr [ebp - 0x8], edi
        cmp eax, edi
        jle L_4bed02
        fild dword ptr [ebp - 0x24]
        lea eax, [ebp - 0x3c]
        lea ecx, [ebp - 0x78]
        mov dword ptr [ebp - 0x80], eax
        lea edx, [ebp - 0x14]
        fstp dword ptr [ebp - 0x4c]
        fild dword ptr [ebp - 0x6c]
        xor eax, eax
        mov dword ptr [ebp + 0xffffff7c], ecx
        mov dword ptr [ebp - 0x7c], edx
        mov dword ptr [ebp - 0x24], eax
        fstp dword ptr [ebp - 0x4]
        jmp L_4beba3
    L_4beba0:
        mov eax, dword ptr [ebp - 0x24]
    L_4beba3:
        mov ecx, dword ptr [esi + 0x64]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        mov ecx, dword ptr [esi + 0x60]
        add edx, eax
        mov dword ptr [ebp - 0x6c], edx
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        add edx, eax
        mov dword ptr [ebp - 0x68], edx
        mov ebx, dword ptr [ebp - 0x68]
        mov ecx, dword ptr [ebp - 0x80]
        mov edx, dword ptr [ebp - 0x6c]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        mov ecx, dword ptr [esi + 0x60]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        add edx, eax
        mov dword ptr [ebp - 0x68], edx
        mov ebx, dword ptr [ebp - 0x68]
        mov ecx, dword ptr [ebp - 0x7c]
        mov edx, dword ptr [ebp + 0xffffff7c]
        fld dword ptr [ebx]
        fadd dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fadd dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x8]
        fadd dword ptr [ecx + 0x8]
        fxch st(2)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        mov ecx, dword ptr [esi + 0x60]
        fld dword ptr [g_RData_004cc000 + 0x7e7c]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        fsub dword ptr [edx + eax*0x1 + 0x8]
        fld dword ptr [g_RData_004cc000 + 0x7e7c]
        fsub dword ptr [ebp - 0x70]
        fld st(0)
        fmul dword ptr [ebp - 0x78]
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x4c]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov ecx, dword ptr [esi + 0x38]
        fld st(0)
        mov dword ptr [ecx + edi*0x1], eax
        fmul dword ptr [ebp - 0x74]
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x4]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov edx, dword ptr [esi + 0x38]
        mov dword ptr [edx + edi*0x1 + 0x4], eax
        mov eax, dword ptr [esi + 0x38]
        mov ecx, dword ptr [esi + 0x60]
        lea ebx, [eax + edi*0x1]
        mov eax, dword ptr [ebp - 0x24]
        mov edx, dword ptr [esi + ecx*0x4 + 0x58]
        fld dword ptr [edx + eax*0x1]
        fmul st(0), st(2)
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x4c]
        call dword ptr [g_Iat__ftol_004cc5ac]
        sub eax, dword ptr [ebx]
        mov dword ptr [ebx + 0x8], eax
        mov ecx, dword ptr [esi + 0x38]
        mov edx, dword ptr [esi + 0x60]
        lea ebx, [ecx + edi*0x1]
        mov ecx, dword ptr [ebp - 0x24]
        mov eax, dword ptr [esi + edx*0x4 + 0x58]
        fld dword ptr [eax + ecx*0x1 + 0x4]
        fmul st(0), st(2)
        fsub dword ptr [g_RData_004cc000 + 0x7ea4]
        fmul dword ptr [ebp - 0x4]
        call dword ptr [g_Iat__ftol_004cc5ac]
        sub eax, dword ptr [ebx + 0x4]
        mov dword ptr [ebx + 0xc], eax
        mov edx, dword ptr [esi + 0x38]
        mov AX, word ptr [esi + 0x44]
        mov word ptr [edx + edi*0x1 + 0x10], AX
        mov ecx, dword ptr [esi + 0x38]
        fmul dword ptr [esi + 0x48]
        fstp dword ptr [ecx + edi*0x1 + 0x14]
        mov edx, dword ptr [esi + 0x38]
        fmul dword ptr [esi + 0x4c]
        fstp dword ptr [edx + edi*0x1 + 0x18]
        mov eax, dword ptr [esi + 0x38]
        mov dword ptr [eax + edi*0x1 + 0x1c], 0x1
        mov ebx, dword ptr [ebp - 0x8]
        push 0x0
        push ebx
        mov ecx, esi
        call ScreenParticle_Respawn
        mov edx, dword ptr [ebp - 0x24]
        mov eax, dword ptr [esi + 0x40]
        inc ebx
        add edx, 0xc
        add edi, 0x20
        cmp ebx, eax
        mov dword ptr [ebp - 0x8], ebx
        mov dword ptr [ebp - 0x24], edx
        jl L_4beba0
    L_4bed02:
        mov ecx, dword ptr [ebp + 0x8]
        push ecx
        mov ecx, esi
        call Widget_Tick
        mov eax, dword ptr [esi + 0x60]
        mov edx, dword ptr [esi + 0x64]
        mov dword ptr [esi + 0x60], edx
        mov dword ptr [esi + 0x64], eax
        pop ebx
    L_4bed1a:
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004bf630 MessageBoxScreen_RunModal - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall MessageBoxScreen_RunModal(int, int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        sub esp, 0x1c
        test eax, eax
        push ebp
        push esi
        push edi
        mov esi, ecx
        jz L_4bf64b
        xor edx, edx
        xor ecx, ecx
        call dword ptr [g_Data_004da000 + 0x91bfc]
    L_4bf64b:
        xor ecx, ecx
        call zVideo_SetFlag0063212c
        xor ecx, ecx
        mov ebp, eax
        call Widget_SetGlobalDirtyMode
        lea eax, [esp + 0xc]
        lea ecx, [esp + 0x10]
        push eax
        push ecx
        lea edx, [esp + 0x2c]
        lea ecx, [esp + 0x28]
        mov dword ptr [esp + 0x20], 0x0
        mov dword ptr [esp + 0x24], 0x0
        call Video_GetSurfaceInfo
        mov dword ptr [esp + 0x14], eax
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        test eax, eax
        lea edi, [esi + 0xa94c]
        jz L_4bf6a8
        call zVideo_GetGlobal_00632228
        push eax
        call zVideo_GetScreenDepth
        push eax
        call zVideo_GetGlobal_00632230
        jmp L_4bf6b9
    L_4bf6a8:
        call zVideo_GetGlobal_00632208
        push eax
        call zVideo_GetScreenDepth
        push eax
        call zVideo_GetGlobal_00632210
    L_4bf6b9:
        mov ecx, eax
        mov edx, edi
        call zRndr_SetFramebuffer
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        mov dword ptr [esi + 0xa95c], 0x0
        mov dword ptr [esi + 0xa960], 0x186a0
        call dword ptr [edx + 0x4]
        mov edx, dword ptr [esp + 0x2c]
        mov ecx, dword ptr [esi + 0xaa34]
        lea eax, [esi + 0xaa34]
        push edx
        push eax
        call dword ptr [ecx + 0x74]
        mov edx, dword ptr [esp + 0x38]
        mov ecx, dword ptr [esi + 0xacd8]
        add esp, 0x8
        lea eax, [esi + 0xacd8]
        push edx
        push eax
        call dword ptr [ecx + 0x74]
        mov eax, dword ptr [esi + 0xaf7c]
        lea ecx, [esi + 0xaf7c]
        add esp, 0x8
        push 0x1
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0xa960]
        test eax, eax
        lea ecx, [eax - 0x1]
        mov dword ptr [esi + 0xa960], ecx
        jle L_4bf775
    L_4bf733:
        xor CL, CL
        call Input_PollAllDevices
        call Timer_UpdateFrame
        call zVideo_NotifyScreenResizeFromSurface
        mov eax, dword ptr [g_Data_004da000 + 0x91424]
        mov edx, dword ptr [esi]
        push eax
        mov ecx, esi
        call dword ptr [edx]
        call zVideo_CallSurfaceSlot3C0_Front
        push 0x1
        push 0x1
        mov edx, edi
        mov ecx, edi
        call zVideo_PresentFrame
        mov eax, dword ptr [esi + 0xa960]
        test eax, eax
        lea ecx, [eax - 0x1]
        mov dword ptr [esi + 0xa960], ecx
        jg L_4bf733
    L_4bf775:
        mov ecx, esi
        call ScreenBase_DrawBackground
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
        mov ecx, ebp
        call zVideo_SetFlag0063212c
        mov ecx, ebp
        call Widget_SetGlobalDirtyMode
        mov eax, dword ptr [esp + 0xc]
        mov ecx, dword ptr [esp + 0x10]
        push eax
        push ecx
        mov ecx, dword ptr [esp + 0x1c]
        lea edx, [esp + 0x20]
        call zRndr_SetFramebuffer
        mov eax, dword ptr [esi + 0xa95c]
        pop edi
        pop esi
        pop ebp
        add esp, 0x1c
        ret 0x10
    }
}

// 0x004c5f39 ArrayDtor_Eh2_Unwind - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ArrayDtor_Eh2_Unwind(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x1c]
        test eax, eax
        jnz L_4c5f4f
        mov edx, dword ptr [ebp + 0x14]
        push edx
        mov eax, dword ptr [ebp + 0x10]
        push eax
        push edi
        push esi
        call ArrayDtor_Eh
    L_4c5f4f:
        ret
    }
}

// 0x004c6078 ArrayCtor_Eh_Unwind - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ArrayCtor_Eh_Unwind(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        test eax, eax
        jnz L_4c608b
        mov eax, dword ptr [ebp + 0x18]
        push eax
        push esi
        push ebx
        push edi
        call ArrayDtor_Eh
    L_4c608b:
        ret
    }
}

// 0x004bdfd0 FadeOverlay_Slot_Draw - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall FadeOverlay_Slot_Draw(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        sub esp, 0x60
        test eax, eax
        push ebx
        push ebp
        push esi
        push edi
        mov esi, ecx
        jz L_4be1f3
        call zVideo_GetGlobal_00632214
        mov ebp, eax
        test ebp, ebp
        mov dword ptr [esp + 0x1c], ebp
        jz L_4bdffa
        call zVideo_CallSurfaceSlot3C0_Back
    L_4bdffa:
        mov eax, dword ptr [esi + 0x84]
        mov edi, dword ptr [eax + 0x10]
        mov CX, word ptr [edi]
        cmp CX, word ptr [esi + 0x44]
        jz L_4be04c
        mov ebx, dword ptr [eax + 0x14]
        xor ecx, ecx
    L_4be011:
        mov DX, word ptr [esi + 0x44]
        mov eax, ecx
        mov word ptr [edi], DX
        add edi, 0x2
        cdq
        and edx, 0xf
        add ecx, 0xff
        add eax, edx
        sar eax, 0x4
        mov byte ptr [ebx], AL
        inc ebx
        cmp ecx, 0xff0
        jl L_4be011
        mov eax, dword ptr [esi + 0x84]
        mov ecx, dword ptr [esi + 0x88]
        push eax
        xor edx, edx
        call dword ptr [g_Data_004da000 + 0x91c18]
    L_4be04c:
        call Render_BeginSceneIfIdle
        mov eax, dword ptr [esi + 0x40]
        xor ebx, ebx
        cmp eax, ebx
        mov dword ptr [esp + 0x14], ebx
        jle L_4be1d7
        mov dword ptr [esp + 0x10], ebx
    L_4be066:
        mov ecx, dword ptr [esi + 0x38]
        mov edx, dword ptr [esp + 0x10]
        lea eax, [edx + ecx*0x1]
        mov ecx, dword ptr [edx + ecx*0x1 + 0x8]
        mov edx, dword ptr [eax + 0xc]
        cmp ecx, edx
        jle L_4be086
        fild dword ptr [eax + 0x1c]
        fld dword ptr [g_RData_004cc000 + 0x7e80]
        jmp L_4be08f
    L_4be086:
        fld dword ptr [g_RData_004cc000 + 0x7e80]
        fild dword ptr [eax + 0x1c]
    L_4be08f:
        fild dword ptr [eax]
        fld dword ptr [g_RData_004cc000 + 0x7e80]
        fxch st(1)
        fstp dword ptr [esp + 0x40]
        mov edx, dword ptr [esi + 0x60]
        fild dword ptr [eax + 0x4]
        fld dword ptr [g_RData_004cc000 + 0x7e80]
        fxch st(1)
        fstp dword ptr [esp + 0x44]
        mov ecx, dword ptr [esi + edx*0x4 + 0x58]
        lea edi, [esi + edx*0x4 + 0x58]
        fld dword ptr [g_RData_004cc000 + 0x7e80]
        fld dword ptr [ebx + ecx*0x1 + 0x8]
        mov ecx, dword ptr [esi + 0x34]
        lea edx, [esp + 0x20]
        fstp dword ptr [esp + 0x48]
        mov ebp, dword ptr [eax + 0x14]
        push ecx
        fxch st(2)
        fstp dword ptr [esp + 0x28]
        fld dword ptr [g_RData_004cc000 + 0x7e80]
        mov dword ptr [esp + 0x24], ebp
        push 0x4
        fild dword ptr [eax]
        lea ecx, [esp + 0x48]
        fadd st(0), st(5)
        fstp dword ptr [esp + 0x54]
        fild dword ptr [eax + 0x4]
        fadd st(0), st(4)
        fstp dword ptr [esp + 0x58]
        mov ebp, dword ptr [edi]
        fld dword ptr [ebx + ebp*0x1 + 0x8]
        fstp dword ptr [esp + 0x5c]
        mov ebp, dword ptr [eax + 0x14]
        fxch st(1)
        fstp dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x30], ebp
        mov ebp, dword ptr [eax + 0x8]
        add ebp, dword ptr [eax]
        mov dword ptr [esp + 0x20], ebp
        fild dword ptr [esp + 0x20]
        fadd st(0), st(4)
        fstp dword ptr [esp + 0x60]
        mov ebp, dword ptr [eax + 0xc]
        add ebp, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x20], ebp
        fild dword ptr [esp + 0x20]
        fadd st(0), st(3)
        fstp dword ptr [esp + 0x64]
        mov ebp, dword ptr [edi]
        fld dword ptr [ebx + ebp*0x1 + 0x8]
        fstp dword ptr [esp + 0x68]
        mov ebp, dword ptr [eax + 0x18]
        fxch st(1)
        fstp dword ptr [esp + 0x3c]
        mov dword ptr [esp + 0x38], ebp
        mov ebp, dword ptr [eax + 0x8]
        add ebp, dword ptr [eax]
        mov dword ptr [esp + 0x20], ebp
        fild dword ptr [esp + 0x20]
        fstp dword ptr [esp + 0x6c]
        mov ebp, dword ptr [eax + 0xc]
        add ebp, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x20], ebp
        fild dword ptr [esp + 0x20]
        fstp dword ptr [esp + 0x70]
        mov edi, dword ptr [edi]
        fld dword ptr [ebx + edi*0x1 + 0x8]
        fstp dword ptr [esp + 0x74]
        mov eax, dword ptr [eax + 0x18]
        fstp dword ptr [esp + 0x44]
        mov dword ptr [esp + 0x40], eax
        fstp st(0)
        fstp st(0)
        call Points_AllInsideIntRect
        test eax, eax
        jz L_4be1b1
        mov ecx, dword ptr [esi + 0x88]
        push 0x0
        push 0x3f800000
        push 0x1
        push ecx
        push 0x4
        lea edx, [esp + 0x34]
        lea ecx, [esp + 0x54]
        call dword ptr [g_Data_004da000 + 0x91c5c]
    L_4be1b1:
        mov eax, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esi + 0x40]
        inc eax
        add ebx, 0xc
        add edx, 0x20
        cmp eax, ecx
        mov dword ptr [esp + 0x14], eax
        mov dword ptr [esp + 0x10], edx
        jl L_4be066
        mov ebp, dword ptr [esp + 0x1c]
    L_4be1d7:
        call dword ptr [g_Data_004da000 + 0x91c6c]
        call Render_EndSceneIfLast
        test ebp, ebp
        jz L_4be202
        call zVideo_NotifyScreenResize
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x60
        ret
    L_4be1f3:
        mov edx, dword ptr [esi + 0x34]
        mov ecx, dword ptr [esi + 0x38]
        push edx
        mov edx, dword ptr [esi + 0x40]
        call Raster_DrawBlendLineList
    L_4be202:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x60
        ret
    }
}

// 0x00407160 Slot_ReturnTrue_Ret8_00407160 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Slot_ReturnTrue_Ret8_00407160(int, int, int, int)
{
    __asm {
        mov eax, 0x1
        ret 0x8
    }
}

// 0x0048eb80 Raster_MaskRect16_0048eb80 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Raster_MaskRect16_0048eb80(int, int)
{
    __asm {
        sub esp, 0x1c
        push ebx
        push ebp
        push esi
        xor esi, esi
        cmp ecx, esi
        push edi
        jz L_48ebaa
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [ecx + 0x4]
        mov dword ptr [esp + 0x1c], eax
        mov eax, dword ptr [ecx + 0x8]
        mov ecx, dword ptr [ecx + 0xc]
        mov dword ptr [esp + 0x20], edx
        mov dword ptr [esp + 0x24], eax
        mov dword ptr [esp + 0x28], ecx
        jmp L_48ebc7
    L_48ebaa:
        mov edx, dword ptr [g_Data_004da000 + 0x911cc]
        mov eax, dword ptr [g_Data_004da000 + 0x911c8]
        dec edx
        dec eax
        mov dword ptr [esp + 0x20], esi
        mov dword ptr [esp + 0x1c], esi
        mov dword ptr [esp + 0x28], edx
        mov dword ptr [esp + 0x24], eax
    L_48ebc7:
        lea ecx, [esp + 0x18]
        lea edx, [esp + 0x10]
        push ecx
        lea ecx, [esp + 0x18]
        call Pixfmt_GetChannelMasks
        cmp dword ptr [g_Data_004da000 + 0x91be8], esi
        jz L_48ec00
        mov ecx, dword ptr [esp + 0x10]
        push 0x3fd33333
        lea edx, [esp + 0x20]
        push 0x33333333
        call Render_QueueScreenRect
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x1c
        ret
    L_48ec00:
        mov eax, dword ptr [esp + 0x14]
        mov edx, eax
        shl edx, 0x10
        or eax, edx
        mov dword ptr [esp + 0x14], eax
        mov eax, dword ptr [esp + 0x10]
        mov ecx, eax
        shl ecx, 0x10
        or eax, ecx
        mov ecx, dword ptr [esp + 0x1c]
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esp + 0x18]
        mov edx, eax
        shl edx, 0x10
        or eax, edx
        mov edx, dword ptr [esp + 0x24]
        mov dword ptr [esp + 0x18], eax
        mov eax, dword ptr [esp + 0x20]
        mov esi, eax
        sub edx, ecx
        imul esi, dword ptr [g_Data_004da000 + 0x911d4]
        add esi, ecx
        mov ecx, dword ptr [g_Data_004da000 + 0x911c4]
        dec edx
        mov edi, eax
        lea esi, [ecx + esi*0x2]
        mov ecx, dword ptr [esp + 0x28]
        sar edx, 0x1
        cmp eax, ecx
        jge L_48ec85
    L_48ec5c:
        mov eax, esi
        mov ecx, edx
    L_48ec60:
        mov ebx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [eax]
        and ebp, ebx
        mov ebx, ecx
        mov dword ptr [eax], ebp
        add eax, 0x4
        dec ecx
        test ebx, ebx
        jnz L_48ec60
        mov eax, dword ptr [g_Data_004da000 + 0x911d4]
        inc edi
        lea esi, [esi + eax*0x2]
        mov eax, dword ptr [esp + 0x28]
        cmp edi, eax
        jl L_48ec5c
    L_48ec85:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x1c
        ret
    }
}

// 0x004b4b30 UiWidget_ForwardVirtual84_004b4b30 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiWidget_ForwardVirtual84_004b4b30(int, int)
{
    __asm {
        test edx, edx
        jz L_4b4b40
        mov eax, dword ptr [edx]
        push ecx
        mov ecx, edx
        call dword ptr [eax + 0x84]
        ret
    L_4b4b40:
        xor eax, eax
        ret
    }
}

// 0x004ba4a0 UiObject_ResetFields_004ba4a0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiObject_ResetFields_004ba4a0(int, int)
{
    __asm {
        mov eax, ecx
        xor ecx, ecx
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [eax + 0x8], ecx
        mov dword ptr [eax + 0xc], ecx
        mov dword ptr [eax + 0x18], ecx
        mov dword ptr [eax + 0x20], ecx
        mov dword ptr [eax + 0x1c], 0x1f4
        ret
    }
}

// 0x004ba4c0 UiObject_ClearFirstWord_004ba4c0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiObject_ClearFirstWord_004ba4c0(int, int)
{
    __asm {
        mov dword ptr [ecx], 0x0
        ret
    }
}

// 0x004bdbe0 UiWidget_Subclass_Ctor_004bdbe0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiWidget_Subclass_Ctor_004bdbe0(int, int)
{
    __asm {
        push esi
        push 0x0
        mov esi, ecx
        push 0x0
        call Widget_BaseCtor
        mov dword ptr [esi + 0x34], 0x0
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7d78
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004bfc80 UiWidget_Subclass_Ctor_004bfc80 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiWidget_Subclass_Ctor_004bfc80(int, int)
{
    __asm {
        push esi
        push 0x0
        mov esi, ecx
        push 0x0
        call Widget_BaseCtor
        mov dword ptr [esi], offset g_RData_004cc000 + 0x8248
        mov byte ptr [esi + 0x3e], 0x0
        mov dword ptr [esi + 0x34], 0x0
        mov dword ptr [esi + 0x38], 0x0
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004c5fe0 Seh_CxxExceptionFilter_004c5fe0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Seh_CxxExceptionFilter_004c5fe0(int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov ecx, dword ptr [eax]
        cmp dword ptr [ecx], 0xe06d7363
        jnz L_4c5ff3
        call dword ptr [g_Iat__terminate__YAXXZ_004cc558]
    L_4c5ff3:
        xor eax, eax
        ret
    }
}

// 0x00423450 Widget_Slot_ThunkMember34To0048eb80 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_ThunkMember34To0048eb80(int, int)
{
    __asm {
        mov ecx, dword ptr [ecx + 0x34]
        jmp Raster_MaskRect16_0048eb80
    }
}

// 0x004b4c50 EditField_SetFocus - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EditField_SetFocus(int, int, int)
{
    __asm {
        mov AL, byte ptr [esp + 0x4]
        mov DL, byte ptr [ecx + 0x370]
        cmp AL, DL
        jz L_4b4c80
        test AL, AL
        mov byte ptr [ecx + 0x370], AL
        jz L_4b4c77
        mov edx, ecx
        mov ecx, offset UiWidget_ForwardVirtual84_004b4b30
        call Input_Keyboard_SetCharCallback
        ret 0x4
    L_4b4c77:
        xor edx, edx
        xor ecx, ecx
        call Input_Keyboard_SetCharCallback
    L_4b4c80:
        ret 0x4
    }
}

// 0x004c5a50 Thunk_MFC42_Ord5265_004c5a50 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_MFC42_Ord5265_004c5a50(int, int)
{
    __asm {
        jmp dword ptr [g_Iat_MFC42_5265_004cc3e4]
    }
}

// 0x004b7250 Button_OnRelease - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_OnRelease(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc4]
        test eax, eax
        jz L_4b7287
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jnz L_4b7287
        mov ecx, dword ptr [esi + 0xec]
        test ecx, ecx
        jz L_4b7280
        call Sound_StopOrRestoreVoice
        mov dword ptr [esi + 0xec], 0x0
    L_4b7280:
        mov ecx, esi
        call Button_Reset
    L_4b7287:
        pop esi
        ret
    }
}

// 0x00403d70 ScalarDeletingDtor_00403d70 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_00403d70(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ImageWidget_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_403d88
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_403d88:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00403eb0 ScalarDeletingDtor_00403eb0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_00403eb0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ToggleImage_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_403ec8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_403ec8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004070e0 UiButton_Slot_004434b0_0_ThenActivate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_004434b0_0_ThenActivate(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00408c20 UiScreen_Slot_Call0040bda0_Then004b5900 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call0040bda0_Then004b5900(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Enter_004e5df0
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00409160 UiButton_Slot_004434b0_0_ThenActivate_B - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_004434b0_0_ThenActivate_B(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00409180 UiScreen_Slot_004434b0_1_Set4f3d7c - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_004434b0_1_Set4f3d7c(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x1
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        push 0x0
        push offset g_Data_004da000 + 0x19e78
        mov ecx, offset g_Data_004da000 + 0x19ca8
        mov dword ptr [g_Data_004da000 + 0x19d7c], 0x1
        call ScreenManager_QueuePush
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00409360 ScalarDeletingDtor_004091e0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004091e0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call TextPanel_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_409378
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_409378:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00409570 TextPanel_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall TextPanel_LoadFromConfig(int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_TextPanel_LoadFromConfig
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x2d8
        mov eax, dword ptr [esp + 0x2ec]
        push ebx
        push ebp
        push esi
        mov esi, dword ptr [esp + 0x2f4]
        push edi
        mov ebp, ecx
        push eax
        push esi
        mov dword ptr [esp + 0x18], ebp
        call Button_LoadFromConfig
        mov edx, offset g_Data_004da000 + 0x99c
        mov ecx, esi
        call ConfigTree_FindChild
        xor edi, edi
        cmp eax, edi
        jz L_40960c
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [ebp + 0xbc]
        mov edx, dword ptr [edx + 0xc]
        mov edx, dword ptr [edx + 0xc]
        add edx, ecx
        mov dword ptr [ebp + 0x15c], edx
        mov ebx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ebp + 0xc0]
        mov ebx, dword ptr [ebx + 0xc]
        mov ebx, dword ptr [ebx + 0x14]
        add ebx, edx
        mov dword ptr [ebp + 0x160], ebx
        mov ebx, dword ptr [eax + 0x4]
        mov ebx, dword ptr [ebx + 0x14]
        mov ebx, dword ptr [ebx + 0xc]
        add ebx, ecx
        mov dword ptr [ebp + 0x164], ebx
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0x14]
        mov eax, dword ptr [ecx + 0x14]
        add eax, edx
        mov dword ptr [ebp + 0x168], eax
    L_40960c:
        mov edx, offset g_Data_004da000 + 0x990
        mov ecx, esi
        call ConfigTree_FindChild
        cmp eax, edi
        jz L_40962c
        mov edx, dword ptr [esp + 0x2fc]
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [edx + 0xa94c], ecx
    L_40962c:
        mov edx, offset g_Data_004da000 + 0x980
        mov ecx, esi
        call ConfigTree_FindChild
        mov ebx, eax
        cmp ebx, edi
        mov dword ptr [esp + 0x30], ebx
        jz L_4098ea
        mov AL, byte ptr [esp + 0x1b]
        mov dword ptr [esp + 0x24], edi
        mov byte ptr [esp + 0x20], AL
        mov dword ptr [esp + 0x28], edi
        mov dword ptr [esp + 0x2c], edi
        mov ecx, dword ptr [ebx + 0x4]
        mov esi, 0x1
        mov dword ptr [esp + 0x2f0], edi
        mov dword ptr [esp + 0x14], esi
        mov eax, dword ptr [ecx + 0x4]
        cmp eax, esi
        mov dword ptr [esp + 0x38], eax
        jle L_409820
    L_40967c:
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [edx + esi*0x8 + 0x4]
        mov edx, dword ptr [esp + 0x24]
        push edx
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x38], ecx
        mov ecx, dword ptr [esp + 0x2c]
        mov edx, ecx
        call ListLabel_CopyRange
        mov edi, eax
        mov eax, dword ptr [esp + 0x28]
        push eax
        push edi
        lea ecx, [esp + 0x28]
        call ListLabel_DestroyRange
        mov ecx, dword ptr [esp + 0x34]
        mov eax, 0x1
        cmp ecx, eax
        mov dword ptr [esp + 0x28], edi
        mov dword ptr [esp + 0x1c], eax
        jle L_4097f4
    L_4096c4:
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [edx + esi*0x8 + 0x4]
        lea ebp, [ecx*0x8 + 0x0]
        mov ecx, dword ptr [eax + ecx*0x8 + 0x4]
        mov ecx, dword ptr [ecx + 0xc]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [ebx + 0x4]
        mov edi, eax
        push 0x0
        push 0x0
        mov eax, dword ptr [edx + esi*0x8 + 0x4]
        push 0x0
        lea ecx, [esp + 0x48]
        mov eax, dword ptr [eax + ebp*0x1 + 0x4]
        mov ebp, dword ptr [eax + 0x14]
        mov ebx, dword ptr [eax + 0x1c]
        mov esi, dword ptr [eax + 0x24]
        call ListLabel_Ctor
        push edi
        lea ecx, [esp + 0x40]
        push offset g_Data_004da000 + 0x97c
        push ecx
        mov byte ptr [esp + 0x2fc], 0x1
        call ListLabel_Printf
        mov edx, dword ptr [esp + 0x1c]
        lea ecx, [esi + esi*0x8]
        add esp, 0xc
        mov eax, dword ptr [edx + 0xc8]
        mov esi, dword ptr [eax + ecx*0x4 + 0x1cec]
        lea eax, [eax + ecx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        mov dword ptr [esp + 0x2e0], ebp
        and esi, eax
        mov dword ptr [esp + 0x2e4], ebx
        test esi, esi
        jz L_4097a9
        mov edx, dword ptr [esi + 0x1c]
        mov eax, dword ptr [esi + 0x8]
        mov ecx, dword ptr [esi + 0x4]
        push 0x2
        push 0x0
        push 0x0
        push 0x0
        push edx
        push eax
        push ecx
        lea ecx, [esp + 0x58]
        call TextWidget_SetFont
        mov eax, dword ptr [esi + 0xc]
        mov dword ptr [esp + 0x188], eax
        mov dword ptr [esp + 0x18c], eax
        mov eax, 0x1
        mov dword ptr [esp + 0x2ac], eax
        mov edx, dword ptr [esi + 0x18]
        mov dword ptr [esp + 0x2a0], edx
        mov dword ptr [esp + 0x2d8], eax
        mov dword ptr [esp + 0x2dc], eax
        jmp L_4097ae
    L_4097a9:
        mov eax, 0x1
    L_4097ae:
        mov edx, dword ptr [esp + 0x28]
        lea ecx, [esp + 0x3c]
        push ecx
        push eax
        push edx
        lea ecx, [esp + 0x2c]
        call ListLabelVector_InsertN
        lea ecx, [esp + 0x3c]
        mov byte ptr [esp + 0x2f0], 0x0
        call ListLabel_Dtor
        mov eax, dword ptr [esp + 0x1c]
        mov ecx, dword ptr [esp + 0x34]
        mov esi, dword ptr [esp + 0x14]
        mov ebx, dword ptr [esp + 0x30]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x1c], eax
        jl L_4096c4
        mov ebp, dword ptr [esp + 0x10]
    L_4097f4:
        mov edx, dword ptr [ebp + 0x154]
        lea ecx, [ebp + 0x14c]
        lea eax, [esp + 0x20]
        push eax
        push 0x1
        push edx
        call GroupVector_InsertN
        mov eax, dword ptr [esp + 0x38]
        inc esi
        cmp esi, eax
        mov dword ptr [esp + 0x14], esi
        jl L_40967c
        xor edi, edi
    L_409820:
        mov dword ptr [ebp + 0x16c], edi
        mov ecx, dword ptr [ebp + 0x150]
        mov eax, dword ptr [ebp + 0x154]
        mov dword ptr [esp + 0x10], ecx
        cmp ecx, eax
        jz L_4098b5
        lea edi, [ecx + 0x8]
    L_40983d:
        mov esi, dword ptr [edi - 0x4]
        mov eax, dword ptr [edi]
        xor ebx, ebx
        cmp esi, eax
        jz L_40986b
    L_409848:
        mov ecx, esi
        call ListLabel_GetLineHeight
        add eax, dword ptr [esi + 0x2a8]
        cmp eax, ebx
        jle L_40985b
        mov ebx, eax
    L_40985b:
        mov eax, dword ptr [edi]
        add esi, 0x2ac
        cmp esi, eax
        jnz L_409848
        mov ecx, dword ptr [esp + 0x10]
    L_40986b:
        mov eax, dword ptr [edi - 0x4]
        mov edx, dword ptr [edi]
        cmp eax, edx
        jz L_409893
    L_409874:
        mov edx, dword ptr [ebp + 0x16c]
        mov esi, dword ptr [eax + 0x2a8]
        add esi, edx
        mov dword ptr [eax + 0x2a8], esi
        mov edx, dword ptr [edi]
        add eax, 0x2ac
        cmp eax, edx
        jnz L_409874
    L_409893:
        mov eax, dword ptr [ebp + 0x16c]
        add ecx, 0x10
        add eax, ebx
        add edi, 0x10
        mov dword ptr [ebp + 0x16c], eax
        mov eax, dword ptr [ebp + 0x154]
        cmp ecx, eax
        mov dword ptr [esp + 0x10], ecx
        jnz L_40983d
    L_4098b5:
        mov edi, dword ptr [esp + 0x28]
        mov esi, dword ptr [esp + 0x24]
        cmp esi, edi
        mov dword ptr [esp + 0x2f0], 0xffffffff
        jz L_4098dd
    L_4098cc:
        mov ecx, esi
        call ListLabel_Dtor
        add esi, 0x2ac
        cmp esi, edi
        jnz L_4098cc
    L_4098dd:
        mov eax, dword ptr [esp + 0x24]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4098ea:
        mov ecx, dword ptr [esp + 0x2e8]
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x2e4
        ret 0x8
    }
}

// 0x0040a590 ScalarDeletingDtor_0040a590 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040a590(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ListLabel_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_40a5a8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40a5a8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040b0a0 ScalarDeletingDtor_0040a940 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040a940(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call OptionListScreen_Dtor_a940
        test byte ptr [esp + 0x8], 0x1
        jz L_40b0b8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40b0b8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040b0c0 ScalarDeletingDtor_0040aa30 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040aa30(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call OptionListScreen_Dtor_aa30
        test byte ptr [esp + 0x8], 0x1
        jz L_40b0d8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40b0d8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040b0e0 ScalarDeletingDtor_0040ab20 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040ab20(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call OptionListScreen_Dtor_ab20
        test byte ptr [esp + 0x8], 0x1
        jz L_40b0f8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40b0f8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040b100 ScalarDeletingDtor_0040ac10 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040ac10(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call OptionListScreen_Dtor_ac10
        test byte ptr [esp + 0x8], 0x1
        jz L_40b118
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40b118:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040b120 ScalarDeletingDtor_0040ad00 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040ad00(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call OptionListScreen_Dtor_ad00
        test byte ptr [esp + 0x8], 0x1
        jz L_40b138
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40b138:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040b140 CommandsDialog_Tick_WaitForKeyOrButton_0040b140 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall CommandsDialog_Tick_WaitForKeyOrButton_0040b140(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        push edi
        mov esi, ecx
        push eax
        call WidgetRoot_DispatchMouseAndTick
        mov eax, dword ptr [esi + 0xcdfc]
        cmp eax, 0x4
        ja L_40b3b9
        cmp eax, 0
        je L_40b164
        cmp eax, 1
        je L_40b180
        cmp eax, 2
        je L_40b1f8
        cmp eax, 3
        je L_40b270
        cmp eax, 4
        je L_40b315
        int 3  // unreachable: the bounds check above excludes other indices
    L_40b164:
        mov edx, dword ptr [esi + 0xc898]
        lea ecx, [esi + 0xc898]
        push 0x0
        call dword ptr [edx + 0x60]
        dec dword ptr [g_Data_004da000 + 0xbe00]
        pop edi
        pop esi
        ret 0x4
    L_40b180:
        mov eax, dword ptr [esi + 0xc898]
        lea edi, [esi + 0xc898]
        push 0x1
        mov ecx, edi
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [edi]
        push offset g_Data_004da000 + 0xad4
        push edi
        call dword ptr [ecx + 0x74]
        add esp, 0x8
        lea ecx, [esi + 0xb47c]
        push 0x0
        call Toggle_SetState
        push 0x0
        lea ecx, [esi + 0xb8c8]
        call Toggle_SetState
        push 0x0
        lea ecx, [esi + 0xbd14]
        call Toggle_SetState
        xor ecx, ecx
        call Input_Keyboard_WaitForKeyPress
        test eax, eax
        jz L_40b3b9
        mov edx, dword ptr [esi + 0xb460]
        mov ecx, esi
        push edx
        push eax
        call Screen_LeaveAndResetSelection
        push 0x0
        lea ecx, [esi + 0xb030]
        call Toggle_SetState
        pop edi
        pop esi
        ret 0x4
    L_40b1f8:
        mov eax, dword ptr [esi + 0xc898]
        lea edi, [esi + 0xc898]
        push 0x1
        mov ecx, edi
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [edi]
        push offset g_Data_004da000 + 0xad4
        push edi
        call dword ptr [ecx + 0x74]
        add esp, 0x8
        lea ecx, [esi + 0xb030]
        push 0x0
        call Toggle_SetState
        push 0x0
        lea ecx, [esi + 0xb8c8]
        call Toggle_SetState
        push 0x0
        lea ecx, [esi + 0xbd14]
        call Toggle_SetState
        xor ecx, ecx
        call Input_Keyboard_WaitForKeyPress
        test eax, eax
        jz L_40b3b9
        mov edx, dword ptr [esi + 0xb8ac]
        mov ecx, esi
        push edx
        push eax
        call CommandsDialog_OnCaptureKey
        push 0x0
        lea ecx, [esi + 0xb47c]
        call Toggle_SetState
        pop edi
        pop esi
        ret 0x4
    L_40b270:
        mov eax, dword ptr [esi + 0xc898]
        lea edi, [esi + 0xc898]
        push 0x1
        mov ecx, edi
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [edi]
        push offset g_Data_004da000 + 0xab4
        push edi
        call dword ptr [ecx + 0x74]
        add esp, 0x8
        lea ecx, [esi + 0xb030]
        push 0x0
        call Toggle_SetState
        push 0x0
        lea ecx, [esi + 0xb47c]
        call Toggle_SetState
        push 0x0
        lea ecx, [esi + 0xbd14]
        call Toggle_SetState
        xor ecx, ecx
        call Input_Keyboard_WaitForKeyPress
        cmp eax, 0x1
        jnz L_40b2e5
        mov dword ptr [esi + 0xcdfc], 0x0
        call Input_ResetAll
        push 0x0
        lea ecx, [esi + 0xb8c8]
        call Toggle_SetState
        pop edi
        pop esi
        ret 0x4
    L_40b2e5:
        xor ecx, ecx
        call Input_WaitJoystickButton
        test eax, eax
        jz L_40b3b9
        mov edx, dword ptr [esi + 0xbcf8]
        mov ecx, esi
        push edx
        push eax
        call CommandsDialog_OnCaptureJoystickButton
        push 0x0
        lea ecx, [esi + 0xb8c8]
        call Toggle_SetState
        pop edi
        pop esi
        ret 0x4
    L_40b315:
        mov eax, dword ptr [esi + 0xc898]
        lea edi, [esi + 0xc898]
        push 0x1
        mov ecx, edi
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [edi]
        push offset g_Data_004da000 + 0xa98
        push edi
        call dword ptr [ecx + 0x74]
        add esp, 0x8
        lea ecx, [esi + 0xb030]
        push 0x0
        call Toggle_SetState
        push 0x0
        lea ecx, [esi + 0xb47c]
        call Toggle_SetState
        lea edi, [esi + 0xb8c8]
        push 0x0
        mov ecx, edi
        call Toggle_SetState
        xor ecx, ecx
        call Input_Keyboard_WaitForKeyPress
        cmp eax, 0x1
        jnz L_40b388
        mov dword ptr [esi + 0xcdfc], 0x0
        call Input_ResetAll
        push 0x0
        mov ecx, edi
        call Toggle_SetState
        pop edi
        pop esi
        ret 0x4
    L_40b388:
        xor ecx, ecx
        call Input_Mouse_WaitForClick
        test eax, eax
        jz L_40b3b9
        mov edx, dword ptr [esi + 0xc144]
        mov ecx, esi
        push edx
        push eax
        call CommandsDialog_OnCaptureMouseButton
        push 0x0
        lea ecx, [esi + 0xbd14]
        call Toggle_SetState
        mov dword ptr [g_Data_004da000 + 0xbe00], 0xa
    L_40b3b9:
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x0040b930 UiScreen_Slot_0042a550_004716b0_0040b680 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_0042a550_004716b0_0040b680(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 0xc8]
        call ControlsScreen_BuildBindingList
        call Thunk_00470820
        mov eax, dword ptr [edi + 0xc2ac]
        mov ecx, edi
        push eax
        call CommandsDialog_RefreshBindingTable
        mov ecx, esi
        call Button_Activate
        pop edi
        pop esi
        ret
    }
}

// 0x0040b960 OptionCycler_Apply_0040b680 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Apply_0040b680(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Cycler_Next
        mov eax, dword ptr [esi + 0x14c]
        mov ecx, dword ptr [esi + 0xc8]
        push eax
        call CommandsDialog_RefreshBindingTable
        pop esi
        ret
    }
}

// 0x0040ba30 OptionRadio_Select1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionRadio_Select1(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc8]
        mov dword ptr [eax + 0xcdfc], 0x1
        call Input_ResetAll
        mov ecx, esi
        call Toggle_Flip
        pop esi
        ret
    }
}

// 0x0040ba60 UiScreen_Slot_0040b3e0_004b9330_Field430 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_0040b3e0_004b9330_Field430(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 0x430]
        mov ecx, dword ptr [esi + 0xc8]
        push edi
        push 0x0
        call Screen_LeaveAndResetSelection
        push edi
        mov ecx, esi
        call ListScreen_ScrollTo
        pop edi
        pop esi
        ret
    }
}

// 0x0040bab0 OptionRadio_Select2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionRadio_Select2(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc8]
        mov dword ptr [eax + 0xcdfc], 0x2
        call Input_ResetAll
        mov ecx, esi
        call Toggle_Flip
        pop esi
        ret
    }
}

// 0x0040bae0 UiButton_Slot_0040b460_Field430 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b460_Field430(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x430]
        mov ecx, dword ptr [ecx + 0xc8]
        push eax
        push 0x0
        call CommandsDialog_OnCaptureKey
        ret
    }
}

// 0x0040bb00 OptionRadio_Select3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionRadio_Select3(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc8]
        mov dword ptr [eax + 0xcdfc], 0x3
        call Input_ResetAll
        mov ecx, esi
        call Toggle_Flip
        pop esi
        ret
    }
}

// 0x0040bb30 UiButton_Slot_0040b4e0_Field430 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b4e0_Field430(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x430]
        mov ecx, dword ptr [ecx + 0xc8]
        push eax
        push 0x0
        call CommandsDialog_OnCaptureJoystickButton
        ret
    }
}

// 0x0040bb50 OptionRadio_Select4_IfNotLocked - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionRadio_Select4_IfNotLocked(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0xbe00]
        push esi
        test eax, eax
        mov esi, ecx
        jg L_40bb78
        mov eax, dword ptr [esi + 0xc8]
        mov dword ptr [eax + 0xcdfc], 0x4
        call Input_ResetAll
        mov ecx, esi
        call Toggle_Flip
    L_40bb78:
        pop esi
        ret
    }
}

// 0x0040bb80 UiButton_Slot_0040b560_IfNotLocked - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b560_IfNotLocked(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0xbe00]
        test eax, eax
        jg L_40bb9d
        mov eax, dword ptr [ecx + 0x430]
        mov ecx, dword ptr [ecx + 0xc8]
        push eax
        push 0x0
        call CommandsDialog_OnCaptureMouseButton
    L_40bb9d:
        ret
    }
}

// 0x0040bba0 UiButton_Slot_0040b5e0_Plus1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b5e0_Plus1(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x1
        mov ecx, dword ptr [esi + 0xc8]
        call ScreenList_StepWrap
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0040bbc0 UiButton_Slot_0040b5e0_Minus1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b5e0_Minus1(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push -0x1
        mov ecx, dword ptr [esi + 0xc8]
        call ScreenList_StepWrap
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0040bbe0 UiButton_Slot_0040b630_Plus1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b630_Plus1(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x1
        mov ecx, dword ptr [esi + 0xc8]
        call ScreenList_Scroll
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0040bc00 UiButton_Slot_0040b630_Minus1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_0040b630_Minus1(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push -0x1
        mov ecx, dword ptr [esi + 0xc8]
        call ScreenList_Scroll
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0040c260 ScalarDeletingDtor_0040c280 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040c280(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call OptionListScreen_Dtor_NoSEH
        test byte ptr [esp + 0x8], 0x1
        jz L_40c278
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40c278:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040c6e0 OptionButton_Apply_HUDType - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionButton_Apply_HUDType(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        xor ecx, ecx
        mov eax, dword ptr [esi + 0xc8]
        mov edx, dword ptr [eax + 0xaeac]
        test edx, edx
        setnz CL
        inc ecx
        call Settings_ApplyHUDType
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0040c9e0 OptionToggle_Apply_GfxFlagBit_A - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Apply_GfxFlagBit_A(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call Setting_Get_GfxFlags
        mov ecx, esi
        mov edi, eax
        call Toggle_Flip
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jz L_40ca09
        or edi, 0x10
        mov ecx, edi
        call Settings_ApplyGfxFlags
        pop edi
        pop esi
        ret
    L_40ca09:
        and edi, 0xffffffef
        mov ecx, edi
        call Settings_ApplyGfxFlags
        pop edi
        pop esi
        ret
    }
}

// 0x0040ca40 OptionToggle_Apply_GfxFlagBit_B - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Apply_GfxFlagBit_B(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call Setting_Get_GfxFlags
        mov ecx, esi
        mov edi, eax
        call Toggle_Flip
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jz L_40ca6e
        or edi, 0x8
        mov ecx, edi
        call Settings_ApplyGfxFlags
        call Raster_InstallFillTable
        pop edi
        pop esi
        ret
    L_40ca6e:
        and edi, 0xfffffff7
        mov ecx, edi
        call Settings_ApplyGfxFlags
        call Raster_InstallFillTable
        pop edi
        pop esi
        ret
    }
}

// 0x0040caa0 Thunk_Toggle_Flip - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_Toggle_Flip(int, int)
{
    __asm {
        jmp Toggle_Flip
    }
}

// 0x0040cad0 OptionCycler_Apply_ObjectLOD - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Apply_ObjectLOD(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Cycler_Next
        mov ecx, dword ptr [esi + 0x14c]
        call Settings_ApplyObjectLOD
        pop esi
        ret
    }
}

// 0x0040cb10 OptionCycler_Apply_TextureMemory - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Apply_TextureMemory(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Cycler_Next
        mov ecx, dword ptr [esi + 0x14c]
        call Settings_StoreTextureMemory
        pop esi
        ret
    }
}

// 0x0040cb70 OptionCycler_Apply_EffectsLevel - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Apply_EffectsLevel(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Cycler_Next
        mov ecx, dword ptr [esi + 0x14c]
        call Settings_ApplyEffectsLevel
        pop esi
        ret
    }
}

// 0x0040cbb0 OptionToggle_Apply_MuteSound - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Apply_MuteSound(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Toggle_Flip
        mov eax, dword ptr [esi + 0x14c]
        xor ecx, ecx
        test eax, eax
        setz CL
        call Settings_ApplyMuteSound
        pop esi
        ret
    }
}

// 0x0040cbf0 OptionCycler_Apply_SoundLOD - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Apply_SoundLOD(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Cycler_Next
        mov ecx, dword ptr [esi + 0x14c]
        call Settings_StoreSoundLOD
        pop esi
        ret
    }
}

// 0x0040cc30 OptionSlider_Apply_SoundVolume - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionSlider_Apply_SoundVolume(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call Slider_SetFromMouse
        fld dword ptr [esi + 0x14c]
        push ecx
        fstp dword ptr [esp]
        call Settings_ApplySoundVolume
        mov edi, dword ptr [esi]
        call Setting_GetFloat_004e5d44
        push ecx
        mov ecx, esi
        fstp dword ptr [esp]
        call dword ptr [edi + 0x84]
        pop edi
        pop esi
        ret
    }
}

// 0x0040cc80 OptionToggle_Apply_CDAudio - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionToggle_Apply_CDAudio(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Toggle_Flip
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jz L_40ccad
        mov ecx, 0x1
        call Settings_StoreCDAudio
        mov edx, 0x5
        mov ecx, 0x2
        call SoundCD_PlayIfPrepared
        pop esi
        ret
    L_40ccad:
        xor ecx, ecx
        call Settings_StoreCDAudio
        call SoundCD_Stop
        pop esi
        ret
    }
}

// 0x0040cd00 OptionSlider_Apply_004a2880 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionSlider_Apply_004a2880(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Slider_SetFromMouse
        fld dword ptr [esi + 0x14c]
        fmul dword ptr [g_RData_004cc000 + 0x2228]
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov edx, eax
        mov ecx, eax
        call SoundCD_SetVolume
        pop esi
        ret
    }
}

// 0x0040ce80 OptionCycler_Apply_Detail_0x00415670 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionCycler_Apply_Detail_0x00415670(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Cycler_Next
        mov esi, dword ptr [esi + 0x14c]
        cmp esi, 0x5
        ja L_40cee0
        cmp esi, 0
        je L_40ce9a
        cmp esi, 1
        je L_40cea6
        cmp esi, 2
        je L_40ceb2
        cmp esi, 3
        je L_40cebe
        cmp esi, 4
        je L_40ceca
        cmp esi, 5
        je L_40ced6
        int 3  // unreachable: the bounds check above excludes other indices
    L_40ce9a:
        mov ecx, 0x5
        call SetGlobal_004edc68
        pop esi
        ret
    L_40cea6:
        mov ecx, 0x3
        call SetGlobal_004edc68
        pop esi
        ret
    L_40ceb2:
        mov ecx, 0x4
        call SetGlobal_004edc68
        pop esi
        ret
    L_40cebe:
        mov ecx, 0x2
        call SetGlobal_004edc68
        pop esi
        ret
    L_40ceca:
        mov ecx, 0x6
        call SetGlobal_004edc68
        pop esi
        ret
    L_40ced6:
        mov ecx, 0x7
        call SetGlobal_004edc68
    L_40cee0:
        pop esi
        ret
    }
}

// 0x0040daa0 ScalarDeletingDtor_0040d590 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040d590(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Dtor_ImageLabelImage
        test byte ptr [esp + 0x8], 0x1
        jz L_40dab8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40dab8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040dbd0 ScalarDeletingDtor_0040d780 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040d780(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Dtor_2Images
        test byte ptr [esp + 0x8], 0x1
        jz L_40dbe8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40dbe8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040f2b0 ScalarDeletingDtor_0040d610 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040d610(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Dtor_3Images
        test byte ptr [esp + 0x8], 0x1
        jz L_40f2c8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40f2c8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0040fa20 ScalarDeletingDtor_0040fa40 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0040fa40(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Dtor_OwnsObject34
        test byte ptr [esp + 0x8], 0x1
        jz L_40fa38
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_40fa38:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00414f40 UiScreen_Slot_Call00409b00_Then004b5900 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call00409b00_Then004b5900(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Enter_004e5de0
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00414f60 UiScreen_Slot_Call00435f50_Then004b5900 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call00435f50_Then004b5900(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        xor ecx, ecx
        call SaveGameScreen_Open
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00414f80 UiScreen_Slot_Call0041c6c0_Then004b5900 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call0041c6c0_Then004b5900(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Enter_004f32c8
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00414fa0 UiButton_Slot_004434b0_0_Activate_Then00413630 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiButton_Slot_004434b0_0_Activate_Then00413630(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        mov ecx, esi
        call Button_Activate
        call CallGlobal004e5ee8Slot18
        pop esi
        ret
    }
}

// 0x00414fc0 UiScreen_Slot_Call0040d1c0_Then004b5900 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call0040d1c0_Then004b5900(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Enter_004e5e08
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00414fe0 UiScreen_Slot_Call004159b0_Then004b5900 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call004159b0_Then004b5900(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Enter_004edc48
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00415000 UiScreen_Slot_Call00408ff0_Then004b5900 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call00408ff0_Then004b5900(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Screen_Enter_004e5dd0
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00415140 UiScreen_Slot_00435f80_Activate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_00435f80_Activate(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x13c64]
        push esi
        test eax, eax
        mov esi, ecx
        jz L_41515f
        mov ecx, 0x1
        call LoadGameScreen_Open
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    L_41515f:
        xor ecx, ecx
        call LoadGameScreen_Open
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00415740 UiScreen_Slot_Set4edc50_Reset_Back - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Set4edc50_Reset_Back(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x1
        mov ecx, offset g_Data_004da000 + 0x19ca8
        mov dword ptr [g_Data_004da000 + 0x13c50], 0x1
        call ScreenManager_QueuePop
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        push 0x0
        push offset g_Data_004da000 + 0x19e78
        mov ecx, offset g_Data_004da000 + 0x19ca8
        mov dword ptr [g_Data_004da000 + 0x19d7c], 0x1
        call ScreenManager_QueuePush
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00419800 UiScreen_Slot_00443160_4f3e48_0041ad80 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_00443160_4f3e48_0041ad80(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x0
        push offset g_Data_004da000 + 0x19e48
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePush
        mov ecx, 0x1
        call Screen_Enter_004f32a0_WithArg
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x00419830 UiScreen_Slot_Activate_00443160_4f3e78 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Activate_00443160_4f3e78(int, int)
{
    __asm {
        call Button_Activate
        push 0x0
        push offset g_Data_004da000 + 0x19e78
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePush
        ret
    }
}

// 0x0041a160 UiScreen_Slot_004434b0_0_00443160_4f3e78 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_004434b0_0_00443160_4f3e78(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        push 0x0
        push offset g_Data_004da000 + 0x19e78
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePush
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0041a350 IntSpinButton_Slot_Step - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall IntSpinButton_Slot_Step(int, int)
{
    __asm {
        sub esp, 0x14
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0x14c]
        test ecx, ecx
        jz L_41a3bb
        mov eax, dword ptr [ecx]
        push edi
        call dword ptr [eax + 0x8c]
        mov ecx, dword ptr [esi + 0x150]
        mov edi, dword ptr [esi + 0x14c]
        add ecx, eax
        mov eax, dword ptr [edi + 0x374]
        cmp ecx, eax
        jge L_41a383
        mov ecx, eax
    L_41a383:
        mov eax, dword ptr [edi + 0x378]
        cmp ecx, eax
        jle L_41a38f
        mov ecx, eax
    L_41a38f:
        push ecx
        lea ecx, [esp + 0xc]
        push offset g_Data_004da000 + 0xcbc
        push ecx
        call dword ptr [g_Iat_sprintf_004cc5c4]
        add esp, 0xc
        lea edx, [esp + 0x8]
        mov ecx, edi
        push edx
        call EditField_SetText
        mov ecx, dword ptr [esi + 0x14c]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x20]
        pop edi
    L_41a3bb:
        mov ecx, esi
        call Button_Activate
        pop esi
        add esp, 0x14
        ret
    }
}

// 0x0041a570 ScalarDeletingDtor_0041a570 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0041a570(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call RadioGroup_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_41a588
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_41a588:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0041a590 ScalarDeletingDtor_0041a590 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0041a590(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Toggle_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_41a5a8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_41a5a8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0041a7b0 EditField_Slot_Focus - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EditField_Slot_Focus(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 0xc8]
        mov ecx, dword ptr [edi + 0xa94c]
        test ecx, ecx
        jz L_41a7d9
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x8c]
        mov ecx, dword ptr [edi + 0xa94c]
        push 0x0
        call EditField_SetFocus
    L_41a7d9:
        push 0x1
        mov ecx, esi
        mov dword ptr [edi + 0xa94c], esi
        call EditField_SetFocus
        mov ecx, esi
        call EditField_GetText
        push eax
        mov ecx, esi
        call EditField_SetText
        mov ecx, esi
        call EditField_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        not ecx
        dec ecx
        push ecx
        lea ecx, [esi + 0x14c]
        call EditField_SetCursor
        mov ecx, esi
        call Widget_SetField36CAndClose
        pop edi
        pop esi
        ret
    }
}

// 0x0041a820 NetGameSetup_Slot_ModeNext - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGameSetup_Slot_ModeNext(int, int)
{
    __asm {
        sub esp, 0x18
        push ebp
        mov ebp, ecx
        push esi
        push edi
        mov esi, dword ptr [ebp + 0xc8]
        mov dword ptr [esp + 0xc], ebp
        mov eax, dword ptr [esi + 0xb0a8]
        lea ecx, [esi + 0xaf5c]
        inc eax
        push eax
        call Cycler_SetIndex
        mov eax, dword ptr [esi + 0xb0a8]
        mov edx, dword ptr [esi + 0xc930]
        lea ecx, [esi + 0xc930]
        cmp eax, 0x2
        jnz L_41a90d
        xor ebp, ebp
        push ebp
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0xc9ec]
        lea ecx, [esi + 0xc9ec]
        push 0x1
        call dword ptr [eax + 0x60]
        mov edx, dword ptr [esi + 0xba20]
        lea edi, [esi + 0xba20]
        mov ecx, edi
        call dword ptr [edx + 0x8c]
        cmp eax, 0x1
        jnz L_41a8cd
        mov ecx, dword ptr [edi + 0x374]
        mov eax, 0x2
        cmp ecx, eax
        jle L_41a8a1
        mov eax, ecx
    L_41a8a1:
        mov ecx, dword ptr [edi + 0x378]
        cmp eax, ecx
        jle L_41a8ad
        mov eax, ecx
    L_41a8ad:
        push eax
        lea eax, [esp + 0x14]
        push offset g_Data_004da000 + 0xcbc
        push eax
        call dword ptr [g_Iat_sprintf_004cc5c4]
        add esp, 0xc
        lea ecx, [esp + 0x10]
        push ecx
        mov ecx, edi
        call EditField_SetText
    L_41a8cd:
        lea ecx, [esi + 0xb778]
        mov dword ptr [edi + 0x374], 0x2
        mov dword ptr [edi + 0x378], 0x63
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0xc4], ebp
        call dword ptr [edx + 0x78]
        mov eax, dword ptr [esi + 0xb8cc]
        lea ecx, [esi + 0xb8cc]
        mov dword ptr [ecx + 0xc4], ebp
        call dword ptr [eax + 0x78]
        mov ebp, dword ptr [esp + 0xc]
        jmp L_41a984
    L_41a90d:
        push 0x1
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0xc9ec]
        lea ecx, [esi + 0xc9ec]
        push 0x0
        call dword ptr [eax + 0x60]
        lea edi, [esi + 0xba20]
        lea ecx, [esi + 0xb3fc]
        mov dword ptr [edi + 0x374], 0x1
        mov dword ptr [edi + 0x378], 0x63
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0xc4], 0x1
        call dword ptr [edx + 0x78]
        mov eax, dword ptr [esi + 0xb778]
        lea ecx, [esi + 0xb778]
        mov dword ptr [ecx + 0xc4], 0x1
        call dword ptr [eax + 0x78]
        mov edx, dword ptr [esi + 0xb8cc]
        lea ecx, [esi + 0xb8cc]
        mov dword ptr [ecx + 0xc4], 0x1
        call dword ptr [edx + 0x78]
    L_41a984:
        mov eax, dword ptr [edi]
        mov ecx, edi
        call dword ptr [eax + 0x20]
        mov edx, dword ptr [esi + 0xbd9c]
        lea ecx, [esi + 0xbd9c]
        call dword ptr [edx + 0x20]
        mov eax, dword ptr [esi + 0xbef0]
        lea ecx, [esi + 0xbef0]
        call dword ptr [eax + 0x20]
        mov ecx, ebp
        call Button_Activate
        pop edi
        pop esi
        pop ebp
        add esp, 0x18
        ret
    }
}

// 0x0041a9c0 NetGameSetup_Slot_ModePrev - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGameSetup_Slot_ModePrev(int, int)
{
    __asm {
        sub esp, 0x18
        push ebp
        mov ebp, ecx
        push esi
        push edi
        mov esi, dword ptr [ebp + 0xc8]
        mov dword ptr [esp + 0xc], ebp
        mov eax, dword ptr [esi + 0xb0a8]
        lea ecx, [esi + 0xaf5c]
        dec eax
        push eax
        call Cycler_SetIndex
        mov eax, dword ptr [esi + 0xb0a8]
        mov edx, dword ptr [esi + 0xc930]
        lea ecx, [esi + 0xc930]
        cmp eax, 0x2
        jnz L_41aaad
        xor ebp, ebp
        push ebp
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0xc9ec]
        lea ecx, [esi + 0xc9ec]
        push 0x1
        call dword ptr [eax + 0x60]
        mov edx, dword ptr [esi + 0xba20]
        lea edi, [esi + 0xba20]
        mov ecx, edi
        call dword ptr [edx + 0x8c]
        cmp eax, 0x1
        jnz L_41aa6d
        mov ecx, dword ptr [edi + 0x374]
        mov eax, 0x2
        cmp ecx, eax
        jle L_41aa41
        mov eax, ecx
    L_41aa41:
        mov ecx, dword ptr [edi + 0x378]
        cmp eax, ecx
        jle L_41aa4d
        mov eax, ecx
    L_41aa4d:
        push eax
        lea eax, [esp + 0x14]
        push offset g_Data_004da000 + 0xcbc
        push eax
        call dword ptr [g_Iat_sprintf_004cc5c4]
        add esp, 0xc
        lea ecx, [esp + 0x10]
        push ecx
        mov ecx, edi
        call EditField_SetText
    L_41aa6d:
        lea ecx, [esi + 0xb778]
        mov dword ptr [edi + 0x374], 0x2
        mov dword ptr [edi + 0x378], 0x63
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0xc4], ebp
        call dword ptr [edx + 0x78]
        mov eax, dword ptr [esi + 0xb8cc]
        lea ecx, [esi + 0xb8cc]
        mov dword ptr [ecx + 0xc4], ebp
        call dword ptr [eax + 0x78]
        mov ebp, dword ptr [esp + 0xc]
        jmp L_41ab24
    L_41aaad:
        push 0x1
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0xc9ec]
        lea ecx, [esi + 0xc9ec]
        push 0x0
        call dword ptr [eax + 0x60]
        lea edi, [esi + 0xba20]
        lea ecx, [esi + 0xb3fc]
        mov dword ptr [edi + 0x374], 0x1
        mov dword ptr [edi + 0x378], 0x63
        mov edx, dword ptr [ecx]
        mov dword ptr [ecx + 0xc4], 0x1
        call dword ptr [edx + 0x78]
        mov eax, dword ptr [esi + 0xb778]
        lea ecx, [esi + 0xb778]
        mov dword ptr [ecx + 0xc4], 0x1
        call dword ptr [eax + 0x78]
        mov edx, dword ptr [esi + 0xb8cc]
        lea ecx, [esi + 0xb8cc]
        mov dword ptr [ecx + 0xc4], 0x1
        call dword ptr [edx + 0x78]
    L_41ab24:
        mov eax, dword ptr [edi]
        mov ecx, edi
        call dword ptr [eax + 0x20]
        mov edx, dword ptr [esi + 0xbd9c]
        lea ecx, [esi + 0xbd9c]
        call dword ptr [edx + 0x20]
        mov eax, dword ptr [esi + 0xbef0]
        lea ecx, [esi + 0xbef0]
        call dword ptr [eax + 0x20]
        mov ecx, ebp
        call Button_Activate
        pop edi
        pop esi
        pop ebp
        add esp, 0x18
        ret
    }
}

// 0x0041bf10 UiScreen_Slot_Virtual40_Global4f32c0Slot4_Back - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Virtual40_Global4f32c0Slot4_Back(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x40]
        mov ecx, dword ptr [g_Data_004da000 + 0x192c0]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x4]
        call CallGlobal004e5ee8Slot18
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0041bf40 UiScreen_Slot_EnterWithMouseRestore - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_EnterWithMouseRestore(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jnz L_41bf93
        xor ecx, ecx
        call InputContext_Push
        xor edx, edx
        mov ecx, 0x1
        call InputMgr_SetMouseButtonBinding
        call Setting_Get_004e5d60
        test eax, eax
        jnz L_41bf89
        xor edx, edx
        xor ecx, ecx
        push edx
        push edx
        call Input_Mouse_CursorRaycastDispatch
        mov eax, dword ptr [g_Data_004da000 + 0x192bc]
        test eax, eax
        jz L_41bf89
        mov ecx, dword ptr [esi + 0xc8]
        push eax
        call Widget_SetField10
    L_41bf89:
        mov dword ptr [esi + 0x14c], 0x1
    L_41bf93:
        mov ecx, esi
        call Button_Highlight
        pop esi
        ret
    }
}

// 0x0041bfa0 UiScreen_Slot_LeaveWithMouseReset - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_LeaveWithMouseReset(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jz L_41bff0
        call InputContext_Pop
        call Setting_Get_004e5d60
        test eax, eax
        jnz L_41bfe6
        xor edx, edx
        mov ecx, 0x1
        push edx
        push edx
        call Input_Mouse_CursorRaycastDispatch
        mov ecx, dword ptr [esi + 0xc8]
        call Widget_GetField10
        mov dword ptr [g_Data_004da000 + 0x192bc], eax
        mov ecx, dword ptr [esi + 0xc8]
        push 0x0
        call Widget_SetField10
    L_41bfe6:
        mov dword ptr [esi + 0x14c], 0x0
    L_41bff0:
        mov ecx, esi
        call Button_Reset
        pop esi
        ret
    }
}

// 0x0041c270 UiScreen_Slot_0041c500IfOwner_Activate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_0041c500IfOwner_Activate(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0xc8]
        test ecx, ecx
        jz L_41c282
        call NewGamePanel_OnStart
    L_41c282:
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x0041c3b0 UiScreen_Slot_Init0x15_004b4e60 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Init0x15_004b4e60(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x15
        lea ecx, [esi + 0x14c]
        call EditField_ResizeBuffer
        call Setting_Get_004e5d4c
        push eax
        mov ecx, esi
        call EditField_SetText
        mov ecx, esi
        call Widget_SetField36CAndClose
        push 0x1
        mov ecx, esi
        call EditField_SetFocus
        pop esi
        ret
    }
}

// 0x0041c480 ScalarDeletingDtor_0041c480 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0041c480(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Button_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_41c498
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_41c498:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0041c4a0 ScalarDeletingDtor_0041c4a0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0041c4a0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call EditField_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_41c4b8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_41c4b8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0041c4c0 ScalarDeletingDtor_0041c4c0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_0041c4c0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ScrollGroup_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_41c4d8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_41c4d8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004348b0 EditField_Slot_LoadTextAndSelect - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EditField_Slot_LoadTextAndSelect(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call EditField_GetText
        push eax
        mov ecx, esi
        call EditField_SetText
        mov ecx, esi
        call EditField_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        not ecx
        dec ecx
        push ecx
        lea ecx, [esi + 0x14c]
        call EditField_SetCursor
        mov ecx, esi
        call Widget_SetField36CAndClose
        pop edi
        pop esi
        ret
    }
}

// 0x00435160 ListScreen_Slot_SelectNext - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListScreen_Slot_SelectNext(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [ecx + 0xc8]
        call Button_Activate
        mov eax, dword ptr [esi + 0xca00]
        test eax, eax
        jnz L_43517a
        xor edx, edx
        jmp L_435193
    L_43517a:
        mov ecx, dword ptr [esi + 0xca04]
        sub ecx, eax
        mov eax, 0x66666667
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_435193:
        mov eax, dword ptr [esi + 0xca0c]
        inc eax
        js L_4351a8
        cmp eax, edx
        jge L_4351a8
        push eax
        mov ecx, esi
        call SaveLoadDialog_SelectIndex
    L_4351a8:
        pop esi
        ret
    }
}

// 0x004351b0 ListScreen_Slot_SelectPrev - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListScreen_Slot_SelectPrev(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [ecx + 0xc8]
        call Button_Activate
        mov eax, dword ptr [esi + 0xca00]
        test eax, eax
        jnz L_4351ca
        xor edx, edx
        jmp L_4351e3
    L_4351ca:
        mov ecx, dword ptr [esi + 0xca04]
        sub ecx, eax
        mov eax, 0x66666667
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4351e3:
        mov eax, dword ptr [esi + 0xca0c]
        dec eax
        js L_4351f8
        cmp eax, edx
        jge L_4351f8
        push eax
        mov ecx, esi
        call SaveLoadDialog_SelectIndex
    L_4351f8:
        pop esi
        ret
    }
}

// 0x00435220 UiScreen_Slot_Call00435a70IfOwner_ThenActivate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call00435a70IfOwner_ThenActivate(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0xc8]
        test ecx, ecx
        jz L_435232
        call LoadGameScreen_Load
    L_435232:
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x004b3ce0 ScalarDeletingDtor_004b3ce0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b3ce0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ImageWidget_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b3cf8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b3cf8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b3d50 ImageWidget_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ImageWidget_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ImageWidget_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7428
        mov dword ptr [esp + 0x10], 0x0
        call ImageWidget_ReleaseOwnedImage
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0xa10
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004b4620 Caret_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Caret_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_Caret_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x8], esi
        call LineWidget_Ctor
        xor edi, edi
        mov ecx, esi
        push edi
        push -0x1
        push edi
        mov dword ptr [esp + 0x20], edi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7500
        mov dword ptr [esi + 0xe8], edi
        mov dword ptr [esi + 0xec], edi
        mov dword ptr [esi + 0xf0], 0x1
        mov dword ptr [esi + 0xf4], 0xa
        mov dword ptr [esi + 0xf8], edi
        mov dword ptr [esi + 0xfc], 0x3eb33333
        mov dword ptr [esi + 0x104], 0x1
        mov dword ptr [esi + 0x100], edi
        call LineWidget_SetPoint
        mov eax, dword ptr [esi + 0xf0]
        push edi
        push eax
        push 0x1
        mov ecx, esi
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xf0]
        push 0x1
        push ecx
        push 0x2
        mov ecx, esi
        call LineWidget_SetPoint
        push 0x1
        push edi
        push 0x3
        mov ecx, esi
        call LineWidget_SetPoint
        mov edx, dword ptr [esi + 0xf4]
        mov ecx, esi
        dec edx
        push edx
        push edi
        push 0x4
        call LineWidget_SetPoint
        mov eax, dword ptr [esi + 0xf4]
        mov ecx, dword ptr [esi + 0xf0]
        dec eax
        push eax
        push ecx
        push 0x5
        mov ecx, esi
        call LineWidget_SetPoint
        mov edx, dword ptr [esi + 0xf4]
        mov eax, dword ptr [esi + 0xf0]
        push edx
        push eax
        push 0x6
        mov ecx, esi
        call LineWidget_SetPoint
        mov edx, dword ptr [esi + 0xf0]
        mov ecx, dword ptr [esi + 0xf4]
        neg edx
        push ecx
        push edx
        push 0x7
        mov ecx, esi
        call LineWidget_SetPoint
        mov eax, dword ptr [esi + 0xf4]
        mov ecx, dword ptr [esi + 0xf0]
        dec eax
        neg ecx
        push eax
        push ecx
        push 0x8
        mov ecx, esi
        call LineWidget_SetPoint
        mov edx, dword ptr [esi + 0xf4]
        dec edx
        push edx
        push edi
        push 0x9
        mov ecx, esi
        call LineWidget_SetPoint
        push 0x1
        push edi
        push 0xa
        mov ecx, esi
        call LineWidget_SetPoint
        mov eax, dword ptr [esi + 0xf0]
        push 0x1
        neg eax
        push eax
        push 0xb
        mov ecx, esi
        call LineWidget_SetPoint
        mov ecx, dword ptr [esi + 0xf0]
        push edi
        neg ecx
        push ecx
        push 0xc
        mov ecx, esi
        call LineWidget_SetPoint
        mov ecx, dword ptr [esp + 0xc]
        mov eax, esi
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004b49e0 EditField_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EditField_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_EditField_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0xc], esi
        call Button_Ctor
        lea edi, [esi + 0x14c]
        xor ebx, ebx
        push 0x100
        mov ecx, edi
        mov dword ptr [esp + 0x1c], ebx
        call TextBuffer_Ctor
        mov dword ptr [edi + 0x110], ebx
        mov dword ptr [edi], offset g_RData_004cc000 + 0x7608
        lea edi, [esi + 0x260]
        mov byte ptr [esp + 0x18], 0x1
        mov ecx, edi
        call Caret_Ctor
        mov byte ptr [esi + 0x370], BL
        mov byte ptr [esi + 0x371], BL
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7578
        mov dword ptr [esi + 0x36c], 0x1
        mov dword ptr [esi + 0x368], ebx
        mov eax, dword ptr [edi]
        push 0x1
        mov ecx, edi
        mov byte ptr [esp + 0x1c], 0x2
        call dword ptr [eax + 0x60]
        mov dword ptr [esi + 0x25c], esi
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x60]
        mov ecx, dword ptr [esp + 0x10]
        mov eax, esi
        pop edi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004b4a90 ScalarDeletingDtor_004b4a90 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b4a90(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call EditField_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b4aa8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b4aa8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b4ac0 EditField_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EditField_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_EditField_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7578
        push 0x0
        mov dword ptr [esp + 0x14], 0x2
        call EditField_SetFocus
        lea ecx, [esi + 0x14c]
        mov dword ptr [esi + 0x260], offset g_RData_004cc000 + 0xa10
        mov byte ptr [esp + 0x10], 0x0
        call KeyDispatch_Dtor
        mov ecx, esi
        mov dword ptr [esp + 0x10], 0xffffffff
        call Button_Dtor
        mov ecx, dword ptr [esp + 0x8]
        pop esi
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    }
}

// 0x004b4c90 Widget_SetField36CAndClose - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_SetField36CAndClose(int, int)
{
    __asm {
        mov dword ptr [ecx + 0x36c], 0x1
        jmp Button_Activate
    }
}

// 0x004b4ee0 Button_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_Button_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x8
        push ebx
        push ebp
        push esi
        push edi
        xor edi, edi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x18], esi
        call ImageWidget_Ctor
        mov AL, byte ptr [esp + 0x13]
        lea ebx, [esi + 0x10c]
        mov dword ptr [esp + 0x20], edi
        mov byte ptr [ebx], AL
        mov dword ptr [ebx + 0x4], edi
        mov dword ptr [ebx + 0x8], edi
        mov dword ptr [ebx + 0xc], edi
        mov CL, byte ptr [esp + 0x13]
        lea ebp, [esi + 0x11c]
        mov byte ptr [ebp], CL
        mov dword ptr [ebp + 0x4], edi
        mov dword ptr [ebp + 0x8], edi
        mov dword ptr [ebp + 0xc], edi
        mov DL, byte ptr [esp + 0x13]
        mov dword ptr [esi + 0x130], edi
        mov byte ptr [esi + 0x12c], DL
        mov dword ptr [esi + 0x134], edi
        mov dword ptr [esi + 0x138], edi
        mov AL, byte ptr [esp + 0x13]
        mov dword ptr [esi + 0x140], edi
        mov byte ptr [esi + 0x13c], AL
        mov dword ptr [esi + 0x144], edi
        mov dword ptr [esi + 0x148], edi
        mov eax, 0x3f800000
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7630
        mov dword ptr [esi + 0xc4], 0x1
        mov dword ptr [esi + 0xc0], edi
        mov dword ptr [esi + 0xbc], edi
        mov dword ptr [esi + 0xc8], edi
        mov dword ptr [esi + 0xdc], edi
        mov dword ptr [esi + 0xe4], edi
        mov dword ptr [esi + 0xe0], edi
        mov dword ptr [esi + 0xe8], edi
        mov dword ptr [esi + 0xf0], eax
        mov dword ptr [esi + 0xec], edi
        mov dword ptr [esi + 0xf4], edi
        mov dword ptr [esi + 0xf8], edi
        mov dword ptr [esi + 0x100], eax
        mov dword ptr [esi + 0xfc], edi
        mov eax, dword ptr [ebx + 0x8]
        mov edi, dword ptr [ebx + 0x4]
        cmp eax, eax
        mov byte ptr [esp + 0x20], 0x4
        mov ecx, eax
        jz L_4b4ff3
    L_4b4fe5:
        mov edx, dword ptr [ecx]
        add ecx, 0x4
        mov dword ptr [edi], edx
        add edi, 0x4
        cmp ecx, eax
        jnz L_4b4fe5
    L_4b4ff3:
        mov eax, dword ptr [ebx + 0x8]
        mov ecx, ebx
        push eax
        push edi
        call Stub_NoOp_TwoStackArgs
        mov dword ptr [ebx + 0x8], edi
        mov ecx, dword ptr [ebp + 0x8]
        mov edi, dword ptr [ebp + 0x4]
        cmp ecx, ecx
        mov eax, ecx
        jz L_4b501c
    L_4b500e:
        mov edx, dword ptr [eax]
        add eax, 0x4
        mov dword ptr [edi], edx
        add edi, 0x4
        cmp eax, ecx
        jnz L_4b500e
    L_4b501c:
        mov eax, dword ptr [ebp + 0x8]
        mov ecx, ebp
        push eax
        push edi
        call Stub_NoOp_TwoStackArgs
        mov dword ptr [ebp + 0x8], edi
        mov ecx, dword ptr [esi + 0x134]
        mov edi, dword ptr [esi + 0x130]
        cmp ecx, ecx
        mov eax, ecx
        jz L_4b504b
    L_4b503d:
        mov edx, dword ptr [eax]
        add eax, 0x4
        mov dword ptr [edi], edx
        add edi, 0x4
        cmp eax, ecx
        jnz L_4b503d
    L_4b504b:
        mov eax, dword ptr [esi + 0x134]
        lea ebx, [esi + 0x12c]
        push eax
        push edi
        mov ecx, ebx
        call Stub_NoOp_TwoStackArgs
        mov dword ptr [ebx + 0x8], edi
        mov edx, dword ptr [esi]
        mov ecx, esi
        mov word ptr [esi + 0x40], 0x1
        call dword ptr [edx + 0x20]
        xor eax, eax
        mov ecx, dword ptr [esp + 0x18]
        mov AL, byte ptr [esi + 0xc]
        pop edi
        and eax, 0x10
        or AL, 0x2
        mov dword ptr [esi + 0xc], eax
        mov eax, esi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x14
        ret
    }
}

// 0x004b50a0 ScalarDeletingDtor_004b50a0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b50a0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Button_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b50b8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b50b8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b50c0 Button_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_Button_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0xc
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x18], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7630
        mov edi, dword ptr [esi + 0x110]
        mov ebp, dword ptr [esi + 0x114]
        mov AL, byte ptr [esp + 0x13]
        cmp edi, ebp
        mov dword ptr [esp + 0x24], 0x4
        mov byte ptr [esp + 0x14], AL
        mov ebx, edi
        jz L_4b5122
    L_4b510a:
        mov ecx, dword ptr [edi]
        push ecx
        lea ecx, [esp + 0x18]
        call DeleteWidgetPtr
        mov dword ptr [ebx], eax
        add edi, 0x4
        add ebx, 0x4
        cmp edi, ebp
        jnz L_4b510a
    L_4b5122:
        mov edi, dword ptr [esi + 0x120]
        mov ebp, dword ptr [esi + 0x124]
        cmp edi, ebp
        mov ebx, edi
        jz L_4b5150
    L_4b5134:
        mov ecx, dword ptr [edi]
        test ecx, ecx
        jz L_4b5140
        mov edx, dword ptr [ecx]
        push 0x1
        call dword ptr [edx]
    L_4b5140:
        mov dword ptr [ebx], 0x0
        add edi, 0x4
        add ebx, 0x4
        cmp edi, ebp
        jnz L_4b5134
    L_4b5150:
        mov edi, dword ptr [esi + 0x130]
        mov ebp, dword ptr [esi + 0x134]
        cmp edi, ebp
        mov ebx, edi
        jz L_4b517e
    L_4b5162:
        mov ecx, dword ptr [edi]
        test ecx, ecx
        jz L_4b516e
        mov eax, dword ptr [ecx]
        push 0x1
        call dword ptr [eax]
    L_4b516e:
        mov dword ptr [ebx], 0x0
        add edi, 0x4
        add ebx, 0x4
        cmp edi, ebp
        jnz L_4b5162
    L_4b517e:
        mov ecx, dword ptr [esi + 0x114]
        mov edx, dword ptr [esi + 0x110]
        lea edi, [esi + 0x10c]
        push ecx
        push edx
        mov ecx, edi
        call PtrVector_EraseRange
        mov eax, dword ptr [esi + 0x124]
        mov ecx, dword ptr [esi + 0x120]
        lea ebx, [esi + 0x11c]
        push eax
        push ecx
        mov ecx, ebx
        call PtrVector_EraseRange
        mov edx, dword ptr [esi + 0x134]
        mov eax, dword ptr [esi + 0x130]
        lea ebp, [esi + 0x12c]
        push edx
        push eax
        mov ecx, ebp
        call PtrVector_EraseRange
        mov ecx, dword ptr [esi + 0xdc]
        test ecx, ecx
        jz L_4b51e9
        cmp ecx, dword ptr [esi + 0x3c]
        jz L_4b51e9
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0xdc], eax
    L_4b51e9:
        mov ecx, dword ptr [esi + 0xf4]
        test ecx, ecx
        jz L_4b5203
        cmp ecx, dword ptr [esi + 0x3c]
        jz L_4b5203
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0xf4], eax
    L_4b5203:
        mov ecx, dword ptr [esi + 0xe4]
        test ecx, ecx
        jz L_4b521d
        cmp ecx, dword ptr [esi + 0x3c]
        jz L_4b521d
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0xe4], eax
    L_4b521d:
        mov ecx, dword ptr [esi + 0xe0]
        test ecx, ecx
        jz L_4b5237
        cmp ecx, dword ptr [esi + 0x3c]
        jz L_4b5237
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0xe0], eax
    L_4b5237:
        mov ecx, dword ptr [esi + 0x3c]
        test ecx, ecx
        jz L_4b524a
        mov eax, dword ptr [esi + 0x34]
        test eax, eax
        jnz L_4b524a
        call Image_FreeUnlessDefault
    L_4b524a:
        mov eax, dword ptr [esi + 0x140]
        push eax
        mov dword ptr [esp + 0x1c], eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        xor eax, eax
        add esp, 0x4
        mov dword ptr [esi + 0x140], eax
        mov dword ptr [esi + 0x144], eax
        mov dword ptr [esi + 0x148], eax
        mov eax, dword ptr [ebp + 0x4]
        push eax
        mov dword ptr [esp + 0x1c], eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        xor eax, eax
        add esp, 0x4
        mov dword ptr [ebp + 0x4], eax
        mov dword ptr [ebp + 0x8], eax
        mov dword ptr [ebp + 0xc], eax
        mov eax, dword ptr [ebx + 0x4]
        push eax
        mov dword ptr [esp + 0x1c], eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        xor ebp, ebp
        add esp, 0x4
        mov dword ptr [ebx + 0x4], ebp
        mov dword ptr [ebx + 0x8], ebp
        mov dword ptr [ebx + 0xc], ebp
        mov eax, dword ptr [edi + 0x4]
        push eax
        mov dword ptr [esp + 0x1c], eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        mov ecx, esi
        mov dword ptr [edi + 0x4], ebp
        mov dword ptr [edi + 0x8], ebp
        mov dword ptr [edi + 0xc], ebp
        mov dword ptr [esp + 0x24], 0xffffffff
        call ImageWidget_Dtor
        mov ecx, dword ptr [esp + 0x1c]
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x18
        ret
    }
}

// 0x004b5630 Button_Highlight - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_Highlight(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0xe4]
        test eax, eax
        jz L_4b5659
        mov ecx, dword ptr [esi + 0xdc]
        test ecx, ecx
        jnz L_4b5651
        mov ecx, dword ptr [esi + 0x3c]
        mov dword ptr [esi + 0xdc], ecx
    L_4b5651:
        push eax
        mov ecx, esi
        call ImageWidget_SetImageNoOwn
    L_4b5659:
        mov ecx, dword ptr [esi + 0xe8]
        test ecx, ecx
        jz L_4b5675
        mov edx, dword ptr [esi + 0xf0]
        push edx
        call Sound_PlayResourceAuto
        mov dword ptr [esi + 0xec], eax
    L_4b5675:
        mov eax, dword ptr [esi + 0x120]
        mov edi, dword ptr [esi + 0x110]
        test eax, eax
        mov eax, dword ptr [esi + 0x114]
        jz L_4b56f4
        cmp edi, eax
        jz L_4b56a5
    L_4b568f:
        mov ecx, dword ptr [edi]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b568f
    L_4b56a5:
        mov edi, dword ptr [esi + 0x130]
        mov eax, dword ptr [esi + 0x134]
        cmp edi, eax
        jz L_4b56cb
    L_4b56b5:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x134]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b56b5
    L_4b56cb:
        mov edi, dword ptr [esi + 0x120]
        mov eax, dword ptr [esi + 0x124]
        cmp edi, eax
        jz L_4b5734
    L_4b56db:
        mov ecx, dword ptr [edi]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x124]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b56db
        pop edi
        pop esi
        ret
    L_4b56f4:
        cmp edi, eax
        jz L_4b570e
    L_4b56f8:
        mov ecx, dword ptr [edi]
        push 0x1
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b56f8
    L_4b570e:
        mov edi, dword ptr [esi + 0x130]
        mov eax, dword ptr [esi + 0x134]
        cmp edi, eax
        jz L_4b5734
    L_4b571e:
        mov ecx, dword ptr [edi]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x134]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b571e
    L_4b5734:
        pop edi
        pop esi
        ret
    }
}

// 0x004b5900 Button_Activate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_Activate(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call Input_ResetAll
        mov eax, dword ptr [esi + 0xf4]
        test eax, eax
        jz L_4b591b
        push eax
        mov ecx, esi
        call ImageWidget_SetImageNoOwn
    L_4b591b:
        mov ecx, dword ptr [esi + 0xec]
        test ecx, ecx
        jz L_4b5934
        call Sound_StopOrRestoreVoice
        mov dword ptr [esi + 0xec], 0x0
    L_4b5934:
        mov ecx, dword ptr [esi + 0xf8]
        test ecx, ecx
        jz L_4b5950
        mov eax, dword ptr [esi + 0x100]
        push eax
        call Sound_PlayResourceAuto
        mov dword ptr [esi + 0xfc], eax
    L_4b5950:
        mov edi, dword ptr [esi + 0x120]
        mov eax, dword ptr [esi + 0x124]
        cmp edi, eax
        jz L_4b5976
    L_4b5960:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x124]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b5960
    L_4b5976:
        mov edi, dword ptr [esi + 0x130]
        test edi, edi
        jz L_4b59c7
        cmp edi, dword ptr [esi + 0x134]
        jz L_4b599e
    L_4b5988:
        mov ecx, dword ptr [edi]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x134]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b5988
    L_4b599e:
        mov edi, dword ptr [esi + 0x110]
        mov eax, dword ptr [esi + 0x114]
        cmp edi, eax
        jz L_4b59ed
    L_4b59ae:
        mov ecx, dword ptr [edi]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b59ae
        pop edi
        pop esi
        ret
    L_4b59c7:
        mov edi, dword ptr [esi + 0x110]
        mov eax, dword ptr [esi + 0x114]
        cmp edi, eax
        jz L_4b59ed
    L_4b59d7:
        mov ecx, dword ptr [edi]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b59d7
    L_4b59ed:
        pop edi
        pop esi
        ret
    }
}

// 0x004b59f0 Button_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Button_LoadFromConfig(int, int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_Button_LoadFromConfig
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x24
        push esi
        mov esi, dword ptr [esp + 0x3c]
        push edi
        mov edi, ecx
        push 0x1
        mov eax, dword ptr [edi]
        mov dword ptr [edi + 0xc8], esi
        call dword ptr [eax + 0x60]
        push edi
        mov ecx, esi
        call Widget_AppendChild
        mov ecx, dword ptr [esi + 0xa944]
        mov dword ptr [edi + 0xbc], ecx
        mov ecx, dword ptr [esp + 0x3c]
        mov edx, dword ptr [esi + 0xa948]
        test ecx, ecx
        mov dword ptr [edi + 0xc0], edx
        jnz L_4b5a5a
        xor eax, eax
        mov ecx, dword ptr [esp + 0x2c]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        add esp, 0x30
        ret 0x8
    L_4b5a5a:
        push ebp
        push ebx
        mov edx, offset g_Data_004da000 + 0xa710
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b5a92
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov ecx, dword ptr [edi + 0xbc]
        add ecx, edx
        mov dword ptr [edi + 0xbc], ecx
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0x14]
        mov eax, dword ptr [edi + 0xc0]
        add eax, ecx
        mov dword ptr [edi + 0xc0], eax
    L_4b5a92:
        mov ecx, dword ptr [esp + 0x44]
        mov ebx, dword ptr [edi + 0xbc]
        mov ebp, dword ptr [edi + 0xc0]
        mov edx, offset g_Data_004da000 + 0xa708
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b5ad9
        mov edx, dword ptr [esi + 0x4]
        mov ecx, edi
        mov eax, dword ptr [edx + 0xc]
        push eax
        call ImageWidget_SetImage
        mov dword ptr [edi + 0xdc], eax
        mov esi, dword ptr [esi + 0x4]
        cmp dword ptr [esi + 0x4], 0x4
        jl L_4b5ad9
        mov ecx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0x1c]
        add ebx, ecx
        add ebp, eax
    L_4b5ad9:
        mov edx, dword ptr [edi]
        push ebp
        push ebx
        mov ecx, edi
        call dword ptr [edx + 0xc]
        mov eax, dword ptr [esp + 0x48]
        mov esi, dword ptr [eax + 0x118]
        test esi, esi
        jz L_4b5b23
        mov edx, dword ptr [edi]
        mov ecx, edi
        call dword ptr [edx + 0x2c]
        test eax, eax
        jz L_4b5b23
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x24], ecx
        mov ecx, dword ptr [eax + 0x8]
        mov dword ptr [esp + 0x2c], ecx
        mov dword ptr [esp + 0x28], edx
        mov edx, dword ptr [eax + 0xc]
        mov eax, dword ptr [edi]
        lea ecx, [esp + 0x24]
        mov dword ptr [esp + 0x30], edx
        push ecx
        push esi
        mov ecx, edi
        call dword ptr [eax + 0x18]
    L_4b5b23:
        mov ecx, dword ptr [esp + 0x44]
        mov edx, offset g_Data_004da000 + 0xa6fc
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        mov dword ptr [esp + 0x1c], esi
        jz L_4b5fec
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b5b60
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        call Image_Load
        mov dword ptr [edi + 0xe4], eax
    L_4b5b60:
        mov edx, offset g_Data_004da000 + 0x217c
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b5ba2
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x48], 0x3f800000
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        cmp edx, 0x3
        jl L_4b5b8d
        mov eax, dword ptr [eax + 0x14]
        mov dword ptr [esp + 0x48], eax
    L_4b5b8d:
        call Sound_ResolveResourceByName
        mov ecx, dword ptr [esp + 0x48]
        mov dword ptr [edi + 0xe8], eax
        mov dword ptr [edi + 0xf0], ecx
    L_4b5ba2:
        mov edx, offset g_Data_004da000 + 0xa6f4
        mov ecx, esi
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b5f48
        mov eax, dword ptr [ebx + 0x4]
        cmp dword ptr [eax + 0x8], 0x4
        jnz L_4b5da9
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x10], 0x1
        dec eax
        cmp eax, 0x1
        mov dword ptr [esp + 0x18], eax
        jl L_4b5f48
    L_4b5bde:
        mov edx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x18], edx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x0
        jz L_4b5c50
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b5c52
    L_4b5c50:
        xor esi, esi
    L_4b5c52:
        xor eax, eax
        mov dword ptr [esp + 0x48], esi
        mov AL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and eax, 0x10
        or AL, 0x2
        mov dword ptr [esi + 0xc], eax
        mov ecx, dword ptr [esp + 0x48]
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov esi, dword ptr [ecx]
        mov ecx, dword ptr [edx + eax*0x8 + 0x4]
        mov ecx, dword ptr [ecx + 0xc]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [esp + 0x48]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x18]
        add esp, 0x8
        mov eax, dword ptr [eax + ecx*0x8 + 0x4]
        mov ecx, dword ptr [esp + 0x48]
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x14]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [edi + 0xc8]
        mov eax, dword ptr [eax + 0x24]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b5d5b
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov edx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], edx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], edx
        mov dword ptr [eax + 0x2a0], edx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
    L_4b5d5b:
        mov ecx, dword ptr [esp + 0x48]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x124]
        lea ecx, [edi + 0x11c]
        lea edx, [esp + 0x48]
        push edx
        push 0x1
        push eax
        call PtrVector_InsertN
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x10], eax
        jle L_4b5bde
        jmp L_4b5f48
    L_4b5da9:
        mov ecx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x1c], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x1
        jz L_4b5e1b
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b5e1d
    L_4b5e1b:
        xor esi, esi
    L_4b5e1d:
        xor edx, edx
        mov dword ptr [esp + 0x48], esi
        mov DL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [ecx + 0xc]
        mov esi, dword ptr [eax]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [esp + 0x48]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x50]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x18]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b5f11
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov ebx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], ebx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], ebx
        mov dword ptr [eax + 0x2a0], ebx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
        jmp L_4b5f16
    L_4b5f11:
        mov ebx, 0x1
    L_4b5f16:
        mov ecx, dword ptr [esp + 0x48]
        push ebx
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x124]
        lea ecx, [edi + 0x11c]
        lea edx, [esp + 0x48]
        push edx
        push ebx
        push eax
        call PtrVector_InsertN
    L_4b5f48:
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, offset g_Data_004da000 + 0xa6ec
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b5fec
        mov edx, offset g_Data_004da000 + 0xa6e4
        mov ecx, esi
        mov dword ptr [esp + 0x48], 0x0
        xor ebx, ebx
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b5f84
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov dword ptr [esp + 0x48], edx
    L_4b5f84:
        mov edx, offset g_Data_004da000 + 0x1428
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b5fb1
        mov eax, dword ptr [eax + 0x4]
        xor ebx, ebx
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [eax + 0x14]
        mov eax, dword ptr [eax + 0x1c]
        and ecx, 0xff
        mov BH, AL
        mov BL, DL
        shl ebx, 0x8
        or ebx, ecx
    L_4b5fb1:
        fld dword ptr [esp + 0x48]
        fcomp dword ptr [g_RData_004cc000 + 0x74d4]
        fnstsw AX
        test AH, 0x40
        jnz L_4b5fec
        mov esi, dword ptr [edi + 0x120]
        mov eax, dword ptr [edi + 0x124]
        cmp esi, eax
        jz L_4b5fec
        mov ebp, dword ptr [esp + 0x48]
    L_4b5fd6:
        mov ecx, dword ptr [esi]
        push ebp
        push ebx
        call ListLabel_StartColourAnim
        mov eax, dword ptr [edi + 0x124]
        add esi, 0x4
        cmp esi, eax
        jnz L_4b5fd6
    L_4b5fec:
        mov ecx, dword ptr [esp + 0x44]
        mov edx, offset g_Data_004da000 + 0xa6dc
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        mov dword ptr [esp + 0x14], esi
        jz L_4b64b6
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b6029
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        call Image_Load
        mov dword ptr [edi + 0xe0], eax
    L_4b6029:
        mov edx, offset g_Data_004da000 + 0x217c
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b606b
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x48], 0x3f800000
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        cmp edx, 0x3
        jl L_4b6056
        mov edx, dword ptr [eax + 0x14]
        mov dword ptr [esp + 0x48], edx
    L_4b6056:
        call Sound_ResolveResourceByName
        mov dword ptr [edi + 0x104], eax
        mov eax, dword ptr [esp + 0x48]
        mov dword ptr [edi + 0x108], eax
    L_4b606b:
        mov edx, offset g_Data_004da000 + 0xa6f4
        mov ecx, esi
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b6412
        mov eax, dword ptr [ebx + 0x4]
        cmp dword ptr [eax + 0x8], 0x4
        jnz L_4b6273
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x10], 0x1
        dec eax
        cmp eax, 0x1
        mov dword ptr [esp + 0x18], eax
        jl L_4b6412
    L_4b60a7:
        mov ecx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x20], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x2
        jz L_4b6119
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b611b
    L_4b6119:
        xor esi, esi
    L_4b611b:
        xor edx, edx
        mov dword ptr [esp + 0x48], esi
        mov DL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov esi, dword ptr [eax]
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        call Call004a5bf0_If004a5b20
        mov ecx, dword ptr [esp + 0x48]
        push eax
        push ecx
        call dword ptr [esi + 0x74]
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x18]
        mov ecx, dword ptr [esp + 0x50]
        add esp, 0x8
        mov eax, dword ptr [edx + eax*0x8 + 0x4]
        mov edx, dword ptr [ecx]
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x1c]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [edi + 0xc8]
        mov eax, dword ptr [eax + 0x24]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b6225
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov edx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], edx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], edx
        mov dword ptr [eax + 0x2a0], edx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
    L_4b6225:
        mov ecx, dword ptr [esp + 0x48]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x144]
        lea ecx, [edi + 0x13c]
        lea edx, [esp + 0x48]
        push edx
        push 0x1
        push eax
        call PtrVector_InsertN
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x10], eax
        jle L_4b60a7
        jmp L_4b6412
    L_4b6273:
        mov ecx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x20], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x3
        jz L_4b62e5
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b62e7
    L_4b62e5:
        xor esi, esi
    L_4b62e7:
        xor edx, edx
        mov dword ptr [esp + 0x48], esi
        mov DL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [ecx + 0xc]
        mov esi, dword ptr [eax]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [esp + 0x48]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x50]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x1c]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b63db
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov ebx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], ebx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], ebx
        mov dword ptr [eax + 0x2a0], ebx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
        jmp L_4b63e0
    L_4b63db:
        mov ebx, 0x1
    L_4b63e0:
        mov ecx, dword ptr [esp + 0x48]
        push ebx
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x144]
        lea ecx, [edi + 0x13c]
        lea edx, [esp + 0x48]
        push edx
        push ebx
        push eax
        call PtrVector_InsertN
    L_4b6412:
        mov ecx, dword ptr [esp + 0x14]
        mov edx, offset g_Data_004da000 + 0xa6ec
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b64b6
        mov edx, offset g_Data_004da000 + 0xa6e4
        mov ecx, esi
        mov dword ptr [esp + 0x48], 0x0
        xor ebx, ebx
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b644e
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov dword ptr [esp + 0x48], edx
    L_4b644e:
        mov edx, offset g_Data_004da000 + 0x1428
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b647b
        mov eax, dword ptr [eax + 0x4]
        xor ebx, ebx
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [eax + 0x14]
        mov eax, dword ptr [eax + 0x1c]
        and ecx, 0xff
        mov BH, AL
        mov BL, DL
        shl ebx, 0x8
        or ebx, ecx
    L_4b647b:
        fld dword ptr [esp + 0x48]
        fcomp dword ptr [g_RData_004cc000 + 0x74d4]
        fnstsw AX
        test AH, 0x40
        jnz L_4b64b6
        mov esi, dword ptr [edi + 0x140]
        mov eax, dword ptr [edi + 0x144]
        cmp esi, eax
        jz L_4b64b6
        mov ebp, dword ptr [esp + 0x48]
    L_4b64a0:
        mov ecx, dword ptr [esi]
        push ebp
        push ebx
        call ListLabel_StartColourAnim
        mov eax, dword ptr [edi + 0x144]
        add esi, 0x4
        cmp esi, eax
        jnz L_4b64a0
    L_4b64b6:
        mov ecx, dword ptr [esp + 0x44]
        mov edx, offset g_Data_004da000 + 0xa6d0
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [esp + 0x14], eax
        jz L_4b6980
        mov esi, eax
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b64f3
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        call Image_Load
        mov dword ptr [edi + 0xf4], eax
    L_4b64f3:
        mov edx, offset g_Data_004da000 + 0x217c
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b6535
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x48], 0x3f800000
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        cmp edx, 0x3
        jl L_4b6520
        mov edx, dword ptr [eax + 0x14]
        mov dword ptr [esp + 0x48], edx
    L_4b6520:
        call Sound_ResolveResourceByName
        mov dword ptr [edi + 0xf8], eax
        mov eax, dword ptr [esp + 0x48]
        mov dword ptr [edi + 0x100], eax
    L_4b6535:
        mov edx, offset g_Data_004da000 + 0xa6f4
        mov ecx, esi
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b68dc
        mov eax, dword ptr [ebx + 0x4]
        cmp dword ptr [eax + 0x8], 0x4
        jnz L_4b673d
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x10], 0x1
        dec eax
        cmp eax, 0x1
        mov dword ptr [esp + 0x18], eax
        jl L_4b68dc
    L_4b6571:
        mov ecx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x20], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x4
        jz L_4b65e3
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b65e5
    L_4b65e3:
        xor esi, esi
    L_4b65e5:
        xor edx, edx
        mov dword ptr [esp + 0x48], esi
        mov DL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov esi, dword ptr [eax]
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        call Call004a5bf0_If004a5b20
        mov ecx, dword ptr [esp + 0x48]
        push eax
        push ecx
        call dword ptr [esi + 0x74]
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x18]
        mov ecx, dword ptr [esp + 0x50]
        add esp, 0x8
        mov eax, dword ptr [edx + eax*0x8 + 0x4]
        mov edx, dword ptr [ecx]
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x1c]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [edi + 0xc8]
        mov eax, dword ptr [eax + 0x24]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b66ef
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov edx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], edx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], edx
        mov dword ptr [eax + 0x2a0], edx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
    L_4b66ef:
        mov ecx, dword ptr [esp + 0x48]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x134]
        lea ecx, [edi + 0x12c]
        lea edx, [esp + 0x48]
        push edx
        push 0x1
        push eax
        call PtrVector_InsertN
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x10], eax
        jle L_4b6571
        jmp L_4b68dc
    L_4b673d:
        mov ecx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x20], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x5
        jz L_4b67af
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b67b1
    L_4b67af:
        xor esi, esi
    L_4b67b1:
        xor edx, edx
        mov dword ptr [esp + 0x48], esi
        mov DL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [ecx + 0xc]
        mov esi, dword ptr [eax]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [esp + 0x48]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x50]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x1c]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b68a5
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov ebx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], ebx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], ebx
        mov dword ptr [eax + 0x2a0], ebx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
        jmp L_4b68aa
    L_4b68a5:
        mov ebx, 0x1
    L_4b68aa:
        mov ecx, dword ptr [esp + 0x48]
        push ebx
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x134]
        lea ecx, [edi + 0x12c]
        lea edx, [esp + 0x48]
        push edx
        push ebx
        push eax
        call PtrVector_InsertN
    L_4b68dc:
        mov ecx, dword ptr [esp + 0x14]
        mov edx, offset g_Data_004da000 + 0xa6ec
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b6980
        mov edx, offset g_Data_004da000 + 0xa6e4
        mov ecx, esi
        mov dword ptr [esp + 0x48], 0x0
        xor ebx, ebx
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b6918
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov dword ptr [esp + 0x48], edx
    L_4b6918:
        mov edx, offset g_Data_004da000 + 0x1428
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b6945
        mov eax, dword ptr [eax + 0x4]
        xor ebx, ebx
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [eax + 0x14]
        mov eax, dword ptr [eax + 0x1c]
        and ecx, 0xff
        mov BH, AL
        mov BL, DL
        shl ebx, 0x8
        or ebx, ecx
    L_4b6945:
        fld dword ptr [esp + 0x48]
        fcomp dword ptr [g_RData_004cc000 + 0x74d4]
        fnstsw AX
        test AH, 0x40
        jnz L_4b6980
        mov esi, dword ptr [edi + 0x130]
        mov eax, dword ptr [edi + 0x134]
        cmp esi, eax
        jz L_4b6980
        mov ebp, dword ptr [esp + 0x48]
    L_4b696a:
        mov ecx, dword ptr [esi]
        push ebp
        push ebx
        call ListLabel_StartColourAnim
        mov eax, dword ptr [edi + 0x134]
        add esi, 0x4
        cmp esi, eax
        jnz L_4b696a
    L_4b6980:
        mov ecx, dword ptr [esp + 0x44]
        mov edx, offset g_Data_004da000 + 0xa6f4
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b6efd
        mov eax, dword ptr [ebx + 0x4]
        cmp dword ptr [eax + 0x8], 0x4
        jnz L_4b6b8a
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x10], 0x1
        dec eax
        cmp eax, 0x1
        mov dword ptr [esp + 0x18], eax
        jl L_4b6efd
    L_4b69be:
        mov eax, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x20], eax
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x6
        jz L_4b6a30
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b6a32
    L_4b6a30:
        xor esi, esi
    L_4b6a32:
        xor ecx, ecx
        mov dword ptr [esp + 0x48], esi
        mov CL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and ecx, 0x10
        or ecx, 0x2
        mov dword ptr [esi + 0xc], ecx
        mov edx, dword ptr [esp + 0x48]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov esi, dword ptr [edx]
        mov edx, dword ptr [eax + ecx*0x8 + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        call Call004a5bf0_If004a5b20
        push eax
        mov eax, dword ptr [esp + 0x4c]
        push eax
        call dword ptr [esi + 0x74]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x18]
        add esp, 0x8
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [esp + 0x48]
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x1c]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [edi + 0xc8]
        mov eax, dword ptr [eax + 0x24]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b6b3c
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov edx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], edx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], edx
        mov dword ptr [eax + 0x2a0], edx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
    L_4b6b3c:
        mov ecx, dword ptr [esp + 0x48]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x114]
        lea ecx, [edi + 0x10c]
        lea edx, [esp + 0x48]
        push edx
        push 0x1
        push eax
        call PtrVector_InsertN
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x10], eax
        jle L_4b69be
        jmp L_4b6efd
    L_4b6b8a:
        mov ecx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x20], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], esi
        test esi, esi
        mov dword ptr [esp + 0x3c], 0x7
        jz L_4b6bfc
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b6bfe
    L_4b6bfc:
        xor esi, esi
    L_4b6bfe:
        xor edx, edx
        mov dword ptr [esp + 0x48], esi
        mov DL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x3c], 0xffffffff
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [ecx + 0xc]
        mov esi, dword ptr [eax]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [esp + 0x48]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x50]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x1c]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b6cf0
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x50]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0xc]
        mov edx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], edx
        mov eax, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], edx
        mov dword ptr [eax + 0x2a0], edx
        mov ecx, dword ptr [esp + 0x48]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
    L_4b6cf0:
        mov ecx, dword ptr [esp + 0x48]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov esi, dword ptr [edi + 0x114]
        mov edx, dword ptr [edi + 0x118]
        sub edx, esi
        mov ebp, esi
        sar edx, 0x2
        cmp edx, 0x1
        jnc L_4b6e38
        mov ecx, dword ptr [edi + 0x110]
        test ecx, ecx
        jnz L_4b6d35
        xor eax, eax
        jmp L_4b6d3c
    L_4b6d35:
        mov eax, esi
        sub eax, ecx
        sar eax, 0x2
    L_4b6d3c:
        cmp eax, 0x1
        jbe L_4b6d52
        test ecx, ecx
        jnz L_4b6d49
        xor eax, eax
        jmp L_4b6d57
    L_4b6d49:
        mov eax, esi
        sub eax, ecx
        sar eax, 0x2
        jmp L_4b6d57
    L_4b6d52:
        mov eax, 0x1
    L_4b6d57:
        test ecx, ecx
        jnz L_4b6d5f
        xor esi, esi
        jmp L_4b6d64
    L_4b6d5f:
        sub esi, ecx
        sar esi, 0x2
    L_4b6d64:
        add eax, esi
        test eax, eax
        mov dword ptr [esp + 0x18], eax
        jge L_4b6d70
        xor eax, eax
    L_4b6d70:
        shl eax, 0x2
        push eax
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, dword ptr [edi + 0x110]
        add esp, 0x4
        cmp esi, ebp
        mov dword ptr [esp + 0x1c], eax
        mov ebx, eax
        jz L_4b6d9f
    L_4b6d8c:
        mov edx, esi
        mov ecx, ebx
        call Value_AssignDword
        add esi, 0x4
        add ebx, 0x4
        cmp esi, ebp
        jnz L_4b6d8c
    L_4b6d9f:
        mov esi, ebx
        mov dword ptr [esp + 0x14], 0x1
    L_4b6da9:
        lea edx, [esp + 0x48]
        mov ecx, esi
        call Value_AssignDword
        mov eax, dword ptr [esp + 0x14]
        add esi, 0x4
        dec eax
        mov dword ptr [esp + 0x14], eax
        jnz L_4b6da9
        lea eax, [ebx + 0x4]
        mov ebx, dword ptr [edi + 0x114]
        cmp ebp, ebx
        mov esi, ebp
        jz L_4b6de6
        mov ebp, eax
    L_4b6dd3:
        mov edx, esi
        mov ecx, ebp
        call Value_AssignDword
        add esi, 0x4
        add ebp, 0x4
        cmp esi, ebx
        jnz L_4b6dd3
    L_4b6de6:
        mov eax, dword ptr [edi + 0x110]
        push eax
        mov dword ptr [esp + 0x24], eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        mov ecx, dword ptr [esp + 0x20]
        mov edx, dword ptr [esp + 0x1c]
        add esp, 0x4
        lea eax, [ecx + edx*0x4]
        mov edx, dword ptr [edi + 0x110]
        test edx, edx
        mov dword ptr [edi + 0x118], eax
        jnz L_4b6e18
        xor eax, eax
        jmp L_4b6e23
    L_4b6e18:
        mov eax, dword ptr [edi + 0x114]
        sub eax, edx
        sar eax, 0x2
    L_4b6e23:
        lea edx, [ecx + eax*0x4 + 0x4]
        mov dword ptr [edi + 0x110], ecx
        mov dword ptr [edi + 0x114], edx
        jmp L_4b6efd
    L_4b6e38:
        mov eax, esi
        sub eax, ebp
        sar eax, 0x2
        cmp eax, 0x1
        jnc L_4b6e9d
        cmp ebp, esi
        mov edx, ebp
        jz L_4b6e5a
    L_4b6e4a:
        lea ebx, [edx + 0x4]
        mov ecx, ebx
        call Value_AssignDword
        mov edx, ebx
        cmp edx, esi
        jnz L_4b6e4a
    L_4b6e5a:
        mov esi, dword ptr [edi + 0x114]
        mov eax, 0x1
        mov ecx, esi
        sub ecx, ebp
        sar ecx, 0x2
        sub eax, ecx
        jz L_4b6e83
        mov ebx, eax
    L_4b6e72:
        lea edx, [esp + 0x48]
        mov ecx, esi
        call Value_AssignDword
        add esi, 0x4
        dec ebx
        jnz L_4b6e72
    L_4b6e83:
        mov eax, dword ptr [edi + 0x114]
        cmp ebp, eax
        jz L_4b6ef6
    L_4b6e8d:
        mov edx, dword ptr [esp + 0x48]
        mov dword ptr [ebp], edx
        add ebp, 0x4
        cmp ebp, eax
        jnz L_4b6e8d
        jmp L_4b6ef6
    L_4b6e9d:
        lea ebx, [esi - 0x4]
        mov dword ptr [esp + 0x10], esi
        cmp ebx, esi
        jz L_4b6ec5
    L_4b6ea8:
        mov ecx, dword ptr [esp + 0x10]
        mov edx, ebx
        call Value_AssignDword
        mov edx, dword ptr [esp + 0x10]
        add ebx, 0x4
        add edx, 0x4
        cmp ebx, esi
        mov dword ptr [esp + 0x10], edx
        jnz L_4b6ea8
    L_4b6ec5:
        mov ecx, dword ptr [edi + 0x114]
        lea eax, [ecx - 0x4]
        cmp ebp, eax
        jz L_4b6ee1
    L_4b6ed2:
        mov edx, dword ptr [eax - 0x4]
        sub eax, 0x4
        sub ecx, 0x4
        cmp eax, ebp
        mov dword ptr [ecx], edx
        jnz L_4b6ed2
    L_4b6ee1:
        lea eax, [ebp + 0x4]
        cmp ebp, eax
        jz L_4b6ef6
    L_4b6ee8:
        mov ecx, dword ptr [esp + 0x48]
        mov dword ptr [ebp], ecx
        add ebp, 0x4
        cmp ebp, eax
        jnz L_4b6ee8
    L_4b6ef6:
        add dword ptr [edi + 0x114], 0x4
    L_4b6efd:
        mov ecx, dword ptr [esp + 0x44]
        mov edx, offset g_Data_004da000 + 0xa6ec
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b6fa1
        mov edx, offset g_Data_004da000 + 0xa6e4
        mov ecx, esi
        mov dword ptr [esp + 0x44], 0x0
        xor ebx, ebx
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b6f39
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0xc]
        mov dword ptr [esp + 0x44], eax
    L_4b6f39:
        mov edx, offset g_Data_004da000 + 0x1428
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b6f66
        mov eax, dword ptr [eax + 0x4]
        xor ebx, ebx
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [eax + 0x14]
        mov eax, dword ptr [eax + 0x1c]
        and ecx, 0xff
        mov BH, AL
        mov BL, DL
        shl ebx, 0x8
        or ebx, ecx
    L_4b6f66:
        fld dword ptr [esp + 0x44]
        fcomp dword ptr [g_RData_004cc000 + 0x74d4]
        fnstsw AX
        test AH, 0x40
        jnz L_4b6fa1
        mov esi, dword ptr [edi + 0x110]
        mov eax, dword ptr [edi + 0x114]
        cmp esi, eax
        jz L_4b6fa1
        mov ebp, dword ptr [esp + 0x44]
    L_4b6f8b:
        mov ecx, dword ptr [esi]
        push ebp
        push ebx
        call ListLabel_StartColourAnim
        mov eax, dword ptr [edi + 0x114]
        add esi, 0x4
        cmp esi, eax
        jnz L_4b6f8b
    L_4b6fa1:
        mov ecx, dword ptr [esp + 0x34]
        pop ebx
        pop ebp
        pop edi
        mov eax, 0x1
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x30
        ret 0x8
    }
}

// 0x004b6fc0 Toggle_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Toggle_Ctor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Button_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x76b8
        mov dword ptr [esi + 0x14c], eax
        mov dword ptr [esi + 0x158], eax
        mov dword ptr [esi + 0x15c], eax
        mov dword ptr [esi + 0x160], eax
        mov dword ptr [esi + 0x150], eax
        mov dword ptr [esi + 0x154], eax
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004b7000 ScalarDeletingDtor_004b7000 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b7000(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Toggle_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b7018
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b7018:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b7020 Toggle_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Toggle_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_Toggle_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x76b8
        mov eax, dword ptr [esi + 0x158]
        mov dword ptr [esp + 0x10], 0x0
        push eax
        call ImageWidget_SetImageNoOwn
        mov eax, dword ptr [esi + 0x15c]
        test eax, eax
        jz L_4b7074
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        mov dword ptr [esi + 0x15c], 0x0
    L_4b7074:
        mov ecx, dword ptr [esi + 0x160]
        test ecx, ecx
        jz L_4b708e
        mov edx, dword ptr [ecx]
        push 0x1
        call dword ptr [edx]
        mov dword ptr [esi + 0x160], 0x0
    L_4b708e:
        mov ecx, esi
        mov dword ptr [esp + 0x10], 0xffffffff
        call Button_Dtor
        mov ecx, dword ptr [esp + 0x8]
        pop esi
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    }
}

// 0x004b7210 Button_OnHover - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Button_OnHover(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc4]
        test eax, eax
        jz L_4b724a
        mov eax, dword ptr [esi + 0x14c]
        test eax, eax
        jnz L_4b724a
        mov ecx, dword ptr [esi + 0xe8]
        test ecx, ecx
        jz L_4b7243
        mov eax, dword ptr [esi + 0xf0]
        push eax
        call Sound_PlayResourceAuto
        mov dword ptr [esi + 0xec], eax
    L_4b7243:
        mov ecx, esi
        call Button_Highlight
    L_4b724a:
        pop esi
        ret
    }
}


// 0x004b7340 Button_LoadStatesFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Button_LoadStatesFromConfig(int, int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_Button_LoadStatesFromConfig
        push eax
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr FS:[0x0], esp
        sub esp, 0x10
        push ebx
        push ebp
        push esi
        mov esi, dword ptr [esp + 0x2c]
        push edi
        push eax
        mov edi, ecx
        push esi
        call Button_LoadFromConfig
        mov ecx, dword ptr [edi + 0x3c]
        mov edx, offset g_Data_004da000 + 0xa740
        mov dword ptr [edi + 0x158], ecx
        mov ecx, esi
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b751f
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b73ad
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        call Image_Load
        mov dword ptr [edi + 0x15c], eax
    L_4b73ad:
        mov edx, offset g_Data_004da000 + 0xa738
        mov ecx, esi
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b751f
        mov eax, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x38], eax
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x18], esi
        test esi, esi
        mov dword ptr [esp + 0x28], 0x0
        jz L_4b7435
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b7437
    L_4b7435:
        xor esi, esi
    L_4b7437:
        mov dword ptr [edi + 0x160], esi
        mov ecx, dword ptr [ebx + 0x4]
        mov esi, dword ptr [esi]
        mov dword ptr [esp + 0x28], 0xffffffff
        mov ecx, dword ptr [ecx + 0xc]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [edi + 0x160]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [edi + 0x160]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x34]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b7500
        mov edx, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [edi + 0x160]
        push 0x2
        push 0x0
        mov eax, dword ptr [ecx]
        push 0x0
        push 0x0
        push edx
        mov edx, dword ptr [esi + 0x8]
        push edx
        mov edx, dword ptr [esi + 0x4]
        push edx
        call dword ptr [eax + 0x80]
        mov eax, dword ptr [edi + 0x160]
        mov ecx, dword ptr [esi + 0xc]
        mov dword ptr [eax + 0x14c], ecx
        mov dword ptr [eax + 0x150], ecx
        mov ecx, 0x1
        mov dword ptr [eax + 0x270], ecx
        mov eax, dword ptr [edi + 0x160]
        mov edx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], edx
        mov dword ptr [eax + 0x29c], ecx
        mov dword ptr [eax + 0x2a0], ecx
    L_4b7500:
        mov ecx, dword ptr [edi + 0x160]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [edi + 0x160]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
    L_4b751f:
        mov ecx, dword ptr [esp + 0x30]
        mov edx, offset g_Data_004da000 + 0xa728
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b7a77
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b7558
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        call Image_Load
        mov dword ptr [edi + 0x154], eax
    L_4b7558:
        mov edx, offset g_Data_004da000 + 0xa738
        mov ecx, esi
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b76ca
        mov eax, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x38], eax
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x18], esi
        test esi, esi
        mov dword ptr [esp + 0x28], 0x1
        jz L_4b75e0
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b75e2
    L_4b75e0:
        xor esi, esi
    L_4b75e2:
        mov dword ptr [edi + 0x160], esi
        mov ecx, dword ptr [ebx + 0x4]
        mov esi, dword ptr [esi]
        mov dword ptr [esp + 0x28], 0xffffffff
        mov ecx, dword ptr [ecx + 0xc]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [edi + 0x160]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [edi + 0x160]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x34]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b76ab
        mov edx, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [edi + 0x160]
        push 0x2
        push 0x0
        mov eax, dword ptr [ecx]
        push 0x0
        push 0x0
        push edx
        mov edx, dword ptr [esi + 0x8]
        push edx
        mov edx, dword ptr [esi + 0x4]
        push edx
        call dword ptr [eax + 0x80]
        mov eax, dword ptr [edi + 0x160]
        mov ecx, dword ptr [esi + 0xc]
        mov dword ptr [eax + 0x14c], ecx
        mov dword ptr [eax + 0x150], ecx
        mov ecx, 0x1
        mov dword ptr [eax + 0x270], ecx
        mov eax, dword ptr [edi + 0x160]
        mov edx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], edx
        mov dword ptr [eax + 0x29c], ecx
        mov dword ptr [eax + 0x2a0], ecx
    L_4b76ab:
        mov ecx, dword ptr [edi + 0x160]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [edi + 0x160]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
    L_4b76ca:
        mov ecx, dword ptr [esp + 0x30]
        mov edx, offset g_Data_004da000 + 0xa6f4
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b7a77
        mov eax, dword ptr [ebx + 0x4]
        cmp dword ptr [eax + 0x8], 0x4
        jnz L_4b78d8
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x10], 0x1
        dec eax
        cmp eax, 0x1
        mov dword ptr [esp + 0x18], eax
        jl L_4b7a77
    L_4b7708:
        mov edx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x18], edx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x1c], esi
        test esi, esi
        mov dword ptr [esp + 0x28], 0x2
        jz L_4b777a
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b777c
    L_4b777a:
        xor esi, esi
    L_4b777c:
        xor eax, eax
        mov dword ptr [esp + 0x34], esi
        mov AL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x28], 0xffffffff
        and eax, 0x10
        or AL, 0x2
        mov dword ptr [esi + 0xc], eax
        mov ecx, dword ptr [esp + 0x34]
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov esi, dword ptr [ecx]
        mov ecx, dword ptr [edx + eax*0x8 + 0x4]
        mov ecx, dword ptr [ecx + 0xc]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [esp + 0x34]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x18]
        add esp, 0x8
        mov eax, dword ptr [eax + ecx*0x8 + 0x4]
        mov ecx, dword ptr [esp + 0x34]
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x14]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx + edx*0x8 + 0x4]
        mov ecx, dword ptr [edi + 0xc8]
        mov eax, dword ptr [eax + 0x24]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b7887
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x3c]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esi + 0xc]
        mov ebp, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], ebp
        mov eax, dword ptr [esp + 0x34]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], ebp
        mov dword ptr [eax + 0x2a0], ebp
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
        jmp L_4b788c
    L_4b7887:
        mov ebp, 0x1
    L_4b788c:
        mov ecx, dword ptr [esp + 0x34]
        push ebp
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x34]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x144]
        lea ecx, [edi + 0x13c]
        lea edx, [esp + 0x34]
        push edx
        push ebp
        push eax
        call PtrVector_InsertN
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x10], eax
        jle L_4b7708
        jmp L_4b7a77
    L_4b78d8:
        mov ecx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x1c], ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x1c], esi
        test esi, esi
        mov dword ptr [esp + 0x28], 0x3
        jz L_4b794a
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b794c
    L_4b794a:
        xor esi, esi
    L_4b794c:
        xor edx, edx
        mov dword ptr [esp + 0x34], esi
        mov DL, byte ptr [esi + 0xc]
        mov dword ptr [esp + 0x28], 0xffffffff
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x34]
        mov ecx, dword ptr [ecx + 0xc]
        mov esi, dword ptr [eax]
        call Call004a5bf0_If004a5b20
        mov edx, dword ptr [esp + 0x34]
        push eax
        push edx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x3c]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x18]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b7a40
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esi + 0x20]
        push 0x2
        push 0x0
        mov dword ptr [ecx + 0x144], eax
        mov eax, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [esp + 0x3c]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esi + 0xc]
        mov ebx, 0x1
        mov dword ptr [ecx + 0x14c], eax
        mov dword ptr [ecx + 0x150], eax
        mov dword ptr [ecx + 0x270], ebx
        mov eax, dword ptr [esp + 0x34]
        mov ecx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], ecx
        mov dword ptr [eax + 0x29c], ebx
        mov dword ptr [eax + 0x2a0], ebx
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [ecx + 0x268], edx
        mov dword ptr [ecx + 0x26c], eax
        jmp L_4b7a45
    L_4b7a40:
        mov ebx, 0x1
    L_4b7a45:
        mov ecx, dword ptr [esp + 0x34]
        push ebx
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x34]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
        mov eax, dword ptr [edi + 0x144]
        lea ecx, [edi + 0x13c]
        lea edx, [esp + 0x34]
        push edx
        push ebx
        push eax
        call PtrVector_InsertN
    L_4b7a77:
        mov ecx, dword ptr [esp + 0x30]
        mov edx, offset g_Data_004da000 + 0xa71c
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b7c22
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b7ab0
        mov ecx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [ecx + 0xc]
        call Image_Load
        mov dword ptr [edi + 0x150], eax
    L_4b7ab0:
        mov edx, offset g_Data_004da000 + 0xa738
        mov ecx, esi
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b7c22
        mov edx, dword ptr [edi + 0xc0]
        mov ebp, dword ptr [edi + 0xbc]
        push 0x2c0
        mov dword ptr [esp + 0x34], edx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x1c], esi
        test esi, esi
        mov dword ptr [esp + 0x28], 0x4
        jz L_4b7b38
        push 0x0
        push 0x0
        push 0x0
        mov ecx, esi
        call ListLabel_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2a4], eax
        mov dword ptr [esi + 0x2ac], eax
        mov dword ptr [esi + 0x2b4], eax
        mov dword ptr [esi + 0x2b8], eax
        mov dword ptr [esi + 0x2bc], 0x1
        jmp L_4b7b3a
    L_4b7b38:
        xor esi, esi
    L_4b7b3a:
        mov dword ptr [edi + 0x160], esi
        mov eax, dword ptr [ebx + 0x4]
        mov esi, dword ptr [esi]
        mov dword ptr [esp + 0x28], 0xffffffff
        mov ecx, dword ptr [eax + 0xc]
        call Call004a5bf0_If004a5b20
        mov ecx, dword ptr [edi + 0x160]
        push eax
        push ecx
        call dword ptr [esi + 0x74]
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [edi + 0x160]
        add esp, 0x8
        mov esi, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add esi, dword ptr [esp + 0x30]
        mov edx, dword ptr [ecx]
        add eax, ebp
        push esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        mov ecx, dword ptr [edi + 0xc8]
        lea edx, [eax + eax*0x8]
        mov esi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg esi
        sbb esi, esi
        and esi, eax
        test esi, esi
        jz L_4b7c03
        mov edx, dword ptr [esi + 0x1c]
        mov ecx, dword ptr [edi + 0x160]
        push 0x2
        push 0x0
        mov eax, dword ptr [ecx]
        push 0x0
        push 0x0
        push edx
        mov edx, dword ptr [esi + 0x8]
        push edx
        mov edx, dword ptr [esi + 0x4]
        push edx
        call dword ptr [eax + 0x80]
        mov eax, dword ptr [edi + 0x160]
        mov ecx, dword ptr [esi + 0xc]
        mov dword ptr [eax + 0x14c], ecx
        mov dword ptr [eax + 0x150], ecx
        mov ecx, 0x1
        mov dword ptr [eax + 0x270], ecx
        mov eax, dword ptr [edi + 0x160]
        mov edx, dword ptr [esi + 0x18]
        mov dword ptr [eax + 0x264], edx
        mov dword ptr [eax + 0x29c], ecx
        mov dword ptr [eax + 0x2a0], ecx
    L_4b7c03:
        mov ecx, dword ptr [edi + 0x160]
        push 0x0
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [edi + 0x160]
        push ecx
        mov ecx, dword ptr [edi + 0xc8]
        call Widget_AppendChild
    L_4b7c22:
        mov eax, dword ptr [edi + 0x158]
        test eax, eax
        jz L_4b7c5b
        mov ecx, dword ptr [edi + 0x18]
        mov edx, dword ptr [edi + 0x14]
        mov dword ptr [edi + 0xd0], ecx
        mov dword ptr [edi + 0xcc], edx
        movsx esi, word ptr [eax + 0x6]
        add esi, ecx
        mov dword ptr [edi + 0xd8], esi
        movsx eax, word ptr [eax + 0x4]
        add eax, edx
        mov dword ptr [edi + 0xd4], eax
        jmp L_4b7d3d
    L_4b7c5b:
        mov eax, dword ptr [edi + 0x110]
        test eax, eax
        jz L_4b7d3d
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x68]
        mov dword ptr [edi + 0xd0], eax
        mov eax, dword ptr [edi + 0x110]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        mov dword ptr [edi + 0xcc], eax
        mov eax, dword ptr [edi + 0x110]
        mov ecx, dword ptr [eax]
        call ListLabel_GetLineHeight
        add eax, dword ptr [edi + 0xd0]
        mov dword ptr [edi + 0xd8], eax
        mov esi, dword ptr [edi + 0x110]
        cmp esi, dword ptr [edi + 0x114]
        jz L_4b7d2a
    L_4b7cb0:
        mov ecx, dword ptr [esi]
        call ListLabel_GetLineHeight
        mov ecx, dword ptr [edi + 0xd8]
        add ecx, eax
        mov dword ptr [edi + 0xd8], ecx
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b7cdb
        mov edx, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [edx + 0x90]
    L_4b7cdb:
        mov ecx, dword ptr [ebx + 0x25c]
        mov ebx, dword ptr [edi + 0xcc]
        mov eax, dword ptr [edi + 0xd4]
        add ecx, ebx
        cmp ecx, eax
        jle L_4b7d17
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b7d09
        mov edx, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [edx + 0x90]
    L_4b7d09:
        mov eax, dword ptr [ebx + 0x25c]
        mov ecx, dword ptr [edi + 0xcc]
        add eax, ecx
    L_4b7d17:
        mov dword ptr [edi + 0xd4], eax
        mov eax, dword ptr [edi + 0x114]
        add esi, 0x4
        cmp esi, eax
        jnz L_4b7cb0
    L_4b7d2a:
        mov eax, dword ptr [edi + 0x110]
        mov ecx, dword ptr [eax]
        call ListLabel_GetLineHeight
        sub dword ptr [edi + 0xd8], eax
    L_4b7d3d:
        mov ecx, dword ptr [esp + 0x20]
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x1c
        ret 0x8
    }
}

// 0x004b7d60 RadioGroup_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RadioGroup_Ctor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Button_Ctor
        xor edx, edx
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7740
        mov dword ptr [esi + 0x14c], edx
        mov dword ptr [esi + 0x150], edx
        lea eax, [esi + 0x1b8]
        mov ecx, 0x14
    L_4b7d87:
        mov dword ptr [eax - 0x50], edx
        mov dword ptr [eax], edx
        add eax, 0x4
        dec ecx
        jnz L_4b7d87
        mov dword ptr [esi + 0x154], edx
        mov dword ptr [esi + 0x158], 0x14
        mov dword ptr [esi + 0x15c], edx
        mov dword ptr [esi + 0x164], edx
        mov dword ptr [esi + 0x160], edx
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004b7dc0 ScalarDeletingDtor_004b7dc0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b7dc0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call RadioGroup_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b7dd8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b7dd8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b7de0 RadioGroup_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RadioGroup_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_RadioGroup_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        push ebp
        mov ebp, ecx
        push esi
        push edi
        mov dword ptr [esp + 0x10], ebp
        mov dword ptr [ebp], offset g_RData_004cc000 + 0x7740
        xor edi, edi
        lea esi, [ebp + 0x1b8]
        mov dword ptr [esp + 0x1c], edi
        mov ebx, 0x14
    L_4b7e18:
        mov ecx, dword ptr [esi - 0x50]
        cmp ecx, edi
        jz L_4b7e28
        mov eax, dword ptr [ecx]
        push 0x1
        call dword ptr [eax]
        mov dword ptr [esi - 0x50], edi
    L_4b7e28:
        mov ecx, dword ptr [esi]
        cmp ecx, edi
        jz L_4b7e36
        mov edx, dword ptr [ecx]
        push 0x1
        call dword ptr [edx]
        mov dword ptr [esi], edi
    L_4b7e36:
        add esi, 0x4
        dec ebx
        jnz L_4b7e18
        mov ecx, ebp
        mov dword ptr [esp + 0x1c], 0xffffffff
        call Button_Dtor
        mov ecx, dword ptr [esp + 0x14]
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004b7ee0 Cycler_Next - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Cycler_Next(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x14c]
        mov edx, dword ptr [ecx + 0x150]
        inc eax
        push esi
        mov dword ptr [ecx + 0x14c], eax
        mov esi, eax
        mov eax, dword ptr [ecx + 0x158]
        cmp eax, edx
        jl L_4b7f02
        mov eax, edx
    L_4b7f02:
        cmp esi, eax
        jl L_4b7f12
        mov eax, dword ptr [ecx + 0x154]
        mov dword ptr [ecx + 0x14c], eax
    L_4b7f12:
        call Button_Activate
        pop esi
        ret
    }
}

// 0x004b7fd0 Cycler_SetItemImage - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall Cycler_SetItemImage(int, int, int, int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_Cycler_SetItemImage
        push eax
        mov dword ptr FS:[0x0], esp
        push ebx
        mov ebx, dword ptr [esp + 0x14]
        push ebp
        push esi
        mov esi, ecx
        push edi
        cmp ebx, dword ptr [esi + 0x150]
        jl L_4b801a
        lea eax, [ebx + 0x1]
        cmp eax, 0x14
        jl L_4b8004
        mov eax, 0x14
    L_4b8004:
        mov ecx, dword ptr [esi + 0x158]
        mov dword ptr [esi + 0x150], eax
        cmp eax, ecx
        jle L_4b801a
        mov dword ptr [esi + 0x158], eax
    L_4b801a:
        cmp ebx, dword ptr [esi + 0x158]
        jg L_4b80e6
        push 0x2c0
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov edi, eax
        add esp, 0x4
        mov dword ptr [esp + 0x20], edi
        xor ebp, ebp
        cmp edi, ebp
        mov dword ptr [esp + 0x18], ebp
        jz L_4b8081
        push ebp
        push ebp
        push ebp
        mov ecx, edi
        call ListLabel_Ctor
        mov dword ptr [edi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [edi + 0x2a8], 0x3eb33333
        mov dword ptr [edi + 0x2a4], ebp
        mov dword ptr [edi + 0x2ac], ebp
        mov dword ptr [edi + 0x2b4], ebp
        mov dword ptr [edi + 0x2b8], ebp
        mov dword ptr [edi + 0x2bc], 0x1
        jmp L_4b8083
    L_4b8081:
        xor edi, edi
    L_4b8083:
        mov ecx, dword ptr [esp + 0x24]
        mov dword ptr [esi + ebx*0x4 + 0x168], edi
        mov eax, dword ptr [edi]
        push ecx
        push edi
        mov dword ptr [esp + 0x20], 0xffffffff
        call dword ptr [eax + 0x74]
        mov eax, dword ptr [esi + 0x164]
        mov edi, dword ptr [esp + 0x34]
        mov ecx, dword ptr [esi + ebx*0x4 + 0x168]
        add esp, 0x8
        add eax, edi
        mov edi, dword ptr [esp + 0x28]
        mov edx, dword ptr [ecx]
        push eax
        mov eax, dword ptr [esi + 0x160]
        add eax, edi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [esi + ebx*0x4 + 0x168]
        push ebp
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + ebx*0x4 + 0x168]
        mov ecx, dword ptr [esi + 0xc8]
        push eax
        call Widget_AppendChild
    L_4b80e6:
        mov ecx, dword ptr [esp + 0x10]
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0xc
        ret 0x10
    }
}

// 0x004b8200 ImageList_SetItemImage - bytes ret 0x10 (i,img,..): grow count +0x150 to i+1 capped 20 (0x14), max +0x158; if i within: operator new(0xbc) ImageWidget_Ctor, ImageWidget_SetImage, Widget_AppendChild
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall ImageList_SetItemImage(int, int, int, int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_ImageList_SetItemImage
        push eax
        mov dword ptr FS:[0x0], esp
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esp + 0x18]
        cmp edi, dword ptr [esi + 0x150]
        jle L_4b8248
        lea eax, [edi + 0x1]
        cmp eax, 0x14
        jl L_4b8232
        mov eax, 0x14
    L_4b8232:
        mov ecx, dword ptr [esi + 0x158]
        mov dword ptr [esi + 0x150], eax
        cmp eax, ecx
        jle L_4b8248
        mov dword ptr [esi + 0x158], eax
    L_4b8248:
        cmp edi, dword ptr [esi + 0x158]
        jg L_4b82cc
        push 0xbc
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov dword ptr [esp + 0x18], eax
        test eax, eax
        mov dword ptr [esp + 0x10], 0x0
        jz L_4b8278
        push 0x0
        mov ecx, eax
        call ImageWidget_Ctor
        jmp L_4b827a
    L_4b8278:
        xor eax, eax
    L_4b827a:
        mov ecx, dword ptr [esp + 0x1c]
        mov dword ptr [esp + 0x10], 0xffffffff
        push ecx
        mov ecx, eax
        mov dword ptr [esi + edi*0x4 + 0x1b8], eax
        call ImageWidget_SetImage
        mov eax, dword ptr [esp + 0x24]
        mov ecx, dword ptr [esi + edi*0x4 + 0x1b8]
        push eax
        mov eax, dword ptr [esp + 0x24]
        mov edx, dword ptr [ecx]
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [esi + edi*0x4 + 0x1b8]
        push 0x0
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + edi*0x4 + 0x1b8]
        mov ecx, dword ptr [esi + 0xc8]
        push eax
        call Widget_AppendChild
    L_4b82cc:
        mov ecx, dword ptr [esp + 0x8]
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0xc
        ret 0x10
    }
}

// 0x004b82e0 Cycler_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Cycler_LoadFromConfig(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push ebx
        push ebp
        push esi
        push edi
        mov edi, dword ptr [esp + 0x14]
        push eax
        mov esi, ecx
        push edi
        call Button_LoadFromConfig
        mov edx, offset g_Data_004da000 + 0xa75c
        mov ecx, edi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b830e
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [esi + 0x15c], ecx
    L_4b830e:
        mov edx, offset g_Data_004da000 + 0xa750
        mov ecx, edi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b8336
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        mov dword ptr [esi + 0x160], ecx
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0x14]
        mov dword ptr [esi + 0x164], eax
    L_4b8336:
        mov edx, offset g_Data_004da000 + 0xa748
        mov ecx, edi
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [esp + 0x14], eax
        jz L_4b843a
        mov ecx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [ecx + 0x4]
        dec ecx
        cmp ecx, 0x14
        jl L_4b835f
        mov ecx, 0x14
    L_4b835f:
        mov edx, dword ptr [esi + 0x158]
        mov dword ptr [esi + 0x150], ecx
        cmp ecx, edx
        jle L_4b8375
        mov dword ptr [esi + 0x158], ecx
    L_4b8375:
        xor ebp, ebp
        test ecx, ecx
        jle L_4b843a
        mov dword ptr [esp + 0x18], 0x8
        jmp L_4b838d
    L_4b8389:
        mov eax, dword ptr [esp + 0x14]
    L_4b838d:
        mov edi, dword ptr [esp + 0x18]
        mov ecx, dword ptr [eax + 0x4]
        add edi, ecx
        mov edx, offset g_Data_004da000 + 0xa738
        mov ecx, edi
        call ConfigTree_FindChild
        mov ebx, eax
        test ebx, ebx
        jz L_4b83e3
        mov eax, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esi + 0xc0]
        mov ecx, dword ptr [eax + 0x1c]
        add edx, ecx
        mov ecx, dword ptr [esi + 0xbc]
        push edx
        mov edx, dword ptr [eax + 0x14]
        add ecx, edx
        push ecx
        mov ecx, dword ptr [eax + 0xc]
        call Call004a5bf0_If004a5b20
        push eax
        push ebp
        mov ecx, esi
        call Cycler_SetItemImage
        mov edx, dword ptr [ebx + 0x4]
        mov ecx, esi
        mov eax, dword ptr [edx + 0x24]
        push eax
        push ebp
        call Cycler_SetItemFont
    L_4b83e3:
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, edi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b8420
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [esi + 0xbc]
        mov edx, dword ptr [esi + 0xc0]
        cmp dword ptr [eax + 0x4], 0x4
        jl L_4b8412
        mov ebx, dword ptr [eax + 0x14]
        mov edi, dword ptr [eax + 0x1c]
        add ecx, ebx
        add edx, edi
    L_4b8412:
        push edx
        push ecx
        mov ecx, dword ptr [eax + 0xc]
        push ecx
        push ebp
        mov ecx, esi
        call ImageList_SetItemImage
    L_4b8420:
        mov ecx, dword ptr [esp + 0x18]
        mov eax, dword ptr [esi + 0x150]
        inc ebp
        add ecx, 0x8
        cmp ebp, eax
        mov dword ptr [esp + 0x18], ecx
        jl L_4b8389
    L_4b843a:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        ret 0x8
    }
}

// 0x004b8450 ToggleImage_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ToggleImage_Ctor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Button_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x77c8
        mov dword ptr [esi + 0x14c], eax
        mov dword ptr [esi + 0x16c], eax
        mov dword ptr [esi + 0x150], eax
        mov dword ptr [esi + 0x178], eax
        mov dword ptr [esi + 0x170], eax
        mov dword ptr [esi + 0x17c], eax
        mov dword ptr [esi + 0x174], eax
        mov dword ptr [esi + 0x15c], eax
        mov dword ptr [esi + 0x154], eax
        mov dword ptr [esi + 0x160], eax
        mov dword ptr [esi + 0x158], eax
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004b84b0 ScalarDeletingDtor_004b84b0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b84b0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ToggleImage_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b84c8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b84c8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b84d0 ToggleImage_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ToggleImage_Dtor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0x16c]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x77c8
        test ecx, ecx
        jz L_4b84f3
        cmp ecx, dword ptr [esi + 0x3c]
        jz L_4b84f3
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0x16c], eax
    L_4b84f3:
        mov ecx, dword ptr [esi + 0x150]
        test ecx, ecx
        jz L_4b850d
        cmp ecx, dword ptr [esi + 0x3c]
        jz L_4b850d
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0x150], eax
    L_4b850d:
        mov ecx, esi
        call Button_Dtor
        pop esi
        ret
    }
}

// 0x004b85c0 Slider_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Slider_LoadFromConfig(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        push eax
        mov esi, ecx
        push edi
        call Button_LoadFromConfig
        mov edx, offset g_Data_004da000 + 0xa764
        mov ecx, edi
        call ConfigTree_FindChild
        mov edi, eax
        test edi, edi
        jz L_4b8639
        mov ecx, dword ptr [edi + 0x4]
        push ebp
        mov ebp, dword ptr [esi + 0xc0]
        push ebx
        mov ecx, dword ptr [ecx + 0xc]
        mov ebx, dword ptr [esi + 0xbc]
        call Image_Load
        mov edx, dword ptr [esi]
        mov ecx, esi
        mov dword ptr [esi + 0x150], eax
        call dword ptr [edx + 0x20]
        mov edi, dword ptr [edi + 0x4]
        cmp dword ptr [edi + 0x4], 0x4
        jl L_4b861e
        mov ecx, dword ptr [edi + 0x14]
        mov eax, dword ptr [edi + 0x1c]
        add ebx, ecx
        add ebp, eax
    L_4b861e:
        mov eax, dword ptr [esi]
        push ebp
        push ebx
        mov ecx, esi
        call dword ptr [eax + 0xc]
        mov ecx, dword ptr [esi + 0x3c]
        mov edx, dword ptr [esi]
        mov dword ptr [esi + 0x16c], ecx
        mov ecx, esi
        call dword ptr [edx + 0x20]
        pop ebx
        pop ebp
    L_4b8639:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x84]
        pop edi
        mov eax, 0x1
        pop esi
        ret 0x8
    }
}

// 0x004b8650 Slider_SetFromMouse - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slider_SetFromMouse(int, int)
{
    __asm {
        sub esp, 0x8
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi + 0xc8]
        mov eax, dword ptr [esi]
        add edi, 0x14
        call dword ptr [eax + 0x64]
        mov ecx, dword ptr [edi]
        sub ecx, eax
        mov eax, dword ptr [esi + 0x3c]
        test eax, eax
        mov dword ptr [esp + 0xc], ecx
        jz L_4b867e
        movsx edx, word ptr [eax + 0x4]
        mov dword ptr [esp + 0x8], edx
        jmp L_4b8686
    L_4b867e:
        mov dword ptr [esp + 0x8], 0x0
    L_4b8686:
        fild dword ptr [esp + 0xc]
        mov eax, dword ptr [esi]
        push ecx
        mov ecx, esi
        fidiv dword ptr [esp + 0xc]
        fstp dword ptr [esp]
        call dword ptr [eax + 0x84]
        mov ecx, esi
        call Button_Activate
        pop edi
        pop esi
        add esp, 0x8
        ret
    }
}

// 0x004b8760 Slider_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slider_Ctor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Button_Ctor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7850
        mov dword ptr [esi + 0x14c], eax
        mov dword ptr [esi + 0x16c], eax
        mov dword ptr [esi + 0x170], eax
        mov dword ptr [esi + 0x150], eax
        mov dword ptr [esi + 0x158], eax
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004b87a0 ScalarDeletingDtor_004b87c0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b87c0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Slider_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b87b8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b87b8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b87c0 Slider_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slider_Dtor(int, int)
{
    __asm {
        mov dword ptr [ecx], offset g_RData_004cc000 + 0x7850
        jmp Button_Dtor
    }
}

// 0x004b87d0 Toggle_OnHoverIfOff - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Toggle_OnHoverIfOff(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x14c]
        test eax, eax
        jnz L_4b87df
        jmp Button_Highlight
    L_4b87df:
        ret
    }
}

// 0x004b87f0 Scrollbar_Slot_ApplyToTarget - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Scrollbar_Slot_ApplyToTarget(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov eax, dword ptr [esi + 0x154]
        mov ecx, dword ptr [esi + 0x150]
        push eax
        call ScrollGroup_Select
        mov ecx, dword ptr [esi + 0x150]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x30]
        mov ecx, esi
        call Button_Activate
        mov eax, dword ptr [esi + 0x150]
        xor edi, edi
        mov ecx, dword ptr [eax + 0x14c]
        test ecx, ecx
        jle L_4b884d
        push ebx
        mov ebx, 0x150
    L_4b8830:
        mov eax, dword ptr [ebx + eax*0x1]
        mov ecx, eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x40]
        mov eax, dword ptr [esi + 0x150]
        inc edi
        add ebx, 0x4
        cmp edi, dword ptr [eax + 0x14c]
        jl L_4b8830
        pop ebx
    L_4b884d:
        pop edi
        pop esi
        ret
    }
}

// 0x004b8850 Toggle_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Toggle_LoadFromConfig(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0xc]
        push esi
        push edi
        push eax
        mov esi, ecx
        push ebp
        call Button_LoadFromConfig
        mov eax, dword ptr [esi + 0x3c]
        mov ecx, dword ptr [esi + 0xe4]
        mov edx, dword ptr [esi]
        mov dword ptr [esi + 0x170], eax
        mov eax, dword ptr [esi + 0xf4]
        mov dword ptr [esi + 0x178], ecx
        mov ecx, esi
        mov dword ptr [esi + 0x16c], eax
        mov dword ptr [esi + 0x174], eax
        call dword ptr [edx + 0x68]
        mov dword ptr [esi + 0xd0], eax
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x64]
        mov ecx, dword ptr [esi + 0x3c]
        mov dword ptr [esi + 0xcc], eax
        test ecx, ecx
        jz L_4b88b3
        movsx edx, word ptr [ecx + 0x4]
        jmp L_4b88b5
    L_4b88b3:
        xor edx, edx
    L_4b88b5:
        add edx, dword ptr [esi + 0xd0]
        test ecx, ecx
        mov dword ptr [esi + 0xd8], edx
        jz L_4b88cb
        movsx ecx, word ptr [ecx + 0x6]
        jmp L_4b88cd
    L_4b88cb:
        xor ecx, ecx
    L_4b88cd:
        add ecx, eax
        mov eax, dword ptr [esi + 0x170]
        test eax, eax
        mov dword ptr [esi + 0xd4], ecx
        jz L_4b890e
        mov ecx, dword ptr [esi + 0x18]
        mov edx, dword ptr [esi + 0x14]
        mov dword ptr [esi + 0xd0], ecx
        mov dword ptr [esi + 0xcc], edx
        movsx edi, word ptr [eax + 0x6]
        add edi, ecx
        mov dword ptr [esi + 0xd8], edi
        movsx ecx, word ptr [eax + 0x4]
        add ecx, edx
        mov dword ptr [esi + 0xd4], ecx
        jmp L_4b89f0
    L_4b890e:
        mov eax, dword ptr [esi + 0x110]
        test eax, eax
        jz L_4b89f0
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x68]
        mov dword ptr [esi + 0xd0], eax
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x64]
        mov dword ptr [esi + 0xcc], eax
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        call ListLabel_GetLineHeight
        add eax, dword ptr [esi + 0xd0]
        mov dword ptr [esi + 0xd8], eax
        mov edi, dword ptr [esi + 0x110]
        cmp edi, dword ptr [esi + 0x114]
        jz L_4b89dd
    L_4b8963:
        mov ecx, dword ptr [edi]
        call ListLabel_GetLineHeight
        mov ecx, dword ptr [esi + 0xd8]
        add ecx, eax
        mov dword ptr [esi + 0xd8], ecx
        mov ebx, dword ptr [edi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b898e
        mov edx, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [edx + 0x90]
    L_4b898e:
        mov ecx, dword ptr [ebx + 0x25c]
        mov ebx, dword ptr [esi + 0xcc]
        mov eax, dword ptr [esi + 0xd4]
        add ecx, ebx
        cmp ecx, eax
        jle L_4b89ca
        mov ebx, dword ptr [edi]
        mov eax, dword ptr [ebx + 0x270]
        test eax, eax
        jz L_4b89bc
        mov edx, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [edx + 0x90]
    L_4b89bc:
        mov eax, dword ptr [ebx + 0x25c]
        mov ecx, dword ptr [esi + 0xcc]
        add eax, ecx
    L_4b89ca:
        mov dword ptr [esi + 0xd4], eax
        mov eax, dword ptr [esi + 0x114]
        add edi, 0x4
        cmp edi, eax
        jnz L_4b8963
    L_4b89dd:
        mov eax, dword ptr [esi + 0x110]
        mov ecx, dword ptr [eax]
        call ListLabel_GetLineHeight
        sub dword ptr [esi + 0xd8], eax
    L_4b89f0:
        mov edx, dword ptr [esi + 0xcc]
        mov eax, dword ptr [esi + 0xd0]
        lea edi, [esi + 0x15c]
        mov dword ptr [esi + 0x158], 0x1
        mov ecx, edi
        mov dword ptr [ecx], edx
        mov edx, dword ptr [esi + 0xd4]
        mov dword ptr [ecx + 0x4], eax
        mov eax, dword ptr [esi + 0xd8]
        mov dword ptr [ecx + 0x8], edx
        mov edx, offset g_Data_004da000 + 0xa770
        mov dword ptr [ecx + 0xc], eax
        mov ecx, ebp
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b8a75
        mov ecx, dword ptr [eax + 0x4]
        mov ebp, dword ptr [esi + 0x160]
        mov ebx, dword ptr [edi]
        mov edx, dword ptr [ecx + 0xc]
        add ebp, edx
        mov dword ptr [esi + 0x160], ebp
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0x14]
        add ebx, edx
        mov dword ptr [edi], ebx
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0x1c]
        mov ecx, ebp
        add edx, ecx
        mov dword ptr [esi + 0x168], edx
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0x24]
        mov eax, ebx
        add ecx, eax
        mov dword ptr [esi + 0x164], ecx
    L_4b8a75:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        ret 0x8
    }
}

// 0x004b8b10 ScrollGroup_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScrollGroup_Ctor(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call Button_Ctor
        lea edi, [esi + 0x150]
        mov ecx, 0xa
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x78d8
        mov dword ptr [esi + 0x14c], 0x0
        rep stosd
        mov eax, esi
        pop edi
        pop esi
        ret
    }
}

// 0x004b8b40 ScalarDeletingDtor_004b8b40 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b8b40(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ScrollGroup_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b8b58
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b8b58:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b8b60 ScrollGroup_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScrollGroup_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ScrollGroup_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        mov ebx, ecx
        push esi
        push edi
        mov dword ptr [esp + 0xc], ebx
        mov dword ptr [ebx], offset g_RData_004cc000 + 0x78d8
        mov dword ptr [esp + 0x18], 0x0
        lea esi, [ebx + 0x150]
        mov edi, 0xa
    L_4b8b98:
        mov ecx, dword ptr [esi]
        test ecx, ecx
        jz L_4b8baa
        mov eax, dword ptr [ecx]
        push 0x1
        call dword ptr [eax]
        mov dword ptr [esi], 0x0
    L_4b8baa:
        add esi, 0x4
        dec edi
        jnz L_4b8b98
        mov ecx, ebx
        mov dword ptr [esp + 0x18], 0xffffffff
        call Button_Dtor
        mov ecx, dword ptr [esp + 0x10]
        pop edi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004b8be0 ScrollGroup_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ScrollGroup_LoadFromConfig(int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ScrollGroup_LoadFromConfig
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x8
        mov eax, dword ptr [esp + 0x1c]
        push ebx
        push ebp
        mov ebp, ecx
        mov ecx, dword ptr [esp + 0x20]
        push esi
        push edi
        mov edx, offset g_Data_004da000 + 0xa77c
        mov dword ptr [ebp + 0xc8], eax
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [esp + 0x10], eax
        jz L_4b8cc6
        mov ecx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [ecx + 0x4]
        dec ecx
        cmp ecx, 0xa
        jl L_4b8c33
        mov ecx, 0xa
    L_4b8c33:
        xor edi, edi
        mov dword ptr [ebp + 0x14c], ecx
        test ecx, ecx
        jle L_4b8cc6
        mov dword ptr [esp + 0x28], 0x8
        lea esi, [ebp + 0x150]
        jmp L_4b8c57
    L_4b8c53:
        mov eax, dword ptr [esp + 0x10]
    L_4b8c57:
        mov ebx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [esp + 0x28]
        push 0x17c
        add ebx, ecx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov dword ptr [esp + 0x14], eax
        test eax, eax
        mov dword ptr [esp + 0x20], 0x0
        jz L_4b8c86
        mov ecx, eax
        call Slider_Ctor
        jmp L_4b8c88
    L_4b8c86:
        xor eax, eax
    L_4b8c88:
        mov ecx, dword ptr [esp + 0x2c]
        mov dword ptr [esi], eax
        mov edx, dword ptr [eax]
        push ecx
        push ebx
        mov ecx, eax
        mov dword ptr [esp + 0x28], 0xffffffff
        call dword ptr [edx + 0x7c]
        mov eax, dword ptr [esi]
        mov edx, dword ptr [esp + 0x28]
        add edx, 0x8
        add esi, 0x4
        mov dword ptr [eax + 0x150], ebp
        mov dword ptr [eax + 0x154], edi
        mov eax, dword ptr [ebp + 0x14c]
        inc edi
        cmp edi, eax
        mov dword ptr [esp + 0x28], edx
        jl L_4b8c53
    L_4b8cc6:
        push 0x0
        mov ecx, ebp
        call ScrollGroup_Select
        mov ecx, dword ptr [esp + 0x18]
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x14
        ret 0x8
    }
}

// 0x004b8d30 OptionListScreen_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall OptionListScreen_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_OptionListScreen_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x8
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x10], esi
        call Toggle_Ctor
        xor edi, edi
        lea ebx, [esi + 0x16c]
        push edi
        push edi
        push edi
        mov ecx, ebx
        mov dword ptr [esp + 0x28], edi
        call ListLabel_Ctor
        mov AL, byte ptr [esp + 0xf]
        mov ecx, dword ptr [esp + 0x14]
        mov dword ptr [ebx], offset g_RData_004cc000 + 0x7960
        mov byte ptr [esi + 0x41c], AL
        mov dword ptr [esi + 0x420], edi
        mov dword ptr [esi + 0x424], edi
        mov dword ptr [esi + 0x428], edi
        mov dword ptr [esi + 0x164], edi
        mov dword ptr [esi + 0x418], edi
        mov dword ptr [esi + 0x434], edi
        mov dword ptr [esi + 0x438], edi
        mov dword ptr [esi + 0x43c], edi
        mov dword ptr [esi + 0x440], edi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1a80
        mov dword ptr [esi + 0x42c], 0xf
        mov dword ptr [esi + 0x430], 0xffffffff
        mov eax, esi
        pop edi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x14
        ret
    }
}

// 0x004b8de0 ListScreen_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ListScreen_LoadFromConfig(int, int, int, int)
{
    __asm {
        sub esp, 0x18
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0x24]
        push esi
        push edi
        mov edi, dword ptr [esp + 0x30]
        xor ebx, ebx
        push edi
        mov esi, ecx
        push ebp
        mov dword ptr [esp + 0x24], ebx
        mov dword ptr [esp + 0x20], ebx
        mov dword ptr [esp + 0x2c], ebx
        mov dword ptr [esp + 0x28], ebx
        call Button_LoadStatesFromConfig
        mov eax, dword ptr [edi + 0x118]
        mov edx, offset g_Data_004da000 + 0xa7b0
        mov ecx, ebp
        mov dword ptr [esp + 0x14], eax
        call ConfigTree_FindChild
        cmp eax, ebx
        jz L_4b8eab
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [esi + 0xc8]
        mov dword ptr [esi + 0x444], eax
        lea edx, [eax + eax*0x8]
        mov edi, dword ptr [ecx + edx*0x4 + 0x1cec]
        lea eax, [ecx + edx*0x4 + 0x1cec]
        neg edi
        sbb edi, edi
        and edi, eax
        cmp edi, ebx
        jz L_4b8eab
        mov ecx, dword ptr [edi + 0x1c]
        mov edx, dword ptr [edi + 0x8]
        mov eax, dword ptr [esi + 0x16c]
        push 0x2
        push 0x0
        push 0x0
        push 0x0
        lea ebx, [esi + 0x16c]
        push ecx
        mov ecx, dword ptr [edi + 0x4]
        push edx
        push ecx
        mov ecx, ebx
        call dword ptr [eax + 0x80]
        mov eax, dword ptr [edi + 0xc]
        mov dword ptr [ebx + 0x14c], eax
        mov dword ptr [ebx + 0x150], eax
        mov eax, 0x1
        mov dword ptr [ebx + 0x270], eax
        mov edx, dword ptr [edi + 0x18]
        mov dword ptr [ebx + 0x264], edx
        mov dword ptr [ebx + 0x29c], eax
        mov dword ptr [ebx + 0x2a0], eax
        xor ebx, ebx
    L_4b8eab:
        mov edx, offset g_Data_004da000 + 0xa7a4
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, ebx
        jz L_4b8ec4
        mov eax, dword ptr [eax + 0x4]
        mov dword ptr [esi + 0x448], eax
    L_4b8ec4:
        mov edx, offset g_Data_004da000 + 0xa79c
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, ebx
        jz L_4b8edd
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [esi + 0x42c], ecx
    L_4b8edd:
        mov edx, offset g_Data_004da000 + 0xa790
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, ebx
        jz L_4b8f29
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        fild dword ptr [ecx + 0xc]
        fstp dword ptr [esi + 0x434]
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        fild dword ptr [ecx + 0x14]
        fstp dword ptr [esi + 0x438]
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0x14]
        fild dword ptr [ecx + 0xc]
        fstp dword ptr [esi + 0x43c]
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0x14]
        fild dword ptr [eax + 0x14]
        fstp dword ptr [esi + 0x440]
    L_4b8f29:
        mov edx, offset g_Data_004da000 + 0xa784
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, ebx
        jz L_4b90c9
        mov eax, dword ptr [eax + 0x4]
        xor ecx, ecx
        mov edi, dword ptr [eax + 0x4]
        mov edx, dword ptr [eax + 0xc]
        cmp edi, 0x2
        jle L_4b8f50
        mov ecx, dword ptr [eax + 0x14]
    L_4b8f50:
        push ecx
        push edx
        mov ecx, esi
        call ListScreen_CreateRows
        mov eax, dword ptr [esi + 0x164]
        mov dword ptr [esp + 0x2c], ebx
        cmp eax, ebx
        jle L_4b9095
        mov dword ptr [esp + 0x10], ebx
    L_4b8f6f:
        mov edx, dword ptr [esi + 0x418]
        mov ecx, ebx
        add ecx, edx
        push ecx
        mov ecx, dword ptr [esp + 0x34]
        call Widget_AppendChild
        mov ebp, dword ptr [esi + 0x418]
        mov ecx, ebx
        add ecx, ebp
        push 0x1
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0x418]
        mov dword ptr [ebx + eax*0x1 + 0x2a8], esi
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jz L_4b8fbc
        mov ecx, dword ptr [esi + 0x418]
        lea edi, [esp + 0x18]
        add ecx, ebx
        push edi
        push eax
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x18]
    L_4b8fbc:
        mov eax, dword ptr [esi + 0x448]
        mov ecx, dword ptr [esi + 0xc8]
        lea eax, [eax + eax*0x8]
        mov edi, dword ptr [ecx + eax*0x4 + 0x1cec]
        lea eax, [ecx + eax*0x4 + 0x1cec]
        neg edi
        sbb edi, edi
        and edi, eax
        test edi, edi
        jz L_4b9072
        mov eax, dword ptr [esi + 0x164]
        xor ebp, ebp
        test eax, eax
        jle L_4b9072
        xor ebx, ebx
    L_4b8ff5:
        mov eax, dword ptr [edi + 0x1c]
        mov ecx, dword ptr [esi + 0x418]
        push 0x2
        push 0x0
        mov edx, dword ptr [ecx + ebx*0x1]
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [edi + 0x8]
        add ecx, ebx
        push eax
        mov eax, dword ptr [edi + 0x4]
        push eax
        call dword ptr [edx + 0x80]
        mov edx, dword ptr [esi + 0x418]
        mov ecx, dword ptr [edi + 0xc]
        mov eax, ebx
        add eax, edx
        mov dword ptr [eax + 0x14c], ecx
        mov dword ptr [eax + 0x150], ecx
        mov ecx, 0x1
        mov dword ptr [eax + 0x270], ecx
        mov edx, dword ptr [esi + 0x418]
        mov eax, ebx
        add ebx, 0x2ac
        add eax, edx
        mov edx, dword ptr [edi + 0x18]
        inc ebp
        mov dword ptr [eax + 0x264], edx
        mov dword ptr [eax + 0x29c], ecx
        mov dword ptr [eax + 0x2a0], ecx
        mov eax, dword ptr [esi + 0x164]
        cmp ebp, eax
        jl L_4b8ff5
        mov ebx, dword ptr [esp + 0x10]
    L_4b9072:
        mov eax, dword ptr [esp + 0x2c]
        mov ecx, dword ptr [esi + 0x164]
        inc eax
        add ebx, 0x2ac
        cmp eax, ecx
        mov dword ptr [esp + 0x2c], eax
        mov dword ptr [esp + 0x10], ebx
        jl L_4b8f6f
        xor ebx, ebx
    L_4b9095:
        mov ecx, dword ptr [esp + 0x30]
        lea edi, [esi + 0x16c]
        push edi
        call Widget_AppendChild
        mov eax, dword ptr [edi]
        push 0x1
        mov ecx, edi
        call dword ptr [eax + 0x60]
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esi + 0x414], esi
        cmp eax, ebx
        jz L_4b90c9
        mov edx, dword ptr [edi]
        lea ecx, [esp + 0x18]
        push ecx
        push eax
        mov ecx, edi
        call dword ptr [edx + 0x18]
    L_4b90c9:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        add esp, 0x18
        ret 0x8
    }
}

// 0x004b90e0 ListScreen_CreateRows - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall ListScreen_CreateRows(int, int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_ListScreen_CreateRows
        push eax
        mov dword ptr FS:[0x0], esp
        push ebx
        push ebp
        push esi
        mov esi, ecx
        xor ebx, ebx
        push edi
        mov ecx, dword ptr [esi + 0x418]
        cmp ecx, ebx
        jz L_4b9113
        mov eax, dword ptr [ecx]
        push 0x3
        call dword ptr [eax]
        mov dword ptr [esi + 0x418], ebx
    L_4b9113:
        mov ebp, dword ptr [esp + 0x20]
        lea ecx, [ebp + ebp*0x8]
        lea eax, [ebp + ecx*0x2]
        lea edx, [eax + eax*0x8]
        lea eax, [edx*0x4 + 0x4]
        push eax
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov dword ptr [esp + 0x20], eax
        cmp eax, ebx
        mov dword ptr [esp + 0x18], ebx
        jz L_4b915b
        push offset ListLabel_Dtor
        push offset ListLabel_Subclass_Ctor_004b92a0
        lea edi, [eax + 0x4]
        push ebp
        push 0x2ac
        push edi
        mov dword ptr [eax], ebp
        call ArrayCtor_Eh
        jmp L_4b915d
    L_4b915b:
        xor edi, edi
    L_4b915d:
        mov eax, dword ptr [esp + 0x24]
        mov dword ptr [esi + 0x418], edi
        xor edi, edi
        cmp eax, ebx
        mov dword ptr [esp + 0x18], 0xffffffff
        mov dword ptr [esi + 0x164], ebp
        mov dword ptr [esi + 0x168], eax
        jle L_4b91e4
        mov dword ptr [esp + 0x20], ebx
    L_4b9185:
        mov ecx, edi
        sub ecx, eax
        imul ecx, dword ptr [esi + 0x42c]
        mov dword ptr [esp + 0x24], ecx
        fild dword ptr [esp + 0x24]
        fiadd dword ptr [esi + 0xc0]
        fadd dword ptr [esi + 0x438]
        call dword ptr [g_Iat__ftol_004cc5ac]
        fild dword ptr [esi + 0xbc]
        mov edx, dword ptr [esi + 0x418]
        push eax
        add ebx, edx
        fadd dword ptr [esi + 0x434]
        mov ebp, dword ptr [ebx]
        call dword ptr [g_Iat__ftol_004cc5ac]
        push eax
        mov ecx, ebx
        call dword ptr [ebp + 0xc]
        mov ebx, dword ptr [esp + 0x20]
        mov eax, dword ptr [esi + 0x168]
        inc edi
        add ebx, 0x2ac
        cmp edi, eax
        mov dword ptr [esp + 0x20], ebx
        jl L_4b9185
    L_4b91e4:
        mov edx, dword ptr [esi + 0xc0]
        mov eax, dword ptr [esi + 0x16c]
        lea ecx, [esi + 0x16c]
        push edx
        mov edx, dword ptr [esi + 0xbc]
        push edx
        call dword ptr [eax + 0xc]
        mov edi, dword ptr [esi + 0x168]
        mov eax, dword ptr [esi + 0x164]
        cmp edi, eax
        jge L_4b9287
        lea eax, [edi + edi*0x8]
        lea eax, [edi + eax*0x2]
        lea ebx, [eax + eax*0x8]
        shl ebx, 0x2
        mov dword ptr [esp + 0x20], ebx
    L_4b9221:
        mov eax, dword ptr [esi + 0x168]
        mov ecx, edi
        sub ecx, eax
        inc ecx
        imul ecx, dword ptr [esi + 0x42c]
        mov dword ptr [esp + 0x24], ecx
        fild dword ptr [esp + 0x24]
        fiadd dword ptr [esi + 0xc0]
        fadd dword ptr [esi + 0x440]
        call dword ptr [g_Iat__ftol_004cc5ac]
        fild dword ptr [esi + 0xbc]
        mov edx, dword ptr [esi + 0x418]
        push eax
        add ebx, edx
        fadd dword ptr [esi + 0x43c]
        mov ebp, dword ptr [ebx]
        call dword ptr [g_Iat__ftol_004cc5ac]
        push eax
        mov ecx, ebx
        call dword ptr [ebp + 0xc]
        mov ebx, dword ptr [esp + 0x20]
        mov eax, dword ptr [esi + 0x164]
        inc edi
        add ebx, 0x2ac
        cmp edi, eax
        mov dword ptr [esp + 0x20], ebx
        jl L_4b9221
    L_4b9287:
        mov ecx, dword ptr [esp + 0x10]
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0xc
        ret 0x8
    }
}

// 0x004b92a0 ListLabel_Subclass_Ctor_004b92a0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_Subclass_Ctor_004b92a0(int, int)
{
    __asm {
        push esi
        push 0x0
        push 0x0
        mov esi, ecx
        push 0x0
        call ListLabel_Ctor
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7960
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004b92c0 ListItem_VectorDeletingDtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListItem_VectorDeletingDtor(int, int, int)
{
    __asm {
        push ebx
        mov BL, byte ptr [esp + 0x8]
        push esi
        push edi
        test BL, 0x2
        mov esi, ecx
        jz L_4b92f6
        mov eax, dword ptr [esi - 0x4]
        lea edi, [esi - 0x4]
        push offset ListLabel_Dtor
        push eax
        push 0x2ac
        push esi
        call ArrayDtor_Eh2
        push edi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        mov eax, esi
        pop edi
        pop esi
        pop ebx
        ret 0x4
    L_4b92f6:
        mov ecx, esi
        call ListLabel_Dtor
        test BL, 0x1
        jz L_4b930b
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b930b:
        mov eax, esi
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004b9710 ScalarDeletingDtor_004b9710 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004b9710(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call AnimImageWidget_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b9728
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b9728:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b9850 ScreenBase_SetActive - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenBase_SetActive(int, int, int)
{
    __asm {
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0xc]
        push esi
        test ebp, ebp
        push edi
        mov ebx, ecx
        jz L_4b9897
        lea esi, [ebx + 0x1c74]
        mov edi, 0xa
    L_4b9869:
        mov ecx, dword ptr [esi]
        test ecx, ecx
        jz L_4b987b
        mov eax, dword ptr [esi + 0x4]
        push eax
        call Sound_PlayResourceAuto
        mov dword ptr [esi + 0x8], eax
    L_4b987b:
        add esi, 0xc
        dec edi
        jnz L_4b9869
        mov ecx, ebx
        call WidgetContainer_RedrawChildren
        push ebp
        mov ecx, ebx
        call SetField4
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    L_4b9897:
        lea esi, [ebx + 0x1c7c]
        mov edi, 0xa
    L_4b98a2:
        mov ecx, dword ptr [esi]
        test ecx, ecx
        jz L_4b98ad
        call Sound_StopOrRestoreVoice
    L_4b98ad:
        mov dword ptr [esi], 0x0
        add esi, 0xc
        dec edi
        jnz L_4b98a2
        push ebp
        mov ecx, ebx
        call SetField4
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    }
}

// 0x004ba020 ListLabel_Subclass_Ctor_004ba020 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_Subclass_Ctor_004ba020(int, int)
{
    __asm {
        push esi
        push edi
        xor edi, edi
        mov esi, ecx
        push edi
        push edi
        push edi
        call ListLabel_Ctor
        mov dword ptr [esi + 0x2a4], edi
        mov dword ptr [esi + 0x2ac], edi
        mov dword ptr [esi + 0x2b4], edi
        mov dword ptr [esi + 0x2b8], edi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esi + 0x2a8], 0x3eb33333
        mov dword ptr [esi + 0x2bc], 0x1
        mov eax, esi
        pop edi
        pop esi
        ret
    }
}

// 0x004ba740 ListLabel_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ListLabel_Ctor(int, int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ListLabel_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        mov eax, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x14]
        push ebx
        push esi
        mov esi, ecx
        xor ebx, ebx
        mov ecx, dword ptr [esp + 0x20]
        push ebx
        push eax
        push ecx
        push edx
        mov ecx, esi
        mov dword ptr [esp + 0x18], esi
        call TextWidget_Ctor
        mov eax, 0xffffff
        push 0xa
        mov dword ptr [esp + 0x18], ebx
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7a88
        mov dword ptr [esi + 0x148], ebx
        mov dword ptr [esi + 0x14c], eax
        mov dword ptr [esi + 0x150], eax
        mov dword ptr [esi + 0x270], 0x1
        call dword ptr [g_Iat_GetStockObject_004cc098]
        mov dword ptr [esi + 0x154], eax
        lea eax, [esi + 0x28c]
        mov ecx, ebx
        mov edx, ebx
        mov dword ptr [eax], ecx
        mov byte ptr [esi + 0x15c], BL
        mov dword ptr [esi + 0x264], ebx
        mov dword ptr [esi + 0x270], 0x1
        mov dword ptr [eax + 0x4], edx
        mov dword ptr [esi + 0x144], ebx
        mov dword ptr [esi + 0x268], 0x1
        mov dword ptr [esi + 0x284], ebx
        mov dword ptr [eax + 0x8], ecx
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [esi + 0x27c], ebx
        mov dword ptr [esi + 0x288], ebx
        mov dword ptr [eax + 0xc], edx
        mov dword ptr [esi + 0x280], ebx
        mov dword ptr [esi + 0x278], ebx
        mov dword ptr [esi + 0x274], ebx
        mov dword ptr [esi + 0x260], ebx
        mov dword ptr [esi + 0x25c], ebx
        mov eax, esi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret 0xc
    }
}

// 0x004ba830 ScalarDeletingDtor_004ba830 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004ba830(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ListLabel_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4ba848
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4ba848:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004ba850 ListLabel_CopyCtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabel_CopyCtor(int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ListLabel_CopyCtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x40
        push esi
        push edi
        mov edi, dword ptr [esp + 0x58]
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0xc], esi
        call TextWidget_CopyCtor
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7a88
        mov dword ptr [esp + 0x50], eax
        mov dword ptr [esi + 0x148], eax
        mov eax, dword ptr [edi + 0x14c]
        lea edx, [esp + 0xc]
        mov dword ptr [esi + 0x14c], eax
        mov ecx, dword ptr [edi + 0x150]
        mov dword ptr [esi + 0x150], ecx
        mov eax, dword ptr [edi + 0x154]
        push edx
        push 0x3c
        push eax
        call dword ptr [g_Iat_GetObjectA_004cc090]
        test eax, eax
        jz L_4ba8cd
        lea ecx, [esp + 0xc]
        push ecx
        call dword ptr [g_Iat_CreateFontIndirectA_004cc094]
        mov dword ptr [esi + 0x154], eax
    L_4ba8cd:
        mov edx, dword ptr [edi + 0x158]
        lea eax, [edi + 0x15c]
        push 0x100
        lea ecx, [esi + 0x15c]
        push eax
        push ecx
        mov dword ptr [esi + 0x158], edx
        call dword ptr [g_Iat_strncpy_004cc5a0]
        mov edx, dword ptr [edi + 0x25c]
        add esp, 0xc
        mov dword ptr [esi + 0x25c], edx
        mov eax, dword ptr [edi + 0x260]
        mov dword ptr [esi + 0x260], eax
        mov ecx, dword ptr [edi + 0x264]
        mov dword ptr [esi + 0x264], ecx
        mov edx, dword ptr [edi + 0x268]
        mov dword ptr [esi + 0x268], edx
        mov eax, dword ptr [edi + 0x26c]
        mov dword ptr [esi + 0x26c], eax
        mov ecx, dword ptr [edi + 0x270]
        mov dword ptr [esi + 0x270], ecx
        mov edx, dword ptr [edi + 0x274]
        mov dword ptr [esi + 0x274], edx
        mov eax, dword ptr [edi + 0x278]
        lea ecx, [edi + 0x27c]
        mov dword ptr [esi + 0x278], eax
        lea edx, [esi + 0x27c]
        mov eax, dword ptr [ecx]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov eax, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], eax
        lea eax, [esi + 0x28c]
        mov ecx, dword ptr [ecx + 0xc]
        mov dword ptr [edx + 0xc], ecx
        lea edx, [edi + 0x28c]
        mov ecx, dword ptr [edi + 0x28c]
        mov dword ptr [eax], ecx
        mov ecx, dword ptr [edx + 0x4]
        mov dword ptr [eax + 0x4], ecx
        mov ecx, dword ptr [edx + 0x8]
        mov dword ptr [eax + 0x8], ecx
        mov edx, dword ptr [edx + 0xc]
        mov dword ptr [eax + 0xc], edx
        mov eax, dword ptr [edi + 0x144]
        mov dword ptr [esi + 0x144], eax
        mov ecx, dword ptr [edi + 0x29c]
        mov dword ptr [esi + 0x29c], ecx
        mov edx, dword ptr [edi + 0x2a0]
        mov ecx, dword ptr [esp + 0x48]
        mov dword ptr [esi + 0x2a0], edx
        mov eax, esi
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x4c
        ret 0x4
    }
}

// 0x004bab40 ListLabel_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ListLabel_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7a88
        mov ecx, dword ptr [esi + 0x148]
        mov dword ptr [esp + 0x10], 0x0
        test ecx, ecx
        jz L_4bab84
        call Image_Free
        mov dword ptr [esi + 0x148], 0x0
    L_4bab84:
        mov eax, dword ptr [esi + 0x154]
        push eax
        call dword ptr [g_Iat_DeleteObject_004cc070]
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0xa10
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004bb790 ListWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListWidget_Ctor(int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ListWidget_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x2cc
        push ebx
        push ebp
        push esi
        xor ebp, ebp
        push edi
        push ebp
        mov ebx, ecx
        push ebp
        push ebp
        mov dword ptr [esp + 0x24], ebx
        call ListLabel_Ctor
        mov AL, byte ptr [esp + 0x13]
        lea edi, [ebx + 0x2a8]
        mov dword ptr [esp + 0x2e4], ebp
        mov byte ptr [edi], AL
        mov dword ptr [edi + 0x4], ebp
        mov dword ptr [edi + 0x8], ebp
        mov dword ptr [edi + 0xc], ebp
        push ebp
        push ebp
        push ebp
        lea ecx, [esp + 0x28]
        mov byte ptr [esp + 0x2f0], 0x1
        mov dword ptr [ebx], offset g_RData_004cc000 + 0x7b40
        mov dword ptr [ebx + 0x2a4], ebp
        call ListLabel_Ctor
        mov dword ptr [esp + 0x1c], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esp + 0x2c4], 0x3eb33333
        mov dword ptr [esp + 0x2c0], ebp
        mov dword ptr [esp + 0x2c8], ebp
        mov dword ptr [esp + 0x2d0], ebp
        mov dword ptr [esp + 0x2d4], ebp
        mov dword ptr [esp + 0x2d8], 0x1
        mov esi, dword ptr [edi + 0x4]
        mov byte ptr [esp + 0x2e4], 0x2
        cmp esi, ebp
        jnz L_4bb848
        xor edx, edx
        jmp L_4bb85e
    L_4bb848:
        mov ecx, dword ptr [edi + 0x8]
        mov eax, 0x2e8ba2e9
        sub ecx, esi
        imul ecx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
    L_4bb85e:
        mov ebp, dword ptr [esp + 0x2ec]
        cmp edx, ebp
        jnc L_4bb89e
        test esi, esi
        jnz L_4bb871
        xor edx, edx
        jmp L_4bb887
    L_4bb871:
        mov ecx, dword ptr [edi + 0x8]
        mov eax, 0x2e8ba2e9
        sub ecx, esi
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bb887:
        lea ecx, [esp + 0x1c]
        mov eax, ebp
        push ecx
        mov ecx, dword ptr [edi + 0x8]
        sub eax, edx
        push eax
        push ecx
        mov ecx, edi
        call ListItemVector_InsertN
        jmp L_4bb905
    L_4bb89e:
        test esi, esi
        jnz L_4bb8a6
        xor edx, edx
        jmp L_4bb8bc
    L_4bb8a6:
        mov ecx, dword ptr [edi + 0x8]
        mov eax, 0x2e8ba2e9
        sub ecx, esi
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bb8bc:
        cmp ebp, edx
        jnc L_4bb905
        mov ecx, dword ptr [edi + 0x8]
        lea edx, [ebp + ebp*0x4]
        lea eax, [ebp + edx*0x2]
        mov edx, ecx
        shl eax, 0x6
        add eax, esi
        push eax
        call ListItem_CopyRange
        mov ebp, dword ptr [edi + 0x8]
        mov dword ptr [esp + 0x14], eax
        cmp eax, ebp
        mov esi, eax
        jz L_4bb8fb
    L_4bb8e5:
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx]
        add esi, 0x2c0
        cmp esi, ebp
        jnz L_4bb8e5
        mov eax, dword ptr [esp + 0x14]
    L_4bb8fb:
        mov ebp, dword ptr [esp + 0x2ec]
        mov dword ptr [edi + 0x8], eax
    L_4bb905:
        lea ecx, [esp + 0x1c]
        mov byte ptr [esp + 0x2e4], 0x1
        call ListLabel_Dtor
        push offset g_Data_004da000 + 0x6be4
        push ebx
        call ListLabel_Printf
        add esp, 0x8
        mov ecx, ebx
        push 0x0
        push 0x0
        call ListWidget_SetPosition
        push ebp
        mov ecx, ebx
        call ListWidget_SetItemCount
        mov eax, dword ptr [ebx]
        push 0x1
        mov ecx, ebx
        call dword ptr [eax + 0x60]
        mov ecx, dword ptr [esp + 0x2dc]
        pop edi
        pop esi
        mov eax, ebx
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x2d8
        ret 0x4
    }
}

// 0x004bb960 ScalarDeletingDtor_00403e20 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_00403e20(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ListWidget_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4bb978
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bb978:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004bbca0 ListWidget_SetItemCount - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListWidget_SetItemCount(int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ListWidget_SetItemCount
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x2c8
        push ebx
        mov ebx, ecx
        push ebp
        push esi
        mov eax, dword ptr [ebx + 0x2ac]
        lea ebp, [ebx + 0x2a8]
        xor esi, esi
        push edi
        cmp eax, esi
        mov dword ptr [esp + 0x14], ebx
        jnz L_4bbcdd
        mov dword ptr [esp + 0x10], esi
        jmp L_4bbcf7
    L_4bbcdd:
        mov ecx, dword ptr [ebp + 0x8]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
        mov dword ptr [esp + 0x10], edx
    L_4bbcf7:
        mov edi, dword ptr [esp + 0x2e8]
        mov eax, dword ptr [esp + 0x10]
        cmp edi, eax
        jz L_4bbe4f
        push esi
        push esi
        push esi
        lea ecx, [esp + 0x24]
        call ListLabel_Ctor
        mov dword ptr [esp + 0x18], offset g_RData_004cc000 + 0x1388
        mov dword ptr [esp + 0x2c0], 0x3eb33333
        mov dword ptr [esp + 0x2bc], 0x0
        mov dword ptr [esp + 0x2c4], esi
        mov dword ptr [esp + 0x2cc], esi
        mov dword ptr [esp + 0x2d0], esi
        mov dword ptr [esp + 0x2d4], 0x1
        mov ecx, dword ptr [ebp + 0x4]
        mov dword ptr [esp + 0x2e0], esi
        cmp ecx, esi
        jnz L_4bbd66
        xor edx, edx
        jmp L_4bbd7c
    L_4bbd66:
        mov edx, dword ptr [ebp + 0x8]
        mov eax, 0x2e8ba2e9
        sub edx, ecx
        imul edx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bbd7c:
        cmp edx, edi
        jnc L_4bbdb5
        cmp ecx, esi
        jnz L_4bbd88
        xor edx, edx
        jmp L_4bbd9e
    L_4bbd88:
        mov edx, dword ptr [ebp + 0x8]
        mov eax, 0x2e8ba2e9
        sub edx, ecx
        imul edx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
    L_4bbd9e:
        mov ecx, edi
        lea eax, [esp + 0x18]
        sub ecx, edx
        mov edx, dword ptr [ebp + 0x8]
        push eax
        push ecx
        push edx
        mov ecx, ebp
        call ListItemVector_InsertN
        jmp L_4bbe2c
    L_4bbdb5:
        cmp ecx, esi
        jnz L_4bbdbd
        xor edx, edx
        jmp L_4bbdd3
    L_4bbdbd:
        mov edx, dword ptr [ebp + 0x8]
        mov eax, 0x2e8ba2e9
        sub edx, ecx
        imul edx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bbdd3:
        cmp edi, edx
        jnc L_4bbe2c
        mov ebx, dword ptr [ebp + 0x8]
        lea edx, [edi + edi*0x4]
        mov esi, ebx
        lea edi, [edi + edx*0x2]
        shl edi, 0x6
        add edi, ecx
        cmp ebx, ebx
        jz L_4bbe03
    L_4bbdeb:
        push esi
        mov ecx, edi
        call ListLabelAnimated_Assign
        add esi, 0x2c0
        add edi, 0x2c0
        cmp esi, ebx
        jnz L_4bbdeb
    L_4bbe03:
        mov ebx, dword ptr [ebp + 0x8]
        mov esi, edi
        cmp edi, ebx
        jz L_4bbe1e
    L_4bbe0c:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        add esi, 0x2c0
        cmp esi, ebx
        jnz L_4bbe0c
    L_4bbe1e:
        mov ebx, dword ptr [esp + 0x14]
        mov dword ptr [ebp + 0x8], edi
        mov edi, dword ptr [esp + 0x2e8]
    L_4bbe2c:
        lea ecx, [esp + 0x18]
        mov dword ptr [esp + 0x2e0], 0xffffffff
        call ListLabel_Dtor
        mov ecx, dword ptr [esp + 0x10]
        push edi
        push ecx
        mov ecx, ebx
        call ListWidget_ClearRange
        jmp L_4bbe56
    L_4bbe4f:
        mov ecx, ebx
        call ListWidget_ClearAll
    L_4bbe56:
        mov esi, dword ptr [ebx]
        mov ecx, ebx
        call dword ptr [esi + 0x68]
        mov edx, dword ptr [ebx]
        push eax
        mov ecx, ebx
        call dword ptr [edx + 0x64]
        push eax
        mov ecx, ebx
        call dword ptr [esi + 0xc]
        mov ecx, dword ptr [esp + 0x2d8]
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x2d4
        ret 0x4
    }
}

// 0x004bbff0 ListItemVector_InsertN - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ListItemVector_InsertN(int, int, int, int, int)
{
    __asm {
        sub esp, 0xc
        push ebx
        push ebp
        push esi
        push edi
        mov edi, ecx
        mov eax, 0x2e8ba2e9
        mov ebp, dword ptr [esp + 0x24]
        mov dword ptr [esp + 0x10], edi
        mov esi, dword ptr [edi + 0x8]
        mov ecx, dword ptr [edi + 0xc]
        sub ecx, esi
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
        cmp edx, ebp
        jnc L_4bc1bf
        mov edi, dword ptr [edi + 0x4]
        test edi, edi
        jnz L_4bc02d
        xor edx, edx
        jmp L_4bc042
    L_4bc02d:
        mov ecx, esi
        mov eax, 0x2e8ba2e9
        sub ecx, edi
        imul ecx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
    L_4bc042:
        cmp ebp, edx
        jnc L_4bc067
        test edi, edi
        jnz L_4bc04e
        xor ecx, ecx
        jmp L_4bc069
    L_4bc04e:
        mov ecx, esi
        mov eax, 0x2e8ba2e9
        sub ecx, edi
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
        mov ecx, edx
        jmp L_4bc069
    L_4bc067:
        mov ecx, ebp
    L_4bc069:
        test edi, edi
        jnz L_4bc071
        xor edx, edx
        jmp L_4bc084
    L_4bc071:
        sub esi, edi
        mov eax, 0x2e8ba2e9
        imul esi
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bc084:
        lea eax, [edx + ecx*0x1]
        test eax, eax
        mov dword ptr [esp + 0x14], eax
        jge L_4bc091
        xor eax, eax
    L_4bc091:
        lea ecx, [eax + eax*0x4]
        lea edx, [eax + ecx*0x2]
        shl edx, 0x6
        push edx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov ecx, dword ptr [esp + 0x24]
        mov dword ptr [esp + 0x1c], eax
        mov ebx, eax
        mov eax, dword ptr [esp + 0x14]
        add esp, 0x4
        mov esi, dword ptr [eax + 0x4]
        cmp esi, ecx
        jz L_4bc0d8
    L_4bc0b8:
        test ebx, ebx
        jz L_4bc0c4
        push esi
        mov ecx, ebx
        call ListLabelAnimated_CopyCtor
    L_4bc0c4:
        mov ecx, dword ptr [esp + 0x20]
        add esi, 0x2c0
        add ebx, 0x2c0
        cmp esi, ecx
        jnz L_4bc0b8
    L_4bc0d8:
        test ebp, ebp
        mov esi, ebx
        jbe L_4bc0fd
        mov edi, ebp
    L_4bc0e0:
        test esi, esi
        jz L_4bc0f0
        mov ecx, dword ptr [esp + 0x28]
        push ecx
        mov ecx, esi
        call ListLabelAnimated_CopyCtor
    L_4bc0f0:
        add esi, 0x2c0
        dec edi
        jnz L_4bc0e0
        mov ecx, dword ptr [esp + 0x20]
    L_4bc0fd:
        lea edx, [ebp + ebp*0x4]
        lea eax, [ebp + edx*0x2]
        mov edx, dword ptr [esp + 0x10]
        shl eax, 0x6
        mov ebp, dword ptr [edx + 0x8]
        cmp ecx, ebp
        lea edi, [ebx + eax*0x1]
        jz L_4bc13a
        mov esi, edi
        sub esi, ebx
        add esi, ecx
        sub esi, eax
    L_4bc11e:
        test edi, edi
        jz L_4bc12a
        push esi
        mov ecx, edi
        call ListLabelAnimated_CopyCtor
    L_4bc12a:
        add esi, 0x2c0
        add edi, 0x2c0
        cmp esi, ebp
        jnz L_4bc11e
    L_4bc13a:
        mov ebx, dword ptr [esp + 0x10]
        mov edi, dword ptr [ebx + 0x8]
        mov esi, dword ptr [ebx + 0x4]
        cmp esi, edi
        jz L_4bc15a
    L_4bc148:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        add esi, 0x2c0
        cmp esi, edi
        jnz L_4bc148
    L_4bc15a:
        mov eax, dword ptr [ebx + 0x4]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        mov eax, dword ptr [esp + 0x18]
        mov esi, dword ptr [esp + 0x1c]
        add esp, 0x4
        lea ecx, [eax + eax*0x4]
        lea edx, [eax + ecx*0x2]
        mov eax, dword ptr [ebx + 0x4]
        shl edx, 0x6
        add edx, esi
        test eax, eax
        mov dword ptr [ebx + 0xc], edx
        jnz L_4bc187
        xor edx, edx
        jmp L_4bc19d
    L_4bc187:
        mov ecx, dword ptr [ebx + 0x8]
        sub ecx, eax
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov eax, edx
        shr eax, 0x1f
        add edx, eax
    L_4bc19d:
        mov ecx, dword ptr [esp + 0x24]
        mov dword ptr [ebx + 0x4], esi
        lea eax, [ecx + edx*0x1]
        lea edx, [eax + eax*0x4]
        lea eax, [eax + edx*0x2]
        shl eax, 0x6
        add eax, esi
        mov dword ptr [ebx + 0x8], eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0xc
    L_4bc1bf:
        mov edi, dword ptr [esp + 0x20]
        mov ecx, esi
        sub ecx, edi
        mov eax, 0x2e8ba2e9
        imul ecx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
        cmp edx, ebp
        jnc L_4bc28a
        lea edx, [ebp + ebp*0x4]
        lea eax, [ebp + edx*0x2]
        shl eax, 0x6
        cmp edi, esi
        mov dword ptr [esp + 0x24], eax
        lea ebx, [edi + eax*0x1]
        jz L_4bc21a
        mov edi, ebx
        sub edi, eax
    L_4bc1fa:
        test ebx, ebx
        jz L_4bc206
        push edi
        mov ecx, ebx
        call ListLabelAnimated_CopyCtor
    L_4bc206:
        add edi, 0x2c0
        add ebx, 0x2c0
        cmp edi, esi
        jnz L_4bc1fa
        mov edi, dword ptr [esp + 0x20]
    L_4bc21a:
        mov eax, dword ptr [esp + 0x10]
        mov ebx, dword ptr [esp + 0x28]
        mov esi, dword ptr [eax + 0x8]
        mov eax, 0x2e8ba2e9
        mov ecx, esi
        sub ecx, edi
        imul ecx
        sar edx, 0x7
        mov ecx, edx
        shr ecx, 0x1f
        add edx, ecx
        sub ebp, edx
        jz L_4bc253
    L_4bc23e:
        test esi, esi
        jz L_4bc24a
        push ebx
        mov ecx, esi
        call ListLabelAnimated_CopyCtor
    L_4bc24a:
        add esi, 0x2c0
        dec ebp
        jnz L_4bc23e
    L_4bc253:
        mov ebp, dword ptr [esp + 0x10]
        mov esi, dword ptr [esp + 0x20]
        mov edi, dword ptr [ebp + 0x8]
        cmp esi, edi
        jz L_4bc274
    L_4bc262:
        push ebx
        mov ecx, esi
        call ListLabelAnimated_Assign
        add esi, 0x2c0
        cmp esi, edi
        jnz L_4bc262
    L_4bc274:
        mov edx, dword ptr [esp + 0x24]
        mov eax, dword ptr [ebp + 0x8]
        add eax, edx
        mov dword ptr [ebp + 0x8], eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0xc
    L_4bc28a:
        test ebp, ebp
        jbe L_4bc314
        lea eax, [ebp + ebp*0x4]
        mov edi, esi
        mov ebx, esi
        lea ebp, [ebp + eax*0x2]
        shl ebp, 0x6
        sub edi, ebp
        cmp edi, esi
        jz L_4bc2c3
    L_4bc2a7:
        test ebx, ebx
        jz L_4bc2b3
        push edi
        mov ecx, ebx
        call ListLabelAnimated_CopyCtor
    L_4bc2b3:
        add edi, 0x2c0
        add ebx, 0x2c0
        cmp edi, esi
        jnz L_4bc2a7
    L_4bc2c3:
        mov ecx, dword ptr [esp + 0x10]
        mov ebx, dword ptr [esp + 0x20]
        mov edi, dword ptr [ecx + 0x8]
        mov esi, edi
        sub esi, ebp
        cmp ebx, esi
        jz L_4bc2ee
    L_4bc2d6:
        sub esi, 0x2c0
        sub edi, 0x2c0
        push esi
        mov ecx, edi
        call ListLabelAnimated_Assign
        cmp esi, ebx
        jnz L_4bc2d6
    L_4bc2ee:
        lea edi, [ebx + ebp*0x1]
        mov esi, ebx
        cmp ebx, edi
        jz L_4bc30d
        mov ebx, dword ptr [esp + 0x28]
    L_4bc2fb:
        push ebx
        mov ecx, esi
        call ListLabelAnimated_Assign
        add esi, 0x2c0
        cmp esi, edi
        jnz L_4bc2fb
    L_4bc30d:
        mov eax, dword ptr [esp + 0x10]
        add dword ptr [eax + 0x8], ebp
    L_4bc314:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0xc
    }
}

// 0x004bc410 ListLabelAnimated_CopyCtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ListLabelAnimated_CopyCtor(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        push edi
        call ListLabel_CopyCtor
        mov eax, dword ptr [edi + 0x2a4]
        mov dword ptr [esi + 0x2a4], eax
        mov ecx, dword ptr [edi + 0x2a8]
        mov dword ptr [esi + 0x2a8], ecx
        mov edx, dword ptr [edi + 0x2ac]
        mov dword ptr [esi + 0x2ac], edx
        mov eax, dword ptr [edi + 0x2b0]
        mov dword ptr [esi + 0x2b0], eax
        mov ecx, dword ptr [edi + 0x2b4]
        mov dword ptr [esi + 0x2b4], ecx
        mov edx, dword ptr [edi + 0x2b8]
        mov dword ptr [esi + 0x2b8], edx
        mov eax, dword ptr [edi + 0x2bc]
        mov dword ptr [esi + 0x2bc], eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x1388
        mov eax, esi
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004bcb50 TextWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall TextWidget_Ctor(int, int, int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_TextWidget_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push 0x0
        push 0x0
        mov dword ptr [esp + 0xc], esi
        call Widget_BaseCtor
        mov eax, dword ptr [esp + 0x18]
        mov dword ptr [esp + 0x10], 0x0
        push eax
        push esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7c70
        mov dword ptr [esi + 0x138], 0x0
        call TextWidget_Printf
        mov ecx, dword ptr [esp + 0x24]
        mov edx, dword ptr [esp + 0x28]
        mov eax, dword ptr [esi]
        mov dword ptr [esi + 0x14], ecx
        add esp, 0x8
        mov ecx, esi
        mov dword ptr [esi + 0x18], edx
        call dword ptr [eax + 0x20]
        mov ecx, dword ptr [esp + 0x24]
        mov edx, dword ptr [esi]
        mov dword ptr [esi + 0x134], ecx
        mov ecx, esi
        call dword ptr [edx + 0x20]
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi + 0x144], 0x0
        mov eax, esi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret 0x10
    }
}

// 0x004bcbe0 TextWidget_CopyCtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall TextWidget_CopyCtor(int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_TextWidget_CopyCtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        push edi
        mov edi, dword ptr [esp + 0x1c]
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0xc], esi
        call Widget_CopyCtor
        lea eax, [edi + 0x34]
        push 0x100
        lea ecx, [esi + 0x34]
        push eax
        push ecx
        mov dword ptr [esp + 0x20], 0x0
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7c70
        call dword ptr [g_Iat_strncpy_004cc5a0]
        mov edx, dword ptr [edi + 0x134]
        add esp, 0xc
        mov dword ptr [esi + 0x134], edx
        mov eax, dword ptr [edi + 0x138]
        mov dword ptr [esi + 0x138], eax
        mov ecx, dword ptr [edi + 0x13c]
        mov dword ptr [esi + 0x13c], ecx
        mov edx, dword ptr [edi + 0x140]
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [esi + 0x140], edx
        mov eax, dword ptr [edi + 0x144]
        pop edi
        mov dword ptr [esi + 0x144], eax
        mov eax, esi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret 0x4
    }
}

// 0x004bcf20 TextField_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall TextField_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_TextField_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        push edi
        xor edi, edi
        mov esi, ecx
        push edi
        push edi
        mov dword ptr [esp + 0x10], esi
        call Widget_BaseCtor
        mov dword ptr [esp + 0x14], edi
        mov dword ptr [esi + 0x130], edi
        mov ecx, 0x3f
        xor eax, eax
        lea edi, [esi + 0x34]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7ce8
        rep stosd
        mov ecx, esi
        call Widget_OrFlagsWithGlobal
        mov ecx, dword ptr [esp + 0xc]
        mov eax, esi
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004bd020 ChatInputPanel_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ChatInputPanel_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ChatInputPanel_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        push ebp
        push esi
        mov ebp, ecx
        push edi
        mov dword ptr [esp + 0x10], ebp
        call WidgetContainer_Ctor
        push offset ListLabel_Dtor
        push offset ListLabel_CtorDefault_004bd100
        lea esi, [ebp + 0x10]
        push 0x4
        push 0x2a4
        push esi
        mov dword ptr [esp + 0x30], 0x0
        call ArrayCtor_Eh
        mov byte ptr [esp + 0x1c], 0x1
        mov dword ptr [ebp], offset g_RData_004cc000 + 0x7d60
        mov edi, 0x1e
        or ebx, 0xffffffff
    L_4bd07b:
        push esi
        mov ecx, ebp
        call Widget_AppendChild
        mov eax, dword ptr [esi]
        push 0x2
        push 0x0
        push 0x0
        push 0x7
        push 0x258
        push 0xd
        push offset g_Data_004da000 + 0xcc0
        mov ecx, esi
        call dword ptr [eax + 0x80]
        mov eax, 0x1
        mov dword ptr [esi + 0x29c], ebx
        mov dword ptr [esi + 0x264], eax
        mov dword ptr [esi + 0x2a0], ebx
        mov dword ptr [esi + 0x144], eax
        mov edx, dword ptr [esi]
        push edi
        push 0x140
        mov ecx, esi
        call dword ptr [edx + 0xc]
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x60]
        add edi, 0x12
        add esi, 0x2a4
        cmp edi, 0x66
        jl L_4bd07b
        mov ecx, dword ptr [esp + 0x14]
        pop edi
        mov eax, ebp
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004bd100 ListLabel_CtorDefault_004bd100 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ListLabel_CtorDefault_004bd100(int, int)
{
    __asm {
        push 0x0
        push 0x0
        push 0x0
        call ListLabel_Ctor
        ret
    }
}

// 0x004bd2d0 MessageLinesPanel_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MessageLinesPanel_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MessageLinesPanel_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        push ebp
        push esi
        mov ebp, ecx
        push edi
        mov dword ptr [esp + 0x10], ebp
        call WidgetContainer_Ctor
        push offset ListLabel_Dtor
        push offset ListLabel_CtorDefault_004bd100
        lea esi, [ebp + 0x10]
        push 0x4
        push 0x2a4
        push esi
        mov dword ptr [esp + 0x30], 0x0
        call ArrayCtor_Eh
        mov ebx, 0x1
        mov dword ptr [ebp], offset g_RData_004cc000 + 0x7d68
        mov byte ptr [esp + 0x1c], BL
        mov edi, 0x159
    L_4bd32c:
        push esi
        mov ecx, ebp
        call Widget_AppendChild
        push 0x2
        push 0x0
        push 0x0
        push 0x6
        mov dword ptr [esi + 0x14c], 0x996a00
        mov dword ptr [esi + 0x150], 0x95c7ff
        mov dword ptr [esi + 0x270], ebx
        mov eax, dword ptr [esi]
        push 0x1f4
        push 0xa
        push offset g_Data_004da000 + 0xcc0
        mov ecx, esi
        call dword ptr [eax + 0x80]
        or eax, 0xffffffff
        mov dword ptr [esi + 0x264], ebx
        mov dword ptr [esi + 0x29c], eax
        mov dword ptr [esi + 0x2a0], eax
        mov dword ptr [esi + 0x144], ebx
        mov edx, dword ptr [esi]
        push edi
        push 0x140
        mov ecx, esi
        call dword ptr [edx + 0xc]
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x60]
        sub edi, 0x12
        add esi, 0x2a4
        cmp edi, 0x111
        jg L_4bd32c
        mov ecx, dword ptr [esp + 0x14]
        pop edi
        mov eax, ebp
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004bdc70 SnowFX_Construct - ../../04_spec/systems/render.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall SnowFX_Construct(int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_SnowFX_Construct
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        push ebp
        push esi
        xor ebp, ebp
        push edi
        mov esi, ecx
        push ebp
        push ebp
        mov dword ptr [esp + 0x18], esi
        call Widget_BaseCtor
        mov dword ptr [esi + 0x34], ebp
        mov ebx, dword ptr [esp + 0x24]
        mov dword ptr [esp + 0x1c], ebp
        mov eax, ebx
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7df0
        shl eax, 0x5
        push eax
        mov dword ptr [esi + 0x3c], ebx
        mov dword ptr [esi + 0x40], ebx
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        cmp ebx, ebp
        mov dword ptr [esi + 0x38], eax
        jle L_4bdcee
        xor eax, eax
        mov edx, ebx
        or ecx, 0xffffffff
    L_4bdccc:
        mov edi, dword ptr [esi + 0x38]
        add eax, 0x20
        dec edx
        mov dword ptr [edi + eax*0x1 - 0x20], ecx
        mov edi, dword ptr [esi + 0x38]
        mov dword ptr [edi + eax*0x1 - 0x1c], ecx
        mov edi, dword ptr [esi + 0x38]
        mov dword ptr [edi + eax*0x1 - 0x18], ecx
        mov edi, dword ptr [esi + 0x38]
        mov dword ptr [edi + eax*0x1 - 0x14], ecx
        jnz L_4bdccc
    L_4bdcee:
        lea edi, [ebx + ebx*0x2]
        mov word ptr [esi + 0x44], 0x7fff
        shl edi, 0x2
        push edi
        mov dword ptr [esi + 0x48], 0x3f800000
        mov dword ptr [esi + 0x4c], 0x3d4ccccd
        mov dword ptr [esi + 0x50], ebp
        mov dword ptr [esi + 0x54], ebp
        mov dword ptr [esi + 0x60], ebp
        mov dword ptr [esi + 0x64], 0x1
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov ecx, dword ptr [esi + 0x60]
        add esp, 0x4
        push edi
        mov dword ptr [esi + ecx*0x4 + 0x58], eax
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov edx, dword ptr [esi + 0x64]
        add esp, 0x4
        xor edi, edi
        cmp ebx, ebp
        mov dword ptr [esi + edx*0x4 + 0x58], eax
        jle L_4bdd4d
    L_4bdd3e:
        push 0x1
        push edi
        mov ecx, esi
        call ScreenParticle_Respawn
        inc edi
        cmp edi, ebx
        jl L_4bdd3e
    L_4bdd4d:
        mov eax, 0x3f800000
        mov dword ptr [esi + 0x74], ebp
        mov dword ptr [esi + 0x78], eax
        mov dword ptr [esi + 0x7c], ebp
        mov dword ptr [esi + 0x70], eax
        mov dword ptr [esi + 0x68], ebp
        mov dword ptr [esi + 0x6c], eax
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        cmp eax, ebp
        jz L_4bddfb
        mov dword ptr [esi + 0x80], offset g_Data_004da000 + 0xa894
        call Image_Alloc
        mov DL, 0xb
        mov ecx, eax
        mov dword ptr [esi + 0x84], eax
        call Image_SetFlags
        mov edi, dword ptr [g_Iat_malloc_004cc5dc]
        push 0x80
        call edi
        add esp, 0x4
        push eax
        push 0x100
        call edi
        mov ecx, dword ptr [esi + 0x84]
        add esp, 0x4
        mov edx, eax
        call Image_SetPixelsAndAlpha
        mov eax, dword ptr [esi + 0x84]
        push 0x8
        mov edx, 0x10
        mov CL, byte ptr [eax + 0x9]
        or CL, 0x20
        mov byte ptr [eax + 0x9], CL
        mov ecx, dword ptr [esi + 0x84]
        call Image_SetSize
        mov edx, dword ptr [esi + 0x84]
        mov ecx, dword ptr [esi + 0x80]
        push 0x1
        push 0x1
        mov AL, byte ptr [edx + 0x9]
        and eax, 0x2
        push eax
        call dword ptr [g_Data_004da000 + 0x91c08]
        mov dword ptr [esi + 0x88], eax
    L_4bddfb:
        mov ecx, dword ptr [esp + 0x14]
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret 0x4
    }
}

// 0x004bde20 ScalarDeletingDtor_004bde20 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004bde20(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Overlay_Dtor_Base
        test byte ptr [esp + 0x8], 0x1
        jz L_4bde38
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bde38:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004bde40 Overlay_Dtor_Base - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Overlay_Dtor_Base(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_Overlay_Dtor_Base
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7df0
        mov eax, dword ptr [esi + 0x38]
        mov dword ptr [esp + 0x10], 0x0
        test eax, eax
        jz L_4bde7b
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bde7b:
        mov eax, dword ptr [esi + 0x58]
        test eax, eax
        jz L_4bde8b
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bde8b:
        mov eax, dword ptr [esi + 0x5c]
        test eax, eax
        jz L_4bde9b
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bde9b:
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        test eax, eax
        jz L_4bdec9
        mov ecx, dword ptr [esi + 0x88]
        test ecx, ecx
        jz L_4bdeb4
        call dword ptr [g_Data_004da000 + 0x91c1c]
    L_4bdeb4:
        mov ecx, dword ptr [esi + 0x84]
        test ecx, ecx
        jz L_4bdec9
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0x84], eax
    L_4bdec9:
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0xa10
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004be280 ScreenParticleOverlay_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenParticleOverlay_Ctor(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push eax
        call SnowFX_Construct
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7eb8
        mov dword ptr [esi + 0x8c], 0x1
        mov dword ptr [esi + 0x90], 0x41a00000
        mov dword ptr [esi + 0x94], 0x43c80000
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004be2c0 ScalarDeletingDtor_004be2c0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004be2c0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Overlay_Dtor_Base
        test byte ptr [esp + 0x8], 0x1
        jz L_4be2d8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4be2d8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004be810 ScreenParticleOverlay_CtorRespawnAll - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenParticleOverlay_CtorRespawnAll(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push eax
        call SnowFX_Construct
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7f30
        mov dword ptr [esi + 0x8c], 0x1
        mov dword ptr [esi + 0x90], 0x41a00000
        mov dword ptr [esi + 0x94], 0x43c80000
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004be850 ScalarDeletingDtor_004be870 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004be870(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Overlay_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4be868
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4be868:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004be870 Overlay_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Overlay_Dtor(int, int)
{
    __asm {
        mov dword ptr [ecx], offset g_RData_004cc000 + 0x7f30
        jmp Overlay_Dtor_Base
    }
}

// 0x004bee40 StaticInitWrapper_004bee40 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticInitWrapper_004bee40(int, int)
{
    __asm {
        call GlobalCompositePanel_StaticInit
        jmp GlobalCompositePanel_StaticAtexit
    }
}

// 0x004bee50 GlobalCompositePanel_StaticInit - bytes: ecx=0x0056bd58; jmp CompositePanel_Ctor 0x004bef90
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GlobalCompositePanel_StaticInit(int, int)
{
    __asm {
        mov ecx, offset g_Data_004da000 + 0x91d58
        jmp CompositePanel_Ctor
    }
}

// 0x004bee60 GlobalCompositePanel_StaticAtexit - bytes: crt_atexit(0x004bee70)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GlobalCompositePanel_StaticAtexit(int, int)
{
    __asm {
        push offset AtexitStub_StaticObj_Dtor_5Widgets_004bee70
        call crt_atexit
        add esp, 0x4
        ret
    }
}

// 0x004bee70 AtexitStub_StaticObj_Dtor_5Widgets_004bee70 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AtexitStub_StaticObj_Dtor_5Widgets_004bee70(int, int)
{
    __asm {
        mov ecx, offset g_Data_004da000 + 0x91d58
        jmp StaticObj_Dtor_5Widgets_004bee80
    }
}

// 0x004bee80 StaticObj_Dtor_5Widgets_004bee80 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticObj_Dtor_5Widgets_004bee80(int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_StaticObj_Dtor_5Widgets
        push eax
        mov dword ptr FS:[0x0], esp
        push esi
        mov esi, ecx
        push offset Widget_BaseDtor
        push 0x5
        lea eax, [esi + 0x70]
        push 0x4c
        push eax
        mov dword ptr [esp + 0x1c], 0x1
        call ArrayDtor_Eh2
        mov ecx, esi
        mov dword ptr [esi + 0x28], offset g_RData_004cc000 + 0xa10
        mov dword ptr [esp + 0xc], 0xffffffff
        call WidgetContainer_Dtor
        mov ecx, dword ptr [esp + 0x4]
        pop esi
        mov dword ptr FS:[0x0], ecx
        add esp, 0xc
        ret
    }
}

// 0x004bef90 CompositePanel_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall CompositePanel_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_CompositePanel_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x10], esi
        call WidgetContainer_Ctor
        xor ebp, ebp
        lea ebx, [esi + 0x28]
        push ebp
        push ebp
        mov ecx, ebx
        mov dword ptr [esp + 0x24], ebp
        call Widget_BaseCtor
        mov dword ptr [ebx + 0x34], ebp
        mov dword ptr [ebx], offset g_RData_004cc000 + 0x7fb0
        push offset Widget_BaseDtor
        push offset UiWidget_Subclass_Ctor_004bdbe0
        lea edi, [esi + 0x70]
        push 0x5
        push 0x4c
        push edi
        mov byte ptr [esp + 0x30], 0x1
        call ArrayCtor_Eh
        xor eax, eax
        mov dword ptr [esi], offset g_RData_004cc000 + 0x7fa8
        mov dword ptr [esi + 0x10], eax
        push ebx
        mov dword ptr [esi + 0x14], eax
        mov ecx, esi
        mov byte ptr [esp + 0x20], 0x2
        mov dword ptr [esi + 0x18], ebp
        mov dword ptr [esi + 0x1c], ebp
        mov dword ptr [esi + 0x20], ebp
        call Widget_AppendChild
        mov edx, dword ptr [ebx]
        push ebp
        mov ecx, ebx
        call dword ptr [edx + 0x60]
        mov ebx, 0x5
    L_4bf01d:
        push edi
        mov ecx, esi
        call Widget_AppendChild
        mov eax, dword ptr [edi]
        push ebp
        mov ecx, edi
        call dword ptr [eax + 0x60]
        add edi, 0x4c
        dec ebx
        jnz L_4bf01d
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        mov dword ptr [esi + 0x1ec], ebp
        call dword ptr [edx + 0x4]
        mov ecx, dword ptr [esp + 0x14]
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004bf800 UiScreen_Slot_Owner200Slot0C_ThenActivate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Owner200Slot0C_ThenActivate(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0xc8]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0xc]
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x004bf820 UiScreen_Slot_Owner200Slot10_ThenActivate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Owner200Slot10_ThenActivate(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0xc8]
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x10]
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x004bf840 LineWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall LineWidget_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_LineWidget_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        push edi
        mov esi, ecx
        push 0x0
        push 0x0
        mov dword ptr [esp + 0x10], esi
        call Widget_BaseCtor
        mov ecx, 0x2a
        xor eax, eax
        lea edi, [esi + 0x34]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x8150
        mov dword ptr [esi + 0xdc], 0x0
        mov dword ptr [esp + 0x14], 0x0
        rep stosd
        mov ecx, esi
        call Widget_OrFlagsWithGlobal
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [esi + 0xe4], 0x0
        mov eax, esi
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004bf980 AnimImageWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall AnimImageWidget_Ctor(int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_AnimImageWidget_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        push edi
        xor edi, edi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0xc], esi
        call ImageWidget_Ctor
        mov eax, dword ptr [esp + 0x20]
        mov dword ptr [esp + 0x14], edi
        mov dword ptr [esi + 0xc0], eax
        mov eax, dword ptr [esp + 0x1c]
        cmp eax, edi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x81c8
        mov dword ptr [esi + 0xbc], edi
        jz L_4bf9d0
        push eax
        mov ecx, esi
        call ImageWidget_SetImageAndNotify
    L_4bf9d0:
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [esi + 0xc8], edi
        mov dword ptr [esi + 0xcc], edi
        mov dword ptr [esi + 0xc4], 0x1
        mov eax, esi
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret 0x8
    }
}

// 0x004bfa00 ScalarDeletingDtor_004bfa00 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004bfa00(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call AnimImageWidget_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4bfa18
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bfa18:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004bfa20 AnimImageWidget_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AnimImageWidget_Dtor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0xbc]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x81c8
        test ecx, ecx
        jz L_4bfa38
        call Image_Free
    L_4bfa38:
        mov ecx, esi
        call ImageWidget_Dtor
        pop esi
        ret
    }
}

// 0x004c86e0 EH_Unwind_TextPanel_LoadFromConfig_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_TextPanel_LoadFromConfig_0(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd2c]
        jmp ListLabelVector_Free
    }
}

// 0x004c86eb EH_Unwind_TextPanel_LoadFromConfig_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_TextPanel_LoadFromConfig_1(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd48]
        jmp ListLabel_Dtor
    }
}

// 0x004cb200 EH_Unwind_ImageWidget_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ImageWidget_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb220 EH_Unwind_Caret_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Caret_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb240 EH_Unwind_EditField_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_EditField_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Button_Dtor
    }
}

// 0x004cb248 EH_Unwind_EditField_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_EditField_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x14c
        jmp KeyDispatch_Dtor
    }
}

// 0x004cb256 EH_Unwind_EditField_Ctor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_EditField_Ctor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x260
        jmp Widget_BaseDtor
    }
}

// 0x004cb270 EH_Unwind_EditField_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_EditField_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Button_Dtor
    }
}

// 0x004cb278 EH_Unwind_EditField_Dtor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_EditField_Dtor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x14c
        jmp KeyDispatch_Dtor
    }
}

// 0x004cb286 EH_Unwind_EditField_Dtor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_EditField_Dtor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x260
        jmp Widget_BaseDtor
    }
}

// 0x004cb2a0 EH_Unwind_Button_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ImageWidget_Dtor
    }
}

// 0x004cb2a8 EH_Unwind_Button_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x10c
        jmp PtrVector_Free
    }
}

// 0x004cb2b6 EH_Unwind_Button_Ctor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Ctor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x11c
        jmp PtrVector_Free
    }
}

// 0x004cb2c4 EH_Unwind_Button_Ctor_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Ctor_3(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x12c
        jmp PtrVector_Free
    }
}

// 0x004cb2d2 EH_Unwind_Button_Ctor_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Ctor_4(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x13c
        jmp PtrVector_Free
    }
}

// 0x004cb2f0 EH_Unwind_Button_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ImageWidget_Dtor
    }
}

// 0x004cb2f8 EH_Unwind_Button_Dtor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Dtor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x10c
        jmp PtrVector_Free
    }
}

// 0x004cb306 EH_Unwind_Button_Dtor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Dtor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x11c
        jmp PtrVector_Free
    }
}

// 0x004cb314 EH_Unwind_Button_Dtor_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Dtor_3(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x12c
        jmp PtrVector_Free
    }
}

// 0x004cb322 EH_Unwind_Button_Dtor_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_Dtor_4(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x13c
        jmp PtrVector_Free
    }
}

// 0x004cb340 EH_Unwind_Button_LoadFromConfig_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb34b EH_Unwind_Button_LoadFromConfig_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_1(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb356 EH_Unwind_Button_LoadFromConfig_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_2(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb361 EH_Unwind_Button_LoadFromConfig_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_3(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb36c EH_Unwind_Button_LoadFromConfig_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_4(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb377 EH_Unwind_Button_LoadFromConfig_5 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_5(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb382 EH_Unwind_Button_LoadFromConfig_6 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_6(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb38d EH_Unwind_Button_LoadFromConfig_7 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadFromConfig_7(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb3b0 EH_Unwind_Toggle_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Toggle_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Button_Dtor
    }
}

// 0x004cb3d0 EH_Unwind_Button_LoadStatesFromConfig_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadStatesFromConfig_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x14]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb3db EH_Unwind_Button_LoadStatesFromConfig_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadStatesFromConfig_1(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x14]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb3e6 EH_Unwind_Button_LoadStatesFromConfig_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadStatesFromConfig_2(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x10]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb3f1 EH_Unwind_Button_LoadStatesFromConfig_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadStatesFromConfig_3(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x10]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb3fc EH_Unwind_Button_LoadStatesFromConfig_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Button_LoadStatesFromConfig_4(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x10]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb420 EH_Unwind_RadioGroup_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_RadioGroup_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Button_Dtor
    }
}

// 0x004cb440 EH_Unwind_Cycler_SetItemImage_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Cycler_SetItemImage_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp + 0x4]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb460 EH_Unwind_ImageList_SetItemImage_0 - bytes ret 0x10 (i,img,..): grow count +0x150 to i+1 capped 20 (0x14), max +0x158; if i within: operator new(0xbc) ImageWidget_Ctor, ImageWidget_SetImage, Widget_AppendChild
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ImageList_SetItemImage_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp + 0x4]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb480 EH_Unwind_ScrollGroup_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScrollGroup_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Button_Dtor
    }
}

// 0x004cb4a0 EH_Unwind_ScrollGroup_LoadFromConfig_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScrollGroup_LoadFromConfig_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x10]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb4c0 EH_Unwind_OptionListScreen_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_OptionListScreen_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Toggle_Dtor
    }
}

// 0x004cb4e0 EH_Unwind_ListScreen_CreateRows_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListScreen_CreateRows_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp + 0x4]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cb500 EH_Unwind_ScreenBase_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenRoot_Dtor
    }
}

// 0x004cb508 EH_Unwind_ScreenBase_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x44
        jmp AnimImageWidget_Dtor
    }
}

// 0x004cb513 EH_Unwind_ScreenBase_Ctor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Ctor_2(int, int)
{
    __asm {
        push offset ImageWidget_Dtor
        push 0x14
        push 0xbc
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0x11c
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb549 EH_Unwind_ScreenBase_Ctor_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Ctor_4(int, int)
{
    __asm {
        push offset UiObject_ClearFirstWord_004ba4c0
        push 0x14
        push 0x24
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0x1cec
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb570 EH_Unwind_ScreenBase_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenRoot_Dtor
    }
}

// 0x004cb578 EH_Unwind_ScreenBase_Dtor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Dtor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x44
        jmp AnimImageWidget_Dtor
    }
}

// 0x004cb583 EH_Unwind_ScreenBase_Dtor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Dtor_2(int, int)
{
    __asm {
        push offset ImageWidget_Dtor
        push 0x14
        push 0xbc
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0x11c
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb5b9 EH_Unwind_ScreenBase_Dtor_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Dtor_4(int, int)
{
    __asm {
        push offset UiObject_ClearFirstWord_004ba4c0
        push 0x14
        push 0x24
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0x1cec
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb5e0 EH_Unwind_ListLabel_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListLabel_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb600 EH_Unwind_ListLabel_CopyCtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListLabel_CopyCtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x4c]
        jmp Widget_BaseDtor
    }
}

// 0x004cb620 EH_Unwind_ListLabel_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListLabel_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb640 EH_Unwind_ListWidget_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListWidget_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp + 0xfffffd30]
        jmp ListLabel_Dtor
    }
}

// 0x004cb64b EH_Unwind_ListWidget_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListWidget_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp + 0xfffffd30]
        add ecx, 0x2a8
        jmp WidgetArray_Destroy
    }
}

// 0x004cb65c EH_Unwind_ListWidget_Ctor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListWidget_Ctor_2(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd34]
        jmp ListLabel_Dtor
    }
}

// 0x004cb680 EH_Unwind_ListWidget_SetItemCount_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ListWidget_SetItemCount_0(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd34]
        jmp ListLabel_Dtor
    }
}

// 0x004cb6a0 EH_Unwind_TextWidget_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_TextWidget_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb6c0 EH_Unwind_TextWidget_CopyCtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_TextWidget_CopyCtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb6e0 EH_Unwind_TextField_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_TextField_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb700 EH_Unwind_ChatInputPanel_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ChatInputPanel_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp WidgetContainer_Dtor
    }
}

// 0x004cb708 EH_Unwind_ChatInputPanel_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ChatInputPanel_Ctor_1(int, int)
{
    __asm {
        push offset ListLabel_Dtor
        push 0x4
        push 0x2a4
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0x10
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb730 EH_Unwind_MessageLinesPanel_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageLinesPanel_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp WidgetContainer_Dtor
    }
}

// 0x004cb738 EH_Unwind_MessageLinesPanel_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageLinesPanel_Ctor_1(int, int)
{
    __asm {
        push offset ListLabel_Dtor
        push 0x4
        push 0x2a4
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0x10
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb760 EH_Unwind_SnowFX_Construct_0 - ../../04_spec/systems/render.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_SnowFX_Construct_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb780 EH_Unwind_Overlay_Dtor_Base_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Overlay_Dtor_Base_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb7a0 EH_Unwind_StaticObj_Dtor_5Widgets_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_StaticObj_Dtor_5Widgets_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        jmp WidgetContainer_Dtor
    }
}

// 0x004cb7a8 EH_Unwind_StaticObj_Dtor_5Widgets_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_StaticObj_Dtor_5Widgets_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        add ecx, 0x28
        jmp Widget_BaseDtor
    }
}

// 0x004cb7c0 EH_Unwind_CompositePanel_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_CompositePanel_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp WidgetContainer_Dtor
    }
}

// 0x004cb7c8 EH_Unwind_CompositePanel_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_CompositePanel_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x28
        jmp Widget_BaseDtor
    }
}

// 0x004cb7d3 EH_Unwind_CompositePanel_Ctor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_CompositePanel_Ctor_2(int, int)
{
    __asm {
        push offset Widget_BaseDtor
        push 0x5
        push 0x4c
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0x70
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb808 EH_Unwind_MessageBoxScreen_Ctor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        add ecx, 0xa978
        jmp ImageWidget_Dtor
    }
}

// 0x004cb816 EH_Unwind_MessageBoxScreen_Ctor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Ctor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        add ecx, 0xaa34
        jmp ListLabel_Dtor
    }
}

// 0x004cb824 EH_Unwind_MessageBoxScreen_Ctor_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Ctor_3(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        add ecx, 0xacd8
        jmp ListLabel_Dtor
    }
}

// 0x004cb832 EH_Unwind_MessageBoxScreen_Ctor_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Ctor_4(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        add ecx, 0xaf7c
        jmp Button_Dtor
    }
}

// 0x004cb840 EH_Unwind_MessageBoxScreen_Ctor_5 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Ctor_5(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        add ecx, 0xb0c8
        jmp Button_Dtor
    }
}

// 0x004cb868 EH_Unwind_MessageBoxScreen_Dtor_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Dtor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0xa978
        jmp ImageWidget_Dtor
    }
}

// 0x004cb876 EH_Unwind_MessageBoxScreen_Dtor_2 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Dtor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0xaa34
        jmp ListLabel_Dtor
    }
}

// 0x004cb884 EH_Unwind_MessageBoxScreen_Dtor_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Dtor_3(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0xacd8
        jmp ListLabel_Dtor
    }
}

// 0x004cb892 EH_Unwind_MessageBoxScreen_Dtor_4 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Dtor_4(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0xaf7c
        jmp Button_Dtor
    }
}

// 0x004cb8a0 EH_Unwind_MessageBoxScreen_Dtor_5 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Dtor_5(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0xb0c8
        jmp Button_Dtor
    }
}

// 0x004cb8c0 EH_Unwind_LineWidget_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_LineWidget_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb8e0 EH_Unwind_AnimImageWidget_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_AnimImageWidget_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ImageWidget_Dtor
    }
}

// 0x004cb900 EH_Unwind_AviWidget_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_AviWidget_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp Widget_BaseDtor
    }
}

// 0x004cb920 EH_Unwind_AviWidget_Open_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_AviWidget_Open_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp + 0x4]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004c86f6 EH_Handler_TextPanel_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 88 48 4d 00 e9 a0 d9 ff ff): MOV EAX,FuncInfo 0x004d4888; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d48a8 (to-state, action) = ['(-1, 0x004c86e0)', '(0, 0x004c86eb)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d48a8[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_TextPanel_LoadFromConfig_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_TextPanel_LoadFromConfig_1)}};
const EhFuncInfo g_EhFuncInfo_004d4888 = {0x19930520u, 2, g_EhUnwindMap_004d48a8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_TextPanel_LoadFromConfig(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d4888
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb208 EH_Handler_ImageWidget_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 f8 73 4d 00 e9 8e ae ff ff): MOV EAX,FuncInfo 0x004d73f8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7418 (to-state, action) = ['(-1, 0x004cb200)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7418[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ImageWidget_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d73f8 = {0x19930520u, 1, g_EhUnwindMap_004d7418, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ImageWidget_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d73f8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb228 EH_Handler_Caret_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 20 74 4d 00 e9 6e ae ff ff): MOV EAX,FuncInfo 0x004d7420; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7440 (to-state, action) = ['(-1, 0x004cb220)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7440[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Caret_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7420 = {0x19930520u, 1, g_EhUnwindMap_004d7440, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Caret_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7420
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb264 EH_Handler_EditField_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 48 74 4d 00 e9 32 ae ff ff): MOV EAX,FuncInfo 0x004d7448; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 3 unwind states, unwind map 0x004d7468 (to-state, action) = ['(-1, 0x004cb240)', '(0, 0x004cb248)', '(1, 0x004cb256)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7468[3] = {{-1, reinterpret_cast<void*>(&EH_Unwind_EditField_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_EditField_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_EditField_Ctor_2)}};
const EhFuncInfo g_EhFuncInfo_004d7448 = {0x19930520u, 3, g_EhUnwindMap_004d7468, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_EditField_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7448
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb294 EH_Handler_EditField_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 80 74 4d 00 e9 02 ae ff ff): MOV EAX,FuncInfo 0x004d7480; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 3 unwind states, unwind map 0x004d74a0 (to-state, action) = ['(-1, 0x004cb270)', '(0, 0x004cb278)', '(1, 0x004cb286)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d74a0[3] = {{-1, reinterpret_cast<void*>(&EH_Unwind_EditField_Dtor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_EditField_Dtor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_EditField_Dtor_2)}};
const EhFuncInfo g_EhFuncInfo_004d7480 = {0x19930520u, 3, g_EhUnwindMap_004d74a0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_EditField_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7480
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb2e0 EH_Handler_Button_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 b8 74 4d 00 e9 b6 ad ff ff): MOV EAX,FuncInfo 0x004d74b8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 5 unwind states, unwind map 0x004d74d8 (to-state, action) = ['(-1, 0x004cb2a0)', '(0, 0x004cb2a8)', '(1, 0x004cb2b6)', '(2, 0x004cb2c4)', '(3, 0x004cb2d2)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d74d8[5] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Button_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_Button_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_Button_Ctor_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_Button_Ctor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_Button_Ctor_4)}};
const EhFuncInfo g_EhFuncInfo_004d74b8 = {0x19930520u, 5, g_EhUnwindMap_004d74d8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Button_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d74b8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb330 EH_Handler_Button_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 00 75 4d 00 e9 66 ad ff ff): MOV EAX,FuncInfo 0x004d7500; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 5 unwind states, unwind map 0x004d7520 (to-state, action) = ['(-1, 0x004cb2f0)', '(0, 0x004cb2f8)', '(1, 0x004cb306)', '(2, 0x004cb314)', '(3, 0x004cb322)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7520[5] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Button_Dtor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_Button_Dtor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_Button_Dtor_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_Button_Dtor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_Button_Dtor_4)}};
const EhFuncInfo g_EhFuncInfo_004d7500 = {0x19930520u, 5, g_EhUnwindMap_004d7520, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Button_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7500
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb398 EH_Handler_Button_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 48 75 4d 00 e9 fe ac ff ff): MOV EAX,FuncInfo 0x004d7548; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 8 unwind states, unwind map 0x004d7568 (to-state, action) = ['(-1, 0x004cb340)', '(-1, 0x004cb34b)', '(-1, 0x004cb356)', '(-1, 0x004cb361)', '(-1, 0x004cb36c)', '(-1, 0x004cb377)', '(-1, 0x004cb382)', '(-1, 0x004cb38d)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7568[8] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_0)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_1)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_2)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_3)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_4)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_5)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_6)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadFromConfig_7)}};
const EhFuncInfo g_EhFuncInfo_004d7548 = {0x19930520u, 8, g_EhUnwindMap_004d7568, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Button_LoadFromConfig(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7548
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb3b8 EH_Handler_Toggle_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 a8 75 4d 00 e9 de ac ff ff): MOV EAX,FuncInfo 0x004d75a8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d75c8 (to-state, action) = ['(-1, 0x004cb3b0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d75c8[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Toggle_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d75a8 = {0x19930520u, 1, g_EhUnwindMap_004d75c8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Toggle_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d75a8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb407 EH_Handler_Button_LoadStatesFromConfig - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 d0 75 4d 00 e9 8f ac ff ff): MOV EAX,FuncInfo 0x004d75d0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 5 unwind states, unwind map 0x004d75f0 (to-state, action) = ['(-1, 0x004cb3d0)', '(-1, 0x004cb3db)', '(-1, 0x004cb3e6)', '(-1, 0x004cb3f1)', '(-1, 0x004cb3fc)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d75f0[5] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadStatesFromConfig_0)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadStatesFromConfig_1)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadStatesFromConfig_2)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadStatesFromConfig_3)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Button_LoadStatesFromConfig_4)}};
const EhFuncInfo g_EhFuncInfo_004d75d0 = {0x19930520u, 5, g_EhUnwindMap_004d75f0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Button_LoadStatesFromConfig(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d75d0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb428 EH_Handler_RadioGroup_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 18 76 4d 00 e9 6e ac ff ff): MOV EAX,FuncInfo 0x004d7618; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7638 (to-state, action) = ['(-1, 0x004cb420)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7638[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_RadioGroup_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7618 = {0x19930520u, 1, g_EhUnwindMap_004d7638, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_RadioGroup_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7618
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb44b EH_Handler_Cycler_SetItemImage - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 40 76 4d 00 e9 4b ac ff ff): MOV EAX,FuncInfo 0x004d7640; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7660 (to-state, action) = ['(-1, 0x004cb440)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7660[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Cycler_SetItemImage_0)}};
const EhFuncInfo g_EhFuncInfo_004d7640 = {0x19930520u, 1, g_EhUnwindMap_004d7660, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Cycler_SetItemImage(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7640
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb46b EH_Handler_ImageList_SetItemImage - bytes ret 0x10 (i,img,..): grow count +0x150 to i+1 capped 20 (0x14), max +0x158; if i within: operator new(0xbc) ImageWidget_Ctor, ImageWidget_SetImage, Widget_AppendChild
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 68 76 4d 00 e9 2b ac ff ff): MOV EAX,FuncInfo 0x004d7668; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7688 (to-state, action) = ['(-1, 0x004cb460)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7688[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ImageList_SetItemImage_0)}};
const EhFuncInfo g_EhFuncInfo_004d7668 = {0x19930520u, 1, g_EhUnwindMap_004d7688, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ImageList_SetItemImage(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7668
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb488 EH_Handler_ScrollGroup_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 90 76 4d 00 e9 0e ac ff ff): MOV EAX,FuncInfo 0x004d7690; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d76b0 (to-state, action) = ['(-1, 0x004cb480)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d76b0[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ScrollGroup_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7690 = {0x19930520u, 1, g_EhUnwindMap_004d76b0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ScrollGroup_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7690
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb4ab EH_Handler_ScrollGroup_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 b8 76 4d 00 e9 eb ab ff ff): MOV EAX,FuncInfo 0x004d76b8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d76d8 (to-state, action) = ['(-1, 0x004cb4a0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d76d8[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ScrollGroup_LoadFromConfig_0)}};
const EhFuncInfo g_EhFuncInfo_004d76b8 = {0x19930520u, 1, g_EhUnwindMap_004d76d8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ScrollGroup_LoadFromConfig(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d76b8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb4c8 EH_Handler_OptionListScreen_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 e0 76 4d 00 e9 ce ab ff ff): MOV EAX,FuncInfo 0x004d76e0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7700 (to-state, action) = ['(-1, 0x004cb4c0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7700[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_OptionListScreen_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d76e0 = {0x19930520u, 1, g_EhUnwindMap_004d7700, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_OptionListScreen_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d76e0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb4eb EH_Handler_ListScreen_CreateRows - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 08 77 4d 00 e9 ab ab ff ff): MOV EAX,FuncInfo 0x004d7708; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7728 (to-state, action) = ['(-1, 0x004cb4e0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7728[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ListScreen_CreateRows_0)}};
const EhFuncInfo g_EhFuncInfo_004d7708 = {0x19930520u, 1, g_EhUnwindMap_004d7728, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ListScreen_CreateRows(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7708
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb5e8 EH_Handler_ListLabel_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 c0 77 4d 00 e9 ae aa ff ff): MOV EAX,FuncInfo 0x004d77c0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d77e0 (to-state, action) = ['(-1, 0x004cb5e0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d77e0[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ListLabel_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d77c0 = {0x19930520u, 1, g_EhUnwindMap_004d77e0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ListLabel_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d77c0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb608 EH_Handler_ListLabel_CopyCtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 e8 77 4d 00 e9 8e aa ff ff): MOV EAX,FuncInfo 0x004d77e8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7808 (to-state, action) = ['(-1, 0x004cb600)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7808[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ListLabel_CopyCtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d77e8 = {0x19930520u, 1, g_EhUnwindMap_004d7808, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ListLabel_CopyCtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d77e8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb628 EH_Handler_ListLabel_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 10 78 4d 00 e9 6e aa ff ff): MOV EAX,FuncInfo 0x004d7810; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7830 (to-state, action) = ['(-1, 0x004cb620)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7830[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ListLabel_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7810 = {0x19930520u, 1, g_EhUnwindMap_004d7830, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ListLabel_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7810
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb667 EH_Handler_ListWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 38 78 4d 00 e9 2f aa ff ff): MOV EAX,FuncInfo 0x004d7838; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 3 unwind states, unwind map 0x004d7858 (to-state, action) = ['(-1, 0x004cb640)', '(0, 0x004cb64b)', '(1, 0x004cb65c)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7858[3] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ListWidget_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_ListWidget_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_ListWidget_Ctor_2)}};
const EhFuncInfo g_EhFuncInfo_004d7838 = {0x19930520u, 3, g_EhUnwindMap_004d7858, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ListWidget_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7838
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb68b EH_Handler_ListWidget_SetItemCount - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 70 78 4d 00 e9 0b aa ff ff): MOV EAX,FuncInfo 0x004d7870; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7890 (to-state, action) = ['(-1, 0x004cb680)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7890[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ListWidget_SetItemCount_0)}};
const EhFuncInfo g_EhFuncInfo_004d7870 = {0x19930520u, 1, g_EhUnwindMap_004d7890, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ListWidget_SetItemCount(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7870
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb6a8 EH_Handler_TextWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 98 78 4d 00 e9 ee a9 ff ff): MOV EAX,FuncInfo 0x004d7898; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d78b8 (to-state, action) = ['(-1, 0x004cb6a0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d78b8[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_TextWidget_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7898 = {0x19930520u, 1, g_EhUnwindMap_004d78b8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_TextWidget_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7898
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb6c8 EH_Handler_TextWidget_CopyCtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 c0 78 4d 00 e9 ce a9 ff ff): MOV EAX,FuncInfo 0x004d78c0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d78e0 (to-state, action) = ['(-1, 0x004cb6c0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d78e0[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_TextWidget_CopyCtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d78c0 = {0x19930520u, 1, g_EhUnwindMap_004d78e0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_TextWidget_CopyCtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d78c0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb6e8 EH_Handler_TextField_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 e8 78 4d 00 e9 ae a9 ff ff): MOV EAX,FuncInfo 0x004d78e8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7908 (to-state, action) = ['(-1, 0x004cb6e0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7908[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_TextField_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d78e8 = {0x19930520u, 1, g_EhUnwindMap_004d7908, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_TextField_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d78e8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb721 EH_Handler_ChatInputPanel_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 10 79 4d 00 e9 75 a9 ff ff): MOV EAX,FuncInfo 0x004d7910; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d7930 (to-state, action) = ['(-1, 0x004cb700)', '(0, 0x004cb708)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7930[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ChatInputPanel_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_ChatInputPanel_Ctor_1)}};
const EhFuncInfo g_EhFuncInfo_004d7910 = {0x19930520u, 2, g_EhUnwindMap_004d7930, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ChatInputPanel_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7910
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb751 EH_Handler_MessageLinesPanel_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 40 79 4d 00 e9 45 a9 ff ff): MOV EAX,FuncInfo 0x004d7940; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d7960 (to-state, action) = ['(-1, 0x004cb730)', '(0, 0x004cb738)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7960[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MessageLinesPanel_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_MessageLinesPanel_Ctor_1)}};
const EhFuncInfo g_EhFuncInfo_004d7940 = {0x19930520u, 2, g_EhUnwindMap_004d7960, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MessageLinesPanel_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7940
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb768 EH_Handler_SnowFX_Construct - ../../04_spec/systems/render.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 70 79 4d 00 e9 2e a9 ff ff): MOV EAX,FuncInfo 0x004d7970; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7990 (to-state, action) = ['(-1, 0x004cb760)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7990[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_SnowFX_Construct_0)}};
const EhFuncInfo g_EhFuncInfo_004d7970 = {0x19930520u, 1, g_EhUnwindMap_004d7990, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_SnowFX_Construct(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7970
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb788 EH_Handler_Overlay_Dtor_Base - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 98 79 4d 00 e9 0e a9 ff ff): MOV EAX,FuncInfo 0x004d7998; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d79b8 (to-state, action) = ['(-1, 0x004cb780)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d79b8[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Overlay_Dtor_Base_0)}};
const EhFuncInfo g_EhFuncInfo_004d7998 = {0x19930520u, 1, g_EhUnwindMap_004d79b8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Overlay_Dtor_Base(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7998
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb7b3 EH_Handler_StaticObj_Dtor_5Widgets - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 c0 79 4d 00 e9 e3 a8 ff ff): MOV EAX,FuncInfo 0x004d79c0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d79e0 (to-state, action) = ['(-1, 0x004cb7a0)', '(0, 0x004cb7a8)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d79e0[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_StaticObj_Dtor_5Widgets_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_StaticObj_Dtor_5Widgets_1)}};
const EhFuncInfo g_EhFuncInfo_004d79c0 = {0x19930520u, 2, g_EhUnwindMap_004d79e0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_StaticObj_Dtor_5Widgets(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d79c0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb7e9 EH_Handler_CompositePanel_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 f0 79 4d 00 e9 ad a8 ff ff): MOV EAX,FuncInfo 0x004d79f0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 3 unwind states, unwind map 0x004d7a10 (to-state, action) = ['(-1, 0x004cb7c0)', '(0, 0x004cb7c8)', '(1, 0x004cb7d3)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7a10[3] = {{-1, reinterpret_cast<void*>(&EH_Unwind_CompositePanel_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_CompositePanel_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_CompositePanel_Ctor_2)}};
const EhFuncInfo g_EhFuncInfo_004d79f0 = {0x19930520u, 3, g_EhUnwindMap_004d7a10, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_CompositePanel_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d79f0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb8c8 EH_Handler_LineWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 c8 7a 4d 00 e9 ce a7 ff ff): MOV EAX,FuncInfo 0x004d7ac8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7ae8 (to-state, action) = ['(-1, 0x004cb8c0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7ae8[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_LineWidget_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7ac8 = {0x19930520u, 1, g_EhUnwindMap_004d7ae8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_LineWidget_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7ac8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb8e8 EH_Handler_AnimImageWidget_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 f0 7a 4d 00 e9 ae a7 ff ff): MOV EAX,FuncInfo 0x004d7af0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7b10 (to-state, action) = ['(-1, 0x004cb8e0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7b10[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_AnimImageWidget_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7af0 = {0x19930520u, 1, g_EhUnwindMap_004d7b10, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_AnimImageWidget_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7af0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb908 EH_Handler_AviWidget_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 18 7b 4d 00 e9 8e a7 ff ff): MOV EAX,FuncInfo 0x004d7b18; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7b38 (to-state, action) = ['(-1, 0x004cb900)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7b38[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_AviWidget_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d7b18 = {0x19930520u, 1, g_EhUnwindMap_004d7b38, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_AviWidget_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7b18
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb92b EH_Handler_AviWidget_Open - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 40 7b 4d 00 e9 6b a7 ff ff): MOV EAX,FuncInfo 0x004d7b40; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d7b60 (to-state, action) = ['(-1, 0x004cb920)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7b60[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_AviWidget_Open_0)}};
const EhFuncInfo g_EhFuncInfo_004d7b40 = {0x19930520u, 1, g_EhUnwindMap_004d7b60, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_AviWidget_Open(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7b40
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x0041ebd0 Thunk_0041ebe0_0041ebd0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_0041ebe0_0041ebd0(int, int)
{
    __asm {
        jmp StaticInit_ZeroList_004f3340
    }
}


// 0x0041ec00 Thunk_0041ec10_0041ec00 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_0041ec10_0041ec00(int, int)
{
    __asm {
        jmp StaticInit_ZeroList_004f3a78
    }
}


// 0x0041ebe0 StaticInit_ZeroList_004f3340 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticInit_ZeroList_004f3340(int, int)
{
    __asm {
        xor eax, eax
        mov dword ptr [g_Data_004da000 + 0x19340], eax
        mov dword ptr [g_Data_004da000 + 0x19348], eax
        mov dword ptr [g_Data_004da000 + 0x19344], eax
        mov dword ptr [g_Data_004da000 + 0x1934c], eax
        ret
    }
}

// 0x0041ec10 StaticInit_ZeroList_004f3a78 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticInit_ZeroList_004f3a78(int, int)
{
    __asm {
        xor eax, eax
        mov dword ptr [g_Data_004da000 + 0x19a78], eax
        mov dword ptr [g_Data_004da000 + 0x19a80], eax
        mov dword ptr [g_Data_004da000 + 0x19a7c], eax
        mov dword ptr [g_Data_004da000 + 0x19a84], eax
        ret
    }
}

// 0x004b7290 Toggle_Flip - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Toggle_Flip(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xc4]
        test eax, eax
        jz L_4b72b7
        mov edx, dword ptr [esi + 0x14c]
        xor eax, eax
        test edx, edx
        setz AL
        push eax
        call Toggle_SetState
        mov ecx, esi
        call Button_Activate
    L_4b72b7:
        pop esi
        ret
    }
}

// 0x004bc4c0 Widget_Slot_Virtual08_Then00498fb0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_Virtual08_Then00498fb0(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x8]
        mov ecx, dword ptr [esi + 0x3c]
        mov edx, dword ptr [esi + 0x34]
        push 0x0
        push ecx
        mov ecx, dword ptr [esi + 0x14]
        push edx
        mov edx, dword ptr [esi + 0x18]
        call Draw_CircleMidpoint
        pop esi
        ret
    }
}

// 0x004c5f33 DataTarget_004c5f33 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_004c5f33(int, int)
{
    __asm {
        mov edi, dword ptr [ebp + 0xc]
        mov esi, dword ptr [ebp + 0x8]
        jmp ArrayDtor_Eh2_Unwind         // the original falls through into 0x004c5f39
    }
}

// 0x004c5fb0 DataTarget_004c5fb0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_004c5fb0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x14]
        push eax
        call Seh_CxxExceptionFilter_004c5fe0
        add esp, 0x4
        ret
    }
}

// 0x004c5fbd DataTarget_004c5fbd - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall DataTarget_004c5fbd(int, int, int, int, int, int)
{
    __asm {
        mov esp, dword ptr [ebp - 0x18]
        mov dword ptr [ebp - 0x4], 0xffffffff
        mov ecx, dword ptr [ebp - 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x10
    }
}

// 0x004c606f DataTarget_004c606f - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_004c606f(int, int)
{
    __asm {
        mov ebx, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp + 0x8]
        mov esi, dword ptr [ebp - 0x1c]
        jmp ArrayCtor_Eh_Unwind          // the original falls through into 0x004c6078
    }
}

// 0x00423440 Widget_Slot_ThunkMember34To0048ea20 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Widget_Slot_ThunkMember34To0048ea20(int, int)
{
    __asm {
        mov ecx, dword ptr [ecx + 0x34]
        jmp Unnamed_0048ea20
    }
}

// 0x00435140 UiScreen_Slot_Activate_00434fb0_1 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Activate_00434fb0_1(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [ecx + 0xc8]
        call Button_Activate
        push 0x1
        mov ecx, esi
        call SaveLoadDialog_DeleteSelected
        pop esi
        ret
    }
}

// 0x00435200 UiScreen_Slot_Call00435240IfOwner_ThenActivate - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall UiScreen_Slot_Call00435240IfOwner_ThenActivate(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 0xc8]
        test ecx, ecx
        jz L_435212
        call SaveGameScreen_Save
    L_435212:
        mov ecx, esi
        call Button_Activate
        pop esi
        ret
    }
}

// 0x004b9540 ScreenBase_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScreenBase_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ScreenBase_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push ebx
        push esi
        push edi
        mov esi, ecx
        push 0x1
        mov dword ptr [esp + 0x10], esi
        call ScreenRoot_Ctor
        xor edi, edi
        lea ebx, [esi + 0x44]
        push 0x1
        push edi
        mov ecx, ebx
        mov dword ptr [esp + 0x20], edi
        call AnimImageWidget_Ctor
        mov dword ptr [ebx], offset g_RData_004cc000 + 0x7a08
        push offset ImageWidget_Dtor
        push offset Menus_Slot_Call4b3d00_0040f2d0
        push 0x14
        lea eax, [esi + 0x11c]
        push 0xbc
        push eax
        mov byte ptr [esp + 0x2c], 0x1
        call ArrayCtor_Eh
        push offset AviWidget_Dtor
        push offset UiWidget_Subclass_Ctor_004bfc80
        push 0xa
        lea ecx, [esi + 0xfcc]
        push 0x144
        push ecx
        mov byte ptr [esp + 0x2c], 0x2
        call ArrayCtor_Eh
        push offset UiObject_ClearFirstWord_004ba4c0
        push offset UiObject_ResetFields_004ba4a0
        push 0x14
        lea edx, [esi + 0x1cec]
        push 0x24
        push edx
        mov byte ptr [esp + 0x2c], 0x3
        call ArrayCtor_Eh
        push offset ListLabel_Dtor
        push offset ListLabel_Subclass_Ctor_004ba020
        push 0x32
        lea eax, [esi + 0x1fbc]
        push 0x2c0
        push eax
        mov byte ptr [esp + 0x2c], 0x4
        call ArrayCtor_Eh
        mov dword ptr [esi], offset g_RData_004cc000 + 0x79f8
        mov dword ptr [esi + 0x114], edi
        mov dword ptr [esi + 0x118], edi
        lea eax, [esi + 0x1c78]
        mov ecx, 0xa
    L_4b9621:
        mov dword ptr [eax - 0x4], edi
        mov dword ptr [eax], 0x3f800000
        mov dword ptr [eax + 0x4], edi
        add eax, 0xc
        dec ecx
        jnz L_4b9621
        mov ecx, offset g_Data_004da000 + 0x69c
        mov dword ptr [esp + 0xc], 0x5
        call Settings_FindNodeByName
        cmp eax, edi
        jnz L_4b964d
        lea eax, [esp + 0xc]
    L_4b964d:
        mov eax, dword ptr [eax]
        add eax, -0x2
        cmp eax, 0x5
        ja L_4b96d6
        cmp eax, 0
        je L_4b965e
        cmp eax, 1
        je L_4b9682
        cmp eax, 2
        je L_4b965e
        cmp eax, 3
        je L_4b9682
        cmp eax, 4
        je L_4b96a2
        cmp eax, 5
        je L_4b96c6
        int 3  // unreachable: the bounds check above excludes other indices
    L_4b965e:
        mov dword ptr [esi + 0xa944], edi
        mov dword ptr [esi + 0xa948], 0xffffffd8
        mov eax, esi
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        add esp, 0x10
        ret
    L_4b9682:
        mov dword ptr [esi + 0xa944], edi
        mov dword ptr [esi + 0xa948], edi
        mov eax, esi
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        add esp, 0x10
        ret
    L_4b96a2:
        mov dword ptr [esi + 0xa944], edi
        mov dword ptr [esi + 0xa948], 0x3c
        mov eax, esi
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        add esp, 0x10
        ret
    L_4b96c6:
        mov dword ptr [esi + 0xa944], edi
        mov dword ptr [esi + 0xa948], 0x90
    L_4b96d6:
        mov ecx, dword ptr [esp + 0x10]
        mov eax, esi
        pop edi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x004b9740 ScreenBase_scalar_deleting_dtor - bytes ret 4: ScreenBase_Dtor 0x004b9760; flag&1 -> operator delete
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenBase_scalar_deleting_dtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ScreenBase_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4b9758
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4b9758:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004b9760 ScreenBase_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScreenBase_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ScreenBase_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x79f8
        mov ecx, dword ptr [esi + 0x114]
        mov dword ptr [esp + 0x10], 0x4
        test ecx, ecx
        jz L_4b97a0
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0x114], eax
    L_4b97a0:
        mov ecx, dword ptr [esi + 0x118]
        test ecx, ecx
        jz L_4b97b5
        call Image_FreeUnlessDefault
        mov dword ptr [esi + 0x118], eax
    L_4b97b5:
        push offset ListLabel_Dtor
        push 0x32
        lea eax, [esi + 0x1fbc]
        push 0x2c0
        push eax
        call ArrayDtor_Eh2
        push offset UiObject_ClearFirstWord_004ba4c0
        push 0x14
        lea ecx, [esi + 0x1cec]
        push 0x24
        push ecx
        mov byte ptr [esp + 0x20], 0x3
        call ArrayDtor_Eh2
        push offset AviWidget_Dtor
        push 0xa
        lea edx, [esi + 0xfcc]
        push 0x144
        push edx
        mov byte ptr [esp + 0x20], 0x2
        call ArrayDtor_Eh2
        push offset ImageWidget_Dtor
        push 0x14
        lea eax, [esi + 0x11c]
        push 0xbc
        push eax
        mov byte ptr [esp + 0x20], 0x1
        call ArrayDtor_Eh2
        lea ecx, [esi + 0x44]
        mov byte ptr [esp + 0x10], 0x0
        call AnimImageWidget_Dtor
        mov ecx, esi
        mov dword ptr [esp + 0x10], 0xffffffff
        call ScreenRoot_Dtor
        mov ecx, dword ptr [esp + 0x8]
        pop esi
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    }
}

// 0x004b98d0 ScreenBase_LoadConfigFile - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ScreenBase_LoadConfigFile(int, int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 0x8]
        push 0x0
        xor edx, edx
        call ConfigTree_ParseFileByBasename
        mov ecx, dword ptr [esp + 0x10]
        mov edx, dword ptr [esp + 0xc]
        push ecx
        push edx
        push eax
        mov ecx, esi
        mov dword ptr [esi + 0xa93c], eax
        call ScreenBase_LoadFromConfig
        pop esi
        ret 0xc
    }
}

// 0x004b9900 ScreenBase_LoadFromConfig - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ScreenBase_LoadFromConfig(int, int, int, int, int)
{
    __asm {
        sub esp, 0xc
        push ebx
        push ebp
        push esi
        push edi
        mov ebp, ecx
        mov dword ptr [esp + 0x14], 0x0
        call zVideo_NotifyScreenResizeFromSurface
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        jnz L_4b992e
        mov ecx, 0x1
        call zVideo_CaptureSurfaceToImage
        mov dword ptr [ebp + 0x114], eax
    L_4b992e:
        mov esi, dword ptr [esp + 0x20]
        test esi, esi
        jz L_4ba005
        mov edx, offset g_Data_004da000 + 0xa824
        mov ecx, esi
        mov dword ptr [esp + 0x14], esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b9959
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        call TextureManager_AddSearchPaths
    L_4b9959:
        mov edx, dword ptr [esp + 0x24]
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [ebp + 0xa940], eax
        jz L_4ba005
        mov edx, offset g_Data_004da000 + 0x5c04
        mov ecx, eax
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b998d
        mov ecx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [ecx + 0xc]
        call TextureManager_AddSearchPaths
    L_4b998d:
        mov ecx, dword ptr [ebp + 0xa940]
        mov edx, offset g_Data_004da000 + 0xdb4
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [esp + 0x10], eax
        jz L_4b9ba6
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0x4]
        cmp ecx, 0x14
        mov edx, ecx
        jl L_4b99bb
        mov edx, 0x14
    L_4b99bb:
        mov ecx, 0x1
        mov dword ptr [esp + 0x24], edx
        cmp edx, ecx
        mov dword ptr [esp + 0x20], ecx
        jle L_4b9ba6
        jmp L_4b99d6
    L_4b99d2:
        mov eax, dword ptr [esp + 0x10]
    L_4b99d6:
        mov eax, dword ptr [eax + 0x4]
        mov edx, dword ptr [eax + ecx*0x8 + 0x4]
        lea ecx, [eax + ecx*0x8]
        mov esi, dword ptr [edx + 0xc]
        lea eax, [esi + esi*0x8]
        lea edx, [ebp + eax*0x4]
        mov eax, 0x1
        mov dword ptr [edx + 0x1cec], eax
        mov dword ptr [edx + 0x1cfc], 0x0
        mov dword ptr [edx + 0x1d00], eax
        mov eax, dword ptr [ecx + 0x4]
        mov eax, dword ptr [eax + 0x14]
        mov dword ptr [edx + 0x1cf0], eax
        mov eax, dword ptr [ecx + 0x4]
        mov eax, dword ptr [eax + 0x1c]
        mov dword ptr [edx + 0x1cf4], eax
        mov eax, dword ptr [ecx + 0x4]
        mov edi, dword ptr [eax + 0x24]
        cmp dword ptr [edi + 0x8], 0x4
        jnz L_4b9a78
        mov edi, dword ptr [edi + 0xc]
        xor eax, eax
        xor ebx, ebx
        add esi, 0xce
        mov AH, byte ptr [edi + 0x1c]
        mov BL, byte ptr [edi + 0xc]
        mov AL, byte ptr [edi + 0x14]
        lea esi, [esi + esi*0x8]
        shl eax, 0x8
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + esi*0x4], eax
        mov eax, dword ptr [ecx + 0x4]
        mov eax, dword ptr [eax + 0x24]
        mov esi, dword ptr [eax + 0x14]
        xor eax, eax
        mov AH, byte ptr [esi + 0x1c]
        mov BL, byte ptr [esi + 0xc]
        mov AL, byte ptr [esi + 0x14]
        shl eax, 0x8
        or eax, ebx
        mov dword ptr [edx + 0x1cfc], eax
        mov dword ptr [edx + 0x1d00], 0x2
        jmp L_4b9a97
    L_4b9a78:
        xor eax, eax
        xor ebx, ebx
        mov AH, byte ptr [edi + 0x1c]
        mov BL, byte ptr [edi + 0xc]
        mov AL, byte ptr [edi + 0x14]
        shl eax, 0x8
        or eax, ebx
        add esi, 0xce
        lea esi, [esi + esi*0x8]
        mov dword ptr [ebp + esi*0x4], eax
    L_4b9a97:
        mov eax, dword ptr [ecx + 0x4]
        cmp dword ptr [eax + 0x4], 0x6
        jl L_4b9aa9
        mov eax, dword ptr [eax + 0x2c]
        mov dword ptr [edx + 0x1d04], eax
    L_4b9aa9:
        mov eax, dword ptr [ecx + 0x4]
        cmp dword ptr [eax + 0x4], 0x7
        jl L_4b9abb
        mov eax, dword ptr [eax + 0x34]
        mov dword ptr [edx + 0x1d08], eax
    L_4b9abb:
        mov ecx, dword ptr [ecx + 0x4]
        cmp dword ptr [ecx + 0x4], 0x8
        jl L_4b9b87
        mov ecx, dword ptr [ecx + 0x3c]
        mov edi, offset g_Data_004da000 + 0x6968
        mov esi, ecx
    L_4b9ad2:
        mov BL, byte ptr [esi]
        mov AL, BL
        cmp BL, byte ptr [edi]
        jnz L_4b9af6
        test AL, AL
        jz L_4b9af2
        mov BL, byte ptr [esi + 0x1]
        mov AL, BL
        cmp BL, byte ptr [edi + 0x1]
        jnz L_4b9af6
        add esi, 0x2
        add edi, 0x2
        test AL, AL
        jnz L_4b9ad2
    L_4b9af2:
        xor eax, eax
        jmp L_4b9afb
    L_4b9af6:
        sbb eax, eax
        sbb eax, -0x1
    L_4b9afb:
        test eax, eax
        jz L_4b9b87
        mov edi, offset g_Data_004da000 + 0x6960
        mov esi, ecx
    L_4b9b0a:
        mov BL, byte ptr [esi]
        mov AL, BL
        cmp BL, byte ptr [edi]
        jnz L_4b9b2e
        test AL, AL
        jz L_4b9b2a
        mov BL, byte ptr [esi + 0x1]
        mov AL, BL
        cmp BL, byte ptr [edi + 0x1]
        jnz L_4b9b2e
        add esi, 0x2
        add edi, 0x2
        test AL, AL
        jnz L_4b9b0a
    L_4b9b2a:
        xor eax, eax
        jmp L_4b9b33
    L_4b9b2e:
        sbb eax, eax
        sbb eax, -0x1
    L_4b9b33:
        test eax, eax
        jnz L_4b9b43
        mov dword ptr [edx + 0x1d0c], 0x2
        jmp L_4b9b91
    L_4b9b43:
        mov edi, offset g_Data_004da000 + 0xa81c
        mov esi, ecx
    L_4b9b4a:
        mov CL, byte ptr [esi]
        mov BL, byte ptr [edi]
        mov AL, CL
        cmp CL, BL
        jnz L_4b9b72
        test AL, AL
        jz L_4b9b6e
        mov CL, byte ptr [esi + 0x1]
        mov BL, byte ptr [edi + 0x1]
        mov AL, CL
        cmp CL, BL
        jnz L_4b9b72
        add esi, 0x2
        add edi, 0x2
        test AL, AL
        jnz L_4b9b4a
    L_4b9b6e:
        xor eax, eax
        jmp L_4b9b77
    L_4b9b72:
        sbb eax, eax
        sbb eax, -0x1
    L_4b9b77:
        test eax, eax
        jnz L_4b9b91
        mov dword ptr [edx + 0x1d0c], 0x1
        jmp L_4b9b91
    L_4b9b87:
        mov dword ptr [edx + 0x1d0c], 0x0
    L_4b9b91:
        mov ecx, dword ptr [esp + 0x20]
        mov eax, dword ptr [esp + 0x24]
        inc ecx
        cmp ecx, eax
        mov dword ptr [esp + 0x20], ecx
        jl L_4b99d2
    L_4b9ba6:
        mov ecx, dword ptr [ebp + 0xa940]
        mov edx, offset g_Data_004da000 + 0xa808
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [esp + 0x20], eax
        jz L_4b9c7c
        mov edx, eax
        mov eax, dword ptr [edx + 0x4]
        mov eax, dword ptr [eax + 0x4]
        cmp eax, 0x14
        mov dword ptr [esp + 0x24], eax
        jl L_4b9bdb
        mov dword ptr [esp + 0x24], 0x14
    L_4b9bdb:
        mov eax, dword ptr [esp + 0x24]
        mov ebx, 0x1
        cmp eax, ebx
        jle L_4b9c7c
        lea esi, [ebp + 0x11c]
    L_4b9bf2:
        mov ecx, dword ptr [ebp + 0xa944]
        mov eax, dword ptr [esp + 0x20]
        mov edx, dword ptr [ebp + 0xa948]
        mov dword ptr [esp + 0x18], ecx
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x10], edx
        mov edx, dword ptr [ecx + ebx*0x8 + 0x4]
        lea edi, [ecx + ebx*0x8]
        mov ecx, esi
        mov eax, dword ptr [edx + 0xc]
        push eax
        call ImageWidget_SetImage
        mov edi, dword ptr [edi + 0x4]
        cmp dword ptr [edi + 0x4], 0x4
        jl L_4b9c43
        mov eax, dword ptr [edi + 0x1c]
        mov ecx, dword ptr [esp + 0x10]
        mov edx, dword ptr [esi]
        add eax, ecx
        mov ecx, dword ptr [edi + 0x14]
        mov edi, dword ptr [esp + 0x18]
        add ecx, edi
        push eax
        push ecx
        mov ecx, esi
        call dword ptr [edx + 0xc]
    L_4b9c43:
        xor edx, edx
        push 0x1
        mov DL, byte ptr [esi + 0xc]
        mov ecx, esi
        and edx, 0x10
        or edx, 0x2
        mov dword ptr [esi + 0xc], edx
        mov eax, dword ptr [esi]
        call dword ptr [eax + 0x60]
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x20]
        push esi
        mov ecx, ebp
        call Widget_AppendChild
        mov eax, dword ptr [esp + 0x24]
        inc ebx
        add esi, 0xbc
        cmp ebx, eax
        jl L_4b9bf2
    L_4b9c7c:
        mov ecx, dword ptr [ebp + 0xa940]
        mov edx, offset g_Data_004da000 + 0xa7f4
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [esp + 0x24], eax
        jz L_4b9d8f
        mov eax, dword ptr [eax + 0x4]
        mov eax, dword ptr [eax + 0x4]
        cmp eax, 0xa
        jl L_4b9ca8
        mov eax, 0xa
    L_4b9ca8:
        mov ebx, 0x1
        mov dword ptr [esp + 0x20], eax
        cmp eax, ebx
        jle L_4b9d8f
        lea esi, [ebp + 0xfcc]
    L_4b9cbf:
        mov ecx, dword ptr [ebp + 0xa944]
        mov eax, dword ptr [esp + 0x24]
        mov edx, dword ptr [ebp + 0xa948]
        mov dword ptr [esp + 0x10], ecx
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x18], edx
        mov edx, dword ptr [ecx + ebx*0x8 + 0x4]
        lea edi, [ecx + ebx*0x8]
        mov ecx, esi
        mov eax, dword ptr [edx + 0xc]
        push eax
        call AviWidget_Open
        mov eax, dword ptr [edi + 0x4]
        cmp dword ptr [eax + 0x4], 0x4
        jl L_4b9d0e
        mov ecx, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add ecx, dword ptr [esp + 0x18]
        mov edx, dword ptr [esi]
        push ecx
        mov ecx, dword ptr [esp + 0x14]
        add eax, ecx
        mov ecx, esi
        push eax
        call dword ptr [edx + 0xc]
    L_4b9d0e:
        mov edi, dword ptr [edi + 0x4]
        cmp dword ptr [edi + 0x4], 0x5
        jl L_4b9d31
        mov eax, dword ptr [edi + 0x24]
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [eax + 0x14]
        mov eax, dword ptr [eax + 0x1c]
        push eax
        call Pixel_FromRGBBytes
        push eax
        mov ecx, esi
        call AviWidget_SetColourKey
    L_4b9d31:
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x20]
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x64]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x68]
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x64]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x68]
        mov eax, dword ptr [ebp + 0x114]
        mov edx, dword ptr [esi]
        push 0x0
        push eax
        mov ecx, esi
        call dword ptr [edx + 0x18]
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x74]
        push esi
        mov ecx, ebp
        call Widget_AppendChild
        mov eax, dword ptr [esp + 0x20]
        inc ebx
        add esi, 0x144
        cmp ebx, eax
        jl L_4b9cbf
    L_4b9d8f:
        mov ecx, dword ptr [ebp + 0xa940]
        mov edx, offset g_Data_004da000 + 0xa7e4
        call ConfigTree_FindChild
        test eax, eax
        mov dword ptr [esp + 0x20], eax
        jz L_4b9ed7
        mov ecx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [ecx + 0x4]
        cmp ecx, 0x32
        mov edx, ecx
        jl L_4b9dbd
        mov edx, 0x32
    L_4b9dbd:
        mov ecx, 0x1
        mov dword ptr [esp + 0x10], edx
        cmp edx, ecx
        mov dword ptr [esp + 0x24], ecx
        jle L_4b9ed7
        lea esi, [ebp + 0x1fbc]
        jmp L_4b9dde
    L_4b9dda:
        mov eax, dword ptr [esp + 0x20]
    L_4b9dde:
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + ecx*0x8 + 0x4]
        mov ebx, dword ptr [esi]
        lea edi, [edx + ecx*0x8]
        mov ecx, dword ptr [eax + 0xc]
        call Call004a5bf0_If004a5b20
        push eax
        push esi
        call dword ptr [ebx + 0x74]
        mov eax, dword ptr [edi + 0x4]
        mov ebx, dword ptr [ebp + 0xa948]
        mov edx, dword ptr [esi]
        add esp, 0x8
        mov ecx, dword ptr [eax + 0x1c]
        mov eax, dword ptr [eax + 0x14]
        add ecx, ebx
        push ecx
        mov ecx, dword ptr [ebp + 0xa944]
        add eax, ecx
        mov ecx, esi
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [edi + 0x4]
        mov eax, dword ptr [ecx + 0x24]
        lea edx, [eax + eax*0x8]
        mov edi, dword ptr [ebp + edx*0x4 + 0x1cec]
        lea eax, [ebp + edx*0x4 + 0x1cec]
        neg edi
        sbb edi, edi
        and edi, eax
        test edi, edi
        jz L_4b9ea7
        mov eax, dword ptr [edi + 0x20]
        push 0x2
        mov dword ptr [esi + 0x144], eax
        mov eax, dword ptr [edi + 0x1c]
        mov ecx, dword ptr [edi + 0x8]
        mov edx, dword ptr [esi]
        push 0x0
        push 0x0
        push 0x0
        push eax
        mov eax, dword ptr [edi + 0x4]
        push ecx
        push eax
        mov ecx, esi
        call dword ptr [edx + 0x80]
        mov eax, dword ptr [edi + 0xc]
        mov ecx, 0x1
        mov dword ptr [esi + 0x14c], eax
        mov dword ptr [esi + 0x150], eax
        mov dword ptr [esi + 0x270], ecx
        mov edx, dword ptr [edi + 0x18]
        mov dword ptr [esi + 0x264], edx
        mov dword ptr [esi + 0x29c], ecx
        mov dword ptr [esi + 0x2a0], ecx
        mov eax, dword ptr [edi + 0x10]
        mov edx, dword ptr [edi + 0x14]
        mov dword ptr [esi + 0x26c], eax
        mov dword ptr [esi + 0x268], edx
        jmp L_4b9eac
    L_4b9ea7:
        mov ecx, 0x1
    L_4b9eac:
        mov eax, dword ptr [esi]
        push ecx
        mov ecx, esi
        call dword ptr [eax + 0x60]
        push esi
        mov ecx, ebp
        call Widget_AppendChild
        mov ecx, dword ptr [esp + 0x24]
        mov eax, dword ptr [esp + 0x10]
        inc ecx
        add esi, 0x2c0
        cmp ecx, eax
        mov dword ptr [esp + 0x24], ecx
        jl L_4b9dda
    L_4b9ed7:
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        jnz L_4b9f13
        mov edx, dword ptr [ebp]
        push 0x1
        mov ecx, ebp
        call dword ptr [edx + 0x4]
        mov eax, dword ptr [ebp]
        push 0x0
        mov ecx, ebp
        call dword ptr [eax]
        mov ecx, 0x1
        call zVideo_CaptureSurfaceToImage
        mov edx, dword ptr [ebp]
        push 0x0
        mov ecx, ebp
        mov dword ptr [ebp + 0x118], eax
        call dword ptr [edx + 0x4]
        mov ecx, ebp
        call ScreenBase_DrawBackground
    L_4b9f13:
        mov ecx, dword ptr [ebp + 0xa940]
        mov edx, offset g_Data_004da000 + 0xa7dc
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4b9f95
        mov edx, offset g_Data_004da000 + 0xa708
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b9f53
        mov eax, dword ptr [eax + 0x4]
        mov edx, dword ptr [ebp + 0x44]
        lea edi, [ebp + 0x44]
        mov ecx, dword ptr [eax + 0xc]
        push ecx
        mov ecx, edi
        call dword ptr [edx + 0x7c]
        push edi
        mov ecx, ebp
        call Widget_SetField10
    L_4b9f53:
        mov edx, offset g_Data_004da000 + 0xa81c
        mov ecx, esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_4b9f6f
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0xc]
        mov dword ptr [ebp + 0x8c], eax
    L_4b9f6f:
        lea ecx, [esp + 0x28]
        mov edx, offset g_Data_004da000 + 0xa7d4
        push ecx
        mov ecx, esi
        mov dword ptr [esp + 0x2c], 0x1
        call ConfigTree_GetIntValue
        mov edx, dword ptr [esp + 0x28]
        lea ecx, [ebp + 0x44]
        push edx
        call AnimImageWidget_SetEnabled
    L_4b9f95:
        mov ecx, dword ptr [ebp + 0xa940]
        mov edx, offset g_Data_004da000 + 0xa7c0
        call ConfigTree_FindChild
        mov edi, eax
        test edi, edi
        jz L_4ba005
        mov eax, dword ptr [edi + 0x4]
        mov ebx, dword ptr [eax + 0x4]
        cmp ebx, 0xa
        jl L_4b9fbb
        mov ebx, 0xa
    L_4b9fbb:
        mov esi, 0x1
        cmp ebx, esi
        jle L_4ba005
        add ebp, 0x1c78
    L_4b9fca:
        mov ecx, dword ptr [edi + 0x4]
        mov dword ptr [esp + 0x28], 0x3f800000
        lea eax, [ecx + esi*0x8]
        mov eax, dword ptr [ecx + esi*0x8 + 0x4]
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        cmp edx, 0x3
        jl L_4b9fee
        mov edx, dword ptr [eax + 0x14]
        mov dword ptr [esp + 0x28], edx
    L_4b9fee:
        call Sound_ResolveResourceByName
        mov dword ptr [ebp - 0x4], eax
        mov eax, dword ptr [esp + 0x28]
        mov dword ptr [ebp], eax
        inc esi
        add ebp, 0xc
        cmp esi, ebx
        jl L_4b9fca
    L_4ba005:
        call zVideo_CallSurfaceSlot3C0_Front
        mov eax, dword ptr [esp + 0x14]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0xc
    }
}

// 0x004bf060 MessageBoxScreen_Ctor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall MessageBoxScreen_Ctor(int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MessageBoxScreen_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x10
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x14], esi
        call ScreenBase_Ctor
        xor ebx, ebx
        lea ecx, [esi + 0xa978]
        push ebx
        mov dword ptr [esp + 0x2c], ebx
        call ImageWidget_Ctor
        push ebx
        push ebx
        lea ecx, [esi + 0xaa34]
        push ebx
        mov byte ptr [esp + 0x34], 0x1
        call ListLabel_Ctor
        push ebx
        push ebx
        lea ecx, [esi + 0xacd8]
        push ebx
        mov byte ptr [esp + 0x34], 0x2
        call ListLabel_Ctor
        lea edi, [esi + 0xaf7c]
        mov byte ptr [esp + 0x28], 0x3
        mov ecx, edi
        call Button_Ctor
        mov dword ptr [edi], offset g_RData_004cc000 + 0x80c8
        lea ebp, [esi + 0xb0c8]
        mov byte ptr [esp + 0x28], 0x4
        mov ecx, ebp
        call Button_Ctor
        mov dword ptr [ebp], offset g_RData_004cc000 + 0x8040
        mov byte ptr [esp + 0x28], 0x5
        mov dword ptr [esi], offset g_RData_004cc000 + 0x8028
        call zVideo_GetSurfaceRect
        mov edx, dword ptr [eax]
        lea ecx, [esi + 0xa94c]
        mov dword ptr [esi + 0xa94c], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov edx, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], edx
        mov eax, dword ptr [eax + 0xc]
        mov dword ptr [ecx + 0xc], eax
        mov eax, dword ptr [esp + 0x30]
        cmp eax, ebx
        jz L_4bf1cd
        mov ecx, dword ptr [esp + 0x34]
        cmp ecx, ebx
        jz L_4bf1cd
        push ebx
        push ecx
        push eax
        mov ecx, esi
        mov dword ptr [esi + 0xa96c], ebx
        mov dword ptr [esi + 0xa970], ebx
        mov dword ptr [esi + 0xa974], ebx
        call ScreenBase_LoadConfigFile
        mov ebx, eax
        test ebx, ebx
        jz L_4bf1a6
        push offset g_Data_004da000 + 0xa8b8
        push edi
        push ebx
        mov ecx, esi
        call ScreenBase_BindButton
        push offset g_Data_004da000 + 0xa8ac
        push ebp
        push ebx
        mov ecx, esi
        call ScreenBase_BindButton
        lea eax, [esi + 0xacd8]
        push offset g_Data_004da000 + 0xa8a4
        push eax
        push ebx
        mov ecx, esi
        call ScreenBase_BindWidget
        lea eax, [esi + 0xaa34]
        push offset g_Data_004da000 + 0xa89c
        push eax
        push ebx
        mov ecx, esi
        call ScreenBase_BindWidget
        push ebx
        mov ecx, esi
        call ScreenBase_CloseConfig
    L_4bf1a6:
        mov edx, dword ptr [esi + 0xacd8]
        lea ecx, [esi + 0xacd8]
        push 0x1
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [esi + 0xaa34]
        lea ecx, [esi + 0xaa34]
        push 0x1
        call dword ptr [eax + 0x60]
        jmp L_4bf51f
    L_4bf1cd:
        mov eax, dword ptr [esi + 0xa954]
        push 0x80
        cdq
        sub eax, edx
        mov dword ptr [esi + 0xa964], 0x12c
        sar eax, 0x1
        sub eax, 0x96
        mov dword ptr [esi + 0xa968], 0xc8
        mov dword ptr [esp + 0x1c], eax
        mov eax, dword ptr [esi + 0xa958]
        cdq
        sub eax, edx
        mov DL, 0x80
        mov ebp, eax
        mov CL, DL
        sar ebp, 0x1
        sub ebp, 0x64
        call Pixel_FromRGBBytes
        mov ecx, dword ptr [esi + 0xa968]
        mov edx, dword ptr [esi + 0xa964]
        mov dword ptr [esp + 0x10], eax
        mov dword ptr [esp + 0x30], ecx
        mov dword ptr [esp + 0x34], edx
        call Image_Alloc
        mov ebx, eax
        mov DL, 0x1
        mov ecx, ebx
        call Image_SetFlags
        mov eax, dword ptr [esp + 0x30]
        mov edx, dword ptr [esp + 0x34]
        push eax
        mov ecx, ebx
        call Image_SetSize
        mov ecx, ebx
        call Image_BytesPerPixel
        imul eax, dword ptr [esp + 0x30]
        imul eax, dword ptr [esp + 0x34]
        push eax
        call dword ptr [g_Iat_malloc_004cc5dc]
        add esp, 0x4
        mov edx, eax
        mov ecx, ebx
        push 0x0
        call Image_SetPixelsAndAlpha
        mov ecx, dword ptr [ebx]
        xor eax, eax
        test ecx, ecx
        jle L_4bf28c
    L_4bf279:
        mov ecx, dword ptr [ebx + 0x10]
        mov DX, word ptr [esp + 0x10]
        mov word ptr [ecx + eax*0x2], DX
        mov ecx, dword ptr [ebx]
        inc eax
        cmp eax, ecx
        jl L_4bf279
    L_4bf28c:
        mov DL, 0xc0
        push 0xc0
        mov CL, DL
        mov dword ptr [esi + 0xa96c], ebx
        call Pixel_FromRGBBytes
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esi + 0xa968]
        cdq
        and edx, 0x3
        add eax, edx
        sar eax, 0x2
        mov dword ptr [esp + 0x30], eax
        mov eax, dword ptr [esi + 0xa964]
        cdq
        and edx, 0x3
        add eax, edx
        sar eax, 0x2
        mov dword ptr [esp + 0x34], eax
        call Image_Alloc
        mov ebx, eax
        mov DL, 0x1
        mov ecx, ebx
        call Image_SetFlags
        mov eax, dword ptr [esp + 0x30]
        mov edx, dword ptr [esp + 0x34]
        push eax
        mov ecx, ebx
        call Image_SetSize
        mov ecx, ebx
        call Image_BytesPerPixel
        imul eax, dword ptr [esp + 0x30]
        imul eax, dword ptr [esp + 0x34]
        push eax
        call dword ptr [g_Iat_malloc_004cc5dc]
        add esp, 0x4
        mov edx, eax
        mov ecx, ebx
        push 0x0
        call Image_SetPixelsAndAlpha
        mov ecx, dword ptr [ebx]
        xor eax, eax
        test ecx, ecx
        jle L_4bf32b
    L_4bf318:
        mov ecx, dword ptr [ebx + 0x10]
        mov DX, word ptr [esp + 0x10]
        mov word ptr [ecx + eax*0x2], DX
        mov ecx, dword ptr [ebx]
        inc eax
        cmp eax, ecx
        jl L_4bf318
    L_4bf32b:
        push 0xa0
        mov DL, 0xc0
        mov CL, 0xa0
        mov dword ptr [esi + 0xa970], ebx
        call Pixel_FromRGBBytes
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esi + 0xa968]
        cdq
        and edx, 0x3
        add eax, edx
        sar eax, 0x2
        mov dword ptr [esp + 0x30], eax
        mov eax, dword ptr [esi + 0xa964]
        cdq
        and edx, 0x3
        add eax, edx
        sar eax, 0x2
        mov dword ptr [esp + 0x34], eax
        call Image_Alloc
        mov ebx, eax
        mov DL, 0x1
        mov ecx, ebx
        call Image_SetFlags
        mov eax, dword ptr [esp + 0x30]
        mov edx, dword ptr [esp + 0x34]
        push eax
        mov ecx, ebx
        call Image_SetSize
        mov ecx, ebx
        call Image_BytesPerPixel
        imul eax, dword ptr [esp + 0x30]
        imul eax, dword ptr [esp + 0x34]
        push eax
        call dword ptr [g_Iat_malloc_004cc5dc]
        add esp, 0x4
        mov edx, eax
        mov ecx, ebx
        push 0x0
        call Image_SetPixelsAndAlpha
        mov ecx, dword ptr [ebx]
        xor eax, eax
        test ecx, ecx
        jle L_4bf3ca
    L_4bf3b7:
        mov ecx, dword ptr [ebx + 0x10]
        mov DX, word ptr [esp + 0x10]
        mov word ptr [ecx + eax*0x2], DX
        mov ecx, dword ptr [ebx]
        inc eax
        cmp eax, ecx
        jl L_4bf3b7
    L_4bf3ca:
        mov eax, dword ptr [esi + 0xa96c]
        lea ecx, [esi + 0xa978]
        push eax
        mov dword ptr [esi + 0xa974], ebx
        call ImageWidget_SetImageNoOwn
        mov ecx, dword ptr [esi + 0xaa34]
        lea ebx, [esi + 0xaa34]
        push offset g_Data_004da000 + 0xbce0
        push ebx
        call dword ptr [ecx + 0x74]
        mov edx, dword ptr [esi + 0xacd8]
        add esp, 0x8
        lea eax, [esi + 0xacd8]
        push offset g_Data_004da000 + 0xbce0
        push eax
        call dword ptr [edx + 0x74]
        mov eax, dword ptr [edi]
        add esp, 0x8
        mov ecx, edi
        push esi
        push 0x0
        call dword ptr [eax + 0x7c]
        mov ecx, dword ptr [esi + 0xa970]
        push ecx
        mov ecx, edi
        call ImageWidget_SetImageNoOwn
        mov dword ptr [edi + 0xdc], eax
        mov edx, dword ptr [esi + 0xa974]
        lea ecx, [esi + 0xa978]
        mov dword ptr [esi + 0xb060], edx
        mov edx, dword ptr [esp + 0x18]
        push ebp
        mov eax, dword ptr [ecx]
        push edx
        call dword ptr [eax + 0xc]
        mov eax, dword ptr [esp + 0x18]
        mov edx, dword ptr [esi + 0xacd8]
        add eax, 0xa
        lea ecx, [esi + 0xacd8]
        mov dword ptr [esp + 0x30], eax
        lea eax, [ebp + 0xa]
        push eax
        mov eax, dword ptr [esp + 0x34]
        push eax
        call dword ptr [edx + 0xc]
        mov ecx, dword ptr [esp + 0x30]
        mov edx, dword ptr [ebx]
        lea eax, [ebp + 0x1e]
        push eax
        push ecx
        mov ecx, ebx
        call dword ptr [edx + 0xc]
        mov eax, dword ptr [esi + 0xa968]
        mov ecx, dword ptr [edi]
        cdq
        and edx, 0x3
        mov dword ptr [esp + 0x30], ecx
        mov ecx, dword ptr [esi + 0xa964]
        add eax, edx
        sar eax, 0x2
        sub ebp, eax
        mov eax, dword ptr [esi + 0xa968]
        lea edx, [ebp + eax*0x1 - 0xa]
        mov eax, ecx
        push edx
        cdq
        sub eax, edx
        mov ebp, eax
        mov eax, ecx
        cdq
        and edx, 0x7
        mov ecx, edi
        add eax, edx
        sar ebp, 0x1
        sar eax, 0x3
        sub ebp, eax
        mov eax, dword ptr [esp + 0x1c]
        add ebp, eax
        mov eax, dword ptr [esp + 0x34]
        push ebp
        call dword ptr [eax + 0xc]
        lea eax, [esi + 0xa978]
        mov ecx, esi
        push eax
        call Widget_AppendChild
        push ebx
        mov ecx, esi
        call Widget_AppendChild
        lea ebp, [esi + 0xacd8]
        mov ecx, esi
        push ebp
        call Widget_AppendChild
        push edi
        mov ecx, esi
        call Widget_AppendChild
        mov edx, dword ptr [ebx]
        push 0x1
        mov ecx, ebx
        call dword ptr [edx + 0x60]
        mov eax, dword ptr [ebp]
        push 0x1
        mov ecx, ebp
        call dword ptr [eax + 0x60]
        mov edx, dword ptr [edi]
        push 0x0
        mov ecx, edi
        call dword ptr [edx + 0x60]
        push 0x0
        mov ecx, esi
        call WidgetContainer_SetChildFlags
    L_4bf51f:
        mov ecx, dword ptr [esp + 0x20]
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x1c
        ret 0x8
    }
}

// 0x004bf540 MessageBoxScreen_scalar_deleting_dtor - bytes ret 4: MessageBoxScreen_Dtor; flag&1 -> operator delete
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MessageBoxScreen_scalar_deleting_dtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call MessageBoxScreen_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4bf558
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bf558:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004bf560 MessageBoxScreen_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MessageBoxScreen_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MessageBoxScreen_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x8], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x8028
        mov edi, dword ptr [esi + 0xa96c]
        mov dword ptr [esp + 0x14], 0x5
        test edi, edi
        jz L_4bf5b5
        mov eax, dword ptr [edi + 0x10]
        test eax, eax
        jz L_4bf5a7
        push eax
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_4bf5a7:
        mov ecx, edi
        mov dword ptr [edi + 0x10], 0x0
        call Image_Free
    L_4bf5b5:
        lea ecx, [esi + 0xb0c8]
        mov dword ptr [esi + 0xa96c], 0x0
        mov byte ptr [esp + 0x14], 0x4
        call Button_Dtor
        lea ecx, [esi + 0xaf7c]
        mov byte ptr [esp + 0x14], 0x3
        call Button_Dtor
        lea ecx, [esi + 0xacd8]
        mov byte ptr [esp + 0x14], 0x2
        call ListLabel_Dtor
        lea ecx, [esi + 0xaa34]
        mov byte ptr [esp + 0x14], 0x1
        call ListLabel_Dtor
        lea ecx, [esi + 0xa978]
        mov byte ptr [esp + 0x14], 0x0
        call ImageWidget_Dtor
        mov ecx, esi
        mov dword ptr [esp + 0x14], 0xffffffff
        call ScreenBase_Dtor
        mov ecx, dword ptr [esp + 0xc]
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004bfcb0 ScalarDeletingDtor_004bfcd0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_004bfcd0(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call AviWidget_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4bfcc8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4bfcc8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004bfcd0 AviWidget_Dtor - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AviWidget_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_AviWidget_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x8], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x8248
        mov edi, dword ptr [esi + 0x34]
        mov dword ptr [esp + 0x14], 0x0
        test edi, edi
        jz L_4bfd1a
        mov ecx, edi
        call AviPlayer_Close
        push edi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        mov dword ptr [esi + 0x34], 0x0
    L_4bfd1a:
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [esi], offset g_RData_004cc000 + 0xa10
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004bfd40 AviWidget_Open - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall AviWidget_Open(int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_AviWidget_Open
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x24
        mov eax, dword ptr [esp + 0x34]
        push ebx
        push ebp
        mov ebx, dword ptr [g_Iat_strncpy_004cc5a0]
        push esi
        push edi
        mov edi, ecx
        push 0x104
        push eax
        lea esi, [edi + 0x3e]
        push esi
        call ebx
        mov ebp, dword ptr [g_Iat__stat_004cc508]
        add esp, 0xc
        lea ecx, [esp + 0x10]
        push ecx
        push esi
        call ebp
        add esp, 0x8
        cmp eax, -0x1
        jnz L_4bfdab
        push 0x0
        mov edx, esi
        mov ecx, 0x5
        call CDCheck_FindDiscDrive
        test eax, eax
        jz L_4bfdab
        push 0x104
        push eax
        push esi
        call ebx
        add esp, 0xc
    L_4bfdab:
        lea edx, [esp + 0x10]
        push edx
        push esi
        call ebp
        add esp, 0x8
        cmp eax, -0x1
        jnz L_4bfdc4
        mov dword ptr [edi + 0x34], 0x0
        jmp L_4bfe01
    L_4bfdc4:
        push 0x1e4
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov dword ptr [esp + 0x44], eax
        test eax, eax
        mov dword ptr [esp + 0x3c], 0x0
        jz L_4bfded
        push 0x0
        push esi
        mov ecx, eax
        call AviPlayer_Open
        jmp L_4bfdef
    L_4bfded:
        xor eax, eax
    L_4bfdef:
        mov dword ptr [edi + 0x34], eax
        mov eax, dword ptr [edi]
        mov ecx, edi
        mov dword ptr [esp + 0x3c], 0xffffffff
        call dword ptr [eax + 0x74]
    L_4bfe01:
        mov ecx, dword ptr [esp + 0x34]
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x30
        ret 0x4
    }
}

// 0x004bfe40 AnimWidget_Slot_Tick - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall AnimWidget_Slot_Tick(int, int, int)
{
    __asm {
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0xc]
        not eax
        test AL, 0x10
        jz L_4bfe85
        push esi
        mov esi, dword ptr [edi + 0x34]
        test esi, esi
        jz L_4bfe6e
        fild dword ptr [esi + 0xec]
        fmul dword ptr [edi + 0x38]
        call dword ptr [g_Iat__ftol_004cc5ac]
        cdq
        idiv dword ptr [esi + 0x4c]
        mov ecx, esi
        push edx
        call AviPlayer_DecodeFrame
    L_4bfe6e:
        mov ecx, dword ptr [esp + 0xc]
        push ecx
        mov ecx, edi
        call Widget_Tick
        fld dword ptr [esp + 0xc]
        fadd dword ptr [edi + 0x38]
        pop esi
        fstp dword ptr [edi + 0x38]
    L_4bfe85:
        pop edi
        ret 0x4
    }
}

// 0x004cb52e EH_Unwind_ScreenBase_Ctor_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Ctor_3(int, int)
{
    __asm {
        push offset AviWidget_Dtor
        push 0xa
        push 0x144
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0xfcc
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb59e EH_Unwind_ScreenBase_Dtor_3 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ScreenBase_Dtor_3(int, int)
{
    __asm {
        push offset AviWidget_Dtor
        push 0xa
        push 0x144
        mov eax, dword ptr [ebp - 0x10]
        add eax, 0xfcc
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cb800 EH_Unwind_MessageBoxScreen_Ctor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        jmp ScreenBase_Dtor
    }
}

// 0x004cb860 EH_Unwind_MessageBoxScreen_Dtor_0 - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MessageBoxScreen_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenBase_Dtor
    }
}

// 0x004cb561 EH_Handler_ScreenBase_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 30 77 4d 00 e9 35 ab ff ff): MOV EAX,FuncInfo 0x004d7730; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 5 unwind states, unwind map 0x004d7750 (to-state, action) = ['(-1, 0x004cb500)', '(0, 0x004cb508)', '(1, 0x004cb513)', '(2, 0x004cb52e)', '(3, 0x004cb549)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7750[5] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Ctor_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Ctor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Ctor_4)}};
const EhFuncInfo g_EhFuncInfo_004d7730 = {0x19930520u, 5, g_EhUnwindMap_004d7750, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ScreenBase_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7730
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb5d1 EH_Handler_ScreenBase_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 78 77 4d 00 e9 c5 aa ff ff): MOV EAX,FuncInfo 0x004d7778; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 5 unwind states, unwind map 0x004d7798 (to-state, action) = ['(-1, 0x004cb570)', '(0, 0x004cb578)', '(1, 0x004cb583)', '(2, 0x004cb59e)', '(3, 0x004cb5b9)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7798[5] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Dtor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Dtor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Dtor_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Dtor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_ScreenBase_Dtor_4)}};
const EhFuncInfo g_EhFuncInfo_004d7778 = {0x19930520u, 5, g_EhUnwindMap_004d7798, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ScreenBase_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7778
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb84e EH_Handler_MessageBoxScreen_Ctor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 28 7a 4d 00 e9 48 a8 ff ff): MOV EAX,FuncInfo 0x004d7a28; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 6 unwind states, unwind map 0x004d7a48 (to-state, action) = ['(-1, 0x004cb800)', '(0, 0x004cb808)', '(1, 0x004cb816)', '(2, 0x004cb824)', '(3, 0x004cb832)', '(4, 0x004cb840)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7a48[6] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Ctor_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Ctor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Ctor_4)}, {4, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Ctor_5)}};
const EhFuncInfo g_EhFuncInfo_004d7a28 = {0x19930520u, 6, g_EhUnwindMap_004d7a48, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MessageBoxScreen_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7a28
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cb8ae EH_Handler_MessageBoxScreen_Dtor - ../../04_spec/systems/ui_widgets.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 78 7a 4d 00 e9 e8 a7 ff ff): MOV EAX,FuncInfo 0x004d7a78; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 6 unwind states, unwind map 0x004d7a98 (to-state, action) = ['(-1, 0x004cb860)', '(0, 0x004cb868)', '(1, 0x004cb876)', '(2, 0x004cb884)', '(3, 0x004cb892)', '(4, 0x004cb8a0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d7a98[6] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Dtor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Dtor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Dtor_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Dtor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Dtor_4)}, {4, reinterpret_cast<void*>(&EH_Unwind_MessageBoxScreen_Dtor_5)}};
const EhFuncInfo g_EhFuncInfo_004d7a78 = {0x19930520u, 6, g_EhUnwindMap_004d7a98, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MessageBoxScreen_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d7a78
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x0041a5b0 NetGameSetup_Slot_Start - ../../04_spec/systems/ui_widgets.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGameSetup_Slot_Start(int, int)
{
    __asm {
        sub esp, 0x78
        push ebx
        mov ebx, dword ptr [ecx + 0xc8]
        push ebp
        push esi
        xor esi, esi
        push edi
        mov dword ptr [esp + 0x10], esi
        call Button_Activate
        mov eax, dword ptr [ebx + 0xc7b4]
        test eax, eax
        jz L_41a5de
        mov dword ptr [esp + 0x10], 0x1
        mov esi, dword ptr [esp + 0x10]
    L_41a5de:
        mov eax, dword ptr [ebx + 0xc918]
        test eax, eax
        jz L_41a5ef
        or esi, 0x2
        mov dword ptr [esp + 0x10], esi
    L_41a5ef:
        call Setting_Get_004e5d90
        test eax, eax
        jnz L_41a731
        mov eax, dword ptr [ebx + 0xcaa8]
        test eax, eax
        jnz L_41a731
        mov eax, dword ptr [ebx + 0xb0a8]
        mov edx, dword ptr [ebx + 0xb3fc]
        lea ecx, [ebx + 0xb3fc]
        inc eax
        mov dword ptr [esp + 0x14], eax
        mov dword ptr [esp + 0x18], esi
        call dword ptr [edx + 0x8c]
        lea ebp, [ebx + 0xba20]
        mov dword ptr [esp + 0x1c], eax
        mov ecx, ebp
        mov eax, dword ptr [ebp]
        call dword ptr [eax + 0x8c]
        mov edx, dword ptr [ebx + 0xc044]
        lea ecx, [ebx + 0xc044]
        mov dword ptr [esp + 0x20], eax
        call dword ptr [edx + 0x8c]
        lea ecx, [ebx + 0xabe8]
        mov dword ptr [esp + 0x28], eax
        call EditField_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x2c]
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, edx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        lea ecx, [esp + 0x14]
        call DPlay_HostSession
        test eax, eax
        jz L_41a6ab
        mov ecx, 0x1
        call Settings_SetNetworkFlag
        call Setting_Get_004e5d4c
        mov ecx, eax
        call DPlay_CreateLocalPlayerAndJoin
    L_41a6ab:
        mov esi, dword ptr [esp + 0x10]
    L_41a6af:
        mov ecx, esi
        mov dword ptr [g_Data_004da000 + 0x19df4], 0x1
        call Net_SetFlags3f88_3f90
        mov edx, dword ptr [ebp]
        mov ecx, ebp
        call dword ptr [edx + 0x8c]
        lea ecx, [ebx + 0xb3fc]
        mov esi, eax
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x8c]
        mov dword ptr [esp + 0x10], eax
        push esi
        fild dword ptr [esp + 0x14]
        push ecx
        mov ecx, offset g_Data_004da000 + 0x16cc0
        fmul dword ptr [g_RData_004cc000 + 0x3ba4]
        fstp dword ptr [esp]
        call Mission_SetRaceTiming
        mov ebx, dword ptr [ebx + 0xb0a8]
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call RecoilApp_GetField20
        mov ecx, dword ptr [eax + 0x1d8]
        add ebx, 0x7
        push ecx
        push ebx
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_SetOutcomeWithParam
        push 0x0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePop
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x78
        ret
    L_41a731:
        mov edx, dword ptr [ebx + 0xba20]
        lea ebp, [ebx + 0xba20]
        mov ecx, ebp
        call dword ptr [edx + 0x8c]
        lea edi, [ebx + 0xb3fc]
        mov dword ptr [esp + 0x10], eax
        mov ecx, edi
        mov eax, dword ptr [edi]
        call dword ptr [eax + 0x8c]
        mov ecx, dword ptr [esp + 0x10]
        mov edx, esi
        push ecx
        mov ecx, dword ptr [ebx + 0xb0a8]
        push eax
        inc ecx
        call NetMsg_SendEvent
        call Net_IsSessionActive
        test eax, eax
        jz L_41a79d
        mov edx, dword ptr [edi]
        mov ecx, edi
        call dword ptr [edx + 0x8c]
        mov edi, eax
        mov eax, dword ptr [ebp]
        mov ecx, ebp
        call dword ptr [eax + 0x8c]
        mov ecx, dword ptr [ebx + 0xb0a8]
        push esi
        push edi
        mov edx, eax
        inc ecx
        call Net_SendIfConnected
    L_41a79d:
        call Resources_ReleaseGroups
        call NetPlayers_FreeAll
        jmp L_41a6af
    }
}

}  // namespace recoil
