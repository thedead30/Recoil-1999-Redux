// Native L1 for Material_FindOrCreate (0x00480ca0) and Material_Shutdown (0x00480f10) on a valid material state -
// random material globals (tests/arena_fuzz.h) make every Shutdown call fault and FindOrCreate end normally rarely.
// State (ledger rows of Material_Free 0x00480dc0 / Material_AllocCopy 0x004812c0 / Material_Compare 0x00480d20): array
// [0x00566a1c] of size [0x00566a18] 0x2C-byte records; used list head [0x004e1164], free list head [0x004e1160], both
// doubly linked through short prev +0x28 / next +0x2A (-1 ends); used count [0x00566a20]; cache [0x00566a24]. A
// cycling material (byte +1 bit 2) owns a 0x1C cycle block at +0x24 (+0x10 frame count, +0x18 frame list). Every
// block is a msvcrt heap block per side (the code frees them). FindOrCreate gets templates equal to the cached
// material, to a used one, or to none (allocation, or the default material 0x005669f0 when the array is full),
// cycling or not; Shutdown tears the state down. Compared: return value (array index / default / null / raw), the
// globals, every record (36 data bytes, links, cycle and frames by content) and, for Shutdown, the frees by role.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_matl.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
using Malloc = void*(__cdecl*)(std::size_t);
std::uint32_t* global(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }

std::vector<std::uint32_t> g_frees;  // Shutdown: freed blocks (raw; mapped to roles after the call)
void __cdecl logging_free(void* p)
{
    g_frees.push_back(addr(p));
    reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(p);
}

struct Spec {  // one random state, built identically on each side
    int size;
    std::vector<std::uint32_t> data;       // 9 words per record (36 bytes)
    std::vector<int> used, freel;          // list orders
    std::vector<std::vector<std::uint32_t>> cycle;  // per record: 7 words + frames (empty: not cycling)
    int cached;                            // index or -1
};

// The cycle block (0x1C bytes) and frame list of a record, as msvcrt blocks.
std::uint32_t make_cycle(const std::vector<std::uint32_t>& c)
{
    auto* blk = static_cast<std::uint32_t*>(reinterpret_cast<Malloc>(recoil::g_Iat_malloc_004cc5dc)(0x1C));
    std::memcpy(blk, c.data(), 0x1C);
    const std::uint32_t n = c[4];
    auto* frames = static_cast<std::uint32_t*>(reinterpret_cast<Malloc>(recoil::g_Iat_malloc_004cc5dc)(4 * (n ? n : 1)));
    for (std::uint32_t k = 0; k < n; ++k) frames[k] = c[7 + k];
    blk[6] = addr(frames);
    return addr(blk);
}

void build(int side, const Spec& s)
{
    auto* arr = static_cast<std::uint32_t*>(reinterpret_cast<Malloc>(recoil::g_Iat_malloc_004cc5dc)(0x2C * s.size));
    std::memset(arr, 0, 0x2C * s.size);
    for (int i = 0; i < s.size; ++i) {
        std::uint32_t* r = arr + 11 * i;
        std::memcpy(r, &s.data[9 * i], 36);
        r[9] = s.cycle[i].empty() ? 0 : make_cycle(s.cycle[i]);
    }
    auto link = [&](const std::vector<int>& order) {
        for (std::size_t k = 0; k < order.size(); ++k) {
            auto* links = reinterpret_cast<std::int16_t*>(arr + 11 * order[k] + 10);
            links[0] = static_cast<std::int16_t>(k ? order[k - 1] : -1);
            links[1] = static_cast<std::int16_t>(k + 1 < order.size() ? order[k + 1] : -1);
        }
    };
    link(s.used);
    link(s.freel);
    *global(side, 0x00566a18) = static_cast<std::uint32_t>(s.size);
    *global(side, 0x00566a1c) = addr(arr);
    *global(side, 0x00566a20) = static_cast<std::uint32_t>(s.used.size());
    *global(side, 0x004e1164) = s.used.empty() ? 0xFFFFFFFFu : static_cast<std::uint32_t>(s.used[0]);
    *global(side, 0x004e1160) = s.freel.empty() ? 0xFFFFFFFFu : static_cast<std::uint32_t>(s.freel[0]);
    *global(side, 0x00566a24) = s.cached < 0 ? 0 : addr(arr + 11 * s.cached);
}

