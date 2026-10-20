// Native L1 for Zar_ParseFooterAndTOC (0x004a63f0) and Zar_WriteEntry (0x004a64d0). Both reach the archive file
// through KERNEL32 (GetFileSize 0x004cc14c, SetFilePointer 0x004cc144, ReadFile 0x004cc134, WriteFile 0x004cc138),
// which the oracle leaves unresolved in the original's import table, so both sides get the same fakes - in the
// original's slots and the port's generated g_Iat_ slots - over an in-memory file (the handle points to it; each
// side has its own copy). Archive object: +4 handle, +8 dirty flag, +0xC entry count, +0x10 capacity, +0x14 TOC
// (0x94-byte records, grown with Zar_GrowTocBuffer through realloc - a real msvcrt block or null).
// Parse: files of 0..11 bytes (under 8: rejected), footer {magic, count} with magic 1 or not, counts that fit the
// file and counts that run past its start. Write: names and alternate names of 0..80 characters (strncpy 0x40),
// alternate name null or not, data of 0..300 bytes, onto files that already hold 0..3 entries.
// Compared: return value, the file bytes, its position, the object words (TOC pointer by null/non-null) and the TOC
// records that hold file data. Zar_WriteEntry leaves the record's flags word (+0x48) and, without an alternate name,
// its alternate name (+0x4C) and pair (+0x8C/+0x90) as stack residue (KG-29): of those only what the code defines is
// compared - flag bit 1, and the alternate name and pair when one is given.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/asset_io_misc.h"
#include "unattributed/savegame.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "GameZRecoil/zReader/zreader.h"
#include "GameZRecoil/zSound/zsnd_grp.h"
#include "unattributed/hud.h"
#include "GameZRecoil/zClass/cls_world.h"
#include "GameZRecoil/zDEClient/zdec_crater.h"
#include "GameZRecoil/zEffect/zeff_anim_save.h"
#include "cloud_harness.h"
#include "GameZRecoil/zWeapon/zwep_init.h"
#include "unattributed/weapon.h"
#include "Battlesport/mission.h"
#include "Battlesport/pickup.h"
#include "Battlesport/player.h"
#include "platform/image/original_data.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"

#include <malloc.h>

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
// Same stack contents below the call on both sides: Zar_WriteEntry reads a stack slot its callers here never write
// (flag bit 2 from it), so without this the result depends on what the previous call left there.
__declspec(noinline) void wipe_stack_below()
{
    volatile std::uint32_t buf[1024];
    for (auto& w : buf) w = 0xCDCDCDCDu;
}
struct File {
    std::vector<unsigned char> bytes;
    long pos = 0;
};
DWORD WINAPI fake_get_file_size(HANDLE h, DWORD* high)
{
    if (high) *high = 0;
    return static_cast<DWORD>(static_cast<File*>(h)->bytes.size());
}
DWORD WINAPI fake_set_file_pointer(HANDLE h, LONG dist, LONG* high, DWORD method)
{
    File* f = static_cast<File*>(h);
    if (high) *high = 0;
    const long base = method == FILE_BEGIN ? 0 : method == FILE_CURRENT ? f->pos : static_cast<long>(f->bytes.size());
    if (base + dist < 0) return INVALID_SET_FILE_POINTER;  // as the real call: fails, position unchanged
    f->pos = base + dist;
    return static_cast<DWORD>(f->pos);
}
BOOL WINAPI fake_read_file(HANDLE h, void* buf, DWORD n, DWORD* got, void*)
{
    File* f = static_cast<File*>(h);
    const long avail = f->pos < static_cast<long>(f->bytes.size()) ? static_cast<long>(f->bytes.size()) - f->pos : 0;
    const DWORD k = static_cast<DWORD>(avail) < n ? static_cast<DWORD>(avail) : n;
    if (k) std::memcpy(buf, f->bytes.data() + f->pos, k);
    f->pos += static_cast<long>(k);
    *got = k;
    return TRUE;
}
BOOL WINAPI fake_write_file(HANDLE h, const void* buf, DWORD n, DWORD* written, void*)
{
    File* f = static_cast<File*>(h);
    if (f->bytes.size() < static_cast<std::size_t>(f->pos) + n) f->bytes.resize(f->pos + n);
    if (n) std::memcpy(f->bytes.data() + f->pos, buf, n);
    f->pos += static_cast<long>(n);
    *written = n;
    return TRUE;
}

struct Slots {
    static constexpr std::uint32_t kVa[4] = {0x004cc14c, 0x004cc144, 0x004cc134, 0x004cc138};
    void** port[4] = {&recoil::g_Iat_GetFileSize_004cc14c, &recoil::g_Iat_SetFilePointer_004cc144, &recoil::g_Iat_ReadFile_004cc134,
                      &recoil::g_Iat_WriteFile_004cc138};
    void* saved[8];
    Slots()
    {
        void* fakes[4] = {reinterpret_cast<void*>(&fake_get_file_size), reinterpret_cast<void*>(&fake_set_file_pointer),
                          reinterpret_cast<void*>(&fake_read_file), reinterpret_cast<void*>(&fake_write_file)};
        for (int i = 0; i < 4; ++i) {
            void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(kVa[i]));
            saved[i] = *o;
            saved[4 + i] = *port[i];
            *o = *port[i] = fakes[i];
        }
    }
    ~Slots()
    {
        for (int i = 0; i < 4; ++i) {
            *reinterpret_cast<void**>(static_cast<std::uintptr_t>(kVa[i])) = saved[i];
            *port[i] = saved[4 + i];
        }
    }
};

using Malloc = void*(__cdecl*)(std::size_t);
using Free = void(__cdecl*)(void*);
Malloc m_malloc() { return reinterpret_cast<Malloc>(recoil::g_Iat_malloc_004cc5dc); }
Free m_free() { return reinterpret_cast<Free>(recoil::g_Iat_free_004cc5b4); }

// The object and TOC after a call, in comparable form (TOC pointer as null / non-null; KG-29 words masked).
std::vector<std::uint32_t> snapshot(const std::uint32_t* obj, const std::vector<bool>* alt_given, std::uint32_t defined)
{
    std::vector<std::uint32_t> s(obj, obj + 8);
    s[1] = 0;  // handle: each side's own file
    s[5] = obj[5] ? 1u : 0u;
    const auto* toc = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(obj[5]));
    for (std::uint32_t e = 0; toc && e < obj[3] && e < defined; ++e) {
        std::vector<std::uint32_t> rec(toc + 37 * e, toc + 37 * e + 37);
        if (alt_given && e < alt_given->size()) {
            rec[0x48 / 4] &= 2u;
            if (!(*alt_given)[e])  // alternate name (+0x4C, 0x40 bytes) and pair not written: residue
                for (int w = 0x4C / 4; w <= 0x90 / 4; ++w) rec[w] = 0;
        }
        s.insert(s.end(), rec.begin(), rec.end());
    }
    return s;
}
// Diagnostics: the first differing snapshot word (8 object words, then 37 words per TOC record).
void report(const std::vector<std::uint32_t>* s, int& shown, const char* what)
{
    if (s[0] == s[1] || shown++ >= 3) return;
    std::size_t i = 0;
    while (i < s[0].size() && i < s[1].size() && s[0][i] == s[1][i]) ++i;
    std::printf("    %s: first difference at snapshot word %zu (record %d +0x%x) of %zu/%zu\n", what, i, i < 8 ? -1 : int((i - 8) / 37),
                i < 8 ? unsigned(4 * i) : unsigned(4 * ((i - 8) % 37)), s[0].size(), s[1].size());
}
}  // namespace

TEST(native_zar_parse_footer_and_toc_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004a63f0), reinterpret_cast<Fn>(&recoil::Zar_ParseFooterAndTOC)};
    Slots slots;
    std::mt19937 rng(0x4a63f0);
    int compared = 0, shown = 0;
    for (int it = 0; it < 1000; ++it) {
        const int kind = static_cast<int>(rng() % 10);
        std::vector<unsigned char> bytes;
        if (kind == 0) {
            bytes.resize(rng() % 8);
        } else {
            const std::uint32_t count = rng() % 5;
            const std::uint32_t claimed = kind == 1 ? count + 1 + rng() % 4 : count;  // runs past the start
            bytes.resize(rng() % 20 + count * 0x94);
            for (auto& b : bytes) b = static_cast<unsigned char>(rng());
            const std::uint32_t magic = kind == 2 ? rng() % 3 + 2 : 1;
            const std::size_t o = bytes.size();
            bytes.resize(o + 8);
            std::memcpy(&bytes[o], &magic, 4);
            std::memcpy(&bytes[o + 4], &claimed, 4);
        }
        const std::uint32_t init[8] = {rng(), 0, rng() % 2, rng() % 3, rng() % 2 ? 0u : rng() % 4, 0, rng(), rng()};
        const bool has_toc = init[4] != 0;
        const long start_pos = static_cast<long>(rng() % 5);
        // TOC records hold file data only when the footer is accepted and the records lie inside the file; a rejected
        // file leaves the TOC as it was, an overlong count reads nothing into the grown buffer (heap residue)
        const bool read_ok = kind >= 3;
        int ret[2];
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            File f{bytes, start_pos};
            std::uint32_t obj[8];
            std::memcpy(obj, init, sizeof obj);
            obj[1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f));
            obj[5] = has_toc ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_malloc()(init[4] * 0x94))) : 0;
            if (has_toc) std::memset(reinterpret_cast<void*>(static_cast<std::uintptr_t>(obj[5])), 0x5A, init[4] * 0x94);
            ret[side] = fn[side](obj, 0);
            snap[side] = snapshot(obj, nullptr, read_ok ? obj[3] : 0);
            after[side] = f.bytes;
            pos[side] = f.pos;
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(obj[5])));
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "parse");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    std::printf("  Zar_ParseFooterAndTOC calls %d (fake KERNEL32 file: short files, bad magic, overlong counts)\n", compared);
}

