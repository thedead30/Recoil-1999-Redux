// SUBSYSTEM: settings
// Settings functions with no attributed original source file (ledger orig_file empty); their placement here is a
// layout choice, not a provenance claim (STAGE2.md section 2). Spec: 04_spec/systems/settings.md
// Instruction-level ports converted from the Ghidra listings with tools/asm_port/ghidra2masm.py.
#include "unattributed/settings.h"
#include "unattributed/sysinfo.h"  // Crt_ChkStk 0x004c6100
#include "platform/advapi.h"  // ADVAPI32 import slots (registry)
#include "platform/msvcrt.h"  // MSVCRT import slots (calloc, free, strncpy, _strdup)
#include "platform/image/original_data.h"
#include "unattributed/asset_io_misc.h"
#include "GameZRecoil/zReader/zreader.h"
#include "GameZRecoil/zSound/zsnd_parm.h"
#include "unattributed/view.h"
#include "unattributed/input.h"
#include "unattributed/menus.h"
#include "platform/iat_msvcrt.h"

namespace recoil {

// .bss, zero at load: CONFIRMED-DATA (0x004e5d00 lies past the file-backed part of .data, raw size 0xbc00).
std::uint32_t g_SettingsBlock_004e5d00[kSettingsBlockWords];
// .bss, zero at load: CONFIRMED-DATA. +0: head of the registered-node list; +4: initialised flag tested by
// Settings_Shutdown; +8/+0xc/+0x10: three heap strings it frees (paths set by Settings_InitPathsAndAutoLoad).
std::uint32_t g_SettingsNodeState_0056bcd0[kSettingsNodeStateWords];

namespace {
const double kD_004ccd98 = 0.02;  // CONFIRMED-BINARY: double 0x3f947ae147ae147b at 0x004ccd98
const char kStr_004da63c[] = "~=";  // CONFIRMED-DATA: .data string at 0x004da63c
const char kStr_004da640[] = "!=";  // CONFIRMED-DATA: .data string at 0x004da640
const char kStr_004da644[] = ">=";  // CONFIRMED-DATA: .data string at 0x004da644
const char kStr_004da648[] = "<=";  // CONFIRMED-DATA: .data string at 0x004da648
const char kStr_004da64c[] = ">";  // CONFIRMED-DATA: .data string at 0x004da64c
const char kStr_004da650[] = "<";  // CONFIRMED-DATA: .data string at 0x004da650
const char kStr_004da654[] = "==";  // CONFIRMED-DATA: .data string at 0x004da654
}  // namespace

// 0x00407220 Preset_CompareOp - (rhs) ECX=operator string, EDX=lhs ret 4: "==" eq, "<" lt, ">" gt, "<=" le, ">=" ge, "!=" ne, "~=" always true; unknown -> false
__declspec(naked) int __fastcall Preset_CompareOp(int, int, int)
{
    __asm {
        push ecx
        push ebx
        push ebp
        push esi
        push edi
        mov dword ptr [esp + 0x10], edx
        xor ebp, ebp
        mov edi, offset kStr_004da654
        mov esi, ecx
    L_407232:
        mov AL, byte ptr [esi]
        mov BL, AL
        cmp AL, byte ptr [edi]
        jnz L_407256
        test BL, BL
        jz L_407252
        mov AL, byte ptr [esi + 0x1]
        mov BL, AL
        cmp AL, byte ptr [edi + 0x1]
        jnz L_407256
        add esi, 0x2
        add edi, 0x2
        test BL, BL
        jnz L_407232
    L_407252:
        xor esi, esi
        jmp L_40725b
    L_407256:
        sbb esi, esi
        sbb esi, -0x1
    L_40725b:
        test esi, esi
        jnz L_407276
        mov eax, dword ptr [esp + 0x18]
        xor ecx, ecx
        cmp edx, eax
        setz CL
        mov ebp, ecx
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_407276:
        mov edi, offset kStr_004da650
        mov esi, ecx
    L_40727d:
        mov AL, byte ptr [esi]
        mov BL, AL
        cmp AL, byte ptr [edi]
        jnz L_4072a1
        test BL, BL
        jz L_40729d
        mov AL, byte ptr [esi + 0x1]
        mov BL, AL
        cmp AL, byte ptr [edi + 0x1]
        jnz L_4072a1
        add esi, 0x2
        add edi, 0x2
        test BL, BL
        jnz L_40727d
    L_40729d:
        xor esi, esi
        jmp L_4072a6
    L_4072a1:
        sbb esi, esi
        sbb esi, -0x1
    L_4072a6:
        test esi, esi
        jnz L_4072c1
        mov eax, dword ptr [esp + 0x18]
        xor ecx, ecx
        cmp edx, eax
        setl CL
        mov ebp, ecx
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_4072c1:
        mov edi, offset kStr_004da64c
        mov esi, ecx
    L_4072c8:
        mov AL, byte ptr [esi]
        mov BL, AL
        cmp AL, byte ptr [edi]
        jnz L_4072ec
        test BL, BL
        jz L_4072e8
        mov AL, byte ptr [esi + 0x1]
        mov BL, AL
        cmp AL, byte ptr [edi + 0x1]
        jnz L_4072ec
        add esi, 0x2
        add edi, 0x2
        test BL, BL
        jnz L_4072c8
    L_4072e8:
        xor esi, esi
        jmp L_4072f1
    L_4072ec:
        sbb esi, esi
        sbb esi, -0x1
    L_4072f1:
        test esi, esi
        jnz L_40730c
        mov eax, dword ptr [esp + 0x18]
        xor ecx, ecx
        cmp edx, eax
        setg CL
        mov ebp, ecx
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_40730c:
        mov edi, offset kStr_004da648
        mov esi, ecx
    L_407313:
        mov AL, byte ptr [esi]
        mov BL, AL
        cmp AL, byte ptr [edi]
        jnz L_407337
        test BL, BL
        jz L_407333
        mov AL, byte ptr [esi + 0x1]
        mov BL, AL
        cmp AL, byte ptr [edi + 0x1]
        jnz L_407337
        add esi, 0x2
        add edi, 0x2
        test BL, BL
        jnz L_407313
    L_407333:
        xor esi, esi
        jmp L_40733c
    L_407337:
        sbb esi, esi
        sbb esi, -0x1
    L_40733c:
        test esi, esi
        jnz L_407357
        mov eax, dword ptr [esp + 0x18]
        xor ecx, ecx
        cmp edx, eax
        setle CL
        mov ebp, ecx
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_407357:
        mov edi, offset kStr_004da644
        mov esi, ecx
    L_40735e:
        mov AL, byte ptr [esi]
        mov BL, AL
        cmp AL, byte ptr [edi]
        jnz L_407382
        test BL, BL
        jz L_40737e
        mov AL, byte ptr [esi + 0x1]
        mov BL, AL
        cmp AL, byte ptr [edi + 0x1]
        jnz L_407382
        add esi, 0x2
        add edi, 0x2
        test BL, BL
        jnz L_40735e
    L_40737e:
        xor esi, esi
        jmp L_407387
    L_407382:
        sbb esi, esi
        sbb esi, -0x1
    L_407387:
        test esi, esi
        jnz L_4073a2
        mov eax, dword ptr [esp + 0x18]
        xor ecx, ecx
        cmp edx, eax
        setge CL
        mov ebp, ecx
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_4073a2:
        mov edi, offset kStr_004da640
        mov esi, ecx
    L_4073a9:
        mov AL, byte ptr [esi]
        mov BL, AL
        cmp AL, byte ptr [edi]
        jnz L_4073cd
        test BL, BL
        jz L_4073c9
        mov AL, byte ptr [esi + 0x1]
        mov BL, AL
        cmp AL, byte ptr [edi + 0x1]
        jnz L_4073cd
        add esi, 0x2
        add edi, 0x2
        test BL, BL
        jnz L_4073a9
    L_4073c9:
        xor esi, esi
        jmp L_4073d2
    L_4073cd:
        sbb esi, esi
        sbb esi, -0x1
    L_4073d2:
        test esi, esi
        jnz L_4073ed
        mov eax, dword ptr [esp + 0x18]
        xor ecx, ecx
        cmp edx, eax
        setnz CL
        mov ebp, ecx
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_4073ed:
        mov esi, offset kStr_004da63c
    L_4073f2:
        mov AL, byte ptr [ecx]
        mov BL, AL
        cmp AL, byte ptr [esi]
        jnz L_407416
        test BL, BL
        jz L_407412
        mov AL, byte ptr [ecx + 0x1]
        mov BL, AL
        cmp AL, byte ptr [esi + 0x1]
        jnz L_407416
        add ecx, 0x2
        add esi, 0x2
        test BL, BL
        jnz L_4073f2
    L_407412:
        xor ecx, ecx
        jmp L_40741b
    L_407416:
        sbb ecx, ecx
        sbb ecx, -0x1
    L_40741b:
        test ecx, ecx
        jnz L_407458
        mov edi, dword ptr [esp + 0x18]
        mov eax, edx
        sub eax, edi
        cdq
        xor eax, edx
        sub eax, edx
        mov dword ptr [esp + 0x18], eax
        fild dword ptr [esp + 0x18]
        fild dword ptr [esp + 0x10]
        fmul qword ptr [kD_004ccd98]
        fcompp
        fnstsw AX
        test AH, 0x41
        jnz L_407456
        mov ebp, 0x1
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    L_407456:
        xor ebp, ebp
    L_407458:
        pop edi
        mov eax, ebp
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret 0x4
    }
}

// 0x004076f0 Stub_Ret - ret
__declspec(naked) int __fastcall Stub_Ret(int, int)
{
    __asm {
        ret
    }
}

// 0x00407e20 Settings_StoreGameCtlOptions - *[0x004e5d3c] = ECX
__declspec(naked) int __fastcall Settings_StoreGameCtlOptions(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x3c]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00407f10 Settings_StoreGameIntensity - [[0x004e5d48]] = ECX.
__declspec(naked) int __fastcall Settings_StoreGameIntensity(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x48]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00407f20 Settings_GetGameIntensity - Returns [[0x004e5d48]] (pair of Settings_StoreGameIntensity).
__declspec(naked) int __fastcall Settings_GetGameIntensity(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x48]
        mov eax, dword ptr [eax]
        ret
    }
}

