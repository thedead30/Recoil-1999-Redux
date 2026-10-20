// Structured native L1s for the turret functions on heap blocks, node trees and the node registry (P3 turret, ported
// in the cloud):
//  - Turret_ReleaseSounds (0x00436e00): Weapon_FreeBeamInstance of +0x104 when set, then Weapon_UntagTargetTree of the
//    node +0xC (clears its tag record from the tree, drops flag 0x40 when the record's +4 is set, frees the record).
//  - Node_SetOwnerAndFlagsRecursive (0x00437e60; ECX node, EDX owner, stack flags; ret 4): +0x40 = owner, +0x24 |= flags
//    on the node and every descendant.
//  - Node_ApplyToMeshesRecursive (0x00437ea0; ECX node, EDX bit): Record_SetFlagBit0(model, bit) on the model of the
//    node (gwNodeGetModel) and of every descendant.
//  - NamedNodePair_Resolve (0x00438990; ECX pair, stack name; ret 4): pair[0] = NodeRegistry_LookupByName(6, name),
//    pair[1] = the child named by 0x004db5ec under it (Node_FindChildByNameRecursive); returns the pair.
// Real trees (up to 30 nodes); models are 0x58-byte blocks; the registry list 6 is built from this test's own links
// (node, prev, next, mark) behind the list's holder and put back afterwards. Compared: freed pointers by role and the
// nodes' flag / owner / tag words; model flag words; the pair by node index.
#include "test.h"
#include "cloud_harness.h"
#include "Battlesport/turret.h"
#include "unattributed/turret.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
struct Tree {
    std::vector<std::vector<std::uint32_t>> node, kids;
    Tree(const std::vector<int>& parent)
        : node(parent.size(), std::vector<std::uint32_t>(0xc4 / 4, 0)), kids(parent.size())
    {
        for (std::size_t k = 1; k < parent.size(); ++k) kids[parent[k]].push_back(ch::addr(node[k].data()));
        for (std::size_t k = 0; k < parent.size(); ++k) {
            node[k][0x5c / 4] = static_cast<std::uint32_t>(kids[k].size());
            node[k][0x60 / 4] = kids[k].empty() ? 0u : ch::addr(kids[k].data());
        }
    }
    std::uint32_t index(std::uint32_t p) const
    {
        for (std::size_t k = 0; k < node.size(); ++k) if (p == ch::addr(node[k].data())) return static_cast<std::uint32_t>(k + 1);
        return p ? 0xbad : 0;
    }
};
std::vector<int> random_parents(std::mt19937& rng, int max)
{
    const int n = 1 + static_cast<int>(rng() % max);
    std::vector<int> parent(n, -1);
    for (int k = 1; k < n; ++k) parent[k] = static_cast<int>(rng() % k);
    return parent;
}
}  // namespace