TEST(native_zar_write_entry_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int, const char*, const void*, std::uint32_t, const char*, const std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x004a64d0), reinterpret_cast<Fn>(&recoil::Zar_WriteEntry)};
    Slots slots;
    std::mt19937 rng(0x4a64d0);
    auto name = [&](int n) {
        std::string s;
        for (int i = 0; i < n; ++i) s += static_cast<char>('a' + rng() % 26);
        return s;
    };
    int compared = 0, shown = 0;
    for (int it = 0; it < 300; ++it) {
        const int entries = 1 + static_cast<int>(rng() % 4);
        std::vector<std::string> names, alts;
        std::vector<std::vector<unsigned char>> data;
        std::vector<bool> alt_given;
        std::vector<std::uint32_t> pairs;
        for (int e = 0; e < entries; ++e) {
            names.push_back(name(static_cast<int>(rng() % 81)));
            alt_given.push_back(rng() % 2 != 0);
            alts.push_back(name(static_cast<int>(rng() % 81)));
            data.emplace_back(rng() % 301);
            for (auto& b : data.back()) b = static_cast<unsigned char>(rng());
            pairs.push_back(rng());
            pairs.push_back(rng());
        }
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        int ret[2][4];
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            File f{start, 0};
            std::uint32_t obj[8] = {0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            for (int e = 0; e < entries; ++e) {
                wipe_stack_below();
                ret[side][e] = fn[side](obj, 0, names[e].c_str(), data[e].data(), static_cast<std::uint32_t>(data[e].size()),
                                        alt_given[e] ? alts[e].c_str() : nullptr, &pairs[2 * e]);
            }
            snap[side] = snapshot(obj, &alt_given, static_cast<std::uint32_t>(entries));
            after[side] = f.bytes;
            pos[side] = f.pos;
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(obj[5])));
        }
        for (int e = 0; e < entries; ++e) CHECK_EQ(ret[0][e], ret[1][e]);
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "write");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    std::printf("  Zar_WriteEntry calls %d sequences of 1..4 entries (fake KERNEL32 file; KG-29 residue masked)\n", compared);
}

// Zar_OpenFile (0x004a61d0): CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0x10000080, 0) into
// +4; on success returns Zar_ParseFooterAndTOC; on failure GetLastError, FormatMessageA (allocating), the error
// report and LocalFree, returning 0. Besides the file fakes above: CreateFileA opens "arc0".."arc3" from a table of
// in-memory files (anything else fails), GetLastError returns 2, FormatMessageA allocates a fixed message with
// LocalAlloc, LocalFree frees it - all logged. Compared: return value, the call log (handles and the message block by
// role), the object words (handle by archive index) and the TOC records read.
namespace {
std::vector<std::uint32_t> g_log;  // CreateFileA / GetLastError / FormatMessageA / LocalFree calls
std::vector<File>* g_archives = nullptr;
HANDLE WINAPI fake_create_file(const char* name, DWORD access, DWORD share, void* sa, DWORD disp, DWORD flags, HANDLE tmpl)
{
    g_log.push_back(10);
    for (const char* c = name; *c; ++c) g_log.push_back(static_cast<unsigned char>(*c));
    g_log.push_back(access); g_log.push_back(share); g_log.push_back(sa ? 1u : 0u); g_log.push_back(disp); g_log.push_back(flags);
    g_log.push_back(tmpl ? 1u : 0u);
    if (std::strncmp(name, "arc", 3) == 0 && name[3] >= '0' && name[3] <= '3' && !name[4]) return &(*g_archives)[name[3] - '0'];
    return INVALID_HANDLE_VALUE;
}
DWORD WINAPI fake_get_last_error() { g_log.push_back(11); return 2; }
void* g_message = nullptr;
DWORD WINAPI fake_format_message(DWORD flags, const void* src, DWORD id, DWORD lang, char* buf, DWORD size, va_list* args)
{
    g_log.push_back(12); g_log.push_back(flags); g_log.push_back(src ? 1u : 0u); g_log.push_back(id); g_log.push_back(lang);
    g_log.push_back(size); g_log.push_back(args ? 1u : 0u);
    g_message = LocalAlloc(LMEM_FIXED, 32);
    std::strcpy(static_cast<char*>(g_message), "file not found");
    *reinterpret_cast<void**>(buf) = g_message;  // FORMAT_MESSAGE_ALLOCATE_BUFFER: buf receives the block
    return 14;
}
HLOCAL WINAPI fake_local_free(HLOCAL p)
{
    g_log.push_back(13);
    g_log.push_back(p == g_message ? 0xA11u : 0xBADu);
    LocalFree(p);
    return nullptr;
}
}  // namespace

TEST(native_zar_open_file_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int, const char*);
    const Fn fn[2] = {rt::original<Fn>(0x004a61d0), reinterpret_cast<Fn>(&recoil::Zar_OpenFile)};
    Slots slots;
    const std::uint32_t extra_va[4] = {0x004cc130, 0x004cc0dc, 0x004cc0f4, 0x004cc0fc};
    void** extra_port[4] = {&recoil::g_Iat_CreateFileA_004cc130, &recoil::g_Iat_GetLastError_004cc0dc, &recoil::g_Iat_FormatMessageA_004cc0f4,
                            &recoil::g_Iat_LocalFree_004cc0fc};
    void* const extra_fake[4] = {reinterpret_cast<void*>(&fake_create_file), reinterpret_cast<void*>(&fake_get_last_error),
                                 reinterpret_cast<void*>(&fake_format_message), reinterpret_cast<void*>(&fake_local_free)};
    void* saved[8];
    for (int i = 0; i < 4; ++i) {
        void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(extra_va[i]));
        saved[i] = *o;
        saved[4 + i] = *extra_port[i];
        *o = *extra_port[i] = extra_fake[i];
    }
    std::mt19937 rng(0x4a61d0);
    int compared = 0;
    for (int it = 0; it < 500; ++it) {
        // four archives: well-formed footers with 0..4 entries, or a bad magic
        std::vector<std::vector<unsigned char>> files(4);
        std::vector<bool> well_formed(4);
        for (int a = 0; a < 4; ++a) {
            const std::uint32_t count = rng() % 5, magic = rng() % 5 == 0 ? 7u : 1u;
            well_formed[a] = magic == 1;
            files[a].resize(rng() % 16 + count * 0x94);
            for (auto& b : files[a]) b = static_cast<unsigned char>(rng());
            const std::size_t o = files[a].size();
            files[a].resize(o + 8);
            std::memcpy(&files[a][o], &magic, 4);
            std::memcpy(&files[a][o + 4], &count, 4);
        }
        const int which = static_cast<int>(rng() % 6);  // 4, 5: names that do not open
        const std::string name = which < 4 ? "arc" + std::to_string(which) : which == 4 ? "missing.zar" : "arc9";
        int ret[2];
        std::vector<std::uint32_t> snap[2], log[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<File> archives;
            for (auto& f : files) archives.push_back(File{f, 0});
            g_archives = &archives;
            std::uint32_t obj[8] = {0, 0, 0, 0, 0, 0, 0x1234, 0x5678};
            g_log.clear();
            ret[side] = fn[side](obj, 0, name.c_str());
            log[side] = g_log;
            snap[side] = snapshot(obj, nullptr, which < 4 && well_formed[which] ? obj[3] : 0);
            const auto h = static_cast<std::uintptr_t>(obj[1]);
            snap[side][1] = h == 0xFFFFFFFFu ? 0xFFFFFFFFu : static_cast<std::uint32_t>((h - reinterpret_cast<std::uintptr_t>(archives.data())) / sizeof(File));
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(obj[5])));
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(log[0] == log[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<void**>(static_cast<std::uintptr_t>(extra_va[i])) = saved[i];
        *extra_port[i] = saved[4 + i];
    }
    std::printf("  Zar_OpenFile calls %d (fake CreateFileA/GetLastError/FormatMessageA/LocalFree + file fakes)\n", compared);
}

namespace {
BOOL WINAPI logging_close_handle(HANDLE h)  // the failure path: ZarArchive_Destruct -> ZarArchive_Close -> CloseHandle
{
    g_log.push_back(14);
    g_log.push_back(h == INVALID_HANDLE_VALUE ? 0xFFFFFFFFu : 1u);
    return TRUE;
}
}  // namespace

