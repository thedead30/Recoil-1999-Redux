// Structured native L1 for World_AttachChild (0x004510e0) and World_DetachChild (0x00451240) (ECX world, EDX child).
// Attach: a child with +0x24 bit 7 gets no cell (-1, -1); else its box is the world's whole extent (bit 8 clear) or the
// bounds of its 8 corners (bit 8: gwNodeGetLocalCorners 0x004487c0 - class 5, the class data's matrix +0x30 over the
// node box +0x74), World_BoxToCell (0x00450840) picks the cell and World_InsertChildInCell (0x00450f60) files it.
// Detach: a child at (-1, -1) goes through gwNodeDetachChild (0x00448660); else it is looked up in its cell (not found ->
// report, 1), shifted out, its cell set to (-1, -1), the world removed from its parents, and the cell queued unless its
// bit 0 is set (World_QueueDirtyCell).
// Each side builds its own grid with World_BuildGrid (0x00450c60, verified: cells of 64..256 units, up to 6 x 6) and
// queue (world data +0..+0xC, empty); the child is class 5 with class data (null class data is the uninitialised-corners
// path of KG-38), a translation-only matrix and a box inside, across or outside the grid. f 0: attach; f 1: attach, then
// detach (sometimes after moving the child's cell index to a cell that does not hold it). Compared: returns, the grid
// data words, the queue entries and every cell's words and child array (cells by (row, column) role), the world and
// child words with arrays by role, list 7 and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_world.h"
#include "platform/image/original_data.h"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <random>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t* at(std::uint32_t p) { return reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(p)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t bits(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
void real_free(std::uint32_t p)
{
    if (p) reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(at(p));
}
}  // namespace

