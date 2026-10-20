// Native L1 for the geometry point/segment/triangle helpers, the triangle list and the Newell plane: port vs
// the ORIGINAL bytes on the real x87 (tests/native_oracle.h). Globals are seeded equal on both sides (the
// original's in the mapped image) and compared after each call.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zGeometry/zgeo_model.h"
#include "GameZRecoil/zGeometry/zgeo_weiler.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <random>

namespace {
std::mt19937 prng(0x9017);
float pf(float lo, float hi)
{
    const float e[] = {0.0f, -0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), 1e-40f, 1.0f, 0.5f};
    if (prng() % 12 == 0) return e[prng() % 7];
    return std::uniform_real_distribution<float>(lo, hi)(prng);
}
std::uint32_t pu(std::uint32_t n) { return static_cast<std::uint32_t>(prng() % n); }
bool same(const void* a, const void* b, std::size_t n) { return std::memcmp(a, b, n) == 0; }
std::uint32_t Pp(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
constexpr std::uintptr_t kOrigTriList = 0x0053a750, kOrigTriCount = 0x0053d750, kOrigPlane = 0x0053d758;
}  // namespace

TEST(native_geometry_point_helpers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using InPolyFn = int(__fastcall*)(const float*, int, const float*);
    using RangeFn = int(__fastcall*)(const float*, const float*, const float*);
    using SnapFn = int(__fastcall*)(const float*, const float*, float*, float);
    using DupFn = int(__fastcall*)(float*, unsigned);
    using RectFn = int(__fastcall*)(const float*, const float*);
    using GradFn = void(__fastcall*)(const float*, const float*, const float*, float, float, float, float*);
    using FindFn = int(__fastcall*)(const void*, const float*);
    using SegFn = int(__fastcall*)(const float*, const float*, const float*, const float*);
    using CcwFn = int(__fastcall*)(int, float*, int);
    auto oIn = rt::original<InPolyFn>(0x00468a10);
    auto oRange = rt::original<RangeFn>(0x00469ca0);
    auto oSnap = rt::original<SnapFn>(0x00469e90);
    auto oDup = rt::original<DupFn>(0x0046a080);
    auto oRect = rt::original<RectFn>(0x0046a620);
    auto oGrad = rt::original<GradFn>(0x0046a8e0);
    auto oFind = rt::original<FindFn>(0x0046ab40);
    auto oSeg = rt::original<SegFn>(0x0046be20);
    auto oCcw = rt::original<CcwFn>(0x0046c620);
    int bad[9] = {};
    for (int n = 0; n < 5000; ++n) {
        float poly[3 * 8];
        for (float& x : poly) x = pf(-10, 10);
        const int count = static_cast<int>(pu(9));
        float pt[3] = {pf(-10, 10), pf(-10, 10), pf(-10, 10)};
        if (n % 9 == 0 && count) std::memcpy(pt, &poly[3 * pu(count)], 12);          // on a vertex
        if (n % 11 == 0) pt[1] = poly[1];                                          // on an edge's y
        if (oIn(pt, count, poly) != recoil::Point2_InPolygonQuadrant(pt, count, poly)) ++bad[0];

        const float* a = &poly[0];
        const float* bb = &poly[3];
        float c[3];
        std::memcpy(c, pt, 12);
        if (n % 5 == 0) c[0] = a[0];
        if (oRange(c, a, bb) != recoil::Point2_OnSegmentRange(c, a, bb)) ++bad[1];

        {   float p1[3], p2[3];
            const float t = pf(-0.2f, 1.2f), eps = pf(0, 0.1f);
            float sa[3], sb[3];
            std::memcpy(sa, a, 12); std::memcpy(sb, bb, 12);
            if (n % 7 == 0) sb[1] = sa[1] + pf(-0.01f, 0.01f);   // near-horizontal
            if (n % 7 == 1) sb[0] = sa[0] + pf(-0.01f, 0.01f);   // near-vertical
            p1[0] = sa[0] + t * (sb[0] - sa[0]) + pf(-0.05f, 0.05f);
            p1[1] = sa[1] + t * (sb[1] - sa[1]) + pf(-0.05f, 0.05f);
            p1[2] = pf(-1, 1);
            std::memcpy(p2, p1, 12);
            const int r1 = oSnap(sa, sb, p1, eps), r2 = recoil::Point2_SnapOntoSegment(sa, sb, p2, eps);
            if (r1 != r2 || !same(p1, p2, 12)) ++bad[2];
        }
        {   float d1[3 * 8], d2[3 * 8];
            for (int i = 0; i < 24; ++i) d1[i] = poly[i];
            for (int i = 1; i < 8; ++i) if (pu(3) == 0) { d1[3 * i] = d1[3 * i - 3] + pf(-0.02f, 0.02f); d1[3 * i + 1] = d1[3 * i - 2]; }
            std::memcpy(d2, d1, sizeof d1);
            const unsigned cnt = pu(9);
            if (oDup(d1, cnt) != recoil::PointArray_RemoveAdjacentDuplicates(d2, cnt) || !same(d1, d2, sizeof d1)) ++bad[3];
        }
        {   float r1[4] = {pf(-5, 5), pf(-5, 5), pf(-5, 5), pf(-5, 5)}, r2[4] = {pf(-5, 5), pf(-5, 5), pf(-5, 5), pf(-5, 5)};
            if (oRect(r1, r2) != recoil::Rect_OverlapMargin1(r1, r2)) ++bad[4];
        }
        {   float o1[2] = {3, 3}, o2[2] = {3, 3};
            const float v1 = pf(-9, 9), v0 = pf(-9, 9), v2 = pf(-9, 9);
            float q[3];
            std::memcpy(q, &poly[6], 12);
            if (n % 13 == 0) std::memcpy(q, a, 12);   // degenerate: det 0
            oGrad(bb, a, q, v1, v0, v2, o1); recoil::Triangle_SolveGradientXZ(bb, a, q, v1, v0, v2, o2);
            if (!same(o1, o2, 8)) ++bad[5];
        }
        {   const std::uint32_t arr[3] = {prng(), Pp(poly), static_cast<std::uint32_t>(count)};
            if (oFind(arr, pt) != recoil::PointArray_Find2D(arr, pt)) ++bad[6];
        }
        if (oSeg(a, bb, &poly[6], &poly[9]) != recoil::Segment2_IntersectStrict(a, bb, &poly[6], &poly[9])) ++bad[7];
        {   float t1[9], t2[9];
            std::memcpy(t1, poly, 36); std::memcpy(t2, poly, 36);
            const int fix = static_cast<int>(pu(2));
            if (oCcw(0, t1, fix) != recoil::Triangle_EnsureCounterClockwise(0, t2, fix) || !same(t1, t2, 36)) ++bad[8];
        }
    }
    for (int i = 0; i < 9; ++i) CHECK_EQ(bad[i], 0);
}

