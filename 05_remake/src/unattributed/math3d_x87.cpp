// SUBSYSTEM: math3d
// math3d functions ported at INSTRUCTION level: naked x87 functions that repeat the original listing
// (same calling convention, same x87 stack sequence), with the original's constants replaced by tagged
// local constants. Used where a C++ translation cannot be exact:
//   - fpatan / fsin / fcos results are 64-bit-mantissa values (the precision-control field does not apply
//     to them), and some functions return such a value unrounded in ST0;
//   - long register-allocated x87 sequences where hand-tracing into C++ is error-prone.
// Placement in src/unattributed is a layout choice (no orig_file). Spec: 04_spec/systems/math3d.md.
// Every function here is CONFIRMED-BINARY from the listing re-read on 2026-09-25 and verified natively
// against the original bytes (tests/test_math3d_native.cpp).
#include "unattributed/math3d.h"

namespace recoil {
namespace {
const float kZeroF = 0.0f;              // CONFIRMED-BINARY: float 0x00000000 at 0x004d2960
const double kSinCosLimitD = 9.22e18;   // CONFIRMED-BINARY: double 0x43dffd01499f4680 at 0x004d2968
const float kZeroF_2918 = 0.0f;           // CONFIRMED-BINARY: float 0x00000000 at 0x004d2918
const float kOneF_291c = 1.0f;            // CONFIRMED-BINARY: float 0x3f800000 at 0x004d291c
const float kZeroF_29c8 = 0.0f;           // CONFIRMED-BINARY: float 0x00000000 at 0x004d29c8
const float kPiF_2998 = 3.14159274f;      // CONFIRMED-BINARY: float 0x40490fdb at 0x004d2998
const float kPiF_2938 = 3.14159274f;      // CONFIRMED-BINARY: float 0x40490fdb at 0x004d2938
const double kNeg095D = -0.95;            // CONFIRMED-BINARY: double 0xbfee666666666666 at 0x004d2930
const double kPos095D = 0.95;             // CONFIRMED-BINARY: double 0x3fee666666666666 at 0x004d2948
// (0x004d29b8 and 0x004d2940 are the same double 9.22e18 as 0x004d2968: kSinCosLimitD)
}  // namespace

// 0x00474d10 Math_ComputeLookAtPitchYaw (ECX a, EDX b, stack out; ret 4):
// d = (a.x-b.x, b.y-a.y, a.z-b.z) as floats; yaw = atan2(d.x, d.z) -> float; h = (float)sqrt(dx*dx + dz*dz);
// pitch = atan2(d.y, h) -> float; out = (pitch, yaw, 0) as dwords.
__declspec(naked) void __fastcall Math_ComputeLookAtPitchYaw(const float*, const float*, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x20
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        lea eax, [ebp - 0x14]
        mov dword ptr [ebp - 0x4], eax
        fstp dword ptr [ebp - 0x14]
        fld dword ptr [edx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ecx + 0x8]
        fsub dword ptr [edx + 0x8]
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [ebp - 0x14]
        fld dword ptr [ebp - 0xc]
        fpatan
        fstp dword ptr [ebp - 0x1c]
        mov ecx, dword ptr [ebp - 0x4]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        faddp st(1), st(0)
        fsqrt
        fstp dword ptr [ebp - 0x8]
        fld dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x8]
        fpatan
        mov eax, dword ptr [ebp + 0x8]
        mov dword ptr [ebp - 0x18], 0x0
        mov ecx, eax
        fstp dword ptr [ebp - 0x20]
        mov edx, dword ptr [ebp - 0x20]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [ebp - 0x1c]
        mov dword ptr [ecx + 0x4], edx
        mov edx, dword ptr [ebp - 0x18]
        mov dword ptr [ecx + 0x8], edx
        mov esp, ebp
        pop ebp
        ret 4
    }
}

// 0x00474d90 Math_LookAtPitch (ECX a, EDX b) -> ST0 = atan2(b.y - a.y, (float)sqrt(dx*dx + dz*dz)),
// returned UNROUNDED in ST0 (callers see the 64-bit-mantissa fpatan result).
__declspec(naked) long double __fastcall Math_LookAtPitch(const float*, const float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x14
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        lea eax, [ebp - 0x14]
        mov dword ptr [ebp - 0x4], eax
        fstp dword ptr [ebp - 0x14]
        fld dword ptr [edx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ecx + 0x8]
        fsub dword ptr [edx + 0x8]
        fstp dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp - 0x4]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        faddp st(1), st(0)
        fsqrt
        fstp dword ptr [ebp - 0x8]
        fld dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x8]
        fpatan
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00474de0 Mat3_YawFromRows (ECX matrix) -> ST0: 0.0 when m[6] == 0 and m[8] == 0 (C3, also NaN),
// else atan2(m[6], m[8]) unrounded.
__declspec(naked) long double __fastcall Mat3_YawFromRows(const float*)
{
    __asm {
        fld dword ptr [ecx + 0x18]
        fcomp dword ptr kZeroF
        fnstsw ax
        test ah, 0x40
        jz do_atan
        fld dword ptr [ecx + 0x20]
        fcomp dword ptr kZeroF
        fnstsw ax
        test ah, 0x40
        jz do_atan
        fld dword ptr kZeroF
        ret
    do_atan:
        fld dword ptr [ecx + 0x18]
        fld dword ptr [ecx + 0x20]
        fpatan
        ret
    }
}

