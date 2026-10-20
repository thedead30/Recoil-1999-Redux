// Native L1, composite: the Weiler-Atherton clipper driven end to end through its entry points
// (WeilerClip_Create 0x00464680, WeilerClip_Run 0x00464810, WeilerClip_ResetRegion 0x00464790,
// WeilerClip_Destroy 0x004647d0) on the ORIGINAL and on the port, with random polygons on a coarse grid (so
// shared vertices, collinear and coincident edges occur often). After every call the two heap graphs are
// compared deeply (tests/heap_graph.h), including the caller's arrays and the result block.
//
// Coverage (build.bat Coverage): the port's entry counters show which clipper functions ran in iterations whose
// whole final state matched the original's. The port is an instruction-level copy, so a function that ran there
// under a matching end state ran in the original on the same input too; iterations that hung or faulted add nothing.
#include "test.h"
#include "native_oracle.h"
#include "heap_graph.h"
#include "watchdog.h"
#include "GameZRecoil/zGeometry/zgeo_weiler.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <random>

namespace {
std::mt19937 wrng(0xC11B);
int wu(int n) { return static_cast<int>(wrng() % static_cast<unsigned>(n)); }
float coord(bool grid)
{
    if (grid) return static_cast<float>(wu(7)) * 10.0f;
    return std::uniform_real_distribution<float>(-50, 50)(wrng);
}
void polygon(float* p, int n, bool grid, int shape)
{
    if (shape == 0) {  // convex-ish: points on a circle (grid-rounded when grid)
        const float cx = coord(grid), cy = coord(grid), r = 10.0f + static_cast<float>(wu(4)) * 10.0f;
        for (int i = 0; i < n; ++i) {
            const float a = 6.2831853f * static_cast<float>(i) / static_cast<float>(n);
            float x = cx + r * std::cos(a), y = cy + r * std::sin(a);
            if (grid) { x = std::round(x / 10.0f) * 10.0f; y = std::round(y / 10.0f) * 10.0f; }
            p[3 * i] = x; p[3 * i + 1] = y; p[3 * i + 2] = coord(false) * 0.1f;
        }
    } else {
        for (int i = 0; i < n; ++i) { p[3 * i] = coord(grid); p[3 * i + 1] = coord(grid); p[3 * i + 2] = coord(false) * 0.1f; }
    }
}
using CreateFn = int(__fastcall*)(int, int, int);
using RunFn = int(__fastcall*)(int, int, int, int, int);
using ResetFn = int(__fastcall*)(int, int, int);
using DestroyFn = void(__fastcall*)(void*);
int I(const void* p) { return static_cast<int>(reinterpret_cast<std::uintptr_t>(p)); }

// same order as recoil::g_WeilerEntryCounts (zgeo_weiler.h)
const std::uint32_t kWeilerEntries[recoil::kWeilerEntryCountSlots] = {
    0x00464680, 0x00464790, 0x00464810, 0x00464b90, 0x00464c90, 0x00464ea0, 0x00464f70, 0x00465ac0, 0x004676c0, 0x00467710,
    0x004680b0, 0x004681a0, 0x004682c0, 0x00468470, 0x00468580, 0x00468650, 0x004687b0, 0x00468c40, 0x00468fa0, 0x004693c0,
    0x00469450, 0x00469560, 0x00469960, 0x00469a30, 0x00469af0, 0x00469b60, 0x00469d60, 0x0046a130, 0x0046a1f0};
}  // namespace

