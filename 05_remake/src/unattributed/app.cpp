// SUBSYSTEM: app
// Functions of subsystem `app` with no attributed original source file (ledger orig_file empty).
// Their placement here is a layout choice, not a provenance claim (STAGE2.md section 2).
// Spec: 04_spec/systems/app.md
#include "unattributed/app.h"

#include <float.h>
#include "platform/image/original_data.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"
#include "unattributed/settings.h"
#include "Battlesport/Briefing.h"
#include "Battlesport/hud.h"
#include "platform/mfc42.h"
#include "GameZRecoil/zFMV/fmv_script.h"
#include "unattributed/menus.h"
#include "GameZRecoil/zSound/zsnd_parm.h"
#include "platform/iat_user32.h"
#include "unattributed/texture.h"
#include "GameZRecoil/zSound/zsnd_cd.h"
#include "platform/iat_mfc42.h"
#include "GameZRecoil/zVideo/zvid_init.h"
#include "unattributed/ui_widgets.h"
#include "GameZRecoil/zInput/zin_kbd.h"
#include "Battlesport/RecoilApp.h"
#include "Battlesport/map.h"
#include "Battlesport/mission.h"
#include "GameZRecoil/zImage/zimg_fonts.h"
#include "GameZRecoil/zInput/zin_init.h"
#include "GameZRecoil/zModel/gmod_init.h"
#include "GameZRecoil/zReader/zreader.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "platform/iat_gdi32.h"
#include "unattributed/mission.h"
#include "unattributed/render_frame.h"
#include "unattributed/sysinfo.h"
#include "platform/msvc_eh.h"
#include "unattributed/zeffect.h"
#include "unattributed/savegame.h"
#include "GameZRecoil/zWeapon/zwep_init.h"
#include "GameZRecoil/zClass/cls_util.h"
#include "unattributed/sound.h"
#include "GameZRecoil/zVideo/zvid_dd.h"
#include "platform/iat_ole32.h"
#include "unattributed/znetwork.h"
#include "GameZRecoil/zNetwork/znet_dplay.h"
#include "GameZRecoil/zSound/zsnd_init.h"
#include "unattributed/zvideo.h"
#include "GameZRecoil/zEffect/zeff_anim_init.h"
#include "GameZRecoil/zError/zerr_old.h"
#include "platform/advapi.h"
#include "GameZRecoil/zVideo/zvid_buff.h"
#include "unattributed/input.h"
#include "Battlesport/hud_hud.h"
#include "GameZRecoil/zClass/Class.h"
#include "GameZRecoil/zClass/Object3d.h"
#include "GameZRecoil/zEffect/zeff_anim_run.h"
#include "unattributed/hud.h"
#include "unattributed/mapscreen.h"
#include "unattributed/scene_update.h"
#include "GameZRecoil/zVideo/zvid_ddd3d.h"
#include "Battlesport/pickup.h"
#include "Battlesport/player.h"
#include "Battlesport/turret.h"
#include "GameZRecoil/zClass/Camera.h"
#include "unattributed/camera.h"
#include "unattributed/net.h"
#include "unattributed/pickup.h"
#include "unattributed/vehicle.h"
#include "platform/iat_comdlg32.h"
#include "platform/iat_shell32.h"
#include "unattributed/weapon.h"

namespace recoil {

// 0x004c6350 Crt_SetFpuControl - CONFIRMED-BINARY (bytes re-read 2026-09-25):
//   push 0x30000 ; push 0x10000 ; call _controlfp ; add esp, 8 ; ret
// Sets the x87 precision-control field to 53 bits. During gameplay the control word reads 0x027F
// (VERIFIED-ORACLE, trace Recoil22; 04_spec/systems/zvideo.md section 1).
void Crt_SetFpuControl()
{
    _controlfp(_PC_53 /* 0x10000 CONFIRMED-BINARY 0x004c6355 */,
               _MCW_PC /* 0x30000 CONFIRMED-BINARY 0x004c6350 */);
}

// 0x0042df90 ScreenBase_ResetVtbl - *this = 0x004ccd50
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ScreenBase_ResetVtbl(int, int)
{
    __asm {
        mov dword ptr [ecx], offset g_RData_004cc000 + 0xd50
        ret
    }
}

// 0x0042eea0 FrameState_Ctor_004d0b90 - vtbl 0x004d0b90; zero +0x10/+0x14; return this
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall FrameState_Ctor_004d0b90(int, int)
{
    __asm {
        mov eax, ecx
        xor ecx, ecx
        mov dword ptr [eax], offset g_RData_004cc000 + 0x4b90
        mov dword ptr [eax + 0x10], ecx
        mov dword ptr [eax + 0x14], ecx
        ret
    }
}

// 0x0042eecd Stub_Ret4 - ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Stub_Ret4(int, int, int)
{
    __asm {
        ret 0x4
    }
}

// 0x00430240 RuntimeClass_Get_004d0bf0 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return &0x004d0bf0 (MFC GetRuntimeClass). Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RuntimeClass_Get_004d0bf0(int, int)
{
    __asm {
        mov eax, offset g_RData_004cc000 + 0x4bf0
        ret
    }
}

// 0x00438980 Version_GetString - return 0x004dd1d4
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Version_GetString(int, int)
{
    __asm {
        mov eax, offset g_Data_004da000 + 0x31d4
        ret
    }
}

// 0x00442260 RuntimeClass_Get_004d1eb0 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return &0x004d1eb0. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RuntimeClass_Get_004d1eb0(int, int)
{
    __asm {
        mov eax, offset g_RData_004cc000 + 0x5eb0
        ret
    }
}

// 0x004428a0 RecoilApp_GetClassInfoPtr - return 0x004d2000 (MFC runtime class)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_GetClassInfoPtr(int, int)
{
    __asm {
        mov eax, offset g_RData_004cc000 + 0x6000
        ret
    }
}

// 0x00442c00 RecoilApp_GetField20 - return [ECX+0x20]
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_GetField20(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x20]
        ret
    }
}

// 0x004437a0 RuntimeClass_Get_004d20e0 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): return &0x004d20e0. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RuntimeClass_Get_004d20e0(int, int)
{
    __asm {
        mov eax, offset g_RData_004cc000 + 0x60e0
        ret
    }
}

// 0x004a59a0 Global_Set_0056b564 - [0x0056b564]=ECX
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Global_Set_0056b564(int, int)
{
    __asm {
        mov dword ptr [g_Data_004da000 + 0x91564], ecx
        ret
    }
}

// 0x004a59b0 Video_IsWindowedNonGlide - bytes: return (renderer 0x0056bbe8 != 2) ? [0x0056b564] : 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Video_IsWindowedNonGlide(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        mov ecx, dword ptr [g_Data_004da000 + 0x91564]
        sub eax, 0x2
        neg eax
        sbb eax, eax
        and eax, ecx
        ret
    }
}

// 0x004a75e0 EmptyStub_004a75e0 - ../../04_spec/systems/render_frame.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EmptyStub_004a75e0(int, int)
{
    __asm {
        xor eax, eax
        ret
    }
}

// 0x0042db50 Atl_InternalQueryInterface - (this, entries, iid, out) ret 0x10: out null -> E_POINTER 0x80004003; IID_IUnknown ({0,0,0xc0,0x46000000}) -> this+entries[0].offset, AddRef, out, S_OK; else walk entries {piid, dw, func}: match (or null iid) -> func==1 (simple offset) AddRef+return, else call func; end of table -> E_NOINTERFACE 0x8
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall Atl_InternalQueryInterface(int, int, int, int, int, int)
{
    __asm {
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0x18]
        push esi
        test ebp, ebp
        push edi
        jnz L_42db68
        mov eax, 0x80004003
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x10
    L_42db68:
        mov edi, dword ptr [esp + 0x1c]
        mov dword ptr [ebp], 0x0
        cmp dword ptr [edi], 0x0
        jnz L_42db9a
        mov eax, dword ptr [edi + 0x4]
        test eax, eax
        jnz L_42db9a
        cmp dword ptr [edi + 0x8], 0xc0
        jnz L_42db9a
        cmp dword ptr [edi + 0xc], 0x46000000
        jnz L_42db9a
        mov eax, dword ptr [esp + 0x18]
        mov esi, dword ptr [eax + 0x4]
        jmp L_42dc09
    L_42db9a:
        mov esi, dword ptr [esp + 0x18]
        mov ecx, dword ptr [esi + 0x8]
        test ecx, ecx
        jz L_42dc1f
    L_42dba5:
        mov eax, dword ptr [esi]
        xor ebx, ebx
        test eax, eax
        setz BL
        test ebx, ebx
        jnz L_42dbd2
        mov edx, dword ptr [eax]
        cmp edx, dword ptr [edi]
        jnz L_42dbf0
        mov edx, dword ptr [eax + 0x4]
        cmp edx, dword ptr [edi + 0x4]
        jnz L_42dbf0
        mov edx, dword ptr [eax + 0x8]
        cmp edx, dword ptr [edi + 0x8]
        jnz L_42dbf0
        mov eax, dword ptr [eax + 0xc]
        mov edx, dword ptr [edi + 0xc]
        cmp eax, edx
        jnz L_42dbf0
    L_42dbd2:
        cmp ecx, 0x1
        jz L_42dc06
        mov edx, dword ptr [esi + 0x4]
        mov eax, dword ptr [esp + 0x14]
        push edx
        push ebp
        push edi
        push eax
        call ecx
        test eax, eax
        jz L_42dc24
        test ebx, ebx
        jnz L_42dbf0
        test eax, eax
        jl L_42dc24
    L_42dbf0:
        mov ecx, dword ptr [esi + 0x14]
        add esi, 0xc
        test ecx, ecx
        jnz L_42dba5
        mov eax, 0x80004002
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x10
    L_42dc06:
        mov esi, dword ptr [esi + 0x4]
    L_42dc09:
        add esi, dword ptr [esp + 0x14]
        push esi
        mov ecx, dword ptr [esi]
        call dword ptr [ecx + 0x4]
        mov dword ptr [ebp], esi
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x10
    L_42dc1f:
        mov eax, 0x80004002
    L_42dc24:
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x10
    }
}

// 0x0042de00 ComPtr_Release - if *this: (*this)->Release() (vfunc+8)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ComPtr_Release(int, int)
{
    __asm {
        mov eax, dword ptr [ecx]
        test eax, eax
        jz L_42de0c
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx + 0x8]
    L_42de0c:
        ret
    }
}

// 0x0042faa0 ComPtr_StopAndReset - bytes: p=*this; if p: p vtbl+0x20(p); p vtbl+0x1c(p,1,0); caller 0x0043c404
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ComPtr_StopAndReset(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        test eax, eax
        jz L_42fabb
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx + 0x20]
        mov eax, dword ptr [esi]
        push 0x0
        push 0x1
        push eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x1c]
    L_42fabb:
        pop esi
        ret
    }
}

// 0x0042dda0 DsBuffer_Init - (desc, a, b) ret 0xc: null or desc size < 100 -> E_INVALIDARG 0x80070057; set fields [1..6]; InitializeCriticalSection x3 at +0x1c/+0x34/+0x4c; return 0
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall DsBuffer_Init(int, int, int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        xor ecx, ecx
        push edi
        cmp esi, ecx
        jnz L_42ddb6
        mov eax, 0x80070057
        pop edi
        pop esi
        ret 0xc
    L_42ddb6:
        cmp dword ptr [esi], 0x64
        jnc L_42ddc5
        mov eax, 0x80070057
        pop edi
        pop esi
        ret 0xc
    L_42ddc5:
        mov eax, dword ptr [esp + 0x10]
        mov edi, dword ptr [g_Iat_InitializeCriticalSection_004cc0e8]
        mov dword ptr [esi + 0x14], ecx
        mov dword ptr [esi + 0x18], ecx
        mov dword ptr [esi + 0x10], eax
        mov eax, dword ptr [esp + 0x14]
        lea ecx, [esi + 0x1c]
        mov dword ptr [esi + 0x8], eax
        push ecx
        mov dword ptr [esi + 0xc], eax
        mov dword ptr [esi + 0x4], eax
        call edi
        lea edx, [esi + 0x34]
        push edx
        call edi
        add esi, 0x4c
        push esi
        call edi
        pop edi
        xor eax, eax
        pop esi
        ret 0xc
    }
}

// 0x00442860 CritSec_Delete - DeleteCriticalSection(ECX ? ECX+8 : 4)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall CritSec_Delete(int, int)
{
    __asm {
        test ecx, ecx
        jz L_442872
        lea eax, [ecx + 0x4]
        add eax, 0x4
        push eax
        call dword ptr [g_Iat_DeleteCriticalSection_004cc0f0]
        ret
    L_442872:
        xor eax, eax
        mov eax, 0x4
        push eax
        call dword ptr [g_Iat_DeleteCriticalSection_004cc0f0]
        ret
    }
}

// 0x00442770 App_Callback_ImportCall_00442770 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): [0x004cc0d8](arg+4), ret 4. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall App_Callback_ImportCall_00442770(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        add eax, 0x4
        push eax
        call dword ptr [g_Iat_InterlockedIncrement_004cc0d8]
        ret 0x4
    }
}

// 0x004a5ad0 Messages_LoadDll - [0x0056b670]=LoadLibraryA(ECX); ok -> [0x0056b568]=GetProcAddress("ZLocGetID" 0x004e2ff8); return ok
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Messages_LoadDll(int, int)
{
    __asm {
        push esi
        push ecx
        xor esi, esi
        call dword ptr [g_Iat_LoadLibraryA_004cc0b8]
        test eax, eax
        mov dword ptr [g_Data_004da000 + 0x91670], eax
        jz L_4a5af9
        push offset g_Data_004da000 + 0x8ff8
        push eax
        mov esi, 0x1
        call dword ptr [g_Iat_GetProcAddress_004cc0bc]
        mov dword ptr [g_Data_004da000 + 0x91568], eax
    L_4a5af9:
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004a5b00 App_FreeLoadedLibrary - bytes: if HMODULE 0x0056b670: FreeLibrary; null; caller 0x0042e96f
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_FreeLoadedLibrary(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x91670]
        test eax, eax
        jz L_4a5b10
        push eax
        call dword ptr [g_Iat_FreeLibrary_004cc16c]
    L_4a5b10:
        mov dword ptr [g_Data_004da000 + 0x91670], 0x0
        ret
    }
}

// 0x004a5980 App_ExitProcess - Stub_Ret (0x004076f0); _fcloseall; ExitProcess(ECX)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_ExitProcess(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Stub_Ret
        call dword ptr [g_Iat__fcloseall_004cc520]
        push esi
        call dword ptr [g_Iat_ExitProcess_004cc0ec]
    }
}

// 0x004a5780 App_RedirectStdioToLogs - bytes: 0x004f3eec=0; null name -> return; freopen('<name>'+suffix 0x004e2ff0, mode 0x004da248, stderr _iob+0x40), fallback GetTempPathA + 'gamez.err'; fprintf 'File started...' ; same for stdout _iob+0x20 with fallback to temp path; calls are IAT (no direct e8 callees)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_RedirectStdioToLogs(int, int)
{
    __asm {
        sub esp, 0x40
        mov dword ptr [g_Data_004da000 + 0x19eec], 0x0
        push ebx
        push ebp
        mov ebp, ecx
        push esi
        test ebp, ebp
        push edi
        jz L_4a596b
        mov edi, ebp
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x10]
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, edx
        lea edx, [esp + 0x10]
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        xor eax, eax
        and ecx, 0x3
        rep movsb
        mov edi, offset g_Data_004da000 + 0x8ff0
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov ebx, ecx
        mov edi, edx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, ebx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov eax, dword ptr [g_Iat__iob_004cc4f8]
        mov ecx, ebx
        mov ebx, dword ptr [g_Iat_freopen_004cc51c]
        and ecx, 0x3
        rep movsb
        add eax, 0x40
        lea ecx, [esp + 0x10]
        push eax
        push offset g_Data_004da000 + 0x248
        push ecx
        call ebx
        mov esi, eax
        add esp, 0xc
        test esi, esi
        jnz L_4a586d
        lea edx, [esp + 0x10]
        push edx
        push 0x40
        call dword ptr [g_Iat_GetTempPathA_004cc13c]
        test eax, eax
        jz L_4a5869
        mov edi, offset g_Data_004da000 + 0x8fe4
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x10]
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edi, edx
        mov edx, ecx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov eax, dword ptr [g_Iat__iob_004cc4f8]
        mov ecx, edx
        and ecx, 0x3
        add eax, 0x40
        rep movsb
        push eax
        lea ecx, [esp + 0x14]
        push offset g_Data_004da000 + 0x248
        push ecx
        call ebx
        add esp, 0xc
        mov esi, eax
    L_4a5869:
        test esi, esi
        jz L_4a5886
    L_4a586d:
        push offset g_Data_004da000 + 0x8fd0
        push esi
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x8
        push esi
        call dword ptr [g_Iat_fflush_004cc4fc]
        add esp, 0x4
    L_4a5886:
        mov edi, ebp
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x10]
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, edx
        lea edx, [esp + 0x10]
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        xor eax, eax
        and ecx, 0x3
        rep movsb
        mov edi, offset g_Data_004da000 + 0x8fc8
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov ebp, ecx
        mov edi, edx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, ebp
        dec edi
        shr ecx, 0x2
        rep movsd
        mov eax, dword ptr [g_Iat__iob_004cc4f8]
        mov ecx, ebp
        and ecx, 0x3
        add eax, 0x20
        rep movsb
        push eax
        lea ecx, [esp + 0x14]
        push offset g_Data_004da000 + 0x248
        push ecx
        call ebx
        mov esi, eax
        add esp, 0xc
        test esi, esi
        jnz L_4a5952
        lea edx, [esp + 0x10]
        push edx
        push 0x40
        call dword ptr [g_Iat_GetTempPathA_004cc13c]
        test eax, eax
        jz L_4a594e
        mov edi, offset g_Data_004da000 + 0x8fbc
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x10]
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov ebp, ecx
        mov edi, edx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, ebp
        dec edi
        shr ecx, 0x2
        rep movsd
        mov eax, dword ptr [g_Iat__iob_004cc4f8]
        mov ecx, ebp
        and ecx, 0x3
        add eax, 0x20
        rep movsb
        push eax
        lea ecx, [esp + 0x14]
        push offset g_Data_004da000 + 0x248
        push ecx
        call ebx
        add esp, 0xc
        mov esi, eax
    L_4a594e:
        test esi, esi
        jz L_4a596b
    L_4a5952:
        push offset g_Data_004da000 + 0x8fd0
        push esi
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x8
        push esi
        call dword ptr [g_Iat_fflush_004cc4fc]
        add esp, 0x4
    L_4a596b:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    }
}

// 0x0040c370 RecoilApp_ProbeDirectXCapabilities - runtime DirectX probe via LoadLibrary/GetProcAddress only (no internal calls): DINPUT.DLL DirectInputCreateA, DDRAW.DLL DirectDrawCreate, DINPUT.DLL DirectInputCreateA again; reports availability (return flags)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_ProbeDirectXCapabilities(int, int)
{
    __asm {
        sub esp, 0x114
        push ebx
        push ebp
        push esi
        push edi
        lea eax, [esp + 0x90]
        xor edi, edi
        mov ebp, edx
        mov esi, ecx
        push eax
        mov dword ptr [esp + 0x14], edi
        mov dword ptr [esp + 0x1c], edi
        mov dword ptr [esp + 0x18], edi
        mov dword ptr [esp + 0x20], edi
        mov dword ptr [esp + 0x24], edi
        mov dword ptr [esp + 0x94], 0x94
        call dword ptr [g_Iat_GetVersionExA_004cc0c0]
        test eax, eax
        jnz L_40c3c1
        mov dword ptr [esi], edi
        mov dword ptr [ebp], edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c3c1:
        cmp dword ptr [esp + 0xa0], 0x2
        jnz L_40c45f
        mov eax, dword ptr [esp + 0x94]
        mov dword ptr [ebp], 0x2
        cmp eax, 0x4
        jnc L_40c3f0
        mov dword ptr [ebp], edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c3f0:
        jnz L_40c466
        push offset g_Data_004da000 + 0xbf4
        mov dword ptr [esi], 0x200
        call dword ptr [g_Iat_LoadLibraryA_004cc0b8]
        mov ebx, eax
        cmp ebx, edi
        jnz L_40c41f
        push offset g_Data_004da000 + 0xbd4
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c41f:
        push offset g_Data_004da000 + 0xbc0
        push ebx
        call dword ptr [g_Iat_GetProcAddress_004cc0bc]
        mov ebp, eax
        push ebx
        call dword ptr [g_Iat_FreeLibrary_004cc16c]
        cmp ebp, edi
        jnz L_40c44e
        push offset g_Data_004da000 + 0xb98
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c44e:
        mov dword ptr [esi], 0x300
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c45f:
        mov dword ptr [ebp], 0x1
    L_40c466:
        mov edi, dword ptr [g_Iat_LoadLibraryA_004cc0b8]
        push offset g_Data_004da000 + 0xb8c
        call edi
        mov ebx, eax
        test ebx, ebx
        jnz L_40c490
        mov dword ptr [esi], eax
        push eax
        mov dword ptr [ebp], eax
        call dword ptr [g_Iat_FreeLibrary_004cc16c]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c490:
        push offset g_Data_004da000 + 0xb78
        push ebx
        call dword ptr [g_Iat_GetProcAddress_004cc0bc]
        test eax, eax
        jnz L_40c4c2
        mov dword ptr [esi], eax
        push ebx
        mov dword ptr [ebp], eax
        call dword ptr [g_Iat_FreeLibrary_004cc16c]
        push offset g_Data_004da000 + 0xb58
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c4c2:
        lea ecx, [esp + 0x10]
        push 0x0
        push ecx
        push 0x0
        call eax
        test eax, eax
        jge L_40c4fb
        mov dword ptr [esi], 0x0
        push ebx
        mov dword ptr [ebp], 0x0
        call dword ptr [g_Iat_FreeLibrary_004cc16c]
        push offset g_Data_004da000 + 0xb40
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c4fb:
        mov eax, dword ptr [esp + 0x10]
        lea ecx, [esp + 0x18]
        mov dword ptr [esi], 0x100
        push ecx
        mov edx, dword ptr [eax]
        push offset g_RData_004cc000 + 0x6f98
        push eax
        call dword ptr [edx]
        test eax, eax
        jge L_40c53f
        mov eax, dword ptr [esp + 0x10]
        push eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x8]
        push ebx
        call dword ptr [g_Iat_FreeLibrary_004cc16c]
        push offset g_Data_004da000 + 0xb28
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c53f:
        mov eax, dword ptr [esp + 0x18]
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x8]
        mov dword ptr [esi], 0x200
        push offset g_Data_004da000 + 0xbf4
        call edi
        mov ebp, eax
        test ebp, ebp
        jnz L_40c583
        push offset g_Data_004da000 + 0xbd4
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        mov eax, dword ptr [esp + 0x10]
        push eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x8]
        push ebx
        call dword ptr [g_Iat_FreeLibrary_004cc16c]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c583:
        push offset g_Data_004da000 + 0xbc0
        push ebp
        call dword ptr [g_Iat_GetProcAddress_004cc0bc]
        mov edi, eax
        push ebp
        mov ebp, dword ptr [g_Iat_FreeLibrary_004cc16c]
        call ebp
        test edi, edi
        jnz L_40c5c1
        push ebx
        call ebp
        mov eax, dword ptr [esp + 0x10]
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x8]
        push offset g_Data_004da000 + 0xb98
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c5c1:
        mov ecx, 0x1b
        xor eax, eax
        lea edi, [esp + 0x24]
        mov dword ptr [esi], 0x300
        rep stosd
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x24], 0x6c
        mov dword ptr [esp + 0x28], 0x1
        mov dword ptr [esp + 0x8c], 0x200
        mov edx, dword ptr [eax]
        push 0x8
        push 0x0
        push eax
        call dword ptr [edx + 0x50]
        test eax, eax
        mov eax, dword ptr [esp + 0x10]
        jge L_40c62a
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx + 0x8]
        push ebx
        call ebp
        mov dword ptr [esi], 0x0
        push offset g_Data_004da000 + 0xb0c
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c62a:
        mov edx, dword ptr [eax]
        lea ecx, [esp + 0x14]
        push 0x0
        push ecx
        lea ecx, [esp + 0x2c]
        push ecx
        push eax
        call dword ptr [edx + 0x18]
        test eax, eax
        jge L_40c669
        mov eax, dword ptr [esp + 0x10]
        push eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x8]
        push ebx
        call ebp
        mov dword ptr [esi], 0x0
        push offset g_Data_004da000 + 0xaf0
        call dword ptr [g_Iat_OutputDebugStringA_004cc0b4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c669:
        mov eax, dword ptr [esp + 0x14]
        lea edx, [esp + 0x1c]
        push edx
        push offset g_RData_004cc000 + 0x6fd8
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx]
        test eax, eax
        jge L_40c698
        mov eax, dword ptr [esp + 0x10]
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x8]
        push ebx
        call ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    L_40c698:
        mov eax, dword ptr [esp + 0x14]
        lea ecx, [esp + 0x20]
        mov dword ptr [esi], 0x500
        push ecx
        mov edx, dword ptr [eax]
        push offset g_RData_004cc000 + 0x6fe8
        push eax
        call dword ptr [edx]
        test eax, eax
        jl L_40c6c5
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esi], 0x600
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x8]
    L_40c6c5:
        mov eax, dword ptr [esp + 0x10]
        push eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x8]
        push ebx
        call ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x114
        ret
    }
}

// 0x004427d0 App_Forward_0042db50_004427d0 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x0042db50(a1, 0x004d1fc8, a2, a3), ret 0xc. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall App_Forward_0042db50_004427d0(int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0xc]
        mov ecx, dword ptr [esp + 0x8]
        mov edx, dword ptr [esp + 0x4]
        push eax
        push ecx
        push offset g_RData_004cc000 + 0x5fc8
        push edx
        call Atl_InternalQueryInterface
        ret 0xc
    }
}

// 0x00404bd0 Loader_WaitForThread - if ECX and [0x004e5c6c]: pump 0x00404140 until done or [0x004e5cb8]/[0x004e5c6c] cleared; [0x004e5c60]=0; wait [0x004e5c64] with Sleep(100); release [0x004e5cb4] (vfunc+8(1)); [0x0056bbf8]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Loader_WaitForThread(int, int)
{
    __asm {
        test ecx, ecx
        push esi
        jz L_404bfe
        mov eax, dword ptr [g_Data_004da000 + 0xbc6c]
        test eax, eax
        jz L_404bfe
    L_404bde:
        mov ecx, 0x64
        call Briefing_WaitKeyOrTimeout
        test eax, eax
        jnz L_404bfe
        mov eax, dword ptr [g_Data_004da000 + 0xbcb8]
        test eax, eax
        jz L_404bfe
        mov eax, dword ptr [g_Data_004da000 + 0xbc6c]
        test eax, eax
        jnz L_404bde
    L_404bfe:
        mov eax, dword ptr [g_Data_004da000 + 0xbc64]
        mov dword ptr [g_Data_004da000 + 0xbc60], 0x0
        test eax, eax
        jnz L_404c24
        mov esi, dword ptr [g_Iat_Sleep_004cc0b0]
    L_404c17:
        push 0x64
        call esi
        mov eax, dword ptr [g_Data_004da000 + 0xbc64]
        test eax, eax
        jz L_404c17
    L_404c24:
        mov ecx, dword ptr [g_Data_004da000 + 0xbcb4]
        test ecx, ecx
        jz L_404c3f
        mov eax, dword ptr [ecx]
        push 0x1
        call dword ptr [eax + 0x8]
        mov dword ptr [g_Data_004da000 + 0xbcb4], 0x0
    L_404c3f:
        mov dword ptr [g_Data_004da000 + 0x91bf8], 0x0
        pop esi
        ret
    }
}

// 0x0042e0f0 ScreenBase_ScalarDeletingDtor - ScreenBase_ResetVtbl; flag&1 -> delete; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScreenBase_ScalarDeletingDtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call ScreenBase_ResetVtbl
        test byte ptr [esp + 0x8], 0x1
        jz L_42e108
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_42e108:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0042eec0 AppScreen_OnArg_0042eec0 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): if arg: 0x00413630(). Subsystem assigned from address neighbours (INFERRED). left for local - the listing stops at CALL 0x00413630 and falls through into 0x0042eecd (RET 4); port_batch cannot port a b
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppScreen_OnArg_0042eec0(int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        test eax, eax
        jz Stub_Ret4
        call CallGlobal004e5ee8Slot18
        jmp Stub_Ret4                    // the original falls through into 0x0042eecd (RET 4); without this the port ran into padding (int3)
    }
}

// 0x0042f890 OperatorDelete_Thunk_0042f890 - bytes ret 4: operator delete(arg)
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall OperatorDelete_Thunk_0042f890(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        ret 0x4
    }
}

// 0x00431b50 ScalarDeletingDtor_00431b50 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): vptr=0x004d1268; if (flags&1) delete this; return this. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ScalarDeletingDtor_00431b50(int, int, int)
{
    __asm {
        mov AL, byte ptr [esp + 0x4]
        push esi
        mov esi, ecx
        test AL, 0x1
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5268
        jz L_431b6a
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_431b6a:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0044263e EH_Continuation_0044263e - exception-handler continuation fragment (restores ExceptionList from EBP-0xc, stores ESI to [EBP+8]); not a standalone function; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall EH_Continuation_0044263e(int, int, int)
{
    __asm {
        test esi, esi
        jz L_442644
        xor edi, edi
    L_442644:
        mov eax, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        mov dword ptr [eax], esi
        mov eax, edi
        pop edi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004a5670 Timer_Reset - zero 0x004e2fb4, 0x0056b424 (dt), 0x0056b428 (t), 0x0056b42c, 0x0056b430; base [0x004e2fb0] = tick (0x004a59d0) * 0.001
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Timer_Reset(int, int)
{
    __asm {
        sub esp, 0x8
        mov dword ptr [g_Data_004da000 + 0x8fb4], 0x0
        mov dword ptr [g_Data_004da000 + 0x91430], 0x0
        mov dword ptr [g_Data_004da000 + 0x9142c], 0x0
        mov dword ptr [g_Data_004da000 + 0x91428], 0x0
        mov dword ptr [g_Data_004da000 + 0x91424], 0x0
        call dword ptr [g_Iat_GetTickCount_004cc140]
        mov dword ptr [esp], eax
        mov dword ptr [esp + 0x4], 0x0
        fild qword ptr [esp]
        fmul dword ptr [g_RData_004cc000 + 0x6f50]
        fstp dword ptr [g_Data_004da000 + 0x8fb0]
        add esp, 0x8
        ret
    }
}

// 0x0042ee50 AppScreen_OnEnter_0042ee50 - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): [this+4]=0; if ![this+0x28]: 0x00463120(this+8, 1). Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppScreen_OnEnter_0042ee50(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x28]
        mov dword ptr [ecx + 0x4], 0x0
        test eax, eax
        jnz L_42ee68
        push 0x1
        add ecx, 0x8
        call Seq_Finish
    L_42ee68:
        ret
    }
}

// 0x00442c10 RecoilApp_OnIdleStep - if vfunc+0xac(main wnd [+0x20]) == 0: vfunc+0xb0(), return vfunc+0x70(); else [0x34]=1, [0x35]=0, ScreenManager_QueuePush([0x31], 0) return 1
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_OnIdleStep(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [esi]
        call RecoilApp_GetField20
        mov eax, dword ptr [eax + 0x20]
        mov ecx, esi
        push eax
        call dword ptr [edi + 0xac]
        test eax, eax
        jnz L_442c3f
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0xb0]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x70]
        pop edi
        pop esi
        ret
    L_442c3f:
        mov ecx, dword ptr [esi + 0xc4]
        push 0x0
        push ecx
        mov ecx, esi
        mov dword ptr [esi + 0xd0], 0x1
        mov dword ptr [esi + 0xd4], 0x0
        call ScreenManager_QueuePush
        pop edi
        mov eax, 0x1
        pop esi
        ret
    }
}

// 0x00430680 MainWnd_SetWindowedChrome - bytes ret 4 (on): GetWindowLongA(GWL_STYLE); on -> style|=0x82ca0000 and SetMenu(menu +0x1d0 handle +4); off -> style&=~0x80000 (WS_SYSMENU), SetMenu(NULL); SetWindowLongA
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MainWnd_SetWindowedChrome(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        push -0x10
        mov eax, dword ptr [esi + 0x20]
        push eax
        call dword ptr [g_Iat_GetWindowLongA_004cc66c]
        mov ecx, dword ptr [esp + 0xc]
        test ecx, ecx
        jz L_4306a5
        lea edi, [esi + 0x1d0]
        or eax, 0x82ca0000
        jmp L_4306ac
    L_4306a5:
        xor edi, edi
        and eax, 0xfff7ffff
    L_4306ac:
        mov ecx, dword ptr [esi + 0x20]
        push eax
        push -0x10
        push ecx
        call dword ptr [g_Iat_SetWindowLongA_004cc69c]
        test edi, edi
        jnz L_4306cd
        mov edx, dword ptr [esi + 0x20]
        push edi
        push edx
        call dword ptr [g_Iat_SetMenu_004cc610]
        pop edi
        pop esi
        ret 0x4
    L_4306cd:
        mov edi, dword ptr [edi + 0x4]
        mov edx, dword ptr [esi + 0x20]
        push edi
        push edx
        call dword ptr [g_Iat_SetMenu_004cc610]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431330 MainWnd_OnToggleSoundArchive - bytes: +0x1d8 = !+0x1d8; CheckMenuItem(menu +0x1d4, 0x9c6b, on?MF_CHECKED 8:0); 0x004f0da0=+0x1d8; SoundArchive_SetEnabled
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnToggleSoundArchive(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        xor eax, eax
        mov edx, dword ptr [esi + 0x1d8]
        test edx, edx
        setz AL
        mov dword ptr [esi + 0x1d8], eax
        neg eax
        sbb eax, eax
        and eax, 0x8
        push eax
        mov eax, dword ptr [esi + 0x1d4]
        push 0x9c6b
        push eax
        call dword ptr [g_Iat_CheckMenuItem_004cc61c]
        mov eax, dword ptr [esi + 0x1d8]
        mov dword ptr [g_Data_004da000 + 0x16da0], eax
        mov ecx, dword ptr [esi + 0x1d8]
        call SoundArchive_SetEnabled
        pop esi
        ret
    }
}