// 0x00474260 Mat3_FromEuler (ECX out 12 floats, stack a, b, g; ret 0xc): rotation Rz(g).Rx(a).Ry(b)
// into out[0..8], translation out[9..11] = 0. Three sin/cos prologues, each choosing FSIN/FCOS or
// FSINCOS by |angle| vs 9.22e18 exactly as the listing does.
__declspec(naked) void __fastcall Mat3_FromEuler(float*, int, float, float, float)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x28
        fld dword ptr [ebp + 0x8]
        lea eax, [ebp - 0x8]
        lea edx, [ebp - 0x4]
        fst qword ptr [ebp - 0x28]
        fld st(0)
        fabs
        fcomp qword ptr kSinCosLimitD
        mov dword ptr [ebp - 0x18], eax
        push ebx
        mov dword ptr [ebp - 0x20], edx
        fnstsw ax
        test ah, 0x41
        jnz a_small
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0x4]
        fcos
        fstp dword ptr [ebp - 0x8]
        jmp a_done
    a_small:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x20]
        mov edx, dword ptr [ebp - 0x18]
        fld qword ptr [ebp - 0x28]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    a_done:
        fld dword ptr [ebp + 0xc]
        lea eax, [ebp - 0xc]
        lea edx, [ebp + 0x8]
        fst qword ptr [ebp - 0x28]
        fld st(0)
        fabs
        fcomp qword ptr kSinCosLimitD
        mov dword ptr [ebp - 0x20], eax
        mov dword ptr [ebp - 0x18], edx
        fnstsw ax
        test ah, 0x41
        jnz b_small
        fld st(0)
        fsin
        fstp dword ptr [ebp + 0x8]
        fcos
        fstp dword ptr [ebp - 0xc]
        jmp b_done
    b_small:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x18]
        mov edx, dword ptr [ebp - 0x20]
        fld qword ptr [ebp - 0x28]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    b_done:
        fld dword ptr [ebp + 0x10]
        lea eax, [ebp - 0x14]
        lea edx, [ebp - 0x10]
        fst qword ptr [ebp - 0x1c]
        fabs
        fcomp qword ptr kSinCosLimitD
        mov dword ptr [ebp + 0xc], eax
        mov dword ptr [ebp - 0x20], edx
        fnstsw ax
        test ah, 0x41
        jnz g_small
        fld qword ptr [ebp - 0x1c]
        fsin
        fld qword ptr [ebp - 0x1c]
        fcos
        jmp g_done
    g_small:
        mov ebx, dword ptr [ebp - 0x20]
        mov edx, dword ptr [ebp + 0xc]
        fld qword ptr [ebp - 0x1c]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
        fld dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x14]
    g_done:
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [ebp - 0x4]
        fld st(2)
        fmul dword ptr [ebp - 0xc]
        fld st(2)
        fmul dword ptr [ebp - 0xc]
        lea eax, [ecx + 0x4]
        pop ebx
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ebp + 0xc]
        fld st(1)
        fmul st(0), st(4)
        add eax, 0x4
        add eax, 0x4
        fadd dword ptr [ebp + 0xc]
        add eax, 0x4
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [ecx]
        fld st(3)
        fmul dword ptr [ebp - 0x8]
        add eax, 0x4
        add eax, 0x4
        fstp dword ptr [eax - 0x24]
        add eax, 0x4
        fld st(0)
        fmul dword ptr [ebp - 0x4]
        fld st(3)
        fmul dword ptr [ebp + 0x8]
        fsubp st(1), st(0)
        fstp dword ptr [eax - 0x24]
        fxch st(1)
        fmul st(0), st(2)
        fsub st(0), st(1)
        fstp dword ptr [eax - 0x20]
        fstp st(0)
        fmul dword ptr [ebp - 0x8]
        fstp dword ptr [eax - 0x1c]
        fmul dword ptr [ebp + 0x8]
        fld dword ptr [ebp + 0xc]
        fmul dword ptr [ebp - 0x4]
        faddp st(1), st(0)
        fstp dword ptr [eax - 0x18]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [ebp - 0x8]
        fstp dword ptr [eax - 0x14]
        fld dword ptr [ebp - 0x4]
        fchs
        fstp dword ptr [eax - 0x10]
        fld dword ptr [ebp - 0xc]
        fmul dword ptr [ebp - 0x8]
        fstp dword ptr [eax - 0xc]
        mov dword ptr [eax - 0x8], 0x0
        mov dword ptr [eax - 0x4], 0x0
        mov dword ptr [eax], 0x0
        mov esp, ebp
        pop ebp
        ret 0xc
    }
}

