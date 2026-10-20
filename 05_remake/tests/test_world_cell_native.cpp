// Structured native L1 for the cls_world cell functions (the arena fuzz would feed them random grids and arrays):
// World_RefreshCell (0x00450a70: ECX world, EDX cellX, stack cellZ; ret 4): null world / class data -> report, 5; a
// cell whose flags +0 lack bit 0 is queued (World_QueueDirtyCell 0x00450030 with the world data as the queue), else 0.
// World_InsertChildInCell (0x00450f60: ECX world, EDX child, stack cellX, cellZ; ret 8): a cell holding 0x7fff children
// counts as none; no cell -> the child joins the world's own children (+0x5c / +0x60) with cell (-1, -1), else the
// cell's array (+0x3a short count, +0x3c) with its cell indices at +0x4c / +0x50; the world joins the child's parents
// (+0x54 / +0x58), and a second parent clears 0x80000 through the child's subtree (gwNodeSetSubtreeFlag80000); a cell
// without bit 0 is then queued. All arrays are grown with realloc (0x004cc4ec).
// Each side: its own world (grid of 1..4 x 1..4 cells, flags random, 0..2 children per cell or 0x7fff, world children
// 0..2), queue (0..3 entries, full or one spare) and child (0..2 parents, one grandchild) in real msvcrt blocks.
// Compared by role: returns, every node / data / cell word with the array pointers replaced by their contents, the
// grandchild's flags, list 7 and the link counter.
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
std::uint32_t real_malloc(std::size_t n)
{
    return addr(reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"))(n));
}
void real_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(at(p)); }
std::uint32_t array_of(const std::vector<std::uint32_t>& v, std::size_t cap)
{
    if (!cap) return 0;
    const std::uint32_t a = real_malloc(4 * cap);
    for (std::size_t k = 0; k < v.size(); ++k) at(a)[k] = v[k];
    return a;
}
}  // namespace