// 0x00431380 MainWnd_OnToggleTextureFlag - bytes: if Texture_GetFlag_004e073c: Texture_SetFlag_004e073c(0), CheckMenuItem(0x9c7b,0) else set(1), CheckMenuItem(0x9c7b,MF_CHECKED)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnToggleTextureFlag(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Texture_GetFlag_004e073c
        test eax, eax
        jz L_4313a9
        xor ecx, ecx
        call Texture_SetFlag_004e073c
        mov eax, dword ptr [esi + 0x1d4]
        push 0x0
        push 0x9c7b
        push eax
        call dword ptr [g_Iat_CheckMenuItem_004cc61c]
        pop esi
        ret
    L_4313a9:
        mov ecx, 0x1
        call Texture_SetFlag_004e073c
        mov ecx, dword ptr [esi + 0x1d4]
        push 0x8
        push 0x9c7b
        push ecx
        call dword ptr [g_Iat_CheckMenuItem_004cc61c]
        pop esi
        ret
    }
}

// 0x00442270 StatusDialog_Printf - vsprintf(0x005392b0, fmt, va); SetDlgItemTextA([0x005392a8], 0x3ff, buffer)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StatusDialog_Printf(int, int)
{
    __asm {
        mov ecx, dword ptr [esp + 0x4]
        lea eax, [esp + 0x8]
        push eax
        push ecx
        push offset g_Data_004da000 + 0x5f2b0
        call dword ptr [g_Iat_vsprintf_004cc4a8]
        mov edx, dword ptr [g_Data_004da000 + 0x5f2a8]
        add esp, 0xc
        push offset g_Data_004da000 + 0x5f2b0
        push 0x3ff
        push edx
        call dword ptr [g_Iat_SetDlgItemTextA_004cc690]
        ret
    }
}

// 0x00442660 App_Callback_Set5392a4_On - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x00442270("0x004dd48c" string); [0x005392a4]=1; return 0, ret 4. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall App_Callback_Set5392a4_On(int, int, int)
{
    __asm {
        push offset g_Data_004da000 + 0x348c
        call StatusDialog_Printf
        add esp, 0x4
        mov dword ptr [g_Data_004da000 + 0x5f2a4], 0x1
        xor eax, eax
        ret 0x4
    }
}

// 0x00442680 App_Callback_Set5392a4_Off - vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x00442270(0x004dd498); Sleep(1000); [0x005392a4]=-1; return 0, ret 8. Subsystem assigned from address neighbours (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall App_Callback_Set5392a4_Off(int, int, int, int)
{
    __asm {
        push offset g_Data_004da000 + 0x3498
        call StatusDialog_Printf
        add esp, 0x4
        push 0x3e8
        call dword ptr [g_Iat_Sleep_004cc0b0]
        mov dword ptr [g_Data_004da000 + 0x5f2a4], 0xffffffff
        xor eax, eax
        ret 0x8
    }
}

// 0x004426b0 StatusDialog_OnProgress - (a,read,total,d,secsLeft) ret 0x14: progress bar item 0x3fd PBM_SETPOS (0x402) = read*100/total; text via StatusDialog_Printf: secsLeft>0 -> "Bytes read %d / %d  Time left ..." (0x004dd4b4) else "Bytes read %d / %d" (0x004dd4a0); return 0
// Register/stack shape from the listing (ECX, EDX, 20 stack bytes).
__declspec(naked) int __fastcall StatusDialog_OnProgress(int, int, int, int, int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0xc]
        push edi
        mov edi, dword ptr [esp + 0x14]
        lea eax, [esi + esi*0x4]
        xor edx, edx
        push 0x0
        lea eax, [eax + eax*0x4]
        shl eax, 0x2
        div edi
        push eax
        mov eax, dword ptr [g_Data_004da000 + 0x5f2a8]
        push 0x402
        push 0x3fd
        push eax
        call dword ptr [g_Iat_SendDlgItemMessageA_004cc6ac]
        mov eax, dword ptr [esp + 0x1c]
        test eax, eax
        jle L_4426ff
        push eax
        push edi
        push esi
        push offset g_Data_004da000 + 0x34b4
        call StatusDialog_Printf
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        ret 0x14
    L_4426ff:
        push edi
        push esi
        push offset g_Data_004da000 + 0x34a0
        call StatusDialog_Printf
        add esp, 0xc
        xor eax, eax
        pop edi
        pop esi
        ret 0x14
    }
}

// 0x00442720 WolPatch_OnStatus_00442720 - vtable-only (G1 2026-09-25); Ghidra decompile read (03_re/decomp_raw/0x00442720_*.c): status 2 "Connecting...", 4 "Finding patch...", 6 "Downloading patch..." via 0x00442270; return 0 (online patch dialog - out of v1 scope). Subsystem from address neighbours/callees (INFERRED).
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall WolPatch_OnStatus_00442720(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        sub eax, 0x2
        jz L_442757
        sub eax, 0x2
        jz L_442745
        sub eax, 0x2
        jnz L_442764
        push offset g_Data_004da000 + 0x3508
        call StatusDialog_Printf
        add esp, 0x4
        xor eax, eax
        ret 0x8
    L_442745:
        push offset g_Data_004da000 + 0x34f4
        call StatusDialog_Printf
        add esp, 0x4
        xor eax, eax
        ret 0x8
    L_442757:
        push offset g_Data_004da000 + 0x34e4
        call StatusDialog_Printf
        add esp, 0x4
    L_442764:
        xor eax, eax
        ret 0x8
    }
}

// 0x00443650 App_ForwardMciNotifyToScreen - bytes ret 8 (a,b): screen=ScreenManager_GetCurrent; SoundCD_OnMciNotify; if screen: return screen vtbl+0x24(a,b) else 0
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall App_ForwardMciNotifyToScreen(int, int, int, int)
{
    __asm {
        push ebx
        push esi
        push edi
        call ScreenManager_GetCurrent
        mov edi, dword ptr [esp + 0x14]
        mov ebx, dword ptr [esp + 0x10]
        mov edx, edi
        mov ecx, ebx
        mov esi, eax
        call SoundCD_OnMciNotify
        test esi, esi
        jz L_44367e
        mov eax, dword ptr [esi]
        push edi
        push ebx
        mov ecx, esi
        call dword ptr [eax + 0x24]
        pop edi
        pop esi
        pop ebx
        ret 0x8
    L_44367e:
        pop edi
        pop esi
        xor eax, eax
        pop ebx
        ret 0x8
    }
}

// 0x004306f0 MainWnd_GetTitle - bytes ret 4: CString out = 'RECOIL (3Dfx)' [0x004dcd5c] if renderer 0x0056bbe8==2 else 'RECOIL' [0x004dcd54]
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MainWnd_GetTitle(int, int, int)
{
    __asm {
        push ecx
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        push esi
        cmp eax, 0x2
        mov dword ptr [esp + 0x4], 0x0
        jnz L_43071b
        mov esi, dword ptr [esp + 0xc]
        push offset g_Data_004da000 + 0x2d5c
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_537_004cc29c]
        mov eax, esi
        pop esi
        pop ecx
        ret 0x4
    L_43071b:
        mov esi, dword ptr [esp + 0xc]
        push offset g_Data_004da000 + 0x2d54
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_537_004cc29c]
        mov eax, esi
        pop esi
        pop ecx
        ret 0x4
    }
}

// 0x00441c60 ComboDialog_GetSelection - (out a, out b, out c) ret 0xc: sel=[+0x114]; a=CString(+0xfc+sel*4).GetBuffer(0x20), b=CString(+0x104+sel*4).GetBuffer(0x20), c=[+0x10c+sel*4]
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall ComboDialog_GetSelection(int, int, int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x20
        mov eax, dword ptr [esi + 0x114]
        lea ecx, [esi + eax*0x4 + 0xfc]
        call dword ptr [g_Iat_MFC42_2915_004cc298]
        mov ecx, dword ptr [esp + 0x8]
        push 0x20
        mov dword ptr [ecx], eax
        mov edx, dword ptr [esi + 0x114]
        lea ecx, [esi + edx*0x4 + 0x104]
        call dword ptr [g_Iat_MFC42_2915_004cc298]
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [ecx], eax
        mov edx, dword ptr [esi + 0x114]
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esi + edx*0x4 + 0x10c]
        pop esi
        mov dword ptr [ecx], eax
        ret 0xc
    }
}

// 0x00441f40 WolLoginDialog_OnOK - net object [0x00538574] vfunc+0x94 for slots 1 and 2: if not logged [+0xe8]==0 -> (slot, name +0xfc/+0x100, empty string 0x004e5ce0, 0); else (slot, name, password +0x104/+0x108, save flag = [+0x10c]/[+0x110]==0); Settings_StoreWOLPasswordFlag; CDialog::OnOK
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall WolLoginDialog_OnOK(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0xe8]
        test eax, eax
        jnz L_441f9d
        mov eax, dword ptr [g_Data_004da000 + 0x5e574]
        mov edx, dword ptr [esi + 0xfc]
        push 0x0
        push offset g_Data_004da000 + 0xbce0
        mov ecx, dword ptr [eax]
        push edx
        push 0x1
        push eax
        call dword ptr [ecx + 0x94]
        mov eax, dword ptr [g_Data_004da000 + 0x5e574]
        mov edx, dword ptr [esi + 0x100]
        push 0x0
        push offset g_Data_004da000 + 0xbce0
        mov ecx, dword ptr [eax]
        push edx
        push 0x2
        push eax
        call dword ptr [ecx + 0x94]
        mov ecx, dword ptr [esi + 0xe8]
        call Settings_StoreWOLPasswordFlag
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_4853_004cc3ec]
        pop esi
        ret
    L_441f9d:
        push edi
        mov edi, dword ptr [esi + 0x10c]
        xor edx, edx
        mov eax, dword ptr [g_Data_004da000 + 0x5e574]
        test edi, edi
        setz DL
        mov ecx, dword ptr [eax]
        push edx
        mov edx, dword ptr [esi + 0x104]
        push edx
        mov edx, dword ptr [esi + 0xfc]
        push edx
        push 0x1
        push eax
        call dword ptr [ecx + 0x94]
        mov edi, dword ptr [esi + 0x110]
        xor edx, edx
        mov eax, dword ptr [g_Data_004da000 + 0x5e574]
        test edi, edi
        setz DL
        mov ecx, dword ptr [eax]
        push edx
        mov edx, dword ptr [esi + 0x108]
        push edx
        mov edx, dword ptr [esi + 0x100]
        push edx
        push 0x2
        push eax
        call dword ptr [ecx + 0x94]
        mov ecx, dword ptr [esi + 0xe8]
        pop edi
        call Settings_StoreWOLPasswordFlag
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_4853_004cc3ec]
        pop esi
        ret
    }
}

// 0x00442010 ComboDialog_CommitEdit - if dirty [+0x118]: CB_DELETESTRING(0x144, sel), edit text -> CString +0xfc+sel*4, CB_INSERTSTRING(0x14a, sel, text), clear dirty
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ComboDialog_CommitEdit(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x118]
        test eax, eax
        jz L_442079
        mov eax, dword ptr [esi + 0x114]
        mov ecx, dword ptr [esi + 0x80]
        push edi
        mov edi, dword ptr [g_Iat_SendMessageA_004cc65c]
        push 0x0
        push eax
        push 0x144
        push ecx
        call edi
        mov edx, dword ptr [esi + 0x114]
        lea ecx, [esi + 0x60]
        lea eax, [esi + edx*0x4 + 0xfc]
        push eax
        call dword ptr [g_Iat_MFC42_3874_004cc238]
        mov eax, dword ptr [esi + 0x114]
        mov edx, dword ptr [esi + 0x80]
        mov ecx, dword ptr [esi + eax*0x4 + 0xfc]
        push ecx
        push eax
        push 0x14a
        push edx
        call edi
        mov dword ptr [esi + 0x118], 0x0
        pop edi
    L_442079:
        pop esi
        ret
    }
}

// 0x00442080 ComboDialog_OnSelChange - sel = SendMessage(hwnd [+0x80], CB_GETCURSEL 0x147); [+0x114]=sel; CWnd::SetWindowText(+0xa0, strings [+0x104+sel*4])
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ComboDialog_OnSelChange(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        push 0x0
        push 0x0
        mov eax, dword ptr [esi + 0x80]
        push 0x147
        push eax
        call dword ptr [g_Iat_SendMessageA_004cc65c]
        mov dword ptr [esi + 0x114], eax
        mov ecx, dword ptr [esi + eax*0x4 + 0x104]
        push ecx
        lea ecx, [esi + 0xa0]
        call dword ptr [g_Iat_MFC42_6199_004cc32c]
        pop esi
        ret
    }
}

// 0x00442100 ComboDialog_OnEditChanged - CWnd::GetWindowText(+0xa0) into CString +0x104+sel*4; if it differs (strcmp) from original +0xf4+sel*4: [+0x10c+sel*4]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ComboDialog_OnEditChanged(int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x114]
        lea ecx, [edi + eax*0x4 + 0x104]
        push ecx
        lea ecx, [edi + 0xa0]
        call dword ptr [g_Iat_MFC42_3874_004cc238]
        mov ebp, dword ptr [edi + 0x114]
        mov esi, dword ptr [edi + ebp*0x4 + 0xf4]
        mov eax, dword ptr [edi + ebp*0x4 + 0x104]
    L_442133:
        mov DL, byte ptr [eax]
        mov BL, byte ptr [esi]
        mov CL, DL
        cmp DL, BL
        jnz L_44215b
        test CL, CL
        jz L_442157
        mov DL, byte ptr [eax + 0x1]
        mov BL, byte ptr [esi + 0x1]
        mov CL, DL
        cmp DL, BL
        jnz L_44215b
        add eax, 0x2
        add esi, 0x2
        test CL, CL
        jnz L_442133
    L_442157:
        xor eax, eax
        jmp L_442160
    L_44215b:
        sbb eax, eax
        sbb eax, -0x1
    L_442160:
        xor ecx, ecx
        test eax, eax
        setnz CL
        test CL, CL
        jz L_442176
        mov dword ptr [edi + ebp*0x4 + 0x10c], 0x0
    L_442176:
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}

// 0x00442220 Dialog9D_Ctor - CDialog::CDialog(this, 0x9d, parent) (0x004c5b64); vtbl 0x004d1ed0; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Dialog9D_Ctor(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        push eax
        mov esi, ecx
        push 0x9d
        call dword ptr [g_Iat_MFC42_324_004cc2bc]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5ed0
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004428b0 RecoilApp_Dtor - vtbl 0x004d2020; drain block-allocated list (+0x11c..+0x144, 0x1000-byte blocks; delete blocks when emptied, free map +0x13c); CWinApp::~CWinApp (0x004c5e58)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_Dtor(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        xor ecx, ecx
        mov dword ptr [esi], offset g_RData_004cc000 + 0x6020
        mov eax, dword ptr [esi + 0x144]
        test eax, eax
        setz CL
        test CL, CL
        jnz L_4429a7
        push edi
        push ebp
        push ebx
        mov ebp, 0x4
    L_4428d6:
        mov ebx, dword ptr [esi + 0x124]
        xor edx, edx
        add ebx, ebp
        dec eax
        test eax, eax
        setz DL
        test DL, DL
        mov dword ptr [esi + 0x124], ebx
        mov ecx, ebx
        mov dword ptr [esi + 0x144], eax
        jnz L_442904
        cmp ecx, dword ptr [esi + 0x120]
        jnz L_44298f
    L_442904:
        mov eax, dword ptr [esi + 0x128]
        lea ecx, [eax + 0x4]
        mov dword ptr [esi + 0x128], ecx
        mov edx, dword ptr [eax]
        push edx
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        mov eax, dword ptr [esi + 0x144]
        add esp, 0x4
        xor ecx, ecx
        test eax, eax
        setz CL
        test CL, CL
        jz L_44296e
        lea ebx, [esi + 0x11c]
        xor eax, eax
        xor ecx, ecx
        xor edx, edx
        mov dword ptr [ebx], eax
        xor edi, edi
        mov dword ptr [ebx + 0x4], ecx
        mov dword ptr [ebx + 0x8], edx
        mov dword ptr [ebx + 0xc], edi
        lea ebx, [esi + 0x12c]
        mov dword ptr [esi + 0x12c], eax
        mov dword ptr [ebx + 0x4], ecx
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [esi + 0x13c]
        push edx
        mov dword ptr [ebx + 0xc], edi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        jmp L_44298f
    L_44296e:
        mov eax, dword ptr [esi + 0x128]
        lea ebx, [esi + 0x11c]
        mov ecx, dword ptr [eax]
        mov dword ptr [ebx], ecx
        mov edi, ecx
        lea edx, [ecx + 0x1000]
        mov dword ptr [ebx + 0x4], edx
        mov dword ptr [ebx + 0x8], edi
        mov dword ptr [ebx + 0xc], eax
    L_44298f:
        mov eax, dword ptr [esi + 0x144]
        xor ecx, ecx
        test eax, eax
        setz CL
        test CL, CL
        jz L_4428d6
        pop ebx
        pop ebp
        pop edi
    L_4429a7:
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_815_004cc250]
        pop esi
        ret
    }
}

// 0x004429d0 RecoilApp_InitInstance - CWinApp::Enable3dControls (0x004c5e5e); main wnd [+0x20] = vfunc+0xb8(); register app in engine struct: [RecoilApp_GetField20()+0xc0] = this; ShowWindow(5) (0x004c5bb8); UpdateWindow; return 1
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_InitInstance(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call dword ptr [g_Iat_MFC42_2621_004cc254]
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0xb8]
        mov ecx, esi
        mov dword ptr [esi + 0x20], eax
        call RecoilApp_GetField20
        mov dword ptr [eax + 0xc0], esi
        mov ecx, dword ptr [esi + 0x20]
        push 0x5
        call dword ptr [g_Iat_MFC42_6215_004cc3bc]
        mov ecx, dword ptr [esi + 0x20]
        mov edx, dword ptr [ecx + 0x20]
        push edx
        call dword ptr [g_Iat_UpdateWindow_004cc6b0]
        mov eax, 0x1
        pop esi
        ret
    }
}

// 0x00442c70 RecoilApp_Constructor - CWinApp::CWinApp(this, 0) (0x004c5e64); byte +0x118; zero +0x11c..+0x144, +0xd0, +0xc4; vtbl 0x004d2020; [+0xc8]=-1; zero 0x10 dwords at +0xd8
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_Constructor(int, int)
{
    __asm {
        push ecx
        push esi
        push edi
        xor edi, edi
        mov esi, ecx
        push edi
        call dword ptr [g_Iat_MFC42_561_004cc258]
        mov AL, byte ptr [esp + 0xb]
        mov ecx, 0x10
        mov byte ptr [esi + 0x118], AL
        mov dword ptr [esi + 0x11c], edi
        mov dword ptr [esi + 0x120], edi
        mov dword ptr [esi + 0x124], edi
        mov dword ptr [esi + 0x128], edi
        mov dword ptr [esi + 0x12c], edi
        mov dword ptr [esi + 0x130], edi
        mov dword ptr [esi + 0x134], edi
        mov dword ptr [esi + 0x138], edi
        mov dword ptr [esi + 0x13c], edi
        mov dword ptr [esi + 0x140], edi
        mov dword ptr [esi + 0x144], edi
        mov dword ptr [esi + 0xd0], edi
        mov dword ptr [esi + 0xc4], edi
        xor eax, eax
        lea edi, [esi + 0xd8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x6020
        mov dword ptr [esi + 0xc8], 0xffffffff
        rep stosd
        mov eax, esi
        pop edi
        pop esi
        pop ecx
        ret
    }
}

// 0x004c81c0 WinMain_Thunk - bytes ret 0x10: AfxWinMain(hInst,hPrev,cmd,show)
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall WinMain_Thunk(int, int, int, int, int, int)
{
    __asm {
        push dword ptr [esp + 0x10]
        push dword ptr [esp + 0x10]
        push dword ptr [esp + 0x10]
        push dword ptr [esp + 0x10]
        call dword ptr [g_Iat_MFC42_1576_004cc3e0]
        ret 0x10
    }
}

// 0x004c81d8 Afx_SetMbcsState - bytes ret 8: AfxGetModuleState; state[0x14]=(byte)a; state+0x1040=b; a==0 -> _setmbcp(-3)
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Afx_SetMbcsState(int, int, int, int)
{
    __asm {
        call dword ptr [g_Iat_MFC42_1168_004cc19c]
        mov ecx, dword ptr [esp + 0x4]
        mov edx, dword ptr [esp + 0x8]
        test ecx, ecx
        mov byte ptr [eax + 0x14], CL
        mov dword ptr [eax + 0x1040], edx
        jnz L_4c81fb
        push -0x3
        call dword ptr [g_Iat__setmbcp_004cc5e0]
        pop ecx
    L_4c81fb:
        push 0x1
        pop eax
        ret 0x8
    }
}

// 0x004429b0 RecoilApp_ScalarDeletingDtor - dtor 0x004428b0; flag&1 -> delete; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall RecoilApp_ScalarDeletingDtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call RecoilApp_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_4429c8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4429c8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0042de10 Slot_ReturnConstAddr_004d0990_0042de10 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ReturnConstAddr_004d0990_0042de10(int, int)
{
    __asm {
        mov eax, offset g_RData_004cc000 + 0x4990
        ret
    }
}

// 0x0042eb00 Slot_ReturnTrue_Ret8_0042eb00 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Slot_ReturnTrue_Ret8_0042eb00(int, int, int, int)
{
    __asm {
        mov eax, 0x1
        ret 0x8
    }
}

// 0x004308c0 MainWnd_EnableVideoModes - mode=0x004086b0(); software: menu item flags +0x1e0..+0x1f0 for modes 2..6 = 8 (checked) if current else 0; hardware: modes 2/3 disabled (1), 4/5 check flags, mode 6 disabled when VRAM [+0x228] <= 0x2bf200, mode 7 (+0x1f4) disabled when VRAM <= 0x480000, else check flag
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_EnableVideoModes(int, int)
{
    __asm {
        push esi
        push edi
        mov esi, ecx
        call Setting_Get_DisplayMode
        mov edi, eax
        call Settings_GetHWCardFlag
        test eax, eax
        jz L_430940
        xor eax, eax
        cmp edi, 0x4
        setnz AL
        dec eax
        xor edx, edx
        and eax, 0x8
        cmp edi, 0x5
        setnz DL
        mov dword ptr [esi + 0x1e8], eax
        mov eax, dword ptr [esi + 0x228]
        dec edx
        mov ecx, 0x1
        and edx, 0x8
        cmp eax, 0x2bf200
        mov dword ptr [esi + 0x1e0], ecx
        mov dword ptr [esi + 0x1e4], ecx
        mov dword ptr [esi + 0x1ec], edx
        jbe L_43092a
        xor edx, edx
        cmp edi, 0x6
        setnz DL
        dec edx
        and edx, 0x8
        mov dword ptr [esi + 0x1f0], edx
        jmp L_430930
    L_43092a:
        mov dword ptr [esi + 0x1f0], ecx
    L_430930:
        cmp eax, 0x480000
        ja L_43099a
        mov dword ptr [esi + 0x1f4], ecx
        pop edi
        pop esi
        ret
    L_430940:
        xor ecx, ecx
        cmp edi, 0x2
        setnz CL
        dec ecx
        xor edx, edx
        and ecx, 0x8
        cmp edi, 0x3
        setnz DL
        dec edx
        xor eax, eax
        and edx, 0x8
        cmp edi, 0x4
        setnz AL
        dec eax
        mov dword ptr [esi + 0x1e0], ecx
        and eax, 0x8
        xor ecx, ecx
        cmp edi, 0x5
        mov dword ptr [esi + 0x1e4], edx
        setnz CL
        dec ecx
        xor edx, edx
        and ecx, 0x8
        cmp edi, 0x6
        setnz DL
        dec edx
        mov dword ptr [esi + 0x1e8], eax
        and edx, 0x8
        mov dword ptr [esi + 0x1ec], ecx
        mov dword ptr [esi + 0x1f0], edx
    L_43099a:
        xor eax, eax
        cmp edi, 0x7
        setnz AL
        dec eax
        pop edi
        and eax, 0x8
        mov dword ptr [esi + 0x1f4], eax
        pop esi
        ret
    }
}

// 0x00431a90 Slot_ZeroECX_Jmp_RecoilApp_ApplySoundAPISetting_00431a90 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ZeroECX_Jmp_RecoilApp_ApplySoundAPISetting_00431a90(int, int)
{
    __asm {
        xor ecx, ecx
        jmp RecoilApp_ApplySoundAPISetting
    }
}

// 0x004420c0 Slot_SetField118_To1_004420c0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_SetField118_To1_004420c0(int, int)
{
    __asm {
        mov dword ptr [ecx + 0x118], 0x1
        ret
    }
}

// 0x004420d0 Thunk_ComboDialog_CommitEdit_004420d0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_ComboDialog_CommitEdit_004420d0(int, int)
{
    __asm {
        jmp ComboDialog_CommitEdit
    }
}

// 0x00442240 Dialog9D_ScalarDeletingDtor - CDialog dtor (0x0043f440); flag&1 -> delete; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Dialog9D_ScalarDeletingDtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        test byte ptr [esp + 0x8], 0x1
        jz L_442258
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_442258:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00442890 Slot_ReturnGlobal_004cc248_00442890 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ReturnGlobal_004cc248_00442890(int, int)
{
    __asm {
        mov eax, dword ptr [g_Iat_MFC42_4274_004cc248]
        ret
    }
}

// 0x00443790 Slot_ReturnGlobal_004cc25c_00443790 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ReturnGlobal_004cc25c_00443790(int, int)
{
    __asm {
        mov eax, dword ptr [g_Iat_MFC42_1842_004cc25c]
        ret
    }
}

// 0x004437b0 Slot_ReturnGlobal_004cc260_004437b0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ReturnGlobal_004cc260_004437b0(int, int)
{
    __asm {
        mov eax, dword ptr [g_Iat_MFC42_4242_004cc260]
        ret
    }
}

// 0x004437c0 Slot_ReturnConstAddr_004d20f8_004437c0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ReturnConstAddr_004d20f8_004437c0(int, int)
{
    __asm {
        mov eax, offset g_RData_004cc000 + 0x60f8
        ret
    }
}

// 0x004438f0 Thunk_004c5e88_004438f0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_004c5e88_004438f0(int, int)
{
    __asm {
        jmp dword ptr [g_Iat_MFC42_4413_004cc278]
    }
}

// 0x00443a40 Window_CacheClientRectScreen_00443a40 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Window_CacheClientRectScreen_00443a40(int, int)
{
    __asm {
        call Video_IsWindowedNonGlide
        test eax, eax
        jz L_443a4e
        jmp zVideo_CacheClientRectScreen
    L_443a4e:
        ret
    }
}

// 0x00472d30 Crt_MathErrHandler_00472d30 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Crt_MathErrHandler_00472d30(int, int)
{
    __asm {
        sub esp, 0x8
        push ebx
        push ebp
        push esi
        push edi
        mov edi, dword ptr [esp + 0x1c]
        mov eax, dword ptr [edi + 0x14]
        mov ecx, dword ptr [edi + 0x10]
        mov edx, dword ptr [edi + 0xc]
        push eax
        mov eax, dword ptr [edi + 0x8]
        push ecx
        mov ecx, dword ptr [edi + 0x4]
        push edx
        mov edx, dword ptr [edi]
        push eax
        push ecx
        push edx
        push offset g_Data_004da000 + 0x6efc
        push 0x178
        push offset g_Data_004da000 + 0x6ed4
        push 0x400
        call Debug_ReportNoop
        mov eax, dword ptr [edi + 0x14]
        mov ecx, dword ptr [edi + 0x10]
        mov edx, dword ptr [edi + 0xc]
        add esp, 0x28
        push eax
        mov eax, dword ptr [edi + 0x8]
        push ecx
        mov ecx, dword ptr [edi + 0x4]
        push edx
        mov edx, dword ptr [edi]
        push eax
        mov eax, dword ptr [g_Iat__iob_004cc4f8]
        push ecx
        push edx
        add eax, 0x40
        push offset g_Data_004da000 + 0x6ea8
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        mov ebp, dword ptr [edi + 0x4]
        add esp, 0x20
        mov esi, offset g_Data_004da000 + 0x6ea0
        mov eax, ebp
        xor edx, edx
    L_472da8:
        mov BL, byte ptr [eax]
        mov CL, BL
        cmp BL, byte ptr [esi]
        jnz L_472dcc
        cmp CL, DL
        jz L_472dc8
        mov BL, byte ptr [eax + 0x1]
        mov CL, BL
        cmp BL, byte ptr [esi + 0x1]
        jnz L_472dcc
        add eax, 0x2
        add esi, 0x2
        cmp CL, DL
        jnz L_472da8
    L_472dc8:
        xor eax, eax
        jmp L_472dd1
    L_472dcc:
        sbb eax, eax
        sbb eax, -0x1
    L_472dd1:
        cmp eax, edx
        jnz L_472e33
        fld qword ptr [edi + 0x8]
        fcom qword ptr [g_RData_004cc000 + 0x6950]
        fst qword ptr [esp + 0x10]
        fnstsw AX
        test AH, 0x41
        jnz L_472dfd
        fstp st(0)
        mov dword ptr [esp + 0x10], 0x0
        mov dword ptr [esp + 0x14], 0x3ff00000
        jmp L_472e1a
    L_472dfd:
        fcomp qword ptr [g_RData_004cc000 + 0x6958]
        fnstsw AX
        test AH, 0x1
        jz L_472e1a
        mov dword ptr [esp + 0x10], 0x0
        mov dword ptr [esp + 0x14], 0xbff00000
    L_472e1a:
        fld qword ptr [esp + 0x10]
        call dword ptr [g_Iat__CIasin_004cc4d0]
        fstp qword ptr [edi + 0x18]
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret
    L_472e33:
        mov esi, offset g_Data_004da000 + 0x6e98
        mov eax, ebp
    L_472e3a:
        mov BL, byte ptr [eax]
        mov CL, BL
        cmp BL, byte ptr [esi]
        jnz L_472e5e
        cmp CL, DL
        jz L_472e5a
        mov BL, byte ptr [eax + 0x1]
        mov CL, BL
        cmp BL, byte ptr [esi + 0x1]
        jnz L_472e5e
        add eax, 0x2
        add esi, 0x2
        cmp CL, DL
        jnz L_472e3a
    L_472e5a:
        xor eax, eax
        jmp L_472e63
    L_472e5e:
        sbb eax, eax
        sbb eax, -0x1
    L_472e63:
        cmp eax, edx
        jnz L_472e7a
        mov dword ptr [edi + 0x18], edx
        mov dword ptr [edi + 0x1c], edx
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret
    L_472e7a:
        mov esi, offset g_Data_004da000 + 0x6e90
        mov eax, ebp
    L_472e81:
        mov BL, byte ptr [eax]
        mov CL, BL
        cmp BL, byte ptr [esi]
        jnz L_472ea5
        cmp CL, DL
        jz L_472ea1
        mov BL, byte ptr [eax + 0x1]
        mov CL, BL
        cmp BL, byte ptr [esi + 0x1]
        jnz L_472ea5
        add eax, 0x2
        add esi, 0x2
        cmp CL, DL
        jnz L_472e81
    L_472ea1:
        xor eax, eax
        jmp L_472eaa
    L_472ea5:
        sbb eax, eax
        sbb eax, -0x1
    L_472eaa:
        cmp eax, edx
        jnz L_472ec1
        mov dword ptr [edi + 0x18], edx
        mov dword ptr [edi + 0x1c], edx
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret
    L_472ec1:
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0x8
        ret
    }
}

// 0x004309b0 MainWnd_OnVideoMode2 - bytes: Settings_ApplyVideoModePreset(2); MainWnd_EnableVideoModes 0x004308c0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnVideoMode2(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, 0x2
        call Settings_ApplyVideoModePreset
        mov ecx, esi
        call MainWnd_EnableVideoModes
        pop esi
        ret
    }
}

// 0x004309d0 MainWnd_OnVideoMode3 - bytes: Settings_ApplyVideoModePreset(3); MainWnd_EnableVideoModes
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnVideoMode3(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, 0x3
        call Settings_ApplyVideoModePreset
        mov ecx, esi
        call MainWnd_EnableVideoModes
        pop esi
        ret
    }
}

// 0x004309f0 MainWnd_OnVideoMode4 - bytes: Settings_ApplyVideoModePreset(4); MainWnd_EnableVideoModes
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnVideoMode4(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, 0x4
        call Settings_ApplyVideoModePreset
        mov ecx, esi
        call MainWnd_EnableVideoModes
        pop esi
        ret
    }
}

// 0x00430a10 MainWnd_OnVideoMode5 - bytes: Settings_ApplyVideoModePreset(5); MainWnd_EnableVideoModes
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnVideoMode5(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, 0x5
        call Settings_ApplyVideoModePreset
        mov ecx, esi
        call MainWnd_EnableVideoModes
        pop esi
        ret
    }
}

// 0x00430a30 MainWnd_OnVideoMode6 - bytes: Settings_ApplyVideoModePreset(6); MainWnd_EnableVideoModes
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnVideoMode6(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, 0x6
        call Settings_ApplyVideoModePreset
        mov ecx, esi
        call MainWnd_EnableVideoModes
        pop esi
        ret
    }
}

// 0x00430a50 MainWnd_OnVideoMode7 - bytes: Settings_ApplyVideoModePreset(7); MainWnd_EnableVideoModes
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnVideoMode7(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        mov ecx, 0x7
        call Settings_ApplyVideoModePreset
        mov ecx, esi
        call MainWnd_EnableVideoModes
        pop esi
        ret
    }
}

