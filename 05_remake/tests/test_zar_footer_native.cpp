// Native L1 for Zar_WriteFooterAndClose (0x004a6360): it calls KERNEL32 SetFilePointer and WriteFile, which the
// oracle leaves unresolved in the original's import table (tests/native_oracle.h), so both sides get the same fakes
// - installed in the original's slots 0x004cc144 / 0x004cc138 and in the port's generated g_Iat_ slots - that log
// every call with its arguments and the bytes written. Archive object: +4 handle, +8 open flag, +0xC entry count
// (0..6), +0x14 index buffer (count * 0x94 random bytes). Compared: return value, the call log and the object words.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/asset_io_misc.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "GameZRecoil/zReader/zreader.h"
#include "platform/image/original_data.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::vector<std::uint32_t> g_log;  // per call: kind, args..., then written bytes
DWORD WINAPI fake_set_file_pointer(HANDLE h, LONG dist, LONG* high, DWORD method)
{
    g_log.push_back(1);
    g_log.push_back(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(h)));
    g_log.push_back(static_cast<std::uint32_t>(dist));
    g_log.push_back(high ? 1u : 0u);
    g_log.push_back(method);
    return 0x1234;
}
BOOL WINAPI fake_write_file(HANDLE h, const void* buf, DWORD n, DWORD* written, void* overlapped)
{
    g_log.push_back(2);
    g_log.push_back(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(h)));
    g_log.push_back(n);
    g_log.push_back(overlapped ? 1u : 0u);
    const auto* b = static_cast<const unsigned char*>(buf);
    for (DWORD i = 0; i < n; ++i) g_log.push_back(b[i]);
    *written = n;
    return TRUE;
}
void __cdecl logging_free(void* p)  // records the block, then really frees it (compared by role below)
{
    g_log.push_back(4);
    g_log.push_back(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)));
    reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(p);
}
BOOL WINAPI fake_close_handle(HANDLE h)
{
    g_log.push_back(3);
    g_log.push_back(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(h)));
    return TRUE;
}
}  // namespace

TEST(native_zar_write_footer_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Footer = int(__fastcall*)(void*, int);
    auto orig = rt::original<Footer>(0x004a6360);
    auto port = reinterpret_cast<Footer>(&recoil::Zar_WriteFooterAndClose);
    auto* o_seek = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc144));
    auto* o_write = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc138));
    void* const saved[4] = {*o_seek, *o_write, recoil::g_Iat_SetFilePointer_004cc144, recoil::g_Iat_WriteFile_004cc138};
    *o_seek = recoil::g_Iat_SetFilePointer_004cc144 = reinterpret_cast<void*>(&fake_set_file_pointer);
    *o_write = recoil::g_Iat_WriteFile_004cc138 = reinterpret_cast<void*>(&fake_write_file);
    std::mt19937 rng(0x4a6360);
    int compared = 0;
    for (int it = 0; it < 500; ++it) {
        const std::uint32_t count = rng() % 7;
        std::vector<unsigned char> index(count * 0x94 + 1);
        for (auto& b : index) b = static_cast<unsigned char>(rng());
        std::uint32_t obj[2][8];
        for (auto& w : obj[0]) w = rng();
        obj[0][3] = count;
        obj[0][5] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(index.data()));
        std::memcpy(obj[1], obj[0], sizeof obj[0]);
        const int edx = static_cast<int>(rng());
        int ret[2];
        std::vector<std::uint32_t> log[2];
        for (int side = 0; side < 2; ++side) {
            g_log.clear();
            ret[side] = (side ? port : orig)(obj[side], edx);
            log[side] = g_log;
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(log[0] == log[1]);
        for (int i = 0; i < 8; ++i) CHECK_EQ(obj[0][i], obj[1][i]);
        ++compared;
    }
    *o_seek = saved[0];
    *o_write = saved[1];
    recoil::g_Iat_SetFilePointer_004cc144 = saved[2];
    recoil::g_Iat_WriteFile_004cc138 = saved[3];
    std::printf("  Zar_WriteFooterAndClose calls %d (fake SetFilePointer/WriteFile, 0..6 index entries)\n", compared);
}

