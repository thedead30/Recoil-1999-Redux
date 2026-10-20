// rcl_dinput.cpp - dinput.dll wrapper for the ORIGINAL Recoil.exe in side-by-side play (tools/sidebyside, KG-60).
// Testing tool only (PLATFORM): placed next to a COPY of the original (Desktop\Recoil Original Windowed); the game
// files are not modified. Loads the real %SystemRoot%\SysWOW64\dinput.dll and, inside Recoil.exe:
//   1. DirectInput devices: SetCooperativeLevel flags -> DISCL_NONEXCLUSIVE | DISCL_BACKGROUND, so the original and the
//      remake can both read the same keyboard and mouse at once (exclusive mode lets only one program have them).
//   2. Direct start of mission N (SBS_START_MISSION, default 1; 0 = normal start): from the game's own message pump,
//      after 20 PeekMessageA calls, Mission_SetDataSearchPaths(N, 0) then App_StartMission(app, 0, N, 0, 1, 1) - the
//      same calls the remake's launcher makes (KG-46). Addresses CONFIRMED-BINARY in 03_re/ledger/functions.csv.
//   3. GetTickCount counts from start-up (the game's float clock steps in 62.5 ms after ~6 days of uptime, KG-54).
// Only acts if the host is the expected Recoil.exe (image base 0x00400000 and the three import slots hold the
// expected functions); otherwise it is a plain pass-through.
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
HMODULE g_real;
using DiCreateFn = HRESULT(WINAPI*)(HINSTANCE, DWORD, void**, void*);
using CreateDevFn = HRESULT(WINAPI*)(void*, REFGUID, void**, void*);
using CoopFn = HRESULT(WINAPI*)(void*, HWND, DWORD);
using PeekFn = BOOL(WINAPI*)(LPMSG, HWND, UINT, UINT, UINT);
CreateDevFn g_create_dev; CoopFn g_coop; PeekFn g_peek;
using FindWinFn = HWND(WINAPI*)(LPCSTR, LPCSTR); FindWinFn g_findwin;
DWORD g_tick_base; int g_peeks, g_start = 1;

constexpr ULONG_PTR kSlotDirectInputCreateA = 0x004cc04c;  // CONFIRMED-BINARY: import slots of Recoil.exe
constexpr ULONG_PTR kSlotPeekMessageA = 0x004cc64c;
constexpr ULONG_PTR kSlotGetTickCount = 0x004cc140;
constexpr ULONG_PTR kSlotFindWindowA = 0x004cc628;     // App_ActivateExistingInstance 0x0042e990: FindWindowA("RecoilClass") -> quit if found
constexpr ULONG_PTR kApp = 0x004f3ca8;                    // main app object (MOV ECX,0x4f3ca8 in Slot_StartMission_1_00431270)
constexpr ULONG_PTR kSetDataSearchPaths = 0x0042ecb0;     // Mission_SetDataSearchPaths
constexpr ULONG_PTR kStartMission = 0x0042e4d0;           // App_StartMission

