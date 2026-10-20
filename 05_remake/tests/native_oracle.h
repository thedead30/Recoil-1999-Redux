// Native L1 oracle: map the ORIGINAL Recoil.exe's sections into this (32-bit) test process at their
// original virtual addresses, so original functions can be called directly and run on the real x87 FPU.
// This settles cases where Ghidra's p-code emulator models the FPU imperfectly (e.g. the C3 flag after
// FCOMP, FCHS of a NaN). Only leaf code that needs no imports and no uninitialised runtime state may be
// called this way. The test executable is linked at a different base (/BASE, CMakeLists.txt) so the
// 0x00400000 region is free.
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

#include "platform/image/original_data.h"
#include "platform/mfc42.h"

namespace rt {

struct Pristine;
inline Pristine& pristine();

#ifndef RECOIL_ORIGINAL_EXE
#define RECOIL_ORIGINAL_EXE "00_original/game_install/Recoil.exe"
#endif

inline bool map_original()
{
    static int state = 0;  // 0 not tried, 1 mapped, -1 failed
    if (state) return state > 0;
    state = -1;
    std::ifstream f(RECOIL_ORIGINAL_EXE, std::ios::binary);
    if (!f) { std::printf("  native oracle: cannot open %s\n", RECOIL_ORIGINAL_EXE); return false; }
    std::vector<char> b((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    auto u32 = [&](size_t o) { std::uint32_t v; std::memcpy(&v, &b[o], 4); return v; };
    auto u16 = [&](size_t o) { std::uint16_t v; std::memcpy(&v, &b[o], 2); return v; };
    const std::uint32_t pe = u32(0x3c);
    const std::uint16_t nsec = u16(pe + 6), opt = u16(pe + 20);
    const std::uint32_t base = u32(pe + 52), size = u32(pe + 80);
    void* mem = VirtualAlloc(reinterpret_cast<void*>(static_cast<std::uintptr_t>(base)), size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!mem)  // the runner reserved the range in advance (test_main.cpp): commit it
        mem = VirtualAlloc(reinterpret_cast<void*>(static_cast<std::uintptr_t>(base)), size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (mem != reinterpret_cast<void*>(static_cast<std::uintptr_t>(base))) {
        MEMORY_BASIC_INFORMATION mbi{};
        VirtualQuery(reinterpret_cast<void*>(static_cast<std::uintptr_t>(base)), &mbi, sizeof mbi);
        std::printf("  native oracle: VirtualAlloc(0x%08x, 0x%x) failed, error %lu; region state 0x%lx type 0x%lx protect 0x%lx base %p size 0x%zx\n",
                    base, size, GetLastError(), mbi.State, mbi.Type, mbi.Protect, mbi.AllocationBase, mbi.RegionSize);
        char name[MAX_PATH] = {};
        if (GetMappedFileNameA(GetCurrentProcess(), reinterpret_cast<void*>(static_cast<std::uintptr_t>(base)), name, MAX_PATH)) std::printf("  mapped file: %s\n", name);
        return false;
    }
    std::memcpy(mem, b.data(), u32(pe + 84));  // headers
    for (int i = 0; i < nsec; ++i) {
        const size_t s = pe + 24 + opt + i * 40;
        const std::uint32_t va = u32(s + 12), rawsz = u32(s + 16), raw = u32(s + 20);
        if (rawsz) std::memcpy(static_cast<char*>(mem) + va, &b[raw], rawsz);
    }
    // Resolve the MSVCRT.dll imports against the system msvcrt.dll (the same DLL the port's IAT slots use,
    // platform/msvcrt.cpp), so original code that allocates or frees shares one heap with the port. Other
    // DLLs stay unresolved: original code that calls them must not be run here.
    char* img = static_cast<char*>(mem);
    auto ru32 = [&](std::uint32_t rva) { std::uint32_t v; std::memcpy(&v, img + rva, 4); return v; };
    HMODULE crt = LoadLibraryA("msvcrt.dll");
    for (std::uint32_t d = u32(pe + 24 + 104); ru32(d + 12); d += 20) {
        if (_stricmp(img + ru32(d + 12), "msvcrt.dll") != 0) continue;
        const std::uint32_t names = ru32(d) ? ru32(d) : ru32(d + 16), iat = ru32(d + 16);
        for (std::uint32_t k = 0; ru32(names + 4 * k); ++k) {
            const std::uint32_t hint = ru32(names + 4 * k);
            if (hint & 0x80000000u) continue;  // by ordinal: none in Recoil.exe's MSVCRT imports
            const auto p = reinterpret_cast<std::uintptr_t>(GetProcAddress(crt, img + hint + 2));
            const std::uint32_t v = static_cast<std::uint32_t>(p);
            std::memcpy(img + iat + 4 * k, &v, 4);
        }
    }
    // MFC42 operator new (#823) / delete (#825): the same platform functions the port's slots hold (platform/mfc42.h),
    // so original code that allocates with new shares the msvcrt heap and the heap recorder with the port.
    const auto op_new = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&recoil::Platform_OperatorNew));
    const auto op_delete = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&recoil::Platform_OperatorDelete));
    std::memcpy(img + (0x004cc2ac - 0x00400000), &op_new, 4);
    std::memcpy(img + (0x004cc2b8 - 0x00400000), &op_delete, 4);
    state = 1;
    pristine();  // capture the untouched data sections now, before any test writes to them
    return true;
}