// 0x00474870 Box8_TransformByMatrix (ECX matrix 12 floats = 3x3 rotation + translation, EDX box
// min/max 6 floats, stack out 24 floats; ret 4): the 8 box corners transformed by the matrix. Converted
// from the Ghidra listing with tools/asm_port/ghidra2masm.py (no absolute data references).
__declspec(naked) void __fastcall Box8_TransformByMatrix(const float*, const float*, float*)
{
    __asm {
        sub esp, 0x3c
        fld dword ptr [edx + 0x10]
        fld dword ptr [ecx + 0x14]
        fld dword ptr [ecx + 0x10]
        fmul dword ptr [edx + 0x10]
        fld dword ptr [ecx + 0x10]
        fxch st(2)
        fmul dword ptr [edx + 0x10]
        fld dword ptr [ecx + 0x14]
        fxch st(4)
        fmul dword ptr [ecx + 0xc]
        fld dword ptr [ecx + 0xc]
        fld dword ptr [edx]
        fmul dword ptr [ecx]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [edx]
        fmul dword ptr [ecx + 0x4]
        fstp dword ptr [esp + 0x10]
        fld dword ptr [edx]
        fmul dword ptr [ecx + 0x8]
        fstp dword ptr [esp + 0x14]
        fld dword ptr [ecx]
        fmul dword ptr [edx + 0xc]
        fstp dword ptr [esp + 0x18]
        fld dword ptr [edx + 0xc]
        fmul dword ptr [ecx + 0x4]
        fstp dword ptr [esp + 0x1c]
        fld dword ptr [edx + 0xc]
        fmul dword ptr [ecx + 0x8]
        fxch st(2)
        fst dword ptr [esp]
        fxch st(4)
        fst dword ptr [esp + 0x4]
        fxch st(3)
        fst dword ptr [esp + 0x8]
        fxch st(4)
        fadd dword ptr [esp + 0xc]
        fstp dword ptr [esp + 0x24]
        fmul dword ptr [edx + 0x4]
        fld dword ptr [esp]
        fxch st(3)
        fadd dword ptr [esp + 0x10]
        fstp dword ptr [esp + 0x28]
        fxch st(4)
        fmul dword ptr [edx + 0x4]
        fld dword ptr [esp + 0x4]
        fxch st(4)
        fadd dword ptr [esp + 0x14]
        fstp dword ptr [esp + 0x2c]
        fxch st(5)
        fmul dword ptr [edx + 0x4]
        fld dword ptr [esp + 0x8]
        fxch st(3)
        fadd dword ptr [esp + 0x18]
        fstp dword ptr [esp + 0x30]
        fxch st(4)
        fst dword ptr [esp]
        fld dword ptr [esp]
        fxch st(4)
        fadd dword ptr [esp + 0x1c]
        fstp dword ptr [esp + 0x34]
        fxch st(5)
        fst dword ptr [esp + 0x4]
        fld dword ptr [esp + 0x4]
        fxch st(3)
        fadd st(0), st(2)
        fstp dword ptr [esp + 0x38]
        fld st(4)
        fxch st(6)
        fadd dword ptr [esp + 0xc]
        fxch st(1)
        fadd dword ptr [esp + 0x10]
        fxch st(6)
        fadd dword ptr [esp + 0x14]
        fxch st(4)
        fadd dword ptr [esp + 0x18]
        fxch st(3)
        fadd dword ptr [esp + 0x1c]
        fxch st(1)
        fstp dword ptr [esp + 0xc]
        fxch st(5)
        fstp dword ptr [esp + 0x10]
        fxch st(2)
        fstp dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x40]
        fstp dword ptr [esp + 0x18]
        fxch st(2)
        fstp dword ptr [esp + 0x1c]
        fld dword ptr [ecx + 0x18]
        fld dword ptr [ecx + 0x1c]
        fld dword ptr [ecx + 0x20]
        fxch st(2)
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        fmul dword ptr [edx + 0x8]
        fxch st(3)
        fadd st(0), st(4)
        fxch st(2)
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        fadd dword ptr [ecx + 0x24]
        fxch st(3)
        fadd dword ptr [ecx + 0x28]
        fxch st(1)
        fadd dword ptr [ecx + 0x2c]
        fld dword ptr [esp + 0x30]
        fld dword ptr [esp + 0x34]
        fxch st(4)
        fstp dword ptr [esp + 0x20]
        fld dword ptr [esp + 0x38]
        fxch st(5)
        fst dword ptr [esp]
        fxch st(3)
        fst dword ptr [esp + 0x4]
        fxch st(2)
        fst dword ptr [esp + 0x8]
        fxch st(3)
        fadd dword ptr [esp + 0x18]
        fstp dword ptr [eax + 0x18]
        fld dword ptr [esp]
        fxch st(2)
        fadd dword ptr [esp + 0x1c]
        fstp dword ptr [eax + 0x1c]
        fld dword ptr [esp + 0x4]
        fxch st(3)
        fadd dword ptr [esp + 0x20]
        fstp dword ptr [eax + 0x20]
        fld dword ptr [esp + 0x8]
        fxch st(2)
        fadd dword ptr [esp + 0xc]
        fstp dword ptr [eax + 0x24]
        fld dword ptr [esp + 0x24]
        fxch st(3)
        fadd dword ptr [esp + 0x10]
        fstp dword ptr [eax + 0x28]
        fld dword ptr [esp + 0x28]
        fxch st(2)
        fadd dword ptr [esp + 0x14]
        fstp dword ptr [eax + 0x2c]
        fld dword ptr [esp + 0x2c]
        fxch st(1)
        fadd dword ptr [esp]
        fstp dword ptr [eax + 0x48]
        fxch st(3)
        fadd dword ptr [esp + 0x4]
        fxch st(4)
        fadd dword ptr [esp + 0x8]
        fxch st(2)
        fadd dword ptr [esp]
        fxch st(1)
        fadd dword ptr [esp + 0x4]
        fxch st(3)
        fadd dword ptr [esp + 0x8]
        fxch st(4)
        fstp dword ptr [eax + 0x4c]
        fxch st(1)
        fstp dword ptr [eax + 0x50]
        fstp dword ptr [eax + 0x54]
        fstp dword ptr [eax + 0x58]
        fstp dword ptr [eax + 0x5c]
        fstp st(0)
        fld dword ptr [edx + 0x14]
        fld dword ptr [edx + 0x14]
        fmul dword ptr [ecx + 0x18]
        fld dword ptr [edx + 0x14]
        fxch st(2)
        fmul dword ptr [ecx + 0x1c]
        fxch st(1)
        fadd dword ptr [ecx + 0x24]
        fxch st(1)
        fadd dword ptr [ecx + 0x28]
        fxch st(2)
        fmul dword ptr [ecx + 0x20]
        fld st(1)
        fld st(3)
        fxch st(1)
        fadd dword ptr [esp + 0xc]
        fxch st(1)
        fadd dword ptr [esp + 0x10]
        fxch st(2)
        fadd dword ptr [ecx + 0x2c]
        fxch st(1)
        fstp dword ptr [eax]
        fxch st(1)
        fstp dword ptr [eax + 0x4]
        fst dword ptr [esp + 0x8]
        fld st(1)
        fld st(3)
        fld dword ptr [esp + 0x8]
        fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x28]
        fxch st(5)
        fadd dword ptr [esp + 0x14]
        fstp dword ptr [eax + 0x8]
        fld dword ptr [esp + 0x2c]
        fxch st(4)
        fadd dword ptr [esp + 0x18]
        fstp dword ptr [eax + 0xc]
        fld dword ptr [esp + 0x30]
        fxch st(3)
        fadd dword ptr [esp + 0x1c]
        fstp dword ptr [eax + 0x10]
        fld dword ptr [esp + 0x34]
        fxch st(2)
        fadd dword ptr [esp + 0x20]
        fstp dword ptr [eax + 0x14]
        fld dword ptr [esp + 0x38]
        fxch st(1)
        fadd st(0), st(6)
        fstp dword ptr [eax + 0x30]
        fxch st(4)
        fadd st(0), st(6)
        fxch st(3)
        fadd dword ptr [esp + 0x8]
        fxch st(2)
        fadd st(0), st(5)
        fxch st(1)
        fadd st(0), st(6)
        fxch st(4)
        fadd dword ptr [esp + 0x8]
        fxch st(3)
        fstp dword ptr [eax + 0x34]
        fxch st(1)
        fstp dword ptr [eax + 0x38]
        fstp dword ptr [eax + 0x3c]
        fxch st(1)
        fstp dword ptr [eax + 0x40]
        fstp dword ptr [eax + 0x44]
        fxch st(1)
        fstp st(0)
        fstp st(0)
        add esp, 0x3c
        ret 0x4

    }
}

