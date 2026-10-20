// Structured native L1 for NodeList_PushHead (0x0044ee10, ECX list 0..15, EDX node). NodeList_AllocLink (0x0044e630)
// counts the link ([0x00539c74], maximum [0x00539c78]) and pops the free link [0x00539c6c] (next at +8) or callocs
// 0x10 bytes; the link gets the node at +0 and is appended after the tail: an empty list (head holder
// [0x004ddef8 + 4*list] -> 0) gets head = tail = link, else link+4 = tail, tail+8 = link, tail = link (tail holder
// [0x004ddf38 + 4*list]) - so despite the name it links at the tail. For list 7 the node's +0x24 bit 0 is set and every
// entry of its +0x54 / +0x58 array that lacks the bit and is not class 2 (+0x34) is pushed the same way (recursion; the
// bit is set before recursing, so cycles end). The arena fuzz fed it random arrays. Each side: its own lists (0..3
// links), free-link pool (0..2 links, or empty so calloc is used) and a graph of 1..6 nodes with 0..3 references each
// (cycles allowed). Compared by role: the return, the counters, the free-list head, every list's head / tail and link
// chain {node, prev, next, +0xC}, and every node's flags word.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/scene_update.h"
#include "platform/image/original_data.h"

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

namespace {
using Fn = int(__fastcall*)(int, void*);
void run_push(const Fn fn[2], std::uint32_t seed, const char* name)
{
    std::mt19937 rng(seed);
    int compared = 0, recursed = 0;
    for (int it = 0; it < 6000; ++it) {
        const int list = rng() % 2 ? 7 : static_cast<int>(rng() % 16);
        const int n_nodes = 1 + static_cast<int>(rng() % 6), n_free = static_cast<int>(rng() % 3);
        int len[16];
        for (auto& l : len) l = static_cast<int>(rng() % 4);
        std::uint32_t count0 = rng() % 50, max0 = count0 + rng() % 3;
        std::vector<std::uint32_t> flags(n_nodes), cls(n_nodes), filler(64);
        std::vector<std::vector<int>> refs(n_nodes);
        for (int i = 0; i < n_nodes; ++i) {
            flags[i] = (rng() & ~1u) | (i && rng() % 3 == 0 ? 1u : 0u);
            cls[i] = rng() % 4;
            const int k = static_cast<int>(rng() % 4);
            for (int r = 0; r < k; ++r) refs[i].push_back(static_cast<int>(rng() % n_nodes));
        }
        for (auto& w : filler) w = rng() | 0x80000000u;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::vector<std::uint32_t>> node(n_nodes, std::vector<std::uint32_t>(0x60 / 4));
            std::vector<std::vector<std::uint32_t>> arr(n_nodes);
            for (int i = 0; i < n_nodes; ++i) {
                for (int w = 0; w < 0x60 / 4; ++w) node[i][w] = filler[w];
                node[i][0x24 / 4] = flags[i];
                node[i][0x34 / 4] = cls[i];
                for (int r : refs[i]) arr[i].push_back(0);
            }
            for (int i = 0; i < n_nodes; ++i) {
                for (std::size_t r = 0; r < refs[i].size(); ++r) arr[i][r] = addr(node[refs[i][r]].data());
                node[i][0x54 / 4] = static_cast<std::uint32_t>(refs[i].size());
                node[i][0x58 / 4] = arr[i].empty() ? 0u : addr(arr[i].data());
            }
            std::uint32_t links[16][3][4], pool[2][4];
            for (int l = 0; l < 16; ++l) {
                for (int e = 0; e < 3; ++e) {
                    links[l][e][0] = filler[l * 3 + e];
                    links[l][e][1] = e ? addr(links[l][e - 1]) : 0u;
                    links[l][e][2] = e + 1 < len[l] ? addr(links[l][e + 1]) : 0u;
                    links[l][e][3] = filler[l];
                }
                *at(*img(side, 0x004ddef8u + 4 * l)) = len[l] ? addr(links[l][0]) : 0u;
                *at(*img(side, 0x004ddf38u + 4 * l)) = len[l] ? addr(links[l][len[l] - 1]) : 0u;
            }
            for (int e = 0; e < 2; ++e) {
                pool[e][0] = filler[40 + e]; pool[e][1] = filler[42 + e];
                pool[e][2] = e + 1 < n_free ? addr(pool[e + 1]) : 0u;
                pool[e][3] = filler[44 + e];
            }
            *img(side, 0x00539c6c) = n_free ? addr(pool[0]) : 0u;
            *img(side, 0x00539c74) = count0;
            *img(side, 0x00539c78) = max0;
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int i = 0; i < n_nodes; ++i) role[addr(node[i].data())] = 0x100u + i;
            for (int l = 0; l < 16; ++l) for (int e = 0; e < 3; ++e) role[addr(links[l][e])] = 0x200u + 3 * l + e;
            for (int e = 0; e < 2; ++e) role[addr(pool[e])] = 0x300u + e;
            std::uint32_t fresh = 0xC000;  // calloc'd links, numbered in order of first sight
            auto r = [&](std::uint32_t v) { auto f = role.find(v); return f != role.end() ? f->second : (role[v] = fresh++); };
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](list, node[0].data())));
            snap[side].push_back(*img(side, 0x00539c74));
            snap[side].push_back(*img(side, 0x00539c78));
            snap[side].push_back(r(*img(side, 0x00539c6c)));
            for (int l = 0; l < 16; ++l) {
                std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
                snap[side].push_back(r(p));
                snap[side].push_back(r(*at(*img(side, 0x004ddf38u + 4 * l))));
                for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) {
                    snap[side].push_back(r(at(p)[0])); snap[side].push_back(r(at(p)[1]));
                    snap[side].push_back(r(at(p)[2])); snap[side].push_back(at(p)[3]);
                }
            }
            for (int i = 0; i < n_nodes; ++i) snap[side].push_back(node[i][0x24 / 4]);
            if (side == 0 && *img(0, 0x00539c74) > count0 + 1) ++recursed;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    CHECK(recursed > 500);
    std::printf("  %s calls %d, %d recursed (lists 0..15, 0..3 links, pool or calloc, node graphs)\n", name, compared, recursed);
}
}  // namespace

TEST(native_node_list_push_head_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const Fn fn[2] = {rt::original<Fn>(0x0044ee10), reinterpret_cast<Fn>(&recoil::NodeList_PushHead)};
    run_push(fn, 0x44ee10, "NodeList_PushHead");
}

// NodeRegistry_Insert (0x0044ed90): the same, but the link goes in at the other end - after the link the head holder
// [0x004ddef8 + 4*list] points at (link+8 = it, it+4 = link; an empty list also sets the tail holder), and the holder
// then points at the new link; list 7 recurses through NodeList_PushHead as above.
TEST(native_node_registry_insert_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const Fn fn[2] = {rt::original<Fn>(0x0044ed90), reinterpret_cast<Fn>(&recoil::NodeRegistry_Insert)};
    run_push(fn, 0x44ed90, "NodeRegistry_Insert");
}
