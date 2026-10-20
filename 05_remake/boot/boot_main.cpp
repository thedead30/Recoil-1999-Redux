// Boot probe (Stage 2 integration check, not a mirrored original file): runs the remake from the ported CRT entry point
// `entry` (0x004c6140, src/unattributed/app.cpp) exactly as Windows would start Recoil.exe, from the game install directory
// (the original opens its data files relative to the working directory). The first thing it cannot do is the next Stage 2
// blocker: an unported function reached through a data pointer stops in ImageData_Unported (platform/image), an unbound
// import or a port bug faults - the vectored handler below prints the fault and the module offset (look it up in
// recoil_boot.map). Nothing here is game behaviour; every value is PLATFORM.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "unattributed/app.h"
#include "platform/nocd.h"
#include "platform/iat_user32.h"
#include "platform/advapi.h"
#include "platform/iat_ddraw.h"
#include "platform/iat_dinput.h"
#include "platform/iat_msvcrt.h"
#include "platform/iat_kernel32.h"
#include "platform/image/original_data.h"
#include "Battlesport/RecoilApp.h"
#include "unattributed/mission.h"
#include "unattributed/vehicle.h"
#include "GameZRecoil/zClass/Object3d.h"
#include <cmath>
#include <direct.h>
#include "unattributed/savegame.h"
#include "unattributed/menus.h"
#include "redux/redux.h"
int InstallVirtualCd(const char* game_dir);   // boot/virtual_cd.cpp

namespace recoil { int __fastcall SoundList_Destroy(int, int); }   // 0x004a05f0 (zSound)
namespace {
// Shared input (KG-60, side-by-side testing, launcher only): RECOIL_SHARED_INPUT=1 rewrites the game's DirectInput cooperative
// flags to DISCL_NONEXCLUSIVE | DISCL_BACKGROUND (via the IDirectInputDeviceA::SetCooperativeLevel vtable slot), so the remake
// and the original (tools/sidebyside/dinput.dll) can read the same keyboard and mouse at once. Off by default.
using DiCreateFn = HRESULT(WINAPI*)(HINSTANCE, DWORD, void**, void*);
using DiCreateDevFn = HRESULT(WINAPI*)(void*, REFGUID, void**, void*);
using DiCoopFn = HRESULT(WINAPI*)(void*, HWND, DWORD);
DiCreateFn g_real_di_create; DiCreateDevFn g_real_create_dev; DiCoopFn g_real_coop;
void PatchVtSlot(void* obj, int slot, void* fn, void** orig)
{
    void** vt = *reinterpret_cast<void***>(obj);
    if (vt[slot] == fn) return;
    if (!*orig) *orig = vt[slot];
    DWORD old;
    VirtualProtect(&vt[slot], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    vt[slot] = fn;
    VirtualProtect(&vt[slot], sizeof(void*), old, &old);
}
HRESULT WINAPI SharedCoop(void* self, HWND w, DWORD flags)
{
    return g_real_coop(self, w, (flags & ~(0x1u | 0x4u)) | 0x2u | 0x8u);   // PLATFORM: EXCLUSIVE|FOREGROUND -> NONEXCLUSIVE|BACKGROUND
}
HRESULT WINAPI SharedCreateDevice(void* self, REFGUID g, void** out, void* outer)
{
    const HRESULT hr = g_real_create_dev(self, g, out, outer);
    if (SUCCEEDED(hr) && out && *out)   // PLATFORM: slot 13 = IDirectInputDeviceA::SetCooperativeLevel
        PatchVtSlot(*out, 13, reinterpret_cast<void*>(&SharedCoop), reinterpret_cast<void**>(&g_real_coop));
    return hr;
}
HRESULT WINAPI SharedDiCreate(HINSTANCE inst, DWORD ver, void** out, void* outer)
{
    const HRESULT hr = g_real_di_create(inst, ver, out, outer);
    if (SUCCEEDED(hr) && out && *out)   // PLATFORM: slot 3 = IDirectInputA::CreateDevice
        PatchVtSlot(*out, 3, reinterpret_cast<void*>(&SharedCreateDevice), reinterpret_cast<void**>(&g_real_create_dev));
    return hr;
}

// Second instance allowed (KG-60, side-by-side only): App_ActivateExistingInstance 0x0042e990 calls FindWindowA("RecoilClass")
// and the app quits if another Recoil window exists - the remake would see the original's window and exit. With
// RECOIL_SHARED_INPUT=1 the FindWindowA slot answers NULL for class "RecoilClass".
using FindWinFn = HWND(WINAPI*)(LPCSTR, LPCSTR);
FindWinFn g_real_findwin;
HWND WINAPI SideBySideFindWindow(LPCSTR cls, LPCSTR title)
{
    if (cls && !IS_INTRESOURCE(cls) && std::strcmp(cls, "RecoilClass") == 0) return nullptr;   // PLATFORM
    return g_real_findwin(cls, title);
}

// Keep running when not focused (KG-60, side by side only): swallow WM_ACTIVATEAPP(FALSE) / WM_ACTIVATE(WA_INACTIVE) on the game
// window and veto minimise, so the remake keeps playing while the original has focus. Installed from the per-frame tick hook.
bool g_keep_active = false;
WNDPROC g_game_proc; HWND g_game_wnd;
LRESULT CALLBACK KeepActiveProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_ACTIVATEAPP && !w) return 0;
    if (m == WM_ACTIVATE && LOWORD(w) == WA_INACTIVE) return 0;
    if (m == WM_NCACTIVATE && !w) return TRUE;
    if (m == WM_SYSCOMMAND && (w & 0xfff0) == SC_MINIMIZE) return 0;
    return CallWindowProcA(g_game_proc, h, m, w, l);
}
void KeepActive()
{
    if (!g_keep_active || g_game_wnd) return;
    EnumThreadWindows(GetCurrentThreadId(), [](HWND h, LPARAM) -> BOOL {
        char c[64]; GetClassNameA(h, c, sizeof c);
        if (std::strcmp(c, "RecoilClass") == 0 && !g_game_wnd) {
            g_game_wnd = h;
            g_game_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&KeepActiveProc)));
            SetWindowsHookExA(WH_CBT, [](int code, WPARAM w, LPARAM l) -> LRESULT {
                const int sw = static_cast<int>(LOWORD(l));
                if (code == HCBT_MINMAX && (sw == SW_MINIMIZE || sw == SW_SHOWMINIMIZED || sw == SW_SHOWMINNOACTIVE || sw == SW_FORCEMINIMIZE)) return 1;
                return CallNextHookEx(nullptr, code, w, l);
            }, nullptr, GetCurrentThreadId());
        }
        return TRUE;
    }, 0);
}

