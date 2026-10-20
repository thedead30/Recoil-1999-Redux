// Structured native L1 for the zeffect record emitters and instance helpers ported in the cloud (P2 zeffect).
//   Anim_EmitEventRecord (0x00461970; 9 stack words), Effect_QueueSpawnRecord (0x00461aa0; 3 words),
//   Anim_EmitAttachRecord (0x00461ba0; node, two vec3 pointers), Anim_EmitAttachVelRecord (0x00461d00; node, vec3, node,
//   vec3): ECX the animation record (+0 name, +0x94 flags), EDX a node. Each takes ClsRecord_ActiveIndex of its nodes and
//   decides from the flags (0x1000 / 0x400 force bits 0x2000 / 0x800) or from the switches [0x004df9b4] / [0x004df9b8],
//   flag 0x100, the name and the indices whether to keep / announce the record; if either, EffectList_AppendEntry, type,
//   name (strncpy 0x20), the index(es) and the arguments (a null vec3 pointer stores zeros); announced -> the hook
//   [0x0053a2e4] (ECX entry); not kept -> EffectList_DecCount. Returns the entry when announced, else 0.
// Effect list as in tests/test_zeffect_native.cpp (array absent or with room), nodes pool records ([0x00539c94], active
// flag random) or null, flags / switches / names random. The hook logs the entry's written words. Compared: the return
// (entry by index), the hook log, the list globals, the written words of every entry.
//   AnimInstance_Reset (0x0045d6c0; ECX instance): null -> -1; gwNodeGetRoot(+0x40) null -> -1; a root that is not
//   class 1 / 2 -> World_AttachChild([0x00575db8], +0x40) - failing -> -1, else flag 0x100 set; class 1 / 2 -> cleared;
//   +0xa8 = 0; AnimInstance_SaveObjectStates; each of the +0x104 scripts (+0xc0, 0x40 each) gets +0x30 = 0, +0x2c = 0 and
//   Script_Rewind. The world here has no class data, so World_AttachChild refuses (the attach itself is its own test).
//   AnimNode_ResolveTargetByName (0x0045e5c0; ECX instance, EDX name): Node_FindByNameRecursive under +0x64, then under
//   +0x40; else the light table entry (+0x118, 0x2c each, name +0x24.. via AnimNode_FindLightByName, index > 0), the sound
//   table entry (+0x11c), else NodeRegistry_LookupByName(6, name) (list 6 pristine: none).
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zEffect/zeff_anim_init.h"
#include "GameZRecoil/zEffect/zeff_anim_run.h"
#include "GameZRecoil/zEffect/zeff_anim_save.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

using namespace ch;

namespace {
std::vector<std::uint32_t> g_hook;
int g_hook_words = 0;
std::uint32_t g_array = 0;
int __fastcall entry_hook(const std::uint32_t* e, int)
{
    g_hook.push_back(0xE000u + (addr(e) - g_array));
    for (int k = 0; k < g_hook_words; ++k) if (k != 1) g_hook.push_back(e[k]);
    return 0;
}

std::uint32_t call_n(void* f, std::uint32_t c, std::uint32_t d, const std::uint32_t* s, int ns)
{
    std::uint32_t r = 0, saved = 0;
    int n = ns;
    __asm {
        mov saved, esp
        mov esi, s
        mov eax, n
    push_loop:
        test eax, eax
        jz pushed
        dec eax
        push dword ptr [esi + eax * 4]
        jmp push_loop
    pushed:
        mov ecx, c
        mov edx, d
        call f
        mov r, eax
        mov esp, saved
    }
    return r;
}
}  // namespace