TEST(native_geometry_snap_to_reference_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    // 0x0046a130 (ECX reference xyz ring, EDX ring count, stack points, point count, vertex eps, edge eps; ret 0x10)
    using SnapRefFn = int(__fastcall*)(int, int, int, int, int, int);
    auto oSnapRef = rt::original<SnapRefFn>(0x0046a130);
    auto bits = [](float f) { int b; std::memcpy(&b, &f, 4); return b; };
    int bad = 0, snapped = 0;
    for (int n = 0; n < 5000; ++n) {
        float ref[3 * 6];
        const int rc = static_cast<int>(pu(7));  // 0 exercises the empty ring
        for (int i = 0; i < 18; ++i) ref[i] = static_cast<float>(pu(9)) * 5.0f;
        float p1[3 * 6], p2[3 * 6];
        const int pc = static_cast<int>(pu(7));
        for (int i = 0; i < 6; ++i) {
            const int k = rc ? static_cast<int>(pu(rc)) : 0;
            const int k2 = rc ? (k + 1) % rc : 0;
            const float t = pf(0, 1);
            switch (pu(3)) {
            case 0: for (int c = 0; c < 3; ++c) p1[3 * i + c] = ref[3 * k + c] + pf(-0.02f, 0.02f); break;   // near a vertex
            case 1: for (int c = 0; c < 3; ++c) p1[3 * i + c] = ref[3 * k + c] + t * (ref[3 * k2 + c] - ref[3 * k + c]) + pf(-0.05f, 0.05f); break;  // near an edge
            default: for (int c = 0; c < 3; ++c) p1[3 * i + c] = pf(-5, 50); break;
            }
        }
        std::memcpy(p2, p1, sizeof p1);
        const float e1 = pf(0, 0.05f), e2 = pf(0, 0.2f);
        const auto I = [](const void* p) { return static_cast<int>(reinterpret_cast<std::uintptr_t>(p)); };
        const int r1 = oSnapRef(I(ref), rc, I(p1), pc, bits(e1), bits(e2));
        const int r2 = recoil::PointArray_SnapToReference(I(ref), rc, I(p2), pc, bits(e1), bits(e2));
        if (r1 != r2 || !same(p1, p2, sizeof p1)) ++bad;
        snapped += r1 != 0;
    }
    CHECK_EQ(bad, 0);
    CHECK(snapped > 500);  // the snapping paths are really taken
}

