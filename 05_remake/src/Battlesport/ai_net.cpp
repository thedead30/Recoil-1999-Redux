// SUBSYSTEM: ai_net
// Original file: Battlesport\ai_net.cpp (ledger orig_file, address range 0x00402080..0x00403870).
// Spec: 04_spec/systems/ai_net.md
#include "Battlesport/ai_net.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"
#include "unattributed/math3d.h"
#include "GameZRecoil/zReader/zreader.h"
#include "unattributed/ui_widgets.h"
#include "Battlesport/hud.h"
#include "GameZRecoil/zClass/Camera.h"
#include "GameZRecoil/zClass/Class.h"
#include "GameZRecoil/zClass/cls_di.h"
#include "unattributed/menus.h"
#include "unattributed/player.h"
#include "unattributed/vehicle.h"
#include "GameZRecoil/zWeapon/zwep_init.h"

namespace recoil {

// 0x00402f60 Vec3_NormalizeInPlace - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00402f60..0x00402fc1).
// len = sqrt((x*x + z*z) + y*y) at 53 bits, stored as float (FST, value kept); if the float's bits with
// the sign masked are zero (TEST 0x7fffffff), v is untouched. Else inv = 1.0 / len (unrounded, FLD1);
// x, y, z scaled (all three computed, then stored x, y, z). Returns the float length.
float __fastcall Vec3_NormalizeInPlace(float* v)
{
    const double xx = static_cast<double>(v[0]) * v[0];
    const double zz = static_cast<double>(v[2]) * v[2];
    const double yy = static_cast<double>(v[1]) * v[1];
    const double len = std::sqrt((xx + zz) + yy);
    const float lenf = static_cast<float>(len);
    std::uint32_t b;
    std::memcpy(&b, &lenf, 4);
    if ((b & 0x7fffffffu) == 0) return lenf;  // CONFIRMED-BINARY: TEST [len], 0x7fffffff at 0x00402f8a
    const double inv = 1.0 / len;             // CONFIRMED-BINARY: FLD1; FDIVRP at 0x00402f93
    const double x = inv * v[0], y = inv * v[1], z = inv * v[2];
    v[0] = static_cast<float>(x);
    v[1] = static_cast<float>(y);
    v[2] = static_cast<float>(z);
    return lenf;
}

// 0x004016a0 NetNode_PickRandomLink - count nonnull links (3 at node+0xc); 1 -> out=0; 2 -> out=0; else out=rand()%n; if out==avoid -> (out+1)%n; returns 1; ret 8
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall NetNode_PickRandomLink(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [edx]
        push esi
        xor esi, esi
        add eax, 0xc
        mov ecx, 0x3
    L_4016ad:
        cmp dword ptr [eax], 0x0
        jz L_4016b3
        inc esi
    L_4016b3:
        add eax, 0x4
        dec ecx
        jnz L_4016ad
        cmp esi, 0x1
        jnz L_4016ce
        mov eax, dword ptr [esp + 0x8]
        mov dword ptr [eax], 0x0
        mov eax, esi
        pop esi
        ret 0x8
    L_4016ce:
        cmp esi, 0x2
        jnz L_4016df
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [ecx], 0x0
        jmp L_4016ee
    L_4016df:
        call dword ptr [g_Iat_rand_004cc5d8]
        cdq
        idiv esi
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [ecx], edx
    L_4016ee:
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [esp + 0xc]
        cmp eax, edx
        jnz L_4016fe
        inc eax
        cdq
        idiv esi
        mov dword ptr [ecx], edx
    L_4016fe:
        mov eax, 0x1
        pop esi
        ret 0x8
    }
}

// 0x00401c60 AiVeh_EnterState1 - if [0x004f36ac]==0: prev [+0xf88]=[+0xf84]; [+0xfa8]=t, [+0xfac]=t+[+0xf94]; [+0xf84]=1; [+0xfb8..]=pos of link [+0xf80] of head; if [+0xff0]==2: [+0xfc4]=self-player (y=0), 0x00402f60
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_EnterState1(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xc
        mov eax, dword ptr [ecx + 0x4]
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        push ebx
        push esi
        mov edx, dword ptr [ecx + 0x4]
        mov ecx, dword ptr [g_Data_004da000 + 0x196ac]
        test ecx, ecx
        push edi
        jnz L_401d43
        mov ecx, dword ptr [eax + 0xf84]
        fld dword ptr [eax + 0xf94]
        mov dword ptr [eax + 0xf88], ecx
        mov esi, dword ptr [g_Data_004da000 + 0x19760]
        mov dword ptr [eax + 0xfa8], esi
        cmp ecx, 0x1
        fadd dword ptr [g_Data_004da000 + 0x19760]
        fstp dword ptr [eax + 0xfac]
        jz L_401cbc
        mov dword ptr [eax + 0xf84], 0x1
    L_401cbc:
        mov ecx, dword ptr [eax + 0xf80]
        mov esi, dword ptr [eax + 0xf78]
        lea edi, [eax + 0xfb8]
        mov ecx, dword ptr [esi + ecx*0x4 + 0xc]
        mov esi, dword ptr [ecx]
        mov dword ptr [edi], esi
        mov esi, dword ptr [ecx + 0x4]
        mov dword ptr [edi + 0x4], esi
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [edi + 0x8], ecx
        mov ecx, dword ptr [eax + 0xff0]
        sub ecx, 0x0
        jz L_401d43
        sub ecx, 0x2
        jnz L_401d43
        add edx, 0x3ec
        lea esi, [eax + 0xfc4]
        mov dword ptr [ebp - 0x8], edx
        lea edx, [eax + 0x3ec]
        mov dword ptr [ebp - 0x4], esi
        mov dword ptr [ebp - 0xc], edx
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
        mov ecx, esi
        mov dword ptr [eax + 0xfc8], 0x0
        call Vec3_NormalizeInPlace
        fstp st(0)
    L_401d43:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00402080 AiVeh_CopyF88ToF84 - [[ECX+4]+0xf84]=[[ECX+4]+0xf88]; ret
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_CopyF88ToF84(int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        mov ecx, dword ptr [eax + 0xf88]
        mov dword ptr [eax + 0xf84], ecx
        ret
    }
}

// 0x00402ff0 NetGraph_AllocAndAppend - malloc(0x58) zeroed; append to list head 0x004e5c58 tail 0x004e5c5c via +0x54; returns it
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_AllocAndAppend(int, int)
{
    __asm {
        push edi
        push 0x58
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov edx, eax
        mov ecx, 0x16
        xor eax, eax
        mov edi, edx
        rep stosd
        mov eax, dword ptr [g_Data_004da000 + 0xbc58]
        add esp, 0x4
        test eax, eax
        jz L_403024
        mov eax, dword ptr [g_Data_004da000 + 0xbc5c]
        mov dword ptr [eax + 0x54], edx
        mov dword ptr [g_Data_004da000 + 0xbc5c], edx
        mov eax, edx
        pop edi
        ret
    L_403024:
        mov dword ptr [g_Data_004da000 + 0xbc58], edx
        mov dword ptr [g_Data_004da000 + 0xbc5c], edx
        mov eax, edx
        pop edi
        ret
    }
}

// 0x00403510 NetGraph_LookupByIndex - Walks net-graph list [0x004e5c58] (next +0x54) returning the graph whose [0] == ECX, else 0.
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_LookupByIndex(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0xbc58]
        test eax, eax
        jz L_403524
    L_403519:
        cmp dword ptr [eax], ecx
        jz L_403526
        mov eax, dword ptr [eax + 0x54]
        test eax, eax
        jnz L_403519
    L_403524:
        xor eax, eax
    L_403526:
        ret
    }
}

// 0x00403530 NetNode_FindById - EDX list head; walk next +0x2c until [+0x28]==ECX; returns node or 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetNode_FindById(int, int)
{
    __asm {
        mov eax, edx
        test eax, eax
        jz L_403542
    L_403536:
        cmp dword ptr [eax + 0x28], ecx
        jz L_403544
        mov eax, dword ptr [eax + 0x2c]
        test eax, eax
        jnz L_403536
    L_403542:
        xor eax, eax
    L_403544:
        ret
    }
}

// 0x00403620 NetEdge_Init - thiscall(p0 xyz, p1 xyz, r): e.dir=p1-p0; e[3]=max(r*0.5, |xz|-r); normalize xz; then 0x004745c0, 0x00474f40(45.0f), 0x00474f40(-45.0f); e[0xd]=r; ret 0x1c
// Register/stack shape from the listing (ECX, EDX, 28 stack bytes).
__declspec(naked) int __fastcall NetEdge_Init(int, int, int, int, int, int, int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xc
        push ebx
        push esi
        mov esi, ecx
        lea eax, [ebp + 0x8]
        lea ecx, [ebp + 0x14]
        push edi
        mov dword ptr [ebp - 0x4], esi
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0xc], ecx
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
        mov ecx, dword ptr [ebp - 0x4]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        faddp st(1), st(0)
        fsqrt
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [ebp + 0x20]
        fmul dword ptr [g_RData_004cc000 + 0x850]
        fld dword ptr [ebp - 0xc]
        fsub dword ptr [ebp + 0x20]
        fcomp
        fnstsw AX
        test AH, 0x41
        jnz L_4036a5
        fstp st(0)
        mov ecx, dword ptr [ebp - 0x4]
        fld dword ptr [ecx]
        fmul dword ptr [ecx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [ecx + 0x8]
        faddp st(1), st(0)
        fsqrt
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [ebp - 0xc]
        fsub dword ptr [ebp + 0x20]
    L_4036a5:
        fstp dword ptr [esi + 0xc]
        mov edx, esi
        mov ecx, esi
        call Math_NormalizeHorizontalVector
        lea edi, [esi + 0x10]
        mov ecx, esi
        mov edx, edi
        call Vec3_PerpXZ
        mov edx, edi
        lea ecx, [esi + 0x1c]
        push 0x42340000
        call Vec3_RotateY
        mov edx, edi
        lea ecx, [esi + 0x28]
        push 0xc2340000
        call Vec3_RotateY
        mov edx, dword ptr [ebp + 0x20]
        mov dword ptr [esi + 0x34], edx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x1c
    }
}

// 0x004036f0 NetGraph_FindNearestNode - ECX=point, EDX=node list (next +0x2c): returns the node with the smallest distance (0x00472670) from the point, starting from -1.0 = none.
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_FindNearestNode(int, int)
{
    __asm {
        push ecx
        push ebx
        push esi
        push edi
        mov esi, edx
        xor edi, edi
        mov ebx, ecx
        test esi, esi
        mov dword ptr [esp + 0xc], 0xbf800000
        jz L_40373c
    L_403706:
        mov edx, esi
        mov ecx, ebx
        call Vec3_DistanceSquared
        fcom dword ptr [esp + 0xc]
        fnstsw AX
        test AH, 0x1
        jnz L_40372b
        fld dword ptr [esp + 0xc]
        fcomp dword ptr [g_RData_004cc000 + 0x858]
        fnstsw AX
        test AH, 0x1
        jz L_403733
    L_40372b:
        fstp dword ptr [esp + 0xc]
        mov edi, esi
        jmp L_403735
    L_403733:
        fstp st(0)
    L_403735:
        mov esi, dword ptr [esi + 0x2c]
        test esi, esi
        jnz L_403706
    L_40373c:
        mov eax, edi
        pop edi
        pop esi
        pop ebx
        pop ecx
        ret
    }
}

// 0x00403750 NetGraph_LinkVehiclesOnSamePath - For every pair of player entries (list [0x004f3a7c]) sharing net path index v+0xf70 where the later vehicle's state v+0x60 != 4 and its group link [3] points to itself: splices it into the earlier entry's circular group (entry[3]).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_LinkVehiclesOnSamePath(int, int)
{
    __asm {
        mov edx, dword ptr [g_Data_004da000 + 0x19a7c]
        test edx, edx
        jz L_4037b4
        push edi
        push esi
        mov edi, 0x4
    L_403761:
        mov eax, dword ptr [edx + 0x4]
        test edx, edx
        mov esi, dword ptr [eax + 0xf70]
        jz L_403772
        mov eax, dword ptr [edx]
        jmp L_403774
    L_403772:
        xor eax, eax
    L_403774:
        test eax, eax
        jz L_4037a4
    L_403778:
        mov ecx, dword ptr [eax + 0x4]
        cmp dword ptr [ecx + 0xf70], esi
        jnz L_403796
        cmp dword ptr [ecx + 0x60], edi
        jz L_403796
        cmp dword ptr [eax + 0xc], eax
        jnz L_403796
        mov ecx, dword ptr [edx + 0xc]
        mov dword ptr [eax + 0xc], ecx
        mov dword ptr [edx + 0xc], eax
    L_403796:
        test eax, eax
        jz L_40379e
        mov eax, dword ptr [eax]
        jmp L_4037a0
    L_40379e:
        xor eax, eax
    L_4037a0:
        test eax, eax
        jnz L_403778
    L_4037a4:
        test edx, edx
        jz L_4037ac
        mov edx, dword ptr [edx]
        jmp L_4037ae
    L_4037ac:
        xor edx, edx
    L_4037ae:
        test edx, edx
        jnz L_403761
        pop esi
        pop edi
    L_4037b4:
        ret
    }
}

