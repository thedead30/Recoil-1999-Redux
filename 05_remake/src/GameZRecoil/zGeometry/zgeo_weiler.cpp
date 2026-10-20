// SUBSYSTEM: geometry
// Original file: GameZRecoil\zGeometry\zgeo_weiler.cpp (ledger orig_file). Spec: 04_spec/systems/geometry.md
// Instruction-level ports: the clipper's structures are read and written at the listing's offsets, so the
// port is layout-exact without restating the layouts here (they are documented in the spec). Each body was
// converted from the Ghidra listing with tools/asm_port/ghidra2masm.py; unmapped absolute references are an
// error in the converter.
#include "GameZRecoil/zGeometry/zgeo_weiler.h"
#include "platform/msvcrt.h"  // MSVCRT import slots (calloc, realloc, free, _iob, fprintf)
#include "unattributed/ui_widgets.h"  // Debug_ReportNoop 0x00404e80

namespace recoil {

// Entry counts for coverage measurement (tests/test_weiler_clip_native.cpp). Compiled in only by the coverage
// build (-DRECOIL_ENTRY_COUNTS=ON); the normal build has no extra instructions. The increment leaves every
// register and flag as it was (PUSH/MOV/LEA/MOV/POP).
#ifdef RECOIL_ENTRY_COUNTS
unsigned g_WeilerEntryCounts[kWeilerEntryCountSlots];
#define RECOIL_ENTRY(i)                                                   \
    __asm push eax                                                        \
    __asm mov eax, dword ptr [g_WeilerEntryCounts + 4 * (i)]              \
    __asm lea eax, [eax + 1]                                              \
    __asm mov dword ptr [g_WeilerEntryCounts + 4 * (i)], eax              \
    __asm pop eax
#else
#define RECOIL_ENTRY(i)
#endif

// 0x00464670 WeilerClip_GetOutput (ECX clip, EDX out): null -> 0; *out = [clip+0x18]; returns [clip+0x14].
__declspec(naked) int __fastcall WeilerClip_GetOutput(const void*, int*)
{
    __asm {
        test ecx, ecx
        jnz L_464677
        xor eax, eax
        ret
    L_464677:
        mov eax, dword ptr [ecx + 0x18]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x14]
        ret
    }
}

// 0x00468410 ClipEdge_ComputeBounds (ECX edge): min/max of the two endpoint x and y ([+0xc], [+0x10]) into +0x18..+0x28.
__declspec(naked) void __fastcall ClipEdge_ComputeBounds(void*)
{
    __asm {
        mov edx, dword ptr [ecx + 0xc]
        push esi
        mov esi, dword ptr [ecx + 0x10]
        fld dword ptr [edx]
        fcomp dword ptr [esi]
        fnstsw AX
        test AH, 0x41
        jnz L_46842b
        mov eax, dword ptr [esi]
        mov dword ptr [ecx + 0x18], eax
        mov eax, dword ptr [edx]
        jmp L_468432
    L_46842b:
        mov eax, dword ptr [edx]
        mov dword ptr [ecx + 0x18], eax
        mov eax, dword ptr [esi]
    L_468432:
        mov dword ptr [ecx + 0x24], eax
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [esi + 0x4]
        fnstsw AX
        test AH, 0x41
        jnz L_468457
        mov eax, dword ptr [esi + 0x4]
        mov dword ptr [ecx + 0x1c], eax
        mov edx, dword ptr [edx + 0x4]
        mov dword ptr [ecx + 0x28], edx
        mov dword ptr [ecx + 0x14], 0x0
        pop esi
        ret
    L_468457:
        mov eax, dword ptr [edx + 0x4]
        mov dword ptr [ecx + 0x1c], eax
        mov edx, dword ptr [esi + 0x4]
        mov dword ptr [ecx + 0x28], edx
        mov dword ptr [ecx + 0x14], 0x0
        pop esi
        ret
    }
}

// 0x004683a0 ClipContour_SwapAxes (ECX contour): per point, swaps a component pair chosen by [+0x4] == 2.
__declspec(naked) void __fastcall ClipContour_SwapAxes(void*)
{
    __asm {
        mov eax, dword ptr [ecx + 0x2c]
        push esi
        test eax, eax
        jz L_4683d3
        mov edx, dword ptr [ecx + 0x4]
        cmp edx, 0x2
        lea edx, [eax + 0x4]
        jz L_4683b5
        mov edx, eax
    L_4683b5:
        mov ecx, dword ptr [ecx + 0x28]
        add eax, 0x8
        test ecx, ecx
        jz L_4683ff
    L_4683bf:
        fld dword ptr [edx]
        mov esi, dword ptr [eax]
        add eax, 0xc
        mov dword ptr [edx], esi
        add edx, 0xc
        fstp dword ptr [eax - 0xc]
        dec ecx
        jnz L_4683bf
        pop esi
        ret
    L_4683d3:
        mov eax, dword ptr [ecx + 0x4]
        cmp eax, 0x2
        mov eax, dword ptr [ecx + 0x18]
        lea edx, [eax + 0x4]
        jz L_4683e3
        mov edx, eax
    L_4683e3:
        mov ecx, dword ptr [ecx + 0x14]
        add eax, 0x8
        test ecx, ecx
        jz L_4683ff
    L_4683ed:
        fld dword ptr [edx]
        mov esi, dword ptr [eax]
        add eax, 0xc
        mov dword ptr [edx], esi
        add edx, 0xc
        fstp dword ptr [eax - 0xc]
        dec ecx
        jnz L_4683ed
    L_4683ff:
        pop esi
        ret
    }
}

// 0x004693a0 ClipArray_Call468410Each (ECX array of 0x3c-byte edges, EDX count): ClipEdge_ComputeBounds on each.
__declspec(naked) void __fastcall ClipArray_Call468410Each(void*, int)
{
    __asm {
        mov eax, edx
        dec edx
        push esi
        mov esi, ecx
        test eax, eax
        jz L_4693bc
        push edi
        lea edi, [edx + 0x1]
    L_4693ae:
        mov ecx, esi
        add esi, 0x3c
        call ClipEdge_ComputeBounds
        dec edi
        jnz L_4693ae
        pop edi
    L_4693bc:
        pop esi
        ret
    }
}

// 0x00469430 ClipEdge_OrientFirst (ECX edge): if the owner's second link is this edge, swaps the owner's link pairs.
__declspec(naked) void __fastcall ClipEdge_OrientFirst(void*)
{
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        cmp dword ptr [eax + 0x4], ecx
        jnz L_46944b
        mov edx, dword ptr [eax]
        mov dword ptr [eax], ecx
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [eax + 0x4], edx
        mov edx, dword ptr [eax + 0x10]
        mov dword ptr [eax + 0x10], ecx
        mov dword ptr [eax + 0xc], edx
    L_46944b:
        ret
    }
}

// 0x00469ae0 IntLine_EvalAt (ECX line, EDX t): [+0x8] = t; [+0x10] = t * [+0x0] + [+0xc] (integer).
__declspec(naked) void __fastcall IntLine_EvalAt(void*, int)
{
    __asm {
        mov dword ptr [ecx + 0x8], edx
        imul edx, dword ptr [ecx]
        add edx, dword ptr [ecx + 0xc]
        mov dword ptr [ecx + 0x10], edx
        ret
    }
}

// 0x00469e50 Point2_NearlyEqual (ECX a, EDX b, stack eps; ret 4): |ax - bx| <= eps and |ay - by| <= eps -> 1.
__declspec(naked) int __fastcall Point2_NearlyEqual(const float*, const float*, float)
{
    __asm {
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [edx + 0x4]
        fxch st(1)
        fabs
        fxch st(1)
        fxch st(1)
        fcomp dword ptr [esp + 0x4]
        fnstsw AX
        test AH, 0x41
        fabs
        jz L_469e82
        fcomp dword ptr [esp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_469e84
        mov eax, 0x1
        ret 0x4
    L_469e82:
        fstp st(0)
    L_469e84:
        xor eax, eax
        ret 0x4
    }
}

// 0x0046a5e0 Vec3Array_RotateX90 (ECX count, EDX points): (x, y, z) -> (x, z, -y) in place.
__declspec(naked) void __fastcall Vec3Array_RotateX90(int, float*)
{
    __asm {
        mov eax, ecx
        dec ecx
        test eax, eax
        jz L_46a5fd
        lea eax, [edx + 0x8]
        inc ecx
    L_46a5eb:
        fld dword ptr [eax - 0x4]
        mov edx, dword ptr [eax]
        fchs
        fstp dword ptr [eax]
        mov dword ptr [eax - 0x4], edx
        add eax, 0xc
        dec ecx
        jnz L_46a5eb
    L_46a5fd:
        ret
    }
}

// 0x0046a600 Vec3Array_RotateX90Inverse (ECX count, EDX points): (x, y, z) -> (x, -z, y) in place.
__declspec(naked) void __fastcall Vec3Array_RotateX90Inverse(int, float*)
{
    __asm {
        mov eax, ecx
        dec ecx
        test eax, eax
        jz L_46a61d
        lea eax, [edx + 0x4]
        inc ecx
    L_46a60b:
        fld dword ptr [eax + 0x4]
        mov edx, dword ptr [eax]
        fchs
        fstp dword ptr [eax]
        mov dword ptr [eax + 0x4], edx
        add eax, 0xc
        dec ecx
        jnz L_46a60b
    L_46a61d:
        ret
    }
}

// 0x00467600 DynArray_Init (ECX array, EDX capacity, stack element size; ret 4): calloc(capacity, size); count 0.
__declspec(naked) void __fastcall DynArray_Init(void*, int, int)
{
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 0x8]
        push esi
        push edi
        mov edi, edx
        push ebx
        mov esi, ecx
        push edi
        call dword ptr [g_Iat_calloc_004cc4ac]
        add esp, 0x8
        mov dword ptr [esi + 0x4], edi
        mov dword ptr [esi + 0xc], eax
        mov dword ptr [esi], ebx
        mov dword ptr [esi + 0x8], 0x0
        mov dword ptr [esi + 0x10], eax
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x00467630 DynArray_Free (ECX array): frees the buffer at [+0xc] if any and zeroes the header.
__declspec(naked) void __fastcall DynArray_Free(void*)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        xor edi, edi
        mov eax, dword ptr [esi + 0xc]
        cmp eax, edi
        jz L_467655
        push eax
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        mov dword ptr [esi + 0x4], edi
        mov dword ptr [esi + 0x8], edi
        mov dword ptr [esi], edi
        mov dword ptr [esi + 0xc], edi
        mov dword ptr [esi + 0x10], edi
    L_467655:
        pop edi
        pop esi
        ret
    }
}

// 0x00467660 DynArray_Push (ECX array, EDX n, stack out-old-buffer or null; ret 4): grows by realloc when full, then
// advances the count by n and the cursor [+0x10].
__declspec(naked) void __fastcall DynArray_Push(void*, int, void**)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, ecx
        mov edi, edx
        mov edx, dword ptr [esi + 0x8]
        mov eax, dword ptr [esi + 0x4]
        mov ebx, edi
        add ebx, edx
        cmp ebx, eax
        jc L_4676a7
        mov ecx, dword ptr [esi]
        lea eax, [edi + eax*0x1 + 0x10]
        imul ecx, eax
        mov edx, dword ptr [esi + 0xc]
        push ecx
        push edx
        mov dword ptr [esi + 0x4], eax
        call dword ptr [g_Iat_realloc_004cc4ec]
        mov ecx, dword ptr [esi]
        add esp, 0x8
        imul ecx, dword ptr [esi + 0x8]
        add ecx, eax
        mov dword ptr [esi + 0xc], eax
        mov dword ptr [esi + 0x10], ecx
        mov ecx, dword ptr [esp + 0x10]
        test ecx, ecx
        jz L_4676a7
        mov dword ptr [ecx], eax
    L_4676a7:
        imul edi, dword ptr [esi]
        mov eax, dword ptr [esi + 0x10]
        mov dword ptr [esi + 0x8], ebx
        add edi, eax
        mov dword ptr [esi + 0x10], edi
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004647d0 WeilerClip_Destroy (ECX clip or null): frees the four dynamic arrays (+0x34, +0x48, +0x5c, +0xc), then the clip.
__declspec(naked) void __fastcall WeilerClip_Destroy(void*)
{
    __asm {
        push esi
        mov esi, ecx
        test esi, esi
        jz L_464801
        lea ecx, [esi + 0x34]
        call DynArray_Free
        lea ecx, [esi + 0x48]
        call DynArray_Free
        lea ecx, [esi + 0x5c]
        call DynArray_Free
        lea ecx, [esi + 0xc]
        call DynArray_Free
        push esi
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_464801:
        pop esi
        ret
    }
}

