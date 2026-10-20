// SUBSYSTEM: geometry
// Original file: GameZRecoil\zGeometry\zgeo_model.cpp (ledger orig_file). Spec: 04_spec/systems/geometry.md
// Instruction-level ports converted from the Ghidra listings with tools/asm_port/ghidra2masm.py (structures are
// accessed at the listing's offsets; MSVCRT calls go through the import slots of platform/msvcrt.h).
#include "GameZRecoil/zGeometry/zgeo_model.h"
#include "GameZRecoil/zGeometry/zgeo_weiler.h"
#include "unattributed/ui_widgets.h"  // Debug_ReportNoop 0x00404e80
#include "platform/msvcrt.h"  // MSVCRT import slots (malloc, realloc, free, memmove)
#include "GameZRecoil/zClass/Class.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"
#include "unattributed/geometry.h"
#include "GameZRecoil/zGeometry/zgeo_convexify.h"
#include "GameZRecoil/zDEClient/zdec_crater.h"
#include "GameZRecoil/zClass/Object3d.h"
#include "unattributed/render_frame.h"

namespace recoil {

// 0x0046a9c0 Rect_FromPoints2D (ECX rect min/max, EDX 3-float points, stack count; ret 4): x/y bounding rectangle.
__declspec(naked) void __fastcall Rect_FromPoints2D(float*, const float*, int)
{
    __asm {
        fld dword ptr [edx]
        mov eax, dword ptr [esp + 0x4]
        add edx, 0xc
        fst dword ptr [ecx + 0x8]
        fstp dword ptr [ecx]
        fld dword ptr [edx - 0x8]
        cmp eax, 0x1
        fst dword ptr [ecx + 0xc]
        fstp dword ptr [ecx + 0x4]
        jle L_46aa33
        push esi
        lea esi, [eax - 0x1]
    L_46a9e0:
        fld dword ptr [edx]
        fcomp dword ptr [ecx]
        fnstsw AX
        test AH, 0x1
        jz L_46a9ef
        mov eax, dword ptr [edx]
        mov dword ptr [ecx], eax
    L_46a9ef:
        fld dword ptr [ecx + 0x8]
        fld dword ptr [edx]
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_46aa02
        mov eax, dword ptr [edx]
        mov dword ptr [ecx + 0x8], eax
    L_46aa02:
        fld dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0x4]
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_46aa17
        fld dword ptr [edx + 0x4]
        fstp dword ptr [ecx + 0x4]
    L_46aa17:
        fld dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0xc]
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_46aa2c
        fld dword ptr [edx + 0x4]
        fstp dword ptr [ecx + 0xc]
    L_46aa2c:
        add edx, 0xc
        dec esi
        jnz L_46a9e0
        pop esi
    L_46aa33:
        ret 0x4
    }
}

// 0x0046aab0 ClipPolygon_CopyOutRotated (ECX polygon, EDX out count, stack in/out buffer; ret 4): realloc the buffer,
// copy the points, Vec3Array_RotateX90 them -> 0.
__declspec(naked) int __fastcall ClipPolygon_CopyOutRotated(const void*, int*, float**)
{
    __asm {
        push ebx
        push ebp
        push esi
        mov esi, ecx
        mov ebp, dword ptr [esp + 0x10]
        mov ebx, edx
        mov eax, dword ptr [esi + 0x8]
        push edi
        mov dword ptr [ebx], eax
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [ebp]
        lea ecx, [eax + eax*0x2]
        shl ecx, 0x2
        push ecx
        push edx
        call dword ptr [g_Iat_realloc_004cc4ec]
        mov dword ptr [ebp], eax
        mov ecx, dword ptr [esi + 0x8]
        mov esi, dword ptr [esi + 0x4]
        mov edi, eax
        lea ecx, [ecx + ecx*0x2]
        add esp, 0x8
        shl ecx, 0x2
        mov eax, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        mov edx, dword ptr [ebp]
        mov ecx, dword ptr [ebx]
        call Vec3Array_RotateX90
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    }
}

// 0x0046ab10 ClipResult_Free (ECX result): frees [+0x4], destroys the clip at [+0x0], frees the result.
__declspec(naked) void __fastcall ClipResult_Free(void*)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [g_Iat_free_004cc5b4]
        mov eax, dword ptr [esi + 0x4]
        test eax, eax
        jz L_46ab27
        push eax
        call edi
        add esp, 0x4
    L_46ab27:
        mov ecx, dword ptr [esi]
        call WeilerClip_Destroy
        push esi
        call edi
        add esp, 0x4
        pop edi
        pop esi
        ret
    }
}

// 0x0046af00 Alloc16Zeroed -> malloc(16) with four zero words.
__declspec(naked) void* Alloc16Zeroed()
{
    __asm {
        push 0x10
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov edx, eax
        xor ecx, ecx
        add esp, 0x4
        mov dword ptr [edx], ecx
        mov dword ptr [edx + 0x4], ecx
        mov dword ptr [edx + 0x8], ecx
        mov dword ptr [edx + 0xc], ecx
        ret
    }
}

// 0x0046af20 Alloc16_Free (ECX block): frees [+0xc] if any, then the block.
__declspec(naked) void __fastcall Alloc16_Free(void*)
{
    __asm {
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [g_Iat_free_004cc5b4]
        mov eax, dword ptr [esi + 0xc]
        test eax, eax
        jz L_46af37
        push eax
        call edi
        add esp, 0x4
    L_46af37:
        push esi
        call edi
        add esp, 0x4
        pop edi
        pop esi
        ret
    }
}

// 0x0046b650 ModelPolygon_GatherPoints (ECX model, EDX polygon, stack buffer; ret 4): realloc to the polygon's point
// count (low byte of [+0x0]) and copy the indexed model vertices -> the buffer.
__declspec(naked) void* __fastcall ModelPolygon_GatherPoints(const void*, const void*, void*)
{
    __asm {
        push ebx
        push esi
        mov esi, edx
        mov ebx, ecx
        mov ecx, dword ptr [esp + 0xc]
        mov eax, dword ptr [esi]
        and eax, 0xff
        lea eax, [eax + eax*0x2]
        shl eax, 0x2
        push eax
        push ecx
        call dword ptr [g_Iat_realloc_004cc4ec]
        mov edx, dword ptr [esi]
        add esp, 0x8
        xor ecx, ecx
        mov dword ptr [esp + 0xc], eax
        test edx, 0xff
        jbe L_46b6be
        push edi
        push ebp
        mov edi, eax
    L_46b686:
        mov edx, dword ptr [esi + 0x8]
        inc ecx
        mov edx, dword ptr [edx + ecx*0x4 - 0x4]
        lea eax, [edx + edx*0x2]
        mov edx, dword ptr [ebx + 0x34]
        lea eax, [edx + eax*0x4]
        mov edx, edi
        add edi, 0xc
        mov ebp, dword ptr [eax]
        mov dword ptr [edx], ebp
        mov ebp, dword ptr [eax + 0x4]
        mov dword ptr [edx + 0x4], ebp
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [edx + 0x8], eax
        mov edx, dword ptr [esi]
        and edx, 0xff
        cmp ecx, edx
        jc L_46b686
        mov eax, dword ptr [esp + 0x14]
        pop ebp
        pop edi
    L_46b6be:
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x0046bf30 EdgeList_FindUsingVertex (ECX vertex, EDX edge count, stack edges, stack out indices; ret 8) -> count.
__declspec(naked) int __fastcall EdgeList_FindUsingVertex(int, int, const void*, int*)
{
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 0xc]
        push esi
        xor eax, eax
        xor esi, esi
        test edx, edx
        jle L_46bf67
        push edi
        mov edi, dword ptr [esp + 0x10]
        push ebp
        add edi, 0x4
    L_46bf47:
        mov ebp, dword ptr [edi + 0x4]
        test ebp, ebp
        jz L_46bf5d
        cmp dword ptr [edi - 0x4], ecx
        jz L_46bf57
        cmp dword ptr [edi], ecx
        jnz L_46bf5d
    L_46bf57:
        mov dword ptr [ebx], esi
        add ebx, 0x4
        inc eax
    L_46bf5d:
        inc esi
        add edi, 0xc
        cmp esi, edx
        jl L_46bf47
        pop ebp
        pop edi
    L_46bf67:
        pop esi
        pop ebx
        ret 0x8
    }
}

// 0x0046bf70 EdgeList_FindEdge (ECX a, EDX b, stack count, stack edges; ret 8) -> the edge, or null.
__declspec(naked) const void* __fastcall EdgeList_FindEdge(int, int, int, const void*)
{
    __asm {
        mov eax, dword ptr [esp + 0x8]
        push ebx
        mov ebx, dword ptr [esp + 0x8]
        push esi
        push edi
        xor edi, edi
        test ebx, ebx
        jle L_46bfb8
    L_46bf81:
        mov esi, dword ptr [eax + 0x8]
        test esi, esi
        mov esi, dword ptr [eax]
        jnz L_46bf9e
        cmp esi, ecx
        jnz L_46bf93
        cmp dword ptr [eax + 0x4], edx
        jz L_46bfb8
    L_46bf93:
        cmp dword ptr [eax + 0x4], ecx
        jnz L_46bfb0
        cmp esi, edx
        jz L_46bfb8
        jmp L_46bfb0
    L_46bf9e:
        cmp esi, ecx
        jnz L_46bfa7
        cmp dword ptr [eax + 0x4], edx
        jz L_46bfba
    L_46bfa7:
        cmp dword ptr [eax + 0x4], ecx
        jnz L_46bfb0
        cmp esi, edx
        jz L_46bfba
    L_46bfb0:
        inc edi
        add eax, 0xc
        cmp edi, ebx
        jl L_46bf81
    L_46bfb8:
        xor eax, eax
    L_46bfba:
        pop edi
        pop esi
        pop ebx
        ret 0x8
    }
}

// 0x0046c5b0 Vec3Array_ReverseKeepFirst (ECX count, EDX points): reverses points 1..count-1 in place.
__declspec(naked) void __fastcall Vec3Array_ReverseKeepFirst(int, float*)
{
    __asm {
        sub esp, 0x10
        mov eax, ecx
        lea ecx, [ecx + ecx*0x2]
        push esi
        mov esi, edx
        cdq
        sub eax, edx
        lea edx, [esi + 0xc]
        sar eax, 0x1
        lea ecx, [esi + ecx*0x4 - 0xc]
        mov esi, eax
        dec eax
        test esi, esi
        jz L_46c61a
        push edi
        inc eax
        push ebp
        push ebx
        mov dword ptr [esp + 0x10], eax
    L_46c5d6:
        mov edi, ecx
        mov ebx, ecx
        sub ecx, 0xc
        mov eax, dword ptr [edi]
        mov esi, dword ptr [edi + 0x4]
        mov edi, dword ptr [edi + 0x8]
        mov dword ptr [esp + 0x1c], edi
        mov edi, edx
        mov ebp, dword ptr [edi]
        mov dword ptr [ebx], ebp
        mov ebp, dword ptr [edi + 0x4]
        mov dword ptr [ebx + 0x4], ebp
        mov edi, dword ptr [edi + 0x8]
        mov dword ptr [ebx + 0x8], edi
        mov edi, edx
        add edx, 0xc
        mov dword ptr [edi], eax
        mov eax, dword ptr [esp + 0x1c]
        mov dword ptr [edi + 0x4], esi
        mov dword ptr [edi + 0x8], eax
        mov eax, dword ptr [esp + 0x10]
        dec eax
        mov dword ptr [esp + 0x10], eax
        jnz L_46c5d6
        pop ebx
        pop ebp
        pop edi
    L_46c61a:
        pop esi
        add esp, 0x10
        ret
    }
}

namespace {
const float kF_004d264c = 1.0;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d264c
const double kD_004d2658 = 0.0;  // CONFIRMED-BINARY: double 0x0000000000000000 at 0x004d2658
const float kF_004d2670 = 0.0;  // CONFIRMED-BINARY: float 0x00000000 at 0x004d2670
const float kF_004d2674 = 1.0;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d2674
const double kD_004d2678 = 0.0;  // CONFIRMED-BINARY: double 0x0000000000000000 at 0x004d2678
}  // namespace

