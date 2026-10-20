// Structured native L1 for the zclass_nodes attach / detach handlers (P2.6), ECX parent, EDX child. Each checks its
// arguments (null parent / child, for some the parent's class data +0x38 -> report, 5) and then runs, or tail-jumps
// into, the generic gwNodeAttachChild (0x004484d0) / gwNodeDetachChild (0x00448660), which grow / shrink the parent's
// children (+0x5c count, +0x60 array) and the child's parents (+0x54 / +0x58) with realloc, touch list 7 and the
// subtree flag 0x80000 (see tests/test_node_attach_native.cpp, whose harness this follows). Some handlers return 0
// whatever the generic call returned (Display_DetachChild): the return is compared, so that quirk is covered.
// Each call: parent with 0..3 children, child with 0..2 parents and one grandchild; for detach the child / parent is
// present in the other's array 0, 1 or 2 times at random places; flags random; arrays in real msvcrt blocks; null
// parent / child / class data 1 call in 10 (none for Class6_DetachChild, which checks nothing). Compared: the return,
// every node word (array pointers replaced by their contents, by role), list 7 and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Animate.h"
#include "GameZRecoil/zClass/Display.h"
#include "GameZRecoil/zClass/Light.h"
#include "GameZRecoil/zClass/Seq.h"
#include "GameZRecoil/zClass/Sound.h"
#include "GameZRecoil/zClass/Switch.h"
#include "GameZRecoil/zClass/Window_nodes.h"
#include "platform/image/original_data.h"

#include <windows.h>

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

using Fn = int(__fastcall*)(void*, void*);
// unchecked: no argument checks at all (Class6_DetachChild): never given nulls - the generic code dereferences them
struct Handler { std::uint32_t va; void* port; const char* name; bool detach; bool unchecked; };

std::vector<Handler> handlers()
{
    return {
        {0x0044f870, reinterpret_cast<void*>(&recoil::Window_DetachChild), "Window_DetachChild", true},
        {0x0044fe50, reinterpret_cast<void*>(&recoil::Display_DetachChild), "Display_DetachChild", true},
        {0x004531c0, reinterpret_cast<void*>(&recoil::Light_DetachChild), "Light_DetachChild", true},
        {0x00452b80, reinterpret_cast<void*>(&recoil::Sound_DetachChild), "Sound_DetachChild", true},
        {0x00452920, reinterpret_cast<void*>(&recoil::Switch_AttachChild), "Switch_AttachChild", false},
        {0x00452970, reinterpret_cast<void*>(&recoil::Switch_DetachChild), "Switch_DetachChild", true},
        {0x00453b40, reinterpret_cast<void*>(&recoil::Anim_AttachChild), "Anim_AttachChild", false},
        {0x00453b80, reinterpret_cast<void*>(&recoil::Anim_DetachChild), "Anim_DetachChild", true},
        {0x00454320, reinterpret_cast<void*>(&recoil::Class6_DetachChild), "Class6_DetachChild", true, true},
    };
}

// Returns the number of calls whose child subtree flag changed (side 0), so each handler's run shows it reached the
// generic code.
int run_handler(const Handler& h)
{
    const Fn fn[2] = {rt::original<Fn>(h.va), reinterpret_cast<Fn>(h.port)};
    std::mt19937 rng(h.va);
    int reached = 0;
    static std::uint32_t parent[49], child[49], grand[49];
    for (int it = 0; it < 4000; ++it) {
        std::uint32_t pi[49], ci[49], gi[49];
        for (auto& w : pi) w = rng();
        for (auto& w : ci) w = rng();
        for (auto& w : gi) w = rng();
        pi[0x24 / 4] = pi[0x24 / 4] & ~1u | (rng() % 3 == 0 ? 1u : 0u);
        pi[0x54 / 4] = 0;  // the parent's own parents: a list-7 push does not recurse
        std::vector<int> kids(rng() % 4, 0), pars(rng() % 3, 0);  // 0 = token, 1 = the other node
        if (h.detach) {
            for (int c = static_cast<int>(rng() % 3); c > 0; --c) kids.insert(kids.begin() + rng() % (kids.size() + 1), 1);
            for (int c = static_cast<int>(rng() % 3); c > 0; --c) pars.insert(pars.begin() + rng() % (pars.size() + 1), 1);
        }
        const unsigned null_kind = rng() % 10 == 0 && !h.unchecked ? 1 + rng() % 3 : 0;  // 1 parent, 2 child, 3 class data
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
            const int r = fn[side](null_kind == 1 ? nullptr : parent, null_kind == 2 ? nullptr : child);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            role[addr(parent)] = 0xA0; role[addr(child)] = 0xA1; role[addr(grand)] = 0xA2;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (int k = 0; k < 49; ++k) if (k != 0x60 / 4) snap[side].push_back(parent[k]);
            // detach keeps the arrays: compare every slot of the original block, the stale ones past the new count too
            const std::uint32_t pn = h.detach ? static_cast<std::uint32_t>(kv.size()) : parent[0x5C / 4];
            for (std::uint32_t k = 0; k < pn && k < 16; ++k) snap[side].push_back(rl(at(parent[0x60 / 4])[k]));
            for (int k = 0; k < 49; ++k) if (k != 0x58 / 4 && k != 0x60 / 4) snap[side].push_back(child[k]);
            const std::uint32_t cn = h.detach ? static_cast<std::uint32_t>(pv.size()) : child[0x54 / 4];
            for (std::uint32_t k = 0; k < cn && k < 16; ++k) snap[side].push_back(rl(at(child[0x58 / 4])[k]));
            snap[side].insert(snap[side].end(), grand, grand + 49);
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0 && ((grand[0x24 / 4] ^ gi[0x24 / 4]) & 0x80000u)) ++reached;
            if (parent[0x60 / 4]) real_free(parent[0x60 / 4]);
            if (child[0x58 / 4]) real_free(child[0x58 / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
    return reached;
}
}  // namespace

TEST(native_zclass_attach_detach_handlers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    for (const Handler& h : handlers()) {
        const int reached = run_handler(h);
        std::printf("  %-28s calls 4000 (%s), %d changed the subtree flag\n", h.name, h.detach ? "detach" : "attach", reached);
        CHECK(reached > 20);
    }
}
