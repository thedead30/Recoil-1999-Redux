// Structured native L1 for NodeList_RunCallbacks (0x0044eaa0) and gwNodeSetModel (0x00447e60).
// NodeList_RunCallbacks (ECX a list's link): saves and clears the flush switch [0x004dded8]; along the links (next at
// +8) a node (link+0) without a callback (+0x48) gets its link marked (+0xC = 1), an unmarked link whose node has
// +0x24 bit 2 has its callback called (ECX = node); then the switch is restored and NodeList_FlushAll (0x0044e920)
// flushes every dirty list when it is set. Each side: its own list (0..4 links over 3 nodes, marks random), callbacks
// that log their node (one also clears another node's callback), switch and dirty word random.
// gwNodeSetModel (ECX node, EDX model): null -> report, 5; an old model (+0x3c) is released (Model_Release: +8 - 1),
// a new one is kept (Model_AddRef) and its box computed into +0x8c (Model_ComputeBoundsCentreRadius - model types
// 2..5 here, which leave the box but still write the centre / radius at model +0x44..+0x50; types 0 / 1 are that
// function's own test) with +0x24 bit 9, none clears bit 9;
// +0x2c bit 0; a node without +0x24 bit 0 goes on list 7; +0x24 bits 0 and 1.
// Compared by role: the callback log, returns, node / model words, every list's links and marks, the dirty words, the
// switch, the free-link head and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Class.h"
#include "unattributed/scene_update.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
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

std::uint32_t g_nodes[3][49];
std::vector<std::uint32_t> g_log;
int __fastcall cb_a(std::uint32_t* node, int) { g_log.push_back(0xA00u + static_cast<std::uint32_t>((node - g_nodes[0]) / 49)); return 7; }
int __fastcall cb_b(std::uint32_t* node, int)
{
    g_log.push_back(0xB00u + static_cast<std::uint32_t>((node - g_nodes[0]) / 49));
    g_nodes[2][0x48 / 4] = 0;  // a callback that changes a later node's state
    return 0;
}

struct Roles {
    std::map<std::uint32_t, std::uint32_t> m{{0, 0}};
    std::uint32_t fresh = 0xC000;
    std::uint32_t operator()(std::uint32_t v) { auto f = m.find(v); return f != m.end() ? f->second : (m[v] = fresh++); }
};
void dump_lists(int side, Roles& r, std::vector<std::uint32_t>& out)
{
    for (int l = 0; l < 16; ++l) {
        std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
        out.push_back(r(p));
        out.push_back(r(*at(*img(side, 0x004ddf38u + 4 * l))));
        out.push_back(*img(side, 0x00539bb4u + 12 * l));
        for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) {
            out.push_back(r(at(p)[0])); out.push_back(r(at(p)[1]));
            out.push_back(r(at(p)[2])); out.push_back(at(p)[3]);
        }
    }
    out.push_back(r(*img(side, 0x00539c6c)));
    out.push_back(*img(side, 0x00539c74));
}
}  // namespace

TEST(native_node_list_run_callbacks_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0044eaa0), reinterpret_cast<Fn>(&recoil::NodeList_RunCallbacks)};
    const std::uint32_t cbs[3] = {0, addr(reinterpret_cast<void*>(&cb_a)), addr(reinterpret_cast<void*>(&cb_b))};
    std::mt19937 rng(0x44eaa0);
    int compared = 0, called = 0;
    for (int it = 0; it < 6000; ++it) {
        const int l = static_cast<int>(rng() % 16), len = static_cast<int>(rng() % 5);
        int who[4];
        std::uint32_t mark[4];
        for (int e = 0; e < 4; ++e) { who[e] = static_cast<int>(rng() % 3); mark[e] = rng() % 3 == 0 ? 1u : 0u; }
        std::uint32_t nflags[3], ncb[3];
        for (int i = 0; i < 3; ++i) { nflags[i] = rng() & ~4u | (rng() % 3 ? 4u : 0u); ncb[i] = cbs[rng() % 3]; }
        const std::uint32_t sw = rng() % 3 ? 1u : 0u, dirty = rng() % 2;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memset(g_nodes, 0, sizeof g_nodes);
            for (int i = 0; i < 3; ++i) { g_nodes[i][0x24 / 4] = nflags[i]; g_nodes[i][0x48 / 4] = ncb[i]; }
            std::uint32_t links[4][4];
            Roles r;
            for (int i = 0; i < 3; ++i) r.m[addr(g_nodes[i])] = 0x100u + i;
            for (int e = 0; e < 4; ++e) {
                links[e][0] = addr(g_nodes[who[e]]);
                links[e][1] = e ? addr(links[e - 1]) : 0u;
                links[e][2] = e + 1 < len ? addr(links[e + 1]) : 0u;
                links[e][3] = mark[e];
                r.m[addr(links[e])] = 0x200u + e;
            }
            *at(*img(side, 0x004ddef8u + 4 * l)) = len ? addr(links[0]) : 0u;
            *at(*img(side, 0x004ddf38u + 4 * l)) = len ? addr(links[len - 1]) : 0u;
            *img(side, 0x00539bb4u + 12 * l) = dirty;
            *img(side, 0x004dded8) = sw;
            g_log.clear();
            fn[side](len ? links[0] : nullptr, 0);
            snap[side] = g_log;
            snap[side].push_back(0xF0F0F0F0u);
            snap[side].push_back(*img(side, 0x004dded8));
            for (int i = 0; i < 3; ++i) { snap[side].push_back(g_nodes[i][0x24 / 4]); snap[side].push_back(g_nodes[i][0x48 / 4]); }
            dump_lists(side, r, snap[side]);
            if (side == 0) called += !g_log.empty();
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  NodeList_RunCallbacks calls %d, %d with callbacks (lists 0..15, 0..4 links, switch / dirty random)\n", compared, called);
}

TEST(native_gw_node_set_model_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*);
    const Fn fn[2] = {rt::original<Fn>(0x00447e60), reinterpret_cast<Fn>(&recoil::gwNodeSetModel)};
    std::mt19937 rng(0x447e60);
    int compared = 0;
    static std::uint32_t node[49], model[2][32];  // the bounds call writes centre / radius at +0x44..+0x50
    for (int it = 0; it < 4000; ++it) {
        std::uint32_t node_init[49], model_init[2][32];
        for (auto& w : node_init) w = rng();
        for (auto& m : model_init) { for (auto& w : m) w = rng(); m[0] = 2 + rng() % 4; m[2] = rng() % 5; }
        node_init[0x24 / 4] = node_init[0x24 / 4] & ~1u | (rng() % 3 == 0 ? 1u : 0u);
        node_init[0x54 / 4] = 0;
        const int old_m = static_cast<int>(rng() % 3) - 1, new_m = static_cast<int>(rng() % 3) - 1;  // -1 none, 0 / 1
        const bool null_node = rng() % 40 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(node, node_init, sizeof node);
            std::memcpy(model, model_init, sizeof model);
            node[0x3c / 4] = old_m < 0 ? 0u : addr(model[old_m]);
            const int r = fn[side](null_node ? nullptr : node, new_m < 0 ? nullptr : model[new_m]);
            Roles rl;
            rl.m[addr(node)] = 0xA0;
            snap[side].push_back(static_cast<std::uint32_t>(r));
            snap[side].insert(snap[side].end(), node, node + 49);
            snap[side].insert(snap[side].end(), &model[0][0], &model[0][0] + 64);
            dump_lists(side, rl, snap[side]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  gwNodeSetModel calls %d (old / new model none, same or other; types 2..5)\n", compared);
}