// Triangulation output and plane state (original .bss, zero at load: CONFIRMED-DATA, no file backing).
// 0x0053a750: triangles as vertex-index triples; 0x0053d750: count. Capacity 1024 INFERRED from the layout
// (0x0053a750 + 1024 * 12 = 0x0053d750, where the count lives); the original never checks it.
std::uint32_t g_TriangleList_0053a750[3 * 1024];
std::uint32_t g_TriangleCount_0053d750;
// 0x0053d758..0x0053d764: plane a, b, c, d written by 0x0046c390 and read by Plane_SolveZForPoints.
float g_NewellPlane_0053d758[4];

// 0x0046a8e0 Triangle_SolveGradientXZ (ECX p1, EDX p0, stack p2, v1, v0, v2, out; ret 0x14): gradient of v over x/z; det 0 -> (0, 0).
__declspec(naked) void __fastcall Triangle_SolveGradientXZ(const float*, const float*, const float*, float, float, float, float*)
{
    __asm {
        sub esp, 0x18
        mov eax, dword ptr [esp + 0x1c]
        fld dword ptr [eax + 0x8]
        fld dword ptr [eax]
        fsub dword ptr [edx]
        fld dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fxch st(3)
        fsub dword ptr [edx + 0x8]
        fxch st(1)
        fsub dword ptr [edx]
        fxch st(3)
        fsub dword ptr [edx + 0x8]
        fld st(2)
        fld st(2)
        fxch st(5)
        fstp dword ptr [esp]
        fxch st(4)
        fmul dword ptr [esp]
        fxch st(4)
        fmul st(0), st(1)
        fld dword ptr [esp + 0x20]
        fld dword ptr [esp + 0x28]
        fxch st(2)
        fsubp st(6), st(0)
        fsub dword ptr [esp + 0x24]
        fxch st(1)
        fsub dword ptr [esp + 0x24]
        fxch st(5)
        fst dword ptr [esp + 0x24]
        fcomp qword ptr [kD_004d2658]
        fstp dword ptr [esp + 0x8]
        fxch st(3)
        fstp dword ptr [esp + 0x14]
        fnstsw AX
        fxch st(1)
        fxch st(2)
        test AH, 0x40
        jnz L_46a99b
        fld dword ptr [kF_004d264c]
        fdiv dword ptr [esp + 0x24]
        mov eax, dword ptr [esp + 0x2c]
        fld dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x14]
        fmul st(0), st(3)
        fxch st(2)
        fxch st(5)
        fxch st(4)
        fmul dword ptr [esp + 0x8]
        fxch st(4)
        fmul dword ptr [esp + 0x8]
        fxch st(4)
        fsubp st(2), st(0)
        fmul dword ptr [esp]
        fsubp st(3), st(0)
        fmul st(0), st(3)
        fxch st(2)
        fmul st(0), st(3)
        fxch st(2)
        fchs
        fstp dword ptr [eax]
        fxch st(1)
        fchs
        fstp dword ptr [eax + 0x4]
        fstp st(0)
        fstp st(0)
        add esp, 0x18
        ret 0x14
    L_46a99b:
        mov eax, dword ptr [esp + 0x2c]
        fstp st(0)
        fstp st(0)
        fstp st(0)
        mov dword ptr [eax], 0x0
        mov dword ptr [eax + 0x4], 0x0
        add esp, 0x18
        ret 0x14
    }
}

// 0x0046ab40 PointArray_Find2D (ECX array: points +4, count +8; EDX point) -> first index within 0.01, else -1.
__declspec(naked) int __fastcall PointArray_Find2D(const void*, const float*)
{
    __asm {
        push ebx
        mov ebx, ecx
        push ebp
        push esi
        mov eax, dword ptr [ebx + 0x8]
        mov esi, dword ptr [ebx + 0x4]
        push edi
        xor edi, edi
        test eax, eax
        mov ebp, edx
        jle L_46ab80
    L_46ab54:
        push 0x3c23d70a
        mov edx, ebp
        mov ecx, esi
        call Point2_NearlyEqual
        test eax, eax
        jnz L_46ab79
        mov eax, dword ptr [ebx + 0x8]
        add esi, 0xc
        inc edi
        cmp edi, eax
        jl L_46ab54
        or eax, 0xffffffff
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    L_46ab79:
        mov eax, edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    L_46ab80:
        pop edi
        pop esi
        pop ebp
        or eax, 0xffffffff
        pop ebx
        ret
    }
}

// 0x0046be20 Segment2_IntersectStrict (ECX a0, EDX a1, stack b0, b1; ret 8): 1 only when both parameters are strictly in (0, 1).
__declspec(naked) int __fastcall Segment2_IntersectStrict(const float*, const float*, const float*, const float*)
{
    __asm {
        sub esp, 0x10
        fld dword ptr [edx + 0x4]
        push esi
        mov esi, dword ptr [esp + 0x18]
        mov eax, dword ptr [esp + 0x1c]
        fld dword ptr [esi + 0x4]
        fld dword ptr [edx]
        fld dword ptr [esi]
        fxch st(2)
        fsub dword ptr [eax + 0x4]
        fxch st(1)
        fsub dword ptr [ecx]
        fxch st(2)
        fsub dword ptr [eax]
        fxch st(3)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fst dword ptr [esp + 0x1c]
        fxch st(2)
        fstp dword ptr [esp + 0xc]
        fst dword ptr [esp + 0x10]
        fxch st(1)
        fmul dword ptr [esp + 0xc]
        fxch st(2)
        fstp dword ptr [esp + 0x4]
        fmul dword ptr [esp + 0x4]
        fsubp st(1), st(0)
        fst dword ptr [esp + 0x18]
        fcomp dword ptr [kF_004d2670]
        fnstsw AX
        test AH, 0x40
        jnz L_46bf19
        fld dword ptr [esi + 0x4]
        fld dword ptr [esi]
        fsub dword ptr [ecx]
        fxch st(1)
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fst dword ptr [esp + 0x8]
        fld st(1)
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        fxch st(1)
        fmul dword ptr [esp + 0x4]
        fld dword ptr [esp + 0x8]
        fxch st(3)
        fmul dword ptr [esp + 0xc]
        fxch st(1)
        fsubp st(2), st(0)
        fxch st(2)
        fmul dword ptr [esp + 0x10]
        fxch st(1)
        fdiv dword ptr [esp + 0x18]
        fxch st(1)
        fsubp st(2), st(0)
        fst dword ptr [esp + 0x1c]
        fxch st(1)
        fdiv dword ptr [esp + 0x18]
        fxch st(1)
        fcomp dword ptr [kF_004d2670]
        fstp dword ptr [esp + 0x18]
        fnstsw AX
        test AH, 0x41
        jnz L_46bf19
        fld dword ptr [esp + 0x18]
        fcomp dword ptr [kF_004d2670]
        fnstsw AX
        test AH, 0x41
        jnz L_46bf19
        fld dword ptr [esp + 0x1c]
        fcomp dword ptr [kF_004d2674]
        fnstsw AX
        test AH, 0x1
        jz L_46bf19
        fld dword ptr [esp + 0x18]
        fcomp dword ptr [kF_004d2674]
        fnstsw AX
        test AH, 0x1
        jz L_46bf19
        mov eax, 0x1
        pop esi
        add esp, 0x10
        ret 0x8
    L_46bf19:
        xor eax, eax
        pop esi
        add esp, 0x10
        ret 0x8
    }
}

// 0x0046bfc0 Triangulate_EmitEar (ECX edge a, EDX edge b, stack apex, edge count, edges; ret 0xc): appends a triangle to
// the global list (capacity 1024, unchecked as in the original) and decrements three edge use counts -> 0.
__declspec(naked) int __fastcall Triangulate_EmitEar(int, int, int, int, void*)
{
    __asm {
        mov eax, [g_TriangleCount_0053d750]
        push ebx
        push ebp
        lea ecx, [ecx + ecx*0x2]
        lea eax, [eax + eax*0x2]
        push esi
        lea edx, [edx + edx*0x2]
        push edi
        lea ebp, [eax*0x4 + g_TriangleList_0053a750]
        mov eax, dword ptr [esp + 0x1c]
        lea esi, [eax + ecx*0x4]
        lea edi, [eax + edx*0x4]
        mov ecx, dword ptr [esi + 0x8]
        test ecx, ecx
        jz L_46c061
        mov ecx, dword ptr [edi + 0x8]
        test ecx, ecx
        jz L_46c061
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [esp + 0x14]
        cmp ecx, edx
        jnz L_46c004
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [esp + 0x1c], ecx
        jmp L_46c008
    L_46c004:
        mov dword ptr [esp + 0x1c], ecx
    L_46c008:
        mov ebx, dword ptr [edi]
        cmp ebx, edx
        jnz L_46c011
        mov ebx, dword ptr [edi + 0x4]
    L_46c011:
        lea edx, [ebx + ecx*0x1]
        add edx, dword ptr [esp + 0x14]
        cmp edx, 0x3
        jz L_46c061
        push eax
        mov eax, dword ptr [esp + 0x1c]
        push eax
        mov edx, ebx
        call EdgeList_FindEdge
        test eax, eax
        jz L_46c061
        mov ecx, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x14]
        mov dword ptr [ebp], ecx
        mov dword ptr [ebp + 0x4], ebx
        mov dword ptr [ebp + 0x8], edx
        mov ebp, dword ptr [g_TriangleCount_0053d750]
        inc ebp
        mov dword ptr [g_TriangleCount_0053d750], ebp
        mov ebx, dword ptr [esi + 0x8]
        dec ebx
        mov dword ptr [esi + 0x8], ebx
        mov edx, dword ptr [edi + 0x8]
        dec edx
        mov dword ptr [edi + 0x8], edx
        mov ecx, dword ptr [eax + 0x8]
        dec ecx
        mov dword ptr [eax + 0x8], ecx
    L_46c061:
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        ret 0xc
    }
}

// 0x0046c3a0 Polygon_NewellPlaneFastSqrt (ECX count, EDX xyz, stack plane out a,b,c,d; ret 4): Newell normal scaled by
// the fast-sqrt bit trick; a zero normal gives scale 0 and d divided by 0.
__declspec(naked) void __fastcall Polygon_NewellPlaneFastSqrt(int, const float*, float*)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x28
        fld dword ptr [kF_004d2670]
        push ebx
        xor eax, eax
        fst dword ptr [ebp - 0x14]
        fld dword ptr [kF_004d2670]
        push esi
        push edi
        fst dword ptr [ebp - 0x18]
        fld dword ptr [kF_004d2670]
        test ecx, ecx
        fst dword ptr [ebp - 0x1c]
        fld dword ptr [kF_004d2670]
        mov ebx, edx
        mov dword ptr [ebp - 0x10], ecx
        fst dword ptr [ebp - 0x20]
        fld dword ptr [kF_004d2670]
        fst dword ptr [ebp - 0x24]
        fld dword ptr [kF_004d2670]
        fst dword ptr [ebp - 0x28]
        jle L_46c55a
        lea esi, [ebx + 0x8]
    L_46c3f1:
        lea edi, [eax + 0x1]
        add esi, 0xc
        mov eax, edi
        cdq
        idiv ecx
        lea eax, [edx + edx*0x2]
        fld dword ptr [ebx + eax*0x4 + 0x8]
        fadd dword ptr [esi - 0xc]
        fld dword ptr [esi - 0x10]
        lea eax, [ebx + eax*0x4]
        fsub dword ptr [eax + 0x4]
        fmulp st(1), st(0)
        faddp st(4), st(0)
        fld dword ptr [eax]
        fadd dword ptr [esi - 0x14]
        fld dword ptr [esi - 0xc]
        fsub dword ptr [eax + 0x8]
        fmulp st(1), st(0)
        faddp st(5), st(0)
        fld dword ptr [esi - 0x14]
        fsub dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fadd dword ptr [esi - 0x10]
        mov eax, edi
        fmulp st(1), st(0)
        cmp eax, ecx
        faddp st(6), st(0)
        fadd dword ptr [esi - 0x14]
        fxch st(1)
        fadd dword ptr [esi - 0x10]
        fxch st(1)
        fxch st(2)
        fadd dword ptr [esi - 0xc]
        fxch st(2)
        jl L_46c3f1
        fst dword ptr [ebp - 0x28]
        fxch st(5)
        fst dword ptr [ebp - 0x14]
        fxch st(4)
        fst dword ptr [ebp - 0x18]
        fxch st(3)
        fst dword ptr [ebp - 0x1c]
        fxch st(5)
        fstp st(0)
        fstp dword ptr [ebp - 0x24]
        fstp dword ptr [ebp - 0x20]
        fxch st(1)
        fxch st(2)
    L_46c468:
        fcom qword ptr [kD_004d2678]
        fnstsw AX
        test AH, 0x40
        jz L_46c49c
        fld st(1)
        fcomp qword ptr [kD_004d2678]
        fnstsw AX
        test AH, 0x40
        jz L_46c49c
        fld st(2)
        fcomp qword ptr [kD_004d2678]
        fnstsw AX
        test AH, 0x40
        jz L_46c49c
        mov dword ptr [ebp - 0x8], 0x0
        jmp L_46c4d1
    L_46c49c:
        fld st(0)
        fmul st(0), st(1)
        fld st(2)
        fmul st(0), st(3)
        faddp st(1), st(0)
        fld st(3)
        fmul st(0), st(4)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x8]
        fstp st(0)
        fstp st(0)
        fstp st(0)
        mov eax, dword ptr [ebp - 0x8]
        sar eax, 0x1
        add eax, 0x1fc00000
        mov dword ptr [ebp - 0x4], eax
        fld dword ptr [ebp - 0x14]
        mov ecx, dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x18]
        fld dword ptr [ebp - 0x1c]
        mov dword ptr [ebp - 0x8], ecx
    L_46c4d1:
        fld dword ptr [ebp - 0x8]
        fcomp qword ptr [kD_004d2678]
        fnstsw AX
        test AH, 0x40
        jnz L_46c4ef
        fld dword ptr [kF_004d2674]
        fdiv dword ptr [ebp - 0x8]
        fstp dword ptr [ebp - 0x4]
        jmp L_46c4f6
    L_46c4ef:
        mov dword ptr [ebp - 0x4], 0x0
    L_46c4f6:
        fld dword ptr [ebp - 0x4]
        mov eax, dword ptr [ebp + 0x8]
        lea edx, [ebp - 0x1c]
        fmul st(0), st(1)
        lea ecx, [ebp - 0x28]
        mov dword ptr [ebp + 0x8], edx
        fstp dword ptr [eax]
        fstp st(0)
        fld dword ptr [ebp - 0x4]
        fmul st(0), st(1)
        fstp dword ptr [eax + 0x4]
        fstp st(0)
        fld dword ptr [ebp - 0x4]
        fmul st(0), st(1)
        mov dword ptr [ebp - 0x4], ecx
        fstp dword ptr [eax + 0x8]
        fstp st(0)
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ebp + 0x8]
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
        fild dword ptr [ebp - 0x10]
        pop edi
        pop esi
        fmul dword ptr [ebp - 0x8]
        pop ebx
        fdivp st(1), st(0)
        fchs
        fstp dword ptr [eax + 0xc]
        mov esp, ebp
        pop ebp
        ret 0x4
    L_46c55a:
        fstp st(0)
        fstp st(0)
        fstp st(0)
        jmp L_46c468
    }
}

