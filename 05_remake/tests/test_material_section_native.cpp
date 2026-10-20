// Native L1 for Material_WriteSection (0x00480600): it writes the material globals and a copy of the material array
// through a FILE, converting texture pointers into texture-table indices (Texture_ToIndex 0x0046d310), so each side
// gets its own state and its own real msvcrt temp file. Layout from the listing: size [0x00566a18], array pointer
// [0x00566a1c] (0x2C-byte records), used count [0x00566a20], free head [0x004e1160], used head [0x004e1164] (index,
// next link = short at +0x2A, negative ends); flag bit 0x100 (byte +1 bit 0): +0x10 texture pointer; bit 0x400
// (byte +1 bit 2): +0x24 cycle record (0x1C bytes; +0x10 frame count, +0x18 frame list of texture pointers).
// Texture pointers are null or entries of each side's own texture table (0x0053d79c). Compared: return value and
// the file written, byte for byte.
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
using Fread = std::size_t(__cdecl*)(void*, std::size_t, std::size_t, void*);
std::uint32_t* global(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
}  // namespace

TEST(native_material_write_section_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fread = reinterpret_cast<Fread>(recoil::g_Iat_fread_004cc4e0);
    const Fn fn[2] = {rt::original<Fn>(0x00480600), reinterpret_cast<Fn>(&recoil::Material_WriteSection)};
    char dir[MAX_PATH], path[2][MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "mso", 0, path[0]);
    GetTempFileNameA(dir, "msp", 0, path[1]);
    std::mt19937 rng(0x480600);
    int compared = 0;
    for (int it = 0; it < 400; ++it) {
        const int size = 1 + static_cast<int>(rng() % 12);
        // used list: a random subset in random order, linked through +0x2A
        std::vector<int> order(size);
        for (int i = 0; i < size; ++i) order[i] = i;
        std::shuffle(order.begin(), order.end(), rng);
        const int used = static_cast<int>(rng() % (size + 1));
        std::vector<std::uint32_t> recs(size * 11);
        // random words never fall in the test exe image (/BASE 0x60000000): there they could equal a port-side
        // texture-table address and be normalised on one side only
        auto rw = [&] { std::uint32_t w = rng(); return (w >> 24) == 0x60 ? w ^ 0x10000000u : w; };
        for (auto& w : recs) w = rw();
        std::vector<int> tex(size), frames(size);
        std::vector<std::vector<int>> frame_tex(size);
        std::vector<std::uint32_t> cyc(size * 7);
        for (auto& w : cyc) w = rw();
        for (int i = 0; i < size; ++i) {
            tex[i] = rng() % 4 == 0 ? -1 : static_cast<int>(rng() % 30);
            frames[i] = static_cast<int>(rng() % 6);
            for (int k = 0; k < frames[i]; ++k) frame_tex[i].push_back(rng() % 5 == 0 ? -1 : static_cast<int>(rng() % 30));
            recs[11 * i + 10] = (recs[11 * i + 10] & 0x0000FFFFu) | 0xFFFF0000u;  // next = -1 unless linked below
        }
        for (int k = 0; k + 1 < used; ++k)
            recs[11 * order[k] + 10] = (recs[11 * order[k] + 10] & 0xFFFFu) | (static_cast<std::uint32_t>(order[k + 1]) << 16);
        const std::uint32_t head = used ? static_cast<std::uint32_t>(order[0]) : 0xFFFFFFFFu;
        const std::uint32_t used_count = rng() % 50, free_head = rng();
        int ret[2];
        std::vector<unsigned char> file[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const std::uint32_t tbase = addr(global(side, 0x0053d79c));
            auto tp = [&](int i) { return i < 0 ? 0u : tbase + 0x24 * static_cast<std::uint32_t>(i); };
            std::vector<std::uint32_t> r = recs, c = cyc;
            std::vector<std::vector<std::uint32_t>> fl(size);
            for (int i = 0; i < size; ++i) {
                r[11 * i + 4] = tp(tex[i]);
                for (int k = 0; k < frames[i]; ++k) fl[i].push_back(tp(frame_tex[i][k]));
                if (fl[i].empty()) fl[i].push_back(0);
                c[7 * i + 4] = static_cast<std::uint32_t>(frames[i]);
                c[7 * i + 6] = addr(fl[i].data());
                r[11 * i + 9] = addr(&c[7 * i]);
            }
            *global(side, 0x00566a18) = static_cast<std::uint32_t>(size);
            *global(side, 0x00566a1c) = addr(r.data());
            *global(side, 0x00566a20) = used_count;
            *global(side, 0x004e1160) = free_head;
            *global(side, 0x004e1164) = head;
            void* f = m_fopen(path[side], "wb");
            ret[side] = fn[side](f, 0);
            m_fclose(f);
            f = m_fopen(path[side], "rb");
            unsigned char buf[4096];
            for (std::size_t k; (k = m_fread(buf, 1, sizeof buf, f)) > 0;) file[side].insert(file[side].end(), buf, buf + k);
            m_fclose(f);
            // The array and the cycle records are written raw, so they carry this side's pointers (+0x24 to its cycle
            // record, cycle +0x18 to its frame list, +0x10 of an untextured material into its texture table): compare
            // those words by role, every other byte raw.
            for (std::size_t o = 0; o + 4 <= file[side].size(); o += 4) {
                std::uint32_t v;
                std::memcpy(&v, &file[side][o], 4);
                if (v >= tbase && v < tbase + 0x24 * 30) v = 0xE0000000u + (v - tbase);
                for (int i = 0; i < size; ++i) {
                    if (v == addr(&c[7 * i])) v = 0xC0000000u + i;
                    else if (v == addr(fl[i].data())) v = 0xD0000000u + i;
                }
                std::memcpy(&file[side][o], &v, 4);
            }
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(file[0] == file[1]);
        if (file[0] != file[1] && compared < 3) {  // name the first differing word
            std::size_t o = 0;
            while (o < file[0].size() && o < file[1].size() && file[0][o] == file[1][o]) ++o;
            o &= ~std::size_t(3);
            std::uint32_t a = 0, b = 0;
            if (o + 4 <= file[0].size()) std::memcpy(&a, &file[0][o], 4);
            if (o + 4 <= file[1].size()) std::memcpy(&b, &file[1][o], 4);
            std::printf("    first difference at 0x%zx of %zu/%zu bytes (array ends 0x%x): original %08x port %08x\n", o,
                        file[0].size(), file[1].size(), 16 + 0x2C * size, a, b);
        }
        ++compared;
    }
    rt::restore_pristine();
    DeleteFileA(path[0]);
    DeleteFileA(path[1]);
    std::printf("  Material_WriteSection calls %d (own material state, texture table and temp file per side)\n", compared);
}
