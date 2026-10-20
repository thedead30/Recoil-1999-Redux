// The case runner of the fake-vtable tests (tests/fake_vtable.h): a World of test-owned blocks with a normaliser
// (pointers into blocks as (block, offset), the fake table and the stack as tokens, the port's image addresses as
// original VAs), a Case (slots, graph, call) and run_cases, which runs every case on both sides from one seed and
// compares the call log, EAX and every block word. Shared by the structured tests of ui_widgets / menus.
#pragma once

#include "watchdog.h"
#include "cloud_harness.h"
#include "fake_vtable.h"

#include <windows.h>

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace vtr {
// fastcall / thiscall / cdecl call with n stack words; ESP is restored whatever the callee pops
__declspec(noinline) inline std::uint32_t call_any(std::uint32_t fn, std::uint32_t ecx_v, std::uint32_t edx_v, const std::uint32_t* args, int n)
{
    std::uint32_t result = 0;
    __asm {
        mov ebx, esp
        mov eax, n
        mov edx, args
    push_loop:
        test eax, eax
        jz pushed
        dec eax
        push dword ptr [edx + eax*4]
        jmp push_loop
    pushed:
        mov ecx, ecx_v
        mov edx, edx_v
        call fn
        mov esp, ebx
        mov result, eax
    }
    return result;
}

struct World {
    struct Block { std::vector<std::uint32_t> w; };
    std::vector<Block> blocks;
    int side = 0;
    // a block of n bytes filled from the seed; returns its address
    std::uint32_t add(std::mt19937& r, std::uint32_t bytes, bool small = false)
    {
        Block b;
        b.w.resize((bytes + 3) / 4);
        for (auto& x : b.w) {
            x = small ? r() % 64 : (r() % 3 ? r() % 0x10000 : r());
            // a random word that lands inside the port's data mirror would be normalised as a pointer on the port side
            // only (w.norm) - a false difference in about 1 of 200 words; keep filler out of that range
            if (recoil::ImageData_VaOf(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(x)))) x ^= 0x80000000u;
        }
        blocks.push_back(std::move(b));
        return ch::addr(blocks.back().w.data());
    }
    std::uint32_t* at(std::uint32_t p) { return ch::at(p); }
    std::uint32_t norm(std::uint32_t v) const
    {
        if (v == 0) return 0;
        if (v == vt::table()) return 0x7AB1E000;
        for (std::size_t k = 0; k < blocks.size(); ++k) {
            const std::uint32_t base = ch::addr(blocks[k].w.data()), size = static_cast<std::uint32_t>(4 * blocks[k].w.size());
            if (v >= base && v <= base + size) return 0xB0000000u + static_cast<std::uint32_t>(k) * 0x100000u + (v - base);
        }
        NT_TIB* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
        if (v >= ch::addr(tib->StackLimit) && v < ch::addr(tib->StackBase)) return 0x57AC0000;
        if (side) {
            const std::uint32_t va = recoil::ImageData_VaOf(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(v)));
            if (va) return va;
        }
        return v;
    }
};

struct Case {
    const char* name;
    std::uint32_t va;
    void* port;
    int iters;
    // slots, the graph (block 0 is ECX unless the call says otherwise) and the call; returns EAX
    std::function<void(std::mt19937&)> slots;
    std::function<void(World&, std::mt19937&)> build;
    std::function<std::uint32_t(World&, std::uint32_t fn, std::mt19937&)> call;
    bool compare_eax = true;
};

inline std::uint32_t fbits(float f) { return ch::fbits(f); }

// an object of `bytes` with the fake table at +0 and at each embedded offset
inline std::uint32_t object(World& w, std::mt19937& r, std::uint32_t bytes, std::initializer_list<std::uint32_t> embedded = {})
{
    const std::uint32_t p = w.add(r, bytes);
    w.at(p)[0] = vt::table();
    for (std::uint32_t e : embedded) w.at(p)[e / 4] = vt::table();
    return p;
}
// an image header: width / height as shorts at +4 / +6
inline std::uint32_t image(World& w, std::mt19937& r)
{
    const std::uint32_t p = w.add(r, 0x40);
    reinterpret_cast<std::int16_t*>(w.at(p))[2] = static_cast<std::int16_t>(r() % 400 - 50);
    reinterpret_cast<std::int16_t*>(w.at(p))[3] = static_cast<std::int16_t>(r() % 300 - 50);
    return p;
}
inline std::uint32_t pick(std::mt19937& r) { return r() % 3 == 0 ? r() : r() % 700; }