// 0x004c62b7 DataTarget_004c62b7 - the __except handler of entry 0x004c6140 (scope table 0x004d4348, handler word
// 0x004d4350). Entered by _except_handler3 with entry's EBP: restores ESP from [ebp-0x18], _exit([ebp-0x78]) (the
// exception code saved by the filter), then unlinks the SEH frame and returns. CONFIRMED-BINARY: listing 0x004c62b7..
// 0x004c62de (15 instructions); entry's port keeps the same frame layout. Was left as a raw original VA (2026-10-07).
__declspec(naked) int __fastcall DataTarget_004c62b7(int, int)
{
    __asm {
        mov esp, dword ptr [ebp - 0x18]
        mov edx, dword ptr [ebp - 0x78]
        push edx
        call dword ptr [g_Iat__exit_004cc564]
        add esp, 0x4
        mov dword ptr [ebp - 0x4], 0xffffffff
        mov ecx, dword ptr [ebp - 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004c6140 entry - bytes: CRT startup: SEH 0x004d4348; Stub_Ret; Crt_SetFpuControl 0x004c6350; __getmainargs/initterm x2 (0x004da0b4..bc); GetStartupInfoA/GetModuleHandleA; AfxWinMain thunk 0x004c81c0; exit
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall entry(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push -0x1
        push offset g_RData_004cc000 + 0x8348
        push dword ptr [g_Iat__except_handler3_004cc554]
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        add esp, -0x68
        push ebx
        push esi
        push edi
        mov dword ptr [ebp - 0x18], esp
        mov dword ptr [ebp - 0x4], 0x0
        push 0x2
        call dword ptr [g_Iat___set_app_type_004cc57c]
        add esp, 0x4
        mov dword ptr [g_Data_004da000 + 0x29fab8], 0xffffffff
        mov dword ptr [g_Data_004da000 + 0x29fabc], 0xffffffff
        call dword ptr [g_Iat___p__fmode_004cc580]
        mov ecx, dword ptr [g_Data_004da000 + 0x92bc4]
        mov dword ptr [eax], ecx
        call dword ptr [g_Iat___p__commode_004cc584]
        mov edx, dword ptr [g_Data_004da000 + 0x92bc0]
        mov dword ptr [eax], edx
        mov eax, dword ptr [g_Iat__adjust_fdiv_004cc588]
        mov ecx, dword ptr [eax]
        mov dword ptr [g_Data_004da000 + 0x29fab0], ecx
        call Stub_Ret
        mov eax, dword ptr [g_Data_004da000 + 0x29fab4]
        test eax, eax
        jnz L_4c61d1
        push offset Crt_MathErrHandler_00472d30
        call dword ptr [g_Iat___setusermatherr_004cc58c]
        add esp, 0x4
    L_4c61d1:
        call Crt_SetFpuControl
        push offset g_Data_004da000 + 0xbc
        push offset g_Data_004da000 + 0xb8
        call dword ptr [g_Iat__initterm_004cc4e4]
        add esp, 0x8
        mov edx, dword ptr [g_Data_004da000 + 0x92bbc]
        mov dword ptr [ebp - 0x6c], edx
        lea eax, [ebp - 0x6c]
        push eax
        mov ecx, dword ptr [g_Data_004da000 + 0x92bb8]
        push ecx
        lea edx, [ebp - 0x64]
        push edx
        lea eax, [ebp - 0x70]
        push eax
        lea ecx, [ebp - 0x60]
        push ecx
        call dword ptr [g_Iat___getmainargs_004cc570]
        add esp, 0x14
        push offset g_Data_004da000 + 0xb4
        push offset g_Data_004da000 + 0x0
        call dword ptr [g_Iat__initterm_004cc4e4]
        add esp, 0x8
        mov edx, dword ptr [g_Iat__acmdln_004cc56c]
        mov esi, dword ptr [edx]
        mov dword ptr [ebp - 0x74], esi
        cmp byte ptr [esi], 0x22
        jnz L_4c62df
    L_4c6237:
        inc esi
        mov dword ptr [ebp - 0x74], esi
        mov AL, byte ptr [esi]
        test AL, AL
        jz L_4c6245
        cmp AL, 0x22
        jnz L_4c6237
    L_4c6245:
        cmp byte ptr [esi], 0x22
        jnz L_4c624e
        inc esi
        mov dword ptr [ebp - 0x74], esi
    L_4c624e:
        mov AL, byte ptr [esi]
        test AL, AL
        jz L_4c625e
        cmp AL, 0x20
        ja L_4c625e
        inc esi
        mov dword ptr [ebp - 0x74], esi
        jmp L_4c624e
    L_4c625e:
        mov dword ptr [ebp - 0x30], 0x0
        lea eax, [ebp - 0x5c]
        push eax
        call dword ptr [g_Iat_GetStartupInfoA_004cc168]
        test byte ptr [ebp - 0x30], 0x1
        jz L_4c627f
        mov eax, dword ptr [ebp - 0x2c]
        and eax, 0xffff
        jmp L_4c6284
    L_4c627f:
        mov eax, 0xa
    L_4c6284:
        push eax
        push esi
        push 0x0
        push 0x0
        call dword ptr [g_Iat_GetModuleHandleA_004cc0d4]
        push eax
        call WinMain_Thunk
        mov dword ptr [ebp - 0x68], eax
        push eax
        call dword ptr [g_Iat_exit_004cc4b0]
        jmp L_4c62c4
    L_4c62c4:
        add esp, 0x4
        mov dword ptr [ebp - 0x4], 0xffffffff
        mov ecx, dword ptr [ebp - 0x10]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_4c62df:
        cmp byte ptr [esi], 0x20
        jbe L_4c624e
        inc esi
        mov dword ptr [ebp - 0x74], esi
        jmp L_4c62df
    }
}

// 0x0042e9f0 App_SwallowSysKeyMessage_IfHWCard_0042e9f0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall App_SwallowSysKeyMessage_IfHWCard_0042e9f0(int, int, int)
{
    __asm {
        push esi
        xor esi, esi
        call Settings_GetHWCardFlag
        test eax, eax
        jz L_42ea16
        mov eax, dword ptr [esp + 0x8]
        mov eax, dword ptr [eax + 0x4]
        cmp eax, 0x104
        jc L_42ea16
        cmp eax, 0x105
        ja L_42ea16
        mov esi, 0x1
    L_42ea16:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x004308a0 Slot_PostMessage_WMCLOSE_004308a0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_PostMessage_WMCLOSE_004308a0(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x20]
        push 0x0
        push 0x0
        push 0x10
        push eax
        call dword ptr [g_Iat_PostMessageA_004cc684]
        ret
    }
}

// 0x004313d0 Screen_UpdateButtonPair_ByState1E0_004313d0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_UpdateButtonPair_ByState1E0_004313d0(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x1e0]
        cmp eax, 0x1
        mov eax, dword ptr [esi]
        jnz L_4313f9
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_4313f9:
        push 0x1
        mov ecx, esi
        call dword ptr [eax]
        cmp dword ptr [edi + 0x1e0], 0x8
        jnz L_431416
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431416:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431430 Screen_UpdateButtonPair_ByState1E4_00431430 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_UpdateButtonPair_ByState1E4_00431430(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x1e4]
        cmp eax, 0x1
        mov eax, dword ptr [esi]
        jnz L_431459
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431459:
        push 0x1
        mov ecx, esi
        call dword ptr [eax]
        cmp dword ptr [edi + 0x1e4], 0x8
        jnz L_431476
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431476:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431490 Screen_UpdateButtonPair_ByState1E8_00431490 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_UpdateButtonPair_ByState1E8_00431490(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x1e8]
        cmp eax, 0x1
        mov eax, dword ptr [esi]
        jnz L_4314b9
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_4314b9:
        push 0x1
        mov ecx, esi
        call dword ptr [eax]
        cmp dword ptr [edi + 0x1e8], 0x8
        jnz L_4314d6
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_4314d6:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004314f0 Screen_UpdateButtonPair_ByState1EC_004314f0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_UpdateButtonPair_ByState1EC_004314f0(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x1ec]
        cmp eax, 0x1
        mov eax, dword ptr [esi]
        jnz L_431519
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431519:
        push 0x1
        mov ecx, esi
        call dword ptr [eax]
        cmp dword ptr [edi + 0x1ec], 0x8
        jnz L_431536
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431536:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431550 Screen_UpdateButtonPair_ByState1F0_00431550 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_UpdateButtonPair_ByState1F0_00431550(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x1f0]
        cmp eax, 0x1
        mov eax, dword ptr [esi]
        jnz L_431579
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431579:
        push 0x1
        mov ecx, esi
        call dword ptr [eax]
        cmp dword ptr [edi + 0x1f0], 0x8
        jnz L_431596
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431596:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004315b0 Screen_UpdateButtonPair_ByState1F4_004315b0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_UpdateButtonPair_ByState1F4_004315b0(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x1f4]
        cmp eax, 0x1
        mov eax, dword ptr [esi]
        jnz L_4315d9
        push 0x0
        mov ecx, esi
        call dword ptr [eax]
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_4315d9:
        push 0x1
        mov ecx, esi
        call dword ptr [eax]
        cmp dword ptr [edi + 0x1f4], 0x8
        jnz L_4315f6
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_4315f6:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431870 Slot_CallVirtual0_1_ThenVirtual4_ByMode8_00431870 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slot_CallVirtual0_1_ThenVirtual4_ByMode8_00431870(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        mov edi, ecx
        mov eax, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [eax]
        cmp dword ptr [edi + 0x1fc], 0x8
        jnz L_431897
        mov edx, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [edx + 0x4]
        pop edi
        pop esi
        ret 0x4
    L_431897:
        mov eax, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [eax + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004318e0 Slot_RemoveMenuItem_9c4e_004318e0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slot_RemoveMenuItem_9c4e_004318e0(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push 0x0
        push 0x9c4e
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [ecx + 0x4]
        push edx
        call dword ptr [g_Iat_RemoveMenu_004cc618]
        ret 0x4
    }
}

// 0x00431a80 Slot_CallVirtual0_Arg1_00431a80 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slot_CallVirtual0_Arg1_00431a80(int, int, int)
{
    __asm {
        mov ecx, dword ptr [esp + 0x4]
        push 0x1
        mov eax, dword ptr [ecx]
        call dword ptr [eax]
        ret 0x4
    }
}

// 0x00442a30 Slot_ExchangeField0xD0_With1_00442a30 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ExchangeField0xD0_With1_00442a30(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xd0]
        mov dword ptr [ecx + 0xd0], 0x1
        ret
    }
}

// 0x004438a0 Slot_CallVirtual10_IsZero_004438a0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slot_CallVirtual10_IsZero_004438a0(int, int, int)
{
    __asm {
        mov ecx, dword ptr [esp + 0x4]
        test ecx, ecx
        jz L_4438b5
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x10]
        neg eax
        sbb eax, eax
        inc eax
        ret 0x4
    L_4438b5:
        xor eax, eax
        ret 0x4
    }
}

// 0x004438c0 App_GetCompanyNameString_004438c0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall App_GetCompanyNameString_004438c0(int, int, int)
{
    __asm {
        push ecx
        push esi
        mov esi, dword ptr [esp + 0xc]
        push offset g_Data_004da000 + 0x38f0
        mov ecx, esi
        mov dword ptr [esp + 0x8], 0x0
        call dword ptr [g_Iat_MFC42_537_004cc29c]
        mov eax, esi
        pop esi
        pop ecx
        ret 0x4
    }
}

// 0x00443a60 Window_LoadGameBitmap_00443a60 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Window_LoadGameBitmap_00443a60(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push eax
        call dword ptr [g_Iat_MFC42_4457_004cc28c]
        cmp eax, -0x1
        jnz L_443a78
        or eax, eax
        pop esi
        ret 0x4
    L_443a78:
        push offset g_Data_004da000 + 0x3904
        push 0x2
        push offset g_Data_004da000 + 0x3904
        call dword ptr [g_Iat_MFC42_1146_004cc1b8]
        push eax
        call dword ptr [g_Iat_LoadBitmapA_004cc6c0]
        push eax
        lea ecx, [esi + 0xc4]
        call dword ptr [g_Iat_MFC42_1641_004cc288]
        call Input_Mouse_ShutdownDevice
        xor eax, eax
        pop esi
        ret 0x4
    }
}

// 0x00443b50 Slot_ForwardVirtualB4_Ret8_00443b50 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Slot_ForwardVirtualB4_Ret8_00443b50(int, int, int, int)
{
    __asm {
        mov edx, dword ptr [esp + 0x8]
        mov ecx, dword ptr [ecx + 0xc0]
        push edx
        mov edx, dword ptr [esp + 0x8]
        mov eax, dword ptr [ecx]
        push edx
        call dword ptr [eax + 0xb4]
        ret 0x8
    }
}

// 0x004c6300 type_info_scalar_deleting_dtor - bytes ret 4: type_info::~type_info; flag&1 -> operator delete
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall type_info_scalar_deleting_dtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call dword ptr [g_Iat___1type_info__UAE_XZ_004cc578]
        test byte ptr [esp + 0x8], 0x1
        jz L_4c6318
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4c6318:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00430a70 Settings_StoreHUDFlagFromSplitScreen_00430a70 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_StoreHUDFlagFromSplitScreen_00430a70(int, int)
{
    __asm {
        call Settings_GetSplitScreenValue
        mov ecx, eax
        neg ecx
        sbb ecx, ecx
        inc ecx
        jmp Settings_StoreHUDFlag
    }
}

// 0x00431900 Settings_ToggleCDAudio_00431900 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_ToggleCDAudio_00431900(int, int)
{
    __asm {
        call Setting_Get_CDAudio
        mov ecx, eax
        neg ecx
        sbb ecx, ecx
        inc ecx
        jmp Settings_StoreCDAudio
    }
}

// 0x00431950 Settings_ToggleJoystickEnabled_00431950 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_ToggleJoystickEnabled_00431950(int, int)
{
    __asm {
        call Setting_Get_004e5d60
        mov ecx, eax
        neg ecx
        sbb ecx, ecx
        inc ecx
        jmp Settings_StoreJoystickEnabled
    }
}

// 0x00431ad0 Settings_ApplySoundAPISetting1_00431ad0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_ApplySoundAPISetting1_00431ad0(int, int)
{
    __asm {
        mov ecx, 0x1
        jmp RecoilApp_ApplySoundAPISetting
    }
}

// 0x00443a50 Wnd_OnMove_DefaultThenCacheClientRect_00443a50 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Wnd_OnMove_DefaultThenCacheClientRect_00443a50(int, int, int, int)
{
    __asm {
        call dword ptr [g_Iat_MFC42_2379_004cc2a4]
        call Window_CacheClientRectScreen_00443a40
        ret 0x8
    }
}


// 0x004318b0 Slot_Call4317d0_Mode1_004318b0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slot_Call4317d0_Mode1_004318b0(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push 0x1
        push eax
        call Screen_UpdatePanelButtonAndAccelerator_004317d0
        ret 0x4
    }
}

// 0x004318c0 Slot_Call4317d0_Mode2_004318c0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slot_Call4317d0_Mode2_004318c0(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push 0x2
        push eax
        call Screen_UpdatePanelButtonAndAccelerator_004317d0
        ret 0x4
    }
}

// 0x004318d0 Slot_Call4317d0_Mode3_004318d0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Slot_Call4317d0_Mode3_004318d0(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push 0x3
        push eax
        call Screen_UpdatePanelButtonAndAccelerator_004317d0
        ret 0x4
    }
}

// 0x00404c80 Loader_StartIfReady - if loader [0x004e5cb4]: 0x00404400(ECX)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Loader_StartIfReady(int, int)
{
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [g_Data_004da000 + 0xbcb4]
        test ecx, ecx
        jz L_404c92
        push eax
        call Briefing_BuildObjectiveSequence
    L_404c92:
        ret
    }
}

// 0x0042dc30 Com_QueryAndCallSlot14 - (obj,a,b,c) ret 0x10: obj->QueryInterface(iid 0x004d43a0); on success other->vfunc+0x10(..,&itf); itf->vfunc+0x14(..,obj); release itf and other; return HRESULT
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall Com_QueryAndCallSlot14(int, int, int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_Com_QueryAndCallSlot14
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x8
        push ebx
        xor ebx, ebx
        push esi
        mov dword ptr [esp + 0xc], ebx
        mov dword ptr [esp + 0x18], ebx
        mov dword ptr [esp + 0x8], ebx
        mov eax, dword ptr [esp + 0x20]
        lea edx, [esp + 0xc]
        push edx
        push offset g_RData_004cc000 + 0x83a0
        mov ecx, dword ptr [eax]
        push eax
        mov byte ptr [esp + 0x24], 0x1
        call dword ptr [ecx]
        mov esi, eax
        cmp esi, ebx
        jl L_42dca6
        mov eax, dword ptr [esp + 0xc]
        lea edx, [esp + 0x8]
        push edx
        mov edx, dword ptr [esp + 0x2c]
        mov ecx, dword ptr [eax]
        push edx
        push eax
        call dword ptr [ecx + 0x10]
        mov esi, eax
        cmp esi, ebx
        jl L_42dca6
        mov edx, dword ptr [esp + 0x2c]
        mov eax, dword ptr [esp + 0x8]
        push edx
        mov edx, dword ptr [esp + 0x28]
        mov ecx, dword ptr [eax]
        push edx
        push eax
        call dword ptr [ecx + 0x14]
        mov esi, eax
    L_42dca6:
        mov eax, dword ptr [esp + 0x8]
        mov byte ptr [esp + 0x18], BL
        cmp eax, ebx
        jz L_42dcb8
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx + 0x8]
    L_42dcb8:
        mov eax, dword ptr [esp + 0xc]
        mov dword ptr [esp + 0x18], 0xffffffff
        cmp eax, ebx
        jz L_42dcce
        mov edx, dword ptr [eax]
        push eax
        call dword ptr [edx + 0x8]
    L_42dcce:
        mov ecx, dword ptr [esp + 0x10]
        mov eax, esi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x14
        ret 0x10
    }
}

// 0x0042dcf0 Com_QueryAndCallSlot18 - (obj,a,b) ret 0xc: as 0x0042dc30 but final call itf->vfunc+0x18(arg); release; return HRESULT
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall Com_QueryAndCallSlot18(int, int, int, int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_Com_QueryAndCallSlot18
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x8
        push ebx
        xor ebx, ebx
        push esi
        mov dword ptr [esp + 0xc], ebx
        mov dword ptr [esp + 0x18], ebx
        mov dword ptr [esp + 0x8], ebx
        mov eax, dword ptr [esp + 0x20]
        lea edx, [esp + 0xc]
        push edx
        push offset g_RData_004cc000 + 0x83a0
        mov ecx, dword ptr [eax]
        push eax
        mov byte ptr [esp + 0x24], 0x1
        call dword ptr [ecx]
        mov esi, eax
        cmp esi, ebx
        jl L_42dd61
        mov eax, dword ptr [esp + 0xc]
        lea edx, [esp + 0x8]
        push edx
        mov edx, dword ptr [esp + 0x28]
        mov ecx, dword ptr [eax]
        push edx
        push eax
        call dword ptr [ecx + 0x10]
        mov esi, eax
        cmp esi, ebx
        jl L_42dd61
        mov eax, dword ptr [esp + 0x8]
        mov edx, dword ptr [esp + 0x28]
        push edx
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x18]
        mov esi, eax
    L_42dd61:
        mov eax, dword ptr [esp + 0x8]
        mov byte ptr [esp + 0x18], BL
        cmp eax, ebx
        jz L_42dd73
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx + 0x8]
    L_42dd73:
        mov eax, dword ptr [esp + 0xc]
        mov dword ptr [esp + 0x18], 0xffffffff
        cmp eax, ebx
        jz L_42dd89
        mov edx, dword ptr [eax]
        push eax
        call dword ptr [edx + 0x8]
    L_42dd89:
        mov ecx, dword ptr [esp + 0x10]
        mov eax, esi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x14
        ret 0xc
    }
}

// 0x0042de20 StaticInitWrapper_0042de20 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticInitWrapper_0042de20(int, int)
{
    __asm {
        call StaticInit_004f3ca8
        jmp StaticInit_RegisterAtexit_0042de50
    }
}

// 0x0042de30 StaticInit_004f3ca8 - ECX=0x004f3ca8; tail jmp 0x0042dfa0 (ctor)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticInit_004f3ca8(int, int)
{
    __asm {
        mov ecx, offset g_Data_004da000 + 0x19ca8
        jmp MainApp_Ctor
    }
}

// 0x0042de40 StaticInit_RegisterAtexit_0042de50 - atexit(0x0042de50) via 0x004c60e0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticInit_RegisterAtexit_0042de50(int, int)
{
    __asm {
        push offset AtexitStub_MainApp_Dtor_0042de50
        call crt_atexit
        add esp, 0x4
        ret
    }
}

// 0x0042de50 AtexitStub_MainApp_Dtor_0042de50 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AtexitStub_MainApp_Dtor_0042de50(int, int)
{
    __asm {
        mov ecx, offset g_Data_004da000 + 0x19ca8
        jmp MainApp_Dtor
    }
}

// 0x0042de60 MainApp_Dtor - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainApp_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MainApp_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x8], esi
        mov edi, offset g_RData_004cc000 + 0xd50
        mov dword ptr [esi + 0x220], edi
        mov dword ptr [esi + 0x208], edi
        lea ecx, [esi + 0x1e0]
        mov dword ptr [esp + 0x14], 0x5
        call Seq_Dtor
        mov dword ptr [esi + 0x1d8], edi
        mov dword ptr [esi + 0x1d0], edi
        mov dword ptr [esp + 0x14], 0xffffffff
        mov dword ptr [esi + 0x1c8], edi
        lea ecx, [esi + 0x1a8]
        mov byte ptr [esp + 0x14], 0x6
        call Seq_Dtor
        mov dword ptr [esi + 0x1a0], edi
        lea ecx, [esi + 0x170]
        mov byte ptr [esp + 0x14], 0x7
        call Seq_Dtor
        mov ecx, esi
        mov dword ptr [esi + 0x160], edi
        mov dword ptr [esp + 0x14], 0xffffffff
        call RecoilApp_Dtor
        mov ecx, dword ptr [esp + 0xc]
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x0042df10 SeqScreen_Dtor_A - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SeqScreen_Dtor_A(int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_SeqScreen_Dtor_A
        push eax
        mov dword ptr FS:[0x0], esp
        push esi
        mov esi, ecx
        lea ecx, [esi + 0x10]
        mov dword ptr [esp + 0xc], 0x0
        call Seq_Dtor
        mov ecx, dword ptr [esp + 0x4]
        mov dword ptr [esi], offset g_RData_004cc000 + 0xd50
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0xc
        ret
    }
}

// 0x0042df50 SeqScreen_Dtor_B - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SeqScreen_Dtor_B(int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_SeqScreen_Dtor_B
        push eax
        mov dword ptr FS:[0x0], esp
        push esi
        mov esi, ecx
        lea ecx, [esi + 0x8]
        mov dword ptr [esp + 0xc], 0x0
        call Seq_Dtor
        mov ecx, dword ptr [esp + 0x4]
        mov dword ptr [esi], offset g_RData_004cc000 + 0xd50
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0xc
        ret
    }
}

// 0x0042dfa0 MainApp_Ctor - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainApp_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MainApp_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x8
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x8], esi
        call RecoilApp_Constructor
        lea ecx, [esi + 0x160]
        mov dword ptr [esp + 0x18], 0x0
        call SeqScreenA_Ctor
        lea edi, [esi + 0x1a0]
        mov dword ptr [esp + 0xc], edi
        mov dword ptr [edi], offset g_RData_004cc000 + 0xd50
        push 0x0
        push 0x0
        push 0x0
        lea ecx, [edi + 0x8]
        mov byte ptr [esp + 0x24], 0x2
        call Seq_Ctor
        mov dword ptr [edi], offset g_RData_004cc000 + 0x4b18
        mov dword ptr [esi + 0x1c8], offset g_RData_004cc000 + 0x4af0
        mov dword ptr [esi + 0x1d0], offset g_RData_004cc000 + 0x4ac8
        lea ecx, [esi + 0x1d8]
        mov byte ptr [esp + 0x18], 0x5
        call SeqScreenB_Ctor
        lea ecx, [esi + 0x208]
        mov byte ptr [esp + 0x18], 0x6
        call FrameState_Ctor_004d0b90
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x220], offset g_RData_004cc000 + 0x4aa0
        mov dword ptr [esi], offset g_RData_004cc000 + 0x49e0
        mov dword ptr [esi + 0x150], 0x0
        mov eax, esi
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x14
        ret
    }
}

// 0x0042e070 SeqScreen_Dtor_C - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SeqScreen_Dtor_C(int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_SeqScreen_Dtor_C
        push eax
        mov dword ptr FS:[0x0], esp
        push esi
        mov esi, ecx
        lea ecx, [esi + 0x8]
        mov dword ptr [esp + 0xc], 0x0
        call Seq_Dtor
        mov ecx, dword ptr [esp + 0x4]
        mov dword ptr [esi], offset g_RData_004cc000 + 0xd50
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0xc
        ret
    }
}

// 0x0042e0b0 Screen_0042de60_ScalarDeletingDtor - dtor 0x0042de60; flag&1 -> delete; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_0042de60_ScalarDeletingDtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call MainApp_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_42e0c8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_42e0c8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0042e0d0 Screen_0042df50_ScalarDeletingDtor - dtor 0x0042df50; flag&1 -> delete; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_0042df50_ScalarDeletingDtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call SeqScreen_Dtor_B
        test byte ptr [esp + 0x8], 0x1
        jz L_42e0e8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_42e0e8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0042e930 MainApp_ExitInstance_0042e930 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainApp_ExitInstance_0042e930(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x19ed0]
        push esi
        test eax, eax
        mov esi, ecx
        jz L_42e974
        call dword ptr [g_Iat_MFC42_1168_004cc19c]
        mov eax, dword ptr [eax + 0x8]
        push eax
        mov eax, dword ptr [g_Data_004da000 + 0x2ac0]
        push eax
        call dword ptr [g_Iat_UnregisterClassA_004cc638]
        call Settings_ResetNetworkState
        call Stub_Ret
        call Settings_Shutdown
        call Archive_Shutdown
        call Container_ShutdownNodePool
        call SaveSystem_Destroy
        call App_FreeLoadedLibrary
    L_42e974:
        call Input_DestroyBindings
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_2725_004cc1a0]
        xor ecx, ecx
        call App_ExitProcess
        xor eax, eax
        pop esi
        ret
    }
}

// 0x0042ed30 SeqScreenB_Ctor - vtbl base; Seq ctor 0x004625e0(0,0,0); vtbl 0x004d0b68; zero [1] and [10]
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SeqScreenB_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_SeqScreenB_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0xd50
        push 0x0
        push 0x0
        push 0x0
        lea ecx, [esi + 0x8]
        mov dword ptr [esp + 0x1c], 0x0
        call Seq_Ctor
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x4b68
        mov dword ptr [esi + 0x4], 0x0
        mov dword ptr [esi + 0x28], 0x0
        mov eax, esi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x0042ed90 Screen_0042e070_ScalarDeletingDtor - dtor 0x0042e070; flag&1 -> delete; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_0042e070_ScalarDeletingDtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call SeqScreen_Dtor_C
        test byte ptr [esp + 0x8], 0x1
        jz L_42eda8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_42eda8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0042edb0 AppScreen_PlayMissionFmv_0042edb0 - REFINED 2026-09-29: BYTE RE-READ 2026-09-29 (Sonnet) - confirms the earlier note and makes it exact: ECX = this. When [this+4] == 0: [this+4] = 0x00417800() (ECX = 0x004f0cc0, ledger Mission_GetOutcome), else 0x004177a0(ECX = 0x004f0cc0, arg [this+4]); then 0x0042ecb0(ECX = [this+4]). Builds the 3-b
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppScreen_PlayMissionFmv_0042edb0(int, int)
{
    __asm {
        push ecx
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 0x4]
        test eax, eax
        jz L_42edc8
        push eax
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_SetOutcome
        jmp L_42edd5
    L_42edc8:
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_GetOutcome
        mov dword ptr [esi + 0x4], eax
    L_42edd5:
        mov ecx, dword ptr [esi + 0x4]
        call Mission_SetDataSearchPaths
        mov AX, word ptr [g_Data_004da000 + 0x2c74]
        mov DL, byte ptr [esi + 0x4]
        mov CL, byte ptr [g_Data_004da000 + 0x2c76]
        mov word ptr [esp + 0x4], AX
        mov eax, dword ptr [esi + 0x28]
        add DL, 0x30
        test eax, eax
        mov byte ptr [esp + 0x6], CL
        mov byte ptr [esp + 0x5], DL
        jnz L_42ee2f
        mov eax, dword ptr [g_Data_004da000 + 0x19eec]
        add esi, 0x8
        test eax, eax
        jz L_42ee12
        mov dword ptr [esi + 0x4], eax
    L_42ee12:
        lea eax, [esp + 0x4]
        mov ecx, esi
        push eax
        push offset g_Data_004da000 + 0x2bd0
        call Seq_Load
        cmp eax, -0x1
        jz L_42ee2f
        mov ecx, esi
        call Seq_BeginAtNow
    L_42ee2f:
        mov eax, 0x1
        pop esi
        pop ecx
        ret
    }
}

// 0x0042ee70 AppScreen_Tick_0042ee70 - REFINED 2026-09-29: BYTE RE-READ 2026-09-29 (Sonnet) - CORRECTS the earlier note (which had the condition as 'if ![this+0x28] && !0x004630e0'): MOV EAX,[ECX+0x28]; TEST EAX,EAX; JNZ 0x0042ee83 -> when [this+0x28] != 0 the call is made directly; otherwise ECX = this+8, CALL 0x004630e0, and JNZ 0x0042
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppScreen_Tick_0042ee70(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x28]
        test eax, eax
        jnz L_42ee83
        add ecx, 0x8
        call Seq_TickAtNow
        test eax, eax
        jnz L_42ee94
    L_42ee83:
        push 0x0
        push offset g_Data_004da000 + 0x19eb0
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call ScreenManager_QueuePush
    L_42ee94:
        xor eax, eax
        ret
    }
}

// 0x004305f0 MainWnd_scalar_deleting_dtor - bytes ret 4: MainWnd_Dtor; flag&1 -> operator delete
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MainWnd_scalar_deleting_dtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call MainWnd_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_430608
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_430608:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00430610 MainWnd_Dtor - bytes: vtbl 0x004d1140; CMenu member +0x1d0 (vtbl 0x004d1248) DestroyMenu, vtbl 0x004d1268; base frame dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MainWnd_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5140
        lea edi, [esi + 0x1d0]
        mov dword ptr [esp + 0x8], edi
        mov dword ptr [edi], offset g_RData_004cc000 + 0x5248
        mov ecx, edi
        mov dword ptr [esp + 0x14], 0x1
        call dword ptr [g_Iat_MFC42_2438_004cc2f8]
        mov ecx, esi
        mov dword ptr [edi], offset g_RData_004cc000 + 0x5268
        mov dword ptr [esp + 0x14], 0xffffffff
        call GameFrame_Dtor
        mov ecx, dword ptr [esp + 0xc]
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x00430c30 MainWnd_OnAbout - bytes: stack CDialog via Dialog67_Ctor(0) (dialog 0x67), DoModal, ~CDialog
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnAbout(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MainWnd_OnAbout
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x60
        push 0x0
        lea ecx, [esp + 0x4]
        call Dialog67_Ctor
        lea ecx, [esp]
        mov dword ptr [esp + 0x68], 0x0
        call dword ptr [g_Iat_MFC42_2514_004cc3f8]
        lea ecx, [esp]
        mov dword ptr [esp + 0x68], 0xffffffff
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        mov ecx, dword ptr [esp + 0x60]
        mov dword ptr FS:[0x0], ecx
        add esp, 0x6c
        ret
    }
}

// 0x00442180 RecoilApp_SetString4b8 - CString [this+0x4b8] = arg CString (0x004c5e40), destroy arg (0x004c5b88); ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall RecoilApp_SetString4b8(int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_RecoilApp_SetString4b8
        push eax
        mov dword ptr FS:[0x0], esp
        lea eax, [esp + 0x10]
        add ecx, 0x4b8
        push eax
        mov dword ptr [esp + 0xc], 0x0
        call dword ptr [g_Iat_MFC42_858_004cc23c]
        lea ecx, [esp + 0x10]
        mov dword ptr [esp + 0x8], 0xffffffff
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        mov ecx, dword ptr [esp]
        mov dword ptr FS:[0x0], ecx
        add esp, 0xc
        ret 0x4
    }
}

// 0x004421d0 RecoilApp_SetString4bc - CString [this+0x4bc] = arg CString; destroy arg; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall RecoilApp_SetString4bc(int, int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_RecoilApp_SetString4bc
        push eax
        mov dword ptr FS:[0x0], esp
        lea eax, [esp + 0x10]
        add ecx, 0x4bc
        push eax
        mov dword ptr [esp + 0xc], 0x0
        call dword ptr [g_Iat_MFC42_858_004cc23c]
        lea ecx, [esp + 0x10]
        mov dword ptr [esp + 0x8], 0xffffffff
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        mov ecx, dword ptr [esp]
        mov dword ptr FS:[0x0], ecx
        add esp, 0xc
        ret 0x4
    }
}

// 0x004422f0 StatusDialog_Refresh - 0x0042dcf0([0x005392a0], 0x004d1868, [0x005392ac]); vfunc+8 on [0x005392a0]
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StatusDialog_Refresh(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x5f2ac]
        mov ecx, dword ptr [g_Data_004da000 + 0x5f2a0]
        push eax
        push offset g_RData_004cc000 + 0x5868
        push ecx
        call Com_QueryAndCallSlot18
        mov eax, dword ptr [g_Data_004da000 + 0x5f2a0]
        push eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x8]
        ret
    }
}

// 0x00442790 RefCounted_Release - InterlockedDecrement(obj+4); 0 -> dtor 0x004427f0 + delete; return count
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall RefCounted_Release(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        lea eax, [esi + 0x4]
        push eax
        call dword ptr [g_Iat_InterlockedDecrement_004cc0cc]
        mov edi, eax
        test edi, edi
        jnz L_4427ba
        test esi, esi
        jz L_4427ba
        mov ecx, esi
        call RefCounted_Dtor
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_4427ba:
        mov eax, edi
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x004427f0 RefCounted_Dtor - vtbl 0x004d1fe0; [1]=1; InterlockedDecrement(global count 0x004f53e4); DeleteCriticalSection(this+8)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RefCounted_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_RefCounted_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x8], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5fe0
        lea edi, [esi + 0x4]
        push offset g_Data_004da000 + 0x1b3e4
        mov dword ptr [esp + 0x18], 0x0
        mov dword ptr [edi], 0x1
        call dword ptr [g_Iat_InterlockedDecrement_004cc0cc]
        neg esi
        sbb esi, esi
        mov dword ptr [esp + 0x14], 0xffffffff
        and esi, edi
        add esi, 0x4
        push esi
        call dword ptr [g_Iat_DeleteCriticalSection_004cc0f0]
        mov ecx, dword ptr [esp + 0xc]
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x00442bc0 Engine_ShutdownSubsystems - calls in order 0x00471c10, 0x0046eb90, 0x004a6100, 0x0048ff60, 0x00460060, 0x004b1180, 0x004518e0, 0x00475e60, 0x004a13d0, 0x0048cd10, tail jmp 0x0048c890
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Engine_ShutdownSubsystems(int, int)
{
    __asm {
        call zInShutdown
        call TextureManager_Shutdown
        call zUtlShutdown
        call zRndrShutdown
        call EffectList_ResetAndFree
        call Weapon_Shutdown_Thunk
        call gClsShutdown
        call gModShutdown
        call zSnd_Shutdown
        call Archive_Shutdown
        jmp Container_ShutdownNodePool
    }
}