// ZarArchive_Close (0x004a62b0): footer (Zar_WriteFooterAndClose) when +8 is set, CloseHandle unless +4 is -1, then
// ZarIndex_Free (frees +0x14, resets the fields). Same fakes plus CloseHandle (slot 0x004cc0e4); the index buffer is
// a real msvcrt heap block per side (the call frees it). Compared: return value, the call log and the object words.
TEST(native_zar_archive_close_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Close = int(__fastcall*)(void*, int);
    using Malloc = void*(__cdecl*)(std::size_t);
    // ZarArchive_Close, then ZarArchive_Destruct (0x004a61b0): Close, then free the name block at +0 (a real heap
    // block or null here; compared as null / non-null)
    const std::uint32_t vas[2] = {0x004a62b0, 0x004a61b0};
    const Close ports[2] = {reinterpret_cast<Close>(&recoil::ZarArchive_Close), reinterpret_cast<Close>(&recoil::ZarArchive_Destruct)};
    const auto m_malloc = reinterpret_cast<Malloc>(recoil::g_Iat_malloc_004cc5dc);
    // free is logged too, so a skipped free (a leak) shows as a difference
    void** o_slot[4] = {reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc144)),
                        reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc138)),
                        reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc0e4)),
                        reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4))};
    void** p_slot[4] = {&recoil::g_Iat_SetFilePointer_004cc144, &recoil::g_Iat_WriteFile_004cc138, &recoil::g_Iat_CloseHandle_004cc0e4,
                        &recoil::g_Iat_free_004cc5b4};
    void* const fakes[4] = {reinterpret_cast<void*>(&fake_set_file_pointer), reinterpret_cast<void*>(&fake_write_file),
                            reinterpret_cast<void*>(&fake_close_handle), reinterpret_cast<void*>(&logging_free)};
    void* saved[8];
    for (int i = 0; i < 4; ++i) {
        saved[i] = *o_slot[i];
        saved[4 + i] = *p_slot[i];
        *o_slot[i] = *p_slot[i] = fakes[i];
    }
    int compared = 0;
    for (int fn = 0; fn < 2; ++fn) {
    auto orig = rt::original<Close>(vas[fn]);
    auto port = ports[fn];
    std::mt19937 rng(vas[fn]);
    for (int it = 0; it < 500; ++it) {
        const bool has_index = rng() % 4 != 0;
        const std::uint32_t count = has_index ? rng() % 5 : 0;
        std::vector<unsigned char> index(count * 0x94 + 1);
        for (auto& b : index) b = static_cast<unsigned char>(rng());
        std::uint32_t init[8];
        for (auto& w : init) w = rng();
        init[1] = rng() % 3 == 0 ? 0xFFFFFFFFu : init[1];
        init[2] = rng() % 2;
        init[3] = count;
        const bool has_name = fn == 1 && rng() % 4 != 0;
        std::uint32_t obj[2][8];
        int ret[2];
        std::vector<std::uint32_t> log[2];
        for (int side = 0; side < 2; ++side) {
            std::memcpy(obj[side], init, sizeof init);
            void* buf = has_index ? m_malloc(index.size()) : nullptr;
            if (buf) std::memcpy(buf, index.data(), index.size());
            obj[side][5] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(buf));
            if (fn == 1) obj[side][0] = has_name ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_malloc(16))) : 0;
            g_log.clear();
            ret[side] = (side ? port : orig)(obj[side], 0);
            log[side] = g_log;
            const std::uint32_t name = fn == 1 && has_name ? obj[side][0] : 0;
            for (std::size_t k = 0; k < log[side].size(); ++k) {  // per-side block addresses (in free records) by role
                if (buf && log[side][k] == static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(buf))) log[side][k] = 0xB0F;
                else if (name && log[side][k] == name) log[side][k] = 0xA0E;
            }
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(log[0] == log[1]);
        if (fn == 1) obj[0][0] = obj[0][0] != 0, obj[1][0] = obj[1][0] != 0;  // per-side (freed) name blocks
        for (int i = 0; i < 8; ++i) CHECK_EQ(obj[0][i], obj[1][i]);
        ++compared;
    }
    }
    for (int i = 0; i < 4; ++i) {
        *o_slot[i] = saved[i];
        *p_slot[i] = saved[4 + i];
    }
    std::printf("  ZarArchive_Close/Destruct calls %d (fake SetFilePointer/WriteFile/CloseHandle, heap index buffers)\n", compared);
}