// 0x004037c0 NetNode_Free - free 3 ptrs at +0x18..+0x20 if nonnull, then free(node)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetNode_Free(int, int)
{
    __asm {
        push ebp
        mov ebp, ecx
        test ebp, ebp
        jz L_4037f3
        push edi
        push esi
        push ebx
        mov ebx, dword ptr [g_Iat_free_004cc5b4]
        lea esi, [ebp + 0x18]
        mov edi, 0x3
    L_4037d8:
        mov eax, dword ptr [esi]
        test eax, eax
        jz L_4037e4
        push eax
        call ebx
        add esp, 0x4
    L_4037e4:
        add esi, 0x4
        dec edi
        jnz L_4037d8
        push ebp
        call ebx
        add esp, 0x4
        pop ebx
        pop esi
        pop edi
    L_4037f3:
        pop ebp
        ret
    }
}

// 0x00401580 AiVeh_AdvanceToLinkedNode - EDX=&cur; next=cur.link[+0xf80]; [+0xf78]=next; if cur id<0 (temp node): NetNode_Free(cur), cur=next, pick link via NetNode_PickRandomLink(-1) (0 if next id<0), mode [[+0xf74]+0x18]==1 -> [+0xf84]=2; else cur=next, find back-link index to old id (4 if none) and PickRandomLink avoiding it. out1=edge 
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall AiVeh_AdvanceToLinkedNode(int, int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x10
        push ebx
        push esi
        push edi
        mov edi, dword ptr [ecx + 0x4]
        mov esi, edx
        mov dword ptr [ebp - 0xc], ecx
        mov eax, dword ptr [edi + 0xf80]
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [ecx + eax*0x4 + 0xc]
        mov dword ptr [edi + 0xf78], edx
        mov ecx, dword ptr [esi]
        mov ebx, dword ptr [ecx + 0x28]
        test ebx, ebx
        jge L_401600
        call NetNode_Free
        mov eax, dword ptr [edi + 0xf78]
        mov dword ptr [esi], eax
        mov ecx, dword ptr [eax + 0x28]
        test ecx, ecx
        jge L_4015cf
        mov dword ptr [edi + 0xf80], 0x0
        jmp L_401650
    L_4015cf:
        mov ecx, dword ptr [ebp - 0xc]
        lea edx, [ebp - 0x4]
        push -0x1
        push edx
        mov edx, esi
        call NetNode_PickRandomLink
        mov eax, dword ptr [ebp - 0x4]
        mov ecx, dword ptr [edi + 0xf74]
        mov dword ptr [edi + 0xf80], eax
        cmp dword ptr [ecx + 0x18], 0x1
        jnz L_401650
        mov dword ptr [edi + 0xf84], 0x2
        jmp L_401650
    L_401600:
        mov dword ptr [esi], edx
        mov dword ptr [ebp - 0x10], 0x4
        mov dword ptr [ebp - 0x8], 0x0
        mov eax, 0xc
    L_401615:
        mov ecx, dword ptr [eax + edx*0x1]
        test ecx, ecx
        jz L_401621
        cmp dword ptr [ecx + 0x28], ebx
        jz L_401635
    L_401621:
        mov ecx, dword ptr [ebp - 0x8]
        add eax, 0x4
        inc ecx
        cmp eax, 0x18
        mov dword ptr [ebp - 0x8], ecx
        jl L_401615
        mov eax, dword ptr [ebp - 0x10]
        jmp L_401638
    L_401635:
        mov eax, dword ptr [ebp - 0x8]
    L_401638:
        mov ecx, dword ptr [ebp - 0xc]
        lea edx, [ebp - 0x4]
        push eax
        push edx
        mov edx, esi
        call NetNode_PickRandomLink
        mov eax, dword ptr [ebp - 0x4]
        mov dword ptr [edi + 0xf80], eax
    L_401650:
        mov ecx, dword ptr [edi + 0xf80]
        mov edx, dword ptr [esi]
        add edi, 0x3ec
        mov eax, dword ptr [edx + ecx*0x4 + 0x18]
        mov ecx, dword ptr [ebp + 0x8]
        mov dword ptr [ebp - 0x10], edi
        mov dword ptr [ecx], eax
        mov edx, dword ptr [esi]
        mov dword ptr [ebp + 0x8], edx
        mov ebx, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp + 0x8]
        mov edx, dword ptr [ebp + 0xc]
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
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x8
    }
}

// 0x00401c00 AiList_ResetNonIdle - if [0x004f36ac]==0: for each node in ring (+0xc) with veh[+0xf84]!=1: 0x00401c60, [+0xfdc]=1, [+0xfe4]=[0x0056b428]+10.0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiList_ResetNonIdle(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x196ac]
        push esi
        push edi
        mov edi, ecx
        test eax, eax
        mov esi, dword ptr [edi + 0xc]
        jnz L_401c52
        cmp esi, edi
        jz L_401c52
        push ebx
        mov ebx, 0x1
    L_401c1a:
        mov eax, dword ptr [esi + 0x4]
        cmp dword ptr [eax + 0xf84], ebx
        jz L_401c4a
        mov ecx, esi
        call AiVeh_EnterState1
        mov ecx, dword ptr [esi + 0x4]
        mov dword ptr [ecx + 0xfdc], ebx
        mov edx, dword ptr [esi + 0x4]
        fld dword ptr [g_Data_004da000 + 0x91428]
        fsub dword ptr [g_RData_004cc000 + 0x828]
        fstp dword ptr [edx + 0xfe4]
    L_401c4a:
        mov esi, dword ptr [esi + 0xc]
        cmp esi, edi
        jnz L_401c1a
        pop ebx
    L_401c52:
        pop edi
        pop esi
        ret
    }
}

// 0x00401f60 AiVeh_PushReturnNode - if 0x00472670() >= 400.0: node=malloc(0x30) zeroed at self pos, id=-1, [+0xc]=EDX, edge malloc(0x3c) NetEdge_Init(node, head, 10.0); push as new head [+0xf78], [+0xf80]=0, [+0xfd0]=t+1.0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_PushReturnNode(int, int)
{
    __asm {
        sub esp, 0x1c
        push ebx
        mov ebx, dword ptr [ecx + 0x4]
        push ebp
        mov dword ptr [esp + 0x8], edx
        lea ebp, [ebx + 0x3ec]
        mov eax, ebp
        mov ecx, dword ptr [eax]
        mov dword ptr [esp + 0x18], ecx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [esp + 0x1c], edx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [esp + 0x20], eax
        mov ecx, dword ptr [ebx + 0xf78]
        mov edx, dword ptr [ecx]
        mov dword ptr [esp + 0xc], edx
        lea edx, [esp + 0xc]
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [esp + 0x10], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [esp + 0x14], ecx
        lea ecx, [esp + 0x18]
        call Vec3_DistanceSquared
        fcomp dword ptr [g_RData_004cc000 + 0x82c]
        fnstsw AX
        test AH, 0x1
        jnz L_402077
        push edi
        push esi
        push 0x30
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov edx, dword ptr [esp + 0x14]
        mov esi, eax
        mov ecx, 0xc
        xor eax, eax
        mov edi, esi
        add esp, 0x4
        rep stosd
        mov dword ptr [esi + 0xc], edx
        mov ecx, dword ptr [ebp]
        mov eax, esi
        push 0x3c
        mov dword ptr [eax], ecx
        mov edx, dword ptr [ebp + 0x4]
        mov dword ptr [eax + 0x4], edx
        mov ecx, dword ptr [ebp + 0x8]
        mov dword ptr [esi + 0x28], 0xffffffff
        mov dword ptr [eax + 0x8], ecx
        call dword ptr [g_Iat_malloc_004cc5dc]
        add esp, 0x4
        mov edi, eax
        mov ecx, 0xf
        xor eax, eax
        mov dword ptr [esi + 0x18], edi
        push 0x41200000
        rep stosd
        mov edx, dword ptr [ebx + 0xf78]
        sub esp, 0xc
        mov eax, esp
        sub esp, 0xc
        mov ecx, dword ptr [edx]
        mov dword ptr [eax], ecx
        mov ecx, dword ptr [edx + 0x4]
        mov dword ptr [eax + 0x4], ecx
        mov ecx, esp
        mov edx, dword ptr [edx + 0x8]
        mov dword ptr [eax + 0x8], edx
        mov eax, esi
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], eax
        mov ecx, dword ptr [esi + 0x18]
        call NetEdge_Init
        mov dword ptr [ebx + 0xf78], esi
        mov dword ptr [ebx + 0xf80], 0x0
        fld dword ptr [g_Data_004da000 + 0x19760]
        fsub dword ptr [g_RData_004cc000 + 0x830]
        pop esi
        pop edi
        fstp dword ptr [ebx + 0xfd0]
    L_402077:
        pop ebp
        pop ebx
        add esp, 0x1c
        ret
    }
}

// 0x00402f10 AiNet_ResetPlayerVehicles - For each entry of player list [0x004f3a7c]: if vehicle [e+4] type +0x60 == 2 and +0xf84 == 1 calls 0x00402080 on the entry. Sets [0x004f36ac] = 1.
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiNet_ResetPlayerVehicles(int, int)
{
    __asm {
        push ebx
        push esi
        mov esi, dword ptr [g_Data_004da000 + 0x19a7c]
        mov ebx, 0x1
        test esi, esi
        jz L_402f4d
        push edi
        mov edi, 0x2
    L_402f27:
        mov eax, dword ptr [esi + 0x4]
        cmp dword ptr [eax + 0x60], edi
        jnz L_402f3e
        cmp dword ptr [eax + 0xf84], ebx
        jnz L_402f3e
        mov ecx, esi
        call AiVeh_CopyF88ToF84
    L_402f3e:
        test esi, esi
        jz L_402f46
        mov esi, dword ptr [esi]
        jmp L_402f48
    L_402f46:
        xor esi, esi
    L_402f48:
        test esi, esi
        jnz L_402f27
        pop edi
    L_402f4d:
        mov dword ptr [g_Data_004da000 + 0x196ac], ebx
        pop esi
        pop ebx
        ret
    }
}

// 0x00403550 NetGraph_LinkNodes - for each node (next +0x2c): 3 link ids at +0xc; id<0 -> clear link and edge; else link=NetNode_FindById, edge=malloc(0x3c) zeroed, NetEdge_Init(node pos, link pos, arg)
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall NetGraph_LinkNodes(int, int, int)
{
    __asm {
        sub esp, 0x18
        mov edx, ecx
        test edx, edx
        mov dword ptr [esp + 0x8], edx
        mov dword ptr [esp], edx
        jz L_403619
        push edi
        push esi
        push ebp
        push ebx
    L_403569:
        mov esi, dword ptr [esp + 0x10]
        mov dword ptr [esp + 0x14], 0x3
        mov eax, esi
        add esi, 0xc
        mov ebx, dword ptr [eax]
        mov ebp, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0x8]
        mov dword ptr [esp + 0x24], ecx
    L_403586:
        mov ecx, dword ptr [esi]
        test ecx, ecx
        jl L_4035e7
        call NetNode_FindById
        mov dword ptr [esi], eax
        push 0x3c
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov edx, dword ptr [esp + 0x30]
        add esp, 0x4
        mov edi, eax
        mov ecx, 0xf
        xor eax, eax
        mov dword ptr [esi + 0xc], edi
        push edx
        rep stosd
        mov eax, dword ptr [esi]
        sub esp, 0xc
        mov ecx, esp
        sub esp, 0xc
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ecx + 0x4], edx
        mov edx, dword ptr [esp + 0x40]
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ecx + 0x8], eax
        mov ecx, esp
        mov dword ptr [ecx], ebx
        mov dword ptr [ecx + 0x4], ebp
        mov dword ptr [ecx + 0x8], edx
        mov ecx, dword ptr [esi + 0xc]
        call NetEdge_Init
        mov edx, dword ptr [esp + 0x18]
        jmp L_4035f4
    L_4035e7:
        mov dword ptr [esi], 0x0
        mov dword ptr [esi + 0xc], 0x0
    L_4035f4:
        mov eax, dword ptr [esp + 0x14]
        add esi, 0x4
        dec eax
        mov dword ptr [esp + 0x14], eax
        jnz L_403586
        mov eax, dword ptr [esp + 0x10]
        mov eax, dword ptr [eax + 0x2c]
        test eax, eax
        mov dword ptr [esp + 0x10], eax
        jnz L_403569
        pop ebx
        pop ebp
        pop esi
        pop edi
    L_403619:
        add esp, 0x18
        ret 0x4
    }
}

// 0x00403800 NetGraph_Free - ECX graph: free each node in list +0x50 (next +0x2c) via 0x004037c0, then free(graph)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_Free(int, int)
{
    __asm {
        push edi
        mov edi, ecx
        test edi, edi
        jz L_403828
        push esi
        mov esi, dword ptr [edi + 0x50]
        test esi, esi
        jz L_40381d
    L_40380f:
        mov ecx, esi
        mov esi, dword ptr [esi + 0x2c]
        call NetNode_Free
        test esi, esi
        jnz L_40380f
    L_40381d:
        push edi
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        pop esi
    L_403828:
        pop edi
        ret
    }
}

// 0x00403830 AiVeh_PopNegativeRouteNodes - q=[ECX+4]; while head [q+0xf78] has [+0x28]<0: head=[+0xc], NetNode_Free(old)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_PopNegativeRouteNodes(int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [ecx + 0x4]
        mov ecx, dword ptr [esi + 0xf78]
        test ecx, ecx
        jz L_403860
        mov eax, dword ptr [ecx + 0x28]
        test eax, eax
        jge L_403860
    L_403845:
        mov eax, dword ptr [ecx + 0xc]
        mov dword ptr [esi + 0xf78], eax
        call NetNode_Free
        mov ecx, dword ptr [esi + 0xf78]
        mov eax, dword ptr [ecx + 0x28]
        test eax, eax
        jl L_403845
    L_403860:
        pop esi
        ret
    }
}

