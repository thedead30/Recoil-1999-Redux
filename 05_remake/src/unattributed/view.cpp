// SUBSYSTEM: view
// View/projection state. All three blocks lie in the part of .data with no file backing (raw size 0xbc00
// of virtual 0x29fac0), so they are zero at load: CONFIRMED-DATA (Recoil.exe section table).
// Spec: 04_spec/systems/view.md
#include "unattributed/view.h"
#include "GameZRecoil/zClass/Display.h"
#include "GameZRecoil/zClass/Window_nodes.h"
#include "unattributed/settings.h"
#include "GameZRecoil/zClass/Camera.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"
#include "unattributed/menus.h"
#include "unattributed/transform.h"
#include "GameZRecoil/zVideo/zvid_buff.h"
#include "GameZRecoil/zVideo/zvid_ddd3d_zvideo.h"

namespace recoil {

float g_ViewParams_00566838[12];
float g_ScreenCentre_005761e0[2];
float g_DefaultAngles_005669d8[3];

// 0x004083d0 Rect_SetOrigin - thiscall(y) EDX=x: r[0]=x r[1]=y; r[2]=x+w[4], r[3]=y+h[5]; r[6]=r[2]-1, r[7]=r[3]-1; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Rect_SetOrigin(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx + 0x10]
        push esi
        mov esi, dword ptr [esp + 0x8]
        mov dword ptr [ecx], edx
        add eax, edx
        mov edx, dword ptr [ecx + 0x14]
        add edx, esi
        mov dword ptr [ecx + 0x8], eax
        mov dword ptr [ecx + 0xc], edx
        dec eax
        dec edx
        mov dword ptr [ecx + 0x4], esi
        mov dword ptr [ecx + 0x18], eax
        mov dword ptr [ecx + 0x1c], edx
        pop esi
        ret 0x4
    }
}

// 0x00408400 Rect_SetSize - thiscall(h) EDX=w: r[4]=w r[5]=h; r[2]=r[0]+w, r[3]=r[1]+h; r[6]=r[2]-1, r[7]=r[3]-1; ret 4
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Rect_SetSize(int, int, int)
{
    __asm {
        mov eax, dword ptr [ecx]
        push esi
        mov esi, dword ptr [esp + 0x8]
        mov dword ptr [ecx + 0x10], edx
        add eax, edx
        mov edx, dword ptr [ecx + 0x4]
        add edx, esi
        mov dword ptr [ecx + 0x8], eax
        mov dword ptr [ecx + 0xc], edx
        dec eax
        dec edx
        mov dword ptr [ecx + 0x14], esi
        mov dword ptr [ecx + 0x18], eax
        mov dword ptr [ecx + 0x1c], edx
        pop esi
        ret 0x4
    }
}

// 0x00408570 Viewport0_AttachView - vp=[[0x004e5d80]]; vp+0x24 = ECX; if non-null applies size (0x0044f8b0) and origin (0x0044f9c0).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Viewport0_AttachView(int, int)
{
    __asm {
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x80]
        push esi
        test ecx, ecx
        mov esi, dword ptr [eax]
        mov dword ptr [esi + 0x24], ecx
        jz L_408599
        mov edx, dword ptr [esi + 0x14]
        push edx
        mov edx, dword ptr [esi + 0x10]
        call WindowClass_SetSize
        mov eax, dword ptr [esi + 0x4]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [esi + 0x24]
        push eax
        call WindowClass_SetOrigin
    L_408599:
        pop esi
        ret
    }
}

// 0x004085b0 Viewport1_AttachView - vp=[[0x004e5d84]]; vp+0x24 = ECX; if non-null applies size (0x0044fe90 with vp+0x10/+0x14) and origin (0x0044ff10 with vp+0/+4).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Viewport1_AttachView(int, int)
{
    __asm {
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x84]
        push esi
        test ecx, ecx
        mov esi, dword ptr [eax]
        mov dword ptr [esi + 0x24], ecx
        jz L_4085d9
        mov edx, dword ptr [esi + 0x14]
        push edx
        mov edx, dword ptr [esi + 0x10]
        call DisplayClass_SetSize
        mov eax, dword ptr [esi + 0x4]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [esi + 0x24]
        push eax
        call DisplayClass_SetOrigin
    L_4085d9:
        pop esi
        ret
    }
}

// 0x00408500 Viewport0_SetSizeF - vp = [[0x004e5d80]]; 0x00408400 on vp with (ECX_in, EDX_in); if vp+0x24 (render view) : 0x0044f8b0(ECX=vp+0x24, EDX=vp+0x10, push vp+0x14).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Viewport0_SetSizeF(int, int)
{
    __asm {
        mov eax, edx
        push esi
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x80]
        push eax
        mov esi, dword ptr [edx]
        mov edx, ecx
        mov ecx, esi
        call Rect_SetSize
        mov ecx, dword ptr [esi + 0x24]
        test ecx, ecx
        jz L_408528
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esi + 0x10]
        push eax
        call WindowClass_SetSize
    L_408528:
        pop esi
        ret
    }
}

