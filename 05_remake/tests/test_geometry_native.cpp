// Native L1 for geometry leaves: port vs the ORIGINAL bytes on the real x87 (tests/native_oracle.h). The
// clipper structures are fed as raw words at the offsets the listings use; where a field is a pointer, each
// side gets its own copy of the pointed-to data, and both the structures and those copies are compared.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zGeometry/zgeo_weiler.h"
#include "unattributed/geometry.h"
#include "unattributed/ui_widgets.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <random>

namespace {
std::mt19937 grng(0x6E0);
float gf(float lo, float hi)
{
    const float e[] = {0.0f, -0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), 1e-40f, 1.0f};
    if (grng() % 10 == 0) return e[grng() % 6];
    return std::uniform_real_distribution<float>(lo, hi)(grng);
}
std::uint32_t fb(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
void put(void* base, int off, std::uint32_t v) { std::memcpy(static_cast<char*>(base) + off, &v, 4); }
void put_ptr(void* base, int off, const void* p) { put(base, off, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p))); }
}  // namespace

TEST(native_geometry_weiler_leaves_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using GetOut = int(__fastcall*)(const void*, int*);
    using P1 = void(__fastcall*)(void*);
    using PI = void(__fastcall*)(void*, int);
    using Near = int(__fastcall*)(const float*, const float*, float);
    using Rot = void(__fastcall*)(int, float*);
    auto oGet = rt::original<GetOut>(0x00464670);
    auto oBounds = rt::original<P1>(0x00468410);
    auto oSwap = rt::original<P1>(0x004683a0);
    auto oEach = rt::original<PI>(0x004693a0);
    auto oOrient = rt::original<P1>(0x00469430);
    auto oEval = rt::original<PI>(0x00469ae0);
    auto oNear = rt::original<Near>(0x00469e50);
    auto oRx = rt::original<Rot>(0x0046a5e0);
    auto oRxi = rt::original<Rot>(0x0046a600);
    int bad[9] = {};
    for (int n = 0; n < 4000; ++n) {
        {   // GetOutput: null and a random blob
            std::uint32_t s[8];
            for (auto& w : s) w = grng();
            int o1 = 7, o2 = 7;
            const void* p = (n % 10 == 0) ? nullptr : s;
            if (oGet(p, &o1) != recoil::WeilerClip_GetOutput(p, &o2) || o1 != o2) ++bad[0];
        }
        {   // ComputeBounds: edge with endpoint pointers at +0xc / +0x10 (each side its own endpoints)
            float a1[2], b1[2], a2[2], b2[2];
            for (int i = 0; i < 2; ++i) { a1[i] = a2[i] = gf(-100, 100); b1[i] = b2[i] = gf(-100, 100); }
            if (n % 7 == 0) std::memcpy(b1, a1, 8), std::memcpy(b2, a1, 8);
            std::uint32_t e1[16], e2[16];
            for (int i = 0; i < 16; ++i) e1[i] = e2[i] = grng();
            put_ptr(e1, 0xc, a1); put_ptr(e1, 0x10, b1); put_ptr(e2, 0xc, a2); put_ptr(e2, 0x10, b2);
            oBounds(e1); recoil::ClipEdge_ComputeBounds(e2);
            e1[3] = e2[3] = 0; e1[4] = e2[4] = 0;  // pointer fields differ by construction
            if (std::memcmp(e1, e2, sizeof e1) || std::memcmp(a1, a2, 8) || std::memcmp(b1, b2, 8)) ++bad[1];
        }
        {   // SwapAxes: [+0x4] mode, [+0x14]/[+0x18] count/points, [+0x28]/[+0x2c] count/points (used if non-null)
            float p1[3 * 6], p2[3 * 6], q1[3 * 6], q2[3 * 6];
            for (int i = 0; i < 18; ++i) { p1[i] = p2[i] = gf(-10, 10); q1[i] = q2[i] = gf(-10, 10); }
            std::uint32_t c1[12] = {}, c2[12] = {};
            const std::uint32_t mode = grng() % 4, cnt = grng() % 7, cnt2 = grng() % 7;
            const bool use2 = grng() % 2;
            c1[1] = c2[1] = mode; c1[5] = c2[5] = cnt; c1[10] = c2[10] = cnt2;
            put_ptr(c1, 0x18, p1); put_ptr(c2, 0x18, p2);
            if (use2) { put_ptr(c1, 0x2c, q1); put_ptr(c2, 0x2c, q2); }
            oSwap(c1); recoil::ClipContour_SwapAxes(c2);
            if (std::memcmp(p1, p2, sizeof p1) || std::memcmp(q1, q2, sizeof q1)) ++bad[2];
        }
        {   // ClipArray: count edges of 0x3c bytes
            constexpr int kN = 5;
            float pts1[kN][4], pts2[kN][4];
            std::uint32_t arr1[kN][15], arr2[kN][15];
            for (int k = 0; k < kN; ++k) {
                for (int i = 0; i < 4; ++i) pts1[k][i] = pts2[k][i] = gf(-50, 50);
                for (int i = 0; i < 15; ++i) arr1[k][i] = arr2[k][i] = grng();
                put_ptr(arr1[k], 0xc, &pts1[k][0]); put_ptr(arr1[k], 0x10, &pts1[k][2]);
                put_ptr(arr2[k], 0xc, &pts2[k][0]); put_ptr(arr2[k], 0x10, &pts2[k][2]);
            }
            const int count = static_cast<int>(grng() % (kN + 1));
            oEach(arr1, count); recoil::ClipArray_Call468410Each(arr2, count);
            for (int k = 0; k < kN; ++k) { arr1[k][3] = arr2[k][3] = arr1[k][4] = arr2[k][4] = 0; }
            if (std::memcmp(arr1, arr2, sizeof arr1)) ++bad[3];
        }
        {   // OrientFirst: edge [+0x4] -> owner; owner [+0x4] == edge triggers the swap
            std::uint32_t own1[6], own2[6], ed1[4], ed2[4];
            for (int i = 0; i < 6; ++i) own1[i] = own2[i] = grng();
            for (int i = 0; i < 4; ++i) ed1[i] = ed2[i] = grng();
            put_ptr(ed1, 4, own1); put_ptr(ed2, 4, own2);
            const bool link = grng() % 2;
            if (link) { put_ptr(own1, 4, ed1); put_ptr(own2, 4, ed2); }
            oOrient(ed1); recoil::ClipEdge_OrientFirst(ed2);
            // compare with the self-pointers normalised to a marker
            auto norm = [](std::uint32_t* o, const void* self) { for (int i = 0; i < 6; ++i) if (o[i] == static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(self))) o[i] = 0xEDEDEDED; };
            norm(own1, ed1); norm(own2, ed2);
            ed1[1] = ed2[1] = 0;
            if (std::memcmp(own1, own2, sizeof own1) || std::memcmp(ed1, ed2, sizeof ed1)) ++bad[4];
        }
        {   std::uint32_t l1[5], l2[5];
            for (int i = 0; i < 5; ++i) l1[i] = l2[i] = grng();
            const int t = static_cast<int>(grng());
            oEval(l1, t); recoil::IntLine_EvalAt(l2, t);
            if (std::memcmp(l1, l2, sizeof l1)) ++bad[5];
        }
        {   const float a[2] = {gf(-5, 5), gf(-5, 5)}, b[2] = {gf(-5, 5), gf(-5, 5)};
            const float eps = gf(0, 3);
            if (oNear(a, b, eps) != recoil::Point2_NearlyEqual(a, b, eps)) ++bad[6];
        }
        {   float v1[18], v2[18];
            for (int i = 0; i < 18; ++i) v1[i] = v2[i] = gf(-9, 9);
            const int c = static_cast<int>(grng() % 7);
            if (n % 2) { oRx(c, v1); recoil::Vec3Array_RotateX90(c, v2); } else { oRxi(c, v1); recoil::Vec3Array_RotateX90Inverse(c, v2); }
            if (std::memcmp(v1, v2, sizeof v1)) ++bad[7 + n % 2];
        }
    }
    for (int i = 0; i < 9; ++i) CHECK_EQ(bad[i], 0);
}