// 0x004080b0 RecoilApp_GetSoundAPICheckboxValue - return *[0x004e5d34]
__declspec(naked) int __fastcall RecoilApp_GetSoundAPICheckboxValue(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x34]
        mov eax, dword ptr [eax]
        ret
    }
}

// 0x00408230 Settings_SetNetworkFlag - *[0x004e5d74] = ECX
__declspec(naked) int __fastcall Settings_SetNetworkFlag(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x74]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00408240 Settings_StoreNetworkModem - *[0x004e5d90] = ECX
__declspec(naked) int __fastcall Settings_StoreNetworkModem(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x90]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00408250 Settings_StoreNetListen - *[0x004e5d78] = ECX
__declspec(naked) int __fastcall Settings_StoreNetListen(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x78]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00408280 Settings_ApplyHWCardFlag - *[0x004e5d58] = ECX; [0x004e5dcc] = ECX
__declspec(naked) int __fastcall Settings_ApplyHWCardFlag(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x58]
        mov dword ptr [eax], ecx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ecx
        ret
    }
}

// 0x00408290 Settings_StoreHWAPI - *[0x004e5d5c] = ECX
__declspec(naked) int __fastcall Settings_StoreHWAPI(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x5c]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x004082a0 Settings_StoreFullScreen - *[0x004e5d54] = ECX
__declspec(naked) int __fastcall Settings_StoreFullScreen(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x54]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x004082b0 Settings_StoreHUDFlag - if [0x004e5dcc]: *[0x004e5d24]=ECX else *[0x004e5d20]=ECX
__declspec(naked) int __fastcall Settings_StoreHUDFlag(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0xcc]
        test eax, eax
        jz L_4082c1
        mov eax, [g_SettingsBlock_004e5d00 + 0x24]
        mov dword ptr [eax], ecx
        ret
    L_4082c1:
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x20]
        mov dword ptr [edx], ecx
        ret
    }
}

// 0x00408300 Settings_Store_004e5d6c - *[0x004e5d6c] = ECX
__declspec(naked) int __fastcall Settings_Store_004e5d6c(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x6c]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00408320 Settings_GetHWAPI - return *[0x004e5d5c]
__declspec(naked) int __fastcall Settings_GetHWAPI(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x5c]
        mov eax, dword ptr [eax]
        ret
    }
}

// 0x00408330 Settings_GetFullScreen - return *[0x004e5d54]
__declspec(naked) int __fastcall Settings_GetFullScreen(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x54]
        mov eax, dword ptr [eax]
        ret
    }
}

// 0x00408340 Settings_GetSplitScreenValue - Returns [[0x004e5d24]] when [0x004e5dcc] nonzero else [[0x004e5d20]].
__declspec(naked) int __fastcall Settings_GetSplitScreenValue(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0xcc]
        test eax, eax
        jz L_408351
        mov eax, [g_SettingsBlock_004e5d00 + 0x24]
        mov eax, dword ptr [eax]
        ret
    L_408351:
        mov ecx, dword ptr [g_SettingsBlock_004e5d00 + 0x20]
        mov eax, dword ptr [ecx]
        ret
    }
}

// 0x004083a0 Settings_StoreJoystickNumAxes - *[0x004e5d64] = ECX
__declspec(naked) int __fastcall Settings_StoreJoystickNumAxes(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x64]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x004083b0 Settings_StoreJoystickNumButtons - *[0x004e5d68] = ECX
__declspec(naked) int __fastcall Settings_StoreJoystickNumButtons(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x68]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00408660 Settings_GetDisplayRect_10 - return [*[0x004e5d84]]+0x10
__declspec(naked) int __fastcall Settings_GetDisplayRect_10(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x84]
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [ecx + 0x10]
        ret
    }
}

// 0x00408670 Settings_GetDisplayRect_14 - return [*[0x004e5d84]]+0x14
__declspec(naked) int __fastcall Settings_GetDisplayRect_14(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x84]
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [ecx + 0x14]
        ret
    }
}

// 0x00408680 Settings_SetDisplayRect_20 - [*[0x004e5d84]]+0x20 = ECX
__declspec(naked) int __fastcall Settings_SetDisplayRect_20(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x84]
        mov edx, dword ptr [eax]
        mov dword ptr [edx + 0x20], ecx
        ret
    }
}

// 0x00408690 Settings_GetDisplayRect_20 - return [*[0x004e5d84]]+0x20
__declspec(naked) int __fastcall Settings_GetDisplayRect_20(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x84]
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [ecx + 0x20]
        ret
    }
}

// 0x004086a0 Settings_Get_004e5d70 - return *[0x004e5d70]
__declspec(naked) int __fastcall Settings_Get_004e5d70(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x70]
        mov eax, dword ptr [eax]
        ret
    }
}

// 0x004086c0 Settings_GetValue_004e5d88 - Returns [[0x004e5d88]].
__declspec(naked) int __fastcall Settings_GetValue_004e5d88(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x88]
        mov eax, dword ptr [eax]
        ret
    }
}

// 0x004086d0 Settings_GetScreenRect_14 - return [*[0x004e5d88]]+0x14
__declspec(naked) int __fastcall Settings_GetScreenRect_14(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x88]
        mov ecx, dword ptr [eax]
        mov eax, dword ptr [ecx + 0x14]
        ret
    }
}

// 0x00408a10 Settings_StoreWOLPasswordFlag - *[0x004e5d94] = ECX
__declspec(naked) int __fastcall Settings_StoreWOLPasswordFlag(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x94]
        mov dword ptr [eax], ecx
        ret
    }
}

// 0x00408a20 Settings_GetWOLPasswordFlag - return *[0x004e5d94]
__declspec(naked) int __fastcall Settings_GetWOLPasswordFlag(int, int)
{
    __asm {
        mov eax, [g_SettingsBlock_004e5d00 + 0x94]
        mov eax, dword ptr [eax]
        ret
    }
}