// Reader_CloseAll (0x0048d2c0): pops every archive from the reader list [0x0056b184] (Container_ListPopCursor);
// unless ECX is 0 and it is the kept archive [0x0056b188], closes it (ZarArchive_Close: footer when dirty,
// CloseHandle), destroys it (ZarArchive_Destruct: frees its name block) and frees it (MFC operator delete, bound to
// free - platform/mfc42); with ECX 0 the kept archive is appended back. Each side builds the scenario with its own
// functions: node pool (Container_InitNodePool), list (Container_CreateList, Container_ListAppend), 0..5 archive
// objects on the msvcrt heap (name block or null, TOC or null, handle or -1, dirty flag). Same fakes as the Close
// test. Compared: the call log (blocks by role), the kept pointer, and what is left in the list (drained with the
// side's own Container_ListPopCursor, payloads by role).
TEST(native_reader_close_all_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    using Malloc = void*(__cdecl*)(std::size_t);
    const Fn close_all[2] = {rt::original<Fn>(0x0048d2c0), reinterpret_cast<Fn>(&recoil::Reader_CloseAll)};
    // then Archive_Shutdown (0x0048cd10): StringSet_Destroy([0x0056b180]), Reader_CloseAll(1),
    // Container_DestroyList([0x0056b184]), both globals zeroed
    const Fn shutdown[2] = {rt::original<Fn>(0x0048cd10), reinterpret_cast<Fn>(&recoil::Archive_Shutdown)};
    using Dup = char*(__cdecl*)(const char*);
    const auto m_strdup = reinterpret_cast<Dup>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "_strdup"));
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn append[2] = {rt::original<Fn>(0x0048ca30), reinterpret_cast<Fn>(&recoil::Container_ListAppend)};
    const Fn pop[2] = {rt::original<Fn>(0x0048cb70), reinterpret_cast<Fn>(&recoil::Container_ListPopCursor)};
    const auto m_malloc = reinterpret_cast<Malloc>(recoil::g_Iat_malloc_004cc5dc);
    void** o_slot[4] = {reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc144)),
                        reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc138)),
                        reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc0e4)),
                        reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4))};
    void** p_slot[4] = {&recoil::g_Iat_SetFilePointer_004cc144, &recoil::g_Iat_WriteFile_004cc138, &recoil::g_Iat_CloseHandle_004cc0e4,
                        &recoil::g_Iat_free_004cc5b4};
    void* const fakes[4] = {reinterpret_cast<void*>(&fake_set_file_pointer), reinterpret_cast<void*>(&fake_write_file),
                            reinterpret_cast<void*>(&fake_close_handle), reinterpret_cast<void*>(&logging_free)};
    auto g = [](int side, std::uint32_t va) {
        return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
    };
    int compared = 0;
    for (int fn = 0; fn < 2; ++fn) {
    std::mt19937 rng(fn ? 0x48cd10u : 0x48d2c0u);
    for (int it = 0; it < 500; ++it) {
        const int n = static_cast<int>(rng() % 6);
        std::vector<std::uint32_t> handle(n), dirty(n), has_name(n), count(n);
        for (int k = 0; k < n; ++k) {
            handle[k] = rng() % 4 == 0 ? 0xFFFFFFFFu : 0x100u + k;
            dirty[k] = rng() % 3 == 0;
            has_name[k] = rng() % 3 != 0;
            count[k] = dirty[k] ? rng() % 3 : 0;
        }
        const int kept = n && rng() % 2 ? static_cast<int>(rng() % n) : -1;
        const std::uint32_t ecx = rng() % 2;
        const int strings = fn ? static_cast<int>(rng() % 4) - 1 : -1;  // -1: no string set
        std::vector<std::uint32_t> log[2], rest[2];
        std::uint32_t kept_after[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            const std::uint32_t list = create[side](0, 0);
            *g(side, 0x0056b184) = list;
            std::vector<std::pair<std::uint32_t, std::uint32_t>> roles;
            std::vector<std::uint32_t> objs;
            for (int k = 0; k < n; ++k) {
                auto* o = static_cast<std::uint32_t*>(m_malloc(0x18));
                o[0] = has_name[k] ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_malloc(16))) : 0;
                o[1] = handle[k];
                o[2] = dirty[k];
                o[3] = count[k];
                o[4] = count[k];
                o[5] = count[k] ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_malloc(count[k] * 0x94))) : 0;
                if (o[5]) std::memset(reinterpret_cast<void*>(static_cast<std::uintptr_t>(o[5])), 0x44, count[k] * 0x94);
                const auto oa = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(o));
                roles.push_back({oa, 0x0B000000u + k});
                if (o[0]) roles.push_back({o[0], 0x0A000000u + k});
                if (o[5]) roles.push_back({o[5], 0x0C000000u + k});
                objs.push_back(oa);
                append[side](list, oa);
            }
            *g(side, 0x0056b188) = kept >= 0 ? objs[kept] : 0;
            roles.push_back({list, 0x0E000000u});
            *g(side, 0x0056b180) = 0;
            if (strings >= 0) {
                const std::uint32_t set = create[side](0, 0);
                roles.push_back({set, 0x0F000000u});
                for (int k = 0; k < strings; ++k) {
                    const auto str = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_strdup("dir")));
                    roles.push_back({str, 0x0D000000u + k});
                    append[side](set, str);
                }
                *g(side, 0x0056b180) = set;
            }
            auto role = [&](std::uint32_t v) {
                for (const auto& [p, r] : roles) if (v == p) return r;
                return v;
            };
            void* saved[8];
            for (int i = 0; i < 4; ++i) { saved[i] = *o_slot[i]; saved[4 + i] = *p_slot[i]; *o_slot[i] = *p_slot[i] = fakes[i]; }
            g_log.clear();
            if (fn) shutdown[side](0, 0);
            else close_all[side](ecx, 0);
            for (int i = 0; i < 4; ++i) { *o_slot[i] = saved[i]; *p_slot[i] = saved[4 + i]; }
            for (auto v : g_log) log[side].push_back(role(v));
            kept_after[side] = role(*g(side, 0x0056b188));
            if (fn) rest[side] = {*g(side, 0x0056b180), *g(side, 0x0056b184)};  // zeroed; the lists are gone
            else for (std::uint32_t p; (p = pop[side](list, 0)) != 0;) rest[side].push_back(role(p));
        }
        CHECK(log[0] == log[1]);
        CHECK_EQ(kept_after[0], kept_after[1]);
        CHECK(rest[0] == rest[1]);
        ++compared;
    }
    }
    rt::restore_pristine();
    std::printf("  Reader_CloseAll/Archive_Shutdown calls %d (reader lists of 0..5 archives, kept archive, fake KERNEL32 + logged frees)\n", compared);
}