// 0x00403040 NetGraph_LoadFromZrdByIndex - sprintf net_%02d -> %s.zrd, open via Settings_OpenDetailPresetFile; version must be 0x69 (105) else error ai_net.cpp line 0x8c; mode byte +0x18 0..3; keys path_width->+0x1c (default 10.0), activate_rad->+0x20, attack_rad->+0x24, attack_dwell->+0x28, not_pursuit_dwell->+0x2c, pursuit_params/pursuit_r
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_LoadFromZrdByIndex(int, int)
{
    __asm {
        sub esp, 0x134
        push ebx
        push ebp
        push esi
        mov esi, ecx
        push edi
        mov edi, dword ptr [g_Iat_sprintf_004cc5c4]
        push esi
        lea eax, [esp + 0x34]
        push offset g_Data_004da000 + 0x23c
        push eax
        call edi
        add esp, 0xc
        lea ecx, [esp + 0x30]
        lea edx, [esp + 0x40]
        push ecx
        push offset g_Data_004da000 + 0x234
        push edx
        call edi
        add esp, 0xc
        xor edx, edx
        lea ecx, [esp + 0x40]
        push 0x0
        call ConfigTree_ParseFileByBasename
        mov ebp, eax
        test ebp, ebp
        mov dword ptr [esp + 0x10], ebp
        jnz L_403098
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x134
        ret
    L_403098:
        mov edx, offset g_Data_004da000 + 0x22c
        mov ecx, ebp
        call ConfigTree_FindChild
        test eax, eax
        jz L_4030da
        mov eax, dword ptr [eax + 0x4]
        cmp dword ptr [eax + 0xc], 0x69
        jz L_4030da
        push offset g_Data_004da000 + 0x208
        push 0x8c
        push offset g_Data_004da000 + 0x1e8
        push 0x200
        call Debug_ReportNoop
        add esp, 0x10
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x134
        ret
    L_4030da:
        call NetGraph_AllocAndAppend
        mov ebx, eax
        mov edx, offset g_Data_004da000 + 0x1e0
        mov ecx, ebp
        mov dword ptr [esp + 0x14], ebx
        mov dword ptr [ebx], esi
        call ConfigTree_FindChild
        test eax, eax
        jz L_403102
        mov ecx, dword ptr [eax + 0x4]
        lea edx, [ebx + 0x4]
        mov edi, dword ptr [ecx + 0xc]
        jmp L_403109
    L_403102:
        lea edi, [esp + 0x30]
        lea edx, [ebx + 0x4]
    L_403109:
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, edx
        mov edx, offset g_Data_004da000 + 0x1d8
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        mov ecx, ebp
        call ConfigTree_FindChild
        test eax, eax
        jz L_4031e4
        mov ecx, dword ptr [eax + 0x4]
        xor eax, eax
        lea edx, [esp + 0x18]
        mov edi, dword ptr [ecx + 0xc]
        or ecx, 0xffffffff
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
        lea ecx, [esp + 0x18]
        push ecx
        call dword ptr [g_Iat__strupr_004cc5c8]
        mov esi, dword ptr [g_Iat_strncmp_004cc5cc]
        add esp, 0x4
        lea edx, [esp + 0x18]
        push 0x2
        push offset g_Data_004da000 + 0x1d4
        push edx
        call esi
        add esp, 0xc
        test eax, eax
        jz L_4031e4
        push 0x2
        lea eax, [esp + 0x1c]
        push offset g_Data_004da000 + 0x1d0
        push eax
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_4031a8
        mov dword ptr [ebx + 0x18], 0x1
        jmp L_4031eb
    L_4031a8:
        push 0x2
        lea ecx, [esp + 0x1c]
        push offset g_Data_004da000 + 0x1cc
        push ecx
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_4031c6
        mov dword ptr [ebx + 0x18], 0x2
        jmp L_4031eb
    L_4031c6:
        push 0x2
        lea edx, [esp + 0x1c]
        push offset g_Data_004da000 + 0x1c8
        push edx
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_4031eb
        mov dword ptr [ebx + 0x18], 0x3
        jmp L_4031eb
    L_4031e4:
        mov dword ptr [ebx + 0x18], 0x0
    L_4031eb:
        mov edx, offset g_Data_004da000 + 0x1bc
        mov ecx, ebp
        call ConfigTree_FindChild
        xor esi, esi
        cmp eax, esi
        jz L_403208
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [ebx + 0x1c], ecx
        jmp L_40320f
    L_403208:
        mov dword ptr [ebx + 0x1c], 0x41200000
    L_40320f:
        mov edx, offset g_Data_004da000 + 0x1ac
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_403228
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0xc]
        mov dword ptr [ebx + 0x20], eax
    L_403228:
        mov edx, offset g_Data_004da000 + 0x1a0
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_403241
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov dword ptr [ebx + 0x24], edx
    L_403241:
        mov edx, offset g_Data_004da000 + 0x190
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_40325a
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [ebx + 0x28], ecx
    L_40325a:
        mov edx, offset g_Data_004da000 + 0x180
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jnz L_40327a
        mov edx, offset g_Data_004da000 + 0x170
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_40328c
    L_40327a:
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        mov dword ptr [ebx + 0x30], ecx
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0x14]
        mov dword ptr [ebx + 0x34], eax
    L_40328c:
        mov edx, offset g_Data_004da000 + 0x15c
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_4032a5
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov dword ptr [ebx + 0x2c], edx
    L_4032a5:
        mov edx, offset g_Data_004da000 + 0x14c
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_4032be
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [ebx + 0x38], ecx
    L_4032be:
        mov edx, offset g_Data_004da000 + 0x140
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_4032e2
        mov edx, dword ptr [eax + 0x4]
        mov ecx, dword ptr [edx + 0xc]
        mov dword ptr [ebx + 0x3c], ecx
        mov edx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0x14]
        mov dword ptr [ebx + 0x40], eax
        jmp L_4032f0
    L_4032e2:
        mov dword ptr [ebx + 0x3c], 0x41000000
        mov dword ptr [ebx + 0x40], 0x40800000
    L_4032f0:
        mov edx, offset g_Data_004da000 + 0x130
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_40330b
        mov ecx, dword ptr [eax + 0x4]
        mov edx, dword ptr [ecx + 0xc]
        mov dword ptr [ebx + 0x48], edx
        jmp L_40330e
    L_40330b:
        mov dword ptr [ebx + 0x48], esi
    L_40330e:
        mov edx, offset g_Data_004da000 + 0x120
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_403329
        mov eax, dword ptr [eax + 0x4]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [ebx + 0x44], ecx
        jmp L_40332c
    L_403329:
        mov dword ptr [ebx + 0x48], esi
    L_40332c:
        mov edx, offset g_Data_004da000 + 0x110
        mov ecx, ebp
        call ConfigTree_FindChild
        cmp eax, esi
        jz L_40342e
        mov edx, dword ptr [eax + 0x4]
        or ecx, 0xffffffff
        xor eax, eax
        lea ebp, [esp + 0x18]
        mov edi, dword ptr [edx + 0xc]
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, ebp
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        lea ecx, [esp + 0x18]
        push ecx
        call dword ptr [g_Iat__strupr_004cc5c8]
        mov esi, dword ptr [g_Iat_strncmp_004cc5cc]
        add esp, 0x4
        lea edx, [esp + 0x18]
        push 0x3
        push offset g_Data_004da000 + 0x10c
        push edx
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_40339c
        mov dword ptr [ebx + 0x4c], 0x3
        jmp L_403431
    L_40339c:
        push 0x3
        lea eax, [esp + 0x1c]
        push offset g_Data_004da000 + 0x108
        push eax
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_4033ba
        mov dword ptr [ebx + 0x4c], 0x1
        jmp L_403431
    L_4033ba:
        push 0x3
        lea ecx, [esp + 0x1c]
        push offset g_Data_004da000 + 0x104
        push ecx
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_4033d4
        mov dword ptr [ebx + 0x4c], eax
        jmp L_403431
    L_4033d4:
        push 0x3
        lea edx, [esp + 0x1c]
        push offset g_Data_004da000 + 0x100
        push edx
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_4033f2
        mov dword ptr [ebx + 0x4c], 0x2
        jmp L_403431
    L_4033f2:
        push 0x3
        lea eax, [esp + 0x1c]
        push offset g_Data_004da000 + 0xfc
        push eax
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_403410
        mov dword ptr [ebx + 0x4c], 0x4
        jmp L_403431
    L_403410:
        push 0x3
        lea ecx, [esp + 0x1c]
        push offset g_Data_004da000 + 0xf8
        push ecx
        call esi
        add esp, 0xc
        test eax, eax
        jnz L_403431
        mov dword ptr [ebx + 0x4c], 0x5
        jmp L_403431
    L_40342e:
        mov dword ptr [ebx + 0x4c], esi
    L_403431:
        xor ebp, ebp
        xor ebx, ebx
    L_403435:
        push ebx
        lea edx, [esp + 0x3c]
        push offset g_Data_004da000 + 0xec
        push edx
        call dword ptr [g_Iat_sprintf_004cc5c4]
        mov ecx, dword ptr [esp + 0x1c]
        add esp, 0xc
        lea edx, [esp + 0x38]
        call ConfigTree_FindChild
        mov esi, eax
        test esi, esi
        jz L_4034d9
        push 0x30
        call dword ptr [g_Iat_malloc_004cc5dc]
        mov edx, eax
        add esp, 0x4
        mov ecx, 0xc
        xor eax, eax
        mov edi, edx
        test ebp, ebp
        rep stosd
        jnz L_403481
        mov eax, dword ptr [esp + 0x14]
        mov dword ptr [eax + 0x50], edx
        jmp L_403484
    L_403481:
        mov dword ptr [ebp + 0x2c], edx
    L_403484:
        mov dword ptr [edx + 0x28], ebx
        mov ecx, dword ptr [esi + 0x4]
        mov ebp, edx
        mov eax, dword ptr [ecx + 0xc]
        mov dword ptr [edx + 0x24], eax
        mov ecx, dword ptr [esi + 0x4]
        mov eax, dword ptr [ecx + 0x14]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [edx], ecx
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [eax + 0x14]
        mov eax, dword ptr [ecx + 0x14]
        mov dword ptr [edx + 0x4], eax
        mov ecx, dword ptr [esi + 0x4]
        mov eax, dword ptr [ecx + 0x14]
        mov ecx, dword ptr [eax + 0x1c]
        mov dword ptr [edx + 0x8], ecx
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [eax + 0x1c]
        mov eax, dword ptr [ecx + 0xc]
        mov dword ptr [edx + 0xc], eax
        mov ecx, dword ptr [esi + 0x4]
        mov eax, dword ptr [ecx + 0x1c]
        mov ecx, dword ptr [eax + 0x14]
        mov dword ptr [edx + 0x10], ecx
        mov eax, dword ptr [esi + 0x4]
        mov ecx, dword ptr [eax + 0x1c]
        mov eax, dword ptr [ecx + 0x1c]
        mov dword ptr [edx + 0x14], eax
    L_4034d9:
        inc ebx
        cmp ebx, 0x63
        jl L_403435
        mov esi, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esi + 0x1c]
        push ecx
        mov ecx, dword ptr [esi + 0x50]
        call NetGraph_LinkNodes
        mov ecx, dword ptr [esp + 0x10]
        call ConfigTree_Destroy
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x134
        ret
    }
}

// 0x00403870 NetGraph_FreeAll - Pops graphs off [0x004e5c58] (next +0x54) freeing each via 0x00403800 until empty.
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_FreeAll(int, int)
{
    __asm {
        mov ecx, dword ptr [g_Data_004da000 + 0xbc58]
        test ecx, ecx
        jz L_403891
    L_40387a:
        mov eax, dword ptr [ecx + 0x54]
        mov dword ptr [g_Data_004da000 + 0xbc58], eax
        call NetGraph_Free
        mov ecx, dword ptr [g_Data_004da000 + 0xbc58]
        test ecx, ecx
        jnz L_40387a
    L_403891:
        ret
    }
}

// 0x00402fd0 NetGraph_LoadAllForMission - Calls NetGraph_LoadFromZrdByIndex with ECX = 1..99.
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall NetGraph_LoadAllForMission(int, int)
{
    __asm {
        push esi
        mov esi, 0x1
    L_402fd6:
        mov ecx, esi
        call NetGraph_LoadFromZrdByIndex
        inc esi
        cmp esi, 0x64
        jl L_402fd6
        pop esi
        ret
    }
}