// Compare saves (KG-61, testing only): RECOIL_COMPARE=1 makes Scroll Lock save the game in place with the game's own
// SaveGame_SaveToFile 0x004c0030 to SavedGames\cmp_<time> (name appended to compare.log), so the ORIGINAL can load the same
// moment (tools/sidebyside/compare_*). RECOIL_LOAD_SAVE=<name> loads that save at start-up instead of the direct mission start,
// repeating LoadGameScreen_Load 0x00435a70's main-menu path (mode 0): SaveGame_LoadFromFile; StopGlobalVoice; SoundList_Destroy
// [0x004f3fc8]; [0x004f3ec4] = _strdup(path); [0x004f3ea8] = 1; ScreenManager_QueuePop(app, 1); QueuePush(app, 0x004f3e80, 0).
// (The control-settings re-apply calls of that path are skipped.)
bool g_compare = false; const char* g_load_save = nullptr; bool g_in_save = false;
void CompareSaveKey()
{
    static bool down = false;
    const bool now = (GetAsyncKeyState(VK_SCROLL) & 0x8000) != 0;
    if (!g_compare || g_in_save || now == down) { down = now; return; }
    down = now;
    if (!now) return;
    g_in_save = true;
    SYSTEMTIME t; GetLocalTime(&t);
    char name[64], path[128];
    std::snprintf(name, sizeof name, "cmp_%02d%02d%02d", t.wHour, t.wMinute, t.wSecond);
    std::snprintf(path, sizeof path, "SavedGames\\%s", name);
    _mkdir("SavedGames");
    const int r = recoil::SaveGame_SaveToFile(reinterpret_cast<int>(path), 0);
    if (FILE* f = std::fopen("compare.log", "a")) { std::fprintf(f, "%s %d\n", name, r); std::fclose(f); }
    MessageBeep(r ? MB_OK : MB_ICONHAND);   // PLATFORM: audible confirmation that the compare save was written (or failed)
    g_in_save = false;
}
// KG-61 frame-0 dump: with RECOIL_LOAD_SAVE + RECOIL_DUMP=<file>, a full memory dump the moment gameplay starts after the
// save is applied - App_EnterGameplay 0x0042eed0 sets [0x004f3df0]=1 right before Mission_BeginPlay (same signal as the
// original's wrapper tools/sidebyside/rcl_dinput.cpp, so both dumps are taken at the identical point).
DWORD g_load_at;
int LoadSaveNow(const char* name);
void LoadPendingSave()   // game thread (GetTickCount slot)
{
    if (!g_load_at || static_cast<int>(GetTickCount() - g_load_at) < 0) return;
    g_load_at = 0;
    const int ok = LoadSaveNow(g_load_save);
    if (FILE* f = std::fopen("compare.log", "a")) { std::fprintf(f, "load %s -> %d\n", g_load_save, ok); std::fclose(f); }
}
void FrameZeroDump()
{
    static int last = -1;
    if (!g_load_save) return;
    const int v = *static_cast<volatile int*>(recoil::ImageData_Address(0x004f3df0));
    if (last == 0 && v == 1) {
        if (const char* d = std::getenv("RECOIL_DUMP")) {
            HANDLE f = CreateFileA(d, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            using DumpFn = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, int, void*, void*, void*);
            auto dump = reinterpret_cast<DumpFn>(GetProcAddress(LoadLibraryA("dbghelp.dll"), "MiniDumpWriteDump"));
            const BOOL ok = f != INVALID_HANDLE_VALUE && dump && dump(GetCurrentProcess(), GetCurrentProcessId(), f, 0x2 | 0x4 | 0x800 | 0x1000, nullptr, nullptr, nullptr);   // PLATFORM: FullMemory|HandleData|FullMemoryInfo|ThreadInfo
            if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
            if (FILE* lf = std::fopen("compare.log", "a")) { std::fprintf(lf, "frame-0 dump %s -> %d\n", d, ok); std::fclose(lf); }
        }
    }
    last = v;
}
int LoadSaveNow(const char* name)
{
    static char path[160];
    std::snprintf(path, sizeof path, "SavedGames\\%s", name);
    if (!recoil::SaveGame_LoadFromFile(reinterpret_cast<int>(path), 0)) return 0;
    recoil::StopGlobalVoice_004edc6c(0, 0);
    int* sounds = reinterpret_cast<int*>(recoil::ImageData_Address(0x004f3fc8));
    if (*sounds) { recoil::SoundList_Destroy(*sounds, 0); *sounds = 0; }
    using StrdupFn = char*(__cdecl*)(const char*);
    *reinterpret_cast<char**>(recoil::ImageData_Address(0x004f3ec4)) = reinterpret_cast<StrdupFn>(recoil::g_Iat__strdup_004cc5e4)(path);
    *reinterpret_cast<int*>(recoil::ImageData_Address(0x004f3ea8)) = 1;
    const int app = static_cast<int>(reinterpret_cast<std::uintptr_t>(recoil::ImageData_Address(0x004f3ca8)));
    // no ScreenManager_QueuePop: the menu pops its own Load Game screen; from the main menu a pop would empty the stack (quit)
    recoil::ScreenManager_QueuePush(app, 0, static_cast<int>(reinterpret_cast<std::uintptr_t>(recoil::ImageData_Address(0x004f3e80))), 0);
    return 1;
}