// Comparable form of a pointer into this side's material world.
std::uint32_t role(int side, std::uint32_t v)
{
    const std::uint32_t arr = *global(side, 0x00566a1c), size = *global(side, 0x00566a18);
    if (arr && v >= arr && v < arr + 0x2C * size) return 0xA0000000u + (v - arr);
    const std::uint32_t def = side ? addr(recoil::ImageData_Address(0x005669f0)) : 0x005669f0u;
    if (v == def) return 0xDEF00000u;
    return v;
}

std::vector<std::uint32_t> snapshot(int side)
{
    std::vector<std::uint32_t> s = {*global(side, 0x00566a18), *global(side, 0x00566a1c) ? 1u : 0u, *global(side, 0x00566a20),
                                    *global(side, 0x004e1164), *global(side, 0x004e1160), role(side, *global(side, 0x00566a24))};
    const std::uint32_t arr = *global(side, 0x00566a1c);
    for (std::uint32_t i = 0; arr && i < *global(side, 0x00566a18); ++i) {
        const auto* r = ptr<std::uint32_t>(arr + 0x2C * i);
        s.insert(s.end(), r, r + 9);
        s.push_back(r[10]);  // links
        if (r[9]) {
            const auto* c = ptr<std::uint32_t>(r[9]);
            s.insert(s.end(), c, c + 6);
            for (std::uint32_t k = 0; k < c[4] && k < 64; ++k) s.push_back(ptr<std::uint32_t>(c[6])[k]);
        } else {
            s.push_back(0xC0C0C0C0u);
        }
    }
    return s;
}

// Release whatever the state still owns (after a FindOrCreate call).
void release(int side)
{
    const auto m_free = reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4);
    const std::uint32_t arr = *global(side, 0x00566a1c);
    for (std::uint32_t i = 0; arr && i < *global(side, 0x00566a18); ++i) {
        const auto* r = ptr<std::uint32_t>(arr + 0x2C * i);
        if (r[9]) {
            m_free(ptr<void>(ptr<std::uint32_t>(r[9])[6]));
            m_free(ptr<void>(r[9]));
        }
    }
    if (arr) m_free(ptr<void>(arr));
}

Spec random_spec(std::mt19937& rng)
{
    Spec s;
    s.size = 1 + static_cast<int>(rng() % 8);
    s.data.resize(9 * s.size);
    for (auto& w : s.data) w = rng() % 4 == 0 ? rng() : rng() % 3;  // small values: materials often compare equal
    std::vector<int> order(s.size);
    for (int i = 0; i < s.size; ++i) order[i] = i;
    std::shuffle(order.begin(), order.end(), rng);
    const int used = static_cast<int>(rng() % (s.size + 1));
    s.used.assign(order.begin(), order.begin() + used);
    s.freel.assign(order.begin() + used, order.end());
    s.cycle.resize(s.size);
    for (int i = 0; i < s.size; ++i) {
        auto* flags = reinterpret_cast<unsigned char*>(&s.data[9 * i]) + 1;
        const bool is_used = std::find(s.used.begin(), s.used.end(), i) != s.used.end();
        if (is_used && rng() % 3 == 0) {
            *flags |= 4;
            const std::uint32_t n = rng() % 4;
            s.cycle[i] = {rng(), rng(), rng(), rng(), n, rng(), 0};
            for (std::uint32_t k = 0; k < n; ++k) s.cycle[i].push_back(rng());
        } else {
            *flags &= ~4;
        }
    }
    s.cached = !s.used.empty() && rng() % 2 ? s.used[rng() % s.used.size()] : -1;
    return s;
}
}  // namespace

