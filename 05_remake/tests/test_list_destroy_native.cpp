// Native L1 for List_DestroyNodes (0x00403db0, list at ECX): std::list tidy - every node after the sentinel [+4]
// (node {next, prev, ...}) is unlinked and deleted (operator delete), decrementing the count [+8]; then the sentinel
// is deleted and [+4] / [+8] are zeroed. Each side builds the same circular list of 0..6 real heap nodes (and a count
// that may disagree with the length); operator delete reaches the msvcrt free slot on both sides (platform/mfc42),
// which is logged with blocks by role. Compared: the free log, the list words afterwards.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/savegame.h"
#include "platform/iat_msvcrt.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

namespace {
std::vector<std::uint32_t> g_frees, g_nodes;
void __cdecl logging_free(void* p)
{
    const auto a = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
    std::uint32_t r = a;
    for (std::size_t k = 0; k < g_nodes.size(); ++k) if (g_nodes[k] == a) r = 0xA0000000u + static_cast<std::uint32_t>(k);
    g_frees.push_back(r);
    reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(p);
}
std::uint32_t real_malloc(std::size_t n)
{
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(
        reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"))(n)));
}
}  // namespace

TEST(native_list_destroy_nodes_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00403db0), reinterpret_cast<Fn>(&recoil::List_DestroyNodes)};
    void* const saved = recoil::g_Iat_free_004cc5b4;
    recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    std::mt19937 rng(0x403db0);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const int n = static_cast<int>(rng() % 7);
        const std::uint32_t count = rng() % 4 == 0 ? rng() % 10 : static_cast<std::uint32_t>(n);
        const std::uint32_t w0 = rng(), w3 = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            g_nodes.clear();
            for (int k = 0; k <= n; ++k) g_nodes.push_back(real_malloc(12));  // [0] sentinel, then the nodes
            for (int k = 0; k <= n; ++k) {
                auto* node = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(g_nodes[k]));
                node[0] = g_nodes[(k + 1) % (n + 1)];
                node[1] = g_nodes[(k + n) % (n + 1)];
                node[2] = 0x1000u + static_cast<std::uint32_t>(k);
            }
            std::uint32_t list[4] = {w0, g_nodes[0], count, w3};
            g_frees.clear();
            fn[side](list, 0);
            snap[side] = g_frees;
            snap[side].insert(snap[side].end(), list, list + 4);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    recoil::g_Iat_free_004cc5b4 = saved;
    std::printf("  List_DestroyNodes calls %d (circular lists of 0..6 heap nodes, counts that may disagree)\n", compared);
}