// Zar_OpenAndRegisterArchive (0x0048d210, name ECX, keep EDX): C++ EH frame (handler 0x004cb10b); operator new(0x18)
// -> ZarArchive_Construct; Zar_OpenFile(name): success -> when EDX the kept archive [0x0056b188] = it, appended to
// the reader list [0x0056b184], returns 1; failure -> ZarArchive_Destruct + operator delete, returns 0. Same fakes as
// the Zar_OpenFile test; each side its own node pool and reader list (0..2 archives already in it). Compared: return
// value, the call log, kept (none / the new archive / the previous one), and the list drained with the side's own
// Container_ListPopCursor (the new archive's words, handle by index). The normal path only: the unwind action runs
// only if a C++ exception passes the frame (Platform_OperatorNew returns NULL rather than throwing).
TEST(native_zar_open_and_register_archive_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn fn[2] = {rt::original<Fn>(0x0048d210), reinterpret_cast<Fn>(&recoil::Zar_OpenAndRegisterArchive)};
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn append[2] = {rt::original<Fn>(0x0048ca30), reinterpret_cast<Fn>(&recoil::Container_ListAppend)};
    const Fn pop[2] = {rt::original<Fn>(0x0048cb70), reinterpret_cast<Fn>(&recoil::Container_ListPopCursor)};
    Slots slots;
    const std::uint32_t extra_va[5] = {0x004cc130, 0x004cc0dc, 0x004cc0f4, 0x004cc0fc, 0x004cc0e4};
    void** extra_port[5] = {&recoil::g_Iat_CreateFileA_004cc130, &recoil::g_Iat_GetLastError_004cc0dc, &recoil::g_Iat_FormatMessageA_004cc0f4,
                            &recoil::g_Iat_LocalFree_004cc0fc, &recoil::g_Iat_CloseHandle_004cc0e4};
    void* const extra_fake[5] = {reinterpret_cast<void*>(&fake_create_file), reinterpret_cast<void*>(&fake_get_last_error),
                                 reinterpret_cast<void*>(&fake_format_message), reinterpret_cast<void*>(&fake_local_free),
                                 reinterpret_cast<void*>(&logging_close_handle)};
    void* saved[10];
    for (int i = 0; i < 5; ++i) {
        void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(extra_va[i]));
        saved[i] = *o;
        saved[5 + i] = *extra_port[i];
        *o = *extra_port[i] = extra_fake[i];
    }
    auto g = [](int side, std::uint32_t va) {
        return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
    };
    std::mt19937 rng(0x48d210);
    int compared = 0;
    for (int it = 0; it < 500; ++it) {
        std::vector<std::vector<unsigned char>> files(4);
        std::vector<bool> well_formed(4);
        for (int a = 0; a < 4; ++a) {
            const std::uint32_t count = rng() % 5, magic = rng() % 5 == 0 ? 7u : 1u;
            well_formed[a] = magic == 1;
            files[a].resize(rng() % 16 + count * 0x94);
            for (auto& b : files[a]) b = static_cast<unsigned char>(rng());
            const std::size_t o = files[a].size();
            files[a].resize(o + 8);
            std::memcpy(&files[a][o], &magic, 4);
            std::memcpy(&files[a][o + 4], &count, 4);
        }
        const int which = static_cast<int>(rng() % 6);
        const std::string name = which < 4 ? "arc" + std::to_string(which) : which == 4 ? "missing.zar" : "arc9";
        const std::uint32_t keep = rng() % 2, before = rng() % 3, kept_before = rng() % 2;
        std::uint32_t ret[2];
        std::vector<std::uint32_t> snap[2], log[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<File> archives;
            for (auto& f : files) archives.push_back(File{f, 0});
            g_archives = &archives;
            init[side](4, 0);
            const std::uint32_t list = create[side](0, 0);
            std::uint32_t old[2][8] = {{0x11}, {0x22}};
            for (std::uint32_t k = 0; k < before; ++k) append[side](list, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(old[k])));
            *g(side, 0x0056b184) = list;
            *g(side, 0x0056b188) = kept_before ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(old[0])) : 0u;
            g_log.clear();
            ret[side] = fn[side](static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(name.c_str())), keep);
            log[side] = g_log;
            snap[side].push_back(ret[side]);
            std::uint32_t added = 0;
            for (std::uint32_t p; (p = pop[side](list, 0)) != 0;) {
                if (p == static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(old[0]))) { snap[side].push_back(0xA0u); continue; }
                if (p == static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(old[1]))) { snap[side].push_back(0xA1u); continue; }
                added = p;
                const auto* o = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(p));
                const auto h = static_cast<std::uintptr_t>(o[1]);
                snap[side].push_back(0xA2u);
                snap[side].push_back(o[0] ? 1u : 0u);
                snap[side].push_back(h == 0xFFFFFFFFu ? 0xFFFFFFFFu : static_cast<std::uint32_t>((h - reinterpret_cast<std::uintptr_t>(archives.data())) / sizeof(File)));
                for (int k = 2; k < 5; ++k) snap[side].push_back(o[k]);
                snap[side].push_back(o[5] ? 1u : 0u);
            }
            const std::uint32_t kept = *g(side, 0x0056b188);
            snap[side].push_back(kept == 0 ? 0u : kept == added ? 2u : kept == static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(old[0])) ? 1u : 3u);
            if (added) {
                auto* o = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(added));
                m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(o[5])));
                m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(o[0])));
                m_free()(o);
            }
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(log[0] == log[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    for (int i = 0; i < 5; ++i) {
        *reinterpret_cast<void**>(static_cast<std::uintptr_t>(extra_va[i])) = saved[i];
        *extra_port[i] = saved[5 + i];
    }
    rt::restore_pristine();
    std::printf("  Zar_OpenAndRegisterArchive calls %d (open / fail, keep flag, reader lists of 0..2; fake KERNEL32 + file fakes)\n", compared);
}

// The EH frame of Zar_OpenAndRegisterArchive: handler 0x004cb10b (MOV EAX,FuncInfo 0x004d72b0; JMP __CxxFrameHandler)
// and its unwind action 0x004cb100 (operator delete of [EBP-0x10], the new'd archive block). No real call reaches it
// (state 0 covers only ZarArchive_Construct, which cannot throw), and the OS would not dispatch to the original's
// handler in its private mapping; so each handler is called directly, as the dispatcher would: a registration node
// {next, handler, state} in a fake frame whose [EBP-0x10] (EBP = node + 0xC) holds a real heap block, with an
// unwinding record (EXCEPTION_UNWINDING) or a search-phase record. msvcrt's __CxxFrameHandler then runs the action on
// both sides; frees are logged through the msvcrt free slot (operator delete ends there on both). Compared: the
// disposition, the free log (block by role) and the state word afterwards.
namespace {
std::vector<std::uint32_t> g_eh_frees;
std::uint32_t g_eh_block = 0;
void __cdecl eh_logging_free(void* p)
{
    const auto a = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
    g_eh_frees.push_back(a == g_eh_block ? 0xB10Cu : a);
    reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(p);
}
}  // namespace

TEST(native_eh_handler_zar_open_and_register_archive_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Handler = int(__cdecl*)(EXCEPTION_RECORD*, void*, CONTEXT*, void*);
    const Handler h[2] = {rt::original<Handler>(0x004cb10b), reinterpret_cast<Handler>(&recoil::EH_Handler_Zar_OpenAndRegisterArchive)};
    void* const saved_free = recoil::g_Iat_free_004cc5b4;
    recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&eh_logging_free);
    int compared = 0;
    for (int kind = 0; kind < 4; ++kind) {
        // kind: state 0 / -1 x unwinding (even) or search phase (odd); a state above the FuncInfo's one state makes
        // msvcrt call terminate() (its inconsistency check) - the frame never holds one
        const std::int32_t state = kind / 2 == 0 ? 0 : -1;
        const bool unwinding = kind % 2 == 0;
        int disp[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t frame[16] = {};
            g_eh_block = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_malloc()(0x18)));
            std::uint32_t* node = frame + 4;
            node[0] = 0xFFFFFFFFu;
            node[1] = side ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(h[1])) : 0x004cb10bu;
            node[2] = static_cast<std::uint32_t>(state);
            frame[3] = g_eh_block;  // [EBP - 0x10] with EBP = node + 0xC
            EXCEPTION_RECORD rec = {};
            rec.ExceptionCode = unwinding ? 0xE06D7363u : 0xC0000005u;
            rec.ExceptionFlags = unwinding ? EXCEPTION_UNWINDING : 0;
            CONTEXT ctx = {};
            g_eh_frees.clear();
            disp[side] = h[side](&rec, node, &ctx, nullptr);
            snap[side] = g_eh_frees;
            snap[side].push_back(node[2]);
            if (g_eh_frees.empty()) m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(g_eh_block)));
        }
        CHECK_EQ(disp[0], disp[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    recoil::g_Iat_free_004cc5b4 = saved_free;
    std::printf("  EH_Handler/Unwind of Zar_OpenAndRegisterArchive calls %d (unwinding / search phase, states 0 / -1)\n", compared);
}

// ZarArchive_FindEntry (0x004a65d0, archive ECX, name arg1; ret 4): case-insensitive (_stricmp) linear search of the
// TOC [+0x14] (count [+0xC], 0x94-byte entries, name at +8) - the entry, or 0. The arena fuzz gave it pointer-sized
// counts (a runaway _stricmp loop); here: TOCs of 0..8 entries from a small name set (duplicates too), queries in
// mixed case, missing names and "". Compared: the result as an entry index (or null).
TEST(native_zar_find_entry_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int, const char*);
    const Fn fn[2] = {rt::original<Fn>(0x004a65d0), reinterpret_cast<Fn>(&recoil::ZarArchive_FindEntry)};
    const char* names[] = {"tex01.pcx", "TEX02.PCX", "model.zbd", "Sound\\hit.wav", "a", "readme"};
    std::mt19937 rng(0x4a65d0);
    int compared = 0;
    for (int it = 0; it < 5000; ++it) {
        const std::uint32_t count = rng() % 9;
        std::vector<unsigned char> toc(0x94 * (count ? count : 1));
        for (auto& b : toc) b = static_cast<unsigned char>(rng());
        for (std::uint32_t e = 0; e < count; ++e) {
            char* name = reinterpret_cast<char*>(&toc[0x94 * e + 8]);
            std::memset(name, 0, 0x40);
            std::strcpy(name, names[rng() % 6]);
        }
        std::string q = rng() % 6 == 0 ? std::string(rng() % 2 ? "missing.dat" : "") : std::string(names[rng() % 6]);
        for (char& c : q) if (rng() % 3 == 0) c = static_cast<char>(std::isupper(static_cast<unsigned char>(c)) ? std::tolower(c) : std::toupper(c));
        std::uint32_t obj[8] = {0, 0, 0, count, count, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(toc.data())), 0, 0};
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) {
            const std::uint32_t r = fn[side](obj, 0, q.c_str());
            ret[side] = r ? (r - obj[5]) / 0x94 : 0xFFFFFFFFu;
        }
        CHECK_EQ(ret[0], ret[1]);
        ++compared;
    }
    std::printf("  ZarArchive_FindEntry calls %d (TOCs of 0..8 entries, mixed-case / missing queries)\n", compared);
}

// ZarArchive_SeekToEntry (0x004a6630, ECX archive, stack name, out size; ret 8): ZarArchive_FindEntry(name); none ->
// -1; else *out = entry[1] when out is set, SetFilePointer(archive +4, entry[0], 0, FILE_BEGIN) and returns +4. Same
// TOCs as above; SetFilePointer faked in both import slots and logged. Compared: return value, *out and the log.
namespace {
std::vector<std::uint32_t> g_seek_log;
DWORD WINAPI logging_set_file_pointer(HANDLE h, LONG dist, LONG* high, DWORD method)
{
    g_seek_log.push_back(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(h)));
    g_seek_log.push_back(static_cast<std::uint32_t>(dist));
    g_seek_log.push_back(high ? 1u : 0u);
    g_seek_log.push_back(method);
    return static_cast<DWORD>(dist);
}
}  // namespace

