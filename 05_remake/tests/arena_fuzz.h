// Generic native L1 oracle for small instruction-level ports (tests/arena_fuzz.h).
//
// Each call gets arguments drawn from a random "arena": a block of words, most of them pointers back into the
// same side's arena (so the code can follow pointer chains of any depth), the rest random values. The original
// and the port each get their own copy of the arena and the same arguments (pointers rebased onto their own
// copy). Afterwards the arenas, EAX, EDX and the heap graphs (tests/heap_graph.h) must agree, with every arena
// pointer compared as an offset. Globals the functions touch are compared through a caller-supplied pair of
// ranges (original VA range in the mapped image vs. the port's copy). Calls run under the watchdog, so a fault
// or hang must happen identically on both sides.
//
// Suited to accessors and small routines whose inputs are "some structures"; routines whose inputs need
// meaning (strings, sizes, valid lists) get their own tests.
#pragma once

#include "heap_graph.h"
#include "native_oracle.h"
#include "platform/image/original_data.h"
#include "watchdog.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <algorithm>
#include <map>
#include <vector>

namespace af {

constexpr int kArenaWords = 8192;  // 32 KB: index arithmetic (idx * record size) can reach far past the pointers' targets,
                                   // and everything it can reach must be the same snapshot on both sides
constexpr int kPointerWords = 192;  // pointers target the first 3/4: code may write past its structure (still compared)

struct GlobalRange {
    std::uint32_t original_va;  // in the mapped original image
    void* port;                 // the port's copy (data image mirror or named block)
    std::uint32_t size;
    bool keep = false;          // a global only a callee touches: reset to its pristine value for every call (not
                                // randomised - random state there makes nearly every call fault), still compared
};

struct Arena {
    alignas(16) std::uint32_t w[kArenaWords];
};

struct Result {
    int calls = 0, abnormal = 0, bad = 0;
    std::string first;
};

// Calls f(ECX = a[0], EDX = a[1], stack a[2..2+nstack)) with a fastcall/stdcall-style callee-pop (the callee pops
// its own stack arguments, as RET n in the listing does). ESP is restored from a saved copy either way.
inline int x87_depth()
{
    unsigned short env[14];
    __asm fnstenv env
    __asm fldenv env
    int used = 0;
    for (int i = 0; i < 8; ++i)
        if (((env[4] >> (2 * i)) & 3) != 3) ++used;
    return used;
}

// ST0 left by a function that returns a float/double (all 80 bits), or none.
struct St0 {
    bool present = false;
    unsigned char bits[10] = {};
    bool operator==(const St0& o) const { return present == o.present && (!present || std::memcmp(bits, o.bits, 10) == 0); }
};

inline void call_raw(void* f, const std::uint32_t* a, int nstack, std::uint32_t* eax, std::uint32_t* edx);
inline void call_with(void* f, const std::uint32_t* a, int nstack, std::uint32_t* eax, std::uint32_t* edx, St0* st0)
{
    const int depth0 = x87_depth();
    call_raw(f, a, nstack, eax, edx);
    const int depth1 = x87_depth();
    st0->present = depth1 > depth0;
    if (st0->present) {
        unsigned char* p = st0->bits;
        __asm {
            mov eax, p
            fstp tbyte ptr [eax]
        }
    }
    for (int d = x87_depth(); d > depth0; --d) __asm fstp st(0)  // never leak x87 registers across calls
}

// Fills the stack just below the caller with one pattern, so a function that reads a local it never wrote reads the
// same bytes on both sides instead of whatever the previous call left there (gwNodeUpdateChildrenBox 0x004491b0 copies
// gwNodeGetLocalCorners' corner buffer into the node even when that call returned 5 without writing it: KG row
// original-defect).
__declspec(noinline) inline void wipe_stack_below()
{
    volatile std::uint32_t buf[1024];
    for (auto& w : buf) w = 0xCDCDCDCDu;
}

inline void call_raw(void* f, const std::uint32_t* a, int nstack, std::uint32_t* eax, std::uint32_t* edx)
{
    wipe_stack_below();
    std::uint32_t r1 = 0, r2 = 0, saved = 0;
    const std::uint32_t c = a[0], d = a[1];
    const std::uint32_t* sa = a + 2;
    int n = nstack;
    __asm {
        mov saved, esp
        mov esi, sa
        mov eax, n
    push_loop:
        test eax, eax
        jz pushed
        dec eax
        push dword ptr [esi + eax * 4]
        jmp push_loop
    pushed:
        mov eax, 0x5A5A5A5A  // same leftover EAX on both sides (functions that return nothing)
        mov ecx, c
        mov edx, d
        call f
        mov r1, eax
        mov r2, edx
        mov esp, saved
    }
    *eax = r1; *edx = r2;
}

// Opt-in for tests of float code: when set, half of the non-pointer words are ordinary float values instead of small
// integers (which read as denormals and barely vary a float comparison). Default off: existing tests are unchanged.
inline bool& real_floats()
{
    static bool on = false;
    return on;
}

// Opt-in (default empty = existing tests unchanged): constants a function compares against (harvested from its own listing by
// tools/asm_port/gen_fuzz_test.py --dictionary, each with +-1). When non-empty a quarter of the non-pointer arena words are drawn from it, so a
// magic-value compare (cmp [eax+0xf38],0x75bcd15) sees both sides of its branch; plain random words never hit such values.
inline std::vector<std::uint32_t>& dictionary()
{
    static std::vector<std::uint32_t> d;
    return d;
}

inline Arena& guarded_arena()
{
    constexpr SIZE_T kGuard = 64 * 1024;
    char* m = static_cast<char*>(VirtualAlloc(nullptr, kGuard + sizeof(Arena) + kGuard, MEM_RESERVE, PAGE_NOACCESS));
    VirtualAlloc(m + kGuard, sizeof(Arena), MEM_COMMIT, PAGE_READWRITE);
    return *reinterpret_cast<Arena*>(m + kGuard);
}

// Run `iters` random calls of original `va` vs port `port`; globals compared over `globals`.
// Both sides run on the SAME arena memory, one after the other from the same snapshot, so arena pointers are
// numerically identical on both sides (also inside floats or computed indices); heap and image addresses are
// normalised through the side's heap log (tests/heap_graph.h).
inline Result run(std::uint32_t va, void* port, int nstack, int iters, std::uint32_t seed,
                  const std::vector<GlobalRange>& globals = {})
{
    Result res;
    std::mt19937 rng(seed);
    hg::Recording rec;
    // The arena sits between no-access guard regions: a loop that walks past it faults identically on both sides
    // instead of reading neighbouring test memory that differs between the runs (after_o is written in between).
    static Arena& arena = guarded_arena();
    static Arena snap, after_o;
    // Both sides start from the same data: the pristine original data sections and the pristine data image
    // (tests/native_oracle.h). A call that ends abnormally may have run wild through data (a count seeded with
    // a pointer), so both are restored again after it.
    double t_restore = 0, t_orig = 0, t_port = 0;  // RECOIL_AF_DIAG: seconds spent in each
    std::map<unsigned, std::pair<int, double>> by_outcome;  // RECOIL_AF_DIAG: original-side outcome -> count, seconds
    auto qpc = [] { LARGE_INTEGER c, f; QueryPerformanceCounter(&c); QueryPerformanceFrequency(&f); return double(c.QuadPart) / double(f.QuadPart); };
    auto restore = [&] { const double t0 = qpc(); rt::restore_pristine(); t_restore += qpc() - t0; };
    restore();
    const auto base = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(arena.w));
    auto random_word = [&](unsigned pointer_share) -> std::uint32_t {
        if (rng() % 8 < pointer_share) return base + 4 * (rng() % kPointerWords);
        if (!dictionary().empty() && rng() % 4 == 0) return dictionary()[rng() % dictionary().size()];
        if (real_floats() && rng() % 2) {  // opt-in (real_floats()): half the scalars are ordinary float values
            static const float kF[] = {0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 2.0f, 3.25f, 10.0f, -10.0f, 100.0f, 0.001f, 1000.0f, 0.25f, -2.5f};
            std::uint32_t u;
            std::memcpy(&u, &kF[rng() % 14], 4);
            return u;
        }
        const unsigned kind = rng() % 4;
        // small scalars: counts and indices stay near real sizes (a global count of 0xFFFF would let a loop walk far
        // past its array through the test process's own data)
        return kind == 0 ? rng() % 16 : kind == 1 ? 0 : kind == 2 ? rng() % 64 : rng() % 256;
    };
    // Hang limit for this function (tests/watchdog.h): 5 ms until 16 calls have ended normally, then 200x the
    // 99th-percentile normal call (floor 1 ms), recomputed every 16 normal calls. These are leaf functions on small
    // inputs that finish in microseconds, so a list walk that loops is abandoned after ~1 ms instead of the 50 ms
    // default, which made hang-heavy functions dominate the run. The percentile, not the maximum: one cold first
    // call (page faults) would otherwise set the limit. A call that is really that long on legitimate input still
    // agrees as "hung" on both sides; one side over, one under shows as a DIFF.
    const double kDefaultLimit = wd::st().limit_ns;
    wd::st().limit_ns = 5e6;
    std::vector<double> normal_ns;
    auto note = [&](unsigned outcome) {
        if (outcome != wd::kOk) return;
        normal_ns.push_back(wd::st().last_ns);
        if (normal_ns.size() % 16 == 0) {
            std::vector<double> v = normal_ns;
            std::sort(v.begin(), v.end());
            const double lim = v[v.size() * 99 / 100] * 200;
            wd::st().limit_ns = lim < 1e6 ? 1e6 : lim > kDefaultLimit ? kDefaultLimit : lim;
        }
    };
    struct RestoreLimit { double v; ~RestoreLimit() { wd::st().limit_ns = v; } } restore_limit{kDefaultLimit};
    // until iters calls have ended normally (the ones that compare data), or 20x that many attempts
    // pristine values of the kept globals, per side (a relocated pointer differs between the original and the mirror)
    std::vector<std::uint32_t> kept[2];
    for (int side = 0; side < 2; ++side)
        for (const GlobalRange& g : globals)
            for (std::uint32_t k = 0; k + 4 <= g.size; k += 4) {
                std::uint32_t v = 0;
                std::memcpy(&v, side ? static_cast<char*>(g.port) + k : reinterpret_cast<char*>(static_cast<std::uintptr_t>(g.original_va)) + k, 4);
                kept[side].push_back(v);
            }
    int hung = 0, runaways = 0, retries = 0;
    const DWORD t_start = GetTickCount();
    for (int n = 0; n < 20 * iters && res.calls - res.abnormal < iters && !res.bad; ++n) {
        hg::Log O, P;
        P.port = true;  // map the port's image addresses back to original VAs (tests/heap_graph.h)
        for (auto& w : snap.w) w = random_word(4);
        std::vector<std::uint32_t> gvals;
        for (const GlobalRange& g : globals)
            for (std::uint32_t k = 0; k + 4 <= g.size; k += 4) gvals.push_back(g.keep ? 0 : random_word(4));
        std::uint32_t args[8];
        for (int i = 0; i < 2 + nstack; ++i) args[i] = rng() % 4 ? base + 4 * (rng() % kPointerWords) : rng() % 64;
        auto set_globals = [&](bool port_side) {
            std::size_t gi = 0;
            for (const GlobalRange& g : globals)
                for (std::uint32_t k = 0; k + 4 <= g.size; k += 4, ++gi) {
                    char* at = port_side ? static_cast<char*>(g.port) + k : reinterpret_cast<char*>(static_cast<std::uintptr_t>(g.original_va)) + k;
                    std::memcpy(at, g.keep ? &kept[port_side][gi] : &gvals[gi], 4);
                }
        };
        auto read_globals = [&](bool port_side) {
            std::vector<std::uint32_t> v;
            for (const GlobalRange& g : globals)
                for (std::uint32_t k = 0; k + 4 <= g.size; k += 4) {
                    std::uint32_t x;
                    std::memcpy(&x, port_side ? static_cast<char*>(g.port) + k : reinterpret_cast<char*>(static_cast<std::uintptr_t>(g.original_va)) + k, 4);
                    v.push_back(x);
                }
            return v;
        };
        std::uint32_t eo = 0, dO = 0, ep = 0, dp = 0;
        St0 fo, fp;
        unsigned so = 0, sp = 0;
        std::vector<std::uint32_t> go, gp;
        auto run_pair = [&] {
            arena = snap;
            set_globals(false);
            hg::current() = &O;
            double tc = qpc();
            so = wd::guarded([&] { call_with(reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)), args, nstack, &eo, &dO, &fo); });
            note(so);
            t_orig += qpc() - tc;
            ++by_outcome[so].first;
            by_outcome[so].second += qpc() - tc;
            after_o = arena;
            go = read_globals(false);
            arena = snap;
            set_globals(true);
            hg::current() = &P;
            tc = qpc();
            sp = wd::guarded([&] { call_with(port, args, nstack, &ep, &dp, &fp); });
            note(sp);
            t_port += qpc() - tc;
            gp = read_globals(true);
        };
        run_pair();
        // Hung on one side, a normal end on the other: re-run the same input before calling it a difference. A tight per-function limit (tests/watchdog.h) meets a legitimately slower input, or a
        // thread that lost its core under parallel load, now and then; a real difference repeats.
        // Up to two re-runs, at the full default limit: under the whole suite's load (tools/run_tests.py) one re-run
        // at 20x a 1 ms limit was still preempted once (Container_GetNth, then 32/32 clean when stressed alone).
        for (int again = 0; again < 2 && ((so == wd::kHung && sp == wd::kOk) || (sp == wd::kHung && so == wd::kOk)); ++again) {
            ++retries;
            restore();
            O.reset();
            P.reset();
            const double lim = wd::st().limit_ns;
            wd::st().limit_ns = kDefaultLimit;
            run_pair();
            wd::st().limit_ns = lim;
        }
        ++res.calls;
        auto fail = [&](const std::string& why) { if (!res.bad++) res.first = "#" + std::to_string(n) + ": " + why; };
        // A call that ran away on both sides (hung on one, faulted on the other) is inconclusive, not a difference:
        // how far a runaway loop gets before the hang limit - through data it zeroes, frees or follows - is timing,
        // and such a call compares no data either way. Hung or faulted against a normal end, or two different
        // fault codes, is still a difference.
        const bool runaway = (so == wd::kHung && sp != wd::kOk) || (sp == wd::kHung && so != wd::kOk);
        if (so == wd::kHung || sp == wd::kHung) ++hung;
        if (runaway && so != sp) ++runaways;
        if (so != sp && !runaway) { fail("outcome " + std::to_string(so) + " vs " + std::to_string(sp)); restore(); break; }
        if (so != wd::kOk) { ++res.abnormal; restore(); continue; }
        if (eo != ep && hg::norm(O, eo) != hg::norm(P, ep)) {
            char m[64];
            std::snprintf(m, sizeof m, "EAX: original %08x port %08x", eo, ep);
            fail(m);
            continue;
        }
        if (!(fo == fp)) { fail("ST0 result"); continue; }
        for (int i = 0; i < kArenaWords; ++i)
            if (after_o.w[i] != arena.w[i] && hg::norm(O, after_o.w[i]) != hg::norm(P, arena.w[i])) {
                char m[96];
                std::snprintf(m, sizeof m, "arena word %d: original %08x port %08x", i, after_o.w[i], arena.w[i]);
                fail(m);
                break;
            }
        for (std::size_t i = 0; i < go.size(); ++i)
            if (go[i] != gp[i] && hg::norm(O, go[i]) != hg::norm(P, gp[i])) { fail("global word " + std::to_string(i)); break; }
        const std::string d = hg::compare(O, P);
        if (!d.empty()) fail(d);
        for (hg::Log* L : {&O, &P})
            for (auto& [p, blk] : L->live)
                if (blk.role >= 1000) hg::real().free_(reinterpret_cast<void*>(p));
    }
    if (std::getenv("RECOIL_AF_DIAG"))  // diagnostics: where the time went
        std::printf("    diag: %d calls, %d with a hang, %d runaway (hung vs fault), %d retried, limit %.2f ms, %lu ms (restore %.1f s, original %.1f s, port %.1f s)\n",
                    res.calls, hung, runaways, retries, wd::st().limit_ns / 1e6, GetTickCount() - t_start,
                    t_restore, t_orig, t_port);
    if (std::getenv("RECOIL_AF_DIAG"))
        std::printf("    diag: watchdog timer %s, polls %.0f, hang detection %.0f ms total, unwind %.0f ms total%s",
                    wd::st().timer ? "high-res" : "NONE (Sleep fallback)", wd::st().polls, wd::st().detect_ms, wd::st().unwind_ms, "\n");
    if (std::getenv("RECOIL_AF_DIAG"))
        for (const auto& [code, v] : by_outcome) std::printf("    diag: outcome %08x x%d %.2f s\n", code, v.first, v.second);
    return res;
}

}  // namespace af