// 0x0046c390 (ledger Geometry_Call46c3a0_Global53d758): Polygon_NewellPlaneFastSqrt into the global plane 0x0053d758.
__declspec(naked) void __fastcall Polygon_NewellPlaneToGlobal(int, const float*)
{
    __asm {
        push offset g_NewellPlane_0053d758
        call Polygon_NewellPlaneFastSqrt
        ret
    }
}

// 0x0046c570 Plane_SolveZForPoints (ECX count, EDX xyz): z = -(a*x + b*y + d) / c with the global plane (no zero check on c).
__declspec(naked) void __fastcall Plane_SolveZForPoints(int, float*)
{
    __asm {
        test ecx, ecx
        jle L_46c5a4
        lea eax, [edx + 0x8]
    L_46c577:
        fld dword ptr [eax - 0x4]
        fld dword ptr [eax - 0x8]
        fmul dword ptr [g_NewellPlane_0053d758]
        fxch st(1)
        fmul dword ptr [g_NewellPlane_0053d758 + 4]
        add eax, 0xc
        dec ecx
        faddp st(1), st(0)
        fadd dword ptr [g_NewellPlane_0053d758 + 0xc]
        fdiv dword ptr [g_NewellPlane_0053d758 + 8]
        fchs
        fstp dword ptr [eax - 0xc]
        jnz L_46c577
    L_46c5a4:
        ret
    }
}

// 0x0046c620 Triangle_EnsureCounterClockwise (ECX unused, EDX xyz of 3 vertices, stack fix; ret 4): 2D cross <= 0 -> fix ?
// reverse winding and 1 : 0; positive -> 1.
__declspec(naked) int __fastcall Triangle_EnsureCounterClockwise(int, float*, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x30
        push ebx
        push esi
        mov esi, edx
        lea eax, [ebp - 0x18]
        mov dword ptr [ebp - 0x4], eax
        push edi
        lea eax, [esi + 0xc]
        mov dword ptr [ebp - 0x8], esi
        mov edi, ecx
        mov dword ptr [ebp - 0xc], eax
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
        lea edx, [esi + 0x18]
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
        lea eax, [ebp - 0x30]
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
        fld dword ptr [ebp - 0x28]
        fcomp dword ptr [kF_004d2670]
        fnstsw AX
        test AH, 0x41
        jz L_46c70f
        mov eax, dword ptr [ebp + 0x8]
        test eax, eax
        jnz L_46c706
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_46c706:
        mov edx, esi
        mov ecx, edi
        call Vec3Array_ReverseKeepFirst
    L_46c70f:
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

namespace {
const double kD_004d2660 = 0.009999999776482582;  // CONFIRMED-BINARY: double 0x3f847ae140000000 at 0x004d2660
const float kF_004d2668 = 0.0;  // CONFIRMED-BINARY: float 0x00000000 at 0x004d2668
}  // namespace

// 0x0053a748: the triangulator's vertex array (pointer, set by Triangulate_RingPair); 0x0053d754: a word written
// by Triangulate_RingPair next to the triangle count. Both .bss (zero at load, CONFIRMED-DATA).
void* g_TriangulateVertices_0053a748;
std::uint32_t g_TriangulateWord_0053d754;

// 0x0046aa40 ClipPolygon_Create - malloc(0x1C) zeroed (unchecked); points = malloc(count ECX *0xC) copy of EDX; Vec3Array_RotateX90Inverse (arguments not visible); count +8; 0x0046a9c0(count) bounds; return polygon
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipPolygon_Create(int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov esi, edx
        mov ebp, ecx
        push 0x1c
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov ebx, eax
        mov ecx, 0x7
        xor eax, eax
        mov edi, ebx
        rep stosd
        lea edi, [ebp + ebp*0x2]
        add esp, 0x4
        shl edi, 0x2
        push edi
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov ecx, edi
        mov dword ptr [ebx + 0x4], eax
        mov edi, eax
        mov eax, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        add esp, 0x4
        and ecx, 0x3
        rep movsb
        mov edx, dword ptr [ebx + 0x4]
        mov ecx, ebp
        call Vec3Array_RotateX90Inverse
        mov edx, dword ptr [ebx + 0x4]
        lea ecx, [ebx + 0xc]
        push ebp
        mov dword ptr [ebx + 0x8], ebp
        call Rect_FromPoints2D
        mov eax, ebx
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}

// 0x0046ac80 ClipPolygon_FindEdgeContaining - (polygon ECX: points +4, count +8; point EDX): for each edge i -> i+1 mod n: |dx| and |dy| >= 0.01: parameters along x and y agree within 0.01, both strictly in (0,1), and the reconstructed point within 0.01 -> i; |dx| >= 0.01, |dy| < 0.01 (horizontal): point y within 0.01 and x parameter in (0,1) -> i; |dx| < 0.01 (vertical): point x within 0.01 and y parameter in (0,1) -> i. None -> -1
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipPolygon_FindEdgeContaining(int, int)
{
    __asm {
        sub esp, 0x14
        push ebx
        push ebp
        mov ebp, dword ptr [ecx + 0x4]
        mov ecx, dword ptr [ecx + 0x8]
        push esi
        push edi
        xor edi, edi
        mov ebx, edx
        test ecx, ecx
        mov dword ptr [esp + 0x20], ecx
        jle L_46ae34
        mov ecx, ebp
    L_46ac9f:
        lea esi, [edi + 0x1]
        mov eax, esi
        cdq
        idiv dword ptr [esp + 0x20]
        lea eax, [edx + edx*0x2]
        fld dword ptr [ebp + eax*0x4]
        fsub dword ptr [ecx]
        lea eax, [ebp + eax*0x4]
        fstp dword ptr [esp + 0x14]
        fld dword ptr [eax + 0x4]
        fsub dword ptr [ecx + 0x4]
        fstp dword ptr [esp + 0x1c]
        fld dword ptr [esp + 0x14]
        fabs
        fcomp qword ptr [kD_004d2660]
        fnstsw AX
        test AH, 0x1
        jz L_46acfa
        fld dword ptr [ecx]
        fsub dword ptr [ebx]
        fabs
        fcomp qword ptr [kD_004d2660]
        fnstsw AX
        test AH, 0x41
        jz L_46ae0e
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fdiv dword ptr [esp + 0x1c]
        jmp L_46ad2e
    L_46acfa:
        fld dword ptr [esp + 0x1c]
        fabs
        fcomp qword ptr [kD_004d2660]
        fnstsw AX
        test AH, 0x1
        jz L_46ad55
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fabs
        fcomp qword ptr [kD_004d2660]
        fnstsw AX
        test AH, 0x41
        jz L_46ae0e
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fdiv dword ptr [esp + 0x14]
    L_46ad2e:
        fcom dword ptr [kF_004d2668]
        fnstsw AX
        test AH, 0x41
        jnz L_46ae0c
        fcomp dword ptr [kF_004d264c]
        fnstsw AX
        test AH, 0x1
        jnz L_46ae2a
        jmp L_46ae0e
    L_46ad55:
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fld dword ptr [ebx + 0x4]
        fsub dword ptr [ecx + 0x4]
        fxch st(1)
        fdiv dword ptr [esp + 0x14]
        fxch st(1)
        fdiv dword ptr [esp + 0x1c]
        fxch st(1)
        fst dword ptr [esp + 0x10]
        fxch st(1)
        fstp dword ptr [esp + 0x18]
        fsub dword ptr [esp + 0x18]
        fabs
        fcomp qword ptr [kD_004d2660]
        fnstsw AX
        test AH, 0x1
        jz L_46ae0e
        fld dword ptr [esp + 0x18]
        fcomp dword ptr [kF_004d2668]
        fnstsw AX
        test AH, 0x41
        jnz L_46ae0e
        fld dword ptr [esp + 0x18]
        fcomp dword ptr [kF_004d264c]
        fnstsw AX
        test AH, 0x1
        jz L_46ae0e
        fld dword ptr [esp + 0x10]
        fcomp dword ptr [kF_004d2668]
        fnstsw AX
        test AH, 0x41
        jnz L_46ae0e
        fld dword ptr [esp + 0x10]
        fcomp dword ptr [kF_004d264c]
        fnstsw AX
        test AH, 0x1
        jz L_46ae0e
        fld dword ptr [esp + 0x10]
        fmul dword ptr [esp + 0x14]
        fadd dword ptr [ecx]
        fsub dword ptr [ebx]
        fabs
        fcomp qword ptr [kD_004d2660]
        fnstsw AX
        test AH, 0x41
        jz L_46ae0e
        fld dword ptr [esp + 0x18]
        fmul dword ptr [esp + 0x1c]
        fadd dword ptr [ecx + 0x4]
        fsub dword ptr [ebx + 0x4]
        fabs
        fcomp qword ptr [kD_004d2660]
        fnstsw AX
        test AH, 0x41
        jnz L_46ae2a
        jmp L_46ae0e
    L_46ae0c:
        fstp st(0)
    L_46ae0e:
        mov eax, dword ptr [esp + 0x20]
        mov edi, esi
        add ecx, 0xc
        cmp edi, eax
        jl L_46ac9f
        or eax, 0xffffffff
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x14
        ret
    L_46ae2a:
        mov eax, edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x14
        ret
    L_46ae34:
        pop edi
        pop esi
        pop ebp
        or eax, 0xffffffff
        pop ebx
        add esp, 0x14
        ret
    }
}

// 0x0046ab90 ClipPolygon_MergePoints - (polygon ECX, points arg, count EDX): per point: existing match (PointArray_Find2D) -> overwrite with the point; else index = 0x0046ac80 (edge containing it); found -> realloc (unchecked) and insert after that index. Returns 1 if any point was placed
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipPolygon_MergePoints(int, int, int)
{
    __asm {
        push ebp
        xor ebp, ebp
        push esi
        mov esi, ecx
        test edx, edx
        jle L_46ac71
        push edi
        push ebx
        mov ebx, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x14], edx
    L_46aba8:
        mov edx, ebx
        mov ecx, esi
        call PointArray_Find2D
        cmp eax, -0x1
        jz L_46abd6
        mov ecx, dword ptr [esi + 0x4]
        lea eax, [eax + eax*0x2]
        lea edx, [ecx + eax*0x4]
        mov eax, ebx
        mov ecx, dword ptr [eax]
        mov dword ptr [edx], ecx
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [edx + 0x4], ecx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [edx + 0x8], eax
        jmp L_46ac58
    L_46abd6:
        mov edx, ebx
        mov ecx, esi
        call ClipPolygon_FindEdgeContaining
        mov edi, eax
        cmp edi, -0x1
        jz L_46ac5d
        mov eax, dword ptr [esi + 0x8]
        mov edx, dword ptr [esi + 0x4]
        inc eax
        lea ecx, [eax + eax*0x2]
        shl ecx, 0x2
        push ecx
        push edx
        call dword ptr [g_Iat_realloc_004cc4ec]
        mov edx, dword ptr [esi + 0x8]
        add esp, 0x8
        mov dword ptr [esi + 0x4], eax
        lea ecx, [edx - 0x1]
        cmp edi, ecx
        jz L_46ac33
        lea ecx, [edi + 0x1]
        lea edx, [edx + edx*0x2]
        shl edx, 0x2
        lea ecx, [ecx + ecx*0x2]
        shl ecx, 0x2
        sub edx, ecx
        add ecx, eax
        push edx
        push ecx
        lea ecx, [edi + 0x2]
        lea ecx, [ecx + ecx*0x2]
        lea edx, [eax + ecx*0x4]
        push edx
        call dword ptr [g_Iat_memmove_004cc4e8]
        add esp, 0xc
    L_46ac33:
        mov ecx, dword ptr [esi + 0x4]
        lea eax, [edi + 0x1]
        lea eax, [eax + eax*0x2]
        lea edx, [ecx + eax*0x4]
        mov eax, ebx
        mov ecx, dword ptr [eax]
        mov dword ptr [edx], ecx
        mov ecx, dword ptr [eax + 0x4]
        mov dword ptr [edx + 0x4], ecx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [edx + 0x8], eax
        mov eax, dword ptr [esi + 0x8]
        inc eax
        mov dword ptr [esi + 0x8], eax
    L_46ac58:
        mov ebp, 0x1
    L_46ac5d:
        mov eax, dword ptr [esp + 0x14]
        add ebx, 0xc
        dec eax
        mov dword ptr [esp + 0x14], eax
        jnz L_46aba8
        pop ebx
        pop edi
    L_46ac71:
        mov eax, ebp
        pop esi
        pop ebp
        ret 0x4
    }
}