TEST(native_zar_seek_to_entry_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int, const char*, std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x004a6630), reinterpret_cast<Fn>(&recoil::ZarArchive_SeekToEntry)};
    void** o_slot = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc144));
    void* const saved[2] = {*o_slot, recoil::g_Iat_SetFilePointer_004cc144};
    *o_slot = recoil::g_Iat_SetFilePointer_004cc144 = reinterpret_cast<void*>(&logging_set_file_pointer);
    const char* names[] = {"tex01.pcx", "TEX02.PCX", "model.zbd", "Sound\\hit.wav", "a", "readme"};
    std::mt19937 rng(0x4a6630);
    int compared = 0;
    for (int it = 0; it < 5000; ++it) {
        const std::uint32_t count = rng() % 9;
        std::vector<unsigned char> toc(0x94 * (count ? count : 1));
        for (auto& b : toc) b = static_cast<unsigned char>(rng());
        for (std::uint32_t e = 0; e < count; ++e) {
            char* name = reinterpret_cast<char*>(&toc[0x94 * e + 8]);
            std::memset(name, 0, 0x40);
            std::strcpy(name, names[rng() % 6]);
        }
        const std::string q = rng() % 6 == 0 ? std::string("missing.dat") : std::string(names[rng() % 6]);
        const bool want_size = rng() % 3 != 0;
        const std::uint32_t handle = rng();
        std::uint32_t obj[8] = {0, handle, 0, count, count, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(toc.data())), 0, 0};
        std::uint32_t ret[2], out[2];
        std::vector<std::uint32_t> log[2];
        for (int side = 0; side < 2; ++side) {
            out[side] = 0xC7C7C7C7u;
            g_seek_log.clear();
            ret[side] = fn[side](obj, 0, q.c_str(), want_size ? &out[side] : nullptr);
            log[side] = g_seek_log;
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_EQ(out[0], out[1]);
        CHECK(log[0] == log[1]);
        ++compared;
    }
    *o_slot = saved[0];
    recoil::g_Iat_SetFilePointer_004cc144 = saved[1];
    std::printf("  ZarArchive_SeekToEntry calls %d (TOCs of 0..8 entries, missing names, with / without out size; logged seek)\n", compared);
}

// Zar_FindEntryInArchives (0x0048d1c0, name at ECX): no reader list [0x0056b184] -> -1; else for each archive
// (Container_Count / Container_GetNth) ZarArchive_SeekToEntry(name, 0) until one is not -1, which is returned; -1.
// Each side builds its own node pool and reader list (Container_InitNodePool / CreateList / ListAppend) of 0..4
// archives (handle, TOC of 0..5 entries), or no list. Compared: return value and the logged seeks.
TEST(native_zar_find_entry_in_archives_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn fn[2] = {rt::original<Fn>(0x0048d1c0), reinterpret_cast<Fn>(&recoil::Zar_FindEntryInArchives)};
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn append[2] = {rt::original<Fn>(0x0048ca30), reinterpret_cast<Fn>(&recoil::Container_ListAppend)};
    void** o_slot = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc144));
    void* const saved[2] = {*o_slot, recoil::g_Iat_SetFilePointer_004cc144};
    *o_slot = recoil::g_Iat_SetFilePointer_004cc144 = reinterpret_cast<void*>(&logging_set_file_pointer);
    const char* names[] = {"tex01.pcx", "TEX02.PCX", "model.zbd", "Sound\\hit.wav", "a", "readme"};
    std::mt19937 rng(0x48d1c0);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const bool no_list = rng() % 10 == 0;
        const int n = static_cast<int>(rng() % 5);
        std::vector<std::vector<unsigned char>> tocs(n);
        std::vector<std::uint32_t> counts(n), handles(n);
        for (int a = 0; a < n; ++a) {
            counts[a] = rng() % 6;
            handles[a] = 0x100u + static_cast<std::uint32_t>(a);
            tocs[a].resize(0x94 * (counts[a] ? counts[a] : 1));
            for (auto& b : tocs[a]) b = static_cast<unsigned char>(rng());
            for (std::uint32_t e = 0; e < counts[a]; ++e) {
                char* name = reinterpret_cast<char*>(&tocs[a][0x94 * e + 8]);
                std::memset(name, 0, 0x40);
                std::strcpy(name, names[rng() % 6]);
            }
        }
        const std::string q = rng() % 5 == 0 ? std::string("missing.dat") : std::string(names[rng() % 6]);
        std::uint32_t ret[2];
        std::vector<std::uint32_t> log[2];
        std::vector<std::vector<std::uint32_t>> objs(n, std::vector<std::uint32_t>(8));
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t* head = static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x0056b184) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056b184)));
            *head = 0;
            if (!no_list) {
                init[side](4, 0);
                const std::uint32_t list = create[side](0, 0);
                for (int a = 0; a < n; ++a) {
                    std::uint32_t* o = objs[a].data();
                    o[0] = 0; o[1] = handles[a]; o[2] = 0; o[3] = counts[a]; o[4] = counts[a];
                    o[5] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(tocs[a].data()));
                    append[side](list, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(o)));
                }
                *head = list;
            }
            g_seek_log.clear();
            ret[side] = fn[side](static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(q.c_str())), 0);
            log[side] = g_seek_log;
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(log[0] == log[1]);
        ++compared;
    }
    *o_slot = saved[0];
    recoil::g_Iat_SetFilePointer_004cc144 = saved[1];
    rt::restore_pristine();
    std::printf("  Zar_FindEntryInArchives calls %d (no list, reader lists of 0..4 archives with TOCs, missing names; logged seek)\n", compared);
}

// SaveGame_WriteNamedEntry (0x004c0630; ECX save object, whose archive object is at +0xC; stack: a record whose +4
// points at the first name, the second name, data, size; ret 0x10; P2.6 blocker, cloud port): the entry name is
// sprintf'd from the format at 0x004e48e8 and the two names into an 80-byte local (first byte from 0x004e5ce0, the
// rest zeroed), then Zar_WriteEntry(archive, name, data, size, no alternate name, no pair). Names of 0..30 characters
// (the 80-byte buffer holds them), data of 0..300 bytes, 1..3 entries per archive, on files already holding 0..39
// bytes. Compared as Zar_WriteEntry's test (KG-29 residue masked: no alternate name is ever given), plus the save
// object's own words. Every other entry goes through SaveGame_WriteRecord (0x004c0010; ECX context, EDX second name,
// stack: data, size; ret 8), which calls it with ECX = the context's first word (the save object) and the context
// itself as the record - so the context's +4 is the pointer to the first name.
TEST(native_save_game_write_named_entry_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t*, int, const std::uint32_t*, const char*, const void*, std::uint32_t);
    const Fn fn[2] = {rt::original<Fn>(0x004c0630), reinterpret_cast<Fn>(&recoil::SaveGame_WriteNamedEntry)};
    using Wrap = int(__fastcall*)(const std::uint32_t*, const char*, const void*, std::uint32_t);
    const Wrap wrap[2] = {rt::original<Wrap>(0x004c0010), reinterpret_cast<Wrap>(&recoil::SaveGame_WriteRecord)};
    Slots slots;
    std::mt19937 rng(0x4c0630);
    auto name = [&](int n) {
        std::string s;
        for (int i = 0; i < n; ++i) s += static_cast<char>('a' + rng() % 26);
        return s;
    };
    int compared = 0, shown = 0;
    for (int it = 0; it < 300; ++it) {
        const int entries = 1 + static_cast<int>(rng() % 3);
        std::vector<std::string> first, second;
        std::vector<std::vector<unsigned char>> data;
        for (int e = 0; e < entries; ++e) {
            first.push_back(name(static_cast<int>(rng() % 31)));
            second.push_back(name(static_cast<int>(rng() % 31)));
            data.emplace_back(rng() % 301);
            for (auto& b : data.back()) b = static_cast<unsigned char>(rng());
        }
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        const std::uint32_t head[3] = {rng(), rng(), rng()};
        const std::vector<bool> no_alt(entries, false);
        int ret[2][3];
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            File f{start, 0};
            std::uint32_t save[11] = {head[0], head[1], head[2], 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0,
                                      0x1234, 0x5678};
            for (int e = 0; e < entries; ++e) {
                const char* p = first[e].c_str();
                const std::uint32_t rec[2] = {0xEEEEEEEEu, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&p))};
                const std::uint32_t ctx[2] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(save)), rec[1]};
                wipe_stack_below();
                ret[side][e] = (e + it) % 2 ? wrap[side](ctx, second[e].c_str(), data[e].data(), static_cast<std::uint32_t>(data[e].size()))
                                            : fn[side](save, 0, rec, second[e].c_str(), data[e].data(), static_cast<std::uint32_t>(data[e].size()));
            }
            snap[side] = snapshot(save + 3, &no_alt, static_cast<std::uint32_t>(entries));
            snap[side].insert(snap[side].end(), save, save + 3);
            after[side] = f.bytes;
            pos[side] = f.pos;
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        for (int e = 0; e < entries; ++e) CHECK_EQ(ret[0][e], ret[1][e]);
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "named entry");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  SaveGame_WriteNamedEntry / SaveGame_WriteRecord calls %d sequences of 1..3 entries (fake KERNEL32 file)\n", compared);
}

// GWWorld_BuildSaveRecords (0x004517a0; ECX save context; P2.6 blocker, cloud port): for each world on list 13 (the
// holder [0x004ddf2c] -> head link, next at +8) a 0x24-byte record of World_GetParam30 / 10 / Params14 / 20 / 28 /
// Param30 again (class data +0x10..+0x30) is written with SaveGame_WriteRecord (context, the world node - its inline
// name is the entry name's second part). 0..3 worlds with short names and random class data, context prefix of 0..20
// characters, on files already holding 0..39 bytes. Compared as the named-entry test.
TEST(native_gw_world_build_save_records_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004517a0), reinterpret_cast<Fn>(&recoil::GWWorld_BuildSaveRecords)};
    Slots slots;
    std::mt19937 rng(0x4517a0);
    auto name = [&](int n) {
        std::string s;
        for (int i = 0; i < n; ++i) s += static_cast<char>('a' + rng() % 26);
        return s;
    };
    int compared = 0, shown = 0, written = 0;
    for (int it = 0; it < 300; ++it) {
        const int worlds = static_cast<int>(rng() % 4);
        const std::string prefix = name(static_cast<int>(rng() % 21));
        std::vector<std::string> names;
        std::vector<std::vector<std::uint32_t>> data(worlds, std::vector<std::uint32_t>(0xac / 4));
        for (int w = 0; w < worlds; ++w) { names.push_back(name(1 + static_cast<int>(rng() % 20))); for (auto& x : data[w]) x = rng(); }
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        const std::vector<bool> no_alt(worlds, false);
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            File f{start, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            std::vector<std::vector<std::uint32_t>> node(worlds, std::vector<std::uint32_t>(49, 0)), cd = data;
            std::uint32_t links[3][4];
            for (int w = 0; w < worlds; ++w) {
                std::strcpy(reinterpret_cast<char*>(node[w].data()), names[w].c_str());
                node[w][0x34 / 4] = 2;
                node[w][0x38 / 4] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(cd[w].data()));
                links[w][0] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(node[w].data()));
                links[w][1] = w ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(links[w - 1])) : 0u;
                links[w][2] = w + 1 < worlds ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(links[w + 1])) : 0u;
                links[w][3] = 0;
            }
            auto* holder = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(
                *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x004ddf2c) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x004ddf2c)))));
            const std::uint32_t saved_head = *holder;
            *holder = worlds ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(links[0])) : 0u;
            const char* p = prefix.c_str();
            const std::uint32_t ctx[2] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(save)),
                                          static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&p))};
            fn[side](ctx, 0);
            *holder = saved_head;
            snap[side] = snapshot(save + 3, &no_alt, static_cast<std::uint32_t>(worlds));
            after[side] = f.bytes;
            pos[side] = f.pos;
            if (side == 0) written += static_cast<int>(save[3 + 3]);
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "world save records");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  GWWorld_BuildSaveRecords calls %d, %d entries written (fake KERNEL32 file)\n", compared, written);
    CHECK(written > 300);
}