// 0x004b3380 Settings_FindNodeByName - ../../04_spec/systems/sound.md
__declspec(naked) int __fastcall Settings_FindNodeByName(int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0]
        mov ebp, ecx
        test edi, edi
        jz L_4b33db
    L_4b3390:
        mov esi, dword ptr [edi + 0x10]
        mov edx, ebp
    L_4b3395:
        mov AL, byte ptr [edx]
        mov CL, byte ptr [esi]
        mov BL, AL
        cmp AL, CL
        jnz L_4b33bd
        test BL, BL
        jz L_4b33b9
        mov CL, byte ptr [edx + 0x1]
        mov AL, byte ptr [esi + 0x1]
        mov BL, CL
        cmp CL, AL
        jnz L_4b33bd
        add edx, 0x2
        add esi, 0x2
        test BL, BL
        jnz L_4b3395
    L_4b33b9:
        xor edx, edx
        jmp L_4b33c2
    L_4b33bd:
        sbb edx, edx
        sbb edx, -0x1
    L_4b33c2:
        test edx, edx
        jz L_4b33d4
        mov edi, dword ptr [edi + 0x18]
        test edi, edi
        jnz L_4b3390
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    L_4b33d4:
        mov eax, edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    L_4b33db:
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        ret
    }
}

// 0x004b2e80 Settings_RegisterNode - ../../04_spec/systems/settings.md
__declspec(naked) int __fastcall Settings_RegisterNode(int, int, int, int)
{
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov edi, edx
        mov ebx, ecx
        call Settings_FindNodeByName
        mov esi, eax
        test esi, esi
        jnz L_4b2f1d
        mov ebp, dword ptr [g_Iat_calloc_004cc4ac]
        push 0x20
        push 0x1
        call ebp
        add esp, 0x8
        mov esi, eax
        push ebx
        call dword ptr [g_Iat__strdup_004cc5e4]
        mov ecx, dword ptr [esp + 0x1c]
        mov dword ptr [esi + 0x10], eax
        mov eax, dword ptr [esp + 0x18]
        add esp, 0x4
        cmp edi, 0x7
        mov dword ptr [esi + 0x8], edi
        mov dword ptr [esi + 0xc], eax
        mov dword ptr [esi + 0x14], ecx
        ja L_4b2f0e
        cmp edi, 0
        je L_4b2edb
        cmp edi, 1
        je L_4b2edb
        cmp edi, 2
        je L_4b2ee4
        cmp edi, 3
        je L_4b2eed
        cmp edi, 4
        je L_4b2eed
        cmp edi, 5
        je L_4b2eed
        cmp edi, 6
        je L_4b2eed
        cmp edi, 7
        je L_4b2eed
        int 3  // unreachable: the bounds check above excludes other indices
    L_4b2edb:
        mov dword ptr [esi + 0xc], 0x4
        jmp L_4b2f0e
    L_4b2ee4:
        mov dword ptr [esi + 0xc], 0x8
        jmp L_4b2f0e
    L_4b2eed:
        test eax, eax
        jnz L_4b2f04
        push esi
        call dword ptr [g_Iat_free_004cc5b4]
        add esp, 0x4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x8
    L_4b2f04:
        push eax
        push 0x1
        call ebp
        add esp, 0x8
        mov dword ptr [esi], eax
    L_4b2f0e:
        mov edx, dword ptr [g_SettingsNodeState_0056bcd0]
        mov dword ptr [esi + 0x18], edx
        mov dword ptr [g_SettingsNodeState_0056bcd0], esi
    L_4b2f1d:
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 0x8
    }
}

// 0x004b32c0 Settings_Shutdown - ../../04_spec/systems/settings.md
__declspec(naked) int __fastcall Settings_Shutdown(int, int)
{
    __asm {
        mov eax, [g_SettingsNodeState_0056bcd0 + 4]
        push ebp
        xor ebp, ebp
        cmp eax, ebp
        jz L_4b3373
        push edi
        mov edi, dword ptr [g_Iat_free_004cc5b4]
        push esi
        mov esi, dword ptr [g_SettingsNodeState_0056bcd0]
        cmp esi, ebp
        push ebx
        jz L_4b332b
        cmp esi, ebp
        jz L_4b3325
    L_4b32e7:
        mov eax, dword ptr [esi + 0x10]
        mov ebx, dword ptr [esi + 0x18]
        cmp eax, ebp
        jz L_4b32fa
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [esi + 0x10], ebp
    L_4b32fa:
        mov eax, dword ptr [esi + 0x8]
        cmp eax, ebp
        jz L_4b3319
        cmp eax, 0x2
        jle L_4b3319
        cmp eax, 0x7
        jg L_4b3319
        mov eax, dword ptr [esi]
        cmp eax, ebp
        jz L_4b3319
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [esi], ebp
    L_4b3319:
        push esi
        call edi
        add esp, 0x4
        cmp ebx, ebp
        mov esi, ebx
        jnz L_4b32e7
    L_4b3325:
        mov dword ptr [g_SettingsNodeState_0056bcd0], ebp
    L_4b332b:
        mov eax, [g_SettingsNodeState_0056bcd0 + 16]
        cmp eax, ebp
        jz L_4b3340
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 16], ebp
    L_4b3340:
        mov eax, [g_SettingsNodeState_0056bcd0 + 12]
        cmp eax, ebp
        jz L_4b3355
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 12], ebp
    L_4b3355:
        mov eax, [g_SettingsNodeState_0056bcd0 + 8]
        cmp eax, ebp
        jz L_4b336a
        push eax
        call edi
        add esp, 0x4
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 8], ebp
    L_4b336a:
        pop ebx
        pop esi
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 4], ebp
        pop edi
    L_4b3373:
        pop ebp
        ret
    }
}

// 0x00408120 Settings_StorePlayerName - copy ECX string into name buffer *[0x004e5d4c] (capacity [+0xc]); longer -> strncpy cap-1 and terminate
__declspec(naked) int __fastcall Settings_StorePlayerName(int, int)
{
    __asm {
        push ebx
        push esi
        mov edx, ecx
        push edi
        mov edi, edx
        or ecx, 0xffffffff
        xor eax, eax
        mov ebx, dword ptr [g_SettingsBlock_004e5d00 + 0x4c]
        repne scasb
        mov esi, dword ptr [ebx + 0xc]
        not ecx
        dec ecx
        cmp ecx, esi
        jnc L_40815f
        mov edi, edx
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, dword ptr [ebx]
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        and ecx, 0x3
        rep movsb
        pop edi
        pop esi
        pop ebx
        ret
    L_40815f:
        mov ecx, dword ptr [ebx]
        dec esi
        push esi
        push edx
        push ecx
        call dword ptr [g_Iat_strncpy_004cc5a0]
        mov eax, [g_SettingsBlock_004e5d00 + 0x4c]
        add esp, 0xc
        mov edx, dword ptr [eax + 0xc]
        mov eax, dword ptr [eax]
        pop edi
        pop esi
        mov byte ptr [edx + eax*0x1 - 0x1], 0x0
        pop ebx
        ret
    }
}

// Registry path pieces (.data, CONFIRMED-DATA from Recoil.exe): 0x004e4668 holds a pointer to 0x004e466c (relocated to
// this copy); 0x004e466c..0x004e467b are the 16 file bytes there ("SOFTWARE\\" then "\\").
unsigned char g_SettingsRegText_004e466c[16] = {0x53, 0x4f, 0x46, 0x54, 0x57, 0x41, 0x52, 0x45, 0x5c, 0x00, 0x00, 0x00, 0x5c, 0x00, 0x00, 0x00};
unsigned char* g_SettingsRegRoot_004e4668 = g_SettingsRegText_004e466c;