// 0x00464b30 WeilerClip_FreeBuffers (ECX clip or null): frees and nulls [+0x1c], [+0x4], [+0xc], [+0x14].
__declspec(naked) void __fastcall WeilerClip_FreeBuffers(void*)
{
    __asm {
        push ebx
        push esi
        mov esi, ecx
        xor ebx, ebx
        cmp esi, ebx
        push edi
        jz L_464b81
        mov eax, dword ptr [esi + 0x1c]
        mov edi, dword ptr [g_Iat_free_004cc5b4]
        cmp eax, ebx
        jz L_464b51
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [esi + 0x1c], ebx
    L_464b51:
        mov eax, dword ptr [esi + 0x4]
        cmp eax, ebx
        jz L_464b61
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [esi + 0x4], ebx
    L_464b61:
        mov eax, dword ptr [esi + 0xc]
        cmp eax, ebx
        jz L_464b71
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [esi + 0xc], ebx
    L_464b71:
        mov eax, dword ptr [esi + 0x14]
        cmp eax, ebx
        jz L_464b81
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [esi + 0x14], ebx
    L_464b81:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x00468700 ClipContour_AppendToOutput (ECX contour, EDX pass): appends the pass's points to the output (realloc above
// 0x80 points) -> AL = 1.
__declspec(naked) bool __fastcall ClipContour_AppendToOutput(void*, int)
{
    __asm {
        push ebx
        mov ebx, ecx
        test byte ptr [ebx], 0x1
        jnz L_46870c
        mov AL, 0x1
        pop ebx
        ret
    L_46870c:
        push edi
        push esi
        push ebp
        cmp edx, 0x3
        lea ebp, [ebx + 0x20]
        jz L_46871a
        lea ebp, [ebx + 0xc]
    L_46871a:
        mov eax, dword ptr [ebx + 0x8]
        mov dword ptr [eax], 0x1
        mov ecx, dword ptr [ebx + 0x8]
        mov eax, dword ptr [ebp + 0x8]
        mov edx, dword ptr [ecx + 0x18]
        add eax, edx
        cmp eax, 0x80
        jbe L_46874f
        lea edx, [eax + eax*0x2]
        mov eax, dword ptr [ecx + 0x1c]
        shl edx, 0x2
        push edx
        push eax
        call dword ptr [g_Iat_realloc_004cc4ec]
        mov ecx, dword ptr [ebx + 0x8]
        add esp, 0x8
        mov dword ptr [ecx + 0x1c], eax
    L_46874f:
        mov edx, dword ptr [ebx + 0x8]
        mov ecx, dword ptr [ebp + 0x8]
        mov eax, dword ptr [edx + 0x4]
        mov dword ptr [eax], ecx
        mov eax, dword ptr [ebx + 0x8]
        mov ecx, dword ptr [eax + 0x18]
        mov eax, dword ptr [eax + 0x4]
        lea edx, [ecx + ecx*0x2]
        mov dword ptr [eax + 0x4], edx
        mov eax, dword ptr [ebx + 0x8]
        mov ecx, dword ptr [ebp + 0x8]
        mov esi, dword ptr [ebp + 0xc]
        mov edx, dword ptr [eax + 0x18]
        mov eax, dword ptr [eax + 0x1c]
        lea ecx, [ecx + ecx*0x2]
        lea edx, [edx + edx*0x2]
        shl ecx, 0x2
        lea edi, [eax + edx*0x4]
        mov edx, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov ebx, dword ptr [ebx + 0x8]
        mov eax, dword ptr [ebp + 0x8]
        pop ebp
        pop esi
        mov ecx, dword ptr [ebx + 0x18]
        pop edi
        add ecx, eax
        mov AL, 0x1
        mov dword ptr [ebx + 0x18], ecx
        pop ebx
        ret
    }
}

namespace {
const double kD_004d2618 = 9.999999747378752e-06;  // CONFIRMED-BINARY: double 0x3ee4f8b580000000 at 0x004d2618
const float kF_004d2620 = 0.0;  // CONFIRMED-BINARY: float 0x00000000 at 0x004d2620
const float kF_004d2624 = 1.0;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d2624
const float kF_004d2648 = -1.0;  // CONFIRMED-BINARY: float 0xbf800000 at 0x004d2648
const float kF_004d264c = 1.0;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d264c
}  // namespace

// 0x00468a10 Point2_InPolygonQuadrant (ECX point, EDX count, stack xyz polygon; ret 4): crossing parity -> 1 inside,
// -1 outside; on the boundary a non-negative FPU-status-derived value. Callers only test < 0.
__declspec(naked) int __fastcall Point2_InPolygonQuadrant(const float*, int, const float*)
{
    __asm {
        sub esp, 0x10
        push ebx
        mov ebx, edx
        mov edx, ecx
        push ebp
        mov ecx, dword ptr [esp + 0x1c]
        lea eax, [ebx + ebx*0x2]
        push esi
        push edi
        fld dword ptr [ecx + eax*0x4 - 0xc]
        fld dword ptr [ecx + eax*0x4 - 0x8]
        lea eax, [ecx + eax*0x4]
        mov dword ptr [esp + 0x10], edx
        fld st(1)
        fcomp dword ptr [edx]
        fnstsw AX
        test AH, 0x1
        jz L_468a41
        or edi, 0xffffffff
        jmp L_468a55
    L_468a41:
        fld st(1)
        fcomp dword ptr [edx]
        fnstsw AX
        test AH, 0x41
        jnz L_468a53
        mov edi, 0x1
        jmp L_468a55
    L_468a53:
        xor edi, edi
    L_468a55:
        fld st(0)
        fld dword ptr [edx + 0x4]
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_468a6a
        or esi, 0xffffffff
        jmp L_468a81
    L_468a6a:
        fld dword ptr [edx + 0x4]
        fld st(1)
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_468a7f
        mov esi, 0x1
        jmp L_468a81
    L_468a7f:
        xor esi, esi
    L_468a81:
        test edi, edi
        mov dword ptr [esp + 0x24], 0x0
        jnz L_468aa1
        test esi, esi
        jnz L_468aa1
        fstp st(0)
        fstp st(0)
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret 0x4
    L_468aa1:
        mov eax, ebx
        dec ebx
        test eax, eax
        mov dword ptr [esp + 0x18], ebx
        jz L_468bf3
    L_468ab0:
        fld dword ptr [ecx]
        fxch st(2)
        fld st(2)
        fcomp dword ptr [edx]
        fld dword ptr [ecx + 0x4]
        fxch st(1)
        fstp dword ptr [esp + 0x1c]
        fxch st(1)
        fstp dword ptr [esp + 0x14]
        fnstsw AX
        mov ebp, edi
        mov ebx, esi
        test AH, 0x1
        jz L_468ad7
        or edi, 0xffffffff
        jmp L_468aeb
    L_468ad7:
        fld st(1)
        fcomp dword ptr [edx]
        fnstsw AX
        test AH, 0x41
        jnz L_468ae9
        mov edi, 0x1
        jmp L_468aeb
    L_468ae9:
        xor edi, edi
    L_468aeb:
        fld dword ptr [edx + 0x4]
        fld st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_468afe
        or esi, 0xffffffff
        jmp L_468b15
    L_468afe:
        fld dword ptr [edx + 0x4]
        fld st(1)
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_468b13
        mov esi, 0x1
        jmp L_468b15
    L_468b13:
        xor esi, esi
    L_468b15:
        add ecx, 0xc
        test edi, edi
        jnz L_468b24
        test esi, esi
        jz L_468c21
    L_468b24:
        cmp edi, ebp
        jz L_468bbb
        xor edx, edx
        test esi, esi
        setge DL
        xor eax, eax
        test ebx, ebx
        setge AL
        cmp edx, eax
        jz L_468bb1
        mov edx, dword ptr [esp + 0x10]
        fld dword ptr [esp + 0x14]
        fld dword ptr [edx + 0x4]
        fsub st(0), st(2)
        fxch st(1)
        fsub st(0), st(2)
        fld dword ptr [esp + 0x1c]
        fxch st(1)
        fdivp st(2), st(0)
        fsub st(0), st(3)
        fmulp st(1), st(0)
        fadd st(0), st(2)
        fcom dword ptr [edx]
        fnstsw AX
        test AH, 0x1
        jz L_468b72
        fstp st(0)
        mov dword ptr [esp + 0x14], 0xffffffff
        jmp L_468b8b
    L_468b72:
        fcomp dword ptr [edx]
        mov dword ptr [esp + 0x14], 0x1
        fnstsw AX
        test AH, 0x41
        jz L_468b8b
        mov dword ptr [esp + 0x14], 0x0
    L_468b8b:
        fild dword ptr [esp + 0x14]
        fld st(0)
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x40
        jnz L_468c0f
        fcomp dword ptr [kF_004d2624]
        fnstsw AX
        test AH, 0x40
        jz L_468be0
        inc dword ptr [esp + 0x24]
        jmp L_468be0
    L_468bb1:
        test esi, esi
        jnz L_468bdc
        test ebx, ebx
        jz L_468c21
        jmp L_468bdc
    L_468bbb:
        xor edx, edx
        test esi, esi
        setge DL
        xor eax, eax
        test ebx, ebx
        setge AL
        cmp edx, eax
        jz L_468bdc
        cmp edi, 0x1
        jnz L_468bd8
        inc dword ptr [esp + 0x24]
        jmp L_468bdc
    L_468bd8:
        test edi, edi
        jz L_468c21
    L_468bdc:
        mov edx, dword ptr [esp + 0x10]
    L_468be0:
        mov eax, dword ptr [esp + 0x18]
        mov ebx, eax
        dec eax
        test ebx, ebx
        mov dword ptr [esp + 0x18], eax
        jnz L_468ab0
    L_468bf3:
        mov eax, dword ptr [esp + 0x24]
        and AL, 0x1
        neg AL
        sbb AL, AL
        fstp st(0)
        and AL, 0x2
        fstp st(0)
        dec AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret 0x4
    L_468c0f:
        fstp st(0)
        fstp st(0)
        fstp st(0)
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret 0x4
    L_468c21:
        pop edi
        pop esi
        fstp st(0)
        pop ebp
        xor AL, AL
        fstp st(0)
        pop ebx
        add esp, 0x10
        ret 0x4
    }
}

// 0x00469ca0 Point2_OnSegmentRange (ECX p, EDX a, stack b; ret 4): p within the a..b range on x (or y when |a.x - b.x| < 1e-5).
__declspec(naked) int __fastcall Point2_OnSegmentRange(const float*, const float*, const float*)
{
    __asm {
        fld dword ptr [edx]
        push esi
        mov esi, dword ptr [esp + 0x8]
        fsub dword ptr [esi]
        fabs
        fcomp qword ptr [kD_004d2618]
        fnstsw AX
        test AH, 0x1
        jz L_469d08
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [esi + 0x4]
        fld dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_469ce8
        fcomp dword ptr [esi + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_469d4f
        fld dword ptr [ecx + 0x4]
        fcomp dword ptr [edx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_469d4f
        mov eax, 0x1
        pop esi
        ret 0x4
    L_469ce8:
        fcomp dword ptr [edx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_469d4f
        fld dword ptr [ecx + 0x4]
        fcomp dword ptr [esi + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_469d4f
        mov eax, 0x1
        pop esi
        ret 0x4
    L_469d08:
        fld dword ptr [edx]
        fcomp dword ptr [esi]
        fld dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_469d32
        fcomp dword ptr [esi]
        fnstsw AX
        test AH, 0x1
        jnz L_469d4f
        fld dword ptr [ecx]
        fcomp dword ptr [edx]
        fnstsw AX
        test AH, 0x41
        jz L_469d4f
        mov eax, 0x1
        pop esi
        ret 0x4
    L_469d32:
        fcomp dword ptr [edx]
        fnstsw AX
        test AH, 0x1
        jnz L_469d4f
        fld dword ptr [ecx]
        fcomp dword ptr [esi]
        fnstsw AX
        test AH, 0x41
        jz L_469d4f
        mov eax, 0x1
        pop esi
        ret 0x4
    L_469d4f:
        xor eax, eax
        pop esi
        ret 0x4
    }
}

// 0x00469e90 Point2_SnapOntoSegment (ECX a, EDX b, stack point, stack eps; ret 8): snaps the point onto a..b when within eps -> 1.
__declspec(naked) int __fastcall Point2_SnapOntoSegment(const float*, const float*, float*, float)
{
    __asm {
        sub esp, 0x18
        fld dword ptr [edx]
        fsub dword ptr [ecx]
        fld dword ptr [esp + 0x20]
        fld dword ptr [edx + 0x4]
        push esi
        mov esi, dword ptr [esp + 0x20]
        fld dword ptr [esi]
        fxch st(3)
        fst dword ptr [esp + 0x4]
        fld dword ptr [esi + 0x4]
        fxch st(1)
        fabs
        fxch st(4)
        fsub dword ptr [ecx]
        fxch st(4)
        fxch st(1)
        fxch st(3)
        fcompp
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fnstsw AX
        fstp dword ptr [esp + 0xc]
        test AH, 0x41
        fstp dword ptr [esp + 0x20]
        fld dword ptr [esp + 0x24]
        jnz L_469f28
        fxch st(1)
        fabs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jnz L_46a06c
        fld dword ptr [esp + 0x20]
        fdiv dword ptr [esp + 0xc]
        fcom dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_46a06a
        fcomp dword ptr [kF_004d2624]
        fnstsw AX
        test AH, 0x1
        jz L_46a06c
        mov eax, dword ptr [ecx]
        mov dword ptr [esi], eax
        mov eax, 0x1
        pop esi
        add esp, 0x18
        ret 0x8
    L_469f28:
        fld dword ptr [esp + 0xc]
        fabs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_469f8a
        fld dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x20]
        fabs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jnz L_46a06a
        fdiv dword ptr [esp + 0x4]
        fcom dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_46a06a
        fcomp dword ptr [kF_004d2624]
        fnstsw AX
        test AH, 0x1
        jz L_46a06c
        mov ecx, dword ptr [ecx + 0x4]
        mov eax, 0x1
        mov dword ptr [esi + 0x4], ecx
        pop esi
        add esp, 0x18
        ret 0x8
    L_469f8a:
        fld dword ptr [esp + 0x20]
        fxch st(1)
        fdiv dword ptr [esp + 0x4]
        fxch st(1)
        fdiv dword ptr [esp + 0xc]
        fxch st(1)
        fst dword ptr [esp + 0x20]
        fxch st(1)
        fstp dword ptr [esp + 0x8]
        fsub dword ptr [esp + 0x8]
        fld dword ptr [esp + 0x24]
        fxch st(1)
        fabs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_46a06c
        fld dword ptr [esp + 0x20]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_46a06c
        fld dword ptr [esp + 0x8]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_46a06c
        fld dword ptr [esp + 0x20]
        fcomp dword ptr [kF_004d2624]
        fnstsw AX
        test AH, 0x1
        jz L_46a06c
        fld dword ptr [esp + 0x8]
        fcomp dword ptr [kF_004d2624]
        fnstsw AX
        test AH, 0x1
        jz L_46a06c
        fld dword ptr [esp + 0x20]
        fmul dword ptr [esp + 0x4]
        fld dword ptr [esp + 0x24]
        fxch st(1)
        fadd dword ptr [ecx]
        fst dword ptr [esp + 0x10]
        fsub dword ptr [esi]
        fabs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jnz L_46a06c
        fld dword ptr [esp + 0x8]
        fmul dword ptr [esp + 0xc]
        fld dword ptr [esp + 0x24]
        fxch st(1)
        fadd dword ptr [ecx + 0x4]
        fxch st(1)
        fld st(1)
        fsub dword ptr [esi + 0x4]
        fabs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jnz L_46a06a
        mov edx, dword ptr [esp + 0x10]
        mov eax, 0x1
        fstp dword ptr [esi + 0x4]
        mov dword ptr [esi], edx
        pop esi
        add esp, 0x18
        ret 0x8
    L_46a06a:
        fstp st(0)
    L_46a06c:
        xor eax, eax
        pop esi
        add esp, 0x18
        ret 0x8
    }
}

// 0x0046a080 PointArray_RemoveAdjacentDuplicates (ECX xyz, EDX count) -> new count (Point2_NearlyEqual 0.01).
__declspec(naked) int __fastcall PointArray_RemoveAdjacentDuplicates(float*, unsigned)
{
    __asm {
        sub esp, 0xc
        push ebx
        push esi
        push edi
        mov esi, edx
        mov edi, ecx
        xor ebx, ebx
        test esi, esi
        mov dword ptr [esp + 0x14], edi
        jbe L_46a127
        lea eax, [edi + 0xc]
        push ebp
        mov dword ptr [esp + 0x10], 0x1
        mov ebp, edi
        mov dword ptr [esp + 0x14], eax
    L_46a0aa:
        mov eax, dword ptr [esp + 0x10]
        xor edx, edx
        div esi
        push 0x3c23d70a
        lea ecx, [edx + edx*0x2]
        lea edx, [edi + ecx*0x4]
        mov ecx, ebp
        call Point2_NearlyEqual
        test eax, eax
        jz L_46a10a
        lea eax, [esi - 0x1]
        cmp ebx, eax
        jz L_46a108
        sub esi, ebx
        mov edi, ebp
        dec esi
        lea ecx, [esi + esi*0x2]
        mov esi, dword ptr [esp + 0x14]
        shl ecx, 0x2
        mov edx, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        mov edx, dword ptr [esp + 0x14]
        and ecx, 0x3
        dec ebx
        rep movsb
        mov esi, dword ptr [esp + 0x10]
        mov edi, dword ptr [esp + 0x18]
        dec esi
        sub edx, 0xc
        mov dword ptr [esp + 0x10], esi
        mov dword ptr [esp + 0x14], edx
        sub ebp, 0xc
    L_46a108:
        mov esi, eax
    L_46a10a:
        mov eax, dword ptr [esp + 0x10]
        mov edx, dword ptr [esp + 0x14]
        inc ebx
        inc eax
        add edx, 0xc
        add ebp, 0xc
        cmp ebx, esi
        mov dword ptr [esp + 0x10], eax
        mov dword ptr [esp + 0x14], edx
        jc L_46a0aa
        pop ebp
    L_46a127:
        mov eax, esi
        pop edi
        pop esi
        pop ebx
        add esp, 0xc
        ret
    }
}

// 0x0046a620 Rect_OverlapMargin1 (ECX a, EDX b: minx, miny, maxx, maxy): overlap with a margin of 1.0 -> 1, else 0.
__declspec(naked) int __fastcall Rect_OverlapMargin1(const float*, const float*)
{
    __asm {
        fld dword ptr [edx + 0x8]
        fsub dword ptr [kF_004d2648]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jz L_46a635
        xor eax, eax
        ret
    L_46a635:
        fld dword ptr [edx]
        fsub dword ptr [kF_004d264c]
        fld dword ptr [ecx + 0x8]
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_46a64c
        xor eax, eax
        ret
    L_46a64c:
        fld dword ptr [edx + 0xc]
        fsub dword ptr [kF_004d2648]
        fld dword ptr [ecx + 0x4]
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_46a664
        xor eax, eax
        ret
    L_46a664:
        fld dword ptr [edx + 0x4]
        fsub dword ptr [kF_004d264c]
        fld dword ptr [ecx + 0xc]
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_46a67c
        xor eax, eax
        ret
    L_46a67c:
        mov eax, 0x1
        ret
    }
}

namespace {
const double kD_004d2610 = 0.0010000000474974513;  // CONFIRMED-BINARY: double 0x3f50624de0000000 at 0x004d2610
const double kD_004d2628 = 0.0;  // CONFIRMED-BINARY: double 0x0000000000000000 at 0x004d2628
const double kD_004d2630 = 1.0;  // CONFIRMED-BINARY: double 0x3ff0000000000000 at 0x004d2630
const float kF_004d2638 = -9.999999747378752e-06;  // CONFIRMED-BINARY: float 0xb727c5ac at 0x004d2638
const float kF_004d263c = -0.9999899864196777;  // CONFIRMED-BINARY: float 0xbf7fff58 at 0x004d263c
const float kF_004d2640 = 65536.0;  // CONFIRMED-BINARY: float 0x47800000 at 0x004d2640
const float kF_004d2644 = -65536.0;  // CONFIRMED-BINARY: float 0xc7800000 at 0x004d2644
const char kStr_004dff14[] = "weiler_init call to weilerInit failed.";  // CONFIRMED-DATA: .data string at 0x004dff14
const char kStr_004dff3c[] = "D:\\Proj\\GameZRecoil\\zGeometry\\zgeo_weiler.cpp";  // CONFIRMED-DATA: .data string at 0x004dff3c
const char kStr_004dff6c[] = "Bad clip region passed to Weiler Clip.";  // CONFIRMED-DATA: .data string at 0x004dff6c
const char kStr_004dff94[] = "%s %d: weiler_clip call to gatherContours failed.\n";  // CONFIRMED-DATA: .data string at 0x004dff94
const char kStr_004dffc8[] = "%s %d: Bad parameter(s) passed to Weiler Clip.\n";  // CONFIRMED-DATA: .data string at 0x004dffc8
const char kStr_004dfff8[] = "%s %d: weilerInit call to _new_contour failed.\n";  // CONFIRMED-DATA: .data string at 0x004dfff8
const char kStr_004e0028[] = "%s %d: weilerInit call to bufEntry failed.\n";  // CONFIRMED-DATA: .data string at 0x004e0028
const char kStr_004e0054[] = "Forward Segment Failed";  // CONFIRMED-DATA: .data string at 0x004e0054
const char kStr_004e006c[] = "%s %d: _weed_out_coincident call to segForward failed.\n";  // CONFIRMED-DATA: .data string at 0x004e006c
const char kStr_004e00a4[] = "WeedOut Error: %s";  // CONFIRMED-DATA: .data string at 0x004e00a4
const char kStr_004e00b8[] = "B_COMPLETELY_INSIDE_A";  // CONFIRMED-DATA: .data string at 0x004e00b8
const char kStr_004e00d0[] = "%s %d: _weiler_intersect call to _divide_edge failed.\n";  // CONFIRMED-DATA: .data string at 0x004e00d0
const char kStr_004e0108[] = "%s %d: weiler_intersect call to bufEntry failed.\n";  // CONFIRMED-DATA: .data string at 0x004e0108
const char kStr_004e013c[] = "weilerIntersect Error: %s";  // CONFIRMED-DATA: .data string at 0x004e013c
const char kStr_004e0158[] = "New_contour could not obtain buffer entry";  // CONFIRMED-DATA: .data string at 0x004e0158
const char kStr_004e0184[] = "%s %d: _merge_contours failed to receive new contour.\n";  // CONFIRMED-DATA: .data string at 0x004e0184
const char kStr_004e01bc[] = "contourMerge:  Failed validation\n";  // CONFIRMED-DATA: .data string at 0x004e01bc
const char kStr_004e01e0[] = "Found to output contours";  // CONFIRMED-DATA: .data string at 0x004e01e0
const char kStr_004e01fc[] = "Failed to output contours";  // CONFIRMED-DATA: .data string at 0x004e01fc
const char kStr_004e0218[] = "%s %d: outputContour call to bufEntry failed.\n";  // CONFIRMED-DATA: .data string at 0x004e0218
const char kStr_004e0248[] = "%s %d: _divide_edge call to bufEntry failed.\n";  // CONFIRMED-DATA: .data string at 0x004e0248
const char kStr_004e0278[] = "bufEntry failed";  // CONFIRMED-DATA: .data string at 0x004e0278
const char kStr_004e0288[] = "%s %d: _gen._outside_rslts call to buf_entry failed\n";  // CONFIRMED-DATA: .data string at 0x004e0288
const char kStr_004e02c0[] = "%s %d: _intersect2d call to buf_entry failed\n";  // CONFIRMED-DATA: .data string at 0x004e02c0
const char kStr_004e02f0[] = "validateXing failed (xing %d) xing_p is NULL!";  // CONFIRMED-DATA: .data string at 0x004e02f0
const char kStr_004e0320[] = "validateXing failed (xing %d) (type = %d)";  // CONFIRMED-DATA: .data string at 0x004e0320
// CONFIRMED-DATA: 81 dwords at 0x004dfdd0 (.data, read only at 0x0046938b, never written; the string at
// 0x004dff14 follows immediately). Index = four base-3 side codes; value = segment relation code.
const int kSegmentClassTable_004dfdd0[81] = {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 14, 2, 2, 8, 23, 2, 0, 2, 2, 13, 2, 2, 5, 22, 0, 0, 18, 6, 2, 2, 15, 2, 2, 0, 0, 2, 2, 2, 3, 2, 2, 2, 0, 0, 2, 2, 12, 2, 2, 7, 21, 0, 0, 19, 4, 2, 2, 16, 2, 2, 0, 2, 20, 9, 2, 2, 17, 2, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0};
}  // namespace

// 0x00464680 WeilerClip_Create - (points ECX, count EDX, axis arg): null points or count -> reports 'Bad clip region passed to Weiler' (zg 0x20D) but continues; count = PointArray_RemoveAdjacentDuplicates; calloc(1, 0x28CC) (unchecked); DynArray_Init with element sizes 0x3C, 0xC, 0x30, 0xC (target arrays not visible); copies points into [+0x18]; count [+0x14]; output +0x28/+0x2C = 0; axis arg nonzero -> ClipContour_SwapAxes; [+4]
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_Create(int, int, int)
{
    RECOIL_ENTRY(0)
    __asm {
        push ecx
        push ebx
        push ebp
        push esi
        push edi
        mov edi, edx
        mov esi, ecx
        test edi, edi
        mov dword ptr [esp + 0x10], esi
        jz L_464695
        test esi, esi
        jnz L_4646b1
    L_464695:
        push offset kStr_004dff6c
        push 0x20d
        push offset kStr_004dff3c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
    L_4646b1:
        mov edx, edi
        mov ecx, esi
        call PointArray_RemoveAdjacentDuplicates
        mov ebp, eax
        push 0x28cc
        push 0x1
        call dword ptr [g_Iat_calloc_004cc4ac]
        add esp, 0x8
        mov ebx, eax
        mov edx, 0x80
        push 0x3c
        lea ecx, [ebx + 0x34]
        call DynArray_Init
        mov edx, 0x80
        lea ecx, [ebx + 0x48]
        push 0xc
        call DynArray_Init
        mov edx, 0x80
        lea ecx, [ebx + 0x5c]
        push 0x30
        call DynArray_Init
        mov edx, ebp
        lea ecx, [ebx + 0xc]
        push 0xc
        call DynArray_Init
        mov edi, dword ptr [ebx + 0x18]
        lea ecx, [ebp + ebp*0x2]
        shl ecx, 0x2
        mov eax, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        xor eax, eax
        and ecx, 0x3
        rep movsb
        mov esi, dword ptr [esp + 0x18]
        mov dword ptr [ebx + 0x14], ebp
        cmp esi, eax
        mov dword ptr [ebx + 0x28], eax
        mov dword ptr [ebx + 0x2c], eax
        jz L_464739
        mov ecx, ebx
        call ClipContour_SwapAxes
    L_464739:
        mov ecx, ebx
        mov dword ptr [ebx + 0x4], esi
        call WeilerClip_TranslateToOrigin
        mov edx, dword ptr [esp + 0x10]
        mov ecx, ebx
        push 0x1
        push ebp
        call WeilerClip_Init
        test eax, eax
        jnz L_464782
        push offset kStr_004dff14
        push 0x24c
        push offset kStr_004dff3c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
        mov ecx, ebx
        call WeilerClip_Destroy
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_464782:
        pop edi
        pop esi
        mov eax, ebx
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    }
}

// 0x00464790 WeilerClip_ResetRegion - bytes read: (clip holder ECX, points EDX, count arg; ret 4): points or count zero -> 0; new = WeilerClip_Create(points, count, old [+4] flag); WeilerClip_Destroy(old); store; return 1
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_ResetRegion(int, int, int)
{
    RECOIL_ENTRY(1)
    __asm {
        mov eax, edx
        push esi
        test eax, eax
        push edi
        mov esi, ecx
        jz L_4647c4
        mov edx, dword ptr [esp + 0xc]
        test edx, edx
        jz L_4647c4
        mov ecx, dword ptr [esi]
        mov ecx, dword ptr [ecx + 0x4]
        push ecx
        mov ecx, eax
        call WeilerClip_Create
        mov ecx, dword ptr [esi]
        mov edi, eax
        call WeilerClip_Destroy
        mov dword ptr [esi], edi
        mov eax, 0x1
        pop edi
        pop esi
        ret 0x4
    L_4647c4:
        pop edi
        xor eax, eax
        pop esi
        ret 0x4
    }
}

// 0x00464810 WeilerClip_Run - (clip ECX, mode EDX, points arg1, count arg2, result block arg3): any null/zero -> fprintf 'Bad parameter passed' (zg 0x2A0), return 0. Stores points +0x2C, count +0x28, mode +0; swaps axes when axis [+4]; translates when [+0x28C8]; DynArray_Init x4 (element sizes 8, 8, 8, 0xC; targets not visible); result block {0, [+0x7C], 0, [+0x90], 0, [+0xA4], 0, [+0xB8]} at arg3, [+8]=arg3. WeilerClip_Classi
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_Run(int, int, int, int, int)
{
    RECOIL_ENTRY(2)
    __asm {
        push ebx
        push ebp
        push esi
        mov esi, ecx
        xor ebp, ebp
        push edi
        cmp esi, ebp
        jz L_464a6c
        cmp dword ptr [esi + 0x14], ebp
        jz L_464a6c
        mov ebx, dword ptr [esp + 0x18]
        cmp ebx, ebp
        jz L_464a6c
        mov eax, dword ptr [esp + 0x14]
        cmp eax, ebp
        jz L_464a6c
        mov edi, dword ptr [esp + 0x1c]
        cmp edi, ebp
        jz L_464a6c
        mov dword ptr [esi + 0x2c], eax
        mov eax, dword ptr [esi + 0x4]
        cmp eax, ebp
        mov dword ptr [esi], edx
        mov dword ptr [esi + 0x28], ebx
        jz L_464861
        call ClipContour_SwapAxes
    L_464861:
        mov AL, byte ptr [esi + 0x28c8]
        test AL, AL
        jz L_464872
        mov ecx, esi
        call WeilerClip_TranslateToOrigin
    L_464872:
        push 0x8
        mov edx, 0x80
        lea ecx, [esi + 0x70]
        call DynArray_Init
        mov edx, 0x80
        lea ecx, [esi + 0x84]
        push 0x8
        call DynArray_Init
        mov edx, 0x80
        lea ecx, [esi + 0x98]
        push 0x8
        call DynArray_Init
        mov edx, 0x80
        lea ecx, [esi + 0xac]
        push 0xc
        call DynArray_Init
        mov dword ptr [esi + 0x8], edi
        mov dword ptr [edi], ebp
        mov eax, dword ptr [esi + 0x7c]
        mov dword ptr [edi + 0x8], ebp
        mov dword ptr [edi + 0x4], eax
        mov ecx, dword ptr [esi + 0x90]
        mov dword ptr [edi + 0xc], ecx
        mov dword ptr [edi + 0x10], ebp
        mov edx, dword ptr [esi + 0xa4]
        mov dword ptr [edi + 0x18], ebp
        mov dword ptr [edi + 0x14], edx
        mov eax, dword ptr [esi + 0xb8]
        mov ecx, esi
        mov dword ptr [edi + 0x1c], eax
        call WeilerClip_ClassifyBounds
        mov ebp, eax
        cmp ebp, 0x1
        jnz L_4648fb
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0xc
    L_4648fb:
        cmp ebp, 0x4
        jnz L_46493f
        mov edx, 0x3
        mov ecx, esi
        call ClipContour_AppendToOutput
        test AL, AL
        jz L_4649fd
        mov AL, byte ptr [esi + 0x28c8]
        test AL, AL
        jz L_464925
        mov ecx, esi
        call WeilerClip_TranslateBack
    L_464925:
        mov eax, dword ptr [esi + 0x4]
        test eax, eax
        jz L_464933
        mov ecx, esi
        call ClipContour_SwapAxes
    L_464933:
        mov eax, 0x3
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0xc
    L_46493f:
        mov ecx, esi
        call WeilerClip_ResetWorkArrays
        mov edx, dword ptr [esi + 0x2c]
        mov ecx, esi
        push 0x2
        push ebx
        call WeilerClip_Init
        test eax, eax
        jz L_464ac3
        mov ecx, esi
        call WeilerClip_ComputeSideTables
        mov ecx, esi
        call WeilerClip_WeedOutCoincident
        test AL, AL
        jz L_464ac3
        mov ecx, esi
        call WeilerClip_FindIntersections
        cmp eax, 0x1
        jz L_464ac3
        test eax, eax
        jz L_4649c4
        mov ecx, esi
        call WeilerClip_MergeContours
        test eax, eax
        jz L_464ac3
        mov ecx, esi
        call WeilerClip_ClassifyContours
        cmp byte ptr [esi + 0x28c9], 0x1
        jnz L_464a97
        cmp ebp, 0x2
        jz L_464a97
        mov ecx, dword ptr [esi + 0x8]
        mov dword ptr [ecx + 0x8], 0x0
        mov edx, dword ptr [esi + 0x8]
        mov dword ptr [edx], 0x0
    L_4649c4:
        cmp ebp, 0x2
        jnz L_4649eb
        mov ecx, esi
        call WeilerClip_GenerateOutsideResult
        test AL, AL
        jz L_4649fd
        mov edx, 0x4
        mov ecx, esi
        call ClipContour_AppendToOutput
        test AL, AL
        jz L_4649fd
        mov edi, 0x4
        jmp L_464a38
    L_4649eb:
        cmp ebp, 0x3
        jnz L_464a33
        mov edx, ebp
        mov ecx, esi
        call ClipContour_AppendToOutput
        test AL, AL
        jnz L_464a2c
    L_4649fd:
        mov ecx, edi
        call WeilerClip_FreeBuffers
        mov AL, byte ptr [esi + 0x28c8]
        test AL, AL
        jz L_464a15
        mov ecx, esi
        call WeilerClip_TranslateBack
    L_464a15:
        mov eax, dword ptr [esi + 0x4]
        test eax, eax
        jz L_464a8e
        mov ecx, esi
        call ClipContour_SwapAxes
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0xc
    L_464a2c:
        mov edi, 0x3
        jmp L_464a38
    L_464a33:
        mov edi, 0x1
    L_464a38:
        mov AL, byte ptr [esi + 0x28c8]
        test AL, AL
        jz L_464a49
        mov ecx, esi
        call WeilerClip_TranslateBack
    L_464a49:
        cmp edi, 0x1
        jz L_464a55
        mov ecx, esi
        call WeilerClip_RestoreZFromPlane
    L_464a55:
        mov eax, dword ptr [esi + 0x4]
        test eax, eax
        jz L_464a63
        mov ecx, esi
        call ClipContour_SwapAxes
    L_464a63:
        mov eax, edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0xc
    L_464a6c:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x2a0
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004dffc8
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
    L_464a8e:
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0xc
    L_464a97:
        mov ecx, esi
        call WeilerClip_OutputContours
        test eax, eax
        jnz L_464af2
        mov eax, [g_Iat__iob_004cc4f8]
        push 0x3b2
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004dff94
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
    L_464ac3:
        mov AL, byte ptr [esi + 0x28c8]
        test AL, AL
        jz L_464ad4
        mov ecx, esi
        call WeilerClip_TranslateBack
    L_464ad4:
        mov eax, dword ptr [esi + 0x4]
        test eax, eax
        jz L_464ae2
        mov ecx, esi
        call ClipContour_SwapAxes
    L_464ae2:
        mov ecx, edi
        call WeilerClip_FreeBuffers
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0xc
    L_464af2:
        mov AL, byte ptr [esi + 0x28c8]
        test AL, AL
        jz L_464b03
        mov ecx, esi
        call WeilerClip_TranslateBack
    L_464b03:
        mov ecx, esi
        call WeilerClip_RestoreZFromPlane
        mov eax, dword ptr [esi + 0x4]
        test eax, eax
        jz L_464b18
        mov ecx, esi
        call ClipContour_SwapAxes
    L_464b18:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x2
        pop ebx
        ret 0xc
    }
}

// 0x00464b90 WeilerClip_Init - (clip ECX, n arg1, owner arg2): node buffer = DynArray_Push (argument not visible); failure -> fprintf 'weilerInit call to bufEntry failed' (zg 0x455) return 0; ClipNodes_BuildRing(n, arg2), [+0x38] = 0; Contour_EnsureBuffer failure -> 'weilerInit call to newContour failed' (0x468) 0; bounds for the ring; second ring with owner 4 at node n, its +0x38 = 0, Contour_EnsureBuffer (0x485). Return 1
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_Init(int, int, int, int)
{
    RECOIL_ENTRY(3)
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov edi, dword ptr [esp + 0x14]
        mov ebx, ecx
        mov ebp, edx
        push 0x0
        lea edx, [edi + edi*0x1]
        lea ecx, [ebx + 0x34]
        call DynArray_Push
        mov esi, eax
        test esi, esi
        jnz L_464bd9
        mov eax, [g_Iat__iob_004cc4f8]
        push 0x455
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004e0028
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x8
    L_464bd9:
        mov ecx, dword ptr [esp + 0x18]
        mov edx, ebp
        push ecx
        push edi
        mov ecx, esi
        call ClipNodes_BuildRing
        mov edx, esi
        mov ecx, ebx
        mov dword ptr [esi + 0x38], 0x0
        call Contour_EnsureBuffer
        test eax, eax
        jnz L_464c27
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x468
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004dfff8
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x8
    L_464c27:
        mov edx, edi
        mov ecx, esi
        call ClipArray_Call468410Each
        lea eax, [edi + edi*0x2]
        mov edx, ebp
        push 0x4
        push edi
        lea eax, [eax + eax*0x4]
        lea esi, [esi + eax*0x4]
        mov ecx, esi
        call ClipNodes_BuildRing
        mov edx, esi
        mov ecx, ebx
        mov dword ptr [esi + 0x38], 0x0
        call Contour_EnsureBuffer
        test eax, eax
        jnz L_464c84
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x485
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004dfff8
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x8
    L_464c84:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        ret 0x8
    }
}

// 0x00464c90 WeilerClip_ClassifyBounds - 2D bounds of output points [+0x2C] (count [+0x28]) and region points [+0x18] (count [+0x14]): disjoint (touching counts as disjoint) -> 1; output box inside region box -> WeilerClip_ClassifyResult(output, 2); region inside output -> (region, 3); partial overlap -> 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_ClassifyBounds(int, int)
{
    RECOIL_ENTRY(4)
    __asm {
        sub esp, 0x24
        mov eax, ecx
        push ebx
        push ebp
        push esi
        mov edx, dword ptr [eax + 0x14]
        mov ecx, dword ptr [eax + 0x28]
        mov ebx, dword ptr [eax + 0x18]
        mov dword ptr [esp + 0xc], edx
        mov edx, dword ptr [eax + 0x2c]
        push edi
        mov esi, edx
        mov edi, ebx
        mov eax, dword ptr [edx]
        mov dword ptr [esp + 0x14], eax
        mov eax, dword ptr [edx + 0x4]
        fld dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x18], eax
        lea eax, [ecx - 0x1]
        fld dword ptr [esp + 0x18]
        mov ebp, eax
        dec eax
        fstp dword ptr [esp + 0x28]
        test ebp, ebp
        jz L_464d24
        lea ebp, [eax + 0x1]
    L_464cd3:
        fcom dword ptr [esi + 0xc]
        add esi, 0xc
        fnstsw AX
        test AH, 0x41
        jnz L_464ce4
        fstp st(0)
        fld dword ptr [esi]
    L_464ce4:
        fld dword ptr [esi]
        fcomp dword ptr [esp + 0x14]
        fnstsw AX
        test AH, 0x41
        jnz L_464cf7
        mov eax, dword ptr [esi]
        mov dword ptr [esp + 0x14], eax
    L_464cf7:
        fld dword ptr [esi + 0x4]
        fcomp dword ptr [esp + 0x28]
        fnstsw AX
        test AH, 0x1
        jz L_464d0c
        fld dword ptr [esi + 0x4]
        fstp dword ptr [esp + 0x28]
    L_464d0c:
        fld dword ptr [esi + 0x4]
        fcomp dword ptr [esp + 0x18]
        fnstsw AX
        test AH, 0x41
        jnz L_464d21
        fld dword ptr [esi + 0x4]
        fstp dword ptr [esp + 0x18]
    L_464d21:
        dec ebp
        jnz L_464cd3
    L_464d24:
        mov eax, dword ptr [ebx]
        mov ebp, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], eax
        mov eax, dword ptr [ebx + 0x4]
        fld dword ptr [esp + 0x1c]
        mov dword ptr [esp + 0x20], eax
        lea eax, [ebp - 0x1]
        fld dword ptr [esp + 0x20]
        mov esi, eax
        dec eax
        fstp dword ptr [esp + 0x30]
        test esi, esi
        jz L_464d9f
        lea esi, [eax + 0x1]
    L_464d4e:
        fcom dword ptr [edi + 0xc]
        add edi, 0xc
        fnstsw AX
        test AH, 0x41
        jnz L_464d5f
        fstp st(0)
        fld dword ptr [edi]
    L_464d5f:
        fld dword ptr [edi]
        fcomp dword ptr [esp + 0x1c]
        fnstsw AX
        test AH, 0x41
        jnz L_464d72
        mov eax, dword ptr [edi]
        mov dword ptr [esp + 0x1c], eax
    L_464d72:
        fld dword ptr [edi + 0x4]
        fcomp dword ptr [esp + 0x30]
        fnstsw AX
        test AH, 0x1
        jz L_464d87
        fld dword ptr [edi + 0x4]
        fstp dword ptr [esp + 0x30]
    L_464d87:
        fld dword ptr [edi + 0x4]
        fcomp dword ptr [esp + 0x20]
        fnstsw AX
        test AH, 0x41
        jnz L_464d9c
        fld dword ptr [edi + 0x4]
        fstp dword ptr [esp + 0x20]
    L_464d9c:
        dec esi
        jnz L_464d4e
    L_464d9f:
        fld st(1)
        fcomp dword ptr [esp + 0x1c]
        fnstsw AX
        test AH, 0x1
        jz L_464e8d
        fld dword ptr [esp + 0x14]
        fcomp
        fnstsw AX
        test AH, 0x41
        jnz L_464e8d
        fld dword ptr [esp + 0x28]
        fcomp dword ptr [esp + 0x20]
        fnstsw AX
        test AH, 0x1
        jz L_464e8d
        fld dword ptr [esp + 0x18]
        fcomp dword ptr [esp + 0x30]
        fnstsw AX
        test AH, 0x41
        jnz L_464e8d
        fld st(1)
        fcomp
        fnstsw AX
        test AH, 0x41
        jz L_464e38
        fld dword ptr [esp + 0x14]
        fcomp dword ptr [esp + 0x1c]
        fnstsw AX
        test AH, 0x1
        jnz L_464e38
        fld dword ptr [esp + 0x28]
        fcomp dword ptr [esp + 0x30]
        fnstsw AX
        test AH, 0x41
        jz L_464e38
        fld dword ptr [esp + 0x18]
        fcomp dword ptr [esp + 0x20]
        fnstsw AX
        test AH, 0x1
        jnz L_464e38
        push 0x2
        push edx
        fstp st(0)
        push ecx
        mov edx, ebx
        mov ecx, ebp
        fstp st(0)
        call WeilerClip_ClassifyResult
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x24
        ret
    L_464e38:
        fxch st(1)
        fcomp
        fnstsw AX
        test AH, 0x1
        fstp st(0)
        jnz L_464e83
        fld dword ptr [esp + 0x14]
        fcomp dword ptr [esp + 0x1c]
        fnstsw AX
        test AH, 0x41
        jz L_464e83
        fld dword ptr [esp + 0x28]
        fcomp dword ptr [esp + 0x30]
        fnstsw AX
        test AH, 0x1
        jnz L_464e83
        fld dword ptr [esp + 0x18]
        fcomp dword ptr [esp + 0x20]
        fnstsw AX
        test AH, 0x41
        jz L_464e83
        push 0x3
        push ebx
        push ebp
        call WeilerClip_ClassifyResult
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x24
        ret
    L_464e83:
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x24
        ret
    L_464e8d:
        pop edi
        pop esi
        fstp st(0)
        pop ebp
        mov eax, 0x1
        fstp st(0)
        pop ebx
        add esp, 0x24
        ret
    }
}

// 0x00464ea0 WeilerClip_ClassifyResult - (count ECX, points EDX, other count arg1, other points arg2, result arg3): result 2 with equal counts and every other point matching some point within 0.001 in x/y -> 4 (identical). Else for each of ECX points 0x00468a10(arg2) (operand not visible) negative -> 0. Returns arg3
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_ClassifyResult(int, int, int, int, int)
{
    RECOIL_ENTRY(5)
    __asm {
        push ecx
        mov eax, dword ptr [esp + 0x10]
        push ebx
        push ebp
        push esi
        cmp eax, 0x2
        push edi
        mov ebp, edx
        mov esi, ecx
        jnz L_464f27
        mov eax, dword ptr [esp + 0x18]
        cmp esi, eax
        jnz L_464f27
        mov edi, dword ptr [esp + 0x1c]
        mov BL, 0x1
        mov dword ptr [esp + 0x10], eax
    L_464ec4:
        test eax, eax
        jz L_464f16
        xor BL, BL
        mov edx, esi
        test esi, esi
        mov ecx, ebp
        jz L_464f06
    L_464ed2:
        fld dword ptr [ecx]
        fsub dword ptr [edi]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_464efa
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [edi + 0x4]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jnz L_464f04
    L_464efa:
        dec edx
        add ecx, 0xc
        test edx, edx
        jnz L_464ed2
        jmp L_464f06
    L_464f04:
        mov BL, 0x1
    L_464f06:
        mov eax, dword ptr [esp + 0x10]
        add edi, 0xc
        dec eax
        test BL, BL
        mov dword ptr [esp + 0x10], eax
        jnz L_464ec4
    L_464f16:
        test BL, BL
        jz L_464f27
        mov eax, 0x4
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0xc
    L_464f27:
        mov eax, esi
        dec esi
        test eax, eax
        jz L_464f62
    L_464f2e:
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x18]
        push ecx
        mov ecx, ebp
        call Point2_InPolygonQuadrant
        test AL, AL
        jl L_464f58
        add ebp, 0xc
        mov edx, esi
        dec esi
        test edx, edx
        jnz L_464f2e
        mov eax, dword ptr [esp + 0x20]
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0xc
    L_464f58:
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0xc
    L_464f62:
        mov eax, dword ptr [esp + 0x20]
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0xc
    }
}

// 0x00464f70 WeilerClip_WeedOutCoincident - clip ECX: for every region edge (nodes from [[+0x54]+4], 0x3C each, count [+0x14]) against every output edge (count [+0x28]): only when both side values in the two tables (+0xC0 row and +0x14C0 row) are below 1e-5 in magnitude (collinear). A 4-bit code from Point2_OnSegmentRange of each endpoint against the other segment selects the case: 3/7/0xB/0xC/0xD/0xE/0xF (containment or shared-direction ov
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_WeedOutCoincident(int, int)
{
    RECOIL_ENTRY(6)
    __asm {
        sub esp, 0x40
        mov eax, dword ptr [ecx + 0x54]
        mov edx, dword ptr [ecx + 0x28]
        mov dword ptr [esp + 0x1c], edx
        push ebx
        mov edx, dword ptr [eax + 0x10]
        push ebp
        mov dword ptr [esp + 0xc], edx
        mov edx, dword ptr [eax + 0x1c]
        push esi
        push edi
        mov edi, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x24], edx
        mov dword ptr [esp + 0x44], eax
        mov eax, dword ptr [eax + 0x28]
        lea edx, [ecx + 0x14c0]
        mov dword ptr [esp + 0x34], eax
        mov dword ptr [esp + 0x1c], edx
        mov edx, dword ptr [ecx + 0x14]
        xor eax, eax
        mov dword ptr [esp + 0x18], ecx
        test edx, edx
        mov dword ptr [esp + 0x30], eax
        jbe L_465a78
    L_464fbc:
        lea edx, [ecx + eax*0x4 + 0xc0]
        mov esi, dword ptr [esp + 0x24]
        mov dword ptr [esp + 0x20], edx
        mov edx, dword ptr [edi + 0xc]
        mov dword ptr [esp + 0x48], edx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x4c], edx
        mov edx, dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x10], edx
        mov edx, dword ptr [esp + 0x2c]
        test edx, edx
        mov dword ptr [esp + 0x28], 0x0
        jbe L_4658ea
    L_464ff5:
        mov eax, dword ptr [esp + 0x20]
        mov ebp, dword ptr [esi + 0xc]
        mov ebx, dword ptr [esi + 0x10]
        fld dword ptr [eax]
        fabs
        fcomp qword ptr [kD_004d2618]
        fnstsw AX
        test AH, 0x1
        jz L_4658a1
        mov ecx, dword ptr [esp + 0x20]
        fld dword ptr [ecx + 0x4]
        fabs
        fcomp qword ptr [kD_004d2618]
        fnstsw AX
        test AH, 0x1
        jz L_4658a1
        mov edx, dword ptr [esp + 0x1c]
        fld dword ptr [edx]
        fabs
        fcomp qword ptr [kD_004d2618]
        fnstsw AX
        test AH, 0x1
        jz L_4658a1
        mov eax, edx
        fld dword ptr [eax + 0x4]
        fabs
        fcomp qword ptr [kD_004d2618]
        fnstsw AX
        test AH, 0x1
        jz L_4658a1
        mov ecx, dword ptr [esp + 0x48]
        push ebx
        mov edx, ebp
        call Point2_OnSegmentRange
        mov edx, ebp
        mov ecx, dword ptr [esp + 0x4c]
        push ebx
        mov dword ptr [esp + 0x3c], eax
        call Point2_OnSegmentRange
        mov ecx, dword ptr [esp + 0x4c]
        mov edx, dword ptr [esp + 0x48]
        push ecx
        mov ecx, ebp
        mov dword ptr [esp + 0x40], eax
        call Point2_OnSegmentRange
        mov ecx, ebx
        mov edx, dword ptr [esp + 0x4c]
        mov dword ptr [esp + 0x40], eax
        push edx
        mov edx, dword ptr [esp + 0x4c]
        call Point2_OnSegmentRange
        mov ecx, dword ptr [esp + 0x38]
        lea edx, [ecx + ecx*0x1]
        mov ecx, dword ptr [esp + 0x3c]
        or edx, ecx
        mov ecx, dword ptr [esp + 0x40]
        shl edx, 0x1
        or edx, ecx
        shl edx, 0x1
        or edx, eax
        lea eax, [edx - 0x3]
        cmp eax, 0xc
        ja L_4658a1
        cmp eax, 0
        je L_4650cf
        cmp eax, 1
        je L_4658a1
        cmp eax, 2
        je L_4651c5
        cmp eax, 3
        je L_465248
        cmp eax, 4
        je L_4652c5
        cmp eax, 5
        je L_4658a1
        cmp eax, 6
        je L_465363
        cmp eax, 7
        je L_4653f1
        cmp eax, 8
        je L_465480
        cmp eax, 9
        je L_465552
        cmp eax, 10
        je L_46563a
        cmp eax, 11
        je L_465704
        cmp eax, 12
        je L_4657b8
        int 3  // unreachable: the bounds check above excludes other indices
    L_4650cf:
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_4650ee
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x1
        jz L_465105
    L_4650ee:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x41
        jz L_465174
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x41
        jz L_465174
    L_465105:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_46511f
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_465139
    L_46511f:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_465174
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_465174
    L_465139:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x18]
        push 0x0
        push 0x0
        push ebx
        push eax
        mov edx, edi
        call ClipEdge_AddPair
        test eax, eax
        jz L_46591d
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x4c], ebp
        mov dword ptr [eax + 0x10], ebp
        mov ecx, dword ptr [edi + 0x8]
        mov dword ptr [edi + 0x10], ebp
        mov edx, dword ptr [esi + 0x8]
        or edx, ecx
        mov dword ptr [esi + 0x8], edx
        mov edx, dword ptr [eax + 0x8]
        jmp L_4651ad
    L_465174:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x18]
        push 0x0
        push 0x0
        push ebp
        push eax
        mov edx, edi
        call ClipEdge_AddPair
        test eax, eax
        jz L_465948
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x4c], ebx
        mov dword ptr [eax + 0x10], ebx
        mov dword ptr [edi + 0x10], ebx
        mov ecx, dword ptr [eax + 0x8]
        mov edx, dword ptr [esi + 0x8]
        or edx, ecx
        mov dword ptr [esi + 0x8], edx
        mov edx, dword ptr [edi + 0x8]
    L_4651ad:
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [eax + 0x8]
        or ecx, edx
        mov dword ptr [eax + 0x8], ecx
        mov ecx, edi
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_4651c5:
        mov ecx, dword ptr [esp + 0x4c]
        fld dword ptr [ecx]
        fsub dword ptr [ebx]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_4651f5
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [ebx + 0x4]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jnz L_4658a1
    L_4651f5:
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esi + 0x8]
        mov ebp, dword ptr [esp + 0x14]
        push eax
        mov edx, dword ptr [ecx + 0x8]
        mov ecx, dword ptr [esp + 0x1c]
        push edx
        push ebx
        push ebp
        mov edx, edi
        call ClipEdge_AddPair
        test eax, eax
        jz L_465973
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esp + 0x4c]
        mov dword ptr [ebp + 0x10], ebx
        mov dword ptr [edi + 0x10], ebx
        mov dword ptr [ecx + 0x10], eax
        mov dword ptr [esi + 0x10], eax
        mov edx, dword ptr [edi + 0x10]
        mov ecx, edi
        mov dword ptr [esp + 0x4c], edx
        call ClipEdge_ComputeBounds
        mov ecx, esi
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_465248:
        mov ecx, dword ptr [esp + 0x4c]
        fld dword ptr [ecx]
        fsub dword ptr [ebp]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_465279
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [ebp + 0x4]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jnz L_4658a1
    L_465279:
        mov eax, dword ptr [esp + 0x4c]
        mov ebx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        push 0x0
        push 0x0
        push eax
        push ebx
        mov edx, esi
        call ClipEdge_AddPair
        test eax, eax
        jz L_46599f
        mov eax, dword ptr [esp + 0x4c]
        mov dword ptr [esp + 0x4c], ebp
        mov dword ptr [ebx + 0x10], eax
        mov dword ptr [esi + 0x10], eax
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x10], ebp
        mov ecx, dword ptr [edi + 0x8]
        mov dword ptr [edi + 0x10], ebp
        mov edx, dword ptr [esi + 0x8]
        or edx, ecx
        mov dword ptr [esi + 0x8], edx
        mov edx, dword ptr [eax + 0x8]
        jmp L_465465
    L_4652c5:
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_4652e4
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x1
        jz L_4652fb
    L_4652e4:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x41
        jz L_465342
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x41
        jz L_465342
    L_4652fb:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_465315
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_46532f
    L_465315:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_465342
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_465342
    L_46532f:
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x4c], ebx
        mov dword ptr [eax + 0x10], ebx
        mov dword ptr [edi + 0x10], ebx
        jmp L_46552c
    L_465342:
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x4c], ebp
        mov dword ptr [eax + 0x10], ebp
        mov edx, dword ptr [edi + 0x8]
        mov dword ptr [edi + 0x10], ebp
        mov ebx, dword ptr [esi + 0x8]
        or ebx, edx
        mov dword ptr [esi + 0x8], ebx
        mov ecx, dword ptr [eax + 0x8]
        jmp L_46553a
    L_465363:
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [ecx]
        fsub dword ptr [ebx]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_465393
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [ebx + 0x4]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jnz L_4658a1
    L_465393:
        mov ebp, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x18]
        push 0x0
        push 0x0
        push ebx
        push ebp
        mov edx, edi
        call ClipEdge_AddPair
        test eax, eax
        jz L_4659cb
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x48]
        mov dword ptr [ebp + 0x10], ebx
        mov dword ptr [edi + 0x10], ebx
        mov dword ptr [eax + 0x10], ecx
        mov edx, dword ptr [esi + 0x8]
        mov dword ptr [esp + 0x4c], ebx
        mov dword ptr [esi + 0x10], ecx
        mov ebx, dword ptr [edi + 0x8]
        or ebx, edx
        mov dword ptr [edi + 0x8], ebx
        mov eax, dword ptr [eax + 0x8]
        mov ecx, dword ptr [ebp + 0x8]
        or ecx, eax
        mov dword ptr [ebp + 0x8], ecx
        mov ecx, edi
        call ClipEdge_ComputeBounds
        mov ecx, esi
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_4653f1:
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [ecx]
        fsub dword ptr [ebp]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_465422
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [ebp + 0x4]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jnz L_4658a1
    L_465422:
        mov ebx, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x18]
        push 0x0
        push 0x0
        push ebp
        push ebx
        mov edx, edi
        call ClipEdge_AddPair
        test eax, eax
        jz L_4659f6
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x48]
        mov dword ptr [ebx + 0x10], ebp
        mov dword ptr [edi + 0x10], ebp
        mov dword ptr [eax + 0xc], ecx
        mov dword ptr [esi + 0xc], ecx
        mov ecx, dword ptr [eax + 0x8]
        mov edx, dword ptr [edi + 0x8]
        or edx, ecx
        mov dword ptr [esp + 0x48], ebp
        mov dword ptr [edi + 0x8], edx
        mov edx, dword ptr [esi + 0x8]
    L_465465:
        mov eax, dword ptr [ebx + 0x8]
        mov ecx, edi
        or eax, edx
        mov dword ptr [ebx + 0x8], eax
        call ClipEdge_ComputeBounds
        mov ecx, esi
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_465480:
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_46549f
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x1
        jz L_4654b6
    L_46549f:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x41
        jz L_46551e
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x41
        jz L_46551e
    L_4654b6:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_4654d0
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_4654ea
    L_4654d0:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_46551e
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_46551e
    L_4654ea:
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x48], ebx
        mov dword ptr [eax + 0xc], ebx
        mov ecx, dword ptr [edi + 0x8]
        mov dword ptr [edi + 0xc], ebx
        mov edx, dword ptr [esi + 0x8]
        or edx, ecx
        mov dword ptr [esi + 0x8], edx
        mov edx, dword ptr [eax + 0x8]
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [eax + 0x8]
        or ecx, edx
        mov dword ptr [eax + 0x8], ecx
        mov ecx, edi
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_46551e:
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x48], ebp
        mov dword ptr [eax + 0xc], ebp
        mov dword ptr [edi + 0xc], ebp
    L_46552c:
        mov eax, dword ptr [eax + 0x8]
        mov ebx, dword ptr [esi + 0x8]
        or ebx, eax
        mov dword ptr [esi + 0x8], ebx
        mov ecx, dword ptr [edi + 0x8]
    L_46553a:
        mov eax, dword ptr [esp + 0x10]
        mov edx, dword ptr [eax + 0x8]
        or edx, ecx
        mov ecx, edi
        mov dword ptr [eax + 0x8], edx
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_465552:
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_465571
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x1
        jz L_465588
    L_465571:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x41
        jz L_4655e9
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x41
        jz L_4655e9
    L_465588:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_4655a2
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_4655bc
    L_4655a2:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_4655e9
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_4655e9
    L_4655bc:
        mov edx, dword ptr [esp + 0x4c]
        mov ebx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        push 0x0
        push 0x0
        push edx
        push ebx
        mov edx, esi
        call ClipEdge_AddPair
        test eax, eax
        jz L_465a22
        mov eax, dword ptr [esp + 0x48]
        mov dword ptr [ebx + 0x10], eax
        mov dword ptr [esi + 0x10], eax
        jmp L_465614
    L_4655e9:
        mov eax, dword ptr [esp + 0x48]
        mov ebx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        push 0x0
        push 0x0
        push eax
        push ebx
        mov edx, esi
        call ClipEdge_AddPair
        test eax, eax
        jz L_465a4d
        mov eax, dword ptr [esp + 0x4c]
        mov dword ptr [ebx + 0x10], eax
        mov dword ptr [esi + 0x10], eax
    L_465614:
        mov ecx, dword ptr [esi + 0x8]
        mov edx, dword ptr [edi + 0x8]
        mov eax, dword ptr [esp + 0x14]
        or edx, ecx
        mov dword ptr [edi + 0x8], edx
        mov edx, dword ptr [ebx + 0x8]
        mov ecx, dword ptr [eax + 0x8]
        or ecx, edx
        mov dword ptr [eax + 0x8], ecx
        mov ecx, esi
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_46563a:
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_465659
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x1
        jz L_465670
    L_465659:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x41
        jz L_4656d4
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x41
        jz L_4656d4
    L_465670:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_46568a
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_4656a4
    L_46568a:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_4656d4
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_4656d4
    L_4656a4:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x10], ecx
        mov dword ptr [esi + 0x10], ecx
        mov ecx, dword ptr [esi + 0x8]
        mov edx, dword ptr [edi + 0x8]
        or edx, ecx
        mov dword ptr [edi + 0x8], edx
        mov edx, dword ptr [eax + 0x8]
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [eax + 0x8]
        or ecx, edx
        mov dword ptr [eax + 0x8], ecx
        mov ecx, esi
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_4656d4:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x10], edx
        mov dword ptr [esi + 0x10], edx
        mov eax, dword ptr [eax + 0x8]
        mov ebx, dword ptr [edi + 0x8]
        or ebx, eax
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [edi + 0x8], ebx
        mov ecx, dword ptr [esi + 0x8]
        mov edx, dword ptr [eax + 0x8]
        or edx, ecx
        mov ecx, esi
        mov dword ptr [eax + 0x8], edx
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_465704:
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_465723
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x1
        jz L_46573a
    L_465723:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x41
        jz L_465788
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x41
        jz L_465788
    L_46573a:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_465754
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_46576e
    L_465754:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_465788
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_465788
    L_46576e:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0xc], edx
        mov dword ptr [esi + 0xc], edx
        mov edx, dword ptr [esi + 0x8]
        mov ebx, dword ptr [edi + 0x8]
        or ebx, edx
        mov dword ptr [edi + 0x8], ebx
        mov ecx, dword ptr [eax + 0x8]
        jmp L_4657a0
    L_465788:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0xc], ecx
        mov dword ptr [esi + 0xc], ecx
        mov edx, dword ptr [eax + 0x8]
        mov ebx, dword ptr [edi + 0x8]
        or ebx, edx
        mov dword ptr [edi + 0x8], ebx
        mov ecx, dword ptr [esi + 0x8]
    L_4657a0:
        mov eax, dword ptr [esp + 0x14]
        mov edx, dword ptr [eax + 0x8]
        or edx, ecx
        mov ecx, esi
        mov dword ptr [eax + 0x8], edx
        call ClipEdge_ComputeBounds
        jmp L_4658a1
    L_4657b8:
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x48]
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_4657d7
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x1
        jz L_4657ee
    L_4657d7:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x41
        jz L_46583d
        fld dword ptr [ebx]
        fcomp dword ptr [ebp]
        fnstsw AX
        test AH, 0x41
        jz L_46583d
    L_4657ee:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jnz L_465808
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_465822
    L_465808:
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_46583d
        fld dword ptr [ebx + 0x4]
        fcomp dword ptr [ebp + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_46583d
    L_465822:
        mov edx, dword ptr [esi + 0x8]
        mov ebp, dword ptr [edi + 0x8]
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x14]
        or ebp, edx
        mov dword ptr [edi + 0x8], ebp
        mov edx, dword ptr [eax + 0x8]
        or dword ptr [ecx + 0x8], edx
        jmp L_46585b
    L_46583d:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esi + 0x8]
        mov edx, dword ptr [eax + 0x8]
        or edx, ecx
        mov dword ptr [eax + 0x8], edx
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [edi + 0x8]
        mov edx, dword ptr [eax + 0x8]
        or ecx, edx
        mov dword ptr [edi + 0x8], ecx
    L_46585b:
        cmp esi, dword ptr [esp + 0x24]
        jnz L_465883
        mov ecx, dword ptr [esp + 0x44]
        mov edx, dword ptr [esi + 0x4]
        mov dword ptr [ecx + 0x1c], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x28], edx
        mov ebx, dword ptr [esi + 0x4]
        lea edx, [ecx + 0x18]
        add ecx, 0x24
        mov dword ptr [ebx + 0x38], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [edx + 0x38], ecx
    L_465883:
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [esi + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov ecx, dword ptr [esi + 0x4]
        mov edx, dword ptr [esi]
        mov dword ptr [ecx], edx
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
    L_4658a1:
        mov eax, dword ptr [esp + 0x18]
        mov edx, dword ptr [esp + 0x20]
        mov ebx, dword ptr [esp + 0x10]
        add esi, 0x3c
        mov ecx, dword ptr [eax + 0x14]
        add ebx, 0x3c
        mov dword ptr [esp + 0x10], ebx
        lea eax, [edx + ecx*0x4 + 0x4]
        mov ecx, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x20], eax
        mov eax, dword ptr [esp + 0x1c]
        add eax, 0x4
        mov dword ptr [esp + 0x1c], eax
        mov eax, dword ptr [esp + 0x28]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x28], eax
        jc L_464ff5
        mov ecx, dword ptr [esp + 0x18]
        mov eax, dword ptr [esp + 0x30]
    L_4658ea:
        mov edx, dword ptr [esp + 0x1c]
        mov ebx, dword ptr [esp + 0x14]
        add edx, 0x4
        add edi, 0x3c
        mov dword ptr [esp + 0x1c], edx
        mov edx, dword ptr [ecx + 0x14]
        add ebx, 0x3c
        inc eax
        cmp eax, edx
        mov dword ptr [esp + 0x14], ebx
        mov dword ptr [esp + 0x30], eax
        jc L_464fbc
        mov AL, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_46591d:
        push offset kStr_004e00b8
        push offset kStr_004e00a4
        push 0x568
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x14
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_465948:
        push offset kStr_004e00b8
        push offset kStr_004e00a4
        push 0x572
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x14
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_465973:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x593
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e006c
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_46599f:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x5b6
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e006c
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_4659cb:
        mov eax, [g_Iat__iob_004cc4f8]
        push 0x5f0
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004e006c
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_4659f6:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x610
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e006c
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_465a22:
        push offset kStr_004e0054
        push offset kStr_004e00a4
        push 0x64a
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x14
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_465a4d:
        push offset kStr_004e0054
        push offset kStr_004e00a4
        push 0x650
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x14
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x40
        ret
    L_465a78:
        pop edi
        pop esi
        pop ebp
        mov AL, 0x1
        pop ebx
        add esp, 0x40
        ret
    }
}