// 0x00401970 AiVeh_KeepRangeDrive - thiscall(fwd,cross,dist) ret 0xc: fwd<=0 -> throttle 0, steer sign(cross); else steer=cross, throttle 1.0 if dist>[[+0xf74]+0x34], -1.0 if dist<[[+0xf74]+0x30], else 0; copy to +0x7c/+0x80
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall AiVeh_KeepRangeDrive(int, int, int, int, int)
{
    __asm {
        fld dword ptr [esp + 0x4]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        mov ecx, dword ptr [ecx + 0x4]
        fnstsw AX
        test AH, 0x41
        jz L_4019c5
        fld dword ptr [esp + 0x8]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        mov dword ptr [ecx + 0x68], 0x0
        mov dword ptr [esp + 0x8], 0xffffffff
        fnstsw AX
        test AH, 0x1
        jnz L_4019ac
        mov dword ptr [esp + 0x8], 0x1
    L_4019ac:
        fild dword ptr [esp + 0x8]
        mov edx, dword ptr [ecx + 0x68]
        mov dword ptr [ecx + 0x7c], edx
        fstp dword ptr [ecx + 0x6c]
        mov eax, dword ptr [ecx + 0x6c]
        mov dword ptr [ecx + 0x80], eax
        ret 0xc
    L_4019c5:
        mov eax, dword ptr [esp + 0x8]
        mov edx, dword ptr [ecx + 0xf74]
        fld dword ptr [esp + 0xc]
        mov dword ptr [ecx + 0x6c], eax
        fcomp dword ptr [edx + 0x34]
        fnstsw AX
        test AH, 0x41
        jnz L_4019f9
        mov eax, dword ptr [ecx + 0x6c]
        mov dword ptr [ecx + 0x68], 0x3f800000
        mov edx, dword ptr [ecx + 0x68]
        mov dword ptr [ecx + 0x80], eax
        mov dword ptr [ecx + 0x7c], edx
        ret 0xc
    L_4019f9:
        fld dword ptr [esp + 0xc]
        fcomp dword ptr [edx + 0x30]
        fnstsw AX
        test AH, 0x1
        jz L_401a20
        mov eax, dword ptr [ecx + 0x6c]
        mov dword ptr [ecx + 0x68], 0xbf800000
        mov edx, dword ptr [ecx + 0x68]
        mov dword ptr [ecx + 0x80], eax
        mov dword ptr [ecx + 0x7c], edx
        ret 0xc
    L_401a20:
        mov eax, dword ptr [ecx + 0x6c]
        mov dword ptr [ecx + 0x68], 0x0
        mov edx, dword ptr [ecx + 0x68]
        mov dword ptr [ecx + 0x80], eax
        mov dword ptr [ecx + 0x7c], edx
        ret 0xc
    }
}

// 0x00402090 AiVeh_SteerTowardPlayerA - d=player[+0x3ec]-self[+0x3ec] (y=0), 0x00402f60(d); with fwd self+0x380 (x,z at +0/+8): cross=fz*dx-fx*dz; if dot<=0 cross=sign(+-1); +0x6c=cross, +0x68=0, +0x7c=0, +0x80=+0x6c
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_SteerTowardPlayerA(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x24
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        push ebx
        lea eax, [ebp - 0x18]
        push esi
        mov esi, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x4], eax
        mov eax, dword ptr [edx + 0x4]
        lea ecx, [esi + 0x3ec]
        add eax, 0x3ec
        mov dword ptr [ebp - 0x8], ecx
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
        lea ecx, [ebp - 0x18]
        mov dword ptr [ebp - 0x14], 0x0
        call Vec3_NormalizeInPlace
        lea ecx, [esi + 0x380]
        fstp st(0)
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0x24], edx
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x20], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [ebp - 0x1c], ecx
        fld dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x18]
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [ebp - 0x10]
        fsubp st(1), st(0)
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [ebp - 0x18]
        fld dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x10]
        faddp st(1), st(0)
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x41
        jz L_40214d
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        mov dword ptr [ebp - 0x4], 0xffffffff
        fnstsw AX
        test AH, 0x1
        jnz L_40214a
        mov dword ptr [ebp - 0x4], 0x1
    L_40214a:
        fild dword ptr [ebp - 0x4]
    L_40214d:
        fstp dword ptr [esi + 0x6c]
        mov edx, dword ptr [esi + 0x6c]
        xor eax, eax
        mov dword ptr [esi + 0x68], eax
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x80], edx
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00402170 AiVeh_SteerTowardPlayerB - byte-identical to 0x00402090 except store order (+0x80 before +0x68/+0x7c); separate call sites
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_SteerTowardPlayerB(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x24
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        push ebx
        lea eax, [ebp - 0x18]
        push esi
        mov esi, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x4], eax
        mov eax, dword ptr [edx + 0x4]
        lea ecx, [esi + 0x3ec]
        add eax, 0x3ec
        mov dword ptr [ebp - 0x8], ecx
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
        lea ecx, [ebp - 0x18]
        mov dword ptr [ebp - 0x14], 0x0
        call Vec3_NormalizeInPlace
        lea ecx, [esi + 0x380]
        fstp st(0)
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0x24], edx
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x20], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [ebp - 0x1c], ecx
        fld dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x18]
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [ebp - 0x10]
        fsubp st(1), st(0)
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [ebp - 0x18]
        fld dword ptr [ebp - 0x1c]
        fmul dword ptr [ebp - 0x10]
        faddp st(1), st(0)
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x41
        jz L_40222d
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        mov dword ptr [ebp - 0x4], 0xffffffff
        fnstsw AX
        test AH, 0x1
        jnz L_40222a
        mov dword ptr [ebp - 0x4], 0x1
    L_40222a:
        fild dword ptr [ebp - 0x4]
    L_40222d:
        fstp dword ptr [esi + 0x6c]
        mov edx, dword ptr [esi + 0x6c]
        xor eax, eax
        mov dword ptr [esi + 0x80], edx
        mov dword ptr [esi + 0x68], eax
        mov dword ptr [esi + 0x7c], eax
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004024a0 AiVeh_LeadTargetIntercept - EDX target; s=1/[[self+0x5e4]+0x24] (projectile speed); solve intercept time from rel pos +0x3ec and rel vel +0xa4 (fast sqrt 0x1fc00000); out=target+0x410 + t*relvel; if no solution out=target pos; out.y += (rand/32767-0.5)*2.0 jitter
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall AiVeh_LeadTargetIntercept(int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x38
        mov eax, dword ptr [ecx + 0x4]
        push ebx
        push esi
        mov esi, dword ptr [edx + 0x4]
        mov ecx, dword ptr [eax + 0x5e4]
        push edi
        lea edi, [esi + 0x3ec]
        mov edx, dword ptr [ecx]
        lea ecx, [ebp - 0x20]
        mov dword ptr [ebp - 0x8], ecx
        mov dword ptr [ebp - 0x10], edi
        fld dword ptr [edx + 0x24]
        fdivr dword ptr [g_RData_004cc000 + 0x814]
        lea edx, [eax + 0x3ec]
        mov dword ptr [ebp - 0xc], edx
        fstp dword ptr [ebp - 0x4]
        mov ebx, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp - 0xc]
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
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x20]
        lea ecx, [ebp - 0x38]
        add eax, 0xa4
        lea edx, [esi + 0xa4]
        mov dword ptr [ebp - 0x10], ecx
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 0x8], edx
        fstp dword ptr [ebp - 0x20]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x1c]
        fstp dword ptr [ebp - 0x1c]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x18]
        fstp dword ptr [ebp - 0x18]
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
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
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x38]
        lea eax, [ebp - 0x2c]
        lea ecx, [ebp - 0x2c]
        mov dword ptr [ebp - 0x10], eax
        mov dword ptr [ebp - 0xc], ecx
        fstp dword ptr [ebp - 0x2c]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x34]
        fstp dword ptr [ebp - 0x28]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x30]
        fstp dword ptr [ebp - 0x24]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x8]
        fld dword ptr [g_RData_004cc000 + 0x814]
        fsub dword ptr [ebp - 0x8]
        fst dword ptr [ebp - 0x8]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x41
        jz L_4025ce
        mov edx, dword ptr [ebp + 0x8]
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
        ret 0x4
    L_4025ce:
        lea ecx, [ebp - 0x20]
        lea edx, [ebp - 0x2c]
        mov dword ptr [ebp - 0x10], ecx
        mov dword ptr [ebp - 0xc], edx
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
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
        lea eax, [ebp - 0x20]
        lea ecx, [ebp - 0x20]
        mov dword ptr [ebp - 0x10], eax
        mov dword ptr [ebp - 0xc], ecx
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x14]
        fld dword ptr [ebp - 0x8]
        fmul dword ptr [ebp - 0x14]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x4]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x14]
        mov eax, dword ptr [ebp - 0x14]
        sar eax, 0x1
        add eax, 0x1fc00000
        mov dword ptr [ebp - 0x10], eax
        fld dword ptr [ebp - 0x10]
        fadd dword ptr [ebp - 0x4]
        lea edx, [ebp - 0x2c]
        add esi, 0x410
        mov dword ptr [ebp - 0x14], edx
        mov dword ptr [ebp - 0x10], esi
        fdiv dword ptr [ebp - 0x8]
        fld st(0)
        fmul dword ptr [ebp - 0x38]
        fstp dword ptr [ebp - 0x2c]
        fld st(0)
        fmul dword ptr [ebp - 0x34]
        fstp dword ptr [ebp - 0x28]
        fmul dword ptr [ebp - 0x30]
        fstp dword ptr [ebp - 0x24]
        mov ebx, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp - 0x14]
        mov edx, dword ptr [ebp + 0x8]
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
        mov esi, dword ptr [ebp + 0x8]
        call dword ptr [g_Iat_rand_004cc5d8]
        mov dword ptr [ebp + 0x8], eax
        pop edi
        fild dword ptr [ebp + 0x8]
        fmul dword ptr [g_RData_004cc000 + 0x838]
        fsub dword ptr [g_RData_004cc000 + 0x820]
        fmul dword ptr [g_RData_004cc000 + 0x83c]
        fsubr dword ptr [esi + 0x4]
        fstp dword ptr [esi + 0x4]
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x004026d0 AiVeh_CircleStrafeTarget - EDX target; r=[[self+0xf74]+0x30]; d=self-target (xz); aim=target+r*rot(d) with c=[0x004da0e4]=0.9659 s=[0x004da0e8]=0.2588 (initial values, 15 deg; .data globals may change); steer toward aim: ahead -> throttle max(1-|cross|,0.25), else throttle 0 turn sign; copy to +0x7c/+0x80
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_CircleStrafeTarget(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x34
        push ebx
        push esi
        mov esi, dword ptr [ecx + 0x4]
        mov edx, dword ptr [edx + 0x4]
        push edi
        mov eax, dword ptr [esi + 0xf74]
        lea edi, [edx + 0x3ec]
        mov dword ptr [ebp - 0xc], edi
        mov ecx, dword ptr [eax + 0x30]
        lea eax, [ebp - 0x28]
        mov dword ptr [ebp - 0x8], eax
        lea eax, [esi + 0x3ec]
        mov dword ptr [ebp - 0x4], ecx
        mov dword ptr [ebp - 0x10], eax
        mov ebx, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp - 0xc]
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
        lea ecx, [ebp - 0x28]
        mov dword ptr [ebp - 0x24], 0x0
        call Vec3_NormalizeInPlace
        fstp st(0)
        fld dword ptr [g_Data_004da000 + 0xe4]
        fmul dword ptr [ebp - 0x28]
        fld dword ptr [g_Data_004da000 + 0xe8]
        fmul dword ptr [ebp - 0x20]
        lea ecx, [ebp - 0x34]
        lea edx, [ebp - 0x1c]
        mov dword ptr [ebp - 0x18], 0x0
        mov dword ptr [ebp - 0x10], ecx
        fsubp st(1), st(0)
        fld dword ptr [g_Data_004da000 + 0xe4]
        fmul dword ptr [ebp - 0x20]
        fld dword ptr [g_Data_004da000 + 0xe8]
        fmul dword ptr [ebp - 0x28]
        mov dword ptr [ebp - 0xc], edx
        mov dword ptr [ebp - 0x8], edi
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x14]
        fld dword ptr [ebp - 0x4]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x1c]
        fstp st(0)
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x14]
        fstp dword ptr [ebp - 0x14]
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
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
        lea eax, [ebp - 0x1c]
        lea ecx, [ebp - 0x34]
        mov dword ptr [ebp - 0x10], eax
        lea eax, [esi + 0x3ec]
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 0x8], ecx
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
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
        mov dword ptr [ebp - 0x18], 0x0
        call Vec3_NormalizeInPlace
        lea edx, [ebp - 0x1c]
        lea eax, [esi + 0x380]
        fstp st(0)
        mov dword ptr [ebp - 0x10], edx
        mov dword ptr [ebp - 0xc], eax
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x8]
        lea ecx, [ebp - 0x1c]
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 0x10], ecx
        mov ebx, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp - 0x10]
        fld dword ptr [ebx + 0x8]
        fmul dword ptr [ecx]
        fld dword ptr [ebx]
        fmul dword ptr [ecx + 0x8]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x8]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x1
        jz L_402876
        fld dword ptr [ebp - 0x4]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        mov dword ptr [esi + 0x68], 0x0
        mov dword ptr [ebp - 0x4], 0xffffffff
        fnstsw AX
        test AH, 0x1
        jnz L_40286e
        mov dword ptr [ebp - 0x4], 0x1
    L_40286e:
        fild dword ptr [ebp - 0x4]
        fstp dword ptr [esi + 0x6c]
        jmp L_40289f
    L_402876:
        fld dword ptr [ebp - 0x4]
        fabs
        fsubr dword ptr [g_RData_004cc000 + 0x814]
        fcom dword ptr [g_RData_004cc000 + 0x818]
        fnstsw AX
        test AH, 0x41
        jz L_402896
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x818]
    L_402896:
        mov edx, dword ptr [ebp - 0x4]
        fstp dword ptr [esi + 0x68]
        mov dword ptr [esi + 0x6c], edx
    L_40289f:
        mov eax, dword ptr [esi + 0x68]
        mov ecx, dword ptr [esi + 0x6c]
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x80], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004028c0 AiVeh_FollowOffsetTarget - thiscall(dist) EDX target; r=[[+0xf74]+0x30], k=[[+0xf74]+0x34]; aim=target+r*dir(+0xfc4) + lateral k*clamp((2r-dist)/r,0,1), negated when [+0xb8]>0 and dist<2r (reverse); steer: ahead or dist>=10.0 -> throttle max(1-|cross|,0.25) else throttle -1.0 steer 0; reverse negates throttle; copy to +0x7c/+
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall AiVeh_FollowOffsetTarget(int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x3c
        push ebx
        push esi
        mov esi, dword ptr [ecx + 0x4]
        mov edx, dword ptr [edx + 0x4]
        add edx, 0x3ec
        push edi
        mov eax, dword ptr [esi + 0xf74]
        mov dword ptr [ebp - 0x18], edx
        mov ecx, dword ptr [eax + 0x30]
        mov eax, dword ptr [eax + 0x34]
        mov dword ptr [ebp - 0x4], ecx
        lea ecx, [esi + 0xfc4]
        fld dword ptr [ebp - 0x4]
        fadd st(0), st(0)
        mov dword ptr [ebp - 0xc], eax
        mov eax, dword ptr [ecx]
        mov dword ptr [ebp - 0x24], eax
        mov eax, dword ptr [ecx + 0x4]
        fstp dword ptr [ebp - 0x8]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x24]
        mov dword ptr [ebp - 0x20], eax
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [ebp - 0x1c], ecx
        lea eax, [ebp - 0x3c]
        lea ecx, [ebp - 0x3c]
        mov dword ptr [ebp - 0x10], eax
        fstp dword ptr [ebp - 0x3c]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x20]
        mov dword ptr [ebp - 0x14], ecx
        fstp dword ptr [ebp - 0x38]
        fld dword ptr [ebp - 0x4]
        fmul dword ptr [ebp - 0x1c]
        fstp dword ptr [ebp - 0x34]
        mov ebx, dword ptr [ebp - 0x18]
        mov ecx, dword ptr [ebp - 0x14]
        mov edx, dword ptr [ebp - 0x10]
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
        fld dword ptr [ebp - 0x24]
        fchs
        fld dword ptr [ebp - 0x8]
        fsub dword ptr [ebp + 0x8]
        fdiv dword ptr [ebp - 0x4]
        fcom dword ptr [g_RData_004cc000 + 0x814]
        fnstsw AX
        test AH, 0x41
        jnz L_402978
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x814]
        jmp L_40298d
    L_402978:
        fcom dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x1
        jz L_40298d
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x810]
    L_40298d:
        fld dword ptr [esi + 0xb8]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x41
        jnz L_4029d2
        fld dword ptr [ebp + 0x8]
        fcomp dword ptr [ebp - 0x8]
        fnstsw AX
        test AH, 0x1
        jz L_4029d2
        fmul dword ptr [ebp - 0xc]
        mov edi, 0x1
        fchs
        fld dword ptr [ebp - 0x1c]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x30]
        fld dword ptr [ebp - 0x38]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x2c]
        fxch st(1)
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x28]
        fstp st(0)
        jmp L_4029f0
    L_4029d2:
        fmul dword ptr [ebp - 0xc]
        fld dword ptr [ebp - 0x1c]
        fmul st(0), st(1)
        xor edi, edi
        fstp dword ptr [ebp - 0x30]
        fld dword ptr [ebp - 0x38]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x2c]
        fxch st(1)
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x28]
        fstp st(0)
    L_4029f0:
        lea eax, [ebp - 0x3c]
        lea ecx, [ebp - 0x30]
        lea edx, [ebp - 0x3c]
        mov dword ptr [ebp + 0x8], eax
        mov dword ptr [ebp - 0x18], ecx
        mov dword ptr [ebp - 0x14], edx
        mov ebx, dword ptr [ebp - 0x14]
        mov ecx, dword ptr [ebp - 0x18]
        mov edx, dword ptr [ebp + 0x8]
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
        lea eax, [ebp - 0x30]
        lea ecx, [esi + 0x3ec]
        lea edx, [ebp - 0x3c]
        mov dword ptr [ebp + 0x8], eax
        mov dword ptr [ebp - 0x18], ecx
        mov dword ptr [ebp - 0x14], edx
        mov ebx, dword ptr [ebp - 0x14]
        mov ecx, dword ptr [ebp - 0x18]
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
        lea ecx, [ebp - 0x30]
        mov dword ptr [ebp - 0x2c], 0x0
        call Vec3_NormalizeInPlace
        lea eax, [esi + 0x380]
        fstp dword ptr [ebp - 0xc]
        mov ecx, dword ptr [eax]
        mov dword ptr [ebp - 0x24], ecx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ebp - 0x20], edx
        mov eax, dword ptr [eax + 0x8]
        test edi, edi
        mov dword ptr [ebp - 0x1c], eax
        jz L_402a9a
        fld dword ptr [ebp - 0x24]
        fchs
        fstp dword ptr [ebp - 0x24]
        fld dword ptr [ebp - 0x1c]
        fchs
        fstp dword ptr [ebp - 0x1c]
    L_402a9a:
        lea ecx, [ebp - 0x30]
        lea edx, [ebp - 0x24]
        mov dword ptr [ebp + 0x8], ecx
        mov dword ptr [ebp - 0x18], edx
        mov ecx, dword ptr [ebp - 0x18]
        mov edx, dword ptr [ebp + 0x8]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x10]
        lea eax, [ebp - 0x30]
        lea ecx, [ebp - 0x24]
        mov dword ptr [ebp - 0x18], eax
        mov dword ptr [ebp - 0x14], ecx
        mov ebx, dword ptr [ebp - 0x14]
        mov ecx, dword ptr [ebp - 0x18]
        fld dword ptr [ebx + 0x8]
        fmul dword ptr [ecx]
        fld dword ptr [ebx]
        fmul dword ptr [ecx + 0x8]
        fsubp st(1), st(0)
        fstp dword ptr [ebp + 0x8]
        fld dword ptr [ebp - 0x10]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x1
        jz L_402b0c
        fld dword ptr [ebp - 0xc]
        fcomp dword ptr [g_RData_004cc000 + 0x81c]
        fnstsw AX
        test AH, 0x1
        jz L_402b0c
        mov dword ptr [esi + 0x68], 0xbf800000
        mov dword ptr [esi + 0x6c], 0x0
        jmp L_402b35
    L_402b0c:
        fld dword ptr [ebp + 0x8]
        fabs
        fsubr dword ptr [g_RData_004cc000 + 0x814]
        fcom dword ptr [g_RData_004cc000 + 0x818]
        fnstsw AX
        test AH, 0x41
        jz L_402b2c
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x818]
    L_402b2c:
        mov edx, dword ptr [ebp + 0x8]
        fstp dword ptr [esi + 0x68]
        mov dword ptr [esi + 0x6c], edx
    L_402b35:
        test edi, edi
        jz L_402b56
        fld dword ptr [esi + 0x68]
        mov ecx, dword ptr [esi + 0x6c]
        fchs
        fst dword ptr [esi + 0x68]
        fstp dword ptr [esi + 0x7c]
        mov dword ptr [esi + 0x80], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    L_402b56:
        mov eax, dword ptr [esi + 0x68]
        mov ecx, dword ptr [esi + 0x6c]
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x80], ecx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

