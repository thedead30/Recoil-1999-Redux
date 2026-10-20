// Structured native L1 for GameZ_OpenAndCheckHeader (0x004556a0, ECX path, EDX 0x24-byte header buffer; P2.6):
// fopen(path, "rb") (0x004cc5b8; failure -> 0); fread(EDX, 0x24, 1) (0x004cc4e0) must return 1, the header's magic
// +0 must be 0x02971222 and its version +4 15 - else report, fclose (0x004cc5c0), 0; returns the open FILE.
// Files written to the temp directory, one per case: missing, empty, short (1..0x23 bytes), right size with the magic
// and version right or either wrong, and longer files. Compared: whether a FILE came back, its position (ftell) and
// the next byte read from it, and the header buffer (0x30 bytes, a guard past the 0x24 the call may write).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_zbd.h"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

TEST(native_zclass_gamez_open_and_check_header_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::FILE*(__fastcall*)(const char*, void*);
    const Fn fn[2] = {rt::original<Fn>(0x004556a0), reinterpret_cast<Fn>(&recoil::GameZ_OpenAndCheckHeader)};
    // the FILE the call returns comes from the system msvcrt (both sides' import slots): use its ftell / fgetc / fclose
    HMODULE crt = GetModuleHandleA("msvcrt.dll");
    auto c_ftell = reinterpret_cast<long(__cdecl*)(std::FILE*)>(GetProcAddress(crt, "ftell"));
    auto c_fgetc = reinterpret_cast<int(__cdecl*)(std::FILE*)>(GetProcAddress(crt, "fgetc"));
    auto c_fclose = reinterpret_cast<int(__cdecl*)(std::FILE*)>(GetProcAddress(crt, "fclose"));
    char dir[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    const std::string path = std::string(dir) + "recoil_test_zbd_header.zbd";
    std::mt19937 rng(0x4556a0);
    int compared = 0, opened = 0;
    for (int it = 0; it < 600; ++it) {
        const int kind = it % 6;  // 0 missing, 1 empty / short, 2 good, 3 bad magic, 4 bad version, 5 good and longer
        std::vector<unsigned char> bytes;
        if (kind == 1) bytes.resize(rng() % 0x24);
        else if (kind >= 2) bytes.resize(kind == 5 ? 0x24 + 1 + rng() % 64 : 0x24);
        for (auto& b : bytes) b = static_cast<unsigned char>(rng());
        auto put = [&](int off, std::uint32_t v) { if (bytes.size() >= static_cast<std::size_t>(off + 4)) std::memcpy(&bytes[off], &v, 4); };
        if (kind != 3) put(0, 0x02971222u);
        if (kind != 4) put(4, 15u);
        if (kind == 3 && rng() % 2) put(0, 0x02971222u ^ (1u << (rng() % 32)));  // one bit off
        if (kind == 4) put(4, rng() % 2 ? 14u : 16u + rng() % 100);
        DeleteFileA(path.c_str());
        if (kind != 0) {
            std::FILE* f = std::fopen(path.c_str(), "wb");
            if (!bytes.empty()) std::fwrite(bytes.data(), 1, bytes.size(), f);
            std::fclose(f);
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            unsigned char header[0x30];
            std::memset(header, 0xCD, sizeof header);
            std::FILE* f = fn[side](path.c_str(), header);
            snap[side].push_back(f ? 1u : 0u);
            if (f) {
                snap[side].push_back(static_cast<std::uint32_t>(c_ftell(f)));
                snap[side].push_back(static_cast<std::uint32_t>(c_fgetc(f)));
                c_fclose(f);
            }
            for (int k = 0; k < 0x30; k += 4) { std::uint32_t w; std::memcpy(&w, header + k, 4); snap[side].push_back(w); }
            if (side == 0) opened += f != nullptr;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path.c_str());
    rt::restore_pristine();
    std::printf("  GameZ_OpenAndCheckHeader calls %d, %d returned a FILE\n", compared, opened);
    CHECK(opened >= 150);
}