void log(const char* fmt, ...) {
    char b[512]; va_list a; va_start(a, fmt); vsnprintf(b, sizeof b, fmt, a); va_end(a);
    if (FILE* f = std::fopen("sidebyside.log", "a")) { std::fprintf(f, "[orig %6lu] %s\n", GetTickCount() - g_tick_base, b); std::fclose(f); }
}
void patch_ptr(void** slot, void* fn) {
    DWORD old; VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &old); *slot = fn; VirtualProtect(slot, sizeof(void*), old, &old);
}
HRESULT WINAPI Coop(void* self, HWND w, DWORD flags) {
    const DWORD nf = (flags & ~(0x1u | 0x4u)) | 0x2u | 0x8u;   // PLATFORM: EXCLUSIVE|FOREGROUND -> NONEXCLUSIVE|BACKGROUND
    log("SetCooperativeLevel %08lx -> %08lx", flags, nf);
    return g_coop(self, w, nf);
}
HRESULT WINAPI CreateDev(void* self, REFGUID g, void** out, void* outer) {
    const HRESULT hr = g_create_dev(self, g, out, outer);
    if (SUCCEEDED(hr) && out && *out) {
        void** vt = *reinterpret_cast<void***>(*out);
        if (vt[13] != reinterpret_cast<void*>(&Coop)) { if (!g_coop) g_coop = reinterpret_cast<CoopFn>(vt[13]); patch_ptr(&vt[13], reinterpret_cast<void*>(&Coop)); }   // IDirectInputDeviceA::SetCooperativeLevel
    }
    return hr;
}
HWND WINAPI FindWin(LPCSTR c, LPCSTR t) {   // let the remake run next to it (both use "RecoilClass")
    if (c && !IS_INTRESOURCE(c) && std::strcmp(c, "RecoilClass") == 0) return nullptr;
    return g_findwin(c, t);
}
// Keep running when not focused: the game pauses/minimises on WM_ACTIVATEAPP(FALSE) / WM_ACTIVATE(WA_INACTIVE). For side by
// side both games must keep running, so the game window is subclassed once and those messages are swallowed; minimise is vetoed.
WNDPROC g_game_proc; HWND g_game_wnd; DWORD g_wnd_tick; HHOOK g_cbt;
LRESULT CALLBACK KeepActiveProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_ACTIVATEAPP && !w) return 0;
    if (m == WM_ACTIVATE && LOWORD(w) == WA_INACTIVE) return 0;
    if (m == WM_NCACTIVATE && !w) return TRUE;
    if (m == WM_SYSCOMMAND && (w & 0xfff0) == SC_MINIMIZE) return 0;
    return CallWindowProcA(g_game_proc, h, m, w, l);
}
LRESULT CALLBACK NoMinimize(int code, WPARAM w, LPARAM l) {
    const int sw = static_cast<int>(LOWORD(l));
    if (code == HCBT_MINMAX && (sw == SW_MINIMIZE || sw == SW_SHOWMINIMIZED || sw == SW_SHOWMINNOACTIVE || sw == SW_FORCEMINIMIZE)) return 1;
    return CallNextHookEx(nullptr, code, w, l);
}
void keep_active() {
    if (g_game_wnd) return;
    EnumThreadWindows(GetCurrentThreadId(), [](HWND h, LPARAM) -> BOOL {
        char c[64]; GetClassNameA(h, c, sizeof c);
        if (std::strcmp(c, "RecoilClass") == 0 && !g_game_wnd) {
            g_game_wnd = h; g_wnd_tick = GetTickCount();
            g_game_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&KeepActiveProc)));
            g_cbt = SetWindowsHookExA(WH_CBT, &NoMinimize, nullptr, GetCurrentThreadId());
            log("keep-active installed on window %p", h);
        }
        return TRUE;
    }, 0);
}
// gameplay-start signal for compare_saves.ps1: App_EnterGameplay 0x0042eed0 sets [0x004f3df0]=1 right before Mission_BeginPlay
int g_play_flag = -1; bool g_loaded = false;
void watch_play() {
    const int v = *reinterpret_cast<volatile int*>(0x004f3df0);
    if (v != g_play_flag) {
        log("play flag %d -> %d", g_play_flag, v);
        // SBS_DUMP=<file>: full memory dump the moment gameplay starts (save applied, zero frames played) - from inside the game thread
        if (g_play_flag == 0 && v == 1 && g_loaded) {
            if (const char* d = std::getenv("SBS_DUMP")) {
                HANDLE f = CreateFileA(d, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                using DumpFn = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, int, void*, void*, void*);
                auto dump = reinterpret_cast<DumpFn>(GetProcAddress(LoadLibraryA("dbghelp.dll"), "MiniDumpWriteDump"));
                const BOOL ok = f != INVALID_HANDLE_VALUE && dump && dump(GetCurrentProcess(), GetCurrentProcessId(), f, 0x2 | 0x4 | 0x800 | 0x1000, nullptr, nullptr, nullptr);   // FullMemory|HandleData|FullMemoryInfo|ThreadInfo
                if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
                log("frame-0 dump %s -> %d", d, ok);
            }
        }
        g_play_flag = v;
    }
}
DWORD WINAPI Tick() { keep_active(); watch_play(); return GetTickCount() - g_tick_base; }
// KG-61: load a saved game (SBS_LOAD_SAVE=<name in SavedGames>) - the sequence of LoadGameScreen_Load 0x00435a70, mode 0
// (control-settings setters skipped). Addresses CONFIRMED-BINARY in 03_re/ledger/functions.csv.
char g_load[MAX_PATH]; DWORD g_load_ms = 8000;   // after the game window appears
int load_save(const char* name) {
    char path[MAX_PATH]; std::snprintf(path, sizeof path, "SavedGames\\%s", name);
    if (!reinterpret_cast<int(__fastcall*)(int, int)>(0x004c0050)(reinterpret_cast<int>(path), 0)) return 0;   // SaveGame_LoadFromFile
    reinterpret_cast<int(__fastcall*)(int, int)>(0x00415630)(0, 0);                                            // StopGlobalVoice
    int* sounds = reinterpret_cast<int*>(0x004f3fc8);
    if (*sounds) { reinterpret_cast<int(__fastcall*)(int, int)>(0x004a05f0)(*sounds, 0); *sounds = 0; }        // SoundList_Destroy
    auto dup = *reinterpret_cast<char*(__cdecl**)(const char*)>(0x004cc5e4);                                  // _strdup import slot
    *reinterpret_cast<char**>(0x004f3ec4) = dup(path);
    *reinterpret_cast<int*>(0x004f3ea8) = 1;
    // no ScreenManager_QueuePop: the menu pops its own Load Game screen; from the main menu a pop would empty the stack (quit)
    reinterpret_cast<int(__fastcall*)(int, int, int, int)>(0x00443160)(static_cast<int>(kApp), 0, 0x004f3e80, 0);   // ScreenManager_QueuePush
    return 1;
}
BOOL WINAPI Peek(LPMSG m, HWND w, UINT lo, UINT hi, UINT rm) {
    if (*g_load && !g_start && ++g_peeks == 20) {   // launcher window: press its Game menu's first item (Play) so the main menu starts
        EnumThreadWindows(GetCurrentThreadId(), [](HWND h, LPARAM) -> BOOL {
            HMENU m = GetMenu(h); HMENU sub = m ? GetSubMenu(m, 0) : nullptr;
            if (!sub) return TRUE;
            const UINT id = GetMenuItemID(sub, 0); char t[64] = {}; GetMenuStringA(sub, 0, t, sizeof t, MF_BYPOSITION);
            log("launcher: Game menu item 0 '%s' id %u -> pressed", t, id);
            PostMessageA(h, WM_COMMAND, id, 0); return FALSE;
        }, 0);
    }
    if (*g_load && g_game_wnd && GetTickCount() - g_wnd_tick > g_load_ms) {   // after the main menu is up (start-up screens must exist first)
        { const int lr = load_save(g_load); g_loaded = lr != 0; log("load %s -> %d", g_load, lr); } *g_load = 0; g_start = 0;
    }
    else if (!*g_load && g_start > 0 && ++g_peeks == 20) {
        const int n = g_start; g_start = 0;
        log("direct start of mission %d", n);
        reinterpret_cast<int(__fastcall*)(int, int)>(kSetDataSearchPaths)(n, 0);
        const int r = reinterpret_cast<int(__fastcall*)(int, int, int, int, int, int)>(kStartMission)(static_cast<int>(kApp), 0, n, 0, 1, 1);
        log("App_StartMission returned %d", r);
    }
    return g_peek(m, w, lo, hi, rm);
}
bool host_is_recoil() {
    if (reinterpret_cast<ULONG_PTR>(GetModuleHandleA(nullptr)) != 0x00400000) return false;
    HMODULE u = GetModuleHandleA("user32.dll"), k = GetModuleHandleA("kernel32.dll");
    if (!u || !k) return false;
    __try {
        return *reinterpret_cast<void**>(kSlotPeekMessageA) == GetProcAddress(u, "PeekMessageA");
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
void init() {
    char p[MAX_PATH]; GetSystemWow64DirectoryA(p, MAX_PATH);
    if (!*p) GetSystemDirectoryA(p, MAX_PATH);
    std::strcat(p, "\\dinput.dll");
    g_real = LoadLibraryA(p);
    if (!host_is_recoil()) return;
    std::remove("sidebyside.log");
    if (const char* s = std::getenv("SBS_START_MISSION")) g_start = std::atoi(s);
    if (const char* s = std::getenv("SBS_LOAD_MS")) g_load_ms = std::atoi(s);
    if (const char* s = std::getenv("SBS_LOAD_SAVE")) std::snprintf(g_load, sizeof g_load, "%s", s);
    g_tick_base = GetTickCount() - 1000;
    g_peek = *reinterpret_cast<PeekFn*>(kSlotPeekMessageA);
    patch_ptr(reinterpret_cast<void**>(kSlotPeekMessageA), reinterpret_cast<void*>(&Peek));
    patch_ptr(reinterpret_cast<void**>(kSlotGetTickCount), reinterpret_cast<void*>(&Tick));
    g_findwin = *reinterpret_cast<FindWinFn*>(kSlotFindWindowA);
    patch_ptr(reinterpret_cast<void**>(kSlotFindWindowA), reinterpret_cast<void*>(&FindWin));
    log("wrapper active: shared input, mission %d direct start, start-up tick, load [%s]", g_start, g_load);
}
}  // namespace

extern "C" HRESULT WINAPI DirectInputCreateA(HINSTANCE inst, DWORD ver, void** out, void* outer) {
    auto fn = reinterpret_cast<DiCreateFn>(GetProcAddress(g_real, "DirectInputCreateA"));
    const HRESULT hr = fn(inst, ver, out, outer);
    if (SUCCEEDED(hr) && out && *out) {
        void** vt = *reinterpret_cast<void***>(*out);
        if (vt[3] != reinterpret_cast<void*>(&CreateDev)) { if (!g_create_dev) g_create_dev = reinterpret_cast<CreateDevFn>(vt[3]); patch_ptr(&vt[3], reinterpret_cast<void*>(&CreateDev)); }   // IDirectInputA::CreateDevice
    }
    return hr;
}
extern "C" HRESULT WINAPI DirectInputCreateW(HINSTANCE i, DWORD v, void** o, void* u) {
    return reinterpret_cast<DiCreateFn>(GetProcAddress(g_real, "DirectInputCreateW"))(i, v, o, u);
}
extern "C" HRESULT WINAPI DirectInputCreateEx(HINSTANCE i, DWORD v, REFIID r, void** o, void* u) {
    return reinterpret_cast<HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, void**, void*)>(GetProcAddress(g_real, "DirectInputCreateEx"))(i, v, r, o, u);
}
BOOL WINAPI DllMain(HINSTANCE, DWORD why, void*) {
    if (why == DLL_PROCESS_ATTACH) init();
    return TRUE;
}