// 0x00443810 GameFrame_scalar_deleting_dtor - bytes ret 4: GameFrame_Dtor; flag&1 -> operator delete
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall GameFrame_scalar_deleting_dtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call GameFrame_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_443828
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_443828:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00443830 GameFrame_Dtor - bytes: vtbl 0x004d21d8; EmptyStub_004a75e0; brush +0xc4 vtbl 0x004d22f8 DeleteObject, vtbl 0x004d1268; ~CFrameWnd
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GameFrame_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_GameFrame_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x61d8
        call EmptyStub_004a75e0
        lea edi, [esi + 0xc4]
        mov dword ptr [esp + 0x8], edi
        mov dword ptr [edi], offset g_RData_004cc000 + 0x62f8
        mov ecx, edi
        mov dword ptr [esp + 0x14], 0x1
        call dword ptr [g_Iat_MFC42_2414_004cc274]
        mov ecx, esi
        mov dword ptr [edi], offset g_RData_004cc000 + 0x5268
        mov dword ptr [esp + 0x14], 0xffffffff
        call dword ptr [g_Iat_MFC42_674_004cc270]
        mov ecx, dword ptr [esp + 0xc]
        pop edi
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x00443900 GameFrame_OnPaint - bytes: CPaintDC; if !0x004a59b0(): memory DC SelectObject bitmap; BitBlt or StretchBlt to 640x480 (0x280x0x1e0) SRCCOPY 0xcc0020; DeleteDC; ~CPaintDC
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GameFrame_OnPaint(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_GameFrame_OnPaint
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x94
        push ebx
        mov ebx, ecx
        push ebx
        lea ecx, [esp + 0x48]
        call dword ptr [g_Iat_MFC42_470_004cc280]
        mov dword ptr [esp + 0xa0], 0x0
        call Video_IsWindowedNonGlide
        test eax, eax
        jnz L_4439f6
        push edi
        push esi
        mov ecx, 0x10
        lea esi, [esp + 0x60]
        lea edi, [esp + 0xc]
        lea eax, [esp + 0x4c]
        rep movsd
        mov esi, dword ptr [esp + 0x50]
        neg eax
        sbb eax, eax
        and eax, esi
        push eax
        call dword ptr [g_Iat_CreateCompatibleDC_004cc0a4]
        mov esi, eax
        lea eax, [ebx + 0xc4]
        test eax, eax
        jz L_443975
        mov eax, dword ptr [eax + 0x4]
    L_443975:
        push eax
        push esi
        call dword ptr [g_Iat_SelectObject_004cc06c]
        mov edx, dword ptr [esp + 0x20]
        mov ecx, dword ptr [esp + 0x18]
        sub edx, ecx
        lea eax, [esp + 0x4c]
        cmp edx, 0x1e0
        jle L_4439c6
        mov edi, dword ptr [esp + 0x50]
        push 0xcc0020
        neg eax
        sbb eax, eax
        push 0x1e0
        and eax, edi
        mov edi, dword ptr [esp + 0x1c]
        push 0x280
        push ecx
        push edi
        push esi
        push edx
        mov edx, dword ptr [esp + 0x38]
        sub edx, edi
        push edx
        push ecx
        push edi
        push eax
        call dword ptr [g_Iat_StretchBlt_004cc0a8]
        jmp L_4439ed
    L_4439c6:
        mov edi, dword ptr [esp + 0x50]
        push 0xcc0020
        neg eax
        sbb eax, eax
        push ecx
        and eax, edi
        mov edi, dword ptr [esp + 0x1c]
        push edi
        push esi
        push edx
        mov edx, dword ptr [esp + 0x30]
        sub edx, edi
        push edx
        push ecx
        push edi
        push eax
        call dword ptr [g_Iat_BitBlt_004cc064]
    L_4439ed:
        push esi
        call dword ptr [g_Iat_DeleteDC_004cc08c]
        pop esi
        pop edi
    L_4439f6:
        lea ecx, [esp + 0x44]
        mov dword ptr [esp + 0xa0], 0xffffffff
        call dword ptr [g_Iat_MFC42_755_004cc27c]
        mov ecx, dword ptr [esp + 0x98]
        pop ebx
        mov dword ptr FS:[0x0], ecx
        add esp, 0xa0
        ret
    }
}

// 0x00443b70 GdiObjA_scalar_deleting_dtor - bytes ret 4: 0x00443b90; flag&1 -> operator delete
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall GdiObjA_scalar_deleting_dtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call GdiObjA_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_443b88
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_443b88:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00443b90 GdiObjA_Dtor - bytes: vtbl 0x004d22f8; CGdiObject::DeleteObject; vtbl 0x004d1268 (CGdiObject base)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GdiObjA_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_GdiObjA_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x62f8
        mov dword ptr [esp + 0x10], 0x0
        call dword ptr [g_Iat_MFC42_2414_004cc274]
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5268
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x00443be0 GdiObjB_scalar_deleting_dtor - bytes ret 4: 0x00443c00; flag&1 -> operator delete
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall GdiObjB_scalar_deleting_dtor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call GdiObjB_Dtor
        test byte ptr [esp + 0x8], 0x1
        jz L_443bf8
        push esi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
    L_443bf8:
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x00443c00 GdiObjB_Dtor - bytes: vtbl 0x004d22f8; DeleteObject; vtbl 0x004d1268; separate SEH frame 0x004cad68 (duplicate instantiation of 0x00443b90)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GdiObjB_Dtor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_GdiObjB_Dtor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push esi
        mov esi, ecx
        mov dword ptr [esp + 0x4], esi
        mov dword ptr [esi], offset g_RData_004cc000 + 0x62f8
        mov dword ptr [esp + 0x10], 0x0
        call dword ptr [g_Iat_MFC42_2414_004cc274]
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5268
        mov dword ptr FS:[0x0], ecx
        pop esi
        add esp, 0x10
        ret
    }
}

// 0x004c9ce0 EH_Unwind_Com_QueryAndCallSlot14_0 - (obj,a,b,c) ret 0x10: obj->QueryInterface(iid 0x004d43a0); on success other->vfunc+0x10(..,&itf); itf->vfunc+0x14(..,obj); release itf and other; return HRESULT
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Com_QueryAndCallSlot14_0(int, int)
{
    __asm {
        lea ecx, [ebp - 0x10]
        jmp ComPtr_Release
    }
}

// 0x004c9ce8 EH_Unwind_Com_QueryAndCallSlot14_1 - (obj,a,b,c) ret 0x10: obj->QueryInterface(iid 0x004d43a0); on success other->vfunc+0x10(..,&itf); itf->vfunc+0x14(..,obj); release itf and other; return HRESULT
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Com_QueryAndCallSlot14_1(int, int)
{
    __asm {
        lea ecx, [ebp - 0x14]
        jmp ComPtr_Release
    }
}

// 0x004c9d00 EH_Unwind_Com_QueryAndCallSlot18_0 - (obj,a,b) ret 0xc: as 0x0042dc30 but final call itf->vfunc+0x18(arg); release; return HRESULT
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Com_QueryAndCallSlot18_0(int, int)
{
    __asm {
        lea ecx, [ebp - 0x10]
        jmp ComPtr_Release
    }
}

// 0x004c9d08 EH_Unwind_Com_QueryAndCallSlot18_1 - (obj,a,b) ret 0xc: as 0x0042dc30 but final call itf->vfunc+0x18(arg); release; return HRESULT
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Com_QueryAndCallSlot18_1(int, int)
{
    __asm {
        lea ecx, [ebp - 0x14]
        jmp ComPtr_Release
    }
}

// 0x004c9d20 EH_Unwind_MainApp_Dtor_0 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp RecoilApp_Dtor
    }
}

// 0x004c9d28 EH_Unwind_MainApp_Dtor_1 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x160
        jmp SeqScreen_Dtor_A
    }
}

// 0x004c9d36 EH_Unwind_MainApp_Dtor_2 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x1a0
        jmp SeqScreen_Dtor_B
    }
}

// 0x004c9d44 EH_Unwind_MainApp_Dtor_3 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_3(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x1c8
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9d52 EH_Unwind_MainApp_Dtor_4 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_4(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x1d0
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9d60 EH_Unwind_MainApp_Dtor_5 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_5(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0xc]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9d68 EH_Unwind_MainApp_Dtor_6 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_6(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9d70 EH_Unwind_MainApp_Dtor_7 - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Dtor_7(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x1c]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9d90 EH_Unwind_SeqScreen_Dtor_A_0 - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_SeqScreen_Dtor_A_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9db0 EH_Unwind_SeqScreen_Dtor_B_0 - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_SeqScreen_Dtor_B_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9dd0 EH_Unwind_MainApp_Ctor_0 - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        jmp RecoilApp_Dtor
    }
}

// 0x004c9dd8 EH_Unwind_MainApp_Ctor_1 - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        add ecx, 0x160
        jmp SeqScreen_Dtor_A
    }
}

// 0x004c9de6 EH_Unwind_MainApp_Ctor_2 - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Ctor_2(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9dee EH_Unwind_MainApp_Ctor_3 - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Ctor_3(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        add ecx, 0x1a0
        jmp SeqScreen_Dtor_B
    }
}

// 0x004c9dfc EH_Unwind_MainApp_Ctor_4 - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Ctor_4(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        add ecx, 0x1c8
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9e0a EH_Unwind_MainApp_Ctor_5 - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Ctor_5(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        add ecx, 0x1d0
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9e18 EH_Unwind_MainApp_Ctor_6 - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainApp_Ctor_6(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        add ecx, 0x1d8
        jmp SeqScreen_Dtor_C
    }
}

// 0x004c9e30 EH_Unwind_SeqScreen_Dtor_C_0 - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_SeqScreen_Dtor_C_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9e50 EH_Unwind_App_CreateMainWindow_0 - new(0x230) main window 0x00430250; return it or 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_App_CreateMainWindow_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x10]
        push eax
        call OperatorDelete_Thunk_0042f890
        ret
    }
}

// 0x004c9e90 EH_Unwind_SeqScreenB_Ctor_0 - vtbl base; Seq ctor 0x004625e0(0,0,0); vtbl 0x004d0b68; zero [1] and [10]
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_SeqScreenB_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp ScreenBase_ResetVtbl
    }
}

// 0x004c9eb0 EH_Unwind_Game_EndSequenceTick_0 - bytes: timer 0x004f3df8: <=0 and won 0x004e5dec -> Sound stop, play FMV 'fmv_zrd'/'GRANDPRIZE' (0x004625e0), 0x00462f50(0), renderer hook, 0x00463850(0xc,1), 0x00462630; else 0x004dd1c0 flag; fade: timer -= clock 0x0056b424 with Screen_SetFadeFill/zVideo present while <=1.0; player +0x5a0 set during
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Game_EndSequenceTick_0(int, int)
{
    __asm {
        lea ecx, [ebp - 0x5c]
        jmp Seq_Dtor
    }
}

// 0x004c9eb8 EH_Unwind_Game_EndSequenceTick_1 - bytes: timer 0x004f3df8: <=0 and won 0x004e5dec -> Sound stop, play FMV 'fmv_zrd'/'GRANDPRIZE' (0x004625e0), 0x00462f50(0), renderer hook, 0x00463850(0xc,1), 0x00462630; else 0x004dd1c0 flag; fade: timer -= clock 0x0056b424 with Screen_SetFadeFill/zVideo present while <=1.0; player +0x5a0 set during
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Game_EndSequenceTick_1(int, int)
{
    __asm {
        lea ecx, [ebp - 0x3c]
        jmp MapMarker_SetVtbl
    }
}

// 0x004c9ed0 EH_Unwind_App_EnterMissionOver_0 - WAS LIBRARY/EXCLUDED Unwind@004c9ed0 (EH funclet; tools/gen_eh_frames.py 2026-09-30). 2 instructions from the Ghidra listing (function created by CreateDumpFunctions.java), first bytes 8d 4d d4 e9: C++ unwind action for state(s) 0 of 0x0042f8e0 App_EnterMissionOver_0042f8e0 - FuncInfo 0x004d6040 (ma
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_App_EnterMissionOver_0(int, int)
{
    __asm {
        lea ecx, [ebp - 0x2c]
        jmp Seq_Dtor
    }
}

// 0x004c9ef0 EH_Unwind_MainWnd_New_0 - bytes: SEH; operator new(0x230); MainWnd_Ctor 0x00430250; returns object or 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_New_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x10]
        push eax
        call OperatorDelete_Thunk_0042f890
        ret
    }
}

// 0x004c9f10 EH_Unwind_MainWnd_Ctor_0 - main frame window (0x230 object): paths "recoil" / "/campaigns", error log "recoil.err", menu resource "MYMENU", registry key Software\Westwood\WOLAPI\4352, serial chars "1234567890"; MFC CFrameWnd/CString setup (0x004c5bxx..0x004c5exx), 0x004306f0, 0x004437d0, 0x00462310, 0x0046d5c0, 0x004a07f0, 0x
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_Ctor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp GameFrame_Dtor
    }
}

// 0x004c9f18 EH_Unwind_MainWnd_Ctor_1 - main frame window (0x230 object): paths "recoil" / "/campaigns", error log "recoil.err", menu resource "MYMENU", registry key Software\Westwood\WOLAPI\4352, serial chars "1234567890"; MFC CFrameWnd/CString setup (0x004c5bxx..0x004c5exx), 0x004306f0, 0x004437d0, 0x00462310, 0x0046d5c0, 0x004a07f0, 0x
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_Ctor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, 0x1d0
        jmp MenuWrap_Dtor
    }
}

// 0x004c9f26 EH_Unwind_MainWnd_Ctor_2 - main frame window (0x230 object): paths "recoil" / "/campaigns", error log "recoil.err", menu resource "MYMENU", registry key Software\Westwood\WOLAPI\4352, serial chars "1234567890"; MFC CFrameWnd/CString setup (0x004c5bxx..0x004c5exx), 0x004306f0, 0x004437d0, 0x00462310, 0x0046d5c0, 0x004a07f0, 0x
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_Ctor_2(int, int)
{
    __asm {
        lea ecx, [ebp - 0x18]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004c9f2e EH_Unwind_MainWnd_Ctor_3 - main frame window (0x230 object): paths "recoil" / "/campaigns", error log "recoil.err", menu resource "MYMENU", registry key Software\Westwood\WOLAPI\4352, serial chars "1234567890"; MFC CFrameWnd/CString setup (0x004c5bxx..0x004c5exx), 0x004306f0, 0x004437d0, 0x00462310, 0x0046d5c0, 0x004a07f0, 0x
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_Ctor_3(int, int)
{
    __asm {
        lea ecx, [ebp - 0x1c]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004c9f36 EH_Unwind_MainWnd_Ctor_4 - main frame window (0x230 object): paths "recoil" / "/campaigns", error log "recoil.err", menu resource "MYMENU", registry key Software\Westwood\WOLAPI\4352, serial chars "1234567890"; MFC CFrameWnd/CString setup (0x004c5bxx..0x004c5exx), 0x004306f0, 0x004437d0, 0x00462310, 0x0046d5c0, 0x004a07f0, 0x
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_Ctor_4(int, int)
{
    __asm {
        lea ecx, [ebp - 0x18]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004c9f50 EH_Unwind_MainWnd_Dtor_0 - bytes: vtbl 0x004d1140; CMenu member +0x1d0 (vtbl 0x004d1248) DestroyMenu, vtbl 0x004d1268; base frame dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        jmp GameFrame_Dtor
    }
}

// 0x004c9f58 EH_Unwind_MainWnd_Dtor_1 - bytes: vtbl 0x004d1140; CMenu member +0x1d0 (vtbl 0x004d1248) DestroyMenu, vtbl 0x004d1268; base frame dtor
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_Dtor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp MenuWrap_SetBaseVtbl
    }
}

// 0x004c9f70 EH_Unwind_MainWnd_OnAbout_0 - bytes: stack CDialog via Dialog67_Ctor(0) (dialog 0x67), DoModal, ~CDialog
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_MainWnd_OnAbout_0(int, int)
{
    __asm {
        lea ecx, [ebp - 0x6c]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004c9f90 EH_Unwind_Menus_RunNetSetupThenStartMission_0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_0(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffcc4]
        jmp NameDlg_dtor
    }
}

// 0x004c9f9b EH_Unwind_Menus_RunNetSetupThenStartMission_1 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_1(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffe78]
        jmp NetSetupDialog_Dtor
    }
}

// 0x004c9fa6 EH_Unwind_Menus_RunNetSetupThenStartMission_2 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_2(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffe78]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004c9fb1 EH_Unwind_Menus_RunNetSetupThenStartMission_3 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_3(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffee0]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004c9fbc EH_Unwind_Menus_RunNetSetupThenStartMission_4 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_4(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff20]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004c9fc7 EH_Unwind_Menus_RunNetSetupThenStartMission_5 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_5(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff60]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004c9fd2 EH_Unwind_Menus_RunNetSetupThenStartMission_6 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_6(int, int)
{
    __asm {
        lea ecx, [ebp - 0x60]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004c9fda EH_Unwind_Menus_RunNetSetupThenStartMission_7 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_7(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffcc4]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004c9fe5 EH_Unwind_Menus_RunNetSetupThenStartMission_8 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_8(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd34]
        jmp dword ptr [g_Iat_MFC42_656_004cc33c]
    }
}

// 0x004c9ff0 EH_Unwind_Menus_RunNetSetupThenStartMission_9 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_9(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd74]
        jmp dword ptr [g_Iat_MFC42_609_004cc334]
    }
}

// 0x004c9ffb EH_Unwind_Menus_RunNetSetupThenStartMission_10 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_10(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffdb4]
        jmp dword ptr [g_Iat_MFC42_609_004cc334]
    }
}

// 0x004ca006 EH_Unwind_Menus_RunNetSetupThenStartMission_11 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_11(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffdf4]
        jmp dword ptr [g_Iat_MFC42_692_004cc344]
    }
}

// 0x004ca011 EH_Unwind_Menus_RunNetSetupThenStartMission_12 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_12(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffe34]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004ca01c EH_Unwind_Menus_RunNetSetupThenStartMission_13 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_13(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffe78]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004ca027 EH_Unwind_Menus_RunNetSetupThenStartMission_14 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_14(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffee0]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004ca032 EH_Unwind_Menus_RunNetSetupThenStartMission_15 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_15(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff20]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004ca03d EH_Unwind_Menus_RunNetSetupThenStartMission_16 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_16(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff60]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004ca048 EH_Unwind_Menus_RunNetSetupThenStartMission_17 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_17(int, int)
{
    __asm {
        lea ecx, [ebp - 0x60]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004ca050 EH_Unwind_Menus_RunNetSetupThenStartMission_18 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_18(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffcc4]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004ca05b EH_Unwind_Menus_RunNetSetupThenStartMission_19 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_19(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd34]
        jmp dword ptr [g_Iat_MFC42_656_004cc33c]
    }
}

// 0x004ca066 EH_Unwind_Menus_RunNetSetupThenStartMission_20 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_20(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd74]
        jmp dword ptr [g_Iat_MFC42_609_004cc334]
    }
}

// 0x004ca071 EH_Unwind_Menus_RunNetSetupThenStartMission_21 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_21(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffdb4]
        jmp dword ptr [g_Iat_MFC42_609_004cc334]
    }
}

// 0x004ca07c EH_Unwind_Menus_RunNetSetupThenStartMission_22 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_22(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffdf4]
        jmp dword ptr [g_Iat_MFC42_692_004cc344]
    }
}

// 0x004ca087 EH_Unwind_Menus_RunNetSetupThenStartMission_23 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_23(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffe34]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004ca092 EH_Unwind_Menus_RunNetSetupThenStartMission_24 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_24(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffe78]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004ca09d EH_Unwind_Menus_RunNetSetupThenStartMission_25 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_25(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffee0]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004ca0a8 EH_Unwind_Menus_RunNetSetupThenStartMission_26 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_26(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff20]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004ca0b3 EH_Unwind_Menus_RunNetSetupThenStartMission_27 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_27(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff60]
        jmp dword ptr [g_Iat_MFC42_793_004cc2f0]
    }
}

// 0x004ca0be EH_Unwind_Menus_RunNetSetupThenStartMission_28 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_28(int, int)
{
    __asm {
        lea ecx, [ebp - 0x60]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004ca0c6 EH_Unwind_Menus_RunNetSetupThenStartMission_29 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_29(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffcc4]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004ca0d1 EH_Unwind_Menus_RunNetSetupThenStartMission_30 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_30(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd34]
        jmp dword ptr [g_Iat_MFC42_656_004cc33c]
    }
}

// 0x004ca0dc EH_Unwind_Menus_RunNetSetupThenStartMission_31 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_31(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffd74]
        jmp dword ptr [g_Iat_MFC42_609_004cc334]
    }
}

// 0x004ca0e7 EH_Unwind_Menus_RunNetSetupThenStartMission_32 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_32(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffdb4]
        jmp dword ptr [g_Iat_MFC42_609_004cc334]
    }
}

// 0x004ca0f2 EH_Unwind_Menus_RunNetSetupThenStartMission_33 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_33(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffdf4]
        jmp dword ptr [g_Iat_MFC42_692_004cc344]
    }
}

// 0x004ca0fd EH_Unwind_Menus_RunNetSetupThenStartMission_34 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_Menus_RunNetSetupThenStartMission_34(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffe34]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004cab6b EH_Unwind_ComboDialog_RunModal_1 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_1(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffed8]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004cab76 EH_Unwind_ComboDialog_RunModal_2 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_2(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff38]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004cab81 EH_Unwind_ComboDialog_RunModal_3 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_3(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff78]
        jmp dword ptr [g_Iat_MFC42_656_004cc33c]
    }
}