// 0x0046bd50 Triangulate_TryAddEdge - (candidate edge ECX {a,b,..}, edge count EDX, edges arg): if EdgeList_FindEdge already has it (arguments not visible) -> count unchanged; else for every edge sharing no endpoint with the candidate, 0x0046be20(vertex a, vertex b) from the global vertex array [0x0053a748] (operands partly hidden) nonzero -> unchanged; otherwise appends the 3-dword edge and returns count+1
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall Triangulate_TryAddEdge(int, int, int)
{
    __asm {
        sub esp, 0x8
        push ebx
        push ebp
        mov eax, [g_TriangulateVertices_0053a748]
        push esi
        mov esi, ecx
        mov ebx, edx
        push edi
        mov ecx, dword ptr [esi]
        lea edx, [ecx + ecx*0x2]
        lea ebp, [eax + edx*0x4]
        mov edx, dword ptr [esi + 0x4]
        lea edi, [edx + edx*0x2]
        lea eax, [eax + edi*0x4]
        mov edi, dword ptr [esp + 0x1c]
        push edi
        push ebx
        mov dword ptr [esp + 0x1c], eax
        call EdgeList_FindEdge
        test eax, eax
        jz L_46bd90
        mov eax, ebx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x4
    L_46bd90:
        test ebx, ebx
        mov dword ptr [esp + 0x10], 0x0
        jle L_46bde9
    L_46bd9c:
        mov eax, dword ptr [edi]
        mov edx, dword ptr [esi]
        cmp eax, edx
        jz L_46bdd9
        mov ecx, dword ptr [edi + 0x4]
        cmp ecx, edx
        jz L_46bdd9
        mov edx, dword ptr [esi + 0x4]
        cmp eax, edx
        jz L_46bdd9
        cmp ecx, edx
        jz L_46bdd9
        lea edx, [ecx + ecx*0x2]
        mov ecx, dword ptr [g_TriangulateVertices_0053a748]
        lea eax, [eax + eax*0x2]
        lea edx, [ecx + edx*0x4]
        lea ecx, [ecx + eax*0x4]
        push edx
        mov edx, dword ptr [esp + 0x18]
        push ecx
        mov ecx, ebp
        call Segment2_IntersectStrict
        test eax, eax
        jnz L_46be10
    L_46bdd9:
        mov eax, dword ptr [esp + 0x10]
        add edi, 0xc
        inc eax
        cmp eax, ebx
        mov dword ptr [esp + 0x10], eax
        jl L_46bd9c
    L_46bde9:
        mov eax, dword ptr [esp + 0x1c]
        lea edx, [ebx + ebx*0x2]
        lea ecx, [eax + edx*0x4]
        mov edx, dword ptr [esi]
        mov dword ptr [ecx], edx
        mov eax, dword ptr [esi + 0x4]
        mov dword ptr [ecx + 0x4], eax
        mov edx, dword ptr [esi + 0x8]
        lea eax, [ebx + 0x1]
        mov dword ptr [ecx + 0x8], edx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x4
    L_46be10:
        pop edi
        pop esi
        mov eax, ebx
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x4
    }
}

// 0x0046c070 Triangulate_RingPair - (outer count ECX, outer xyz EDX, inner count arg1, inner xyz arg2): total = n+m; triangle count [0x0053d750] = 0; edge buffer malloc(total^2 * 0xC) and vertex array [0x0053a748] = malloc(total*0xC) (unchecked); copies outer then inner vertices; plane = Polygon_NewellPlaneFastSqrt into [0x0053d758] (via 0x0046c390) and Plane_SolveZForPoints re-projects all vertices, copying the inner ones back into
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall Triangulate_RingPair(int, int, int, int)
{
    __asm {
        sub esp, 0x9c
        mov eax, dword ptr [esp + 0xa0]
        push ebx
        mov ebx, ecx
        push ebp
        push esi
        push edi
        lea ebp, [eax + ebx*0x1]
        mov edi, dword ptr [g_Iat_malloc_004cc5dc]
        mov eax, ebp
        mov esi, edx
        imul eax, ebp
        mov dword ptr [esp + 0x1c], esi
        mov dword ptr [g_TriangleCount_0053d750], 0x0
        lea ecx, [eax + eax*0x2]
        mov dword ptr [g_TriangulateWord_0053d754], ebp
        shl ecx, 0x2
        push ecx
        call edi
        mov dword ptr [esp + 0x1c], eax
        mov eax, [g_TriangulateWord_0053d754]
        add esp, 0x4
        lea edx, [eax + eax*0x2]
        shl edx, 0x2
        push edx
        call edi
        lea edx, [ebx + ebx*0x2]
        mov [g_TriangulateVertices_0053a748], eax
        shl edx, 0x2
        mov ecx, edx
        mov edi, eax
        mov eax, ecx
        mov dword ptr [esp + 0x14], edx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        mov eax, dword ptr [esp + 0xb4]
        and ecx, 0x3
        add esp, 0x4
        rep movsb
        mov esi, dword ptr [esp + 0xb4]
        lea ecx, [eax + eax*0x2]
        mov eax, [g_TriangulateVertices_0053a748]
        shl ecx, 0x2
        mov dword ptr [esp + 0x14], ecx
        lea edi, [eax + edx*0x1]
        mov edx, ecx
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        mov edx, dword ptr [esp + 0x1c]
        and ecx, 0x3
        rep movsb
        mov ecx, ebx
        call Polygon_NewellPlaneToGlobal
        mov eax, [g_TriangulateVertices_0053a748]
        mov esi, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0xb0]
        lea edx, [eax + esi*0x1]
        call Plane_SolveZForPoints
        mov ecx, dword ptr [esp + 0x14]
        mov edx, dword ptr [g_TriangulateVertices_0053a748]
        mov edi, dword ptr [esp + 0xb4]
        mov eax, ecx
        add esi, edx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        mov edi, dword ptr [esp + 0x18]
        lea ecx, [ebx - 0x1]
        mov esi, 0x1
        mov dword ptr [edi + 0x4], ecx
        cmp ebx, esi
        mov dword ptr [edi], 0x0
        mov dword ptr [edi + 0x8], esi
        lea eax, [edi + 0xc]
        mov ecx, esi
        jle L_46c18f
    L_46c17c:
        lea edx, [ecx - 0x1]
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [eax], edx
        mov dword ptr [eax + 0x8], esi
        add eax, 0xc
        inc ecx
        cmp ecx, ebx
        jl L_46c17c
    L_46c18f:
        mov edx, dword ptr [esp + 0xb0]
        lea ecx, [ebp - 0x1]
        mov dword ptr [eax], ebx
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [eax + 0x8], esi
        add eax, 0xc
        cmp edx, esi
        jle L_46c1ca
        dec edx
        lea ecx, [ebx + 0x1]
        mov dword ptr [esp + 0x10], edx
    L_46c1b0:
        lea edx, [ecx - 0x1]
        mov dword ptr [eax + 0x4], ecx
        mov dword ptr [eax], edx
        mov edx, dword ptr [esp + 0x10]
        mov dword ptr [eax + 0x8], esi
        add eax, 0xc
        inc ecx
        dec edx
        mov dword ptr [esp + 0x10], edx
        jnz L_46c1b0
    L_46c1ca:
        xor eax, eax
        mov dword ptr [esp + 0x28], 0x2
        test ebx, ebx
        mov dword ptr [esp + 0x14], eax
        jle L_46c219
    L_46c1dc:
        mov ecx, dword ptr [esp + 0xb0]
        xor esi, esi
        test ecx, ecx
        mov dword ptr [esp + 0x20], eax
        jle L_46c210
    L_46c1ed:
        lea eax, [esi + ebx*0x1]
        push edi
        mov edx, ebp
        lea ecx, [esp + 0x24]
        mov dword ptr [esp + 0x28], eax
        call Triangulate_TryAddEdge
        mov ebp, eax
        inc esi
        cmp esi, dword ptr [esp + 0xb0]
        jl L_46c1ed
        mov eax, dword ptr [esp + 0x14]
    L_46c210:
        inc eax
        cmp eax, ebx
        mov dword ptr [esp + 0x14], eax
        jl L_46c1dc
    L_46c219:
        mov eax, [g_TriangulateWord_0053d754]
        xor esi, esi
        test eax, eax
        mov dword ptr [esp + 0x14], esi
        jle L_46c2a1
    L_46c228:
        lea ecx, [esp + 0x2c]
        mov edx, ebp
        push ecx
        push edi
        mov ecx, esi
        call EdgeList_FindUsingVertex
        xor ecx, ecx
        test eax, eax
        mov dword ptr [esp + 0x18], eax
        mov dword ptr [esp + 0x1c], ecx
        jle L_46c293
        lea edx, [esp + 0x2c]
        mov dword ptr [esp + 0x10], edx
    L_46c24d:
        xor ebx, ebx
        lea esi, [esp + 0x2c]
    L_46c253:
        cmp ecx, ebx
        jz L_46c273
        mov ecx, dword ptr [esp + 0x10]
        mov eax, dword ptr [esp + 0x14]
        mov edx, dword ptr [esi]
        push edi
        mov ecx, dword ptr [ecx]
        push ebp
        push eax
        call Triangulate_EmitEar
        mov ecx, dword ptr [esp + 0x1c]
        mov eax, dword ptr [esp + 0x18]
    L_46c273:
        inc ebx
        add esi, 0x4
        cmp ebx, eax
        jl L_46c253
        mov esi, dword ptr [esp + 0x10]
        inc ecx
        add esi, 0x4
        cmp ecx, eax
        mov dword ptr [esp + 0x1c], ecx
        mov dword ptr [esp + 0x10], esi
        jl L_46c24d
        mov esi, dword ptr [esp + 0x14]
    L_46c293:
        mov eax, [g_TriangulateWord_0053d754]
        inc esi
        cmp esi, eax
        mov dword ptr [esp + 0x14], esi
        jl L_46c228
    L_46c2a1:
        mov eax, [g_TriangleCount_0053d750]
        lea edx, [eax + eax*0x8]
        lea eax, [edx*0x4 + 0x4]
        push eax
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov ecx, dword ptr [g_TriangleCount_0053d750]
        mov ebx, eax
        add esp, 0x4
        xor esi, esi
        mov dword ptr [ebx], ecx
        mov ecx, dword ptr [g_TriangleCount_0053d750]
        test ecx, ecx
        mov dword ptr [esp + 0x18], ebx
        lea eax, [ebx + 0x4]
        jle L_46c35e
        mov ecx, offset g_TriangleList_0053a750 + 8
    L_46c2e0:
        mov edx, dword ptr [ecx - 0x8]
        mov ebx, dword ptr [g_TriangulateVertices_0053a748]
        add ecx, 0xc
        lea edx, [edx + edx*0x2]
        lea edx, [ebx + edx*0x4]
        mov ebx, eax
        add eax, 0xc
        mov ebp, dword ptr [edx]
        mov dword ptr [ebx], ebp
        mov ebp, dword ptr [edx + 0x4]
        mov dword ptr [ebx + 0x4], ebp
        mov edx, dword ptr [edx + 0x8]
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [ecx - 0x10]
        mov ebx, dword ptr [g_TriangulateVertices_0053a748]
        lea edx, [edx + edx*0x2]
        lea edx, [ebx + edx*0x4]
        mov ebx, eax
        add eax, 0xc
        mov ebp, dword ptr [edx]
        mov dword ptr [ebx], ebp
        mov ebp, dword ptr [edx + 0x4]
        mov dword ptr [ebx + 0x4], ebp
        mov edx, dword ptr [edx + 0x8]
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [ecx - 0xc]
        mov ebx, dword ptr [g_TriangulateVertices_0053a748]
        lea edx, [edx + edx*0x2]
        lea edx, [ebx + edx*0x4]
        mov ebx, eax
        add eax, 0xc
        inc esi
        mov ebp, dword ptr [edx]
        mov dword ptr [ebx], ebp
        mov ebp, dword ptr [edx + 0x4]
        mov dword ptr [ebx + 0x4], ebp
        mov edx, dword ptr [edx + 0x8]
        mov dword ptr [ebx + 0x8], edx
        mov edx, dword ptr [g_TriangleCount_0053d750]
        cmp esi, edx
        jl L_46c2e0
        mov ebx, dword ptr [esp + 0x18]
    L_46c35e:
        mov esi, dword ptr [g_Iat_free_004cc5b4]
        push edi
        call esi
        mov eax, [g_TriangulateVertices_0053a748]
        add esp, 0x4
        push eax
        call esi
        add esp, 0x4
        mov eax, ebx
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x9c
        ret 0x8
    }
}

