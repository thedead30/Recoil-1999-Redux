// Native L1 for Material_ReadSection (0x004808c0), the reader of Material_WriteSection's layout: (FILE ECX) reads the
// array size [0x00566a18], used count [0x00566a20], free head [0x004e1160] and used head [0x004e1164] (a short read
// reports, -1); size 0 -> 0; allocates the array [0x00566a1c] when null, reallocs it when the new size exceeds the
// previous one, and reads it (0x2C per material). Then per used material (used list through the short at +0x2A): a
// textured one (byte +1 bit 0) gets +0x10 = Texture_FromIndex(+0x10), any other the colour word +2 =
// Pixel_PackRGB(+4); a cycling one (byte +1 bit 2) gets a malloc'd 0x1C cycle and frame list read from the file, frame
// indices -> texture pointers. Returns the size.
// Real files in that layout (a quarter cut short); each side its own texture table (0x0053d79c), 565 format and
// previous array state (none, or a smaller / larger msvcrt block). Compared: return value, the globals and, on
// success, every used material (texture pointers table-relative, cycles by content; unused records raw).
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
using Fopen = void*(__cdecl*)(const char*, const char*);
using Fclose = int(__cdecl*)(void*);
using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t bits(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
void* m_malloc(std::size_t n) { return reinterpret_cast<void*(__cdecl*)(std::size_t)>(recoil::g_Iat_malloc_004cc5dc)(n); }
void m_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4)(ptr<void>(p)); }
}  // namespace

TEST(native_material_read_section_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004808c0), reinterpret_cast<Fn>(&recoil::Material_ReadSection)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "mts", 0, path);
    std::mt19937 rng(0x4808c0);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0;
    for (int it = 0; it < 800; ++it) {
        const int size = it % 12 == 0 ? 0 : 1 + static_cast<int>(rng() % 10);
        std::vector<int> order(size);
        for (int i = 0; i < size; ++i) order[i] = i;
        std::shuffle(order.begin(), order.end(), rng);
        const int used = size ? static_cast<int>(rng() % (size + 1)) : 0;
        std::vector<std::uint32_t> recs(11 * (size ? size : 1));
        std::vector<std::vector<std::uint32_t>> cycles(size);
        for (int i = 0; i < size; ++i) {
            std::uint32_t* r = recs.data() + 11 * i;
            for (int k = 0; k < 11; ++k) r[k] = rng();
            r[0] = (r[0] & ~0x0500u) | (rng() % 2 ? 0x100u : 0u) | (rng() % 3 == 0 ? 0x400u : 0u);
            r[1] = bits(fr(0, 255)); r[2] = bits(fr(0, 255)); r[3] = bits(fr(0, 255));  // colour floats +4..+0xC
            r[4] = rng() % 30;                                                              // texture index
            r[10] = (r[10] & 0xFFFFu) | 0xFFFF0000u;
        }
        for (int k = 0; k + 1 < used; ++k) recs[11 * order[k] + 10] = (recs[11 * order[k] + 10] & 0xFFFFu) | (static_cast<std::uint32_t>(order[k + 1]) << 16);
        std::vector<unsigned char> file;
        auto put = [&](std::uint32_t w) { const auto* b = reinterpret_cast<const unsigned char*>(&w); file.insert(file.end(), b, b + 4); };
        put(static_cast<std::uint32_t>(size)); put(rng() % 50); put(rng()); put(used ? static_cast<std::uint32_t>(order[0]) : 0xFFFFFFFFu);
        for (int i = 0; i < size; ++i) for (int k = 0; k < 11; ++k) put(recs[11 * i + k]);
        for (int k = 0; k < used; ++k) {
            const int i = order[k];
            if (!(recs[11 * i] & 0x400u)) continue;
            std::vector<std::uint32_t>& c = cycles[i];
            c.resize(7);
            for (auto& w : c) w = rng();
            c[4] = rng() % 4;  // frame count
            for (auto w : c) put(w);
            for (std::uint32_t f = 0; f < c[4]; ++f) { const std::uint32_t fi = rng() % 30; c.push_back(fi); put(fi); }
        }
        if (rng() % 4 == 0 && !file.empty()) file.resize(rng() % file.size());
        const int prev = static_cast<int>(rng() % 3);  // 0 no array, 1 smaller, 2 larger
        const int prev_size = prev == 1 ? (size > 1 ? size - 1 : 1) : size + 3;
        void* w = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), w);
        m_fclose(w);
        int ret[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const std::uint32_t fmt[5] = {11, 5, 3, 0x1f, 0x3f};
            for (int k = 0; k < 5; ++k) *img(side, 0x00632170u + 4 * k) = fmt[k];
            const std::uint32_t tbase = addr(img(side, 0x0053d79c));
            *img(side, 0x00566a1c) = prev ? addr(m_malloc(0x2C * prev_size)) : 0;
            *img(side, 0x00566a18) = prev ? static_cast<std::uint32_t>(prev_size) : 0;
            void* f = m_fopen(path, "rb");
            ret[side] = fn[side](f, 0);
            m_fclose(f);
            const std::uint32_t arr = *img(side, 0x00566a1c);
            snap[side] = {static_cast<std::uint32_t>(ret[side]), *img(side, 0x00566a18), *img(side, 0x00566a20), *img(side, 0x004e1160),
                          *img(side, 0x004e1164), arr ? 1u : 0u};
            if (ret[side] > 0) {
                for (int i = 0; i < size; ++i) {
                    const std::uint32_t* r = ptr<std::uint32_t>(arr) + 11 * i;
                    const bool is_used = std::find(order.begin(), order.begin() + used, i) != order.begin() + used;
                    for (int k = 0; k < 11; ++k) {
                        std::uint32_t v = r[k];
                        if (is_used && k == 4 && (r[0] & 0x100u)) v = v - tbase + 0x70000000u;  // texture pointer
                        if (is_used && k == 9 && (r[0] & 0x400u)) v = 0xC7C7C7C7u;              // cycle pointer: content below
                        snap[side].push_back(v);
                    }
                    if (is_used && (r[0] & 0x400u) && r[9]) {
                        const std::uint32_t* c = ptr<std::uint32_t>(r[9]);
                        for (int k = 0; k < 7; ++k) snap[side].push_back(k == 6 ? 0u : c[k]);
                        for (std::uint32_t f = 0; f < c[4]; ++f) snap[side].push_back(ptr<std::uint32_t>(c[6])[f] - tbase + 0x70000000u);
                        m_free(c[6]);
                        m_free(r[9]);
                    }
                }
            }
            m_free(arr);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  Material_ReadSection calls %d (real files, previous array none/smaller/larger, short files)\n", compared);
}
