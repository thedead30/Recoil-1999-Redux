// Native L1 for the zReader node pool as a sequence: Container_InitNodePool (0x0048c7d0, count ECX) creates the pool
// list in [0x0056ae70] when it is null and grows it by count 12-byte nodes; Container_AllocNodeWithPayload
// (0x0048ca10, payload ECX) takes a node (growing the pool when empty) and stores the payload; Container_FreeNode
// (0x0048cae0, node ECX) returns the node to the pool and its payload. Counters: [0x0056ae74] nodes allocated,
// [0x0056ae78] nodes free, [0x0056ae7c] grow count. AllocNodeWithPayload cannot be fuzzed alone (tests/arena_fuzz.h):
// its pool globals are callee-only and their pristine value (a null pool) faults every call.
// Each side starts pristine, initialises with 0..5 nodes, then runs 60 random alloc / free steps. After each step:
// return value (nodes by order of first appearance on that side), counters, and the pool's free list (walked from its
// head +0x10 through the circular +4 links, nodes by id).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zReader/zreader.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <map>
#include <random>
#include <vector>

namespace {
std::uint32_t* global(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }

struct Ids {
    std::map<std::uint32_t, std::uint32_t> id;
    std::uint32_t of(std::uint32_t p)
    {
        if (!p) return 0;
        auto it = id.find(p);
        if (it != id.end()) return it->second;
        const auto n = static_cast<std::uint32_t>(0x1D000000u + id.size());
        id[p] = n;
        return n;
    }
};

std::vector<std::uint32_t> state(int side, Ids& ids)
{
    std::vector<std::uint32_t> s = {*global(side, 0x0056ae70) ? 1u : 0u, *global(side, 0x0056ae74), *global(side, 0x0056ae78),
                                    *global(side, 0x0056ae7c)};
    const std::uint32_t pool = *global(side, 0x0056ae70);
    if (!pool) return s;
    const auto* list = ptr<std::uint32_t>(pool);
    s.push_back(list[0]);  // free count kept in the list
    std::uint32_t n = list[4];
    for (int k = 0; n && k < 256; ++k) {
        s.push_back(ids.of(n));
        n = ptr<std::uint32_t>(n)[1];
        if (n == list[4]) break;
    }
    return s;
}
}  // namespace

TEST(native_node_pool_sequence_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, int);
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn alloc[2] = {rt::original<Fn>(0x0048ca10), reinterpret_cast<Fn>(&recoil::Container_AllocNodeWithPayload)};
    const Fn release[2] = {rt::original<Fn>(0x0048cae0), reinterpret_cast<Fn>(&recoil::Container_FreeNode)};
    std::mt19937 rng(0x48ca10);
    int compared = 0;
    for (int it = 0; it < 300; ++it) {
        const std::uint32_t seed = rng();
        std::vector<std::uint32_t> trace[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::mt19937 ops(seed);
            Ids ids;
            std::vector<std::uint32_t> live;
            init[side](ops() % 6, 0);
            const std::vector<std::uint32_t> s0 = state(side, ids);
            trace[side].insert(trace[side].end(), s0.begin(), s0.end());
            for (int step = 0; step < 60; ++step) {
                if (live.empty() || ops() % 3) {
                    const std::uint32_t payload = ops();
                    const std::uint32_t node = alloc[side](payload, 0);
                    trace[side].push_back(ids.of(node));
                    trace[side].push_back(node ? ptr<std::uint32_t>(node)[0] : 0);
                    if (node) live.push_back(node);
                } else {
                    const std::size_t k = ops() % live.size();
                    trace[side].push_back(release[side](live[k], 0));
                    live.erase(live.begin() + static_cast<std::ptrdiff_t>(k));
                }
                const std::vector<std::uint32_t> s = state(side, ids);
                trace[side].insert(trace[side].end(), s.begin(), s.end());
            }
        }
        CHECK(trace[0] == trace[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Container_InitNodePool/AllocNodeWithPayload/FreeNode sequences %d (60 steps each)\n", compared);
}