TEST(native_weiler_clip_end_to_end_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    auto oCreate = rt::original<CreateFn>(0x00464680);
    auto oRun = rt::original<RunFn>(0x00464810);
    auto oReset = rt::original<ResetFn>(0x00464790);
    auto oDestroy = rt::original<DestroyFn>(0x004647d0);
    int bad = 0, runs = 0, hangs = 0, faults = 0, inconclusive = 0;
    unsigned credited[recoil::kWeilerEntryCountSlots] = {};  // entries in iterations that ended matching
    std::string first;
    {
        hg::Recording rec;
        const int iters = std::getenv("RECOIL_ITERS") ? std::atoi(std::getenv("RECOIL_ITERS")) : 3000;
        for (int n = 0; n < iters && !bad; ++n) {
            if (std::getenv("RECOIL_VERBOSE")) { std::printf("  iter %d\n", n); std::fflush(stdout); }
#ifdef RECOIL_ENTRY_COUNTS
            unsigned before[recoil::kWeilerEntryCountSlots];
            std::memcpy(before, recoil::g_WeilerEntryCounts, sizeof before);
#endif
            const bool grid = wu(3) != 0;
            const int rn = 3 + wu(6), sn = 3 + wu(6);
            // zeroed: an uninitialised word could look like a pointer into one side's arrays
            float regO[3 * 8] = {}, regP[3 * 8] = {}, subO[3 * 8] = {}, subP[3 * 8] = {}, reg2O[3 * 8] = {}, reg2P[3 * 8] = {};
            polygon(regO, rn, grid, wu(2));
            if (wu(8) == 0) std::memcpy(subO, regO, sizeof subO); else polygon(subO, sn, grid, wu(2));
            polygon(reg2O, rn, grid, wu(2));
            if (wu(8) == 0) {  // far from the origin (beyond +-65536): the clipper translates to the origin and back
                const float ox = 100000.0f + static_cast<float>(wu(4)) * 50000.0f, oy = -120000.0f;
                for (float* a : {regO, subO, reg2O})
                    for (int i = 0; i < 8; ++i) { a[3 * i] += ox; a[3 * i + 1] += oy; }
            }
            std::memcpy(regP, regO, sizeof regO); std::memcpy(subP, subO, sizeof subO); std::memcpy(reg2P, reg2O, sizeof reg2O);
            std::uint32_t resO[8] = {}, resP[8] = {};
            void* holderO[1] = {};
            void* holderP[1] = {};
            hg::Log O, P;
            // caller-owned regions take fixed roles on both sides
            O.add(regO, 1, sizeof regO); P.add(regP, 1, sizeof regP);
            O.add(subO, 2, sizeof subO); P.add(subP, 2, sizeof subP);
            O.add(resO, 3, sizeof resO); P.add(resP, 3, sizeof resP);
            O.add(reg2O, 4, sizeof reg2O); P.add(reg2P, 4, sizeof reg2P);
            O.add(holderO, 5, sizeof holderO); P.add(holderP, 5, sizeof holderP);
            const int axis = wu(4) == 0 ? 1 + wu(2) : 0;
            const int mode = wu(4);
            auto step = [&](const char* what) {
                const std::string d = hg::compare(O, P);
                if (!d.empty() && !bad) { ++bad; first = std::string(what) + " #" + std::to_string(n) + ": " + d; }
            };
            // Run one call on each side under the guard. Both sides must end the same way (normally, hung, or
            // with the same exception); an abnormal ending on both abandons the iteration (its state leaks).
            auto both = [&](const char* what, auto fo, auto fp) -> bool {
                hg::current() = &O; const unsigned a = wd::guarded(fo);
                hg::current() = &P; const unsigned b = wd::guarded(fp);
                // hung on one side, faulted on the other: a runaway that the wall-clock limit cut at different
                // points (a loaded machine preempts one side) - inconclusive, as in the arena fuzz
                if (a != b && (a == wd::kHung || b == wd::kHung) && a != wd::kOk && b != wd::kOk) { ++inconclusive; return false; }
                if (a != b) {
                    ++bad;
                    char msg[96];
                    std::snprintf(msg, sizeof msg, "%s #%d: original ended 0x%x, port 0x%x", what, n, a, b);
                    first = msg;
                    return false;
                }
                if (a == wd::kHung) { ++hangs; return false; }
                if (a != wd::kOk) { ++faults; return false; }
                return true;
            };
            int cO = 0, cP = 0;
            if (!both("create", [&] { cO = oCreate(I(regO), rn, axis); }, [&] { cP = recoil::WeilerClip_Create(I(regP), rn, axis); })) continue;
            if ((cO == 0) != (cP == 0)) { ++bad; first = "create result"; break; }
            step("create");
            if (!cO) continue;
            holderO[0] = reinterpret_cast<void*>(static_cast<std::uintptr_t>(cO));
            holderP[0] = reinterpret_cast<void*>(static_cast<std::uintptr_t>(cP));
            const int m = wu(20) == 0 ? 0 : sn;  // a zero count takes the bad-parameter path
            int rO = 0, rP = 0;
            ++runs;
            if (!both("run", [&] { rO = oRun(cO, mode, I(subO), m, I(resO)); }, [&] { rP = recoil::WeilerClip_Run(cP, mode, I(subP), m, I(resP)); })) continue;
            if (rO != rP) { ++bad; first = "run result " + std::to_string(rO) + " vs " + std::to_string(rP) + " #" + std::to_string(n); break; }
            step("run");
            if (wu(3) == 0) {
                int eO = 0, eP = 0;
                if (!both("reset", [&] { eO = oReset(I(holderO), I(reg2O), rn); }, [&] { eP = recoil::WeilerClip_ResetRegion(I(holderP), I(reg2P), rn); })) continue;
                if (eO != eP) { ++bad; first = "reset result"; break; }
                step("reset");
                if (eO) {
                    std::uint32_t r2O[8] = {}, r2P[8] = {};
                    O.add(r2O, 6, sizeof r2O); P.add(r2P, 6, sizeof r2P);
                    int sO = 0, sP = 0;
                    ++runs;
                    if (!both("rerun", [&] { sO = oRun(I(holderO[0]), mode, I(subO), sn, I(r2O)); },
                              [&] { sP = recoil::WeilerClip_Run(I(holderP[0]), mode, I(subP), sn, I(r2P)); })) continue;
                    if (sO != sP) { ++bad; first = "rerun result"; break; }
                    step("rerun");
                    O.live.erase(reinterpret_cast<std::uintptr_t>(r2O)); P.live.erase(reinterpret_cast<std::uintptr_t>(r2P));
                }
            }
            if (!both("destroy", [&] { oDestroy(holderO[0]); }, [&] { recoil::WeilerClip_Destroy(holderP[0]); })) continue;
            step("destroy");
            // release anything the calls left allocated (none expected once the clip is destroyed)
            for (hg::Log* L : {&O, &P})
                for (auto& [p, blk] : L->live)
                    if (blk.role >= 1000) hg::real().free_(reinterpret_cast<void*>(p));
#ifdef RECOIL_ENTRY_COUNTS
            if (!bad)  // reached only when every call of this iteration ended normally and matched
                for (int i = 0; i < recoil::kWeilerEntryCountSlots; ++i) credited[i] += recoil::g_WeilerEntryCounts[i] - before[i];
#endif
        }
    }
    if (bad) std::printf("  first difference: %s\n", first.c_str());
    std::printf("  %d runs, %d hung on both sides (abandoned after 50 ms), %d faulted identically on both sides, %d hung/faulted (inconclusive)\n", runs, hangs, faults, inconclusive);
#ifdef RECOIL_ENTRY_COUNTS
    int unhit = 0;
    std::printf("  coverage (entries in matching iterations):");
    for (int i = 0; i < recoil::kWeilerEntryCountSlots; ++i) {
        std::printf(" %06x=%u", kWeilerEntries[i] & 0xFFFFFF, credited[i]);
        if (!credited[i]) ++unhit;
    }
    std::printf("\n  unexercised: %d\n", unhit);
#else
    (void)credited;
    (void)kWeilerEntries;
#endif
    CHECK_EQ(bad, 0);
}
