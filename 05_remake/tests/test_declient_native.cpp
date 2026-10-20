// Structured native L1 for declient functions ported in the cloud (P2 declient) that the arena fuzz cannot exercise
// fairly: callbacks through pointers, heap objects, record ranges, Object3D placement on real nodes.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zDEClient/zdec_crater.h"
#include "GameZRecoil/zDEClient/zdec_init.h"
#include "GameZRecoil/zDEClient/zdec_qsand.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

using namespace ch;

// Record52_CopyRange (0x00458a30; stack: first, last, destination; ret 0xc): copies the 0x34-byte records [first, last)
// to the destination (a null destination copies nothing but still advances) and returns the destination's end; first ==
// last returns the destination unchanged. Ranges of 0..5 records, destination real or null. Compared: the return by
// offset and the destination bytes.
TEST(native_declient_record52_copy_range_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(int, int, const std::uint32_t*, const std::uint32_t*, std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x00458a30), reinterpret_cast<Fn>(&recoil::Record52_CopyRange)};
    std::mt19937 rng(0x458a30);
    int compared = 0;
    for (int it = 0; it < 4000; ++it) {
        const int n = static_cast<int>(rng() % 6);
        std::vector<std::uint32_t> src(13 * 6);
        for (auto& w : src) w = rng();
        const bool null_dst = rng() % 5 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<std::uint32_t> dst(13 * 7, 0xA5A5A5A5u);
            const std::uint32_t* d = null_dst ? nullptr : dst.data();
            const std::uint32_t r = fn[side](0, 0, src.data(), src.data() + 13 * n, const_cast<std::uint32_t*>(d));
            snap[side].push_back(r - addr(d));
            snap[side].insert(snap[side].end(), dst.begin(), dst.end());
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  Record52_CopyRange calls %d\n", compared);
}

namespace {
std::vector<std::uint32_t> g_log;
int __fastcall cmd_hook(std::uint32_t* interp, std::uint32_t user, std::uint32_t arg)
{
    g_log.push_back(addr(interp)); g_log.push_back(user); g_log.push_back(arg);
    return 7;
}
int __fastcall record_cb_a(const std::uint32_t* data, int)
{
    g_log.push_back(0xAAAA);
    for (int k = 0; k < 12; ++k) g_log.push_back(data[k]);
    return 0;
}
int __fastcall record_cb_b(const std::uint32_t* data, int)
{
    g_log.push_back(0xBBBB);
    for (int k = 0; k < 12; ++k) g_log.push_back(data[k]);
    return 0;
}
}  // namespace

