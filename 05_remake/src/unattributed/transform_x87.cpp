// SUBSYSTEM: transform
// Instruction-level x87 ports of transform functions (decision D1: fsin/fcos/fsincos results are 64-bit
// precision whatever the precision control, and the register sequences are long). Each body was converted
// from the Ghidra listing with tools/asm_port/ghidra2masm.py; every absolute reference is mapped to a
// tagged constant or to the port's own global (unmapped references are an error in the converter).
// Spec: 04_spec/systems/transform_stack.md
#include "unattributed/transform.h"
#include "unattributed/math3d.h"
#include "unattributed/view.h"

namespace recoil {

namespace {
const double kSinCosLimitD_2968 = 9.22e18;  // CONFIRMED-BINARY: double 0x43dffd01499f4680 at 0x004d2968 (fsin range guard)
const double kSinCosLimitD_29b8 = 9.22e18;  // CONFIRMED-BINARY: double 0x43dffd01499f4680 at 0x004d29b8
const double kZeroD_2970 = 0.0;             // CONFIRMED-BINARY: double 0x0000000000000000 at 0x004d2970
const double kOneD_2950 = 1.0;              // CONFIRMED-BINARY: double 0x3ff0000000000000 at 0x004d2950
const float kOneF_297c = 1.0f;              // CONFIRMED-BINARY: float 0x3f800000 at 0x004d297c
const float kHalfF_29b0 = 0.5f;             // CONFIRMED-BINARY: float 0x3f000000 at 0x004d29b0
}  // namespace


// 0x00473370 zTransformConcatenateLocal (ECX src 12 floats, EDX mode): identity level -> copy; mode 2 -> rotation only;
// else current := src . current (row vectors). Flag := 0.
__declspec(naked) void __fastcall zTransformConcatenateLocal(const float*, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x3c
        mov eax, [g_TransformFlagSP_004e0e84]
        push esi
        mov esi, ecx
        push edi
        mov ecx, dword ptr [eax]
        mov dword ptr [ebp - 0x4], esi
        test ecx, ecx
        jz L_4733a8
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov ecx, 0xc
        mov edi, dword ptr [edx]
        rep movsd
        mov eax, [g_TransformFlagSP_004e0e84]
        mov dword ptr [eax], 0x0
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    L_4733a8:
        cmp edx, 0x2
        push ebx
        lea eax, [ebp - 0x3c]
        jnz L_4734d3
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov dword ptr [ebp - 0x8], eax
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0xc], edx
        mov eax, dword ptr [ebp - 0x4]
        mov ebx, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp - 0x8]
        fld dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fld dword ptr [eax + 0x8]
        fld dword ptr [ebx]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0xc]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0x18]
        fmul st(0), st(3)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x4]
        fmul st(0), st(5)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x10]
        fmul st(0), st(4)
        fxch st(2)
        fstp dword ptr [ecx]
        faddp st(1), st(0)
        fld dword ptr [ebx + 0x1c]
        fmul st(0), st(2)
        fxch st(4)
        fmul dword ptr [ebx + 0x8]
        fxch st(4)
        faddp st(1), st(0)
        fxch st(1)
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        fmul dword ptr [ebx + 0x14]
        fxch st(1)
        fstp dword ptr [ecx + 0x4]
        faddp st(1), st(0)
        fld dword ptr [eax + 0xc]
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x14]
        fld dword ptr [eax + 0x10]
        fxch st(2)
        fstp dword ptr [ecx + 0x8]
        fld dword ptr [ebx]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0xc]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0x18]
        fmul st(0), st(3)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x4]
        fmul st(0), st(5)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x10]
        fmul st(0), st(4)
        fxch st(2)
        fstp dword ptr [ecx + 0xc]
        faddp st(1), st(0)
        fld dword ptr [ebx + 0x1c]
        fmul st(0), st(2)
        fxch st(4)
        fmul dword ptr [ebx + 0x8]
        fxch st(4)
        faddp st(1), st(0)
        fxch st(1)
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        fmul dword ptr [ebx + 0x14]
        fxch st(1)
        fstp dword ptr [ecx + 0x10]
        faddp st(1), st(0)
        fld dword ptr [eax + 0x18]
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x20]
        fld dword ptr [eax + 0x1c]
        fxch st(2)
        fstp dword ptr [ecx + 0x14]
        fld dword ptr [ebx]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0xc]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0x18]
        fmul st(0), st(3)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x4]
        fmul st(0), st(5)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x10]
        fmul st(0), st(4)
        fxch st(2)
        fstp dword ptr [ecx + 0x18]
        faddp st(1), st(0)
        fld dword ptr [ebx + 0x1c]
        fmul st(0), st(2)
        fxch st(4)
        fmul dword ptr [ebx + 0x8]
        fxch st(4)
        faddp st(1), st(0)
        fxch st(1)
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        fmul dword ptr [ebx + 0x14]
        fxch st(1)
        fstp dword ptr [ecx + 0x1c]
        faddp st(1), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ecx + 0x20]
        // KG-25 deviation: temp words 9..11 := current translation (the original leaves them unwritten)
        mov edx, dword ptr [ebx + 0x24]
        mov dword ptr [ecx + 0x24], edx
        mov edx, dword ptr [ebx + 0x28]
        mov dword ptr [ecx + 0x28], edx
        mov edx, dword ptr [ebx + 0x2c]
        mov dword ptr [ecx + 0x2c], edx
        jmp L_473653
    L_4734d3:
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov dword ptr [ebp - 0xc], eax
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0x8], edx
        mov eax, dword ptr [ebp - 0x4]
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        fld dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fld dword ptr [eax + 0x8]
        fld dword ptr [ebx]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0xc]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0x18]
        fmul st(0), st(3)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x4]
        fmul st(0), st(5)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x10]
        fmul st(0), st(4)
        fxch st(2)
        fstp dword ptr [ecx]
        faddp st(1), st(0)
        fld dword ptr [ebx + 0x1c]
        fmul st(0), st(2)
        fxch st(4)
        fmul dword ptr [ebx + 0x8]
        fxch st(4)
        faddp st(1), st(0)
        fxch st(1)
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        fmul dword ptr [ebx + 0x14]
        fxch st(1)
        fstp dword ptr [ecx + 0x4]
        faddp st(1), st(0)
        fld dword ptr [eax + 0xc]
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x14]
        fld dword ptr [eax + 0x10]
        fxch st(2)
        fstp dword ptr [ecx + 0x8]
        fld dword ptr [ebx]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0xc]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0x18]
        fmul st(0), st(3)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x4]
        fmul st(0), st(5)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x10]
        fmul st(0), st(4)
        fxch st(2)
        fstp dword ptr [ecx + 0xc]
        faddp st(1), st(0)
        fld dword ptr [ebx + 0x1c]
        fmul st(0), st(2)
        fxch st(4)
        fmul dword ptr [ebx + 0x8]
        fxch st(4)
        faddp st(1), st(0)
        fxch st(1)
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        fmul dword ptr [ebx + 0x14]
        fxch st(1)
        fstp dword ptr [ecx + 0x10]
        faddp st(1), st(0)
        fld dword ptr [eax + 0x18]
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x20]
        fld dword ptr [eax + 0x1c]
        fxch st(2)
        fstp dword ptr [ecx + 0x14]
        fld dword ptr [ebx]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0xc]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0x18]
        fmul st(0), st(3)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x4]
        fmul st(0), st(5)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x10]
        fmul st(0), st(4)
        fxch st(2)
        fstp dword ptr [ecx + 0x18]
        faddp st(1), st(0)
        fld dword ptr [ebx + 0x1c]
        fmul st(0), st(2)
        fxch st(4)
        fmul dword ptr [ebx + 0x8]
        fxch st(4)
        faddp st(1), st(0)
        fxch st(1)
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        fmul dword ptr [ebx + 0x14]
        fxch st(1)
        fstp dword ptr [ecx + 0x1c]
        faddp st(1), st(0)
        fld dword ptr [eax + 0x24]
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x2c]
        fld dword ptr [eax + 0x28]
        fxch st(2)
        fstp dword ptr [ecx + 0x20]
        fld dword ptr [ebx]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0xc]
        fmul st(0), st(3)
        fld dword ptr [ebx + 0x18]
        fmul st(0), st(3)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x4]
        fmul st(0), st(5)
        fxch st(1)
        faddp st(2), st(0)
        fld dword ptr [ebx + 0x10]
        fmul st(0), st(4)
        fxch st(2)
        fadd dword ptr [ebx + 0x24]
        fxch st(1)
        faddp st(2), st(0)
        fstp dword ptr [ecx + 0x24]
        fld dword ptr [ebx + 0x1c]
        fmul st(0), st(2)
        fxch st(4)
        fmul dword ptr [ebx + 0x8]
        fxch st(1)
        faddp st(4), st(0)
        fxch st(1)
        fmul dword ptr [ebx + 0x20]
        fxch st(3)
        fadd dword ptr [ebx + 0x28]
        fxch st(2)
        fmul dword ptr [ebx + 0x14]
        fxch st(1)
        fadd dword ptr [ebx + 0x2c]
        fxch st(2)
        fstp dword ptr [ecx + 0x28]
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ecx + 0x2c]
    L_473653:
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        lea eax, [ebp - 0x3c]
        mov dword ptr [ebp - 0x4], 0xc
        mov dword ptr [ebp - 0xc], eax
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0x8], edx
        mov ecx, dword ptr [ebp - 0x4]
        mov edi, dword ptr [ebp - 0x8]
        mov esi, dword ptr [ebp - 0xc]
        rep movsd
        mov eax, [g_TransformFlagSP_004e0e84]
        pop ebx
        pop edi
        pop esi
        mov dword ptr [eax], 0x0
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00473690 zTransformScaleLocal (stack sx, sy, sz; ret 0xc): identity level -> diagonal; else rows scaled.
__declspec(naked) void __stdcall zTransformScaleLocal(float, float, float)
{
    __asm {
        mov eax, [g_TransformFlagSP_004e0e84]
        sub esp, 0x30
        cmp dword ptr [eax], 0x0
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        jz L_4736da
        mov edx, dword ptr [ecx]
        mov eax, dword ptr [esp + 0x34]
        mov dword ptr [edx], eax
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov eax, dword ptr [esp + 0x38]
        mov edx, dword ptr [ecx]
        mov dword ptr [edx + 0x10], eax
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov eax, dword ptr [esp + 0x3c]
        mov edx, dword ptr [ecx]
        mov dword ptr [edx + 0x20], eax
        mov eax, [g_TransformFlagSP_004e0e84]
        mov dword ptr [eax], 0x0
        add esp, 0x30
        ret 0xc
    L_4736da:
        mov eax, dword ptr [ecx]
        fld dword ptr [esp + 0x34]
        fmul dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [esp + 0x34]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [esp + 0x34]
        fld dword ptr [eax + 0xc]
        fld dword ptr [eax + 0x10]
        fld dword ptr [eax + 0x14]
        fxch st(4)
        fst dword ptr [esp + 0x4]
        fxch st(3)
        fst dword ptr [esp + 0x8]
        fxch st(2)
        fmul dword ptr [esp + 0x38]
        mov edx, dword ptr [eax + 0x24]
        mov ecx, dword ptr [eax + 0x28]
        mov dword ptr [esp + 0x24], edx
        mov edx, dword ptr [eax + 0x2c]
        add eax, 0x4
        mov dword ptr [esp + 0x28], ecx
        fst dword ptr [esp + 0xc]
        fxch st(1)
        fmul dword ptr [esp + 0x38]
        add eax, 0x4
        mov dword ptr [esp + 0x2c], edx
        add eax, 0x4
        add eax, 0x4
        fst dword ptr [esp + 0x10]
        fxch st(4)
        fmul dword ptr [esp + 0x38]
        add eax, 0x4
        add eax, 0x4
        add eax, 0x4
        fst dword ptr [esp + 0x14]
        fld dword ptr [eax - 0x4]
        fmul dword ptr [esp + 0x3c]
        add eax, 0x4
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [esp + 0x18]
        fld dword ptr [eax - 0xc]
        fmul dword ptr [esp + 0x3c]
        fstp dword ptr [esp + 0x1c]
        fld dword ptr [eax - 0x8]
        fmul dword ptr [esp + 0x3c]
        fstp dword ptr [esp + 0x20]
        fxch st(5)
        fstp dword ptr [eax - 0x28]
        fxch st(2)
        fstp dword ptr [eax - 0x24]
        fstp dword ptr [eax - 0x20]
        fstp dword ptr [eax - 0x1c]
        fstp dword ptr [eax - 0x18]
        fstp dword ptr [eax - 0x14]
        fld dword ptr [esp + 0x18]
        fld dword ptr [esp + 0x1c]
        fld dword ptr [esp + 0x20]
        fxch st(2)
        fstp dword ptr [eax - 0x10]
        fld dword ptr [esp + 0x24]
        fxch st(1)
        fstp dword ptr [eax - 0xc]
        fld dword ptr [esp + 0x28]
        fxch st(2)
        fstp dword ptr [eax - 0x8]
        fld dword ptr [esp + 0x2c]
        fxch st(1)
        fstp dword ptr [eax - 0x4]
        fxch st(1)
        fstp dword ptr [eax]
        fstp dword ptr [eax + 0x4]
        mov eax, [g_TransformFlagSP_004e0e84]
        mov dword ptr [eax], 0x0
        add esp, 0x30
        ret 0xc
    }
}

