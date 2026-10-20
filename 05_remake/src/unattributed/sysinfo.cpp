// SUBSYSTEM: sysinfo
// Functions of subsystem `sysinfo` with no attributed original source file (ledger orig_file empty); their placement
// here is a layout choice, not a provenance claim (STAGE2.md section 2). Spec: 04_spec/systems/sysinfo.md
#include "unattributed/sysinfo.h"
#include "GameZRecoil/zVideo/zvid_dd.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"
#include "unattributed/settings.h"
#include "platform/iat_dsound.h"

namespace recoil {

// 0x004c6100 Crt_ChkStk - instruction-level port (tools/asm_port/ghidra2masm.py). 0x1000 is the page size the probe
// walks: CONFIRMED-BINARY, CMP EAX,0x1000 at 0x004c6101.
__declspec(naked) void Crt_ChkStk()
{
    __asm {
        push ecx
        cmp eax, 0x1000
        lea ecx, [esp + 0x8]
        jc L_4c6120
    L_4c610c:
        sub ecx, 0x1000
        sub eax, 0x1000
        test dword ptr [ecx], eax
        cmp eax, 0x1000
        jnc L_4c610c
    L_4c6120:
        sub ecx, eax
        mov eax, esp
        test dword ptr [ecx], eax
        mov esp, ecx
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [eax + 0x4]
        push eax
        ret
    }
}

// 0x004b2fa0 SysInfo_ReleaseObject0056bcbc - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_ReleaseObject0056bcbc(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x91cbc]
        test eax, eax
        jz L_4b2fb9
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx + 0x8]
        mov dword ptr [g_Data_004da000 + 0x91cbc], 0x0
    L_4b2fb9:
        ret
    }
}

// 0x004b2fc0 SysInfo_DirectSoundGetCaps - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_DirectSoundGetCaps(int, int)
{
    __asm {
        mov dword ptr [ecx], 0x60
        mov eax, dword ptr [g_Data_004da000 + 0x91cbc]
        push ecx
        push eax
        mov edx, dword ptr [eax]
        call dword ptr [edx + 0x10]
        ret
    }
}