TEST(native_zeffect_emit_records_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    struct Emit { std::uint32_t va; void* port; const char* name; const char* args; int words; };
    const Emit emits[] = {
        {0x00461970, reinterpret_cast<void*>(&recoil::Anim_EmitEventRecord), "Anim_EmitEventRecord", "vvvvvvvvv", 20},
        {0x00461aa0, reinterpret_cast<void*>(&recoil::Effect_QueueSpawnRecord), "Effect_QueueSpawnRecord", "vvv", 14},
        {0x00461ba0, reinterpret_cast<void*>(&recoil::Anim_EmitAttachRecord), "Anim_EmitAttachRecord", "npp", 18},
        {0x00461d00, reinterpret_cast<void*>(&recoil::Anim_EmitAttachVelRecord), "Anim_EmitAttachVelRecord", "npnp", 19},
    };
    const char* const names[] = {"", "boom", "attach_me"};
    std::mt19937 rng(0x461970);
    int failed = 0;
    for (const Emit& em : emits) {
        const int nargs = static_cast<int>(std::strlen(em.args));
        int emitted = 0;
        for (int it = 0; it < 3000; ++it) {
            std::uint32_t rec_init[0x98 / 4], pool_init[49 * 4], vals[9];
            for (auto& w : rec_init) w = rng();
            for (auto& w : pool_init) w = rng();
            for (auto& w : vals) w = rng();
            std::memset(rec_init, 0, 0x20);
            std::strcpy(reinterpret_cast<char*>(rec_init), names[rng() % 3]);
            rec_init[0x94 / 4] = rng() % 3 ? rng() & 0x3D00u : 0u;
            for (int k = 0; k < 4; ++k) pool_init[49 * k + 0xc0 / 4] = rng() % 3 ? 0x1000000u : 0u;
            int node_sel[5];
            for (int& s : node_sel) s = rng() % 5 == 0 ? -1 : static_cast<int>(rng() % 4);
            bool vec_null[5];
            for (bool& b : vec_null) b = rng() % 4 == 0;
            const bool has_array = rng() % 3 != 0, has_hook = rng() % 4 != 0;
            const std::uint32_t sw[2] = {rng() % 3 ? 1u : 0u, rng() % 3 ? 1u : 0u}, count0 = rng() % 3, mask = rng();
            std::vector<std::uint32_t> snap[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                static std::uint32_t pool[49 * 4], vecs[5][3];
                std::memcpy(pool, pool_init, sizeof pool);
                for (int k = 0; k < 5; ++k) for (int w = 0; w < 3; ++w) vecs[k][w] = 0x3F800000u + 0x100000u * (k * 3 + w);
                const std::uint32_t arr = has_array ? addr(c_malloc(0x50 * 4)) : 0u;
                *img(side, 0x0053a2d8) = arr;
                *img(side, 0x0053a2dc) = has_array ? 4u : 0u;
                *img(side, 0x0053a2e0) = has_array ? count0 : 0u;
                *img(side, 0x0053a2e8) = mask;
                *img(side, 0x0053a2e4) = has_hook ? addr(reinterpret_cast<void*>(&entry_hook)) : 0u;
                *img(side, 0x004df9b4) = sw[0];
                *img(side, 0x004df9b8) = sw[1];
                *img(side, 0x00539c94) = addr(pool);
                std::uint32_t args[9];
                for (int k = 0; k < nargs; ++k) {
                    const char kind = em.args[k];
                    args[k] = kind == 'v' ? vals[k] : kind == 'n' ? (node_sel[k] < 0 ? 0u : addr(&pool[49 * node_sel[k]]))
                                                                   : (vec_null[k] ? 0u : addr(vecs[k]));
                }
                g_hook.clear();
                g_hook_words = em.words;
                g_array = arr;
                const std::uint32_t node = node_sel[4] < 0 ? 0u : addr(&pool[49 * node_sel[4]]);
                const std::uint32_t r = call_n(side ? em.port : rt::original<void*>(em.va), addr(rec_init), node, args, nargs);
                const std::uint32_t now = *img(side, 0x0053a2d8);
                g_array = now;
                snap[side].push_back(r ? 0xE000u + (r - now) : 0u);
                snap[side].insert(snap[side].end(), g_hook.begin(), g_hook.end());
                snap[side].push_back(now == arr ? 1u : 2u);
                snap[side].push_back(*img(side, 0x0053a2dc));
                const std::uint32_t n = *img(side, 0x0053a2e0);
                snap[side].push_back(n);
                // the entry the call appended, if any (kept: the count went up; announced only: back down by DecCount but
                // the entry is still in the array); only the words the emitter writes
                const std::uint32_t first = has_array ? count0 : 0u;
                if (now && (r != 0 || n != first))
                    for (int w = 0; w < em.words; ++w) snap[side].push_back(at(now)[20 * first + w]);
                if (side == 0) emitted += r != 0;
                if (now) c_free(now);
                if (arr && arr != now) c_free(arr);
            }
            if (snap[0] != snap[1]) { if (!failed++) std::printf("  DIFF %s call %d\n", em.name, it); }
        }
        std::printf("  %-28s calls 3000, %d announced\n", em.name, emitted);
        if (emitted < 300) ++failed;
    }
    rt::restore_pristine();
    CHECK_EQ(failed, 0);
}