// 0x00465ac0 WeilerClip_FindIntersections - clip ECX, headers [+0x54]: nested walk over region ring edges (from [hdr+4], companion ring [hdr+0x10]) x output ring edges (from [hdr+0x1C], companion [hdr+0x28]); refreshes edge bounds when dirty (+0x14); skips pairs whose bounds don't overlap or whose crossing links (+0x30/+0x34) are already shared with the current edges. ClipSegments_Intersect2D code: 1 -> report 'weilerIntersect Error: %s' (z
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_FindIntersections(int, int)
{
    RECOIL_ENTRY(7)
    __asm {
        sub esp, 0x44
        mov eax, dword ptr [ecx + 0x54]
        push ebx
        push ebp
        push esi
        mov ebp, dword ptr [eax + 0x28]
        mov ebx, dword ptr [eax + 0x10]
        mov esi, dword ptr [eax + 0x1c]
        push edi
        mov edi, dword ptr [eax + 0x4]
        xor edx, edx
        mov dword ptr [esp + 0x20], ecx
        mov dword ptr [esp + 0x24], edx
        mov dword ptr [esp + 0x1c], edx
        mov dword ptr [esp + 0x14], ebp
        mov dword ptr [esp + 0x40], edi
        mov dword ptr [esp + 0x28], edx
    L_465af0:
        mov eax, dword ptr [edi + 0xc]
        mov ecx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x44], eax
        mov dword ptr [esp + 0x48], ecx
        mov dword ptr [esp + 0x18], 0x0
        mov dword ptr [esp + 0x2c], esi
    L_465b0a:
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0xc]
        mov dword ptr [esp + 0x50], eax
        mov eax, dword ptr [edi + 0x14]
        test eax, eax
        mov dword ptr [esp + 0x4c], edx
        jz L_465b26
        mov ecx, edi
        call ClipEdge_ComputeBounds
    L_465b26:
        mov eax, dword ptr [esi + 0x14]
        test eax, eax
        jz L_465b34
        mov ecx, esi
        call ClipEdge_ComputeBounds
    L_465b34:
        fld dword ptr [edi + 0x18]
        fcomp dword ptr [esi + 0x24]
        fnstsw AX
        test AH, 0x41
        jz L_4670e6
        fld dword ptr [edi + 0x24]
        fcomp dword ptr [esi + 0x18]
        fnstsw AX
        test AH, 0x1
        jnz L_4670e6
        fld dword ptr [edi + 0x1c]
        fcomp dword ptr [esi + 0x28]
        fnstsw AX
        test AH, 0x41
        jz L_4670e6
        fld dword ptr [edi + 0x28]
        fcomp dword ptr [esi + 0x1c]
        fnstsw AX
        test AH, 0x1
        jnz L_4670e6
        mov eax, dword ptr [esi + 0x30]
        test eax, eax
        jz L_465ba3
        cmp eax, dword ptr [edi + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [edi + 0x34]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x34]
        jz L_4670e6
    L_465ba3:
        mov eax, dword ptr [esi + 0x34]
        test eax, eax
        jz L_465bce
        cmp eax, dword ptr [edi + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [edi + 0x34]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x34]
        jz L_4670e6
    L_465bce:
        mov eax, dword ptr [ebp + 0x30]
        test eax, eax
        jz L_465bf9
        cmp eax, dword ptr [edi + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [edi + 0x34]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x34]
        jz L_4670e6
    L_465bf9:
        mov eax, dword ptr [ebp + 0x34]
        test eax, eax
        jz L_465c24
        cmp eax, dword ptr [edi + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [edi + 0x34]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x30]
        jz L_4670e6
        cmp eax, dword ptr [ebx + 0x34]
        jz L_4670e6
    L_465c24:
        mov ecx, dword ptr [esp + 0x50]
        sub esp, 0xc
        mov eax, esp
        sub esp, 0xc
        mov ebp, dword ptr [ecx]
        lea edx, [esp + 0x28]
        mov dword ptr [eax], ebp
        mov ebp, dword ptr [ecx + 0x4]
        mov dword ptr [eax + 0x4], ebp
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [eax + 0x8], ecx
        mov eax, dword ptr [esp + 0x64]
        mov ecx, esp
        sub esp, 0xc
        mov ebp, dword ptr [eax]
        mov dword ptr [ecx], ebp
        mov ebp, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], ebp
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], eax
        mov ecx, dword ptr [esp + 0x6c]
        mov eax, esp
        sub esp, 0xc
        mov ebp, dword ptr [ecx]
        mov dword ptr [eax], ebp
        mov ebp, dword ptr [ecx + 0x4]
        mov dword ptr [eax + 0x4], ebp
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [eax + 0x8], ecx
        mov eax, dword ptr [esp + 0x74]
        mov ecx, esp
        mov ebp, dword ptr [eax]
        mov dword ptr [ecx], ebp
        mov ebp, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], ebp
        mov ebp, dword ptr [esp + 0x50]
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], eax
        mov ecx, ebp
        call ClipSegments_Intersect2D
        cmp eax, 0x1
        mov dword ptr [esp + 0x3c], eax
        jz L_467125
        xor ecx, ecx
        cmp eax, ecx
        jz L_4670d4
        mov edx, dword ptr [esp + 0x10]
        cmp edx, ecx
        jz L_465cee
        mov dword ptr [edx + 0x24], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x28], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x1c], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x20], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x14], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x18], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0xc], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x10], ecx
        mov edx, dword ptr [esp + 0x10]
    L_465cee:
        add eax, -0x3
        cmp eax, 0x14
        ja L_4670af
        cmp eax, 0
        je L_465d01
        cmp eax, 1
        je L_466054
        cmp eax, 2
        je L_466054
        cmp eax, 3
        je L_466525
        cmp eax, 4
        je L_466663
        cmp eax, 5
        je L_466781
        cmp eax, 6
        je L_4668aa
        cmp eax, 7
        je L_4670af
        cmp eax, 8
        je L_4670af
        cmp eax, 9
        je L_4669cd
        cmp eax, 10
        je L_4660c2
        cmp eax, 11
        je L_466b40
        cmp eax, 12
        je L_466a4f
        cmp eax, 13
        je L_4661e8
        cmp eax, 14
        je L_466c28
        cmp eax, 15
        je L_466d06
        cmp eax, 16
        je L_46630e
        cmp eax, 17
        je L_466ef4
        cmp eax, 18
        je L_466e0e
        cmp eax, 19
        je L_4663ec
        cmp eax, 20
        je L_466f9b
        int 3  // unreachable: the bounds check above excludes other indices
    L_465d01:
        mov eax, dword ptr [esp + 0x50]
        mov edx, dword ptr [esp + 0x4c]
        mov ecx, dword ptr [esp + 0x44]
        push eax
        call Point2_OnSegmentRange
        mov ecx, dword ptr [esp + 0x50]
        mov edx, dword ptr [esp + 0x4c]
        push ecx
        mov ecx, dword ptr [esp + 0x4c]
        mov dword ptr [esp + 0x34], eax
        call Point2_OnSegmentRange
        mov edx, dword ptr [esp + 0x48]
        mov ecx, dword ptr [esp + 0x4c]
        push edx
        mov edx, dword ptr [esp + 0x48]
        mov dword ptr [esp + 0x38], eax
        call Point2_OnSegmentRange
        mov edx, dword ptr [esp + 0x44]
        mov ecx, dword ptr [esp + 0x50]
        mov dword ptr [esp + 0x38], eax
        mov eax, dword ptr [esp + 0x48]
        push eax
        call Point2_OnSegmentRange
        mov ecx, dword ptr [esp + 0x30]
        lea edx, [ecx + ecx*0x1]
        mov ecx, dword ptr [esp + 0x34]
        or edx, ecx
        mov ecx, dword ptr [esp + 0x38]
        shl edx, 0x1
        or edx, ecx
        shl edx, 0x1
        or edx, eax
        lea eax, [edx - 0x5]
        cmp eax, 0x5
        ja L_4670af
        cmp eax, 0
        je L_465d81
        cmp eax, 1
        je L_465e60
        cmp eax, 2
        je L_4670af
        cmp eax, 3
        je L_4670af
        cmp eax, 4
        je L_465eeb
        cmp eax, 5
        je L_465f76
        int 3  // unreachable: the bounds check above excludes other indices
    L_465d81:
        push 0x0
        mov edx, 0x1
        lea ecx, [ebp + 0x5c]
        call DynArray_Push
        test eax, eax
        mov dword ptr [esp + 0x10], eax
        jz L_467131
        xor ecx, ecx
        mov dword ptr [eax + 0x24], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x28], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x1c], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x20], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x14], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x18], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0xc], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x10], ecx
        mov eax, dword ptr [esi + 0x4]
        cmp esi, eax
        jz L_465e26
        mov ecx, dword ptr [esi + 0x10]
        mov edx, dword ptr [eax + 0xc]
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_465e26
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [edx + 0x4]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_465e26
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0x14
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], eax
        jmp L_465e43
    L_465e26:
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0x17
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
    L_465e43:
        mov ecx, dword ptr [esp + 0x48]
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], ecx
        jmp L_4670b3
    L_465e60:
        push 0x0
        mov edx, 0x1
        lea ecx, [ebp + 0x5c]
        call DynArray_Push
        test eax, eax
        mov dword ptr [esp + 0x10], eax
        jz L_46715a
        xor ecx, ecx
        mov dword ptr [eax + 0x24], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x28], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x1c], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x20], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x14], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x18], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0xc], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x10], ecx
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x48]
        mov dword ptr [eax + 0x2c], 0x11
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], ecx
    L_465ed4:
        mov ebp, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
        jmp L_4670b3
    L_465eeb:
        push 0x0
        mov edx, 0x1
        lea ecx, [ebp + 0x5c]
        call DynArray_Push
        test eax, eax
        mov dword ptr [esp + 0x10], eax
        jz L_46716e
        xor ecx, ecx
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x24], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x28], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x1c], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x20], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x14], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x18], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0xc], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x10], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x2c], 0x15
        mov eax, dword ptr [esp + 0x50]
        mov ecx, dword ptr [esp + 0x10]
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], edx
        jmp L_4670b3
    L_465f76:
        push 0x0
        mov edx, 0x1
        lea ecx, [ebp + 0x5c]
        call DynArray_Push
        test eax, eax
        mov dword ptr [esp + 0x10], eax
        jz L_467182
        xor ecx, ecx
        mov dword ptr [eax + 0x24], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x28], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x1c], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x20], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x14], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x18], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0xc], ecx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x10], ecx
        mov eax, dword ptr [esi]
        cmp esi, eax
        jz L_46601a
        mov ecx, dword ptr [esi + 0xc]
        mov edx, dword ptr [eax + 0x10]
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_46601a
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [edx + 0x4]
        fabs
        fcomp qword ptr [kD_004d2610]
        fnstsw AX
        test AH, 0x41
        jz L_46601a
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0xc
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], eax
        jmp L_466037
    L_46601a:
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0xf
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], eax
    L_466037:
        mov ecx, dword ptr [esp + 0x44]
        mov edx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ecx]
        mov dword ptr [edx], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], ecx
        jmp L_4670b3
    L_466054:
        push 0x1
        push edi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467196
        mov edx, dword ptr [esp + 0x10]
        push 0x1
        push ebx
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467196
        mov edx, dword ptr [esp + 0x10]
        push 0x1
        push esi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467196
        mov ebp, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x20]
        push 0x1
        push ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467196
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4660c2:
        push 0x1
        push edi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4671a0
        mov edx, dword ptr [esp + 0x10]
        push 0x0
        push ebx
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4671a0
        mov ecx, dword ptr [esi]
        push ebx
        mov edx, esi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_466515
        cmp eax, 0
        je L_4661ca
        cmp eax, 1
        je L_466141
        cmp eax, 2
        je L_466104
        cmp eax, 3
        je L_466515
        cmp eax, 4
        je L_466515
        cmp eax, 5
        je L_466515
        cmp eax, 6
        je L_466515
        cmp eax, 7
        je L_46617d
        int 3  // unreachable: the bounds check above excludes other indices
    L_466104:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x18
        mov ecx, dword ptr [edi + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], eax
        mov ecx, dword ptr [ebp]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_466141:
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0x18
        mov edx, dword ptr [edi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], ecx
        mov edx, dword ptr [esi]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_46617d:
        mov edx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [edx + 0x2c], 0x5
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], edx
        mov eax, dword ptr [ebp]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], edx
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], edx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4661ca:
        mov ebp, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4661e8:
        push 0x0
        push edi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4671cf
        mov edx, dword ptr [esp + 0x10]
        push 0x1
        push ebx
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4671cf
        mov ecx, dword ptr [esi]
        push ebx
        mov edx, esi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_466515
        cmp eax, 0
        je L_4662f0
        cmp eax, 1
        je L_466267
        cmp eax, 2
        je L_46622a
        cmp eax, 3
        je L_466515
        cmp eax, 4
        je L_466515
        cmp eax, 5
        je L_466515
        cmp eax, 6
        je L_466515
        cmp eax, 7
        je L_4662a3
        int 3  // unreachable: the bounds check above excludes other indices
    L_46622a:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x19
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
        mov ecx, dword ptr [ebp]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_466267:
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0x19
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], ecx
        mov edx, dword ptr [esi]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4662a3:
        mov edx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [edx + 0x2c], 0x4
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
        mov eax, dword ptr [ebp]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], edx
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], edx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4662f0:
        mov ebp, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_46630e:
        push 0x1
        push edi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4671fe
        mov edx, dword ptr [esp + 0x10]
        push 0x0
        push ebx
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4671fe
        mov edx, dword ptr [esi + 0x4]
        push edi
        mov ecx, esi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_466515
        cmp eax, 0
        je L_4664f7
        cmp eax, 1
        je L_46638e
        cmp eax, 2
        je L_466351
        cmp eax, 3
        je L_466515
        cmp eax, 4
        je L_466515
        cmp eax, 5
        je L_466515
        cmp eax, 6
        je L_466515
        cmp eax, 7
        je L_4663cb
        int 3  // unreachable: the bounds check above excludes other indices
    L_466351:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x18
        mov ecx, dword ptr [edi + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], eax
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_46638e:
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0x18
        mov edx, dword ptr [edi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], ecx
        mov edx, dword ptr [esi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4663cb:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x4
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], edx
        jmp L_4664c5
    L_4663ec:
        push 0x0
        push edi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467205
        mov edx, dword ptr [esp + 0x10]
        push 0x1
        push ebx
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467205
        mov edx, dword ptr [esi + 0x4]
        push ebx
        mov ecx, esi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_466515
        cmp eax, 0
        je L_4664f7
        cmp eax, 1
        je L_46646c
        cmp eax, 2
        je L_46642f
        cmp eax, 3
        je L_466515
        cmp eax, 4
        je L_466515
        cmp eax, 5
        je L_466515
        cmp eax, 6
        je L_466515
        cmp eax, 7
        je L_4664a9
        int 3  // unreachable: the bounds check above excludes other indices
    L_46642f:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x19
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_46646c:
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0x19
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], ecx
        mov edx, dword ptr [esi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4664a9:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x5
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
    L_4664c5:
        mov ebp, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ebp + 0x4]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], edx
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_4664f7:
        mov ebp, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_466515:
        mov edx, dword ptr [edi + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x48], edx
        jmp L_4670b3
    L_466525:
        push 0x1
        push esi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467234
        mov ebp, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x20]
        push 0x0
        push ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467234
        mov ecx, dword ptr [edi]
        push esi
        mov edx, edi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_4669c0
        cmp eax, 0
        je L_466648
        cmp eax, 1
        je L_4665b6
        cmp eax, 2
        je L_46656d
        cmp eax, 3
        je L_4669c0
        cmp eax, 4
        je L_4669c0
        cmp eax, 5
        je L_4669c0
        cmp eax, 6
        je L_4669c0
        cmp eax, 7
        je L_4665ff
        int 3  // unreachable: the bounds check above excludes other indices
    L_46656d:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [eax + 0x2c], 0xa
        mov ecx, dword ptr [esi + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], eax
        mov ecx, dword ptr [ebx]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], eax
        mov eax, dword ptr [edi + 0x8]
        or AL, 0x3
        mov dword ptr [edi + 0x8], eax
        mov eax, dword ptr [edi]
        add eax, 0x8
        or dword ptr [eax], 0x3
        jmp L_4670b3
    L_4665b6:
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [ecx + 0x2c], 0xa
        mov edx, dword ptr [esi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], ecx
        mov edx, dword ptr [edi]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], ecx
        mov eax, dword ptr [edi + 0x8]
        or AL, 0x3
        mov dword ptr [edi + 0x8], eax
        mov eax, dword ptr [edi]
        add eax, 0x8
        or dword ptr [eax], 0x3
        jmp L_4670b3
    L_4665ff:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [edx + 0x2c], 0x4
        mov eax, dword ptr [ebp + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], edx
        mov eax, dword ptr [ebx]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], edx
        mov eax, dword ptr [edi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], edx
        jmp L_4670b3
    L_466648:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [ebx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], ecx
        jmp L_4670b3
    L_466663:
        push 0x0
        push esi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467263
        mov ebp, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x20]
        push 0x1
        push ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467263
        mov ecx, dword ptr [edi]
        push ebp
        mov edx, edi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_4669c0
        cmp eax, 0
        je L_466766
        cmp eax, 1
        je L_4666e4
        cmp eax, 2
        je L_4666ab
        cmp eax, 3
        je L_4669c0
        cmp eax, 4
        je L_4669c0
        cmp eax, 5
        je L_4669c0
        cmp eax, 6
        je L_4669c0
        cmp eax, 7
        je L_46671d
        int 3  // unreachable: the bounds check above excludes other indices
    L_4666ab:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [edx + 0x2c], 0xb
        mov eax, dword ptr [ebp + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], edx
        mov eax, dword ptr [ebx]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], edx
        jmp L_4670b3
    L_4666e4:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [eax + 0x2c], 0xb
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov ecx, dword ptr [edi]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], eax
        jmp L_4670b3
    L_46671d:
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [ecx + 0x2c], 0x5
        mov edx, dword ptr [esi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], ecx
        mov edx, dword ptr [ebx]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], ecx
        mov edx, dword ptr [edi]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], ecx
        jmp L_4670b3
    L_466766:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [ebx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], eax
        jmp L_4670b3
    L_466781:
        push 0x1
        push esi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467291
        mov ebp, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x20]
        push 0x0
        push ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_467291
        mov edx, dword ptr [edi + 0x4]
        push esi
        mov ecx, edi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_4669c0
        cmp eax, 0
        je L_46688f
        cmp eax, 1
        je L_4667f9
        cmp eax, 2
        je L_4667ca
        cmp eax, 3
        je L_4669c0
        cmp eax, 4
        je L_4669c0
        cmp eax, 5
        je L_4669c0
        cmp eax, 6
        je L_4669c0
        cmp eax, 7
        je L_466844
        int 3  // unreachable: the bounds check above excludes other indices
    L_4667ca:
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x2c], 0xa
        mov edx, dword ptr [esi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], ecx
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], ecx
        jmp L_466826
    L_4667f9:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0xa
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
    L_466826:
        mov eax, dword ptr [edi + 0x8]
        mov dword ptr [esp + 0x1c], 0x1
        or AL, 0x3
        mov dword ptr [edi + 0x8], eax
        mov eax, dword ptr [edi + 0x4]
        add eax, 0x8
        or dword ptr [eax], 0x3
        jmp L_4670b3
    L_466844:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [eax + 0x2c], 0x5
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
        mov ecx, dword ptr [edi + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], eax
        jmp L_4670b3
    L_46688f:
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [ebx + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
        jmp L_4670b3
    L_4668aa:
        push 0x0
        push esi
        mov ecx, ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4672c0
        mov ebp, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x20]
        push 0x1
        push ebp
        call ClipEdge_Divide
        test eax, eax
        jz L_4672c0
        mov edx, dword ptr [edi + 0x4]
        push ebp
        mov ecx, edi
        call ClipEdge_ClassifyJoin
        cmp eax, 0x7
        ja L_4669c0
        cmp eax, 0
        je L_4669b2
        cmp eax, 1
        je L_46692d
        cmp eax, 2
        je L_4668f3
        cmp eax, 3
        je L_4669c0
        cmp eax, 4
        je L_4669c0
        cmp eax, 5
        je L_4669c0
        cmp eax, 6
        je L_4669c0
        cmp eax, 7
        je L_466967
        int 3  // unreachable: the bounds check above excludes other indices
    L_4668f3:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [eax + 0x2c], 0xb
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
        jmp L_4670b3
    L_46692d:
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [ecx + 0x2c], 0xb
        mov edx, dword ptr [ebp + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], ecx
        mov edx, dword ptr [edi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], ecx
        jmp L_4670b3
    L_466967:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x1c], 0x1
        mov dword ptr [edx + 0x2c], 0x4
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], edx
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
        jmp L_4670b3
    L_4669b2:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], ecx
    L_4669c0:
        mov dword ptr [esp + 0x1c], 0x1
        jmp L_4670b3
    L_4669cd:
        mov edx, dword ptr [edi]
        mov ecx, dword ptr [esi]
        push ebp
        push edi
        push edx
        mov edx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor ecx, ecx
        cmp eax, 0
        jne Lsw_4669ec_0
        mov CL, 0
        jmp L_466a38
    Lsw_4669ec_0:
        cmp eax, 1
        jne Lsw_4669ec_1
        mov CL, 4
        jmp L_4670af
    Lsw_4669ec_1:
        cmp eax, 2
        jne Lsw_4669ec_2
        mov CL, 4
        jmp L_4670af
    Lsw_4669ec_2:
        cmp eax, 3
        jne Lsw_4669ec_3
        mov CL, 1
        jmp L_466a13
    Lsw_4669ec_3:
        cmp eax, 4
        jne Lsw_4669ec_4
        mov CL, 2
        jmp L_4669f3
    Lsw_4669ec_4:
        cmp eax, 5
        jne Lsw_4669ec_5
        mov CL, 4
        jmp L_4670af
    Lsw_4669ec_5:
        cmp eax, 6
        jne Lsw_4669ec_6
        mov CL, 4
        jmp L_4670af
    Lsw_4669ec_6:
        cmp eax, 7
        jne Lsw_4669ec_7
        mov CL, 4
        jmp L_4670af
    Lsw_4669ec_7:
        cmp eax, 8
        jne Lsw_4669ec_8
        mov CL, 3
        jmp L_466aeb
    Lsw_4669ec_8:
        cmp eax, 9
        jne Lsw_4669ec_9
        mov CL, 3
        jmp L_466aeb
    Lsw_4669ec_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_4669f3:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x18
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], edx
        jmp L_466e51
    L_466a13:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x18
        mov ecx, dword ptr [ebp]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        jmp L_466e86
    L_466a38:
        mov ebp, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], ecx
        jmp L_4670b3
    L_466a4f:
        mov edx, dword ptr [ebx]
        mov ecx, dword ptr [esi]
        push ebp
        push ebx
        push edx
        mov edx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor ecx, ecx
        cmp eax, 0
        jne Lsw_466a6e_0
        mov CL, 0
        jmp L_466b2d
    Lsw_466a6e_0:
        cmp eax, 1
        jne Lsw_466a6e_1
        mov CL, 4
        jmp L_4670af
    Lsw_466a6e_1:
        cmp eax, 2
        jne Lsw_466a6e_2
        mov CL, 4
        jmp L_4670af
    Lsw_466a6e_2:
        cmp eax, 3
        jne Lsw_466a6e_3
        mov CL, 4
        jmp L_4670af
    Lsw_466a6e_3:
        cmp eax, 4
        jne Lsw_466a6e_4
        mov CL, 4
        jmp L_4670af
    Lsw_466a6e_4:
        cmp eax, 5
        jne Lsw_466a6e_5
        mov CL, 1
        jmp L_466ab6
    Lsw_466a6e_5:
        cmp eax, 6
        jne Lsw_466a6e_6
        mov CL, 2
        jmp L_466a75
    Lsw_466a6e_6:
        cmp eax, 7
        jne Lsw_466a6e_7
        mov CL, 4
        jmp L_4670af
    Lsw_466a6e_7:
        cmp eax, 8
        jne Lsw_466a6e_8
        mov CL, 3
        jmp L_466aeb
    Lsw_466a6e_8:
        cmp eax, 9
        jne Lsw_466a6e_9
        mov CL, 3
        jmp L_466aeb
    Lsw_466a6e_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_466a75:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0xa
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], edx
        mov eax, dword ptr [ebx]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], edx
        mov ebp, dword ptr [edi + 0x8]
        or ebp, 0x2
        mov dword ptr [edi + 0x8], ebp
        mov eax, dword ptr [edi]
        add eax, 0x8
        or dword ptr [eax], 0x2
        jmp L_4670af
    L_466ab6:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x19
        mov ecx, dword ptr [ebp]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        mov ecx, dword ptr [ebx]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], eax
        jmp L_4670b3
    L_466aeb:
        cmp eax, 0x8
        jnz L_466afd
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x2c], 0x5
        jmp L_466b08
    L_466afd:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x4
    L_466b08:
        mov eax, dword ptr [ebx]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], edx
        mov eax, dword ptr [edi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], edx
        jmp L_466beb
    L_466b2d:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], ecx
        jmp L_4670af
    L_466b40:
        mov edx, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esi]
        push ebp
        push edx
        push ebx
        mov edx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor ecx, ecx
        cmp eax, 0
        jne Lsw_466b60_0
        mov CL, 0
        jmp L_466c15
    Lsw_466b60_0:
        cmp eax, 1
        jne Lsw_466b60_1
        mov CL, 4
        jmp L_4670af
    Lsw_466b60_1:
        cmp eax, 2
        jne Lsw_466b60_2
        mov CL, 4
        jmp L_4670af
    Lsw_466b60_2:
        cmp eax, 3
        jne Lsw_466b60_3
        mov CL, 1
        jmp L_466b87
    Lsw_466b60_3:
        cmp eax, 4
        jne Lsw_466b60_4
        mov CL, 2
        jmp L_466b67
    Lsw_466b60_4:
        cmp eax, 5
        jne Lsw_466b60_5
        mov CL, 4
        jmp L_4670af
    Lsw_466b60_5:
        cmp eax, 6
        jne Lsw_466b60_6
        mov CL, 4
        jmp L_4670af
    Lsw_466b60_6:
        cmp eax, 7
        jne Lsw_466b60_7
        mov CL, 4
        jmp L_4670af
    Lsw_466b60_7:
        cmp eax, 8
        jne Lsw_466b60_8
        mov CL, 3
        jmp L_466bac
    Lsw_466b60_8:
        cmp eax, 9
        jne Lsw_466b60_9
        mov CL, 3
        jmp L_466bac
    Lsw_466b60_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_466b67:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x18
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], edx
        jmp L_466f38
    L_466b87:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x18
        mov ecx, dword ptr [ebp]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], eax
        jmp L_466f6e
    L_466bac:
        cmp eax, 0x8
        jnz L_466bbe
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x2c], 0x5
        jmp L_466bc9
    L_466bbe:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x4
    L_466bc9:
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], edx
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
    L_466beb:
        mov ebp, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ebp]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], edx
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], edx
        jmp L_4670b3
    L_466c15:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], ecx
        jmp L_4670af
    L_466c28:
        mov edx, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esi]
        push ebp
        push edx
        push ebx
        mov edx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor ecx, ecx
        cmp eax, 0
        jne Lsw_466c48_0
        mov CL, 0
        jmp L_465ed4
    Lsw_466c48_0:
        cmp eax, 1
        jne Lsw_466c48_1
        mov CL, 3
        jmp L_4670af
    Lsw_466c48_1:
        cmp eax, 2
        jne Lsw_466c48_2
        mov CL, 3
        jmp L_4670af
    Lsw_466c48_2:
        cmp eax, 3
        jne Lsw_466c48_3
        mov CL, 3
        jmp L_4670af
    Lsw_466c48_3:
        cmp eax, 4
        jne Lsw_466c48_4
        mov CL, 3
        jmp L_4670af
    Lsw_466c48_4:
        cmp eax, 5
        jne Lsw_466c48_5
        mov CL, 1
        jmp L_466c4f
    Lsw_466c48_5:
        cmp eax, 6
        jne Lsw_466c48_6
        mov CL, 1
        jmp L_466c4f
    Lsw_466c48_6:
        cmp eax, 7
        jne Lsw_466c48_7
        mov CL, 3
        jmp L_4670af
    Lsw_466c48_7:
        cmp eax, 8
        jne Lsw_466c48_8
        mov CL, 2
        jmp L_466c9d
    Lsw_466c48_8:
        cmp eax, 9
        jne Lsw_466c48_9
        mov CL, 2
        jmp L_466c9d
    Lsw_466c48_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_466c4f:
        mov edx, dword ptr [esp + 0x10]
        cmp eax, 0x3
        mov dword ptr [edx + 0x2c], 0x19
        jnz L_466c72
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
        jmp L_466c83
    L_466c72:
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], edx
    L_466c83:
        mov ebp, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ebp]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], edx
        jmp L_4670b3
    L_466c9d:
        cmp eax, 0x8
        jnz L_466caf
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x2c], 0x5
        jmp L_466cba
    L_466caf:
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x2c], 0x4
    L_466cba:
        mov edx, dword ptr [ebx + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], ecx
        mov edx, dword ptr [edi + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], ecx
        mov edx, dword ptr [ebp]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x30], ecx
        mov edx, dword ptr [esi]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x30], ecx
        jmp L_4670b3
    L_466d06:
        mov ecx, dword ptr [ebx]
        mov edx, dword ptr [esi + 0x4]
        push ebp
        push ebx
        push ecx
        mov ecx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor edx, edx
        cmp eax, 0
        jne Lsw_466d26_0
        mov DL, 0
        jmp L_466dfb
    Lsw_466d26_0:
        cmp eax, 1
        jne Lsw_466d26_1
        mov DL, 4
        jmp L_4670af
    Lsw_466d26_1:
        cmp eax, 2
        jne Lsw_466d26_2
        mov DL, 4
        jmp L_4670af
    Lsw_466d26_2:
        cmp eax, 3
        jne Lsw_466d26_3
        mov DL, 1
        jmp L_466d5e
    Lsw_466d26_3:
        cmp eax, 4
        jne Lsw_466d26_4
        mov DL, 2
        jmp L_466d2d
    Lsw_466d26_4:
        cmp eax, 5
        jne Lsw_466d26_5
        mov DL, 4
        jmp L_4670af
    Lsw_466d26_5:
        cmp eax, 6
        jne Lsw_466d26_6
        mov DL, 4
        jmp L_4670af
    Lsw_466d26_6:
        cmp eax, 7
        jne Lsw_466d26_7
        mov DL, 4
        jmp L_4670af
    Lsw_466d26_7:
        cmp eax, 8
        jne Lsw_466d26_8
        mov DL, 3
        jmp L_466d93
    Lsw_466d26_8:
        cmp eax, 9
        jne Lsw_466d26_9
        mov DL, 3
        jmp L_466d93
    Lsw_466d26_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_466d2d:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x2c], 0x18
        mov ecx, dword ptr [esi + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], eax
        mov ecx, dword ptr [edi]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], eax
        jmp L_4670af
    L_466d5e:
        mov ecx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x2c], 0x18
        mov edx, dword ptr [ebp + 0x4]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x30], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], ecx
        mov edx, dword ptr [edi]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], ecx
        jmp L_4670b3
    L_466d93:
        cmp eax, 0x8
        jnz L_466da5
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x5
        jmp L_466db0
    L_466da5:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x2c], 0x4
    L_466db0:
        mov ecx, dword ptr [ebx]
        mov edx, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], eax
        mov ecx, dword ptr [edi]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], eax
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov ecx, dword ptr [esi + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], eax
        jmp L_4670b3
    L_466dfb:
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], edx
        jmp L_4670af
    L_466e0e:
        mov eax, dword ptr [ebx]
        mov edx, dword ptr [esi + 0x4]
        push ebp
        push ebx
        push eax
        mov ecx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor ecx, ecx
        cmp eax, 0
        jne Lsw_466e2e_0
        mov CL, 0
        jmp L_466edd
    Lsw_466e2e_0:
        cmp eax, 1
        jne Lsw_466e2e_1
        mov CL, 4
        jmp L_4670af
    Lsw_466e2e_1:
        cmp eax, 2
        jne Lsw_466e2e_2
        mov CL, 4
        jmp L_4670af
    Lsw_466e2e_2:
        cmp eax, 3
        jne Lsw_466e2e_3
        mov CL, 1
        jmp L_466e66
    Lsw_466e2e_3:
        cmp eax, 4
        jne Lsw_466e2e_4
        mov CL, 2
        jmp L_466e35
    Lsw_466e2e_4:
        cmp eax, 5
        jne Lsw_466e2e_5
        mov CL, 4
        jmp L_4670af
    Lsw_466e2e_5:
        cmp eax, 6
        jne Lsw_466e2e_6
        mov CL, 4
        jmp L_4670af
    Lsw_466e2e_6:
        cmp eax, 7
        jne Lsw_466e2e_7
        mov CL, 4
        jmp L_4670af
    Lsw_466e2e_7:
        cmp eax, 8
        jne Lsw_466e2e_8
        mov CL, 3
        jmp L_466e9b
    Lsw_466e2e_8:
        cmp eax, 9
        jne Lsw_466e2e_9
        mov CL, 3
        jmp L_466e9b
    Lsw_466e2e_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_466e35:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x18
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
    L_466e51:
        mov eax, dword ptr [edi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], edx
        jmp L_4670af
    L_466e66:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x18
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
    L_466e86:
        mov ecx, dword ptr [edi]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x34], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], eax
        jmp L_4670b3
    L_466e9b:
        cmp eax, 0x8
        jnz L_466ead
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x2c], 0x5
        jmp L_466eb8
    L_466ead:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x4
    L_466eb8:
        mov eax, dword ptr [ebx]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], edx
        mov eax, dword ptr [edi]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x34], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x30], edx
        jmp L_467079
    L_466edd:
        mov ebp, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x30], ecx
        jmp L_4670b3
    L_466ef4:
        mov edx, dword ptr [ebx + 0x4]
        push ebp
        push edx
        mov edx, dword ptr [esi + 0x4]
        push ebx
        mov ecx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor ecx, ecx
        cmp eax, 0
        jne Lsw_466f15_0
        mov CL, 0
        jmp L_466f84
    Lsw_466f15_0:
        cmp eax, 1
        jne Lsw_466f15_1
        mov CL, 4
        jmp L_4670af
    Lsw_466f15_1:
        cmp eax, 2
        jne Lsw_466f15_2
        mov CL, 4
        jmp L_4670af
    Lsw_466f15_2:
        cmp eax, 3
        jne Lsw_466f15_3
        mov CL, 1
        jmp L_466f4e
    Lsw_466f15_3:
        cmp eax, 4
        jne Lsw_466f15_4
        mov CL, 2
        jmp L_466f1c
    Lsw_466f15_4:
        cmp eax, 5
        jne Lsw_466f15_5
        mov CL, 4
        jmp L_4670af
    Lsw_466f15_5:
        cmp eax, 6
        jne Lsw_466f15_6
        mov CL, 4
        jmp L_4670af
    Lsw_466f15_6:
        cmp eax, 7
        jne Lsw_466f15_7
        mov CL, 4
        jmp L_4670af
    Lsw_466f15_7:
        cmp eax, 8
        jne Lsw_466f15_8
        mov CL, 3
        jmp L_46703a
    Lsw_466f15_8:
        cmp eax, 9
        jne Lsw_466f15_9
        mov CL, 3
        jmp L_46703a
    Lsw_466f15_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_466f1c:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x18
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
    L_466f38:
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
        jmp L_4670af
    L_466f4e:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x18
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
    L_466f6e:
        mov ecx, dword ptr [edi + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], eax
        jmp L_4670b3
    L_466f84:
        mov ebp, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], ecx
        jmp L_4670b3
    L_466f9b:
        mov edx, dword ptr [ebx + 0x4]
        push ebp
        push edx
        mov edx, dword ptr [esi + 0x4]
        push ebx
        mov ecx, esi
        call ClipEdge_ClassifyTouch
        cmp eax, 0x9
        ja L_4670af
        xor ecx, ecx
        cmp eax, 0
        jne Lsw_466fbc_0
        mov CL, 0
        jmp L_4670a1
    Lsw_466fbc_0:
        cmp eax, 1
        jne Lsw_466fbc_1
        mov CL, 4
        jmp L_4670af
    Lsw_466fbc_1:
        cmp eax, 2
        jne Lsw_466fbc_2
        mov CL, 4
        jmp L_4670af
    Lsw_466fbc_2:
        cmp eax, 3
        jne Lsw_466fbc_3
        mov CL, 4
        jmp L_4670af
    Lsw_466fbc_3:
        cmp eax, 4
        jne Lsw_466fbc_4
        mov CL, 4
        jmp L_4670af
    Lsw_466fbc_4:
        cmp eax, 5
        jne Lsw_466fbc_5
        mov CL, 1
        jmp L_467007
    Lsw_466fbc_5:
        cmp eax, 6
        jne Lsw_466fbc_6
        mov CL, 2
        jmp L_466fc3
    Lsw_466fbc_6:
        cmp eax, 7
        jne Lsw_466fbc_7
        mov CL, 4
        jmp L_4670af
    Lsw_466fbc_7:
        cmp eax, 8
        jne Lsw_466fbc_8
        mov CL, 3
        jmp L_46703a
    Lsw_466fbc_8:
        cmp eax, 9
        jne Lsw_466fbc_9
        mov CL, 3
        jmp L_46703a
    Lsw_466fbc_9:
        int 3  // unreachable: the bounds check above excludes other indices
    L_466fc3:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0xa
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], edx
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
        mov ebp, dword ptr [edi + 0x8]
        or ebp, 0x2
        mov dword ptr [edi + 0x8], ebp
        mov eax, dword ptr [edi + 0x4]
        add eax, 0x8
        or dword ptr [eax], 0x2
        jmp L_4670af
    L_467007:
        mov eax, dword ptr [esp + 0x10]
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x2c], 0x19
        mov ecx, dword ptr [ebx + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], eax
        mov ecx, dword ptr [ebp + 0x4]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x30], edx
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], eax
        jmp L_4670b3
    L_46703a:
        cmp eax, 0x8
        jnz L_46704c
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x2c], 0x5
        jmp L_467057
    L_46704c:
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x2c], 0x4
    L_467057:
        mov eax, dword ptr [ebx + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], edx
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x34], edx
    L_467079:
        mov ebp, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [ebp + 0x4]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ebp + 0x34], edx
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x30], ecx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], edx
        jmp L_4670b3
    L_4670a1:
        mov eax, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x34], eax
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ebx + 0x34], ecx
    L_4670af:
        mov ebp, dword ptr [esp + 0x14]
    L_4670b3:
        mov eax, dword ptr [esp + 0x1c]
        test eax, eax
        jz L_4670d8
        mov eax, dword ptr [esp + 0x18]
        mov esi, dword ptr [esi + 0x4]
        mov ebp, dword ptr [ebp + 0x4]
        inc eax
        mov dword ptr [esp + 0x1c], 0x0
        mov dword ptr [esp + 0x18], eax
        jmp L_4670d8
    L_4670d4:
        mov ebp, dword ptr [esp + 0x14]
    L_4670d8:
        mov edx, dword ptr [esp + 0x3c]
        mov ecx, dword ptr [esp + 0x24]
        or ecx, edx
        mov dword ptr [esp + 0x24], ecx
    L_4670e6:
        mov eax, dword ptr [esp + 0x18]
        mov esi, dword ptr [esi + 0x4]
        mov ebp, dword ptr [ebp + 0x4]
        inc eax
        mov dword ptr [esp + 0x18], eax
        mov eax, dword ptr [esp + 0x2c]
        cmp esi, eax
        mov dword ptr [esp + 0x14], ebp
        jnz L_465b0a
        mov ecx, dword ptr [esp + 0x28]
        mov edi, dword ptr [edi + 0x4]
        mov edx, dword ptr [esp + 0x40]
        mov ebx, dword ptr [ebx + 0x4]
        inc ecx
        cmp edi, edx
        mov dword ptr [esp + 0x28], ecx
        jz L_4672ef
        jmp L_465af0
    L_467125:
        push offset kStr_004e013c
        push 0x735
        jmp L_46713b
    L_467131:
        push offset kStr_004e013c
        push 0x76a
    L_46713b:
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_46715a:
        push 0x78a
        push offset kStr_004dff3c
        push offset kStr_004e0108
        jmp L_467272
    L_46716e:
        push 0x7a2
        push offset kStr_004dff3c
        push offset kStr_004e0108
        jmp L_4672a0
    L_467182:
        push 0x7bd
        push offset kStr_004dff3c
        push offset kStr_004e0108
        jmp L_4672cf
    L_467196:
        push 0x7ea
        jmp L_467268
    L_4671a0:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x7fc
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e00d0
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_4671cf:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x829
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e00d0
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_4671fe:
        push 0x85e
        jmp L_467268
    L_467205:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x893
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e00d0
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_467234:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x8c9
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e00d0
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_467263:
        push 0x904
    L_467268:
        push offset kStr_004dff3c
        push offset kStr_004e00d0
    L_467272:
        mov eax, [g_Iat__iob_004cc4f8]
        add eax, 0x40
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_467291:
        push 0x939
        push offset kStr_004dff3c
        push offset kStr_004e00d0
    L_4672a0:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        add ecx, 0x40
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_4672c0:
        push 0x974
        push offset kStr_004dff3c
        push offset kStr_004e00d0
    L_4672cf:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        add edx, 0x40
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    L_4672ef:
        mov eax, dword ptr [esp + 0x20]
        mov ecx, dword ptr [eax + 0x64]
        test ecx, ecx
        jz L_46738a
        mov eax, edx
    L_467300:
        mov ecx, dword ptr [eax + 0x30]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_46730e
        mov dword ptr [ecx + 0x14], eax
    L_46730e:
        mov ecx, dword ptr [eax + 0x34]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_46731c
        mov dword ptr [ecx + 0xc], eax
    L_46731c:
        mov ecx, dword ptr [ebx + 0x30]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_46732a
        mov dword ptr [ecx + 0x18], ebx
    L_46732a:
        mov ecx, dword ptr [ebx + 0x34]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_467338
        mov dword ptr [ecx + 0x10], ebx
    L_467338:
        mov eax, dword ptr [eax + 0x4]
        mov ebx, dword ptr [ebx + 0x4]
        cmp eax, edx
        jnz L_467300
        mov edx, dword ptr [esp + 0x2c]
        mov eax, edx
    L_467348:
        mov ecx, dword ptr [eax + 0x30]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_467356
        mov dword ptr [ecx + 0x24], eax
    L_467356:
        mov ecx, dword ptr [eax + 0x34]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_467364
        mov dword ptr [ecx + 0x1c], eax
    L_467364:
        mov ecx, dword ptr [ebp + 0x30]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_467372
        mov dword ptr [ecx + 0x28], ebp
    L_467372:
        mov ecx, dword ptr [ebp + 0x34]
        test ecx, ecx
        mov dword ptr [esp + 0x10], ecx
        jz L_467380
        mov dword ptr [ecx + 0x20], ebp
    L_467380:
        mov eax, dword ptr [eax + 0x4]
        mov ebp, dword ptr [ebp + 0x4]
        cmp eax, edx
        jnz L_467348
    L_46738a:
        mov eax, dword ptr [esp + 0x24]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x44
        ret
    }
}