// 0x004cab8c EH_Unwind_ComboDialog_RunModal_4 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_4(int, int)
{
    __asm {
        lea ecx, [ebp - 0x48]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004cab94 EH_Unwind_ComboDialog_RunModal_5 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_5(int, int)
{
    __asm {
        lea ecx, [ebp - 0x44]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004cab9c EH_Unwind_ComboDialog_RunModal_6 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_6(int, int)
{
    __asm {
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        lea eax, [ebp - 0x3c]
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cabaf EH_Unwind_ComboDialog_RunModal_7 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_7(int, int)
{
    __asm {
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        lea eax, [ebp - 0x34]
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cabc2 EH_Unwind_ComboDialog_RunModal_8 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_8(int, int)
{
    __asm {
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        lea eax, [ebp - 0x2c]
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cabd5 EH_Unwind_ComboDialog_RunModal_9 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_9(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffed8]
        jmp dword ptr [g_Iat_MFC42_641_004cc2b4]
    }
}

// 0x004cabe0 EH_Unwind_ComboDialog_RunModal_10 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_10(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff38]
        jmp dword ptr [g_Iat_MFC42_616_004cc348]
    }
}

// 0x004cabeb EH_Unwind_ComboDialog_RunModal_11 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_11(int, int)
{
    __asm {
        lea ecx, [ebp + 0xffffff78]
        jmp dword ptr [g_Iat_MFC42_656_004cc33c]
    }
}

// 0x004cabf6 EH_Unwind_ComboDialog_RunModal_12 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_12(int, int)
{
    __asm {
        lea ecx, [ebp - 0x48]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004cabfe EH_Unwind_ComboDialog_RunModal_13 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_13(int, int)
{
    __asm {
        lea ecx, [ebp - 0x44]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004cac06 EH_Unwind_ComboDialog_RunModal_14 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_14(int, int)
{
    __asm {
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        lea eax, [ebp - 0x3c]
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cac19 EH_Unwind_ComboDialog_RunModal_15 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_15(int, int)
{
    __asm {
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        lea eax, [ebp - 0x34]
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cac2c EH_Unwind_ComboDialog_RunModal_16 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_16(int, int)
{
    __asm {
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        lea eax, [ebp - 0x2c]
        push eax
        call ArrayDtor_Eh2
        ret
    }
}

// 0x004cac50 EH_Unwind_RecoilApp_SetString4b8_0 - CString [this+0x4b8] = arg CString (0x004c5e40), destroy arg (0x004c5b88); ret 4
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_RecoilApp_SetString4b8_0(int, int)
{
    __asm {
        lea ecx, [ebp + 0x4]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004cac70 EH_Unwind_RecoilApp_SetString4bc_0 - CString [this+0x4bc] = arg CString; destroy arg; ret 4
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_RecoilApp_SetString4bc_0(int, int)
{
    __asm {
        lea ecx, [ebp + 0x4]
        jmp dword ptr [g_Iat_MFC42_800_004cc2a0]
    }
}

// 0x004cacb0 EH_Unwind_RefCounted_Dtor_0 - vtbl 0x004d1fe0; [1]=1; InterlockedDecrement(global count 0x004f53e4); DeleteCriticalSection(this+8)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_RefCounted_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp CritSec_Delete
    }
}

// 0x004cace0 EH_Unwind_GameFrame_New_0 - bytes: SEH; operator new(0xcc); GameFrame_Ctor(class name 'gamez' 0x004dd8e8)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_GameFrame_New_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x10]
        push eax
        call OperatorDelete_Thunk_0042f890
        ret
    }
}

// 0x004cad00 EH_Unwind_GameFrame_Dtor_0 - bytes: vtbl 0x004d21d8; EmptyStub_004a75e0; brush +0xc4 vtbl 0x004d22f8 DeleteObject, vtbl 0x004d1268; ~CFrameWnd
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_GameFrame_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x14]
        jmp dword ptr [g_Iat_MFC42_674_004cc270]
    }
}

// 0x004cad08 EH_Unwind_GameFrame_Dtor_1 - bytes: vtbl 0x004d21d8; EmptyStub_004a75e0; brush +0xc4 vtbl 0x004d22f8 DeleteObject, vtbl 0x004d1268; ~CFrameWnd
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_GameFrame_Dtor_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp MenuWrap_SetBaseVtbl
    }
}

// 0x004cad20 EH_Unwind_GameFrame_OnPaint_0 - bytes: CPaintDC; if !0x004a59b0(): memory DC SelectObject bitmap; BitBlt or StretchBlt to 640x480 (0x280x0x1e0) SRCCOPY 0xcc0020; DeleteDC; ~CPaintDC
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_GameFrame_OnPaint_0(int, int)
{
    __asm {
        lea ecx, [ebp - 0x60]
        jmp dword ptr [g_Iat_MFC42_755_004cc27c]
    }
}

// 0x004cad40 EH_Unwind_GdiObjA_Dtor_0 - bytes: vtbl 0x004d22f8; CGdiObject::DeleteObject; vtbl 0x004d1268 (CGdiObject base)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_GdiObjA_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp MenuWrap_SetBaseVtbl
    }
}

// 0x004cad60 EH_Unwind_GdiObjB_Dtor_0 - bytes: vtbl 0x004d22f8; DeleteObject; vtbl 0x004d1268; separate SEH frame 0x004cad68 (duplicate instantiation of 0x00443b90)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_GdiObjB_Dtor_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x10]
        jmp MenuWrap_SetBaseVtbl
    }
}

// 0x004c9cf0 EH_Handler_Com_QueryAndCallSlot14 - (obj,a,b,c) ret 0x10: obj->QueryInterface(iid 0x004d43a0); on success other->vfunc+0x10(..,&itf); itf->vfunc+0x14(..,obj); release itf and other; return HRESULT
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 08 5e 4d 00 e9 a6 c3 ff ff): MOV EAX,FuncInfo 0x004d5e08; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d5e28 (to-state, action) = ['(-1, 0x004c9ce0)', '(0, 0x004c9ce8)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5e28[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Com_QueryAndCallSlot14_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_Com_QueryAndCallSlot14_1)}};
const EhFuncInfo g_EhFuncInfo_004d5e08 = {0x19930520u, 2, g_EhUnwindMap_004d5e28, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Com_QueryAndCallSlot14(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5e08
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9d10 EH_Handler_Com_QueryAndCallSlot18 - (obj,a,b) ret 0xc: as 0x0042dc30 but final call itf->vfunc+0x18(arg); release; return HRESULT
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 38 5e 4d 00 e9 86 c3 ff ff): MOV EAX,FuncInfo 0x004d5e38; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d5e58 (to-state, action) = ['(-1, 0x004c9d00)', '(0, 0x004c9d08)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5e58[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Com_QueryAndCallSlot18_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_Com_QueryAndCallSlot18_1)}};
const EhFuncInfo g_EhFuncInfo_004d5e38 = {0x19930520u, 2, g_EhUnwindMap_004d5e58, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Com_QueryAndCallSlot18(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5e38
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9d78 EH_Handler_MainApp_Dtor - reset member vtbls (+0x220,+0x208,+0x1d8,+0x1d0,+0x1c8,+0x1a0,+0x160 -> 0x004ccd50); 3 Seq member dtors (0x00462630); RecoilApp_Dtor
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 68 5e 4d 00 e9 1e c3 ff ff): MOV EAX,FuncInfo 0x004d5e68; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 8 unwind states, unwind map 0x004d5e88 (to-state, action) = ['(-1, 0x004c9d20)', '(0, 0x004c9d28)', '(1, 0x004c9d36)', '(2, 0x004c9d44)', '(3, 0x004c9d52)', '(-1, 0x004c9d60)', '(1, 0x004c9d68)', '(0, 0x004c9d70)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5e88[8] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_4)}, {-1, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_5)}, {1, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_6)}, {0, reinterpret_cast<void*>(&EH_Unwind_MainApp_Dtor_7)}};
const EhFuncInfo g_EhFuncInfo_004d5e68 = {0x19930520u, 8, g_EhUnwindMap_004d5e88, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MainApp_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5e68
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9d98 EH_Handler_SeqScreen_Dtor_A - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 c8 5e 4d 00 e9 fe c2 ff ff): MOV EAX,FuncInfo 0x004d5ec8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d5ee8 (to-state, action) = ['(-1, 0x004c9d90)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5ee8[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_SeqScreen_Dtor_A_0)}};
const EhFuncInfo g_EhFuncInfo_004d5ec8 = {0x19930520u, 1, g_EhUnwindMap_004d5ee8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_SeqScreen_Dtor_A(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5ec8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9db8 EH_Handler_SeqScreen_Dtor_B - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 f0 5e 4d 00 e9 de c2 ff ff): MOV EAX,FuncInfo 0x004d5ef0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d5f10 (to-state, action) = ['(-1, 0x004c9db0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5f10[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_SeqScreen_Dtor_B_0)}};
const EhFuncInfo g_EhFuncInfo_004d5ef0 = {0x19930520u, 1, g_EhUnwindMap_004d5f10, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_SeqScreen_Dtor_B(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5ef0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9e26 EH_Handler_MainApp_Ctor - RecoilApp_Constructor; SeqScreenA_Ctor member; Seq member at [0x68] (ctor 0x004625e0, vtbls 0x004d0b18/0x004d0af0/0x004d0ac8 at [0x68]/[0x72]/[0x74]); SeqScreenB_Ctor; FrameState_Ctor; [0x88] vtbl 0x004d0aa0; this vtbl 0x004d09e0; [0x54]=0
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 18 5f 4d 00 e9 70 c2 ff ff): MOV EAX,FuncInfo 0x004d5f18; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 7 unwind states, unwind map 0x004d5f38 (to-state, action) = ['(-1, 0x004c9dd0)', '(0, 0x004c9dd8)', '(1, 0x004c9de6)', '(1, 0x004c9dee)', '(3, 0x004c9dfc)', '(4, 0x004c9e0a)', '(5, 0x004c9e18)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5f38[7] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MainApp_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_MainApp_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_MainApp_Ctor_2)}, {1, reinterpret_cast<void*>(&EH_Unwind_MainApp_Ctor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_MainApp_Ctor_4)}, {4, reinterpret_cast<void*>(&EH_Unwind_MainApp_Ctor_5)}, {5, reinterpret_cast<void*>(&EH_Unwind_MainApp_Ctor_6)}};
const EhFuncInfo g_EhFuncInfo_004d5f18 = {0x19930520u, 7, g_EhUnwindMap_004d5f38, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MainApp_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5f18
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9e38 EH_Handler_SeqScreen_Dtor_C - Seq member dtor 0x00462630; vtbl base 0x004ccd50
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 70 5f 4d 00 e9 5e c2 ff ff): MOV EAX,FuncInfo 0x004d5f70; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d5f90 (to-state, action) = ['(-1, 0x004c9e30)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5f90[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_SeqScreen_Dtor_C_0)}};
const EhFuncInfo g_EhFuncInfo_004d5f70 = {0x19930520u, 1, g_EhUnwindMap_004d5f90, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_SeqScreen_Dtor_C(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5f70
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9e5a EH_Handler_App_CreateMainWindow - new(0x230) main window 0x00430250; return it or 0
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 98 5f 4d 00 e9 3c c2 ff ff): MOV EAX,FuncInfo 0x004d5f98; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d5fb8 (to-state, action) = ['(-1, 0x004c9e50)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d5fb8[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_App_CreateMainWindow_0)}};
const EhFuncInfo g_EhFuncInfo_004d5f98 = {0x19930520u, 1, g_EhUnwindMap_004d5fb8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_App_CreateMainWindow(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5f98
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9e98 EH_Handler_SeqScreenB_Ctor - vtbl base; Seq ctor 0x004625e0(0,0,0); vtbl 0x004d0b68; zero [1] and [10]
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 e8 5f 4d 00 e9 fe c1 ff ff): MOV EAX,FuncInfo 0x004d5fe8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6008 (to-state, action) = ['(-1, 0x004c9e90)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6008[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_SeqScreenB_Ctor_0)}};
const EhFuncInfo g_EhFuncInfo_004d5fe8 = {0x19930520u, 1, g_EhUnwindMap_004d6008, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_SeqScreenB_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d5fe8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9ec0 EH_Handler_Game_EndSequenceTick - bytes: timer 0x004f3df8: <=0 and won 0x004e5dec -> Sound stop, play FMV 'fmv_zrd'/'GRANDPRIZE' (0x004625e0), 0x00462f50(0), renderer hook, 0x00463850(0xc,1), 0x00462630; else 0x004dd1c0 flag; fade: timer -= clock 0x0056b424 with Screen_SetFadeFill/zVideo present while <=1.0; player +0x5a0 set during, cleared + Settings_ApplyMuteSound at end; returns 0
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 10 60 4d 00 e9 d6 c1 ff ff): MOV EAX,FuncInfo 0x004d6010; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d6030 (to-state, action) = ['(-1, 0x004c9eb0)', '(0, 0x004c9eb8)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6030[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Game_EndSequenceTick_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_Game_EndSequenceTick_1)}};
const EhFuncInfo g_EhFuncInfo_004d6010 = {0x19930520u, 2, g_EhUnwindMap_004d6030, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Game_EndSequenceTick(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6010
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9ed8 EH_Handler_App_EnterMissionOver - 
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 40 60 4d 00 e9 be c1 ff ff): MOV EAX,FuncInfo 0x004d6040; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6060 (to-state, action) = ['(-1, 0x004c9ed0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6060[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_App_EnterMissionOver_0)}};
const EhFuncInfo g_EhFuncInfo_004d6040 = {0x19930520u, 1, g_EhUnwindMap_004d6060, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_App_EnterMissionOver(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6040
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9efa EH_Handler_MainWnd_New - bytes: SEH; operator new(0x230); MainWnd_Ctor 0x00430250; returns object or 0
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 68 60 4d 00 e9 9c c1 ff ff): MOV EAX,FuncInfo 0x004d6068; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6088 (to-state, action) = ['(-1, 0x004c9ef0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6088[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MainWnd_New_0)}};
const EhFuncInfo g_EhFuncInfo_004d6068 = {0x19930520u, 1, g_EhUnwindMap_004d6088, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MainWnd_New(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6068
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9f3e EH_Handler_MainWnd_Ctor - main frame window (0x230 object): paths "recoil" / "/campaigns", error log "recoil.err", menu resource "MYMENU", registry key Software\Westwood\WOLAPI\4352, serial chars "1234567890"; MFC CFrameWnd/CString setup (0x004c5bxx..0x004c5exx), 0x004306f0, 0x004437d0, 0x00462310, 0x0046d5c0, 0x004a07f0, 0x004a7470; ret 0x1c
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 90 60 4d 00 e9 58 c1 ff ff): MOV EAX,FuncInfo 0x004d6090; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 5 unwind states, unwind map 0x004d60b0 (to-state, action) = ['(-1, 0x004c9f10)', '(0, 0x004c9f18)', '(1, 0x004c9f26)', '(1, 0x004c9f2e)', '(3, 0x004c9f36)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d60b0[5] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MainWnd_Ctor_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_MainWnd_Ctor_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_MainWnd_Ctor_2)}, {1, reinterpret_cast<void*>(&EH_Unwind_MainWnd_Ctor_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_MainWnd_Ctor_4)}};
const EhFuncInfo g_EhFuncInfo_004d6090 = {0x19930520u, 5, g_EhUnwindMap_004d60b0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MainWnd_Ctor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6090
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9f60 EH_Handler_MainWnd_Dtor - bytes: vtbl 0x004d1140; CMenu member +0x1d0 (vtbl 0x004d1248) DestroyMenu, vtbl 0x004d1268; base frame dtor
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 d8 60 4d 00 e9 36 c1 ff ff): MOV EAX,FuncInfo 0x004d60d8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d60f8 (to-state, action) = ['(-1, 0x004c9f50)', '(-1, 0x004c9f58)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d60f8[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MainWnd_Dtor_0)}, {-1, reinterpret_cast<void*>(&EH_Unwind_MainWnd_Dtor_1)}};
const EhFuncInfo g_EhFuncInfo_004d60d8 = {0x19930520u, 2, g_EhUnwindMap_004d60f8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MainWnd_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d60d8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004c9f78 EH_Handler_MainWnd_OnAbout - bytes: stack CDialog via Dialog67_Ctor(0) (dialog 0x67), DoModal, ~CDialog
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 08 61 4d 00 e9 1e c1 ff ff): MOV EAX,FuncInfo 0x004d6108; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6128 (to-state, action) = ['(-1, 0x004c9f70)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6128[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_MainWnd_OnAbout_0)}};
const EhFuncInfo g_EhFuncInfo_004d6108 = {0x19930520u, 1, g_EhUnwindMap_004d6128, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_MainWnd_OnAbout(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6108
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004ca108 EH_Handler_Menus_RunNetSetupThenStartMission - ../../04_spec/systems/app.md
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 30 61 4d 00 e9 8e bf ff ff): MOV EAX,FuncInfo 0x004d6130; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 35 unwind states, unwind map 0x004d6150 (to-state, action) = ['(-1, 0x004c9f90)', '(0, 0x004c9f9b)', '(0, 0x004c9fa6)', '(2, 0x004c9fb1)', '(3, 0x004c9fbc)', '(4, 0x004c9fc7)', '(5, 0x004c9fd2)', '(-1, 0x004c9fda)', '(7, 0x004c9fe5)', '(8, 0x004c9ff0)', '(9, 0x004c9ffb)', '(10, 0x004ca006)', '(11, 0x004ca011)', '(0, 0x004ca01c)', '(13, 0x004ca027)', '(14, 0x004ca032)', '(15, 0x004ca03d)', '(16, 0x004ca048)', '(-1, 0x004ca050)', '(18, 0x004ca05b)', '(19, 0x004ca066)', '(20, 0x004ca071)', '(21, 0x004ca07c)', '(22, 0x004ca087)', '(0, 0x004ca092)', '(24, 0x004ca09d)', '(25, 0x004ca0a8)', '(26, 0x004ca0b3)', '(27, 0x004ca0be)', '(-1, 0x004ca0c6)', '(29, 0x004ca0d1)', '(30, 0x004ca0dc)', '(31, 0x004ca0e7)', '(32, 0x004ca0f2)', '(33, 0x004ca0fd)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6150[35] = {{-1, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_0)}, {0, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_1)}, {0, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_4)}, {4, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_5)}, {5, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_6)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_7)}, {7, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_8)}, {8, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_9)}, {9, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_10)}, {10, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_11)}, {11, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_12)}, {0, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_13)}, {13, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_14)}, {14, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_15)}, {15, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_16)}, {16, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_17)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_18)}, {18, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_19)}, {19, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_20)}, {20, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_21)}, {21, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_22)}, {22, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_23)}, {0, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_24)}, {24, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_25)}, {25, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_26)}, {26, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_27)}, {27, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_28)}, {-1, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_29)}, {29, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_30)}, {30, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_31)}, {31, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_32)}, {32, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_33)}, {33, reinterpret_cast<void*>(&EH_Unwind_Menus_RunNetSetupThenStartMission_34)}};
const EhFuncInfo g_EhFuncInfo_004d6130 = {0x19930520u, 35, g_EhUnwindMap_004d6150, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_Menus_RunNetSetupThenStartMission(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6130
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cac58 EH_Handler_RecoilApp_SetString4b8 - CString [this+0x4b8] = arg CString (0x004c5e40), destroy arg (0x004c5b88); ret 4
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 50 6c 4d 00 e9 3e b4 ff ff): MOV EAX,FuncInfo 0x004d6c50; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6c70 (to-state, action) = ['(-1, 0x004cac50)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6c70[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_RecoilApp_SetString4b8_0)}};
const EhFuncInfo g_EhFuncInfo_004d6c50 = {0x19930520u, 1, g_EhUnwindMap_004d6c70, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_RecoilApp_SetString4b8(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6c50
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cac78 EH_Handler_RecoilApp_SetString4bc - CString [this+0x4bc] = arg CString; destroy arg; ret 4
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 78 6c 4d 00 e9 1e b4 ff ff): MOV EAX,FuncInfo 0x004d6c78; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6c98 (to-state, action) = ['(-1, 0x004cac70)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6c98[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_RecoilApp_SetString4bc_0)}};
const EhFuncInfo g_EhFuncInfo_004d6c78 = {0x19930520u, 1, g_EhUnwindMap_004d6c98, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_RecoilApp_SetString4bc(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6c78
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cacb8 EH_Handler_RefCounted_Dtor - vtbl 0x004d1fe0; [1]=1; InterlockedDecrement(global count 0x004f53e4); DeleteCriticalSection(this+8)
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 10 6d 4d 00 e9 de b3 ff ff): MOV EAX,FuncInfo 0x004d6d10; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6d30 (to-state, action) = ['(-1, 0x004cacb0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6d30[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_RefCounted_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d6d10 = {0x19930520u, 1, g_EhUnwindMap_004d6d30, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_RefCounted_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6d10
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cacea EH_Handler_GameFrame_New - bytes: SEH; operator new(0xcc); GameFrame_Ctor(class name 'gamez' 0x004dd8e8)
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 b0 6d 4d 00 e9 ac b3 ff ff): MOV EAX,FuncInfo 0x004d6db0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6dd0 (to-state, action) = ['(-1, 0x004cace0)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6dd0[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_GameFrame_New_0)}};
const EhFuncInfo g_EhFuncInfo_004d6db0 = {0x19930520u, 1, g_EhUnwindMap_004d6dd0, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_GameFrame_New(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6db0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cad10 EH_Handler_GameFrame_Dtor - bytes: vtbl 0x004d21d8; EmptyStub_004a75e0; brush +0xc4 vtbl 0x004d22f8 DeleteObject, vtbl 0x004d1268; ~CFrameWnd
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 d8 6d 4d 00 e9 86 b3 ff ff): MOV EAX,FuncInfo 0x004d6dd8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d6df8 (to-state, action) = ['(-1, 0x004cad00)', '(-1, 0x004cad08)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6df8[2] = {{-1, reinterpret_cast<void*>(&EH_Unwind_GameFrame_Dtor_0)}, {-1, reinterpret_cast<void*>(&EH_Unwind_GameFrame_Dtor_1)}};
const EhFuncInfo g_EhFuncInfo_004d6dd8 = {0x19930520u, 2, g_EhUnwindMap_004d6df8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_GameFrame_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6dd8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cad28 EH_Handler_GameFrame_OnPaint - bytes: CPaintDC; if !0x004a59b0(): memory DC SelectObject bitmap; BitBlt or StretchBlt to 640x480 (0x280x0x1e0) SRCCOPY 0xcc0020; DeleteDC; ~CPaintDC
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 08 6e 4d 00 e9 6e b3 ff ff): MOV EAX,FuncInfo 0x004d6e08; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6e28 (to-state, action) = ['(-1, 0x004cad20)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6e28[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_GameFrame_OnPaint_0)}};
const EhFuncInfo g_EhFuncInfo_004d6e08 = {0x19930520u, 1, g_EhUnwindMap_004d6e28, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_GameFrame_OnPaint(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6e08
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cad48 EH_Handler_GdiObjA_Dtor - bytes: vtbl 0x004d22f8; CGdiObject::DeleteObject; vtbl 0x004d1268 (CGdiObject base)
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 30 6e 4d 00 e9 4e b3 ff ff): MOV EAX,FuncInfo 0x004d6e30; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6e50 (to-state, action) = ['(-1, 0x004cad40)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6e50[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_GdiObjA_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d6e30 = {0x19930520u, 1, g_EhUnwindMap_004d6e50, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_GdiObjA_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6e30
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cad68 EH_Handler_GdiObjB_Dtor - bytes: vtbl 0x004d22f8; DeleteObject; vtbl 0x004d1268; separate SEH frame 0x004cad68 (duplicate instantiation of 0x00443b90)
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 58 6e 4d 00 e9 2e b3 ff ff): MOV EAX,FuncInfo 0x004d6e58; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 1 unwind states, unwind map 0x004d6e78 (to-state, action) = ['(-1, 0x004cad60)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6e78[1] = {{-1, reinterpret_cast<void*>(&EH_Unwind_GdiObjB_Dtor_0)}};
const EhFuncInfo g_EhFuncInfo_004d6e58 = {0x19930520u, 1, g_EhUnwindMap_004d6e78, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_GdiObjB_Dtor(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6e58
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x00430c90 App_FatalErrorShutdown_00430c90 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_FatalErrorShutdown_00430c90(int, int)
{
    __asm {
        sub esp, 0x100
        cmp ecx, -0x1
        jnz L_430d6c
        push edi
        push esi
        mov ecx, 0x12
        call Message_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x8]
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
        mov ecx, 0x30
        call Message_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x88]
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
        xor ecx, ecx
        call Loader_WaitForThread
        call Render_FlipToGDISurface
        call zSnd_Shutdown
        call Net_Shutdown
        call zVideo_Close
        lea ecx, [esp + 0x88]
        lea edx, [esp + 0x8]
        push ecx
        push edx
        push offset g_Data_004da000 + 0x14ac
        call dword ptr [g_Iat_printf_004cc4dc]
        add esp, 0xc
        push 0x3e8
        call dword ptr [g_Iat_Sleep_004cc0b0]
        push 0x10
        call dword ptr [g_Iat_MessageBeep_004cc660]
        mov edx, dword ptr [g_Data_004da000 + 0x19eec]
        lea eax, [esp + 0x8]
        push 0x10
        lea ecx, [esp + 0x8c]
        push eax
        push ecx
        push edx
        call dword ptr [g_Iat_MessageBoxA_004cc664]
        xor ecx, ecx
        call App_ExitProcess
        pop esi
        pop edi
    L_430d6c:
        add esp, 0x100
        ret
    }
}

// 0x00441cb0 ComboDialog_RunModal - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ComboDialog_RunModal(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_ComboDialog_RunModal
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x12c
        push 0x0
        lea ecx, [esp + 0x14]
        call WolLoginDialog_Ctor
        lea ecx, [esp + 0x10]
        mov dword ptr [esp + 0x134], 0x0
        call dword ptr [g_Iat_MFC42_2514_004cc3f8]
        cmp eax, 0x1
        jnz L_441e48
        lea eax, [esp]
        lea ecx, [esp + 0xc]
        push eax
        lea edx, [esp + 0x8]
        push ecx
        push edx
        lea ecx, [esp + 0x1c]
        call ComboDialog_GetSelection
        mov eax, dword ptr [esp + 0x4]
        push ecx
        mov ecx, esp
        mov dword ptr [esp + 0xc], esp
        push eax
        call dword ptr [g_Iat_MFC42_537_004cc29c]
        mov ecx, dword ptr [g_Data_004da000 + 0x5e568]
        call RecoilApp_SetString4b8
        mov edx, dword ptr [esp + 0xc]
        push ecx
        mov ecx, esp
        mov dword ptr [esp + 0xc], esp
        push edx
        call dword ptr [g_Iat_MFC42_537_004cc29c]
        mov ecx, dword ptr [g_Data_004da000 + 0x5e568]
        call RecoilApp_SetString4bc
        mov eax, dword ptr [g_Data_004da000 + 0x5e568]
        mov ecx, dword ptr [esp]
        mov dword ptr [eax + 0x4c4], ecx
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        lea edx, [esp + 0x11c]
        push 0x4
        push edx
        mov dword ptr [esp + 0x144], 0x8
        call ArrayDtor_Eh2
        lea eax, [esp + 0x10c]
        mov byte ptr [esp + 0x134], 0x7
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        push eax
        call ArrayDtor_Eh2
        lea ecx, [esp + 0x104]
        mov byte ptr [esp + 0x134], 0x6
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        push ecx
        call ArrayDtor_Eh2
        lea edx, [esp + 0xfc]
        mov byte ptr [esp + 0x134], 0x5
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        push edx
        call ArrayDtor_Eh2
        lea ecx, [esp + 0xf4]
        mov byte ptr [esp + 0x134], 0x4
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0xf0]
        mov byte ptr [esp + 0x134], 0x3
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0xb0]
        mov byte ptr [esp + 0x134], 0x2
        call dword ptr [g_Iat_MFC42_656_004cc33c]
        lea ecx, [esp + 0x70]
        mov byte ptr [esp + 0x134], 0x1
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x10]
        mov dword ptr [esp + 0x134], 0xffffffff
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        mov eax, 0x1
        mov ecx, dword ptr [esp + 0x12c]
        mov dword ptr FS:[0x0], ecx
        add esp, 0x138
        ret
    L_441e48:
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        lea eax, [esp + 0x11c]
        push 0x4
        push eax
        mov dword ptr [esp + 0x144], 0x10
        call ArrayDtor_Eh2
        lea ecx, [esp + 0x10c]
        mov byte ptr [esp + 0x134], 0xf
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        push ecx
        call ArrayDtor_Eh2
        lea edx, [esp + 0x104]
        mov byte ptr [esp + 0x134], 0xe
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        push edx
        call ArrayDtor_Eh2
        lea eax, [esp + 0xfc]
        mov byte ptr [esp + 0x134], 0xd
        push dword ptr [g_Iat_MFC42_800_004cc2a0]
        push 0x2
        push 0x4
        push eax
        call ArrayDtor_Eh2
        lea ecx, [esp + 0xf4]
        mov byte ptr [esp + 0x134], 0xc
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0xf0]
        mov byte ptr [esp + 0x134], 0xb
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0xb0]
        mov byte ptr [esp + 0x134], 0xa
        call dword ptr [g_Iat_MFC42_656_004cc33c]
        lea ecx, [esp + 0x70]
        mov byte ptr [esp + 0x134], 0x9
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x10]
        mov dword ptr [esp + 0x134], 0xffffffff
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        mov ecx, dword ptr [esp + 0x12c]
        xor eax, eax
        mov dword ptr FS:[0x0], ecx
        add esp, 0x138
        ret
    }
}

// 0x004422a0 StatusDialog_CreateComObject - CoCreateInstance(clsid 0x004d18e8, 0, 1, iid 0x004d1858, &0x005392a0); 0x004425c0(&0x0053929c); 0x0042dc30([0x005392a0], [0x0053929c], 0x004d1868, &0x005392ac); tail
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StatusDialog_CreateComObject(int, int)
{
    __asm {
        push offset g_Data_004da000 + 0x5f2a0
        push offset g_RData_004cc000 + 0x5858
        push 0x1
        push 0x0
        push offset g_RData_004cc000 + 0x58e8
        call dword ptr [g_Iat_CoCreateInstance_004cc708]
        push offset g_Data_004da000 + 0x5f29c
        call RefCounted_Create
        mov eax, dword ptr [g_Data_004da000 + 0x5f29c]
        mov ecx, dword ptr [g_RData_004cc000 + 0x5fcc]
        mov edx, dword ptr [g_Data_004da000 + 0x5f2a0]
        push offset g_Data_004da000 + 0x5f2ac
        add eax, ecx
        push offset g_RData_004cc000 + 0x5868
        push eax
        push edx
        call Com_QueryAndCallSlot14
        ret
    }
}

// 0x00442320 WolPatch_DialogProc_00442320 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall WolPatch_DialogProc_00442320(int, int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        sub esp, 0x100
        cmp eax, 0x30
        push ebx
        push esi
        push edi
        ja L_44237f
        jz L_442516
        sub eax, 0x2
        jnz L_4423f5
        mov esi, dword ptr [esp + 0x110]
        push 0x1
        push esi
        call dword ptr [g_Iat_KillTimer_004cc654]
        call StatusDialog_Refresh
        push offset g_Data_004da000 + 0x5f198
        call dword ptr [g_Iat_SetCurrentDirectoryA_004cc124]
        mov eax, dword ptr [g_Data_004da000 + 0x5f2a4]
        push eax
        push esi
        call dword ptr [g_Iat_EndDialog_004cc6a4]
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0x100
        ret 0x10
    L_44237f:
        sub eax, 0x110
        jz L_44242c
        dec eax
        jz L_4423e4
        sub eax, 0x2
        jz L_4423a0
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        add esp, 0x100
        ret 0x10
    L_4423a0:
        mov eax, dword ptr [g_Data_004da000 + 0x5f2a4]
        test eax, eax
        jnz L_4423c5
        mov eax, dword ptr [g_Data_004da000 + 0x5f2a0]
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x14]
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0x100
        ret 0x10
    L_4423c5:
        mov edx, dword ptr [esp + 0x110]
        push edx
        call dword ptr [g_Iat_DestroyWindow_004cc694]
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0x100
        ret 0x10
    L_4423e4:
        mov eax, dword ptr [esp + 0x118]
        and eax, 0xffff
        sub eax, 0x2
        jz L_442403
    L_4423f5:
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        add esp, 0x100
        ret 0x10
    L_442403:
        mov eax, dword ptr [g_Data_004da000 + 0x5f2a0]
        push eax
        mov ecx, dword ptr [eax]
        call dword ptr [ecx + 0x10]
        mov edx, dword ptr [g_Data_004da000 + 0x5f2a8]
        push edx
        call dword ptr [g_Iat_DestroyWindow_004cc694]
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0x100
        ret 0x10
    L_44242c:
        call StatusDialog_CreateComObject
        mov esi, dword ptr [esp + 0x110]
        mov edi, dword ptr [g_Iat_SetDlgItemTextA_004cc690]
        push offset g_Data_004da000 + 0x1b240
        push 0x400
        push esi
        call edi
        push offset g_Data_004da000 + 0x5f198
        push 0x100
        call dword ptr [g_Iat_GetCurrentDirectoryA_004cc12c]
        mov eax, dword ptr [g_Data_004da000 + 0x1b2c0]
        lea edx, [esp + 0xc]
        lea ecx, [eax + 0x151]
        add eax, 0x51
        push ecx
        push eax
        push offset g_Data_004da000 + 0x3484
        push edx
        call dword ptr [g_Iat_sprintf_004cc5c4]
        mov eax, dword ptr [g_Data_004da000 + 0x1b2c0]
        mov ebx, dword ptr [g_Iat_SetCurrentDirectoryA_004cc124]
        add esp, 0x10
        add eax, 0x1d4
        push eax
        call ebx
        test eax, eax
        jnz L_4424b8
        mov ecx, dword ptr [g_Data_004da000 + 0x1b2c0]
        push eax
        add ecx, 0x1d4
        push ecx
        call dword ptr [g_Iat_CreateDirectoryA_004cc11c]
        mov edx, dword ptr [g_Data_004da000 + 0x1b2c0]
        add edx, 0x1d4
        push edx
        call ebx
    L_4424b8:
        push offset g_Data_004da000 + 0x1b240
        push 0x400
        push esi
        call edi
        mov eax, dword ptr [g_Data_004da000 + 0x1b2c0]
        mov ecx, dword ptr [g_Data_004da000 + 0x5f2a0]
        push offset g_Data_004da000 + 0x3468
        lea edi, [eax + 0x151]
        mov edx, dword ptr [ecx]
        push edi
        lea edi, [esp + 0x14]
        push edi
        lea edi, [eax + 0x193]
        push edi
        lea edi, [eax + 0x172]
        add eax, 0x10
        push edi
        push eax
        push ecx
        call dword ptr [edx + 0xc]
        push 0x0
        push 0x32
        push 0x1
        push esi
        mov dword ptr [g_Data_004da000 + 0x5f2a8], esi
        mov dword ptr [g_Data_004da000 + 0x5f2a4], 0x0
        call dword ptr [g_Iat_SetTimer_004cc658]
    L_442516:
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        add esp, 0x100
        ret 0x10
    }
}

// 0x00442530 App_PromptForEachListItem - count list ECX (next +0xc); for each item i: [0x004f52c0]=item, GetMessageByID(0x004f5240, 0x80, msg 0x3043, i, count) (0x004a5b60), DialogBoxParamA(hinst [0x004f3ef8], dialog 0xa2, parent [0x004f3eec], proc 0x00442320); -1 -> return 0; return 1
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_PromptForEachListItem(int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov edi, ecx
        xor ebx, ebx
        xor esi, esi
        mov eax, edi
        test edi, edi
        jz L_4425aa
    L_442540:
        mov eax, dword ptr [eax + 0xc]
        inc ebx
        test eax, eax
        jnz L_442540
        test edi, edi
        jz L_4425aa
        mov ebp, dword ptr [g_Iat_DialogBoxParamA_004cc6a8]
    L_442552:
        inc esi
        push ebx
        push esi
        push 0x3043
        push 0x80
        push offset g_Data_004da000 + 0x1b240
        mov dword ptr [g_Data_004da000 + 0x1b2c0], edi
        call GetMessageByID
        mov eax, dword ptr [g_Data_004da000 + 0x19eec]
        mov ecx, dword ptr [g_Data_004da000 + 0x19ef8]
        add esp, 0x14
        push 0x0
        push offset WolPatch_DialogProc_00442320
        push eax
        push 0xa2
        push ecx
        call ebp
        cmp eax, -0x1
        jz L_4425a3
        mov edi, dword ptr [edi + 0xc]
        test edi, edi
        jnz L_442552
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    L_4425a3:
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    L_4425aa:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        ret
    }
}

// 0x004425c0 RefCounted_Create - new(0x20); base ctor 0x00441600; vtbl 0x004d1fe0; InterlockedIncrement(0x004f53e4); EH continuation 0x0044263e stores result
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RefCounted_Create(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push -0x1
        push offset EH_Handler_RefCounted_Create
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        sub esp, 0xc
        push ebx
        push esi
        push edi
        mov edi, 0x8007000e
        mov dword ptr [ebp - 0x10], esp
        push 0x20
        mov dword ptr [ebp - 0x14], edi
        mov dword ptr [ebp - 0x18], 0x0
        mov dword ptr [ebp - 0x4], 0x0
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        mov esi, eax
        add esp, 0x4
        mov dword ptr [ebp - 0x1c], esi
        test esi, esi
        mov byte ptr [ebp - 0x4], 0x1
        jz L_44262e
        lea ecx, [esi + 0x4]
        call CritSecHolder_Init
        push offset g_Data_004da000 + 0x1b3e4
        mov byte ptr [ebp - 0x4], 0x2
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5fe0
        call dword ptr [g_Iat_InterlockedIncrement_004cc0d8]
        jmp EH_Continuation_0044263e
    L_44262e:
        xor esi, esi
        jmp EH_Continuation_0044263e
    }
}

// 0x00442632 EH_Catch_RefCounted_Create_0 - new(0x20); base ctor 0x00441600; vtbl 0x004d1fe0; InterlockedIncrement(0x004f53e4); EH continuation 0x0044263e stores result
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Catch_RefCounted_Create_0(int, int)
{
    __asm {
        mov eax, offset EH_Cont_RefCounted_Create_0
        ret
    }
}

// 0x00442638 EH_Cont_RefCounted_Create_0 - new(0x20); base ctor 0x00441600; vtbl 0x004d1fe0; InterlockedIncrement(0x004f53e4); EH continuation 0x0044263e stores result
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Cont_RefCounted_Create_0(int, int)
{
    __asm {
        mov edi, dword ptr [ebp - 0x14]
        mov esi, dword ptr [ebp - 0x18]
        jmp EH_Continuation_0044263e
    }
}

// 0x00442d00 RecoilApp_Run - SetThreadPriority(thread [+0x2c], 2); loop: pump PeekMessage/PumpMessage (vfunc+0x64; 0 -> vfunc+0x70 ExitInstance, return); Net_ReceiveMessages (0x0048ae70); cur=ScreenManager_GetCurrent; if not active [0x34]: WaitMessage when idle; if no queued screen command [0x51]: cur->vfunc+0x10() nonzero -> v
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_Run(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push -0x1
        push offset EH_Handler_RecoilApp_Run
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        sub esp, 0x20
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [ebp - 0x10], esp
        push 0x2
        mov eax, dword ptr [esi + 0x2c]
        push eax
        call dword ptr [g_Iat_SetThreadPriority_004cc128]
        mov dword ptr [ebp - 0x4], 0x0
    L_442d37:
        mov ebx, dword ptr [g_Iat_PeekMessageA_004cc64c]
    L_442d3d:
        push 0x0
        push 0x0
        push 0x0
        lea ecx, [esi + 0x34]
        push 0x0
        push ecx
        call ebx
        test eax, eax
        jz L_442d72
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0x64]
        test eax, eax
        jnz L_442d3d
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax + 0x70]
        mov ecx, dword ptr [ebp - 0xc]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_442d72:
        or ecx, 0xffffffff
        call Net_ReceiveMessages
        mov ecx, esi
        call ScreenManager_GetCurrent
        mov edi, eax
        mov eax, dword ptr [esi + 0xd0]
        test eax, eax
        jz L_442fea
        mov edx, dword ptr [esi + 0x144]
        xor ecx, ecx
        test edx, edx
        setz CL
        test CL, CL
        jnz L_442fbc
        lea edx, [esi + 0x11c]
        mov eax, dword ptr [esi + 0x11c]
        mov dword ptr [ebp - 0x28], eax
        mov ecx, dword ptr [edx + 0x4]
        mov dword ptr [ebp - 0x24], ecx
        mov eax, dword ptr [edx + 0x8]
        mov ebx, dword ptr [eax]
        mov edx, dword ptr [edx + 0xc]
        mov dword ptr [ebp - 0x1c], edx
        mov dword ptr [ebp - 0x14], ebx
        mov eax, dword ptr [ebx + 0x4]
        dec eax
        jz L_442e97
        dec eax
        jz L_442e3a
        dec eax
        jnz L_442edc
        mov eax, dword ptr [ebx + 0x8]
        test eax, eax
        jz L_442edc
        test edi, edi
        jz L_442df3
        mov eax, dword ptr [edi]
        mov ecx, edi
        call dword ptr [eax + 0x18]
    L_442df3:
        mov eax, dword ptr [esi + 0xc8]
        test eax, eax
        jge L_442e07
        mov dword ptr [esi + 0xc8], 0x0
    L_442e07:
        cmp dword ptr [esi + 0xc8], 0x10
        jl L_442e1a
        mov dword ptr [esi + 0xc8], 0xf
    L_442e1a:
        mov ecx, dword ptr [ebx + 0x8]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0xc]
        test eax, eax
        jnz L_442e85
        test edi, edi
        jz L_442edc
        mov edx, dword ptr [edi]
        mov ecx, edi
        call dword ptr [edx + 0xc]
        jmp L_442edc
    L_442e3a:
        mov eax, dword ptr [ebx + 0x8]
        test eax, eax
        jz L_442edc
        mov eax, dword ptr [esi + 0xc8]
        mov ecx, dword ptr [esi + eax*0x4 + 0xd8]
        mov eax, dword ptr [ebx + 0xc]
        push eax
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0x1c]
        mov ecx, dword ptr [ebx + 0x8]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0xc]
        test eax, eax
        jz L_442edc
        mov ecx, dword ptr [esi + 0xc8]
        inc ecx
        mov eax, ecx
        mov dword ptr [esi + 0xc8], ecx
        cmp eax, 0x10
        jl L_442e85
        mov dword ptr [esi + 0xc8], 0xf
    L_442e85:
        mov eax, dword ptr [esi + 0xc8]
        mov ecx, dword ptr [ebx + 0x8]
        mov dword ptr [esi + eax*0x4 + 0xd8], ecx
        jmp L_442edc
    L_442e97:
        test edi, edi
        jz L_442ea2
        mov edx, dword ptr [edi]
        mov ecx, edi
        call dword ptr [edx + 0x18]
    L_442ea2:
        mov ecx, dword ptr [esi + 0xc8]
        xor eax, eax
        mov dword ptr [esi + ecx*0x4 + 0xd8], eax
        mov ecx, dword ptr [esi + 0xc8]
        dec ecx
        mov dword ptr [esi + 0xc8], ecx
        jns L_442ec6
        mov dword ptr [esi + 0xc8], eax
    L_442ec6:
        mov edx, dword ptr [esi + 0xc8]
        mov ecx, dword ptr [esi + edx*0x4 + 0xd8]
        mov edx, dword ptr [ebx + 0xc]
        push edx
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0x20]
    L_442edc:
        mov ecx, dword ptr [esi + 0x124]
        mov eax, dword ptr [esi + 0x144]
        add ecx, 0x4
        dec eax
        mov dword ptr [esi + 0x124], ecx
        mov edi, ecx
        xor ecx, ecx
        mov dword ptr [esi + 0x144], eax
        test eax, eax
        setz CL
        test CL, CL
        jnz L_442f11
        cmp edi, dword ptr [esi + 0x120]
        jnz L_442fae
    L_442f11:
        mov eax, dword ptr [esi + 0x128]
        lea edx, [eax + 0x4]
        mov dword ptr [esi + 0x128], edx
        mov eax, dword ptr [eax]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        mov eax, dword ptr [esi + 0x144]
        add esp, 0x4
        xor ecx, ecx
        test eax, eax
        setz CL
        test CL, CL
        jz L_442f8a
        lea ebx, [esi + 0x11c]
        xor eax, eax
        xor ecx, ecx
        xor edx, edx
        mov dword ptr [ebx], eax
        xor edi, edi
        mov dword ptr [ebx + 0x4], ecx
        mov dword ptr [ebx + 0x8], edx
        mov dword ptr [ebx + 0xc], edi
        lea ebx, [esi + 0x12c]
        mov dword ptr [esi + 0x12c], eax
        mov dword ptr [ebx + 0x4], ecx
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [esi + 0x13c]
        push edx
        mov dword ptr [ebx + 0xc], edi
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        mov ebx, dword ptr [ebp - 0x14]
        add esp, 0x4
        push ebx
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        jmp L_442d37
    L_442f8a:
        mov eax, dword ptr [esi + 0x128]
        lea edi, [esi + 0x11c]
        mov ecx, dword ptr [eax]
        mov dword ptr [edi], ecx
        mov ebx, ecx
        lea edx, [ecx + 0x1000]
        mov dword ptr [edi + 0x4], edx
        mov dword ptr [edi + 0x8], ebx
        mov ebx, dword ptr [ebp - 0x14]
        mov dword ptr [edi + 0xc], eax
    L_442fae:
        push ebx
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        add esp, 0x4
        jmp L_442d37
    L_442fbc:
        test edi, edi
        jz L_442d3d
        mov eax, dword ptr [edi]
        mov ecx, edi
        call dword ptr [eax + 0x10]
        test eax, eax
        jz L_442d3d
        mov edx, dword ptr [esi]
        mov ecx, esi
        call dword ptr [edx + 0xa8]
        push 0x0
        call dword ptr [g_Iat_PostQuitMessage_004cc6b8]
        jmp L_442d3d
    L_442fea:
        push 0x0
        push 0x0
        push 0x0
        lea eax, [esi + 0x34]
        push 0x0
        push eax
        call ebx
        test eax, eax
        jnz L_442d3d
        call dword ptr [g_Iat_WaitMessage_004cc6b4]
        jmp L_442d3d
    }
}

// 0x0044300b AppRun_CatchMemoryException - SetThreadPriority(thread [+0x2c], 2); loop: pump PeekMessage/PumpMessage (vfunc+0x64; 0 -> vfunc+0x70 ExitInstance, return); Net_ReceiveMessages (0x0048ae70); cur=ScreenManager_GetCurrent; if not active [0x34]: WaitMessage when idle; if no queued screen command [0x51]: cur->vfunc+0x10() nonzero -> v
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppRun_CatchMemoryException(int, int)
{
    __asm {
        push 0x0
        push 0x10
        push offset g_Data_004da000 + 0x38cc
        push offset g_Data_004da000 + 0x3890
        push 0x0
        call dword ptr [g_Iat_MessageBoxExA_004cc648]
        push 0x0
        call dword ptr [g_Iat_exit_004cc4b0]
        add esp, 0x4
        mov eax, offset EH_Cont_RecoilApp_Run_0
        ret
    }
}

// 0x00443032 AppRun_CatchFileException - SetThreadPriority(thread [+0x2c], 2); loop: pump PeekMessage/PumpMessage (vfunc+0x64; 0 -> vfunc+0x70 ExitInstance, return); Net_ReceiveMessages (0x0048ae70); cur=ScreenManager_GetCurrent; if not active [0x34]: WaitMessage when idle; if no queued screen command [0x51]: cur->vfunc+0x10() nonzero -> v
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppRun_CatchFileException(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x18]
        mov eax, offset g_Data_004da000 + 0x3880
        mov ecx, dword ptr [ecx + 0x8]
        cmp ecx, 0xe
        ja L_4430a9
        cmp ecx, 0
        je L_443049
        cmp ecx, 1
        je L_443049
        cmp ecx, 2
        je L_443050
        cmp ecx, 3
        je L_443057
        cmp ecx, 4
        je L_44305e
        cmp ecx, 5
        je L_443065
        cmp ecx, 6
        je L_44306c
        cmp ecx, 7
        je L_443073
        cmp ecx, 8
        je L_44307a
        cmp ecx, 9
        je L_443081
        cmp ecx, 10
        je L_443088
        cmp ecx, 11
        je L_44308f
        cmp ecx, 12
        je L_443096
        cmp ecx, 13
        je L_44309d
        cmp ecx, 14
        je L_4430a4
        int 3  // unreachable: the bounds check above excludes other indices
    L_443049:
        mov eax, offset g_Data_004da000 + 0xbce0
        jmp L_4430a9
    L_443050:
        mov eax, offset g_Data_004da000 + 0x3860
        jmp L_4430a9
    L_443057:
        mov eax, offset g_Data_004da000 + 0x383c
        jmp L_4430a9
    L_44305e:
        mov eax, offset g_Data_004da000 + 0x3808
        jmp L_4430a9
    L_443065:
        mov eax, offset g_Data_004da000 + 0x37e8
        jmp L_4430a9
    L_44306c:
        mov eax, offset g_Data_004da000 + 0x37b4
        jmp L_4430a9
    L_443073:
        mov eax, offset g_Data_004da000 + 0x3780
        jmp L_4430a9
    L_44307a:
        mov eax, offset g_Data_004da000 + 0x3758
        jmp L_4430a9
    L_443081:
        mov eax, offset g_Data_004da000 + 0x3724
        jmp L_4430a9
    L_443088:
        mov eax, offset g_Data_004da000 + 0x3708
        jmp L_4430a9
    L_44308f:
        mov eax, offset g_Data_004da000 + 0x36cc
        jmp L_4430a9
    L_443096:
        mov eax, offset g_Data_004da000 + 0x368c
        jmp L_4430a9
    L_44309d:
        mov eax, offset g_Data_004da000 + 0x3678
        jmp L_4430a9
    L_4430a4:
        mov eax, offset g_Data_004da000 + 0x3658
    L_4430a9:
        push 0x0
        push 0x10
        push offset g_Data_004da000 + 0x364c
        push eax
        push 0x0
        call dword ptr [g_Iat_MessageBoxExA_004cc648]
        push 0x0
        call dword ptr [g_Iat_exit_004cc4b0]
        add esp, 0x4
        mov eax, offset EH_Cont_RecoilApp_Run_0
        ret
    }
}

// 0x004430cc AppRun_CatchGeneralException - SetThreadPriority(thread [+0x2c], 2); loop: pump PeekMessage/PumpMessage (vfunc+0x64; 0 -> vfunc+0x70 ExitInstance, return); Net_ReceiveMessages (0x0048ae70); cur=ScreenManager_GetCurrent; if not active [0x34]: WaitMessage when idle; if no queued screen command [0x51]: cur->vfunc+0x10() nonzero -> v
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppRun_CatchGeneralException(int, int)
{
    __asm {
        push 0x0
        push 0x10
        push offset g_Data_004da000 + 0x363c
        push offset g_Data_004da000 + 0x3610
        push 0x0
        call dword ptr [g_Iat_MessageBoxExA_004cc648]
        push 0x0
        call dword ptr [g_Iat_exit_004cc4b0]
        add esp, 0x4
        mov eax, offset EH_Cont_RecoilApp_Run_0
        ret
    }
}

// 0x004430f3 EH_Cont_RecoilApp_Run_0 - SetThreadPriority(thread [+0x2c], 2); loop: pump PeekMessage/PumpMessage (vfunc+0x64; 0 -> vfunc+0x70 ExitInstance, return); Net_ReceiveMessages (0x0048ae70); cur=ScreenManager_GetCurrent; if not active [0x34]: WaitMessage when idle; if no queued screen command [0x51]: cur->vfunc+0x10() nonzero -> v
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Cont_RecoilApp_Run_0(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0xc]
        pop edi
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00443ab0 GameFrame_OnDestroy - bytes: DPlay_DestroyLocalPlayer, zVideo_Close, SoundCD_Stop, CFrameWnd::OnDestroy, DeleteObject(brush +0xc4)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GameFrame_OnDestroy(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call DPlay_DestroyLocalPlayer
        call zVideo_Close
        call SoundCD_Stop
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_4499_004cc290]
        lea ecx, [esi + 0xc4]
        call dword ptr [g_Iat_MFC42_2414_004cc274]
        pop esi
        ret
    }
}

// 0x004cab60 EH_Unwind_ComboDialog_RunModal_0 - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_ComboDialog_RunModal_0(int, int)
{
    __asm {
        lea ecx, [ebp + 0xfffffed8]
        jmp ComboDialog_Dtor
    }
}

// 0x004cac90 EH_Unwind_RefCounted_Create_0 - new(0x20); base ctor 0x00441600; vtbl 0x004d1fe0; InterlockedIncrement(0x004f53e4); EH continuation 0x0044263e stores result
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_RefCounted_Create_0(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x1c]
        push eax
        call dword ptr [g_Iat_MFC42_825_004cc2b8]
        pop ecx
        ret
    }
}

// 0x004cac9b EH_Unwind_RefCounted_Create_1 - new(0x20); base ctor 0x00441600; vtbl 0x004d1fe0; InterlockedIncrement(0x004f53e4); EH continuation 0x0044263e stores result
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall EH_Unwind_RefCounted_Create_1(int, int)
{
    __asm {
        mov ecx, dword ptr [ebp - 0x1c]
        jmp CritSec_Delete
    }
}

// 0x004cac3f EH_Handler_ComboDialog_RunModal - stack dialog built by 0x00441750; CDialog::DoModal; IDOK (1): ComboDialog_GetSelection -> (a, b, c); RecoilApp_SetString4b8(a), RecoilApp_SetString4bc(b), [[0x00538568]+0x4c4]=c; destroy locals (4 CString arrays, 2 CStrings, CEdit, CComboBox, CDialog); return 1; else destroy locals, return 0
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 a8 6b 4d 00 e9 57 b4 ff ff): MOV EAX,FuncInfo 0x004d6ba8; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 17 unwind states, unwind map 0x004d6bc8 (to-state, action) = ['(-1, 0x004cab60)', '(-1, 0x004cab6b)', '(1, 0x004cab76)', '(2, 0x004cab81)', '(3, 0x004cab8c)', '(4, 0x004cab94)', '(5, 0x004cab9c)', '(6, 0x004cabaf)', '(7, 0x004cabc2)', '(-1, 0x004cabd5)', '(9, 0x004cabe0)', '(10, 0x004cabeb)', '(11, 0x004cabf6)', '(12, 0x004cabfe)', '(13, 0x004cac06)', '(14, 0x004cac19)', '(15, 0x004cac2c)'], no try blocks;
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6bc8[17] = {{-1, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_0)}, {-1, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_1)}, {1, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_2)}, {2, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_3)}, {3, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_4)}, {4, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_5)}, {5, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_6)}, {6, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_7)}, {7, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_8)}, {-1, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_9)}, {9, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_10)}, {10, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_11)}, {11, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_12)}, {12, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_13)}, {13, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_14)}, {14, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_15)}, {15, reinterpret_cast<void*>(&EH_Unwind_ComboDialog_RunModal_16)}};
const EhFuncInfo g_EhFuncInfo_004d6ba8 = {0x19930520u, 17, g_EhUnwindMap_004d6bc8, 0, nullptr};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_ComboDialog_RunModal(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6ba8
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004caca3 EH_Handler_RefCounted_Create - new(0x20); base ctor 0x00441600; vtbl 0x004d1fe0; InterlockedIncrement(0x004f53e4); EH continuation 0x0044263e stores result
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 a0 6c 4d 00 e9 f3 b3 ff ff): MOV EAX,FuncInfo 0x004d6ca0; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 5 unwind states, unwind map 0x004d6cc0 (to-state, action) = ['(-1, 0x00000000)', '(0, 0x004cac90)', '(1, 0x004cac9b)', '(-1, 0x00000000)', '(-1, 0x00000000)'], 1 try block(s) at 0x004d6ce8 [(0, 2, 3, [('0x00000000', '0x00442632')])];
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6cc0[5] = {{-1, nullptr}, {0, reinterpret_cast<void*>(&EH_Unwind_RefCounted_Create_0)}, {1, reinterpret_cast<void*>(&EH_Unwind_RefCounted_Create_1)}, {-1, nullptr}, {-1, nullptr}};
const EhHandlerType g_EhHandlerTypes_004d6d00[1] = {{0x0u, nullptr, 0, reinterpret_cast<void*>(&EH_Catch_RefCounted_Create_0)}};
const EhTryBlockMapEntry g_EhTryMap_004d6ce8[1] = {{0, 2, 3, 1, const_cast<EhHandlerType*>(g_EhHandlerTypes_004d6d00)}};
const EhFuncInfo g_EhFuncInfo_004d6ca0 = {0x19930520u, 5, g_EhUnwindMap_004d6cc0, 1, g_EhTryMap_004d6ce8};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_RefCounted_Create(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6ca0
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x004cacd0 EH_Handler_RecoilApp_Run - SetThreadPriority(thread [+0x2c], 2); loop: pump PeekMessage/PumpMessage (vfunc+0x64; 0 -> vfunc+0x70 ExitInstance, return); Net_ReceiveMessages (0x0048ae70); cur=ScreenManager_GetCurrent; if not active [0x34]: WaitMessage when idle; if no queued screen command [0x51]: cur->vfunc+0x10() nonzero -> vfunc+0xa8, PostQuitMessage(0). Screen command queue (block deque +0x11c..+0x144, 0x1000-byte blocks, head [0x49]): type 1 pop: cur->vfunc+0x18 exit, stack[[0x32]] cleared, depth-- (clamp 0), top->vfunc+0x20(arg); type 2 push: top->vfunc+0x1c(arg), new->vfunc+0xc() ok -> depth++ (clamp 15), stack[depth]=new; type 3 replace: cur->vfunc+0x18, clamp depth 0..15, new->vfunc+0xc() ok -> stack[depth]=new else cur->vfunc+0xc; pop command, free drained blocks
// Generated by tools/gen_eh_frames.py from the exe bytes (b8 38 6d 4d 00 e9 c6 b3 ff ff): MOV EAX,FuncInfo 0x004d6d38; JMP 0x004c60a0 (-> __CxxFrameHandler).
// FuncInfo CONFIRMED-BINARY: magic 0x19930520, 2 unwind states, unwind map 0x004d6d58 (to-state, action) = ['(-1, 0x00000000)', '(-1, 0x00000000)'], 1 try block(s) at 0x004d6d68 [(0, 0, 1, [('0x004dd5f0', '0x0044300b'), ('0x004dd5d0', '0x00443032'), ('0x004dd5b0', '0x004430cc')])];
// the port's table points at the port's funclets. Needs /SAFESEH:NO (CMakeLists.txt).
namespace {
const EhUnwindMapEntry g_EhUnwindMap_004d6d58[2] = {{-1, nullptr}, {-1, nullptr}};
const EhHandlerType g_EhHandlerTypes_004d6d80[3] = {{0x0u, g_Data_004da000 + 0x35f0, -44, reinterpret_cast<void*>(&AppRun_CatchMemoryException)}, {0x0u, g_Data_004da000 + 0x35d0, -24, reinterpret_cast<void*>(&AppRun_CatchFileException)}, {0x0u, g_Data_004da000 + 0x35b0, -48, reinterpret_cast<void*>(&AppRun_CatchGeneralException)}};
const EhTryBlockMapEntry g_EhTryMap_004d6d68[1] = {{0, 0, 1, 3, const_cast<EhHandlerType*>(g_EhHandlerTypes_004d6d80)}};
const EhFuncInfo g_EhFuncInfo_004d6d38 = {0x19930520u, 2, g_EhUnwindMap_004d6d58, 1, g_EhTryMap_004d6d68};
}  // namespace

__declspec(naked) int __fastcall EH_Handler_RecoilApp_Run(int, int)
{
    __asm {
        mov eax, offset g_EhFuncInfo_004d6d38
        jmp dword ptr [g_Iat___CxxFrameHandler_004cc5b0]
    }
}

// 0x0042f9d0 AppScreen_Shutdown_0042f9d0 - REFINED 2026-09-29: BYTE RE-READ 2026-09-29 (Sonnet) - confirmed: CALL 0x0048a980; ECX = 0x004f3ca8, CALL 0x0042e430; CALL 0x004a1f40; MOV EAX,1; RET. | previous note: vtable-only (G1 2026-09-25); full listing read in Ghidra (03_re/decomp/vtable_orphans_listing.txt): 0x0048a980(); App_ShutdownGame(0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AppScreen_Shutdown_0042f9d0(int, int)
{
    __asm {
        call DPlay_DestroyLocalPlayer
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call App_ShutdownGame
        call zSnd_ReleaseDevice
        mov eax, 0x1
        ret
    }
}

// 0x004c8201 AfxInitAppState_Ctor - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AfxInitAppState_Ctor(int, int)
{
    __asm {
        push esi
        push 0x421
        mov esi, ecx
        push 0x0
        call Afx_SetMbcsState
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004c8214 Thunk_004c8219_004c8214 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Thunk_004c8219_004c8214(int, int)
{
    __asm {
        jmp StaticInit_AfxInitAppState_0056cc28
    }
}


// 0x004317d0 Screen_UpdatePanelButtonAndAccelerator_004317d0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall Screen_UpdatePanelButtonAndAccelerator_004317d0(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        mov edx, dword ptr [ecx + 0x1f8]
        sub esp, 0x40
        cmp edx, eax
        jl L_431841
        mov edx, dword ptr [ecx + eax*0x4 + 0x1fc]
        push edi
        push esi
        mov esi, dword ptr [esp + 0x4c]
        cmp edx, 0x8
        lea edi, [eax - 0x1]
        jnz L_431801
        mov eax, dword ptr [esi]
        push 0x1
        mov ecx, esi
        call dword ptr [eax + 0x4]
        jmp L_43180a
    L_431801:
        mov edx, dword ptr [esi]
        push 0x0
        mov ecx, esi
        call dword ptr [edx + 0x4]
    L_43180a:
        mov ecx, edi
        call ZVideoTable_ElementAddr_00633e58_004a7450
        push eax
        mov ecx, edi
        call ZVideoTable_ElementAddr_00633e78_004a7430
        push eax
        lea eax, [esp + 0x10]
        push offset g_Data_004da000 + 0x2d70
        push eax
        call dword ptr [g_Iat_sprintf_004cc5c4]
        mov edx, dword ptr [esi]
        add esp, 0x10
        lea eax, [esp + 0x8]
        mov ecx, esi
        push eax
        call dword ptr [edx + 0xc]
        pop esi
        pop edi
        add esp, 0x40
        ret 0x8
    L_431841:
        mov edx, dword ptr [esp + 0x44]
        mov ecx, dword ptr [ecx + eax*0x4 + 0x20c]
        push 0x0
        push ecx
        mov eax, dword ptr [edx + 0xc]
        mov ecx, dword ptr [eax + 0x4]
        push ecx
        call dword ptr [g_Iat_RemoveMenu_004cc618]
        add esp, 0x40
        ret 0x8
    }
}

// 0x004c8219 StaticInit_AfxInitAppState_0056cc28 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall StaticInit_AfxInitAppState_0056cc28(int, int)
{
    __asm {
        mov ecx, offset g_Data_004da000 + 0x92c28
        jmp AfxInitAppState_Ctor
    }
}

// 0x00442a10 RecoilApp_TakeFieldD0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_TakeFieldD0(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0xd0]
        mov dword ptr [ecx + 0xd0], 0x0
        ret
    }
}

