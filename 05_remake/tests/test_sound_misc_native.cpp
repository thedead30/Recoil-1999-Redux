// Structured native L1 for sound functions ported in the cloud as zclass_nodes blockers (P2.6).
//   Sound_ReportDirectSoundError (0x004a4330; ECX HRESULT, EDX source path, one stack argument: line; ret 4): S_OK -> 1
//     with no report; else the HRESULT's name (or a generic text) is sprintf'd into a 256-byte local and handed to the
//     no-op reporter, 0. Only the return is observable (the reporter does nothing): every mapped HRESULT, their
//     neighbours and random values.
//   Sound_StreamCue_PickWeightedRandomVariant (0x004a4d10; ECX resource): count +0x20 == 1 -> the variant (+0x24) when
//     its short +2 is set, else 0. Else sums the eligible (+2 set) weights (+8), draws msvcrt rand(), walks the array
//     for the variant whose running sum passes rand / RAND_MAX of the total; with +8 set the chosen variant's weight is
//     scaled by +0x10, the others below the floor [0x004d2f44] are lifted to 0.001, and all eligible weights are
//     renormalised. rand is seeded identically (srand) before each side. Resources of 1..6 variants, weights from a
//     set with zero, eligibility random. Compared: the return as a variant index and every resource / variant word.
//   Sound_StreamCue_State0_SelectVariant (0x004a4cb0) / Sound_StreamCue_State2_ReplayGapWait (0x004a4fd0) (ECX cue:
//     +0x28 timer, +0x2c replay count, +0x30 current variant, +0x34 state, +0x38 resource): state 0 clears the timer,
//     count and variant, then with no variants goes to state 4, else draws a variant (PickWeightedRandomVariant: state
//     1, or 4 when none is eligible); state 2 adds the frame time [0x0056b424] to the timer and, once it is not below
//     the resource gap +0x18, clears it and draws again. Same resources as above plus empty ones, rand seeded per side.
//     Compared: the returns and every cue / resource / variant word (the variant pointer as an index).
//   Sound_DuplicateHardwareBuffer (0x0049f830; ECX voice): see the ledger. Its DirectSound calls go through fake COM
//     objects: buffers whose GetStatus (vtable +0x24) reports a per-buffer status and logs, a device [0x0056b2b0] whose
//     DuplicateSoundBuffer (vtable +0x14) fails (logged) or hands out a fresh fake buffer. Voices idle or busy, source
//     buffer present or not, inline slot used or not, 0..6 duplicates (real msvcrt array) idle / busy, device failing
//     or not. Compared by role: the return, the call log, the voice's words, the duplicate array and new records.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zSound/zsnd_create.h"
#include "GameZRecoil/zSound/zsnd_grp.h"
#include "GameZRecoil/zSound/zsnd_parm.h"
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
std::uint32_t fbits(float f) { std::uint32_t u; std::memcpy(&u, &f, 4); return u; }
HMODULE crt() { return GetModuleHandleA("msvcrt.dll"); }
}  // namespace

TEST(native_sound_report_direct_sound_error_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t, const char*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004a4330), reinterpret_cast<Fn>(&recoil::Sound_ReportDirectSoundError)};
    const std::uint32_t mapped[] = {0, 0x80004001, 0x80004005, 0x80040110, 0x8007000e, 0x80070057, 0x8878000a, 0x8878001e,
                                    0x88780032, 0x88780046, 0x88780064, 0x88780078, 0x88780082, 0x88780096, 0x887800a0};
    std::mt19937 rng(0x4a4330);
    int compared = 0, ok = 0;
    for (int it = 0; it < 3000; ++it) {
        const std::uint32_t base = mapped[rng() % 15];
        const std::uint32_t hr = it % 3 == 0 ? base : it % 3 == 1 ? base + static_cast<std::uint32_t>(static_cast<int>(rng() % 3) - 1) : rng();
        int r[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            r[side] = fn[side](hr, "D:/Proj/GameZRecoil/zSound/zsnd_create.cpp", 0x123);
        }
        CHECK_EQ(r[0], r[1]);
        ok += r[0] == 1;
        ++compared;
    }
    std::printf("  Sound_ReportDirectSoundError calls %d, %d returned 1 (S_OK)\n", compared, ok);
}