// 0x00408530 Viewport0_SetOrigin - vp = [[0x004e5d80]]; 0x004083d0 on vp with (ECX_in, EDX_in); if vp+0x24: 0x0044f8b0(vp+0x24, vp+0x10, vp+0x14) and 0x0044f9c0(ECX=vp+0x24, EDX=vp+0, push vp+4).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Viewport0_SetOrigin(int, int)
{
    __asm {
        mov eax, edx
        push esi
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x80]
        push eax
        mov esi, dword ptr [edx]
        mov edx, ecx
        mov ecx, esi
        call Rect_SetOrigin
        mov ecx, dword ptr [esi + 0x24]
        test ecx, ecx
        jz L_408566
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esi + 0x10]
        push eax
        call WindowClass_SetSize
        mov ecx, dword ptr [esi + 0x4]
        mov edx, dword ptr [esi]
        push ecx
        mov ecx, dword ptr [esi + 0x24]
        call WindowClass_SetOrigin
    L_408566:
        pop esi
        ret
    }
}

// 0x004085e0 Viewport1_SetOrigin - Same as Viewport0_SetOrigin on vp = [[0x004e5d84]] with view calls 0x0044fe90 and 0x0044ff10.
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Viewport1_SetOrigin(int, int)
{
    __asm {
        mov eax, edx
        push esi
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x84]
        push eax
        mov esi, dword ptr [edx]
        mov edx, ecx
        mov ecx, esi
        call Rect_SetOrigin
        mov ecx, dword ptr [esi + 0x24]
        test ecx, ecx
        jz L_408616
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esi + 0x10]
        push eax
        call DisplayClass_SetSize
        mov ecx, dword ptr [esi + 0x4]
        mov edx, dword ptr [esi]
        push ecx
        mov ecx, dword ptr [esi + 0x24]
        call DisplayClass_SetOrigin
    L_408616:
        pop esi
        ret
    }
}

// 0x00408620 Viewport1_SetSize - vp = [[0x004e5d84]]; 0x00408400 on vp; if vp+0x24: 0x0044fe90(vp+0x24, vp+0x10, vp+0x14).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Viewport1_SetSize(int, int)
{
    __asm {
        mov eax, edx
        push esi
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x84]
        push eax
        mov esi, dword ptr [edx]
        mov edx, ecx
        mov ecx, esi
        call Rect_SetSize
        mov ecx, dword ptr [esi + 0x24]
        test ecx, ecx
        jz L_408648
        mov eax, dword ptr [esi + 0x14]
        mov edx, dword ptr [esi + 0x10]
        push eax
        call DisplayClass_SetSize
    L_408648:
        pop esi
        ret
    }
}

// 0x00408480 Viewport_SetCamera - [[0x004e5d7c]+8] = ECX. If non-null: vp = [[0x004e5d80]]; Camera_GetFOV; fov_x = vp.w (+0x10) * fov_y / vp.h (+0x14); Camera_SetFOV(fov_x, fov_y); r = 0x00408030(); Settings_ApplyObjectLOD(ECX=r).
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Viewport_SetCamera(int, int)
{
    __asm {
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x7c]
        sub esp, 0x8
        test ecx, ecx
        push edi
        mov edi, dword ptr [eax]
        mov dword ptr [edi + 0x8], ecx
        jz L_4084d6
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x80]
        push esi
        lea eax, [esp + 0x8]
        mov esi, dword ptr [edx]
        push eax
        lea edx, [esp + 0x10]
        call Camera_GetFOV
        fild dword ptr [esi + 0x10]
        mov ecx, dword ptr [esp + 0x8]
        push ecx
        fmul dword ptr [esp + 0xc]
        fidiv dword ptr [esi + 0x14]
        fstp dword ptr [esp + 0x10]
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [edi + 0x8]
        push edx
        call Camera_SetFOV
        call Setting_Get_004e5d10
        mov ecx, eax
        call Settings_ApplyObjectLOD
        pop esi
    L_4084d6:
        pop edi
        add esp, 0x8
        ret
    }
}

// 0x00472ed0 View_GetProjectionScale - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall View_GetProjectionScale(int, int)
{
    __asm {
        sub esp, 0x8
        mov eax, dword ptr [g_ViewParams_00566838 + 0x18]
        mov ecx, dword ptr [g_ViewParams_00566838 + 0x1c]
        mov dword ptr [esp], eax
        mov dword ptr [esp + 0x4], ecx
        mov edx, ecx
        add esp, 0x8
        ret
    }
}