// 0x004676c0 Contour_EnsureBuffer - (EDX contour): if [+0x38] null: 0x00467660(0) (argument ECX not visible); failure -> 'New contour could not obtain buffer' (zg 0xC6F) return 0; buffer[1] = contour, buffer[0] = contour[+8]; store; return 1
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall Contour_EnsureBuffer(int, int)
{
    RECOIL_ENTRY(8)
    __asm {
        push esi
        mov esi, edx
        mov eax, dword ptr [esi + 0x38]
        test eax, eax
        jnz L_467708
        push 0x0
        mov edx, 0x1
        add ecx, 0x48
        call DynArray_Push
        test eax, eax
        jnz L_4676fd
        push offset kStr_004e0158
        push 0xc6f
        push offset kStr_004dff3c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
        xor eax, eax
        pop esi
        ret
    L_4676fd:
        mov dword ptr [eax + 0x4], esi
        mov ecx, dword ptr [esi + 0x8]
        mov dword ptr [eax], ecx
        mov dword ptr [esi + 0x38], eax
    L_467708:
        mov eax, 0x1
        pop esi
        ret
    }
}

// 0x00467710 WeilerClip_MergeContours - clip ECX, crossings [+0x68] (0x30 bytes each), count [+0x64]: ClipCrossings_Validate failure -> report 'contourMerge: Failed validation' (zg 0xC9A), 0. Per crossing (links at +0xC..+0x28, code +0x2C) a code-specific relink of the node rings: 4 and 5 swap both pairs of next/prev links (two contours cross), 6/7 and 8/9 relink one side each, 10/11 depend on whether the second link is null, 12..25 fur
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_MergeContours(int, int)
{
    RECOIL_ENTRY(9)
    __asm {
        sub esp, 0x18
        push ebx
        push ebp
        push esi
        push edi
        mov edi, ecx
        push 0x0
        mov esi, dword ptr [edi + 0x68]
        mov ecx, dword ptr [edi + 0x64]
        mov edx, esi
        call ClipCrossings_Validate
        test eax, eax
        jnz L_467752
        push offset kStr_004e01bc
        push 0xc9a
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467752:
        mov eax, dword ptr [edi + 0x64]
        mov dword ptr [esp + 0x20], 0x0
        test eax, eax
        jbe L_468040
        lea ebp, [esi + 0x10]
        mov dword ptr [esp + 0x1c], ebp
    L_46776c:
        mov eax, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp]
        mov ebx, dword ptr [ebp + 0x4]
        mov esi, dword ptr [ebp + 0x10]
        mov dword ptr [esp + 0x18], eax
        mov eax, dword ptr [ebp + 0xc]
        mov ebp, dword ptr [ebp + 0x14]
        mov dword ptr [esp + 0x14], ebp
        mov ebp, dword ptr [esp + 0x1c]
        mov dword ptr [esp + 0x24], ebx
        mov ebp, dword ptr [ebp + 0x18]
        mov dword ptr [esp + 0x10], ebp
        mov ebp, dword ptr [esp + 0x1c]
        mov ebp, dword ptr [ebp + 0x1c]
        add ebp, -0x3
        cmp ebp, 0x16
        ja L_467cb0
        cmp ebp, 0
        je L_467cb0
        cmp ebp, 1
        je L_4677b2
        cmp ebp, 2
        je L_467806
        cmp ebp, 3
        je L_467b01
        cmp ebp, 4
        je L_467b39
        cmp ebp, 5
        je L_467b73
        cmp ebp, 6
        je L_467ba9
        cmp ebp, 7
        je L_467bdf
        cmp ebp, 8
        je L_467c4c
        cmp ebp, 9
        je L_467a17
        cmp ebp, 10
        je L_467856
        cmp ebp, 11
        je L_467a35
        cmp ebp, 12
        je L_467a54
        cmp ebp, 13
        je L_46788e
        cmp ebp, 14
        je L_467a74
        cmp ebp, 15
        je L_467a91
        cmp ebp, 16
        je L_4678c8
        cmp ebp, 17
        je L_467aac
        cmp ebp, 18
        je L_467ac8
        cmp ebp, 19
        je L_4678fa
        cmp ebp, 20
        je L_467ae5
        cmp ebp, 21
        je L_467932
        cmp ebp, 22
        je L_4679a1
        int 3  // unreachable: the bounds check above excludes other indices
    L_4677b2:
        mov ebp, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [edx + 0x4], ebp
        mov ebp, dword ptr [esp + 0x18]
        mov dword ptr [ebx], eax
        mov ebx, dword ptr [esp + 0x14]
        mov dword ptr [ebp], ebx
        mov ebp, dword ptr [esp + 0x24]
        mov dword ptr [eax + 0x4], ebp
        mov dword ptr [esi + 0x4], ecx
        mov ecx, dword ptr [esp + 0x18]
        mov dword ptr [ebx], ecx
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [ecx], edx
        mov edx, eax
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467cdc
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467cdc
        jmp L_467cb0
    L_467806:
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x4], ebp
        mov ebp, dword ptr [esp + 0x10]
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [ebx], ebp
        mov ebp, dword ptr [esp + 0x18]
        mov dword ptr [ebp], esi
        mov dword ptr [eax + 0x4], edx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x4], ebp
        mov esi, dword ptr [esp + 0x14]
        mov dword ptr [esi], ecx
        mov dword ptr [edx], ebx
        mov edx, eax
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467ce6
        mov edx, esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467ce6
        jmp L_467cb0
    L_467856:
        mov eax, dword ptr [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x4], eax
        mov dword ptr [ebx], edx
        mov dword ptr [eax], ecx
        mov dword ptr [edx], ebx
        mov edx, ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d12
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d12
        jmp L_467cb0
    L_46788e:
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esp + 0x18]
        mov esi, dword ptr [esp + 0x14]
        mov dword ptr [edx + 0x4], ecx
        mov dword ptr [eax], esi
        mov dword ptr [esi], eax
        mov dword ptr [ecx], edx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d3e
        mov edx, esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d3e
        jmp L_467cb0
    L_4678c8:
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [ebx], eax
        mov dword ptr [eax + 0x4], ebx
        mov dword ptr [esi + 0x4], ecx
        mov edx, ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d48
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d48
        jmp L_467cb0
    L_4678fa:
        mov ecx, dword ptr [esp + 0x18]
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [ecx], esi
        mov dword ptr [eax + 0x4], edx
        mov dword ptr [esi + 0x4], ecx
        mov edx, eax
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d74
        mov edx, dword ptr [esp + 0x10]
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467d74
        jmp L_467cb0
    L_467932:
        test esi, esi
        jz L_46796c
        mov ebp, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [ebx], ebp
        mov dword ptr [esi + 0x4], ecx
        mov edx, esi
        mov ecx, edi
        mov dword ptr [ebp], ebx
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467da0
        mov edx, ebp
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467da0
        jmp L_467cb0
    L_46796c:
        mov esi, dword ptr [esp + 0x14]
        mov edx, eax
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [ebx], eax
        mov dword ptr [eax + 0x4], ebx
        mov dword ptr [esi], ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467daa
        mov edx, esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467daa
        jmp L_467cb0
    L_4679a1:
        test esi, esi
        jz L_4679de
        mov eax, dword ptr [esp + 0x18]
        mov ebx, dword ptr [esp + 0x10]
        mov dword ptr [esi + 0x4], eax
        mov ecx, edi
        mov dword ptr [eax], esi
        mov dword ptr [ebx], edx
        mov dword ptr [edx + 0x4], ebx
        mov edx, esi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467dd6
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467dd6
        jmp L_467cb0
    L_4679de:
        mov ecx, dword ptr [esp + 0x18]
        mov esi, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [ecx], eax
        mov dword ptr [esi], edx
        mov dword ptr [edx + 0x4], esi
        mov edx, eax
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467e02
        mov edx, esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467e02
        jmp L_467cb0
    L_467a17:
        mov eax, dword ptr [esp + 0x10]
        mov edx, ebx
        mov dword ptr [ebx], eax
        mov ecx, edi
        mov dword ptr [eax], ebx
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467e0c
        jmp L_467cb0
    L_467a35:
        mov eax, dword ptr [esp + 0x14]
        mov edx, ecx
        mov dword ptr [ecx + 0x4], eax
        mov dword ptr [eax], ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467e38
        jmp L_467cb0
    L_467a54:
        mov eax, dword ptr [esp + 0x18]
        mov edx, dword ptr [esp + 0x14]
        mov ecx, edi
        mov dword ptr [eax], edx
        mov dword ptr [edx], eax
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467e64
        jmp L_467cb0
    L_467a74:
        mov eax, dword ptr [esp + 0x10]
        mov ecx, edi
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [eax], edx
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467e6e
        jmp L_467cb0
    L_467a91:
        mov dword ptr [ebx], eax
        mov edx, ebx
        mov ecx, edi
        mov dword ptr [eax + 0x4], ebx
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467e9a
        jmp L_467cb0
    L_467aac:
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [esi + 0x4], ecx
        mov edx, ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467ec6
        jmp L_467cb0
    L_467ac8:
        mov edx, dword ptr [esp + 0x18]
        mov ecx, edi
        mov dword ptr [edx], esi
        mov dword ptr [esi + 0x4], edx
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467ed0
        jmp L_467cb0
    L_467ae5:
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [eax + 0x4], edx
        mov edx, eax
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467efc
        jmp L_467cb0
    L_467b01:
        mov ecx, dword ptr [esp + 0x18]
        mov esi, dword ptr [esp + 0x14]
        mov dword ptr [ebx], eax
        mov edx, eax
        mov dword ptr [ecx], esi
        mov dword ptr [eax + 0x4], ebx
        mov dword ptr [esi], ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f28
        mov edx, esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f28
        jmp L_467cb0
    L_467b39:
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esp + 0x18]
        mov dword ptr [ebx], ecx
        mov edx, esi
        mov dword ptr [eax], esi
        mov dword ptr [esi + 0x4], eax
        mov dword ptr [ecx], ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f32
        mov edx, dword ptr [esp + 0x14]
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f32
        jmp L_467cb0
    L_467b73:
        mov esi, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [eax + 0x4], edx
        mov dword ptr [esi], ecx
        mov edx, eax
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f5e
        mov edx, esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f5e
        jmp L_467cb0
    L_467ba9:
        mov ebx, dword ptr [esp + 0x10]
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [edx + 0x4], ebx
        mov dword ptr [esi + 0x4], ecx
        mov dword ptr [ebx], edx
        mov edx, ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f8a
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f8a
        jmp L_467cb0
    L_467bdf:
        test edx, edx
        jz L_467c1a
        mov esi, dword ptr [esp + 0x18]
        mov ecx, dword ptr [esp + 0x14]
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [esi], ecx
        mov dword ptr [eax + 0x4], edx
        mov dword ptr [ecx], esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f91
        mov edx, esi
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467f91
        jmp L_467cb0
    L_467c1a:
        mov edx, dword ptr [esp + 0x14]
        mov dword ptr [ecx + 0x4], edx
        mov dword ptr [ebx], eax
        mov dword ptr [eax + 0x4], ebx
        mov dword ptr [edx], ecx
        mov edx, ecx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467fbd
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467fbd
        jmp L_467cb0
    L_467c4c:
        mov eax, dword ptr [esp + 0x10]
        test edx, edx
        jz L_467c84
        mov ebx, dword ptr [esp + 0x18]
        mov dword ptr [edx + 0x4], eax
        mov dword ptr [eax], edx
        mov ecx, edi
        mov dword ptr [ebx], esi
        mov dword ptr [esi + 0x4], ebx
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467fe9
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_467fe9
        jmp L_467cb0
    L_467c84:
        mov dword ptr [ecx + 0x4], eax
        mov dword ptr [eax], ecx
        mov edx, ecx
        mov dword ptr [ebx], esi
        mov ecx, edi
        mov dword ptr [esi + 0x4], ebx
        call Contour_EnsureBuffer
        test eax, eax
        jz L_468014
        mov edx, ebx
        mov ecx, edi
        call Contour_EnsureBuffer
        test eax, eax
        jz L_468014
    L_467cb0:
        mov eax, dword ptr [esp + 0x20]
        mov ebp, dword ptr [esp + 0x1c]
        mov ecx, dword ptr [edi + 0x64]
        inc eax
        add ebp, 0x30
        cmp eax, ecx
        mov dword ptr [esp + 0x20], eax
        mov dword ptr [esp + 0x1c], ebp
        jc L_46776c
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467cdc:
        push 0xcc5
        jmp L_467fee
    L_467ce6:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xce0
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467d12:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xcf5
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467d3e:
        push 0xd0a
        jmp L_467fee
    L_467d48:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xd1f
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467d74:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xd34
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467da0:
        push 0xd4a
        jmp L_467fee
    L_467daa:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xd5a
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467dd6:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xd71
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467e02:
        push 0xd82
        jmp L_467fee
    L_467e0c:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xd95
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467e38:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xda7
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467e64:
        push 0xdb9
        jmp L_467fee
    L_467e6e:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xdcb
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467e9a:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xddd
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467ec6:
        push 0xdef
        jmp L_467fee
    L_467ed0:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xe01
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467efc:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xe13
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467f28:
        push 0xe28
        jmp L_467fee
    L_467f32:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xe3d
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467f5e:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xe52
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467f8a:
        push 0xe67
        jmp L_467fee
    L_467f91:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xe7d
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467fbd:
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xe8d
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0184
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_467fe9:
        push 0xea4
    L_467fee:
        mov eax, [g_Iat__iob_004cc4f8]
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004e0184
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_468014:
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0xeb4
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e0184
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret
    L_468040:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        add esp, 0x18
        ret
    }
}

