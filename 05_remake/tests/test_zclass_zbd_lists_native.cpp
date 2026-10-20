// Structured native L1 for the GameZ node-list writer / reader (cls_zbd.c; P2.6), both (ECX array, EDX count, one stack
// argument: the FILE; count 0 -> 0 with no I/O).
//   GameZ_WriteNodePointerList (0x004543f0): grows the scratch array [0x00539cd8] (capacity [0x00539cdc]) with realloc
//     when the count exceeds the capacity, copies the node pointers into it and turns each into a record index
//     (ClsRecord_ToIndex: null -> -1, else (p - pool [0x00539c94]) / 0xC4), then fwrite(scratch, 4 * count, 1); a
//     failed write -> report, -1; else 0.
//   GameZ_ReadNodeIndexList (0x00454bf0): fread(array, 4 * count, 1) (short -> report, -1), then each index in place
//     becomes a record pointer (ClsRecord_FromIndex: negative -> 0, else pool + 0xC4 * index); 0.
// Each call: its own pool of 6 records; pointers to records, into the middle of a record, below the pool or null;
// indices -3..8; the scratch array absent or a real msvcrt block of capacity 1..8; files written, cut short, or opened
// read-only for the writer (fwrite fails). Compared: the return, the file's bytes after the call (writer) or the array
// by role (reader), the scratch array's contents and capacity, and the file position.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_zbd.h"
#include "platform/image/original_data.h"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t* at(std::uint32_t p) { return reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(p)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }

struct Crt {  // the system msvcrt, the CRT both sides' import slots use
    HMODULE m = GetModuleHandleA("msvcrt.dll");
    void*(__cdecl* open)(const char*, const char*) = reinterpret_cast<void*(__cdecl*)(const char*, const char*)>(GetProcAddress(m, "fopen"));
    int(__cdecl* close)(void*) = reinterpret_cast<int(__cdecl*)(void*)>(GetProcAddress(m, "fclose"));
    long(__cdecl* tell)(void*) = reinterpret_cast<long(__cdecl*)(void*)>(GetProcAddress(m, "ftell"));
    std::size_t(__cdecl* write)(const void*, std::size_t, std::size_t, void*) =
        reinterpret_cast<std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*)>(GetProcAddress(m, "fwrite"));
    std::size_t(__cdecl* read)(void*, std::size_t, std::size_t, void*) =
        reinterpret_cast<std::size_t(__cdecl*)(void*, std::size_t, std::size_t, void*)>(GetProcAddress(m, "fread"));
    void*(__cdecl* alloc)(std::size_t) = reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(m, "malloc"));
    void(__cdecl* release)(void*) = reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(m, "free"));
};

std::vector<unsigned char> read_all(const Crt& c, const char* path)
{
    std::vector<unsigned char> b(4096);
    void* f = c.open(path, "rb");
    b.resize(f ? c.read(b.data(), 1, b.size(), f) : 0);
    if (f) c.close(f);
    return b;
}
}  // namespace