// 0x004b2960 Settings_LoadFromRegistry - ../../04_spec/systems/settings.md
__declspec(naked) int __fastcall Settings_LoadFromRegistry(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x14
        push ebx
        push esi
        push edi
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 8]
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 12]
        not ecx
        dec ecx
        mov edx, ecx
        or ecx, 0xffffffff
        repne scasb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 16]
        not ecx
        dec ecx
        add edx, ecx
        or ecx, 0xffffffff
        repne scasb
        mov edi, dword ptr [g_SettingsRegRoot_004e4668]
        not ecx
        dec ecx
        add edx, ecx
        or ecx, 0xffffffff
        repne scasb
        not ecx
        dec ecx
        lea eax, [edx + ecx*0x1 + 0x5]
        add eax, 0x3
        and AL, 0xfc
        call Crt_ChkStk
        mov ecx, dword ptr [g_SettingsRegText_004e466c]
        mov ebx, esp
        mov eax, ebx
        mov dword ptr [eax], ecx
        mov edx, dword ptr [g_SettingsRegText_004e466c + 4]
        mov dword ptr [eax + 0x4], edx
        mov CX, word ptr [g_SettingsRegText_004e466c + 8]
        mov word ptr [eax + 0x8], CX
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 8]
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, offset g_SettingsRegText_004e466c + 12
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 12]
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        or ecx, 0xffffffff
        mov edi, ebx
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, offset g_SettingsRegText_004e466c + 12
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 16]
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        lea eax, [ebp - 0x8]
        and ecx, 0x3
        push eax
        push 0x20019
        push 0x0
        rep movsb
        mov esi, dword ptr [g_Iat_RegOpenKeyExA_004cc00c]
        push ebx
        push 0x80000001
        call esi
        test eax, eax
        jz L_4b2acd
        xor eax, eax
        lea esp, [ebp - 0x20]
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_4b2acd:
        lea ecx, [ebp - 0xc]
        push ecx
        push 0x20019
        push 0x0
        push ebx
        push 0x80000002
        call esi
        test eax, eax
        jz L_4b2afa
        mov edx, dword ptr [ebp - 0x8]
        push edx
        call dword ptr [g_Iat_RegCloseKey_004cc004]
        xor eax, eax
        lea esp, [ebp - 0x20]
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_4b2afa:
        mov esi, dword ptr [g_SettingsNodeState_0056bcd0]
        test esi, esi
        jz L_4b2ba1
        mov edi, dword ptr [ebp - 0x14]
    L_4b2b0b:
        mov eax, dword ptr [esi + 0x14]
        cmp eax, 0x1
        jnz L_4b2b18
        lea ebx, [ebp - 0x8]
        jmp L_4b2b1f
    L_4b2b18:
        test eax, eax
        jnz L_4b2b96
        lea ebx, [ebp - 0xc]
    L_4b2b1f:
        mov eax, dword ptr [esi + 0x8]
        cmp eax, 0x7
        ja L_4b2b57
        cmp eax, 0
        je L_4b2b39
        cmp eax, 1
        je L_4b2b39
        cmp eax, 2
        je L_4b2b44
        cmp eax, 3
        je L_4b2b4f
        cmp eax, 4
        je L_4b2b4f
        cmp eax, 5
        je L_4b2b4f
        cmp eax, 6
        je L_4b2b4f
        cmp eax, 7
        je L_4b2b4f
        int 3  // unreachable: the bounds check above excludes other indices
    L_4b2b39:
        mov dword ptr [ebp - 0x4], 0x4
        mov edi, esi
        jmp L_4b2b57
    L_4b2b44:
        mov dword ptr [ebp - 0x4], 0x8
        mov edi, esi
        jmp L_4b2b57
    L_4b2b4f:
        mov eax, dword ptr [esi + 0xc]
        mov edi, dword ptr [esi]
        mov dword ptr [ebp - 0x4], eax
    L_4b2b57:
        mov eax, dword ptr [esi + 0x10]
        lea ecx, [ebp - 0x10]
        push ecx
        mov ecx, dword ptr [ebx]
        lea edx, [ebp - 0x14]
        push 0x0
        push edx
        push 0x0
        push eax
        push ecx
        call dword ptr [g_Iat_RegQueryValueExA_004cc010]
        test eax, eax
        jnz L_4b2b96
        mov edx, dword ptr [ebp - 0x10]
        mov eax, dword ptr [ebp - 0x4]
        cmp edx, eax
        jnz L_4b2b96
        mov edx, dword ptr [esi + 0x10]
        lea eax, [ebp - 0x4]
        push eax
        mov eax, dword ptr [ebx]
        lea ecx, [ebp - 0x14]
        push edi
        push ecx
        push 0x0
        push edx
        push eax
        call dword ptr [g_Iat_RegQueryValueExA_004cc010]
    L_4b2b96:
        mov esi, dword ptr [esi + 0x18]
        test esi, esi
        jnz L_4b2b0b
    L_4b2ba1:
        mov ecx, dword ptr [ebp - 0x8]
        mov esi, dword ptr [g_Iat_RegCloseKey_004cc004]
        push ecx
        call esi
        mov edx, dword ptr [ebp - 0xc]
        push edx
        call esi
        lea esp, [ebp - 0x20]
        mov eax, 0x1
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x004b2bf0 Settings_SaveToRegistry - ../../04_spec/systems/settings.md
__declspec(naked) int __fastcall Settings_SaveToRegistry(int, int)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xc
        push ebx
        push esi
        push edi
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 8]
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 12]
        mov esi, dword ptr [g_SettingsRegRoot_004e4668]
        not ecx
        dec ecx
        mov edx, ecx
        or ecx, 0xffffffff
        repne scasb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 16]
        not ecx
        dec ecx
        add edx, ecx
        or ecx, 0xffffffff
        repne scasb
        not ecx
        dec ecx
        mov edi, esi
        add edx, ecx
        or ecx, 0xffffffff
        repne scasb
        not ecx
        dec ecx
        lea eax, [edx + ecx*0x1 + 0x5]
        add eax, 0x3
        and AL, 0xfc
        call Crt_ChkStk
        or ecx, 0xffffffff
        mov edi, esi
        xor eax, eax
        mov ebx, esp
        repne scasb
        not ecx
        sub edi, ecx
        mov eax, ecx
        mov esi, edi
        mov edi, ebx
        shr ecx, 0x2
        rep movsd
        mov ecx, eax
        xor eax, eax
        and ecx, 0x3
        rep movsb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 8]
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, offset g_SettingsRegText_004e466c + 12
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 12]
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov edx, ecx
        mov esi, edi
        or ecx, 0xffffffff
        mov edi, ebx
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, offset g_SettingsRegText_004e466c + 12
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        and ecx, 0x3
        rep movsb
        mov edi, dword ptr [g_SettingsNodeState_0056bcd0 + 16]
        or ecx, 0xffffffff
        repne scasb
        not ecx
        sub edi, ecx
        mov esi, edi
        mov edx, ecx
        mov edi, ebx
        or ecx, 0xffffffff
        repne scasb
        mov ecx, edx
        dec edi
        shr ecx, 0x2
        rep movsd
        mov ecx, edx
        lea eax, [ebp - 0xc]
        and ecx, 0x3
        push eax
        rep movsb
        lea ecx, [ebp - 0x4]
        mov esi, dword ptr [g_Iat_RegCreateKeyExA_004cc008]
        push ecx
        push 0x0
        push 0x20006
        push 0x0
        push 0x0
        push 0x0
        push ebx
        push 0x80000001
        call esi
        test eax, eax
        jz L_4b2d74
        mov edx, dword ptr [ebp - 0x4]
        push edx
        call dword ptr [g_Iat_RegCloseKey_004cc004]
        xor eax, eax
        lea esp, [ebp - 0x18]
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_4b2d74:
        lea eax, [ebp - 0xc]
        lea ecx, [ebp - 0x8]
        push eax
        push ecx
        push 0x0
        push 0x20006
        push 0x0
        push 0x0
        push 0x0
        push ebx
        push 0x80000002
        call esi
        test eax, eax
        jz L_4b2db3
        mov edx, dword ptr [ebp - 0x4]
        mov esi, dword ptr [g_Iat_RegCloseKey_004cc004]
        push edx
        call esi
        mov eax, dword ptr [ebp - 0x8]
        push eax
        call esi
        xor eax, eax
        lea esp, [ebp - 0x18]
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_4b2db3:
        mov esi, dword ptr [g_SettingsNodeState_0056bcd0]
        test esi, esi
        jz L_4b2e29
        mov edi, dword ptr [g_Iat_RegSetValueExA_004cc014]
    L_4b2dc3:
        mov ecx, dword ptr [esi + 0x14]
        cmp ecx, 0x1
        jnz L_4b2dd0
        lea ecx, [ebp - 0x4]
        jmp L_4b2dd7
    L_4b2dd0:
        test ecx, ecx
        jnz L_4b2e22
        lea ecx, [ebp - 0x8]
    L_4b2dd7:
        mov edx, dword ptr [esi + 0x8]
        cmp edx, 0x7
        ja L_4b2e1e
        cmp edx, 0
        je L_4b2de6
        cmp edx, 1
        je L_4b2df8
        cmp edx, 2
        je L_4b2df8
        cmp edx, 3
        je L_4b2e0a
        cmp edx, 4
        je L_4b2e0a
        cmp edx, 5
        je L_4b2e0a
        cmp edx, 6
        je L_4b2e0a
        cmp edx, 7
        je L_4b2e0a
        int 3  // unreachable: the bounds check above excludes other indices
    L_4b2de6:
        mov edx, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0x10]
        mov ecx, dword ptr [ecx]
        push edx
        push esi
        push 0x4
        push 0x0
        push eax
        push ecx
        jmp L_4b2e1c
    L_4b2df8:
        mov edx, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi + 0x10]
        mov ecx, dword ptr [ecx]
        push edx
        push esi
        push 0x3
        push 0x0
        push eax
        push ecx
        jmp L_4b2e1c
    L_4b2e0a:
        mov edx, dword ptr [esi + 0xc]
        mov eax, dword ptr [esi]
        push edx
        mov edx, dword ptr [esi + 0x10]
        push eax
        mov eax, dword ptr [ecx]
        push 0x3
        push 0x0
        push edx
        push eax
    L_4b2e1c:
        call edi
    L_4b2e1e:
        test eax, eax
        jnz L_4b2e4a
    L_4b2e22:
        mov esi, dword ptr [esi + 0x18]
        test esi, esi
        jnz L_4b2dc3
    L_4b2e29:
        mov ecx, dword ptr [ebp - 0x4]
        mov esi, dword ptr [g_Iat_RegCloseKey_004cc004]
        push ecx
        call esi
        mov edx, dword ptr [ebp - 0x8]
        push edx
        call esi
        mov eax, 0x1
        lea esp, [ebp - 0x18]
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    L_4b2e4a:
        lea esp, [ebp - 0x18]
        xor eax, eax
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// 0x00407470 Preset_EvaluateCondition - config node ECX: string "DEFAULT" -> 1; list of 4 {name, op, value}: name CPU_CLASS / CPU_MHZ / VIDEO_KB / RAM_KB / HW_ACCEL selects the measured system value, rhs = ConfigValue_ToInt(value node), result = Preset_CompareOp(op, measured, rhs); else 0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Preset_EvaluateCondition(int, int)
{
    __asm {
        push ecx
        mov edx, dword ptr [ecx]
        push ebx
        push ebp
        push esi
        push edi
        xor edi, edi
        sub edx, 0x3
        jz L_407636
        dec edx
        jnz L_407674
        mov ecx, dword ptr [ecx + 0x4]
        cmp dword ptr [ecx + 0x4], 0x4
        jnz L_407674
        mov edi, dword ptr [ecx + 0xc]
        mov eax, dword ptr [ecx + 0x14]
        add ecx, 0x18
        mov dword ptr [esp + 0x10], eax
        call ConfigValue_ToInt
        xor ebp, ebp
        mov esi, offset g_Data_004da000 + 0x688
        mov ecx, edi
    L_4074b1:
        mov DL, byte ptr [ecx]
        mov BL, DL
        cmp DL, byte ptr [esi]
        jnz L_4074d5
        test BL, BL
        jz L_4074d1
        mov DL, byte ptr [ecx + 0x1]
        mov BL, DL
        cmp DL, byte ptr [esi + 0x1]
        jnz L_4074d5
        add ecx, 0x2
        add esi, 0x2
        test BL, BL
        jnz L_4074b1
    L_4074d1:
        xor ecx, ecx
        jmp L_4074da
    L_4074d5:
        sbb ecx, ecx
        sbb ecx, -0x1
    L_4074da:
        test ecx, ecx
        jnz L_4074f8
        mov ebp, dword ptr [g_SettingsBlock_004e5d00 + 0xa8]
        mov ecx, dword ptr [esp + 0x10]
        push eax
        mov edx, ebp
        call Preset_CompareOp
        mov edi, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret
    L_4074f8:
        mov esi, offset g_Data_004da000 + 0x680
        mov ecx, edi
    L_4074ff:
        mov DL, byte ptr [ecx]
        mov BL, DL
        cmp DL, byte ptr [esi]
        jnz L_407523
        test BL, BL
        jz L_40751f
        mov DL, byte ptr [ecx + 0x1]
        mov BL, DL
        cmp DL, byte ptr [esi + 0x1]
        jnz L_407523
        add ecx, 0x2
        add esi, 0x2
        test BL, BL
        jnz L_4074ff
    L_40751f:
        xor ecx, ecx
        jmp L_407528
    L_407523:
        sbb ecx, ecx
        sbb ecx, -0x1
    L_407528:
        test ecx, ecx
        jnz L_407546
        mov ebp, dword ptr [g_SettingsBlock_004e5d00 + 0xac]
        mov ecx, dword ptr [esp + 0x10]
        push eax
        mov edx, ebp
        call Preset_CompareOp
        mov edi, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret
    L_407546:
        mov esi, offset g_Data_004da000 + 0x674
        mov ecx, edi
    L_40754d:
        mov DL, byte ptr [ecx]
        mov BL, DL
        cmp DL, byte ptr [esi]
        jnz L_407571
        test BL, BL
        jz L_40756d
        mov DL, byte ptr [ecx + 0x1]
        mov BL, DL
        cmp DL, byte ptr [esi + 0x1]
        jnz L_407571
        add ecx, 0x2
        add esi, 0x2
        test BL, BL
        jnz L_40754d
    L_40756d:
        xor ecx, ecx
        jmp L_407576
    L_407571:
        sbb ecx, ecx
        sbb ecx, -0x1
    L_407576:
        test ecx, ecx
        jnz L_407594
        mov ebp, dword ptr [g_SettingsBlock_004e5d00 + 0xb8]
        mov ecx, dword ptr [esp + 0x10]
        push eax
        mov edx, ebp
        call Preset_CompareOp
        mov edi, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret
    L_407594:
        mov esi, offset g_Data_004da000 + 0x66c
        mov ecx, edi
    L_40759b:
        mov DL, byte ptr [ecx]
        mov BL, DL
        cmp DL, byte ptr [esi]
        jnz L_4075bf
        test BL, BL
        jz L_4075bb
        mov DL, byte ptr [ecx + 0x1]
        mov BL, DL
        cmp DL, byte ptr [esi + 0x1]
        jnz L_4075bf
        add ecx, 0x2
        add esi, 0x2
        test BL, BL
        jnz L_40759b
    L_4075bb:
        xor ecx, ecx
        jmp L_4075c4
    L_4075bf:
        sbb ecx, ecx
        sbb ecx, -0x1
    L_4075c4:
        test ecx, ecx
        jnz L_4075e2
        mov ebp, dword ptr [g_SettingsBlock_004e5d00 + 0xb4]
        mov ecx, dword ptr [esp + 0x10]
        push eax
        mov edx, ebp
        call Preset_CompareOp
        mov edi, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret
    L_4075e2:
        mov esi, offset g_Data_004da000 + 0x660
        mov ecx, edi
    L_4075e9:
        mov DL, byte ptr [ecx]
        mov BL, DL
        cmp DL, byte ptr [esi]
        jnz L_40760d
        test BL, BL
        jz L_407609
        mov DL, byte ptr [ecx + 0x1]
        mov BL, DL
        cmp DL, byte ptr [esi + 0x1]
        jnz L_40760d
        add ecx, 0x2
        add esi, 0x2
        test BL, BL
        jnz L_4075e9
    L_407609:
        xor ecx, ecx
        jmp L_407612
    L_40760d:
        sbb ecx, ecx
        sbb ecx, -0x1
    L_407612:
        test ecx, ecx
        jnz L_407622
        mov ebp, dword ptr [g_SettingsBlock_004e5d00 + 0xb0]
        shr ebp, 0x6
        and ebp, 0x1
    L_407622:
        mov ecx, dword ptr [esp + 0x10]
        push eax
        mov edx, ebp
        call Preset_CompareOp
        mov edi, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret
    L_407636:
        mov ecx, dword ptr [ecx + 0x4]
        mov esi, offset g_Data_004da000 + 0x658
    L_40763e:
        mov AL, byte ptr [ecx]
        mov BL, byte ptr [esi]
        mov DL, AL
        cmp AL, BL
        jnz L_407666
        test DL, DL
        jz L_407662
        mov AL, byte ptr [ecx + 0x1]
        mov BL, byte ptr [esi + 0x1]
        mov DL, AL
        cmp AL, BL
        jnz L_407666
        add ecx, 0x2
        add esi, 0x2
        test DL, DL
        jnz L_40763e
    L_407662:
        xor ecx, ecx
        jmp L_40766b
    L_407666:
        sbb ecx, ecx
        sbb ecx, -0x1
    L_40766b:
        test ecx, ecx
        jnz L_407674
        mov edi, 0x1
    L_407674:
        mov eax, edi
        pop edi
        pop esi
        pop ebp
        pop ebx
        pop ecx
        ret
    }
}