// runs every case on both sides; returns the number of functions that differed
inline int run_cases(const std::vector<Case>& list, const char* what)
{
    int failed = 0, total = 0;
    for (const Case& k : list) {
        static const bool heapcheck = std::getenv("RECOIL_HEAPCHECK") != nullptr;
        if (heapcheck) { std::printf("  case %s\n", k.name); std::fflush(stdout); }  // the last one printed crashed
        std::mt19937 seq(k.va);
        int bad = 0;
        for (int it = 0; it < k.iters; ++it) {
            const std::uint32_t seed = seq();
            std::vector<std::uint32_t> snap[2];
            // a hung-vs-normal disagreement is re-run once at 20x the hang limit (a very long call - e.g. a
            // 365k-entry pixel log - can outlast the per-call limit on one side only; KG-31 pattern, as arena_fuzz.h)
            for (int attempt = 0; attempt < 2; ++attempt) {
                const double saved_limit = wd::st().limit_ns;
                if (attempt) wd::st().limit_ns = saved_limit * 20;
                for (int side = 0; side < 2; ++side) {
                    rt::restore_pristine();
                    std::mt19937 r(seed);
                    World w;
                    w.side = side;
                    w.blocks.reserve(64);
                    vt::reset();
                    k.slots(r);
                    k.build(w, r);
                    if (heapcheck) {  // corruption before the call = the build or the previous iteration's teardown
                        HANDLE hs[64];
                        const DWORD n = GetProcessHeaps(64, hs);
                        for (DWORD h = 0; h < n && h < 64; ++h)
                            if (!HeapValidate(hs[h], 0, nullptr)) {
                                std::printf("  HEAP CORRUPT before the call: %s (%s side, iteration %d)\n", k.name, side ? "port" : "original", it);
                                std::fflush(stdout);
                                std::abort();
                            }
                    }
                    vt::norm() = [&w](std::uint32_t v) { return w.norm(v); };
                    ch::wipe_stack();
                    const std::uint32_t fn = side ? ch::addr(k.port) : k.va;
                    // the original faults on some generated inputs; how the call ended must match, and a faulted call's
                    // half-written state is not compared (as in arena_fuzz.h)
                    std::uint32_t eax = 0;
                    const unsigned outcome = wd::guarded([&] { eax = k.call(w, fn, r); });
                    if (outcome != wd::kOk) {
                        snap[side] = {0xFA017000u, outcome == wd::kHung ? 1u : outcome};
                        rt::restore_pristine();
                        continue;
                    }
                    if (heapcheck) {  // RECOIL_HEAPCHECK=1: name the first case whose call leaves a process heap corrupt
                        HANDLE heaps[64];
                        const DWORD nh = GetProcessHeaps(64, heaps);
                        for (DWORD h = 0; h < nh && h < 64; ++h)
                            if (!HeapValidate(heaps[h], 0, nullptr)) {
                                std::printf("  HEAP CORRUPT after %s (%s side, iteration %d)\n", k.name, side ? "port" : "original", it);
                                std::fflush(stdout);
                                std::abort();
                            }
                    }
                    snap[side] = vt::log();
                    snap[side].push_back(k.compare_eax ? w.norm(eax) : 0u);
                    for (const auto& b : w.blocks)
                        for (std::uint32_t x : b.w) snap[side].push_back(w.norm(x));
                }
                wd::st().limit_ns = saved_limit;
                const bool hung0 = snap[0].size() == 2 && snap[0][0] == 0xFA017000u && snap[0][1] == 1u;
                const bool hung1 = snap[1].size() == 2 && snap[1][0] == 0xFA017000u && snap[1][1] == 1u;
                const bool ok0 = !(snap[0].size() == 2 && snap[0][0] == 0xFA017000u);
                const bool ok1 = !(snap[1].size() == 2 && snap[1][0] == 0xFA017000u);
                if (!((hung0 && ok1) || (hung1 && ok0))) break;
            }
            if (snap[0] != snap[1]) {
                if (!bad) {  // evidence for the first differing case: where the snapshots part and with what
                    std::size_t i = 0;
                    while (i < snap[0].size() && i < snap[1].size() && snap[0][i] == snap[1][i]) ++i;
                    std::printf("  %-44s first diff at word %u of %u/%u: original %08x port %08x\n", k.name,
                                static_cast<unsigned>(i), static_cast<unsigned>(snap[0].size()), static_cast<unsigned>(snap[1].size()),
                                i < snap[0].size() ? snap[0][i] : 0u, i < snap[1].size() ? snap[1][i] : 0u);
                    for (int s = 0; s < 2; ++s)  // a guarded call that did not end normally: say how (1 = hung, else the code)
                        if (snap[s].size() == 2 && snap[s][0] == 0xFA017000u)
                            std::printf("    %s side ended abnormally: %08x (iteration %d)\n", s ? "port" : "original", snap[s][1], it);
                }
                ++bad;
            }
            ++total;
        }
        if (bad) ++failed;
        // every function it ran, 0 for a match: tools/verify_batch.py takes the line as the evidence the function was exercised
        std::printf("  %-44s %d of %d differ\n", k.name, bad, k.iters);
    }
    vt::norm() = [](std::uint32_t v) { return v; };
    rt::restore_pristine();
    std::printf("  %s: %d calls, %d functions differ\n", what, total, failed);
    return failed;
}
}  // namespace vtr