// DEClient_BuildHazardSaveRecords (0x00457b40; ECX save context; P2 declient, cloud port): a 0x34-byte header record is
// written with SaveGame_WriteRecord (name part 0x004dd244); then for each 0x34-byte hazard record in [[0x00539df4],
// [0x00539df8]) while the last write succeeded: type 1 -> sprintf(name, 0x004df65c, counter A++), type 3 ->
// sprintf(name, 0x004df668, counter B++), each written the same way; other types skipped. 0..4 records of types 0..4 on
// files already holding 0..39 bytes. Compared as the named-entry test.
TEST(native_declient_build_hazard_save_records_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00457b40), reinterpret_cast<Fn>(&recoil::DEClient_BuildHazardSaveRecords)};
    Slots slots;
    std::mt19937 rng(0x457b40);
    int compared = 0, shown = 0, written = 0;
    for (int it = 0; it < 300; ++it) {
        const int n = static_cast<int>(rng() % 5);
        std::vector<std::uint32_t> recs(13 * 4);
        for (auto& w : recs) w = rng();
        for (int k = 0; k < n; ++k) recs[13 * k] = rng() % 5;
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        const std::vector<bool> no_alt(8, false);
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            File f{start, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x00539df4) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x00539df4))) =
                static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(recs.data()));
            *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x00539df8) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x00539df8))) =
                static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(recs.data() + 13 * n));
            const char* prefix = "hz";
            const std::uint32_t ctx[2] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(save)),
                                          static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&prefix))};
            wipe_stack_below();
            fn[side](ctx, 0);
            snap[side] = snapshot(save + 3, &no_alt, save[3 + 3]);
            after[side] = f.bytes;
            pos[side] = f.pos;
            if (side == 0) written += static_cast<int>(save[3 + 3]);
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "hazard save records");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  DEClient_BuildHazardSaveRecords calls %d, %d entries written (fake KERNEL32 file)\n", compared, written);
    CHECK(written > 400);
}

// Anim_BuildNodeSaveRecords (0x00461430; ECX save context; P2 zeffect, cloud port; a 0x4480-byte frame, Crt_ChkStk):
// for the animation records 1 .. [0x00575daa) (0x134 bytes each at [0x00575dac]) while the last write succeeded: state
// +0x98 == 5, or flags +0x94 with 0x1000 but not 0x2000, or the switch [0x004df9b4] off -> skipped; else a record of
// 0x58 + 0x44 * count bytes: the name (strncpy 0x20), state and node count +0x105, then per node of the table +0x110
// (0x60 each, node at +0x24): ClsRecord_ActiveIndex, +0x24 bit 2, class-data bit 4 and either the local matrix
// (Object3D_GetLocalMatrix) or position / rotation / scale (Object3D getters); a null or non-class-5 node gets index -1.
// No table -> the name from 0x004dfad0 and zero state / count. The entry is named sprintf(0x004dfac4, index + [0x0053a2e0])
// and written with SaveGame_WriteRecord. Nodes are pool records ([0x00539c94], active flag random) with class data of
// real floats. The stack below the call is wiped first (the record has bytes the code never writes).
TEST(native_zeffect_anim_build_node_save_records_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00461430), reinterpret_cast<Fn>(&recoil::Anim_BuildNodeSaveRecords)};
    Slots slots;
    std::mt19937 rng(0x461430);
    const float vals[] = {0.0f, 1.0f, -2.5f, 10.0f, 0.5f};
    int compared = 0, shown = 0, written = 0;
    for (int it = 0; it < 300; ++it) {
        const int nrec = 1 + static_cast<int>(rng() % 4);
        std::vector<std::uint32_t> recs_init(77 * 4), pool_init(49 * 4), data_init(36 * 4);
        for (auto& w : recs_init) w = rng();
        for (auto& w : pool_init) w = rng();
        for (auto& w : data_init) w = ch::fbits(vals[rng() % 5]);
        int nodes[4], node_kind[4][3];
        for (int r = 0; r < 4; ++r) {
            std::uint32_t* rec = &recs_init[77 * r];
            std::snprintf(reinterpret_cast<char*>(rec), 16, "anim%d", r);
            reinterpret_cast<unsigned char*>(rec)[0x98] = static_cast<unsigned char>(rng() % 6 == 0 ? 5 : rng() % 4);
            rec[0x94 / 4] = rng() % 4 == 0 ? 0x1000u : rng() % 4 == 0 ? 0x3000u : 0u;
            nodes[r] = static_cast<int>(rng() % 4);
            reinterpret_cast<unsigned char*>(rec)[0x105] = static_cast<unsigned char>(nodes[r]);
            for (int k = 0; k < 3; ++k) node_kind[r][k] = static_cast<int>(rng() % 4);  // 0 null, 1 other class, 2..3 class 5
        }
        for (int k = 0; k < 4; ++k) { pool_init[49 * k + 0x34 / 4] = 5; pool_init[49 * k + 0xc0 / 4] = rng() % 2 ? 0x1000000u : 0u; }
        for (int k = 0; k < 4; ++k) data_init[36 * k] = rng() & 0x10u;
        const bool sw = rng() % 8 != 0, no_table = rng() % 6 == 0;
        const std::uint32_t base = rng() % 5;
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            File f{{}, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            std::vector<std::uint32_t> recs = recs_init, pool = pool_init, data = data_init;
            static std::uint32_t tables[4][0x60 / 4 * 3];
            for (int r = 0; r < 4; ++r) {
                for (auto& w : tables[r]) w = 0;
                for (int k = 0; k < 3; ++k) {
                    const int n = (r + k) % 4;
                    tables[r][0x60 / 4 * k + 0x24 / 4] = node_kind[r][k] == 0 ? 0u : ch::addr(&pool[49 * n]);
                }
                recs[77 * r + 0x110 / 4] = no_table ? 0u : ch::addr(tables[r]);
            }
            for (int k = 0; k < 4; ++k) {
                pool[49 * k + 0x38 / 4] = ch::addr(&data[36 * k]);
                if (node_kind[k % 4][0] == 1) pool[49 * k + 0x34 / 4] = 3;
            }
            *reinterpret_cast<std::int16_t*>(ch::img(side, 0x00575daa)) = static_cast<std::int16_t>(nrec);
            *ch::img(side, 0x00575dac) = ch::addr(recs.data());
            *ch::img(side, 0x004df9b4) = sw ? 1u : 0u;
            *ch::img(side, 0x0053a2e0) = base;
            *ch::img(side, 0x00539c94) = ch::addr(pool.data());
            const char* prefix = "an";
            const std::uint32_t ctx[2] = {ch::addr(save), ch::addr(&prefix)};
            ch::wipe_stack();
            wipe_stack_below();
            fn[side](ctx, 0);
            const std::vector<bool> no_alt(8, false);
            snap[side] = snapshot(save + 3, &no_alt, save[3 + 3]);
            after[side] = f.bytes;
            if (side == 0) written += static_cast<int>(save[3 + 3]);
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "anim node save records");
        CHECK(after[0] == after[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Anim_BuildNodeSaveRecords calls %d, %d entries written (fake KERNEL32 file)\n", compared, written);
    CHECK(written > 150);
}

namespace {
std::vector<std::string> g_create_log;
File g_created;
HANDLE WINAPI fake_create_file_log(const char* name, DWORD access, DWORD share, void* sa, DWORD disposition, DWORD flags, HANDLE tmpl)
{
    char line[256];
    std::snprintf(line, sizeof line, "%s|%lx|%lx|%p|%lx|%lx|%p", name, access, share, sa, disposition, flags, tmpl);
    g_create_log.push_back(line);
    if (std::strncmp(name, "bad", 3) == 0) return INVALID_HANDLE_VALUE;
    g_created = File{};
    return &g_created;
}
}  // namespace

// ZarWriter_Create (0x004a6270; ECX archive object, one stack argument: path; ret 4): CreateFileA(path, read | write,
// no sharing, CREATE_ALWAYS, normal) into +4; returns handle != INVALID. KERNEL32 is unresolved in the oracle, so both
// import slots get one logging fake (a path starting "bad" fails). Compared: the return, +4 (valid / invalid) and the
// CreateFileA arguments.
// ZarArchive_ReadEntry (0x004a6670; ECX archive, stack: name, buffer, size in / out; ret 0xc): ZarArchive_FindEntry;
// none -> 0x10001; *size = the entry size and larger than the old *size -> 0x10002; else SetFilePointer(offset) and
// ReadFile(size) into the buffer, 0. Archives of 0..4 entries whose data lies in the fake file. Compared: the return,
// *size, the buffer and the file position.
// SaveGame_FlushTempFileToEntry (0x004c0700; ECX save object, stack: FILE, record, second name; ret 0xc): fflush, size
// by fseek / ftell, rewind, malloc, fread, SaveGame_WriteNamedEntry(record, name, data, size), free, _rmtmp (which
// closes the tmpfile). Temp files of 0..200 bytes. Compared as the named-entry test.
TEST(native_savegame_zar_writer_and_temp_entry_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Create = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*);
    using Read = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*, unsigned char*, std::uint32_t*);
    using Flush = int(__fastcall*)(std::uint32_t*, int, void*, const std::uint32_t*, const char*);
    const Create create[2] = {rt::original<Create>(0x004a6270), reinterpret_cast<Create>(&recoil::ZarWriter_Create)};
    const Read read[2] = {rt::original<Read>(0x004a6670), reinterpret_cast<Read>(&recoil::ZarArchive_ReadEntry)};
    const Flush flush[2] = {rt::original<Flush>(0x004c0700), reinterpret_cast<Flush>(&recoil::SaveGame_FlushTempFileToEntry)};
    Slots slots;
    void** o_create = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc130));
    void* const saved[2] = {*o_create, recoil::g_Iat_CreateFileA_004cc130};
    *o_create = recoil::g_Iat_CreateFileA_004cc130 = reinterpret_cast<void*>(&fake_create_file_log);
    auto c_tmpfile = ch::crt_fn<void*(__cdecl*)()>("tmpfile");
    std::mt19937 rng(0x4a6670);
    const char* names[] = {"a.sav", "B.SAV", "world", "x"};
    int compared = 0, reads = 0;
    for (int it = 0; it < 1500; ++it) {
        const int f = it % 3;
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> file_bytes(300);
        for (auto& b : file_bytes) b = static_cast<unsigned char>(rng());
        const std::uint32_t count = rng() % 5;
        std::vector<unsigned char> toc(0x94 * 5, 0);
        for (std::uint32_t e = 0; e < count; ++e) {
            const std::uint32_t off = rng() % 200, size = rng() % 60;
            std::memcpy(&toc[0x94 * e], &off, 4);
            std::memcpy(&toc[0x94 * e + 4], &size, 4);
            std::strcpy(reinterpret_cast<char*>(&toc[0x94 * e + 8]), names[rng() % 4]);
        }
        const char* q = rng() % 5 == 0 ? "none" : names[rng() % 4];
        const std::uint32_t size_in = rng() % 80;
        const std::string path = rng() % 4 ? "save01.zar" : "bad.zar";
        std::vector<unsigned char> tmp(rng() % 201);
        for (auto& b : tmp) b = static_cast<unsigned char>(rng());
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            g_create_log.clear();
            File file{file_bytes, 0};
            std::uint32_t obj[8] = {0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&file)), 0, count, count,
                                    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(toc.data())), 0, 0};
            if (f == 0) {
                const std::uint32_t r = create[side](obj, 0, path.c_str());
                snap[side].push_back(r);
                snap[side].push_back(obj[1] == reinterpret_cast<std::uintptr_t>(INVALID_HANDLE_VALUE) ? 0xFFu : 1u);
                for (const std::string& s : g_create_log) for (char c : s) snap[side].push_back(static_cast<unsigned char>(c));
            } else if (f == 1) {
                unsigned char buf[96];
                std::memset(buf, 0xCD, sizeof buf);
                std::uint32_t size = size_in;
                const std::uint32_t r = read[side](obj, 0, q, buf, &size);
                snap[side].push_back(r);
                snap[side].push_back(size);
                snap[side].insert(snap[side].end(), buf, buf + sizeof buf);
                snap[side].push_back(static_cast<std::uint32_t>(file.pos));
                if (side == 0) reads += r == 0;
            } else {
                std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&file)), 0, 0, 0, 0, 0x1234, 0x5678};
                file = File{};
                void* t = c_tmpfile();
                if (!tmp.empty()) ch::c_fwrite(tmp.data(), 1, tmp.size(), t);
                const char* prefix = "tf";
                const std::uint32_t rec[2] = {0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&prefix))};
                flush[side](save, 0, t, rec, "entry");
                const std::vector<bool> no_alt(2, false);
                snap[side] = snapshot(save + 3, &no_alt, save[3 + 3]);
                snap[side].insert(snap[side].end(), file.bytes.begin(), file.bytes.end());
                m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    *o_create = saved[0];
    recoil::g_Iat_CreateFileA_004cc130 = saved[1];
    rt::restore_pristine();
    std::printf("  ZarWriter_Create / ZarArchive_ReadEntry / SaveGame_FlushTempFileToEntry calls %d, %d entries read\n", compared, reads);
    CHECK(reads > 50);
}