// 0x00407680 Settings_GetHardwarePresetOrDefault - (default) ECX=preset node ret 4: child by name (ConfigTree_FindChild 0x0048cf70); for each entry 1..n-1: if Preset_EvaluateCondition -> return ConfigValue_ToInt(value); else return default
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Settings_GetHardwarePresetOrDefault(int, int, int)
{
    __asm {
        push ebx
        push esi
        test ecx, ecx
        push edi
        jz L_4076d8
        call ConfigTree_FindChild
        mov edi, eax
        test edi, edi
        jz L_4076d8
        mov eax, dword ptr [edi + 0x4]
        mov esi, 0x1
        mov ebx, dword ptr [eax + 0x4]
        cmp ebx, esi
        jle L_4076d8
    L_4076a1:
        mov ecx, dword ptr [edi + 0x4]
        mov ecx, dword ptr [ecx + esi*0x8 + 0x4]
        add ecx, 0x8
        call Preset_EvaluateCondition
        test eax, eax
        jnz L_4076c3
        inc esi
        cmp esi, ebx
        jl L_4076a1
        mov eax, dword ptr [esp + 0x10]
        pop edi
        pop esi
        pop ebx
        ret 0x4
    L_4076c3:
        mov edx, dword ptr [edi + 0x4]
        mov ecx, dword ptr [edx + esi*0x8 + 0x4]
        add ecx, 0x10
        call ConfigValue_ToInt
        pop edi
        pop esi
        pop ebx
        ret 0x4
    L_4076d8:
        mov eax, dword ptr [esp + 0x10]
        pop edi
        pop esi
        pop ebx
        ret 0x4
    }
}