TEST(native_geometry_triangle_list_and_plane_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using EarFn = int(__fastcall*)(int, int, int, int, void*);
    using NewellFn = void(__fastcall*)(int, const float*, float*);
    using NewellGFn = void(__fastcall*)(int, const float*);
    using SolveFn = void(__fastcall*)(int, float*);
    auto oEar = rt::original<EarFn>(0x0046bfc0);
    auto oNewell = rt::original<NewellFn>(0x0046c3a0);
    auto oNewellG = rt::original<NewellGFn>(0x0046c390);
    auto oSolve = rt::original<SolveFn>(0x0046c570);
    auto* oList = reinterpret_cast<std::uint32_t*>(kOrigTriList);
    auto* oCount = reinterpret_cast<std::uint32_t*>(kOrigTriCount);
    auto* oPlane = reinterpret_cast<float*>(kOrigPlane);
    int bad[4] = {};
    for (int n = 0; n < 5000; ++n) {
        {   // EmitEar: edges are (v0, v1, use count) triples over vertices 0..5
            std::uint32_t e1[3 * 8], e2[3 * 8];
            for (int i = 0; i < 8; ++i) { e1[3 * i] = pu(6); e1[3 * i + 1] = pu(6); e1[3 * i + 2] = pu(3); }
            std::memcpy(e2, e1, sizeof e1);
            const std::uint32_t start = pu(1000);
            *oCount = start; recoil::g_TriangleCount_0053d750 = start;
            for (int i = 0; i < 3 * 8; ++i) oList[3 * start + i] = recoil::g_TriangleList_0053a750[3 * start + i] = 0xEEEEEEEEu;
            const int ea = static_cast<int>(pu(8)), eb = static_cast<int>(pu(8)), apex = static_cast<int>(pu(6)), cnt = static_cast<int>(pu(9));
            const int r1 = oEar(ea, eb, apex, cnt, e1), r2 = recoil::Triangulate_EmitEar(ea, eb, apex, cnt, e2);
            if (r1 != r2 || !same(e1, e2, sizeof e1) || *oCount != recoil::g_TriangleCount_0053d750
                || !same(&oList[3 * start], &recoil::g_TriangleList_0053a750[3 * start], 4 * 3 * 8)) ++bad[0];
        }
        float poly[3 * 8];
        for (float& x : poly) x = pf(-10, 10);
        const int count = static_cast<int>(pu(9));
        if (n % 17 == 0) for (int i = 0; i < 24; ++i) poly[i] = static_cast<float>(i % 3);   // collinear: zero normal
        {   float p1[4] = {9, 9, 9, 9}, p2[4] = {9, 9, 9, 9};
            oNewell(count, poly, p1); recoil::Polygon_NewellPlaneFastSqrt(count, poly, p2);
            if (!same(p1, p2, 16)) ++bad[1];
        }
        {   oNewellG(count, poly); recoil::Polygon_NewellPlaneToGlobal(count, poly);
            if (!same(oPlane, recoil::g_NewellPlane_0053d758, 16)) ++bad[2];
        }
        {   for (int i = 0; i < 4; ++i) { const float v = pf(-3, 3); oPlane[i] = v; recoil::g_NewellPlane_0053d758[i] = v; }
            float z1[3 * 8], z2[3 * 8];
            std::memcpy(z1, poly, sizeof z1); std::memcpy(z2, poly, sizeof z2);
            const int c = static_cast<int>(pu(9)) - (n % 23 == 0 ? 3 : 0);
            oSolve(c, z1); recoil::Plane_SolveZForPoints(c, z2);
            if (!same(z1, z2, sizeof z1)) ++bad[3];
        }
    }
    for (int i = 0; i < 4; ++i) CHECK_EQ(bad[i], 0);
}
