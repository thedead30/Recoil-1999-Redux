// Native L1 for Container_FindFirstByPredicate (0x0048cbd0): the arena fuzz cannot supply the predicate it calls
// (CALL EDI), so each call gets a real circular list (list +0 non-empty flag, +0x10 head; node +0 payload, +4 next;
// 0..9 nodes, a null list, or an empty one) and a real __fastcall predicate(payload ECX, key EDX) that logs each
// call and returns 0 on a match (the listing's loop continues while it returns nonzero). Compared: return value
// and the whole sequence of predicate calls (payload, key).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zReader/zreader.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <utility>
#include <vector>

namespace {
std::vector<std::pair<std::uint32_t, std::uint32_t>> g_calls;
int __fastcall predicate(std::uint32_t payload, std::uint32_t key)
{
    g_calls.push_back({payload, key});
    return payload != key;
}
}  // namespace

TEST(native_container_find_first_by_predicate_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Find = std::uint32_t(__fastcall*)(void*, void*, std::uint32_t);
    // the function itself, then Container_FindFirstByPredicateThunk (0x0048cc50), which forwards to it
    const std::uint32_t vas[2] = {0x0048cbd0, 0x0048cc50};
    const Find ports[2] = {reinterpret_cast<Find>(&recoil::Container_FindFirstByPredicate),
                           reinterpret_cast<Find>(&recoil::Container_FindFirstByPredicateThunk)};
    int compared = 0;
    for (int fn = 0; fn < 2; ++fn) {
    auto orig = rt::original<Find>(vas[fn]);
    auto port = ports[fn];
    std::mt19937 rng(vas[fn]);
    for (int it = 0; it < 3000; ++it) {
        const int n = static_cast<int>(rng() % 10);
        std::uint32_t list[8] = {};
        std::vector<std::uint32_t> nodes(2 * (n ? n : 1));
        for (int i = 0; i < n; ++i) {
            nodes[2 * i] = rng() % 6;  // few distinct payloads: matches, repeats and misses
            nodes[2 * i + 1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&nodes[2 * ((i + 1) % n)]));
        }
        list[0] = n ? 1 + rng() % 3 : 0;
        list[4] = n ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&nodes[0])) : 0;
        const std::uint32_t key = rng() % 7;
        void* lp = (rng() % 16 == 0) ? nullptr : list;
        std::uint32_t ret[2];
        std::vector<std::pair<std::uint32_t, std::uint32_t>> calls[2];
        for (int side = 0; side < 2; ++side) {
            g_calls.clear();
            ret[side] = (side ? port : orig)(lp, reinterpret_cast<void*>(&predicate), key);
            calls[side] = g_calls;
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(calls[0] == calls[1]);
        ++compared;
    }
    }
    std::printf("  Container_FindFirstByPredicate and Thunk calls %d (circular lists of 0..9 nodes)\n", compared);
}