namespace {
const float kF_004d2648 = -1.0;  // CONFIRMED-BINARY: float 0xbf800000 at 0x004d2648
const char kStr_004e034c[] = "D:\\Proj\\GameZRecoil\\zGeometry\\zgeo_model.cpp";  // CONFIRMED-DATA: .data string at 0x004e034c
const char kStr_004e03b0[] = "Error getting linear buffer of polygon vertices";  // CONFIRMED-DATA: .data string at 0x004e03b0
const char kStr_004e03e0[] = "Skipping clip of polygon with (%d) verts";  // CONFIRMED-DATA: .data string at 0x004e03e0
const char kStr_004e04c0[] = "Intersection found, no polygons...";  // CONFIRMED-DATA: .data string at 0x004e04c0
const char kStr_004e04e4[] = "Weiler algorithm clip error occurred.";  // CONFIRMED-DATA: .data string at 0x004e04e4
}  // namespace

// 0x0046a7f0 ClipPolygon_ComputeUVs - (count ECX, xyz EDX, model arg1, parent polygon arg2): from the parent's first three vertices and uvs solves du and dv gradients over x/z (Triangle_SolveGradientXZ); malloc count*8 uvs (unchecked, pointer returned in EAX) with uv = uv0 + gradient . (p - p1) where p1 is the parent's second vertex
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipPolygon_ComputeUVs(int, int, int, int)
{
    __asm {
        sub esp, 0x14
        push ebx
        push ebp
        push esi
        push edi
        mov edi, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x10], edx
        mov ebx, ecx
        mov eax, dword ptr [edi + 0x8]
        mov ecx, dword ptr [esp + 0x28]
        mov edi, dword ptr [edi + 0x10]
        mov edx, dword ptr [eax]
        mov ecx, dword ptr [ecx + 0x34]
        lea edx, [edx + edx*0x2]
        lea ebp, [ecx + edx*0x4]
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [eax + 0x8]
        lea edx, [edx + edx*0x2]
        lea eax, [eax + eax*0x2]
        lea esi, [ecx + edx*0x4]
        lea edx, [esp + 0x14]
        lea ecx, [ecx + eax*0x4]
        mov eax, dword ptr [edi + 0x10]
        push edx
        mov edx, dword ptr [edi + 0x8]
        push eax
        mov eax, dword ptr [edi]
        push edx
        push eax
        mov dword ptr [esp + 0x3c], ecx
        push ecx
        mov edx, esi
        mov ecx, ebp
        call Triangle_SolveGradientXZ
        mov edx, dword ptr [edi + 0x14]
        mov eax, dword ptr [edi + 0xc]
        lea ecx, [esp + 0x1c]
        push ecx
        mov ecx, dword ptr [edi + 0x4]
        push edx
        mov edx, dword ptr [esp + 0x34]
        push eax
        push ecx
        push edx
        mov edx, esi
        mov ecx, ebp
        call Triangle_SolveGradientXZ
        lea eax, [ebx*0x8 + 0x0]
        push eax
        call dword ptr [g_Iat_malloc_004cc5dc]
        add esp, 0x4
        test ebx, ebx
        jle L_46a8ca
        mov ecx, dword ptr [esp + 0x10]
        mov edx, eax
    L_46a880:
        fld dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fsub dword ptr [esi + 0x8]
        fxch st(1)
        fsub dword ptr [esi]
        fxch st(1)
        fmul dword ptr [esp + 0x18]
        fxch st(1)
        fmul dword ptr [esp + 0x14]
        add ecx, 0xc
        add edx, 0x8
        dec ebx
        faddp st(1), st(0)
        fadd dword ptr [edi + 0x8]
        fstp dword ptr [edx - 0x8]
        fld dword ptr [ecx - 0xc]
        fld dword ptr [ecx - 0x4]
        fsub dword ptr [esi + 0x8]
        fxch st(1)
        fsub dword ptr [esi]
        fxch st(1)
        fmul dword ptr [esp + 0x20]
        fxch st(1)
        fmul dword ptr [esp + 0x1c]
        faddp st(1), st(0)
        fadd dword ptr [edi + 0xc]
        fstp dword ptr [edx - 0x4]
        jnz L_46a880
    L_46a8ca:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x14
        ret 0x8
    }
}

// 0x0046b030 ClipModel_SnapToRegion - (clip ECX: points +4, count +8; object EDX): object model [+0x3C]; with flag 0x200 and bounds outside the rect by more than 1.0 -> 0. For each polygon (>= 3 vertices, else 'Skipping clip of polygon' 0x36B with the previous count as the argument): gather points (0x375 on failure), rotate, bounds, overlap -> PointArray_SnapToReference(region points, count, eps 0.1, 0.1) sets changed. Returns changed
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipModel_SnapToRegion(int, int)
{
    __asm {
        sub esp, 0x1c
        push ebx
        push ebp
        push esi
        mov ebx, ecx
        xor esi, esi
        push edi
        cmp ebx, esi
        mov dword ptr [esp + 0x18], esi
        jz L_46b1d9
        cmp edx, esi
        jz L_46b1d9
        mov ecx, dword ptr [edx + 0x3c]
        cmp ecx, esi
        mov dword ptr [esp + 0x14], ecx
        jz L_46b1cd
        mov eax, dword ptr [edx + 0x24]
        test AH, 0x2
        jz L_46b0e2
        fld dword ptr [ebx + 0x14]
        fsub dword ptr [kF_004d2648]
        fld dword ptr [edx + 0x8c]
        fcompp
        fnstsw AX
        test AH, 0x41
        jz L_46b1d9
        fld dword ptr [ebx + 0xc]
        fsub dword ptr [kF_004d264c]
        fld dword ptr [edx + 0x98]
        fcompp
        fnstsw AX
        test AH, 0x1
        jnz L_46b1d9
        fld dword ptr [edx + 0x94]
        fld dword ptr [ebx + 0x10]
        fsub dword ptr [kF_004d264c]
        fxch st(1)
        fchs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x41
        jz L_46b1d9
        fld dword ptr [edx + 0xa0]
        fld dword ptr [ebx + 0x18]
        fsub dword ptr [kF_004d2648]
        fxch st(1)
        fchs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jnz L_46b1d9
    L_46b0e2:
        mov eax, dword ptr [ecx + 0xc]
        mov ebp, dword ptr [ecx + 0x30]
        cmp eax, esi
        mov dword ptr [esp + 0x10], esi
        jle L_46b1bf
        mov edi, dword ptr [esp + 0x18]
        jmp L_46b0fe
    L_46b0fa:
        mov ecx, dword ptr [esp + 0x14]
    L_46b0fe:
        mov eax, dword ptr [ebp]
        and eax, 0xff
        cmp eax, 0x3
        jnc L_46b12a
        push edi
        push offset kStr_004e03e0
        push 0x36b
        push offset kStr_004e034c
        push 0x400
        call Debug_ReportNoop
        add esp, 0x14
        jmp L_46b1a4
    L_46b12a:
        push esi
        mov edx, ebp
        mov edi, eax
        call ModelPolygon_GatherPoints
        mov esi, eax
        test esi, esi
        jnz L_46b158
        push offset kStr_004e03b0
        push 0x375
        push offset kStr_004e034c
        push 0x400
        call Debug_ReportNoop
        add esp, 0x10
        jmp L_46b1a4
    L_46b158:
        mov edx, esi
        mov ecx, edi
        call Vec3Array_RotateX90Inverse
        mov edx, esi
        lea ecx, [esp + 0x1c]
        push edi
        call Rect_FromPoints2D
        lea edx, [ebx + 0xc]
        lea ecx, [esp + 0x1c]
        call Rect_OverlapMargin1
        test eax, eax
        jz L_46b1a4
        mov eax, dword ptr [ebx + 0x8]
        mov ecx, dword ptr [ebx + 0x4]
        push 0x3dcccccd
        push 0x3dcccccd
        push eax
        push ecx
        mov edx, edi
        mov ecx, esi
        call PointArray_SnapToReference
        test eax, eax
        jz L_46b1a4
        mov dword ptr [esp + 0x18], 0x1
    L_46b1a4:
        mov edx, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        inc eax
        add ebp, 0x1c
        mov ecx, dword ptr [edx + 0xc]
        mov dword ptr [esp + 0x10], eax
        cmp eax, ecx
        jl L_46b0fa
    L_46b1bf:
        test esi, esi
        jz L_46b1cd
        push esi
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46b1cd:
        mov eax, dword ptr [esp + 0x18]
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x1c
        ret
    L_46b1d9:
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0x1c
        ret
    }
}