// Process-relative GetTickCount (KG-54, original defect workaround in the platform layer): the game keeps its clock as a FLOAT
// in seconds, (float)GetTickCount() * 0.001 (Timer_Reset 0x004a5670 -> [0x004e2fb0]). After ~6 days of Windows uptime a float
// near 5e5 s can only step in 62.5 ms, so frame time arrives in coarse uneven chunks (jitter, wrong speed) - in the original too.
// The game code is unchanged; the GetTickCount slot answers milliseconds since start-up instead. RECOIL_TICK_FROM_BOOT=0 disables.
DWORD g_tick_base;
void NearTurretTick();
DWORD WINAPI ProcessTickCount()   // PLATFORM; also runs the KG-58 teleport check on the game thread (called every frame)
{
    NearTurretTick();
    KeepActive();
    CompareSaveKey();
    LoadPendingSave();
    FrameZeroDump();
    return GetTickCount() - g_tick_base;
}

// Direct-start shortcut (KG-46, launcher only, not original behaviour): skip the intro / menus and start mission N the way the
// menu's mission slots do (Slot_StartMission_1_00431270: App_StartMission(N, 0, 1, [slot+0x1d8]) on the main app object
// 0x004f3ca8). It is issued once from inside the game's own message pump, on the game thread, after the main loop has run a
// few times (the wrapped PeekMessageA slot), so everything InitInstance set up is in place. RECOIL_START_MISSION=0 disables it.
using PeekFn = BOOL(WINAPI*)(LPMSG, HWND, UINT, UINT, UINT);
PeekFn g_real_peek;
int g_start_mission = 1, g_peeks = 0;
float g_near_turret = 0.0f;   // RECOIL_NEAR_TURRET (KG-58)
constexpr int kPeeksBeforeStart = 20;          // PLATFORM: pump iterations to let start-up settle, not a game value
constexpr std::uint32_t kMainAppVa = 0x004f3ca8;  // CONFIRMED-BINARY: MOV ECX,0x4f3ca8 in Slot_StartMission_1_00431270