// 0x004737e0 Transform_TranslateLocal (stack x, y, z; ret 0xc): identity level -> translation set; else t := v . R + t.
__declspec(naked) void __stdcall Transform_TranslateLocal(float, float, float)
{
    __asm {
        mov eax, [g_TransformFlagSP_004e0e84]
        sub esp, 0x30
        cmp dword ptr [eax], 0x0
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        jz L_47382b
        mov edx, dword ptr [ecx]
        mov eax, dword ptr [esp + 0x34]
        mov dword ptr [edx + 0x24], eax
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov eax, dword ptr [esp + 0x38]
        mov edx, dword ptr [ecx]
        mov dword ptr [edx + 0x28], eax
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov eax, dword ptr [esp + 0x3c]
        mov edx, dword ptr [ecx]
        mov dword ptr [edx + 0x2c], eax
        mov eax, [g_TransformFlagSP_004e0e84]
        mov dword ptr [eax], 0x0
        add esp, 0x30
        ret 0xc
    L_47382b:
        fld dword ptr [esp + 0x38]
        fld dword ptr [esp + 0x3c]
        fld dword ptr [esp + 0x38]
        fld dword ptr [esp + 0x34]
        fld dword ptr [esp + 0x34]
        mov eax, dword ptr [ecx]
        fld dword ptr [esp + 0x38]
        fld dword ptr [eax + 0x20]
        fxch st(6)
        fmul dword ptr [eax + 0xc]
        fld dword ptr [esp + 0x34]
        fxch st(6)
        fmul dword ptr [eax + 0x18]
        mov edx, dword ptr [eax]
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [esp], edx
        mov dword ptr [esp + 0x4], ecx
        faddp st(1), st(0)
        fxch st(5)
        fmul dword ptr [eax]
        mov edx, dword ptr [eax + 0x8]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [esp + 0x8], edx
        mov edx, dword ptr [eax + 0x10]
        faddp st(5), st(0)
        fxch st(3)
        fmul dword ptr [eax + 0x10]
        fld dword ptr [esp + 0x3c]
        fxch st(5)
        fadd dword ptr [eax + 0x24]
        mov dword ptr [esp + 0xc], ecx
        mov dword ptr [esp + 0x10], edx
        mov ecx, dword ptr [eax + 0x14]
        mov edx, dword ptr [eax + 0x18]
        mov dword ptr [esp + 0x14], ecx
        mov ecx, dword ptr [eax + 0x1c]
        fstp dword ptr [esp + 0x24]
        fxch st(2)
        fmul dword ptr [eax + 0x4]
        fld dword ptr [esp + 0x3c]
        fxch st(1)
        faddp st(3), st(0)
        fxch st(1)
        fmul dword ptr [eax + 0x8]
        fld dword ptr [esp]
        fxch st(5)
        fmul dword ptr [eax + 0x1c]
        add eax, 0x4
        mov dword ptr [esp + 0x18], edx
        add eax, 0x4
        mov dword ptr [esp + 0x1c], ecx
        faddp st(3), st(0)
        fxch st(3)
        fmul dword ptr [eax + 0xc]
        fld dword ptr [esp + 0x4]
        fxch st(3)
        fadd dword ptr [eax + 0x20]
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [esp + 0x28]
        fxch st(1)
        fmul dword ptr [eax + 0x10]
        fxch st(1)
        faddp st(3), st(0)
        fld dword ptr [esp + 0x8]
        fld dword ptr [esp + 0xc]
        fxch st(2)
        faddp st(4), st(0)
        fld dword ptr [esp + 0x10]
        fxch st(4)
        fadd dword ptr [eax + 0x1c]
        fstp dword ptr [esp + 0x2c]
        fxch st(4)
        fstp dword ptr [eax - 0x10]
        fxch st(1)
        fstp dword ptr [eax - 0xc]
        fxch st(2)
        fstp dword ptr [eax - 0x8]
        fld dword ptr [esp + 0x14]
        fxch st(2)
        fstp dword ptr [eax - 0x4]
        fld dword ptr [esp + 0x18]
        fxch st(1)
        fstp dword ptr [eax]
        fld dword ptr [esp + 0x1c]
        fxch st(2)
        fstp dword ptr [eax + 0x4]
        add eax, 0x4
        mov edx, dword ptr [esp + 0x24]
        add eax, 0x4
        mov ecx, dword ptr [esp + 0x28]
        fstp dword ptr [eax]
        add eax, 0x4
        fstp dword ptr [eax]
        add eax, 0x4
        fstp dword ptr [eax]
        add eax, 0x4
        mov dword ptr [eax], edx
        mov edx, dword ptr [esp + 0x2c]
        add eax, 0x4
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 0x4], edx
        mov eax, [g_TransformFlagSP_004e0e84]
        mov dword ptr [eax], 0x0
        add esp, 0x30
        ret 0xc
    }
}