// 0x0046bb90 ClipModel_AgainstRegion - (clip ECX, model EDX): either null -> 0. For each polygon (count +0xC, 0x1C at +0x30): < 3 vertices -> report 'Skipping clip of polygon with %d' (zg2 0x5CE); gather points (failure report 0x5D5); rotate (Vec3Array_RotateX90Inverse), bounds, Rect_OverlapMargin1 false -> result 1, else WeilerClip_Run 0x00464810(points, n, result block). Result 0 -> 'Weiler algorithm clip error occurred' (0x5ED), fre
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes); parameter meaning in the spec line above.
__declspec(naked) int __fastcall ClipModel_AgainstRegion(int, int)
{
    __asm {
        sub esp, 0x38
        push ebx
        push ebp
        mov ebp, edx
        xor ebx, ebx
        push esi
        cmp ebp, ebx
        push edi
        mov dword ptr [esp + 0x10], ecx
        jz L_46bcb9
        cmp ecx, ebx
        jz L_46bcb9
        mov ecx, 0x8
        xor eax, eax
        lea edi, [esp + 0x28]
        mov dword ptr [esp + 0x14], ebx
        rep stosd
        mov eax, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp + 0x30]
        cmp eax, ebx
        jle L_46bc9e
    L_46bbce:
        mov esi, dword ptr [edi]
        and esi, 0xff
        cmp esi, 0x3
        jge L_46bbfd
        push esi
        push offset kStr_004e03e0
        push 0x5ce
        push offset kStr_004e034c
        push 0x400
        call Debug_ReportNoop
        add esp, 0x14
        jmp L_46bc87
    L_46bbfd:
        push ebx
        mov edx, edi
        mov ecx, ebp
        call ModelPolygon_GatherPoints
        mov ebx, eax
        test ebx, ebx
        jnz L_46bc2b
        push offset kStr_004e03b0
        push 0x5d5
        push offset kStr_004e034c
        push 0x400
        call Debug_ReportNoop
        add esp, 0x10
        jmp L_46bc87
    L_46bc2b:
        mov edx, ebx
        mov ecx, esi
        call Vec3Array_RotateX90Inverse
        mov edx, ebx
        lea ecx, [esp + 0x18]
        push esi
        call Rect_FromPoints2D
        lea ecx, [esp + 0x18]
        mov eax, dword ptr [esp + 0x10]
        lea edx, [eax + 0xc]
        call Rect_OverlapMargin1
        test eax, eax
        jz L_46bc6d
        mov eax, dword ptr [esp + 0x10]
        lea ecx, [esp + 0x28]
        push ecx
        push esi
        mov ecx, dword ptr [eax]
        push ebx
        mov edx, 0x4
        call WeilerClip_Run
        jmp L_46bc72
    L_46bc6d:
        mov eax, 0x1
    L_46bc72:
        cmp eax, 0x4
        ja L_46bc7e
        cmp eax, 0
        je L_46bcc3
        cmp eax, 1
        je L_46bc7e
        cmp eax, 2
        je L_46bcf7
        cmp eax, 3
        je L_46bd1b
        cmp eax, 4
        je L_46bd1b
        int 3  // unreachable: the bounds check above excludes other indices
    L_46bc7e:
        lea ecx, [esp + 0x28]
        call WeilerClip_FreeBuffers
    L_46bc87:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [ebp + 0xc]
        inc eax
        add edi, 0x1c
        cmp eax, ecx
        mov dword ptr [esp + 0x14], eax
        jl L_46bbce
    L_46bc9e:
        test ebx, ebx
        jz L_46bcac
        push ebx
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46bcac:
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x38
        ret
    L_46bcb9:
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x38
        ret
    L_46bcc3:
        push offset kStr_004e04e4
        push 0x5ed
        push offset kStr_004e034c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
        test ebx, ebx
        jz L_46bcb9
        push ebx
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x38
        ret
    L_46bcf7:
        mov eax, dword ptr [esp + 0x38]
        test eax, eax
        jnz L_46bd1b
        push offset kStr_004e04c0
        push 0x5f7
        push offset kStr_004e034c
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
    L_46bd1b:
        test ebx, ebx
        jz L_46bcb9
        push ebx
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x38
        ret
    }
}

// 0x0046ae40 ClipGroupList_Release - (list ECX: count +8, 0xC-byte groups +0xC {n, buffer, node}): per group, n times: 0x00447f00 / 0x00447e60 detach model and Model_Free when one was attached (arguments not visible); node +0x39 byte++; free buffer and zero n/buffer. Return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall ClipGroupList_Release(int, int)
{
    __asm {
        sub esp, 0x8
        push ebp
        push edi
        mov edi, ecx
        xor ebp, ebp
        mov dword ptr [esp + 0x8], ebp
        cmp dword ptr [edi + 0x8], ebp
        jle L_46aeea
        push esi
        push ebx
        xor esi, esi
    L_46ae5a:
        mov eax, dword ptr [edi + 0xc]
        xor ebx, ebx
        cmp dword ptr [esi + eax*0x1], ebp
        jle L_46aea0
    L_46ae64:
        mov eax, dword ptr [esi + eax*0x1 + 0x4]
        lea edx, [esp + 0x14]
        mov ecx, dword ptr [eax + ebx*0x8]
        call gwNodeGetModel
        mov ecx, dword ptr [edi + 0xc]
        mov edx, dword ptr [ecx + esi*0x1 + 0x4]
        mov ecx, dword ptr [edx + ebx*0x8]
        lea eax, [edx + ebx*0x8]
        mov edx, dword ptr [edx + ebx*0x8 + 0x4]
        call gwNodeSetModel
        mov ecx, dword ptr [esp + 0x14]
        cmp ecx, ebp
        jz L_46ae97
        call Model_Free
    L_46ae97:
        mov eax, dword ptr [edi + 0xc]
        inc ebx
        cmp ebx, dword ptr [esi + eax*0x1]
        jl L_46ae64
    L_46aea0:
        mov eax, dword ptr [edi + 0xc]
        mov eax, dword ptr [eax + esi*0x1 + 0x8]
        mov DL, byte ptr [eax + 0x39]
        inc DL
        mov byte ptr [eax + 0x39], DL
        mov ecx, dword ptr [edi + 0xc]
        mov eax, dword ptr [ecx + esi*0x1 + 0x4]
        cmp eax, ebp
        jz L_46aed1
        push eax
        call dword ptr [g_Iat_free_004cc5b4]
        mov edx, dword ptr [edi + 0xc]
        add esp, 0x4
        mov dword ptr [edx + esi*0x1 + 0x4], ebp
        mov eax, dword ptr [edi + 0xc]
        mov dword ptr [eax + esi*0x1], ebp
    L_46aed1:
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [edi + 0x8]
        inc eax
        add esi, 0xc
        cmp eax, ecx
        mov dword ptr [esp + 0x10], eax
        jl L_46ae5a
        pop ebx
        pop esi
    L_46aeea:
        pop edi
        xor eax, eax
        pop ebp
        add esp, 0x8
        ret
    }
}