TEST(native_turret_release_sounds_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int);
    const F fn[2] = {rt::original<F>(0x00436e00), reinterpret_cast<F>(&recoil::Turret_ReleaseSounds)};
    std::mt19937 rng(0x436e00);
    for (int it = 0; it < 500; ++it) {
        const std::vector<int> parent = random_parents(rng, 8);
        const int n = static_cast<int>(parent.size());
        const bool beam = rng() % 3 != 0, tagged = rng() % 3 != 0, rec4 = rng() % 2 != 0, null_node = rng() % 8 == 0;
        std::vector<std::uint32_t> f24(n);
        for (auto& f : f24) f = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            Tree t(parent);
            ch::Roles rl;
            std::uint32_t turret[0x108 / 4] = {};
            if (beam) { turret[0x104 / 4] = ch::addr(ch::c_malloc(0x20)); rl.set(turret[0x104 / 4], 1); }
            std::uint32_t rec = 0;
            if (tagged) {
                auto* r = static_cast<std::uint32_t*>(ch::c_malloc(16));
                r[0] = 0x55; r[1] = rec4 ? 0x66 : 0; r[2] = 0x77; r[3] = 0x88;
                rec = ch::addr(r);
                rl.set(rec, 2);
            }
            for (int k = 0; k < n; ++k) {
                t.node[k][0x24 / 4] = f24[k];
                t.node[k][0xbc / 4] = rec;  // the whole tree carries the record, as Weapon_TagTargetTree leaves it
            }
            turret[0xc / 4] = null_node ? 0u : ch::addr(t.node[0].data());
            ch::freed().clear();
            {
                ch::FreeHook hook;
                snap[side].push_back(static_cast<std::uint32_t>(fn[side](turret, 0)));
            }
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            for (int k = 0; k < n; ++k) {
                snap[side].push_back(t.node[k][0x24 / 4]);
                snap[side].push_back(rl(t.node[k][0xbc / 4]));
            }
            if (beam && !ch::was_freed(turret[0x104 / 4])) ch::c_free(turret[0x104 / 4]);
            if (rec && !ch::was_freed(rec)) ch::c_free(rec);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

TEST(native_turret_node_trees_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using S = int(__fastcall*)(std::uint32_t*, std::uint32_t, std::uint32_t);
    using M = int(__fastcall*)(std::uint32_t*, std::uint32_t);
    const S owner[2] = {rt::original<S>(0x00437e60), reinterpret_cast<S>(&recoil::Node_SetOwnerAndFlagsRecursive)};
    const M meshes[2] = {rt::original<M>(0x00437ea0), reinterpret_cast<M>(&recoil::Node_ApplyToMeshesRecursive)};
    std::mt19937 rng(0x437e60);
    for (int it = 0; it < 2000; ++it) {
        const std::vector<int> parent = random_parents(rng, 30);
        const int n = static_cast<int>(parent.size()), root = static_cast<int>(rng() % n);
        const bool mesh_op = rng() % 2 != 0;
        const std::uint32_t own = rng(), flags = rng() % 3 ? 1u << (rng() % 32) : rng(), bit = rng();
        std::vector<std::uint32_t> f24(n), m4(n);
        std::vector<bool> has_model(n);
        for (int k = 0; k < n; ++k) { f24[k] = rng(); m4[k] = rng(); has_model[k] = rng() % 3 != 0; }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            Tree t(parent);
            std::vector<std::vector<std::uint32_t>> model(n, std::vector<std::uint32_t>(0x58 / 4, 0));
            for (int k = 0; k < n; ++k) {
                t.node[k][0x24 / 4] = f24[k];
                model[k][1] = m4[k];
                t.node[k][0x3c / 4] = has_model[k] ? ch::addr(model[k].data()) : 0u;
            }
            const int ret = mesh_op ? meshes[side](t.node[root].data(), bit) : owner[side](t.node[root].data(), own, flags);
            (void)ret;  // returns what EAX held
            for (int k = 0; k < n; ++k) {
                snap[side].push_back(t.node[k][0x24 / 4]);
                snap[side].push_back(t.node[k][0x40 / 4]);
                snap[side].push_back(model[k][1]);
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

TEST(native_turret_named_node_pair_resolve_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*);
    const F fn[2] = {rt::original<F>(0x00438990), reinterpret_cast<F>(&recoil::NamedNodePair_Resolve)};
    const std::string child_name = reinterpret_cast<const char*>(0x004db5ec);  // as the original image holds it
    const char* const pool[] = {"turret1", "turret2", "Turret1", "base", ""};
    std::mt19937 rng(0x438990);
    int compared = 0, found = 0;
    for (int it = 0; it < 2000; ++it) {
        const std::vector<int> parent = random_parents(rng, 12);
        const int n = static_cast<int>(parent.size());
        std::vector<int> name(n);
        for (int& x : name) x = static_cast<int>(rng() % 6);  // 5 = the child name
        // registry list 6: some of the tree's nodes, in random order
        std::vector<int> listed;
        for (int k = 0; k < n; ++k) if (rng() % 2) listed.push_back(k);
        std::shuffle(listed.begin(), listed.end(), rng);
        const std::string query = rng() % 5 ? pool[rng() % 5] : child_name;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            Tree t(parent);
            for (int k = 0; k < n; ++k)
                std::strcpy(reinterpret_cast<char*>(t.node[k].data()), name[k] == 5 ? child_name.c_str() : pool[name[k]]);
            std::vector<std::uint32_t> links(4 * listed.size() + 4, 0);
            for (std::size_t i = 0; i < listed.size(); ++i) {
                std::uint32_t* l = &links[4 * i];
                l[0] = ch::addr(t.node[listed[i]].data());
                l[1] = i ? ch::addr(&links[4 * (i - 1)]) : 0u;
                l[2] = i + 1 < listed.size() ? ch::addr(&links[4 * (i + 1)]) : 0u;
            }
            std::uint32_t* holder = ch::at(*ch::img(side, 0x004ddef8 + 4 * 6));
            const std::uint32_t saved = *holder;
            *holder = listed.empty() ? 0u : ch::addr(links.data());
            std::uint32_t pair[2] = {0xaaaa, 0xbbbb};
            const std::uint32_t r = fn[side](pair, 0, query.c_str());
            *holder = saved;
            snap[side].push_back(r == ch::addr(pair) ? 1u : 0xbadu);
            snap[side].push_back(t.index(pair[0]));
            snap[side].push_back(t.index(pair[1]));
            if (side == 0) found += pair[1] != 0;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  NamedNodePair_Resolve calls %d, %d with the child found\n", compared, found);
    CHECK(found > 50);
}
