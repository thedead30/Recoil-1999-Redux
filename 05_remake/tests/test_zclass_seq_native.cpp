// Structured native L1 for the Seq.c node functions that allocate or edit heap blocks (P2.6).
//   Seq_Create (0x00453ee0): gwNodeAlloc (0x004478c0; empty pool -> report, 0); class 7, class data calloc(1, 0x28)
//     with +0x10 = 1, registered on list 11 (NodeRegistry_Insert 0x0044ed90); returns the node.
//   NodeType6_Create (0x004542a0): gwNodeAlloc, NOT checked (an empty pool faults at the +0x34 write, as World_Create -
//     KG-40 - not exercised); class 6, class data calloc(1, 0x50) with its defaults; returns the node.
//   Seq_InsertEntry (0x00453f40, ECX seq, EDX child, index, float value): null seq / child / class data -> report, 5;
//     gwNodeAttachChild (0x004484d0) non-zero -> returned; else the class data is realloc'd to 0x28 + 8 * count (+0x1c),
//     count + 1, the entries (+0x20, 8 bytes: node, float) from the index up shift by one and {child, value} goes at
//     the index; 0.
//   Seq_DetachChild (0x00454000, ECX seq, EDX child): null checks as above; gwNodeDetachChild (0x00448660) non-zero ->
//     returned; else the first entry naming the child is removed (the rest shift down, count - 1; none -> nothing), and
//     the cursor +0x14 is reset to 0 when it is no longer below the count; 0.
// Pools, lists and child / parent arrays as in tests/test_node_create_native.cpp and tests/test_node_attach_native.cpp;
// the class data and every array are real msvcrt blocks (realloc moves them), indices 0..count, 0..4 entries, the
// child named by 0..2 of them. Compared by role: returns, every pool / node word, the class data words, the child and
// parent arrays, every list's links and marks, the free-link head and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Seq.h"
#include "platform/image/original_data.h"

#include <windows.h>

#include <algorithm>
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
std::uint32_t real_malloc(std::size_t n)
{
    return addr(reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"))(n));
}
void real_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(at(p)); }
std::uint32_t array_of(const std::vector<std::uint32_t>& v)
{
    if (v.empty()) return 0;
    const std::uint32_t a = real_malloc(4 * v.size());
    for (std::size_t k = 0; k < v.size(); ++k) at(a)[k] = v[k];
    return a;
}
std::uint32_t fbits(float f) { std::uint32_t u; std::memcpy(&u, &f, 4); return u; }

// every list's links (node by role) and marks, the dirty words, the free-link head and the link counter
template <class Role>
void snap_lists(int side, std::vector<std::uint32_t>& out, Role rl)
{
    for (int l = 0; l < 16; ++l) {
        std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
        out.push_back(*img(side, 0x00539bb4u + 12 * l));
        for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { out.push_back(rl(at(p)[0])); out.push_back(at(p)[3]); }
    }
    out.push_back(rl(*img(side, 0x00539c6c)));
    out.push_back(*img(side, 0x00539c74));
}
}  // namespace