// 0x00402be0 AiVeh_DriveToNextNode - target=[head+0xc] pos; d=target-self+0x3ec (y=0); dist=0x00402f60; if dist<5.0: head=next, zero +0x68/+0x6c/+0x7c/+0x80, +0xfa4=time+4.0; elif behind: turn sign; else throttle=max(1-|cross|,0.25) forward, steer=cross
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_DriveToNextNode(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x28
        push ebx
        push esi
        mov esi, dword ptr [ecx + 0x4]
        mov eax, dword ptr [esi + 0xf78]
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0x28], edx
        lea edx, [ebp - 0x1c]
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0x24], eax
        lea eax, [esi + 0x3ec]
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0x20], ecx
        lea ecx, [ebp - 0x28]
        mov dword ptr [ebp - 0xc], ecx
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
        lea ecx, [ebp - 0x1c]
        mov dword ptr [ebp - 0x18], 0x0
        call Vec3_NormalizeInPlace
        fcomp dword ptr [g_RData_004cc000 + 0x844]
        fnstsw AX
        test AH, 0x1
        jz L_402c91
        mov edx, dword ptr [esi + 0xf78]
        mov eax, dword ptr [edx + 0xc]
        mov dword ptr [esi + 0xf78], eax
        xor eax, eax
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x68], eax
        mov dword ptr [esi + 0x80], eax
        mov dword ptr [esi + 0x6c], eax
        fld dword ptr [g_Data_004da000 + 0x19760]
        fsub dword ptr [g_RData_004cc000 + 0x848]
        fstp dword ptr [esi + 0xfa4]
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_402c91:
        lea ecx, [ebp - 0x1c]
        lea eax, [esi + 0x380]
        mov dword ptr [ebp - 0xc], ecx
        mov dword ptr [ebp - 0x8], eax
        mov ecx, dword ptr [ebp - 0x8]
        mov edx, dword ptr [ebp - 0xc]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x10]
        lea edx, [ebp - 0x1c]
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0xc], edx
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        fld dword ptr [ebx + 0x8]
        fmul dword ptr [ecx]
        fld dword ptr [ebx]
        fmul dword ptr [ecx + 0x8]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x10]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x1
        jz L_402d1b
        fld dword ptr [ebp - 0x4]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        xor eax, eax
        mov dword ptr [ebp - 0x4], 0xffffffff
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x68], eax
        fnstsw AX
        test AH, 0x1
        jnz L_402d09
        mov dword ptr [ebp - 0x4], 0x1
    L_402d09:
        fild dword ptr [ebp - 0x4]
        fst dword ptr [esi + 0x80]
        fstp dword ptr [esi + 0x6c]
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_402d1b:
        fld dword ptr [ebp - 0x4]
        fabs
        fsubr dword ptr [g_RData_004cc000 + 0x814]
        fcom dword ptr [g_RData_004cc000 + 0x818]
        fnstsw AX
        test AH, 0x41
        jz L_402d3b
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x818]
    L_402d3b:
        mov eax, dword ptr [ebp - 0x4]
        fst dword ptr [esi + 0x7c]
        fstp dword ptr [esi + 0x68]
        mov ecx, eax
        mov dword ptr [esi + 0x80], eax
        mov dword ptr [esi + 0x6c], ecx
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00402d60 AiVeh_ReverseToNextNode - as 0x00402be0 with forward vector negated, throttle negative, arrival wait time+14.0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_ReverseToNextNode(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x34
        push ebx
        push esi
        mov esi, dword ptr [ecx + 0x4]
        mov eax, dword ptr [esi + 0xf78]
        mov ecx, dword ptr [eax + 0xc]
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0x34], edx
        lea edx, [ebp - 0x28]
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x4], edx
        mov dword ptr [ebp - 0x30], eax
        lea eax, [esi + 0x3ec]
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0x2c], ecx
        lea ecx, [ebp - 0x34]
        mov dword ptr [ebp - 0xc], ecx
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
        lea ecx, [ebp - 0x28]
        mov dword ptr [ebp - 0x24], 0x0
        call Vec3_NormalizeInPlace
        fcomp dword ptr [g_RData_004cc000 + 0x844]
        fnstsw AX
        test AH, 0x1
        jz L_402e11
        mov edx, dword ptr [esi + 0xf78]
        mov eax, dword ptr [edx + 0xc]
        mov dword ptr [esi + 0xf78], eax
        xor eax, eax
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x68], eax
        mov dword ptr [esi + 0x80], eax
        mov dword ptr [esi + 0x6c], eax
        fld dword ptr [g_Data_004da000 + 0x19760]
        fsub dword ptr [g_RData_004cc000 + 0x84c]
        fstp dword ptr [esi + 0xfa4]
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_402e11:
        lea ecx, [esi + 0x380]
        mov edx, dword ptr [esi + 0x380]
        mov dword ptr [ebp - 0x1c], edx
        lea edx, [ebp - 0x28]
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0xc], edx
        fld dword ptr [ebp - 0x1c]
        mov dword ptr [ebp - 0x18], eax
        mov ecx, dword ptr [ecx + 0x8]
        fchs
        fstp dword ptr [ebp - 0x1c]
        mov dword ptr [ebp - 0x14], ecx
        lea eax, [ebp - 0x1c]
        fld dword ptr [ebp - 0x14]
        fchs
        fstp dword ptr [ebp - 0x14]
        mov dword ptr [ebp - 0x8], eax
        mov ecx, dword ptr [ebp - 0x8]
        mov edx, dword ptr [ebp - 0xc]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x10]
        lea ecx, [ebp - 0x28]
        lea edx, [ebp - 0x1c]
        mov dword ptr [ebp - 0xc], ecx
        mov dword ptr [ebp - 0x8], edx
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        fld dword ptr [ebx + 0x8]
        fmul dword ptr [ecx]
        fld dword ptr [ebx]
        fmul dword ptr [ecx + 0x8]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        fld dword ptr [ebp - 0x10]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x1
        jz L_402ec6
        fld dword ptr [ebp - 0x4]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        xor eax, eax
        mov dword ptr [ebp - 0x4], 0xffffffff
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x68], eax
        fnstsw AX
        test AH, 0x1
        jnz L_402eb4
        mov dword ptr [ebp - 0x4], 0x1
    L_402eb4:
        fild dword ptr [ebp - 0x4]
        fst dword ptr [esi + 0x80]
        fstp dword ptr [esi + 0x6c]
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_402ec6:
        fld dword ptr [ebp - 0x4]
        fabs
        fsubr dword ptr [g_RData_004cc000 + 0x814]
        fcom dword ptr [g_RData_004cc000 + 0x818]
        fnstsw AX
        test AH, 0x41
        jz L_402ee6
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x818]
    L_402ee6:
        mov eax, dword ptr [ebp - 0x4]
        fchs
        fst dword ptr [esi + 0x7c]
        fstp dword ptr [esi + 0x68]
        mov ecx, eax
        mov dword ptr [esi + 0x80], eax
        mov dword ptr [esi + 0x6c], ecx
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00401060 AiVeh_StateTick - [+0xec0..]=player pos +0x410; switch [+0xf84]: 0 -> 0x00401180 then TryDetect; 1 -> AiVeh_CombatTick; 2 -> SteerTowardPlayerA, TryDetect, on hit PushReturnNode; 3 -> SteerTowardPlayerB, TryDetect; 4 -> AiVeh_RouteTick, TryDetect, on hit PushReturnNode; 5 -> +0x6c=0, if [+0x1018]==0 [+0xf84]=[+0xf8c]
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_StateTick(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        push esi
        push edi
        mov edi, ecx
        mov ecx, dword ptr [eax + 0x4]
        mov esi, dword ptr [edi + 0x4]
        add ecx, 0x410
        mov eax, dword ptr [ecx]
        lea edx, [esi + 0xec0]
        mov dword ptr [esi + 0xec0], eax
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [edx + 0x4], eax
        mov eax, dword ptr [esi + 0xf84]
        mov ecx, dword ptr [ecx + 0x8]
        cmp eax, 0x5
        mov dword ptr [edx + 0x8], ecx
        ja L_401164
        cmp eax, 0
        je L_4010a5
        cmp eax, 1
        je L_4010f4
        cmp eax, 2
        je L_4010fe
        cmp eax, 3
        je L_40112a
        cmp eax, 4
        je L_40113b
        cmp eax, 5
        je L_4010ae
        int 3  // unreachable: the bounds check above excludes other indices
    L_4010a5:
        mov ecx, edi
        call AiVeh_PatrolTick
        jmp L_4010cb
    L_4010ae:
        mov eax, dword ptr [esi + 0x1018]
        mov dword ptr [esi + 0x6c], 0x0
        test eax, eax
        jnz L_4010cb
        mov ecx, dword ptr [esi + 0xf8c]
        mov dword ptr [esi + 0xf84], ecx
    L_4010cb:
        mov ecx, edi
        call AiVeh_TryDetectPlayer
        test eax, eax
        jz L_401164
        mov edx, dword ptr [esi + 0xf80]
        mov eax, dword ptr [esi + 0xf78]
        mov ecx, edi
        mov edx, dword ptr [eax + edx*0x4 + 0xc]
        call AiVeh_PushReturnNode
        pop edi
        pop esi
        ret
    L_4010f4:
        mov ecx, edi
        call AiVeh_CombatTick
        pop edi
        pop esi
        ret
    L_4010fe:
        mov ecx, edi
        call AiVeh_SteerTowardPlayerA
        mov ecx, edi
        call AiVeh_TryDetectPlayer
        test eax, eax
        jz L_401164
        mov ecx, dword ptr [esi + 0xf80]
        mov edx, dword ptr [esi + 0xf78]
        mov edx, dword ptr [edx + ecx*0x4 + 0xc]
        mov ecx, edi
        call AiVeh_PushReturnNode
        pop edi
        pop esi
        ret
    L_40112a:
        mov ecx, edi
        call AiVeh_SteerTowardPlayerB
        mov ecx, edi
        call AiVeh_TryDetectPlayer
        pop edi
        pop esi
        ret
    L_40113b:
        mov ecx, edi
        call AiVeh_RouteTick
        mov ecx, edi
        call AiVeh_TryDetectPlayer
        test eax, eax
        jz L_401164
        mov eax, dword ptr [esi + 0xf80]
        mov ecx, dword ptr [esi + 0xf78]
        mov edx, dword ptr [ecx + eax*0x4 + 0xc]
        mov ecx, edi
        call AiVeh_PushReturnNode
    L_401164:
        pop edi
        pop esi
        ret
    }
}