TEST(native_sound_stream_cue_pick_weighted_random_variant_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004a4d10), reinterpret_cast<Fn>(&recoil::Sound_StreamCue_PickWeightedRandomVariant)};
    auto c_srand = reinterpret_cast<void(__cdecl*)(unsigned)>(GetProcAddress(crt(), "srand"));
    const float weights[] = {0.0f, 0.0005f, 0.1f, 0.25f, 0.5f, 1.0f, 2.0f, 10.0f};
    std::mt19937 rng(0x4a4d10);
    int compared = 0, picked = 0;
    for (int it = 0; it < 20000; ++it) {
        const int n = 1 + static_cast<int>(rng() % 6);
        std::vector<std::uint32_t> var_init(6 * n);
        for (auto& w : var_init) w = rng();
        for (int k = 0; k < n; ++k) {
            const std::uint16_t eligible = rng() % 4 ? static_cast<std::uint16_t>(1 + rng() % 3) : 0;
            std::memcpy(reinterpret_cast<char*>(&var_init[6 * k]) + 2, &eligible, 2);
            var_init[6 * k + 2] = fbits(weights[rng() % 8]);
        }
        std::uint32_t res_init[12];
        for (auto& w : res_init) w = rng();
        res_init[0x8 / 4] = rng() % 2;
        res_init[0x10 / 4] = fbits(rng() % 2 ? 0.5f : 0.25f);
        res_init[0x20 / 4] = static_cast<std::uint32_t>(n);
        const unsigned seed = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<std::uint32_t> var = var_init;
            std::uint32_t res[12];
            std::memcpy(res, res_init, sizeof res);
            res[0x24 / 4] = addr(var.data());
            c_srand(seed);
            const std::uint32_t r = fn[side](res, 0);
            snap[side].push_back(r ? (r - addr(var.data())) / 0x18 : 0xFFFFFFFFu);
            snap[side].push_back(r ? (r - addr(var.data())) % 0x18 : 0u);
            for (int k = 0; k < 12; ++k) if (k != 0x24 / 4) snap[side].push_back(res[k]);
            snap[side].insert(snap[side].end(), var.begin(), var.end());
            if (side == 0) picked += r != 0;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  Sound_StreamCue_PickWeightedRandomVariant calls %d, %d picked a variant\n", compared, picked);
    CHECK(picked > 5000);
}