// 0x00473970 Transform_RotateXLocal (stack angle; ret 4): current := Rx . current.
__declspec(naked) void __stdcall Transform_RotateXLocal(float)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x44
        fld dword ptr [ebp + 0x8]
        lea eax, [ebp + 0x8]
        lea ecx, [ebp - 0x4]
        fst qword ptr [ebp - 0x14]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_2968]
        mov dword ptr [ebp - 0x8], eax
        push ebx
        mov dword ptr [ebp - 0xc], ecx
        fnstsw AX
        test AH, 0x41
        jnz L_4739a8
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0x4]
        fcos
        fstp dword ptr [ebp + 0x8]
        jmp L_4739b9
    L_4739a8:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x8]
        fld qword ptr [ebp - 0x14]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_4739b9:
        mov edx, dword ptr [g_TransformFlagSP_004e0e84]
        cmp dword ptr [edx], 0x0
        jz L_473a0d
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [eax]
        mov dword ptr [ecx + 0x10], edx
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, dword ptr [ebp - 0x4]
        mov ecx, dword ptr [eax]
        mov dword ptr [ecx + 0x14], edx
        mov eax, [g_TransformMatrixSP_004e0e88]
        fld dword ptr [ebp - 0x4]
        mov ecx, dword ptr [eax]
        fchs
        fstp dword ptr [ecx + 0x1c]
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov ecx, dword ptr [ebp + 0x8]
        mov eax, dword ptr [edx]
        mov dword ptr [eax + 0x20], ecx
        mov eax, [g_TransformFlagSP_004e0e84]
        mov dword ptr [eax], 0x0
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_473a0d:
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov eax, dword ptr [edx]
        fld dword ptr [eax]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x18]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0xc]
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [eax + 0x8]
        mov dword ptr [ebp - 0x40], ecx
        mov ecx, dword ptr [eax + 0x24]
        faddp st(1), st(0)
        mov dword ptr [ebp - 0x3c], edx
        mov edx, dword ptr [eax + 0x28]
        mov dword ptr [ebp - 0x20], ecx
        mov ecx, dword ptr [eax + 0x2c]
        fstp dword ptr [ebp - 0x38]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0x10]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x1c]
        mov dword ptr [ebp - 0x1c], edx
        mov edx, dword ptr [ebp - 0x40]
        add eax, 0x4
        mov dword ptr [ebp - 0x18], ecx
        faddp st(1), st(0)
        mov ecx, dword ptr [ebp - 0x3c]
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ebp - 0x34]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x14]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0x8]
        add eax, 0x4
        add eax, 0x4
        faddp st(1), st(0)
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ebp - 0x30]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x4]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x10]
        add eax, 0x4
        add eax, 0x4
        fsubp st(1), st(0)
        add eax, 0x4
        fstp dword ptr [ebp - 0x2c]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0xc]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x18]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x28]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x8]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x14]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x24]
        fstp dword ptr [eax - 0x28]
        mov dword ptr [eax - 0x24], edx
        mov edx, dword ptr [ebp - 0x38]
        mov dword ptr [eax - 0x20], ecx
        mov ecx, dword ptr [ebp - 0x34]
        mov dword ptr [eax - 0x1c], edx
        mov edx, dword ptr [ebp - 0x30]
        mov dword ptr [eax - 0x18], ecx
        mov ecx, dword ptr [ebp - 0x2c]
        mov dword ptr [eax - 0x14], edx
        mov edx, dword ptr [ebp - 0x28]
        mov dword ptr [eax - 0x10], ecx
        mov ecx, dword ptr [ebp - 0x24]
        mov dword ptr [eax - 0xc], edx
        mov edx, dword ptr [ebp - 0x20]
        mov dword ptr [eax - 0x8], ecx
        mov ecx, dword ptr [ebp - 0x1c]
        mov dword ptr [eax - 0x4], edx
        mov edx, dword ptr [ebp - 0x18]
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 0x4], edx
        mov eax, [g_TransformFlagSP_004e0e84]
        pop ebx
        mov dword ptr [eax], 0x0
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x00473b10 Transform_RotateYLocal (stack angle; ret 4): current := Ry . current.
__declspec(naked) void __stdcall Transform_RotateYLocal(float)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x44
        fld dword ptr [ebp + 0x8]
        lea eax, [ebp - 0x4]
        lea ecx, [ebp + 0x8]
        fst qword ptr [ebp - 0x14]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_2968]
        mov dword ptr [ebp - 0x8], eax
        push ebx
        mov dword ptr [ebp - 0xc], ecx
        fnstsw AX
        test AH, 0x41
        jnz L_473b48
        fld st(0)
        fsin
        fstp dword ptr [ebp + 0x8]
        fcos
        fstp dword ptr [ebp - 0x4]
        jmp L_473b59
    L_473b48:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x8]
        fld qword ptr [ebp - 0x14]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_473b59:
        mov edx, dword ptr [g_TransformFlagSP_004e0e84]
        cmp dword ptr [edx], 0x0
        jz L_473bae
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, dword ptr [ebp - 0x4]
        mov ecx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov eax, [g_TransformMatrixSP_004e0e88]
        fld dword ptr [ebp + 0x8]
        mov ecx, dword ptr [eax]
        fchs
        fstp dword ptr [ecx + 0x8]
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov ecx, dword ptr [ebp + 0x8]
        mov eax, dword ptr [edx]
        mov dword ptr [eax + 0x18], ecx
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov ecx, dword ptr [ebp - 0x4]
        mov eax, dword ptr [edx]
        mov dword ptr [eax + 0x20], ecx
        mov edx, dword ptr [g_TransformFlagSP_004e0e84]
        mov dword ptr [edx], 0x0
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_473bae:
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        fld dword ptr [ebp - 0x4]
        mov eax, dword ptr [edx]
        fmul dword ptr [eax]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0x18]
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [eax + 0x10]
        mov dword ptr [ebp - 0x38], ecx
        mov ecx, dword ptr [eax + 0x14]
        fsubp st(1), st(0)
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x4]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0x1c]
        mov dword ptr [ebp - 0x34], edx
        mov edx, dword ptr [eax + 0x24]
        mov dword ptr [ebp - 0x30], ecx
        mov ecx, dword ptr [eax + 0x28]
        fsubp st(1), st(0)
        mov dword ptr [ebp - 0x20], edx
        mov edx, dword ptr [eax + 0x2c]
        mov dword ptr [ebp - 0x1c], ecx
        add eax, 0x4
        fstp dword ptr [ebp - 0x40]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x4]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0x1c]
        mov ecx, dword ptr [ebp - 0x40]
        mov dword ptr [ebp - 0x18], edx
        add eax, 0x4
        fsubp st(1), st(0)
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ebp - 0x3c]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x8]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x10]
        mov edx, dword ptr [ebp - 0x3c]
        add eax, 0x4
        add eax, 0x4
        faddp st(1), st(0)
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ebp - 0x2c]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x1c]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x4]
        add eax, 0x4
        add eax, 0x4
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x28]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x20]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x8]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x24]
        fstp dword ptr [eax - 0x28]
        mov dword ptr [eax - 0x24], ecx
        mov ecx, dword ptr [ebp - 0x38]
        mov dword ptr [eax - 0x20], edx
        mov edx, dword ptr [ebp - 0x34]
        mov dword ptr [eax - 0x1c], ecx
        mov ecx, dword ptr [ebp - 0x30]
        mov dword ptr [eax - 0x18], edx
        mov edx, dword ptr [ebp - 0x2c]
        mov dword ptr [eax - 0x14], ecx
        mov ecx, dword ptr [ebp - 0x28]
        mov dword ptr [eax - 0x10], edx
        mov edx, dword ptr [ebp - 0x24]
        mov dword ptr [eax - 0xc], ecx
        mov ecx, dword ptr [ebp - 0x20]
        mov dword ptr [eax - 0x8], edx
        mov edx, dword ptr [ebp - 0x1c]
        mov dword ptr [eax - 0x4], ecx
        mov ecx, dword ptr [ebp - 0x18]
        mov dword ptr [eax], edx
        mov dword ptr [eax + 0x4], ecx
        mov edx, dword ptr [g_TransformFlagSP_004e0e84]
        pop ebx
        mov dword ptr [edx], 0x0
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x00473cc0 Transform_RotateZLocal (stack angle; ret 4): current := Rz . current.
__declspec(naked) void __stdcall Transform_RotateZLocal(float)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x44
        fld dword ptr [ebp + 0x8]
        lea eax, [ebp + 0x8]
        lea ecx, [ebp - 0x4]
        fst qword ptr [ebp - 0x14]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_2968]
        mov dword ptr [ebp - 0x8], eax
        push ebx
        mov dword ptr [ebp - 0xc], ecx
        fnstsw AX
        test AH, 0x41
        jnz L_473cf8
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0x4]
        fcos
        fstp dword ptr [ebp + 0x8]
        jmp L_473d09
    L_473cf8:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x8]
        fld qword ptr [ebp - 0x14]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_473d09:
        mov edx, dword ptr [g_TransformFlagSP_004e0e84]
        cmp dword ptr [edx], 0x0
        jz L_473d5d
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, dword ptr [ebp - 0x4]
        mov ecx, dword ptr [eax]
        mov dword ptr [ecx + 0x4], edx
        mov eax, [g_TransformMatrixSP_004e0e88]
        fld dword ptr [ebp - 0x4]
        mov ecx, dword ptr [eax]
        fchs
        fstp dword ptr [ecx + 0xc]
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov ecx, dword ptr [ebp + 0x8]
        mov eax, dword ptr [edx]
        mov dword ptr [eax + 0x10], ecx
        mov edx, dword ptr [g_TransformFlagSP_004e0e84]
        mov dword ptr [edx], 0x0
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_473d5d:
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        fld dword ptr [ebp - 0x4]
        mov eax, dword ptr [edx]
        fmul dword ptr [eax + 0xc]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax]
        mov ecx, dword ptr [eax + 0x18]
        mov edx, dword ptr [eax + 0x1c]
        mov dword ptr [ebp - 0x2c], ecx
        mov ecx, dword ptr [eax + 0x20]
        faddp st(1), st(0)
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x10]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0x4]
        mov dword ptr [ebp - 0x28], edx
        mov edx, dword ptr [eax + 0x24]
        mov dword ptr [ebp - 0x24], ecx
        mov ecx, dword ptr [eax + 0x28]
        faddp st(1), st(0)
        mov dword ptr [ebp - 0x20], edx
        mov edx, dword ptr [eax + 0x2c]
        mov dword ptr [ebp - 0x1c], ecx
        add eax, 0x4
        fstp dword ptr [ebp - 0x40]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax + 0x10]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax + 0x4]
        mov ecx, dword ptr [ebp - 0x40]
        mov dword ptr [ebp - 0x18], edx
        add eax, 0x4
        faddp st(1), st(0)
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ebp - 0x3c]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x4]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x10]
        mov edx, dword ptr [ebp - 0x3c]
        add eax, 0x4
        add eax, 0x4
        fsubp st(1), st(0)
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ebp - 0x38]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x10]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x1c]
        add eax, 0x4
        add eax, 0x4
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x34]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [eax - 0x14]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [eax - 0x20]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x30]
        fstp dword ptr [eax - 0x28]
        mov dword ptr [eax - 0x24], ecx
        mov ecx, dword ptr [ebp - 0x38]
        mov dword ptr [eax - 0x20], edx
        mov edx, dword ptr [ebp - 0x34]
        mov dword ptr [eax - 0x1c], ecx
        mov ecx, dword ptr [ebp - 0x30]
        mov dword ptr [eax - 0x18], edx
        mov edx, dword ptr [ebp - 0x2c]
        mov dword ptr [eax - 0x14], ecx
        mov ecx, dword ptr [ebp - 0x28]
        mov dword ptr [eax - 0x10], edx
        mov edx, dword ptr [ebp - 0x24]
        mov dword ptr [eax - 0xc], ecx
        mov ecx, dword ptr [ebp - 0x20]
        mov dword ptr [eax - 0x8], edx
        mov edx, dword ptr [ebp - 0x1c]
        mov dword ptr [eax - 0x4], ecx
        mov ecx, dword ptr [ebp - 0x18]
        mov dword ptr [eax], edx
        mov dword ptr [eax + 0x4], ecx
        mov edx, dword ptr [g_TransformFlagSP_004e0e84]
        pop ebx
        mov dword ptr [edx], 0x0
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004745e0 zTransformRotateVectors (ECX points, EDX count): v := v . R in place (no-op on an identity level).
__declspec(naked) void __fastcall zTransformRotateVectors(float*, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x8
        mov eax, [g_TransformFlagSP_004e0e84]
        push ebx
        cmp dword ptr [eax], 0x0
        jnz L_47465c
        mov eax, edx
        dec edx
        test eax, eax
        jz L_47465c
        inc edx
    L_4745f9:
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov dword ptr [ebp - 0x8], ecx
        add ecx, 0xc
        mov eax, dword ptr [eax]
        mov dword ptr [ebp - 0x4], eax
        mov eax, dword ptr [ebp - 0x8]
        mov ebx, dword ptr [ebp - 0x4]
        fld dword ptr [eax]
        fmul dword ptr [ebx]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x4]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x8]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0xc]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x10]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x14]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x18]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x1c]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x8]
        fstp dword ptr [eax + 0x4]
        fstp dword ptr [eax]
        dec edx
        jnz L_4745f9
    L_47465c:
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00474670 zTransformRotateVectorsTransposed (ECX points, EDX count): v := R . v in place (no-op on identity).
__declspec(naked) void __fastcall zTransformRotateVectorsTransposed(float*, int)
{
    __asm {
        mov eax, [g_TransformFlagSP_004e0e84]
        sub esp, 0xc
        push esi
        mov esi, dword ptr [eax]
        test esi, esi
        jnz L_474704
        test edx, edx
        jle L_474704
        lea eax, [ecx + 0x8]
    L_47468a:
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        add eax, 0xc
        dec edx
        mov ecx, dword ptr [ecx]
        fld dword ptr [ecx + 0x14]
        fld dword ptr [ecx + 0x10]
        fld dword ptr [ecx + 0x20]
        fld dword ptr [ecx + 0x1c]
        fxch st(3)
        fmul dword ptr [eax - 0xc]
        fld dword ptr [ecx + 0x8]
        fxch st(3)
        fmul dword ptr [eax - 0x10]
        fld dword ptr [ecx + 0x4]
        fxch st(1)
        faddp st(2), st(0)
        fxch st(2)
        fmul dword ptr [eax - 0xc]
        fld dword ptr [ecx + 0xc]
        fmul dword ptr [eax - 0x14]
        faddp st(2), st(0)
        fxch st(4)
        fmul dword ptr [eax - 0x10]
        fld dword ptr [ecx + 0x18]
        fxch st(1)
        faddp st(5), st(0)
        fxch st(3)
        fmul dword ptr [eax - 0xc]
        fld dword ptr [ecx]
        fxch st(4)
        fmul dword ptr [eax - 0x14]
        faddp st(5), st(0)
        fxch st(2)
        fmul dword ptr [eax - 0x10]
        fxch st(4)
        fstp dword ptr [esp + 0xc]
        fxch st(3)
        faddp st(1), st(0)
        fld dword ptr [esp + 0xc]
        fxch st(2)
        fmul dword ptr [eax - 0x14]
        faddp st(1), st(0)
        fstp dword ptr [eax - 0x14]
        fxch st(1)
        fstp dword ptr [eax - 0x10]
        fstp dword ptr [eax - 0xc]
        jnz L_47468a
    L_474704:
        pop esi
        add esp, 0xc
        ret
    }
}

