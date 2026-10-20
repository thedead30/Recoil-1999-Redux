// Native L1 for Texture_WriteDirectory (0x0046d360) and Texture_ReadDirectory (0x0046d420): they move the texture
// table (count 0x0053d798, 0x24-byte entries at 0x0053d79c, +0x20 a pointer into the table or null, written to the
// file as an index) through a FILE, so each side gets its own table (the original's, and the port's via
// recoil::ImageData_Address) and its own real msvcrt temp file.
// Write: 0..20 entries, +0x20 null or an entry of the same table - the files written and the return must be equal.
// Read: files of count*0x24 bytes with +0x20 indices (-1 or 0..count-1), also count 0, count > 0x1000 and a short
// file - return, count and every entry must be equal, +0x20 compared after mapping into each side's table.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zImage/zimg_texture.h"
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
using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
using Fread = std::size_t(__cdecl*)(void*, std::size_t, std::size_t, void*);
constexpr std::uint32_t kCount = 0x0053d798, kTable = 0x0053d79c;
std::uint32_t* table(int side)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(kTable) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(kTable)));
}
std::uint32_t* count(int side)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(kCount) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(kCount)));
}
std::uint32_t base(int side) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(table(side))); }
}  // namespace

TEST(native_texture_directory_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uintptr_t, std::uintptr_t);
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    const auto m_fread = reinterpret_cast<Fread>(recoil::g_Iat_fread_004cc4e0);
    const Fn write_fn[2] = {rt::original<Fn>(0x0046d360), reinterpret_cast<Fn>(&recoil::Texture_WriteDirectory)};
    const Fn read_fn[2] = {rt::original<Fn>(0x0046d420), reinterpret_cast<Fn>(&recoil::Texture_ReadDirectory)};
    char dir[MAX_PATH], path[2][MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "tdo", 0, path[0]);
    GetTempFileNameA(dir, "tdp", 0, path[1]);
    std::mt19937 rng(0x46d360);
    int compared = 0;
    // write
    for (int it = 0; it < 300; ++it) {
        const int n = static_cast<int>(rng() % 21);
        std::vector<std::uint32_t> words(n * 9);
        std::vector<int> link(n);
        for (int e = 0; e < n; ++e) {
            for (int k = 0; k < 8; ++k) words[9 * e + k] = rng();
            link[e] = rng() % 4 == 0 ? -1 : static_cast<int>(rng() % n);
        }
        int ret[2];
        std::vector<unsigned char> file[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *count(side) = static_cast<std::uint32_t>(n);
            for (int e = 0; e < n; ++e) {
                std::memcpy(table(side) + 9 * e, &words[9 * e], 32);
                table(side)[9 * e + 8] = link[e] < 0 ? 0 : base(side) + 0x24 * link[e];
            }
            void* f = m_fopen(path[side], "wb");
            ret[side] = write_fn[side](reinterpret_cast<std::uintptr_t>(f), 0);
            m_fclose(f);
            f = m_fopen(path[side], "rb");
            unsigned char buf[4096];
            for (std::size_t k; (k = m_fread(buf, 1, sizeof buf, f)) > 0;) file[side].insert(file[side].end(), buf, buf + k);
            m_fclose(f);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(file[0] == file[1]);
        ++compared;
    }
    // read
    for (int it = 0; it < 300; ++it) {
        const int kind = static_cast<int>(rng() % 10);
        int n = static_cast<int>(rng() % 21);
        if (kind == 0) n = 0;
        if (kind == 1) n = 0x1001 + static_cast<int>(rng() % 4);
        std::vector<std::uint32_t> words((n <= 0x1000 ? n : 1) * 9);
        for (std::size_t w = 0; w < words.size(); ++w) words[w] = rng();
        for (int e = 0; n <= 0x1000 && e < n; ++e) words[9 * e + 8] = rng() % 4 == 0 ? 0xFFFFFFFFu : rng() % n;
        const std::size_t bytes = kind == 2 && n ? words.size() * 4 - 1 - rng() % 0x24 : words.size() * 4;
        void* f = m_fopen(path[0], "wb");
        m_fwrite(words.data(), 1, bytes, f);
        m_fclose(f);
        int ret[2];
        std::uint32_t cnt[2];
        std::vector<std::uint32_t> got[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            void* r = m_fopen(path[0], "rb");
            ret[side] = read_fn[side](static_cast<std::uintptr_t>(n), reinterpret_cast<std::uintptr_t>(r));
            m_fclose(r);
            cnt[side] = *count(side);
            got[side].assign(table(side), table(side) + 9 * 21);
            for (int e = 0; e < 21; ++e) {
                std::uint32_t& p = got[side][9 * e + 8];
                if (p >= base(side) && p < base(side) + 0x24 * 0x1000) p = p - base(side) + 0x80000000u;  // table-relative
            }
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_EQ(cnt[0], cnt[1]);
        CHECK(got[0] == got[1]);
        ++compared;
    }
    rt::restore_pristine();
    DeleteFileA(path[0]);
    DeleteFileA(path[1]);
    std::printf("  Texture_WriteDirectory/ReadDirectory calls %d (own table and temp file per side; error paths)\n", compared);
}