namespace {
// Call f like the original's callers do (cdecl, caller pops three arguments) with known EAX/ECX/EDX/flags;
// return what EAX, ECX, EDX, ESP and EFLAGS are afterwards.
struct RegsOut { std::uint32_t eax, ecx, edx, esp_delta, eflags; };
RegsOut call_noop(const void* f, std::uint32_t seed)
{
    RegsOut r{};
    std::uint32_t esp0 = 0, esp1 = 0, a = 0, c = 0, d = 0, fl = 0;
    __asm {
        mov esp0, esp
        push 0x11
        push 0x22
        push 0x33
        mov eax, seed
        mov ecx, eax
        not ecx
        mov edx, eax
        rol edx, 7
        cmp eax, ecx
        call f
        pushfd
        pop fl
        add esp, 12
        mov a, eax
        mov c, ecx
        mov d, edx
        mov esp1, esp
    }
    r.eax = a; r.ecx = c; r.edx = d; r.esp_delta = esp0 - esp1; r.eflags = fl & 0x8D5;  // CF PF AF ZF SF OF
    return r;
}
}  // namespace

TEST(native_debug_report_noop_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    int bad = 0;
    for (std::uint32_t s = 1; s < 5000; s = s * 7 + 13) {
        const RegsOut o = call_noop(reinterpret_cast<const void*>(0x00404e80), s);
        const RegsOut p = call_noop(reinterpret_cast<const void*>(&recoil::Debug_ReportNoop), s);
        if (std::memcmp(&o, &p, sizeof o)) ++bad;
    }
    CHECK_EQ(bad, 0);
}

TEST(native_geometry_bfe_tolerance_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using SetF = void(__stdcall*)(float);
    auto oSet = rt::original<SetF>(0x00476460);
    auto oGet = rt::original<long double(__cdecl*)()>(0x00476470);
    // load-time value is the same on both sides
    CHECK_EQ(*reinterpret_cast<std::uint32_t*>(0x004e0fc0), fb(recoil::g_BFETolerance_004e0fc0));
    int bad = 0;
    for (int n = 0; n < 1000; ++n) {
        const float v = gf(-1, 1);
        oSet(v); recoil::Geom_SetBFETolerance(v);
        const double g1 = static_cast<double>(oGet()), g2 = static_cast<double>(recoil::Geom_GetBFETolerance());
        if (*reinterpret_cast<std::uint32_t*>(0x004e0fc0) != fb(recoil::g_BFETolerance_004e0fc0) || std::memcmp(&g1, &g2, 8)) ++bad;
    }
    CHECK_EQ(bad, 0);
}