// SaveGame_CloseTempRecord (0x004c00a0; ECX FILE, EDX record, one stack argument: second name; ret 4): with an active
// save object [0x0056bf70], SaveGame_FlushTempFileToEntry(object, FILE, record, name); else nothing. Compared as above.
TEST(native_savegame_close_temp_record_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, const std::uint32_t*, const char*);
    const Fn fn[2] = {rt::original<Fn>(0x004c00a0), reinterpret_cast<Fn>(&recoil::SaveGame_CloseTempRecord)};
    Slots slots;
    auto c_tmpfile = ch::crt_fn<void*(__cdecl*)()>("tmpfile");
    auto c_rmtmp = ch::crt_fn<int(__cdecl*)()>("_rmtmp");
    std::mt19937 rng(0x4c00a0);
    for (int it = 0; it < 300; ++it) {
        const bool active = rng() % 4 != 0;
        std::vector<unsigned char> tmp(rng() % 150);
        for (auto& b : tmp) b = static_cast<unsigned char>(rng());
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            File file{};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&file)), 0, 0, 0, 0, 0x1234, 0x5678};
            *ch::img(side, 0x0056bf70) = active ? ch::addr(save) : 0u;
            void* t = c_tmpfile();
            if (!tmp.empty()) ch::c_fwrite(tmp.data(), 1, tmp.size(), t);
            const char* prefix = "cr";
            const std::uint32_t rec[2] = {0, ch::addr(&prefix)};
            fn[side](t, rec, "x");
            const std::vector<bool> no_alt(2, false);
            snap[side] = snapshot(save + 3, &no_alt, save[3 + 3]);
            snap[side].insert(snap[side].end(), file.bytes.begin(), file.bytes.end());
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
            c_rmtmp();
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// Weapon_BuildSubsystemSaveRecord (0x004b1140; ECX save context; P3 weapon, cloud port): one 4-byte record holding
// [0x00779aa0] through SaveGame_WriteRecord with the second name at 0x004e42f8. Context prefixes of 0..20 characters,
// random values, files already holding 0..39 bytes. Compared as the named-entry test.
TEST(native_weapon_build_subsystem_save_record_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004b1140), reinterpret_cast<Fn>(&recoil::Weapon_BuildSubsystemSaveRecord)};
    Slots slots;
    std::mt19937 rng(0x4b1140);
    int compared = 0, shown = 0;
    for (int it = 0; it < 200; ++it) {
        std::string prefix;
        for (int i = static_cast<int>(rng() % 21); i > 0; --i) prefix += static_cast<char>('a' + rng() % 26);
        const std::uint32_t value = rng();
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        const std::vector<bool> no_alt(1, false);
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        int ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x00779aa0) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x00779aa0))) = value;
            File f{start, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            const char* p = prefix.c_str();
            const std::uint32_t ctx[2] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(save)),
                                          static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&p))};
            wipe_stack_below();
            ret[side] = fn[side](ctx, 0);
            snap[side] = snapshot(save + 3, &no_alt, 1);
            after[side] = f.bytes;
            pos[side] = f.pos;
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "weapon save record");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Weapon_BuildSubsystemSaveRecord calls %d (fake KERNEL32 file)\n", compared);
}

namespace {
std::uint32_t ch_addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
// the stack below the caller filled with one pattern: Weapon_BuildMineSaveRecords writes 0x60-byte stack records it
// only partly fills (as cloud_harness.h's ch::wipe_stack)
__declspec(noinline) void wipe_stack_zar()
{
    volatile unsigned char* p = static_cast<volatile unsigned char*>(_alloca(0x4000));
    for (int k = 0; k < 0x4000; ++k) p[k] = 0xCD;
}
}  // namespace

// Weapon_BuildMineSaveRecords (0x0043cc70; ECX save context; P3 weapon, cloud port): a header record ('Dummy' at
// 0x004dd244 into +0x3C, first word 1) under 0x004dd234, then for the four mine weapons of the player block
// ([[0x004f3a88]+4] + 0x2B0 / 0x35C, sub-slots 0 / 0x54, +0x5F0) each projectile of the weapon's list (+0x58, link at
// +0; Weapon_ProjectileIterBegin/Next through 0x0056bcb0) as a 0x60-byte record: the weapon name (0x20), the position
// +0x44..+0x4C, the scale of node +0xC (Object3D_GetScale), the name of node +4 (gwNodeValidate, 0x24), under
// 'MineData%03d' (0x004dd224) numbered in order. Stops the weapon loop when a write returns 0. 0..3 projectiles per
// weapon, weapons missing at random, context prefixes of 0..10 characters. Compared as the named-entry test (records
// and file bytes).
TEST(native_weapon_build_mine_save_records_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0043cc70), reinterpret_cast<Fn>(&recoil::Weapon_BuildMineSaveRecords)};
    Slots slots;
    std::mt19937 rng(0x43cc70);
    static const char* const wnames[] = {"mine", "Proximity Mine", "a weapon name longer than thirty-two chars"};
    static const char* const nnames[] = {"mine_a", "", "node name of more than thirty-six characters here"};
    int compared = 0, shown = 0, written = 0;
    for (int it = 0; it < 200; ++it) {
        std::string prefix;
        for (int i = static_cast<int>(rng() % 11); i > 0; --i) prefix += static_cast<char>('a' + rng() % 26);
        struct W { bool present; int name; int projectiles; };
        W w[4];
        for (W& x : w) x = W{rng() % 4 != 0, static_cast<int>(rng() % 3), static_cast<int>(rng() % 4)};
        std::vector<std::uint32_t> pdata(4 * 3 * 0x60 / 4), ndata(4 * 3 * 0x40);
        for (auto& x : pdata) x = rng();
        for (auto& x : ndata) x = rng();
        std::vector<int> pnode(12);
        for (int& x : pnode) x = static_cast<int>(rng() % 3);
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            // projectiles (0x60 bytes: link +0, name node +4, scale node +0xC, position +0x44), their nodes (0xC4 bytes,
            // name inline, class data +0x38 with the scale at +0x24)
            std::vector<std::uint32_t> proj = pdata, nd = ndata;
            std::vector<std::vector<std::uint32_t>> nodes(12, std::vector<std::uint32_t>(0xc4 / 4, 0));
            std::vector<std::vector<std::uint32_t>> weap(4, std::vector<std::uint32_t>(0x60 / 4, 0));
            std::vector<std::uint32_t> blk(0xa00 / 4, 0);
            for (int k = 0; k < 4; ++k) {
                weap[k][0] = ch_addr(wnames[w[k].name]);
                std::uint32_t prev = 0;
                for (int p = w[k].projectiles - 1; p >= 0; --p) {
                    const int i = 3 * k + p;
                    std::uint32_t* pr = &proj[i * 0x60 / 4];
                    pr[0] = prev;
                    std::strcpy(reinterpret_cast<char*>(nodes[i].data()), nnames[pnode[i]]);
                    nodes[i][0x38 / 4] = ch_addr(&nd[i * 0x40]);
                    pr[1] = ch_addr(nodes[i].data());
                    pr[3] = ch_addr(nodes[i].data());
                    prev = ch_addr(pr);
                }
                weap[k][0x58 / 4] = prev;
                static const std::uint32_t off[4] = {0x2b0 + 0x5f0, 0x2b0 + 0x54 + 0x5f0, 0x35c + 0x5f0, 0x35c + 0x54 + 0x5f0};
                blk[off[k] / 4] = w[k].present ? ch_addr(weap[k].data()) : 0u;
            }
            std::uint32_t player[2] = {0, ch_addr(blk.data())};
            *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x004f3a88) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x004f3a88))) = ch_addr(player);
            File f{start, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            const char* p = prefix.c_str();
            const std::uint32_t ctx[2] = {ch_addr(save), ch_addr(&p)};
            wipe_stack_zar();
            fn[side](ctx, 0);
            const std::uint32_t entries = save[3 + 3];
            const std::vector<bool> no_alt(entries, false);
            snap[side] = snapshot(save + 3, &no_alt, entries);
            after[side] = f.bytes;
            pos[side] = f.pos;
            if (side == 0) written += static_cast<int>(entries);
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "mine save records");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Weapon_BuildMineSaveRecords calls %d, %d entries written (fake KERNEL32 file)\n", compared, written);
    CHECK(written > 400);
}

