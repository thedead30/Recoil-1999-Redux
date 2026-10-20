// Structured native L1 for gwNodeAttachChild (0x004484d0) and gwNodeDetachChild (0x00448660) (ECX parent, EDX child).
// Attach: the child is appended to the parent's children (+0x5c count, +0x60 array) and the parent to the child's
// parents (+0x54 / +0x58), both grown with realloc (0x004cc4ec); a second parent clears 0x80000 through the child's
// subtree (gwNodeSetSubtreeFlag80000 0x00449b40, EDX 0).
// Detach: the first occurrence of the child is removed from the parent's children (the rest shift down; not found ->
// report, nothing removed) and the first occurrence of the parent from the child's parents; a child left with exactly
// one parent while the PARENT's +0x24 has bit 0x80000 gets 0x80000 set through its subtree (EDX 1).
// Both: parent +0x2c bit 1; a parent without +0x24 bit 0 goes on list 7 (NodeRegistry_Insert); parent +0x24 bits 0, 1.
// Each: parent with 0..3 children, child with 0..2 parents and one grandchild, the child / parent present in the other's
// array 0, 1 or 2 times at random places, subtree flags random; arrays in real msvcrt blocks. Compared: returns, every
// node word (array pointers replaced by their contents, by role), list 7 and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Class.h"
#include "GameZRecoil/zClass/Object3d.h"
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
}  // namespace

namespace {
using Fn = int(__fastcall*)(void*, void*);
// nulls: also pass a null parent / child / parent class data now and then (the Object3D wrappers check them)
void run_attach(const Fn attach[2], const Fn detach[2], std::uint32_t seed, const char* name, bool nulls)
{
    std::mt19937 rng(seed);
    int compared = 0, per[2] = {}, subtree = 0;
    static std::uint32_t parent[49], child[49], grand[49];
    for (int it = 0; it < 8000; ++it) {
        const int f = it % 2;
        std::uint32_t pi[49], ci[49], gi[49];
        for (auto& w : pi) w = rng();
        for (auto& w : ci) w = rng();
        for (auto& w : gi) w = rng();
        pi[0x24 / 4] = pi[0x24 / 4] & ~1u | (rng() % 3 == 0 ? 1u : 0u);
        pi[0x54 / 4] = 0;  // the parent's own parents: a list-7 push does not recurse
        // the parent's children / the child's parents: tokens, with 0..2 copies of the other node at random places
        std::vector<int> kids(rng() % 4, 0), pars(rng() % 3, 0);  // 0 = token, 1 = the other node
        if (f == 1) {
            for (int c = static_cast<int>(rng() % 3); c > 0; --c) kids.insert(kids.begin() + rng() % (kids.size() + 1), 1);
            for (int c = static_cast<int>(rng() % 3); c > 0; --c) pars.insert(pars.begin() + rng() % (pars.size() + 1), 1);
        }
        const unsigned null_kind = nulls && rng() % 10 == 0 ? 1 + rng() % 3 : 0;  // 1 parent, 2 child, 3 class data
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(parent, pi, sizeof parent);
            std::memcpy(child, ci, sizeof child);
            std::memcpy(grand, gi, sizeof grand);
            std::vector<std::uint32_t> kv, pv;
            for (std::size_t k = 0; k < kids.size(); ++k) kv.push_back(kids[k] ? addr(child) : 0xE0000000u + static_cast<std::uint32_t>(k));
            for (std::size_t k = 0; k < pars.size(); ++k) pv.push_back(pars[k] ? addr(parent) : 0xE1000000u + static_cast<std::uint32_t>(k));
            parent[0x5C / 4] = static_cast<std::uint32_t>(kv.size()); parent[0x60 / 4] = array_of(kv);
            child[0x54 / 4] = static_cast<std::uint32_t>(pv.size()); child[0x58 / 4] = array_of(pv);
            std::uint32_t gk[1] = {addr(grand)};
            child[0x5C / 4] = 1; child[0x60 / 4] = addr(gk);
            grand[0x54 / 4] = 1; grand[0x5C / 4] = 0;
            if (null_kind == 3) parent[0x38 / 4] = 0;
            const int r = (f ? detach : attach)[side](null_kind == 1 ? nullptr : parent, null_kind == 2 ? nullptr : child);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            role[addr(parent)] = 0xA0; role[addr(child)] = 0xA1; role[addr(grand)] = 0xA2;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (int k = 0; k < 49; ++k) if (k != 0x60 / 4) snap[side].push_back(parent[k]);
            // detach keeps the arrays: compare every slot of the original block, the stale ones past the new count too
            const std::uint32_t pn = f ? static_cast<std::uint32_t>(kv.size()) : parent[0x5C / 4];
            for (std::uint32_t k = 0; k < pn && k < 16; ++k) snap[side].push_back(rl(at(parent[0x60 / 4])[k]));
            for (int k = 0; k < 49; ++k) if (k != 0x58 / 4 && k != 0x60 / 4) snap[side].push_back(child[k]);
            const std::uint32_t cn = f ? static_cast<std::uint32_t>(pv.size()) : child[0x54 / 4];
            for (std::uint32_t k = 0; k < cn && k < 16; ++k) snap[side].push_back(rl(at(child[0x58 / 4])[k]));
            snap[side].insert(snap[side].end(), grand, grand + 49);
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0 && ((grand[0x24 / 4] ^ gi[0x24 / 4]) & 0x80000u)) ++subtree;
            if (parent[0x60 / 4]) real_free(parent[0x60 / 4]);
            if (child[0x58 / 4]) real_free(child[0x58 / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++per[f];
        ++compared;
    }
    rt::restore_pristine();
    CHECK(subtree > 200);
    std::printf("  %s: attach calls %d, detach calls %d, %d changed the subtree flag (0..2 copies, not found%s)\n", name, per[0], per[1],
                subtree, nulls ? ", null arguments" : "");
}
}  // namespace

TEST(native_gw_node_attach_detach_child_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const Fn attach[2] = {rt::original<Fn>(0x004484d0), reinterpret_cast<Fn>(&recoil::gwNodeAttachChild)};
    const Fn detach[2] = {rt::original<Fn>(0x00448660), reinterpret_cast<Fn>(&recoil::gwNodeDetachChild)};
    run_attach(attach, detach, 0x4484d0, "gwNodeAttachChild / gwNodeDetachChild", false);
}

// Object3D_AttachChild (0x0044db10) / Object3D_DetachChild (0x0044db60): a null parent, a null child or a parent without
// class data (+0x38) -> report, 5; else a tail jump into gwNodeAttachChild / gwNodeDetachChild.
TEST(native_object3d_attach_detach_child_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const Fn attach[2] = {rt::original<Fn>(0x0044db10), reinterpret_cast<Fn>(&recoil::Object3D_AttachChild)};
    const Fn detach[2] = {rt::original<Fn>(0x0044db60), reinterpret_cast<Fn>(&recoil::Object3D_DetachChild)};
    run_attach(attach, detach, 0x44db10, "Object3D_AttachChild / Object3D_DetachChild", true);
}
