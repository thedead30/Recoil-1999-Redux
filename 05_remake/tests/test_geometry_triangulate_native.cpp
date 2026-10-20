// Native L1 for the clip-polygon helpers and the triangulation chain (zgeo_model / zgeo_convexify): each function
// is called directly on the ORIGINAL and on the port with the same random input. Heap results are compared as
// graphs (tests/heap_graph.h, which also records malloc/realloc/free/fprintf), globals are seeded equal on both
// sides and compared, and every call runs under the watchdog (tests/watchdog.h) so a hang or fault on either
// side is reported as an outcome to compare instead of stopping the run.
#include "test.h"
#include "native_oracle.h"
#include "heap_graph.h"
#include "watchdog.h"
#include "GameZRecoil/zGeometry/zgeo_convexify.h"
#include "GameZRecoil/zGeometry/zgeo_model.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <random>
#include <string>

namespace {
std::mt19937 trng(0x7A1A);
int tu(int n) { return static_cast<int>(trng() % static_cast<unsigned>(n)); }
float tf(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(trng); }
int I(const void* p) { return static_cast<int>(reinterpret_cast<std::uintptr_t>(p)); }
using F2 = int(__fastcall*)(int, int);
using F3 = int(__fastcall*)(int, int, int);
using F4 = int(__fastcall*)(int, int, int, int);
using F5 = int(__fastcall*)(int, int, int, int, int);

// A simple polygon: n points around a centre at increasing angles (star-shaped, so never self-intersecting),
// on a coarse grid half of the time so collinear and repeated coordinates occur.
void ring(float* p, int n, bool grid)
{
    float angles[16];
    for (int i = 0; i < n; ++i) angles[i] = tf(0, 6.2831853f);
    std::sort(angles, angles + n);
    const float cx = tf(-20, 20), cy = tf(-20, 20);
    for (int i = 0; i < n; ++i) {
        const float r = tf(5, 30);
        float x = cx + r * std::cos(angles[i]), y = cy + r * std::sin(angles[i]);
        if (grid) { x = std::round(x / 5.0f) * 5.0f; y = std::round(y / 5.0f) * 5.0f; }
        p[3 * i] = x; p[3 * i + 1] = y; p[3 * i + 2] = tf(-1, 1);
    }
}

constexpr std::uintptr_t kOrigVerts = 0x0053a748, kOrigTriList = 0x0053a750, kOrigTriCount = 0x0053d750,
                         kOrigWord = 0x0053d754, kOrigPlane = 0x0053d758;
template <class T> T& at(std::uintptr_t va) { return *reinterpret_cast<T*>(va); }

struct Pair {
    hg::Log O, P;
    int bad = 0, abnormal = 0;
    std::string first;
    // Run both calls under the guard; the outcomes (normal / hung / exception code) must agree.
    template <class FO, class FP>
    bool both(const char* what, int n, FO fo, FP fp)
    {
        hg::current() = &O; const unsigned a = wd::guarded(fo);
        hg::current() = &P; const unsigned b = wd::guarded(fp);
        if (a != b) { fail(what, n, "outcome 0x" + std::to_string(a) + " vs 0x" + std::to_string(b)); return false; }
        if (a != wd::kOk) { ++abnormal; return false; }
        return true;
    }
    void fail(const char* what, int n, const std::string& d)
    {
        if (!bad++) first = std::string(what) + " #" + std::to_string(n) + ": " + d;
    }
    void compare(const char* what, int n)
    {
        const std::string d = hg::compare(O, P);
        if (!d.empty()) fail(what, n, d);
    }
    void release()
    {
        for (hg::Log* L : {&O, &P})
            for (auto& [p, blk] : L->live)
                if (blk.role >= 1000) hg::real().free_(reinterpret_cast<void*>(p));
    }
};
}  // namespace