// Vehicle_BuildListSaveRecord (0x0041f6a0; ECX save context; P3 player, cloud port): for each vehicle of the list
// [0x004f3a7c] (next at +0, vehicle block at +4) a 0x80-byte record (size word 0x80, then the block's +0x3BC / +0x3EC
// vectors, +0xF70, +0xF84..+0xF8C, +0xFB0..+0xFE0, +0xFE8..+0xFF4, +0x60, +0xF30..+0xF38 and, for the local player
// ([0x004f3a88]), [[+8]+4]+0xA4) under the name of node +0xED0, through SaveGame_WriteRecord; stops when a write returns
// 0. 0..3 vehicles, the player among them or not, context prefixes of 0..10 characters. The record's last word is only
// written for the player (a stack leftover otherwise: the stack is wiped the same on both sides). Compared as the
// named-entry test (records and file bytes).
TEST(native_vehicle_build_list_save_record_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0041f6a0), reinterpret_cast<Fn>(&recoil::Vehicle_BuildListSaveRecord)};
    Slots slots;
    std::mt19937 rng(0x41f6a0);
    static const char* const vnames[] = {"tank", "hover", "a much longer vehicle node name"};
    int compared = 0, shown = 0, written = 0;
    for (int it = 0; it < 200; ++it) {
        std::string prefix;
        for (int i = static_cast<int>(rng() % 11); i > 0; --i) prefix += static_cast<char>('a' + rng() % 26);
        const int count = static_cast<int>(rng() % 4), player = count ? static_cast<int>(rng() % (count + 1)) : 0;
        std::vector<std::uint32_t> data(3 * 0x1000 / 4);
        for (auto& x : data) x = rng();
        std::vector<int> vn(3);
        for (int& x : vn) x = static_cast<int>(rng() % 3);
        const std::uint32_t a4 = rng();
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> blk = data;
            std::vector<std::vector<std::uint32_t>> node(3, std::vector<std::uint32_t>(0xc4 / 4, 0));
            std::uint32_t link[3][3], sub[2] = {0, 0}, inner[0xa8 / 4] = {};
            inner[0xa4 / 4] = a4;
            sub[1] = ch_addr(inner);
            for (int k = 0; k < 3; ++k) {
                std::strcpy(reinterpret_cast<char*>(node[k].data()), vnames[vn[k]]);
                blk[(k * 0x1000 + 0xed0) / 4] = ch_addr(node[k].data());
                link[k][0] = k + 1 < count ? ch_addr(link[k + 1]) : 0u;
                link[k][1] = ch_addr(&blk[k * 0x1000 / 4]);
                link[k][2] = ch_addr(sub);
            }
            auto g = [&](std::uint32_t va) {
                return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
            };
            *g(0x004f3a7c) = count ? ch_addr(link[0]) : 0u;
            *g(0x004f3a88) = player < count ? ch_addr(link[player]) : 0x1234u;
            File f{start, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            const char* p = prefix.c_str();
            const std::uint32_t ctx[2] = {ch_addr(save), ch_addr(&p)};
            wipe_stack_zar();
            fn[side](ctx, 0);
            const std::uint32_t entries = save[3 + 3];
            const std::vector<bool> no_alt(entries, false);
            snap[side] = snapshot(save + 3, &no_alt, entries);
            after[side] = f.bytes;
            pos[side] = f.pos;
            if (side == 0) written += static_cast<int>(entries);
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "vehicle list save records");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Vehicle_BuildListSaveRecord calls %d, %d entries written (fake KERNEL32 file)\n", compared, written);
    CHECK(written > 150);
}

// Pickup_BuildSaveRecords (0x0041e780; ECX save context; P3 pickup, cloud port): for each pickup of the list
// [0x004f331c] (next at +0x50) whose node (+0x24) has flag 0x40000, a 0x30-byte record ([+4]+8, +0, +8, +0xC..+0x20,
// +0x28, +0x30) under the node's name through SaveGame_WriteRecord; the loop goes on while the last value it tested is
// non-zero (a write's result, or the pickup's +0x30 word after a skipped one). 0..4 pickups, flags at random, context
// prefixes of 0..10 characters. Compared as the named-entry test (records and file bytes).
TEST(native_pickup_build_save_records_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0041e780), reinterpret_cast<Fn>(&recoil::Pickup_BuildSaveRecords)};
    Slots slots;
    std::mt19937 rng(0x41e780);
    static const char* const pnames[] = {"health", "ammo_box", "a pickup node with a long name"};
    int compared = 0, shown = 0, written = 0;
    for (int it = 0; it < 200; ++it) {
        std::string prefix;
        for (int i = static_cast<int>(rng() % 11); i > 0; --i) prefix += static_cast<char>('a' + rng() % 26);
        const int count = static_cast<int>(rng() % 5);
        std::vector<std::uint32_t> data(4 * 0x54 / 4), inner(4 * 4);
        for (auto& x : data) x = rng() % 3 ? rng() : 0;
        for (auto& x : inner) x = rng();
        std::vector<int> pn(4);
        std::vector<std::uint32_t> flag(4);
        for (int k = 0; k < 4; ++k) { pn[k] = static_cast<int>(rng() % 3); flag[k] = (rng() & ~0x40000u) | (rng() % 3 ? 0x40000u : 0u); }
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> pk = data, in = inner;
            std::vector<std::vector<std::uint32_t>> node(4, std::vector<std::uint32_t>(0xc4 / 4, 0));
            for (int k = 0; k < 4; ++k) {
                std::strcpy(reinterpret_cast<char*>(node[k].data()), pnames[pn[k]]);
                node[k][0x24 / 4] = flag[k];
                std::uint32_t* p = &pk[k * 0x54 / 4];
                p[1] = ch_addr(&in[4 * k]);
                p[0x24 / 4] = ch_addr(node[k].data());
                p[0x50 / 4] = k + 1 < count ? ch_addr(&pk[(k + 1) * 0x54 / 4]) : 0u;
            }
            *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x004f331c) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x004f331c))) =
                count ? ch_addr(pk.data()) : 0u;
            File f{start, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            const char* p = prefix.c_str();
            const std::uint32_t ctx[2] = {ch_addr(save), ch_addr(&p)};
            wipe_stack_zar();
            fn[side](ctx, 0);
            const std::uint32_t entries = save[3 + 3];
            const std::vector<bool> no_alt(entries, false);
            snap[side] = snapshot(save + 3, &no_alt, entries);
            after[side] = f.bytes;
            pos[side] = f.pos;
            if (side == 0) written += static_cast<int>(entries);
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "pickup save records");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Pickup_BuildSaveRecords calls %d, %d entries written (fake KERNEL32 file)\n", compared, written);
    CHECK(written > 150);
}