// Windowed start (KG-47, launcher only): the game reads FullScreen from HKCU\Software\Zipper\RECOIL\1.0 through the RegQueryValueExA
// slot (0x004cc010); the launcher answers 0 for that one value so the remake opens in a window (full-screen 640x480 exclusive mode
// fails on this machine: "Error opening video"). The user's registry is not changed. RECOIL_WINDOWED=0 disables it.
using RegQueryFn = LONG(WINAPI*)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
RegQueryFn g_real_regquery;
LONG WINAPI WindowedRegQuery(HKEY key, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size)
{
    const LONG r = g_real_regquery(key, name, reserved, type, data, size);
    if (r == ERROR_SUCCESS && name && _stricmp(name, "FullScreen") == 0 && data && size && *size >= 4) {
        const DWORD zero = 0;
        std::memcpy(data, &zero, 4);
    }
    return r;
}

// File-open trace (diagnostic, RECOIL_TRACE_FILES=1): prints every fopen / CreateFileA the game makes and whether it succeeded.
using FopenFn = void*(__cdecl*)(const char*, const char*);
using CreateFileFn = HANDLE(WINAPI*)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
FopenFn g_real_fopen;
CreateFileFn g_real_createfile;
void* __cdecl TraceFopen(const char* name, const char* mode)
{
    void* f = g_real_fopen(name, mode);
    std::fprintf(stderr, "file: fopen(%s, %s) -> %s\n", name ? name : "(null)", mode ? mode : "", f ? "ok" : "FAILED");
    return f;
}
HANDLE WINAPI TraceCreateFile(LPCSTR name, DWORD acc, DWORD share, LPSECURITY_ATTRIBUTES sa, DWORD disp, DWORD flags, HANDLE tmpl)
{
    HANDLE h = g_real_createfile(name, acc, share, sa, disp, flags, tmpl);
    std::fprintf(stderr, "file: CreateFileA(%s) -> %s\n", name ? name : "(null)", h != INVALID_HANDLE_VALUE ? "ok" : "FAILED");
    return h;
}

