// Structured native L1 for two callers of NodeList_PushHead (0x0044ee10) whose arena fuzz would feed the node lists
// random pointers:
// World_QueueDirtyCell (0x00450030: ECX node, EDX queue {flags, count, capacity, array}, stack cell; ret 4): a full
// queue grows by one through realloc (0x004cc4ec); the cell is appended and gets bit 0 of its first word; a node without
// +0x24 bit 0 is pushed on list 7 (the bit is set again if the push returns 0); the node gets bit 1 and the queue flags
// bit 4. Each side: a real msvcrt array (0..3 entries, capacity full or one spare), a node with 0..2 references.
// gwNodeSetActionCallbackFirst (0x00447fe0: ECX node, EDX callback): null -> report, 5; list index +0x44 outside 0..5
// -> report, 1; old callback +0x48 zero and new non-zero -> NodeList_PushHead(+0x44, node); old non-zero and new zero ->
// NodeList_MarkNode(+0x44, node) (0x0044eed0); then +0x48 = callback, 0. Each side: its own lists 0..5 (0..3 links,
// some for the node, marks random).
// Compared by role: returns, the queue / node words, every array entry, every list's head / tail and link chain
// {node, prev, next, mark}, the dirty words 0x00539bb4 + 12*list and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Class.h"
#include "GameZRecoil/zClass/cls_world.h"
#include "platform/image/original_data.h"

#include <windows.h>

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

struct Roles {
    std::map<std::uint32_t, std::uint32_t> m{{0, 0}};
    std::uint32_t fresh = 0xC000;  // blocks the call allocated, numbered in order of first sight
    std::uint32_t operator()(std::uint32_t v) { auto f = m.find(v); return f != m.end() ? f->second : (m[v] = fresh++); }
};

void dump_lists(int side, int n, Roles& r, std::vector<std::uint32_t>& out)
{
    for (int l = 0; l < n; ++l) {
        std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
        out.push_back(r(p));
        out.push_back(r(*at(*img(side, 0x004ddf38u + 4 * l))));
        out.push_back(*img(side, 0x00539bb4u + 12 * l));
        for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) {
            out.push_back(r(at(p)[0])); out.push_back(r(at(p)[1]));
            out.push_back(r(at(p)[2])); out.push_back(at(p)[3]);
        }
    }
    out.push_back(*img(side, 0x00539c74));
}
}  // namespace

TEST(native_world_queue_dirty_cell_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*, void*);
    const Fn fn[2] = {rt::original<Fn>(0x00450030), reinterpret_cast<Fn>(&recoil::World_QueueDirtyCell)};
    std::mt19937 rng(0x450030);
    int compared = 0, pushed = 0;
    for (int it = 0; it < 4000; ++it) {
        const std::uint32_t count = rng() % 4, cap = count + (rng() % 2 ? 0u : 1u);
        const std::uint32_t qflags = rng(), cell0 = rng() & ~1u | (rng() % 4 == 0 ? 1u : 0u);
        const std::uint32_t nflags = rng() & ~3u | (rng() % 3 == 0 ? 1u : 0u) | (rng() % 4 == 0 ? 2u : 0u);
        const int nrefs = static_cast<int>(rng() % 3);
        std::uint32_t rclass[2], rflags[2];
        for (int k = 0; k < 2; ++k) { rclass[k] = rng() % 4; rflags[k] = rng() & ~1u | (rng() % 3 == 0 ? 1u : 0u); }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t node[0x60 / 4] = {}, ref[2][0x60 / 4] = {}, cell[4] = {cell0, 1, 2, 3};
            node[0x24 / 4] = nflags;
            std::uint32_t refs[2] = {addr(ref[0]), addr(ref[1])};
            node[0x54 / 4] = static_cast<std::uint32_t>(nrefs);
            node[0x58 / 4] = addr(refs);
            for (int k = 0; k < 2; ++k) { ref[k][0x24 / 4] = rflags[k]; ref[k][0x34 / 4] = rclass[k]; }
            std::uint32_t q[4] = {qflags, count, cap, real_malloc(4 * (cap ? cap : 1))};
            for (std::uint32_t k = 0; k < count; ++k) at(q[3])[k] = 0xE0000000u + k;
            Roles r;
            r.m[addr(node)] = 0xA0; r.m[addr(ref[0])] = 0xA1; r.m[addr(ref[1])] = 0xA2; r.m[addr(cell)] = 0xA3;
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](node, q, cell)));
            snap[side].push_back(q[0]); snap[side].push_back(q[1]); snap[side].push_back(q[2]);
            for (std::uint32_t k = 0; k < q[1]; ++k) snap[side].push_back(r(at(q[3])[k]));
            snap[side].push_back(cell[0]);
            snap[side].push_back(node[0x24 / 4]); snap[side].push_back(ref[0][0x24 / 4]); snap[side].push_back(ref[1][0x24 / 4]);
            dump_lists(side, 16, r, snap[side]);
            if (side == 0 && !(nflags & 1)) ++pushed;
            real_free(q[3]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  World_QueueDirtyCell calls %d, %d pushed on list 7 (arrays full or one spare, node references)\n", compared, pushed);
}