// 0x00474710 zTransformRotateVectorsTo (ECX src, EDX dst, stack count; ret 4): dst := src . R (copied on identity).
__declspec(naked) void __fastcall zTransformRotateVectorsTo(const float*, float*, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x8
        mov eax, [g_TransformFlagSP_004e0e84]
        push ebx
        push esi
        mov esi, ecx
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [ebp + 0x8]
        push edi
        mov edi, edx
        test ecx, ecx
        jz L_474748
        lea ecx, [eax + eax*0x2]
        shl ecx, 0x2
        mov edx, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_474748:
        mov ecx, eax
        dec eax
        test ecx, ecx
        jz L_4747bf
        lea ecx, [eax + 0x1]
    L_474752:
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov dword ptr [ebp + 0x8], edi
        mov dword ptr [ebp - 0x8], esi
        add esi, 0xc
        mov eax, dword ptr [edx]
        add edi, 0xc
        mov dword ptr [ebp - 0x4], eax
        mov eax, dword ptr [ebp - 0x8]
        mov ebx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp + 0x8]
        fld dword ptr [eax]
        fmul dword ptr [ebx]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x4]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x8]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0xc]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x10]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x14]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x18]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x1c]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fstp dword ptr [edx + 0x8]
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx]
        dec ecx
        jnz L_474752
    L_4747bf:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004747d0 zTransformPoints (ECX points, EDX count): p := p . R + t in place (no-op on an identity level).