TEST(native_world_cell_functions_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Refresh = int(__fastcall*)(void*, int, int);
    using Insert = int(__fastcall*)(void*, void*, int, int);
    const Refresh refresh[2] = {rt::original<Refresh>(0x00450a70), reinterpret_cast<Refresh>(&recoil::World_RefreshCell)};
    const Insert insert[2] = {rt::original<Insert>(0x00450f60), reinterpret_cast<Insert>(&recoil::World_InsertChildInCell)};
    std::mt19937 rng(0x450a70);
    int compared = 0, per[2] = {};
    static std::uint32_t world[49], data[64], child[49], grand[49];
    for (int it = 0; it < 8000; ++it) {
        const int f = it % 2;
        const int gx = 1 + static_cast<int>(rng() % 4), gz = 1 + static_cast<int>(rng() % 4);
        std::uint32_t world_init[49], data_init[64], child_init[49], grand_init[49];
        for (auto& w : world_init) w = rng();
        for (auto& w : data_init) w = rng();
        for (auto& w : child_init) w = rng();
        for (auto& w : grand_init) w = rng();
        world_init[0x24 / 4] &= ~1u;
        if (rng() % 3 == 0) world_init[0x24 / 4] |= 1u;
        world_init[0x54 / 4] = 0;  // no parents: a list-7 push does not recurse
        const std::uint32_t wkids = rng() % 3, qcount = rng() % 4, qcap = qcount + (rng() % 2 ? 0u : 1u);
        const std::uint32_t cparents = rng() % 3;
        std::vector<std::uint32_t> cell_init(16 * gx * gz);
        std::vector<int> cell_n(gx * gz);
        for (int c = 0; c < gx * gz; ++c) {
            for (int w = 0; w < 16; ++w) cell_init[16 * c + w] = rng();
            cell_init[16 * c] = (cell_init[16 * c] & ~1u) | (rng() % 2 ? 1u : 0u);
            cell_n[c] = rng() % 8 == 0 ? 0x7fff : rng() % 8 == 0 ? 0x7ffe : static_cast<int>(rng() % 3);  // full, one short of full
        }
        const bool null_world = rng() % 40 == 0, null_data = f == 0 && rng() % 40 == 0;  // InsertChildInCell has no class-data check
        const int x = f == 0 ? static_cast<int>(rng() % gx) : static_cast<int>(rng() % (gx + 1)) - 1;
        const int z = f == 0 ? static_cast<int>(rng() % gz) : static_cast<int>(rng() % (gz + 1)) - 1;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(world, world_init, sizeof world);
            std::memcpy(data, data_init, sizeof data);
            std::memcpy(child, child_init, sizeof child);
            std::memcpy(grand, grand_init, sizeof grand);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            role[addr(world)] = 0xA0; role[addr(child)] = 0xA1; role[addr(grand)] = 0xA2;
            world[0x38 / 4] = null_data ? 0u : addr(data);
            std::vector<std::uint32_t> wk;
            for (std::uint32_t k = 0; k < wkids; ++k) wk.push_back(0xE0000000u + k);
            world[0x5C / 4] = wkids; world[0x60 / 4] = array_of(wk, wkids);
            std::vector<std::uint32_t> qe;
            for (std::uint32_t k = 0; k < qcount; ++k) qe.push_back(0xE1000000u + k);
            data[1] = qcount; data[2] = qcap; data[3] = array_of(qe, qcap ? qcap : 1);
            std::vector<std::uint32_t> rows(gz);
            std::vector<std::vector<std::uint32_t>> cells(gz, std::vector<std::uint32_t>(16 * gx));
            for (int r = 0; r < gz; ++r) {
                for (int c = 0; c < gx; ++c) {
                    std::uint32_t* cw = &cells[r][16 * c];
                    std::memcpy(cw, &cell_init[16 * (r * gx + c)], 64);
                    const int n = cell_n[r * gx + c];
                    reinterpret_cast<std::int16_t*>(cw)[0x3a / 2] = static_cast<std::int16_t>(n);
                    std::vector<std::uint32_t> ce;
                    for (int k = 0; k < (n >= 0x7ffe ? 0 : n); ++k) ce.push_back(0xE2000000u + 16 * (r * gx + c) + k);
                    cw[0x3c / 4] = array_of(ce, ce.size());
                    role[addr(cw)] = 0x1000u + r * 16 + c;
                }
                rows[r] = addr(cells[r].data());
            }
            data[0x80 / 4] = addr(rows.data());
            std::vector<std::uint32_t> cp;
            for (std::uint32_t k = 0; k < cparents; ++k) cp.push_back(0xE3000000u + k);
            child[0x54 / 4] = cparents; child[0x58 / 4] = array_of(cp, cparents);
            std::uint32_t gkids[1] = {addr(grand)};
            child[0x5C / 4] = 1; child[0x60 / 4] = addr(gkids);
            grand[0x5C / 4] = 0;
            grand[0x54 / 4] = 1;
            int r;
            if (f == 0) r = refresh[side](null_world ? nullptr : world, x, z);
            else r = insert[side](world, child, x, z);
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            auto arr = [&](std::uint32_t a, std::uint32_t n) { snap[side].push_back(a ? 1u : 0u); for (std::uint32_t k = 0; a && k < n && k < 64; ++k) snap[side].push_back(rl(at(a)[k])); };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (int k = 0; k < 49; ++k) if (k != 0x38 / 4 && k != 0x60 / 4) snap[side].push_back(world[k]);
            arr(world[0x60 / 4], world[0x5C / 4]);
            for (int k = 0; k < 64; ++k) if (k != 3 && k != 0x80 / 4) snap[side].push_back(data[k]);
            arr(data[3], data[1]);
            for (int rr = 0; rr < gz; ++rr)
                for (int c = 0; c < gx; ++c) {
                    const std::uint32_t* cw = &cells[rr][16 * c];
                    for (int k = 0; k < 15; ++k) snap[side].push_back(cw[k]);
                    const int n = reinterpret_cast<const std::int16_t*>(cw)[0x3a / 2];
                    if (n >= 0x7ffe) {  // a near-full cell starts with no array: only the appended last entry is defined
                        snap[side].push_back(cw[0x3c / 4] ? 1u : 0u);
                        if (cw[0x3c / 4] && n == 0x7fff && cell_n[rr * gx + c] == 0x7ffe) snap[side].push_back(rl(at(cw[0x3c / 4])[n - 1]));
                    } else {
                        arr(cw[0x3c / 4], static_cast<std::uint32_t>(n));
                    }
                }
            for (int k = 0; k < 49; ++k) if (k != 0x58 / 4 && k != 0x60 / 4) snap[side].push_back(child[k]);
            arr(child[0x58 / 4], child[0x54 / 4]);
            snap[side].push_back(grand[0x24 / 4]);
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            snap[side].push_back(*img(side, 0x00539c74));
            // free every live block once (realloc may have moved or kept each)
            std::vector<std::uint32_t> live = {world[0x60 / 4], data[3], child[0x58 / 4]};
            for (int rr = 0; rr < gz; ++rr) for (int c = 0; c < gx; ++c) live.push_back(cells[rr][16 * c + 15]);
            for (std::uint32_t b : live) if (b) real_free(b);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++per[f];
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  World_RefreshCell calls %d, World_InsertChildInCell calls %d (grids 1..4 x 1..4, full and one-short cells, -1 indices)\n", per[0], per[1]);
}