// 0x004729b0 Vec3_SubNormalize (ECX a, EDX b, stack out; ret 4) -> ST0 length: out = b - a, then
// Vec3_NormalizeInPlace(out), whose float length is left in ST0 as the return value.
// Converted from the listing with tools/asm_port/ghidra2masm.py (constants and callees mapped).
__declspec(naked) float __fastcall Vec3_SubNormalize(const float*, const float*, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x8
        push ebx
        mov dword ptr [ebp - 0x8], edx
        mov dword ptr [ebp - 0x4], ecx
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp + 0x8]
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
        mov ecx, dword ptr [ebp + 0x8]
        call Vec3_NormalizeInPlace
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004729f0 Vec3_LerpNormalize (ECX v, EDX other, stack t; ret 4): Vec3_Lerp(v, other, t); Vec3_NormalizeInPlace(v).
// Converted from the listing with tools/asm_port/ghidra2masm.py (constants and callees mapped).
__declspec(naked) void __fastcall Vec3_LerpNormalize(float*, const float*, float)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        push esi
        mov esi, ecx
        push eax
        call Vec3_Lerp
        mov ecx, esi
        call Vec3_NormalizeInPlace
        fstp st(0)
        pop esi
        ret 0x4
    }
}

// 0x00475070 Tri_Normal (ECX a, EDX b, stack c, stack out; ret 8): out = normalise((b - a) x (c - a)).
// Converted from the listing with tools/asm_port/ghidra2masm.py (constants and callees mapped).
__declspec(naked) void __fastcall Tri_Normal(const float*, const float*, const float*, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x24
        lea eax, [ebp - 0x24]
        push ebx
        mov dword ptr [ebp - 0xc], edx
        mov dword ptr [ebp - 0x4], ecx
        mov dword ptr [ebp - 0x8], eax
        mov ebx, dword ptr [ebp - 0xc]
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
        lea ecx, [ebp - 0x18]
        mov dword ptr [ebp - 0xc], ecx
        mov ebx, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp - 0xc]
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
        lea edx, [ebp - 0x18]
        lea eax, [ebp - 0x24]
        mov dword ptr [ebp + 0x8], edx
        mov dword ptr [ebp - 0xc], eax
        mov ebx, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp + 0x8]
        mov edx, dword ptr [ebp + 0xc]
        fld dword ptr [ebx]
        fld st(0)
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ebx + 0x4]
        fld st(0)
        fmul dword ptr [ecx + 0x8]
        fld dword ptr [ebx + 0x8]
        fld st(0)
        fmul dword ptr [ecx]
        fxch st(5)
        fmul dword ptr [ecx + 0x8]
        fxch st(3)
        fmul dword ptr [ecx]
        fxch st(3)
        fsubp st(5), st(0)
        fmul dword ptr [ecx + 0x4]
        fxch st(2)
        fsubp st(3), st(0)
        fxch st(1)
        fsubp st(1), st(0)
        fxch st(2)
        fstp dword ptr [edx + 0x4]
        fstp dword ptr [edx + 0x8]
        fstp dword ptr [edx]
        mov ecx, dword ptr [ebp + 0xc]
        call Vec3_NormalizeInPlace
        fstp st(0)
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x8
    }
}