__declspec(naked) void __fastcall zTransformPoints(float*, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x8
        mov eax, [g_TransformFlagSP_004e0e84]
        push ebx
        cmp dword ptr [eax], 0x0
        jnz L_47485d
        mov eax, edx
        dec edx
        test eax, eax
        jz L_47485d
        inc edx
    L_4747e9:
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov dword ptr [ebp - 0x8], ecx
        add ecx, 0xc
        mov eax, dword ptr [eax]
        mov dword ptr [ebp - 0x4], eax
        mov eax, dword ptr [ebp - 0x8]
        mov ebx, dword ptr [ebp - 0x4]
        fld dword ptr [eax]
        fmul dword ptr [ebx]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x4]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x8]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0xc]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x10]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x14]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x18]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x1c]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fxch st(2)
        fadd dword ptr [ebx + 0x24]
        fxch st(1)
        fadd dword ptr [ebx + 0x28]
        fxch st(2)
        fadd dword ptr [ebx + 0x2c]
        fxch st(1)
        fstp dword ptr [eax]
        fstp dword ptr [eax + 0x8]
        fstp dword ptr [eax + 0x4]
        dec edx
        jnz L_4747e9
    L_47485d:
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00474010 zTransformTRSLocal (ECX angles[3], EDX translation[3], stack scale[3]; ret 4): local = Rz.Rx.Ry, rows
// scaled where scale != 1, translation where != 0; zTransformConcatenateLocal(local, 1).
__declspec(naked) void __fastcall zTransformTRSLocal(const float*, const float*, const float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x58
        fld dword ptr [ecx + 0x8]
        fsin
        lea eax, [ebp - 0x14]
        push ebx
        push esi
        mov dword ptr [ebp - 0x1c], eax
        mov esi, edx
        lea edx, [ebp - 0x10]
        mov dword ptr [ebp - 0x20], edx
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ecx]
        fst qword ptr [ebp - 0x28]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_2968]
        fnstsw AX
        test AH, 0x41
        jnz L_474052
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0x10]
        fcos
        fstp dword ptr [ebp - 0x14]
        jmp L_474063
    L_474052:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x20]
        mov edx, dword ptr [ebp - 0x1c]
        fld qword ptr [ebp - 0x28]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_474063:
        fld dword ptr [ecx + 0x4]
        fst qword ptr [ebp - 0x28]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_2968]
        lea eax, [ebp - 0x18]
        lea edx, [ebp - 0xc]
        mov dword ptr [ebp - 0x20], eax
        mov dword ptr [ebp - 0x1c], edx
        fnstsw AX
        test AH, 0x41
        jnz L_474094
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0xc]
        fcos
        fstp dword ptr [ebp - 0x18]
        jmp L_4740a5
    L_474094:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x1c]
        mov edx, dword ptr [ebp - 0x20]
        fld qword ptr [ebp - 0x28]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_4740a5:
        fld dword ptr [ecx + 0x8]
        fst qword ptr [ebp - 0x28]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_2968]
        lea eax, [ebp - 0x8]
        lea edx, [ebp - 0x4]
        mov dword ptr [ebp - 0x20], eax
        mov dword ptr [ebp - 0x1c], edx
        fnstsw AX
        test AH, 0x41
        jnz L_4740d6
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0x4]
        fcos
        fstp dword ptr [ebp - 0x8]
        jmp L_4740e7
    L_4740d6:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x1c]
        mov edx, dword ptr [ebp - 0x20]
        fld qword ptr [ebp - 0x28]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_4740e7:
        fld dword ptr [ebp - 0xc]
        fmul dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x18]
        fld dword ptr [ebp - 0x8]
        fmul dword ptr [ebp - 0x18]
        mov dword ptr [ebp - 0x34], 0x0
        mov dword ptr [ebp - 0x30], 0x0
        mov dword ptr [ebp - 0x2c], 0x0
        fstp dword ptr [ebp - 0x1c]
        fld st(1)
        fmul dword ptr [ebp - 0x4]
        fadd dword ptr [ebp - 0x1c]
        fstp dword ptr [ebp - 0x58]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x14]
        fstp dword ptr [ebp - 0x54]
        fld st(0)
        fmul dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x8]
        fmul dword ptr [ebp - 0xc]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x50]
        fxch st(1)
        fmul dword ptr [ebp - 0x8]
        fsub st(0), st(1)
        fstp dword ptr [ebp - 0x4c]
        fstp st(0)
        fld dword ptr [ebp - 0x8]
        fmul dword ptr [ebp - 0x14]
        fstp dword ptr [ebp - 0x48]
        fld dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0xc]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x44]
        fld dword ptr [ebp - 0xc]
        fmul dword ptr [ebp - 0x14]
        fstp dword ptr [ebp - 0x40]
        fld dword ptr [ebp - 0x10]
        fchs
        fstp dword ptr [ebp - 0x3c]
        fld dword ptr [ebp - 0x18]
        fmul dword ptr [ebp - 0x14]
        fstp dword ptr [ebp - 0x38]
        fld dword ptr [esi]
        fcomp qword ptr [kZeroD_2970]
        fnstsw AX
        test AH, 0x40
        jnz L_474189
        mov eax, dword ptr [esi]
        mov dword ptr [ebp - 0x34], eax
    L_474189:
        fld dword ptr [esi + 0x4]
        fcomp qword ptr [kZeroD_2970]
        fnstsw AX
        test AH, 0x40
        jnz L_47419f
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [ebp - 0x30], ecx
    L_47419f:
        fld dword ptr [esi + 0x8]
        fcomp qword ptr [kZeroD_2970]
        fnstsw AX
        test AH, 0x40
        jnz L_4741b5
        mov edx, dword ptr [esi + 0x8]
        mov dword ptr [ebp - 0x2c], edx
    L_4741b5:
        mov ecx, dword ptr [ebp + 0x8]
        fld dword ptr [ecx]
        fcomp qword ptr [kOneD_2950]
        fnstsw AX
        test AH, 0x40
        jnz L_4741df
        fld dword ptr [ecx]
        fmul dword ptr [ebp - 0x58]
        fstp dword ptr [ebp - 0x58]
        fld dword ptr [ecx]
        fmul dword ptr [ebp - 0x54]
        fstp dword ptr [ebp - 0x54]
        fld dword ptr [ecx]
        fmul dword ptr [ebp - 0x50]
        fstp dword ptr [ebp - 0x50]
    L_4741df:
        fld dword ptr [ecx + 0x4]
        fcomp qword ptr [kOneD_2950]
        fnstsw AX
        test AH, 0x40
        jnz L_47420a
        fld dword ptr [ebp - 0x4c]
        fmul dword ptr [ecx + 0x4]
        fstp dword ptr [ebp - 0x4c]
        fld dword ptr [ebp - 0x48]
        fmul dword ptr [ecx + 0x4]
        fstp dword ptr [ebp - 0x48]
        fld dword ptr [ebp - 0x44]
        fmul dword ptr [ecx + 0x4]
        fstp dword ptr [ebp - 0x44]
    L_47420a:
        fld dword ptr [ecx + 0x8]
        fcomp qword ptr [kOneD_2950]
        fnstsw AX
        test AH, 0x40
        jnz L_474235
        fld dword ptr [ebp - 0x40]
        fmul dword ptr [ecx + 0x8]
        fstp dword ptr [ebp - 0x40]
        fld dword ptr [ebp - 0x3c]
        fmul dword ptr [ecx + 0x8]
        fstp dword ptr [ebp - 0x3c]
        fld dword ptr [ebp - 0x38]
        fmul dword ptr [ecx + 0x8]
        fstp dword ptr [ebp - 0x38]
    L_474235:
        mov edx, 0x1
        lea ecx, [ebp - 0x58]
        call zTransformConcatenateLocal
        mov eax, [g_TransformFlagSP_004e0e84]
        pop esi
        pop ebx
        mov dword ptr [eax], 0x0
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004757c0 Quat_FromEuler (ECX out quaternion w,x,y,z; stack three angles; ret 0xc): from half angles.
__declspec(naked) void __fastcall Quat_FromEuler(float*, int, float, float, float)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x30
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [kHalfF_29b0]
        lea eax, [ebp - 0x8]
        lea edx, [ebp - 0xc]
        mov dword ptr [ebp - 0x20], eax
        push ebx
        mov dword ptr [ebp - 0x1c], edx
        fst qword ptr [ebp - 0x30]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_29b8]
        fnstsw AX
        test AH, 0x41
        jnz L_4757fe
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0xc]
        fcos
        fstp dword ptr [ebp - 0x8]
        jmp L_47580f
    L_4757fe:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x1c]
        mov edx, dword ptr [ebp - 0x20]
        fld qword ptr [ebp - 0x30]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_47580f:
        fld dword ptr [ebp + 0xc]
        fmul dword ptr [kHalfF_29b0]
        lea eax, [ebp + 0x8]
        lea edx, [ebp - 0x4]
        mov dword ptr [ebp - 0x20], eax
        mov dword ptr [ebp - 0x1c], edx
        fst qword ptr [ebp - 0x30]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD_29b8]
        fnstsw AX
        test AH, 0x41
        jnz L_475846
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0x4]
        fcos
        fstp dword ptr [ebp + 0x8]
        jmp L_475857
    L_475846:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x1c]
        mov edx, dword ptr [ebp - 0x20]
        fld qword ptr [ebp - 0x30]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_475857:
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [ebp - 0x8]
        lea eax, [ebp - 0x14]
        lea edx, [ebp - 0x10]
        mov dword ptr [ebp - 0x24], eax
        mov dword ptr [ebp - 0x28], edx
        fstp dword ptr [ebp - 0x1c]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x8]
        fstp dword ptr [ebp - 0x18]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [ebp - 0xc]
        fstp dword ptr [ebp + 0xc]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0xc]
        fstp dword ptr [ebp - 0x20]
        fld dword ptr [ebp + 0x10]
        fmul dword ptr [kHalfF_29b0]
        fst qword ptr [ebp - 0x30]
        fabs
        fcomp qword ptr [kSinCosLimitD_29b8]
        fnstsw AX
        test AH, 0x41
        jnz L_4758ae
        fld qword ptr [ebp - 0x30]
        fsin
        fld qword ptr [ebp - 0x30]
        fcos
        jmp L_4758c3
    L_4758ae:
        mov ebx, dword ptr [ebp - 0x28]
        mov edx, dword ptr [ebp - 0x24]
        fld qword ptr [ebp - 0x30]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
        fld dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x14]
    L_4758c3:
        fld dword ptr [ebp - 0x20]
        fmul st(0), st(2)
        fld dword ptr [ebp - 0x1c]
        fmul st(0), st(2)
        pop ebx
        faddp st(1), st(0)
        fstp dword ptr [ecx]
        fld dword ptr [ebp + 0xc]
        fmul st(0), st(2)
        fld dword ptr [ebp - 0x18]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fstp dword ptr [ecx + 0x4]
        fld dword ptr [ebp + 0xc]
        fmul st(0), st(1)
        fld dword ptr [ebp - 0x18]
        fmul st(0), st(3)
        fsubp st(1), st(0)
        fstp dword ptr [ecx + 0x8]
        fld dword ptr [ebp - 0x1c]
        fmul st(0), st(2)
        fld dword ptr [ebp - 0x20]
        fmul st(0), st(2)
        fsubp st(1), st(0)
        fstp dword ptr [ecx + 0xc]
        fstp st(0)
        fstp st(0)
        mov esp, ebp
        pop ebp
        ret 0xc
    }
}