TEST(native_geometry_clip_polygon_helpers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    auto oCreate = rt::original<F2>(0x0046aa40);
    auto oFindEdge = rt::original<F2>(0x0046ac80);
    auto oMerge = rt::original<F3>(0x0046ab90);
    hg::Recording rec;
    int bad = 0, abnormal = 0, found = 0, merged = 0;
    std::string first;
    for (int n = 0; n < 3000 && !bad; ++n) {
        Pair s;
        float pts[3 * 12] = {};
        const int count = 3 + tu(8);
        ring(pts, count, tu(2) == 0);
        float ptsO[3 * 12], ptsP[3 * 12];
        std::memcpy(ptsO, pts, sizeof pts); std::memcpy(ptsP, pts, sizeof pts);
        s.O.add(ptsO, 1, sizeof ptsO); s.P.add(ptsP, 1, sizeof ptsP);
        int pO = 0, pP = 0;
        if (s.both("create", n, [&] { pO = oCreate(count, I(ptsO)); }, [&] { pP = recoil::ClipPolygon_Create(count, I(ptsP)); })) {
            s.compare("create", n);
            // a probe point: on an edge, near a vertex, or anywhere
            const float* poly = reinterpret_cast<const float*>(static_cast<std::uintptr_t>(*reinterpret_cast<std::uint32_t*>(pO + 4)));
            float probe[3];
            const int k = tu(count), k2 = (k + 1) % count;
            const float t = tf(0, 1);
            for (int c = 0; c < 3; ++c) probe[c] = (tu(3) == 0) ? tf(-40, 40) : poly[3 * k + c] + t * (poly[3 * k2 + c] - poly[3 * k + c]);
            if (tu(4) == 0) std::memcpy(probe, &poly[3 * k], 12);
            int fO = 0, fP = 0;
            if (s.both("find edge", n, [&] { fO = oFindEdge(pO, I(probe)); }, [&] { fP = recoil::ClipPolygon_FindEdgeContaining(pP, I(probe)); })) {
                if (fO != fP) s.fail("find edge", n, "index " + std::to_string(fO) + " vs " + std::to_string(fP));
                found += fO >= 0;
            }
            // merge a few points: some on edges, some on vertices, some off the polygon
            float add[3 * 4] = {};  // zeroed: an uninitialised word could look like a pointer into one side
            const int m = 1 + tu(4);
            for (int i = 0; i < m; ++i) {
                const int j = tu(count), j2 = (j + 1) % count;
                const float u = tf(0, 1);
                for (int c = 0; c < 3; ++c) add[3 * i + c] = (tu(4) == 0) ? tf(-40, 40) : poly[3 * j + c] + u * (poly[3 * j2 + c] - poly[3 * j + c]);
            }
            float addO[3 * 4], addP[3 * 4];
            std::memcpy(addO, add, sizeof add); std::memcpy(addP, add, sizeof add);
            s.O.add(addO, 2, sizeof addO); s.P.add(addP, 2, sizeof addP);
            int mO = 0, mP = 0;
            if (s.both("merge", n, [&] { mO = oMerge(pO, m, I(addO)); }, [&] { mP = recoil::ClipPolygon_MergePoints(pP, m, I(addP)); })) {
                if (mO != mP) s.fail("merge", n, "result");
                merged += mO != 0;
                s.compare("merge", n);
            }
        }
        abnormal += s.abnormal;
        if (s.bad) { bad = s.bad; first = s.first; }
        s.release();
    }
    if (bad) std::printf("  first difference: %s\n", first.c_str());
    std::printf("  edge found %d times, merge placed points %d times, %d abnormal endings on both sides\n", found, merged, abnormal);
    CHECK_EQ(bad, 0);
    CHECK(found > 300);
    CHECK(merged > 300);
}

