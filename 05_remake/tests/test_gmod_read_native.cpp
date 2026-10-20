// Native L1 for Model_ReadThreeDwords (0x00481bc0): the arena fuzz cannot supply a FILE, so both sides read a real
// msvcrt FILE (opened through the msvcrt import slots, the same CRT the original's imports resolve to) holding
// 0..16 bytes - every failure point (first, second, third fread short) and success. Compared: return value, the
// three outputs, and the file position afterwards.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>

TEST(native_gmod_read_three_dwords_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    using Ftell = long(__cdecl*)(void*);
    using Read3 = int(__fastcall*)(void*, std::uint32_t*, std::uint32_t*, std::uint32_t*);
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    const auto m_ftell = reinterpret_cast<Ftell>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "ftell"));
    auto orig = rt::original<Read3>(0x00481bc0);
    auto port = reinterpret_cast<Read3>(&recoil::Model_ReadThreeDwords);
    char path[MAX_PATH], dir[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "gmd", 0, path);
    std::mt19937 rng(0x481bc0);
    int compared = 0;
    for (int len = 0; len <= 16; ++len) {
        for (int rep = 0; rep < 8; ++rep) {
            unsigned char bytes[16];
            for (unsigned char& b : bytes) b = static_cast<unsigned char>(rng());
            void* w = m_fopen(path, "wb");
            CHECK(w != nullptr);
            if (!w) return;
            m_fwrite(bytes, 1, len, w);
            m_fclose(w);
            std::uint32_t out[2][3];
            int ret[2];
            long pos[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                void* f = m_fopen(path, "rb");
                for (std::uint32_t& o : out[side]) o = 0xCDCDCDCDu;
                ret[side] = (side ? port : orig)(f, &out[side][0], &out[side][1], &out[side][2]);
                pos[side] = m_ftell(f);
                m_fclose(f);
            }
            CHECK_EQ(ret[0], ret[1]);
            CHECK_EQ(pos[0], pos[1]);
            for (int i = 0; i < 3; ++i) CHECK_EQ(out[0][i], out[1][i]);
            ++compared;
        }
    }
    DeleteFileA(path);
    std::printf("  Model_ReadThreeDwords calls %d (lengths 0..16)\n", compared);
}