// 0x0046bb30 ClipPolygon_EmitToModel - (model ECX, polygon arg: flags +0 (bit 8 -> arg), +4, count +0x10, material +0x14, +0x18 out): pts = 0x0046b650(0) (other arguments not visible); Model_Call483650_Arg2Zero(pts, count, 0, 0, 0, material, [+4], flag, &arg+0x18); free pts; return result
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ClipPolygon_EmitToModel(int, int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, dword ptr [esp + 0xc]
        mov eax, edx
        mov ebx, ecx
        push edi
        push 0x0
        mov edx, esi
        mov ecx, eax
        call ModelPolygon_GatherPoints
        mov edx, dword ptr [esi]
        mov edi, eax
        mov ecx, edx
        lea eax, [esi + 0x18]
        shr ecx, 0x8
        and ecx, 0x1
        push eax
        mov eax, dword ptr [esi + 0x4]
        push ecx
        mov ecx, dword ptr [esi + 0x14]
        push eax
        mov eax, dword ptr [esi + 0x10]
        push ecx
        push 0x0
        push 0x0
        push 0x0
        push eax
        push edi
        and edx, 0xff
        mov ecx, ebx
        call Model_Call483650_Arg2Zero
        test edi, edi
        mov esi, eax
        jz L_46bb87
        push edi
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46bb87:
        mov eax, esi
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x0046ba90 Geometry_AddChildPolygon - count EDX < 3 -> 'Attempting to add child polygon with...' (zg2 0x111) return -1; parent polygon arg3: no material +0x10 -> uvs none, material 0x0046a690(); else uvs = 0x0046a7f0(arg2, parent) and material parent +0x14; Model_Call483650_Arg2Zero(points arg1, uvs, 0, 0, 0, material, parent +4, parent
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall Geometry_AddChildPolygon(int, int, int, int, int)
{
    __asm {
        push ecx
        push ebx
        mov ebx, edx
        cmp ebx, 0x3
        mov dword ptr [esp + 0x4], ecx
        jge L_46bac2
        push ebx
        push offset g_Data_004da000 + 0x648c
        push 0x111
        push offset g_Data_004da000 + 0x634c
        push 0x800
        call Debug_ReportNoop
        add esp, 0x14
        or eax, 0xffffffff
        pop ebx
        pop ecx
        ret 0xc
    L_46bac2:
        push edi
        push esi
        mov esi, dword ptr [esp + 0x1c]
        push ebp
        mov ebp, dword ptr [esp + 0x18]
        mov eax, dword ptr [esi + 0x10]
        test eax, eax
        jz L_46baea
        mov eax, dword ptr [esp + 0x1c]
        push esi
        push eax
        mov edx, ebp
        mov ecx, ebx
        call ClipPolygon_ComputeUVs
        mov edi, eax
        mov eax, dword ptr [esi + 0x14]
        jmp L_46baf1
    L_46baea:
        xor edi, edi
        call Material_CreateRandomColour
    L_46baf1:
        mov edx, dword ptr [esi]
        lea ecx, [esi + 0x18]
        shr edx, 0x8
        push ecx
        mov ecx, dword ptr [esi + 0x4]
        and edx, 0x1
        push edx
        push ecx
        mov ecx, dword ptr [esp + 0x1c]
        push eax
        push 0x0
        push 0x0
        push 0x0
        push edi
        push ebp
        mov edx, ebx
        call Model_Call483650_Arg2Zero
        test edi, edi
        mov esi, eax
        jz L_46bb26
        push edi
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46bb26:
        mov eax, esi
        pop ebp
        pop esi
        pop edi
        pop ebx
        pop ecx
        ret 0xc
    }
}

// 0x0046b6d0 ClipModel_SplitByRegion - (clip ECX, model EDX, out new model arg): either null -> 1 with nothing written. newModel = Model_Alloc (0 -> return 0); result block zeroed; SetDword0_FromEDX (arguments not visible). Per polygon (< 3 vertices reported 'Skipping clip of polygon with %d' zg2 0x4D7; gather failure 0x4DF): rotate, bou
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ClipModel_SplitByRegion(int, int, int)
{
    __asm {
        sub esp, 0x54
        push ebx
        push ebp
        push esi
        push edi
        mov ebx, edx
        xor edi, edi
        mov esi, ecx
        cmp ebx, edi
        mov dword ptr [esp + 0x14], ebx
        mov dword ptr [esp + 0x30], esi
        mov dword ptr [esp + 0x1c], edi
        mov dword ptr [esp + 0x20], edi
        mov dword ptr [esp + 0x18], edi
        jz L_46b9ee
        cmp esi, edi
        jz L_46b9ee
        call Model_Alloc
        mov ebp, eax
        cmp ebp, edi
        mov dword ptr [esp + 0x10], ebp
        jnz L_46b71c
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x54
        ret 0x4
    L_46b71c:
        mov ecx, 0x8
        xor eax, eax
        lea edi, [esp + 0x44]
        xor edx, edx
        rep stosd
        mov ecx, ebp
        call SetDword0_FromEDX
        mov eax, dword ptr [ebx + 0xc]
        mov ebp, dword ptr [ebx + 0x30]
        test eax, eax
        mov dword ptr [esp + 0x24], ebp
        mov dword ptr [esp + 0x2c], 0x0
        jle L_46b9b9
    L_46b74c:
        mov edi, dword ptr [ebp]
        and edi, 0xff
        cmp edi, 0x3
        jge L_46b77c
        push edi
        push offset g_Data_004da000 + 0x63e0
        push 0x4d7
        push offset g_Data_004da000 + 0x634c
        push 0x400
        call Debug_ReportNoop
        add esp, 0x14
        jmp L_46b99e
    L_46b77c:
        mov eax, dword ptr [esp + 0x20]
        mov edx, ebp
        push eax
        mov ecx, ebx
        call ModelPolygon_GatherPoints
        mov ebx, eax
        test ebx, ebx
        mov dword ptr [esp + 0x20], ebx
        jnz L_46b7b5
        push offset g_Data_004da000 + 0x63b0
        push 0x4df
        push offset g_Data_004da000 + 0x634c
        push 0x400
        call Debug_ReportNoop
        add esp, 0x10
        jmp L_46b99a
    L_46b7b5:
        mov edx, ebx
        mov ecx, edi
        call Vec3Array_RotateX90Inverse
        mov edx, ebx
        lea ecx, [esp + 0x34]
        push edi
        call Rect_FromPoints2D
        lea edx, [esi + 0xc]
        lea ecx, [esp + 0x34]
        call Rect_OverlapMargin1
        test eax, eax
        jz L_46b812
        mov eax, dword ptr [esp + 0x18]
        test eax, eax
        jz L_46b7f8
        mov ecx, dword ptr [esi + 0x8]
        mov edx, dword ptr [esi + 0x4]
        push ecx
        mov ecx, esi
        call WeilerClip_ResetRegion
        mov dword ptr [esp + 0x18], 0x0
    L_46b7f8:
        mov ecx, dword ptr [esi]
        lea edx, [esp + 0x44]
        push edx
        push edi
        push ebx
        mov edx, 0x3
        call WeilerClip_Run
        mov edi, 0x1
        jmp L_46b819
    L_46b812:
        mov edi, 0x1
        mov eax, edi
    L_46b819:
        cmp eax, 0x4
        ja L_46b991
        cmp eax, 0
        je L_46ba19
        cmp eax, 1
        je L_46b983
        cmp eax, 2
        je L_46b829
        cmp eax, 3
        je L_46b8c3
        cmp eax, 4
        je L_46b8cc
        int 3  // unreachable: the bounds check above excludes other indices
    L_46b829:
        mov eax, dword ptr [esp + 0x48]
        mov edx, dword ptr [esp + 0x60]
        mov dword ptr [esp + 0x1c], edi
        mov ecx, dword ptr [eax + 0x4]
        lea ecx, [edx + ecx*0x4]
        mov edx, dword ptr [eax]
        push ecx
        mov ecx, esi
        call ClipPolygon_MergePoints
        test eax, eax
        jz L_46b84d
        mov dword ptr [esp + 0x18], edi
    L_46b84d:
        mov edx, dword ptr [esp + 0x60]
        lea ecx, [esp + 0x4c]
        push edx
        mov edx, dword ptr [esp + 0x60]
        call Convexify
        mov ebx, eax
        test ebx, ebx
        jz L_46b991
        mov edx, dword ptr [ebx + 0xc]
        mov ecx, dword ptr [ebx + 0x8]
        call Vec3Array_RotateX90
        mov eax, dword ptr [ebx]
        mov edi, dword ptr [ebx + 0x4]
        test eax, eax
        mov dword ptr [esp + 0x24], 0x0
        jbe L_46b8b7
    L_46b885:
        mov edx, dword ptr [edi]
        cmp edx, 0x3
        jc L_46b8a8
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [edi + 0x4]
        push ebp
        push eax
        mov eax, dword ptr [ebx + 0xc]
        lea ecx, [eax + ecx*0x4]
        push ecx
        mov ecx, dword ptr [esp + 0x1c]
        call Geometry_AddChildPolygon
        add edi, 0x8
    L_46b8a8:
        mov eax, dword ptr [esp + 0x24]
        mov ecx, dword ptr [ebx]
        inc eax
        cmp eax, ecx
        mov dword ptr [esp + 0x24], eax
        jc L_46b885
    L_46b8b7:
        mov ecx, ebx
        call Alloc3Ptr_Free
        jmp L_46b991
    L_46b8c3:
        mov dword ptr [esp + 0x1c], edi
        jmp L_46b991
    L_46b8cc:
        mov eax, dword ptr [esp + 0x4c]
        test eax, eax
        jz L_46b9fd
        mov ecx, dword ptr [esi]
        mov dword ptr [esp + 0x1c], edi
        mov edi, dword ptr [ebp]
        lea edx, [esp + 0x28]
        and edi, 0xff
        call WeilerClip_GetOutput
        mov edx, dword ptr [esp + 0x28]
        mov ebp, eax
        push edx
        push ebp
        mov edx, ebx
        mov ecx, edi
        call Triangulate_RingPair
        mov ebx, eax
        mov edx, ebp
        mov eax, dword ptr [esp + 0x28]
        mov ecx, esi
        push eax
        call ClipPolygon_MergePoints
        test eax, eax
        jz L_46b91d
        mov dword ptr [esp + 0x18], 0x1
    L_46b91d:
        mov eax, dword ptr [ebx]
        add edi, ebp
        cmp eax, edi
        lea esi, [ebx + 0x4]
        jl L_46ba3c
        xor edi, edi
        test eax, eax
        jle L_46b96f
    L_46b932:
        push 0x1
        mov edx, esi
        mov ecx, 0x3
        call Triangle_EnsureCounterClockwise
        mov edx, esi
        mov ecx, 0x3
        call Vec3Array_RotateX90
        mov ecx, dword ptr [esp + 0x24]
        mov edx, dword ptr [esp + 0x14]
        push ecx
        mov ecx, dword ptr [esp + 0x14]
        push edx
        push esi
        mov edx, 0x3
        call Geometry_AddChildPolygon
        mov eax, dword ptr [ebx]
        add esi, 0x24
        inc edi
        cmp edi, eax
        jl L_46b932
    L_46b96f:
        push ebx
        call dword ptr [g_Iat_free_004cc5b4]
        mov esi, dword ptr [esp + 0x34]
        mov ebp, dword ptr [esp + 0x28]
        add esp, 0x4
        jmp L_46b991
    L_46b983:
        mov edx, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x10]
        push ebp
        call ClipPolygon_EmitToModel
    L_46b991:
        lea ecx, [esp + 0x44]
        call WeilerClip_FreeBuffers
    L_46b99a:
        mov ebx, dword ptr [esp + 0x14]
    L_46b99e:
        mov eax, dword ptr [esp + 0x2c]
        mov ecx, dword ptr [ebx + 0xc]
        inc eax
        add ebp, 0x1c
        cmp eax, ecx
        mov dword ptr [esp + 0x2c], eax
        mov dword ptr [esp + 0x24], ebp
        jl L_46b74c
    L_46b9b9:
        mov eax, dword ptr [esp + 0x1c]
        test eax, eax
        jnz L_46b9d2
        mov ecx, dword ptr [esp + 0x10]
        call Model_Free
        mov dword ptr [esp + 0x10], 0x0
    L_46b9d2:
        mov eax, dword ptr [esp + 0x68]
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [eax], ecx
        mov eax, dword ptr [esp + 0x20]
        test eax, eax
        jz L_46b9ee
        push eax
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46b9ee:
        mov eax, 0x1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x54
        ret 0x4
    L_46b9fd:
        push offset g_Data_004da000 + 0x6458
        push 0x548
        push offset g_Data_004da000 + 0x634c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x10
    L_46ba19:
        test ebx, ebx
        jz L_46ba27
        push ebx
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46ba27:
        mov ecx, dword ptr [esp + 0x10]
        call Model_Free
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x54
        ret 0x4
    L_46ba3c:
        mov eax, dword ptr [esp + 0x20]
        test eax, eax
        jz L_46ba4e
        push eax
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
    L_46ba4e:
        mov ecx, dword ptr [esp + 0x10]
        call Model_Free
        push ebx
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x54
        ret 0x4
    }
}

// 0x0046b550 ClipObject_Dispatch - (clip rect ECX: minx +0xC, miny +0x10, maxx +0x14, maxy +0x18; object EDX; out arg): either null -> 1. Object has model [+0x3C]: flag 0x200 with bounds [+0x8C..+0xA0] outside the rect by more than 1.0 (y/z negated) -> skipped (1). Otherwise flag 0x20000 -> *out = 0 and 0x0046bb90; flag 0x10000 -> 0x
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ClipObject_Dispatch(int, int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        xor edi, edi
        mov esi, 0x1
        test ecx, ecx
        jz L_46b642
        test edx, edx
        jz L_46b642
        mov ebx, dword ptr [edx + 0x3c]
        test ebx, ebx
        jz L_46b5fb
        mov ebp, dword ptr [edx + 0x24]
        test ebp, 0x200
        jz L_46b5f2
        fld dword ptr [ecx + 0x14]
        fsub dword ptr [g_RData_004cc000 + 0x6648]
        fld dword ptr [edx + 0x8c]
        fcompp
        fnstsw AX
        test AH, 0x41
        jz L_46b5ed
        fld dword ptr [ecx + 0xc]
        fsub dword ptr [g_RData_004cc000 + 0x664c]
        fld dword ptr [edx + 0x98]
        fcompp
        fnstsw AX
        test AH, 0x1
        jnz L_46b5ed
        fld dword ptr [edx + 0x94]
        fld dword ptr [ecx + 0x10]
        fsub dword ptr [g_RData_004cc000 + 0x664c]
        fxch st(1)
        fchs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x41
        jz L_46b5ed
        fld dword ptr [edx + 0xa0]
        fld dword ptr [ecx + 0x18]
        fsub dword ptr [g_RData_004cc000 + 0x6648]
        fxch st(1)
        fchs
        fxch st(1)
        fcompp
        fnstsw AX
        test AH, 0x1
        jz L_46b5f2
    L_46b5ed:
        mov edi, 0x1
    L_46b5f2:
        test edi, edi
        jz L_46b604
        mov esi, 0x1
    L_46b5fb:
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    L_46b604:
        test ebp, 0x20000
        jz L_46b627
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [eax], 0x0
        mov edx, dword ptr [edx + 0x3c]
        call ClipModel_AgainstRegion
        mov esi, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    L_46b627:
        test ebp, 0x10000
        jz L_46b5fb
        mov edx, dword ptr [esp + 0x14]
        push edx
        mov edx, ebx
        call ClipModel_SplitByRegion
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x4
    L_46b642:
        pop edi
        pop esi
        pop ebp
        mov eax, 0x1
        pop ebx
        ret 0x4
    }
}

