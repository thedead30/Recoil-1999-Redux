// Native L1 for the model-level clip functions (zgeo_model): ClipPolygon_ComputeUVs 0x0046a7f0,
// ClipModel_SnapToRegion 0x0046b030 and ClipModel_AgainstRegion 0x0046bb90, ORIGINAL vs port on synthetic
// models. The models are raw words at the offsets the listings read (model: polygon count +0xC, polygons +0x30
// in 0x1C-byte records, vertices +0x34; polygon: vertex count in the low byte of +0x0, indices +0x8, uvs +0x10;
// object: flags +0x24, model +0x3C, bounds +0x8C..+0xA0; region holder: Weiler clip +0x0, points +0x4,
// count +0x8, rectangle +0xC..+0x18). Both sides run identical instructions on identical bytes, so the
// comparison does not depend on this reading being complete - only the realism of the inputs does.
#include "test.h"
#include "native_oracle.h"
#include "heap_graph.h"
#include "watchdog.h"
#include "GameZRecoil/zGeometry/zgeo_model.h"
#include "GameZRecoil/zGeometry/zgeo_weiler.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>

namespace {
std::mt19937 mrng(0x30DE1);
int mu(int n) { return static_cast<int>(mrng() % static_cast<unsigned>(n)); }
float mf(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(mrng); }
int I(const void* p) { return static_cast<int>(reinterpret_cast<std::uintptr_t>(p)); }
using F2 = int(__fastcall*)(int, int);
using F3 = int(__fastcall*)(int, int, int);
using F4 = int(__fastcall*)(int, int, int, int);

constexpr int kVerts = 24, kPolys = 6;
// One side's copy of a synthetic model (so writes on one side cannot leak into the other).
struct ModelSide {
    float verts[3 * kVerts];
    std::uint32_t idx[kPolys][8];
    float uvs[kPolys][6];
    std::uint32_t polys[kPolys][7];
    std::uint32_t model[16];
    std::uint32_t object[0xA4 / 4];
    void link(int npolys)
    {
        for (int p = 0; p < kPolys; ++p) { polys[p][2] = static_cast<std::uint32_t>(I(idx[p])); polys[p][4] = static_cast<std::uint32_t>(I(uvs[p])); }
        model[0xC / 4] = static_cast<std::uint32_t>(npolys);
        model[0x30 / 4] = static_cast<std::uint32_t>(I(polys));
        model[0x34 / 4] = static_cast<std::uint32_t>(I(verts));
        object[0x3C / 4] = static_cast<std::uint32_t>(I(model));
    }
    void register_in(hg::Log& L, int base)
    {
        L.add(verts, base, sizeof verts); L.add(idx, base + 1, sizeof idx); L.add(uvs, base + 2, sizeof uvs);
        L.add(polys, base + 3, sizeof polys); L.add(model, base + 4, sizeof model); L.add(object, base + 5, sizeof object);
    }
};

void make_model(ModelSide& a, int& npolys, bool grid)
{
    std::memset(&a, 0, sizeof a);
    for (int i = 0; i < kVerts; ++i) {
        float x = mf(-40, 40), z = mf(-40, 40);
        if (grid) { x = std::round(x / 10.0f) * 10.0f; z = std::round(z / 10.0f) * 10.0f; }
        a.verts[3 * i] = x; a.verts[3 * i + 1] = mf(-2, 2); a.verts[3 * i + 2] = z;
    }
    npolys = 1 + mu(kPolys);
    for (int p = 0; p < kPolys; ++p) {
        const int n = mu(10) == 0 ? mu(3) : 3 + mu(2);  // mostly triangles and quads, a few degenerate
        // indices around a small fan so polygons are compact and non-degenerate
        const int base = mu(kVerts - 5);
        for (int k = 0; k < n; ++k) a.idx[p][k] = static_cast<std::uint32_t>(base + k);
        a.polys[p][0] = (mrng() & 0xFF00u) | static_cast<std::uint32_t>(n);
        for (int k = 0; k < 6; ++k) a.uvs[p][k] = mf(0, 1);
        // filler below 0x10000: a random word could otherwise equal a live heap address on one side only
        for (int k : {1, 3, 5, 6}) a.polys[p][k] = mrng() & 0xFFFFu;
    }
    for (auto& w : a.model) w = w ? w : 0;
}

struct Pair {
    hg::Log O, P;
    int bad = 0, abnormal = 0;
    std::string first;
    template <class FO, class FP>
    bool both(const char* what, int n, FO fo, FP fp)
    {
        hg::current() = &O; const unsigned a = wd::guarded(fo);
        hg::current() = &P; const unsigned b = wd::guarded(fp);
        if (a != b) { fail(what, n, "outcome " + std::to_string(a) + " vs " + std::to_string(b)); return false; }
        if (a != wd::kOk) { ++abnormal; return false; }
        return true;
    }
    void fail(const char* what, int n, const std::string& d) { if (!bad++) first = std::string(what) + " #" + std::to_string(n) + ": " + d; }
    void compare(const char* what, int n) { const std::string d = hg::compare(O, P); if (!d.empty()) fail(what, n, d); }
    void release()
    {
        for (hg::Log* L : {&O, &P})
            for (auto& [p, blk] : L->live)
                if (blk.role >= 1000) hg::real().free_(reinterpret_cast<void*>(p));
    }
};
}  // namespace

