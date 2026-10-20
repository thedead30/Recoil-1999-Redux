// Native L1 for the material texture passes (gmod_matl):
// Material_ReleaseTextures (0x00480f80): material at ECX (null-safe); when flags bits 0x100 and 0x200 are both set,
// TextureSlot_UnloadChain on the base slot [+0x10] and on each frame slot of the cycle [+0x24] ([cycle+0x10] count,
// [cycle+0x18] slot array).
// Material_ReuploadTextures (0x00480fd0): along the used list (head [0x004e1164], array [0x00566a1c], short link
// +0x2A, ends on a negative link): for a material with 0x100 set, 0x200 clear, a texture [+0x10] whose [tex+4] is
// non-zero: Image_FreeOwnedBuffers([tex]) when the render mode [0x0056bbe8] is 2, then the zVideo hook [0x0056bc14]
// with ECX = [tex+4].
// Each side builds the same scenario in its own memory (real msvcrt image blocks and buffers, slot chains, material
// arrays); free (both import slots) and the hooks are logged with blocks by role. Compared: the logs and the slot /
// image words afterwards.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_matl.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

namespace {
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t real_malloc(std::size_t n)
{
    return addr(reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"))(n));
}
void real_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(ptr<void>(p)); }

std::vector<std::uint32_t> g_log, g_roles;
std::vector<bool> g_freed;
std::uint32_t role(std::uint32_t p)
{
    for (std::size_t k = 0; k < g_roles.size(); ++k) if (p && p == g_roles[k]) return 0xA0000000u + static_cast<std::uint32_t>(k);
    return p;
}
std::uint32_t owned(std::size_t n)  // a heap block that gets a role
{
    const std::uint32_t b = real_malloc(n);
    g_roles.push_back(b);
    g_freed.push_back(false);
    return b;
}
void __cdecl logging_free(void* p)
{
    const std::uint32_t r = role(addr(p));
    g_log.push_back(4);
    g_log.push_back(r);
    if (r >= 0xA0000000u && r - 0xA0000000u < g_freed.size()) g_freed[r - 0xA0000000u] = true;
    real_free(addr(p));
}
void __fastcall fake_surface(int ecx, int)
{
    g_log.push_back(5);
    g_log.push_back(role(static_cast<std::uint32_t>(ecx)));
}
void __fastcall fake_upload(int ecx, int)
{
    g_log.push_back(6);
    g_log.push_back(role(static_cast<std::uint32_t>(ecx)));
}
void free_unfreed()
{
    for (std::size_t k = 0; k < g_roles.size(); ++k) if (!g_freed[k]) real_free(g_roles[k]);
    g_roles.clear();
    g_freed.clear();
}
// an image block: buffers +0x10/+0x14/+0x18 real or null, flag byte +9, surface +0x30
std::uint32_t make_image(const std::uint32_t* init)
{
    const std::uint32_t im = owned(0x40);
    auto* w = ptr<std::uint32_t>(im);
    for (int k = 0; k < 16; ++k) w[k] = init[k];
    for (int s = 0; s < 3; ++s) w[4 + s] = (init[4 + s] & 3) == 0 ? 0u : owned(8);
    w[12] = init[12] & 1 ? init[12] : 0u;
    return im;
}
struct Slots {
    std::uint32_t mem[3][9];
};
void hooks(int side)
{
    *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc28))) = reinterpret_cast<void*>(&fake_surface);
    *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc14))) = reinterpret_cast<void*>(&fake_upload);
}
struct FreeHook {
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* saved[2];
    FreeHook()
    {
        saved[0] = *o;
        saved[1] = recoil::g_Iat_free_004cc5b4;
        *o = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    }
    ~FreeHook()
    {
        *o = saved[0];
        recoil::g_Iat_free_004cc5b4 = saved[1];
    }
};
}  // namespace

