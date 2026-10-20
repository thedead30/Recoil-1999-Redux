// Structured native L1 for the zclass_nodes tree walks (P2.6, Switch.c by SUBSYS attribution), each recursing through
// the children (+0x5C count, +0x60 array). The arena fuzz would recurse through random child pointers without bound
// (KG-31, gwNodeFindByName), so they run here on real trees (depth <= 4, 0..3 children each, <= 40 nodes).
//   Node_FindChildByNameRecursive (0x00452770, ECX node, EDX name): null -> 0; the node when its inline name (+0)
//     equals EDX (inlined strcmp); else the children from the LAST to the first, first match returned. Names from a
//     small pool with repeats and case variants, so which duplicate wins is compared.
//   gwNode_VisitTreeUntil1 (0x00452810, ECX node, EDX visitor): calls the visitor (ECX node, EDX visitor); 1 -> return 1;
//     else the children from the last to the first, 1 as soon as a subtree returns 1, else 0. The visitor logs the node
//     and returns 1 for a chosen node (or none), other values now and then (only 1 stops the walk).
//   Node_UpdateLightsRecursive (0x00452860, ECX node, EDX flag): model instance +0x3C set ->
//     ModelInstance_SetCyclePartsFlag0x200 (each 0x1C-byte part of [+0x30], count [+0xC], whose material [+0x14] has
//     0x100 gets 0x200 = EDX bit 0); then the children from the first to the last.
//   ClassNode_ReleaseRecursive (0x004528b0, ECX node): model -> ModelInstance_Call480f80OnCycleParts
//     (Material_ReleaseTextures on each 0x100 material: with 0x200 as well it unloads the texture slot chain [+0x10]:
//     here one slot whose image is the default image 0x004e06e0 (so Image_FreeUnlessDefault frees nothing) and whose
//     state +0x1C is 1 (unloaded: image 0, state 3) or 2 (left alone); no cycle [+0x24]); then the children.
//   ClassNode_ReleaseModelsRecursive (0x004528e0, ECX node, EDX flag): model -> Record_SetFlagBit0 (model +4 bit 0 =
//     EDX bit 0); then the children.
// Models: 1 node in 2 has one (0..3 parts, materials shared between parts and nodes, flag words random). Compared: the
// return (as a node index where it is a node), the visit log by node index, every node, model, part and material word.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Switch.h"
#include "platform/image/original_data.h"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }

struct Shape {
    std::vector<int> parent, name;
    std::vector<int> model;                  // per node: model index or -1
    std::vector<std::vector<int>> parts;     // per model: material index of each part
    std::vector<std::uint32_t> model_words;  // per model: 16 random words
    std::vector<std::uint32_t> mat_flags;    // per material: flag word
    std::vector<std::uint32_t> slot_state;   // per material: its texture slot's state +0x1C (1 or 2)
};

struct Tree {
    std::vector<std::vector<std::uint32_t>> nodes, kids, models, part_arrays, mats, slots;
    std::uint32_t default_image = 0;  // this side's address of the default image 0x004e06e0
};

const char* const kNames[] = {"root", "tank", "Tank", "turret", "wheel", "", "tankx"};

Shape make_shape(std::mt19937& rng)
{
    Shape s;
    std::vector<int> depth{0};
    s.parent.push_back(-1);
    s.name.push_back(static_cast<int>(rng() % 7));
    for (std::size_t i = 0; i < s.parent.size() && s.parent.size() < 40; ++i) {
        if (depth[i] >= 4) continue;
        const int k = static_cast<int>(rng() % 4);
        for (int c = 0; c < k; ++c) {
            s.parent.push_back(static_cast<int>(i));
            depth.push_back(depth[i] + 1);
            s.name.push_back(static_cast<int>(rng() % 7));
        }
    }
    const int nmat = 1 + static_cast<int>(rng() % 4);
    for (int m = 0; m < nmat; ++m) { s.mat_flags.push_back(rng()); s.slot_state.push_back(1 + rng() % 2); }
    for (std::size_t i = 0; i < s.parent.size(); ++i) {
        if (rng() % 2) { s.model.push_back(-1); continue; }
        s.model.push_back(static_cast<int>(s.parts.size()));
        std::vector<int> p(rng() % 4);
        for (int& m : p) m = static_cast<int>(rng() % nmat);
        s.parts.push_back(p);
        for (int w = 0; w < 16; ++w) s.model_words.push_back(rng());
    }
    return s;
}

