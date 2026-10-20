// Structured native L1 for World_BuildGrid (0x00450c60: ECX world node, stack cellX, cellZ floats; ret 8). The grid
// data (+0x38): an existing row table (+0x80) is freed first through World_FreeGrid (0x00450e40: each cell's +0x3C
// buffer, each row and the table unless +0xA8 is set); then the cell sizes +0x54 / +0x58, margins +0x70 / +0x74, half
// sizes +0x5c / +0x60, inverse sizes +0x64 / +0x68, the negative half diagonal +0x6c (fast sqrt), the grid size
// +0x78 / +0x7c (trunc(extent / cell), +1 when short of the extent) and the calloc'd rows of 0x40-byte cells, each with
// flag bit 8, its corner, its box and its bounding sphere (Math_BoxToSphere 0x004525d0) and index -1. The arena fuzz fed
// it denormal sizes. Real grids: cells of 16..256 (and random) units, z negative as in the traces, extents an exact or
// fractional number of cells, with or without an existing grid (cell buffers real msvcrt blocks). free is logged in
// both import slots. Compared: the return, the grid data words (the new table pointer as a role), every word of every
// new cell and the freed blocks in order, by role (table, row, cell buffer).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_world.h"
#include "platform/iat_msvcrt.h"

#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t bits(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
std::uint32_t real_calloc(std::size_t n, std::size_t s)
{
    return addr(reinterpret_cast<void*(__cdecl*)(std::size_t, std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "calloc"))(n, s));
}
void real_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(ptr<void>(p)); }
std::vector<std::uint32_t> g_freed;
void __cdecl logging_free(void* p) { g_freed.push_back(addr(p)); real_free(addr(p)); }
}  // namespace

TEST(native_world_build_grid_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int, float, float);
    const Fn fn[2] = {rt::original<Fn>(0x00450c60), reinterpret_cast<Fn>(&recoil::World_BuildGrid)};
    void** o_free = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* const saved[2] = {*o_free, recoil::g_Iat_free_004cc5b4};
    *o_free = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    std::mt19937 rng(0x450c60);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0, rebuilt = 0;
    for (int it = 0; it < 3000; ++it) {
        const float cx = rng() % 5 ? static_cast<float>(16 << (rng() % 5)) : fr(8, 300);
        const float cz = -(rng() % 5 ? static_cast<float>(16 << (rng() % 5)) : fr(8, 300));
        const int nx = 1 + static_cast<int>(rng() % 16), nz = 1 + static_cast<int>(rng() % 16);
        const float ex = cx * (rng() % 2 ? nx : nx - 1 + fr(0.01f, 0.99f));
        const float ez = cz * (rng() % 2 ? nz : nz - 1 + fr(0.01f, 0.99f));
        std::uint32_t init[64];
        for (auto& w : init) w = rng() | 0x80000000u;
        init[0x34 / 4] = bits(fr(-3000, 3000)); init[0x38 / 4] = bits(fr(-3000, 3000));
        init[0x3C / 4] = bits(ex); init[0x40 / 4] = bits(ez);
        const bool old = rng() % 2;
        const int ox = 1 + static_cast<int>(rng() % 4), oz = 1 + static_cast<int>(rng() % 4);
        const std::uint32_t keep_rows = rng() % 4 == 0 ? 1u : 0u;  // +0xA8: rows and table not freed
        std::vector<int> has_buf(ox * oz);
        for (auto& h : has_buf) h = static_cast<int>(rng() % 2);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t data[64], node[20] = {};
            std::memcpy(data, init, sizeof data);
            node[0x38 / 4] = addr(data);
            data[0x80 / 4] = 0;
            std::vector<std::pair<std::uint32_t, std::uint32_t>> roles;  // block -> role
            if (old) {
                data[0x78 / 4] = static_cast<std::uint32_t>(ox);
                data[0x7C / 4] = static_cast<std::uint32_t>(oz);
                data[0xA8 / 4] = keep_rows;
                const std::uint32_t table = real_calloc(oz, 4);
                roles.push_back({table, 1});
                for (int r = 0; r < oz; ++r) {
                    ptr<std::uint32_t>(table)[r] = real_calloc(ox, 0x40);
                    roles.push_back({ptr<std::uint32_t>(table)[r], 0x10u + r});
                    for (int c = 0; c < ox; ++c) {
                        const std::uint32_t b = has_buf[r * ox + c] ? real_calloc(1, 16) : 0u;
                        ptr<std::uint32_t>(ptr<std::uint32_t>(table)[r])[16 * c + 15] = b;
                        if (b) roles.push_back({b, 0x100u + 16 * r + c});
                    }
                }
                data[0x80 / 4] = table;
            }
            g_freed.clear();
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](node, 0, cx, cz)));
            const std::vector<std::uint32_t> freed = g_freed;
            for (std::uint32_t f : freed) {
                std::uint32_t role = 0xBAD;
                for (auto& [b, r] : roles) if (b == f) role = r;
                snap[side].push_back(role);
            }
            snap[side].push_back(0xF0F0F0F0u);
            for (auto& [b, r] : roles)  // blocks the call left (+0xA8 set, or a mutant): free them now
                if (std::find(freed.begin(), freed.end(), b) == freed.end()) real_free(b);
            const std::uint32_t table = data[0x80 / 4];
            for (int k = 0; k < 64; ++k) snap[side].push_back(k == 0x80 / 4 ? (table ? 1u : 0u) : data[k]);
            const int gx = static_cast<int>(data[0x78 / 4]), gz = static_cast<int>(data[0x7C / 4]);
            if (table && gx > 0 && gz > 0 && gx < 64 && gz < 64) {
                for (int r = 0; r < gz; ++r) {
                    const std::uint32_t row = ptr<std::uint32_t>(table)[r];
                    for (int w = 0; w < 16 * gx; ++w) snap[side].push_back(ptr<std::uint32_t>(row)[w]);
                    real_free(row);
                }
                real_free(table);
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        rebuilt += old;
        ++compared;
    }
    *o_free = saved[0];
    recoil::g_Iat_free_004cc5b4 = saved[1];
    std::printf("  World_BuildGrid calls %d, %d over an existing grid (cells 16..256 / random, exact or fractional extents)\n", compared, rebuilt);
}
