// Structured native L1 for the class constructors / destructor built on gwNodeAlloc (0x004478c0):
// World_Create (0x004501c0): a pool node, class 2, class data calloc(1, 0xac) with its defaults, registered on list 13
// (the gwNodeAlloc result is not checked: an empty pool faults at the +0x34 write - KG-40, not exercised here).
// Object3D_Create (0x0044daa0): empty pool -> report, 0; else class 5, class data calloc(1, 0x90), Object3D_ResetTransform
// (0x0044d9e0); returns the node (0 if the reset failed).
// World_Destroy (0x00450240, ECX world): World_FreeGrid (0x00450e40); then frees the class data's arrays +0x94, +0x98,
// +0xa0, +0xa4 and the queue array +0xc, and deletes the node (gwNodeDelete 0x00447b60).
// Each side: its own pool of 2..6 records (random words, free chain in random order; Object3D_Create also with an empty
// pool), its own lists (pristine); World_Destroy on a world made by World_Create, with a grid (World_BuildGrid) or none,
// arrays present or null, the flush switch on or off. free is logged in both import slots.
// Compared by role: returns, every pool word, the class data blocks' words, the free head, the free log, the deferred
// chain, every list's links, marks and dirty word, the free-link head and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Object3d.h"
#include "GameZRecoil/zClass/cls_world.h"
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

TEST(native_node_create_destroy_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F0 = std::uint32_t(__fastcall*)(int, int);
    using F1 = int(__fastcall*)(std::uint32_t, int);
    using Build = int(__fastcall*)(std::uint32_t, int, float, float);
    const F0 wcreate[2] = {rt::original<F0>(0x004501c0), reinterpret_cast<F0>(&recoil::World_Create)};
    const F0 ocreate[2] = {rt::original<F0>(0x0044daa0), reinterpret_cast<F0>(&recoil::Object3D_Create)};
    const F1 wdestroy[2] = {rt::original<F1>(0x00450240), reinterpret_cast<F1>(&recoil::World_Destroy)};
    const Build build[2] = {rt::original<Build>(0x00450c60), reinterpret_cast<Build>(&recoil::World_BuildGrid)};
    void** o_free = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* const saved[2] = {*o_free, recoil::g_Iat_free_004cc5b4};
    *o_free = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    std::mt19937 rng(0x4501c0);
    int compared = 0, per[3] = {};
    for (int it = 0; it < 6000; ++it) {
        const int f = it % 3;  // 0 World_Create, 1 Object3D_Create, 2 World_Create + World_Destroy
        const int n = 2 + static_cast<int>(rng() % 5);
        std::vector<std::uint32_t> pool_init(49 * n);
        for (auto& w : pool_init) w = rng();
        std::vector<int> order(n);
        for (int i = 0; i < n; ++i) order[i] = i;
        std::shuffle(order.begin(), order.end(), rng);
        const int chain = f == 1 ? static_cast<int>(rng() % (n + 1)) : 1 + static_cast<int>(rng() % n);
        for (int k = 0; k < chain; ++k) {
            std::uint32_t& w = pool_init[49 * order[k] + 0xC0 / 4];
            w = (w & 0xFF000000u) | (k + 1 < chain ? static_cast<std::uint32_t>(order[k + 1]) : 0xFFFFFFu);
        }
        const std::uint32_t head = chain ? static_cast<std::uint32_t>(order[0]) : 0xFFFFFFFFu;
        const bool grid = rng() % 2, sw = rng() % 2;
        bool arr[5];
        for (auto& a : arr) a = rng() % 2;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> pool = pool_init;
            *img(side, 0x00539c94) = addr(pool.data());
            *img(side, 0x004de4c8) = head;
            *img(side, 0x004dded8) = sw;
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int i = 0; i < n; ++i) role[addr(&pool[49 * i])] = 0x100u + i;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            g_freed.clear();
            std::uint32_t r = (f == 1 ? ocreate : wcreate)[side](0, 0);
            snap[side].push_back(rl(r));
            std::uint32_t data = r ? at(r)[0x38 / 4] : 0u;
            const std::uint32_t data_words = f == 1 ? 0x90 / 4 : 0xac / 4;
            if (data) { rl(data); for (std::uint32_t k = 0; k < data_words; ++k) snap[side].push_back(at(data)[k]); }
            std::vector<std::uint32_t> owned;  // blocks the test allocated, freed afterwards unless the call did
            if (f == 2) {
                if (grid) build[side](r, 0, 64.0f, -64.0f);
                const int offs[5] = {0x94, 0x98, 0xa0, 0xa4, 0x0c};
                for (int a = 0; a < 5; ++a) {
                    std::uint32_t& w = at(data)[offs[a] / 4];
                    if (a == 4 && w) continue;  // keep a queue array the grid build left (none: the queue is empty)
                    w = arr[a] ? real_malloc(8) : 0u;
                    if (w) { role[w] = 0x300u + a; owned.push_back(w); }
                }
                g_freed.clear();
                snap[side].push_back(static_cast<std::uint32_t>(wdestroy[side](r, 0)));
                for (std::uint32_t p : g_freed) snap[side].push_back(p == data ? 0xDA7Au : rl(p));
                snap[side].push_back(0xF0F0F0F0u);
            }
            for (int i = 0; i < 49 * n; ++i) snap[side].push_back(i % 49 == 0x38 / 4 ? rl(pool[i]) : pool[i]);
            snap[side].push_back(*img(side, 0x004de4c8));
            for (std::uint32_t p = *img(side, 0x00539c70), guard = 0; guard < 64; ++guard) {
                snap[side].push_back(rl(p));
                if (!p) break;
                snap[side].push_back(rl(at(p)[0]));
                p = at(p)[2];
            }
            for (int l = 0; l < 16; ++l) {
                std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
                snap[side].push_back(*img(side, 0x00539bb4u + 12 * l));
                for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            }
            snap[side].push_back(rl(*img(side, 0x00539c6c)));
            snap[side].push_back(*img(side, 0x00539c74));
            for (std::uint32_t b : owned)
                if (std::find(g_freed.begin(), g_freed.end(), b) == g_freed.end()) real_free(b);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++per[f];
        ++compared;
    }
    rt::restore_pristine();
    *o_free = saved[0];
    recoil::g_Iat_free_004cc5b4 = saved[1];
    std::printf("  World_Create %d, Object3D_Create %d (empty pools too), World_Create + World_Destroy %d (grid / arrays / switch)\n",
                per[0], per[1], per[2]);
}