// 0x004680b0 WeilerClip_ClassifyContours - sets [+0x28C9]=1; for each contour header (count [+0x50], 3 dwords at [+0x54]) with a first edge: flags = edge flags, orientation fix when flags == 6 (swap endpoints); walks the ring (ClipEdge_OrientFirst) OR-ing edge flags and counting edges, dropping cross links (+0x38) except for the first flagged edge which flips its endpoint pairs; any contour with flags & 3 == 3 clears [+0x28C9]
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_ClassifyContours(int, int)
{
    RECOIL_ENTRY(10)
    __asm {
        sub esp, 0x8
        mov eax, dword ptr [ecx + 0x50]
        push ebx
        mov ebx, 0x1
        push edi
        mov edi, dword ptr [ecx + 0x54]
        mov dword ptr [esp + 0xc], ecx
        test eax, eax
        mov byte ptr [ecx + 0x28c9], BL
        jz L_46818f
        push esi
        push ebp
        mov dword ptr [esp + 0x10], eax
    L_4680d8:
        mov eax, dword ptr [edi + 0x4]
        test eax, eax
        jz L_46817b
        mov esi, eax
        mov eax, dword ptr [esi + 0x8]
        mov ebp, eax
        mov dword ptr [edi], eax
        mov eax, dword ptr [esi + 0x8]
        and ebp, 0x3
        cmp eax, 0x6
        jnz L_468101
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [esi], ecx
        mov dword ptr [esi + 0x4], eax
    L_468101:
        mov ecx, esi
        mov dword ptr [edi + 0x8], ebx
        call ClipEdge_OrientFirst
        cmp eax, esi
        jz L_468166
    L_46810f:
        xor edx, edx
        cmp ebp, edx
        jnz L_468142
        mov ecx, dword ptr [eax + 0x8]
        test CL, 0x3
        jz L_468142
        mov eax, dword ptr [edi]
        mov ebp, ebx
        or eax, ecx
        mov dword ptr [edi], eax
        mov dword ptr [edi + 0x8], ebx
        mov eax, dword ptr [esi + 0x4]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [esi + 0x10]
        mov dword ptr [esi], eax
        mov eax, dword ptr [esi + 0xc]
        mov dword ptr [esi + 0xc], ecx
        mov dword ptr [esi + 0x4], edx
        mov dword ptr [esi + 0x10], eax
        mov ecx, esi
        jmp L_46815d
    L_468142:
        mov ecx, dword ptr [edi + 0x8]
        inc ecx
        mov dword ptr [edi + 0x8], ecx
        mov ecx, dword ptr [eax + 0x8]
        or dword ptr [edi], ecx
        mov ecx, dword ptr [eax + 0x38]
        cmp ecx, edx
        jz L_46815b
        mov dword ptr [ecx + 0x4], edx
        mov dword ptr [eax + 0x38], edx
    L_46815b:
        mov ecx, eax
    L_46815d:
        call ClipEdge_OrientFirst
        cmp eax, esi
        jnz L_46810f
    L_468166:
        mov edx, dword ptr [edi]
        and edx, 0x3
        cmp DL, 0x3
        jnz L_46817b
        mov eax, dword ptr [esp + 0x14]
        mov byte ptr [eax + 0x28c9], 0x0
    L_46817b:
        mov eax, dword ptr [esp + 0x10]
        add edi, 0xc
        dec eax
        mov dword ptr [esp + 0x10], eax
        jnz L_4680d8
        pop ebp
        pop esi
    L_46818f:
        pop edi
        pop ebx
        add esp, 0x8
        ret
    }
}