TEST(native_world_attach_detach_child_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*);
    using Build = int(__fastcall*)(void*, int, float, float);
    const Fn attach[2] = {rt::original<Fn>(0x004510e0), reinterpret_cast<Fn>(&recoil::World_AttachChild)};
    const Fn detach[2] = {rt::original<Fn>(0x00451240), reinterpret_cast<Fn>(&recoil::World_DetachChild)};
    const Build build[2] = {rt::original<Build>(0x00450c60), reinterpret_cast<Build>(&recoil::World_BuildGrid)};
    std::mt19937 rng(0x4510e0);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0, per[2] = {}, celled = 0;
    static std::uint32_t world[49], data[64], child[49], cdata[24];
    for (int it = 0; it < 6000; ++it) {
        const int f = it % 2;
        const float cell = static_cast<float>(64 << (rng() % 3));
        const int nx = 1 + static_cast<int>(rng() % 6), nz = 1 + static_cast<int>(rng() % 6);
        const float ox = fr(-2000, 2000), oz = fr(-2000, 2000), ex = cell * nx, ez = -cell * nz;
        std::uint32_t wi[49], di[64], ci[49], cdi[24];
        for (auto& w : wi) w = rng();
        for (auto& w : di) w = rng();
        for (auto& w : ci) w = rng();
        for (auto& w : cdi) w = bits(fr(-1, 1));
        wi[0x24 / 4] = wi[0x24 / 4] & ~1u | (rng() % 3 == 0 ? 1u : 0u);
        wi[0x54 / 4] = 0; wi[0x5C / 4] = 0; wi[0x60 / 4] = 0;
        di[0] = rng(); di[1] = 0; di[2] = 0; di[3] = 0; di[0x80 / 4] = 0;
        di[0x34 / 4] = bits(ox); di[0x38 / 4] = bits(oz); di[0x3C / 4] = bits(ex); di[0x40 / 4] = bits(ez);
        di[0x44 / 4] = bits(ox + ex); di[0x48 / 4] = bits(oz + ez); di[0xA8 / 4] = 0;
        const unsigned kind = rng() % 4;  // 0 no cell (bit 7), 1 world extent (bit 8 clear), 2 / 3 corners
        ci[0x24 / 4] = (ci[0x24 / 4] & ~0x181u) | (kind == 0 ? 0x80u : kind >= 2 ? 0x100u : 0u) | (rng() % 3 == 0 ? 1u : 0u);
        ci[0x34 / 4] = 5;
        ci[0x54 / 4] = 0; ci[0x58 / 4] = 0; ci[0x5C / 4] = 0;
        cdi[0] = rng() & ~8u;  // class-5 corners through the matrix
        const float m[12] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
        for (int k = 0; k < 12; ++k) cdi[0x30 / 4 + k] = bits(m[k]);
        const float half = kind == 3 ? fr(cell, 3 * cell) : fr(1, cell / 3);  // 3: spans several cells
        const float cx = fr(ox - cell, ox + ex + cell), cz = fr(oz + ez - cell, oz + cell), cy = fr(-50, 50);
        const float box[6] = {cx - half, cy - 5, cz - half, cx + half, cy + 5, cz + half};
        for (int k = 0; k < 6; ++k) ci[0x74 / 4 + k] = bits(box[k]);
        const bool move = f == 1 && rng() % 4 == 0;
        const int mx = static_cast<int>(rng() % nx), mz = static_cast<int>(rng() % nz);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(world, wi, sizeof world);
            std::memcpy(data, di, sizeof data);
            std::memcpy(child, ci, sizeof child);
            std::memcpy(cdata, cdi, sizeof cdata);
            world[0x38 / 4] = addr(data);
            child[0x38 / 4] = addr(cdata);
            build[side](world, 0, cell, -cell);
            int r = attach[side](world, child);
            snap[side].push_back(static_cast<std::uint32_t>(r));
            if (f == 1) {
                if (move && static_cast<int>(child[0x4C / 4]) >= 0) { child[0x4C / 4] = static_cast<std::uint32_t>(mx); child[0x50 / 4] = static_cast<std::uint32_t>(mz); }
                r = detach[side](world, child);
                snap[side].push_back(static_cast<std::uint32_t>(r));
            }
            const int gx = static_cast<int>(data[0x78 / 4]), gz = static_cast<int>(data[0x7C / 4]);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            role[addr(world)] = 0xA0; role[addr(child)] = 0xA1; role[addr(data)] = 0xA2;
            for (int rr = 0; rr < gz; ++rr) for (int c = 0; c < gx; ++c) role[at(data[0x80 / 4])[rr] + 64u * c] = 0x1000u + 16 * rr + c;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            for (int k = 0; k < 64; ++k) if (k != 3 && k != 0x80 / 4) snap[side].push_back(data[k]);
            for (std::uint32_t k = 0; k < data[1] && k < 64; ++k) snap[side].push_back(rl(at(data[3])[k]));
            for (int rr = 0; rr < gz; ++rr)
                for (int c = 0; c < gx; ++c) {
                    const std::uint32_t* cw = at(at(data[0x80 / 4])[rr] + 64u * c);
                    for (int k = 0; k < 15; ++k) snap[side].push_back(cw[k]);
                    const int n = reinterpret_cast<const std::int16_t*>(cw)[0x3a / 2];
                    for (int k = 0; k < n && k < 16; ++k) snap[side].push_back(rl(at(cw[15])[k]));
                }
            for (int k = 0; k < 49; ++k) if (k != 0x38 / 4 && k != 0x60 / 4) snap[side].push_back(world[k]);
            for (std::uint32_t k = 0; k < world[0x5C / 4] && k < 16; ++k) snap[side].push_back(rl(at(world[0x60 / 4])[k]));
            for (int k = 0; k < 49; ++k) if (k != 0x38 / 4 && k != 0x58 / 4) snap[side].push_back(child[k]);
            for (std::uint32_t k = 0; k < child[0x54 / 4] && k < 16; ++k) snap[side].push_back(rl(at(child[0x58 / 4])[k]));
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0 && f == 0 && static_cast<int>(child[0x4C / 4]) >= 0) ++celled;
            for (int rr = 0; rr < gz; ++rr) {
                for (int c = 0; c < gx; ++c) real_free(at(at(data[0x80 / 4])[rr] + 64u * c)[15]);
                real_free(at(data[0x80 / 4])[rr]);
            }
            real_free(data[0x80 / 4]); real_free(data[3]); real_free(world[0x60 / 4]); real_free(child[0x58 / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++per[f];
        ++compared;
    }
    rt::restore_pristine();
    CHECK(celled > 300);
    std::printf("  World_AttachChild calls %d (%d into a cell), then World_DetachChild calls %d (grids up to 6 x 6, moved cells)\n", per[0], celled, per[1]);
}