// 0x004759d0 Quat_Product (ECX a, EDX b, stack out; ret 4): 4-component quaternion product.
__declspec(naked) void __fastcall Quat_Product(const float*, const float*, float*)
{
    __asm {
        fld dword ptr [edx + 0xc]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        fmul dword ptr [ecx + 0xc]
        fld dword ptr [edx]
        fmul dword ptr [ecx]
        fld dword ptr [edx + 0x4]
        fxch st(3)
        faddp st(2), st(0)
        fxch st(2)
        fmul dword ptr [ecx + 0x4]
        fxch st(2)
        faddp st(1), st(0)
        fxch st(1)
        mov eax, dword ptr [esp + 0x4]
        faddp st(1), st(0)
        fstp dword ptr [eax]
        fld dword ptr [ecx]
        fld dword ptr [edx]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [edx + 0xc]
        fxch st(2)
        fmul dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0xc]
        fxch st(3)
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        fsubp st(2), st(0)
        fxch st(2)
        fmul dword ptr [edx + 0x8]
        fxch st(2)
        fsubp st(1), st(0)
        fxch st(1)
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x4]
        fld dword ptr [ecx]
        fld dword ptr [edx]
        fmul dword ptr [ecx + 0x8]
        fld dword ptr [ecx + 0xc]
        fxch st(2)
        fmul dword ptr [edx + 0x8]
        fld dword ptr [edx + 0xc]
        fxch st(3)
        fmul dword ptr [edx + 0x4]
        fxch st(1)
        fsubp st(2), st(0)
        fxch st(2)
        fmul dword ptr [ecx + 0x4]
        fxch st(2)
        fsubp st(1), st(0)
        fxch st(1)
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x8]
        fld dword ptr [ecx]
        fld dword ptr [edx]
        fmul dword ptr [ecx + 0xc]
        fld dword ptr [edx + 0x8]
        fxch st(2)
        fmul dword ptr [edx + 0xc]
        fld dword ptr [ecx + 0x8]
        fxch st(3)
        fmul dword ptr [ecx + 0x4]
        fxch st(1)
        fsubp st(2), st(0)
        fxch st(2)
        fmul dword ptr [edx + 0x4]
        fxch st(2)
        fsubp st(1), st(0)
        fxch st(1)
        faddp st(1), st(0)
        fstp dword ptr [eax + 0xc]
        ret 0x4
    }
}