// 0x00473e60 View_SetCameraMatrix - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall View_SetCameraMatrix(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x8
        push ebx
        push esi
        mov esi, ecx
        push edi
        mov ecx, 0xc
        mov edi, offset g_CameraWorldB_00566920
        mov eax, offset g_CameraWorldB_00566920 + 0xc
        rep movsd
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
        mov eax, offset g_CameraWorldB_00566920 + 0x18
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
        mov eax, dword ptr [g_CameraWorldB_00566920 + 0xc]
        mov ecx, 0xc
        mov esi, offset g_CameraWorldB_00566920
        mov edi, offset g_ViewMatrixA_005668e8
        rep movsd
        fld dword ptr [g_ViewMatrixA_005668e8 + 0x8]
        mov edx, dword ptr [g_ViewMatrixA_005668e8 + 0x18]
        mov ecx, dword ptr [g_CameraWorldB_00566920 + 0x4]
        fstp dword ptr [g_ViewMatrixA_005668e8 + 0x18]
        fld dword ptr [g_ViewMatrixA_005668e8 + 0x14]
        mov dword ptr [g_ViewMatrixA_005668e8 + 0x4], eax
        mov eax, dword ptr [g_ViewMatrixA_005668e8 + 0x1c]
        fstp dword ptr [g_ViewMatrixA_005668e8 + 0x1c]
        mov esi, offset g_ViewMatrixA_005668e8 + 0x24
        mov dword ptr [g_ViewMatrixA_005668e8 + 0xc], ecx
        mov dword ptr [g_ViewMatrixA_005668e8 + 0x8], edx
        mov dword ptr [g_ViewMatrixA_005668e8 + 0x14], eax
        mov dword ptr [ebp - 0x8], esi
        mov dword ptr [ebp - 0x4], esi
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
        mov dword ptr [ebp - 0x8], esi
        mov eax, dword ptr [ebp - 0x8]
        mov ebx, offset g_ViewMatrixA_005668e8
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
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00473fc0 View_TransformPointsCameraY - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall View_TransformPointsCameraY(int, int, int)
{
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [esp + 0x4]
        test ecx, ecx
        jle L_474005
        add eax, 0x4
    L_473fcd:
        fld dword ptr [eax - 0x4]
        fld dword ptr [g_CameraWorldB_00566920 + 0x10]
        fmul dword ptr [eax]
        fld dword ptr [eax + 0x4]
        fxch st(2)
        fmul dword ptr [g_CameraWorldB_00566920 + 0x4]
        fxch st(2)
        fmul dword ptr [g_CameraWorldB_00566920 + 0x1c]
        fxch st(2)
        faddp st(1), st(0)
        fxch st(1)
        add eax, 0xc
        add edx, 0x4
        dec ecx
        faddp st(1), st(0)
        fadd dword ptr [g_CameraWorldB_00566920 + 0x28]
        fstp dword ptr [edx - 0x4]
        jnz L_473fcd
    L_474005:
        ret 0x4
    }
}

// 0x004743e0 View_SetFieldOfView - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 8 stack bytes).
__declspec(naked) int __fastcall View_SetFieldOfView(int, int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
        mov ecx, dword ptr [esp + 0x8]
        mov dword ptr [g_Data_004da000 + 0x8c918], eax
        mov dword ptr [g_Data_004da000 + 0x8c91c], ecx
        ret 0x8
    }
}