void NearTurretTick()
{
    // Teleport to the nearest turret (KG-58, launcher test shortcut): RECOIL_NEAR_TURRET=<distance> (game units, e.g. 40),
    // once, a few pump iterations after the direct start. Turret table 0x004f3fe8[count 0x004f3fd8], turret+8 = node
    // (CONFIRMED-BINARY: MOV [ESI+0x8],EBX at 0x004367ac); player vehicle v = [[0x004f36a4]+4], its node v+0xed0
    // (as Vehicle_TeleportToNode 0x0042be70); move with Vehicle_Teleport(ECX=v, EDX=pos, yaw).
    static DWORD s_started = 0;
    if (g_start_mission == 0 && !s_started) s_started = GetTickCount();
    static const DWORD s_delay = std::getenv("RECOIL_NEAR_TURRET_DELAY") ? static_cast<DWORD>(std::atoi(std::getenv("RECOIL_NEAR_TURRET_DELAY"))) : 8000;
    if (g_near_turret > 0.0f && s_started && GetTickCount() - s_started > s_delay) {   // PLATFORM: delay after the direct start (dismiss the briefing first)
        const float dist = g_near_turret; g_near_turret = 0.0f;   // once
        auto rd = [](std::uint32_t va) { return *reinterpret_cast<int*>(recoil::ImageData_Address(va)); };
        const int v = *reinterpret_cast<int*>(rd(0x004f36a4) + 4);
        float px, py, pz;
        recoil::Object3D_GetPosition(*reinterpret_cast<int*>(v + 0xed0), reinterpret_cast<int>(&px),
                                     reinterpret_cast<int>(&py), reinterpret_cast<int>(&pz));
        const int n = rd(0x004f3fd8);
        int best = -1; float bd = 1e30f, bx = 0, by = 0, bz = 0;
        for (int i = 0; i < n; ++i) {
            const int t = *reinterpret_cast<int*>(reinterpret_cast<char*>(recoil::ImageData_Address(0x004f3fe8)) + 4 * i);
            if (!t || !*reinterpret_cast<int*>(t + 8)) continue;
            float x, y, z;
            if (recoil::Object3D_GetPosition(*reinterpret_cast<int*>(t + 8), reinterpret_cast<int>(&x),
                                             reinterpret_cast<int>(&y), reinterpret_cast<int>(&z)) != 0) continue;
            const float d = (x - px) * (x - px) + (z - pz) * (z - pz);
            if (const char* want = std::getenv("RECOIL_TURRET_NAME"))   // pick a turret by node name (inline at node+0)
                if (std::strcmp(reinterpret_cast<const char*>(*reinterpret_cast<int*>(t + 8)), want) != 0) continue;
            if (d < bd) { bd = d; best = i; bx = x; by = y; bz = z; }
        }
        if (FILE* lf = std::fopen("teleport.log", "a")) { std::fprintf(lf, "trigger: %d turrets, best %d, player (%.1f %.1f %.1f)\n", n, best, px, py, pz); std::fclose(lf); }
        if (best >= 0) {
            const float dx = px - bx, dz = pz - bz, len = std::sqrt(dx * dx + dz * dz) + 1e-6f;
            float pos[3] = {bx + dx / len * dist, py, bz + dz / len * dist};
            const float yaw = std::atan2(-dx, -dz);   // face the turret (sign convention INFERRED; check on screen)
            int yawbits; std::memcpy(&yawbits, &yaw, 4);
            recoil::Vehicle_Teleport(v, reinterpret_cast<int>(pos), yawbits);
            if (FILE* lf = std::fopen("teleport.log", "a")) {   // own file: the game redirects stderr and a killed run loses it
                std::fprintf(lf, "teleported to turret %d of %d (at %.1f %.1f %.1f) from (%.1f %.1f %.1f) to (%.1f %.1f %.1f)\n",
                             best, n, bx, by, bz, px, py, pz, pos[0], pos[1], pos[2]);
                std::fclose(lf);
            }
        }
    }
}

BOOL WINAPI StartMissionPeek(LPMSG msg, HWND wnd, UINT lo, UINT hi, UINT rm)
{
    if (++g_peeks == kPeeksBeforeStart && g_start_mission > 0) {
        const int n = g_start_mission;
        g_start_mission = 0;
        const int app = static_cast<int>(reinterpret_cast<std::uintptr_t>(recoil::ImageData_Address(kMainAppVa)));
        std::fprintf(stderr, "boot: direct start - App_StartMission(%d, 0, 1, 1) on the app object\n", n);
        std::fflush(stderr);
        // what the skipped mission-intro screen does first (AppScreen_PlayMissionFmv_0042edb0 -> Mission_SetDataSearchPaths(id)):
        // opens zbd\m<id>\zrdr.zbd and sets the mission's reader / texture search paths
        if (g_load_save) {   // KG-61: start from a compare save instead - same steps as the original's wrapper (rcl_dinput.cpp):
            // press the launcher's Game > Start (first item); LoadPendingSave() then loads once the main menu is up
            EnumThreadWindows(GetCurrentThreadId(), [](HWND h, LPARAM) -> BOOL {
                HMENU m = GetMenu(h); HMENU sub = m ? GetSubMenu(m, 0) : nullptr;
                if (!sub) return TRUE;
                PostMessageA(h, WM_COMMAND, GetMenuItemID(sub, 0), 0); return FALSE;
            }, 0);
            g_load_at = GetTickCount() + 9000;
            return g_real_peek(msg, wnd, lo, hi, rm);
        }
        recoil::Mission_SetDataSearchPaths(n, 0);
        // last argument 1 = load the mission from the shipped .zbd archives (Mission_Load runs m<n>_zbd.gs; 0 runs m<n>.gs, the loose developer files)
        const int r = recoil::App_StartMission(app, 0, n, 0, 1, 1);
        std::fprintf(stderr, "boot: App_StartMission returned %d\n", r);
        std::fflush(stderr);
    }
    return g_real_peek(msg, wnd, lo, hi, rm);
}