// 0x00474bc0 zTransformScreenToView (ECX screen points x,y,z, EDX out, stack count; ret 4): w = 1/z;
// out = ((x - cx) * kx * w, (y - cy) * ky * w, w).
__declspec(naked) void __fastcall zTransformScreenToView(const float*, float*, int)
{
    __asm {
        push edi
        mov edi, dword ptr [esp + 0x8]
        test edi, edi
        jle L_474c16
        push esi
        lea esi, [edx + 0x8]
        lea eax, [ecx + 0x4]
        sub edx, ecx
        mov ecx, edi
    L_474bd4:
        fld dword ptr [kOneF_297c]
        fdiv dword ptr [eax + 0x4]
        add esi, 0xc
        add eax, 0xc
        dec ecx
        fst dword ptr [esi - 0xc]
        fld dword ptr [eax - 0x10]
        fsub dword ptr [g_ScreenCentre_005761e0]
        fmul dword ptr [g_ViewParams_00566838 + 0x28]
        fmul st(0), st(1)
        fstp dword ptr [esi - 0x14]
        fstp st(0)
        fld dword ptr [eax - 0xc]
        fsub dword ptr [g_ScreenCentre_005761e0 + 4]
        fmul dword ptr [g_ViewParams_00566838 + 0x2c]
        fmul dword ptr [esi - 0xc]
        fstp dword ptr [edx + eax*0x1 - 0xc]
        jnz L_474bd4
        pop esi
    L_474c16:
        pop edi
        ret 0x4
    }
}

