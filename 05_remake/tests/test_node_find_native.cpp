// Structured native L1 for gwNodeFindByName (0x00447bc0, ECX node, EDX name): null -> report, 0; the node when its
// inline name (+0) equals the string (inlined strcmp); else the first match depth-first through the children
// (+0x5C count, +0x60 array). The arena fuzz fed it random child counts and pointers - unbounded recursion that crashed
// the shard under full-suite load (KG-31, twice) - so it is tested here on real trees (depth <= 4, 0..3 children,
// names from a small pool with repeats and case variants) and dropped from native_fuzz_scene3. Compared: the result
// as a node index.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Class.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
struct Tree {
    std::vector<std::vector<std::uint32_t>> nodes;  // 0xC4 bytes each
    std::vector<std::vector<std::uint32_t>> kids;   // child arrays
};
}  // namespace

TEST(native_gw_node_find_by_name_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, const char*);
    const Fn fn[2] = {rt::original<Fn>(0x00447bc0), reinterpret_cast<Fn>(&recoil::gwNodeFindByName)};
    const char* names[] = {"root", "tank", "Tank", "turret", "wheel", "", "tankx"};
    std::mt19937 rng(0x447bc0);
    int compared = 0, found = 0;
    for (int it = 0; it < 3000; ++it) {
        // shape: node 0 is the root; each node gets 0..3 children while the depth allows
        std::vector<int> parent{-1}, depth{0}, name{static_cast<int>(rng() % 7)};
        for (std::size_t i = 0; i < parent.size() && parent.size() < 40; ++i) {
            if (depth[i] >= 4) continue;
            const int k = static_cast<int>(rng() % 4);
            for (int c = 0; c < k; ++c) { parent.push_back(static_cast<int>(i)); depth.push_back(depth[i] + 1); name.push_back(static_cast<int>(rng() % 7)); }
        }
        const char* q = rng() % 6 == 0 ? "missing" : names[rng() % 7];
        const bool null_node = rng() % 30 == 0;
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) {
            Tree t;
            t.nodes.assign(parent.size(), std::vector<std::uint32_t>(49, 0));
            t.kids.assign(parent.size(), {});
            for (std::size_t i = 0; i < parent.size(); ++i) std::strcpy(reinterpret_cast<char*>(t.nodes[i].data()), names[name[i]]);
            for (std::size_t i = 1; i < parent.size(); ++i) t.kids[parent[i]].push_back(addr(t.nodes[i].data()));
            for (std::size_t i = 0; i < parent.size(); ++i) {
                t.nodes[i][0x5C / 4] = static_cast<std::uint32_t>(t.kids[i].size());
                t.nodes[i][0x60 / 4] = t.kids[i].empty() ? 0u : addr(t.kids[i].data());
            }
            const std::uint32_t r = fn[side](null_node ? nullptr : t.nodes[0].data(), q);
            ret[side] = 0xFFFFFFFFu;
            for (std::size_t i = 0; i < parent.size(); ++i) if (r == addr(t.nodes[i].data())) ret[side] = static_cast<std::uint32_t>(i);
            if (r && ret[side] == 0xFFFFFFFFu) ret[side] = 0xBADu;
        }
        CHECK_EQ(ret[0], ret[1]);
        found += ret[0] != 0xFFFFFFFFu;
        ++compared;
    }
    CHECK(found > 500);
    std::printf("  gwNodeFindByName calls %d, %d found (trees of depth <= 4, repeated / case-variant names)\n", compared, found);
}
