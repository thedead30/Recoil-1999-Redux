// Structured native L1s for the AI path-net functions on rand, heap blocks and the graph list (P3 ai_net, ported in the
// cloud):
//  - NetNode_PickRandomLink (0x004016a0; ECX -, EDX node holder, stack out, previous; ret 8): counts the node's three
//    link words (+0xC..+0x14 of [EDX]; at least one set here - none divides by zero); one link -> out 0, returns 1; two -> out 0; else out rand() % count; an out
//    equal to the previous one steps on by one (mod count); returns 1 (or the count of 1). rand is msvcrt's on both
//    sides; each side is seeded the same with srand first.
//  - NetGraph_AllocAndAppend (0x00402ff0): mallocs a zeroed 0x58-byte node and appends it to the graph list
//    [0x004e5c58] head / [0x004e5c5c] tail (link at +0x54); returns it.
//  - NetNode_Free (0x004037c0): frees the node's three link blocks (+0x18..+0x20) when set, then the node; null-safe.
// Compared: returns and outputs, the list globals and nodes by role, freed pointers by role.
#include "test.h"
#include "cloud_harness.h"
#include "Battlesport/ai_net.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

TEST(native_ai_net_pick_random_link_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, std::uint32_t*, std::uint32_t*, std::uint32_t);
    const F fn[2] = {rt::original<F>(0x004016a0), reinterpret_cast<F>(&recoil::NetNode_PickRandomLink)};
    const auto srand_ = ch::crt_fn<void(__cdecl*)(unsigned)>("srand");
    std::mt19937 rng(0x4016a0);
    for (int it = 0; it < 5000; ++it) {
        std::uint32_t node[6];
        for (auto& w : node) w = rng();
        for (int k = 3; k < 6; ++k) node[k] = rng() % 2 ? 0 : rng() | 1;
        if (!node[3] && !node[4] && !node[5]) node[3 + rng() % 3] = 1;  // no links: rand() % 0 faults on both sides
        const std::uint32_t prev = rng() % 4, seed = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t holder[1] = {ch::addr(node)}, out = 0xdddddddd;
            srand_(seed);
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](0, holder, &out, prev)));
            snap[side].push_back(out);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