void build(const Shape& s, Tree& t, int side)
{
    t.default_image = side ? addr(recoil::ImageData_Address(0x004e06e0)) : 0x004e06e0u;
    const std::size_t n = s.parent.size();
    t.nodes.assign(n, std::vector<std::uint32_t>(49, 0));
    t.kids.assign(n, {});
    t.mats.assign(s.mat_flags.size(), std::vector<std::uint32_t>(16, 0));  // +0x24 cycle: null
    t.slots.assign(s.mat_flags.size(), std::vector<std::uint32_t>(9, 0));   // +0x20 next slot: null
    for (std::size_t m = 0; m < s.mat_flags.size(); ++m) {
        t.mats[m][0] = s.mat_flags[m];
        t.mats[m][0x10 / 4] = addr(t.slots[m].data());
        t.slots[m][0] = t.default_image;
        t.slots[m][0x1C / 4] = s.slot_state[m];
    }
    t.models.assign(s.parts.size(), {});
    t.part_arrays.assign(s.parts.size(), {});
    for (std::size_t m = 0; m < s.parts.size(); ++m) {
        t.models[m].assign(s.model_words.begin() + 16 * m, s.model_words.begin() + 16 * (m + 1));
        t.part_arrays[m].assign(7 * s.parts[m].size() + 1, 0x5A5A5A5Au);
        for (std::size_t p = 0; p < s.parts[m].size(); ++p) t.part_arrays[m][7 * p + 0x14 / 4] = addr(t.mats[s.parts[m][p]].data());
        t.models[m][0xC / 4] = static_cast<std::uint32_t>(s.parts[m].size());
        t.models[m][0x30 / 4] = addr(t.part_arrays[m].data());
    }
    for (std::size_t i = 0; i < n; ++i) {
        std::strcpy(reinterpret_cast<char*>(t.nodes[i].data()), kNames[s.name[i]]);
        t.nodes[i][0x3C / 4] = s.model[i] < 0 ? 0u : addr(t.models[s.model[i]].data());
    }
    for (std::size_t i = 1; i < n; ++i) t.kids[s.parent[i]].push_back(addr(t.nodes[i].data()));
    for (std::size_t i = 0; i < n; ++i) {
        t.nodes[i][0x5C / 4] = static_cast<std::uint32_t>(t.kids[i].size());
        t.nodes[i][0x60 / 4] = t.kids[i].empty() ? 0u : addr(t.kids[i].data());
    }
}

std::uint32_t index_of(const Tree& t, std::uint32_t p)
{
    if (!p) return 0xFFFFFFFFu;
    for (std::size_t i = 0; i < t.nodes.size(); ++i) if (p == addr(t.nodes[i].data())) return static_cast<std::uint32_t>(i);
    return 0xBADu;
}

// every word, pointers into the tree as roles
void snapshot(const Tree& t, std::vector<std::uint32_t>& out)
{
    auto role = [&](std::uint32_t v) -> std::uint32_t {
        for (std::size_t i = 0; i < t.nodes.size(); ++i) if (v == addr(t.nodes[i].data())) return 0xA0000000u + static_cast<std::uint32_t>(i);
        for (std::size_t i = 0; i < t.kids.size(); ++i) if (!t.kids[i].empty() && v == addr(t.kids[i].data())) return 0xA1000000u + static_cast<std::uint32_t>(i);
        for (std::size_t i = 0; i < t.models.size(); ++i) if (v == addr(t.models[i].data())) return 0xA2000000u + static_cast<std::uint32_t>(i);
        for (std::size_t i = 0; i < t.part_arrays.size(); ++i) if (v == addr(t.part_arrays[i].data())) return 0xA3000000u + static_cast<std::uint32_t>(i);
        for (std::size_t i = 0; i < t.mats.size(); ++i) if (v == addr(t.mats[i].data())) return 0xA4000000u + static_cast<std::uint32_t>(i);
        for (std::size_t i = 0; i < t.slots.size(); ++i) if (v == addr(t.slots[i].data())) return 0xA5000000u + static_cast<std::uint32_t>(i);
        return v && v == t.default_image ? 0xA6000000u : v;
    };
    for (const auto* g : {&t.nodes, &t.kids, &t.models, &t.part_arrays, &t.mats, &t.slots})
        for (const auto& v : *g) for (std::uint32_t w : v) out.push_back(role(w));
}

// the visitor for gwNode_VisitTreeUntil1: logs each node, returns g_result_of(node)
std::vector<std::uint32_t> g_visits;
std::uint32_t g_stop_at = 0;  // node address that returns 1
std::uint32_t g_other = 0;    // node address that returns 2 (not 1: the walk goes on)
int __fastcall visitor(void* node, void*)
{
    g_visits.push_back(addr(node));
    return addr(node) == g_stop_at ? 1 : addr(node) == g_other ? 2 : 0;
}
}  // namespace