// 0x00475b80 Quat_FromRotationVector (ECX v, EDX out): a = |v|; a == 0 -> (1, 0, 0, 0); else (cos a, v * sin(a) / a).
// Converted from the listing with tools/asm_port/ghidra2masm.py (constants and callees mapped).
__declspec(naked) void __fastcall Quat_FromRotationVector(const float*, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x18
        push esi
        push edi
        mov esi, edx
        mov edi, ecx
        mov dword ptr [ebp - 0x10], esi
        mov dword ptr [ebp - 0xc], edi
        mov ecx, dword ptr [ebp - 0xc]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fsqrt
        fstp dword ptr [ebp - 0x8]
        fld dword ptr [ebp - 0x8]
        fcomp dword ptr [kZeroF_29c8]
        fnstsw AX
        test AH, 0x40
        jz L_475bd7
        xor eax, eax
        mov dword ptr [esi], 0x3f800000
        mov dword ptr [esi + 0x4], eax
        mov dword ptr [esi + 0x8], eax
        mov dword ptr [esi + 0xc], eax
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    L_475bd7:
        fld dword ptr [ebp - 0x8]
        lea eax, [ebp - 0x4]
        fst qword ptr [ebp - 0x18]
        fabs
        fcomp qword ptr [kSinCosLimitD]
        mov dword ptr [ebp - 0xc], eax
        fnstsw AX
        test AH, 0x41
        jnz L_475c00
        fld qword ptr [ebp - 0x18]
        fsin
        fld qword ptr [ebp - 0x18]
        fcos
        fstp dword ptr [esi]
        jmp L_475c14
    L_475c00:
        push ebx
        mov ebx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
        fld qword ptr [ebp - 0x18]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
        fld dword ptr [ebp - 0x4]
        pop ebx
    L_475c14:
        fdiv dword ptr [ebp - 0x8]
        fld st(0)
        fmul dword ptr [edi]
        fstp dword ptr [esi + 0x4]
        fld dword ptr [edi + 0x4]
        fmul st(0), st(1)
        fstp dword ptr [esi + 0x8]
        fld dword ptr [edi + 0x8]
        fmul st(0), st(1)
        pop edi
        fstp dword ptr [esi + 0xc]
        pop esi
        fstp st(0)
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00475210 Math_RaySphereNormal (ECX, EDX, stack r, dir, out; ret 0xc) -> EAX 0/1: stable quadratic with the fast-sqrt
// bit trick on the discriminant, result normalised with Vec3_NormalizeInPlace.
// Converted from the listing with tools/asm_port/ghidra2masm.py (constants and callees mapped).
__declspec(naked) int __fastcall Math_RaySphereNormal(const float*, const float*, float, const float*, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x1c
        lea eax, [ebp - 0x1c]
        push ebx
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0xc], ecx
        mov dword ptr [ebp - 0x8], eax
        mov ebx, dword ptr [ebp - 0xc]
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
        lea ecx, [ebp - 0x1c]
        mov dword ptr [ebp - 0xc], ecx
        mov ecx, dword ptr [ebp - 0xc]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x8]
        fld dword ptr [ebp - 0x8]
        fcomp dword ptr [kZeroF]
        fnstsw AX
        test AH, 0x40
        jz L_475281
        xor eax, eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0xc
    L_475281:
        lea edx, [ebp - 0x1c]
        mov dword ptr [ebp - 0xc], edx
        mov ecx, dword ptr [ebp + 0xc]
        mov edx, dword ptr [ebp - 0xc]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        mov eax, dword ptr [ebp - 0x4]
        mov dword ptr [ebp - 0x10], eax
        mov ecx, dword ptr [ebp + 0xc]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [ecx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [ebp + 0x8]
        fsubr dword ptr [ebp - 0xc]
        fst dword ptr [ebp + 0x8]
        fcomp dword ptr [kZeroF]
        fld dword ptr [ebp - 0x4]
        fnstsw AX
        test AH, 0x40
        jz L_475304
        fcomp dword ptr [kZeroF]
        fnstsw AX
        test AH, 0x41
        jz L_4752fa
        xor eax, eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0xc
    L_4752fa:
        fld dword ptr [ebp - 0x4]
        fadd st(0), st(0)
        fdiv dword ptr [ebp - 0x8]
        jmp L_475367
    L_475304:
        fmul dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x8]
        fmul dword ptr [ebp + 0x8]
        fsubp st(1), st(0)
        fst dword ptr [ebp - 0xc]
        fcomp dword ptr [kZeroF]
        fnstsw AX
        test AH, 0x1
        jz L_475328
        xor eax, eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0xc
    L_475328:
        mov eax, dword ptr [ebp - 0xc]
        sar eax, 0x1
        add eax, 0x1fc00000
        mov dword ptr [ebp - 0x8], eax
        fld dword ptr [ebp + 0x8]
        fcomp dword ptr [kZeroF]
        fnstsw AX
        test AH, 0x1
        jz L_475354
        fld dword ptr [ebp + 0x8]
        fchs
        fstp dword ptr [ebp + 0x8]
        fld dword ptr [ebp - 0x4]
        fchs
        jmp L_475357
    L_475354:
        fld dword ptr [ebp - 0x10]
    L_475357:
        fcom dword ptr [ebp - 0x8]
        fnstsw AX
        test AH, 0x41
        jnz L_4753bc
        fsub dword ptr [ebp - 0x8]
    L_475364:
        fdivr dword ptr [ebp + 0x8]
    L_475367:
        fld st(0)
        fmul dword ptr [ebp - 0x1c]
        lea ecx, [ebp - 0x1c]
        mov dword ptr [ebp + 0x8], ecx
        fstp dword ptr [ebp - 0x1c]
        fld st(0)
        fmul dword ptr [ebp - 0x18]
        fstp dword ptr [ebp - 0x18]
        fmul dword ptr [ebp - 0x14]
        fstp dword ptr [ebp - 0x14]
        mov ebx, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [ebp + 0x8]
        mov edx, dword ptr [ebp + 0x10]
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
        mov ecx, dword ptr [ebp + 0x10]
        call Vec3_NormalizeInPlace
        fstp st(0)
        mov eax, 0x1
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0xc
    L_4753bc:
        fadd dword ptr [ebp - 0x8]
        fcom dword ptr [kZeroF]
        fnstsw AX
        test AH, 0x41
        jz L_475364
        fstp st(0)
        xor eax, eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0xc
    }
}