TEST(native_zclass_seq_create_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F0 = std::uint32_t(__fastcall*)(int, int);
    const F0 create[2][2] = {{rt::original<F0>(0x00453ee0), reinterpret_cast<F0>(&recoil::Seq_Create)},
                             {rt::original<F0>(0x004542a0), reinterpret_cast<F0>(&recoil::NodeType6_Create)}};
    std::mt19937 rng(0x453ee0);
    int per[2] = {}, empty = 0;
    for (int it = 0; it < 6000; ++it) {
        const int f = it % 2;  // 0 Seq_Create, 1 NodeType6_Create
        const int n = 2 + static_cast<int>(rng() % 5);
        std::vector<std::uint32_t> pool_init(49 * n);
        for (auto& w : pool_init) w = rng();
        std::vector<int> order(n);
        for (int i = 0; i < n; ++i) order[i] = i;
        std::shuffle(order.begin(), order.end(), rng);
        const int chain = f == 0 ? static_cast<int>(rng() % (n + 1)) : 1 + static_cast<int>(rng() % n);
        for (int k = 0; k < chain; ++k) {
            std::uint32_t& w = pool_init[49 * order[k] + 0xC0 / 4];
            w = (w & 0xFF000000u) | (k + 1 < chain ? static_cast<std::uint32_t>(order[k + 1]) : 0xFFFFFFu);
        }
        const std::uint32_t head = chain ? static_cast<std::uint32_t>(order[0]) : 0xFFFFFFFFu;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> pool = pool_init;
            *img(side, 0x00539c94) = addr(pool.data());
            *img(side, 0x004de4c8) = head;
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int i = 0; i < n; ++i) role[addr(&pool[49 * i])] = 0x100u + i;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            const std::uint32_t r = create[f][side](0, 0);
            snap[side].push_back(rl(r));
            const std::uint32_t data = r ? at(r)[0x38 / 4] : 0u;
            if (data) { rl(data); for (std::uint32_t k = 0; k < (f ? 0x50u : 0x28u) / 4; ++k) snap[side].push_back(at(data)[k]); }
            for (int i = 0; i < 49 * n; ++i) snap[side].push_back(i % 49 == 0x38 / 4 ? rl(pool[i]) : pool[i]);
            snap[side].push_back(*img(side, 0x004de4c8));
            snap[side].push_back(*img(side, 0x00539c98));
            snap_lists(side, snap[side], rl);
            if (data) real_free(data);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++per[f];
        empty += chain == 0;
    }
    rt::restore_pristine();
    std::printf("  Seq_Create %d (%d with an empty pool), NodeType6_Create %d\n", per[0], empty, per[1]);
}