// 0x004681a0 WeilerClip_OutputContours - for each contour header (count [+0x50], 3 dwords [+0x54]) with an edge: mode byte [+0] bit 0 and class 3 -> ClipContour_Output(+0x70, output [+8]); bit 1 and class 6 or 2 -> (+0x84, output+8); bit 2 and class 1 or 5 -> (+0x98, output+0x10). Failure -> report 'Failed to output contours' (zg 0xF5E/0xF71, third case 'Found to output contours' 0xF7D) and return 0. Return 1
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_OutputContours(int, int)
{
    RECOIL_ENTRY(11)
    __asm {
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov ebx, dword ptr [esi + 0x50]
        mov edi, dword ptr [esi + 0x54]
        test ebx, ebx
        jz L_4682ac
    L_4681b3:
        mov eax, dword ptr [edi + 0x4]
        test eax, eax
        jz L_468231
        test byte ptr [esi], 0x1
        jz L_4681d9
        cmp dword ptr [edi], 0x3
        jnz L_4681d9
        mov eax, dword ptr [esi + 0x8]
        lea ecx, [esi + 0x70]
        push eax
        push ecx
        mov edx, edi
        mov ecx, esi
        call ClipContour_Output
        test eax, eax
        jz L_468246
    L_4681d9:
        test byte ptr [esi], 0x2
        jz L_468205
        mov eax, dword ptr [edi]
        cmp eax, 0x6
        jz L_4681ea
        cmp eax, 0x2
        jnz L_468205
    L_4681ea:
        mov edx, dword ptr [esi + 0x8]
        lea eax, [esi + 0x84]
        add edx, 0x8
        mov ecx, esi
        push edx
        push eax
        mov edx, edi
        call ClipContour_Output
        test eax, eax
        jz L_468268
    L_468205:
        test byte ptr [esi], 0x4
        jz L_468231
        mov eax, dword ptr [edi]
        cmp eax, 0x1
        jz L_468216
        cmp eax, 0x5
        jnz L_468231
    L_468216:
        mov ecx, dword ptr [esi + 0x8]
        lea edx, [esi + 0x98]
        add ecx, 0x10
        push ecx
        push edx
        mov edx, edi
        mov ecx, esi
        call ClipContour_Output
        test eax, eax
        jz L_46828a
    L_468231:
        dec ebx
        add edi, 0xc
        test ebx, ebx
        jnz L_4681b3
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        ret
    L_468246:
        push offset kStr_004e01fc
        push 0xf5e
        push offset kStr_004dff3c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        ret
    L_468268:
        push offset kStr_004e01fc
        push 0xf71
        push offset kStr_004dff3c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        ret
    L_46828a:
        push offset kStr_004e01e0
        push 0xf7d
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        ret
    L_4682ac:
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        ret
    }
}

// 0x004682c0 ClipContour_Output - (output ECX [+8] record, contour EDX, out contour counter arg2): header = DynArray_Push (header array; argument partly hidden) -> {n = contour [+8], start = output point count*3}, counter++; points = DynArray_Push on the output point array; failure -> fprintf 'outputContour call to bufEntry failed' (zg 0xFB9) return 0; output count += n; copies the first edge start point then each edge end point a
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipContour_Output(int, int, int, int)
{
    RECOIL_ENTRY(12)
    __asm {
        push ecx
        push ebx
        push ebp
        push esi
        push edi
        mov edi, edx
        mov ebp, ecx
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, 0x1
        mov esi, dword ptr [edi + 0x4]
        mov ebx, dword ptr [ebp + 0x8]
        add ecx, 0x4
        mov eax, dword ptr [esi]
        push ecx
        mov ecx, dword ptr [esp + 0x1c]
        mov dword ptr [esp + 0x14], eax
        call DynArray_Push
        test eax, eax
        jnz L_4682f7
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x8
    L_4682f7:
        mov ecx, dword ptr [ebx + 0x18]
        lea edx, [ecx + ecx*0x2]
        mov dword ptr [eax + 0x4], edx
        mov ecx, dword ptr [edi + 0x8]
        mov dword ptr [eax], ecx
        mov eax, dword ptr [esp + 0x1c]
        lea edx, [ebx + 0x1c]
        mov ecx, dword ptr [eax]
        push edx
        inc ecx
        mov dword ptr [eax], ecx
        mov edx, dword ptr [edi + 0x8]
        lea ecx, [ebp + 0xac]
        call DynArray_Push
        test eax, eax
        jnz L_46834f
        mov eax, [g_Iat__iob_004cc4f8]
        push 0xfb9
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004e0218
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x8
    L_46834f:
        mov ecx, dword ptr [edi + 0x8]
        mov edx, dword ptr [ebx + 0x18]
        add edx, ecx
        mov ecx, eax
        mov dword ptr [ebx + 0x18], edx
        mov edx, dword ptr [esi + 0xc]
        add eax, 0xc
        mov edi, dword ptr [edx]
        mov dword ptr [ecx], edi
        mov edi, dword ptr [edx + 0x4]
        mov dword ptr [ecx + 0x4], edi
        mov edx, dword ptr [edx + 0x8]
        mov dword ptr [ecx + 0x8], edx
    L_468372:
        mov ecx, dword ptr [esi + 0x10]
        mov edx, eax
        add eax, 0xc
        mov edi, dword ptr [ecx]
        mov dword ptr [edx], edi
        mov edi, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], edi
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], ecx
        mov esi, dword ptr [esi + 0x4]
        cmp esi, dword ptr [esp + 0x10]
        jnz L_468372
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        pop ecx
        ret 0x8
    }
}

// 0x00468470 WeilerClip_ComputeSideTables - two passes: pass 1 for each clip-edge (output points [+0x2C], count [+0x28]) stores the 2D cross product of every region point [+0x18] (count [+0x14]) against the edge into the float table at +0xC0, one row per edge plus a copy of the row's first value; the last edge wraps to the first point. Pass 2 swaps roles (region edges against output points) into the table at +0x14C0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_ComputeSideTables(int, int)
{
    RECOIL_ENTRY(13)
    __asm {
        sub esp, 0x10
        mov eax, dword ptr [ecx + 0x18]
        push ebx
        mov ebx, dword ptr [ecx + 0x14]
        push ebp
        mov ebp, dword ptr [ecx + 0x2c]
        push esi
        mov esi, dword ptr [ecx + 0x28]
        push edi
        mov dword ptr [esp + 0x1c], ecx
        lea edx, [ecx + 0xc0]
        mov dword ptr [esp + 0x10], eax
        mov dword ptr [esp + 0x18], 0x2
    L_468499:
        dec esi
        mov eax, ebp
        test esi, esi
        jbe L_4684fc
        mov dword ptr [esp + 0x14], esi
    L_4684a4:
        mov esi, dword ptr [esp + 0x10]
        mov ecx, edx
        test ebx, ebx
        jbe L_4684e3
        mov edi, ebx
    L_4684b0:
        fld dword ptr [eax + 0x10]
        fld dword ptr [esi]
        fld dword ptr [esi + 0x4]
        fld dword ptr [eax + 0xc]
        fxch st(3)
        fsub dword ptr [eax + 0x4]
        fxch st(2)
        fsub dword ptr [eax]
        fxch st(1)
        fsub dword ptr [eax + 0x4]
        fxch st(3)
        fsub dword ptr [eax]
        fxch st(1)
        fmulp st(2), st(0)
        add esi, 0xc
        add edx, 0x4
        fmulp st(2), st(0)
        fxch st(1)
        fsubp st(1), st(0)
        dec edi
        fstp dword ptr [edx - 0x4]
        jnz L_4684b0
    L_4684e3:
        mov ecx, dword ptr [ecx]
        add eax, 0xc
        mov dword ptr [edx], ecx
        mov ecx, dword ptr [esp + 0x14]
        add edx, 0x4
        dec ecx
        mov dword ptr [esp + 0x14], ecx
        jnz L_4684a4
        mov ecx, dword ptr [esp + 0x1c]
    L_4684fc:
        mov esi, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x14], edx
        test ebx, ebx
        jbe L_46853d
        mov edi, ebx
    L_46850a:
        fld dword ptr [esi]
        fld dword ptr [ebp + 0x4]
        fld dword ptr [esi + 0x4]
        fld dword ptr [ebp]
        fxch st(3)
        fsub dword ptr [eax]
        fxch st(2)
        fsub dword ptr [eax + 0x4]
        fxch st(1)
        fsub dword ptr [eax + 0x4]
        fxch st(3)
        fsub dword ptr [eax]
        fxch st(1)
        fmulp st(2), st(0)
        add esi, 0xc
        add edx, 0x4
        fmulp st(2), st(0)
        fxch st(1)
        fsubp st(1), st(0)
        dec edi
        fstp dword ptr [edx - 0x4]
        jnz L_46850a
    L_46853d:
        mov eax, dword ptr [esp + 0x14]
        mov ebp, dword ptr [esp + 0x10]
        mov esi, ebx
        mov eax, dword ptr [eax]
        mov dword ptr [edx], eax
        mov eax, dword ptr [esp + 0x18]
        mov edx, dword ptr [ecx + 0x2c]
        mov ebx, dword ptr [ecx + 0x28]
        dec eax
        mov dword ptr [esp + 0x10], edx
        lea edx, [ecx + 0x14c0]
        mov dword ptr [esp + 0x18], eax
        jnz L_468499
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x00468580 ClipEdge_Divide - (edge arg1, new point EDX, mark arg2): unless the point equals an endpoint within 0.001 (operands not visible): entry = DynArray_Push(0); failure -> fprintf 'divide edge call to bufEntry failed' (zg 0x113A) return 0; new edge from the point to edge's old end [+0x10], edge end = point, linked after edge (+4), copies +8 and +0x34, +0x14 = 1 on both, +0x38 = 0; else the existing next edge. If arg2: e
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipEdge_Divide(int, int, int, int)
{
    RECOIL_ENTRY(14)
    __asm {
        push ebx
        push esi
        mov esi, dword ptr [esp + 0xc]
        mov ebx, ecx
        push edi
        mov edi, edx
        mov ecx, dword ptr [esi + 0x10]
        push 0x3a83126f
        call Point2_NearlyEqual
        test eax, eax
        jz L_4685a1
        mov eax, dword ptr [esi + 0x4]
        jmp L_468617
    L_4685a1:
        push 0x0
        mov edx, 0x1
        lea ecx, [ebx + 0x34]
        call DynArray_Push
        test eax, eax
        jnz L_4685dd
        mov eax, [g_Iat__iob_004cc4f8]
        push 0x113a
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004e0248
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        ret 0x8
    L_4685dd:
        mov dword ptr [eax + 0xc], edi
        mov ecx, dword ptr [esi + 0x10]
        mov dword ptr [eax + 0x10], ecx
        mov edx, dword ptr [esi + 0x4]
        mov dword ptr [esi + 0x10], edi
        mov dword ptr [eax + 0x4], edx
        mov dword ptr [eax], esi
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [ecx], eax
        mov edx, dword ptr [esi + 0x8]
        mov dword ptr [esi + 0x4], eax
        mov dword ptr [eax + 0x8], edx
        mov dword ptr [eax + 0x38], 0x0
        mov ecx, dword ptr [esi + 0x34]
        mov dword ptr [eax + 0x34], ecx
        mov ecx, 0x1
        mov dword ptr [eax + 0x14], ecx
        mov dword ptr [esi + 0x14], ecx
    L_468617:
        mov ecx, dword ptr [esp + 0x14]
        test ecx, ecx
        jz L_468630
        mov dword ptr [esi + 0x34], edi
        mov dword ptr [eax + 0x30], edi
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        ret 0x8
    L_468630:
        pop edi
        mov dword ptr [eax + 0x30], 0x0
        pop esi
        mov eax, 0x1
        pop ebx
        ret 0x8
    }
}

// 0x00468650 ClipEdge_AddPair - (first edge EDX, second edge arg1, shared value arg2, flags arg3/arg4): twice: entry = DynArray_Push(0) (array ECX not visible); failure -> 'bufEntry failed' (zg 0x1181) return 0; entry +0 = edge, +4 = edge[+4], +8 = edge[+8] | flags, +0xC = arg2, +0x10 = edge[+0x10], +0x30 = 0, +0x34 = edge[+0x34], +0x38 = 0; ClipEdge_ComputeBounds; links after edge[+4] (edge[+4]->next = entry, edge[+4] = entry);
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipEdge_AddPair(int, int, int, int, int, int)
{
    RECOIL_ENTRY(15)
    __asm {
        push ecx
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0x18]
        push esi
        add ecx, 0x34
        push edi
        mov edi, edx
        mov ebx, 0x1
        mov dword ptr [esp + 0x10], ecx
    L_468667:
        mov ecx, dword ptr [esp + 0x10]
        push 0x0
        mov edx, 0x1
        call DynArray_Push
        mov esi, eax
        xor eax, eax
        cmp esi, eax
        jz L_4686d3
        mov dword ptr [esi], edi
        mov ecx, dword ptr [edi + 0x4]
        mov dword ptr [esi + 0x4], ecx
        mov edx, dword ptr [edi + 0x8]
        mov ecx, dword ptr [esp + 0x1c]
        or edx, ebp
        mov dword ptr [esi + 0x8], edx
        mov dword ptr [esi + 0xc], ecx
        mov edx, dword ptr [edi + 0x10]
        mov dword ptr [esi + 0x30], eax
        mov dword ptr [esi + 0x10], edx
        mov ecx, dword ptr [edi + 0x34]
        mov dword ptr [esi + 0x34], ecx
        mov ecx, esi
        mov dword ptr [esi + 0x38], eax
        call ClipEdge_ComputeBounds
        mov edx, dword ptr [edi + 0x4]
        mov ebp, dword ptr [esp + 0x24]
        mov eax, ebx
        dec ebx
        mov dword ptr [edx], esi
        mov dword ptr [edi + 0x4], esi
        mov edi, dword ptr [esp + 0x18]
        test eax, eax
        jnz L_468667
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x10
    L_4686d3:
        push offset kStr_004e0278
        push 0x1181
        push offset kStr_004dff3c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x10
    }
}

// 0x004687b0 WeilerClip_GenerateOutsideResult - mode byte [+0] bit 1 clear -> 1. Else pushes a header {count = output n + region n + 2, start = output point count*3} and that many points (DynArray_Push; failure -> fprintf 'gen outside rslts call to bufEntry failed' (zg 0x11F7/0x120F) and return 0); finds the rightmost vertex (ties within 1e-5 by larger y) of region and output, WeilerClip_FindCrossingVertex, then emits the output ring forward fr
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_GenerateOutsideResult(int, int)
{
    RECOIL_ENTRY(16)
    __asm {
        sub esp, 0x10
        push ebx
        push ebp
        mov ebp, ecx
        push esi
        push edi
        mov eax, dword ptr [ebp + 0x28]
        mov ebx, dword ptr [ebp + 0x14]
        mov ecx, dword ptr [ebp + 0x2c]
        mov esi, dword ptr [ebp + 0x8]
        mov dword ptr [esp + 0x18], eax
        mov AL, byte ptr [ebp]
        test AL, 0x2
        mov dword ptr [esp + 0x1c], ebx
        mov dword ptr [esp + 0x14], ecx
        jz L_4689f7
        lea edx, [esi + 0xc]
        lea ecx, [ebp + 0x84]
        push edx
        mov edx, 0x1
        call DynArray_Push
        mov edi, eax
        test edi, edi
        jnz L_468821
        mov eax, [g_Iat__iob_004cc4f8]
        push 0x11f7
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004e0288
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret
    L_468821:
        mov eax, dword ptr [esi + 0x18]
        mov edx, dword ptr [esp + 0x18]
        lea ecx, [eax + eax*0x2]
        lea eax, [edx + ebx*0x1 + 0x2]
        mov dword ptr [edi + 0x4], ecx
        mov dword ptr [edi], eax
        mov ebx, dword ptr [esi + 0x8]
        lea ecx, [esi + 0x1c]
        inc ebx
        push ecx
        mov dword ptr [esi + 0x8], ebx
        mov edx, dword ptr [edi]
        lea ecx, [ebp + 0xac]
        call DynArray_Push
        mov ebx, eax
        test ebx, ebx
        jnz L_46887e
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x120f
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e0288
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        xor AL, AL
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x10
        ret
    L_46887e:
        mov eax, dword ptr [edi]
        mov ecx, dword ptr [esi + 0x18]
        add ecx, eax
        mov eax, dword ptr [esp + 0x1c]
        dec eax
        mov dword ptr [esi + 0x18], ecx
        mov edx, dword ptr [ebp + 0x18]
        mov esi, eax
        dec eax
        mov dword ptr [esp + 0x10], edx
        test esi, esi
        lea ecx, [edx + 0xc]
        jz L_4688d8
        lea esi, [eax + 0x1]
    L_4688a1:
        fld dword ptr [ecx]
        fcomp dword ptr [edx]
        fnstsw AX
        test AH, 0x41
        jz L_4688cc
        fld dword ptr [ecx]
        fsub dword ptr [edx]
        fabs
        fcomp qword ptr [kD_004d2618]
        fnstsw AX
        test AH, 0x1
        jz L_4688d2
        fld dword ptr [ecx + 0x4]
        fcomp dword ptr [edx + 0x4]
        fnstsw AX
        test AH, 0x41
        jnz L_4688d2
    L_4688cc:
        mov edx, ecx
        mov dword ptr [esp + 0x10], edx
    L_4688d2:
        add ecx, 0xc
        dec esi
        jnz L_4688a1
    L_4688d8:
        mov edi, dword ptr [esp + 0x18]
        mov ecx, dword ptr [esp + 0x14]
        mov esi, ecx
        add ecx, 0xc
        lea eax, [edi - 0x1]
        mov dword ptr [esp + 0x18], eax
        mov edx, eax
        dec eax
        test edx, edx
        jz L_468929
        lea edx, [eax + 0x1]
    L_4688f6:
        fld dword ptr [ecx]
        fcomp dword ptr [esi]
        fnstsw AX
        test AH, 0x41
        jz L_468921
        fld dword ptr [ecx]
        fsub dword ptr [esi]
        fabs
        fcomp qword ptr [kD_004d2618]
        fnstsw AX
        test AH, 0x1
        jz L_468923
        fld dword ptr [esi + 0x4]
        fcomp dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x1
        jz L_468923
    L_468921:
        mov esi, ecx
    L_468923:
        add ecx, 0xc
        dec edx
        jnz L_4688f6
    L_468929:
        push ebp
        lea edx, [esp + 0x14]
        mov ecx, esi
        call WeilerClip_FindCrossingVertex
        lea eax, [edi + edi*0x2]
        mov ecx, dword ptr [esp + 0x14]
        test edi, edi
        lea eax, [ecx + eax*0x4 - 0xc]
        jz L_46897c
        mov edx, dword ptr [esp + 0x18]
        inc edx
        mov dword ptr [esp + 0x18], edx
    L_46894d:
        mov ecx, esi
        mov edx, ebx
        add ebx, 0xc
        cmp esi, eax
        mov edi, dword ptr [ecx]
        mov dword ptr [edx], edi
        mov edi, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], edi
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edx + 0x8], ecx
        jz L_46896d
        add esi, 0xc
        jmp L_468971
    L_46896d:
        mov esi, dword ptr [esp + 0x14]
    L_468971:
        mov ecx, dword ptr [esp + 0x18]
        dec ecx
        mov dword ptr [esp + 0x18], ecx
        jnz L_46894d
    L_46897c:
        mov eax, dword ptr [esi]
        mov edx, ebx
        add ebx, 0xc
        mov dword ptr [edx], eax
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [edx + 0x4], ecx
        mov eax, dword ptr [esi + 0x8]
        mov dword ptr [edx + 0x8], eax
        mov edx, dword ptr [esp + 0x1c]
        mov ecx, dword ptr [ebp + 0x18]
        test edx, edx
        jz L_4689e3
        lea eax, [edx]
        mov dword ptr [esp + 0x1c], eax
        mov eax, dword ptr [esp + 0x10]
    L_4689a6:
        mov edi, dword ptr [eax]
        mov esi, ebx
        add ebx, 0xc
        mov dword ptr [esi], edi
        mov edi, dword ptr [eax + 0x4]
        mov dword ptr [esi + 0x4], edi
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [esi + 0x8], eax
        mov eax, dword ptr [esp + 0x10]
        cmp eax, ecx
        jz L_4689c8
        sub eax, 0xc
        jmp L_4689d2
    L_4689c8:
        mov esi, dword ptr [ebp + 0x18]
        lea eax, [edx + edx*0x2]
        lea eax, [esi + eax*0x4 - 0xc]
    L_4689d2:
        mov esi, dword ptr [esp + 0x1c]
        mov dword ptr [esp + 0x10], eax
        dec esi
        mov dword ptr [esp + 0x1c], esi
        jnz L_4689a6
        jmp L_4689e7
    L_4689e3:
        mov eax, dword ptr [esp + 0x10]
    L_4689e7:
        mov ecx, dword ptr [eax]
        mov dword ptr [ebx], ecx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ebx + 0x4], edx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ebx + 0x8], eax
    L_4689f7:
        pop edi
        pop esi
        pop ebp
        mov AL, 0x1
        pop ebx
        add esp, 0x10
        ret
    }
}