// 0x00442a50 Engine_InitSubsystems - 0x0048c7d0; 0x0048cc70; then each init logged via printf "<name>Init: %s" PASSED/FAILED: gModInit 0x00475c40, gClsInit 0x004a75e0, zEffInit 0x00460020, zRndrInit 0x0048fd80, zSndInit 0x004a12c0 (success = nonzero; others success = 0), zUtlInit 0x004a75e0, zWepInit 0x004b1090, zImgInit 0x0046eb20, 0x
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Engine_InitSubsystems(int, int, int)
{
    __asm {
        push ebx
        mov ebx, ecx
        push esi
        push edi
        xor ecx, ecx
        call Container_InitNodePool
        xor ecx, ecx
        call Archive_InitRegistry
        call Render_InitRenderState
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442a76
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442a76:
        mov esi, dword ptr [g_Iat_printf_004cc4dc]
        push eax
        push offset g_Data_004da000 + 0x35a0
        call esi
        add esp, 0x8
        call EmptyStub_004a75e0
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442a9a
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442a9a:
        push eax
        push offset g_Data_004da000 + 0x3590
        call esi
        add esp, 0x8
        call EffectConfig_Reset
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442ab8
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442ab8:
        push eax
        push offset g_Data_004da000 + 0x3580
        call esi
        add esp, 0x8
        call zRndrInit
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442ad6
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442ad6:
        push eax
        push offset g_Data_004da000 + 0x3570
        call esi
        mov edi, dword ptr [esp + 0x18]
        add esp, 0x8
        mov ecx, edi
        call zSndInit
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jnz L_442afa
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442afa:
        push eax
        push offset g_Data_004da000 + 0x3560
        call esi
        add esp, 0x8
        call EmptyStub_004a75e0
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442b18
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442b18:
        push eax
        push offset g_Data_004da000 + 0x3550
        call esi
        add esp, 0x8
        call Weapon_InitSubsystem
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442b36
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442b36:
        push eax
        push offset g_Data_004da000 + 0x3540
        call esi
        add esp, 0x8
        xor ecx, ecx
        call TextureTable_Init
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442b56
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442b56:
        push eax
        push offset g_Data_004da000 + 0x3530
        call esi
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        add esp, 0x8
        cmp eax, 0x2
        jnz L_442b75
        mov ecx, 0x5
        call Input_SetMouseMode_Exchange
    L_442b75:
        mov edx, dword ptr [ebx + 0x6c]
        mov ecx, edi
        call zInInit
        test eax, eax
        mov eax, offset g_Data_004da000 + 0x2af8
        jz L_442b8d
        mov eax, offset g_Data_004da000 + 0x2af0
    L_442b8d:
        push eax
        push offset g_Data_004da000 + 0x3520
        call esi
        add esp, 0x8
        call Timer_Reset
        mov ecx, 0x1
        call Global_Set_0056b564
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        ret 0x4
    }
}

// 0x0042e110 App_CreateMainWindow - new(0x230) main window 0x00430250; return it or 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_CreateMainWindow(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_App_CreateMainWindow
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push 0x230
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov dword ptr [esp], eax
        test eax, eax
        mov dword ptr [esp + 0xc], 0x0
        jz L_42e159
        mov ecx, eax
        call MainWnd_Ctor
        mov ecx, dword ptr [esp + 0x4]
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    L_42e159:
        mov ecx, dword ptr [esp + 0x4]
        xor eax, eax
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    }
}

// 0x0042e520 MainApp_InitInstance_0042e520 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainApp_InitInstance_0042e520(int, int)
{
    __asm {
        sub esp, 0x738
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov dword ptr [esp + 0x10], esi
        call App_ActivateExistingInstance
        test eax, eax
        jnz L_42e544
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x738
        ret
    L_42e544:
        mov ecx, 0xa
        xor eax, eax
        lea edi, [esp + 0x1c]
        rep stosd
        mov eax, dword ptr [g_Iat_DefWindowProcA_004cc63c]
        mov dword ptr [esp + 0x1c], 0xb
        mov dword ptr [esp + 0x20], eax
        call dword ptr [g_Iat_MFC42_1168_004cc19c]
        mov eax, dword ptr [eax + 0x8]
        push 0x97
        push 0xe
        push 0x97
        mov dword ptr [esp + 0x38], eax
        call dword ptr [g_Iat_MFC42_1146_004cc1b8]
        push eax
        call dword ptr [g_Iat_LoadIconA_004cc668]
        push 0x7f00
        push 0xc
        push 0x7f00
        mov dword ptr [esp + 0x3c], eax
        call dword ptr [g_Iat_MFC42_1146_004cc1b8]
        push eax
        call dword ptr [g_Iat_LoadCursorA_004cc60c]
        xor edi, edi
        mov dword ptr [esp + 0x34], eax
        push edi
        call dword ptr [g_Iat_CreateSolidBrush_004cc068]
        mov ecx, dword ptr [g_Data_004da000 + 0x2ac0]
        lea edx, [esp + 0x1c]
        push edx
        mov dword ptr [esp + 0x3c], eax
        mov dword ptr [esp + 0x40], edi
        mov dword ptr [esp + 0x44], ecx
        call dword ptr [g_Iat_MFC42_1232_004cc1c0]
        test eax, eax
        jnz RecoilApp_BootstrapAndCDCheck
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x738
        ret
    }
}

// 0x004301e0 MainWnd_New - bytes: SEH; operator new(0x230); MainWnd_Ctor 0x00430250; returns object or 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_New(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MainWnd_New
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push 0x230
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov dword ptr [esp], eax
        test eax, eax
        mov dword ptr [esp + 0xc], 0x0
        jz L_430229
        mov ecx, eax
        call MainWnd_Ctor
        mov ecx, dword ptr [esp + 0x4]
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    L_430229:
        mov ecx, dword ptr [esp + 0x4]
        xor eax, eax
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    }
}

// 0x00430250 MainWnd_Ctor - main frame window (0x230 object): paths "recoil" / "/campaigns", error log "recoil.err", menu resource "MYMENU", registry key Software\Westwood\WOLAPI\4352, serial chars "1234567890"; MFC CFrameWnd/CString setup (0x004c5bxx..0x004c5exx), 0x004306f0, 0x004437d0, 0x00462310, 0x0046d5c0, 0x004a07f0, 0x
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_Ctor(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_MainWnd_Ctor
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x10
        push ebx
        push ebp
        push esi
        push edi
        mov esi, ecx
        push offset g_Data_004da000 + 0x2d4c
        mov dword ptr [esp + 0x20], esi
        call GameFrame_Ctor
        xor edi, edi
        mov dword ptr [esi + 0x1d0], offset g_RData_004cc000 + 0x5248
        mov dword ptr [esp + 0x28], edi
        mov dword ptr [esi + 0x1d4], edi
        lea eax, [esp + 0x14]
        mov ecx, esi
        push eax
        mov byte ptr [esp + 0x2c], 0x1
        mov dword ptr [esi], offset g_RData_004cc000 + 0x5140
        call MainWnd_GetTitle
        mov ebp, dword ptr [eax]
        push edi
        push edi
        push edi
        mov edi, dword ptr [g_Iat_GetSystemMetrics_004cc640]
        push 0x4
        mov byte ptr [esp + 0x38], 0x2
        call edi
        push 0xf
        mov ebx, eax
        call edi
        push 0x21
        add ebx, eax
        call edi
        lea ecx, [ebx + eax*0x2 + 0x1e0]
        push ecx
        push 0x20
        call edi
        lea edx, [eax + eax*0x1 + 0x280]
        mov eax, dword ptr [g_Data_004da000 + 0x2ac0]
        push edx
        push 0x80000000
        push 0x80000000
        push 0x82ca0000
        push ebp
        push eax
        push 0x20000
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_2152_004cc3b8]
        lea ecx, [esp + 0x14]
        mov byte ptr [esp + 0x28], 0x1
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        xor ebx, ebx
        mov dword ptr [esi + 0x1dc], 0x1
        mov dword ptr [esi + 0x22c], ebx
        call dword ptr [g_Iat_GetCommandLineA_004cc0f8]
        push eax
        call dword ptr [g_Iat__strdup_004cc5e4]
        add esp, 0x4
        mov ebp, eax
        mov dword ptr [esp + 0x14], ebp
        push offset g_Data_004da000 + 0xd08
        push ebp
        call dword ptr [g_Iat_strtok_004cc4c0]
        mov edi, eax
        add esp, 0x8
        cmp edi, ebx
        jz L_43039d
        mov ebp, dword ptr [g_Iat_strncmp_004cc5cc]
    L_430350:
        push 0x4
        push offset g_Data_004da000 + 0x2d40
        push edi
        call ebp
        add esp, 0xc
        test eax, eax
        jnz L_43036d
        mov dword ptr [esi + 0x22c], 0x1
        jmp L_430384
    L_43036d:
        push 0x4
        push offset g_Data_004da000 + 0x2d34
        push edi
        call ebp
        add esp, 0xc
        test eax, eax
        jnz L_430384
        mov dword ptr [esi + 0x1dc], ebx
    L_430384:
        push offset g_Data_004da000 + 0xd08
        push ebx
        call dword ptr [g_Iat_strtok_004cc4c0]
        mov edi, eax
        add esp, 0x8
        cmp edi, ebx
        jnz L_430350
        mov ebp, dword ptr [esp + 0x14]
    L_43039d:
        push ebp
        call dword ptr [g_Iat_free_004cc5b4]
        mov ecx, dword ptr [esi + 0x20]
        add esp, 0x4
        mov edx, 0xe00
        push offset g_Data_004da000 + 0x2d28
        call SetGlobals_0053a2f0
        push offset g_Data_004da000 + 0x2d20
        push 0x4
        push offset g_Data_004da000 + 0x2d20
        call dword ptr [g_Iat_MFC42_1146_004cc1b8]
        push eax
        call dword ptr [g_Iat_LoadMenuA_004cc644]
        lea edi, [esi + 0x1d0]
        push eax
        mov ecx, edi
        call dword ptr [g_Iat_MFC42_1644_004cc218]
        cmp edi, ebx
        jnz L_4303e7
        xor edi, edi
        jmp L_4303ea
    L_4303e7:
        mov edi, dword ptr [edi + 0x4]
    L_4303ea:
        mov ecx, dword ptr [esi + 0x20]
        push edi
        push ecx
        call dword ptr [g_Iat_SetMenu_004cc610]
        mov eax, dword ptr [esi + 0x22c]
        mov ebp, dword ptr [g_Iat_GetSubMenu_004cc614]
        cmp eax, ebx
        jnz L_43041b
        mov edx, dword ptr [esi + 0x1d4]
        mov edi, dword ptr [g_Iat_RemoveMenu_004cc618]
        push 0x400
        push 0x1
        push edx
        jmp L_430459
    L_43041b:
        mov eax, dword ptr [esi + 0x1d4]
        push 0x1
        push eax
        call ebp
        push eax
        call dword ptr [g_Iat_MFC42_2863_004cc214]
        mov ecx, dword ptr [eax + 0x4]
        mov edi, dword ptr [g_Iat_RemoveMenu_004cc618]
        push ebx
        push 0x9c6b
        push ecx
        call edi
        mov edx, dword ptr [esi + 0x1d4]
        push 0x1
        push edx
        call ebp
        push eax
        call dword ptr [g_Iat_MFC42_2863_004cc214]
        mov eax, dword ptr [eax + 0x4]
        push ebx
        push 0x9c7b
        push eax
    L_430459:
        call edi
        mov ecx, dword ptr [esi + 0x1d4]
        push 0x2
        push ecx
        call ebp
        push eax
        call dword ptr [g_Iat_MFC42_2863_004cc214]
        mov edx, dword ptr [eax + 0x4]
        push ebx
        push 0x9c4e
        push edx
        call edi
        mov ecx, dword ptr [g_Data_004da000 + 0x19d14]
        mov eax, dword ptr [esi + 0x20]
        mov dword ptr [g_Data_004da000 + 0x19ef8], ecx
        lea ecx, [esp + 0x10]
        mov dword ptr [g_Data_004da000 + 0x19eec], eax
        call dword ptr [g_Iat_MFC42_540_004cc3c0]
        lea edx, [esp + 0x14]
        mov ecx, esi
        push edx
        mov byte ptr [esp + 0x2c], 0x3
        call MainWnd_GetTitle
        mov eax, dword ptr [eax]
        lea ecx, [esp + 0x10]
        push eax
        push offset g_Data_004da000 + 0x97c
        push ecx
        mov byte ptr [esp + 0x34], 0x4
        call dword ptr [g_Iat_MFC42_2818_004cc3b0]
        add esp, 0xc
        lea ecx, [esp + 0x14]
        mov byte ptr [esp + 0x28], 0x3
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, esi
        push edx
        call dword ptr [g_Iat_MFC42_6199_004cc32c]
        mov edi, 0x1
        mov byte ptr [esi + 0xcc], BL
        mov dword ptr [esi + 0x1d8], edi
        mov dword ptr [esi + 0x1fc], ebx
        mov dword ptr [esi + 0x200], ebx
        mov dword ptr [esi + 0x204], ebx
        mov dword ptr [esi + 0x208], ebx
        mov dword ptr [esi + 0x20c], 0x9c83
        mov dword ptr [esi + 0x210], 0x9c72
        mov dword ptr [esi + 0x214], 0x9c75
        mov dword ptr [esi + 0x218], 0x9c76
        call Texture_GetFlag_004e073c
        test eax, eax
        jz L_430544
        mov eax, dword ptr [esi + 0x1d4]
        push 0x8
        push 0x9c7b
        push eax
        jmp L_430551
    L_430544:
        mov ecx, dword ptr [esi + 0x1d4]
        push ebx
        push 0x9c7b
        push ecx
    L_430551:
        call dword ptr [g_Iat_CheckMenuItem_004cc61c]
        mov edx, dword ptr [esi + 0x1d8]
        mov dword ptr [g_Data_004da000 + 0x16da0], edx
        mov ecx, dword ptr [esi + 0x1d8]
        call SoundArchive_SetEnabled
        call zVideo_GetGlobal_00632f9c
        mov dword ptr [esi + 0x1f8], eax
        lea eax, [esp + 0x18]
        push eax
        push 0x20019
        push ebx
        push offset g_Data_004da000 + 0x2d00
        push 0x80000002
        call dword ptr [g_Iat_RegOpenKeyExA_004cc00c]
        test eax, eax
        jnz L_4305a9
        mov ecx, dword ptr [esp + 0x18]
        mov dword ptr [g_Data_004da000 + 0x19efc], edi
        push ecx
        call dword ptr [g_Iat_RegCloseKey_004cc004]
    L_4305a9:
        push ebx
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_1768_004cc210]
        push 0x7f00
        push ebx
        call dword ptr [g_Iat_LoadCursorA_004cc60c]
        push eax
        call dword ptr [g_Iat_SetCursor_004cc620]
        lea ecx, [esp + 0x10]
        mov byte ptr [esp + 0x28], 0x1
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        mov ecx, dword ptr [esp + 0x20]
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x1c
        ret
    }
}

// 0x00431610 MainWnd_SelectDriverWindowedDefault - bytes ret 4: zVideo_SelectDriver; Settings_StoreHWAPI; [0x0056bc2c](this+0x228); +0x220=Settings_GetFullScreen; Settings_StoreFullScreen; Settings_ApplyHWCardFlag; +0x21c=Setting_Get_DisplayMode; MainWnd_OnVideoMode5
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall MainWnd_SelectDriverWindowedDefault(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        mov ecx, edi
        call zVideo_SelectDriver
        mov ecx, eax
        call Settings_StoreHWAPI
        lea eax, [esi + 0x228]
        lea edx, [esi + 0x224]
        push eax
        mov ecx, edi
        call dword ptr [g_Data_004da000 + 0x91c2c]
        call Settings_GetFullScreen
        mov ecx, 0x1
        mov dword ptr [esi + 0x220], eax
        call Settings_StoreFullScreen
        mov ecx, 0x1
        call Settings_ApplyHWCardFlag
        call Setting_Get_DisplayMode
        mov ecx, esi
        mov dword ptr [esi + 0x21c], eax
        call MainWnd_OnVideoMode5
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431680 MainWnd_SelectDriverAndMode - bytes: zVideo_SelectDriver; Settings_StoreHWAPI; Settings_ApplyHWCardFlag; Settings_StoreFullScreen; Settings_ApplyVideoModePreset; MainWnd_EnableVideoModes
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_SelectDriverAndMode(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        or ecx, 0xffffffff
        call zVideo_SelectDriver
        mov ecx, eax
        call Settings_StoreHWAPI
        xor ecx, ecx
        call Settings_ApplyHWCardFlag
        mov ecx, dword ptr [esi + 0x220]
        call Settings_StoreFullScreen
        mov ecx, dword ptr [esi + 0x21c]
        call Settings_ApplyVideoModePreset
        mov ecx, esi
        call MainWnd_EnableVideoModes
        pop esi
        ret
    }
}