namespace {
// Seq_InsertEntry (insert = true) / Seq_DetachChild on a seq node and a child with real arrays and class data
void run_seq_edit(bool insert)
{
    using Ins = int(__fastcall*)(void*, void*, int, float);
    using Det = int(__fastcall*)(void*, void*);
    const Ins ins[2] = {rt::original<Ins>(0x00453f40), reinterpret_cast<Ins>(&recoil::Seq_InsertEntry)};
    const Det det[2] = {rt::original<Det>(0x00454000), reinterpret_cast<Det>(&recoil::Seq_DetachChild)};
    const float values[] = {0.0f, 0.5f, 1.0f, 2.5f, -1.0f, 30.0f};
    std::mt19937 rng(insert ? 0x453f40u : 0x454000u);
    int compared = 0, edited = 0;
    static std::uint32_t seq[49], child[49], grand[49];
    for (int it = 0; it < 4000; ++it) {
        std::uint32_t si[49], ci[49], gi[49], di[10];
        for (auto& w : si) w = rng();
        for (auto& w : ci) w = rng();
        for (auto& w : gi) w = rng();
        for (auto& w : di) w = rng();
        si[0x24 / 4] = si[0x24 / 4] & ~1u | (rng() % 3 == 0 ? 1u : 0u);
        si[0x54 / 4] = 0;
        const int count = static_cast<int>(rng() % 5);
        di[0x1c / 4] = static_cast<std::uint32_t>(count);
        di[0x14 / 4] = rng() % (count + 2);
        std::vector<int> entry_is_child(count, 0), kids(rng() % 4, 0), pars(rng() % 3, 0);
        std::vector<std::uint32_t> entry_val(count);
        for (auto& v : entry_val) v = fbits(values[rng() % 6]);
        if (!insert) {
            for (int c = static_cast<int>(rng() % 3); c > 0 && count; --c) entry_is_child[rng() % count] = 1;
            for (int c = static_cast<int>(rng() % 3); c > 0; --c) kids.insert(kids.begin() + rng() % (kids.size() + 1), 1);
            for (int c = static_cast<int>(rng() % 3); c > 0; --c) pars.insert(pars.begin() + rng() % (pars.size() + 1), 1);
        }
        const int index = static_cast<int>(rng() % (count + 1));
        const float value = values[rng() % 6];
        const unsigned null_kind = rng() % 10 == 0 ? 1 + rng() % 3 : 0;  // 1 seq, 2 child, 3 class data
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(seq, si, sizeof seq);
            std::memcpy(child, ci, sizeof child);
            std::memcpy(grand, gi, sizeof grand);
            std::vector<std::uint32_t> kv, pv;
            for (std::size_t k = 0; k < kids.size(); ++k) kv.push_back(kids[k] ? addr(child) : 0xE0000000u + static_cast<std::uint32_t>(k));
            for (std::size_t k = 0; k < pars.size(); ++k) pv.push_back(pars[k] ? addr(seq) : 0xE1000000u + static_cast<std::uint32_t>(k));
            seq[0x5C / 4] = static_cast<std::uint32_t>(kv.size()); seq[0x60 / 4] = array_of(kv);
            child[0x54 / 4] = static_cast<std::uint32_t>(pv.size()); child[0x58 / 4] = array_of(pv);
            std::uint32_t gk[1] = {addr(grand)};
            child[0x5C / 4] = 1; child[0x60 / 4] = addr(gk);
            grand[0x54 / 4] = 1; grand[0x5C / 4] = 0;
            std::uint32_t data = real_malloc(0x28 + 8 * count);
            std::memcpy(at(data), di, 0x20);
            for (int e = 0; e < count; ++e) {
                at(data)[0x20 / 4 + 2 * e] = entry_is_child[e] ? addr(child) : 0xE2000000u + static_cast<std::uint32_t>(e);
                at(data)[0x24 / 4 + 2 * e] = entry_val[e];
            }
            seq[0x38 / 4] = null_kind == 3 ? 0u : data;
            void* s = null_kind == 1 ? nullptr : seq;
            void* c = null_kind == 2 ? nullptr : child;
            const int r = insert ? ins[side](s, c, index, value) : det[side](s, c);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            role[addr(seq)] = 0xA0; role[addr(child)] = 0xA1; role[addr(grand)] = 0xA2;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (int k = 0; k < 49; ++k) if (k != 0x60 / 4) snap[side].push_back(k == 0x38 / 4 ? rl(seq[k]) : seq[k]);
            const std::uint32_t pn = insert ? seq[0x5C / 4] : static_cast<std::uint32_t>(kv.size());
            for (std::uint32_t k = 0; k < pn && k < 16; ++k) snap[side].push_back(rl(at(seq[0x60 / 4])[k]));
            for (int k = 0; k < 49; ++k) if (k != 0x58 / 4 && k != 0x60 / 4) snap[side].push_back(child[k]);
            const std::uint32_t cn = insert ? child[0x54 / 4] : static_cast<std::uint32_t>(pv.size());
            for (std::uint32_t k = 0; k < cn && k < 16; ++k) snap[side].push_back(rl(at(child[0x58 / 4])[k]));
            snap[side].insert(snap[side].end(), grand, grand + 49);
            // the class data (realloc may have moved it: the node's pointer is the live one)
            const std::uint32_t live = null_kind == 3 ? data : seq[0x38 / 4];
            const int n_after = static_cast<int>(at(live)[0x1c / 4]);
            const int n_words = 0x20 / 4 + 2 * (std::max)(count, (std::min)(n_after, count + 1));
            for (int k = 0; k < n_words; ++k) snap[side].push_back(k >= 0x20 / 4 && k % 2 == 0 ? rl(at(live)[k]) : at(live)[k]);
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0 && r == 0 && n_after != count) ++edited;
            real_free(live);
            if (seq[0x60 / 4]) real_free(seq[0x60 / 4]);
            if (child[0x58 / 4]) real_free(child[0x58 / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  %s calls %d, %d changed the entry count\n", insert ? "Seq_InsertEntry" : "Seq_DetachChild", compared, edited);
    CHECK(edited > 500);
}
}  // namespace

TEST(native_zclass_seq_insert_detach_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    run_seq_edit(true);
    run_seq_edit(false);
}