TEST(native_zeffect_instance_reset_and_resolve_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t*, const char*);
    const Fn reset[2] = {rt::original<Fn>(0x0045d6c0), reinterpret_cast<Fn>(&recoil::AnimInstance_Reset)};
    const Fn resolve[2] = {rt::original<Fn>(0x0045e5c0), reinterpret_cast<Fn>(&recoil::AnimNode_ResolveTargetByName)};
    const char* const names[] = {"a", "b", "lamp", "horn", "zz"};
    std::mt19937 rng(0x45d6c0);
    int compared = 0, hits = 0;
    for (int it = 0; it < 4000; ++it) {
        const int f = it % 2;
        std::uint32_t inst_init[0x130 / 4], scripts_init[2 * 16], tree_init[4][49];
        for (auto& w : inst_init) w = rng();
        for (auto& w : scripts_init) w = rng();
        for (auto& t : tree_init) for (auto& w : t) w = rng();
        const int root_cls = static_cast<int>(rng() % 4), nscripts = static_cast<int>(rng() % 3);
        int nm[4], light_nm[3], sound_nm[3];
        for (int& x : nm) x = static_cast<int>(rng() % 5);
        for (int k = 0; k < 3; ++k) { light_nm[k] = static_cast<int>(rng() % 5); sound_nm[k] = static_cast<int>(rng() % 5); }
        const int nlights = static_cast<int>(rng() % 4), nsounds = static_cast<int>(rng() % 4);
        const char* q = names[rng() % 5];
        const bool null_inst = f == 0 && rng() % 20 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t inst[0x130 / 4], scripts[2 * 16], tree[4][49], kids0[1], kids2[1], world[49], lights[3 * 11], sounds[3 * 11];
            std::memcpy(inst, inst_init, sizeof inst);
            std::memcpy(scripts, scripts_init, sizeof scripts);
            std::memcpy(tree, tree_init, sizeof tree);
            // node 0 -> child 1 (the +0x40 subtree); node 2 -> child 3 (the +0x64 subtree); node 1's parent is node 0
            // no parents / children: count AND array pointer (a random +0x58 reached realloc through World_AttachChild ->
            // World_InsertChildInCell, corrupting the heap on both sides)
            for (int k = 0; k < 4; ++k) {
                std::strcpy(reinterpret_cast<char*>(tree[k]), names[nm[k]]);
                tree[k][0x54 / 4] = 0; tree[k][0x58 / 4] = 0; tree[k][0x5c / 4] = 0; tree[k][0x60 / 4] = 0;
            }
            kids0[0] = addr(tree[1]); kids2[0] = addr(tree[3]);
            tree[0][0x5c / 4] = 1; tree[0][0x60 / 4] = addr(kids0);
            tree[2][0x5c / 4] = 1; tree[2][0x60 / 4] = addr(kids2);
            tree[0][0x34 / 4] = static_cast<std::uint32_t>(root_cls);
            inst[0x40 / 4] = addr(tree[0]);
            inst[0x64 / 4] = addr(tree[2]);
            // +0x104 scripts, +0x105 saved nodes (0), +0x107 lights, +0x108 sounds
            reinterpret_cast<unsigned char*>(inst)[0x104] = static_cast<unsigned char>(nscripts);
            reinterpret_cast<unsigned char*>(inst)[0x105] = 0;
            reinterpret_cast<unsigned char*>(inst)[0x107] = static_cast<unsigned char>(nlights);
            reinterpret_cast<unsigned char*>(inst)[0x108] = static_cast<unsigned char>(nsounds);
            inst[0xc0 / 4] = addr(scripts);
            for (int k = 0; k < 3; ++k) {
                static char lname[3][8], sname[3][8];
                std::strcpy(lname[k], names[light_nm[k]]); std::strcpy(sname[k], names[sound_nm[k]]);
                lights[11 * k + 0x24 / 4] = addr(lname[k]); lights[11 * k + 0x28 / 4] = 0x1000u + k;
                sounds[11 * k + 0x24 / 4] = addr(sname[k]); sounds[11 * k + 0x28 / 4] = 0x2000u + k;
            }
            inst[0x118 / 4] = addr(lights);
            inst[0x11c / 4] = addr(sounds);
            std::memset(world, 0, sizeof world);
            world[0x34 / 4] = 2;
            // a world always has class data: World_AttachChild reads it (+0x40 ...) without a null check - with none the
            // original faulted at 0x00451100. A zeroed block = an empty grid (no cells), so the child joins the world list.
            static std::uint32_t world_data[64];
            std::memset(world_data, 0, sizeof world_data);
            world[0x38 / 4] = addr(world_data);
            *img(side, 0x00575db8) = addr(world);
            const std::uint32_t r = (f ? resolve : reset)[side](null_inst ? nullptr : inst, q);
            Roles rl;
            for (int k = 0; k < 4; ++k) rl.set(tree[k], 0xA0u + k);
            for (int k = 0; k < 3; ++k) { rl.set(lights + 11 * k, 0xB0u + k); rl.set(sounds + 11 * k, 0xC0u + k); }
            snap[side].push_back(f ? rl(r) : r);
            if (!f) {
                snap[side].push_back(inst[0x94 / 4]);
                snap[side].push_back(inst[0xa8 / 4]);
                snap[side].insert(snap[side].end(), scripts, scripts + 32);
                for (auto& t : tree) for (int w = 0; w < 49; ++w) snap[side].push_back(rl(t[w]));
            }
            if (side == 0) hits += f ? r != 0 : r == 0;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  AnimInstance_Reset / AnimNode_ResolveTargetByName calls %d, %d succeeded\n", compared, hits);
    CHECK(hits > 800);
}

