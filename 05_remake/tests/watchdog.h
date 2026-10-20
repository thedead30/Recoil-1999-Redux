// Watchdog for native L1 tests of code that can loop forever on some inputs (the original included): a call
// made through wd::guarded() that runs past the limit is abandoned - a watchdog thread suspends the calling
// thread, redirects it to a longjmp back into guarded(), and guarded() returns kHung. The abandoned call's heap
// state is leaked, which a test can tolerate. Both sides are run through it, so "the original hangs and the port
// hangs" can be checked as agreement rather than killing the test run.
//
// Timing (measured on this machine, tests/arena_fuzz.h RECOIL_AF_DIAG): the limit is wall time on
// QueryPerformanceCounter, polled every 0.5 ms on a high-resolution waitable timer. Two cheaper-looking designs
// failed: Sleep(1) sleeps a full 15.6 ms tick (Windows 11 ignores timeBeginPeriod for a process without a visible
// window), and per-thread CPU time (QueryThreadCycleTime) is only brought up to date at a clock tick, even for a
// suspended thread - either way every abandoned call cost ~16-19 ms, which made hang-heavy functions dominate a
// run. Wall time can count a stretch where the thread waited for a core (tests run in parallel,
// tools/run_tests.py); a normal call lasts microseconds, so that needs a >1 ms preemption inside it, and it would
// show as a disagreement (hung vs ok), never as a false pass.
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <malloc.h>  // _resetstkoflw

#include <csetjmp>
#include <cstdint>

namespace wd {

struct State {
    std::jmp_buf jb;
    HANDLE thread = nullptr;
    HANDLE watcher = nullptr;
    HANDLE timer = nullptr;
    volatile LONG armed = 0;
    volatile LONG gen = 0;   // bumped at every arm: the watcher only abandons the call it measured (see watch)
    volatile LONG busy = 0;  // > 0 while test-harness code runs inside the call (tests/heap_graph.h recorders)
    volatile LONGLONG start = 0;  // QueryPerformanceCounter at arm
    double qpc_to_ns = 0;
    // Default limit 50 ms: the composite tests' normal calls (a whole clip, triangulation) take well under 1 ms,
    // and 259 genuine hangs per clipper run at 250 ms made it the longest job (131 s). tests/arena_fuzz.h tightens
    // it per function from its normal calls (floor 1 ms).
    double limit_ns = 50e6;
    double last_ns = 0;  // duration of the last call that ended by itself (normally or by an exception)
    std::uintptr_t ntdll_lo = 0, ntdll_hi = 0, crt_lo = 0, crt_hi = 0;
    unsigned short fpu_cw = 0;
    // diagnostics (RECOIL_AF_DIAG): totals over abandoned calls
    volatile LONGLONG fired = 0;
    double detect_ms = 0, unwind_ms = 0, polls = 0;
};
inline State& st()
{
    static State s;
    return s;
}

inline LONGLONG qpc()
{
    LARGE_INTEGER q;
    QueryPerformanceCounter(&q);
    return q.QuadPart;
}

// Runs on the abandoned thread (entered by SetThreadContext): reset the x87 state the loop left behind,
// restore the control word saved at arm time, and unwind to guarded().
inline void escape()
{
    unsigned short cw = st().fpu_cw;
    __asm fninit
    __asm fldcw cw
    std::longjmp(st().jb, 1);
}

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

inline DWORD WINAPI watch(void*)
{
    State& s = st();
    for (;;) {
        LARGE_INTEGER due;
        due.QuadPart = -5000;  // 0.5 ms, relative, 100 ns units
        if (s.timer && SetWaitableTimer(s.timer, &due, 0, nullptr, nullptr, FALSE)) WaitForSingleObject(s.timer, INFINITE);
        else Sleep(1);
        const LONG g = s.gen;
        if (!s.armed) continue;
        s.polls += 1;
        if (double(qpc() - s.start) * s.qpc_to_ns < s.limit_ns) continue;
        SuspendThread(s.thread);
        // Re-check with the thread stopped (GetThreadContext waits for the suspend to take effect): between the
        // check above and the suspend, the call may have ended and the next one been armed (a race that, under
        // load, abandoned a fresh call at once). Never abandon the thread inside msvcrt either: its routines finish or
        // fault on their own, and a longjmp out of one (sprintf, realloc, strtok) can leave its internal state half
        // updated and crash a later test (seen once, rare) - up to 4x the limit: past that it is abandoned anyway, or a
        // loop of CRT calls (_stricmp over a runaway count) would starve the watchdog. Never abandon the thread inside ntdll - it is then dispatching an
        // exception the call raised, which must finish - nor inside harness code (NoEscape).
        CONTEXT c{};
        c.ContextFlags = CONTEXT_CONTROL;
        if (GetThreadContext(s.thread, &c) && s.gen == g && s.armed && s.busy == 0
            && !(c.Eip >= s.ntdll_lo && c.Eip < s.ntdll_hi)
            && !(c.Eip >= s.crt_lo && c.Eip < s.crt_hi && double(qpc() - s.start) * s.qpc_to_ns < 4 * s.limit_ns)) {
            c.Esp = (c.Esp - 512) & ~15u;
            c.Eip = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&escape));
            SetThreadContext(s.thread, &c);
            s.fired = qpc();
            s.armed = 0;
        }
        ResumeThread(s.thread);
    }
}