namespace {
using CbFn = int(__fastcall*)(void*, std::uint32_t);
// mode 0: fn(node, callback) with node +0x44 = index; mode 1: gwNodeSetActionList fn(node, new index)
void run_callback(const CbFn fn[2], std::uint32_t seed, const char* name, int mode)
{
    std::mt19937 rng(seed);
    int compared = 0;
    for (int it = 0; it < 6000; ++it) {
        const bool null_node = rng() % 30 == 0;
        const std::uint32_t index = static_cast<std::uint32_t>(static_cast<int>(rng() % 9) - 1);  // -1..7
        const std::uint32_t old_cb = rng() % 2 ? 0u : 0x00401000u + (rng() & 0xFFF0u);
        const std::uint32_t new_cb = mode == 1 ? static_cast<std::uint32_t>(static_cast<int>(rng() % 9) - 1)
                                                 : rng() % 2 ? 0u : 0x00402000u + (rng() & 0xFFF0u);
        const std::uint32_t nflags = rng() & ~1u;
        int len[6], is_node[6][3];
        std::uint32_t mark[6][3];
        for (int l = 0; l < 6; ++l) {
            len[l] = static_cast<int>(rng() % 4);
            for (int e = 0; e < 3; ++e) { is_node[l][e] = static_cast<int>(rng() % 2); mark[l][e] = rng() % 3 == 0 ? 1u : 0u; }
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t node[0x60 / 4] = {};
            node[0x24 / 4] = nflags;
            node[0x44 / 4] = index;
            node[0x48 / 4] = old_cb;
            std::uint32_t links[6][3][4];
            Roles r;
            r.m[addr(node)] = 0xA0;
            for (int l = 0; l < 6; ++l) {
                for (int e = 0; e < 3; ++e) {
                    links[l][e][0] = is_node[l][e] ? addr(node) : 0xD0000000u + 3 * l + e;
                    links[l][e][1] = e ? addr(links[l][e - 1]) : 0u;
                    links[l][e][2] = e + 1 < len[l] ? addr(links[l][e + 1]) : 0u;
                    links[l][e][3] = mark[l][e];
                    r.m[addr(links[l][e])] = 0x200u + 3 * l + e;
                }
                *at(*img(side, 0x004ddef8u + 4 * l)) = len[l] ? addr(links[l][0]) : 0u;
                *at(*img(side, 0x004ddf38u + 4 * l)) = len[l] ? addr(links[l][len[l] - 1]) : 0u;
            }
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](null_node ? nullptr : node, new_cb)));
            snap[side].push_back(node[0x24 / 4]); snap[side].push_back(node[0x48 / 4]);
            snap[side].push_back(node[0x44 / 4]);
            dump_lists(side, 6, r, snap[side]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  %s calls %d (list index -1..7, old / new %s, lists 0..5)\n", name, compared,
                mode ? "index, callback set or clear" : "callback set or clear");
}
}  // namespace

TEST(native_gw_node_set_action_callback_first_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const CbFn fn[2] = {rt::original<CbFn>(0x00447fe0), reinterpret_cast<CbFn>(&recoil::gwNodeSetActionCallbackFirst)};
    run_callback(fn, 0x447fe0, "gwNodeSetActionCallbackFirst", 0);
}

// gwNodeSetActionCallback (0x00447f30): the same, but a callback being set appends through NodeRegistry_Insert.
TEST(native_gw_node_set_action_callback_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const CbFn fn[2] = {rt::original<CbFn>(0x00447f30), reinterpret_cast<CbFn>(&recoil::gwNodeSetActionCallback)};
    run_callback(fn, 0x447f30, "gwNodeSetActionCallback", 0);
}

// gwNodeSetActionList (0x00448090: ECX node, EDX new index): null -> report, 5; with a callback (+0x48) set, an old
// index +0x44 in 0..5 -> NodeList_MarkNode(old, node) and a new index in 0..5 -> NodeRegistry_Insert(new, node); then
// +0x44 = new index, 0.
TEST(native_gw_node_set_action_list_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const CbFn fn[2] = {rt::original<CbFn>(0x00448090), reinterpret_cast<CbFn>(&recoil::gwNodeSetActionList)};
    run_callback(fn, 0x448090, "gwNodeSetActionList", 1);
}