TEST(native_zclass_gamez_write_node_pointer_list_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int, void*);
    const Fn fn[2] = {rt::original<Fn>(0x004543f0), reinterpret_cast<Fn>(&recoil::GameZ_WriteNodePointerList)};
    const Crt c;
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "wnp", 0, path);
    std::mt19937 rng(0x4543f0);
    int compared = 0, written = 0, grown = 0;
    for (int it = 0; it < 3000; ++it) {
        const int count = static_cast<int>(rng() % 9);
        std::vector<int> kind(count), rec(count), off(count);
        for (int k = 0; k < count; ++k) { kind[k] = static_cast<int>(rng() % 5); rec[k] = static_cast<int>(rng() % 6); off[k] = 4 * static_cast<int>(1 + rng() % 48); }
        const int cap = rng() % 3 == 0 ? 0 : 1 + static_cast<int>(rng() % 8);
        const bool read_only = rng() % 8 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t pool[64 + 6 * 49];  // 64 words below the pool: pointers "below" stay in this array
            std::uint32_t* base = pool + 64;
            std::vector<std::uint32_t> ptrs(count + 1, 0xEEEEEEEEu);
            for (int k = 0; k < count; ++k)
                ptrs[k] = kind[k] == 0 ? 0u : kind[k] == 1 ? addr(base) - 4u * (1 + rec[k]) : kind[k] == 2 ? addr(base + 49 * rec[k]) + off[k]
                                                                                                          : addr(base + 49 * rec[k]);
            *img(side, 0x00539c94) = addr(base);
            const std::uint32_t scratch = cap ? addr(c.alloc(4 * cap)) : 0u;
            if (scratch) std::memset(at(scratch), 0x77, 4 * cap);
            *img(side, 0x00539cd8) = scratch;
            *img(side, 0x00539cdc) = static_cast<std::uint32_t>(cap);
            if (read_only) { void* w = c.open(path, "wb"); c.write("zbd!", 1, 4, w); c.close(w); }
            void* f = c.open(path, read_only ? "rb" : "wb");
            const int r = fn[side](ptrs.data(), count, f);
            snap[side].push_back(static_cast<std::uint32_t>(r));
            snap[side].push_back(static_cast<std::uint32_t>(c.tell(f)));
            c.close(f);
            const std::vector<unsigned char> bytes = read_all(c, path);
            snap[side].push_back(static_cast<std::uint32_t>(bytes.size()));
            for (unsigned char b : bytes) snap[side].push_back(b);
            const std::uint32_t now = *img(side, 0x00539cd8), now_cap = *img(side, 0x00539cdc);
            snap[side].push_back(now_cap);
            snap[side].push_back(now == scratch ? 0x5A5Au : now ? 0x5B5Bu : 0u);
            for (std::uint32_t k = 0; now && k < now_cap; ++k) snap[side].push_back(k < static_cast<std::uint32_t>(count) || now == scratch ? at(now)[k] : 0u);
            for (std::uint32_t w : ptrs) snap[side].push_back(w == 0xEEEEEEEEu ? w : w - addr(pool));  // the input is not written
            if (now) c.release(at(now));
            *img(side, 0x00539cd8) = 0;
            if (side == 0) { written += r == 0 && count > 0 && !read_only; grown += now != scratch; }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  GameZ_WriteNodePointerList calls %d, %d wrote, %d grew the scratch array\n", compared, written, grown);
    CHECK(written > 1000 && grown > 300);
}

TEST(native_zclass_gamez_read_node_index_list_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t*, int, void*);
    const Fn fn[2] = {rt::original<Fn>(0x00454bf0), reinterpret_cast<Fn>(&recoil::GameZ_ReadNodeIndexList)};
    const Crt c;
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "rni", 0, path);
    std::mt19937 rng(0x454bf0);
    int compared = 0, read_ok = 0;
    for (int it = 0; it < 3000; ++it) {
        const int count = static_cast<int>(rng() % 9);
        std::vector<std::int32_t> idx(count + 3);
        for (auto& v : idx) v = static_cast<std::int32_t>(rng() % 12) - 3;
        std::size_t bytes = 4 * idx.size();
        if (rng() % 5 == 0) bytes = rng() % (4 * count + 1);  // short
        const int skip = static_cast<int>(rng() % 3);          // words already read before the call
        {
            void* w = c.open(path, "wb");
            c.write(idx.data(), 1, bytes, w);
            c.close(w);
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t pool[12 * 49];
            *img(side, 0x00539c94) = addr(pool);
            std::vector<std::uint32_t> arr(count + 2, 0xAAAAAAAAu);
            void* f = c.open(path, "rb");
            std::uint32_t junk[3];
            if (skip) c.read(junk, 4, skip, f);
            const int r = fn[side](arr.data(), count, f);
            snap[side].push_back(static_cast<std::uint32_t>(r));
            snap[side].push_back(static_cast<std::uint32_t>(c.tell(f)));
            c.close(f);
            for (std::uint32_t w : arr) {
                const std::uint32_t d = w - addr(pool);
                snap[side].push_back(w >= addr(pool) && d < sizeof pool && d % 0xC4 == 0 ? 0xB0000000u + d / 0xC4 : w);
            }
            if (side == 0) read_ok += r == 0 && count > 0;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  GameZ_ReadNodeIndexList calls %d, %d read a list\n", compared, read_ok);
    CHECK(read_ok > 1000);
}
