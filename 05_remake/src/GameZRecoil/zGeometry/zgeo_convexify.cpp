// SUBSYSTEM: geometry
// Original file: GameZRecoil\\zGeometry\\zgeo_convexify.cpp (ledger orig_file). Spec: 04_spec/systems/geometry.md
// Instruction-level ports converted from the Ghidra listings with tools/asm_port/ghidra2masm.py.
#include "GameZRecoil/zGeometry/zgeo_convexify.h"
#include "platform/msvcrt.h"  // MSVCRT import slots (malloc, free, _iob, fprintf)
#include "unattributed/geometry.h"  // Alloc3Ptr_Free 0x0046c720
#include "unattributed/ui_widgets.h"  // Debug_ReportNoop 0x00404e80

namespace recoil {

namespace {
const float kF_004d2680 = 0.0;  // CONFIRMED-BINARY: float 0x00000000 at 0x004d2680
const char kStr_004e050c[] = "convexify(): One or more inputs are null\n";  // CONFIRMED-DATA: .data string at 0x004e050c
const char kStr_004e0538[] = "D:\\Proj\\GameZRecoil\\zGeometry\\zgeo_convexify.cpp";  // CONFIRMED-DATA: .data string at 0x004e0538
const char kStr_004e056c[] = "convexify(): Invalid input polygon size (%d) verts.";  // CONFIRMED-DATA: .data string at 0x004e056c
const char kStr_004e05a0[] = "Error in recursive triangulate 3\n";  // CONFIRMED-DATA: .data string at 0x004e05a0
const char kStr_004e05c4[] = "Error in recursive triangulate 4\n";  // CONFIRMED-DATA: .data string at 0x004e05c4
const char kStr_004e05e8[] = "Error in recursive triangulate 2\n";  // CONFIRMED-DATA: .data string at 0x004e05e8
const char kStr_004e060c[] = "Error in recursive triangulate 1\n";  // CONFIRMED-DATA: .data string at 0x004e060c
const char kStr_004e0630[] = "Error in TRIANGULATE: only %d verts received\n";  // CONFIRMED-DATA: .data string at 0x004e0630
}  // namespace

// 0x0046ced0 Triangulate_SplitPolygon - (vertex count ECX, coordinate floats EDX, index list arg1 (stride arg3: first index = x offset, second = y offset), out parts arg2): picks the vertex with the smallest x (ties by smaller y) and its neighbours; searches the other vertices lying within the neighbours' y range and left of the farther neighbour for the one strictly inside the triangle (prev, v, next) with the smallest squared distance
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall Triangulate_SplitPolygon(int, int, int, int, int)
{
    __asm {
        sub esp, 0x3c
        push ebx
        mov ebx, dword ptr [esp + 0x44]
        push ebp
        push esi
        mov eax, dword ptr [ebx]
        push edi
        mov edi, dword ptr [esp + 0x58]
        cmp ecx, 0x1
        fld dword ptr [edx + eax*0x4]
        lea eax, [edi*0x4 + 0x0]
        mov ebp, ebx
        fstp dword ptr [esp + 0x20]
        lea esi, [ebx + eax*0x1]
        jle L_46cf57
        lea eax, [ecx - 0x1]
        mov dword ptr [esp + 0x1c], eax
    L_46cf00:
        mov eax, dword ptr [esi]
        fld dword ptr [edx + eax*0x4]
        fcomp dword ptr [esp + 0x20]
        lea edi, [edx + eax*0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_46cf33
        fld dword ptr [edi]
        fcomp dword ptr [esp + 0x20]
        fnstsw AX
        test AH, 0x40
        jz L_46cf3b
        mov eax, dword ptr [ebp + 0x4]
        mov ebx, dword ptr [esi + 0x4]
        fld dword ptr [edx + eax*0x4]
        fcomp dword ptr [edx + ebx*0x4]
        fnstsw AX
        test AH, 0x41
        jnz L_46cf3b
    L_46cf33:
        mov eax, dword ptr [edi]
        mov ebp, esi
        mov dword ptr [esp + 0x20], eax
    L_46cf3b:
        mov edi, dword ptr [esp + 0x58]
        mov ebx, dword ptr [esp + 0x1c]
        lea eax, [edi*0x4 + 0x0]
        add esi, eax
        dec ebx
        mov dword ptr [esp + 0x1c], ebx
        jnz L_46cf00
        mov ebx, dword ptr [esp + 0x50]
    L_46cf57:
        cmp ebp, ebx
        jnz L_46cf67
        lea esi, [ecx - 0x1]
        imul esi, edi
        lea esi, [ebp + esi*0x4]
        jmp L_46cf6b
    L_46cf67:
        mov esi, ebp
        sub esi, eax
    L_46cf6b:
        mov dword ptr [esp + 0x18], esi
        lea esi, [ecx - 0x1]
        imul esi, edi
        lea esi, [ebx + esi*0x4]
        cmp ebp, esi
        jnz L_46cf82
        mov dword ptr [esp + 0x1c], ebx
        jmp L_46cf8a
    L_46cf82:
        add eax, ebp
        mov dword ptr [esp + 0x1c], eax
        mov ebx, eax
    L_46cf8a:
        mov eax, dword ptr [esp + 0x18]
        mov eax, dword ptr [eax + 0x4]
        fld dword ptr [edx + eax*0x4]
        lea esi, [edx + eax*0x4]
        mov eax, dword ptr [ebx + 0x4]
        fcomp dword ptr [edx + eax*0x4]
        lea edi, [edx + eax*0x4]
        fnstsw AX
        test AH, 0x41
        jnz L_46cfad
        fld dword ptr [edi]
        mov eax, dword ptr [esi]
        jmp L_46cfb1
    L_46cfad:
        fld dword ptr [esi]
        mov eax, dword ptr [edi]
    L_46cfb1:
        mov dword ptr [esp + 0x20], eax
        lea eax, [ebp + 0x4]
        fld dword ptr [esp + 0x20]
        mov dword ptr [esp + 0x30], eax
        mov eax, dword ptr [eax]
        fcomp dword ptr [edx + eax*0x4]
        lea esi, [edx + eax*0x4]
        fnstsw AX
        test AH, 0x1
        jz L_46cfd7
        mov eax, dword ptr [esi]
        mov dword ptr [esp + 0x20], eax
        jmp L_46cfe4
    L_46cfd7:
        fcom dword ptr [esi]
        fnstsw AX
        test AH, 0x41
        jnz L_46cfe4
        fstp st(0)
        fld dword ptr [esi]
    L_46cfe4:
        mov eax, dword ptr [esp + 0x18]
        mov ebx, dword ptr [ebx]
        mov eax, dword ptr [eax]
        fld dword ptr [edx + eax*0x4]
        fcomp dword ptr [edx + ebx*0x4]
        fnstsw AX
        test AH, 0x41
        mov eax, dword ptr [esp + 0x18]
        jz L_46d001
        mov eax, dword ptr [esp + 0x1c]
    L_46d001:
        mov edi, dword ptr [esp + 0x50]
        mov dword ptr [esp + 0x14], eax
        test ecx, ecx
        mov byte ptr [esp + 0x13], 0x0
        mov dword ptr [esp + 0x28], 0x4b189680
        jle L_46d18b
        mov dword ptr [esp + 0x2c], ecx
    L_46d022:
        cmp edi, dword ptr [esp + 0x18]
        jz L_46d173
        cmp edi, ebp
        jz L_46d173
        cmp edi, dword ptr [esp + 0x1c]
        jz L_46d173
        mov eax, dword ptr [edi + 0x4]
        fld dword ptr [edx + eax*0x4]
        fcomp dword ptr [esp + 0x20]
        lea esi, [edx + eax*0x4]
        mov dword ptr [esp + 0x34], esi
        fnstsw AX
        test AH, 0x41
        jz L_46d173
        fcom dword ptr [esi]
        fnstsw AX
        test AH, 0x41
        jz L_46d173
        mov eax, dword ptr [esp + 0x14]
        mov esi, dword ptr [edi]
        mov eax, dword ptr [eax]
        fld dword ptr [edx + esi*0x4]
        fcomp dword ptr [edx + eax*0x4]
        fnstsw AX
        test AH, 0x1
        jz L_46d173
        mov eax, dword ptr [ebp]
        fld dword ptr [edx + eax*0x4]
        fsub dword ptr [edx + esi*0x4]
        fld dword ptr [edx + ebx*0x4]
        fld dword ptr [edx + ebx*0x4 + 0x4]
        fld dword ptr [edx + eax*0x4 + 0x4]
        fxch st(2)
        fsub dword ptr [edx + esi*0x4]
        fxch st(1)
        fsub dword ptr [edx + esi*0x4 + 0x4]
        fxch st(2)
        fsub dword ptr [edx + esi*0x4 + 0x4]
        fxch st(3)
        fst dword ptr [esp + 0x24]
        fld st(1)
        mov eax, dword ptr [esp + 0x18]
        fld st(3)
        fxch st(2)
        fstp dword ptr [esp + 0x44]
        mov eax, dword ptr [eax]
        fxch st(1)
        fmul dword ptr [esp + 0x44]
        fxch st(4)
        fstp dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x48]
        fld dword ptr [edx + eax*0x4 + 0x4]
        fld dword ptr [edx + eax*0x4]
        fsub dword ptr [edx + esi*0x4]
        fxch st(1)
        fsub dword ptr [edx + esi*0x4 + 0x4]
        fxch st(5)
        fxch st(1)
        fxch st(2)
        fcompp
        fstp dword ptr [esp + 0x3c]
        fnstsw AX
        fxch st(2)
        fstp dword ptr [esp + 0x40]
        test AH, 0x1
        jz L_46d16f
        fld dword ptr [esp + 0x3c]
        fld dword ptr [esp + 0x40]
        fmul st(0), st(3)
        fxch st(1)
        fmul st(0), st(2)
        fcompp
        fnstsw AX
        fstp st(0)
        test AH, 0x41
        fstp st(0)
        jz L_46d173
        fld dword ptr [esp + 0x40]
        fld dword ptr [esp + 0x3c]
        fmul dword ptr [esp + 0x48]
        fxch st(1)
        fmul dword ptr [esp + 0x44]
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_46d173
        mov eax, dword ptr [esp + 0x30]
        fld dword ptr [esp + 0x24]
        mov eax, dword ptr [eax]
        fld dword ptr [edx + eax*0x4]
        mov eax, dword ptr [esp + 0x34]
        fsub dword ptr [eax]
        fxch st(1)
        fst dword ptr [esp + 0x24]
        fld st(1)
        fxch st(1)
        fmul dword ptr [esp + 0x24]
        fxch st(1)
        fmul st(0), st(2)
        faddp st(1), st(0)
        fstp st(1)
        fld st(0)
        fcomp dword ptr [esp + 0x28]
        fnstsw AX
        test AH, 0x1
        jz L_46d171
        fstp dword ptr [esp + 0x28]
        mov dword ptr [esp + 0x38], edi
        mov byte ptr [esp + 0x13], 0x1
        jmp L_46d173
    L_46d16f:
        fstp st(0)
    L_46d171:
        fstp st(0)
    L_46d173:
        mov eax, dword ptr [esp + 0x58]
        shl eax, 0x2
        add edi, eax
        mov eax, dword ptr [esp + 0x2c]
        dec eax
        mov dword ptr [esp + 0x2c], eax
        jnz L_46d022
    L_46d18b:
        mov edx, dword ptr [esp + 0x58]
        mov eax, dword ptr [esp + 0x50]
        imul edx, ecx
        fstp st(0)
        lea ebx, [eax + edx*0x4]
        mov AL, byte ptr [esp + 0x13]
        test AL, AL
        jnz L_46d1b5
        mov ebp, dword ptr [esp + 0x18]
        mov eax, dword ptr [esp + 0x14]
        cmp eax, ebp
        jnz L_46d1bd
        mov ebp, dword ptr [esp + 0x1c]
        jmp L_46d1bd
    L_46d1b5:
        mov edx, dword ptr [esp + 0x38]
        mov dword ptr [esp + 0x14], edx
    L_46d1bd:
        mov edx, dword ptr [esp + 0x14]
        cmp edx, ebp
        jbe L_46d265
        mov esi, dword ptr [esp + 0x58]
        mov eax, edx
        sub eax, ebp
        sar eax, 0x2
        cdq
        idiv esi
        mov edx, dword ptr [esp + 0x54]
        lea edi, [edx + 0x8]
        mov dword ptr [esp + 0x54], edi
        inc eax
        sub ecx, eax
        mov dword ptr [edx], eax
        add ecx, 0x2
        mov dword ptr [edx + 0x4], ecx
        mov ecx, esi
        imul ecx, eax
        shl ecx, 0x2
        mov eax, ecx
        mov esi, ebp
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        mov ecx, dword ptr [esp + 0x58]
        mov esi, dword ptr [esp + 0x14]
        imul ecx, dword ptr [edx]
        mov edx, dword ptr [esp + 0x54]
        sub ebx, esi
        sar ebx, 0x2
        shl ebx, 0x2
        lea edx, [edx + ecx*0x4]
        mov ecx, ebx
        mov eax, ecx
        mov edi, edx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        mov eax, dword ptr [esp + 0x58]
        and ecx, 0x3
        rep movsb
        mov esi, dword ptr [esp + 0x50]
        mov ecx, ebp
        sub ecx, esi
        lea edi, [edx + ebx*0x1]
        sar ecx, 0x2
        add ecx, eax
        mov eax, 0x1
        shl ecx, 0x2
        mov edx, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x3c
        ret 0xc
    L_46d265:
        mov esi, dword ptr [esp + 0x58]
        mov eax, ebp
        sub eax, edx
        sub ebx, ebp
        sar eax, 0x2
        cdq
        idiv esi
        mov edx, dword ptr [esp + 0x54]
        sar ebx, 0x2
        lea edi, [edx + 0x8]
        mov dword ptr [esp + 0x54], edi
        shl ebx, 0x2
        inc eax
        sub ecx, eax
        mov dword ptr [edx], eax
        add ecx, 0x2
        mov dword ptr [edx + 0x4], ecx
        mov ecx, esi
        imul ecx, eax
        mov esi, dword ptr [esp + 0x14]
        shl ecx, 0x2
        mov eax, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        mov eax, dword ptr [esp + 0x58]
        and ecx, 0x3
        rep movsb
        mov ecx, eax
        mov esi, ebp
        imul ecx, dword ptr [edx]
        mov edx, dword ptr [esp + 0x54]
        lea edx, [edx + ecx*0x4]
        mov ecx, ebx
        mov ebp, ecx
        mov edi, edx
        shr ecx, 0x2
        rep movsd
        mov ecx, ebp
        and ecx, 0x3
        rep movsb
        mov ecx, dword ptr [esp + 0x14]
        mov esi, dword ptr [esp + 0x50]
        sub ecx, esi
        lea edi, [edx + ebx*0x1]
        sar ecx, 0x2
        add ecx, eax
        shl ecx, 0x2
        mov eax, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        mov eax, 0x1
        and ecx, 0x3
        rep movsb
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x3c
        ret 0xc
    }
}

// 0x0046cb50 Triangulate_PolygonRecursive - (vertex count ECX, vertex data arg1 or null for identity indices, stride mode arg2: 1 -> 2 dwords per vertex else 3): count < 3 -> fprintf 'Error in TRIANGULATE, only %d vertices' and 0. Exactly 3 -> malloc {1, identity index list}. Else work buffer {n, data} and result {n-2, ...} (mallocs unchecked); 0x0046ced0 splits the polygon into two parts (sizes at work +0/+4); failure -> free both, 0. Part
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall Triangulate_PolygonRecursive(int, int, int, int)
{
    __asm {
        sub esp, 0xc
        push ebx
        push ebp
        push esi
        mov esi, ecx
        cmp esi, 0x3
        push edi
        mov dword ptr [esp + 0x14], edx
        jge L_46cb86
        mov eax, [g_Iat__iob_004cc4f8]
        push esi
        add eax, 0x40
        push offset kStr_004e0630
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0xc
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x8
    L_46cb86:
        mov edx, dword ptr [esp + 0x24]
        xor ebx, ebx
        cmp edx, 0x1
        setnz BL
        add ebx, 0x2
        cmp esi, 0x3
        jnz L_46cbd8
        lea ecx, [ebx + ebx*0x2]
        lea edx, [ecx*0x4 + 0x4]
        push edx
        call dword ptr [g_Iat_malloc_004cc5dc]
        lea ebx, [ebx + ebx*0x2 - 0x1]
        add esp, 0x4
        test ebx, ebx
        mov dword ptr [eax], 0x1
        jl L_46cec1
        lea ecx, [eax + ebx*0x4 + 0x4]
    L_46cbc4:
        mov dword ptr [ecx], ebx
        dec ebx
        sub ecx, 0x4
        test ebx, ebx
        jge L_46cbc4
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x8
    L_46cbd8:
        lea eax, [esi + 0x2]
        mov edi, dword ptr [g_Iat_malloc_004cc5dc]
        imul eax, ebx
        lea ecx, [eax*0x4 + 0x8]
        push ecx
        call edi
        mov ebp, eax
        lea eax, [esi - 0x1]
        imul eax, ebx
        add esp, 0x4
        lea edx, [eax + eax*0x2]
        lea eax, [edx*0x4 + 0x4]
        push eax
        call edi
        lea ecx, [esi - 0x2]
        add esp, 0x4
        mov dword ptr [eax], ecx
        mov ecx, dword ptr [esp + 0x20]
        lea edi, [eax + 0x4]
        mov dword ptr [esp + 0x10], eax
        test ecx, ecx
        mov dword ptr [esp + 0x18], edi
        jz L_46cc40
        mov edx, ebx
        imul edx, esi
        dec edx
        js L_46cc55
        lea eax, [edi + edx*0x4]
        sub ecx, edi
        inc edx
    L_46cc2f:
        mov edi, dword ptr [ecx + eax*0x1]
        mov dword ptr [eax], edi
        sub eax, 0x4
        dec edx
        jnz L_46cc2f
        mov edi, dword ptr [esp + 0x18]
        jmp L_46cc55
    L_46cc40:
        mov eax, ebx
        imul eax, esi
        dec eax
        js L_46cc55
        lea ecx, [edi + eax*0x4]
    L_46cc4b:
        mov dword ptr [ecx], eax
        dec eax
        sub ecx, 0x4
        test eax, eax
        jge L_46cc4b
    L_46cc55:
        mov edx, dword ptr [esp + 0x14]
        push ebx
        push ebp
        push edi
        mov ecx, esi
        mov dword ptr [esp + 0x2c], edi
        mov dword ptr [ebp], esi
        call Triangulate_SplitPolygon
        test eax, eax
        jnz L_46cc90
        mov esi, dword ptr [g_Iat_free_004cc5b4]
        push ebp
        call esi
        mov edx, dword ptr [esp + 0x14]
        add esp, 0x4
        push edx
        call esi
        add esp, 0x4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x8
    L_46cc90:
        mov ecx, dword ptr [ebp]
        cmp ecx, 0x3
        jz L_46cc9e
        cmp dword ptr [ebp + 0x4], 0x3
        jz L_46ccbe
    L_46cc9e:
        mov eax, dword ptr [ebp + 0x4]
        lea esi, [ebp + 0x8]
        add ecx, eax
        imul ecx, ebx
        shl ecx, 0x2
        mov eax, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        mov edi, dword ptr [esp + 0x20]
    L_46ccbe:
        mov esi, dword ptr [ebp]
        cmp esi, 0x3
        jnz L_46ccfc
        mov ecx, dword ptr [ebp + 0x4]
        cmp ecx, esi
        jz L_46ce46
        mov eax, dword ptr [esp + 0x24]
        lea edx, [ebx + ebx*0x2]
        push eax
        lea edi, [edi + edx*0x4]
        lea edx, [ebx + ebx*0x2]
        lea eax, [ebp + edx*0x4 + 0x8]
        mov edx, dword ptr [esp + 0x18]
        push eax
        call Triangulate_PolygonRecursive
        test eax, eax
        jnz L_46ce22
        push offset kStr_004e060c
        jmp L_46cd7b
    L_46ccfc:
        cmp dword ptr [ebp + 0x4], 0x3
        jnz L_46cdaf
        mov edx, ebx
        lea eax, [ebx + ebx*0x2]
        imul edx, esi
        shl eax, 0x2
        mov ecx, eax
        lea esi, [ebp + edx*0x4 + 0x8]
        mov edx, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov ecx, ebx
        mov edi, dword ptr [esp + 0x20]
        imul ecx, dword ptr [ebp]
        shl ecx, 0x2
        add edi, eax
        lea eax, [ebp + 0x8]
        mov edx, ecx
        mov esi, eax
        shr ecx, 0x2
        mov dword ptr [esp + 0x20], edi
        rep movsd
        mov ecx, edx
        mov edx, dword ptr [esp + 0x14]
        and ecx, 0x3
        rep movsb
        mov ecx, dword ptr [esp + 0x24]
        push ecx
        mov ecx, dword ptr [ebp]
        push eax
        call Triangulate_PolygonRecursive
        test eax, eax
        jz L_46cd76
        imul ebx, dword ptr [eax]
        mov edi, dword ptr [esp + 0x20]
        lea esi, [eax + 0x4]
        lea ecx, [ebx + ebx*0x2]
        shl ecx, 0x2
        jmp L_46ce2e
    L_46cd76:
        push offset kStr_004e05e8
    L_46cd7b:
        mov eax, [g_Iat__iob_004cc4f8]
        add eax, 0x40
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        mov esi, dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x8
        push ebp
        call esi
        mov ecx, dword ptr [esp + 0x14]
        add esp, 0x4
        push ecx
        call esi
        add esp, 0x4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x8
    L_46cdaf:
        mov edx, dword ptr [esp + 0x24]
        lea eax, [ebp + 0x8]
        push edx
        mov edx, dword ptr [esp + 0x18]
        push eax
        mov ecx, esi
        call Triangulate_PolygonRecursive
        test eax, eax
        jz L_46ce91
        mov ecx, ebx
        lea esi, [eax + 0x4]
        imul ecx, dword ptr [eax]
        push eax
        lea ecx, [ecx + ecx*0x2]
        shl ecx, 0x2
        mov edx, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        mov edx, dword ptr [esp + 0x24]
        and ecx, 0x3
        rep movsb
        mov ecx, ebx
        mov esi, dword ptr [g_Iat_free_004cc5b4]
        imul ecx, dword ptr [eax]
        lea ecx, [ecx + ecx*0x2]
        lea edi, [edx + ecx*0x4]
        call esi
        mov ecx, ebx
        mov eax, dword ptr [esp + 0x28]
        imul ecx, dword ptr [ebp]
        add esp, 0x4
        lea edx, [ebp + ecx*0x4 + 0x8]
        mov ecx, dword ptr [ebp + 0x4]
        push eax
        push edx
        mov edx, dword ptr [esp + 0x1c]
        call Triangulate_PolygonRecursive
        test eax, eax
        jz L_46ce5e
    L_46ce22:
        imul ebx, dword ptr [eax]
        lea esi, [eax + 0x4]
        lea ecx, [ebx + ebx*0x2]
        shl ecx, 0x2
    L_46ce2e:
        mov edx, ecx
        push eax
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46ce46:
        push ebp
        call dword ptr [g_Iat_free_004cc5b4]
        mov eax, dword ptr [esp + 0x14]
        add esp, 0x4
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x8
    L_46ce5e:
        mov eax, [g_Iat__iob_004cc4f8]
        push offset kStr_004e05c4
        add eax, 0x40
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x8
        push ebp
        call esi
        mov ecx, dword ptr [esp + 0x14]
        add esp, 0x4
        push ecx
        call esi
        add esp, 0x4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x8
    L_46ce91:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push offset kStr_004e05a0
        add edx, 0x40
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        mov esi, dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x8
        push ebp
        call esi
        mov eax, dword ptr [esp + 0x14]
        add esp, 0x4
        push eax
        call esi
        add esp, 0x4
        xor eax, eax
    L_46cec1:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x8
    }
}

// 0x0046c760 Convexify - (contour list ECX {count, headers (n, start)}, total vertex count EDX, xyz data arg): EDX < 1 or null data -> fprintf 'convexify(): One or more inputs are invalid', 0. Result malloc(0x10) {pieces, headers malloc(EDX*8-0x10), vertex total, xyz malloc(EDX*0x24-0x48)} (unchecked, sized for EDX-2 triangles). Per contour: < 3 skipped; 3 -> copied as one triangle; 4 -> finds the first vertex with a posi
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall Convexify(int, int, int)
{
    __asm {
        sub esp, 0x18
        push ebx
        push ebp
        mov ebp, edx
        push esi
        test ebp, ebp
        push edi
        mov dword ptr [esp + 0x1c], ecx
        jle L_46cb29
        mov esi, dword ptr [esp + 0x2c]
        test esi, esi
        jz L_46cb29
        mov edi, dword ptr [g_Iat_malloc_004cc5dc]
        push 0x10
        call edi
        mov ebx, eax
        lea eax, [ebp + ebp*0x8]
        add esp, 0x4
        lea ecx, [eax*0x4 + 0xffffffb8]
        push ecx
        call edi
        add esp, 0x4
        lea edx, [ebp*0x8 + 0xfffffff0]
        mov dword ptr [ebx + 0xc], eax
        push edx
        call edi
        mov ecx, dword ptr [ebx + 0xc]
        mov dword ptr [ebx + 0x4], eax
        mov dword ptr [esp + 0x14], ecx
        mov ecx, dword ptr [esp + 0x20]
        mov dword ptr [ebx + 0x8], 0x0
        mov dword ptr [ebx], 0x0
        lea ebp, [eax - 0x8]
        mov eax, dword ptr [ecx + 0x4]
        mov ecx, dword ptr [ecx]
        add esp, 0x4
        test ecx, ecx
        mov dword ptr [esp + 0x14], eax
        mov dword ptr [esp + 0x1c], ecx
        jz L_46caeb
    L_46c7e4:
        mov ecx, dword ptr [eax]
        cmp ecx, 0x3
        jc L_46cad3
        jnz L_46c835
        add ebp, 0x8
        mov dword ptr [ebp], 0x3
        mov ecx, dword ptr [ebx + 0x8]
        lea edx, [ecx + ecx*0x2]
        mov dword ptr [ebp + 0x4], edx
        mov edi, dword ptr [ebx]
        mov edx, dword ptr [ebx + 0x8]
        inc edi
        add edx, 0x3
        mov dword ptr [ebx], edi
        mov dword ptr [ebx + 0x8], edx
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [esp + 0x10]
        lea esi, [esi + ecx*0x4]
        mov ecx, 0x9
        mov edi, edx
        add edx, 0x24
        rep movsd
        mov esi, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x10], edx
        jmp L_46cad3
    L_46c835:
        cmp ecx, 0x4
        jnz L_46ca47
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x24], 0xffffffff
        lea eax, [esi + edx*0x4]
        mov ecx, eax
        mov dword ptr [esp + 0x20], eax
        lea edx, [ecx + 0xc]
        lea edi, [edx + 0xc]
        mov dword ptr [esp + 0x18], edi
        xor edi, edi
        jmp L_46c864
    L_46c860:
        mov eax, dword ptr [esp + 0x20]
    L_46c864:
        cmp edi, 0x2
        jnz L_46c86f
        mov dword ptr [esp + 0x18], eax
        jmp L_46c876
    L_46c86f:
        cmp edi, 0x3
        jnz L_46c876
        mov edx, eax
    L_46c876:
        mov eax, dword ptr [esp + 0x18]
        fld dword ptr [edx + 0x4]
        fld dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fld dword ptr [edx]
        fxch st(2)
        fsub dword ptr [ecx]
        fxch st(2)
        fsub dword ptr [ecx]
        fxch st(3)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(2)
        fmulp st(1), st(0)
        fxch st(2)
        fmulp st(1), st(0)
        fsubp st(1), st(0)
        fcomp dword ptr [kF_004d2680]
        fnstsw AX
        test AH, 0x41
        jz L_46c8ca
        mov eax, dword ptr [esp + 0x18]
        add ecx, 0xc
        add edx, 0xc
        add eax, 0xc
        inc edi
        mov dword ptr [esp + 0x18], eax
        cmp edi, 0x4
        jl L_46c860
        mov eax, dword ptr [esp + 0x24]
        jmp L_46c8cd
    L_46c8ca:
        lea eax, [edi + 0x1]
    L_46c8cd:
        cdq
        xor eax, edx
        mov edi, 0x3
        sub eax, edx
        and eax, edi
        xor eax, edx
        sub eax, edx
        jns L_46c92b
        add ebp, 0x8
        mov ecx, dword ptr [esp + 0x14]
        mov dword ptr [ebp], 0x4
        mov eax, dword ptr [ebx + 0x8]
        lea eax, [eax + eax*0x2]
        mov dword ptr [ebp + 0x4], eax
        mov edi, dword ptr [ebx]
        mov edx, dword ptr [ebx + 0x8]
        mov eax, dword ptr [esp + 0x10]
        inc edi
        add edx, 0x4
        mov dword ptr [ebx], edi
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [ecx + 0x4]
        mov ecx, 0xc
        mov edi, eax
        add eax, 0x30
        lea esi, [esi + edx*0x4]
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esp + 0x14]
        rep movsd
        mov esi, dword ptr [esp + 0x2c]
        jmp L_46cad3
    L_46c92b:
        mov edx, dword ptr [ebx]
        add edx, 0x2
        test AL, 0x1
        mov dword ptr [ebx], edx
        jz L_46c9b5
        add ebp, 0x8
        mov dword ptr [ebp], edi
        mov eax, dword ptr [ebx + 0x8]
        add ebp, 0x8
        lea eax, [eax + eax*0x2]
        mov dword ptr [ebp - 0x4], eax
        mov edx, dword ptr [ebx + 0x8]
        add edx, edi
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [esp + 0x14]
        mov edi, eax
        mov ecx, dword ptr [edx + 0x4]
        lea esi, [esi + ecx*0x4]
        mov ecx, 0x6
        rep movsd
        mov ecx, dword ptr [edx + 0x4]
        mov esi, dword ptr [esp + 0x2c]
        lea ecx, [esi + ecx*0x4 + 0x24]
        lea esi, [eax + 0x18]
        add eax, 0x24
        mov edi, dword ptr [ecx]
        mov dword ptr [esi], edi
        mov edi, dword ptr [ecx + 0x4]
        mov dword ptr [esi + 0x4], edi
        mov edi, eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [esi + 0x8], ecx
        mov dword ptr [ebp], 0x3
        mov ecx, dword ptr [ebx + 0x8]
        lea ecx, [ecx + ecx*0x2]
        mov dword ptr [ebp + 0x4], ecx
        mov esi, dword ptr [ebx + 0x8]
        mov ecx, dword ptr [esp + 0x2c]
        add esi, 0x3
        mov dword ptr [ebx + 0x8], esi
        mov edx, dword ptr [edx + 0x4]
        lea esi, [ecx + edx*0x4 + 0xc]
        mov ecx, 0x9
        jmp L_46ca31
    L_46c9b5:
        add ebp, 0x8
        mov ecx, 0x9
        mov dword ptr [ebp], edi
        mov eax, dword ptr [ebx + 0x8]
        add ebp, 0x8
        lea edx, [eax + eax*0x2]
        mov dword ptr [ebp - 0x4], edx
        mov edx, dword ptr [ebx + 0x8]
        add edx, edi
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [esp + 0x14]
        mov eax, dword ptr [edx + 0x4]
        lea esi, [esi + eax*0x4]
        mov eax, dword ptr [esp + 0x10]
        mov edi, eax
        add eax, 0x24
        rep movsd
        mov dword ptr [ebp], 0x3
        mov ecx, dword ptr [ebx + 0x8]
        lea ecx, [ecx + ecx*0x2]
        mov dword ptr [ebp + 0x4], ecx
        mov esi, dword ptr [ebx + 0x8]
        add esi, 0x3
        mov dword ptr [ebx + 0x8], esi
        mov ecx, dword ptr [edx + 0x4]
        mov esi, dword ptr [esp + 0x2c]
        lea ecx, [esi + ecx*0x4]
        mov esi, eax
        mov edi, dword ptr [ecx]
        mov dword ptr [esi], edi
        mov edi, dword ptr [ecx + 0x4]
        mov dword ptr [esi + 0x4], edi
        lea edi, [eax + 0xc]
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [esi + 0x8], ecx
        mov edx, dword ptr [edx + 0x4]
        mov ecx, dword ptr [esp + 0x2c]
        lea esi, [ecx + edx*0x4 + 0x18]
        mov ecx, 0x6
    L_46ca31:
        add eax, 0x24
        rep movsd
        mov esi, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x10], eax
        mov eax, dword ptr [esp + 0x14]
        jmp L_46cad3
    L_46ca47:
        jbe L_46caf7
        mov edx, dword ptr [eax + 0x4]
        push 0x0
        push 0x0
        lea edx, [esi + edx*0x4]
        call Triangulate_PolygonRecursive
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ebx]
        add edx, ecx
        mov dword ptr [esp + 0x24], eax
        mov dword ptr [ebx], edx
        mov edx, dword ptr [eax]
        test edx, edx
        lea ecx, [eax + 0x4]
        jz L_46cac5
        mov dword ptr [esp + 0x18], edx
    L_46ca75:
        add ebp, 0x8
        mov edi, 0x9
        mov dword ptr [ebp], 0x3
        mov edx, dword ptr [ebx + 0x8]
        lea edx, [edx + edx*0x2]
        mov dword ptr [ebp + 0x4], edx
        mov eax, dword ptr [ebx + 0x8]
        add eax, 0x3
        mov dword ptr [ebx + 0x8], eax
    L_46ca96:
        mov edx, dword ptr [esp + 0x14]
        mov eax, dword ptr [ecx]
        add ecx, 0x4
        add eax, dword ptr [edx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        add edx, 0x4
        dec edi
        mov dword ptr [esp + 0x10], edx
        mov eax, dword ptr [esi + eax*0x4]
        mov dword ptr [edx - 0x4], eax
        jnz L_46ca96
        mov eax, dword ptr [esp + 0x18]
        dec eax
        mov dword ptr [esp + 0x18], eax
        jnz L_46ca75
        mov eax, dword ptr [esp + 0x24]
    L_46cac5:
        push eax
        call dword ptr [g_Iat_free_004cc5b4]
        mov eax, dword ptr [esp + 0x18]
        add esp, 0x4
    L_46cad3:
        mov ecx, dword ptr [esp + 0x1c]
        add eax, 0x8
        dec ecx
        mov dword ptr [esp + 0x14], eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], ecx
        jnz L_46c7e4
    L_46caeb:
        mov eax, ebx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret 0x4
    L_46caf7:
        mov ecx, dword ptr [eax]
        push ecx
        push offset kStr_004e056c
        push 0x38b
        push offset kStr_004e0538
        push 0x100
        call Debug_ReportNoop
        add esp, 0x14
        mov ecx, ebx
        call Alloc3Ptr_Free
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret 0x4
    L_46cb29:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push offset kStr_004e050c
        add edx, 0x40
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x8
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret 0x4
    }
}

}  // namespace recoil