LONG CALLBACK report(EXCEPTION_POINTERS* e)
{
    const DWORD code = e->ExceptionRecord->ExceptionCode;
    if (code == 0xE06D7363u || code == DBG_PRINTEXCEPTION_C || code == 0x406D1388u) return EXCEPTION_CONTINUE_SEARCH;  // C++ throw, OutputDebugString, thread name: PLATFORM
    static int n = 0;
    if (++n > 3) return EXCEPTION_CONTINUE_SEARCH;
    const auto at = reinterpret_cast<std::uintptr_t>(e->ExceptionRecord->ExceptionAddress);
    HMODULE mod = nullptr;
    char name[MAX_PATH] = "?";
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(at), &mod))
        GetModuleFileNameA(mod, name, MAX_PATH);
    std::fprintf(stderr, "boot: exception 0x%08lx at 0x%08x (%s + 0x%x), eax %08lx ecx %08lx edx %08lx esi %08lx edi %08lx esp %08lx ebp %08lx\n",
                 code, static_cast<unsigned>(at), name, static_cast<unsigned>(at - reinterpret_cast<std::uintptr_t>(mod)),
                 e->ContextRecord->Eax, e->ContextRecord->Ecx, e->ContextRecord->Edx, e->ContextRecord->Esi, e->ContextRecord->Edi,
                 e->ContextRecord->Esp, e->ContextRecord->Ebp);
    if (code == EXCEPTION_ACCESS_VIOLATION)
        std::fprintf(stderr, "boot:   %s 0x%08lx\n", e->ExceptionRecord->ExceptionInformation[0] ? "write" : "read",
                     static_cast<unsigned long>(e->ExceptionRecord->ExceptionInformation[1]));
    std::fflush(stderr);
    return EXCEPTION_CONTINUE_SEARCH;
}
}  // namespace