TEST(native_material_release_textures_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn f[2] = {rt::original<Fn>(0x00480f80), reinterpret_cast<Fn>(&recoil::Material_ReleaseTextures)};
    FreeHook fh;
    std::mt19937 rng(0x480f80);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const bool null_mat = rng() % 16 == 0;
        const std::uint32_t flags = rng();
        const int frames = static_cast<int>(rng() % 4);
        const bool has_cycle = rng() % 4 != 0;
        // up to 1 + 3 chains of up to 3 slots: state, image kind (0 null, 1 default, else real), chain length
        std::uint32_t st[4][3], kind[4][3], len[4], iw[4][3][16], sw[4][3][9];
        for (int c = 0; c < 4; ++c) {
            len[c] = rng() % 4;
            for (int s = 0; s < 3; ++s) {
                const std::uint32_t r = rng() % 5;
                st[c][s] = r < 3 ? 1u : r == 3 ? 3u : rng();
                kind[c][s] = rng() % 4;
                for (auto& w : iw[c][s]) w = rng();
                for (auto& w : sw[c][s]) w = rng();
            }
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            hooks(side);
            const std::uint32_t def = addr(img(side, 0x004e06e0));
            g_roles.clear();
            g_freed.clear();
            g_roles.push_back(def);
            g_freed.push_back(true);  // never freed by the test
            Slots chain[4];
            std::uint32_t head[4];
            for (int c = 0; c < 4; ++c) {
                for (int s = 0; s < 3; ++s) {
                    std::uint32_t* sl = chain[c].mem[s];
                    for (int k = 0; k < 9; ++k) sl[k] = sw[c][s][k];
                    sl[7] = st[c][s];
                    sl[8] = static_cast<std::uint32_t>(s + 1) < len[c] ? addr(chain[c].mem[s + 1]) : 0u;
                    sl[0] = kind[c][s] == 0 ? 0u : kind[c][s] == 1 ? def : make_image(iw[c][s]);
                }
                head[c] = len[c] ? addr(chain[c].mem[0]) : 0u;
            }
            std::uint32_t frame_arr[3] = {head[1], head[2], head[3]};
            std::uint32_t cycle[7] = {1, 2, 3, 4, static_cast<std::uint32_t>(frames), 5, addr(frame_arr)};
            std::uint32_t mat[11];
            for (int k = 0; k < 11; ++k) mat[k] = sw[0][0][k % 9] ^ 0x55u;
            mat[0] = flags;
            mat[4] = head[0];
            mat[9] = has_cycle ? addr(cycle) : 0u;
            g_log.clear();
            f[side](null_mat ? nullptr : mat, 0);
            snap[side] = g_log;
            for (int c = 0; c < 4; ++c)
                for (int s = 0; s < 3; ++s) {
                    const std::uint32_t* sl = chain[c].mem[s];
                    snap[side].push_back(role(sl[0]));
                    snap[side].push_back(sl[7]);
                }
            free_unfreed();
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Material_ReleaseTextures calls %d (flags, base chain + 0..3 cycle frame chains, real / null / default images)\n", compared);
}

TEST(native_material_reupload_textures_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(int, int);
    const Fn f[2] = {rt::original<Fn>(0x00480fd0), reinterpret_cast<Fn>(&recoil::Material_ReuploadTextures)};
    FreeHook fh;
    std::mt19937 rng(0x480fd0);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const int size = static_cast<int>(rng() % 7);
        const int used = size ? static_cast<int>(rng() % (size + 1)) : 0;
        std::vector<int> order(size);
        for (int i = 0; i < size; ++i) order[i] = i;
        for (int i = size - 1; i > 0; --i) std::swap(order[i], order[rng() % (i + 1)]);
        const std::uint32_t mode = rng() % 3 == 0 ? rng() % 4 : 2u;
        const std::uint32_t head_neg = rng() | 0x80000000u;
        std::vector<std::uint32_t> mw(11 * 7), iw(16 * 7);
        std::vector<std::uint32_t> tex_kind(7), surf(7);
        for (auto& w : mw) w = rng();
        for (auto& w : iw) w = rng();
        for (int i = 0; i < 7; ++i) {
            tex_kind[i] = rng() % 4;  // 0 no texture, 1 texture with no surface (not uploaded), else with a surface (or not, random)
            surf[i] = rng() % 4 == 0 ? 0u : rng() | 1u;
            const std::uint32_t fl = rng() % 4;
            mw[11 * i] = (mw[11 * i] & ~0x300u) | (fl & 1 ? 0x100u : 0u) | (fl & 2 ? 0x200u : 0u);
            if (rng() % 3) mw[11 * i] = (mw[11 * i] & ~0x300u) | 0x100u;
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            hooks(side);
            g_roles.clear();
            g_freed.clear();
            std::uint32_t mats[7][11], tex[7][2];
            for (int i = 0; i < 7; ++i) {
                for (int k = 0; k < 11; ++k) mats[i][k] = mw[11 * i + k];
                mats[i][10] = (mats[i][10] & 0xFFFFu) | 0xFFFF0000u;
                tex[i][0] = tex_kind[i] ? make_image(&iw[16 * i]) : 0u;  // an uploaded texture always has its image
                tex[i][1] = tex_kind[i] == 1 ? 0u : surf[i];
                mats[i][4] = tex_kind[i] ? addr(tex[i]) : 0u;
            }
            for (int k = 0; k + 1 < used; ++k) mats[order[k]][10] = (mats[order[k]][10] & 0xFFFFu) | (static_cast<std::uint32_t>(order[k + 1]) << 16);
            *img(side, 0x004e1164) = used ? static_cast<std::uint32_t>(order[0]) : head_neg;
            *img(side, 0x00566a1c) = addr(mats);
            *img(side, 0x0056bbe8) = mode;
            g_log.clear();
            f[side](0, 0);
            snap[side] = g_log;
            for (int i = 0; i < 7; ++i) {
                snap[side].push_back(role(tex[i][0]));
                if (tex_kind[i]) {
                    const std::uint32_t* w = ptr<std::uint32_t>(tex[i][0]);
                    for (int k = 0; k < 16; ++k) snap[side].push_back(k >= 4 && k <= 6 ? role(w[k]) : w[k]);
                }
            }
            free_unfreed();
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Material_ReuploadTextures calls %d (used lists of 0..6, textures with / without images, render modes)\n", compared);
}

// ModelInstance_Call480f80OnCycleParts (0x004841f0, model at ECX): for each polygon (+0xC count, +0x30 records of
// 0x1C) whose material [+0x14] has byte +1 bit 0: Material_ReleaseTextures(material). Each side: 0..5 polygons over
// 3 materials (flags 0x100 / 0x200 random), each material's base slot a one-slot chain (state 1 or 3) holding a real
// image; free and the surface hook logged. Compared: the log and the slots afterwards.
TEST(native_model_instance_release_cycle_parts_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn f[2] = {rt::original<Fn>(0x004841f0), reinterpret_cast<Fn>(&recoil::ModelInstance_Call480f80OnCycleParts)};
    FreeHook fh;
    std::mt19937 rng(0x4841f0);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const int polys = static_cast<int>(rng() % 6);
        std::uint32_t pmat[5], mflags[3], st[3], iw[3][16], pw[5][7];
        for (int p = 0; p < 5; ++p) { pmat[p] = rng() % 3; for (auto& w : pw[p]) w = rng(); }
        for (int m = 0; m < 3; ++m) {
            mflags[m] = rng() & ~0x300u;
            if (rng() % 4) mflags[m] |= 0x100u;
            if (rng() % 3) mflags[m] |= 0x200u;
            st[m] = rng() % 3 ? 1u : 3u;
            for (auto& w : iw[m]) w = rng();
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            hooks(side);
            g_roles.clear();
            g_freed.clear();
            std::uint32_t slot[3][9] = {}, mat[3][11] = {}, poly[5][7], model[16] = {};
            for (int m = 0; m < 3; ++m) {
                slot[m][0] = make_image(iw[m]);
                slot[m][7] = st[m];
                mat[m][0] = mflags[m];
                mat[m][4] = addr(slot[m]);
            }
            for (int p = 0; p < 5; ++p) {
                for (int k = 0; k < 7; ++k) poly[p][k] = pw[p][k];
                poly[p][5] = addr(mat[pmat[p]]);
            }
            model[3] = static_cast<std::uint32_t>(polys);
            model[12] = addr(poly);
            g_log.clear();
            f[side](model, 0);
            snap[side] = g_log;
            for (int m = 0; m < 3; ++m) { snap[side].push_back(role(slot[m][0])); snap[side].push_back(slot[m][7]); }
            free_unfreed();
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  ModelInstance_Call480f80OnCycleParts calls %d (0..5 polygons over 3 materials, one-slot chains)\n", compared);
}
