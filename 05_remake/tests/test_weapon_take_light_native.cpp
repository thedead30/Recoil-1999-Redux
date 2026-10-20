// Structured native L1 for Weapon_TakeLightFromPool (0x004b2520): ECX = an RGB float triple; pops the head of the light pool ([0x00778938], next link at
// node+0x40) - or returns 0 when the pool is empty - then Light_SetFalloffRange(0.1, 0.2) and Light_SetColor(r, g, b) on the popped light and World_AddLight
// ([0x00778920] world node, the light). The arena fuzz cannot drive it (84% of random pool heads fault on both sides and the first differing heap call is
// noise, for_opus.md 2026-09-29); this builds a real pool of 0..3 lights and a real world (arrays of 0..2 earlier entries, grown by realloc like
// test_world_add_native.cpp) on each side and compares: the return, the pool head afterwards, every word of every light node and of its class data, and the
// world's arrays and counts (pointers by role: world, light, class data, earlier entries).
#include "test.h"
#include "native_oracle.h"
#include "platform/image/original_data.h"
#include "GameZRecoil/zWeapon/zwep_init.h"

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
// the original's global at its own address, the port's global in its data image
std::uint32_t& global(bool port_side, std::uint32_t va)
{
    return port_side ? *static_cast<std::uint32_t*>(recoil::ImageData_Address(va)) : *ptr<std::uint32_t>(va);
}
constexpr std::uint32_t kPoolHead = 0x00778938, kWorld = 0x00778920;
}  // namespace

TEST(native_weapon_take_light_from_pool_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*);
    const Fn fn[2] = {rt::original<Fn>(0x004b2520u), reinterpret_cast<Fn>(&recoil::Weapon_TakeLightFromPool)};
    std::mt19937 rng(0x4b2520);
    static const float kColor[] = {0.0f, 1.0f, 0.5f, 0.25f, 0.75f, -1.0f, 2.0f, 0.1f, 255.0f};
    const int wc = 0x90, wa = wc + 4, wb = wc + 8;  // world data: light count, light array, class-data array (as in test_world_add_native.cpp)
    const int cc = 0xDC, ca = cc + 4;                // light class data: world count, world list
    int compared = 0, empty_pool = 0, returned = 0;
    for (int it = 0; it < 2000; ++it) {
        rt::restore_pristine();
        const int n_pool = static_cast<int>(rng() % 4), n_world = static_cast<int>(rng() % 3), n_child = static_cast<int>(rng() % 3);
        std::uint32_t node_init[4][32], cdata_init[4][60], wdata_init[60];
        for (auto& l : node_init) for (auto& w : l) w = rng() % 4 ? 0u : rng() % 64;
        for (auto& l : cdata_init) for (auto& w : l) w = rng() % 4 ? 0u : rng() % 64;
        for (auto& w : wdata_init) w = rng() % 4 ? 0u : rng() % 64;
        std::uint32_t color[3];
        for (auto& c : color) { const float f = kColor[rng() % 9]; std::memcpy(&c, &f, 4); }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t node[4][32], cdata[4][60], wdata[60], world[16] = {}, col[3];
            std::memcpy(node, node_init, sizeof node);
            std::memcpy(cdata, cdata_init, sizeof cdata);
            std::memcpy(wdata, wdata_init, sizeof wdata);
            std::memcpy(col, color, sizeof col);
            std::uint32_t others[8];
            for (int k = 0; k < 8; ++k) others[k] = 0xE0000000u + static_cast<std::uint32_t>(k);
            for (int i = 0; i < 4; ++i) {
                node[i][0x38 / 4] = addr(cdata[i]);
                node[i][0x40 / 4] = (i + 1 < n_pool) ? addr(node[i + 1]) : 0u;  // pool link
                cdata[i][cc / 4] = static_cast<std::uint32_t>(n_child);
                cdata[i][ca / 4] = n_child ? real_malloc(4 * n_child) : 0u;
                for (int k = 0; k < n_child; ++k) ptr<std::uint32_t>(cdata[i][ca / 4])[k] = others[k];
            }
            world[0x38 / 4] = addr(wdata);
            wdata[wc / 4] = static_cast<std::uint32_t>(n_world);
            wdata[wa / 4] = n_world ? real_malloc(4 * n_world) : 0u;
            wdata[wb / 4] = n_world ? real_malloc(4 * n_world) : 0u;
            for (int k = 0; k < n_world; ++k) { ptr<std::uint32_t>(wdata[wa / 4])[k] = others[k]; ptr<std::uint32_t>(wdata[wb / 4])[k] = others[4 + k]; }
            global(side != 0, kPoolHead) = n_pool ? addr(node[0]) : 0u;
            global(side != 0, kWorld) = addr(world);
            const std::uint32_t ret = static_cast<std::uint32_t>(fn[side](col, nullptr));
            auto role = [&](std::uint32_t v) -> std::uint32_t {
                if (v == addr(world)) return 0xA0u;
                if (v == addr(wdata)) return 0xA1u;
                for (int i = 0; i < 4; ++i) {
                    if (v == addr(node[i])) return 0xB0u + i;
                    if (v == addr(cdata[i])) return 0xC0u + i;
                }
                return v;
            };
            snap[side].push_back(role(ret));
            snap[side].push_back(role(global(side != 0, kPoolHead)));
            snap[side].push_back(role(global(side != 0, kWorld)));
            for (int i = 0; i < 4; ++i) {
                for (int k = 0; k < 32; ++k) snap[side].push_back(role(node[i][k]));
                for (int k = 0; k < 60; ++k) if (k != ca / 4) snap[side].push_back(role(cdata[i][k]));
                for (std::uint32_t k = 0; k < cdata[i][cc / 4] && cdata[i][ca / 4]; ++k) snap[side].push_back(role(ptr<std::uint32_t>(cdata[i][ca / 4])[k]));
            }
            const std::uint32_t nw = wdata[wc / 4];
            snap[side].push_back(nw);
            for (std::uint32_t k = 0; k < nw && wdata[wa / 4]; ++k) { snap[side].push_back(role(ptr<std::uint32_t>(wdata[wa / 4])[k])); snap[side].push_back(role(ptr<std::uint32_t>(wdata[wb / 4])[k])); }
            for (int k = 0; k < 60; ++k) if (k != wc / 4 && k != wa / 4 && k != wb / 4) snap[side].push_back(wdata[k]);
            if (side == 0) { if (!n_pool) ++empty_pool; else ++returned; }
            for (int i = 0; i < 4; ++i) real_free(cdata[i][ca / 4]);
            real_free(wdata[wa / 4]);
            real_free(wdata[wb / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  Weapon_TakeLightFromPool calls %d (pool of 0..3 lights; %d empty pools, %d that popped a light)\n", compared, empty_pool, returned);
}
