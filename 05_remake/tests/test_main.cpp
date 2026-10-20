#include "test.h"
#include "platform/image/original_data.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

// The native L1 oracle (native_oracle.h) needs the original image range 0x00400000..0x007c9000 free.
// The Windows loader maps C_437.NLS at 0x00400000 during process start-up, before main. So the runner
// relaunches itself suspended, reserves that range in the child BEFORE the child's loader runs, and
// resumes it; the tests run in the child.
static int run_in_reserved_child()
{
    char exe[MAX_PATH];
    GetModuleFileNameA(nullptr, exe, MAX_PATH);
    SetEnvironmentVariableA("RECOIL_TESTS_CHILD", "1");
    STARTUPINFOA si{};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi{};
    if (!CreateProcessA(exe, GetCommandLineA(), nullptr, nullptr, TRUE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
        std::printf("cannot relaunch for the native oracle (error %lu)\n", GetLastError());
        return -1;
    }
    VirtualAllocEx(pi.hProcess, reinterpret_cast<void*>(0x00400000), 0x003c9000, MEM_RESERVE, PAGE_NOACCESS);
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    // A crashed child exits with its exception code (0xC0000005...), which is negative as an int: report
    // it as a failure here rather than let main() fall back to running the tests without the reservation.
    if (code >= 0x80000000u) {
        std::printf("test child crashed: exception 0x%08lx\n", code);
        std::fflush(stdout);
        return 1;
    }
    return static_cast<int>(code);
}

// usage: recoil_tests [--list] [NAME...]    NAME: a test name, or a prefix ending in '*'; none = every test.
// tools/run_tests.py runs the tests as parallel processes through this filter (and splits the fuzz tests with
// RECOIL_SHARD, tests/arena_fuzz.h).
static bool selected(const char* name, int argc, char** argv)
{
    bool any = false;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') continue;
        any = true;
        const std::size_t n = std::strlen(argv[i]);
        if (n && argv[i][n - 1] == '*' ? std::strncmp(name, argv[i], n - 1) == 0 : std::strcmp(name, argv[i]) == 0) return true;
    }
    return !any;
}

// A fault that escapes every guard (tests/watchdog.h) ends the child; say where, so a rare escape can be traced:
// exception code, faulting address and the module it lies in, ESP.
static LONG WINAPI report_escape(EXCEPTION_POINTERS* e)
{
    const auto at = reinterpret_cast<std::uintptr_t>(e->ExceptionRecord->ExceptionAddress);
    HMODULE mod = nullptr;
    char name[MAX_PATH] = "?";
    if (at >= 0x00400000 && at < 0x007c9000) std::snprintf(name, sizeof name, "Recoil.exe image (original)");
    else if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                reinterpret_cast<const char*>(at), &mod))
        GetModuleFileNameA(mod, name, MAX_PATH);
    std::printf("ESCAPED FAULT %08lx at %08x (%s +0x%x), esp %08lx, access %s %08x\n", e->ExceptionRecord->ExceptionCode,
                static_cast<unsigned>(at), name, mod ? static_cast<unsigned>(at - reinterpret_cast<std::uintptr_t>(mod)) : 0u,
                e->ContextRecord->Esp, e->ExceptionRecord->NumberParameters > 1 && e->ExceptionRecord->ExceptionInformation[0] ? "write" : "read",
                e->ExceptionRecord->NumberParameters > 1 ? static_cast<unsigned>(e->ExceptionRecord->ExceptionInformation[1]) : 0u);
    std::fflush(stdout);
    return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char** argv)
{
    SetUnhandledExceptionFilter(report_escape);
    if (argc > 1 && std::strcmp(argv[1], "--list") == 0) {
        for (auto& c : rt::registry()) std::printf("%s\n", c.name);
        return 0;
    }
    if (!std::getenv("RECOIL_TESTS_CHILD")) {
        int rc = run_in_reserved_child();
        if (rc >= 0) return rc;
    }
    if (recoil::ImageData_IsRuntime()) {   // runtime data image: fill the mirrors from the player's Recoil.exe first
        const char* exe = std::getenv("RECOIL_ORIGINAL_EXE");
        if (!exe) exe = RECOIL_ORIGINAL_EXE;
        char why[256] = {};
        if (!recoil::ImageData_LoadFromExe(exe, why, sizeof why)) {
            std::printf("FAIL data image from %s: %s\n", exe, why);
            return 2;
        }
    }
    int ran = 0;
    for (auto& c : rt::registry()) {
        if (!selected(c.name, argc, argv)) continue;
        int before = rt::failures();
        std::printf("run  %s\n", c.name);
        std::fflush(stdout);  // so a crash names the test that caused it
        c.fn();
        std::printf("%s %s\n", rt::failures() == before ? "ok  " : "FAIL", c.name);
        ++ran;
    }
    std::printf("%d tests, %d failures\n", ran, rt::failures());
    std::fflush(stdout);
    return rt::failures() == 0 ? 0 : 1;
}