// 0x00468c40 ClipSegments_Intersect2D - (point array ECX, out point ptr EDX, segment A a0 xyz p1-p3 / a1 p4-p6, segment B b0 p7-p9 / b1 p10-p12): code = 0x00468fa0(&b0, &b1) (the decompile types the int code as float; values are the denormal case labels = ints). Codes 4/5: proper crossing - parallel -> 0 and no point; else DynArray_Push a point at the A-parameter crossing, z interpolated along B (by x, or by y when B is vertical). 6/7 -
// Register/stack shape from the listing (ECX, EDX, 48 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipSegments_Intersect2D(int, int, int, int, int, int, int, int, int, int, int, int, int, int)
{
    RECOIL_ENTRY(17)
    __asm {
        sub esp, 0x28
        lea eax, [esp + 0x50]
        push ebx
        push ebp
        push esi
        push edi
        mov edi, ecx
        mov ebp, edx
        push edi
        lea ecx, [esp + 0x58]
        push eax
        push ecx
        lea edx, [esp + 0x54]
        lea ecx, [esp + 0x48]
        xor esi, esi
        call ClipSegments_Classify
        mov ebx, eax
        cmp ebx, 0x17
        ja L_468f4c
        xor edx, edx
        cmp ebx, 0
        jne Lsw_468c78_0
        mov DL, 0
        jmp L_468f4c
    Lsw_468c78_0:
        cmp ebx, 1
        jne Lsw_468c78_1
        mov DL, 6
        jmp L_468f4c
    Lsw_468c78_1:
        cmp ebx, 2
        jne Lsw_468c78_2
        mov DL, 6
        jmp L_468f4c
    Lsw_468c78_2:
        cmp ebx, 3
        jne Lsw_468c78_3
        mov DL, 6
        jmp L_468f4c
    Lsw_468c78_3:
        cmp ebx, 4
        jne Lsw_468c78_4
        mov DL, 1
        jmp L_468c7f
    Lsw_468c78_4:
        cmp ebx, 5
        jne Lsw_468c78_5
        mov DL, 1
        jmp L_468c7f
    Lsw_468c78_5:
        cmp ebx, 6
        jne Lsw_468c78_6
        mov DL, 2
        jmp L_468e97
    Lsw_468c78_6:
        cmp ebx, 7
        jne Lsw_468c78_7
        mov DL, 2
        jmp L_468e97
    Lsw_468c78_7:
        cmp ebx, 8
        jne Lsw_468c78_8
        mov DL, 3
        jmp L_468ef2
    Lsw_468c78_8:
        cmp ebx, 9
        jne Lsw_468c78_9
        mov DL, 3
        jmp L_468ef2
    Lsw_468c78_9:
        cmp ebx, 10
        jne Lsw_468c78_10
        mov DL, 6
        jmp L_468f4c
    Lsw_468c78_10:
        cmp ebx, 11
        jne Lsw_468c78_11
        mov DL, 6
        jmp L_468f4c
    Lsw_468c78_11:
        cmp ebx, 12
        jne Lsw_468c78_12
        mov DL, 4
        jmp L_468ddc
    Lsw_468c78_12:
        cmp ebx, 13
        jne Lsw_468c78_13
        mov DL, 4
        jmp L_468ddc
    Lsw_468c78_13:
        cmp ebx, 14
        jne Lsw_468c78_14
        mov DL, 4
        jmp L_468ddc
    Lsw_468c78_14:
        cmp ebx, 15
        jne Lsw_468c78_15
        mov DL, 4
        jmp L_468ddc
    Lsw_468c78_15:
        cmp ebx, 16
        jne Lsw_468c78_16
        mov DL, 4
        jmp L_468ddc
    Lsw_468c78_16:
        cmp ebx, 17
        jne Lsw_468c78_17
        mov DL, 4
        jmp L_468ddc
    Lsw_468c78_17:
        cmp ebx, 18
        jne Lsw_468c78_18
        mov DL, 5
        jmp L_468e38
    Lsw_468c78_18:
        cmp ebx, 19
        jne Lsw_468c78_19
        mov DL, 5
        jmp L_468e38
    Lsw_468c78_19:
        cmp ebx, 20
        jne Lsw_468c78_20
        mov DL, 5
        jmp L_468e38
    Lsw_468c78_20:
        cmp ebx, 21
        jne Lsw_468c78_21
        mov DL, 5
        jmp L_468e38
    Lsw_468c78_21:
        cmp ebx, 22
        jne Lsw_468c78_22
        mov DL, 5
        jmp L_468e38
    Lsw_468c78_22:
        cmp ebx, 23
        jne Lsw_468c78_23
        mov DL, 5
        jmp L_468e38
    Lsw_468c78_23:
        int 3  // unreachable: the bounds check above excludes other indices
    L_468c7f:
        fld dword ptr [esp + 0x54]
        fld dword ptr [esp + 0x58]
        fld dword ptr [esp + 0x48]
        fxch st(2)
        fsub dword ptr [esp + 0x60]
        fld dword ptr [esp + 0x4c]
        fxch st(2)
        fsub dword ptr [esp + 0x64]
        fxch st(3)
        fsub dword ptr [esp + 0x3c]
        fxch st(2)
        fsub dword ptr [esp + 0x40]
        fxch st(1)
        fst qword ptr [esp + 0x10]
        fxch st(3)
        fst qword ptr [esp + 0x18]
        fxch st(2)
        fstp qword ptr [esp + 0x20]
        fxch st(1)
        fmul qword ptr [esp + 0x20]
        fxch st(1)
        fstp qword ptr [esp + 0x28]
        fxch st(1)
        fmul qword ptr [esp + 0x28]
        fsubp st(1), st(0)
        fst qword ptr [esp + 0x30]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x40
        jnz L_468dd5
        push 0x0
        mov edx, 0x1
        lea ecx, [edi + 0x5c]
        call DynArray_Push
        mov esi, eax
        test esi, esi
        jnz L_468d01
        push 0x1301
        jmp L_468eb1
    L_468d01:
        fld qword ptr [kD_004d2630]
        fdiv qword ptr [esp + 0x30]
        fld qword ptr [esp + 0x10]
        fld dword ptr [esp + 0x54]
        fld qword ptr [esp + 0x18]
        fld dword ptr [esp + 0x58]
        fxch st(3)
        fmul st(0), st(4)
        fxch st(2)
        fsub dword ptr [esp + 0x3c]
        fxch st(1)
        fmul st(0), st(4)
        fxch st(2)
        fchs
        fxch st(2)
        fmulp st(1), st(0)
        fxch st(2)
        fsub dword ptr [esp + 0x40]
        fld qword ptr [esp + 0x20]
        fxch st(2)
        fmulp st(1), st(0)
        fld qword ptr [esp + 0x28]
        fxch st(1)
        faddp st(3), st(0)
        fxch st(2)
        fstp qword ptr [esp + 0x28]
        fmul qword ptr [esp + 0x28]
        fxch st(1)
        fmul qword ptr [esp + 0x28]
        fxch st(1)
        fadd dword ptr [esp + 0x3c]
        fstp dword ptr [esi]
        fadd dword ptr [esp + 0x40]
        fst dword ptr [esp + 0x10]
        fstp st(1)
        fstp dword ptr [esi + 0x4]
        fld dword ptr [esp + 0x54]
        fsub dword ptr [esp + 0x60]
        fld st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x40
        jnz L_468da7
        fld dword ptr [esp + 0x54]
        fsub dword ptr [esi]
        fld dword ptr [esp + 0x68]
        fsub dword ptr [esp + 0x5c]
        fxch st(1)
        fdiv st(0), st(2)
        fxch st(1)
        fmulp st(1), st(0)
        fadd dword ptr [esp + 0x5c]
        fstp dword ptr [esi + 0x8]
        fstp st(0)
        jmp L_468f4c
    L_468da7:
        fstp st(0)
        fld dword ptr [esp + 0x58]
        fld dword ptr [esp + 0x58]
        fsub dword ptr [esp + 0x10]
        fxch st(1)
        fsub dword ptr [esp + 0x64]
        fld dword ptr [esp + 0x68]
        fxch st(1)
        fdivp st(2), st(0)
        fsub dword ptr [esp + 0x5c]
        fmulp st(1), st(0)
        fadd dword ptr [esp + 0x5c]
        fstp dword ptr [esi + 0x8]
        jmp L_468f4c
    L_468dd5:
        xor ebx, ebx
        jmp L_468f4c
    L_468ddc:
        push 0x0
        mov edx, 0x1
        lea ecx, [edi + 0x5c]
        call DynArray_Push
        mov esi, eax
        test esi, esi
        jnz L_468e22
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x132a
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e02c0
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x28
        ret 0x30
    L_468e22:
        mov edx, dword ptr [esp + 0x54]
        mov dword ptr [esi], edx
        mov eax, dword ptr [esp + 0x58]
        mov dword ptr [esi + 0x4], eax
        mov ecx, dword ptr [esp + 0x5c]
        jmp L_468f49
    L_468e38:
        push 0x0
        mov edx, 0x1
        lea ecx, [edi + 0x5c]
        call DynArray_Push
        mov esi, eax
        test esi, esi
        jnz L_468e7e
        mov edx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x1340
        push offset kStr_004dff3c
        add edx, 0x40
        push offset kStr_004e02c0
        push edx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x28
        ret 0x30
    L_468e7e:
        mov eax, dword ptr [esp + 0x60]
        mov dword ptr [esi], eax
        mov ecx, dword ptr [esp + 0x64]
        mov dword ptr [esi + 0x4], ecx
        mov edx, dword ptr [esp + 0x68]
        mov dword ptr [esi + 0x8], edx
        jmp L_468f4c
    L_468e97:
        push 0x0
        mov edx, 0x1
        lea ecx, [edi + 0x5c]
        call DynArray_Push
        mov esi, eax
        test esi, esi
        jnz L_468edc
        push 0x1351
    L_468eb1:
        mov eax, [g_Iat__iob_004cc4f8]
        push offset kStr_004dff3c
        add eax, 0x40
        push offset kStr_004e02c0
        push eax
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x28
        ret 0x30
    L_468edc:
        mov ecx, dword ptr [esp + 0x3c]
        mov dword ptr [esi], ecx
        mov edx, dword ptr [esp + 0x40]
        mov dword ptr [esi + 0x4], edx
        mov eax, dword ptr [esp + 0x44]
        mov dword ptr [esi + 0x8], eax
        jmp L_468f4c
    L_468ef2:
        push 0x0
        mov edx, 0x1
        lea ecx, [edi + 0x5c]
        call DynArray_Push
        mov esi, eax
        test esi, esi
        jnz L_468f38
        mov ecx, dword ptr [g_Iat__iob_004cc4f8]
        push 0x1363
        push offset kStr_004dff3c
        add ecx, 0x40
        push offset kStr_004e02c0
        push ecx
        call dword ptr [g_Iat_fprintf_004cc5bc]
        add esp, 0x10
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x28
        ret 0x30
    L_468f38:
        mov edx, dword ptr [esp + 0x48]
        mov dword ptr [esi], edx
        mov eax, dword ptr [esp + 0x4c]
        mov dword ptr [esi + 0x4], eax
        mov ecx, dword ptr [esp + 0x50]
    L_468f49:
        mov dword ptr [esi + 0x8], ecx
    L_468f4c:
        test esi, esi
        jz L_468f53
        mov dword ptr [esi + 0x2c], ebx
    L_468f53:
        mov dword ptr [ebp], esi
        pop edi
        pop esi
        mov eax, ebx
        pop ebp
        pop ebx
        add esp, 0x28
        ret 0x30
    }
}

// 0x00468fa0 ClipSegments_Classify - (segment A a0 ECX / a1 EDX, segment B b0 arg1 / b1 arg2, clip arg3): side values of A endpoints against B (d1, d2) and B endpoints against A (d3, d4); both A endpoints strictly on one side, or both B endpoints strictly on one side -> 0. When exactly two of the four are zero, the signs are flipped according to Point2_InPolygonQuadrant against the region points [arg3+0x18] (operands partly hidden) t
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipSegments_Classify(int, int, int, int, int)
{
    RECOIL_ENTRY(18)
    __asm {
        sub esp, 0x1c
        push esi
        mov esi, dword ptr [esp + 0x24]
        push edi
        mov edi, dword ptr [esp + 0x2c]
        fld dword ptr [edi + 0x4]
        fld dword ptr [edi]
        fsub dword ptr [esi]
        fld dword ptr [ecx]
        fxch st(2)
        fsub dword ptr [esi + 0x4]
        fld dword ptr [ecx + 0x4]
        fxch st(3)
        fsub dword ptr [esi]
        fxch st(3)
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fstp dword ptr [esp + 0x10]
        fxch st(1)
        fstp dword ptr [esp + 0xc]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0xc]
        fld dword ptr [edx]
        fld dword ptr [edx + 0x4]
        fxch st(2)
        fsubp st(3), st(0)
        fsub dword ptr [esi]
        fxch st(1)
        fsub dword ptr [esi + 0x4]
        fxch st(2)
        fst dword ptr [esp + 0x14]
        fcom dword ptr [kF_004d2620]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fxch st(2)
        fmul dword ptr [esp + 0xc]
        fnstsw AX
        fsubp st(2), st(0)
        fxch st(1)
        test AH, 0x1
        fstp dword ptr [esp + 0x2c]
        jz L_46902b
        fld dword ptr [esp + 0x2c]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x1
        jnz L_4690e3
    L_46902b:
        fcom dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_469055
        fld dword ptr [esp + 0x2c]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_469055
        fstp st(0)
        xor eax, eax
        pop edi
        pop esi
        add esp, 0x1c
        ret 0xc
    L_469055:
        fld dword ptr [edx]
        fld dword ptr [edx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fsub dword ptr [ecx]
        fld dword ptr [esi + 0x4]
        fld dword ptr [esi]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(3)
        fstp dword ptr [esp + 0x8]
        fmul dword ptr [esp + 0x8]
        fxch st(2)
        fmul st(0), st(1)
        fld dword ptr [edi]
        fld dword ptr [edi + 0x4]
        fxch st(2)
        fsubp st(4), st(0)
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(3)
        fst dword ptr [esp + 0x28]
        fcomp dword ptr [kF_004d2620]
        fmul dword ptr [esp + 0x8]
        fxch st(2)
        fmul st(0), st(1)
        fnstsw AX
        fsubp st(2), st(0)
        fxch st(1)
        test AH, 0x1
        fstp dword ptr [esp + 0x8]
        fstp st(0)
        jz L_4690c1
        fld dword ptr [esp + 0x8]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x1
        jnz L_4690e3
    L_4690c1:
        fld dword ptr [esp + 0x28]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_4690ef
        fld dword ptr [esp + 0x8]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_4690ef
    L_4690e3:
        fstp st(0)
        xor eax, eax
        pop edi
        pop esi
        add esp, 0x1c
        ret 0xc
    L_4690ef:
        fcom dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x40
        jz L_469103
        mov edi, 0x1
        jmp L_469105
    L_469103:
        xor edi, edi
    L_469105:
        fld dword ptr [esp + 0x2c]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x40
        jz L_46911d
        mov edx, 0x1
        jmp L_46911f
    L_46911d:
        xor edx, edx
    L_46911f:
        fld dword ptr [esp + 0x28]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x40
        jz L_469137
        mov ecx, 0x1
        jmp L_469139
    L_469137:
        xor ecx, ecx
    L_469139:
        fld dword ptr [esp + 0x8]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x40
        jz L_469151
        mov eax, 0x1
        jmp L_469153
    L_469151:
        xor eax, eax
    L_469153:
        add eax, ecx
        add eax, edx
        add eax, edi
        cmp eax, 0x2
        jnz L_4692be
        fstp st(0)
        fld dword ptr [esp + 0x28]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x40
        jz L_469227
        fld dword ptr [esp + 0x8]
        fld dword ptr [esp + 0xc]
        fld dword ptr [esp + 0x10]
        fxch st(2)
        fcomp dword ptr [kF_004d2620]
        fmul dword ptr [kF_004d2638]
        fxch st(1)
        fmul dword ptr [kF_004d2638]
        fxch st(1)
        fsubr dword ptr [esi]
        fxch st(1)
        fsubr dword ptr [esi + 0x4]
        fxch st(1)
        fstp dword ptr [esp + 0x18]
        fnstsw AX
        fstp dword ptr [esp + 0x1c]
        test AH, 0x41
        mov eax, dword ptr [esp + 0x30]
        jnz L_4691f0
        mov ecx, dword ptr [eax + 0x18]
        mov edx, dword ptr [eax + 0x14]
        push ecx
        lea ecx, [esp + 0x1c]
        call Point2_InPolygonQuadrant
        test AL, AL
        jle L_4692ba
        fld dword ptr [esp + 0x14]
        fchs
        fld dword ptr [esp + 0x2c]
        fchs
        fstp dword ptr [esp + 0x2c]
        fld dword ptr [esp + 0x8]
        fchs
        fstp dword ptr [esp + 0x8]
        jmp L_4692be
    L_4691f0:
        mov edx, dword ptr [eax + 0x18]
        lea ecx, [esp + 0x18]
        push edx
        mov edx, dword ptr [eax + 0x14]
        call Point2_InPolygonQuadrant
        test AL, AL
        jge L_4692ba
        fld dword ptr [esp + 0x14]
        fchs
        fld dword ptr [esp + 0x2c]
        fchs
        fstp dword ptr [esp + 0x2c]
        fld dword ptr [esp + 0x8]
        fchs
        fstp dword ptr [esp + 0x8]
        jmp L_4692be
    L_469227:
        fld dword ptr [esp + 0x28]
        fcomp dword ptr [kF_004d2620]
        fld dword ptr [esp + 0x10]
        fld dword ptr [esp + 0xc]
        fmul dword ptr [kF_004d263c]
        fxch st(1)
        fmul dword ptr [kF_004d263c]
        fxch st(1)
        fsubr dword ptr [esi]
        fxch st(1)
        fsubr dword ptr [esi + 0x4]
        fxch st(1)
        fstp dword ptr [esp + 0x18]
        fnstsw AX
        fstp dword ptr [esp + 0x1c]
        test AH, 0x41
        mov eax, dword ptr [esp + 0x30]
        mov ecx, dword ptr [eax + 0x18]
        mov edx, dword ptr [eax + 0x14]
        push ecx
        lea ecx, [esp + 0x1c]
        jnz L_469295
        call Point2_InPolygonQuadrant
        test AL, AL
        jle L_4692ba
        fld dword ptr [esp + 0x14]
        fchs
        fld dword ptr [esp + 0x2c]
        fchs
        fstp dword ptr [esp + 0x2c]
        fld dword ptr [esp + 0x28]
        fchs
        fstp dword ptr [esp + 0x28]
        jmp L_4692be
    L_469295:
        call Point2_InPolygonQuadrant
        test AL, AL
        jge L_4692ba
        fld dword ptr [esp + 0x14]
        fchs
        fld dword ptr [esp + 0x2c]
        fchs
        fstp dword ptr [esp + 0x2c]
        fld dword ptr [esp + 0x28]
        fchs
        fstp dword ptr [esp + 0x28]
        jmp L_4692be
    L_4692ba:
        fld dword ptr [esp + 0x14]
    L_4692be:
        fcom qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_4692d1
        fstp st(0)
        xor ecx, ecx
        jmp L_4692e8
    L_4692d1:
        fcomp qword ptr [kD_004d2628]
        mov ecx, 0x1
        fnstsw AX
        test AH, 0x40
        jnz L_4692e8
        mov ecx, 0x2
    L_4692e8:
        fld dword ptr [esp + 0x2c]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_4692fd
        xor edi, edi
        jmp L_469318
    L_4692fd:
        fld dword ptr [esp + 0x2c]
        fcomp qword ptr [kD_004d2628]
        mov edi, 0x1
        fnstsw AX
        test AH, 0x40
        jnz L_469318
        mov edi, 0x2
    L_469318:
        fld dword ptr [esp + 0x28]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_46932d
        xor esi, esi
        jmp L_469348
    L_46932d:
        fld dword ptr [esp + 0x28]
        fcomp qword ptr [kD_004d2628]
        mov esi, 0x1
        fnstsw AX
        test AH, 0x40
        jnz L_469348
        mov esi, 0x2
    L_469348:
        fld dword ptr [esp + 0x8]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_46935d
        xor edx, edx
        jmp L_469378
    L_46935d:
        fld dword ptr [esp + 0x8]
        fcomp qword ptr [kD_004d2628]
        mov edx, 0x1
        fnstsw AX
        test AH, 0x40
        jnz L_469378
        mov edx, 0x2
    L_469378:
        lea eax, [edi + ecx*0x2]
        pop edi
        add ecx, eax
        mov eax, ecx
        lea ecx, [esi + eax*0x2]
        pop esi
        add eax, ecx
        lea edx, [edx + eax*0x2]
        add eax, edx
        mov eax, dword ptr [eax*0x4 + kSegmentClassTable_004dfdd0]
        add esp, 0x1c
        ret 0xc
    }
}

// 0x004693c0 ClipNodes_BuildRing - (nodes ECX (0x3C each), points EDX (0xC each), count arg1, owner arg2): node i: +0xC = &point i, +0x10 = &point i+1, +0 = self, +4 = next node, +8 = owner, +0x30/+0x34/+0x38 = 0; last node wraps: first node +0 = last node, last +4 = first, last +0x10 = first point
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipNodes_BuildRing(int, int, int, int)
{
    RECOIL_ENTRY(19)
    __asm {
        push ecx
        push ebx
        mov ebx, dword ptr [esp + 0xc]
        push esi
        push edi
        xor edi, edi
        mov dword ptr [esp + 0xc], edx
        cmp ebx, edi
        mov esi, edx
        lea eax, [ecx - 0x3c]
        jz L_46940b
        push ebp
        mov ebp, dword ptr [esp + 0x1c]
    L_4693dc:
        add eax, 0x3c
        lea edx, [eax - 0x3c]
        mov dword ptr [eax + 0xc], esi
        mov dword ptr [eax], edx
        lea edx, [eax + 0x3c]
        add esi, 0xc
        dec ebx
        mov dword ptr [eax + 0x4], edx
        mov dword ptr [eax + 0x8], ebp
        mov dword ptr [eax + 0x10], esi
        mov dword ptr [eax + 0x34], edi
        mov dword ptr [eax + 0x30], edi
        mov dword ptr [eax + 0x38], edi
        jnz L_4693dc
        mov edx, dword ptr [esp + 0x10]
        mov ebx, dword ptr [esp + 0x18]
        pop ebp
    L_46940b:
        lea esi, [ebx + ebx*0x2]
        pop edi
        lea esi, [esi + esi*0x4]
        lea esi, [ecx + esi*0x4 - 0x3c]
        mov dword ptr [ecx], esi
        pop esi
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [eax + 0x10], edx
        pop ebx
        pop ecx
        ret 0x8
    }
}

// 0x00469450 ClipEdge_ClassifyJoin - (edge ECX, next edge EDX, reference edge arg): 0 unless edge end == next start (pointer equality); start of edge and end of next on the same side of the reference line -> 2 when the turn (edge start, next end, shared point) is clockwise or straight, 1 when counter-clockwise; on opposite sides -> 7
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipEdge_ClassifyJoin(int, int, int)
{
    RECOIL_ENTRY(20)
    __asm {
        push ecx
        push esi
        mov esi, dword ptr [edx + 0xc]
        push edi
        mov edi, dword ptr [ecx + 0x10]
        xor eax, eax
        cmp edi, esi
        jnz L_469555
        mov esi, dword ptr [esp + 0x10]
        mov ecx, dword ptr [ecx + 0xc]
        mov edx, dword ptr [edx + 0x10]
        mov eax, dword ptr [esi + 0xc]
        mov esi, dword ptr [esi + 0x10]
        fld dword ptr [esi]
        fld dword ptr [esi + 0x4]
        fsub dword ptr [eax + 0x4]
        fxch st(1)
        fsub dword ptr [eax]
        fld dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fxch st(1)
        fsub dword ptr [eax]
        fxch st(1)
        fsub dword ptr [eax + 0x4]
        fxch st(3)
        fstp dword ptr [esp + 0x10]
        fxch st(1)
        fstp dword ptr [esp + 0x8]
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x8]
        fld dword ptr [edx]
        fld dword ptr [edx + 0x4]
        fxch st(2)
        fsubp st(3), st(0)
        fsub dword ptr [eax]
        fxch st(1)
        fsub dword ptr [eax + 0x4]
        fld st(2)
        fcomp qword ptr [kD_004d2628]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x8]
        fnstsw AX
        fsubp st(1), st(0)
        test AH, 0x1
        fstp dword ptr [esp + 0x10]
        jnz L_4694e5
        fld dword ptr [esp + 0x10]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469505
    L_4694e5:
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x41
        jz L_469550
        fld dword ptr [esp + 0x10]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x41
        jz L_469550
        jmp L_469507
    L_469505:
        fstp st(0)
    L_469507:
        fld dword ptr [edx]
        fld dword ptr [edi + 0x4]
        fld dword ptr [edi]
        fld dword ptr [edx + 0x4]
        fxch st(3)
        fsub dword ptr [ecx]
        fxch st(2)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fsub dword ptr [ecx]
        fxch st(3)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fmulp st(2), st(0)
        fmulp st(2), st(0)
        fxch st(1)
        fsubp st(1), st(0)
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x41
        jnz L_469545
        mov eax, 0x1
        pop edi
        pop esi
        pop ecx
        ret 0x4
    L_469545:
        mov eax, 0x2
        pop edi
        pop esi
        pop ecx
        ret 0x4
    L_469550:
        mov eax, 0x7
    L_469555:
        pop edi
        pop esi
        pop ecx
        ret 0x4
    }
}