// 0x00474c20 zTransformScreenToWorld (ECX screen points, EDX out, stack count; ret 4): screen -> view (above) -> world
// through camera matrix B. Its local buffer holds ONE point; every caller passes count 1 (spec: LATENT DEFECT).
__declspec(naked) void __fastcall zTransformScreenToWorld(const float*, float*, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x44
        push ebx
        push esi
        mov esi, dword ptr [ebp + 0x8]
        mov ebx, edx
        push edi
        push esi
        lea edx, [ebp - 0x14]
        call zTransformScreenToView
        lea ecx, [ebp - 0x44]
        call zTransformStackPushRef
        call zTransformLoadCameraWorld
        mov eax, [g_TransformFlagSP_004e0e84]
        mov ecx, ebx
        cmp dword ptr [eax], 0x0
        jz L_474c77
        lea ecx, [esi + esi*0x2]
        lea esi, [ebp - 0x14]
        shl ecx, 0x2
        mov edx, ecx
        mov edi, ebx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        call zTransformStackPop
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_474c77:
        lea edi, [ebp - 0x14]
        sub edi, ebx
    L_474c7c:
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov dword ptr [ebp + 0x8], ecx
        mov edx, dword ptr [eax]
        lea eax, [ecx + edi*0x1]
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0x8], eax
        add ecx, 0xc
        mov eax, dword ptr [ebp - 0x8]
        mov ebx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp + 0x8]
        fld dword ptr [eax]
        fmul dword ptr [ebx]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x4]
        fld dword ptr [eax]
        fmul dword ptr [ebx + 0x8]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0xc]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x10]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebx + 0x14]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x18]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x1c]
        fld dword ptr [eax + 0x8]
        fmul dword ptr [ebx + 0x20]
        fxch st(2)
        faddp st(5), st(0)
        faddp st(3), st(0)
        faddp st(1), st(0)
        fxch st(2)
        fadd dword ptr [ebx + 0x24]
        fxch st(1)
        fadd dword ptr [ebx + 0x28]
        fxch st(2)
        fadd dword ptr [ebx + 0x2c]
        fxch st(1)
        fstp dword ptr [edx]
        fstp dword ptr [edx + 0x8]
        fstp dword ptr [edx + 0x4]
        dec esi
        jnz L_474c7c
        call zTransformStackPop
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x00472fb0 zTransformBillboardYaw (stack yaw; ret 4): parent position, absolute yaw, times view A.
__declspec(naked) void __stdcall zTransformBillboardYaw(float)
{
    __asm {
        mov eax, [g_TransformFlagSP_004e0e84]
        sub esp, 0x34
        mov ecx, dword ptr [eax - 0x4]
        test ecx, ecx
        jnz L_472fd3
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov ecx, dword ptr [ecx - 0x4]
        call Mat3_YawFromRows
        fstp dword ptr [esp]
        jmp L_472fdb
    L_472fd3:
        mov dword ptr [esp], 0x0
    L_472fdb:
        call zTransformLoadIdentity
        fld dword ptr [esp + 0x38]
        fsub dword ptr [esp]
        push ecx
        fstp dword ptr [esp]
        call Transform_RotateYLocal
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, 0x1
        mov ecx, dword ptr [eax - 0x4]
        call zTransformConcatenateLocal
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov ecx, dword ptr [eax - 0x4]
        mov edx, dword ptr [eax]
        add ecx, 0x24
        add edx, 0x24
        mov eax, dword ptr [ecx]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], ecx
        lea ecx, [esp + 0x4]
        call zTransformStackPushRef
        call zTransformLoadView
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, 0x1
        mov ecx, dword ptr [eax - 0x4]
        call zTransformConcatenateLocal
        call zTransformStackPop
        lea ecx, [esp + 0x4]
        call zTransformLoad
        add esp, 0x34
        ret 0x4
    }
}

// 0x00473060 zTransformBillboardFull: full-orientation billboard facing the camera.
__declspec(naked) void zTransformBillboardFull()
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x80
        mov eax, [g_TransformFlagSP_004e0e84]
        push ebx
        mov ecx, dword ptr [eax - 0x4]
        test ecx, ecx
        jnz L_473089
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        lea edx, [ebp - 0x14]
        mov ecx, dword ptr [ecx - 0x4]
        call Mat3_ToEuler
        jmp L_4730a3
    L_473089:
        mov edx, dword ptr [g_DefaultAngles_005669d8]
        mov eax, [g_DefaultAngles_005669d8 + 4]
        mov ecx, dword ptr [g_DefaultAngles_005669d8 + 8]
        mov dword ptr [ebp - 0x14], edx
        mov dword ptr [ebp - 0x10], eax
        mov dword ptr [ebp - 0xc], ecx
    L_4730a3:
        call zTransformLoadCameraWorld
        mov edx, dword ptr [g_TransformMatrixSP_004e0e88]
        mov eax, dword ptr [edx]
        add eax, 0xc
        mov dword ptr [ebp - 0x4], eax
        mov dword ptr [ebp - 0x8], eax
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0x4]
        mov eax, dword ptr [ebx]
        mov edx, dword ptr [ebx + 0x4]
        mov ebx, dword ptr [ebx + 0x8]
        xor eax, 0x80000000
        xor edx, 0x80000000
        xor ebx, 0x80000000
        mov dword ptr [ecx], eax
        mov dword ptr [ecx + 0x4], edx
        mov dword ptr [ecx + 0x8], ebx
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov eax, dword ptr [eax]
        add eax, 0x18
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0x4], eax
        mov ebx, dword ptr [ebp - 0x4]
        mov ecx, dword ptr [ebp - 0x8]
        mov eax, dword ptr [ebx]
        mov edx, dword ptr [ebx + 0x4]
        mov ebx, dword ptr [ebx + 0x8]
        xor eax, 0x80000000
        xor edx, 0x80000000
        xor ebx, 0x80000000
        mov dword ptr [ecx], eax
        mov dword ptr [ecx + 0x4], edx
        mov dword ptr [ecx + 0x8], ebx
        mov ecx, dword ptr [g_TransformMatrixSP_004e0e88]
        lea edx, [ebp - 0x20]
        mov ecx, dword ptr [ecx]
        call Mat3_ToEuler
        mov edx, dword ptr [ebp - 0xc]
        mov eax, dword ptr [ebp - 0x14]
        push edx
        mov edx, dword ptr [ebp - 0x10]
        push eax
        lea ecx, [ebp - 0x30]
        push edx
        call Quat_FromEuler
        mov eax, dword ptr [ebp - 0x18]
        mov edx, dword ptr [ebp - 0x20]
        push eax
        mov eax, dword ptr [ebp - 0x1c]
        push edx
        lea ecx, [ebp - 0x40]
        push eax
        call Quat_FromEuler
        lea ecx, [ebp - 0x50]
        lea edx, [ebp - 0x30]
        push ecx
        lea ecx, [ebp - 0x40]
        call Quat_Product
        lea edx, [ebp - 0x80]
        lea ecx, [ebp - 0x50]
        call Quat_ToMatrix3
        mov ecx, dword ptr [g_DefaultAngles_005669d8 + 8]
        mov edx, dword ptr [g_DefaultAngles_005669d8]
        mov eax, [g_DefaultAngles_005669d8 + 4]
        mov dword ptr [ebp - 0x54], ecx
        lea ecx, [ebp - 0x80]
        mov dword ptr [ebp - 0x5c], edx
        mov dword ptr [ebp - 0x58], eax
        call zTransformLoad
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, 0x1
        mov ecx, dword ptr [eax - 0x4]
        call zTransformConcatenateLocal
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov ecx, dword ptr [eax - 0x4]
        mov edx, dword ptr [eax]
        add ecx, 0x24
        add edx, 0x24
        mov eax, dword ptr [ecx]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], ecx
        lea ecx, [ebp - 0x80]
        call zTransformStackPushRef
        call zTransformLoadView
        mov eax, [g_TransformMatrixSP_004e0e88]
        mov edx, 0x1
        mov ecx, dword ptr [eax - 0x4]
        call zTransformConcatenateLocal
        call zTransformStackPop
        lea ecx, [ebp - 0x80]
        call zTransformLoad
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

}  // namespace recoil
