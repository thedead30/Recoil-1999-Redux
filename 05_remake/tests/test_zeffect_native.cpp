// Structured native L1 for zeffect functions ported in the cloud (P2 zeffect) that need real strings, trees, heap arrays
// or a callback.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zEffect/zeff_anim_init.h"
#include "GameZRecoil/zEffect/zeff_anim_save.h"
#include "GameZRecoil/zEffect/zeff_init.h"
#include "unattributed/zeffect.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

using namespace ch;

// AnimData_SetZbdName (0x0045e210; ECX name): longer than 0x80 -> report, nothing copied; else copied with its NUL to
// [0x0053a1c0]. Names of 0..0x90 characters. Compared: the 0x90 bytes at 0x0053a1c0.
TEST(native_zeffect_set_zbd_name_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const char*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0045e210), reinterpret_cast<Fn>(&recoil::AnimData_SetZbdName)};
    std::mt19937 rng(0x45e210);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        std::string s(rng() % 3 == 0 ? 0x78 + rng() % 0x18 : rng() % 0x40, 'a');
        for (char& c : s) c = static_cast<char>('a' + rng() % 26);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            fn[side](s.c_str(), 0);
            const std::uint32_t* g = img(side, 0x0053a1c0);
            snap[side].assign(g, g + 0x90 / 4);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  AnimData_SetZbdName calls %d\n", compared);
}

// Node_FindByNameRecursive (0x0045e650; ECX node, EDX name): null -> 0; the node when its inline name matches, else the
// children (+0x5c / +0x60) first to last, the first match. AnimNode_FindInTree (0x00461ec0; ECX node): the node's model
// (gwNodeGetModel) when it has one, else the children first to last, the first model found; none -> 0. Real trees
// (depth <= 3, 0..3 children, names with repeats, models on some nodes). Compared: the return as a node / model index.
TEST(native_zeffect_tree_searches_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Find = std::uint32_t(__fastcall*)(void*, const char*);
    using Model = std::uint32_t(__fastcall*)(void*, int);
    const Find find[2] = {rt::original<Find>(0x0045e650), reinterpret_cast<Find>(&recoil::Node_FindByNameRecursive)};
    const Model model[2] = {rt::original<Model>(0x00461ec0), reinterpret_cast<Model>(&recoil::AnimNode_FindInTree)};
    const char* const names[] = {"root", "arm", "Arm", "gun", "", "armx"};
    std::mt19937 rng(0x45e650);
    int compared = 0, hits = 0;
    for (int it = 0; it < 4000; ++it) {
        const int f = it % 2;
        std::vector<int> parent{-1}, depth{0}, name{static_cast<int>(rng() % 6)}, has_model{rng() % 4 == 0};
        for (std::size_t i = 0; i < parent.size() && parent.size() < 30; ++i)
            if (depth[i] < 3) for (int c = static_cast<int>(rng() % 4); c > 0; --c) {
                parent.push_back(static_cast<int>(i)); depth.push_back(depth[i] + 1);
                name.push_back(static_cast<int>(rng() % 6)); has_model.push_back(rng() % 4 == 0);
            }
        const char* q = rng() % 6 == 0 ? "none" : names[rng() % 6];
        const bool null_node = rng() % 30 == 0;
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const std::size_t n = parent.size();
            std::vector<std::vector<std::uint32_t>> nodes(n, std::vector<std::uint32_t>(49, 0)), kids(n);
            std::vector<std::uint32_t> models(n);
            for (std::size_t i = 0; i < n; ++i) {
                std::strcpy(reinterpret_cast<char*>(nodes[i].data()), names[name[i]]);
                nodes[i][0x3c / 4] = has_model[i] ? addr(&models[i]) : 0u;
            }
            for (std::size_t i = 1; i < n; ++i) kids[parent[i]].push_back(addr(nodes[i].data()));
            for (std::size_t i = 0; i < n; ++i) { nodes[i][0x5c / 4] = static_cast<std::uint32_t>(kids[i].size()); nodes[i][0x60 / 4] = kids[i].empty() ? 0u : addr(kids[i].data()); }
            void* root = null_node && f == 0 ? nullptr : nodes[0].data();
            const std::uint32_t r = f ? model[side](root, 0) : find[side](root, q);
            ret[side] = 0xFFFFu;
            if (!r) ret[side] = 0;
            for (std::size_t i = 0; i < n; ++i) { if (r == addr(nodes[i].data())) ret[side] = 0x100u + static_cast<std::uint32_t>(i); if (r == addr(&models[i])) ret[side] = 0x200u + static_cast<std::uint32_t>(i); }
        }
        CHECK_EQ(ret[0], ret[1]);
        hits += ret[0] != 0;
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Node_FindByNameRecursive / AnimNode_FindInTree calls %d, %d hits\n", compared, hits);
    CHECK(hits > 1000);
}

