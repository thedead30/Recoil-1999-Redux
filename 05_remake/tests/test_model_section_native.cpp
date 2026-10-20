// Native L1 for Model_WriteSection (0x004815c0). Layout from the listing: count [0x00576200], model array
// [0x00576204] (0x58-byte records), used [0x00576208], free head [0x0057620c]. It writes the three header words; for
// count 0 it stops. Otherwise it writes the array as a placeholder, then per model: +0x10 x 12 bytes from +0x34,
// +0x14 x 12 from +0x38, +0x18 x 12 from +0x40, +0x1C x 0x4C records from +0x3C (each: +0xC x 12 bytes from +0x2C),
// and +0xC polygons (0x1C bytes from +0x30, copied with +0x14 material pointer -> Material_ToIndex, base
// [0x00566a1c]), then per polygon (n = low byte of +0): n x 4 from +8, n x 4 from +0xC if flag 0x200 and non-null,
// n x 8 from +0x10 if the material is textured (byte +1 bit 0). Each model's start offset goes into its record at
// +0x54, and the array is rewritten in place at the end.
// Each side gets its own globals, buffers, material array and real msvcrt temp file, built from one random spec.
// Compared: return value, the file (words that point into a side's own buffers - the original writes its pointers
// raw - by buffer and offset, everything else raw) and every record's +0x54.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

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

// One side's memory: every buffer is a vector of words; the spec fills them identically on both sides.
struct Mem {
    std::vector<std::vector<std::uint32_t>> bufs;
    std::uint32_t* make(std::size_t words, std::mt19937& fill)
    {
        bufs.emplace_back(words ? words : 1);
        // filler words never look like an address of this process (heap, stacks, images all lie below 0x80000000):
        // role() would otherwise map a random word that happens to fall in one side's buffer
        for (auto& w : bufs.back()) w = fill() | 0x80000000u;
        return bufs.back().data();
    }
    std::uint32_t role(std::uint32_t v) const
    {
        for (std::size_t k = 0; k < bufs.size(); ++k) {
            const std::uint32_t lo = addr(bufs[k].data());
            if (v >= lo && v < lo + 4 * bufs[k].size()) return 0xA0000000u + static_cast<std::uint32_t>(k << 16) + (v - lo);
        }
        return v;
    }
};
}  // namespace

TEST(native_model_write_section_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fread = reinterpret_cast<Fread>(recoil::g_Iat_fread_004cc4e0);
    const Fn fn[2] = {rt::original<Fn>(0x004815c0), reinterpret_cast<Fn>(&recoil::Model_WriteSection)};
    char dir[MAX_PATH], path[2][MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "mdo", 0, path[0]);
    GetTempFileNameA(dir, "mdp", 0, path[1]);
    std::mt19937 rng(0x4815c0);
    int compared = 0;
    for (int it = 0; it < 300; ++it) {
        const std::uint32_t seed = rng();
        const int n = it % 10 == 0 ? 0 : 1 + static_cast<int>(rng() % 4);
        int ret[2];
        std::vector<unsigned char> file[2];
        std::vector<std::uint32_t> offsets[2];
        Mem mem[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            Mem& m = mem[side];
            std::mt19937 spec(seed), fill(seed ^ 0x5A5A5A5Au);
            std::uint32_t* mats = m.make(8 * 11, fill);  // 8 materials of 0x2C bytes
            std::uint32_t* models = m.make(n * 22, fill);
            for (int i = 0; i < n; ++i) {
                std::uint32_t* r = models + 22 * i;
                const std::uint32_t polys = spec() % 5, a = spec() % 6, b = spec() % 6, c = spec() % 6, d = spec() % 4;
                r[3] = polys; r[4] = a; r[5] = b; r[6] = c; r[7] = d;
                r[13] = addr(m.make(3 * a, fill));
                r[14] = addr(m.make(3 * b, fill));
                r[16] = addr(m.make(3 * c, fill));
                std::uint32_t* recs = m.make(19 * d, fill);
                for (std::uint32_t k = 0; k < d; ++k) {
                    const std::uint32_t cnt = spec() % 5;
                    recs[19 * k + 3] = cnt;
                    recs[19 * k + 11] = addr(m.make(3 * cnt, fill));
                }
                r[15] = addr(recs);
                std::uint32_t* pl = m.make(7 * polys, fill);
                for (std::uint32_t p = 0; p < polys; ++p) {
                    const std::uint32_t nv = spec() % 7;
                    pl[7 * p] = (spec() & 0xFFFFFF00u) | nv;
                    pl[7 * p + 2] = addr(m.make(nv, fill));
                    pl[7 * p + 3] = spec() % 3 == 0 ? 0 : addr(m.make(nv, fill));
                    pl[7 * p + 4] = addr(m.make(2 * nv, fill));
                    pl[7 * p + 5] = addr(mats + 11 * (spec() % 8));
                }
                r[12] = addr(pl);
            }
            *global(side, 0x00576200) = static_cast<std::uint32_t>(n);
            *global(side, 0x00576204) = addr(models);
            *global(side, 0x00576208) = spec();
            *global(side, 0x0057620c) = spec();
            *global(side, 0x00566a1c) = addr(mats);
            void* f = m_fopen(path[side], "wb");
            ret[side] = fn[side](f, 0);
            m_fclose(f);
            f = m_fopen(path[side], "rb");
            unsigned char buf[4096];
            for (std::size_t k; (k = m_fread(buf, 1, sizeof buf, f)) > 0;) file[side].insert(file[side].end(), buf, buf + k);
            m_fclose(f);
            for (std::size_t o = 0; o + 4 <= file[side].size(); o += 4) {
                std::uint32_t v;
                std::memcpy(&v, &file[side][o], 4);
                v = m.role(v);
                std::memcpy(&file[side][o], &v, 4);
            }
            for (int i = 0; i < n; ++i) offsets[side].push_back(models[22 * i + 21]);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(file[0] == file[1]);
        CHECK(offsets[0] == offsets[1]);
        if (file[0] != file[1] && compared < 3) {
            std::size_t o = 0;
            while (o < file[0].size() && o < file[1].size() && file[0][o] == file[1][o]) ++o;
            std::printf("    first difference at byte 0x%zx of %zu/%zu\n", o, file[0].size(), file[1].size());
        }
        ++compared;
    }
    rt::restore_pristine();
    DeleteFileA(path[0]);
    DeleteFileA(path[1]);
    std::printf("  Model_WriteSection calls %d (own model state, material array and temp file per side)\n", compared);
}