// Mission_BuildSaveRecord (0x00417430; ECX mission, stack save context; ret 4; P3 mission, cloud port): a 0x5C-byte
// record of mission words (+0xDC, +0xE0, +0x138, +0x114, +0x124, +0x108, +0x134, +0x2454..+0x2464, ten words at +0x53C
// stride 0x31C) and the game intensity ([[0x004e5d48]]) under 0x004daf9c. Mission_BuildLatePhaseFlag (0x004176b0; ECX
// save context): a 4-byte record holding 1 under 0x004dafcc. Random mission blocks and intensities, context prefixes of
// 0..10 characters. Compared as the named-entry test (records and file bytes).
TEST(native_mission_build_save_records_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Rec = int(__fastcall*)(const std::uint32_t*, int, const std::uint32_t*);
    using Flag = int(__fastcall*)(const std::uint32_t*, int);
    const Rec rec[2] = {rt::original<Rec>(0x00417430), reinterpret_cast<Rec>(&recoil::Mission_BuildSaveRecord)};
    const Flag flag[2] = {rt::original<Flag>(0x004176b0), reinterpret_cast<Flag>(&recoil::Mission_BuildLatePhaseFlag)};
    Slots slots;
    std::mt19937 rng(0x417430);
    int compared = 0, shown = 0;
    for (int it = 0; it < 200; ++it) {
        std::string prefix;
        for (int i = static_cast<int>(rng() % 11); i > 0; --i) prefix += static_cast<char>('a' + rng() % 26);
        std::vector<std::uint32_t> mission(0x2480 / 4);
        for (auto& x : mission) x = rng();
        const std::int32_t intensity = static_cast<std::int32_t>(rng() % 4);
        const int order = static_cast<int>(rng() % 3);  // record, flag, or both
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        long pos[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::int32_t level = intensity;
            *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x004e5d48) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x004e5d48))) = ch_addr(&level);
            File f{start, 0};
            std::uint32_t save[11] = {0, 0, 0, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            const char* p = prefix.c_str();
            const std::uint32_t ctx[2] = {ch_addr(save), ch_addr(&p)};
            wipe_stack_zar();
            if (order != 1) rec[side](mission.data(), 0, ctx);
            if (order != 0) flag[side](ctx, 0);
            const std::uint32_t entries = save[3 + 3];
            const std::vector<bool> no_alt(entries, false);
            snap[side] = snapshot(save + 3, &no_alt, entries);
            after[side] = f.bytes;
            pos[side] = f.pos;
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "mission save records");
        CHECK(after[0] == after[1]);
        CHECK_EQ(pos[0], pos[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Mission_BuildSaveRecord / Mission_BuildLatePhaseFlag calls %d (fake KERNEL32 file)\n", compared);
}

// SoundFile_LoadFromArchive (0x004a5600; ECX sound-file record, one stack argument: the archive object; ret 4) and
// SoundFile_LoadFromDisk (0x004a5540; ECX record): 1 when already loaded ([+0]); else the entry named [+4] is sized and
// read (ZarArchive_ReadEntry twice, over the fake KERNEL32 file of this file) or the file at path [+4] is opened
// (fopen, File_GetSize, fread, fclose - real msvcrt, a temp file), into calloc(size, 1) [+0xc], size [+8], and parsed
// (Wav_ParseRiffChunks) into [+0]. RIFF/WAVE images with fmt / data / cue chunks, a missing name / path, an empty
// entry. Compared: the return, the record (pointers into the data as offsets), the data bytes.
TEST(native_sound_file_load_from_archive_and_disk_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Load = std::uint32_t(__fastcall*)(std::uint32_t*, int, std::uint32_t*);
    const Load from_zar[2] = {rt::original<Load>(0x004a5600), reinterpret_cast<Load>(&recoil::SoundFile_LoadFromArchive)};
    const Load from_disk[2] = {rt::original<Load>(0x004a5540), reinterpret_cast<Load>(&recoil::SoundFile_LoadFromDisk)};
    Slots slots;
    std::mt19937 rng(0x4a5600);
    const std::string path = ch::temp_path("snd");
    const char* names[] = {"boom.wav", "shot.wav", "x"};
    int loaded = 0;
    for (int it = 0; it < 600; ++it) {
        const bool disk = it % 2 != 0;
        // a RIFF image: fmt / data / cue chunks with odd sizes, sometimes a bad tag
        std::vector<unsigned char> wav(12);
        auto put = [&wav](std::size_t at, std::uint32_t v) { std::memcpy(&wav[at], &v, 4); };
        const std::uint32_t tags[3] = {0x20746d66, 0x61746164, 0x20657563};
        for (std::uint32_t k = 0, n = rng() % 5; k < n; ++k) {
            const std::uint32_t size = rng() % 40, at = static_cast<std::uint32_t>(wav.size());
            wav.resize(at + 8 + ((size + 1) & ~1u));
            for (std::size_t b = at + 8; b < wav.size(); ++b) wav[b] = static_cast<unsigned char>(rng());
            put(at, tags[rng() % 3]);
            put(at + 4, size);
        }
        put(0, rng() % 8 ? 0x46464952u : 0x58464952u);
        put(4, static_cast<std::uint32_t>(wav.size()) - 8);
        put(8, 0x45564157u);
        if (!disk && rng() % 10 == 0) wav.clear();  // an empty entry (a 0-byte file would be parsed past its end)
        // the archive: the image at an offset in the fake file, one or two entries
        std::vector<unsigned char> file_bytes(40 + wav.size());
        for (auto& b : file_bytes) b = static_cast<unsigned char>(rng());
        if (!wav.empty()) std::memcpy(&file_bytes[40], wav.data(), wav.size());
        const std::uint32_t count = 1 + rng() % 2;
        std::vector<unsigned char> toc(0x94 * 2, 0);
        const std::uint32_t off = 40, size = static_cast<std::uint32_t>(wav.size());
        std::memcpy(&toc[0], &off, 4);
        std::memcpy(&toc[4], &size, 4);
        std::strcpy(reinterpret_cast<char*>(&toc[8]), names[0]);
        std::strcpy(reinterpret_cast<char*>(&toc[0x94 + 8]), names[1]);
        const char* want = names[rng() % 3];
        if (disk) ch::write_file(path, std::string(wav.begin(), wav.end()));
        const std::uint32_t loaded_flag = rng() % 6 == 0 ? 1u : 0u;
        const int no_name = rng() % 8 == 0, missing = rng() % 6 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            File file{file_bytes, 0};
            std::uint32_t obj[8] = {0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&file)), 0, count, count,
                                    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(toc.data())), 0, 0};
            const char* nm = no_name ? nullptr : disk ? (missing ? "no_such_file.wav" : path.c_str()) : want;
            std::uint32_t rec[9] = {loaded_flag, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(nm)), 0, 0, 0, 0, 0, 0, 0};
            ch::wipe_stack();
            const std::uint32_t r = disk ? from_disk[side](rec, 0, nullptr) : from_zar[side](rec, 0, obj);
            snap[side].push_back(r);
            const std::uint32_t data = rec[3], n = rec[2];
            for (int k = 0; k < 9; ++k) {
                if (k == 1) continue;
                const std::uint32_t v = rec[k];
                snap[side].push_back(k >= 3 && data && v >= data && v <= data + n + 8 ? 0xD0000000u + (v - data) : v);
            }
            if (data) {
                const unsigned char* b = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(data));
                snap[side].insert(snap[side].end(), b, b + n);
                ch::c_free(data);
                if (side == 0) ++loaded;
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    DeleteFileA(path.c_str());
    rt::restore_pristine();
    std::printf("  SoundFile_LoadFromArchive / SoundFile_LoadFromDisk: %d loads\n", loaded);
    CHECK(loaded > 100);
}

// NamedRecord_Construct (0x004a53f0; ECX record, stack: name, load flag; ret 8): _strdup(name) into [+4], the other
// eight words zeroed, SoundFile_LoadFromDisk when the flag is set (the name a temp RIFF file, or a missing one).
// Compared: the return (the record), the record words (the copy by content, data pointers as offsets), the data.
TEST(native_sound_named_record_construct_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Ctor = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*, std::uint32_t);
    const Ctor ctor[2] = {rt::original<Ctor>(0x004a53f0), reinterpret_cast<Ctor>(&recoil::NamedRecord_Construct)};
    std::mt19937 rng(0x4a53f0);
    const std::string path = ch::temp_path("nrc");
    for (int it = 0; it < 200; ++it) {
        std::string wav = "RIFF....WAVEfmt ";
        const std::uint32_t fmt = 16, data = rng() % 30;
        wav.append(reinterpret_cast<const char*>(&fmt), 4).append(16, '\x01').append("data").append(reinterpret_cast<const char*>(&data), 4);
        for (std::uint32_t k = 0; k < data; ++k) wav.push_back(static_cast<char>(rng()));
        const std::uint32_t riff = static_cast<std::uint32_t>(wav.size()) - 8;
        std::memcpy(&wav[4], &riff, 4);
        ch::write_file(path, wav);
        const std::uint32_t flag = rng() % 3 ? 1u : 0u;
        const std::string name = rng() % 5 ? path : std::string("no_such_file.wav");
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t rec[9];
            for (auto& x : rec) x = 0xCDCDCDCD;
            const std::uint32_t r = ctor[side](rec, 0, name.c_str(), flag);
            snap[side].push_back(r == static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(rec)) ? 1u : r);
            const char* copy = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(rec[1]));
            snap[side].push_back(copy && name == copy ? 1u : 0u);
            const std::uint32_t d = rec[3], n = rec[2];
            for (int k = 0; k < 9; ++k)
                if (k != 1) snap[side].push_back(k >= 3 && d && rec[k] >= d && rec[k] <= d + n + 8 ? 0xD0000000u + (rec[k] - d) : rec[k]);
            if (d) {
                const unsigned char* b = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(d));
                snap[side].insert(snap[side].end(), b, b + n);
                ch::c_free(d);
            }
            if (rec[1]) ch::c_free(rec[1]);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    DeleteFileA(path.c_str());
}

// HUD_BuildTimerSaveRecord (0x0040fb90; hud, ported in the cloud; ECX save context, EDX the HUD timer object): writes
// the timer float [+0x2a4] as a 4-byte named entry (second name 0x004dad20) through SaveGame_WriteRecord - so through
// SaveGame_WriteNamedEntry and Zar_WriteEntry onto the fake KERNEL32 file. First names of 0..30 characters, 1..3
// records per archive. Compared as the named-entry test.
TEST(native_hud_timer_save_record_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, const std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x0040fb90), reinterpret_cast<Fn>(&recoil::HUD_BuildTimerSaveRecord)};
    Slots slots;
    std::mt19937 rng(0x40fb90);
    int compared = 0, shown = 0;
    for (int it = 0; it < 200; ++it) {
        const int entries = 1 + static_cast<int>(rng() % 3);
        std::vector<std::string> first;
        std::vector<float> times;
        for (int e = 0; e < entries; ++e) {
            std::string nm;
            for (int i = 0, n = static_cast<int>(rng() % 31); i < n; ++i) nm += static_cast<char>('a' + rng() % 26);
            first.push_back(nm);
            times.push_back(static_cast<float>(rng() % 100000) / 7.0f);
        }
        std::vector<unsigned char> start(rng() % 40);
        for (auto& b : start) b = static_cast<unsigned char>(rng());
        const std::vector<bool> no_alt(entries, false);
        int ret[2][3];
        std::vector<std::uint32_t> snap[2];
        std::vector<unsigned char> after[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            File f{start, 0};
            std::uint32_t save[11] = {1, 2, 3, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&f)), 0, 0, 0, 0, 0x1234, 0x5678};
            for (int e = 0; e < entries; ++e) {
                const char* p = first[e].c_str();
                const std::uint32_t ctx[2] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(save)),
                                              static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&p))};
                std::uint32_t hud[0x2a8 / 4] = {};
                std::memcpy(&hud[0x2a4 / 4], &times[e], 4);
                wipe_stack_below();
                ret[side][e] = fn[side](ctx, hud);
            }
            snap[side] = snapshot(save + 3, &no_alt, static_cast<std::uint32_t>(entries));
            snap[side].insert(snap[side].end(), save, save + 3);
            after[side] = f.bytes;
            m_free()(reinterpret_cast<void*>(static_cast<std::uintptr_t>(save[3 + 5])));
        }
        for (int e = 0; e < entries; ++e) CHECK_EQ(ret[0][e], ret[1][e]);
        CHECK_SNAP(snap[0], snap[1]);
        report(snap, shown, "timer record");
        CHECK(after[0] == after[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  HUD_BuildTimerSaveRecord calls %d sequences of 1..3 records (fake KERNEL32 file)\n", compared);
}