TEST(native_geometry_triangulation_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    auto oSplit = rt::original<F5>(0x0046ced0);
    auto oRecursive = rt::original<F4>(0x0046cb50);
    auto oConvexify = rt::original<F3>(0x0046c760);
    auto oRingPair = rt::original<F4>(0x0046c070);
    auto oTryAdd = rt::original<F3>(0x0046bd50);
    hg::Recording rec;
    int bad = 0, abnormal = 0, calls[5] = {};
    std::string first;
    for (int n = 0; n < 3000 && !bad; ++n) {
        Pair s;
        const int count = 3 + tu(10);
        float pts[3 * 16] = {};
        ring(pts, count, tu(2) == 0);
        const int op = n % 5;
        if (std::getenv("RECOIL_VERBOSE")) { std::printf("  iter %d op %d\n", n, op); std::fflush(stdout); }
        if (op == 0) {  // Triangulate_SplitPolygon on an index list (stride 2 or 3 dwords: x and y float offsets)
            const int stride = 2 + tu(2);
            std::uint32_t idx[3 * 16] = {};
            for (int i = 0; i < count; ++i) { idx[stride * i] = 3 * i; idx[stride * i + 1] = 3 * i + 1; if (stride == 3) idx[stride * i + 2] = trng(); }
            std::uint32_t outO[160], outP[160];
            std::memset(outO, 0xAB, sizeof outO); std::memset(outP, 0xAB, sizeof outP);
            int rO = 0, rP = 0;
            if (s.both("split", n, [&] { rO = oSplit(count, I(pts), I(idx), I(outO), stride); },
                       [&] { rP = recoil::Triangulate_SplitPolygon(count, I(pts), I(idx), I(outP), stride); })) {
                if (rO != rP || std::memcmp(outO, outP, sizeof outO)) s.fail("split", n, "result");
                ++calls[0];
            }
        } else if (op == 1) {  // Triangulate_PolygonRecursive: identity indices (null) or an index list
            const int stride_mode = 1 + tu(2);
            const int stride = stride_mode == 1 ? 2 : 3;
            std::uint32_t idx[3 * 16] = {};
            for (int i = 0; i < count; ++i) { idx[stride * i] = 3 * i; idx[stride * i + 1] = 3 * i + 1; }
            const bool identity = tu(2) == 0;
            const int c = tu(12) == 0 ? tu(3) : count;  // < 3 takes the error path
            int rO = 0, rP = 0;
            if (s.both("recursive", n, [&] { rO = oRecursive(c, I(pts), identity ? 0 : I(idx), stride_mode); },
                       [&] { rP = recoil::Triangulate_PolygonRecursive(c, I(pts), identity ? 0 : I(idx), stride_mode); })) {
                if ((rO == 0) != (rP == 0)) s.fail("recursive", n, "result");
                s.compare("recursive", n);
                ++calls[1];
            }
        } else if (op == 2) {  // Convexify: contour list {count, headers (n, start)} over one xyz array
            float data[3 * 40] = {};
            std::uint32_t headers[2 * 4];
            const int contours = 1 + tu(3);
            int total = 0;
            for (int k = 0; k < contours; ++k) {
                const int cn = tu(8) == 0 ? tu(3) : 3 + tu(8);  // some degenerate (< 3) contours
                ring(&data[3 * total], cn, tu(2) == 0);
                headers[2 * k] = cn; headers[2 * k + 1] = total;
                total += cn;
            }
            std::uint32_t list[2] = {static_cast<std::uint32_t>(contours), static_cast<std::uint32_t>(I(headers))};
            const int t = tu(20) == 0 ? 0 : total;
            s.O.add(data, 1, sizeof data); s.P.add(data, 1, sizeof data);
            s.O.add(headers, 2, sizeof headers); s.P.add(headers, 2, sizeof headers);
            s.O.add(list, 3, sizeof list); s.P.add(list, 3, sizeof list);
            int rO = 0, rP = 0;
            if (s.both("convexify", n, [&] { rO = oConvexify(I(list), t, I(data)); }, [&] { rP = recoil::Convexify(I(list), t, I(data)); })) {
                if ((rO == 0) != (rP == 0)) s.fail("convexify", n, "result");
                s.compare("convexify", n);
                ++calls[2];
            }
        } else if (op == 3) {  // Triangulate_RingPair: an outer ring and an inner ring inside it; globals compared
            float inner[3 * 16] = {};
            const int m = 3 + tu(5);
            for (int i = 0; i < m; ++i) {
                const float a = 6.2831853f * static_cast<float>(i) / static_cast<float>(m);
                inner[3 * i] = pts[0] * 0.1f + 3.0f * std::cos(a); inner[3 * i + 1] = pts[1] * 0.1f + 3.0f * std::sin(a); inner[3 * i + 2] = tf(-1, 1);
            }
            float innerO[3 * 16], innerP[3 * 16];
            std::memcpy(innerO, inner, sizeof inner); std::memcpy(innerP, inner, sizeof inner);
            s.O.add(pts, 1, sizeof pts); s.P.add(pts, 1, sizeof pts);
            s.O.add(innerO, 2, sizeof innerO); s.P.add(innerP, 2, sizeof innerP);
            at<std::uint32_t>(kOrigTriCount) = recoil::g_TriangleCount_0053d750 = 0;
            at<std::uint32_t>(kOrigWord) = recoil::g_TriangulateWord_0053d754 = 0;
            int rO = 0, rP = 0;
            if (s.both("ring pair", n, [&] { rO = oRingPair(count, I(pts), m, I(innerO)); },
                       [&] { rP = recoil::Triangulate_RingPair(count, I(pts), m, I(innerP)); })) {
                if ((rO == 0) != (rP == 0)) s.fail("ring pair", n, "result");
                s.compare("ring pair", n);
                const std::uint32_t tc = at<std::uint32_t>(kOrigTriCount);
                std::uint32_t off = 0, offP = 0;
                const int vO = s.O.find(reinterpret_cast<void*>(static_cast<std::uintptr_t>(at<std::uint32_t>(kOrigVerts))), &off);
                const int vP = s.P.find(recoil::g_TriangulateVertices_0053a748, &offP);
                if (tc != recoil::g_TriangleCount_0053d750 || at<std::uint32_t>(kOrigWord) != recoil::g_TriangulateWord_0053d754
                    || vO != vP || off != offP
                    || std::memcmp(reinterpret_cast<void*>(kOrigTriList), recoil::g_TriangleList_0053a750, 12 * (tc < 1024 ? tc : 1024))
                    || std::memcmp(reinterpret_cast<void*>(kOrigPlane), recoil::g_NewellPlane_0053d758, 16))
                    s.fail("ring pair", n, "globals");
                ++calls[3];
            }
        } else {  // Triangulate_TryAddEdge on a vertex array and an edge list
            float verts[3 * 16];
            std::memcpy(verts, pts, sizeof verts);
            at<std::uint32_t>(kOrigVerts) = static_cast<std::uint32_t>(I(verts));
            recoil::g_TriangulateVertices_0053a748 = verts;
            std::uint32_t eO[3 * 20], eP[3 * 20];
            const int ec = tu(10);
            for (int i = 0; i < ec; ++i) { eO[3 * i] = tu(count); eO[3 * i + 1] = tu(count); eO[3 * i + 2] = tu(2); }
            std::memcpy(eP, eO, sizeof eO);
            std::uint32_t cand[3] = {static_cast<std::uint32_t>(tu(count)), static_cast<std::uint32_t>(tu(count)), 1};
            int rO = 0, rP = 0;
            if (s.both("try add edge", n, [&] { rO = oTryAdd(I(cand), ec, I(eO)); }, [&] { rP = recoil::Triangulate_TryAddEdge(I(cand), ec, I(eP)); })) {
                if (rO != rP || std::memcmp(eO, eP, 12 * (rO > ec ? rO : ec))) s.fail("try add edge", n, "result");
                ++calls[4];
            }
        }
        abnormal += s.abnormal;
        if (s.bad) { bad = s.bad; first = s.first; }
        s.release();
    }
    if (bad) std::printf("  first difference: %s\n", first.c_str());
    std::printf("  normal calls: split %d, recursive %d, convexify %d, ring pair %d, try add %d; %d abnormal endings on both sides\n",
                calls[0], calls[1], calls[2], calls[3], calls[4], abnormal);
    CHECK_EQ(bad, 0);
    for (int c : calls) CHECK(c > 200);
}