// 0x004b2fe0 SysInfo_HasCpuid - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_HasCpuid(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push ebx
        mov dword ptr [ebp - 0x4], 0x0
        push ebx
        push ecx
        push edx
        pushfd
        pop eax
        mov ecx, eax
        xor eax, 0x200000
        push eax
        popfd
        pushfd
        pop eax
        xor eax, ecx
        mov dword ptr [ebp - 0x4], eax
        pop edx
        pop ecx
        pop ebx
        mov ecx, dword ptr [ebp - 0x4]
        xor eax, eax
        test ecx, ecx
        setnz AL
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3020 SysInfo_CpuidMmxBit - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_CpuidMmxBit(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push ebx
        push ebx
        push ecx
        push edx
        mov eax, 0x1
        cpuid
        test edx, 0x800000
        jnz L_4b3039
        xor eax, eax
    L_4b3039:
        mov dword ptr [ebp - 0x4], eax
        pop edx
        pop ecx
        pop ebx
        mov ecx, dword ptr [ebp - 0x4]
        xor eax, eax
        test ecx, ecx
        setnz AL
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3050 Cpu_IsP6Model3Plus - ../../04_spec/systems/rasteriser.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Cpu_IsP6Model3Plus(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push ebx
        push esi
        xor esi, esi
        push ebx
        push ecx
        push edx
        mov eax, 0x1
        cpuid
        mov dword ptr [ebp - 0x4], eax
        pop edx
        pop ecx
        pop ebx
        mov eax, dword ptr [ebp - 0x4]
        and eax, 0x630
        cmp eax, 0x630
        jnz L_4b307c
        mov esi, 0x1
    L_4b307c:
        mov eax, esi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3090 SysInfo_CopyInfoBlock - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_CopyInfoBlock(int, int)
{
    __asm {
        mov edx, ecx
        mov eax, offset g_SettingsNodeState_0056bcd0 + 0x14
        test edx, edx
        jz L_4b30ac
        push edi
        push esi
        mov ecx, 0xc
        mov esi, eax
        mov edi, edx
        mov eax, edx
        rep movsd
        pop esi
        pop edi
    L_4b30ac:
        ret
    }
}

// 0x004b3210 SysInfo_ReturnZero - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_ReturnZero(int, int)
{
    __asm {
        xor eax, eax
        ret
    }
}

// 0x004b3220 SysInfo_HasPositiveVideoValue - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_HasPositiveVideoValue(int, int)
{
    __asm {
        call zVideo_GetGlobal_00632f9c
        xor ecx, ecx
        test eax, eax
        setg CL
        mov eax, ecx
        ret
    }
}

// 0x004b3230 SysInfo_TotalPhysMemKB - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_TotalPhysMemKB(int, int)
{
    __asm {
        sub esp, 0x20
        lea eax, [esp]
        mov dword ptr [esp], 0x20
        push eax
        call dword ptr [g_Iat_GlobalMemoryStatus_004cc148]
        mov eax, dword ptr [esp + 0x8]
        shr eax, 0xa
        add esp, 0x20
        ret
    }
}

// 0x004b33f0 SysInfo_CanToggleEflagsId - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_CanToggleEflagsId(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        mov dword ptr [ebp - 0x4], 0x1
        pushfd
        pop eax
        mov ecx, eax
        xor eax, 0x200000
        push eax
        popfd
        pushfd
        pop eax
        xor eax, ecx
        jnz L_4b3413
        mov dword ptr [ebp - 0x4], 0x0
    L_4b3413:
        mov AX, word ptr [ebp - 0x4]
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3510 SysInfo_DivFlagsProbe - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_DivFlagsProbe(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        mov dword ptr [ebp - 0x4], 0x0
        mov AX, 0x5555
        xor DX, DX
        mov CX, 0x2
        div CX
        clc
        jnz L_4b352e
        jmp L_4b352f
    L_4b352e:
        stc
    L_4b352f:
        pushf
        pop AX
        and AL, 0x1
        xor AL, 0x1
        mov word ptr [ebp - 0x4], AX
        mov AL, byte ptr [ebp - 0x4]
        and eax, 0x1
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3550 SysInfo_Is8086Probe - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_Is8086Probe(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        mov dword ptr [ebp - 0x4], 0xffff
        pushf
        pop AX
        mov CX, AX
        and AX, 0xfff
        push AX
        popf
        pushf
        pop AX
        and AX, 0xf000
        cmp AX, 0xf000
        mov word ptr [ebp - 0x4], 0x0
        jz L_4b3584
        mov word ptr [ebp - 0x4], 0xffff
    L_4b3584:
        push CX
        popf
        mov AX, word ptr [ebp - 0x4]
        mov AX, word ptr [ebp - 0x4]
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b35a0 SysInfo_Is286Probe - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_Is286Probe(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push ebx
        mov dword ptr [ebp - 0x4], 0xffff
        pushf
        pop CX
        mov BX, CX
        or CX, 0xf000
        push CX
        popf
        pushf
        pop AX
        and AX, 0xf000
        mov word ptr [ebp - 0x4], 0x2
        jz L_4b35d2
        mov word ptr [ebp - 0x4], 0xffff
    L_4b35d2:
        push BX
        popf
        mov AX, word ptr [ebp - 0x4]
        mov AX, word ptr [ebp - 0x4]
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b35f0 SysInfo_Is386Probe - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_Is386Probe(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push ebx
        mov dword ptr [ebp - 0x4], 0xffff
        mov BX, SP
        and SP, 0xfffc
        pushfd
        pop eax
        mov ecx, eax
        xor eax, 0x40000
        push eax
        popfd
        pushfd
        pop eax
        xor eax, ecx
        mov word ptr [ebp - 0x4], 0x3
        jz L_4b3620
        mov word ptr [ebp - 0x4], 0xffff
    L_4b3620:
        push ecx
        popfd
        mov SP, BX
        mov AX, word ptr [ebp - 0x4]
        and eax, 0xffff
        mov AX, word ptr [ebp - 0x4]
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3640 SysInfo_CpuidFamily - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_CpuidFamily(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x20
        mov eax, dword ptr [g_Data_004da000 + 0xa68c]
        mov ecx, dword ptr [g_Data_004da000 + 0xa690]
        mov edx, dword ptr [g_Data_004da000 + 0xa694]
        mov dword ptr [ebp - 0x14], eax
        mov eax, dword ptr [g_Data_004da000 + 0xa67c]
        mov dword ptr [ebp - 0x10], ecx
        mov ecx, dword ptr [g_Data_004da000 + 0xa680]
        mov dword ptr [ebp - 0xc], edx
        mov edx, dword ptr [g_Data_004da000 + 0xa684]
        push ebx
        mov dword ptr [ebp - 0x8], 0xffff
        mov byte ptr [ebp - 0x1], 0x0
        mov dword ptr [ebp - 0x20], eax
        mov dword ptr [ebp - 0x1c], ecx
        mov dword ptr [ebp - 0x18], edx
        xor eax, eax
        cpuid
        mov dword ptr [ebp - 0x14], ebx
        mov dword ptr [ebp - 0x10], edx
        mov dword ptr [ebp - 0xc], ecx
        xor eax, eax
        mov ecx, 0x1
    L_4b369a:
        mov DL, byte ptr [ebp + eax*0x1 - 0x14]
        mov BL, byte ptr [ebp + eax*0x1 - 0x20]
        cmp DL, BL
        jz L_4b36ac
        mov dword ptr [g_Data_004da000 + 0x91d14], ecx
    L_4b36ac:
        inc eax
        cmp eax, 0xc
        jl L_4b369a
        cmp eax, 0x1
        jl L_4b36da
        xor eax, eax
        inc eax
        cpuid
        mov byte ptr [ebp - 0x1], AL
        and byte ptr [ebp - 0x1], 0xf
        and AL, 0xf0
        shr AL, 0x4
        mov byte ptr [ebp - 0x2], AL
        and eax, 0xf00
        shr eax, 0x8
        and eax, 0xf
        mov word ptr [ebp - 0x8], AX
    L_4b36da:
        mov AX, word ptr [ebp - 0x8]
        mov AX, word ptr [ebp - 0x8]
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b37f0 SysInfo_SpeedByBsfLoop - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall SysInfo_SpeedByBsfLoop(int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x20
        push ebx
        push esi
        lea eax, [ebp - 0x20]
        push edi
        xor ebx, ebx
        mov dword ptr [ebp - 0x8], ecx
        push eax
        or esi, 0xffffffff
        mov edi, ebx
        call dword ptr [g_Iat_QueryPerformanceFrequency_004cc154]
        test eax, eax
        jnz L_4b382c
        mov eax, dword ptr [ebp + 0x8]
        mov ecx, ebx
        mov edx, eax
        mov dword ptr [edx], edi
        mov dword ptr [edx + 0x4], ecx
        mov dword ptr [edx + 0x8], ecx
        mov dword ptr [edx + 0xc], ebx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_4b382c:
        mov edi, dword ptr [g_Iat_QueryPerformanceCounter_004cc150]
        mov dword ptr [ebp - 0x4], 0xa
    L_4b3839:
        lea eax, [ebp - 0x18]
        push eax
        call edi
        mov eax, 0x80000000
        mov BX, 0xfa0
    L_4b3848:
        bsf ecx, eax
        dec BX
        jnz L_4b3848
        lea ecx, [ebp - 0x10]
        push ecx
        call edi
        mov eax, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp - 0x18]
        sub eax, edx
        cmp eax, esi
        jnc L_4b3863
        mov esi, eax
    L_4b3863:
        mov eax, dword ptr [ebp - 0x4]
        dec eax
        mov dword ptr [ebp - 0x4], eax
        jnz L_4b3839
        lea eax, [esi + esi*0x4]
        mov ecx, dword ptr [ebp - 0x20]
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea esi, [eax + eax*0x4]
        mov eax, 0xcccccccd
        mul ecx
        shl esi, 0x5
        mov edi, edx
        mov eax, esi
        shr edi, 0x3
        xor edx, edx
        div edi
        xor edx, edx
        mov esi, eax
        div ecx
        shr ecx, 0x1
        cmp edx, ecx
        jbe L_4b38a0
        inc esi
    L_4b38a0:
        mov edi, dword ptr [ebp - 0x8]
        xor edx, edx
        mov eax, edi
        div esi
        xor edx, edx
        mov ecx, eax
        mov eax, edi
        div esi
        mov eax, esi
        mov ebx, ecx
        shr eax, 0x1
        cmp edx, eax
        jbe L_4b38bc
        inc ecx
    L_4b38bc:
        mov eax, dword ptr [ebp + 0x8]
        mov edx, eax
        mov dword ptr [edx], edi
        pop edi
        mov dword ptr [edx + 0x4], esi
        pop esi
        mov dword ptr [edx + 0x8], ebx
        pop ebx
        mov dword ptr [edx + 0xc], ecx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004b38e0 SysInfo_SpeedByTscQpc - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall SysInfo_SpeedByTscQpc(int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x38
        push ebx
        push esi
        push edi
        xor edi, edi
        xor ebx, ebx
        mov dword ptr [ebp - 0x4], edi
        mov dword ptr [ebp - 0xc], edi
        mov dword ptr [ebp - 0x10], edi
        call dword ptr [g_Iat_GetCurrentThread_004cc0e0]
        mov dword ptr [ebp - 0x18], eax
        lea eax, [ebp - 0x38]
        push eax
        xor esi, esi
        call dword ptr [g_Iat_QueryPerformanceFrequency_004cc154]
        test eax, eax
        jnz L_4b392d
        mov eax, dword ptr [ebp + 0x8]
        mov ecx, esi
        mov edx, eax
        mov dword ptr [edx], ecx
        mov dword ptr [edx + 0x4], ecx
        mov dword ptr [edx + 0x8], ecx
        mov dword ptr [edx + 0xc], esi
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_4b392a:
        mov edi, dword ptr [ebp - 0x8]
    L_4b392d:
        mov esi, dword ptr [ebp - 0x4]
        lea eax, [ebp - 0x30]
        inc esi
        push eax
        mov dword ptr [ebp - 0x4], esi
        mov esi, dword ptr [g_Iat_QueryPerformanceCounter_004cc150]
        mov dword ptr [ebp - 0x14], edi
        mov dword ptr [ebp - 0x8], ebx
        call esi
        mov edi, dword ptr [ebp - 0x18]
        mov ecx, dword ptr [ebp - 0x30]
        mov edx, dword ptr [ebp - 0x2c]
        push edi
        mov dword ptr [ebp - 0x28], ecx
        mov dword ptr [ebp - 0x24], edx
        call dword ptr [g_Iat_GetThreadPriority_004cc15c]
        mov ebx, eax
        cmp ebx, 0x7fffffff
        jz L_4b396f
        push 0xf
        push edi
        call dword ptr [g_Iat_SetThreadPriority_004cc128]
    L_4b396f:
        mov eax, dword ptr [ebp - 0x28]
        mov edx, dword ptr [ebp - 0x30]
        mov ecx, eax
        sub ecx, edx
        cmp ecx, 0x32
        jnc L_4b3998
    L_4b397e:
        lea edx, [ebp - 0x28]
        push edx
        call esi
        rdtsc
        mov dword ptr [ebp - 0x20], eax
        mov eax, dword ptr [ebp - 0x28]
        mov edx, dword ptr [ebp - 0x30]
        mov ecx, eax
        sub ecx, edx
        cmp ecx, 0x32
        jc L_4b397e
    L_4b3998:
        mov edx, dword ptr [ebp - 0x24]
        xor ecx, ecx
        cmp ecx, 0x3e8
        mov dword ptr [ebp - 0x30], eax
        mov dword ptr [ebp - 0x2c], edx
        jnc L_4b39c8
    L_4b39ab:
        lea edx, [ebp - 0x28]
        push edx
        call esi
        rdtsc
        mov dword ptr [ebp - 0x1c], eax
        mov eax, dword ptr [ebp - 0x28]
        mov edx, dword ptr [ebp - 0x30]
        mov ecx, eax
        sub ecx, edx
        cmp ecx, 0x3e8
        jc L_4b39ab
    L_4b39c8:
        cmp ebx, 0x7fffffff
        jz L_4b39db
        push ebx
        push edi
        call dword ptr [g_Iat_SetThreadPriority_004cc128]
        mov eax, dword ptr [ebp - 0x28]
    L_4b39db:
        mov edx, dword ptr [ebp - 0x30]
        mov esi, dword ptr [ebp - 0x1c]
        sub eax, edx
        mov edi, dword ptr [ebp - 0x20]
        mov ecx, dword ptr [ebp - 0x38]
        sub esi, edi
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea edi, [eax + eax*0x4]
        mov eax, 0xcccccccd
        mul ecx
        shl edi, 0x5
        mov ebx, edx
        mov eax, edi
        shr ebx, 0x3
        xor edx, edx
        div ebx
        mov edx, dword ptr [ebp - 0x10]
        mov edi, eax
        mov eax, dword ptr [ebp - 0xc]
        add edx, edi
        add eax, esi
        mov dword ptr [ebp - 0x10], edx
        mov dword ptr [ebp - 0xc], eax
        mov eax, edi
        xor edx, edx
        div ecx
        shr ecx, 0x1
        cmp edx, ecx
        jbe L_4b3a2e
        inc edi
    L_4b3a2e:
        mov eax, esi
        xor edx, edx
        div edi
        xor edx, edx
        mov ebx, eax
        mov eax, esi
        div edi
        shr edi, 0x1
        cmp edx, edi
        jbe L_4b3a43
        inc ebx
    L_4b3a43:
        mov edx, dword ptr [ebp - 0x14]
        mov eax, dword ptr [ebp - 0x8]
        lea ecx, [edx + eax*0x1]
        mov eax, dword ptr [ebp - 0x4]
        add ecx, ebx
        cmp eax, 0x3
        jl L_4b392a
        cmp eax, 0x14
        jge L_4b3a9e
        lea eax, [ebx + ebx*0x2]
        sub eax, ecx
        cdq
        xor eax, edx
        sub eax, edx
        cmp eax, 0x3
        jg L_4b392a
        mov eax, dword ptr [ebp - 0x8]
        lea eax, [eax + eax*0x2]
        sub eax, ecx
        cdq
        xor eax, edx
        sub eax, edx
        cmp eax, 0x3
        jg L_4b392a
        mov eax, dword ptr [ebp - 0x14]
        lea eax, [eax + eax*0x2]
        sub eax, ecx
        cdq
        xor eax, edx
        sub eax, edx
        cmp eax, 0x3
        jg L_4b392a
    L_4b3a9e:
        mov esi, dword ptr [ebp - 0xc]
        mov edi, dword ptr [ebp - 0x10]
        xor edx, edx
        lea eax, [esi + esi*0x4]
        shl eax, 0x1
        div edi
        xor edx, edx
        mov ecx, eax
        lea eax, [esi + esi*0x4]
        lea eax, [eax + eax*0x4]
        shl eax, 0x2
        div edi
        lea edx, [ecx + ecx*0x4]
        shl edx, 0x1
        sub eax, edx
        cmp eax, 0x6
        jc L_4b3ac9
        inc ecx
    L_4b3ac9:
        mov eax, esi
        xor edx, edx
        div edi
        mov edx, eax
        lea ebx, [eax + eax*0x4]
        shl ebx, 0x1
        sub ecx, ebx
        cmp ecx, 0x6
        jc L_4b3ae0
        lea edx, [eax + 0x1]
    L_4b3ae0:
        mov ecx, dword ptr [ebp + 0x8]
        mov ebx, ecx
        mov dword ptr [ebx], esi
        mov dword ptr [ebx + 0x4], edi
        pop edi
        pop esi
        mov dword ptr [ebx + 0x8], eax
        mov eax, ecx
        mov dword ptr [ebx + 0xc], edx
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004b3b00 SysInfo_ReadCmosSeconds - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_ReadCmosSeconds(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        mov dword ptr [ebp - 0x4], 0x0
        xor AX, AX
        out 0x70, AL
        xor AX, AX
        in AL, 0x71
        mov word ptr [ebp - 0x4], AX
        mov eax, dword ptr [ebp - 0x4]
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3b20 SysInfo_ReadTsc - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_ReadTsc(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x8
        xor eax, eax
        mov dword ptr [ebp - 0x4], eax
        mov dword ptr [ebp - 0x8], eax
        rdtsc
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0x4], edx
        mov eax, dword ptr [ebp - 0x4]
        mov dword ptr [ecx], eax
        mov eax, dword ptr [ebp - 0x8]
        mov dword ptr [edx], eax
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3ca0 SysInfo_Sub64 - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes).
__declspec(naked) int __fastcall SysInfo_Sub64(int, int, int, int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x10
        push ebx
        mov dword ptr [ebp - 0x8], edx
        mov dword ptr [ebp - 0x4], ecx
        mov eax, dword ptr [ebp + 0xc]
        mov ebx, dword ptr [ebp - 0x8]
        sub eax, ebx
        mov dword ptr [ebp - 0xc], eax
        mov eax, dword ptr [ebp + 0x8]
        mov ebx, dword ptr [ebp - 0x4]
        sbb eax, ebx
        mov dword ptr [ebp - 0x10], eax
        mov eax, dword ptr [ebp + 0x14]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp + 0x10]
        pop ebx
        mov dword ptr [eax], ecx
        mov ecx, dword ptr [ebp - 0x10]
        mov dword ptr [edx], ecx
        mov eax, dword ptr [eax]
        mov esp, ebp
        pop ebp
        ret 0x10
    }
}

// 0x004c60b0 crt_onexit - -
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall crt_onexit(int, int)
{
    __asm {
        cmp dword ptr [g_Data_004da000 + 0x29fabc], -0x1
        jnz L_4c60c8
        mov eax, dword ptr [esp + 0x4]
        push eax
        call dword ptr [g_Iat__onexit_004cc560]
        add esp, 0x4
        ret
    L_4c60c8:
        mov ecx, dword ptr [esp + 0x4]
        push offset g_Data_004da000 + 0x29fab8
        push offset g_Data_004da000 + 0x29fabc
        push ecx
        call dword ptr [g_Iat___dllonexit_004cc55c]
        add esp, 0xc
        ret
    }
}

// 0x004b3160 SysInfo_GetCpuVendorString - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_GetCpuVendorString(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xc
        push esi
        mov esi, ecx
        call SysInfo_HasCpuid
        test eax, eax
        jz L_4b319e
        push ebx
        push ebx
        push ecx
        push edx
        mov eax, 0x0
        cpuid
        mov dword ptr [ebp - 0xc], ebx
        mov dword ptr [ebp - 0x8], edx
        mov dword ptr [ebp - 0x4], ecx
        pop edx
        pop ecx
        pop ebx
        lea eax, [ebp - 0xc]
        push 0xc
        push eax
        push esi
        call dword ptr [g_Iat_strncpy_004cc5a0]
        add esp, 0xc
        mov byte ptr [esi + 0xc], 0x0
        pop ebx
    L_4b319e:
        pop esi
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b31f0 SysInfo_HasMmx - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_HasMmx(int, int)
{
    __asm {
        push esi
        xor esi, esi
        call SysInfo_HasCpuid
        test eax, eax
        jz L_4b3203
        call SysInfo_CpuidMmxBit
        mov esi, eax
    L_4b3203:
        mov eax, esi
        pop esi
        ret
    }
}

// 0x004b3420 SysInfo_GetCpuFamily - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_GetCpuFamily(int, int)
{
    __asm {
        call SysInfo_CanToggleEflagsId
        test AX, AX
        jz L_4b3431
        call SysInfo_CpuidFamily
        jmp L_4b3465
    L_4b3431:
        call SysInfo_DivFlagsProbe
        and eax, 0xffff
        mov dword ptr [g_Data_004da000 + 0x91d14], eax
        call SysInfo_Is8086Probe
        test AX, AX
        jz L_4b3465
        call SysInfo_Is286Probe
        cmp AX, 0x2
        jz L_4b3465
        call SysInfo_Is386Probe
        cmp AX, 0x3
        jz L_4b3465
        mov eax, 0x4
    L_4b3465:
        mov ecx, dword ptr [g_Data_004da000 + 0x91d14]
        test ecx, ecx
        jz L_4b3472
        or AH, 0x80
    L_4b3472:
        ret
    }
}

// 0x004b3480 SysInfo_CpuidIsGenuineIntel - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_CpuidIsGenuineIntel(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x1c
        mov eax, dword ptr [g_Data_004da000 + 0xa68c]
        mov ecx, dword ptr [g_Data_004da000 + 0xa690]
        mov edx, dword ptr [g_Data_004da000 + 0xa694]
        mov dword ptr [ebp - 0x10], eax
        mov eax, dword ptr [g_Data_004da000 + 0xa67c]
        mov dword ptr [ebp - 0xc], ecx
        mov ecx, dword ptr [g_Data_004da000 + 0xa680]
        mov dword ptr [ebp - 0x8], edx
        mov edx, dword ptr [g_Data_004da000 + 0xa684]
        mov dword ptr [ebp - 0x4], 0x0
        mov dword ptr [ebp - 0x1c], eax
        mov dword ptr [ebp - 0x18], ecx
        mov dword ptr [ebp - 0x14], edx
        call SysInfo_CanToggleEflagsId
        test AX, AX
        jz L_4b3509
        push ebx
        xor eax, eax
        cpuid
        mov dword ptr [ebp - 0x10], ebx
        mov dword ptr [ebp - 0xc], edx
        mov dword ptr [ebp - 0x8], ecx
        xor eax, eax
        mov ecx, 0x1
    L_4b34e0:
        mov DL, byte ptr [ebp + eax*0x1 - 0x10]
        mov BL, byte ptr [ebp + eax*0x1 - 0x1c]
        cmp DL, BL
        jz L_4b34f2
        mov dword ptr [g_Data_004da000 + 0x91d14], ecx
    L_4b34f2:
        inc eax
        cmp eax, 0xc
        jl L_4b34e0
        cmp eax, 0x1
        jl L_4b3505
        xor eax, eax
        inc eax
        cpuid
        mov dword ptr [ebp - 0x4], edx
    L_4b3505:
        mov eax, dword ptr [ebp - 0x4]
        pop ebx
    L_4b3509:
        mov eax, dword ptr [ebp - 0x4]
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b3b50 SysInfo_SpeedByTscCmos - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall SysInfo_SpeedByTscCmos(int, int, int)
{
    __asm {
        sub esp, 0x28
        push ebx
        push ebp
        push esi
        push edi
        call dword ptr [g_Iat_GetCurrentThread_004cc0e0]
        mov ebp, eax
        xor eax, eax
        mov dword ptr [esp + 0x28], eax
        push ebp
        mov dword ptr [esp + 0x30], eax
        mov dword ptr [esp + 0x34], eax
        mov dword ptr [esp + 0x38], eax
        call dword ptr [g_Iat_GetThreadPriority_004cc15c]
        mov ebx, eax
        cmp ebx, 0x7fffffff
        jz L_4b3b8d
        lea ecx, [ebx + 0x1]
        push ecx
        push ebp
        call dword ptr [g_Iat_SetThreadPriority_004cc128]
    L_4b3b8d:
        call SysInfo_ReadCmosSeconds
        mov esi, eax
    L_4b3b94:
        call SysInfo_ReadCmosSeconds
        mov edi, eax
        cmp edi, esi
        jge L_4b3ba6
        sub eax, esi
        add eax, 0xa
        jmp L_4b3bb1
    L_4b3ba6:
        mov edx, edi
        xor eax, eax
        sub edx, esi
        test edx, edx
        setg AL
    L_4b3bb1:
        test eax, eax
        jz L_4b3b94
        lea edx, [esp + 0x18]
        lea ecx, [esp + 0x1c]
        call SysInfo_ReadTsc
    L_4b3bc2:
        call SysInfo_ReadCmosSeconds
        mov esi, eax
        cmp esi, edi
        jge L_4b3bd4
        sub eax, edi
        add eax, 0xa
        jmp L_4b3bdf
    L_4b3bd4:
        sub eax, edi
        xor ecx, ecx
        test eax, eax
        setg CL
        mov eax, ecx
    L_4b3bdf:
        test eax, eax
        jz L_4b3bc2
        lea edx, [esp + 0x10]
        lea ecx, [esp + 0x14]
        call SysInfo_ReadTsc
        cmp ebx, 0x7fffffff
        jz L_4b3c00
        push ebx
        push ebp
        call dword ptr [g_Iat_SetThreadPriority_004cc128]
    L_4b3c00:
        mov ecx, dword ptr [esp + 0x10]
        lea edx, [esp + 0x20]
        lea eax, [esp + 0x24]
        push edx
        mov edx, dword ptr [esp + 0x18]
        push eax
        push ecx
        mov ecx, dword ptr [esp + 0x28]
        push edx
        mov edx, dword ptr [esp + 0x28]
        call SysInfo_Sub64
        mov ebp, dword ptr [esp + 0x20]
        mov eax, 0x431bde83
        mul ebp
        mov eax, 0x4f8b588f
        mov ebx, edx
        mul ebp
        mov eax, ebp
        sub eax, edx
        shr ebx, 0x12
        shr eax, 0x1
        add eax, edx
        lea ecx, [ebx + ebx*0x4]
        shr eax, 0x10
        shl ecx, 0x1
        sub eax, ecx
        mov dword ptr [esp + 0x30], ebx
        cmp eax, 0x6
        jc L_4b3c54
        inc ebx
    L_4b3c54:
        lea eax, [edi + edi*0x4]
        pop edi
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea edx, [eax + eax*0x4]
        lea eax, [esi + esi*0x4]
        shl edx, 0x6
        lea eax, [eax + eax*0x4]
        pop esi
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea eax, [eax + eax*0x4]
        lea ecx, [eax + eax*0x4]
        mov eax, dword ptr [esp + 0x34]
        shl ecx, 0x6
        sub ecx, edx
        mov edx, eax
        mov dword ptr [edx], ebp
        pop ebp
        mov dword ptr [edx + 0x4], ecx
        mov ecx, dword ptr [esp + 0x24]
        mov dword ptr [edx + 0x8], ecx
        mov dword ptr [edx + 0xc], ebx
        pop ebx
        add esp, 0x28
        ret 0x4
    }
}

// 0x004c60e0 crt_atexit - -
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall crt_atexit(int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push eax
        call crt_onexit
        add esp, 0x4
        neg eax
        sbb eax, eax
        neg eax
        dec eax
        ret
    }
}

// 0x004b31b0 SysInfo_GetCpuFamilyWord - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_GetCpuFamilyWord(int, int)
{
    __asm {
        call SysInfo_GetCpuFamily
        and eax, 0xffff
        ret
    }
}

// 0x004b36f0 SysInfo_MeasureCpuSpeed - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall SysInfo_MeasureCpuSpeed(int, int, int)
{
    __asm {
        sub esp, 0x10
        push ebx
        push ebp
        push esi
        push edi
        mov esi, ecx
        call SysInfo_GetCpuFamily
        mov edi, eax
        call SysInfo_CpuidIsGenuineIntel
        xor ebp, ebp
        xor edx, edx
        test edi, 0x8000
        mov ecx, edx
        mov ebx, edx
        jz L_4b3730
        mov eax, dword ptr [esp + 0x24]
        mov esi, eax
        mov dword ptr [esi], edx
        mov dword ptr [esi + 0x4], ecx
        mov dword ptr [esi + 0x8], ebx
        mov dword ptr [esi + 0xc], edx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret 0x4
    L_4b3730:
        test esi, esi
        jg L_4b3751
        mov ecx, edi
        and ecx, 0xffff
        mov ecx, dword ptr [ecx*0x4 + g_Data_004da000 + 0xa6a0]
        lea ecx, [ecx + ecx*0x4]
        lea ecx, [ecx + ecx*0x4]
        lea ecx, [ecx + ecx*0x4]
        shl ecx, 0x5
        jmp L_4b3770
    L_4b3751:
        cmp esi, 0x96
        jg L_4b376c
        lea ecx, [esi + esi*0x4]
        mov ebp, 0x1
        lea ecx, [ecx + ecx*0x4]
        lea ecx, [ecx + ecx*0x4]
        shl ecx, 0x5
        jmp L_4b3770
    L_4b376c:
        mov ecx, dword ptr [esp + 0x24]
    L_4b3770:
        test AL, 0x10
        jz L_4b3794
        test ebp, ebp
        jnz L_4b3794
        test esi, esi
        jnz L_4b3788
        lea edx, [esp + 0x10]
        push edx
        call SysInfo_SpeedByTscQpc
        jmp L_4b37a4
    L_4b3788:
        lea edx, [esp + 0x10]
        push edx
        call SysInfo_SpeedByTscCmos
        jmp L_4b37a4
    L_4b3794:
        cmp DI, 0x3
        jc L_4b37cc
        lea edx, [esp + 0x10]
        push edx
        call SysInfo_SpeedByBsfLoop
    L_4b37a4:
        mov ecx, eax
        mov eax, dword ptr [esp + 0x24]
        mov edx, eax
        mov esi, dword ptr [ecx]
        mov dword ptr [edx], esi
        mov esi, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], esi
        mov esi, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], esi
        mov ecx, dword ptr [ecx + 0xc]
        mov dword ptr [edx + 0xc], ecx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret 0x4
    L_4b37cc:
        mov eax, dword ptr [esp + 0x24]
        mov ecx, edx
        mov esi, eax
        pop edi
        mov dword ptr [esi], edx
        mov dword ptr [esi + 0x4], ecx
        mov dword ptr [esi + 0x8], ecx
        mov dword ptr [esi + 0xc], edx
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret 0x4
    }
}

// 0x004b31c0 SysInfo_GetField0CFrom004b36f0 - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_GetField0CFrom004b36f0(int, int)
{
    __asm {
        sub esp, 0x20
        lea eax, [esp + 0x10]
        xor ecx, ecx
        push eax
        call SysInfo_MeasureCpuSpeed
        mov ecx, dword ptr [eax]
        mov dword ptr [esp], ecx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x4], edx
        mov ecx, dword ptr [eax + 0x8]
        mov dword ptr [esp + 0x8], ecx
        mov eax, dword ptr [eax + 0xc]
        add esp, 0x20
        ret
    }
}

// 0x004b2f50 SysInfo_GetDirectSound - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_GetDirectSound(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x91cbc]
        push esi
        test eax, eax
        mov esi, ecx
        jz L_4b2f78
        cmp esi, dword ptr [g_Data_004da000 + 0x91cc8]
        jz L_4b2f98
        test eax, eax
        jz L_4b2f78
        mov ecx, dword ptr [eax]
        push eax
        call dword ptr [ecx + 0x8]
        mov dword ptr [g_Data_004da000 + 0x91cbc], 0x0
    L_4b2f78:
        push 0x0
        push offset g_Data_004da000 + 0x91cbc
        push esi
        call dword ptr [g_Iat_DSOUND_1_004cc05c]
        test eax, eax
        jnz L_4b2f96
        mov dword ptr [g_Data_004da000 + 0x91cc8], esi
        mov eax, dword ptr [g_Data_004da000 + 0x91cbc]
        pop esi
        ret
    L_4b2f96:
        xor eax, eax
    L_4b2f98:
        pop esi
        ret
    }
}