// 0x004316c0 Screen_SelectExclusiveState_004316c0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Screen_SelectExclusiveState_004316c0(int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0xc]
        mov esi, ecx
        mov eax, dword ptr [esi + edi*0x4 + 0x1fc]
        test eax, eax
        jnz L_43171d
        test edi, edi
        jz L_4316df
        lea ecx, [edi - 0x1]
        call ZVideoTable_ElementAddr_00633e78_004a7430
    L_4316df:
        test edi, edi
        mov dword ptr [esi + edi*0x4 + 0x1fc], 0x8
        jnz L_4316f7
        mov ecx, esi
        call MainWnd_SelectDriverAndMode
        jmp L_431702
    L_4316f7:
        lea eax, [edi - 0x1]
        mov ecx, esi
        push eax
        call MainWnd_SelectDriverWindowedDefault
    L_431702:
        xor eax, eax
        lea ecx, [esi + 0x1fc]
    L_43170a:
        cmp eax, edi
        jz L_431714
        mov dword ptr [ecx], 0x0
    L_431714:
        inc eax
        add ecx, 0x4
        cmp eax, 0x4
        jl L_43170a
    L_43171d:
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431730 MainWnd_InitDisplayModeList - if windowed-capable (0x00408320) and mode count (0x004a7480)>0: [+0x1fc]=0, slot[count]=8, 0x00431610(count-1); else [+0x1fc]=8, [+0x21c]=5, [+0x220]=0x00408330(); 0x00431680
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall MainWnd_InitDisplayModeList(int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call Settings_GetHWAPI
        test eax, eax
        jz L_431765
        call zVideo_GetDriverCount
        test eax, eax
        jz L_431765
        mov dword ptr [esi + 0x1fc], 0x0
        mov dword ptr [esi + eax*0x4 + 0x1fc], 0x8
        dec eax
        mov ecx, esi
        push eax
        call MainWnd_SelectDriverWindowedDefault
        pop esi
        ret
    L_431765:
        mov dword ptr [esi + 0x1fc], 0x8
        mov dword ptr [esi + 0x21c], 0x5
        call Settings_GetFullScreen
        mov ecx, esi
        mov dword ptr [esi + 0x220], eax
        call MainWnd_SelectDriverAndMode
        pop esi
        ret
    }
}

// 0x00431790 Slot_Call_004316c0_Arg0_00431790 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_Call_004316c0_Arg0_00431790(int, int)
{
    __asm {
        push 0x0
        call Screen_SelectExclusiveState_004316c0
        ret
    }
}

// 0x004317a0 Slot_Call_004316c0_Arg1_004317a0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_Call_004316c0_Arg1_004317a0(int, int)
{
    __asm {
        push 0x1
        call Screen_SelectExclusiveState_004316c0
        ret
    }
}

// 0x004317b0 Slot_Call_004316c0_Arg2_004317b0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_Call_004316c0_Arg2_004317b0(int, int)
{
    __asm {
        push 0x2
        call Screen_SelectExclusiveState_004316c0
        ret
    }
}

// 0x004317c0 Slot_Call_004316c0_Arg3_004317c0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_Call_004316c0_Arg3_004317c0(int, int)
{
    __asm {
        push 0x3
        call Screen_SelectExclusiveState_004316c0
        ret
    }
}

// 0x00443730 GameFrame_New - bytes: SEH; operator new(0xcc); GameFrame_Ctor(class name 'gamez' 0x004dd8e8)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall GameFrame_New(int, int)
{
    __asm {
        push -0x1
        push offset EH_Handler_GameFrame_New
        mov eax, FS:[0x0]
        push eax
        mov dword ptr FS:[0x0], esp
        push ecx
        push 0xcc
        call dword ptr [g_Iat_MFC42_823_004cc2ac]
        add esp, 0x4
        mov dword ptr [esp], eax
        test eax, eax
        mov dword ptr [esp + 0xc], 0x0
        jz L_44377e
        push offset g_Data_004da000 + 0x38e8
        mov ecx, eax
        call GameFrame_Ctor
        mov ecx, dword ptr [esp + 0x4]
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    L_44377e:
        mov ecx, dword ptr [esp + 0x4]
        xor eax, eax
        mov dword ptr FS:[0x0], ecx
        add esp, 0x10
        ret
    }
}

// 0x004437d0 GameFrame_Ctor - bytes ret 4: CFrameWnd ctor; +0xc8=0; CBrush member +0xc4 vtbl 0x004d22e0; vtbl 0x004d21d8; 0x004a5780; Render_BringUpDirectDraw
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall GameFrame_Ctor(int, int, int)
{
    __asm {
        push esi
        mov esi, ecx
        call dword ptr [g_Iat_MFC42_366_004cc268]
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [esi + 0xc8], 0x0
        mov dword ptr [esi + 0xc4], offset g_RData_004cc000 + 0x62e0
        mov dword ptr [esi], offset g_RData_004cc000 + 0x61d8
        call App_RedirectStdioToLogs
        call Render_BringUpDirectDraw
        mov eax, esi
        pop esi
        ret 0x4
    }
}

// 0x0042f8a0 DataTarget_0042f8a0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall DataTarget_0042f8a0(int, int, int)
{
    __asm {
        call Setting_Get_CDAudio
        test eax, eax
        jz L_42f8d5
        push esi
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_GetOutcome
        mov esi, eax
        call SoundCD_GetTrackCount
        mov ecx, eax
        mov eax, esi
        cdq
        sub ecx, 0x2
        idiv ecx
        mov ecx, edx
        mov edx, 0x5
        add ecx, 0x2
        call SoundCD_PlayIfPrepared
        pop esi
    L_42f8d5:
        ret 0x4
    }
}

// 0x004306e0 DataTarget_004306e0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_004306e0(int, int)
{
    __asm {
        mov eax, offset g_RData_004cc000 + 0x4c08
        ret
    }
}

// 0x00430a90 DataTarget_00430a90 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall DataTarget_00430a90(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        push 0x1
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax]
        mov edi, dword ptr [esi]
        call Settings_GetSplitScreenValue
        push eax
        mov ecx, esi
        call dword ptr [edi + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00430ab0 DataTarget_00430ab0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_00430ab0(int, int)
{
    __asm {
        call Settings_GetFullScreen
        test eax, eax
        jz L_430ac0
        xor ecx, ecx
        jmp Settings_StoreFullScreen
    L_430ac0:
        mov ecx, 0x1
        jmp Settings_StoreFullScreen
    }
}

// 0x00431920 DataTarget_00431920 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall DataTarget_00431920(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        push 0x1
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax]
        mov edi, dword ptr [esi]
        call Setting_Get_CDAudio
        neg eax
        sbb eax, eax
        mov ecx, esi
        neg eax
        push eax
        call dword ptr [edi + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431970 DataTarget_00431970 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall DataTarget_00431970(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        push 0x1
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax]
        mov edi, dword ptr [esi]
        call Setting_Get_004e5d60
        neg eax
        sbb eax, eax
        mov ecx, esi
        neg eax
        push eax
        call dword ptr [edi + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431aa0 DataTarget_00431aa0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall DataTarget_00431aa0(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        push 0x1
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax]
        mov edi, dword ptr [esi]
        call RecoilApp_GetSoundAPICheckboxValue
        neg eax
        sbb eax, eax
        mov ecx, esi
        inc eax
        push eax
        call dword ptr [edi + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00430ad0 DataTarget_00430ad0 - launcher ON_COMMAND id 106 (message map entry 0x004d0d00): Help > Docs.
// CONFIRMED-BINARY: bytes 0x00430ad0..0x00430bee read 2026-10-07, including the switch cases the Ghidra listing missed
// (0x00430b38..0x00430bc7) and its tables: byte table 0x00430c04 = {0,4,1,1,4x7,2,4x19,3}, dword table 0x00430bf0 =
// {430b38, 430b80, 430ba4, 430b5c, 430bc8}. r = FindExecutableA("Docs\Index.html" 0x004db5dc, NULL, buf); r > 31 or an
// unlisted value -> ShellExecuteA([0x004f3eec], "open" 0x004db5d4, "Docs\Index.html", 0, 0, 0); r = 0 -> message 0x20,
// 2/3 -> 0x22, 11 -> 0x24, 31 -> 0x21, each via Message_GetText + CWnd::MessageBox (MFC42 #4224, MB_ICONEXCLAMATION 0x30).
// The jump table is written as a compare chain on r (same targets).
__declspec(naked) int __fastcall DataTarget_00430ad0(int, int)
{
    __asm {
        sub esp, 0x180
        lea eax, [esp + 0x80]
        push ebx
        push ebp
        push esi
        push edi
        push eax
        push 0x0
        mov ebp, ecx
        push offset g_Data_004da000 + 0x15dc
        call dword ptr [g_Iat_FindExecutableA_004cc600]
        mov ecx, 0x19
        mov ebx, eax
        call Message_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x10]
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
        cmp ebx, 0x1f
        rep movsb
        ja L_430bc8
        cmp ebx, 0
        je L_430b38
        cmp ebx, 2
        je L_430b80
        cmp ebx, 3
        je L_430b80
        cmp ebx, 11
        je L_430ba4
        cmp ebx, 31
        je L_430b5c
        jmp L_430bc8
    L_430b38:
        lea edx, [esp + 0x10]
        push 0x30
        push edx
        mov ecx, 0x20
        call Message_GetText
        push eax
        mov ecx, ebp
        call dword ptr [g_Iat_MFC42_4224_004cc318]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x180
        ret
    L_430b5c:
        lea eax, [esp + 0x10]
        push 0x30
        push eax
        mov ecx, 0x21
        call Message_GetText
        push eax
        mov ecx, ebp
        call dword ptr [g_Iat_MFC42_4224_004cc318]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x180
        ret
    L_430b80:
        lea ecx, [esp + 0x10]
        push 0x30
        push ecx
        mov ecx, 0x22
        call Message_GetText
        push eax
        mov ecx, ebp
        call dword ptr [g_Iat_MFC42_4224_004cc318]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x180
        ret
    L_430ba4:
        lea edx, [esp + 0x10]
        push 0x30
        push edx
        mov ecx, 0x24
        call Message_GetText
        push eax
        mov ecx, ebp
        call dword ptr [g_Iat_MFC42_4224_004cc318]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x180
        ret
    L_430bc8:
        mov eax, dword ptr [g_Data_004da000 + 0x19eec]
        push 0x0
        push 0x0
        push 0x0
        push offset g_Data_004da000 + 0x15dc
        push offset g_Data_004da000 + 0x15d4
        push eax
        call dword ptr [g_Iat_ShellExecuteA_004cc604]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x180
        ret
    }
}

// 0x00431ae0 DataTarget_00431ae0 - ON_UPDATE_COMMAND_UI for launcher menu id 40066 "A3D" (message map entry 0x004d10d8).
// CONFIRMED-BINARY: listing 0x00431ae0..0x00431b03 (20 instructions). pCmdUI->Enable(TRUE) (vtable +0);
// pCmdUI->SetCheck(Sound_GetAPIMode() == 1) (vtable +4). It was missing from the port and the address table, so MFC
// called the original VA inside the remake and aborted when the Options menu opened (2026-10-07).
__declspec(naked) int __fastcall DataTarget_00431ae0(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 0x8]
        push edi
        push 0x1
        mov eax, dword ptr [esi]
        mov ecx, esi
        call dword ptr [eax]
        mov edi, dword ptr [esi]
        call Sound_GetAPIMode
        dec eax
        mov ecx, esi
        neg eax
        sbb eax, eax
        inc eax
        push eax
        call dword ptr [edi + 0x4]
        pop edi
        pop esi
        ret 0x4
    }
}

// 0x00431b10 Slot_Forward443a20_ThenVirtualA8_00431b10 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall Slot_Forward443a20_ThenVirtualA8_00431b10(int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0xc]
        push esi
        mov esi, ecx
        push edi
        mov ecx, dword ptr [esp + 0x10]
        mov edi, dword ptr [esp + 0xc]
        push eax
        push ecx
        push edi
        mov ecx, esi
        call DataTarget_00443a20
        cmp edi, 0x4
        jz L_431b34
        cmp edi, 0x1
        jnz L_431b42
    L_431b34:
        mov ecx, dword ptr [esi + 0xc0]
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0xa8]
    L_431b42:
        pop edi
        pop esi
        ret 0xc
    }
}

// 0x004420e0 DataTarget_004420e0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_004420e0(int, int)
{
    __asm {
        mov edx, dword ptr [ecx + 0xe8]
        xor eax, eax
        test edx, edx
        setz AL
        mov dword ptr [ecx + 0xe8], eax
        ret
    }
}

// 0x00443a20 DataTarget_00443a20 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall DataTarget_00443a20(int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0xc]
        mov edx, dword ptr [esp + 0x8]
        push eax
        mov eax, dword ptr [esp + 0x8]
        push edx
        push eax
        call dword ptr [g_Iat_MFC42_5030_004cc284]
        call Window_CacheClientRectScreen_00443a40
        ret 0xc
    }
}

// 0x004c62a2 DataTarget_004c62a2 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_004c62a2(int, int)
{
    __asm {
        mov eax, dword ptr [ebp - 0x14]
        mov ecx, dword ptr [eax]
        mov ecx, dword ptr [ecx]
        mov dword ptr [ebp - 0x78], ecx
        push eax
        push ecx
        call dword ptr [g_Iat__XcptFilter_004cc568]
        add esp, 0x8
        ret
    }
}

// 0x00443ae0 MainWnd_OnActivate_00443ae0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall MainWnd_OnActivate_00443ae0(int, int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0xc]
        push esi
        mov esi, ecx
        push edi
        mov ecx, dword ptr [esp + 0x10]
        mov edi, dword ptr [esp + 0xc]
        push eax
        push ecx
        push edi
        mov ecx, esi
        call dword ptr [g_Iat_MFC42_4337_004cc294]
        mov ecx, dword ptr [esi + 0xc0]
        call ScreenManager_GetCurrent
        test eax, eax
        jz L_443b11
        mov edx, dword ptr [eax]
        push edi
        mov ecx, eax
        call dword ptr [edx + 0x4]
    L_443b11:
        mov ecx, dword ptr [esi + 0xc0]
        test edi, edi
        jnz L_443b32
        mov eax, dword ptr [ecx]
        call dword ptr [eax + 0xa8]
        call Input_SetAllDeviceBit1AndAcquireMouse_00471ae0
        call Stub_Ret
        pop edi
        pop esi
        ret 0xc
    L_443b32:
        mov edx, dword ptr [ecx]
        call dword ptr [edx + 0xa4]
        call Input_ResumeOnActivate
        call zVideo_RestoreIfMinimised_004a7770
        pop edi
        pop esi
        ret 0xc
    }
}

// 0x0042f280 App_RenderFrameAndPresent - (resetOnly) ret 4: arg!=0: 0x004a56d0, stop DoT (0x0045d6b0), viewport/screen settings (0x004085a0, 0x00408650, 0x004086c0), 0x0044ea70, return 1. Else per-frame: 0x0049f620 sound update; clear 0x004a7b30; 0x00410140; hardware vs software paths: begin scene (0x004a7b20 / 0x004a6830 / 0x004a6760), vi
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall App_RenderFrameAndPresent(int, int, int)
{
    __asm {
        sub esp, 0xc
        push ebx
        push esi
        push edi
        mov esi, ecx
        call Timer_UpdateFrame
        mov eax, dword ptr [g_Data_004da000 + 0x19a90]
        test eax, eax
        jz L_42f2b8
        xor ecx, ecx
        call Input_Keyboard_WaitForKeyPress
        test eax, eax
        jz L_42f2b8
        mov ecx, dword ptr [g_Data_004da000 + 0x19a90]
        xor edx, edx
        mov dword ptr [g_Data_004da000 + 0x19a90], 0x0
        call Wrap_0045d570_Arg1
    L_42f2b8:
        mov CL, 0x1
        call Input_PollAllDevices
        call Setting_Get_ViewportRect
        mov dword ptr [esi + 0xc], eax
        call Setting_Get_ScreenRect
        mov dword ptr [esi + 0x8], eax
        call Settings_GetValue_004e5d88
        mov dword ptr [esi + 0x4], eax
        call NodeList_RunAll
        mov eax, dword ptr [g_Data_004da000 + 0xbdec]
        test eax, eax
        jz L_42f2f3
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0xc
        ret 0x4
    L_42f2f3:
        xor ecx, ecx
        call Sound_UpdateFrame
        mov eax, dword ptr [g_Data_004da000 + 0x19768]
        test eax, eax
        jz L_42f335
        mov eax, dword ptr [g_Data_004da000 + 0x196c0]
        test eax, eax
        jz L_42f335
        mov ecx, dword ptr [g_Data_004da000 + 0x196bc]
        lea edx, [esp + 0xc]
        call gwNodeGetWorldPosition
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x10]
        mov edx, dword ptr [esp + 0xc]
        push eax
        push ecx
        mov ecx, dword ptr [g_Data_004da000 + 0x196c0]
        push edx
        call Object3D_SetPosition
    L_42f335:
        call zVideo_IsD3DDeviceCreated
        mov ebx, eax
        call Hud_ConsumeCounter_004e61bc
        mov edi, eax
        mov ecx, edi
        or ecx, ebx
        call zVideo_SetD3DDeviceCreated
        test edi, edi
        mov ebx, eax
        jz L_42f357
        mov edi, dword ptr [esi + 0x4]
        jmp L_42f35a
    L_42f357:
        mov edi, dword ptr [esi + 0xc]
    L_42f35a:
        call Settings_GetHWCardFlag
        test eax, eax
        jz L_42f36f
        mov edx, dword ptr [esi + 0x4]
        mov ecx, edi
        call zVideo_CallDispatch_006333cc
        jmp L_42f376
    L_42f36f:
        mov ecx, edi
        call zVideo_CallSurfaceSlot3C8
    L_42f376:
        mov ecx, ebx
        call zVideo_SetD3DDeviceCreated
        call Setting_Get_004e5d6c
        test eax, eax
        jz L_42f39c
        call zVideo_GetGlobal_00632208
        push eax
        call Settings_GetDisplayRect_20
        mov edi, dword ptr [esi + 0xc]
        push eax
        call zVideo_GetGlobal_00632210
        jmp L_42f3b0
    L_42f39c:
        call zVideo_GetGlobal_00632228
        push eax
        call Settings_GetDisplayRect_20
        mov edi, dword ptr [esi + 0xc]
        push eax
        call zVideo_GetGlobal_00632230
    L_42f3b0:
        mov ecx, eax
        mov edx, edi
        call zRndr_SetFramebuffer
        call Render_DispatchCameraList
        mov edx, dword ptr [esi + 0xc]
        xor ecx, ecx
        call GlobalCompositePanel_SetSlot
        mov ecx, offset g_Data_004da000 + 0x19c98
        call Hud_GetRect_004edb58
        call Setting_Get_004e5d6c
        test eax, eax
        jz L_42f41f
        mov eax, dword ptr [g_Data_004da000 + 0x19c9c]
        cdq
        sub eax, edx
        mov edi, eax
        mov eax, dword ptr [g_Data_004da000 + 0x19ca4]
        cdq
        sub eax, edx
        mov ecx, eax
        mov eax, dword ptr [g_Data_004da000 + 0x19c98]
        cdq
        sub eax, edx
        sar eax, 0x1
        mov dword ptr [g_Data_004da000 + 0x19c98], eax
        mov eax, dword ptr [g_Data_004da000 + 0x19ca0]
        cdq
        sub eax, edx
        sar edi, 0x1
        sar ecx, 0x1
        sar eax, 0x1
        mov dword ptr [g_Data_004da000 + 0x19c9c], edi
        mov dword ptr [g_Data_004da000 + 0x19ca4], ecx
        mov dword ptr [g_Data_004da000 + 0x19ca0], eax
        jmp L_42f42b
    L_42f41f:
        mov ecx, dword ptr [g_Data_004da000 + 0x19ca4]
        mov edi, dword ptr [g_Data_004da000 + 0x19c9c]
    L_42f42b:
        mov eax, dword ptr [esi + 0xc]
        mov eax, dword ptr [eax + 0xc]
        cmp ecx, eax
        jle L_42f445
        cmp edi, eax
        jge L_42f43e
        mov dword ptr [g_Data_004da000 + 0x19c9c], eax
    L_42f43e:
        mov edx, offset g_Data_004da000 + 0x19c98
        jmp L_42f447
    L_42f445:
        xor edx, edx
    L_42f447:
        mov ecx, 0x1
        call GlobalCompositePanel_SetSlot
        mov ecx, 0x1
        call Input_Keyboard_ConsumeKeyEdge
        mov edi, eax
        and edi, 0x3
        call Settings_GetHWCardFlag
        mov ebx, dword ptr [esp + 0x1c]
        test eax, eax
        jz L_42f4d3
        call PointQueue_Clear
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_TickAndCheckObjectiveCompletion
        mov ecx, offset g_Data_004da000 + 0x19ab0
        call Hud_UpdateTargetMarkers
        call zVideo_NotifyScreenResize
        mov ecx, dword ptr [g_Data_004da000 + 0x91424]
        push ecx
        call Manager_0056bd58_Tick
        test edi, edi
        jz L_42f4ae
        call zVideo_CallSurfaceSlot3C0_Back
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0xc
        ret 0x4
    L_42f4ae:
        mov ecx, dword ptr [esi + 0x4]
        call Render_SetViewportSizeFromRect
        call Hud_Tick
        call Settings_GetNetworkFlag
        test eax, eax
        jz L_42f4c9
        call NetExitHost_Call0
    L_42f4c9:
        call zVideo_CallSurfaceSlot3C0_Back
        jmp L_42f5b7
    L_42f4d3:
        call Setting_Get_004e5d6c
        test eax, eax
        jz L_42f543
        call zVideo_NotifyScreenResize
        mov edx, dword ptr [g_Data_004da000 + 0x91424]
        push edx
        call Manager_0056bd58_Tick
        call zVideo_CallSurfaceSlot3C0_Back
        test ebx, ebx
        jz L_42f502
        mov edx, dword ptr [esi + 0x8]
        mov ecx, dword ptr [esi + 0xc]
        call dword ptr [g_Data_004da000 + 0x91bfc]
    L_42f502:
        call zVideo_NotifyScreenResizeFromSurface
        test edi, edi
        jnz L_42f557
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_TickAndCheckObjectiveCompletion
        mov ecx, dword ptr [esi + 0x4]
        call Render_SetViewportSizeFromRect
        xor ecx, ecx
        push 0x40000000
        call PointQueue_FlushAll
        mov ecx, offset g_Data_004da000 + 0x19ab0
        call Hud_UpdateTargetMarkers
        call Hud_Tick
        call Settings_GetNetworkFlag
        test eax, eax
        jz L_42f5b2
        jmp L_42f5ad
    L_42f543:
        call zVideo_NotifyScreenResizeFromSurface
        mov eax, dword ptr [g_Data_004da000 + 0x91424]
        push eax
        call Manager_0056bd58_Tick
        test edi, edi
        jz L_42f577
    L_42f557:
        call zVideo_CallSurfaceSlot3C0_Front
        push 0x1
        push 0x0
        xor edx, edx
        xor ecx, ecx
        call zVideo_PresentFrame
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0xc
        ret 0x4
    L_42f577:
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_TickAndCheckObjectiveCompletion
        mov ecx, dword ptr [esi + 0x4]
        call Render_SetViewportSizeFromRect
        xor ecx, ecx
        push 0x3f800000
        call PointQueue_FlushAll
        mov ecx, offset g_Data_004da000 + 0x19ab0
        call Hud_UpdateTargetMarkers
        call Hud_Tick
        call Settings_GetNetworkFlag
        test eax, eax
        jz L_42f5b2
    L_42f5ad:
        call NetExitHost_Call0
    L_42f5b2:
        call zVideo_CallSurfaceSlot3C0_Front
    L_42f5b7:
        test ebx, ebx
        jz L_42f5c9
        mov ecx, dword ptr [esi + 0x4]
        push 0x0
        push 0x0
        mov edx, ecx
        call zVideo_PresentFrame
    L_42f5c9:
        pop edi
        pop esi
        xor eax, eax
        pop ebx
        add esp, 0xc
        ret 0x4
    }
}

// 0x0042f5e0 Game_EndSequenceTick - bytes: timer 0x004f3df8: <=0 and won 0x004e5dec -> Sound stop, play FMV 'fmv_zrd'/'GRANDPRIZE' (0x004625e0), 0x00462f50(0), renderer hook, 0x00463850(0xc,1), 0x00462630; else 0x004dd1c0 flag; fade: timer -= clock 0x0056b424 with Screen_SetFadeFill/zVideo present while <=1.0; player +0x5a0 set during
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Game_EndSequenceTick(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        and esp, 0xfffffff8
        fld dword ptr [g_Data_004da000 + 0x19df8]
        mov eax, FS:[0x0]
        push -0x1
        fcomp qword ptr [g_RData_004cc000 + 0x49b8]
        push offset EH_Handler_Game_EndSequenceTick
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x60
        fnstsw AX
        push esi
        push edi
        test AH, 0x41
        mov esi, ecx
        jnz L_42f72a
        push 0x0
        mov dword ptr [g_Data_004da000 + 0x31c0], 0x0
        call App_RenderFrameAndPresent
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fcomp qword ptr [g_RData_004cc000 + 0x49d0]
        fnstsw AX
        test AH, 0x41
        jnz L_42f685
        mov ecx, 0x1
        call zVideo_SetD3DDeviceCreated
        mov edi, eax
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x5a0], 0x1
        call Settings_GetHWCardFlag
        mov ecx, dword ptr [esi + 0x4]
        test eax, eax
        jz L_42f677
        mov edx, ecx
        call zVideo_CallDispatch_006333cc
        mov ecx, edi
        call zVideo_SetD3DDeviceCreated
        jmp L_42f6c7
    L_42f677:
        call zVideo_CallSurfaceSlot3C8
        mov ecx, edi
        call zVideo_SetD3DDeviceCreated
        jmp L_42f6c7
    L_42f685:
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fcomp qword ptr [g_RData_004cc000 + 0x49b8]
        fnstsw AX
        test AH, 0x41
        jnz L_42f6a4
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fstp qword ptr [esp + 0xc]
        jmp L_42f6b4
    L_42f6a4:
        mov dword ptr [esp + 0xc], 0x0
        mov dword ptr [esp + 0x10], 0x0
    L_42f6b4:
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0xc]
        push eax
        push ecx
        xor edx, edx
        xor ecx, ecx
        call Screen_SetFadeFill
    L_42f6c7:
        mov ecx, dword ptr [esi + 0x4]
        push 0x0
        push 0x0
        mov edx, ecx
        call zVideo_PresentFrame
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fsub dword ptr [g_Data_004da000 + 0x91424]
        fst dword ptr [g_Data_004da000 + 0x19df8]
        fcomp qword ptr [g_RData_004cc000 + 0x49b8]
        fnstsw AX
        test AH, 0x41
        jz L_42f86e
        xor ecx, ecx
        call Settings_ApplyMuteSound
        call CallGlobal004e5ee8Slot18
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        mov eax, dword ptr [edx + 0x4]
        mov dword ptr [eax + 0x5a0], 0x0
        xor eax, eax
        mov ecx, dword ptr [esp + 0x68]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    L_42f72a:
        mov eax, dword ptr [g_Data_004da000 + 0xbdec]
        test eax, eax
        jz L_42f81b
        call Sound_BuildPlayingVoiceList
        mov ecx, eax
        call SoundList_StopPlayingVoices
        call SoundCD_Stop
        push 0x0
        push offset g_Data_004da000 + 0x2cb0
        push offset g_Data_004da000 + 0x2bd0
        lea ecx, [esp + 0x24]
        call Seq_Ctor
        push 0x0
        lea ecx, [esp + 0x1c]
        mov dword ptr [esp + 0x74], 0x0
        call Seq_Play
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        test eax, eax
        jz L_42f783
        xor edx, edx
        xor ecx, ecx
        call dword ptr [g_Data_004da000 + 0x91bfc]
    L_42f783:
        xor ecx, ecx
        call zVideo_SetFlag0063212c
        xor ecx, ecx
        call Widget_SetGlobalDirtyMode
        push 0x1
        push 0xc
        lea ecx, [esp + 0x40]
        call SeqBlur_Ctor
        mov edx, dword ptr [esp + 0x38]
        push 0x0
        push 0x0
        lea ecx, [esp + 0x40]
        mov byte ptr [esp + 0x78], 0x1
        call dword ptr [edx + 0x8]
        mov eax, dword ptr [esp + 0x38]
        push 0x0
        push 0x0
        lea ecx, [esp + 0x40]
        call dword ptr [eax + 0x4]
        test eax, eax
        jz L_42f7d8
    L_42f7c5:
        mov edx, dword ptr [esp + 0x38]
        push 0x0
        push 0x0
        lea ecx, [esp + 0x40]
        call dword ptr [edx + 0x4]
        test eax, eax
        jnz L_42f7c5
    L_42f7d8:
        mov eax, dword ptr [esp + 0x38]
        lea ecx, [esp + 0x38]
        call dword ptr [eax + 0xc]
        xor ecx, ecx
        call MapScreen_SetActiveMarker
        call Screen_Enter_004e5de0
        lea ecx, [esp + 0x18]
        mov dword ptr [esp + 0x38], offset g_RData_004cc000 + 0x2e50
        mov dword ptr [esp + 0x70], 0xffffffff
        call Seq_Dtor
        xor eax, eax
        mov ecx, dword ptr [esp + 0x68]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    L_42f81b:
        push 0x1
        mov ecx, esi
        mov dword ptr [g_Data_004da000 + 0x31c0], 0x1
        call App_RenderFrameAndPresent
        test eax, eax
        jz L_42f86e
        call Settings_GetNetworkFlag
        test eax, eax
        jz L_42f853
        call NetExitHost_Show
        xor eax, eax
        mov ecx, dword ptr [esp + 0x68]
        mov dword ptr FS:[0x0], ecx
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    L_42f853:
        mov ecx, dword ptr [esi + 0x4]
        call Render_SetViewportSizeFromRect
        mov eax, dword ptr [g_Data_004da000 + 0xbdec]
        test eax, eax
        jnz L_42f86e
        mov ecx, 0x1
        call MapScreen_SetActiveMarker
    L_42f86e:
        mov ecx, dword ptr [esp + 0x68]
        pop edi
        pop esi
        xor eax, eax
        mov esp, ebp
        mov dword ptr FS:[0x0], ecx
        pop ebp
        ret
    }
}

// 0x0048ea20 Unnamed_0048ea20 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Unnamed_0048ea20(int, int)
{
    __asm {
        sub esp, 0x28
        push ebx
        push ebp
        push esi
        xor esi, esi
        cmp ecx, esi
        push edi
        jz L_48ea4a
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [ecx + 0x4]
        mov dword ptr [esp + 0x28], eax
        mov eax, dword ptr [ecx + 0x8]
        mov ecx, dword ptr [ecx + 0xc]
        mov dword ptr [esp + 0x2c], edx
        mov dword ptr [esp + 0x30], eax
        mov dword ptr [esp + 0x34], ecx
        jmp L_48ea67
    L_48ea4a:
        mov edx, dword ptr [g_Data_004da000 + 0x911cc]
        mov eax, dword ptr [g_Data_004da000 + 0x911c8]
        dec edx
        dec eax
        mov dword ptr [esp + 0x2c], esi
        mov dword ptr [esp + 0x28], esi
        mov dword ptr [esp + 0x34], edx
        mov dword ptr [esp + 0x30], eax
    L_48ea67:
        lea ecx, [esp + 0x18]
        lea edx, [esp + 0x10]
        push ecx
        lea ecx, [esp + 0x18]
        call Pixfmt_GetChannelMasks
        cmp dword ptr [g_Data_004da000 + 0x91be8], esi
        jz L_48eaa0
        mov ecx, dword ptr [esp + 0x18]
        push 0x3fd33333
        lea edx, [esp + 0x2c]
        push 0x33333333
        call Render_QueueScreenRect
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x28
        ret
    L_48eaa0:
        mov esi, dword ptr [esp + 0x10]
        mov edi, dword ptr [esp + 0x14]
        mov eax, esi
        mov edx, edi
        shl eax, 0x10
        or esi, eax
        mov eax, dword ptr [esp + 0x18]
        mov ecx, eax
        mov ebx, dword ptr [esp + 0x30]
        shl ecx, 0x10
        or eax, ecx
        mov ecx, dword ptr [esp + 0x28]
        shl edx, 0x10
        mov dword ptr [esp + 0x18], eax
        mov eax, dword ptr [esp + 0x2c]
        or edi, edx
        mov edx, eax
        imul edx, dword ptr [g_Data_004da000 + 0x911d4]
        sub ebx, ecx
        add edx, ecx
        mov ecx, dword ptr [g_Data_004da000 + 0x911c4]
        dec ebx
        sar ebx, 0x1
        lea ecx, [ecx + edx*0x2]
        mov edx, dword ptr [esp + 0x34]
        cmp eax, edx
        mov dword ptr [esp + 0x14], edi
        mov dword ptr [esp + 0x10], esi
        mov dword ptr [esp + 0x24], ebx
        mov dword ptr [esp + 0x1c], ecx
        mov dword ptr [esp + 0x20], eax
        jge L_48eb69
        jmp L_48eb1e
    L_48eb08:
        mov esi, dword ptr [esp + 0x10]
        mov edi, dword ptr [esp + 0x14]
        mov ebx, dword ptr [esp + 0x24]
        jmp L_48eb1e
    L_48eb16:
        mov esi, dword ptr [esp + 0x10]
        mov edi, dword ptr [esp + 0x14]
    L_48eb1e:
        mov eax, dword ptr [ecx]
        mov ebp, esi
        shr ebp, 0x1
        and ebp, esi
        mov esi, edi
        shr esi, 0x1
        and esi, edi
        mov edx, eax
        or ebp, esi
        mov esi, dword ptr [esp + 0x18]
        shr edx, 0x1
        and ebp, edx
        and eax, esi
        or ebp, eax
        mov edx, ebx
        mov dword ptr [ecx], ebp
        add ecx, 0x4
        dec ebx
        test edx, edx
        jnz L_48eb16
        mov eax, dword ptr [g_Data_004da000 + 0x911d4]
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x34]
        lea ecx, [ecx + eax*0x2]
        mov eax, dword ptr [esp + 0x20]
        inc eax
        mov dword ptr [esp + 0x1c], ecx
        cmp eax, edx
        mov dword ptr [esp + 0x20], eax
        jl L_48eb08
    L_48eb69:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x28
        ret
    }
}

