// Structured native L1 for World_AddLight (0x00451360) and World_AddSound (0x00451590) (ECX world node, EDX child
// node; the arena fuzz diverges on their realloc use): the world data (+0x38) gets the child appended to one array and
// the child's class data to a parallel one (light: count +0x90, arrays +0x94 / +0x98; sound: +0x9C, +0xA0 / +0xA4),
// both realloc'd; the child's data gets the world appended to its own list (light: count +0xDC, list +0xE0; sound:
// +0x8C / +0x90). Real msvcrt arrays with 0..3 existing entries (or null at count 0). Compared: return, the counts
// and every array entry (pointers by role: world, child, child data, earlier entries).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_world.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t real_malloc(std::size_t n)
{
    return addr(reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"))(n));
}
void real_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(ptr<void>(p)); }
}  // namespace

TEST(native_world_add_light_sound_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*);
    std::mt19937 rng(0x451360);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const bool sound = it % 2;
        const Fn fn[2] = {rt::original<Fn>(sound ? 0x00451590u : 0x00451360u),
                          reinterpret_cast<Fn>(sound ? &recoil::World_AddSound : &recoil::World_AddLight)};
        const int wc = sound ? 0x9C : 0x90, wa = wc + 4, wb = wc + 8;  // world data: count, child array, data array
        const int cc = sound ? 0x8C : 0xDC, ca = cc + 4;                 // child data: count, world list
        const int n_world = static_cast<int>(rng() % 4), n_child = static_cast<int>(rng() % 4);
        std::uint32_t wdata_init[60], cdata_init[60];
        for (auto& w : wdata_init) w = rng() | 0x80000000u;
        for (auto& w : cdata_init) w = rng() | 0x80000000u;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t wdata[60], cdata[60], world[16] = {}, child[16] = {};
            std::memcpy(wdata, wdata_init, sizeof wdata);
            std::memcpy(cdata, cdata_init, sizeof cdata);
            world[0x38 / 4] = addr(wdata);
            child[0x38 / 4] = addr(cdata);
            std::uint32_t others[8];  // role tokens for the earlier entries
            for (int k = 0; k < 8; ++k) others[k] = 0xE0000000u + static_cast<std::uint32_t>(k);
            wdata[wc / 4] = static_cast<std::uint32_t>(n_world);
            wdata[wa / 4] = n_world ? real_malloc(4 * n_world) : 0u;
            wdata[wb / 4] = n_world ? real_malloc(4 * n_world) : 0u;
            for (int k = 0; k < n_world; ++k) { ptr<std::uint32_t>(wdata[wa / 4])[k] = others[k]; ptr<std::uint32_t>(wdata[wb / 4])[k] = others[4 + k]; }
            cdata[cc / 4] = static_cast<std::uint32_t>(n_child);
            cdata[ca / 4] = n_child ? real_malloc(4 * n_child) : 0u;
            for (int k = 0; k < n_child; ++k) ptr<std::uint32_t>(cdata[ca / 4])[k] = others[k];
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](world, child)));
            auto role = [&](std::uint32_t v) { return v == addr(world) ? 0xA0u : v == addr(child) ? 0xA1u : v == addr(cdata) ? 0xA2u : v; };
            const std::uint32_t nw = wdata[wc / 4], nc = cdata[cc / 4];
            snap[side].push_back(nw); snap[side].push_back(nc);
            for (std::uint32_t k = 0; k < nw; ++k) { snap[side].push_back(role(ptr<std::uint32_t>(wdata[wa / 4])[k])); snap[side].push_back(role(ptr<std::uint32_t>(wdata[wb / 4])[k])); }
            for (std::uint32_t k = 0; k < nc; ++k) snap[side].push_back(role(ptr<std::uint32_t>(cdata[ca / 4])[k]));
            for (int k = 0; k < 60; ++k) if (k != wc / 4 && k != wa / 4 && k != wb / 4) snap[side].push_back(wdata[k]);
            for (int k = 0; k < 60; ++k) if (k != cc / 4 && k != ca / 4) snap[side].push_back(cdata[k]);
            real_free(wdata[wa / 4]); real_free(wdata[wb / 4]); real_free(cdata[ca / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  World_AddLight / World_AddSound calls %d (arrays of 0..3 existing entries, grown by realloc)\n", compared);
}