TEST(native_ai_net_graph_alloc_and_free_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = std::uint32_t(__fastcall*)(std::uint32_t, int);
    const F alloc[2] = {rt::original<F>(0x00402ff0), reinterpret_cast<F>(&recoil::NetGraph_AllocAndAppend)};
    const F release[2] = {rt::original<F>(0x004037c0), reinterpret_cast<F>(&recoil::NetNode_Free)};
    std::mt19937 rng(0x402ff0);
    for (int it = 0; it < 300; ++it) {
        const int adds = static_cast<int>(rng() % 5);
        const bool preexisting = rng() % 2 != 0;
        std::vector<int> links(adds);
        for (int& l : links) l = static_cast<int>(rng() % 8);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            std::uint32_t old[0x58 / 4] = {};
            rl.set(old, 1);
            *ch::img(side, 0x004e5c58) = preexisting ? ch::addr(old) : 0u;
            *ch::img(side, 0x004e5c5c) = preexisting ? ch::addr(old) : 0u;
            std::vector<std::uint32_t> nodes;
            for (int k = 0; k < adds; ++k) {
                const std::uint32_t p = alloc[side](0, 0);
                nodes.push_back(p);
                snap[side].push_back(rl(p));
                bool zero = true;
                for (int w = 0; w < 0x54 / 4; ++w) zero = zero && ch::at(p)[w] == 0;
                snap[side].push_back(zero);
            }
            snap[side].push_back(rl(*ch::img(side, 0x004e5c58)));
            snap[side].push_back(rl(*ch::img(side, 0x004e5c5c)));
            snap[side].push_back(rl(old[0x54 / 4]));
            for (std::uint32_t p : nodes) snap[side].push_back(rl(ch::at(p)[0x54 / 4]));
            // free them through NetNode_Free with 0..3 link blocks each (a null node too)
            ch::freed().clear();
            {
                ch::FreeHook hook;
                release[side](0, 0);
                for (std::size_t k = 0; k < nodes.size(); ++k)
                    for (int s = 0; s < 3; ++s)
                        if ((links[k] >> s) & 1) {
                            ch::at(nodes[k])[0x18 / 4 + s] = ch::addr(ch::c_malloc(8));
                            rl.set(ch::at(nodes[k])[0x18 / 4 + s], 0x100 + 4 * static_cast<std::uint32_t>(k) + s);
                        }
                for (std::uint32_t p : nodes) release[side](p, 0);
            }
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

namespace {
float rf(std::mt19937& rng) { return static_cast<float>(static_cast<int>(rng() % 20001) - 10000) / 64.0f; }
// a graph of n nodes (0x30 bytes: position +0, links +0xC..+0x14, edges +0x18..+0x20, id +0x28, next +0x2C), made with
// the CRT heap so NetNode_Free can free them
std::vector<std::uint32_t> make_nodes(int n, const std::vector<std::uint32_t>& init)
{
    std::vector<std::uint32_t> v;
    for (int k = 0; k < n; ++k) {
        auto* p = static_cast<std::uint32_t*>(ch::c_malloc(0x30));
        std::memcpy(p, &init[k * 12], 0x30);
        v.push_back(ch::addr(p));
    }
    return v;
}
}  // namespace

// NetGraph_LinkNodes (0x00403550; ECX first node, stack radius; ret 4): for every node of the chain (+0x2C) and each of
// its three link slots holding an id >= 0, the slot becomes the node with that id (NetNode_FindById over the chain, or
// 0) and a zeroed 0x3C-byte edge is malloc'd into the matching +0x18 slot and set up by NetEdge_Init(this position,
// linked position, radius); negative ids clear the slot and its edge. NetGraph_Free (0x00403800): NetNode_Free on each
// node of the graph's chain [+0x50] (edges, then the node), then the graph. Chains of 1..8 nodes with ids 0..9 (some
// repeated, some negative), link ids -2..10. Compared: every node word (links by node index, edges as set / not), the
// edge blocks' floats, the freed pointers by role.
TEST(native_ai_net_graph_link_and_free_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using L = int(__fastcall*)(std::uint32_t, int, float);
    using F = int(__fastcall*)(std::uint32_t, int);
    const L link[2] = {rt::original<L>(0x00403550), reinterpret_cast<L>(&recoil::NetGraph_LinkNodes)};
    const F graph_free[2] = {rt::original<F>(0x00403800), reinterpret_cast<F>(&recoil::NetGraph_Free)};
    std::mt19937 rng(0x403550);
    for (int it = 0; it < 500; ++it) {
        const int n = 1 + static_cast<int>(rng() % 8);
        std::vector<std::uint32_t> init(n * 12);
        for (int k = 0; k < n; ++k) {
            std::uint32_t* w = &init[k * 12];
            for (int c = 0; c < 3; ++c) w[c] = ch::fbits(rf(rng));
            for (int s = 0; s < 3; ++s) w[3 + s] = static_cast<std::uint32_t>(static_cast<int>(rng() % 13) - 2);
            for (int s = 0; s < 3; ++s) w[6 + s] = 0;
            for (int s = 9; s < 12; ++s) w[s] = rng();
            w[0x28 / 4] = static_cast<std::uint32_t>(rng() % 6 == 0 ? -1 : static_cast<int>(rng() % 10));
        }
        const float radius = 1.0f + static_cast<float>(rng() % 200) / 10.0f;
        const bool null_first = rng() % 20 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            std::vector<std::uint32_t> nodes = make_nodes(n, init);
            for (int k = 0; k < n; ++k) {
                ch::at(nodes[k])[0x2c / 4] = k + 1 < n ? nodes[k + 1] : 0u;
                rl.set(nodes[k], 0x100 + k);
            }
            ch::wipe_stack();
            link[side](null_first ? 0u : nodes[0], 0, radius);
            for (int k = 0; k < n; ++k) {
                const std::uint32_t* w = ch::at(nodes[k]);
                for (int c = 0; c < 12; ++c) snap[side].push_back(c >= 3 && c < 6 ? rl(w[c]) : c >= 6 && c < 9 ? (w[c] ? 1u : 0u) : w[c]);
                for (int s = 0; s < 3; ++s)
                    if (w[6 + s]) {
                        rl.set(w[6 + s], 0x200 + 4 * static_cast<std::uint32_t>(k) + s);
                        for (int e = 0; e < 0x3c / 4; ++e) snap[side].push_back(ch::at(w[6 + s])[e]);
                    }
            }
            // free the whole graph through NetGraph_Free
            auto* graph = static_cast<std::uint32_t*>(ch::c_malloc(0x54));
            std::memset(graph, 0, 0x54);
            graph[0x50 / 4] = nodes[0];
            rl.set(graph, 0x300);
            ch::freed().clear();
            {
                ch::FreeHook hook;
                graph_free[side](ch::addr(graph), 0);
            }
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// The route of an AI vehicle (block [ECX+4]: position +0x3EC, route node +0xF78, link index +0xF80, state +0xF84,
// timer +0xFD0):
//  - AiVeh_PushReturnNode (0x00401f60; EDX previous): when the vehicle is far enough from its route node (squared
//    distance against the constant at 0x004cc82c) pushes a temporary node (id -1, +0xC = EDX) at its position with an
//    edge (NetEdge_Init, radius 10) to the route node, makes it the route node, index 0, timer [0x004f3760] - 0x004cc830.
//  - AiVeh_PopNegativeRouteNodes (0x00403830): frees route nodes with negative ids, following +0xC.
//  - AiVeh_AdvanceToLinkedNode (0x00401580; EDX route holder, stack out edge, out vector; ret 8): moves along the chosen
//    link; a temporary node is freed and a random link picked (NetNode_PickRandomLink, rand seeded the same on both
//    sides), else the link back to the node it came from is avoided; hands out the edge and the offset to the node.
// Graphs of 2..6 nodes with 1..3 links each (edges as 0x3C-byte blocks), temporary chains of 0..2 nodes on top.
// Compared: returns, the vehicle words, the holder, outputs, node words (by role), freed pointers by role.
TEST(native_ai_net_vehicle_route_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using P = int(__fastcall*)(std::uint32_t*, std::uint32_t);
    using D = int(__fastcall*)(std::uint32_t*, int);
    using A = int(__fastcall*)(std::uint32_t*, std::uint32_t*, std::uint32_t*, float*);
    const P push[2] = {rt::original<P>(0x00401f60), reinterpret_cast<P>(&recoil::AiVeh_PushReturnNode)};
    const D pop[2] = {rt::original<D>(0x00403830), reinterpret_cast<D>(&recoil::AiVeh_PopNegativeRouteNodes)};
    const A advance[2] = {rt::original<A>(0x00401580), reinterpret_cast<A>(&recoil::AiVeh_AdvanceToLinkedNode)};
    const auto srand_ = ch::crt_fn<void(__cdecl*)(unsigned)>("srand");
    std::mt19937 rng(0x401f60);
    for (int it = 0; it < 1500; ++it) {
        const int op = static_cast<int>(rng() % 3), n = 2 + static_cast<int>(rng() % 5), temps = static_cast<int>(rng() % 3);
        std::vector<std::uint32_t> init((n + temps) * 12);
        std::vector<std::uint32_t> linkto((n + temps) * 3);
        for (int k = 0; k < n + temps; ++k) {
            std::uint32_t* w = &init[k * 12];
            for (int c = 0; c < 3; ++c) w[c] = ch::fbits(rf(rng));
            for (int c = 3; c < 12; ++c) w[c] = 0;
            w[0x28 / 4] = k < n ? static_cast<std::uint32_t>(k) : 0xffffffffu;
            for (int s = 0; s < 3; ++s) linkto[k * 3 + s] = k < n ? (s == 0 || rng() % 2 ? rng() % n : 0xffu) : (s == 0 ? (k + 1 < n + temps ? k + 1 : rng() % n) : 0xffu);
        }
        std::vector<std::uint32_t> blk0(0x1000 / 4);
        for (auto& w : blk0) w = ch::fbits(rf(rng));
        // the advance reads the link the index names, so it starts on link 0 (always present)
        const std::uint32_t link_index = op == 2 ? 0 : rng() % 3, prev = rng(), seed = rng(), state = rng() % 3, mode18 = rng() % 2;
        const float timer = rf(rng);
        const int start = n + temps > n ? n : static_cast<int>(rng() % n);  // the first temporary node, or a graph node
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            std::vector<std::uint32_t> nodes = make_nodes(n + temps, init);
            for (int k = 0; k < n + temps; ++k) {
                rl.set(nodes[k], 0x100 + k);
                for (int s = 0; s < 3; ++s) {
                    const std::uint32_t t = linkto[k * 3 + s];
                    if (t == 0xffu) continue;
                    ch::at(nodes[k])[3 + s] = nodes[t];
                    auto* e = static_cast<std::uint32_t*>(ch::c_malloc(0x3c));
                    for (int x = 0; x < 15; ++x) e[x] = init[(k * 12 + x) % init.size()] ^ static_cast<std::uint32_t>(s);
                    ch::at(nodes[k])[6 + s] = ch::addr(e);
                    rl.set(e, 0x200 + 4 * static_cast<std::uint32_t>(k) + s);
                }
            }
            // temporary nodes chain on through +0xC (their first link)
            std::vector<std::uint32_t> blk = blk0;
            blk[0xf78 / 4] = nodes[start];
            blk[0xf80 / 4] = link_index;
            blk[0xf84 / 4] = state;
            std::uint32_t mode[0x1c / 4] = {};
            mode[0x18 / 4] = mode18;
            blk[0xf74 / 4] = ch::addr(mode);
            std::uint32_t veh[2] = {0, ch::addr(blk.data())};
            *ch::img(side, 0x004f3760) = ch::fbits(timer);
            std::uint32_t holder[1] = {nodes[start]}, out_edge = 0xeeee;
            float out_vec[3] = {-1, -1, -1};
            srand_(seed);
            ch::wipe_stack();
            ch::freed().clear();
            int ret;
            {
                ch::FreeHook hook;
                ret = op == 0 ? push[side](veh, prev) : op == 1 ? pop[side](veh, 0) : advance[side](veh, holder, &out_edge, out_vec);
            }
            snap[side].push_back(op == 2 ? 0u : static_cast<std::uint32_t>(ret));
            for (std::uint32_t off : {0xf78u, 0xf80u, 0xf84u, 0xfd0u}) snap[side].push_back(off == 0xf78 ? rl(blk[off / 4]) : blk[off / 4]);
            snap[side].push_back(rl(holder[0]));
            snap[side].push_back(rl(out_edge));
            for (float f : out_vec) snap[side].push_back(ch::fbits(f));
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            // a pushed node: its words and its edge (then released)
            const std::uint32_t top = blk[0xf78 / 4];
            if (op == 0 && rl(top) >= 0xC000) {
                for (int c = 0; c < 12; ++c) snap[side].push_back(c >= 3 && c < 9 ? rl(ch::at(top)[c]) : ch::at(top)[c]);
                for (int e = 0; e < 15; ++e) snap[side].push_back(ch::at(ch::at(top)[6])[e]);
                ch::c_free(ch::at(top)[6]);
                ch::c_free(top);
            }
            // release what is left (not logged)
            for (int k = 0; k < n + temps; ++k)
                if (!ch::was_freed(nodes[k])) {
                    for (int s = 0; s < 3; ++s) if (ch::at(nodes[k])[6 + s]) ch::c_free(ch::at(nodes[k])[6 + s]);
                    ch::c_free(nodes[k]);
                }
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}