// 0x00474400 View_SetProjection - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 32 stack bytes).
__declspec(naked) int __fastcall View_SetProjection(int, int, int, int, int, int, int, int, int, int)
{
    __asm {
        fld dword ptr [esp + 0x18]
        fld dword ptr [esp + 0x14]
        fmul dword ptr [esp + 0xc]
        fld dword ptr [g_RData_004cc000 + 0x697c]
        fxch st(2)
        fmul dword ptr [esp + 0x10]
        fld dword ptr [g_RData_004cc000 + 0x697c]
        fld dword ptr [g_RData_004cc000 + 0x697c]
        fld dword ptr [g_RData_004cc000 + 0x697c]
        fxch st(5)
        fdiv dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x18]
        mov dword ptr [g_ViewParams_00566838 + 0x20], eax
        mov dword ptr [g_ViewParams_00566838 + 0x24], ecx
        fstp dword ptr [g_ViewParams_00566838]
        fxch st(1)
        fdiv dword ptr [esp + 0x18]
        fxch st(3)
        fstp dword ptr [g_ViewParams_00566838 + 0x18]
        fdiv dword ptr [g_ViewParams_00566838 + 0x18]
        fxch st(1)
        fstp dword ptr [g_ViewParams_00566838 + 0x1c]
        fxch st(2)
        fdiv dword ptr [g_ViewParams_00566838 + 0x1c]
        fld dword ptr [esp + 0x10]
        fld dword ptr [esp + 0xc]
        fadd dword ptr [esp + 0x4]
        fxch st(1)
        fadd dword ptr [esp + 0x8]
        fxch st(3)
        fstp dword ptr [g_ViewParams_00566838 + 0x4]
        fld dword ptr [esp + 0xc]
        fxch st(3)
        fld dword ptr [esp + 0x10]
        fxch st(5)
        fstp dword ptr [g_ViewParams_00566838 + 0x28]
        fld dword ptr [esp + 0x4]
        fxch st(3)
        fstp dword ptr [g_ViewParams_00566838 + 0x2c]
        fld dword ptr [esp + 0x8]
        fxch st(4)
        fstp dword ptr [g_ViewParams_00566838 + 0x10]
        fld dword ptr [esp + 0x1c]
        fxch st(5)
        fstp dword ptr [g_ViewParams_00566838 + 0x14]
        fld dword ptr [esp + 0x20]
        fxch st(3)
        fstp dword ptr [g_Data_004da000 + 0x8c430]
        fxch st(3)
        fstp dword ptr [g_Data_004da000 + 0x8c434]
        fstp dword ptr [g_ScreenCentre_005761e0]
        fxch st(1)
        fstp dword ptr [g_ScreenCentre_005761e0 + 0x4]
        fxch st(1)
        fstp dword ptr [g_ViewParams_00566838 + 0x8]
        fstp dword ptr [g_ViewParams_00566838 + 0xc]
        ret 0x20
    }
}

// 0x00474b20 View_ProjectPointsInvZ - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall View_ProjectPointsInvZ(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
    L_474b24:
        fld dword ptr [g_RData_004cc000 + 0x697c]
        fdiv dword ptr [ecx + 0x8]
        add ecx, 0xc
        add edx, 0xc
        dec eax
        fst dword ptr [edx - 0x4]
        fld dword ptr [ecx - 0xc]
        fmul dword ptr [g_ViewParams_00566838 + 0x18]
        fmul st(0), st(1)
        fadd dword ptr [g_ScreenCentre_005761e0]
        fstp dword ptr [edx - 0xc]
        fstp st(0)
        fld dword ptr [ecx - 0x8]
        fmul dword ptr [g_ViewParams_00566838 + 0x1c]
        fmul dword ptr [edx - 0x4]
        fadd dword ptr [g_ScreenCentre_005761e0 + 0x4]
        fstp dword ptr [edx - 0x8]
        jnz L_474b24
        ret 0x4
    }
}

// 0x00474b70 View_ProjectPoints - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall View_ProjectPoints(int, int, int)
{
    __asm {
        mov eax, dword ptr [esp + 0x4]
    L_474b74:
        fld dword ptr [g_RData_004cc000 + 0x697c]
        fdiv dword ptr [ecx + 0x8]
        add ecx, 0xc
        add edx, 0xc
        dec eax
        fld st(0)
        fmul dword ptr [ecx - 0xc]
        fld st(1)
        fxch st(1)
        fmul dword ptr [g_ViewParams_00566838 + 0x18]
        fadd dword ptr [g_ScreenCentre_005761e0]
        fstp dword ptr [edx - 0xc]
        fld dword ptr [ecx - 0x8]
        fmul st(0), st(2)
        fmul dword ptr [g_ViewParams_00566838 + 0x1c]
        fadd dword ptr [g_ScreenCentre_005761e0 + 0x4]
        fstp dword ptr [edx - 0x8]
        fmul dword ptr [g_ViewParams_00566838 + 0x8]
        fstp dword ptr [edx - 0x4]
        fstp st(0)
        jnz L_474b74
        ret 0x4
    }
}

// 0x00474fc0 View_ExpFalloffLookup - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall View_ExpFalloffLookup(int, int, int)
{
    __asm {
        push ecx
        mov eax, dword ptr [g_Data_004da000 + 0x6e8c]
        xor ecx, ecx
        cmp eax, ecx
        jz L_47501d
        mov dword ptr [g_Data_004da000 + 0x8c9d0], 0x424c0000
        mov dword ptr [esp], ecx
        mov eax, offset g_Data_004da000 + 0x8c438
    L_474fdf:
        fild dword ptr [esp]
        mov edx, dword ptr [esp]
        add eax, 0x4
        inc edx
        cmp eax, offset g_ViewParams_00566838
        fmul dword ptr [g_RData_004cc000 + 0x69a0]
        mov dword ptr [esp], edx
        fchs
        fldl2e
        fmulp st(1), st(0)
        fld st(0)
        frndint
        fxch st(1)
        fsub st(0), st(1)
        f2xm1
        fld1
        faddp st(1), st(0)
        fscale
        fstp st(1)
        fstp dword ptr [eax - 0x4]
        jl L_474fdf
        mov dword ptr [g_Data_004da000 + 0x6e8c], ecx
    L_47501d:
        fld dword ptr [esp + 0x8]
        fcomp dword ptr [g_RData_004cc000 + 0x69a4]
        fnstsw AX
        test AH, 0x41
        jnz L_475038
        fld dword ptr [g_RData_004cc000 + 0x6960]
        pop ecx
        ret 0x4
    L_475038:
        fld dword ptr [esp + 0x8]
        fcomp qword ptr [g_RData_004cc000 + 0x6970]
        fnstsw AX
        test AH, 0x1
        jz L_475053
        fld dword ptr [g_RData_004cc000 + 0x697c]
        pop ecx
        ret 0x4
    L_475053:
        fld dword ptr [g_Data_004da000 + 0x8c9d0]
        fmul dword ptr [esp + 0x8]
        call dword ptr [g_Iat__ftol_004cc5ac]
        fld dword ptr [eax*0x4 + g_Data_004da000 + 0x8c438]
        pop ecx
        ret 0x4
    }
}

