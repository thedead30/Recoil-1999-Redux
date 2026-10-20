// Structured native L1 for gwNodeDelete (0x00447b60, ECX node): null -> report, 5; clears +0x24 bit 1, unregisters the
// node from its lists (List_UnregisterNode 0x0044f000: marks its links, sets the dirty words), then NodeList_FlushAll
// (0x0044e920): with the flush switch [0x004dded8] on, the dirty lists are flushed and the node freed now (gwNodeFree
// 0x00447a70: refused while it has children, parents or a model; else frees its child / parent arrays and class data
// and returns the slot to the pool [0x00539c94] free list [0x004de4c8]); with the switch off the node is deferred
// (NodeList_DeferNode 0x0044ed60, chain [0x00539c70]). Returns 0.
// Each side: its own pool of 4 records, the node one of them (class 0..13, list index -1..7, callback, flags random;
// children / parents / model present now and then; arrays and class data real msvcrt blocks or null), its own 16 lists
// (0..2 links, some for the node, the others for other pool records, marks random), switch on or off; free is logged in both import slots.
// Compared by role: return, every pool word (block pointers as roles), the free head, the deferred chain, every list's
// links and marks, the dirty words, the free log, the free-link head and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Class.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <windows.h>

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
std::uint32_t real_malloc(std::size_t n)
{
    return addr(reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"))(n));
}
void real_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(at(p)); }
std::vector<std::uint32_t> g_freed;
void __cdecl logging_free(void* p) { g_freed.push_back(addr(p)); if (p) real_free(addr(p)); }
}  // namespace

TEST(native_gw_node_delete_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00447b60), reinterpret_cast<Fn>(&recoil::gwNodeDelete)};
    void** o_free = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* const saved[2] = {*o_free, recoil::g_Iat_free_004cc5b4};
    *o_free = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    std::mt19937 rng(0x447b60);
    int compared = 0, freed_now = 0, deferred = 0;
    for (int it = 0; it < 4000; ++it) {
        std::vector<std::uint32_t> pool_init(49 * 4);
        for (auto& w : pool_init) w = rng();
        const int k = static_cast<int>(rng() % 4);
        std::uint32_t* n = &pool_init[49 * k];
        n[0x34 / 4] = rng() % 14;
        n[0x44 / 4] = static_cast<std::uint32_t>(static_cast<int>(rng() % 9) - 1);
        if (rng() % 3 == 0) n[0x48 / 4] = 0;
        n[0x5C / 4] = rng() % 6 == 0 ? 1u : 0u;
        n[0x54 / 4] = rng() % 6 == 0 ? 1u : 0u;
        n[0x3C / 4] = rng() % 6 == 0 ? 0x12345670u : 0u;
        const bool has_arr[3] = {rng() % 2 == 0, rng() % 2 == 0, rng() % 5 != 0};  // +0x60, +0x58, +0x38
        const std::uint32_t head = rng() % 2 ? 0xFFFFFFFFu : static_cast<std::uint32_t>((k + 1) % 4);
        const std::uint32_t sw = rng() % 2;
        int len[16], is_node[16][2];
        std::uint32_t mark[16][2];
        for (int l = 0; l < 16; ++l) {
            len[l] = static_cast<int>(rng() % 3);
            for (int e = 0; e < 2; ++e) { is_node[l][e] = static_cast<int>(rng() % 2); mark[l][e] = rng() % 4 == 0 ? 1u : 0u; }
        }
        const bool null_node = rng() % 40 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> pool = pool_init;
            std::uint32_t* node = &pool[49 * k];
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int i = 0; i < 4; ++i) role[addr(&pool[49 * i])] = 0x100u + i;
            std::vector<std::uint32_t> blocks;
            const int offs[3] = {0x60, 0x58, 0x38};
            for (int a = 0; a < 3; ++a) {
                node[offs[a] / 4] = has_arr[a] ? real_malloc(16) : 0u;
                if (has_arr[a]) { role[node[offs[a] / 4]] = 0x300u + a; blocks.push_back(node[offs[a] / 4]); }
            }
            std::uint32_t links[16][2][4];
            for (int l = 0; l < 16; ++l) {
                for (int e = 0; e < 2; ++e) {
                    links[l][e][0] = addr(&pool[49 * (is_node[l][e] ? k : (k + 1 + e) % 4)]);  // other nodes: real records (list 7 reads their flags)
                    links[l][e][1] = e ? addr(links[l][e - 1]) : 0u;
                    links[l][e][2] = e + 1 < len[l] ? addr(links[l][e + 1]) : 0u;
                    links[l][e][3] = mark[l][e];
                    role[addr(links[l][e])] = 0x200u + 2 * l + e;
                }
                *at(*img(side, 0x004ddef8u + 4 * l)) = len[l] ? addr(links[l][0]) : 0u;
                *at(*img(side, 0x004ddf38u + 4 * l)) = len[l] ? addr(links[l][len[l] - 1]) : 0u;
            }
            *img(side, 0x00539c94) = addr(pool.data());
            *img(side, 0x004de4c8) = head;
            *img(side, 0x004dded8) = sw;
            g_freed.clear();
            const int r = fn[side](null_node ? nullptr : node, 0);
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (std::uint32_t f : g_freed) snap[side].push_back(rl(f));
            snap[side].push_back(0xF0F0F0F0u);
            for (int i = 0; i < 4 * 49; ++i) {
                const int w = i % 49;
                const bool ptr_word = i / 49 == k && (w == 0x60 / 4 || w == 0x58 / 4 || w == 0x38 / 4);
                snap[side].push_back(ptr_word ? rl(pool[i]) : pool[i]);
            }
            snap[side].push_back(*img(side, 0x004de4c8));
            for (std::uint32_t p = *img(side, 0x00539c70), guard = 0; guard < 64; ++guard) {
                snap[side].push_back(rl(p));
                if (!p) break;
                snap[side].push_back(rl(at(p)[0]));
                p = at(p)[2];
            }
            for (int l = 0; l < 16; ++l) {
                std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
                snap[side].push_back(rl(p));
                snap[side].push_back(rl(*at(*img(side, 0x004ddf38u + 4 * l))));
                snap[side].push_back(*img(side, 0x00539bb4u + 12 * l));
                for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) {
                    snap[side].push_back(rl(at(p)[0])); snap[side].push_back(rl(at(p)[1]));
                    snap[side].push_back(rl(at(p)[2])); snap[side].push_back(at(p)[3]);
                }
            }
            snap[side].push_back(rl(*img(side, 0x00539c6c)));
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0) { freed_now += !g_freed.empty(); deferred += *img(0, 0x00539c70) != 0; }
            for (std::uint32_t b : blocks)
                if (std::find(g_freed.begin(), g_freed.end(), b) == g_freed.end()) real_free(b);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    *o_free = saved[0];
    recoil::g_Iat_free_004cc5b4 = saved[1];
    std::printf("  gwNodeDelete calls %d, %d freed now, %d deferred (classes 0..13, lists 0..15, switch on / off, refusals)\n", compared, freed_now, deferred);
}