// Held by harness code that the call under test calls into (the heap recorders): abandoning the thread there
// would leave the harness's own structures half-updated. The watcher waits until it is released. guarded()
// clears it at every arm, since an exception raised inside such code skips the destructor.
struct NoEscape {
    NoEscape() { InterlockedIncrement(&st().busy); }
    ~NoEscape() { InterlockedDecrement(&st().busy); }
};

enum Outcome : unsigned { kOk = 0, kHung = 1 };  // any other value: the SEH exception code the call raised

template <class F>
unsigned seh_raw(F& f)
{
    __try {
        f();
    } __except (GetExceptionCode() == EXCEPTION_BREAKPOINT || GetExceptionCode() == EXCEPTION_SINGLE_STEP
                        || GetExceptionCode() == 0x4000001E || GetExceptionCode() == 0x4000001F  // WoW64 forms
                    ? EXCEPTION_CONTINUE_SEARCH
                    : EXCEPTION_EXECUTE_HANDLER) {
        unsigned short cw = st().fpu_cw;
        __asm fninit
        __asm fldcw cw
        return GetExceptionCode();
    }
    return kOk;
}

// A stack overflow (runaway recursion through a cyclic input, e.g. ConfigTree_FreeNode) consumes the thread's stack
// guard page; re-arm it once the handler has unwound (_resetstkoflw may not run inside the handler), or the next
// overflow has no guard page and kills the process with an access violation.
template <class F>
unsigned seh_call(F& f)
{
    const unsigned r = seh_raw(f);
    if (r == EXCEPTION_STACK_OVERFLOW) _resetstkoflw();
    return r;
}

// Calls f(): kOk, kHung (abandoned after the limit), or the exception code (e.g. 0xC0000005) it raised.
template <class F>
unsigned guarded(F&& f)
{
    State& s = st();
    if (!s.watcher) {
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &s.thread, 0, FALSE, DUPLICATE_SAME_ACCESS);
        const auto nt = reinterpret_cast<std::uintptr_t>(GetModuleHandleA("ntdll.dll"));
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(nt);
        s.ntdll_lo = nt;
        s.ntdll_hi = nt + reinterpret_cast<const IMAGE_NT_HEADERS*>(nt + dos->e_lfanew)->OptionalHeader.SizeOfImage;
        const auto crt = reinterpret_cast<std::uintptr_t>(LoadLibraryA("msvcrt.dll"));
        s.crt_lo = crt;
        s.crt_hi = crt + reinterpret_cast<const IMAGE_NT_HEADERS*>(crt + reinterpret_cast<const IMAGE_DOS_HEADER*>(crt)->e_lfanew)->OptionalHeader.SizeOfImage;
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        s.qpc_to_ns = 1e9 / double(f.QuadPart);
        s.timer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        s.watcher = CreateThread(nullptr, 0, watch, nullptr, 0, nullptr);
    }
    unsigned short cw;
    __asm fnstcw cw
    s.fpu_cw = cw;
    if (setjmp(s.jb)) {
        const LONGLONG e = qpc();
        s.detect_ms += double(s.fired - s.start) * s.qpc_to_ns / 1e6;
        s.unwind_ms += double(e - s.fired) * s.qpc_to_ns / 1e6;
        return kHung;
    }
    // Every call starts from the same x87 state: empty register stack, the saved control word. The ABI has the
    // stack empty at a call anyway, but an abandoned or faulted call can leave registers behind (call_with only
    // pops back to the depth it found), and a nearly full stack turns the next computation's pushes into stack
    // overflows - the indefinite NaN 0xffc00000 on one side only. Seen under heavy parallel load (32 processes):
    // 2 of 32 Polygon_NewellPlane runs differed after runaway calls.
    __asm fninit
    __asm fldcw cw
    InterlockedIncrement(&s.gen);
    s.busy = 0;
    s.start = qpc();
    s.armed = 1;
    const unsigned r = seh_call(f);
    s.armed = 0;
    s.last_ns = double(qpc() - s.start) * s.qpc_to_ns;
    return r;
}

}  // namespace wd