// 0x00469560 ClipEdge_ClassifyTouch - (edge1 ECX, edge2 EDX meeting at a shared point: edge1 end == edge2 start, and edgeA arg1 end == edgeB arg2 start, else 0): side tests of edge1 start and edge2 end against the incoming/outgoing pair (A,B) give s1, s2 in {1,-1}. Both 1: 4 if all four turns around the shared point are clockwise (strictly negative), else 3. Mixed: 8 when s1 == 1 else 9 (returns 9 - (s1 != 1)). Both -1: 6 if the same 
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipEdge_ClassifyTouch(int, int, int, int, int)
{
    RECOIL_ENTRY(21)
    __asm {
        sub esp, 0x8
        mov eax, dword ptr [ecx + 0x10]
        push ebx
        push ebp
        mov ebp, dword ptr [edx + 0xc]
        push esi
        cmp eax, ebp
        push edi
        mov dword ptr [esp + 0x10], eax
        jnz L_46994a
        mov esi, dword ptr [esp + 0x1c]
        mov ebx, dword ptr [esp + 0x20]
        mov eax, dword ptr [esi + 0x10]
        mov edi, dword ptr [ebx + 0xc]
        cmp eax, edi
        jnz L_46994a
        fld dword ptr [eax + 0x4]
        mov esi, dword ptr [esi + 0xc]
        mov ecx, dword ptr [ecx + 0xc]
        fld dword ptr [eax]
        fsub dword ptr [esi]
        fld dword ptr [ecx]
        fxch st(2)
        fsub dword ptr [esi + 0x4]
        fld dword ptr [ecx + 0x4]
        fxch st(3)
        fsub dword ptr [esi]
        fxch st(3)
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fstp dword ptr [esp + 0x20]
        fxch st(1)
        fstp dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        mov ebx, dword ptr [ebx + 0x10]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fld dword ptr [ebx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [edi + 0x4]
        fxch st(1)
        fsub dword ptr [edi]
        fld dword ptr [ecx + 0x4]
        fld dword ptr [ecx]
        fsub dword ptr [edi]
        fxch st(1)
        fsub dword ptr [edi + 0x4]
        fxch st(1)
        fmul st(0), st(3)
        fxch st(1)
        fmul st(0), st(2)
        fnstsw AX
        fsubp st(1), st(0)
        test AH, 0x1
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        jz L_469639
        test AH, 0x1
        jnz L_46962f
        fld dword ptr [ebx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fsub dword ptr [esi]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_46966d
    L_46962f:
        mov dword ptr [esp + 0x14], 0x1
        jmp L_469675
    L_469639:
        test AH, 0x1
        jz L_46966d
        fld dword ptr [ebx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fsub dword ptr [esi]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        mov dword ptr [esp + 0x14], 0x1
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jnz L_469675
    L_46966d:
        mov dword ptr [esp + 0x14], 0xffffffff
    L_469675:
        mov edx, dword ptr [edx + 0x10]
        fld dword ptr [edx + 0x4]
        fld dword ptr [edx]
        fsub dword ptr [esi]
        fxch st(1)
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fld dword ptr [edx + 0x4]
        fld dword ptr [edx]
        fsub dword ptr [edi]
        fxch st(1)
        fsub dword ptr [edi + 0x4]
        fxch st(1)
        fmul st(0), st(3)
        fxch st(1)
        fmul st(0), st(2)
        fnstsw AX
        fsubp st(1), st(0)
        test AH, 0x1
        fcomp qword ptr [kD_004d2628]
        fstp st(0)
        fstp st(0)
        fnstsw AX
        jz L_4696f4
        test AH, 0x1
        jnz L_469720
        fld dword ptr [ebx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fsub dword ptr [esi]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469727
        mov eax, 0x1
        jmp L_46972a
    L_4696f4:
        test AH, 0x1
        jz L_469727
        fld dword ptr [ebx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fsub dword ptr [esi]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469727
    L_469720:
        mov eax, 0x1
        jmp L_46972a
    L_469727:
        or eax, 0xffffffff
    L_46972a:
        mov edi, dword ptr [esp + 0x14]
        cmp edi, -0x1
        jnz L_469832
        cmp eax, edi
        jnz L_469832
        mov eax, dword ptr [esp + 0x10]
        fld dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fsub dword ptr [ecx]
        fld dword ptr [esi + 0x4]
        fld dword ptr [esi]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(3)
        fstp dword ptr [esp + 0x1c]
        fxch st(1)
        fstp dword ptr [esp + 0x20]
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469823
        fld dword ptr [edx + 0x4]
        fld dword ptr [edx]
        fsub dword ptr [ebp]
        fld dword ptr [esi]
        fld dword ptr [esi + 0x4]
        fxch st(3)
        fsub dword ptr [ebp + 0x4]
        fxch st(1)
        fsub dword ptr [ebp]
        fxch st(3)
        fsub dword ptr [ebp + 0x4]
        fxch st(2)
        fstp dword ptr [esp + 0x14]
        fstp dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x14]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469823
        fld dword ptr [ebx + 0x4]
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469823
        fld dword ptr [ebx + 0x4]
        fld dword ptr [ebx]
        fsub dword ptr [ebp]
        fxch st(1)
        fsub dword ptr [ebp + 0x4]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x14]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469823
        mov eax, 0x6
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_469823:
        mov eax, 0x5
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_469832:
        cmp edi, 0x1
        jnz L_469936
        cmp eax, edi
        jnz L_469936
        mov eax, dword ptr [esp + 0x10]
        fld dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fsub dword ptr [ecx]
        fld dword ptr [esi + 0x4]
        fld dword ptr [esi]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(3)
        fstp dword ptr [esp + 0x1c]
        fxch st(1)
        fstp dword ptr [esp + 0x20]
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469927
        fld dword ptr [edx + 0x4]
        fld dword ptr [edx]
        fsub dword ptr [ebp]
        fld dword ptr [esi]
        fld dword ptr [esi + 0x4]
        fxch st(3)
        fsub dword ptr [ebp + 0x4]
        fxch st(1)
        fsub dword ptr [ebp]
        fxch st(3)
        fsub dword ptr [ebp + 0x4]
        fxch st(2)
        fstp dword ptr [esp + 0x14]
        fstp dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x14]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469927
        fld dword ptr [ebx + 0x4]
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469927
        fld dword ptr [ebx + 0x4]
        fld dword ptr [ebx]
        fsub dword ptr [ebp]
        fxch st(1)
        fsub dword ptr [ebp + 0x4]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0x14]
        fsubp st(1), st(0)
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469927
        mov eax, 0x4
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_469927:
        mov eax, 0x3
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_469936:
        mov eax, edi
        dec eax
        neg eax
        sbb eax, eax
        add eax, 0x9
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0xc
    L_46994a:
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0x8
        ret 0xc
    }
}

// 0x00469960 WeilerClip_TranslateToOrigin - output points [+0x2C] null: if the first region point [+0x18] is within +-65536 in x and y -> translated flag [+0x28C8] = 0; else offset [+0x28C0/+0x28C4] = first point xy, flag = 1, subtract from all [+0x14] region points. Output set: subtracts the stored offset from the [+0x28] output points
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_TranslateToOrigin(int, int)
{
    RECOIL_ENTRY(22)
    __asm {
        mov eax, dword ptr [ecx + 0x2c]
        push esi
        test eax, eax
        jz L_469996
        mov esi, dword ptr [ecx + 0x28]
        test esi, esi
        jz L_469a20
    L_469973:
        fld dword ptr [eax]
        fsub dword ptr [ecx + 0x28c0]
        fld dword ptr [eax + 0x4]
        fxch st(1)
        lea edx, [eax + 0x4]
        add eax, 0xc
        dec esi
        fstp dword ptr [eax - 0xc]
        fsub dword ptr [ecx + 0x28c4]
        fstp dword ptr [edx]
        jnz L_469973
        pop esi
        ret
    L_469996:
        mov edx, dword ptr [ecx + 0x18]
        fld dword ptr [edx]
        fcomp dword ptr [kF_004d2640]
        fnstsw AX
        test AH, 0x1
        jz L_4699e0
        fld dword ptr [edx]
        fcomp dword ptr [kF_004d2644]
        fnstsw AX
        test AH, 0x41
        jnz L_4699e0
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [kF_004d2640]
        fnstsw AX
        test AH, 0x1
        jz L_4699e0
        fld dword ptr [edx + 0x4]
        fcomp dword ptr [kF_004d2644]
        fnstsw AX
        test AH, 0x41
        jnz L_4699e0
        mov byte ptr [ecx + 0x28c8], 0x0
        pop esi
        ret
    L_4699e0:
        mov eax, dword ptr [edx]
        mov esi, dword ptr [ecx + 0x14]
        mov dword ptr [ecx + 0x28c0], eax
        mov eax, dword ptr [edx + 0x4]
        test esi, esi
        mov dword ptr [ecx + 0x28c4], eax
        mov byte ptr [ecx + 0x28c8], 0x1
        jz L_469a20
    L_4699ff:
        fld dword ptr [edx]
        fsub dword ptr [ecx + 0x28c0]
        fld dword ptr [edx + 0x4]
        fxch st(1)
        lea eax, [edx + 0x4]
        add edx, 0xc
        dec esi
        fstp dword ptr [edx - 0xc]
        fsub dword ptr [ecx + 0x28c4]
        fstp dword ptr [eax]
        jnz L_4699ff
    L_469a20:
        pop esi
        ret
    }
}

// 0x00469a30 WeilerClip_ResetWorkArrays - disassembly read: clip ECX with n = [+0x14]: IntLine_EvalAt (set count) on work arrays +0x34 (2n), +0x48 (2), +0x5C, +0x70, +0x84, +0x98, +0xAC (0). Nodes [+0x40], contour headers [+0x54]: ClipNodes_BuildRing(nodes, points [+0x18], n, owner 1), header0 +4 = nodes, nodes +0x38 = header0, ClipArray_Call468410Each(nodes, n); second ring at nodes + n*0x3C with owner 4, header at [+0x54]+0xC, same link
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_ResetWorkArrays(int, int)
{
    RECOIL_ENTRY(23)
    __asm {
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov edx, dword ptr [esi + 0x14]
        mov edi, dword ptr [esi + 0x40]
        mov ebx, dword ptr [esi + 0x54]
        lea ecx, [esi + 0x34]
        shl edx, 0x1
        call IntLine_EvalAt
        mov edx, 0x2
        lea ecx, [esi + 0x48]
        call IntLine_EvalAt
        xor edx, edx
        lea ecx, [esi + 0x5c]
        call IntLine_EvalAt
        xor edx, edx
        lea ecx, [esi + 0x70]
        call IntLine_EvalAt
        xor edx, edx
        lea ecx, [esi + 0x84]
        call IntLine_EvalAt
        xor edx, edx
        lea ecx, [esi + 0x98]
        call IntLine_EvalAt
        xor edx, edx
        lea ecx, [esi + 0xac]
        call IntLine_EvalAt
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esi + 0x18]
        push 0x1
        push eax
        mov ecx, edi
        call ClipNodes_BuildRing
        mov dword ptr [edi + 0x38], ebx
        mov dword ptr [ebx + 0x4], edi
        mov edx, dword ptr [esi + 0x14]
        mov ecx, edi
        call ClipArray_Call468410Each
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esi + 0x18]
        push 0x4
        push eax
        lea ecx, [eax + eax*0x2]
        add ebx, 0xc
        lea ecx, [ecx + ecx*0x4]
        lea edi, [edi + ecx*0x4]
        mov ecx, edi
        call ClipNodes_BuildRing
        mov dword ptr [edi + 0x38], ebx
        mov dword ptr [ebx + 0x4], edi
        mov edx, dword ptr [esi + 0x14]
        mov ecx, edi
        call ClipArray_Call468410Each
        pop edi
        pop esi
        pop ebx
        ret
    }
}

// 0x00469af0 WeilerClip_TranslateBack - offset x/y = [+0x28C0]/[+0x28C4]; adds it to the xy of each output point [+0x2C] (count [+0x28]) and of each region point [[+8]+0x1C] (count [[+8]+0x18])
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_TranslateBack(int, int)
{
    RECOIL_ENTRY(24)
    __asm {
        sub esp, 0x8
        mov eax, dword ptr [ecx + 0x28c0]
        mov edx, dword ptr [ecx + 0x28c4]
        push esi
        mov esi, dword ptr [ecx + 0x28]
        test esi, esi
        mov dword ptr [esp + 0x4], eax
        mov dword ptr [esp + 0x8], edx
        jz L_469b2e
        mov eax, dword ptr [ecx + 0x2c]
    L_469b12:
        fld dword ptr [esp + 0x4]
        fadd dword ptr [eax]
        fld dword ptr [esp + 0x8]
        fadd dword ptr [eax + 0x4]
        fxch st(1)
        fstp dword ptr [eax]
        lea edx, [eax + 0x4]
        add eax, 0xc
        dec esi
        fstp dword ptr [edx]
        jnz L_469b12
    L_469b2e:
        mov ecx, dword ptr [ecx + 0x8]
        mov edx, dword ptr [ecx + 0x18]
        test edx, edx
        jz L_469b57
        mov eax, dword ptr [ecx + 0x1c]
    L_469b3b:
        fld dword ptr [esp + 0x4]
        fadd dword ptr [eax]
        fld dword ptr [esp + 0x8]
        fadd dword ptr [eax + 0x4]
        fxch st(1)
        fstp dword ptr [eax]
        lea ecx, [eax + 0x4]
        add eax, 0xc
        dec edx
        fstp dword ptr [ecx]
        jnz L_469b3b
    L_469b57:
        pop esi
        add esp, 0x8
        ret
    }
}

// 0x00469b60 WeilerClip_RestoreZFromPlane - plane from the first three output points [+0x2C] (dz/dx, dz/dy by 2D determinant); determinant 0 -> nothing; else each result point in [[+8]+0x1C] (count [[+8]+0x18]) gets z from that plane through point 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_RestoreZFromPlane(int, int)
{
    RECOIL_ENTRY(25)
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x34
        push ebx
        push esi
        mov esi, ecx
        push edi
        lea eax, [ebp - 0x18]
        mov edi, dword ptr [esi + 0x2c]
        mov dword ptr [ebp - 0x4], eax
        mov dword ptr [ebp - 0xc], edi
        lea eax, [edi + 0xc]
        mov dword ptr [ebp - 0x8], eax
        mov ebx, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp - 0x8]
        mov edx, dword ptr [ebp - 0x4]
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
        lea ecx, [ebp - 0x24]
        lea edx, [edi + 0x18]
        mov dword ptr [ebp - 0xc], ecx
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0x4], edx
        mov ebx, dword ptr [ebp - 0x4]
        mov ecx, dword ptr [ebp - 0x8]
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
        lea eax, [ebp - 0x34]
        lea ecx, [ebp - 0x24]
        lea edx, [ebp - 0x18]
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x4], edx
        mov ebx, dword ptr [ebp - 0x4]
        mov ecx, dword ptr [ebp - 0x8]
        mov edx, dword ptr [ebp - 0xc]
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
        fld dword ptr [ebp - 0x2c]
        fcomp dword ptr [kF_004d2620]
        fnstsw AX
        test AH, 0x40
        jnz L_469c8c
        fld dword ptr [ebp - 0x34]
        fdiv dword ptr [ebp - 0x2c]
        mov edx, dword ptr [esi + 0x8]
        xor ecx, ecx
        fstp dword ptr [ebp - 0x34]
        fld dword ptr [ebp - 0x30]
        fdiv dword ptr [ebp - 0x2c]
        fstp dword ptr [ebp - 0x30]
        fld dword ptr [ebp - 0x34]
        fmul dword ptr [edi]
        fld dword ptr [edi + 0x4]
        fmul dword ptr [ebp - 0x30]
        faddp st(1), st(0)
        fadd dword ptr [edi + 0x8]
        fchs
        fstp dword ptr [ebp - 0x28]
        mov edi, dword ptr [edx + 0x18]
        mov eax, dword ptr [edx + 0x1c]
        test edi, edi
        jbe L_469c8c
    L_469c6b:
        fld dword ptr [ebp - 0x34]
        fmul dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fmul dword ptr [ebp - 0x30]
        inc ecx
        add eax, 0xc
        faddp st(1), st(0)
        fadd dword ptr [ebp - 0x28]
        fchs
        fstp dword ptr [eax - 0x4]
        mov edx, dword ptr [esi + 0x8]
        cmp ecx, dword ptr [edx + 0x18]
        jc L_469c6b
    L_469c8c:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00469d60 WeilerClip_FindCrossingVertex - (segment start ECX, in/out end pointer EDX, polygon arg: count +0x14, points +0x18): for each polygon edge, if start and *end lie on opposite sides of it (2D cross products), *end = the edge endpoint with larger x than start (else the previous vertex) and the scan restarts
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall WeilerClip_FindCrossingVertex(int, int, int)
{
    RECOIL_ENTRY(26)
    __asm {
        push ecx
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0x10]
        push esi
        push edi
        mov ebx, dword ptr [ebp + 0x14]
        mov edi, dword ptr [ebp + 0x18]
        lea eax, [ebx + ebx*0x2]
        lea esi, [edi + eax*0x4 - 0xc]
        mov eax, ebx
        dec ebx
        test eax, eax
        jz L_469e3e
    L_469d81:
        fld dword ptr [edi + 0x4]
        fld dword ptr [edi]
        fsub dword ptr [esi]
        fld dword ptr [ecx]
        fld dword ptr [ecx + 0x4]
        fxch st(3)
        fsub dword ptr [esi + 0x4]
        fxch st(1)
        fsub dword ptr [esi]
        fxch st(3)
        fsub dword ptr [esi + 0x4]
        fxch st(2)
        fstp dword ptr [esp + 0x10]
        mov eax, dword ptr [edx]
        fstp dword ptr [esp + 0x18]
        fxch st(1)
        fmul dword ptr [esp + 0x18]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fld dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fxch st(2)
        fsubp st(3), st(0)
        fsub dword ptr [esi]
        fxch st(1)
        fsub dword ptr [esi + 0x4]
        fld st(2)
        fcomp qword ptr [kD_004d2628]
        fxch st(1)
        fmul dword ptr [esp + 0x18]
        fxch st(1)
        fmul dword ptr [esp + 0x10]
        fnstsw AX
        fsubp st(1), st(0)
        test AH, 0x41
        fstp dword ptr [esp + 0x18]
        jnz L_469df5
        fld dword ptr [esp + 0x18]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jnz L_469e15
    L_469df5:
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x1
        jz L_469e33
        fld dword ptr [esp + 0x18]
        fcomp qword ptr [kD_004d2628]
        fnstsw AX
        test AH, 0x41
        jnz L_469e33
        jmp L_469e17
    L_469e15:
        fstp st(0)
    L_469e17:
        fld dword ptr [edi]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jnz L_469e24
        mov esi, edi
    L_469e24:
        mov dword ptr [edx], esi
        mov ebx, dword ptr [ebp + 0x14]
        mov edi, dword ptr [ebp + 0x18]
        lea eax, [ebx + ebx*0x2]
        lea esi, [edi + eax*0x4 - 0xc]
    L_469e33:
        mov eax, ebx
        dec ebx
        test eax, eax
        jnz L_469d81
    L_469e3e:
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    }
}

// 0x0046a130 PointArray_SnapToReference - (reference xyz ECX, ref count EDX, points arg1, count arg2, snap eps arg3, second eps arg4): for every reference x point pair: nearly equal within arg3 -> copy the reference point over it, changed = 1; else 0x00469e90(point, arg4) nonzero -> changed = 1. Returns changed
// Register/stack shape from the listing (ECX, EDX, 16 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall PointArray_SnapToReference(int, int, int, int, int, int)
{
    RECOIL_ENTRY(27)
    __asm {
        sub esp, 0xc
        push ebx
        xor ebx, ebx
        push ebp
        cmp edx, ebx
        mov dword ptr [esp + 0x10], edx
        mov ebp, ecx
        mov dword ptr [esp + 0x8], ebx
        jle L_46a1d7
        push edi
        push esi
    L_46a14b:
        mov eax, dword ptr [esp + 0x24]
        test eax, eax
        jle L_46a1cc
        mov esi, dword ptr [esp + 0x20]
        lea ecx, [ebx + ebx*0x2]
        mov dword ptr [esp + 0x14], eax
        lea edi, [ebp + ecx*0x4]
    L_46a162:
        mov edx, dword ptr [esp + 0x28]
        mov ecx, edi
        push edx
        mov edx, esi
        call Point2_NearlyEqual
        test eax, eax
        jz L_46a192
        mov eax, edi
        mov ecx, esi
        mov dword ptr [esp + 0x10], 0x1
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], eax
        jmp L_46a1ba
    L_46a192:
        lea eax, [ebx + 0x1]
        mov ecx, dword ptr [esp + 0x2c]
        cdq
        idiv dword ptr [esp + 0x18]
        push ecx
        push esi
        mov ecx, edi
        lea edx, [edx + edx*0x2]
        lea edx, [ebp + edx*0x4]
        call Point2_SnapOntoSegment
        test eax, eax
        jz L_46a1ba
        mov dword ptr [esp + 0x10], 0x1
    L_46a1ba:
        mov eax, dword ptr [esp + 0x14]
        add esi, 0xc
        dec eax
        mov dword ptr [esp + 0x14], eax
        jnz L_46a162
        mov edx, dword ptr [esp + 0x18]
    L_46a1cc:
        inc ebx
        cmp ebx, edx
        jl L_46a14b
        pop esi
        pop edi
    L_46a1d7:
        mov eax, dword ptr [esp + 0x8]
        pop ebp
        pop ebx
        add esp, 0xc
        ret 0x10
    }
}

// 0x0046a1f0 ClipCrossings_Validate - (crossing count ECX, crossing records EDX (0x30 bytes: 8 link dwords +0xC..+0x28, code +0x2C), out failing index arg): per crossing code 4..0x19 requires specific link fields to be non-null (a fixed table per code; codes outside 4..0x19 pass). First failure -> *arg = index, report 'validateXing failed (xing %d, type %d)' (zg 0x1788) and return 0 after finishing that record; else 1
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipCrossings_Validate(int, int, int)
{
    RECOIL_ENTRY(28)
    __asm {
        sub esp, 0x18
        push ebx
        push ebp
        push esi
        push edi
        test ecx, ecx
        mov dword ptr [esp + 0x1c], edx
        mov dword ptr [esp + 0x24], ecx
        mov dword ptr [esp + 0x10], 0x1
        mov dword ptr [esp + 0x20], 0x0
        jle L_46a576
    L_46a217:
        mov eax, dword ptr [esp + 0x10]
        test eax, eax
        jz L_46a576
        mov edx, dword ptr [esp + 0x1c]
        mov eax, dword ptr [edx + 0xc]
        mov ecx, dword ptr [edx + 0x10]
        mov edi, dword ptr [edx + 0x14]
        mov esi, dword ptr [edx + 0x18]
        mov ebp, dword ptr [edx + 0x1c]
        mov ebx, dword ptr [edx + 0x20]
        mov edx, dword ptr [edx + 0x24]
        mov dword ptr [esp + 0x14], edx
        mov edx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [edx + 0x28]
        mov dword ptr [esp + 0x18], edx
        mov edx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [edx + 0x2c]
        add edx, -0x3
        cmp edx, 0x16
        ja L_46a4f6
        cmp edx, 0
        je L_46a4f6
        cmp edx, 1
        je L_46a265
        cmp edx, 2
        je L_46a265
        cmp edx, 3
        je L_46a41f
        cmp edx, 4
        je L_46a448
        cmp edx, 5
        je L_46a45a
        cmp edx, 6
        je L_46a47c
        cmp edx, 7
        je L_46a48e
        cmp edx, 8
        je L_46a4ca
        cmp edx, 9
        je L_46a398
        cmp edx, 10
        je L_46a291
        cmp edx, 11
        je L_46a3a9
        cmp edx, 12
        je L_46a3ad
        cmp edx, 13
        je L_46a29d
        cmp edx, 14
        je L_46a3ce
        cmp edx, 15
        je L_46a3df
        cmp edx, 16
        je L_46a2c2
        cmp edx, 17
        je L_46a3e9
        cmp edx, 18
        je L_46a3ed
        cmp edx, 19
        je L_46a2df
        cmp edx, 20
        je L_46a406
        cmp edx, 21
        je L_46a2fc
        cmp edx, 22
        je L_46a34a
        int 3  // unreachable: the bounds check above excludes other indices
    L_46a265:
        test eax, eax
        jz L_46a4c0
        test ecx, ecx
        jz L_46a4c0
        test edi, edi
        jz L_46a4c0
        test esi, esi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        test ebx, ebx
        jmp L_46a2a7
    L_46a291:
        test eax, eax
        jz L_46a4c0
        test edi, edi
        jmp L_46a2a7
    L_46a29d:
        test ecx, ecx
        jz L_46a4c0
        test esi, esi
    L_46a2a7:
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x18]
        jmp L_46a4bc
    L_46a2c2:
        test eax, eax
        jz L_46a4c0
        test edi, edi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        jmp L_46a3f5
    L_46a2df:
        test ecx, ecx
        jz L_46a4c0
        test esi, esi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        jmp L_46a3f5
    L_46a2fc:
        test ebx, ebx
        jz L_46a319
        test eax, eax
        jz L_46a4c0
        test edi, edi
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], edi
        jmp L_46a4f6
    L_46a319:
        test eax, eax
        jz L_46a4c0
        test edi, edi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], 0x0
        jmp L_46a4f6
    L_46a34a:
        test ebx, ebx
        jz L_46a367
        test ecx, ecx
        jz L_46a4c0
        test esi, esi
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], esi
        jmp L_46a4f6
    L_46a367:
        test ecx, ecx
        jz L_46a4c0
        test esi, esi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], 0x0
        jmp L_46a4f6
    L_46a398:
        test edi, edi
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x18]
        jmp L_46a4bc
    L_46a3a9:
        test eax, eax
        jmp L_46a3af
    L_46a3ad:
        test esi, esi
    L_46a3af:
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], 0x0
        jmp L_46a4f6
    L_46a3ce:
        test ecx, ecx
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x18]
        jmp L_46a4bc
    L_46a3df:
        test edi, edi
        jz L_46a4c0
        jmp L_46a40e
    L_46a3e9:
        test eax, eax
        jmp L_46a3ef
    L_46a3ed:
        test esi, esi
    L_46a3ef:
        jz L_46a4c0
    L_46a3f5:
        test ebx, ebx
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], ebx
        jmp L_46a4f6
    L_46a406:
        test ecx, ecx
        jz L_46a4c0
    L_46a40e:
        test ebp, ebp
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], ebp
        jmp L_46a4f6
    L_46a41f:
        test esi, esi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], 0x0
        jmp L_46a4f6
    L_46a448:
        test edi, edi
        jz L_46a4c0
        test esi, esi
        jz L_46a4c0
        test ebx, ebx
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x18]
        jmp L_46a4bc
    L_46a45a:
        test eax, eax
        jz L_46a4c0
        test ecx, ecx
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], 0x0
        jmp L_46a4f6
    L_46a47c:
        test eax, eax
        jz L_46a4c0
        test ecx, ecx
        jz L_46a4c0
        test ebx, ebx
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x18]
        jmp L_46a4bc
    L_46a48e:
        test ecx, ecx
        jz L_46a4ac
        test esi, esi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
        test eax, eax
        jnz L_46a4f6
        mov dword ptr [esp + 0x10], 0x0
        jmp L_46a4f6
    L_46a4ac:
        test eax, eax
        jz L_46a4c0
        test edi, edi
        jz L_46a4c0
        test ebp, ebp
        jz L_46a4c0
        mov eax, dword ptr [esp + 0x14]
    L_46a4bc:
        test eax, eax
        jnz L_46a4f6
    L_46a4c0:
        mov dword ptr [esp + 0x10], 0x0
        jmp L_46a4f6
    L_46a4ca:
        xor edx, edx
        cmp ecx, edx
        jz L_46a4e0
        cmp esi, edx
        jz L_46a4f2
        cmp ebx, edx
        jz L_46a4f2
        cmp dword ptr [esp + 0x18], edx
        jnz L_46a4f6
        jmp L_46a4f2
    L_46a4e0:
        cmp eax, edx
        jz L_46a4f2
        cmp edi, edx
        jz L_46a4f2
        cmp ebx, edx
        jz L_46a4f2
        cmp dword ptr [esp + 0x18], edx
        jnz L_46a4f6
    L_46a4f2:
        mov dword ptr [esp + 0x10], edx
    L_46a4f6:
        mov eax, dword ptr [esp + 0x10]
        test eax, eax
        jnz L_46a556
        mov eax, dword ptr [esp + 0x2c]
        mov esi, dword ptr [esp + 0x20]
        test eax, eax
        jz L_46a50c
        mov dword ptr [eax], esi
    L_46a50c:
        mov eax, dword ptr [esp + 0x1c]
        test eax, eax
        jz L_46a537
        mov eax, dword ptr [eax + 0x2c]
        push eax
        push esi
        push offset kStr_004e0320
        push 0x1788
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x18
        jmp L_46a55a
    L_46a537:
        push esi
        push offset kStr_004e02f0
        push 0x179a
        push offset kStr_004dff3c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x14
        jmp L_46a55a
    L_46a556:
        mov esi, dword ptr [esp + 0x20]
    L_46a55a:
        mov ecx, dword ptr [esp + 0x1c]
        mov eax, dword ptr [esp + 0x24]
        inc esi
        add ecx, 0x30
        cmp esi, eax
        mov dword ptr [esp + 0x20], esi
        mov dword ptr [esp + 0x1c], ecx
        jl L_46a217
    L_46a576:
        mov eax, dword ptr [esp + 0x10]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x18
        ret 0x4
    }
}

}  // namespace recoil