// Pristine copies of the original's data sections (from the end of its import table to the end of the image) and of the port's data image, taken
// right after mapping; restore_pristine() puts both back, so a test can start from the same data on both sides.
struct Pristine {
    std::vector<unsigned char> orig, rdata, data;
    std::vector<std::vector<unsigned char>> blocks;  // named port data blocks (KG-37), in ImageData_Block order
};
// First address after the original's import address table (the slots tests redirect are never restored).
inline std::uint32_t restore_from()
{
    const auto* base = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(0x00400000));
    std::uint32_t pe, rva, size;
    std::memcpy(&pe, base + 0x3c, 4);
    std::memcpy(&rva, base + pe + 24 + 96 + 12 * 8, 4);
    std::memcpy(&size, base + pe + 24 + 96 + 12 * 8 + 4, 4);
    return (0x00400000 + rva + size + 3) & ~3u;
}
inline Pristine& pristine()
{
    static Pristine p;
    static bool taken = false;
    if (!taken) {
        taken = true;
        const auto* lo = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(restore_from()));
        const auto* hi = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(0x007c9000));
        p.orig.assign(lo, hi);
        p.rdata.assign(recoil::g_RData_004cc000, recoil::g_RData_004cc000 + recoil::kRDataSize);
        p.data.assign(recoil::g_Data_004da000, recoil::g_Data_004da000 + recoil::kImageTail);
        void* at;
        std::uint32_t size;
        for (unsigned i = 0; recoil::ImageData_Block(i, &at, &size); ++i)
            p.blocks.emplace_back(static_cast<unsigned char*>(at), static_cast<unsigned char*>(at) + size);
    }
    return p;
}
inline void restore_pristine()
{
    Pristine& p = pristine();
    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(restore_from())), p.orig.data(), p.orig.size());
    std::memcpy(recoil::g_RData_004cc000, p.rdata.data(), p.rdata.size());
    std::memcpy(recoil::g_Data_004da000, p.data.data(), p.data.size());
    void* at;
    std::uint32_t size;
    for (unsigned i = 0; i < p.blocks.size() && recoil::ImageData_Block(i, &at, &size); ++i)
        std::memcpy(at, p.blocks[i].data(), p.blocks[i].size());  // named blocks too (KG-37)
}

template <class F>
F original(std::uint32_t va)
{
    return reinterpret_cast<F>(static_cast<std::uintptr_t>(va));
}

}  // namespace rt