// Portable mode (snapshot folders, 2026-10-07): when the exe sits in a folder holding the game data (zbd\) and no folder
// argument is given, that folder is the game folder, a DDraw.dll next to the exe is used, and the start-up is the normal
// one (launcher + menus) like the original. Development runs keep the defaults below.
char g_exe_dir[MAX_PATH];
bool g_portable = false;
void DetectPortable(int argc)
{
    GetModuleFileNameA(nullptr, g_exe_dir, MAX_PATH);
    if (char* s = std::strrchr(g_exe_dir, '\\')) *s = 0;
    char probe[MAX_PATH];
    std::snprintf(probe, sizeof probe, "%s\\zbd", g_exe_dir);
    const DWORD a = GetFileAttributesA(probe);
    g_portable = argc <= 1 && a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

int main(int argc, char** argv)
{
    DetectPortable(argc);
    const char* dir = argc > 1 ? argv[1] : (g_portable ? g_exe_dir : RECOIL_GAME_DIR);
    // never run inside the original install (00_original): the ported code writes files in its game folder (it once overwrote
    // zbd\m1\gamez.zbd there). Use a copy - 05_remake/build/game_sandbox by default.
    {
        char full[MAX_PATH];
        GetFullPathNameA(dir, MAX_PATH, full, nullptr);
        if (std::strstr(full, "00_original") || std::strstr(full, "Recoil Runtime Copy")) {
            std::fprintf(stderr, "boot: refusing to run in %s - use a copy of the game folder (05_remake/build/game_sandbox)\n", full);
            return 3;
        }
    }
    if (!SetCurrentDirectoryA(dir)) {
        std::fprintf(stderr, "boot: cannot enter the game directory %s\n", dir);
        return 2;
    }
    if (g_portable) {   // launched by double-click like the original: no console window; launcher messages go to boot.log
        FreeConsole();
        FILE* f = nullptr;
        freopen_s(&f, "boot.log", "w", stderr);
    }
    AddVectoredExceptionHandler(1, report);
    redux::Init();   // Recoil Redux switches (redux.ini in the game folder); none set = Recoil Classic
    // no-CD mode (KG-45, deviation, on unless RECOIL_NOCD=0): the game directory stands in for the Recoil CD
    const char* nocd = std::getenv("RECOIL_NOCD");
    if (!nocd || std::strcmp(nocd, "0") != 0) {
        char full[MAX_PATH];
        GetFullPathNameA(".", MAX_PATH, full, nullptr);
        recoil::Platform_EnableNoCd(full);
        std::fprintf(stderr, "boot: no-CD mode - %s stands in for the CD (RECOIL_NOCD=0 to disable)\n", full);
        // the CD's music (KG-24): music\trackNN.wav from the player's disc image play through a virtual CD-audio drive
        if (const int n = InstallVirtualCd(full))
            std::fprintf(stderr, "boot: CD music - %d audio track(s) from %s\\music (virtual CD drive)\n", n, full);
        else
            std::fprintf(stderr, "boot: no music\\trackNN.wav in the game folder - CD music needs the real disc\n");
    }
    // DirectDraw scaffolding (KG-44, temporary, launcher only): RECOIL_DDRAW_DIR names a folder holding a DDraw.dll (+ D3DImm.dll) to
    // bind the two DDRAW slots to - on this machine the user's dgVoodoo copy, the way the original runs here. The release uses the D8
    // layer (own DirectDraw / Direct3D 5 objects on D3D11), not this. Default: "../../../../Recoil/Recoil Runtime Copy" if present.
    {
        const char* dd = std::getenv("RECOIL_DDRAW_DIR");
        const char* def = RECOIL_DEFAULT_DDRAW_DIR;   // build setting (CMake RECOIL_DEFAULT_DDRAW_DIR), empty by default
        char local[MAX_PATH];
        std::snprintf(local, sizeof local, "%s\\DDraw.dll", g_exe_dir);
        const bool local_dd = g_portable && GetFileAttributesA(local) != INVALID_FILE_ATTRIBUTES;
        const char* use = dd ? dd : local_dd ? g_exe_dir : (GetFileAttributesA(def) != INVALID_FILE_ATTRIBUTES ? def : nullptr);
        if (use && *use) {
            // DDraw.dll loads its D3DImm.dll by name: load that first by full path, so the name resolves to it. Then put
            // the DLL search order back: SetDllDirectory also drops the game folder (the current directory) from the search,
            // and the game loads MESSAGES.DLL by name from there (found 2026-10-08: "MESSAGES.DLL could not be found"
            // with a dgVoodoo folder that is not a game copy).
            SetDllDirectoryA(use);
            char p[MAX_PATH];
            std::snprintf(p, sizeof p, "%s\\D3DImm.dll", use);
            LoadLibraryA(p);
            std::snprintf(p, sizeof p, "%s\\DDraw.dll", use);
            HMODULE m = LoadLibraryA(p);
            SetDllDirectoryA(nullptr);
            if (m) {
                recoil::g_Iat_DirectDrawCreate_004cc040 = reinterpret_cast<void*>(GetProcAddress(m, "DirectDrawCreate"));
                recoil::g_Iat_DirectDrawEnumerateA_004cc044 = reinterpret_cast<void*>(GetProcAddress(m, "DirectDrawEnumerateA"));
                std::fprintf(stderr, "boot: DirectDraw from %s (scaffolding, KG-44)\n", p);
            } else {
                std::fprintf(stderr, "boot: cannot load %s - system DirectDraw\n", p);
            }
        }
    }
    if (const char* tf = std::getenv("RECOIL_TRACE_FILES"); tf && std::strcmp(tf, "0") != 0) {
        g_real_fopen = reinterpret_cast<FopenFn>(recoil::g_Iat_fopen_004cc5b8);
        recoil::g_Iat_fopen_004cc5b8 = reinterpret_cast<void*>(&TraceFopen);
        g_real_createfile = reinterpret_cast<CreateFileFn>(recoil::g_Iat_CreateFileA_004cc130);
        recoil::g_Iat_CreateFileA_004cc130 = reinterpret_cast<void*>(&TraceCreateFile);
    }
    // windowed start (KG-47): opt-in only since 2026-10-07 (user: windowed mode not working well) - RECOIL_WINDOWED=1
    // reads FullScreen as 0; otherwise the registry's FullScreen value is used, as in the original
    const char* win = std::getenv("RECOIL_WINDOWED");
    if (win && std::strcmp(win, "1") == 0) {
        g_real_regquery = reinterpret_cast<RegQueryFn>(recoil::g_Iat_RegQueryValueExA_004cc010);
        recoil::g_Iat_RegQueryValueExA_004cc010 = reinterpret_cast<void*>(&WindowedRegQuery);
        std::fprintf(stderr, "boot: windowed start (FullScreen read as 0; RECOIL_WINDOWED=1)\n");
    }
    // direct start (KG-46): RECOIL_START_MISSION=N (default 1; 0 = normal start-up)
    const char* sm = std::getenv("RECOIL_START_MISSION");
    g_start_mission = sm ? std::atoi(sm) : (g_portable ? 0 : 1);   // portable: normal start-up like the original
    g_compare = std::getenv("RECOIL_COMPARE") != nullptr;
    g_load_save = std::getenv("RECOIL_LOAD_SAVE");
    if (const char* nt = std::getenv("RECOIL_NEAR_TURRET")) g_near_turret = static_cast<float>(std::atof(nt));
    if (g_start_mission > 0) {
        g_real_peek = reinterpret_cast<PeekFn>(recoil::g_Iat_PeekMessageA_004cc64c);
        recoil::g_Iat_PeekMessageA_004cc64c = reinterpret_cast<void*>(&StartMissionPeek);
        std::fprintf(stderr, "boot: direct start of mission %d (RECOIL_START_MISSION=0 for the normal start-up)\n", g_start_mission);
    }
    // shared input (KG-60): RECOIL_SHARED_INPUT=1
    if (const char* si = std::getenv("RECOIL_SHARED_INPUT"); si && std::strcmp(si, "0") != 0) {
        g_real_di_create = reinterpret_cast<DiCreateFn>(recoil::g_Iat_DirectInputCreateA_004cc04c);
        recoil::g_Iat_DirectInputCreateA_004cc04c = reinterpret_cast<void*>(&SharedDiCreate);
        g_real_findwin = reinterpret_cast<FindWinFn>(recoil::g_Iat_FindWindowA_004cc628);
        recoil::g_Iat_FindWindowA_004cc628 = reinterpret_cast<void*>(&SideBySideFindWindow);
        g_keep_active = std::getenv("RECOIL_KEEP_ACTIVE") != nullptr;   // off: crashes the remake at 0x0040bb00 (Mission_HandleCommand) - under investigation
    }
    // process-relative tick (KG-54): RECOIL_TICK_FROM_BOOT=0 keeps the raw since-Windows-start value
    if (const char* tk = std::getenv("RECOIL_TICK_FROM_BOOT"); !tk || std::strcmp(tk, "0") != 0) {
        g_tick_base = GetTickCount() - 1000;   // PLATFORM: start at 1 s, never 0
        recoil::g_Iat_GetTickCount_004cc140 = reinterpret_cast<void*>(&ProcessTickCount);
        std::fprintf(stderr, "boot: GetTickCount counts from start-up (float clock stays fine; RECOIL_TICK_FROM_BOOT=0 to disable)\n");
    }
    // Release builds carry no bytes of Recoil.exe: the data image is filled from the player's own copy here (runtime image,
    // tools/asm_port/gen_data_image.py --runtime). Compiled-in developer builds skip this (ImageData_IsRuntime() == false).
    if (recoil::ImageData_IsRuntime()) {
        char exe[MAX_PATH] = {}, self[MAX_PATH] = {}, why[256] = {};
        GetModuleFileNameA(nullptr, self, MAX_PATH);
        const char* env = std::getenv("RECOIL_ORIGINAL_EXE");
        char cand[3][MAX_PATH] = {};
        if (env) std::snprintf(cand[0], MAX_PATH, "%s", env);
        std::snprintf(cand[1], MAX_PATH, "%s\\original\\Recoil.exe", g_exe_dir);   // where Setup keeps the player's original
        GetFullPathNameA("Recoil.exe", MAX_PATH, cand[2], nullptr);                // a developer game folder holding the original
        for (auto& c : cand)
            if (*c && GetFileAttributesA(c) != INVALID_FILE_ATTRIBUTES && _stricmp(c, self) != 0) { std::snprintf(exe, MAX_PATH, "%s", c); break; }
        if (!*exe || !recoil::ImageData_LoadFromExe(exe, why, sizeof why)) {
            char msg[600];
            std::snprintf(msg, sizeof msg, "The Recoil remake needs your original Recoil.exe (the 1999-01-29 build, e.g. from the Recoil Classic CD).\n\n%s%s%s\n\nRun Setup again and point it at your copy of Recoil.",
                          *exe ? why : "It was not found in the original folder next to the game.", *exe ? "\n" : "", exe);
            std::fprintf(stderr, "boot: %s\n", msg);
            MessageBoxA(nullptr, msg, "Recoil remake", MB_OK | MB_ICONERROR);
            return 4;
        }
        std::fprintf(stderr, "boot: data image loaded from %s\n", exe);
    }
    std::fprintf(stderr, "boot: calling the ported entry (0x004c6140) in %s\n", dir);
    std::fflush(stderr);
    return recoil::entry(0, 0);
}