// 0x004b30b0 SysInfo_Collect - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall SysInfo_Collect(int, int)
{
    __asm {
        sub esp, 0x60
        push esi
        mov esi, ecx
        call SysInfo_GetCpuVendorString
        call SysInfo_GetCpuFamilyWord
        mov dword ptr [esi + 0x10], eax
        call SysInfo_GetField0CFrom004b36f0
        mov dword ptr [esi + 0x14], eax
        call SysInfo_HasMmx
        mov ecx, dword ptr [esi + 0x18]
        mov edx, ecx
        xor edx, eax
        and edx, 0x1
        xor edx, ecx
        mov dword ptr [esi + 0x18], edx
        call SysInfo_ReturnZero
        mov ecx, dword ptr [esi + 0x18]
        and eax, 0x1
        and ecx, 0xfffffffd
        shl eax, 0x1
        or ecx, eax
        mov dword ptr [esi + 0x18], ecx
        call SysInfo_TotalPhysMemKB
        mov dword ptr [esi + 0x1c], eax
        call SysInfo_ReturnZero
        mov edx, dword ptr [esi + 0x18]
        and eax, 0x1
        and edx, 0xfffffffb
        shl eax, 0x2
        or edx, eax
        mov dword ptr [esi + 0x18], edx
        call SysInfo_HasPositiveVideoValue
        mov ecx, dword ptr [esi + 0x18]
        and eax, 0x1
        shl eax, 0x6
        and ecx, 0xffffffbf
        or eax, ecx
        xor ecx, ecx
        mov dword ptr [esi + 0x18], eax
        call SysInfo_GetDirectSound
        test eax, eax
        jz L_4b314b
        lea ecx, [esp + 0x4]
        call SysInfo_DirectSoundGetCaps
        mov edx, dword ptr [esp + 0x48]
        shr edx, 0xa
        mov dword ptr [esi + 0x24], edx
        call SysInfo_ReleaseObject0056bcbc
    L_4b314b:
        call SysInfo_ReturnZero
        mov dword ptr [esi + 0x28], eax
        xor eax, eax
        pop esi
        add esp, 0x60
        ret
    }
}

}  // namespace recoil