// 0x004080a0 RecoilApp_ApplySoundAPISetting - *[0x004e5d34] = ECX; tail jmp 0x004a1290
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_ApplySoundAPISetting(int, int)
{
    __asm {
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x34]
        mov dword ptr [eax], ecx
        jmp Sound_SetAPIModePreInit
    }
}

// 0x004086e0 Settings_ScreenRect_SetSize - bytes re-read 2026-09-24 (8bc2 50 a1885d4e00 8bd1 8b08 e8->0x00408400 c3): Rect_SetSize (0x00408400) on [*[0x004e5d88]] with (ECX=w, EDX=h). Renamed: was Settings_ScreenRect_SetOrigin (names of 0x004086e0 and 0x00408700 were swapped in the ledger)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_ScreenRect_SetSize(int, int)
{
    __asm {
        mov eax, edx
        push eax
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x88]
        mov edx, ecx
        mov ecx, dword ptr [eax]
        call Rect_SetSize
        ret
    }
}

// 0x00408700 Settings_ScreenRect_SetOrigin - bytes re-read 2026-09-24 (same shape, e8->0x004083d0 Rect_SetOrigin): Rect_SetOrigin on [*[0x004e5d88]] with (ECX=x, EDX=y). Renamed: was Settings_ScreenRect_SetSize (swapped)
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_ScreenRect_SetOrigin(int, int)
{
    __asm {
        mov eax, edx
        push eax
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x88]
        mov edx, ecx
        mov ecx, dword ptr [eax]
        call Rect_SetOrigin
        ret
    }
}

// 0x00408720 Settings_ApplyVideoModePreset - disassembly re-read 2026-09-24 (caveat resolved): mode ECX; jump table 0x004089a4 for modes 2..7, others -> *[0x004e5d30]=0 and return. Each case: *[0x004e5d30]=mode; Viewport0_SetOrigin(0,0); Viewport0_SetSizeF(renderW, renderH); Settings_ScreenRect_SetOrigin(0,0); Settings_ScreenRect_SetSize(W,H);
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_ApplyVideoModePreset(int, int)
{
    __asm {
        lea eax, [ecx - 0x2]
        cmp eax, 0x5
        ja L_408995
        cmp eax, 0
        je L_408733
        cmp eax, 1
        je L_40879a
        cmp eax, 2
        je L_408802
        cmp eax, 3
        je L_408867
        cmp eax, 4
        je L_4088cb
        cmp eax, 5
        je L_408930
        int 3  // unreachable: the bounds check above excludes other indices
    L_408733:
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x30]
        xor edx, edx
        xor ecx, ecx
        mov dword ptr [eax], 0x2
        call Viewport0_SetOrigin
        mov edx, 0xc8
        mov ecx, 0x140
        call Viewport0_SetSizeF
        xor edx, edx
        xor ecx, ecx
        call Settings_ScreenRect_SetOrigin
        mov edx, 0x190
        mov ecx, 0x280
        call Settings_ScreenRect_SetSize
        xor edx, edx
        xor ecx, ecx
        call Viewport1_SetOrigin
        mov edx, 0x190
        mov ecx, 0x280
        call Viewport1_SetSize
        mov ecx, 0x10
        call Settings_SetDisplayRect_20
        mov ecx, 0x1
        jmp Settings_Store_004e5d6c
    L_40879a:
        mov ecx, dword ptr [g_SettingsBlock_004e5d00 + 0x30]
        xor edx, edx
        mov dword ptr [ecx], 0x3
        xor ecx, ecx
        call Viewport0_SetOrigin
        mov edx, 0xf0
        mov ecx, 0x140
        call Viewport0_SetSizeF
        xor edx, edx
        xor ecx, ecx
        call Settings_ScreenRect_SetOrigin
        mov edx, 0x1e0
        mov ecx, 0x280
        call Settings_ScreenRect_SetSize
        xor edx, edx
        xor ecx, ecx
        call Viewport1_SetOrigin
        mov edx, 0x1e0
        mov ecx, 0x280
        call Viewport1_SetSize
        mov ecx, 0x10
        call Settings_SetDisplayRect_20
        mov ecx, 0x1
        jmp Settings_Store_004e5d6c
    L_408802:
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x30]
        xor ecx, ecx
        mov dword ptr [edx], 0x4
        xor edx, edx
        call Viewport0_SetOrigin
        mov edx, 0x190
        mov ecx, 0x280
        call Viewport0_SetSizeF
        xor edx, edx
        xor ecx, ecx
        call Settings_ScreenRect_SetOrigin
        mov edx, 0x190
        mov ecx, 0x280
        call Settings_ScreenRect_SetSize
        xor edx, edx
        xor ecx, ecx
        call Viewport1_SetOrigin
        mov edx, 0x190
        mov ecx, 0x280
        call Viewport1_SetSize
        mov ecx, 0x10
        call Settings_SetDisplayRect_20
        xor ecx, ecx
        jmp Settings_Store_004e5d6c
    L_408867:
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x30]
        xor edx, edx
        xor ecx, ecx
        mov dword ptr [eax], 0x5
        call Viewport0_SetOrigin
        mov edx, 0x1e0
        mov ecx, 0x280
        call Viewport0_SetSizeF
        xor edx, edx
        xor ecx, ecx
        call Settings_ScreenRect_SetOrigin
        mov edx, 0x1e0
        mov ecx, 0x280
        call Settings_ScreenRect_SetSize
        xor edx, edx
        xor ecx, ecx
        call Viewport1_SetOrigin
        mov edx, 0x1e0
        mov ecx, 0x280
        call Viewport1_SetSize
        mov ecx, 0x10
        call Settings_SetDisplayRect_20
        xor ecx, ecx
        jmp Settings_Store_004e5d6c
    L_4088cb:
        mov ecx, dword ptr [g_SettingsBlock_004e5d00 + 0x30]
        xor edx, edx
        mov dword ptr [ecx], 0x6
        xor ecx, ecx
        call Viewport0_SetOrigin
        mov edx, 0x258
        mov ecx, 0x320
        call Viewport0_SetSizeF
        xor edx, edx
        xor ecx, ecx
        call Settings_ScreenRect_SetOrigin
        mov edx, 0x258
        mov ecx, 0x320
        call Settings_ScreenRect_SetSize
        xor edx, edx
        xor ecx, ecx
        call Viewport1_SetOrigin
        mov edx, 0x258
        mov ecx, 0x320
        call Viewport1_SetSize
        mov ecx, 0x10
        call Settings_SetDisplayRect_20
        xor ecx, ecx
        jmp Settings_Store_004e5d6c
    L_408930:
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x30]
        xor ecx, ecx
        mov dword ptr [edx], 0x7
        xor edx, edx
        call Viewport0_SetOrigin
        mov edx, 0x300
        mov ecx, 0x400
        call Viewport0_SetSizeF
        xor edx, edx
        xor ecx, ecx
        call Settings_ScreenRect_SetOrigin
        mov edx, 0x300
        mov ecx, 0x400
        call Settings_ScreenRect_SetSize
        xor edx, edx
        xor ecx, ecx
        call Viewport1_SetOrigin
        mov edx, 0x300
        mov ecx, 0x400
        call Viewport1_SetSize
        mov ecx, 0x10
        call Settings_SetDisplayRect_20
        xor ecx, ecx
        jmp Settings_Store_004e5d6c
    L_408995:
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x30]
        mov dword ptr [eax], 0x0
        ret
    }
}