TEST(native_material_find_or_create_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00480ca0), reinterpret_cast<Fn>(&recoil::Material_FindOrCreate)};
    std::mt19937 rng(0x480ca0);
    int compared = 0;
    for (int it = 0; it < 1500; ++it) {
        const Spec s = random_spec(rng);
        // template: a copy of a used (possibly cached) material's 36 bytes, or random; cycling templates too
        std::uint32_t tdata[11] = {};
        const int src = !s.used.empty() && rng() % 3 ? s.used[rng() % s.used.size()] : -1;
        for (int k = 0; k < 9; ++k) tdata[k] = src >= 0 ? s.data[9 * src + k] : (rng() % 4 == 0 ? rng() : rng() % 3);
        if (rng() % 4 == 0) tdata[8] = rng() % 2 ? 0 : rng();  // +0x20 differs: the compare's copy-or-fail branch
        const bool tcycle = rng() % 4 == 0;
        std::vector<std::uint32_t> tc;
        if (tcycle) {
            reinterpret_cast<unsigned char*>(tdata)[1] |= 4;
            const std::uint32_t n = rng() % 4;
            tc = {rng(), rng(), rng(), rng(), n, rng(), 0};
            for (std::uint32_t k = 0; k < n; ++k) tc.push_back(rng());
        } else {
            reinterpret_cast<unsigned char*>(tdata)[1] &= ~4;
        }
        std::uint32_t ret[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            build(side, s);
            std::uint32_t t[11];
            std::memcpy(t, tdata, sizeof t);
            t[9] = tcycle ? make_cycle(tc) : 0;
            ret[side] = role(side, fn[side](t, 0));
            snap[side] = snapshot(side);
            release(side);
            if (t[9]) {
                const auto m_free = reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4);
                m_free(ptr<void>(ptr<std::uint32_t>(t[9])[6]));
                m_free(ptr<void>(t[9]));
            }
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Material_FindOrCreate calls %d (valid material states: cache hit, list hit, allocation, array full)\n", compared);
}

TEST(native_material_shutdown_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(int, int);
    const Fn fn[2] = {rt::original<Fn>(0x00480f10), reinterpret_cast<Fn>(&recoil::Material_Shutdown)};
    auto* o_free = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* const saved[2] = {*o_free, recoil::g_Iat_free_004cc5b4};
    std::mt19937 rng(0x480f10);
    int compared = 0;
    for (int it = 0; it < 1000; ++it) {
        const Spec s = random_spec(rng);
        std::uint32_t ret[2];
        std::vector<std::uint32_t> snap[2], frees[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            build(side, s);
            // roles of this side's blocks, before the call frees them
            std::vector<std::pair<std::uint32_t, std::uint32_t>> roles = {{*global(side, 0x00566a1c), 0xA77A0000u}};
            for (int i = 0; i < s.size; ++i) {
                const std::uint32_t c = ptr<std::uint32_t>(*global(side, 0x00566a1c))[11 * i + 9];
                if (c) {
                    roles.push_back({c, 0xC0000000u + i});
                    roles.push_back({ptr<std::uint32_t>(c)[6], 0xF0000000u + i});
                }
            }
            g_frees.clear();
            *o_free = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
            ret[side] = fn[side](0, 0);
            *o_free = saved[0];
            recoil::g_Iat_free_004cc5b4 = saved[1];
            for (auto& f : g_frees)
                for (const auto& [p, r] : roles)
                    if (f == p) { f = r; break; }
            frees[side] = g_frees;
            snap[side] = snapshot(side);  // the array is gone: globals only
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        CHECK(frees[0] == frees[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Material_Shutdown calls %d (valid material states torn down; frees by role)\n", compared);
}