// 0x00474e10 Mat3_ToEuler (ECX matrix, EDX out pitch/yaw/roll): yaw = Mat3_YawFromRows; pitch = atan2(-m7, hypot);
// roll from row 0 rotated by -yaw (Vec3_RotateY) and -pitch (Vec3_RotateX).
// Converted from the listing with tools/asm_port/ghidra2masm.py (constants and callees mapped).
__declspec(naked) void __fastcall Mat3_ToEuler(const float*, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x28
        push esi
        push edi
        mov edi, edx
        mov esi, ecx
        call Mat3_YawFromRows
        fstp dword ptr [ebp - 0x8]
        lea eax, [esi + 0x18]
        mov dword ptr [ebp - 0x4], eax
        mov ecx, dword ptr [ebp - 0x4]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        faddp st(1), st(0)
        fsqrt
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [esi + 0x1c]
        fchs
        fld dword ptr [ebp - 0xc]
        fpatan
        push ecx
        mov edx, esi
        lea ecx, [ebp - 0x28]
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x8]
        fchs
        fstp dword ptr [esp]
        call Vec3_RotateY
        fld dword ptr [ebp - 0x4]
        push ecx
        lea edx, [ebp - 0x28]
        fchs
        fstp dword ptr [esp]
        lea ecx, [ebp - 0x1c]
        call Vec3_RotateX
        lea ecx, [ebp - 0x1c]
        mov dword ptr [ebp - 0xc], ecx
        mov ecx, dword ptr [ebp - 0xc]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        faddp st(1), st(0)
        fsqrt
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x18]
        fld dword ptr [ebp - 0x10]
        fpatan
        fld dword ptr [esi + 0x10]
        fcomp dword ptr [kZeroF]
        fnstsw AX
        test AH, 0x1
        jz L_474eaa
        fsubr dword ptr [kPiF_2998]
    L_474eaa:
        mov edx, dword ptr [ebp - 0x4]
        mov eax, dword ptr [ebp - 0x8]
        fstp dword ptr [edi + 0x8]
        mov dword ptr [edi], edx
        mov dword ptr [edi + 0x4], eax
        pop edi
        pop esi
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00472a10 Vec3_Slerp (ECX from, EDX to, stack t, stack out; ret 8): t == 0 / t == 1 copies; dot < -0.95 builds a
// perpendicular (Vec2_PerpNormalized); dot > 0.95 lerps; else the fsin/fpatan slerp.
// Converted from the listing with tools/asm_port/ghidra2masm.py (constants and callees mapped).
__declspec(naked) void __fastcall Vec3_Slerp(const float*, const float*, float, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x24
        fld dword ptr [ebp + 0x8]
        fcomp dword ptr [kZeroF_2918]
        push ebx
        push esi
        push edi
        mov edi, edx
        mov esi, ecx
        mov dword ptr [ebp - 0x14], edi
        fnstsw AX
        mov dword ptr [ebp - 0x18], esi
        test AH, 0x40
        jz L_472a4f
        mov eax, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [esi]
        mov dword ptr [eax], ecx
        mov edx, dword ptr [esi + 0x4]
        mov dword ptr [eax + 0x4], edx
        mov ecx, dword ptr [esi + 0x8]
        mov dword ptr [eax + 0x8], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x8
    L_472a4f:
        fld dword ptr [ebp + 0x8]
        fcomp dword ptr [kOneF_291c]
        fnstsw AX
        test AH, 0x40
        jz L_472a7b
        mov edx, dword ptr [ebp + 0xc]
        mov eax, dword ptr [edi]
        mov dword ptr [edx], eax
        mov ecx, dword ptr [edi + 0x4]
        mov dword ptr [edx + 0x4], ecx
        mov eax, dword ptr [edi + 0x8]
        mov dword ptr [edx + 0x8], eax
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x8
    L_472a7b:
        mov ecx, dword ptr [ebp - 0x18]
        mov edx, dword ptr [ebp - 0x14]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [ebp - 0xc]
        fcomp qword ptr [kNeg095D]
        fnstsw AX
        test AH, 0x1
        jz L_472b69
        lea edx, [ebp - 0x24]
        mov ecx, esi
        call Vec2_PerpNormalized
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [kPiF_2938]
        lea ecx, [ebp - 0x8]
        lea edx, [ebp - 0x4]
        mov dword ptr [ebp - 0x18], ecx
        mov dword ptr [ebp - 0x14], edx
        fst qword ptr [ebp - 0x10]
        fld st(0)
        fabs
        fcomp qword ptr [kSinCosLimitD]
        fnstsw AX
        test AH, 0x41
        jnz L_472aef
        fld st(0)
        fsin
        fstp dword ptr [ebp - 0x4]
        fcos
        fstp dword ptr [ebp - 0x8]
        jmp L_472b00
    L_472aef:
        fstp st(0)
        mov ebx, dword ptr [ebp - 0x14]
        mov edx, dword ptr [ebp - 0x18]
        fld qword ptr [ebp - 0x10]
        fsincos
        fstp dword ptr [edx]
        fstp dword ptr [ebx]
    L_472b00:
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [ebp - 0x4]
        mov eax, dword ptr [ebp + 0xc]
        fstp dword ptr [ebp - 0x24]
        fld dword ptr [ebp - 0x20]
        fmul dword ptr [ebp - 0x4]
        fstp dword ptr [ebp - 0x20]
        fld dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x4]
        fstp dword ptr [ebp - 0x1c]
        fld dword ptr [ebp - 0x8]
        fmul dword ptr [esi]
        fstp dword ptr [eax]
        fld dword ptr [esi + 0x4]
        fmul dword ptr [ebp - 0x8]
        fstp dword ptr [eax + 0x4]
        fld dword ptr [esi + 0x8]
        fmul dword ptr [ebp - 0x8]
        fstp dword ptr [eax + 0x8]
        lea eax, [ebp - 0x24]
        mov dword ptr [ebp + 0x8], eax
        mov ebx, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [ebp + 0xc]
        mov edx, dword ptr [ebp + 0xc]
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
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x8
    L_472b69:
        fld dword ptr [ebp - 0xc]
        fcomp qword ptr [kPos095D]
        fnstsw AX
        test AH, 0x41
        jnz L_472be9
        fld dword ptr [kOneF_291c]
        fsub dword ptr [ebp + 0x8]
        mov eax, dword ptr [ebp + 0xc]
        lea ecx, [ebp - 0x24]
        fld st(0)
        fmul dword ptr [esi]
        fstp dword ptr [eax]
        fld dword ptr [esi + 0x4]
        fmul st(0), st(1)
        fstp dword ptr [eax + 0x4]
        fld dword ptr [esi + 0x8]
        fmul st(0), st(1)
        fstp dword ptr [eax + 0x8]
        fstp st(0)
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [edi]
        fstp dword ptr [ebp - 0x24]
        fld dword ptr [edi + 0x4]
        fmul dword ptr [ebp + 0x8]
        fstp dword ptr [ebp - 0x20]
        fld dword ptr [edi + 0x8]
        fmul dword ptr [ebp + 0x8]
        mov dword ptr [ebp + 0x8], ecx
        fstp dword ptr [ebp - 0x1c]
        mov ebx, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [ebp + 0x8]
        mov edx, dword ptr [ebp + 0xc]
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
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x8
    L_472be9:
        fld dword ptr [ebp - 0xc]
        fmul dword ptr [ebp - 0xc]
        fsubr dword ptr [kOneF_291c]
        fst dword ptr [ebp - 0x8]
        fcomp dword ptr [kZeroF_2918]
        fnstsw AX
        test AH, 0x41
        jz L_472c0e
        mov dword ptr [ebp - 0x4], 0x0
        jmp L_472c21
    L_472c0e:
        mov eax, dword ptr [ebp - 0x8]
        sar eax, 0x1
        add eax, 0x1fc00000
        mov dword ptr [ebp - 0x18], eax
        mov edx, dword ptr [ebp - 0x18]
        mov dword ptr [ebp - 0x4], edx
    L_472c21:
        fld dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0xc]
        fpatan
        mov eax, dword ptr [ebp + 0xc]
        lea ecx, [ebp - 0x24]
        fld dword ptr [kOneF_291c]
        fsub dword ptr [ebp + 0x8]
        fmul st(0), st(1)
        fsin
        fld st(0)
        fmul dword ptr [esi]
        fstp dword ptr [eax]
        fld dword ptr [esi + 0x4]
        fmul st(0), st(1)
        fstp dword ptr [eax + 0x4]
        fld dword ptr [esi + 0x8]
        fmul st(0), st(1)
        fstp dword ptr [eax + 0x8]
        fstp st(0)
        fmul dword ptr [ebp + 0x8]
        mov dword ptr [ebp + 0x8], ecx
        fsin
        fld st(0)
        fmul dword ptr [edi]
        fstp dword ptr [ebp - 0x24]
        fld dword ptr [edi + 0x4]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x20]
        fld dword ptr [edi + 0x8]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x1c]
        fstp st(0)
        mov ebx, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [ebp + 0x8]
        mov edx, dword ptr [ebp + 0xc]
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
        fld dword ptr [kOneF_291c]
        fdiv dword ptr [ebp - 0x4]
        pop edi
        pop esi
        pop ebx
        fld st(0)
        fmul dword ptr [eax]
        fstp dword ptr [eax]
        fld st(0)
        fmul dword ptr [eax + 0x4]
        fstp dword ptr [eax + 0x4]
        fmul dword ptr [eax + 0x8]
        fstp dword ptr [eax + 0x8]
        mov esp, ebp
        pop ebp
        ret 0x8
    }
}

}  // namespace recoil
