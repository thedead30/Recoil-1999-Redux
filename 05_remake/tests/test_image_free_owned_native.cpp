// Native L1 for Image_FreeOwnedBuffers (0x0046ecf0): image at ECX; frees +0x10 when non-null and byte +9 bit 0x20
// (clears both), +0x14 when non-null and bit 0x40, +0x18 when non-null, bit 0x80 and not bit 0x10. The arena fuzz
// almost never sees a null pointer and faults on every free of an arena word, so here: real msvcrt blocks or null in
// each slot and every flag byte. Compared: the image words, the three pointers as null / kept (a kept block is freed by
// the test afterwards).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zImage/zimg_fonts.h"
#include "platform/iat_msvcrt.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

namespace {
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t m_malloc(std::size_t n)
{
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(reinterpret_cast<void*(__cdecl*)(std::size_t)>(recoil::g_Iat_malloc_004cc5dc)(n)));
}
void m_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4)(ptr<void>(p)); }
}  // namespace

TEST(native_image_free_owned_buffers_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046ecf0), reinterpret_cast<Fn>(&recoil::Image_FreeOwnedBuffers)};
    std::mt19937 rng(0x46ecf0);
    int compared = 0;
    for (int it = 0; it < 4096; ++it) {
        const std::uint32_t flags = it < 256 ? static_cast<std::uint32_t>(it) : rng() & 0xFF;
        const unsigned nulls = rng() % 8;
        std::uint32_t init[8];
        for (auto& w : init) w = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t img[8];
            for (int k = 0; k < 8; ++k) img[k] = init[k];
            reinterpret_cast<unsigned char*>(img)[9] = static_cast<unsigned char>(flags);
            std::uint32_t blk[3];
            for (int s = 0; s < 3; ++s) img[4 + s] = blk[s] = (nulls >> s) & 1 ? 0u : m_malloc(16 + s);
            fn[side](img, 0);
            for (int k = 0; k < 8; ++k) {
                if (k >= 4 && k <= 6) snap[side].push_back(img[k] == 0 ? 0u : img[k] == blk[k - 4] ? 1u : 2u);
                else snap[side].push_back(img[k]);
            }
            for (int s = 0; s < 3; ++s) if (img[4 + s]) m_free(img[4 + s]);  // kept blocks; freed ones were nulled
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  Image_FreeOwnedBuffers calls %d (real blocks or null per slot, every flag byte)\n", compared);
}

// Image_Free (0x0046ecc0, null-safe): releases the hardware surface [+0x30] through the zVideo hook [0x0056bc28] when
// set (ECX = image), Image_FreeOwnedBuffers, then frees the image; Image_FreeUnlessDefault (0x0046d5a0) skips the
// built-in default image 0x004e06e0. Each side: a real msvcrt image block with real or null buffers and every flag
// byte, a null ECX, or its own default image; free (both import slots) and the hook are logged. Compared: return
// value and the log with block addresses by role.
namespace {
std::vector<std::uint32_t> g_log;
std::uint32_t g_role_base[5];
std::uint32_t role(std::uint32_t p)
{
    for (int k = 0; k < 5; ++k) if (p && p == g_role_base[k]) return 0xA0000000u + k;
    return p;
}
void __cdecl logging_free(void* p)
{
    g_log.push_back(4);
    g_log.push_back(role(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p))));
    reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(p);
}
void __fastcall fake_release_surface(int ecx, int)
{
    g_log.push_back(5);
    g_log.push_back(role(static_cast<std::uint32_t>(ecx)));
}
}  // namespace