// 0x00401180 AiVeh_PatrolTick - state 0 patrol: target = link [+0xf80] of head; if AiVeh_ProbeAhead(): AdvanceToLinkedNode, [+0xf8c]=[+0xf84], [+0xf84]=5, [+0x1018]=1, normalize dir (0x004727f0), zero controls. Else steer to node: ahead -> throttle max(1-|cross|,0.25), [+0xfec]=1; behind and [+0xfec] -> advance, recurse; else thro
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_PatrolTick(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x48
        push ebx
        push esi
        push edi
        mov edi, ecx
        mov eax, dword ptr [edi + 0x8]
        mov esi, dword ptr [edi + 0x4]
        mov ecx, dword ptr [eax + 0x4]
        mov eax, dword ptr [esi + 0xf78]
        mov dword ptr [ebp - 0x4], eax
        mov edx, dword ptr [esi + 0xf80]
        mov dword ptr [ebp - 0x20], ecx
        mov ecx, dword ptr [eax + edx*0x4 + 0x18]
        mov dword ptr [ebp - 0x8], ecx
        mov edx, dword ptr [esi + 0xf80]
        mov ecx, edi
        mov eax, dword ptr [eax + edx*0x4 + 0xc]
        mov dword ptr [ebp - 0xc], eax
        call AiVeh_ProbeAhead
        test eax, eax
        jz L_401264
        lea ecx, [ebp - 0x30]
        lea edx, [ebp - 0x8]
        push ecx
        push edx
        lea edx, [ebp - 0x4]
        mov ecx, edi
        call AiVeh_AdvanceToLinkedNode
        mov eax, dword ptr [esi + 0xf80]
        mov ecx, dword ptr [ebp - 0x4]
        mov edx, dword ptr [ecx + eax*0x4 + 0xc]
        mov eax, dword ptr [esi + 0xf84]
        mov dword ptr [ebp - 0xc], edx
        lea ecx, [ebp - 0x48]
        lea edx, [esi + 0x3ec]
        mov dword ptr [esi + 0xf8c], eax
        mov dword ptr [esi + 0xf84], 0x5
        mov dword ptr [esi + 0x1018], 0x1
        mov dword ptr [ebp - 0x10], ecx
        mov dword ptr [ebp - 0x14], edx
        mov ebx, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp - 0x14]
        mov edx, dword ptr [ebp - 0x10]
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
        lea edx, [esi + 0x1000]
        lea ecx, [ebp - 0x48]
        mov dword ptr [ebp - 0x44], 0x0
        call Math_NormalizeHorizontalVector
        xor ebx, ebx
        mov dword ptr [esi + 0x68], ebx
        mov dword ptr [esi + 0x7c], ebx
        mov dword ptr [esi + 0x6c], ebx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_401264:
        lea eax, [ebp - 0x30]
        lea ecx, [esi + 0x3ec]
        mov dword ptr [ebp - 0x14], eax
        mov dword ptr [ebp - 0x10], ecx
        mov ebx, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp - 0x14]
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
        lea ecx, [ebp - 0x30]
        mov dword ptr [ebp - 0x2c], 0x0
        call Vec3_NormalizeInPlace
        lea edx, [esi + 0x380]
        fstp dword ptr [ebp - 0x24]
        mov eax, dword ptr [edx]
        mov dword ptr [ebp - 0x3c], eax
        lea eax, [ebp - 0x30]
        mov ecx, dword ptr [edx + 0x4]
        mov dword ptr [ebp - 0x14], eax
        mov dword ptr [ebp - 0x38], ecx
        lea ecx, [ebp - 0x3c]
        mov edx, dword ptr [edx + 0x8]
        mov dword ptr [ebp - 0x10], ecx
        mov dword ptr [ebp - 0x34], edx
        mov ecx, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp - 0x14]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x1c]
        lea edx, [ebp - 0x30]
        lea eax, [ebp - 0x3c]
        mov dword ptr [ebp - 0x14], edx
        mov dword ptr [ebp - 0x18], eax
        mov ebx, dword ptr [ebp - 0x18]
        mov ecx, dword ptr [ebp - 0x14]
        fld dword ptr [ebx + 0x8]
        fmul dword ptr [ecx]
        fld dword ptr [ebx]
        fmul dword ptr [ecx + 0x8]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x1c]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        xor ebx, ebx
        fnstsw AX
        test AH, 0x1
        jz L_40136a
        cmp dword ptr [esi + 0xfec], ebx
        jz L_401341
        lea ecx, [ebp - 0x30]
        lea edx, [ebp - 0x8]
        push ecx
        push edx
        lea edx, [ebp - 0x4]
        mov ecx, edi
        call AiVeh_AdvanceToLinkedNode
        mov ecx, edi
        mov dword ptr [esi + 0xfec], ebx
        call AiVeh_PatrolTick
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_401341:
        fld dword ptr [ebp - 0x10]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        mov dword ptr [esi + 0x68], ebx
        mov dword ptr [ebp - 0x10], 0xffffffff
        fnstsw AX
        test AH, 0x1
        jnz L_401362
        mov dword ptr [ebp - 0x10], 0x1
    L_401362:
        fild dword ptr [ebp - 0x10]
        fstp dword ptr [esi + 0x6c]
        jmp L_40139d
    L_40136a:
        fld dword ptr [ebp - 0x10]
        fabs
        fsubr dword ptr [g_RData_004cc000 + 0x814]
        fcom dword ptr [g_RData_004cc000 + 0x818]
        fnstsw AX
        test AH, 0x41
        jz L_40138a
        fstp st(0)
        fld dword ptr [g_RData_004cc000 + 0x818]
    L_40138a:
        mov eax, dword ptr [ebp - 0x10]
        mov dword ptr [esi + 0xfec], 0x1
        fstp dword ptr [esi + 0x68]
        mov dword ptr [esi + 0x6c], eax
    L_40139d:
        mov ecx, dword ptr [esi + 0x68]
        mov edx, dword ptr [esi + 0x6c]
        mov eax, dword ptr [ebp - 0x20]
        mov dword ptr [esi + 0x7c], ecx
        mov dword ptr [esi + 0x80], edx
        cmp dword ptr [eax + 0xa4], 0x2
        jnz L_4013e5
        mov ecx, dword ptr [ebp - 0xc]
        fld dword ptr [ecx + 0x4]
        fsub dword ptr [esi + 0x3f0]
        fadd dword ptr [eax + 0x228]
        fmul dword ptr [g_Data_004da000 + 0xc0]
        fsub dword ptr [esi + 0x3bc]
        fmul dword ptr [g_Data_004da000 + 0xc4]
        fst dword ptr [esi + 0x74]
        fstp dword ptr [esi + 0x88]
    L_4013e5:
        fld dword ptr [ebp - 0x24]
        fcomp dword ptr [g_RData_004cc000 + 0x81c]
        fnstsw AX
        test AH, 0x1
        jz L_40140d
        lea edx, [ebp - 0x30]
        lea eax, [ebp - 0x8]
        push edx
        push eax
        lea edx, [ebp - 0x4]
        mov ecx, edi
        call AiVeh_AdvanceToLinkedNode
        mov dword ptr [esi + 0xfec], ebx
    L_40140d:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00401420 AiVeh_ProbeAhead - if [+0x4dc] or [+0x4e4]: [+0xffc]++, return 1. v=vel +0xa4; L=len(v) (0x00402f60, called twice when >=1.0, else 1.0); probe=pos+v*(L*0.5-[[ECX+8]+4+0x114]); 0x00423b10(2,&{-1,-1}) sweep; return 1 if list counts [+0x4a0] or [+0x480] nonzero else 0; 0x00423530 cleanup
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_ProbeAhead(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x38
        push ebx
        push esi
        push edi
        mov edi, ecx
        mov esi, dword ptr [edi + 0x4]
        mov eax, dword ptr [edi + 0x8]
        mov ecx, dword ptr [esi + 0x4dc]
        mov eax, dword ptr [eax + 0x4]
        test ecx, ecx
        mov dword ptr [ebp - 0x4], eax
        jnz L_40155f
        mov ecx, dword ptr [esi + 0x4e4]
        test ecx, ecx
        jnz L_40155f
        lea ebx, [esi + 0x3ec]
        mov ecx, ebx
        mov edx, dword ptr [ecx]
        mov dword ptr [ebp - 0x38], edx
        mov edx, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x34], edx
        lea edx, [esi + 0xa4]
        mov ecx, dword ptr [ecx + 0x8]
        mov dword ptr [ebp - 0x30], ecx
        fld dword ptr [eax + 0x110]
        fadd dword ptr [ebp - 0x34]
        fstp dword ptr [ebp - 0x34]
        mov eax, dword ptr [edx]
        mov dword ptr [ebp - 0x20], eax
        mov ecx, dword ptr [edx + 0x4]
        mov dword ptr [ebp - 0x1c], ecx
        lea ecx, [ebp - 0x20]
        mov edx, dword ptr [edx + 0x8]
        mov dword ptr [ebp - 0x18], edx
        call Vec3_NormalizeInPlace
        fcomp dword ptr [g_RData_004cc000 + 0x814]
        fnstsw AX
        test AH, 0x1
        jz L_4014ac
        fld dword ptr [g_RData_004cc000 + 0x814]
        jmp L_4014b4
    L_4014ac:
        lea ecx, [ebp - 0x20]
        call Vec3_NormalizeInPlace
    L_4014b4:
        fmul dword ptr [g_RData_004cc000 + 0x820]
        mov eax, dword ptr [ebp - 0x4]
        lea ecx, [ebp - 0x2c]
        lea edx, [ebp - 0x2c]
        mov dword ptr [ebp - 0x4], ecx
        fsub dword ptr [eax + 0x114]
        mov dword ptr [ebp - 0x8], ebx
        mov dword ptr [ebp - 0xc], edx
        fld st(0)
        fmul dword ptr [ebp - 0x20]
        fstp dword ptr [ebp - 0x2c]
        fld st(0)
        fmul dword ptr [ebp - 0x1c]
        fstp dword ptr [ebp - 0x28]
        fmul dword ptr [ebp - 0x18]
        fstp dword ptr [ebp - 0x24]
        mov ebx, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [ebp - 0x8]
        mov edx, dword ptr [ebp - 0x4]
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
        or eax, 0xffffffff
        lea edx, [ebp - 0x38]
        mov dword ptr [ebp - 0x14], eax
        mov dword ptr [ebp - 0x10], eax
        lea eax, [ebp - 0x14]
        mov ecx, edi
        push eax
        push 0x2
        call Vehicle_TestSweepSegments
        mov eax, dword ptr [esi + 0x4a0]
        test eax, eax
        jnz L_40154a
        mov eax, dword ptr [esi + 0x480]
        test eax, eax
        jnz L_40154a
        mov ecx, edi
        xor esi, esi
        call Vehicle_ClearControllerLists
        mov eax, esi
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_40154a:
        mov ecx, edi
        mov esi, 0x1
        call Vehicle_ClearControllerLists
        mov eax, esi
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_40155f:
        mov eax, dword ptr [esi + 0xffc]
        pop edi
        inc eax
        mov dword ptr [esi + 0xffc], eax
        pop esi
        mov eax, 0x1
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00401710 AiVeh_CombatTick - if [+0xfd0]<=t and [+0xff0]!=3: AiVeh_PushReturnNode. d=player-self (xz), pitch=dy/|d|; fwd=dot(fwd,d), cross; if [+0xffc]>6 -> [+0xff0]=5. switch [+0xff0]: 0 KeepRangeDrive (fwd kept), 1 CircleOrAim, 2 FollowOrAim, 3 0x00401180, 5 SteerTowardPlayerB, 6 restore [+0xff4] when [+0x1018]==0; fwd=1.0 ex
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_CombatTick(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x38
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        push ebx
        push esi
        push edi
        mov edi, ecx
        fld dword ptr [g_Data_004da000 + 0x19760]
        mov eax, dword ptr [edi + 0x8]
        mov esi, dword ptr [edi + 0x4]
        mov ecx, dword ptr [eax + 0x4]
        mov eax, dword ptr [edx + 0x4]
        add eax, 0x3ec
        mov dword ptr [ebp - 0x14], ecx
        mov ecx, dword ptr [eax]
        mov dword ptr [ebp - 0x38], ecx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [ebp - 0x34], edx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [ebp - 0x30], eax
        fcomp dword ptr [esi + 0xfd0]
        fnstsw AX
        test AH, 0x1
        jnz L_40176f
        cmp dword ptr [esi + 0xff0], 0x3
        jz L_40176f
        mov edx, dword ptr [esi + 0xf78]
        mov ecx, edi
        call AiVeh_PushReturnNode
    L_40176f:
        lea ecx, [ebp - 0x20]
        lea eax, [esi + 0x3ec]
        lea edx, [ebp - 0x38]
        mov dword ptr [ebp - 0x10], ecx
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 0x8], edx
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
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
        mov eax, dword ptr [ebp - 0x1c]
        lea ecx, [ebp - 0x20]
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0x1c], 0x0
        call Vec3_NormalizeInPlace
        fst dword ptr [ebp - 0xc]
        fcomp dword ptr [g_RData_004cc000 + 0x810]
        fnstsw AX
        test AH, 0x40
        jnz L_4017d7
        fld dword ptr [ebp - 0x8]
        fdiv dword ptr [ebp - 0xc]
        fstp dword ptr [ebp - 0x8]
        jmp L_4017de
    L_4017d7:
        mov dword ptr [ebp - 0x8], 0x0
    L_4017de:
        lea ecx, [esi + 0x380]
        mov edx, dword ptr [esi + 0x380]
        mov dword ptr [ebp - 0x2c], edx
        mov eax, dword ptr [ecx + 0x4]
        mov dword ptr [ebp - 0x28], eax
        mov eax, dword ptr [esi + 0xffc]
        mov ecx, dword ptr [ecx + 0x8]
        cmp eax, 0x6
        mov dword ptr [ebp - 0x24], ecx
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [ebp - 0x20]
        fld dword ptr [ebp - 0x2c]
        fmul dword ptr [ebp - 0x18]
        fsubp st(1), st(0)
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x2c]
        fmul dword ptr [ebp - 0x20]
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [ebp - 0x18]
        faddp st(1), st(0)
        fstp dword ptr [ebp - 0x4]
        jle L_401830
        mov dword ptr [esi + 0xff0], 0x5
    L_401830:
        mov eax, dword ptr [esi + 0xff0]
        mov ebx, dword ptr [ebp - 0xc]
        cmp eax, 0x6
        ja L_4018aa
        cmp eax, 0
        je L_401845
        cmp eax, 1
        je L_401857
        cmp eax, 2
        je L_401869
        cmp eax, 3
        je L_40189c
        cmp eax, 4
        je L_4018aa
        cmp eax, 5
        je L_401893
        cmp eax, 6
        je L_40187b
        int 3  // unreachable: the bounds check above excludes other indices
    L_401845:
        mov edx, dword ptr [ebp - 0x10]
        mov eax, dword ptr [ebp - 0x4]
        push ebx
        push edx
        push eax
        mov ecx, edi
        call AiVeh_KeepRangeDrive
        jmp L_4018aa
    L_401857:
        mov ecx, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp - 0x4]
        push ebx
        push ecx
        push edx
        mov ecx, edi
        call AiVeh_CircleOrAim
        jmp L_4018a3
    L_401869:
        mov eax, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp - 0x4]
        push ebx
        push eax
        push ecx
        mov ecx, edi
        call AiVeh_FollowOrAim
        jmp L_4018a3
    L_40187b:
        mov eax, dword ptr [esi + 0x1018]
        test eax, eax
        jnz L_4018a3
        mov edx, dword ptr [esi + 0xff4]
        mov dword ptr [esi + 0xff0], edx
        jmp L_4018a3
    L_401893:
        mov ecx, edi
        call AiVeh_SteerTowardPlayerB
        jmp L_4018a3
    L_40189c:
        mov ecx, edi
        call AiVeh_PatrolTick
    L_4018a3:
        mov dword ptr [ebp - 0x4], 0x3f800000
    L_4018aa:
        mov eax, dword ptr [ebp - 0x14]
        cmp dword ptr [eax + 0xa4], 0x2
        jnz L_4018ec
        fld dword ptr [g_Data_004da000 + 0xc8]
        fmul dword ptr [ebp - 0x8]
        fsub dword ptr [esi + 0x3bc]
        fmul dword ptr [g_Data_004da000 + 0xcc]
        fst dword ptr [esi + 0x74]
        fstp dword ptr [esi + 0x88]
        fld dword ptr [ebp - 0x34]
        fsub dword ptr [esi + 0x3f0]
        fmul dword ptr [g_Data_004da000 + 0xd0]
        fst dword ptr [esi + 0x70]
        fstp dword ptr [esi + 0x84]
    L_4018ec:
        mov ecx, dword ptr [ebp - 0x4]
        push ecx
        push ebx
        mov ecx, edi
        call AiVeh_FireDecision
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        mov eax, dword ptr [edx + 0x4]
        cmp dword ptr [eax + 0x60], 0x4
        jz L_401925
        lea edx, [esi + 0xfb8]
        lea ecx, [esi + 0x3ec]
        call Vec3_DistanceSquared
        fcomp dword ptr [esi + 0xfb4]
        fnstsw AX
        test AH, 0x41
        jnz L_40193e
    L_401925:
        mov ecx, edi
        call AiVeh_CopyF88ToF84
        fld dword ptr [esi + 0xf98]
        fadd dword ptr [g_Data_004da000 + 0x19760]
        fstp dword ptr [esi + 0xf90]
    L_40193e:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00401a40 AiVeh_CircleOrAim - if 0x00401420()==0 -> AiVeh_CircleStrafeTarget; else Vehicle_ComputeAimYawToPoint, zero +0x68/+0x6c/+0x7c/+0x80, [+0xff4]=[+0xff0], [+0xff0]=6; ret 0xc
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall AiVeh_CircleOrAim(int, int, int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, ecx
        mov esi, dword ptr [edi + 0x4]
        call AiVeh_ProbeAhead
        test eax, eax
        jnz L_401a62
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        mov ecx, edi
        call AiVeh_CircleStrafeTarget
        pop edi
        pop esi
        ret 0xc
    L_401a62:
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        mov ecx, edi
        mov edx, dword ptr [eax + 0x4]
        add edx, 0x3ec
        call Vehicle_ComputeAimYawToPoint
        mov ecx, dword ptr [esi + 0xff0]
        xor eax, eax
        mov dword ptr [esi + 0x80], eax
        mov dword ptr [esi + 0x6c], eax
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x68], eax
        mov dword ptr [esi + 0xff4], ecx
        mov dword ptr [esi + 0xff0], 0x6
        pop edi
        pop esi
        ret 0xc
    }
}