// 0x00479ce0 View_SetupFromCamera - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall View_SetupFromCamera(int, int)
{
    __asm {
        mov dword ptr [g_Data_004da000 + 0x9c214], ecx
        sub esp, 0x20
        fld dword ptr [ecx + 0xb0]
        fcomp qword ptr [g_RData_004cc000 + 0x6a80]
        fnstsw AX
        test AH, 0x1
        jz L_479d0c
        mov dword ptr [ecx + 0xb0], 0x3f800000
        mov ecx, dword ptr [g_Data_004da000 + 0x9c214]
    L_479d0c:
        fld dword ptr [ecx + 0xb0]
        mov eax, dword ptr [g_Data_004da000 + 0x91be8]
        fadd st(0), st(0)
        test eax, eax
        fstp dword ptr [g_Data_004da000 + 0x9c224]
        jnz L_479d3e
        fld dword ptr [g_RData_004cc000 + 0x6a8c]
        fdiv dword ptr [g_Data_004da000 + 0x9c224]
        push ecx
        fstp dword ptr [esp]
        call Render_FillVertexScratchColour
        mov ecx, dword ptr [g_Data_004da000 + 0x9c214]
    L_479d3e:
        mov eax, dword ptr [ecx + 0xb4]
        lea edx, [esp + 0x14]
        mov dword ptr [g_Data_004da000 + 0x9c230], eax
        mov ecx, dword ptr [ecx + 0x4]
        push edx
        lea edx, [esp + 0x14]
        call Class_GetCameraFields_0_4
        test eax, eax
        jz L_479d6e
        mov dword ptr [esp + 0x10], 0x0
        mov dword ptr [esp + 0x14], 0x0
    L_479d6e:
        mov ecx, dword ptr [g_Data_004da000 + 0x9c214]
        lea eax, [esp + 0x1c]
        push eax
        lea edx, [esp + 0x1c]
        mov ecx, dword ptr [ecx + 0x4]
        call Class_GetCameraFields_8_C
        test eax, eax
        jz L_479d9d
        call Render_GetSurfaceWidthCopy
        mov dword ptr [esp + 0x18], eax
        call Render_GetSurfaceHeightCopy
        mov dword ptr [esp + 0x1c], eax
        jmp L_479da1
    L_479d9d:
        mov eax, dword ptr [esp + 0x1c]
    L_479da1:
        mov ecx, dword ptr [g_Data_004da000 + 0x91be8]
        test ecx, ecx
        jz L_479e38
        mov edx, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esp + 0x18]
        add ecx, edx
        mov edx, dword ptr [esp + 0x14]
        add eax, edx
        mov dword ptr [esp + 0xc], ecx
        fild dword ptr [esp + 0xc]
        mov dword ptr [esp + 0x8], eax
        fild dword ptr [esp + 0x8]
        fild dword ptr [esp + 0x10]
        fld st(2)
        fxch st(2)
        fst dword ptr [esp]
        fxch st(2)
        fsub dword ptr [g_RData_004cc000 + 0x6a90]
        fxch st(2)
        fsub dword ptr [g_RData_004cc000 + 0x6a90]
        fild dword ptr [esp + 0x14]
        fxch st(2)
        fst dword ptr [esp + 0xc]
        fld st(0)
        fxch st(4)
        fst dword ptr [esp + 0x4]
        fld dword ptr [esp + 0x4]
        fxch st(4)
        fst dword ptr [esp + 0x4]
        fxch st(3)
        fst dword ptr [esp + 0x8]
        fld dword ptr [esp + 0x8]
        fxch st(6)
        fstp dword ptr [g_Data_004da000 + 0x9c21c]
        fxch st(1)
        fstp dword ptr [g_Data_004da000 + 0x9c228]
        fxch st(3)
        fstp dword ptr [g_Data_004da000 + 0x9c234]
        fxch st(1)
        fstp dword ptr [g_Data_004da000 + 0x9c220]
        fxch st(1)
        jmp L_479ee8
    L_479e38:
        mov ecx, dword ptr [esp + 0x10]
        mov edx, dword ptr [esp + 0x18]
        add edx, ecx
        mov ecx, dword ptr [esp + 0x14]
        add eax, ecx
        mov dword ptr [esp + 0xc], edx
        fild dword ptr [esp + 0xc]
        mov dword ptr [esp + 0x8], eax
        fild dword ptr [esp + 0x8]
        fild dword ptr [esp + 0x10]
        fild dword ptr [esp + 0x14]
        fld st(3)
        fxch st(2)
        fst dword ptr [esp + 0xc]
        fld st(0)
        fxch st(2)
        fst dword ptr [esp + 0x4]
        fxch st(4)
        fst dword ptr [esp]
        fxch st(2)
        fsub dword ptr [g_RData_004cc000 + 0x6a94]
        fxch st(3)
        fsub dword ptr [g_RData_004cc000 + 0x6a94]
        fxch st(4)
        fsub dword ptr [g_RData_004cc000 + 0x6a94]
        fxch st(2)
        fsub dword ptr [g_RData_004cc000 + 0x6a94]
        fld st(5)
        fld dword ptr [esp]
        fxch st(5)
        fsub dword ptr [g_RData_004cc000 + 0x6a98]
        fstp dword ptr [g_Data_004da000 + 0x9c21c]
        fsub dword ptr [g_RData_004cc000 + 0x6a9c]
        fxch st(5)
        fsub dword ptr [g_RData_004cc000 + 0x6aa0]
        fxch st(3)
        fsub dword ptr [g_RData_004cc000 + 0x6a98]
        fxch st(4)
        fsub dword ptr [g_RData_004cc000 + 0x6a9c]
        fxch st(1)
        fsub dword ptr [g_RData_004cc000 + 0x6aa0]
        fxch st(5)
        fstp dword ptr [g_Data_004da000 + 0x9c228]
        fxch st(2)
        fstp dword ptr [g_Data_004da000 + 0x9c234]
        fxch st(2)
        fstp dword ptr [g_Data_004da000 + 0x9c220]
    L_479ee8:
        fstp dword ptr [g_Data_004da000 + 0x9c22c]
        fxch st(1)
        fstp dword ptr [g_Data_004da000 + 0x9c238]
        mov edx, dword ptr [esp + 0x4]
        mov eax, dword ptr [g_Data_004da000 + 0x9c214]
        fstp dword ptr [g_Data_004da000 + 0x9c23c]
        mov dword ptr [g_Data_004da000 + 0x9c240], edx
        fsub dword ptr [g_RData_004cc000 + 0x6aa0]
        fstp dword ptr [g_Data_004da000 + 0x9c244]
        fld dword ptr [esp]
        fsub dword ptr [g_RData_004cc000 + 0x6aa0]
        fstp dword ptr [g_Data_004da000 + 0x9c248]
        mov ecx, dword ptr [eax + 0xb4]
        mov edx, dword ptr [eax + 0xb0]
        fild dword ptr [esp + 0x1c]
        push ecx
        mov ecx, dword ptr [eax + 0x1d8]
        push edx
        mov edx, dword ptr [eax + 0x1d4]
        fmul dword ptr [g_RData_004cc000 + 0x6aa4]
        push ecx
        push edx
        push ecx
        mov eax, dword ptr [esp + 0x18]
        fstp dword ptr [esp]
        fild dword ptr [esp + 0x2c]
        push ecx
        mov ecx, dword ptr [esp + 0x24]
        fmul dword ptr [g_RData_004cc000 + 0x6aa4]
        fstp dword ptr [esp]
        push eax
        push ecx
        call View_SetProjection
        mov eax, dword ptr [g_Data_004da000 + 0x9c214]
        mov edx, dword ptr [eax + 0xec]
        mov eax, dword ptr [eax + 0xe8]
        push edx
        push eax
        call View_SetFieldOfView
        add esp, 0x20
        ret
    }
}