TEST(native_geometry_model_clip_functions_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    auto oUV = rt::original<F4>(0x0046a7f0);
    auto oSnap = rt::original<F2>(0x0046b030);
    auto oAgainst = rt::original<F2>(0x0046bb90);
    auto oCreate = rt::original<F3>(0x00464680);
    // The port is called through function pointers of the same types, so both sides' calls have identical frames:
    // these functions store addresses of stack-resident blocks, which must then line up exactly.
    F4 pUV = &recoil::ClipPolygon_ComputeUVs;
    F2 pSnap = &recoil::ClipModel_SnapToRegion, pAgainst = &recoil::ClipModel_AgainstRegion;
    F3 pCreate = &recoil::WeilerClip_Create;
    hg::Recording rec;
    int bad = 0, abnormal = 0, uv = 0, snapped = 0, against_calls = 0, against_ok[2] = {};
    std::string first;
    static ModelSide mo, mp;  // static: large; one set per side
    for (int n = 0; n < 3000 && !bad; ++n) {
        Pair s;
        int npolys = 0;
        make_model(mo, npolys, mu(2) == 0);
        std::memcpy(&mp, &mo, sizeof mo);
        mo.link(npolys); mp.link(npolys);
        mo.register_in(s.O, 10); mp.register_in(s.P, 10);
        const int op = n % 3;
        if (op == 0) {  // ComputeUVs: points on the parent's plane, uvs from the parent's first three vertices
            float pts[3 * 8] = {};
            const int count = mu(9);
            for (int i = 0; i < count; ++i) { pts[3 * i] = mf(-40, 40); pts[3 * i + 1] = mf(-2, 2); pts[3 * i + 2] = mf(-40, 40); }
            s.O.add(pts, 1, sizeof pts); s.P.add(pts, 1, sizeof pts);
            const int p = mu(npolys);
            int rO = 0, rP = 0;
            if (s.both("uvs", n, [&] { rO = hg::call_fastcall(oUV, count, I(pts), I(mo.model), I(mo.polys[p])); },
                       [&] { rP = hg::call_fastcall(pUV, count, I(pts), I(mp.model), I(mp.polys[p])); })) {
                s.compare("uvs", n);
                ++uv;
            }
        } else if (op == 1) {  // SnapToRegion: region points near the model's rotated (x, -z) vertices
            float rO_[3 * 8], rP_[3 * 8];
            const int rc = 3 + mu(6);
            for (int i = 0; i < rc; ++i) {
                const int v = mu(kVerts);
                rO_[3 * i] = mo.verts[3 * v] + (mu(2) ? mf(-0.08f, 0.08f) : mf(-5, 5));
                rO_[3 * i + 1] = -mo.verts[3 * v + 2] + (mu(2) ? mf(-0.08f, 0.08f) : mf(-5, 5));
                rO_[3 * i + 2] = mf(-1, 1);
            }
            std::memcpy(rP_, rO_, sizeof rO_);
            std::uint32_t holderO[7], holderP[7];
            float rect[4] = {mf(-60, 0), mf(-60, 0), mf(0, 60), mf(0, 60)};
            holderO[0] = holderP[0] = 0;
            holderO[1] = static_cast<std::uint32_t>(I(rO_)); holderP[1] = static_cast<std::uint32_t>(I(rP_));
            holderO[2] = holderP[2] = static_cast<std::uint32_t>(rc);
            std::memcpy(&holderO[3], rect, 16); std::memcpy(&holderP[3], rect, 16);
            s.O.add(rO_, 1, sizeof rO_); s.P.add(rP_, 1, sizeof rP_);
            s.O.add(holderO, 2, sizeof holderO); s.P.add(holderP, 2, sizeof holderP);
            const std::uint32_t flags = mu(3) == 0 ? 0x200u : 0u;
            mo.object[0x24 / 4] = mp.object[0x24 / 4] = flags;
            for (int off : {0x8C, 0x94, 0x98, 0xA0}) { const float v = mf(-80, 80); std::memcpy(&mo.object[off / 4], &v, 4); std::memcpy(&mp.object[off / 4], &v, 4); }
            int a = 0, b = 0;
            if (s.both("snap", n, [&] { a = hg::call_fastcall(oSnap, I(holderO), I(mo.object)); }, [&] { b = hg::call_fastcall(pSnap, I(holderP), I(mp.object)); })) {
                if (a != b) s.fail("snap", n, "result");
                snapped += a != 0;
                s.compare("snap", n);
            }
        } else {  // AgainstRegion: each side clips with its own Weiler clip built from the same region polygon
            float reg[3 * 8] = {};
            const int rc = 3 + mu(5);
            const float cx = mf(-20, 20), cy = mf(-20, 20), r = mf(5, 40);
            for (int i = 0; i < rc; ++i) {
                const float ang = 6.2831853f * static_cast<float>(i) / static_cast<float>(rc);
                reg[3 * i] = cx + r * std::cos(ang); reg[3 * i + 1] = cy + r * std::sin(ang);
            }
            float regO[3 * 8], regP[3 * 8];
            std::memcpy(regO, reg, sizeof reg); std::memcpy(regP, reg, sizeof reg);
            s.O.add(regO, 1, sizeof regO); s.P.add(regP, 1, sizeof regP);
            int cO = 0, cP = 0;
            if (s.both("create", n, [&] { cO = hg::call_fastcall(oCreate, I(regO), rc, 0); }, [&] { cP = hg::call_fastcall(pCreate, I(regP), rc, 0); }) && cO && cP) {
                std::uint32_t holderO[7], holderP[7];
                const float rect[4] = {cx - r, cy - r, cx + r, cy + r};
                holderO[0] = static_cast<std::uint32_t>(cO); holderP[0] = static_cast<std::uint32_t>(cP);
                holderO[1] = static_cast<std::uint32_t>(I(regO)); holderP[1] = static_cast<std::uint32_t>(I(regP));
                holderO[2] = holderP[2] = static_cast<std::uint32_t>(rc);
                std::memcpy(&holderO[3], rect, 16); std::memcpy(&holderP[3], rect, 16);
                s.O.add(holderO, 2, sizeof holderO); s.P.add(holderP, 2, sizeof holderP);
                int a = 0, b = 0;
                if (s.both("against", n, [&] { a = hg::call_fastcall(oAgainst, I(holderO), I(mo.model)); }, [&] { b = hg::call_fastcall(pAgainst, I(holderP), I(mp.model)); })) {
                    if (a != b) s.fail("against", n, "result");
                    ++against_calls;
                    ++against_ok[a != 0];
                    s.compare("against", n);
                }
            }
        }
        abnormal += s.abnormal;
        if (s.bad) { bad = s.bad; first = s.first; }
        s.release();
    }
    if (bad) std::printf("  first difference: %s\n", first.c_str());
    std::printf("  uvs %d, snap changed %d, against %d (returned 1: %d, 0: %d); %d abnormal endings on both sides\n",
                uv, snapped, against_calls, against_ok[1], against_ok[0], abnormal);
    CHECK_EQ(bad, 0);
    CHECK(uv > 500);
    CHECK(snapped > 100);
    CHECK(against_ok[0] > 50 && against_ok[1] > 50);
}