// 0x00401ab0 AiVeh_FollowOrAim - if 0x00401420()==0 -> AiVeh_FollowOffsetTarget(arg3); else aim as 0x00401a40 and [+0xff0]=6; ret 0xc
// Register/stack shape from the listing (ECX, EDX, 12 stack bytes).
__declspec(naked) int __fastcall AiVeh_FollowOrAim(int, int, int, int, int)
{
    __asm {
        push esi
        push edi
        mov edi, ecx
        mov esi, dword ptr [edi + 0x4]
        call AiVeh_ProbeAhead
        test eax, eax
        jnz L_401ad7
        mov eax, dword ptr [esp + 0x14]
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        push eax
        mov ecx, edi
        call AiVeh_FollowOffsetTarget
        pop edi
        pop esi
        ret 0xc
    L_401ad7:
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        mov edx, dword ptr [ecx + 0x4]
        mov ecx, edi
        add edx, 0x3ec
        call Vehicle_ComputeAimYawToPoint
        mov edx, dword ptr [esi + 0xff0]
        xor eax, eax
        mov dword ptr [esi + 0x80], eax
        mov dword ptr [esi + 0x6c], eax
        mov dword ptr [esi + 0x7c], eax
        mov dword ptr [esi + 0x68], eax
        mov dword ptr [esi + 0xff4], edx
        mov dword ptr [esi + 0xff0], 0x6
        pop edi
        pop esi
        ret 0xc
    }
}

// 0x00401b20 AiVeh_TryDetectPlayer - if [0x004f36ac]==0 and [+0xf90]<t: if 0x00472670() < [+0xfb0] and Collision_TraceWithPlayerPos(1)!=0: 0x00401c60, [+0x448]=1, if [[+0xf74]+0x48] then AiList_ResetNonIdle, [+0xffc]=0, return 1; else [+0x448]=0. return 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_TryDetectPlayer(int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x196ac]
        sub esp, 0xc
        test eax, eax
        push ebx
        mov ebx, ecx
        push esi
        push edi
        mov esi, dword ptr [ebx + 0x4]
        jnz L_401bf3
        fld dword ptr [g_Data_004da000 + 0x19760]
        fcomp dword ptr [esi + 0xf90]
        fnstsw AX
        test AH, 0x41
        jnz L_401bf3
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        lea edi, [esi + 0x410]
        mov edx, edi
        mov ecx, dword ptr [eax + 0x4]
        add ecx, 0x410
        call Vec3_DistanceSquared
        fcomp dword ptr [esi + 0xfb0]
        fnstsw AX
        test AH, 0x1
        jz L_401bf3
        mov edx, dword ptr [edi + 0x4]
        mov ecx, dword ptr [edi]
        mov eax, dword ptr [edi + 0x8]
        mov dword ptr [esp + 0x10], edx
        fld dword ptr [esp + 0x10]
        fsub dword ptr [g_RData_004cc000 + 0x824]
        mov dword ptr [esp + 0xc], ecx
        mov ecx, dword ptr [esi + 0xed0]
        push 0x1
        lea edx, [esp + 0x10]
        mov dword ptr [esp + 0x18], eax
        fstp dword ptr [esp + 0x14]
        call Collision_TraceWithPlayerPos
        test eax, eax
        jz L_401be9
        mov ecx, ebx
        call AiVeh_EnterState1
        mov ecx, dword ptr [esi + 0xf74]
        mov dword ptr [esi + 0x448], 0x1
        mov eax, dword ptr [ecx + 0x48]
        test eax, eax
        jz L_401bd3
        mov ecx, ebx
        call AiList_ResetNonIdle
    L_401bd3:
        mov dword ptr [esi + 0xffc], 0x0
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        add esp, 0xc
        ret
    L_401be9:
        mov dword ptr [esi + 0x448], 0x0
    L_401bf3:
        pop edi
        pop esi
        xor eax, eax
        pop ebx
        add esp, 0xc
        ret
    }
}

