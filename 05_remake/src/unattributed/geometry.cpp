// SUBSYSTEM: geometry
// Geometry functions with no attributed original source file (ledger orig_file empty); their placement here
// is a layout choice, not a provenance claim (STAGE2.md section 2). Spec: 04_spec/systems/geometry.md
#include "unattributed/geometry.h"
#include "platform/msvcrt.h"  // MSVCRT import slot (free)
#include "GameZRecoil/zModel/gmod_matl.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

namespace recoil {

float g_BFETolerance_004e0fc0 = 0.005f;  // CONFIRMED-DATA: dword 0x3ba3d70a at 0x004e0fc0 in Recoil.exe .data

// 0x00476460 Geom_SetBFETolerance (stack value; ret 4): back-face cull epsilon := value.
__declspec(naked) void __stdcall Geom_SetBFETolerance(float)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov [g_BFETolerance_004e0fc0], eax
        ret 0x4
    }
}

// 0x00476470 Geom_GetBFETolerance -> ST0: the back-face cull epsilon.
__declspec(naked) long double Geom_GetBFETolerance()
{
    __asm {
        fld dword ptr [g_BFETolerance_004e0fc0]
        ret
    }
}

// 0x0046c720 Alloc3Ptr_Free (ECX block or null): frees [+0x4] and [+0xc] if set, then the block.
__declspec(naked) void __fastcall Alloc3Ptr_Free(void*)
{
    __asm {
        push esi
        mov esi, ecx
        test esi, esi
        jz L_46c74f
        mov eax, dword ptr [esi + 0x4]
        push edi
        mov edi, dword ptr [g_Iat_free_004cc5b4]
        test eax, eax
        jz L_46c73b
        push eax
        call edi
        add esp, 0x4
    L_46c73b:
        mov eax, dword ptr [esi + 0xc]
        test eax, eax
        jz L_46c748
        push eax
        call edi
        add esp, 0x4
    L_46c748:
        push esi
        call edi
        add esp, 0x4
        pop edi
    L_46c74f:
        pop esi
        ret
    }
}

// 0x0046a690 Material_CreateRandomColour - bytes read: local material Material_Init; r, g, b floats (+4/+8/+0xC) = rand() * 0.0077822 (255/32767, 0x004d2650); 16-bit colour word +2 = 5-6-5 packing of ftol(r), ftol(g), ftol(b) with the low bits of the existing word merged; [0x0053a73c] = Material_FindOrCreate(local)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Material_CreateRandomColour(int, int)
{
    __asm {
        sub esp, 0x38
        push esi
        lea ecx, [esp + 0x14]
        call Material_Init
        mov esi, dword ptr [g_Iat_rand_004cc5d8]
        call esi
        mov dword ptr [esp + 0x4], eax
        fild dword ptr [esp + 0x4]
        fmul dword ptr [g_RData_004cc000 + 0x6650]
        fstp dword ptr [esp + 0xc]
        call esi
        mov dword ptr [esp + 0x4], eax
        fild dword ptr [esp + 0x4]
        fmul dword ptr [g_RData_004cc000 + 0x6650]
        fstp dword ptr [esp + 0x8]
        call esi
        mov dword ptr [esp + 0x4], eax
        mov eax, dword ptr [esp + 0x8]
        fild dword ptr [esp + 0x4]
        mov ecx, dword ptr [esp + 0xc]
        mov dword ptr [esp + 0x18], eax
        mov esi, dword ptr [esp + 0x16]
        mov dword ptr [esp + 0x1c], ecx
        fmul dword ptr [g_RData_004cc000 + 0x6650]
        and esi, 0x7ff
        fstp dword ptr [esp + 0x10]
        mov edx, dword ptr [esp + 0x10]
        fld dword ptr [esp + 0x8]
        mov dword ptr [esp + 0x20], edx
        call dword ptr [g_Iat__ftol_004cc5ac]
        fld dword ptr [esp + 0xc]
        shl eax, 0xb
        or esi, eax
        mov word ptr [esp + 0x16], SI
        call dword ptr [g_Iat__ftol_004cc5ac]
        fld dword ptr [esp + 0x10]
        mov SI, AX
        mov eax, dword ptr [esp + 0x16]
        and esi, 0x3f
        and eax, 0xf81f
        shl esi, 0x5
        or esi, eax
        mov word ptr [esp + 0x16], SI
        call dword ptr [g_Iat__ftol_004cc5ac]
        mov CL, byte ptr [esp + 0x16]
        xor AL, CL
        lea ecx, [esp + 0x14]
        and eax, 0x1f
        xor SI, AX
        mov word ptr [esp + 0x16], SI
        call Material_FindOrCreate
        mov dword ptr [g_Data_004da000 + 0x6073c], eax
        pop esi
        add esp, 0x38
        ret
    }
}

}  // namespace recoil