// 0x0042bb30 Game_HandleStateEvent - disassembly re-read 2026-09-24 (caveat resolved; replaces the decompile-based note, which misnamed several callees). (evt) ret 4, ECX = sender. 0: if ECX == [0x004f3a90] clear it. 1: [0x004e5cd8]=1, Camera_SetViewVehicleToPlayer. 2: Camera_ReleaseViewVehicleIfSinglePlayer, Inset_Disable, Input_Mouse
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Game_HandleStateEvent(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        cmp eax, 0xa
        jg L_42bb9f
        jz L_42bb92
        sub eax, 0x0
        jz L_42bb79
        dec eax
        jz L_42bb67
        dec eax
        jnz L_42bdaf
        call Camera_ReleaseViewVehicleIfSinglePlayer
        call Inset_Disable
        xor edx, edx
        xor ecx, ecx
        push edx
        push edx
        call Input_Mouse_CursorRaycastDispatch
        call Hud_HideBothPanels
        ret 0x4
    L_42bb67:
        mov dword ptr [g_Data_004da000 + 0xbcd8], 0x1
        call Camera_SetViewVehicleToPlayer
        ret 0x4
    L_42bb79:
        cmp ecx, dword ptr [g_Data_004da000 + 0x19a90]
        jnz L_42bdaf
        mov dword ptr [g_Data_004da000 + 0x19a90], 0x0
        ret 0x4
    L_42bb92:
        call Vehicle_SyncPlayerTransformFromObject
        call Hud_ClearMessagePanels
        ret 0x4
    L_42bb9f:
        cmp eax, 0xe
        jg L_42bc0d
        jz L_42bbdb
        cmp eax, 0xb
        jnz L_42bdaf
        call Settings_GetNetworkFlag
        test eax, eax
        jnz L_42bdaf
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        push 0x0
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x60], 0x6
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        call Vehicle_UpdateEngineSound
        ret 0x4
    L_42bbdb:
        call Settings_GetNetworkFlag
        test eax, eax
        jnz L_42bc05
        mov dword ptr [g_Data_004da000 + 0x196b0], eax
        call Inset_Disable
        xor edx, edx
        xor ecx, ecx
        push edx
        push edx
        call Input_Mouse_CursorRaycastDispatch
        xor ecx, ecx
        call HudTimer_SetPaused
        call CallGlobal004e5ee8Slot18
    L_42bc05:
        call Turrets_Release
        ret 0x4
    L_42bc0d:
        cmp eax, 0x63
        jg L_42bd5d
        jz L_42bd50
        add eax, -0xf
        cmp eax, 0xc
        ja L_42bdaf
        cmp eax, 0
        je L_42bc2f
        cmp eax, 1
        je L_42bcd6
        cmp eax, 2
        je L_42bce4
        cmp eax, 3
        je L_42bdaf
        cmp eax, 4
        je L_42bdaf
        cmp eax, 5
        je L_42bcf2
        cmp eax, 6
        je L_42bdaf
        cmp eax, 7
        je L_42bdaf
        cmp eax, 8
        je L_42bdaf
        cmp eax, 9
        je L_42bdaf
        cmp eax, 10
        je L_42bcfb
        cmp eax, 11
        je L_42bd14
        cmp eax, 12
        je L_42bd39
        int 3  // unreachable: the bounds check above excludes other indices
    L_42bc2f:
        call Settings_GetNetworkFlag
        test eax, eax
        jnz L_42bcce
        mov dword ptr [g_Data_004da000 + 0x196b0], 0x1
        call Settings_GetSplitScreenValue
        test eax, eax
        jz L_42bc60
        call Setting_Get_HUDType
        mov ecx, eax
        call Hud_ApplyTypeChange
        call Inset_Enable
    L_42bc60:
        push 0x3f000000
        xor edx, edx
        push 0x3f000000
        mov ecx, 0x1
        call Input_Mouse_CursorRaycastDispatch
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        push 0x40a00000
        mov eax, dword ptr [edx + 0x4]
        mov ecx, dword ptr [eax + 0x5e4]
        mov edx, dword ptr [ecx]
        mov ecx, dword ptr [edx + 0x8]
        call Hud_ShowMessage
        mov ecx, 0x1
        call HudTimer_SetPaused
        call CallGlobal004e5ee8Slot18
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_GetOutcome
        cmp eax, 0x1
        jnz L_42bcce
        mov eax, dword ptr [g_Data_004da000 + 0x16de4]
        test eax, eax
        jnz L_42bcce
        mov eax, dword ptr [g_Data_004da000 + 0x1911c]
        test eax, eax
        jnz L_42bcce
        mov ecx, 0x1
        call Sound_PlayPowerup
    L_42bcce:
        call Turrets_ForEachNode
        ret 0x4
    L_42bcd6:
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        call Vehicle_ZeroMotion
        ret 0x4
    L_42bce4:
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        call Vehicle_TeleportToNode
        ret 0x4
    L_42bcf2:
        mov dword ptr [g_Data_004da000 + 0x19a90], ecx
        ret 0x4
    L_42bcfb:
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0xf38], 0x0
        xor ecx, ecx
        call Hud_ForwardToGlobal_004e62f0
    L_42bd14:
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        push ecx
        mov edx, dword ptr [ecx + 0x4]
        fld dword ptr [edx + 0xf34]
        fsub dword ptr [g_RData_004cc000 + 0x4860]
        xor edx, edx
        fstp dword ptr [esp]
        push 0x0
        call Vehicle_TakeDamage
        ret 0x4
    L_42bd39:
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        push 0x41200000
        push 0x0
        xor edx, edx
        call Vehicle_TakeDamage
        ret 0x4
    L_42bd50:
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_AdvanceToNext
        ret 0x4
    L_42bd5d:
        add eax, 0xfffffc71
        cmp eax, 0x3
        ja L_42bdaf
        cmp eax, 0
        je L_42bd6e
        cmp eax, 1
        je L_42bd76
        cmp eax, 2
        je L_42bd8a
        cmp eax, 3
        je L_42bd9e
        int 3  // unreachable: the bounds check above excludes other indices
    L_42bd6e:
        call PickupCrate_TryDropIfClear
        ret 0x4
    L_42bd76:
        push 0x1
        mov edx, 0x20
        mov ecx, offset g_Data_004da000 + 0x2a98
        call Pickup_SpawnAtNamedNode
        ret 0x4
    L_42bd8a:
        push 0x1
        mov edx, 0x24
        mov ecx, offset g_Data_004da000 + 0x2a90
        call Pickup_SpawnAtNamedNode
        ret 0x4
    L_42bd9e:
        push 0x1
        mov edx, 0x1e
        mov ecx, offset g_Data_004da000 + 0x2a88
        call Pickup_SpawnAtNamedNode
    L_42bdaf:
        ret 0x4
    }
}

// 0x0042eed0 App_EnterGameplay - game session start (929 bytes, 73 calls): Timer_Reset; dt [0x0056b424] = 0.1 (0x3dcccccd); effects level from settings (HW/SW); HUD load (hud.zrd), 0x00410160/0x00414210; loading progress lines via GetMessageByID ids 3/5/6 into loading screen (0x00414180); "Loading common sounds" group "COMMON"; Mis
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_EnterGameplay(int, int)
{
    __asm {
        sub esp, 0x104
        push ebx
        mov ebx, dword ptr [g_Data_004da000 + 0x16df8]
        push esi
        push edi
        mov esi, ecx
        call Settings_GetHWCardFlag
        test eax, eax
        jz L_42eefb
        lea eax, [esp + 0xc]
        push 0x0
        push eax
        push 0x1
        push 0x61
        call dword ptr [g_Iat_SystemParametersInfoA_004cc624]
    L_42eefb:
        call Timer_Reset
        mov dword ptr [g_Data_004da000 + 0x91424], 0x3dcccccd
        call Settings_GetNetworkFlag
        test eax, eax
        jz L_42ef18
        call NetExitDialog_InitOnGameLoad
    L_42ef18:
        call Setting_Get_EffectsLevel
        mov edi, eax
        call Settings_GetHWCardFlag
        test eax, eax
        jnz L_42ef31
        test edi, edi
        jnz L_42ef31
        mov edi, 0x1
    L_42ef31:
        mov ecx, edi
        call Settings_ApplyEffectsLevel
        mov ecx, offset g_Data_004da000 + 0x2ca8
        call Hud_Init
        call Loading_InitCheckpointTable
        mov ecx, offset g_Data_004da000 + 0x2c90
        call Loading_Checkpoint
        mov ecx, offset g_Data_004da000 + 0x2c88
        call SoundBank_FindAndLoad
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_GetOutcome
        mov ecx, eax
        call Briefing_StartLoaderThread
        call Version_GetString
        push eax
        push 0x3
        lea ecx, [esp + 0x18]
        push 0x100
        push ecx
        call GetMessageByID
        add esp, 0x10
        lea ecx, [esp + 0x10]
        call Loading_Checkpoint
        call zVideo_GetDriverName
        push eax
        push 0x5
        lea edx, [esp + 0x18]
        push 0x100
        push edx
        call GetMessageByID
        add esp, 0x10
        lea ecx, [esp + 0x10]
        call Loading_Checkpoint
        call zVideo_GetDeviceGuidOrName
        push eax
        push 0x6
        lea eax, [esp + 0x18]
        push 0x100
        push eax
        call GetMessageByID
        add esp, 0x10
        lea ecx, [esp + 0x10]
        call Loading_Checkpoint
        mov ecx, 0x10d
        call Message_GetText
        mov ecx, eax
        call Loading_Checkpoint
        push offset g_Data_004da000 + 0xfa8
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_LoadObjectivesArray
        call Player_RegisterSaveHandlers
        mov ecx, ebx
        call Loader_StartIfReady
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_Load
        test eax, eax
        jnz L_42f019
        pop edi
        pop esi
        pop ebx
        add esp, 0x104
        ret
    L_42f019:
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_InitGameplay
        mov ecx, 0x1
        call Loader_WaitForThread
        mov ecx, 0x1
        call Hud_ApplyTypeChange
        mov eax, dword ptr [esi + 0x14]
        test eax, eax
        jz L_42f09c
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fcomp qword ptr [g_RData_004cc000 + 0x49b8]
        fnstsw AX
        test AH, 0x41
        jnz L_42f065
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fsub dword ptr [g_RData_004cc000 + 0x49c0]
        fstp dword ptr [g_Data_004da000 + 0x19df8]
        jmp L_42f079
    L_42f065:
        mov ecx, 0x1
        mov dword ptr [g_Data_004da000 + 0x19df8], 0x40a00000
        call Settings_ApplyMuteSound
    L_42f079:
        mov ecx, dword ptr [esi + 0x14]
        call SaveGame_LoadFromFile
        mov ecx, dword ptr [esi + 0x14]
        push ecx
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        mov dword ptr [esi + 0x14], 0x0
        push offset g_Data_004da000 + 0xfec
        jmp L_42f0a1
    L_42f09c:
        push offset g_Data_004da000 + 0x2c78
    L_42f0a1:
        push offset g_Data_004da000 + 0xfdc
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_StopConfiguredAnims
        call Setting_Get_ViewportRect
        mov dword ptr [esi + 0xc], eax
        call Setting_Get_ScreenRect
        mov dword ptr [esi + 0x8], eax
        call Settings_GetValue_004e5d88
        mov dword ptr [esi + 0x4], eax
        call Input_Keyboard_InitialPollAndClear
        call Input_Mouse_RecenterCursor
        call Settings_GetHWCardFlag
        test eax, eax
        jz L_42f0e9
        xor ecx, ecx
        call Camera_SetGlobal_004ddd34
        xor ecx, ecx
        call Camera_SetGlobal_004ddd10
    L_42f0e9:
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fcomp qword ptr [g_RData_004cc000 + 0x49b8]
        fnstsw AX
        test AH, 0x41
        jnz L_42f110
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fsub dword ptr [g_RData_004cc000 + 0x49c8]
        fstp dword ptr [g_Data_004da000 + 0x19df8]
        jmp L_42f124
    L_42f110:
        mov ecx, 0x1
        mov dword ptr [g_Data_004da000 + 0x19df8], 0x3f800000
        call Settings_ApplyMuteSound
    L_42f124:
        push 0x0
        mov ecx, esi
        call App_RenderFrameAndPresent
        call Input_Keyboard_InitialPollAndClear
        call Input_Mouse_RecenterCursor
        mov ecx, 0x1
        mov dword ptr [g_Data_004da000 + 0x91bd8], 0x0
        mov dword ptr [g_Data_004da000 + 0x19df0], 0x1
        call zVideo_SetFlag0063212c
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_BeginPlay
        call Input_GetMouseAvailable
        test eax, eax
        jz L_42f177
        mov dword ptr [g_Data_004da000 + 0x87c74], 0x0
        call Input_Mouse_SetAcquired
    L_42f177:
        call Input_ResetAll
        call Setting_Get_GfxFlags
        mov ecx, eax
        call Settings_ApplyGfxFlags
        call Setting_Get_004e5d60
        mov ecx, eax
        call Joystick_EnableAndSetRanges
        mov ecx, eax
        call Settings_StoreJoystickEnabled
        call ControlFlags_GetBit2
        mov ecx, eax
        call ControlFlags_SetBit2
        call ControlFlags_GetBit3Mode
        mov ecx, eax
        call ControlFlags_SetBit3AndCamera
        call ControlFlags_GetBit0
        mov ecx, eax
        call ControlFlags_SetBit0
        call ControlFlags_GetBit1
        mov ecx, eax
        call ControlFlags_SetBit1
        call Setting_Get_CDAudio
        test eax, eax
        jz L_42f1fe
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_GetOutcome
        mov esi, eax
        call SoundCD_GetTrackCount
        mov ecx, eax
        mov eax, esi
        cdq
        sub ecx, 0x2
        idiv ecx
        mov ecx, edx
        mov edx, 0x5
        add ecx, 0x2
        call SoundCD_PlayIfPrepared
    L_42f1fe:
        call Settings_GetNetworkFlag
        test eax, eax
        jz L_42f262
        call Net_IsSessionActive
        test eax, eax
        jnz L_42f25d
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fcomp qword ptr [g_RData_004cc000 + 0x49b8]
        fnstsw AX
        test AH, 0x41
        jnz L_42f249
        fld dword ptr [g_Data_004da000 + 0x19df8]
        fsub dword ptr [g_RData_004cc000 + 0x49c0]
        fstp dword ptr [g_Data_004da000 + 0x19df8]
        call Hud_ClearMessagePanels
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0x104
        ret
    L_42f249:
        mov ecx, 0x1
        mov dword ptr [g_Data_004da000 + 0x19df8], 0x40a00000
        call Settings_ApplyMuteSound
    L_42f25d:
        call Hud_ClearMessagePanels
    L_42f262:
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        add esp, 0x104
        ret
    }
}

// 0x0042f8e0 App_EnterMissionOver_0042f8e0 - REFINED 2026-09-29: BYTE RE-READ 2026-09-29 (Sonnet) - confirms the earlier note; strings and imports now resolved: SEH frame (handler 0x004c9ed8). 0x00414180(ECX = 0x004dccdc = 'Leaving Play State'); when 0x00408310() != 0: SystemParametersInfoA(0x61 = SPI_SCREENSAVERRUNNING, 0, &buf, 0) via [0x004
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall App_EnterMissionOver_0042f8e0(int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_App_EnterMissionOver
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x24
        mov ecx, offset g_Data_004da000 + 0x2cdc
        call Loading_Checkpoint
        call Settings_GetHWCardFlag
        test eax, eax
        jz L_42f91c
        lea eax, [esp]
        push 0x0
        push eax
        push 0x0
        push 0x61
        call dword ptr [g_Iat_SystemParametersInfoA_004cc624]
    L_42f91c:
        call SoundCD_Stop
        xor ecx, ecx
        call zVideo_SetFlag0063212c
        call Settings_GetNetworkFlag
        test eax, eax
        jz L_42f940
        mov ecx, offset g_Data_004da000 + 0x2cc8
        call Loading_Checkpoint
        call NetExitHost_Release
    L_42f940:
        call Settings_GetNetworkFlag
        test eax, eax
        jnz L_42f95f
        mov ecx, offset g_Data_004da000 + 0x114c
        call Loading_Checkpoint
        call Sound_BuildPlayingVoiceList
        mov ecx, eax
        call SoundList_StopPlayingVoices
    L_42f95f:
        push 0x0
        push offset g_Data_004da000 + 0x2cbc
        push offset g_Data_004da000 + 0x2bd0
        lea ecx, [esp + 0x10]
        call Seq_Ctor
        push 0x1
        lea ecx, [esp + 0x8]
        mov dword ptr [esp + 0x30], 0x0
        call Seq_Play
        mov eax, dword ptr [g_Data_004da000 + 0x19d7c]
        test eax, eax
        jnz L_42f99a
        mov ecx, offset g_Data_004da000 + 0x16cc0
        call Mission_Unload
    L_42f99a:
        xor ecx, ecx
        call Reader_CloseAll
        lea ecx, [esp + 0x4]
        mov dword ptr [esp + 0x2c], 0xffffffff
        call Seq_Dtor
        mov ecx, dword ptr [esp + 0x24]
        mov dword ptr FS:[0x0], ecx
        add esp, 0x30
        ret
    }
}

// 0x00430740 Slot_ClearFlagsThenLoadArchives_00430740 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_ClearFlagsThenLoadArchives_00430740(int, int)
{
    __asm {
        xor eax, eax
        mov ecx, offset g_Data_004da000 + 0x19ca8
        mov dword ptr [g_Data_004da000 + 0x19df4], eax
        mov dword ptr [g_Data_004da000 + 0x19ea8], eax
        jmp Screen_LoadArchivesThenIdle
    }
}

// 0x00430d80 Menus_RunNetSetupThenStartMission_00430d80 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Menus_RunNetSetupThenStartMission_00430d80(int, int)
{
    __asm {
        mov eax, FS:[0x0]
        push -0x1
        push offset EH_Handler_Menus_RunNetSetupThenStartMission
        push eax
        mov dword ptr FS:[0x0], esp
        sub esp, 0x3ac
        push ebx
        xor ebx, ebx
        push esi
        mov esi, ecx
        push ebx
        call dword ptr [g_Iat_CoInitialize_004cc70c]
        cmp eax, ebx
        jl L_43124c
        push ebx
        lea ecx, [esp + 0x88]
        call NameDlg_ctor
        push ebx
        lea ecx, [esp + 0x23c]
        mov dword ptr [esp + 0x3c0], ebx
        call NetSetupDialog_Ctor
        mov ecx, offset g_RData_004cc000 + 0xd78
        mov byte ptr [esp + 0x3bc], 0x1
        call DPlay_InitSession
        mov ecx, offset App_FatalErrorShutdown_00430c90
        call Net_SetGlobal_0056aafc_00489f90
        lea ecx, [esp + 0x84]
        mov dword ptr [g_Data_004da000 + 0x19df4], 0x1
        mov dword ptr [g_Data_004da000 + 0x19ea8], 0x1
        call dword ptr [g_Iat_MFC42_2514_004cc3f8]
        cmp eax, 0x1
        jnz L_431143
        mov ecx, dword ptr [esp + 0x234]
        call Settings_StorePlayerName
        cmp dword ptr [esp + 0xec], ebx
        jnz L_431008
        mov eax, dword ptr [esp + 0xe4]
        lea ecx, [esp + 0x10]
        mov dword ptr [esp + 0x20], eax
        call DPlay_JoinSession
        test eax, eax
        jz L_431143
        mov ecx, 0x1
        call Settings_SetNetworkFlag
        mov ecx, dword ptr [esp + 0x234]
        call DPlay_CreateLocalPlayerAndJoin
        mov ecx, dword ptr [esp + 0x234]
        call Settings_StorePlayerName
        cmp dword ptr [esp + 0x10], 0xff
        jbe L_430e9a
        push 0x2
        mov edx, offset Net_Handler_004344b0
        mov ecx, 0x14
        mov dword ptr [g_Data_004da000 + 0x19d6c], offset g_Data_004da000 + 0x19ec8
        mov dword ptr [esp + 0x14], 0x1
        call NetMsgQueue_Push
    L_430e9a:
        mov ecx, dword ptr [esp + 0x14]
        call Net_SetFlags3f88_3f90
        mov edx, dword ptr [esp + 0x18]
        mov ecx, dword ptr [esp + 0x1c]
        mov dword ptr [esp + 0x8], edx
        mov dword ptr [esp + 0xc], ebx
        fild qword ptr [esp + 0x8]
        push ecx
        push ecx
        mov ecx, offset g_Data_004da000 + 0x16cc0
        fmul dword ptr [g_RData_004cc000 + 0x5138]
        fstp dword ptr [esp]
        call Mission_SetRaceTiming
        mov eax, dword ptr [esi + 0x1d8]
        mov ecx, dword ptr [esp + 0x10]
        push eax
        push 0x1
        add ecx, 0x6
        push ebx
        push ecx
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call App_StartMission
        lea ecx, [esp + 0x3a0]
        mov byte ptr [esp + 0x3bc], 0x6
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0x360]
        mov byte ptr [esp + 0x3bc], 0x5
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x320]
        mov byte ptr [esp + 0x3bc], 0x4
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x2e0]
        mov byte ptr [esp + 0x3bc], 0x3
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x2a0]
        mov byte ptr [esp + 0x3bc], 0x2
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x238]
        mov byte ptr [esp + 0x3bc], BL
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        lea ecx, [esp + 0x234]
        mov dword ptr [esp + 0x3bc], 0xc
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0x1f4]
        mov byte ptr [esp + 0x3bc], 0xb
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x1b4]
        mov byte ptr [esp + 0x3bc], 0xa
        call dword ptr [g_Iat_MFC42_692_004cc344]
        lea ecx, [esp + 0x174]
        mov byte ptr [esp + 0x3bc], 0x9
        call dword ptr [g_Iat_MFC42_609_004cc334]
        lea ecx, [esp + 0x134]
        mov byte ptr [esp + 0x3bc], 0x8
        call dword ptr [g_Iat_MFC42_609_004cc334]
        lea ecx, [esp + 0xf4]
        mov byte ptr [esp + 0x3bc], 0x7
        call dword ptr [g_Iat_MFC42_656_004cc33c]
        lea ecx, [esp + 0x84]
        mov dword ptr [esp + 0x3bc], 0xffffffff
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        mov ecx, dword ptr [esp + 0x3b4]
        mov dword ptr FS:[0x0], ecx
        pop esi
        pop ebx
        add esp, 0x3b8
        ret
    L_431008:
        mov ecx, 0x1
        call Settings_SetNetworkFlag
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call Screen_LoadArchivesThenIdle
        xor ecx, ecx
        call Screen_Enter_004f32a0_WithArg
        lea ecx, [esp + 0x3a0]
        mov byte ptr [esp + 0x3bc], 0x11
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0x360]
        mov byte ptr [esp + 0x3bc], 0x10
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x320]
        mov byte ptr [esp + 0x3bc], 0xf
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x2e0]
        mov byte ptr [esp + 0x3bc], 0xe
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x2a0]
        mov byte ptr [esp + 0x3bc], 0xd
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x238]
        mov byte ptr [esp + 0x3bc], BL
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        lea ecx, [esp + 0x234]
        mov dword ptr [esp + 0x3bc], 0x17
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0x1f4]
        mov byte ptr [esp + 0x3bc], 0x16
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x1b4]
        mov byte ptr [esp + 0x3bc], 0x15
        call dword ptr [g_Iat_MFC42_692_004cc344]
        lea ecx, [esp + 0x174]
        mov byte ptr [esp + 0x3bc], 0x14
        call dword ptr [g_Iat_MFC42_609_004cc334]
        lea ecx, [esp + 0x134]
        mov byte ptr [esp + 0x3bc], 0x13
        call dword ptr [g_Iat_MFC42_609_004cc334]
        lea ecx, [esp + 0xf4]
        mov byte ptr [esp + 0x3bc], 0x12
        call dword ptr [g_Iat_MFC42_656_004cc33c]
        lea ecx, [esp + 0x84]
        mov dword ptr [esp + 0x3bc], 0xffffffff
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        mov ecx, dword ptr [esp + 0x3b4]
        mov dword ptr FS:[0x0], ecx
        pop esi
        pop ebx
        add esp, 0x3b8
        ret
    L_431143:
        lea ecx, [esp + 0x3a0]
        mov byte ptr [esp + 0x3bc], 0x1c
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0x360]
        mov byte ptr [esp + 0x3bc], 0x1b
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x320]
        mov byte ptr [esp + 0x3bc], 0x1a
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x2e0]
        mov byte ptr [esp + 0x3bc], 0x19
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x2a0]
        mov byte ptr [esp + 0x3bc], 0x18
        call dword ptr [g_Iat_MFC42_793_004cc2f0]
        lea ecx, [esp + 0x238]
        mov byte ptr [esp + 0x3bc], BL
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
        lea ecx, [esp + 0x234]
        mov dword ptr [esp + 0x3bc], 0x22
        call dword ptr [g_Iat_MFC42_800_004cc2a0]
        lea ecx, [esp + 0x1f4]
        mov byte ptr [esp + 0x3bc], 0x21
        call dword ptr [g_Iat_MFC42_616_004cc348]
        lea ecx, [esp + 0x1b4]
        mov byte ptr [esp + 0x3bc], 0x20
        call dword ptr [g_Iat_MFC42_692_004cc344]
        lea ecx, [esp + 0x174]
        mov byte ptr [esp + 0x3bc], 0x1f
        call dword ptr [g_Iat_MFC42_609_004cc334]
        lea ecx, [esp + 0x134]
        mov byte ptr [esp + 0x3bc], 0x1e
        call dword ptr [g_Iat_MFC42_609_004cc334]
        lea ecx, [esp + 0xf4]
        mov byte ptr [esp + 0x3bc], 0x1d
        call dword ptr [g_Iat_MFC42_656_004cc33c]
        lea ecx, [esp + 0x84]
        mov dword ptr [esp + 0x3bc], 0xffffffff
        call dword ptr [g_Iat_MFC42_641_004cc2b4]
    L_43124c:
        call Net_Shutdown
        xor ecx, ecx
        call Settings_SetNetworkFlag
        mov ecx, dword ptr [esp + 0x3b4]
        pop esi
        mov dword ptr FS:[0x0], ecx
        pop ebx
        add esp, 0x3b8
        ret
    }
}

// 0x00431270 Slot_StartMission_1_00431270 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_StartMission_1_00431270(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x1d8]
        mov ecx, offset g_Data_004da000 + 0x19ca8
        push eax
        push 0x1
        push 0x0
        push 0x1
        call App_StartMission
        ret
    }
}

// 0x00431290 Slot_StartMission_2_00431290 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_StartMission_2_00431290(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x1d8]
        mov ecx, offset g_Data_004da000 + 0x19ca8
        push eax
        push 0x1
        push 0x0
        push 0x2
        call App_StartMission
        ret
    }
}

// 0x004312b0 Slot_StartMission_3_004312b0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_StartMission_3_004312b0(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x1d8]
        mov ecx, offset g_Data_004da000 + 0x19ca8
        push eax
        push 0x1
        push 0x0
        push 0x3
        call App_StartMission
        ret
    }
}

// 0x004312d0 Slot_StartMission_4_004312d0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_StartMission_4_004312d0(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x1d8]
        mov ecx, offset g_Data_004da000 + 0x19ca8
        push eax
        push 0x1
        push 0x0
        push 0x4
        call App_StartMission
        ret
    }
}

// 0x004312f0 Slot_StartMission_5_004312f0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_StartMission_5_004312f0(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x1d8]
        mov ecx, offset g_Data_004da000 + 0x19ca8
        push eax
        push 0x1
        push 0x0
        push 0x5
        call App_StartMission
        ret
    }
}

// 0x00431310 Slot_StartMission_6_00431310 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Slot_StartMission_6_00431310(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x1d8]
        mov ecx, offset g_Data_004da000 + 0x19ca8
        push eax
        push 0x1
        push 0x0
        push 0x6
        call App_StartMission
        ret
    }
}

// 0x00430760 DataTarget_00430760 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_00430760(int, int)
{
    __asm {
        mov dword ptr [g_Data_004da000 + 0x19df4], 0x1
        jmp L_430770
    L_430770:
        sub esp, 0x24c
        mov AX, word ptr [g_Data_004da000 + 0x19f08]
        push ebx
        push ebp
        push esi
        mov ebx, ecx
        mov edx, dword ptr [g_Data_004da000 + 0x19ef8]
        push edi
        mov word ptr [esp + 0x15c], AX
        mov ecx, 0x3f
        xor eax, eax
        lea edi, [esp + 0x15e]
        push 0x100
        rep stosd
        lea ecx, [esp + 0x60]
        push ecx
        push 0xc8
        push edx
        stosw
        call dword ptr [g_Iat_LoadStringA_004cc650]
        mov CL, byte ptr [esp + eax*0x1 + 0x5b]
        mov AL, byte ptr [esp + 0x5c]
        test AL, AL
        jz L_4307d7
        lea eax, [esp + 0x5c]
    L_4307c8:
        cmp byte ptr [eax], CL
        jnz L_4307cf
        mov byte ptr [eax], 0x0
    L_4307cf:
        mov DL, byte ptr [eax + 0x1]
        inc eax
        test DL, DL
        jnz L_4307c8
    L_4307d7:
        mov ecx, 0x13
        xor eax, eax
        lea edi, [esp + 0x10]
        lea ebp, [ebx + 0xcc]
        rep stosd
        mov eax, dword ptr [ebx + 0x20]
        lea ecx, [esp + 0x5c]
        mov dword ptr [esp + 0x14], eax
        xor eax, eax
        mov dword ptr [esp + 0x40], eax
        mov dword ptr [esp + 0x3c], eax
        lea eax, [esp + 0x10]
        lea edx, [esp + 0x15c]
        push eax
        mov dword ptr [esp + 0x14], 0x4c
        mov dword ptr [esp + 0x20], ecx
        mov dword ptr [esp + 0x2c], 0x1
        mov dword ptr [esp + 0x30], ebp
        mov dword ptr [esp + 0x34], 0x104
        mov dword ptr [esp + 0x38], edx
        mov dword ptr [esp + 0x3c], 0x200
        mov dword ptr [esp + 0x50], offset g_Data_004da000 + 0x2d6c
        mov dword ptr [esp + 0x48], 0x1800
        call dword ptr [g_Iat_GetOpenFileNameA_004cc700]
        test eax, eax
        jz L_430881
        mov edi, dword ptr [esp + 0x2c]
        or ecx, 0xffffffff
        xor eax, eax
        push 0x1
        repne scasb
        not ecx
        sub edi, ecx
        push 0x1
        mov edx, ecx
        mov esi, edi
        mov edi, ebp
        push ebp
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        push eax
        and ecx, 0x3
        rep movsb
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call App_StartMission
    L_430881:
        mov eax, dword ptr [ebx + 0x20]
        push 0x1
        push 0x0
        push eax
        call dword ptr [g_Iat_InvalidateRect_004cc680]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x24c
        ret
    }
}

// 0x00430770 DataTarget_00430770 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall DataTarget_00430770(int, int)
{
    __asm {
        sub esp, 0x24c
        mov AX, word ptr [g_Data_004da000 + 0x19f08]
        push ebx
        push ebp
        push esi
        mov ebx, ecx
        mov edx, dword ptr [g_Data_004da000 + 0x19ef8]
        push edi
        mov word ptr [esp + 0x15c], AX
        mov ecx, 0x3f
        xor eax, eax
        lea edi, [esp + 0x15e]
        push 0x100
        rep stosd
        lea ecx, [esp + 0x60]
        push ecx
        push 0xc8
        push edx
        stosw
        call dword ptr [g_Iat_LoadStringA_004cc650]
        mov CL, byte ptr [esp + eax*0x1 + 0x5b]
        mov AL, byte ptr [esp + 0x5c]
        test AL, AL
        jz L_4307d7
        lea eax, [esp + 0x5c]
    L_4307c8:
        cmp byte ptr [eax], CL
        jnz L_4307cf
        mov byte ptr [eax], 0x0
    L_4307cf:
        mov DL, byte ptr [eax + 0x1]
        inc eax
        test DL, DL
        jnz L_4307c8
    L_4307d7:
        mov ecx, 0x13
        xor eax, eax
        lea edi, [esp + 0x10]
        lea ebp, [ebx + 0xcc]
        rep stosd
        mov eax, dword ptr [ebx + 0x20]
        lea ecx, [esp + 0x5c]
        mov dword ptr [esp + 0x14], eax
        xor eax, eax
        mov dword ptr [esp + 0x40], eax
        mov dword ptr [esp + 0x3c], eax
        lea eax, [esp + 0x10]
        lea edx, [esp + 0x15c]
        push eax
        mov dword ptr [esp + 0x14], 0x4c
        mov dword ptr [esp + 0x20], ecx
        mov dword ptr [esp + 0x2c], 0x1
        mov dword ptr [esp + 0x30], ebp
        mov dword ptr [esp + 0x34], 0x104
        mov dword ptr [esp + 0x38], edx
        mov dword ptr [esp + 0x3c], 0x200
        mov dword ptr [esp + 0x50], offset g_Data_004da000 + 0x2d6c
        mov dword ptr [esp + 0x48], 0x1800
        call dword ptr [g_Iat_GetOpenFileNameA_004cc700]
        test eax, eax
        jz L_430881
        mov edi, dword ptr [esp + 0x2c]
        or ecx, 0xffffffff
        xor eax, eax
        push 0x1
        repne scasb
        not ecx
        sub edi, ecx
        push 0x1
        mov edx, ecx
        mov esi, edi
        mov edi, ebp
        push ebp
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        push eax
        and ecx, 0x3
        rep movsb
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call App_StartMission
    L_430881:
        mov eax, dword ptr [ebx + 0x20]
        push 0x1
        push 0x0
        push eax
        call dword ptr [g_Iat_InvalidateRect_004cc680]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x24c
        ret
    }
}

// 0x004319a0 Menus_StartZoneLobbyThenMission_004319a0 - ../../04_spec/systems/app.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Menus_StartZoneLobbyThenMission_004319a0(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x19f04]
        sub esp, 0x304
        test eax, eax
        push ebx
        push esi
        push edi
        mov ebx, 0x1
        jnz L_431a30
        mov ecx, 0x12
        mov dword ptr [g_Data_004da000 + 0x19f04], ebx
        call Message_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x10]
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
        mov ecx, 0x26
        call Message_GetText
        mov edi, eax
        or ecx, 0xffffffff
        xor eax, eax
        lea edx, [esp + 0x110]
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, edx
        lea edx, [esp + 0x110]
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        lea ecx, [esp + 0x10]
        call Net_CheckWinsock20_0043ce80
        test eax, eax
        jnz L_431a30
        xor ebx, ebx
    L_431a30:
        test ebx, ebx
        jz L_431a71
        lea ecx, [esp + 0xc]
        mov dword ptr [g_Data_004da000 + 0x19df4], 0x1
        mov dword ptr [g_Data_004da000 + 0x19ea8], 0x1
        call Zone_RunLobby
        test eax, eax
        jz L_431a71
        mov eax, dword ptr [g_Data_004da000 + 0x16da0]
        mov ecx, dword ptr [esp + 0xc]
        push eax
        push 0x1
        add ecx, 0x6
        push 0x0
        push ecx
        mov ecx, offset g_Data_004da000 + 0x19ca8
        call App_StartMission
    L_431a71:
        pop edi
        pop esi
        pop ebx
        add esp, 0x304
        ret
    }
}

}  // namespace recoil
