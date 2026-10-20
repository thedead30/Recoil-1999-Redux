// Native L1 for File_GetSize (0x004a5c50): the arena fuzz cannot supply a FILE, so both sides get a real msvcrt
// FILE (opened through the msvcrt import slots, the same CRT the original's imports resolve to) of 0..4096 bytes,
// positioned at a random offset first, plus a null FILE. Compared: return value and the file position afterwards
// (the function saves and restores it around fseek to the end).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "platform/iat_msvcrt.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

TEST(native_zar_file_get_size_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    using Fseek = int(__cdecl*)(void*, long, int);
    using Ftell = long(__cdecl*)(void*);
    using Size = int(__fastcall*)(void*, int);
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    const auto m_fseek = reinterpret_cast<Fseek>(recoil::g_Iat_fseek_004cc490);
    const auto m_ftell = reinterpret_cast<Ftell>(recoil::g_Iat_ftell_004cc4a0);
    auto orig = rt::original<Size>(0x004a5c50);
    auto port = reinterpret_cast<Size>(&recoil::File_GetSize);
    char path[MAX_PATH], dir[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "zfs", 0, path);
    std::mt19937 rng(0x4a5c50);
    CHECK_EQ(orig(nullptr, 0), port(nullptr, 0));
    int compared = 1;
    for (int it = 0; it < 200; ++it) {
        const int len = it < 8 ? it : static_cast<int>(rng() % 4097);
        std::vector<unsigned char> bytes(len ? len : 1);
        for (auto& b : bytes) b = static_cast<unsigned char>(rng());
        void* w = m_fopen(path, "wb");
        CHECK(w != nullptr);
        if (!w) return;
        m_fwrite(bytes.data(), 1, len, w);
        m_fclose(w);
        const long start = len ? static_cast<long>(rng() % (len + 1)) : 0;
        const int edx = static_cast<int>(rng());
        int ret[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            void* f = m_fopen(path, "rb");
            m_fseek(f, start, SEEK_SET);
            ret[side] = (side ? port : orig)(f, edx);
            pos[side] = m_ftell(f);
            m_fclose(f);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    DeleteFileA(path);
    std::printf("  File_GetSize calls %d (files of 0..4096 bytes at random positions, null FILE)\n", compared);
}