// 0x004753e0 Tri_SetupTextureGradients - ../../04_spec/systems/view.md
// Register/stack shape from the listing (ECX, EDX, 24 stack bytes).
__declspec(naked) int __fastcall Tri_SetupTextureGradients(int, int, int, int, int, int, int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x54
        lea eax, [ebp - 0x30]
        mov dword ptr [ebp - 0x4], ecx
        mov dword ptr [ebp - 0xc], eax
        lea eax, [ecx + 0xc]
        push ebx
        add ecx, 0x18
        push esi
        push edi
        mov esi, edx
        mov dword ptr [ebp - 0x8], eax
        mov dword ptr [ebp - 0x10], ecx
        mov ebx, dword ptr [ebp - 0x10]
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
        lea ecx, [ebp - 0x3c]
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 0x10], ecx
        mov ebx, dword ptr [ebp - 0x4]
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
        lea edx, [ebp - 0x54]
        lea eax, [ebp - 0x3c]
        lea ecx, [ebp - 0x30]
        mov dword ptr [ebp - 0x10], edx
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 0x8], ecx
        mov ebx, dword ptr [ebp - 0x8]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [ebp - 0x10]
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
        lea edx, [ebp - 0x54]
        mov dword ptr [ebp - 0x10], edx
        mov ecx, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp - 0x4]
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
        fld dword ptr [ebp - 0x8]
        fcomp dword ptr [g_RData_004cc000 + 0x6960]
        xor ebx, ebx
        fnstsw AX
        test AH, 0x40
        jz L_4754ec
        mov edi, dword ptr [ebp + 0x8]
        mov eax, dword ptr [ebp + 0xc]
        mov dword ptr [edi], ebx
        mov dword ptr [edi + 0x4], ebx
        mov dword ptr [eax], 0x447a0000
        jmp L_47551b
    L_4754ec:
        fld dword ptr [g_RData_004cc000 + 0x697c]
        fdiv dword ptr [ebp - 0x8]
        mov edi, dword ptr [ebp + 0x8]
        mov ecx, dword ptr [ebp + 0xc]
        fld st(0)
        fmul dword ptr [ebp - 0x54]
        fmul dword ptr [g_ViewParams_00566838 + 0x28]
        fstp dword ptr [edi]
        fld st(0)
        fmul dword ptr [ebp - 0x50]
        fmul dword ptr [g_ViewParams_00566838 + 0x2c]
        fstp dword ptr [edi + 0x4]
        fmul dword ptr [ebp - 0x4c]
        fstp dword ptr [ecx]
    L_47551b:
        lea edx, [ebp - 0x30]
        mov dword ptr [ebp + 0x8], edx
        mov ecx, dword ptr [ebp + 0x8]
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
        lea eax, [ebp - 0x3c]
        mov dword ptr [ebp + 0x8], eax
        mov ecx, dword ptr [ebp + 0x8]
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
        lea ecx, [ebp - 0x3c]
        lea edx, [ebp - 0x30]
        mov dword ptr [ebp - 0x10], ecx
        mov dword ptr [ebp - 0x14], edx
        mov ecx, dword ptr [ebp - 0x14]
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
        fstp dword ptr [ebp + 0x8]
        fld dword ptr [ebp - 0xc]
        fmul dword ptr [ebp - 0x8]
        fld dword ptr [ebp + 0x8]
        fmul dword ptr [ebp + 0x8]
        fsubp st(1), st(0)
        fcom dword ptr [g_RData_004cc000 + 0x6960]
        fnstsw AX
        test AH, 0x40
        jz L_4755ca
        mov eax, dword ptr [ebp + 0x10]
        mov ecx, dword ptr [ebp + 0x1c]
        fstp st(0)
        mov dword ptr [eax], ebx
        mov dword ptr [eax + 0x4], ebx
        mov eax, dword ptr [ebp + 0x14]
        mov dword ptr [eax], ebx
        mov eax, dword ptr [ebp + 0x18]
        mov dword ptr [eax], ebx
        mov dword ptr [eax + 0x4], ebx
        mov dword ptr [ecx], ebx
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret 0x18
    L_4755ca:
        fdivr dword ptr [g_RData_004cc000 + 0x697c]
        lea edx, [ebp - 0x24]
        lea eax, [ebp - 0x48]
        lea ecx, [ebp - 0x24]
        mov dword ptr [ebp - 0x14], edx
        mov dword ptr [ebp - 0x10], eax
        mov dword ptr [ebp - 0x18], ecx
        fld dword ptr [ebp - 0x8]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0x8]
        fld dword ptr [ebp + 0x8]
        fmul st(0), st(1)
        fstp dword ptr [ebp + 0x8]
        fld dword ptr [ebp - 0xc]
        fmul st(0), st(1)
        fstp dword ptr [ebp - 0xc]
        fstp st(0)
        fld dword ptr [esi + 0x10]
        fsub dword ptr [esi + 0x8]
        fld dword ptr [esi]
        fsub dword ptr [esi + 0x8]
        fld st(1)
        fmul dword ptr [ebp - 0xc]
        fld st(1)
        fmul dword ptr [ebp + 0x8]
        fsubp st(1), st(0)
        fld st(0)
        fmul dword ptr [ebp - 0x30]
        fstp dword ptr [ebp - 0x48]
        fld st(0)
        fmul dword ptr [ebp - 0x2c]
        fstp dword ptr [ebp - 0x44]
        fmul dword ptr [ebp - 0x28]
        fstp dword ptr [ebp - 0x40]
        fmul dword ptr [ebp - 0x8]
        fxch st(1)
        fmul dword ptr [ebp + 0x8]
        fsubp st(1), st(0)
        fld st(0)
        fmul dword ptr [ebp - 0x3c]
        fstp dword ptr [ebp - 0x24]
        fld st(0)
        fmul dword ptr [ebp - 0x38]
        fstp dword ptr [ebp - 0x20]
        fmul dword ptr [ebp - 0x34]
        fstp dword ptr [ebp - 0x1c]
        mov ebx, dword ptr [ebp - 0x18]
        mov ecx, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp - 0x14]
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
        lea edx, [ebp - 0x24]
        mov dword ptr [ebp - 0x18], edx
        mov ecx, dword ptr [ebp - 0x18]
        mov edx, dword ptr [ebp - 0x4]
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
        fld dword ptr [esi]
        fsub dword ptr [ebp - 0x14]
        mov eax, dword ptr [ebp + 0x10]
        mov ecx, dword ptr [ebp + 0x14]
        fld st(0)
        fmul dword ptr [edi]
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [g_ViewParams_00566838 + 0x28]
        faddp st(1), st(0)
        fstp dword ptr [eax]
        fld st(0)
        fmul dword ptr [edi + 0x4]
        fld dword ptr [ebp - 0x20]
        fmul dword ptr [g_ViewParams_00566838 + 0x2c]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x4]
        mov eax, dword ptr [ebp + 0xc]
        fmul dword ptr [eax]
        fadd dword ptr [ebp - 0x1c]
        fstp dword ptr [ecx]
        fld dword ptr [esi + 0x14]
        fsub dword ptr [esi + 0xc]
        fld dword ptr [esi + 0x4]
        fsub dword ptr [esi + 0xc]
        fld st(1)
        fmul dword ptr [ebp - 0xc]
        fld st(1)
        fmul dword ptr [ebp + 0x8]
        fsubp st(1), st(0)
        fld st(0)
        fmul dword ptr [ebp - 0x30]
        fstp dword ptr [ebp - 0x48]
        lea edx, [ebp - 0x24]
        lea ecx, [ebp - 0x48]
        fld st(0)
        fmul dword ptr [ebp - 0x2c]
        mov dword ptr [ebp + 0xc], edx
        lea edx, [ebp - 0x24]
        mov dword ptr [ebp + 0x10], edx
        fstp dword ptr [ebp - 0x44]
        fmul dword ptr [ebp - 0x28]
        fstp dword ptr [ebp - 0x40]
        fmul dword ptr [ebp - 0x8]
        fxch st(1)
        fmul dword ptr [ebp + 0x8]
        mov dword ptr [ebp + 0x8], ecx
        fsubp st(1), st(0)
        fld st(0)
        fmul dword ptr [ebp - 0x3c]
        fstp dword ptr [ebp - 0x24]
        fld st(0)
        fmul dword ptr [ebp - 0x38]
        fstp dword ptr [ebp - 0x20]
        fmul dword ptr [ebp - 0x34]
        fstp dword ptr [ebp - 0x1c]
        mov ebx, dword ptr [ebp + 0x10]
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
        lea ecx, [ebp - 0x24]
        mov dword ptr [ebp + 0xc], ecx
        mov ecx, dword ptr [ebp + 0xc]
        mov edx, dword ptr [ebp - 0x4]
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fld dword ptr [ecx + 0x4]
        fmul dword ptr [edx + 0x4]
        fld dword ptr [ecx + 0x8]
        fmul dword ptr [edx + 0x8]
        fxch st(1)
        faddp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [ebp + 0x8]
        fld dword ptr [esi + 0x4]
        fsub dword ptr [ebp + 0x8]
        mov ecx, dword ptr [ebp + 0x18]
        mov edx, dword ptr [ebp + 0x1c]
        fld st(0)
        fmul dword ptr [edi]
        fld dword ptr [ebp - 0x24]
        fmul dword ptr [g_ViewParams_00566838 + 0x28]
        faddp st(1), st(0)
        fstp dword ptr [ecx]
        fld st(0)
        fmul dword ptr [edi + 0x4]
        fld dword ptr [ebp - 0x20]
        fmul dword ptr [g_ViewParams_00566838 + 0x2c]
        pop edi
        pop esi
        pop ebx
        faddp st(1), st(0)
        fstp dword ptr [ecx + 0x4]
        fmul dword ptr [eax]
        fadd dword ptr [ebp - 0x1c]
        fstp dword ptr [edx]
        mov esp, ebp
        pop ebp
        ret 0x18
    }
}

}  // namespace recoil
