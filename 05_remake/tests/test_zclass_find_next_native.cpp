// Structured native L1 for NodeList_FindNext (0x0044f690; ECX key, EDX list, one stack argument: a predicate; P2.6).
// A resumable search: ECX != 0 starts it - key [0x00539ba0] = ECX, cursor [0x00539b98] = NodeList_GetTail(EDX) (the
// link the list's holder [0x004ddef8 + 4 * list] points at) - and returns 0. ECX == 0 continues from the cursor: for each
// link (cursor advanced to link +8 first) the predicate is called with ECX = the link's node; non-zero -> that node is
// returned (the cursor already past it, so the next call resumes after it); cursor exhausted -> 0.
// Each run: one list (0..15) of 0..5 links over node records (some repeated), a start call, then continue calls until
// one returns 0 (at most 8); 1 run in 10 continues without starting (the cursor, cleared after every run, is null:
// 0 at once). The
// predicate logs its node and the key it sees in [0x00539ba0] and accepts a random subset of the nodes. Compared by
// role: every return, the predicate log, and the key and cursor after each call.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/zclass_nodes.h"
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
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }

int g_side = 0;
std::vector<std::uint32_t> g_log;
std::vector<std::uint32_t> g_accept;  // node addresses the predicate accepts
int __fastcall predicate(void* node, void*)
{
    g_log.push_back(addr(node));
    g_log.push_back(*img(g_side, 0x00539ba0));
    for (std::uint32_t a : g_accept) if (a == addr(node)) return static_cast<int>(1 + (a & 2));  // any non-zero accepts
    return 0;
}
}  // namespace

TEST(native_zclass_node_list_find_next_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, int, void*);
    const Fn fn[2] = {rt::original<Fn>(0x0044f690), reinterpret_cast<Fn>(&recoil::NodeList_FindNext)};
    std::mt19937 rng(0x44f690);
    int compared = 0, found = 0;
    static std::uint32_t nodes[4][8];
    static std::uint32_t links[2][5][4];
    for (int it = 0; it < 4000; ++it) {
        const int list = static_cast<int>(rng() % 16), len = static_cast<int>(rng() % 6);
        int which[5];
        for (int& w : which) w = static_cast<int>(rng() % 4);
        bool accept[4];
        for (bool& a : accept) a = rng() % 3 == 0;
        const bool start = rng() % 10 != 0;
        const std::uint32_t key = 0x100u + rng() % 64;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            g_side = side;  // the key [0x00539ba0] carries over from the previous run, the same on both sides
            std::uint32_t (*l)[4] = links[side];
            for (int e = 0; e < 5; ++e) {
                l[e][0] = addr(nodes[which[e]]);
                l[e][1] = e ? addr(l[e - 1]) : 0u;
                l[e][2] = e + 1 < len ? addr(l[e + 1]) : 0u;
                l[e][3] = 0;
            }
            // save and replace this list's head (restored below: other tests expect pristine lists)
            std::uint32_t* holder = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(*img(side, 0x004ddef8u + 4 * list)));
            const std::uint32_t saved_head = *holder;
            *holder = len ? addr(l[0]) : 0u;
            g_accept.clear();
            for (int k = 0; k < 4; ++k) if (accept[k]) g_accept.push_back(addr(nodes[k]));
            g_log.clear();
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int k = 0; k < 4; ++k) role[addr(nodes[k])] = 0xA0u + k;
            for (int e = 0; e < 5; ++e) role[addr(l[e])] = 0xB0u + e;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : v; };
            if (start) {
                snap[side].push_back(fn[side](key, list, reinterpret_cast<void*>(&predicate)));
                snap[side].push_back(*img(side, 0x00539ba0));
                snap[side].push_back(rl(*img(side, 0x00539b98)));
            }
            for (int call = 0; call < 8; ++call) {
                const std::uint32_t r = fn[side](0, 0x55, reinterpret_cast<void*>(&predicate));  // EDX not read
                snap[side].push_back(rl(r));
                snap[side].push_back(*img(side, 0x00539ba0));
                snap[side].push_back(rl(*img(side, 0x00539b98)));
                if (side == 0 && r) ++found;
                if (!r) break;
            }
            for (std::uint32_t v : g_log) snap[side].push_back(rl(v));
            *holder = saved_head;
            *img(side, 0x00539b98) = 0;  // never left pointing at links that are about to be rebuilt
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  NodeList_FindNext runs %d, %d nodes found\n", compared, found);
    CHECK(found > 1000);
}