// AnimInstance_BindNode (0x0045ed80; ECX instance, EDX node): null either or state +0x98 == 5 -> 0. +0x40 = the node.
// When the old +0x40 equalled +0x64: the node's name to +0x20 and to +0x44, +0x64 = the node. Else the name to +0x20 and
// Node_FindByNameRecursive(node, +0x44): found -> +0x64 = it; not found -> report, state 5, 0. Then every saved-node
// entry (+0x110, 0x60 each, count +0x105) and every +0x114 entry (0x28 each, count +0x106) with a non-null +0x24 is
// re-resolved by name (AnimNode_ResolveTargetByName; a failure -> report, state 5, 0); every condition (+0x128, 0x30
// each, count +0x10b) of type 2 / 3 is resolved (the first by ResolveTargetByName, the next ones under the previous hit
// with Node_FindChildByNameRecursive; type 2 stores it at +0x2c and restarts). Returns the instance.
// Real trees and tables with names from a small pool (some unresolvable). Compared: the return, the instance words
// (tree pointers by role), every table entry word.
TEST(native_zeffect_instance_bind_node_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t*, std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x0045ed80), reinterpret_cast<Fn>(&recoil::AnimInstance_BindNode)};
    const char* const names[] = {"a", "b", "c", "lamp", "zz"};
    std::mt19937 rng(0x45ed80);
    int compared = 0, bound = 0;
    for (int it = 0; it < 4000; ++it) {
        std::uint32_t inst_init[0x130 / 4];
        for (auto& w : inst_init) w = rng();
        int nm[6], saved_nm[3], other_nm[3], cond_nm[3], cond_type[3];
        for (int& x : nm) x = static_cast<int>(rng() % 5);
        for (int k = 0; k < 3; ++k) {
            saved_nm[k] = rng() % 4 ? static_cast<int>(rng() % 3) : 4;
            other_nm[k] = rng() % 4 ? static_cast<int>(rng() % 3) : 4;
            cond_nm[k] = static_cast<int>(rng() % 5);
            cond_type[k] = static_cast<int>(rng() % 4);
        }
        const int nsaved = static_cast<int>(rng() % 3), nother = static_cast<int>(rng() % 3), ncond = static_cast<int>(rng() % 4);
        const bool same = rng() % 2, dead = rng() % 15 == 0, null_node = rng() % 30 == 0;
        const int prev_name = static_cast<int>(rng() % 5);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t inst[0x130 / 4], tree[6][49], kids[6][2], saved[3 * 24], other[3 * 10], conds[3 * 12], lights[1];
            std::memcpy(inst, inst_init, sizeof inst);
            // trees: 0 -> 1, 2 (the new node); 3 -> 4, 5 (the old +0x40 / +0x64)
            for (int k = 0; k < 6; ++k) { std::memset(tree[k], 0, sizeof tree[k]); std::strcpy(reinterpret_cast<char*>(tree[k]), names[nm[k]]); }
            kids[0][0] = addr(tree[1]); kids[0][1] = addr(tree[2]); tree[0][0x5c / 4] = 2; tree[0][0x60 / 4] = addr(kids[0]);
            kids[3][0] = addr(tree[4]); kids[3][1] = addr(tree[5]); tree[3][0x5c / 4] = 2; tree[3][0x60 / 4] = addr(kids[3]);
            inst[0x40 / 4] = addr(tree[3]);
            inst[0x64 / 4] = same ? addr(tree[3]) : addr(tree[4]);
            std::memset(reinterpret_cast<char*>(inst) + 0x44, 0, 0x20);
            std::strcpy(reinterpret_cast<char*>(inst) + 0x44, names[prev_name]);
            unsigned char* b = reinterpret_cast<unsigned char*>(inst);
            b[0x98] = dead ? 5 : 1;
            b[0x105] = static_cast<unsigned char>(nsaved);
            b[0x106] = static_cast<unsigned char>(nother);
            b[0x107] = 0; b[0x108] = 0;
            b[0x10b] = static_cast<unsigned char>(ncond);
            std::memset(saved, 0, sizeof saved); std::memset(other, 0, sizeof other); std::memset(conds, 0, sizeof conds);
            for (int k = 0; k < 3; ++k) {
                std::strcpy(reinterpret_cast<char*>(saved + 24 * k), names[saved_nm[k]]);
                saved[24 * k + 0x24 / 4] = 0xABCD0000u + k;  // non-null: re-resolved
                std::strcpy(reinterpret_cast<char*>(other + 10 * k), names[other_nm[k]]);
                other[10 * k + 0x24 / 4] = 0xABCE0000u + k;
                reinterpret_cast<unsigned char*>(conds + 12 * k)[4] = static_cast<unsigned char>(cond_type[k]);
                std::strcpy(reinterpret_cast<char*>(conds + 12 * k + 3), names[cond_nm[k]]);
            }
            inst[0x110 / 4] = addr(saved); inst[0x114 / 4] = addr(other); inst[0x128 / 4] = addr(conds);
            inst[0x118 / 4] = addr(lights); inst[0x11c / 4] = addr(lights);
            const std::uint32_t r = fn[side](inst, null_node ? nullptr : tree[0]);
            Roles rl;
            for (int k = 0; k < 6; ++k) rl.set(tree[k], 0xA0u + k);
            rl.set(inst, 0xB0);
            snap[side].push_back(rl(r));
            for (int k = 0; k < 0x130 / 4; ++k) snap[side].push_back(rl(inst[k]));
            for (auto w : saved) snap[side].push_back(rl(w));
            for (auto w : other) snap[side].push_back(rl(w));
            for (auto w : conds) snap[side].push_back(rl(w));
            if (side == 0) bound += r != 0;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  AnimInstance_BindNode calls %d, %d bound\n", compared, bound);
    CHECK(bound > 500);
}