TEST(native_sound_stream_cue_select_and_gap_states_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    const Fn st0[2] = {rt::original<Fn>(0x004a4cb0), reinterpret_cast<Fn>(&recoil::Sound_StreamCue_State0_SelectVariant)};
    const Fn st2[2] = {rt::original<Fn>(0x004a4fd0), reinterpret_cast<Fn>(&recoil::Sound_StreamCue_State2_ReplayGapWait)};
    auto c_srand = reinterpret_cast<void(__cdecl*)(unsigned)>(GetProcAddress(crt(), "srand"));
    const float vals[] = {0.0f, 0.0005f, 0.1f, 0.25f, 0.5f, 1.0f, 2.0f, 10.0f};
    std::mt19937 rng(0x4a4cb0);
    int compared = 0, drew = 0;
    for (int it = 0; it < 20000; ++it) {
        const int f = it % 2;
        const int n = static_cast<int>(rng() % 5);
        std::vector<std::uint32_t> var_init(6 * (n ? n : 1));
        for (auto& w : var_init) w = rng();
        for (int k = 0; k < n; ++k) {
            const std::uint16_t eligible = rng() % 4 ? 1 : 0;
            std::memcpy(reinterpret_cast<char*>(&var_init[6 * k]) + 2, &eligible, 2);
            var_init[6 * k + 2] = fbits(vals[rng() % 8]);
        }
        std::uint32_t res_init[12], cue_init[16];
        for (auto& w : res_init) w = rng();
        for (auto& w : cue_init) w = rng();
        res_init[0x8 / 4] = rng() % 2;
        res_init[0x10 / 4] = fbits(0.5f);
        res_init[0x18 / 4] = fbits(vals[rng() % 8]);
        res_init[0x20 / 4] = static_cast<std::uint32_t>(n);
        cue_init[0x28 / 4] = fbits(vals[rng() % 8]);
        const float dt = vals[rng() % 8];
        const unsigned seed = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *reinterpret_cast<float*>(img(side, 0x0056b424)) = dt;
            std::vector<std::uint32_t> var = var_init;
            std::uint32_t res[12], cue[16];
            std::memcpy(res, res_init, sizeof res);
            std::memcpy(cue, cue_init, sizeof cue);
            res[0x24 / 4] = addr(var.data());
            cue[0x38 / 4] = addr(res);
            c_srand(seed);
            snap[side].push_back((f ? st2 : st0)[side](cue, 0));
            for (int k = 0; k < 16; ++k) {
                const std::uint32_t w = cue[k];
                snap[side].push_back(k == 0x38 / 4 ? 0u : k == 0x30 / 4 && w >= addr(var.data()) && w < addr(var.data()) + 4 * var.size()
                                                             ? 0xA0000000u + (w - addr(var.data())) : w);
            }
            for (int k = 0; k < 12; ++k) if (k != 0x24 / 4) snap[side].push_back(res[k]);
            snap[side].insert(snap[side].end(), var.begin(), var.end());
            if (side == 0) drew += cue[0x34 / 4] == 1;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  stream cue states 0 / 2 calls %d, %d drew a variant\n", compared, drew);
    CHECK(drew > 3000);
}

namespace {
// fake DirectSound objects: {vtable, status or fail flag, id}
std::vector<std::uint32_t> g_log;
std::uint32_t g_vtbl[16];
std::vector<std::vector<std::uint32_t>> g_new_buffers;  // handed out by the fake device (per side, cleared per call)
HRESULT __stdcall fake_get_status(std::uint32_t* self, DWORD* out)
{
    g_log.push_back(0x57A7u); g_log.push_back(self[2]);
    *out = self[1];
    return 0;
}
HRESULT __stdcall fake_duplicate(std::uint32_t* self, std::uint32_t* src, std::uint32_t** out)
{
    g_log.push_back(0xD0Bu); g_log.push_back(src ? src[2] : 0xFFFFu);
    if (self[1]) return static_cast<HRESULT>(0x88780046u);
    g_new_buffers.push_back({addr(g_vtbl), 0, 0x9000u + static_cast<std::uint32_t>(g_new_buffers.size())});
    *out = g_new_buffers.back().data();
    return 0;
}
}  // namespace

TEST(native_sound_duplicate_hardware_buffer_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0049f830), reinterpret_cast<Fn>(&recoil::Sound_DuplicateHardwareBuffer)};
    auto c_malloc = reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(crt(), "malloc"));
    auto c_free = reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(crt(), "free"));
    for (auto& w : g_vtbl) w = 0;
    g_vtbl[0x14 / 4] = addr(reinterpret_cast<void*>(&fake_duplicate));
    g_vtbl[0x24 / 4] = addr(reinterpret_cast<void*>(&fake_get_status));
    g_new_buffers.reserve(16);  // no reallocation while pointers are handed out
    std::mt19937 rng(0x49f830);
    int compared = 0, per[4] = {};  // 0 none, 1 inline slot, 2 existing duplicate, 3 new duplicate
    for (int it = 0; it < 6000; ++it) {
        const int ndup = static_cast<int>(rng() % 7);
        std::uint32_t voice_init[0x88 / 4];
        for (auto& w : voice_init) w = rng();
        voice_init[0] = rng() % 6 == 0 ? 1u : 0u;
        const bool has_src = rng() % 8 != 0, inline_used = rng() % 2;
        const std::uint32_t src_status = rng() % 2, dev_fail = rng() % 4 == 0;
        std::vector<std::uint32_t> dup_state(ndup), dup_status(ndup);
        std::vector<bool> dup_null(ndup);
        for (int k = 0; k < ndup; ++k) { dup_state[k] = rng() % 3 == 0; dup_status[k] = rng() % 3 != 0; dup_null[k] = rng() % 10 == 0; }
        const bool null_voice = rng() % 40 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            g_log.clear();
            g_new_buffers.clear();
            std::uint32_t src[3] = {addr(g_vtbl), src_status, 0x5000u};
            std::uint32_t device[3] = {addr(g_vtbl), dev_fail, 0x6000u};
            std::uint32_t voice[0x88 / 4];
            std::memcpy(voice, voice_init, sizeof voice);
            voice[0x4c / 4] = has_src ? addr(src) : 0u;
            voice[0x44 / 4] = inline_used ? 1u : 0u;
            std::vector<std::vector<std::uint32_t>> dups(ndup), dup_buf(ndup);
            const std::uint32_t arr = ndup ? addr(c_malloc(4 * ndup)) : 0u;
            for (int k = 0; k < ndup; ++k) {
                dup_buf[k] = {addr(g_vtbl), dup_status[k], 0x7000u + static_cast<std::uint32_t>(k)};
                dups[k].assign(15, 0x3C3C3C3Cu);
                dups[k][0] = dup_state[k];
                dups[k][2] = addr(dup_buf[k].data());
                at(arr)[k] = dup_null[k] ? 0u : addr(dups[k].data());
            }
            voice[0x80 / 4] = static_cast<std::uint32_t>(ndup);
            voice[0x84 / 4] = arr;
            *img(side, 0x0056b2b0) = addr(device);
            const std::uint32_t r = fn[side](null_voice ? nullptr : voice, 0);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            role[addr(voice + 0x44 / 4)] = 0xA0;
            for (int k = 0; k < ndup; ++k) role[addr(dups[k].data())] = 0xB0u + k;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            snap[side].push_back(rl(r));
            snap[side].insert(snap[side].end(), g_log.begin(), g_log.end());
            snap[side].push_back(0xF0F0F0F0u);
            for (int k = 0; k < 0x88 / 4; ++k) snap[side].push_back(k == 0x84 / 4 ? (voice[k] == arr ? 0xA1u : voice[k] ? 0xA2u : 0u) : k == 0x4c / 4 ? (voice[k] ? 1u : 0u) : voice[k]);
            const std::uint32_t now = voice[0x84 / 4];
            for (std::uint32_t k = 0; now && k < voice[0x80 / 4] && k < 8; ++k) snap[side].push_back(rl(at(now)[k]));
            const bool fresh_record = r && role.find(r)->second >= 0xC000;
            if (fresh_record) {  // a new record: 15 words, +8 the new buffer (by its id)
                for (int k = 0; k < 15; ++k) snap[side].push_back(k == 2 ? (at(r)[2] ? at(at(r)[2])[2] : 0u) : at(r)[k]);
                c_free(at(r));
            }
            if (side == 0) per[!r ? 0 : r == addr(voice + 0x44 / 4) ? 1 : fresh_record ? 3 : 2]++;
            if (now) c_free(at(now));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Sound_DuplicateHardwareBuffer calls %d: none %d, inline slot %d, existing duplicate %d, new duplicate %d\n", compared,
                per[0], per[1], per[2], per[3]);
    CHECK(per[1] > 300 && per[2] > 300 && per[3] > 300);
}