TEST(native_zclass_tree_walks_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, std::uint32_t);
    struct Walk { std::uint32_t va; void* port; const char* name; };
    const Walk walks[] = {
        {0x00452770, reinterpret_cast<void*>(&recoil::Node_FindChildByNameRecursive), "Node_FindChildByNameRecursive"},
        {0x00452810, reinterpret_cast<void*>(&recoil::gwNode_VisitTreeUntil1), "gwNode_VisitTreeUntil1"},
        {0x00452860, reinterpret_cast<void*>(&recoil::Node_UpdateLightsRecursive), "Node_UpdateLightsRecursive"},
        {0x004528b0, reinterpret_cast<void*>(&recoil::ClassNode_ReleaseRecursive), "ClassNode_ReleaseRecursive"},
        {0x004528e0, reinterpret_cast<void*>(&recoil::ClassNode_ReleaseModelsRecursive), "ClassNode_ReleaseModelsRecursive"},
    };
    for (int w = 0; w < 5; ++w) {
        const Fn fn[2] = {rt::original<Fn>(walks[w].va), reinterpret_cast<Fn>(walks[w].port)};
        std::mt19937 rng(walks[w].va);
        int compared = 0, hits = 0, changed = 0;
        for (int it = 0; it < 3000; ++it) {
            const Shape s = make_shape(rng);
            const int n = static_cast<int>(s.parent.size());
            const char* q = rng() % 6 == 0 ? "missing" : kNames[rng() % 7];
            const int stop = rng() % 4 == 0 ? -1 : static_cast<int>(rng() % n), other = static_cast<int>(rng() % n);
            const std::uint32_t flag = rng();
            const bool null_node = w == 0 && rng() % 30 == 0;  // only the name search checks for a null node
            std::vector<std::uint32_t> snap[2], before;
            for (int side = 0; side < 2; ++side) {
                Tree t;
                build(s, t, side);
                if (side == 0) snapshot(t, before);
                g_visits.clear();
                g_stop_at = stop < 0 ? 0u : addr(t.nodes[stop].data());
                g_other = addr(t.nodes[other].data());
                void* root = null_node ? nullptr : t.nodes[0].data();
                const std::uint32_t arg = w == 0 ? addr(q) : w == 1 ? addr(reinterpret_cast<void*>(&visitor)) : flag;
                const std::uint32_t r = fn[side](root, arg);
                snap[side].push_back(w == 0 ? index_of(t, r) : r);
                for (std::uint32_t v : g_visits) snap[side].push_back(index_of(t, v));
                snap[side].push_back(0xF0F0F0F0u);
                snapshot(t, snap[side]);
                if (side == 0) {
                    hits += w == 0 ? r != 0 : w == 1 ? r == 1 : 0;
                    std::vector<std::uint32_t> after;
                    snapshot(t, after);
                    changed += after != before;
                }
            }
            CHECK_SNAP(snap[0], snap[1]);
            ++compared;
        }
        std::printf("  %-34s calls %d, %d found / stopped, %d changed a word\n", walks[w].name, compared, hits, changed);
        if (w <= 1) CHECK(hits > 500);
        if (w >= 2) CHECK(changed > 500);
    }
}

// ClassNode_Release (0x004528a0, ECX node): null -> return; else ClassNode_ReleaseRecursive on the tree, then a tail
// jump into TextureManager_LoadPending (0x0046de50), which here finds texture archive list A ([0x0053d768] count,
// [0x0053d76c] records of 0xa4 bytes) already open (one record whose FILE +0x80 is a real msvcrt FILE of an empty temp
// file, so TextureArchiveListA_OpenBySize opens nothing), no texture slots ([0x0053d798] = 0), and closes the archive
// files (TextureArchive_CloseAllFiles: fclose, +0x80 = 0). Compared: the return, the tree words (as above) and the
// archive record.
TEST(native_zclass_class_node_release_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004528a0), reinterpret_cast<Fn>(&recoil::ClassNode_Release)};
    HMODULE crt = GetModuleHandleA("msvcrt.dll");
    auto c_fopen = reinterpret_cast<void*(__cdecl*)(const char*, const char*)>(GetProcAddress(crt, "fopen"));
    auto c_fclose = reinterpret_cast<int(__cdecl*)(void*)>(GetProcAddress(crt, "fclose"));
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "cnr", 0, path);  // creates the (empty) file
    auto img = [](int side, std::uint32_t va) {
        return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
    };
    std::mt19937 rng(0x4528a0);
    int compared = 0, changed = 0;
    for (int it = 0; it < 1000; ++it) {
        const Shape s = make_shape(rng);
        const bool null_node = rng() % 20 == 0;
        std::vector<std::uint32_t> snap[2], before;
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            Tree t;
            build(s, t, side);
            if (side == 0) snapshot(t, before);
            static std::uint32_t record[0xa4 / 4];
            for (auto& w : record) w = 0x11111111u * static_cast<std::uint32_t>(it % 15 + 1);
            void* f = c_fopen(path, "rb");
            record[0x80 / 4] = addr(f);
            *img(side, 0x0053d768) = 1;
            *img(side, 0x0053d76c) = addr(record);
            *img(side, 0x0053d798) = 0;
            const std::uint32_t r = fn[side](null_node ? nullptr : t.nodes[0].data(), 0);
            snap[side].push_back(null_node ? 0xDDDDu : r);  // the null path returns whatever EAX held
            snapshot(t, snap[side]);
            for (int k = 0; k < 0xa4 / 4; ++k) snap[side].push_back(k == 0x80 / 4 ? (record[k] == addr(f) ? 0xF11Eu : record[k]) : record[k]);
            if (record[0x80 / 4]) c_fclose(f);
            if (side == 0) {
                std::vector<std::uint32_t> after;
                snapshot(t, after);
                changed += after != before;
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  ClassNode_Release calls %d, %d changed a tree word\n", compared, changed);
    CHECK(changed > 150);
}