// Script_CmdCallback (0x0045c6f0; ECX interpreter, one stack argument: command; ret 4): hook +0x6c set -> hook(ECX
// interpreter, EDX user word +0x70, stack: command +0xc); returns 2. DEClient_DispatchRecords (0x00457c50; ECX callback
// A, EDX callback B): for each 0x34-byte record in [[0x00539df4], [0x00539df8]) copied to a local, type 1 -> A, type 3
// -> B (when set) with ECX = the record's data (+4); other types nothing. Hooks log what they receive. Records of types
// 0..4, 0..5 of them, callbacks present or not. Compared: the hook log, the return.
TEST(native_declient_callbacks_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Cmd = std::uint32_t(__fastcall*)(std::uint32_t*, int, const std::uint32_t*);
    using Disp = std::uint32_t(__fastcall*)(void*, void*);
    const Cmd cmd[2] = {rt::original<Cmd>(0x0045c6f0), reinterpret_cast<Cmd>(&recoil::Script_CmdCallback)};
    const Disp disp[2] = {rt::original<Disp>(0x00457c50), reinterpret_cast<Disp>(&recoil::DEClient_DispatchRecords)};
    std::mt19937 rng(0x457c50);
    int compared = 0, calls = 0;
    for (int it = 0; it < 4000; ++it) {
        const int f = it % 2, n = static_cast<int>(rng() % 6);
        std::vector<std::uint32_t> recs(13 * 6);
        for (auto& w : recs) w = rng();
        for (int k = 0; k < n; ++k) recs[13 * k] = rng() % 5;
        const bool has_a = rng() % 4 != 0, has_b = rng() % 4 != 0;
        std::uint32_t interp[0x80 / 4], command[8];
        for (auto& w : interp) w = rng();
        for (auto& w : command) w = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            g_log.clear();
            std::uint32_t it2[0x80 / 4];
            std::memcpy(it2, interp, sizeof it2);
            it2[0x6c / 4] = has_a ? addr(reinterpret_cast<void*>(&cmd_hook)) : 0u;
            std::uint32_t r;
            if (f == 0) {
                r = cmd[side](it2, 0, command);
                for (auto& w : g_log) if (w == addr(it2)) w = 0x17;
            } else {
                *img(side, 0x00539df4) = addr(recs.data());
                *img(side, 0x00539df8) = addr(recs.data() + 13 * n);
                r = disp[side](has_a ? reinterpret_cast<void*>(&record_cb_a) : nullptr, has_b ? reinterpret_cast<void*>(&record_cb_b) : nullptr);
                r = 0;  // EAX is whatever the last callback or load left
            }
            snap[side] = g_log;
            snap[side].push_back(r);
            if (side == 0) calls += static_cast<int>(g_log.size());
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Script_CmdCallback / DEClient_DispatchRecords calls %d, %d words logged\n", compared, calls);
    CHECK(calls > 2000);
}

// DEObjectA_Create (0x004563d0; ECX 11-word template) / DEObjectB_Create (0x00457040; 10-word template): malloc(0x50)
// zeroed, type word (3 / 1), the template copied to +4, malloc(12 * count +8) at +0x30 / +0x2c, Alloc16Zeroed at +0x48 /
// +0x44; flags +4 & 0x1008: A takes +0xc / +0x10 from [0x00539d24] / [0x00539d28] when +0xc is 0; B points +0x4c at the
// table [0x00539ce4] (count [0x00539ce0], 0xc-byte entries) and then at the first entry after the zeroth whose +0 equals
// +0xc. DEObjectA_Free (0x00455ea0) / DEObjectB_Free (0x00456ad0): null -> nothing; free the array, Alloc16_Free the
// 16-byte block (it frees its +0xc first), free the object. Each create is followed by its free; free logged.
// Compared: the object's words (pointers by role), the free log by role.
TEST(native_declient_objects_create_free_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Create = std::uint32_t(__fastcall*)(const std::uint32_t*, int);
    using Free = int(__fastcall*)(std::uint32_t, int);
    const Create create[2][2] = {{rt::original<Create>(0x004563d0), reinterpret_cast<Create>(&recoil::DEObjectA_Create)},
                                 {rt::original<Create>(0x00457040), reinterpret_cast<Create>(&recoil::DEObjectB_Create)}};
    const Free destroy[2][2] = {{rt::original<Free>(0x00455ea0), reinterpret_cast<Free>(&recoil::DEObjectA_Free)},
                                {rt::original<Free>(0x00456ad0), reinterpret_cast<Free>(&recoil::DEObjectB_Free)}};
    FreeHook hook;
    std::mt19937 rng(0x4563d0);
    int compared = 0;
    for (int it = 0; it < 4000; ++it) {
        const int f = it % 2;
        std::uint32_t tmpl[11], table[18], globals[2] = {rng(), rng()};
        for (auto& w : tmpl) w = rng();
        tmpl[0] = rng() % 3 ? (rng() % 2 ? 0x8u : 0x1000u) | (rng() & 0xF0F0u) : rng() & ~0x1008u;
        tmpl[1] = rng() % 6;  // count
        tmpl[2] = rng() % 3 ? 0u : rng() % 5;
        for (auto& w : table) w = rng() % 5;
        const std::uint32_t tcount = rng() % 7;
        const bool null_free = rng() % 20 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *img(side, 0x00539d24) = globals[0];
            *img(side, 0x00539d28) = globals[1];
            *img(side, 0x00539ce0) = tcount;
            *img(side, 0x00539ce4) = addr(table);
            freed().clear();
            const std::uint32_t o = create[f][side](tmpl, 0);
            Roles rl;
            rl.set(table, 0xA0);
            for (int k = 1; k < 6; ++k) rl.set(table + 3 * k, 0xA0u + k);
            rl.set(o, 0xB0);
            for (int k = 0; k < 20; ++k) snap[side].push_back(k == 12 || k == 11 || k == 17 || k == 18 || k == 19 ? rl(at(o)[k]) : at(o)[k]);
            const std::uint32_t block = at(o)[f ? 0x44 / 4 : 0x48 / 4];
            for (int k = 0; k < 4; ++k) snap[side].push_back(at(block)[k]);
            freed().clear();
            destroy[f][side](null_free ? 0u : o, 0);
            for (std::uint32_t p : freed()) snap[side].push_back(rl(p));
            if (null_free) {  // clean up what the call did not
                c_free(at(o)[f ? 0x2c / 4 : 0x30 / 4]);
                c_free(block);
                c_free(o);
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  DEObjectA / DEObjectB create + free calls %d\n", compared);
}

// Anim_PlaceAndScaleAlongSegment_Static (0x00458c10; ECX Object3D node, EDX start xyz, stack end xyz; ret 4) and
// _Animated (0x00458ce0; ECX node, EDX start, stack t0, end, t1; ret 0xc): null node -> the float constant 0x004d2460 in
// ST0. Else Object3D_SetPosition (start / lerp(start, end, t0)), Math_ComputeLookAtPitchYaw -> Object3D_SetRotation
// (pitch, yaw, 0), Object3D_GetScale, Object3D_SetScale(x, y, fast sqrt of the squared length) and the length in ST0.
// Real class-5 nodes with class data (the setters mark the subtree dirty and queue the node on list 7), coordinates from a
// set with repeats and zero-length segments. Compared: ST0, every node / class-data word, list 7 by role.
TEST(native_declient_place_and_scale_along_segment_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using S = float(__fastcall*)(std::uint32_t*, const float*, const float*);
    using A = float(__fastcall*)(std::uint32_t*, const float*, float, const float*, float);
    const S st[2] = {rt::original<S>(0x00458c10), reinterpret_cast<S>(&recoil::Anim_PlaceAndScaleAlongSegment_Static)};
    const A an[2] = {rt::original<A>(0x00458ce0), reinterpret_cast<A>(&recoil::Anim_PlaceAndScaleAlongSegment_Animated)};
    const float vals[] = {0.0f, 1.0f, -1.0f, 2.5f, 10.0f, -7.25f, 0.5f, 100.0f};
    std::mt19937 rng(0x458c10);
    int compared = 0;
    for (int it = 0; it < 6000; ++it) {
        const int f = it % 2;
        float a[3], b[3];
        for (float& v : a) v = vals[rng() % 8];
        for (float& v : b) v = rng() % 6 ? vals[rng() % 8] : a[&v - b];
        const float t0 = vals[rng() % 8] / 10, t1 = vals[rng() % 8] / 10;
        std::uint32_t node_i[49], data_i[0x90 / 4];
        for (auto& w : node_i) w = rng();
        for (auto& w : data_i) w = fbits(vals[rng() % 8]);
        node_i[0x24 / 4] &= ~(1u << 25);
        node_i[0x34 / 4] = 5;
        node_i[0x5c / 4] = 0;
        node_i[0x54 / 4] = 0;
        data_i[0] = rng() & 0xFF;
        const bool null_node = rng() % 20 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t node[49], data[0x90 / 4];
            std::memcpy(node, node_i, sizeof node);
            std::memcpy(data, data_i, sizeof data);
            node[0x38 / 4] = addr(data);
            const float r = f ? an[side](null_node ? nullptr : node, a, t0, b, t1) : st[side](null_node ? nullptr : node, a, b);
            Roles rl;
            rl.set(node, 0xA0);
            rl.set(data, 0xA1);
            snap[side].push_back(fbits(r));
            for (int k = 0; k < 49; ++k) snap[side].push_back(rl(node[k]));
            snap[side].insert(snap[side].end(), data, data + 0x90 / 4);
            snap_list(side, 7, snap[side], rl);
            snap_link_state(side, snap[side], rl);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Anim_PlaceAndScaleAlongSegment static / animated calls %d\n", compared);
}
