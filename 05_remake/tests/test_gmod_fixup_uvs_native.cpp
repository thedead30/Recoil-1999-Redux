// Native L1 for Polygon_FixupUVs (0x004843b0). It copies a polygon's uvs into a 0x200-byte local buffer (64
// vertices) sized by the polygon's vertex count, so a random count (tests/arena_fuzz.h) overruns its stack frame and
// the SEH records the harness relies on - the crash escaped the guard. Each call here gets a well-formed model
// instead (layout read from the listing): model +0x30 polygon array (0x1C-byte entries), +0x34 vertex array (xyz
// floats, 12 bytes); polygon +0 low byte = vertex count (0..40), +8 vertex index list, +0x10 uv pairs, +0x14
// material (bit 0 of byte +1 = textured). Vertices include duplicates and collinear runs, so degenerate normals and
// zero determinants are reached. Each side gets its own identical copy; compared: return value, model and polygon
// words, every uv (as bits) and the vertex array.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
struct Side {
    std::uint32_t model[32];
    std::vector<std::uint32_t> polys, verts, material;
    std::vector<std::vector<std::uint32_t>> idx, uv;
};
std::uint32_t bits(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
// EAX is left holding a pointer into the call's buffers (the function returns nothing): compare it by which buffer
// and offset, as the fuzz harness does for heap pointers (tests/heap_graph.h); anything else raw.
std::uint64_t role(const Side& d, std::uint32_t v)
{
    std::vector<std::pair<const std::uint32_t*, std::size_t>> bufs = {
        {d.model, 32}, {d.polys.data(), d.polys.size()}, {d.verts.data(), d.verts.size()}, {d.material.data(), d.material.size()}};
    for (const auto& b : d.idx) bufs.push_back({b.data(), b.size()});
    for (const auto& b : d.uv) bufs.push_back({b.data(), b.size()});
    for (std::size_t k = 0; k < bufs.size(); ++k) {
        const std::uint32_t lo = addr(bufs[k].first);
        if (v >= lo && v <= lo + 4 * bufs[k].second) return (std::uint64_t(k + 1) << 32) | (v - lo);
    }
    return v;
}
}  // namespace

TEST(native_gmod_fixup_uvs_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fix = std::uint32_t(__fastcall*)(void*, int);
    auto orig = rt::original<Fix>(0x004843b0);
    auto port = reinterpret_cast<Fix>(&recoil::Polygon_FixupUVs);
    std::mt19937 rng(0x4843b0);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const int nverts = 3 + static_cast<int>(rng() % 60);
        const int npolys = 1 + static_cast<int>(rng() % 4);
        std::vector<std::uint32_t> verts(nverts * 3);
        const int mode = static_cast<int>(rng() % 4);  // 0 random, 1 planar, 2 collinear, 3 few distinct points
        for (int v = 0; v < nverts; ++v) {
            float x = fr(-100, 100), y = fr(-100, 100), z = fr(-100, 100);
            if (mode == 1) z = 0.5f * x - 0.25f * y + 3.0f;
            if (mode == 2) { const float t = fr(-10, 10); x = t; y = 2 * t; z = -t; }
            if (mode == 3) { x = static_cast<float>(rng() % 3); y = static_cast<float>(rng() % 2); z = 1.0f; }
            verts[3 * v] = bits(x); verts[3 * v + 1] = bits(y); verts[3 * v + 2] = bits(z);
        }
        std::vector<std::vector<std::uint32_t>> idx(npolys), uv(npolys);
        std::vector<std::uint32_t> material(npolys * 4), poly_head(npolys);
        for (int p = 0; p < npolys; ++p) {
            const int n = static_cast<int>(rng() % 41);
            idx[p].resize(n ? n : 1);
            uv[p].resize(2 * (n ? n : 1));
            for (auto& i : idx[p]) i = rng() % nverts;
            for (auto& u : uv[p]) u = bits(fr(-4, 4));
            material[4 * p] = (rng() % 4 ? 0x100u : 0u) | (rng() & 0xFFFF00FEu);
            poly_head[p] = (rng() & 0xFFFFFF00u) | static_cast<std::uint32_t>(n);
        }
        const int which = static_cast<int>(rng() % npolys);
        Side s[2];
        std::uint64_t ret[2];
        for (int side = 0; side < 2; ++side) {
            Side& d = s[side];
            std::mt19937 fill(it);
            for (auto& w : d.model) w = fill();
            d.verts = verts;
            d.material = material;
            d.idx = idx;
            d.uv = uv;
            d.polys.assign(npolys * 7, 0);
            for (int p = 0; p < npolys; ++p) {
                d.polys[7 * p] = poly_head[p];
                d.polys[7 * p + 1] = fill();
                d.polys[7 * p + 2] = addr(d.idx[p].data());
                d.polys[7 * p + 3] = fill();
                d.polys[7 * p + 4] = addr(d.uv[p].data());
                d.polys[7 * p + 5] = addr(&d.material[4 * p]);
                d.polys[7 * p + 6] = fill();
            }
            d.model[0x30 / 4] = addr(d.polys.data());
            d.model[0x34 / 4] = addr(d.verts.data());
            rt::restore_pristine();
            ret[side] = role(d, (side ? port : orig)(d.model, which));
            d.model[0x30 / 4] = d.model[0x34 / 4] = 0;  // per-side buffer addresses
            for (int p = 0; p < npolys; ++p) d.polys[7 * p + 2] = d.polys[7 * p + 4] = d.polys[7 * p + 5] = 0;
        }
        CHECK_EQ(ret[0], ret[1]);
        for (int i = 0; i < 32; ++i) CHECK_EQ(s[0].model[i], s[1].model[i]);
        CHECK(s[0].polys == s[1].polys);
        CHECK(s[0].uv == s[1].uv);
        CHECK(s[0].verts == s[1].verts);
        CHECK(s[0].idx == s[1].idx);
        CHECK(s[0].material == s[1].material);
        ++compared;
    }
    std::printf("  Polygon_FixupUVs calls %d (well-formed models, polygons of 0..40 vertices)\n", compared);
}