TEST(native_image_free_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    void** o_free = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* const saved[2] = {*o_free, recoil::g_Iat_free_004cc5b4};
    *o_free = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    int compared = 0;
    for (int fn = 0; fn < 2; ++fn) {
        const std::uint32_t va = fn ? 0x0046d5a0u : 0x0046ecc0u;
        const Fn f[2] = {rt::original<Fn>(va), reinterpret_cast<Fn>(fn ? &recoil::Image_FreeUnlessDefault : &recoil::Image_Free)};
        std::mt19937 rng(va);
        for (int it = 0; it < 3000; ++it) {
            int kind = static_cast<int>(rng() % 10);  // 0 null, 1 default image (Image_FreeUnlessDefault only), else a real image
            if (kind == 1 && fn == 0) kind = 0;
            const std::uint32_t flags = rng() & 0xFF, nulls = rng() % 16, surface = rng() % 2 ? rng() | 1u : 0u;
            std::uint32_t init[16];
            for (auto& w : init) w = rng();
            int ret[2];
            std::vector<std::uint32_t> snap[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                void* hook = side ? recoil::ImageData_Address(0x0056bc28) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056bc28));
                *static_cast<void**>(hook) = reinterpret_cast<void*>(&fake_release_surface);
                const std::uint32_t def = side ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(recoil::ImageData_Address(0x004e06e0))) : 0x004e06e0u;
                std::uint32_t img = 0;
                for (auto& b : g_role_base) b = 0;
                if (kind == 1) img = def;
                else if (kind > 1) {
                    img = m_malloc(0x40);
                    auto* w = ptr<std::uint32_t>(img);
                    for (int k = 0; k < 16; ++k) w[k] = init[k];
                    reinterpret_cast<unsigned char*>(w)[9] = static_cast<unsigned char>(flags);
                    for (int s = 0; s < 3; ++s) w[4 + s] = (nulls >> s) & 1 ? 0u : m_malloc(8);
                    w[12] = surface;
                    g_role_base[0] = img;
                    for (int s = 0; s < 3; ++s) g_role_base[1 + s] = w[4 + s];
                }
                if (kind == 1) g_role_base[4] = def;
                g_log.clear();
                ret[side] = f[side](ptr<void>(img), 0);
                snap[side] = g_log;
                // blocks the call kept (not freed) are freed here: the image is gone only if its free was logged
                bool image_freed = false;
                for (std::size_t k = 0; k + 1 < g_log.size(); k += 2) if (g_log[k] == 4 && g_log[k + 1] == 0xA0000000u) image_freed = true;
                if (kind > 1 && !image_freed) {
                    auto* w = ptr<std::uint32_t>(img);
                    for (int s = 0; s < 3; ++s) if (w[4 + s]) m_free(w[4 + s]);
                    m_free(img);
                }
                if (kind > 1 && image_freed)
                    for (int s = 0; s < 3; ++s) {  // buffers not freed by the call leak with the image: free them now
                        const std::uint32_t b = g_role_base[1 + s];
                        bool freed = false;
                        for (std::size_t k = 0; k + 1 < g_log.size(); k += 2) if (g_log[k] == 4 && g_log[k + 1] == 0xA0000001u + s) freed = true;
                        if (b && !freed) m_free(b);
                    }
            }
            CHECK_EQ(ret[0], ret[1]);
            CHECK_SNAP(snap[0], snap[1]);
            ++compared;
        }
    }
    *o_free = saved[0];
    recoil::g_Iat_free_004cc5b4 = saved[1];
    rt::restore_pristine();
    std::printf("  Image_Free/Image_FreeUnlessDefault calls %d (real images, null, default image; logged free + surface hook)\n", compared);
}

// TextureSlot_UnloadChain (0x0046e250): from the slot at ECX along [+0x20] while the state [+0x1C] is 1: frees the
// image [+0] with Image_FreeUnlessDefault, nulls it and sets the state to 3. Each side: a chain of 0..4 slots (real
// memory; states 1/2/3/other; image a real msvcrt block, null or that side's default image; the chain may end early
// on a null link). Compared: the slot words (images by role) and the free log.
TEST(native_texture_slot_unload_chain_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn f[2] = {rt::original<Fn>(0x0046e250), reinterpret_cast<Fn>(&recoil::TextureSlot_UnloadChain)};
    void** o_free = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* const saved[2] = {*o_free, recoil::g_Iat_free_004cc5b4};
    *o_free = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    std::mt19937 rng(0x46e250);
    int compared = 0;
    for (int it = 0; it < 4000; ++it) {
        const int n = static_cast<int>(rng() % 5);
        std::uint32_t init[4][9], state[4], kind[4];
        for (int s = 0; s < 4; ++s) {
            for (auto& w : init[s]) w = rng();
            const std::uint32_t st = rng() % 5;
            state[s] = st < 3 ? 1u : st == 3 ? 2u + rng() % 2 : rng();
            kind[s] = rng() % 4;  // 0 null, 1 default, else real
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *static_cast<void**>(side ? recoil::ImageData_Address(0x0056bc28) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056bc28))) =
                reinterpret_cast<void*>(&fake_release_surface);
            const std::uint32_t def = side ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(recoil::ImageData_Address(0x004e06e0))) : 0x004e06e0u;
            std::uint32_t slot[4][9];
            for (auto& b : g_role_base) b = 0;
            g_role_base[4] = def;
            for (int s = 0; s < 4; ++s) {
                for (int k = 0; k < 9; ++k) slot[s][k] = init[s][k];
                slot[s][7] = state[s];
                slot[s][8] = s + 1 < n ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(slot[s + 1])) : 0u;
                if (kind[s] == 0) slot[s][0] = 0;
                else if (kind[s] == 1) slot[s][0] = def;
                else {
                    slot[s][0] = m_malloc(0x40);
                    auto* w = ptr<std::uint32_t>(slot[s][0]);
                    for (int k = 0; k < 16; ++k) w[k] = 0;
                    g_role_base[s] = slot[s][0];
                }
            }
            g_log.clear();
            f[side](n ? slot[0] : nullptr, 0);
            snap[side] = g_log;
            for (int s = 0; s < 4; ++s) {
                for (int k = 0; k < 9; ++k) snap[side].push_back(k == 8 ? (slot[s][8] ? 1u : 0u) : k == 0 ? role(slot[s][0]) : slot[s][k]);
                if (kind[s] > 1 && slot[s][0]) m_free(slot[s][0]);  // images the call kept
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    *o_free = saved[0];
    recoil::g_Iat_free_004cc5b4 = saved[1];
    rt::restore_pristine();
    std::printf("  TextureSlot_UnloadChain calls %d (chains of 0..4 slots, states, real / null / default images)\n", compared);
}
