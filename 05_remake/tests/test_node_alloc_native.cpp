// Structured native L1 for gwNodeAlloc (0x004478c0, no arguments): the free index [0x004de4c8] (-1 -> report, return
// 0) selects a 0xC4-byte record of the pool [0x00539c94]; the next free index is the sign-extended low 24 bits of the
// record's +0xC0; the record's first 0xC0 bytes are zeroed, the count [0x00539c98] goes up, the node is registered on
// list 6 (NodeRegistry_Insert), gets the defaults (+0x24 0x0108001c, +0x44 1, +0x4c / +0x50 -1, byte +0x30 0xff),
// the name 'Default_node_name' through sprintf("%s") (0x004cc5c4) and +0xbc 0. Each side: its own pool of 2..8 records
// (random words, a free chain through some of them in random order, high bytes of +0xC0 random), its own list 6
// (0..2 links). Compared: the return as a record index, the free index, the count, every pool word and list 6 by role.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Class.h"
#include "platform/image/original_data.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
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
}  // namespace

TEST(native_gw_node_alloc_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(int, int);
    const Fn fn[2] = {rt::original<Fn>(0x004478c0), reinterpret_cast<Fn>(&recoil::gwNodeAlloc)};
    std::mt19937 rng(0x4478c0);
    int compared = 0, allocated = 0;
    for (int it = 0; it < 4000; ++it) {
        const int n = 2 + static_cast<int>(rng() % 7);
        std::vector<std::uint32_t> pool_init(49 * n);
        for (auto& w : pool_init) w = rng();
        std::vector<int> order(n);
        for (int i = 0; i < n; ++i) order[i] = i;
        std::shuffle(order.begin(), order.end(), rng);
        const int chain = static_cast<int>(rng() % (n + 1));  // records on the free list (0 = empty)
        for (int k = 0; k < chain; ++k) {
            const std::uint32_t next = k + 1 < chain ? static_cast<std::uint32_t>(order[k + 1]) : 0xFFFFFFu;
            std::uint32_t& w = pool_init[49 * order[k] + 0xC0 / 4];
            w = (w & 0xFF000000u) | next;
        }
        const std::uint32_t head = chain ? static_cast<std::uint32_t>(order[0]) : 0xFFFFFFFFu;
        const std::uint32_t count0 = rng() % 100;
        const int len6 = static_cast<int>(rng() % 3);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> pool = pool_init;
            std::uint32_t links[2][4];
            for (int e = 0; e < 2; ++e) {
                links[e][0] = 0xD0000000u + e;
                links[e][1] = e ? addr(links[e - 1]) : 0u;
                links[e][2] = e + 1 < len6 ? addr(links[e + 1]) : 0u;
                links[e][3] = 0;
            }
            *at(*img(side, 0x004ddef8u + 4 * 6)) = len6 ? addr(links[0]) : 0u;
            *at(*img(side, 0x004ddf38u + 4 * 6)) = len6 ? addr(links[len6 - 1]) : 0u;
            *img(side, 0x00539c94) = addr(pool.data());
            *img(side, 0x004de4c8) = head;
            *img(side, 0x00539c98) = count0;
            const std::uint32_t r = fn[side](0, 0);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int i = 0; i < n; ++i) role[addr(&pool[49 * i])] = 0x100u + i;
            for (int e = 0; e < 2; ++e) role[addr(links[e])] = 0x200u + e;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto x = role.find(v); return x != role.end() ? x->second : (role[v] = fresh++); };
            snap[side].push_back(rl(r));
            snap[side].push_back(*img(side, 0x004de4c8));
            snap[side].push_back(*img(side, 0x00539c98));
            snap[side].insert(snap[side].end(), pool.begin(), pool.end());
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 6));
            snap[side].push_back(rl(p));
            snap[side].push_back(rl(*at(*img(side, 0x004ddf38u + 4 * 6))));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) {
                snap[side].push_back(rl(at(p)[0])); snap[side].push_back(rl(at(p)[1]));
                snap[side].push_back(rl(at(p)[2])); snap[side].push_back(at(p)[3]);
            }
            snap[side].push_back(*img(side, 0x00539c74));
        }
        CHECK_SNAP(snap[0], snap[1]);
        allocated += chain > 0;
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  gwNodeAlloc calls %d, %d allocated (pools of 2..8 records, shuffled free chains, empty pool)\n", compared, allocated);
}
