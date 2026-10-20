// Structured native L1 for the cls_world float functions (the arena fuzz only feeds them denormal floats):
// World_PointToCellClamped (0x00450650: ECX world, EDX &xcell, stack x, z, &zcell, &x out, &z out, &inside; ret 0x18),
// World_PointToCellClampedSimple (0x00450790: ECX, EDX &xcell, stack x, z, &zcell; ret 0xC),
// World_BoxToCell (0x00450840: ECX, EDX &cellX, stack minX, maxX, minZ, maxZ, &cellZ; ret 0x14),
// World_SetOrigin (0x00450c00) and World_SetExtent (0x00450c30) (ECX, stack two floats; ret 8).
// The world node's +0x38 holds the grid data: origin x/z +0x34/+0x38, extent +0x3C/+0x40 (z negative, as in the
// traces), far corner +0x44/+0x48, inverse cell sizes +0x64/+0x68, margins +0x70/+0x74. Real grids, points and boxes
// inside, near the edges and outside. Compared: every output word and the grid data words.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_world.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t bits(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
}  // namespace

TEST(native_world_grid_float_functions_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Clamp = int(__fastcall*)(void*, std::int32_t*, float, float, std::int32_t*, float*, float*, std::int32_t*);
    using Simple = int(__fastcall*)(void*, std::int32_t*, float, float, std::int32_t*);
    using Box = int(__fastcall*)(void*, std::int32_t*, float, float, float, float, std::int32_t*);
    using Set2 = int(__fastcall*)(void*, int, float, float);
    const Clamp clamp[2] = {rt::original<Clamp>(0x00450650), reinterpret_cast<Clamp>(&recoil::World_PointToCellClamped)};
    const Simple simple[2] = {rt::original<Simple>(0x00450790), reinterpret_cast<Simple>(&recoil::World_PointToCellClampedSimple)};
    const Box box[2] = {rt::original<Box>(0x00450840), reinterpret_cast<Box>(&recoil::World_BoxToCell)};
    const Set2 origin[2] = {rt::original<Set2>(0x00450c00), reinterpret_cast<Set2>(&recoil::World_SetOrigin)};
    const Set2 extent[2] = {rt::original<Set2>(0x00450c30), reinterpret_cast<Set2>(&recoil::World_SetExtent)};
    std::mt19937 rng(0x450650);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0;
    for (int it = 0; it < 20000; ++it) {
        const float cell = static_cast<float>(16 << (rng() % 4));
        const int nx = 1 + static_cast<int>(rng() % 20), nz = 1 + static_cast<int>(rng() % 20);
        const float ox = fr(-3000, 3000), oz = fr(-3000, 3000);
        const float ex = cell * nx, ez = -cell * nz;
        std::uint32_t g[43];
        for (auto& w : g) w = rng() | 0x80000000u;
        auto setf = [&](int off, float v) { g[off / 4] = bits(v); };
        setf(0x34, ox); setf(0x38, oz); setf(0x3C, ex); setf(0x40, ez); setf(0x44, ox + ex); setf(0x48, oz + ez);
        setf(0x64, 1.0f / cell); setf(0x68, -1.0f / cell); setf(0x70, fr(0, cell)); setf(0x74, fr(0, cell));
        const int kind = static_cast<int>(rng() % 3);  // 0 inside, 1 near an edge, 2 anywhere
        auto px = [&] { return kind == 0 ? fr(ox, ox + ex) : kind == 1 ? (rng() % 2 ? ox : ox + ex) + fr(-1, 1) : fr(ox - 2 * ex, ox + 3 * ex); };
        auto pz = [&] { return kind == 0 ? fr(oz + ez, oz) : kind == 1 ? (rng() % 2 ? oz : oz + ez) + fr(-1, 1) : fr(oz + 3 * ez, oz - 2 * ez); };
        // the cell grid (+0x80 rows of +0x78 cells of 0x40 bytes, +0x7C rows): World_BoxToCell reads the cell's bounds
        std::vector<std::vector<std::uint32_t>> cells(static_cast<std::size_t>(nz), std::vector<std::uint32_t>(16 * static_cast<std::size_t>(nx)));
        for (auto& row : cells) for (auto& w : row) w = bits(fr(-4000, 4000));
        std::vector<std::uint32_t> rows(static_cast<std::size_t>(nz));
        for (int r = 0; r < nz; ++r) rows[r] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(cells[r].data()));
        g[0x78 / 4] = static_cast<std::uint32_t>(nx);
        g[0x7C / 4] = static_cast<std::uint32_t>(nz);
        g[0x80 / 4] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(rows.data()));
        const float x = px(), z = pz(), x2 = px(), z2 = pz();
        const float sa = fr(-5000, 5000), sb = fr(-5000, 5000);
        const int fn = it % 5;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t grid[43];
            std::memcpy(grid, g, sizeof grid);
            std::uint32_t node[20] = {};
            node[0x38 / 4] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(grid));
            std::int32_t o[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
            float fo[2] = {-7.0f, -8.0f};
            int r = 0;
            if (fn == 0) r = clamp[side](node, &o[0], x, z, &o[1], &fo[0], &fo[1], &o[2]);
            else if (fn == 1) r = simple[side](node, &o[0], x, z, &o[1]);
            else if (fn == 2) r = box[side](node, &o[0], x < x2 ? x : x2, x < x2 ? x2 : x, z < z2 ? z : z2, z < z2 ? z2 : z, &o[1]);
            else if (fn == 3) r = origin[side](node, 0, sa, sb);
            else r = extent[side](node, 0, sa, sb);
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (auto v : o) snap[side].push_back(static_cast<std::uint32_t>(v));
            snap[side].push_back(bits(fo[0])); snap[side].push_back(bits(fo[1]));
            snap[side].insert(snap[side].end(), grid, grid + 43);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  World grid float functions calls %d (real grids, points / boxes inside, at the edges, outside)\n", compared);
}