// The effect list: array [0x0053a2d8] of 0x50-byte entries (+4 id, +8 32-byte name, +0x28 key), capacity [0x0053a2dc],
// count [0x0053a2e0], id mask [0x0053a2e8].
//   EffectList_Free (0x004603d0): frees the array (when set), zeroes it and the count.
//   EffectList_Contains (0x00460400; ECX entry): 1 when an entry has the same key and the same name (strncmp 0x20).
//   EffectList_AppendEntry (0x00460ae0): no array -> malloc(1000 entries), capacity 1000, count 0; full -> malloc of
//     twice the capacity, the old entries copied, the old array freed; the new entry's +4 = mask & count & 0xffffff;
//     returns it, count + 1.
// Arrays: none, or a real msvcrt block of capacity 1..3 holding 0..capacity entries (names / keys from small pools).
// free logged. Compared: returns (entries by index), the globals (array by role), the entries' words, the free log.
TEST(native_zeffect_effect_list_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const std::uint32_t*, int);
    const Fn fns[3][2] = {{rt::original<Fn>(0x004603d0), reinterpret_cast<Fn>(&recoil::EffectList_Free)},
                          {rt::original<Fn>(0x00460400), reinterpret_cast<Fn>(&recoil::EffectList_Contains)},
                          {rt::original<Fn>(0x00460ae0), reinterpret_cast<Fn>(&recoil::EffectList_AppendEntry)}};
    const char* const names[] = {"smoke", "fire", "smoke2", "Fire"};
    FreeHook hook;
    std::mt19937 rng(0x460400);
    int compared = 0, hits = 0, grown = 0;
    for (int it = 0; it < 20000; ++it) {
        const int f = it % 3;
        const bool has_array = rng() % 5 != 0;
        const int cap = 1 + static_cast<int>(rng() % 3), count = has_array ? (rng() % 2 == 0 ? cap : static_cast<int>(rng() % (cap + 1))) : 0;
        int ename[3], ekey[3];
        for (int k = 0; k < 3; ++k) { ename[k] = static_cast<int>(rng() % 4); ekey[k] = static_cast<int>(rng() % 3); }
        std::uint32_t probe[20] = {};
        std::strcpy(reinterpret_cast<char*>(probe + 2), names[rng() % 4]);
        probe[0x28 / 4] = rng() % 3;
        const std::uint32_t mask = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const std::uint32_t arr = has_array ? addr(c_malloc(0x50 * cap)) : 0u;
            for (int k = 0; k < cap && arr; ++k) {
                std::uint32_t* e = at(arr) + 20 * k;
                std::memset(e, 0x3C, 0x50);
                std::memset(e + 2, 0, 0x20);
                std::strcpy(reinterpret_cast<char*>(e + 2), names[ename[k]]);
                e[0x28 / 4] = static_cast<std::uint32_t>(ekey[k]);
            }
            *img(side, 0x0053a2d8) = arr;
            *img(side, 0x0053a2dc) = has_array ? static_cast<std::uint32_t>(cap) : 0u;
            *img(side, 0x0053a2e0) = static_cast<std::uint32_t>(count);
            *img(side, 0x0053a2e8) = mask;
            freed().clear();
            const std::uint32_t r = fns[f][side](probe, 0);
            const std::uint32_t now = *img(side, 0x0053a2d8), n = *img(side, 0x0053a2e0);
            Roles rl;
            rl.set(arr, 0xA0);
            snap[side].push_back(f == 2 && r ? 0xE000u + (r - now) : r);
            snap[side].push_back(rl(now));
            snap[side].push_back(*img(side, 0x0053a2dc));
            snap[side].push_back(n);
            for (std::uint32_t k = 0; now && k < n && k < 8; ++k)
                for (int w = 0; w < 20; ++w)
                    if (!(f == 2 && k + 1 == n && w != 1)) snap[side].push_back(at(now)[20 * k + w]);  // a new entry: only its id is written
            for (std::uint32_t p : freed()) snap[side].push_back(rl(p));
            if (side == 0) { hits += f == 1 && r; grown += f == 2 && now != arr; }
            if (now) c_free(now);
            if (arr && arr != now && !was_freed(arr)) c_free(arr);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  EffectList Free / Contains / AppendEntry calls %d, %d found, %d reallocated\n", compared, hits, grown);
    CHECK(hits > 100 && grown > 300);
}

namespace {
std::vector<std::uint32_t> g_hook_args;
int __cdecl optional_hook(std::uint32_t a) { g_hook_args.push_back(a); return static_cast<int>(a * 3 + 1); }
}  // namespace

// CallOptionalHook_0056b568 (0x004a5b20; ECX argument): the hook [0x0056b568] (cdecl, one argument) when set, its
// return; else 0. Compared: the return and the hook's argument log.
TEST(native_zeffect_call_optional_hook_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, int);
    const Fn fn[2] = {rt::original<Fn>(0x004a5b20), reinterpret_cast<Fn>(&recoil::CallOptionalHook_0056b568)};
    std::mt19937 rng(0x4a5b20);
    for (int it = 0; it < 500; ++it) {
        const bool set = rng() % 3 != 0;
        const std::uint32_t arg = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            g_hook_args.clear();
            *img(side, 0x0056b568) = set ? addr(reinterpret_cast<void*>(&optional_hook)) : 0u;
            snap[side].push_back(fn[side](arg, 0));
            snap[side].insert(snap[side].end(), g_hook_args.begin(), g_hook_args.end());
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}