// 0x0046b1f0 AreaPartition_ClipObjects - (partition cell arg1, result list arg2 {+4 points, +8 count, +0xC 0xC-byte groups}): null cell or EDX -> report 'Null Area Partition (0x%08x) or ...' (zg2 0x3AF), -1. region = ClipPolygon_Create (arguments not visible; null -> -1). Appends group {n = cell object count short +0x3A, calloc(n, 8) pairs
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall AreaPartition_ClipObjects(int, int, int, int)
{
    __asm {
        sub esp, 0x20
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0x2c]
        push esi
        test ebp, ebp
        push edi
        mov dword ptr [esp + 0x20], 0x0
        jz L_46b51c
        test edx, edx
        jz L_46b51c
        call ClipPolygon_Create
        test eax, eax
        mov dword ptr [esp + 0x14], eax
        jz L_46b53a
        mov esi, dword ptr [esp + 0x38]
        mov eax, dword ptr [esi + 0x8]
        mov ecx, dword ptr [esi + 0xc]
        inc eax
        lea eax, [eax + eax*0x2]
        shl eax, 0x2
        push eax
        push ecx
        call dword ptr [g_Iat_realloc_004cc4ec]
        mov ecx, dword ptr [esi + 0x8]
        add esp, 0x8
        mov dword ptr [esi + 0xc], eax
        lea edx, [ecx + ecx*0x2]
        inc ecx
        mov dword ptr [esi + 0x8], ecx
        push 0x8
        lea edi, [eax + edx*0x4]
        mov dword ptr [esp + 0x30], edi
        mov dword ptr [edi + 0x8], ebp
        movsx eax, word ptr [ebp + 0x3a]
        mov dword ptr [edi], eax
        movsx ecx, word ptr [ebp + 0x3a]
        push ecx
        call dword ptr [g_Iat_calloc_004cc4ac]
        add esp, 0x8
        mov dword ptr [esp + 0x1c], eax
        mov dword ptr [edi + 0x4], eax
        call DEClient_GetCurrentWorld
        mov esi, eax
        mov ebx, dword ptr [g_Iat_malloc_004cc5dc]
        movsx eax, word ptr [ebp + 0x3a]
        add eax, dword ptr [esi + 0x5c]
        lea edi, [eax*0x4 + 0x0]
        push edi
        call ebx
        add esp, 0x4
        mov dword ptr [esp + 0x24], eax
        mov dword ptr [esp + 0x18], 0x0
        push edi
        call ebx
        mov edi, eax
        mov eax, dword ptr [esi + 0x5c]
        add esp, 0x4
        xor ebx, ebx
        xor edx, edx
        mov dword ptr [esp + 0x28], edi
        test eax, eax
        mov dword ptr [esp + 0x10], ebx
        jle L_46b311
        mov ebx, dword ptr [esp + 0x24]
    L_46b2c0:
        mov eax, dword ptr [esi + 0x60]
        mov eax, dword ptr [eax + edx*0x4]
        mov ecx, dword ptr [eax + 0x24]
        test CL, 0x4
        jz L_46b301
        test ecx, 0x20000
        jz L_46b2e4
        mov ecx, dword ptr [esp + 0x18]
        mov dword ptr [ebx], eax
        inc ecx
        add ebx, 0x4
        mov dword ptr [esp + 0x18], ecx
    L_46b2e4:
        mov ecx, dword ptr [esi + 0x60]
        mov eax, dword ptr [ecx + edx*0x4]
        test dword ptr [eax + 0x24], 0x10000
        jz L_46b301
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [edi], eax
        inc ecx
        add edi, 0x4
        mov dword ptr [esp + 0x10], ecx
    L_46b301:
        mov eax, dword ptr [esi + 0x5c]
        inc edx
        cmp edx, eax
        jl L_46b2c0
        mov ebx, dword ptr [esp + 0x10]
        mov edi, dword ptr [esp + 0x28]
    L_46b311:
        xor edx, edx
        cmp word ptr [ebp + 0x3a], DX
        jle L_46b375
        mov eax, dword ptr [esp + 0x24]
        lea esi, [edi + ebx*0x4]
        mov ebx, dword ptr [esp + 0x18]
        lea edi, [eax + ebx*0x4]
    L_46b327:
        mov ecx, dword ptr [ebp + 0x3c]
        mov eax, dword ptr [ecx + edx*0x4]
        mov ecx, dword ptr [eax + 0x24]
        test CL, 0x4
        jz L_46b360
        test ecx, 0x20000
        jz L_46b343
        mov dword ptr [edi], eax
        inc ebx
        add edi, 0x4
    L_46b343:
        mov eax, dword ptr [ebp + 0x3c]
        mov eax, dword ptr [eax + edx*0x4]
        test dword ptr [eax + 0x24], 0x10000
        jz L_46b360
        mov ecx, dword ptr [esp + 0x10]
        mov dword ptr [esi], eax
        inc ecx
        add esi, 0x4
        mov dword ptr [esp + 0x10], ecx
    L_46b360:
        movsx ecx, word ptr [ebp + 0x3a]
        inc edx
        cmp edx, ecx
        jl L_46b327
        mov edi, dword ptr [esp + 0x28]
        mov dword ptr [esp + 0x18], ebx
        mov ebx, dword ptr [esp + 0x10]
    L_46b375:
        test ebx, ebx
        mov esi, 0x1
        jle L_46b3a6
    L_46b37e:
        mov ebp, dword ptr [esp + 0x14]
        mov edx, dword ptr [edi]
        mov ecx, ebp
        call ClipModel_SnapToRegion
        test eax, eax
        jz L_46b39e
        mov edx, dword ptr [ebp + 0x8]
        lea ecx, [ebp + 0xc]
        push edx
        mov edx, dword ptr [ebp + 0x4]
        call Rect_FromPoints2D
    L_46b39e:
        add edi, 0x4
        dec ebx
        jnz L_46b37e
        jmp L_46b3aa
    L_46b3a6:
        mov ebp, dword ptr [esp + 0x14]
    L_46b3aa:
        mov edx, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [ebp + 0x4]
        push 0x0
        call WeilerClip_Create
        mov dword ptr [ebp], eax
        xor ebx, ebx
        mov edi, dword ptr [esp + 0x24]
    L_46b3c0:
        cmp ebx, dword ptr [esp + 0x18]
        jge L_46b3e1
        mov eax, dword ptr [esp + 0x1c]
        mov edx, dword ptr [edi]
        add eax, 0x4
        mov ecx, ebp
        push eax
        call ClipObject_Dispatch
        mov esi, eax
        inc ebx
        add edi, 0x4
        test esi, esi
        jnz L_46b3c0
    L_46b3e1:
        xor ebp, ebp
        test esi, esi
        jz L_46b431
        mov ecx, dword ptr [esp + 0x1c]
        mov ebx, dword ptr [esp + 0x28]
        lea edi, [ecx + 0x4]
    L_46b3f2:
        cmp ebp, dword ptr [esp + 0x10]
        jge L_46b431
        mov eax, dword ptr [esp + 0x1c]
        mov edx, dword ptr [ebx]
        mov ecx, dword ptr [esp + 0x14]
        push edi
        mov dword ptr [eax], edx
        call ClipObject_Dispatch
        mov esi, eax
        mov eax, dword ptr [edi]
        test eax, eax
        jz L_46b429
        mov edx, dword ptr [esp + 0x20]
        mov ecx, dword ptr [esp + 0x1c]
        inc edx
        add ecx, 0x8
        mov dword ptr [esp + 0x20], edx
        mov dword ptr [esp + 0x1c], ecx
        add edi, 0x8
    L_46b429:
        inc ebp
        add ebx, 0x4
        test esi, esi
        jnz L_46b3f2
    L_46b431:
        mov edi, dword ptr [esp + 0x20]
        mov ebx, dword ptr [g_Iat_free_004cc5b4]
        test edi, edi
        jz L_46b493
        test esi, esi
        jz L_46b493
        mov ecx, dword ptr [esp + 0x34]
        movsx edx, word ptr [ecx + 0x3a]
        cmp edi, edx
        jz L_46b480
        mov esi, dword ptr [esp + 0x2c]
        lea eax, [edi*0x8 + 0x0]
        push eax
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [esi], edi
        push ecx
        call dword ptr [g_Iat_realloc_004cc4ec]
        mov edx, dword ptr [esp + 0x40]
        mov ecx, dword ptr [esp + 0x1c]
        add esp, 0x8
        mov dword ptr [esi + 0x4], eax
        lea eax, [edx + 0x4]
        push eax
        call ClipPolygon_CopyOutRotated
        jmp L_46b4eb
    L_46b480:
        mov edx, dword ptr [esp + 0x38]
        lea ecx, [edx + 0x4]
        push ecx
        mov ecx, dword ptr [esp + 0x18]
        call ClipPolygon_CopyOutRotated
        jmp L_46b4eb
    L_46b493:
        mov esi, dword ptr [esp + 0x38]
        mov edx, dword ptr [esi + 0x8]
        dec edx
        mov dword ptr [esi + 0x8], edx
        mov edx, dword ptr [esp + 0x2c]
        mov eax, dword ptr [edx + 0x4]
        test eax, eax
        jz L_46b4af
        push eax
        call ebx
        add esp, 0x4
    L_46b4af:
        mov eax, dword ptr [esi + 0x8]
        test eax, eax
        jnz L_46b4c8
        mov eax, dword ptr [esi + 0xc]
        push eax
        call ebx
        add esp, 0x4
        mov dword ptr [esi + 0xc], 0x0
        jmp L_46b4df
    L_46b4c8:
        mov edx, dword ptr [esi + 0xc]
        lea ecx, [eax + eax*0x2]
        shl ecx, 0x2
        push ecx
        push edx
        call dword ptr [g_Iat_realloc_004cc4ec]
        add esp, 0x8
        mov dword ptr [esi + 0xc], eax
    L_46b4df:
        mov dword ptr [esp + 0x20], 0x0
        mov edi, dword ptr [esp + 0x20]
    L_46b4eb:
        mov ecx, dword ptr [esp + 0x14]
        call ClipResult_Free
        mov eax, dword ptr [esp + 0x24]
        test eax, eax
        jz L_46b502
        push eax
        call ebx
        add esp, 0x4
    L_46b502:
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        jz L_46b510
        push eax
        call ebx
        add esp, 0x4
    L_46b510:
        mov eax, edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x20
        ret 0x8
    L_46b51c:
        push edx
        push ebp
        push offset g_Data_004da000 + 0x640c
        push 0x3af
        push offset g_Data_004da000 + 0x634c
        push 0x100
        call Debug_ReportNoop
        add esp, 0x18
    L_46b53a:
        pop edi
        pop esi
        pop ebp
        or eax, 0xffffffff
        pop ebx
        add esp, 0x20
        ret 0x8
    }
}

// 0x0046af40 ClipGroup_BuildModelNode - (group ECX: n +0, node/value pairs +4; out node arg): null -> 0. node = 0x0044daa0() (arguments not visible); none -> *out = 0, return 0. *out = node; 0x00448330; first pair whose object flag [+0x24] has 0x10000 and 0x00448180 gives a value other than 0xFF -> 0x00448330 again; 0x00447d70; model = Mo
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall ClipGroup_BuildModelNode(int, int, int)
{
    __asm {
        sub esp, 0x8
        push ebx
        push ebp
        push esi
        push edi
        mov edi, ecx
        mov dword ptr [esp + 0x14], edx
        test edi, edi
        jnz L_46af5d
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x4
    L_46af5d:
        call Object3D_Create
        mov ebx, eax
        test ebx, ebx
        jnz L_46af82
        mov eax, dword ptr [esp + 0x1c]
        test eax, eax
        jz L_46afff
        mov dword ptr [eax], ebx
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x4
    L_46af82:
        mov ebp, dword ptr [esp + 0x1c]
        test ebp, ebp
        jz L_46af8d
        mov dword ptr [ebp], ebx
    L_46af8d:
        mov edx, 0xff
        mov ecx, ebx
        call gwNodeSetByte30
        mov eax, dword ptr [edi]
        xor esi, esi
        test eax, eax
        jle L_46afda
        mov ebp, 0x10000
    L_46afa6:
        mov eax, dword ptr [edi + 0x4]
        mov ecx, dword ptr [eax + esi*0x8]
        test dword ptr [ecx + 0x24], ebp
        jz L_46afc6
        lea edx, [esp + 0x10]
        call gwNodeGetByte30
        mov edx, dword ptr [esp + 0x10]
        cmp edx, 0xff
        jnz L_46afcf
    L_46afc6:
        mov eax, dword ptr [edi]
        inc esi
        cmp esi, eax
        jl L_46afa6
        jmp L_46afd6
    L_46afcf:
        mov ecx, ebx
        call gwNodeSetByte30
    L_46afd6:
        mov ebp, dword ptr [esp + 0x1c]
    L_46afda:
        mov edx, 0x1
        mov ecx, ebx
        call gwNodeSetFlag20000
        call Model_Alloc
        mov esi, eax
        test esi, esi
        jnz L_46b00b
        test ebp, ebp
        jz L_46aff8
        mov dword ptr [ebp], eax
    L_46aff8:
        mov ecx, ebx
        call gwNodeDelete
    L_46afff:
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x4
    L_46b00b:
        mov ecx, dword ptr [esp + 0x14]
        mov edx, ebx
        call gwNodeAttachChildByClass
        mov edx, esi
        mov ecx, ebx
        call gwNodeSetModel
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x8
        ret 0x4
    }
}

// 0x0046a770 Geometry_AddPolygonToModel - 0x00476320 (argument not visible); count EDX < 3 -> 'Attempting to generate polygon with...' (zg2 0x9F, severity 0x800) return -1; material arg2 and extra arg3 both 0 -> material = 0x0046a690(); Model_Call483650_Arg2Zero(points arg1, extra arg3, 0, 0, 0, material, 0, 0, local)
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall Geometry_AddPolygonToModel(int, int, int, int, int)
{
    __asm {
        push ecx
        push ebx
        mov ebx, ecx
        push esi
        mov esi, edx
        lea ecx, [esp + 0x8]
        call Render_InitRecordBytes00FFFFFF
        cmp esi, 0x3
        jge L_46a7ab
        push esi
        push offset g_Data_004da000 + 0x637c
        push 0x9f
        push offset g_Data_004da000 + 0x634c
        push 0x800
        call Debug_ReportNoop
        add esp, 0x14
        or eax, 0xffffffff
        pop esi
        pop ebx
        pop ecx
        ret 0xc
    L_46a7ab:
        mov eax, dword ptr [esp + 0x14]
        push edi
        mov edi, dword ptr [esp + 0x1c]
        test edi, edi
        jnz L_46a7c1
        test eax, eax
        jnz L_46a7c1
        call Material_CreateRandomColour
    L_46a7c1:
        lea ecx, [esp + 0xc]
        mov edx, dword ptr [esp + 0x14]
        push ecx
        push 0x0
        push 0x0
        push eax
        push 0x0
        push 0x0
        push 0x0
        push edi
        push edx
        mov edx, esi
        mov ecx, ebx
        call Model_Call483650_Arg2Zero
        pop edi
        pop esi
        pop ebx
        pop ecx
        ret 0xc
    }
}

}  // namespace recoil