// 0x00407e00 Settings_ResetNetworkState - call 0x00429f80; Settings_SetNetworkFlag(0) (0x00408230); Settings_StoreNetworkModem(0) (0x00408240); tail jmp 0x004b2bf0
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall Settings_ResetNetworkState(int, int)
{
    __asm {
        call CommandTable_Clear
        xor ecx, ecx
        call Settings_SetNetworkFlag
        xor ecx, ecx
        call Settings_StoreNetworkModem
        jmp Settings_SaveToRegistry
    }
}

// 0x00407700 RecoilApp_LoadUserSettings - Settings_OpenDetailPresetFile(0) (detail.zrd) fail -> 0; 0x004b3090; registers settings nodes (Settings_RegisterNode(0,1)) into globals and applies defaults via Settings_GetHardwarePresetOrDefault: HWCardFlag->0x004e5d58, EffectsLevel_SW/HW->0x004e5d00/04, GfxFlags_SW/HW (Transparency, Lighting, Per
// Register/stack shape from the listing (ECX, EDX, 0 stack bytes).
__declspec(naked) int __fastcall RecoilApp_LoadUserSettings(int, int)
{
    __asm {
        sub esp, 0x104
        mov ecx, 0x26
        xor eax, eax
        xor edx, edx
        push ebx
        push edi
        mov edi, offset g_SettingsBlock_004e5d00
        xor ebx, ebx
        rep stosd
        push ebx
        mov ecx, offset g_Data_004da000 + 0x8c0
        call ConfigTree_ParseFileByBasename
        mov edi, eax
        cmp edi, ebx
        jnz L_407736
        xor eax, eax
        pop edi
        pop ebx
        add esp, 0x104
        ret
    L_407736:
        push esi
        push ebp
        mov ecx, offset g_SettingsBlock_004e5d00 + 0x98
        call SysInfo_CopyInfoBlock
        mov ebp, 0x1
        xor edx, edx
        push ebp
        push ebx
        mov ecx, offset g_Data_004da000 + 0x8b4
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x58], eax
        jz L_407765
        mov ecx, ebp
        call Settings_ApplyHWCardFlag
    L_407765:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x8a4
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00], eax
        jz L_407796
        push ebp
        mov edx, offset g_Data_004da000 + 0x8a4
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebx
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_ApplyEffectsLevel
    L_407796:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x894
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x4], eax
        jz L_4077c7
        push ebx
        mov edx, offset g_Data_004da000 + 0x894
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebp
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_ApplyEffectsLevel
    L_4077c7:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x888
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x8], eax
        jz L_407865
        mov AL, byte ptr [g_SettingsBlock_004e5d00 + 0xb0]
        xor esi, esi
        test AL, 0x1
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebx
        jz L_4077f8
        mov esi, 0x4
    L_4077f8:
        push ebp
        mov edx, offset g_Data_004da000 + 0x878
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_40780c
        or esi, 0x2
    L_40780c:
        push ebp
        mov edx, offset g_Data_004da000 + 0x86c
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_40781f
        or esi, ebp
    L_40781f:
        push ebp
        mov edx, offset g_Data_004da000 + 0x860
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_407833
        or esi, 0x8
    L_407833:
        push ebx
        mov edx, offset g_Data_004da000 + 0x850
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_407847
        or esi, 0x10
    L_407847:
        push ebx
        mov edx, offset g_Data_004da000 + 0x840
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_40785e
        or esi, 0x10000
    L_40785e:
        mov ecx, esi
        call Settings_ApplyGfxFlags
    L_407865:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x834
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xc], eax
        jz L_407903
        mov AL, byte ptr [g_SettingsBlock_004e5d00 + 0xb0]
        xor esi, esi
        test AL, 0x1
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebp
        jz L_407896
        mov esi, 0x4
    L_407896:
        push ebp
        mov edx, offset g_Data_004da000 + 0x878
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_4078aa
        or esi, 0x2
    L_4078aa:
        push ebp
        mov edx, offset g_Data_004da000 + 0x86c
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_4078bd
        or esi, ebp
    L_4078bd:
        push ebp
        mov edx, offset g_Data_004da000 + 0x860
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_4078d1
        or esi, 0x8
    L_4078d1:
        push ebp
        mov edx, offset g_Data_004da000 + 0x824
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_4078e5
        or esi, 0x10
    L_4078e5:
        push ebx
        mov edx, offset g_Data_004da000 + 0x840
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        test eax, eax
        jz L_4078fc
        or esi, 0x10000
    L_4078fc:
        mov ecx, esi
        call Settings_ApplyGfxFlags
    L_407903:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x814
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x10], eax
        jz L_407934
        push ebx
        mov edx, offset g_Data_004da000 + 0x814
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebx
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_ApplyObjectLOD
    L_407934:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x804
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x14], eax
        jz L_407965
        push ebx
        mov edx, offset g_Data_004da000 + 0x804
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebp
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_ApplyObjectLOD
    L_407965:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x7f0
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x18], eax
        jz L_407996
        push ebx
        mov edx, offset g_Data_004da000 + 0x7f0
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebx
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_StoreTextureMemory
    L_407996:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x7dc
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x1c], eax
        jz L_4079c7
        push ebx
        mov edx, offset g_Data_004da000 + 0x7dc
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebp
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_StoreTextureMemory
    L_4079c7:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x7cc
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x3c], eax
        jz L_4079e8
        mov ecx, 0x8
        call Settings_StoreGameCtlOptions
    L_4079e8:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x7bc
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x48], eax
        jz L_407a06
        mov ecx, ebp
        call Settings_StoreGameIntensity
    L_407a06:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x7b0
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x40], eax
        jz L_407a24
        xor ecx, ecx
        call Settings_ApplyMuteSound
    L_407a24:
        push ebp
        push ebx
        mov edx, ebp
        mov ecx, offset g_Data_004da000 + 0x7a4
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x44], eax
        jz L_407a45
        push 0x3f800000
        call Settings_ApplySoundVolume
    L_407a45:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x798
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x38], eax
        jz L_407a70
        push ebx
        mov edx, offset g_Data_004da000 + 0x798
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_StoreSoundLOD
    L_407a70:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x78c
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x34], eax
        jz L_407a8e
        xor ecx, ecx
        call RecoilApp_ApplySoundAPISetting
    L_407a8e:
        push ebp
        push 0x16
        mov edx, 0x3
        mov ecx, offset g_Data_004da000 + 0x780
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x4c], eax
        jz L_407ad2
        lea eax, [esp + 0x10]
        lea ecx, [esp + 0x14]
        push eax
        push ecx
        mov dword ptr [esp + 0x18], 0xfe
        call dword ptr [g_Iat_GetUserNameA_004cc000]
        lea ecx, [esp + 0x14]
        mov edx, dword ptr [esp + 0x10]
        mov byte ptr [esp + edx*0x1 + 0x14], BL
        call Settings_StorePlayerName
    L_407ad2:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x778
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x50], eax
        jz L_407af0
        mov ecx, ebp
        call Settings_StoreCDAudio
    L_407af0:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x76c
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x54], eax
        jz L_407b0e
        mov ecx, ebp
        call Settings_StoreFullScreen
    L_407b0e:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x760
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x20], eax
        jz L_407b3f
        push ebp
        mov edx, offset g_Data_004da000 + 0x760
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebx
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_StoreHUDFlag
    L_407b3f:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x754
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x24], eax
        jz L_407b70
        push ebp
        mov edx, offset g_Data_004da000 + 0x754
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebp
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_StoreHUDFlag
    L_407b70:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x748
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x28], eax
        jz L_407ba1
        push ebp
        mov edx, offset g_Data_004da000 + 0x748
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebx
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_ApplyHUDType
    L_407ba1:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x73c
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x2c], eax
        jz L_407bd2
        push ebp
        mov edx, offset g_Data_004da000 + 0x73c
        mov ecx, edi
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], ebp
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_ApplyHUDType
    L_407bd2:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x734
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x5c], eax
        jz L_407bf0
        mov ecx, ebp
        call Settings_StoreHWAPI
    L_407bf0:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x728
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x60], eax
        jz L_407c0e
        xor ecx, ecx
        call Settings_StoreJoystickEnabled
    L_407c0e:
        push ebp
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x718
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x94], eax
        jz L_407c2c
        mov ecx, ebp
        call Settings_StoreWOLPasswordFlag
    L_407c2c:
        push 0x2
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x708
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x64], eax
        jz L_407c4b
        xor ecx, ecx
        call Settings_StoreJoystickNumAxes
    L_407c4b:
        push 0x2
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x6f4
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x68], eax
        jz L_407c6a
        xor ecx, ecx
        call Settings_StoreJoystickNumButtons
    L_407c6a:
        push 0x2
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x6ec
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x74], eax
        jz L_407c89
        xor ecx, ecx
        call Settings_SetNetworkFlag
    L_407c89:
        push 0x2
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x6dc
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x90], eax
        jz L_407ca8
        xor ecx, ecx
        call Settings_StoreNetworkModem
    L_407ca8:
        push 0x2
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x6d0
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x78], eax
        jz L_407cc7
        xor ecx, ecx
        call Settings_StoreNetListen
    L_407cc7:
        push 0x2
        push 0xc
        mov edx, 0x7
        mov ecx, offset g_Data_004da000 + 0x6c8
        call Settings_RegisterNode
        mov edx, 0x7
        mov ecx, offset g_Data_004da000 + 0x6c0
        push 0x2
        push 0x28
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x7c], eax
        call Settings_RegisterNode
        mov edx, 0x7
        mov ecx, offset g_Data_004da000 + 0x6b8
        push 0x2
        push 0x28
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x80], eax
        call Settings_RegisterNode
        mov edx, 0x7
        mov ecx, offset g_Data_004da000 + 0x6b0
        push 0x2
        push 0x28
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x84], eax
        call Settings_RegisterNode
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x6a4
        push 0x2
        push ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x88], eax
        call Settings_RegisterNode
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x69c
        push ebp
        push ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x6c], eax
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x30], eax
        jz L_407d67
        push 0x5
        mov edx, offset g_Data_004da000 + 0x69c
        mov ecx, edi
        call Settings_GetHardwarePresetOrDefault
        mov ecx, eax
        call Settings_ApplyVideoModePreset
    L_407d67:
        push 0x2
        push ebx
        xor edx, edx
        mov ecx, offset g_Data_004da000 + 0x694
        call Settings_RegisterNode
        cmp eax, ebx
        mov dword ptr [g_SettingsBlock_004e5d00 + 0x70], eax
        jz L_407d81
        mov dword ptr [eax], ebp
    L_407d81:
        call ControlsScreen_BuildBindingList
        call Settings_LoadFromRegistry
        call Thunk_00470820
        xor ecx, ecx
        call Settings_SetNetworkFlag
        xor ecx, ecx
        call Settings_StoreNetworkModem
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x7c]
        mov ecx, dword ptr [eax]
        mov dword ptr [ecx + 0x8], ebx
        mov edx, dword ptr [g_SettingsBlock_004e5d00 + 0x80]
        mov eax, dword ptr [edx]
        mov dword ptr [eax + 0x24], ebx
        mov ecx, dword ptr [g_SettingsBlock_004e5d00 + 0x84]
        mov edx, dword ptr [ecx]
        mov dword ptr [edx + 0x24], ebx
        mov eax, dword ptr [g_SettingsBlock_004e5d00 + 0x88]
        mov ecx, dword ptr [eax]
        mov dword ptr [ecx + 0x24], ebx
        mov ecx, edi
        call ConfigTree_Destroy
        call Settings_GetHWCardFlag
        mov dword ptr [g_SettingsBlock_004e5d00 + 0xcc], eax
        call RecoilApp_GetSoundAPICheckboxValue
        mov ecx, eax
        call RecoilApp_ApplySoundAPISetting
        mov eax, ebp
        pop ebp
        pop esi
        pop edi
        pop ebx
        add esp, 0x104
        ret
    }
}

// 0x004b3260 Settings_InitPathsAndAutoLoad - ../../04_spec/systems/settings.md
// Register/stack shape from the listing (ECX, EDX, 4 stack bytes).
__declspec(naked) int __fastcall Settings_InitPathsAndAutoLoad(int, int, int)
{
    __asm {
        push esi
        mov esi, dword ptr [g_Iat__strdup_004cc5e4]
        push edi
        mov edi, edx
        push ecx
        call esi
        add esp, 0x4
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 0x8], eax
        push edi
        call esi
        add esp, 0x4
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 0xc], eax
        mov eax, dword ptr [esp + 0xc]
        push eax
        call esi
        add esp, 0x4
        mov ecx, offset g_SettingsNodeState_0056bcd0 + 0x14
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 0x10], eax
        mov dword ptr [g_SettingsNodeState_0056bcd0], 0x0
        mov dword ptr [g_SettingsNodeState_0056bcd0 + 0x4], 0x1
        call SysInfo_Collect
        pop edi
        pop esi
        ret 0x4
    }
}

}  // namespace recoil