// 0x00401d50 Collision_TraceWithPlayerPos - (dir,vec EDX): player=[[0x004f3a88]+4]; Collision_GridDDATraversal between player +0x410 and EDX (endpoint order by dir==1); returns 0 iff traversal==0 and local flag set, else 1
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Collision_TraceWithPlayerPos(int, int, int)
{
    __asm {
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        sub esp, 0x504
        push ebx
        push esi
        mov esi, dword ptr [eax + 0x4]
        mov ebx, ecx
        push edi
        mov edi, edx
        mov ecx, dword ptr [esi + 0x444]
        xor edx, edx
        mov dword ptr [g_Data_004da000 + 0xa3a28], ecx
        mov ecx, ebx
        call gwNodeSetFlag10
        mov ecx, dword ptr [esi + 0xed0]
        xor edx, edx
        call gwNodeSetFlag10
        mov ecx, 0x1
        call Collision_SetQueryMode
        mov ecx, 0x40000
        call Collision_SetQueryMask
        mov eax, dword ptr [esp + 0x514]
        lea edx, [esp + 0xc]
        cmp eax, 0x1
        jnz L_401dcc
        mov eax, dword ptr [edi + 0x8]
        mov ecx, dword ptr [edi + 0x4]
        push eax
        mov eax, dword ptr [edi]
        push ecx
        mov ecx, dword ptr [esi + 0x418]
        push eax
        mov eax, dword ptr [esi + 0x414]
        push ecx
        mov ecx, dword ptr [esi + 0x410]
        push eax
        jmp L_401deb
    L_401dcc:
        mov eax, dword ptr [esi + 0x418]
        mov ecx, dword ptr [esi + 0x414]
        push eax
        mov eax, dword ptr [esi + 0x410]
        push ecx
        mov ecx, dword ptr [edi + 0x8]
        push eax
        mov eax, dword ptr [edi + 0x4]
        push ecx
        mov ecx, dword ptr [edi]
        push eax
    L_401deb:
        push ecx
        mov ecx, dword ptr [g_Data_004da000 + 0x196b8]
        call Collision_GridDDATraversal
        xor ecx, ecx
        mov edi, eax
        call Collision_SetQueryMode
        mov ecx, dword ptr [esi + 0xed0]
        mov edx, 0x1
        call gwNodeSetFlag10
        mov edx, 0x1
        mov ecx, ebx
        call gwNodeSetFlag10
        test edi, edi
        jnz L_401e36
        mov eax, dword ptr [esp + 0xc]
        test eax, eax
        jz L_401e36
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        add esp, 0x504
        ret 0x4
    L_401e36:
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        add esp, 0x504
        ret 0x4
    }
}

// 0x00401e50 Collision_TraceWithCameraPos - as Collision_TraceWithPlayerPos but endpoint is Camera_GetWorldPosition; (dir,vec EDX); returns 0 iff traversal==0 and local flag set, else 1; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Collision_TraceWithCameraPos(int, int, int)
{
    __asm {
        sub esp, 0x510
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        push ebx
        push esi
        mov ebx, ecx
        mov esi, edx
        lea ecx, [esp + 0x10]
        push edi
        mov edi, dword ptr [eax + 0x4]
        lea edx, [esp + 0x10]
        push ecx
        mov ecx, dword ptr [g_Data_004da000 + 0x196bc]
        push edx
        lea edx, [esp + 0x14]
        call Camera_GetWorldPosition
        mov eax, dword ptr [edi + 0x444]
        xor edx, edx
        mov ecx, ebx
        mov dword ptr [g_Data_004da000 + 0xa3a28], eax
        call gwNodeSetFlag10
        mov ecx, dword ptr [edi + 0xed0]
        xor edx, edx
        call gwNodeSetFlag10
        mov ecx, 0x1
        call Collision_SetQueryMode
        mov ecx, 0x40000
        call Collision_SetQueryMask
        mov eax, dword ptr [esp + 0x520]
        lea edx, [esp + 0x18]
        cmp eax, 0x1
        jnz L_401ede
        mov ecx, dword ptr [esi + 0x8]
        mov eax, dword ptr [esi + 0x4]
        push ecx
        mov ecx, dword ptr [esi]
        push eax
        mov eax, dword ptr [esp + 0x1c]
        push ecx
        mov ecx, dword ptr [esp + 0x1c]
        push eax
        mov eax, dword ptr [esp + 0x1c]
        push ecx
        jmp L_401ef7
    L_401ede:
        mov ecx, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x10]
        push ecx
        mov ecx, dword ptr [esp + 0x10]
        push eax
        mov eax, dword ptr [esi + 0x8]
        push ecx
        mov ecx, dword ptr [esi + 0x4]
        push eax
        mov eax, dword ptr [esi]
        push ecx
    L_401ef7:
        mov ecx, dword ptr [g_Data_004da000 + 0x196b8]
        push eax
        call Collision_GridDDATraversal
        xor ecx, ecx
        mov esi, eax
        call Collision_SetQueryMode
        mov ecx, dword ptr [edi + 0xed0]
        mov edx, 0x1
        call gwNodeSetFlag10
        mov edx, 0x1
        mov ecx, ebx
        call gwNodeSetFlag10
        test esi, esi
        jnz L_401f42
        mov eax, dword ptr [esp + 0x18]
        test eax, eax
        jz L_401f42
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        add esp, 0x510
        ret 0x4
    L_401f42:
        pop edi
        pop esi
        mov eax, 0x1
        pop ebx
        add esp, 0x510
        ret 0x4
    }
}

// 0x00402250 AiVeh_FireDecision - thiscall(dist,dot) ret 8; w=[self+0x5e4]; burst window: if [+0xfac]<t: [+0xfa8]=t+[+0xf98], [+0xfac]=[+0xfa8]+[+0xf94]. Not-firing: if w[0x10]<t, [+0xfa8]<t, [+0x18]==0, dot>0.75, w[0x12]<dist<w[0x13], Collision_TraceWithPlayerPos(1)!=0 and player[+0x60]!=4: [+0x5a4]=1, w[0x10]=t+w[0x11]/max([+0xf30
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall AiVeh_FireDecision(int, int, int, int)
{
    __asm {
        fld dword ptr [g_Data_004da000 + 0x19760]
        push ebp
        mov ebp, ecx
        push esi
        push edi
        mov esi, dword ptr [ebp + 0x4]
        fcomp dword ptr [esi + 0xfac]
        mov edi, dword ptr [esi + 0x5e4]
        fnstsw AX
        test AH, 0x41
        jnz L_402293
        fld dword ptr [esi + 0xf98]
        fadd dword ptr [g_Data_004da000 + 0x19760]
        fld dword ptr [esi + 0xf94]
        fadd st(0), st(1)
        fxch st(1)
        fstp dword ptr [esi + 0xfa8]
        fstp dword ptr [esi + 0xfac]
    L_402293:
        fld dword ptr [g_Data_004da000 + 0x19760]
        fcomp dword ptr [edi + 0x40]
        mov eax, dword ptr [esi + 0x5bc]
        test eax, eax
        fnstsw AX
        jnz L_40242e
        test AH, 0x41
        jnz L_402497
        fld dword ptr [g_Data_004da000 + 0x19760]
        fcomp dword ptr [esi + 0xfa8]
        fnstsw AX
        test AH, 0x41
        jnz L_402497
        mov eax, dword ptr [esi + 0x18]
        test eax, eax
        jnz L_402497
        fld dword ptr [esp + 0x14]
        fcomp dword ptr [g_RData_004cc000 + 0x834]
        fnstsw AX
        test AH, 0x41
        jnz L_402497
        fld dword ptr [esp + 0x10]
        fcomp dword ptr [edi + 0x4c]
        fnstsw AX
        test AH, 0x1
        jz L_402497
        fld dword ptr [esp + 0x10]
        fcomp dword ptr [edi + 0x48]
        fnstsw AX
        test AH, 0x41
        jnz L_402497
        mov ecx, dword ptr [esi + 0xed0]
        push 0x1
        lea edx, [esi + 0x410]
        call Collision_TraceWithPlayerPos
        test eax, eax
        jz L_402497
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        mov ecx, dword ptr [eax + 0x4]
        cmp dword ptr [ecx + 0x60], 0x4
        jz L_402497
        fld dword ptr [esi + 0xf30]
        fcomp dword ptr [g_RData_004cc000 + 0x820]
        mov dword ptr [esi + 0x5a4], 0x1
        fnstsw AX
        test AH, 0x41
        jnz L_402362
        fld dword ptr [esi + 0xf30]
        jmp L_402368
    L_402362:
        fld dword ptr [g_RData_004cc000 + 0x820]
    L_402368:
        fld dword ptr [edi + 0x44]
        fdiv st(0), st(1)
        mov edx, dword ptr [edi]
        fadd dword ptr [g_Data_004da000 + 0x19760]
        fstp dword ptr [edi + 0x40]
        mov eax, dword ptr [edx + 0x54]
        mov ecx, eax
        shr ecx, 0x1
        test CL, 0x1
        fstp st(0)
        jz L_4023ac
        mov edx, dword ptr [esi]
        mov dword ptr [esi + 0x5bc], 0x1
        mov ecx, dword ptr [edi + 0x50]
        call Weapon_ActivateProjectile
        fld dword ptr [g_Data_004da000 + 0x19760]
        fadd dword ptr [edi + 0x44]
        fstp dword ptr [edi + 0x40]
        pop edi
        pop esi
        pop ebp
        ret 0x8
    L_4023ac:
        test AH, 0x40
        jz L_402400
        mov dword ptr [esi + 0xddc], 0x1
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        push 0x40a00000
        mov eax, dword ptr [edx + 0x4]
        add eax, 0x410
        mov dword ptr [esi + 0xde0], eax
        mov ecx, dword ptr [g_Data_004da000 + 0x19a88]
        mov edx, dword ptr [ecx + 0x4]
        mov ecx, 0x908
        add edx, 0xa4
        mov dword ptr [esi + 0xde4], edx
        call Message_GetText
        mov ecx, eax
        call Hud_ShowMessage
        pop edi
        pop esi
        pop ebp
        ret 0x8
    L_402400:
        xor eax, eax
        mov ecx, ebp
        mov dword ptr [esi + 0xddc], eax
        mov dword ptr [esi + 0xde0], eax
        mov dword ptr [esi + 0xde4], eax
        mov edx, dword ptr [g_Data_004da000 + 0x19a88]
        add esi, 0xec0
        push esi
        call AiVeh_LeadTargetIntercept
        pop edi
        pop esi
        pop ebp
        ret 0x8
    L_40242e:
        test AH, 0x41
        jz L_402481
        fld dword ptr [esp + 0x14]
        fcomp dword ptr [g_RData_004cc000 + 0x834]
        fnstsw AX
        test AH, 0x1
        jnz L_402481
        fld dword ptr [esp + 0x10]
        fcomp dword ptr [edi + 0x4c]
        fnstsw AX
        test AH, 0x41
        jz L_402481
        mov eax, dword ptr [g_Data_004da000 + 0x19a88]
        mov eax, dword ptr [eax + 0x4]
        cmp dword ptr [eax + 0x60], 0x4
        jz L_402481
        add eax, 0x410
        add esi, 0xec0
        mov ecx, dword ptr [eax]
        mov dword ptr [esi], ecx
        mov edx, dword ptr [eax + 0x4]
        mov dword ptr [esi + 0x4], edx
        mov eax, dword ptr [eax + 0x8]
        mov dword ptr [esi + 0x8], eax
        pop edi
        pop esi
        pop ebp
        ret 0x8
    L_402481:
        mov dword ptr [esi + 0x5a4], 0x0
        fld dword ptr [edi + 0x44]
        fadd dword ptr [g_Data_004da000 + 0x19760]
        fstp dword ptr [edi + 0x40]
    L_402497:
        pop edi
        pop esi
        pop ebp
        ret 0x8
    }
}

// 0x00402b70 AiVeh_RouteTick - q=[ECX+4]; if [q+0xfa4] < global time 0x004f3760: head [q+0xf78]==target [q+0xf7c] -> 0x00402be0; elif head.next(+0xc)==target and head[+0x28]!=-1 -> 0x00402d60; else 0x00401180. Always sets [q+0xfdc]=1
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall AiVeh_RouteTick(int, int)
{
    __asm {
        fld dword ptr [g_Data_004da000 + 0x19760]
        push esi
        mov esi, dword ptr [ecx + 0x4]
        fcomp dword ptr [esi + 0xfa4]
        fnstsw AX
        test AH, 0x41
        jnz L_402bc9
        mov eax, dword ptr [esi + 0xf78]
        mov edx, dword ptr [esi + 0xf7c]
        cmp eax, edx
        jnz L_402ba8
        call AiVeh_DriveToNextNode
        mov dword ptr [esi + 0xfdc], 0x1
        pop esi
        ret
    L_402ba8:
        cmp dword ptr [eax + 0xc], edx
        jnz L_402bc4
        cmp dword ptr [eax + 0x28], -0x1
        jz L_402bc4
        call AiVeh_ReverseToNextNode
        mov dword ptr [esi + 0xfdc], 0x1
        pop esi
        ret
    L_402bc4:
        call AiVeh_PatrolTick
    L_402bc9:
        mov dword ptr [esi + 0xfdc], 0x1
        pop esi
        ret
    }
}

}  // namespace recoil
