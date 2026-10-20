// Structured native L1 for the texture functions the arena fuzz cannot drive fairly (they free or walk arena words):
// TextureArchive_CloseAllFiles (0x0046d730): for each of [0x0053d768] 0xA4-byte records at [0x0053d76c], fclose the
//   FILE* at +0x80 when set and null it.
// TextureArchiveListB_FreeAll (0x0046d780): for each of [0x0053d770] records at [0x0053d774], fclose +0x80 and free
//   +0x9C when set (nulling them); then free the array and zero both globals.
//   Both: fclose faked in both msvcrt slots (0x004cc5c0) - FILE* values are tokens, logged; free logged by role.
// Texture_ResampleSquare (0x0046e9b0, image ECX, size EDX): nearest-neighbour resample of the 16-bit pixels [+0x10]
//   (width short +4, height short +6) and, when set, the alpha bytes [+0x14] to size x size; frees the old buffers,
//   installs the new ones, sets both sizes. Real images (sizes 1..48, random pixels, alpha or not), sizes 1..64.
// TextureArchiveListA_FreeAll (0x0046d6b0): the list A teardown - TextureArchive_CloseAllFiles, then free each
//   record's +0x9C, free the array [0x0053d76c], zero both globals. Same test as ListB.
// Image_ReadHeader (0x0046ed70, FILE ECX, image EDX): null either -> -1; fread 16 header bytes and apply them
//   (Image_SetSize, Image_SetFlags, Image_SetByte8, words +0xE / +0xC); 0. Real temp files with full headers (a short
//   read would leave stack residue, as KG-28).
// TextureManager_AddSearchPaths (0x0046ebd0, path list ECX): the set [0x0053d794] is created by
//   StringSet_CreateAndAddTokens on the first call, appended to by StringSet_AddSemicolonTokens afterwards. Sequences
//   of 1..4 calls on each side's own node pool; the set is drained with the side's own Container_ListPopCursor.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zImage/zimg_fonts.h"
#include "GameZRecoil/zReader/zreader.h"
#include "unattributed/texture.h"
#include "unattributed/render_frame.h"
#include "unattributed/settings.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"
#include "platform/iat_kernel32.h"
#include "watchdog.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t* img(int side, std::uint32_t va) { return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : ptr<void>(va)); }
void* real_malloc(std::size_t n) { return reinterpret_cast<void*(__cdecl*)(std::size_t)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"))(n); }
void real_free(void* p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(p); }

std::vector<std::uint32_t> g_log, g_roles;
std::uint32_t role(std::uint32_t p)
{
    for (std::size_t k = 0; k < g_roles.size(); ++k) if (p && p == g_roles[k]) return 0xA0000000u + static_cast<std::uint32_t>(k);
    return p;
}
void __cdecl logging_free(void* p)
{
    g_log.push_back(1);
    g_log.push_back(role(addr(p)));
    for (auto& r : g_roles) if (r == addr(p)) r = 0;  // freed: never matched again (the address may be reused)
    real_free(p);
}
int __cdecl fake_fclose(void* f)
{
    g_log.push_back(2);
    g_log.push_back(addr(f));
    return 0;
}
struct Hooks {  // free and fclose, both import tables
    void* s[4];
    Hooks()
    {
        s[0] = *ptr<void*>(0x004cc5b4); s[1] = recoil::g_Iat_free_004cc5b4;
        s[2] = *ptr<void*>(0x004cc5c0); s[3] = recoil::g_Iat_fclose_004cc5c0;
        *ptr<void*>(0x004cc5b4) = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
        *ptr<void*>(0x004cc5c0) = recoil::g_Iat_fclose_004cc5c0 = reinterpret_cast<void*>(&fake_fclose);
    }
    ~Hooks()
    {
        *ptr<void*>(0x004cc5b4) = s[0]; recoil::g_Iat_free_004cc5b4 = s[1];
        *ptr<void*>(0x004cc5c0) = s[2]; recoil::g_Iat_fclose_004cc5c0 = s[3];
    }
};
}  // namespace

TEST(native_texture_archive_teardown_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(int, int);
    Hooks hooks;
    int compared = 0;
    for (int which = 0; which < 3; ++which) {  // 0 CloseAllFiles, 1 ListB_FreeAll, 2 ListA_FreeAll (list A: CloseAllFiles first)
        const std::uint32_t va = which == 0 ? 0x0046d730u : which == 1 ? 0x0046d780u : 0x0046d6b0u;
        const Fn fn[2] = {rt::original<Fn>(va), reinterpret_cast<Fn>(which == 0 ? &recoil::TextureArchive_CloseAllFiles
                                                                   : which == 1 ? &recoil::TextureArchiveListB_FreeAll : &recoil::TextureArchiveListA_FreeAll)};
        const std::uint32_t g_count = which == 1 ? 0x0053d770u : 0x0053d768u, g_arr = which == 1 ? 0x0053d774u : 0x0053d76cu;
        std::mt19937 rng(va);
        for (int it = 0; it < 2000; ++it) {
            const int n = static_cast<int>(rng() % 5);
            const bool null_arr = which != 0 && n == 0 && rng() % 2;
            std::vector<std::uint32_t> words(41 * 5);
            for (auto& w : words) w = rng() | 0x80000000u;
            std::vector<std::uint32_t> file(5), buf(5);
            for (int k = 0; k < 5; ++k) { file[k] = rng() % 3 ? 0x7000u + static_cast<std::uint32_t>(k) : 0u; buf[k] = rng() % 3; }
            std::vector<std::uint32_t> snap[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                g_roles.clear();
                auto* arr = static_cast<std::uint32_t*>(real_malloc(0xA4 * 5));
                g_roles.push_back(addr(arr));
                for (int k = 0; k < 5; ++k) {
                    std::memcpy(arr + 41 * k, &words[41 * k], 0xA4);
                    arr[41 * k + 0x80 / 4] = file[k];
                    arr[41 * k + 0x9C / 4] = buf[k] ? addr(real_malloc(8)) : 0u;
                    g_roles.push_back(arr[41 * k + 0x9C / 4]);
                }
                *img(side, g_count) = static_cast<std::uint32_t>(n);
                *img(side, g_arr) = null_arr ? 0u : addr(arr);
                g_log.clear();
                fn[side](0, 0);
                snap[side] = g_log;
                snap[side].push_back(*img(side, g_count));
                snap[side].push_back(role(*img(side, g_arr)));
                const bool arr_freed = g_roles[0] == 0;
                if (!arr_freed) {
                    for (int k = 0; k < 5; ++k)
                        for (int w = 0; w < 41; ++w) snap[side].push_back(w == 0x9C / 4 ? role(arr[41 * k + w]) : arr[41 * k + w]);
                }
                for (std::size_t k = 1; k < g_roles.size(); ++k) if (g_roles[k]) real_free(ptr<void>(g_roles[k]));
                if (!arr_freed) real_free(arr);
            }
            CHECK_SNAP(snap[0], snap[1]);
            ++compared;
        }
    }
    rt::restore_pristine();
    std::printf("  TextureArchive_CloseAllFiles / ListB_FreeAll / ListA_FreeAll calls %d (0..4 records, FILE* tokens, owned buffers)\n", compared);
}

TEST(native_texture_resample_square_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046e9b0), reinterpret_cast<Fn>(&recoil::Texture_ResampleSquare)};
    Hooks hooks;
    std::mt19937 rng(0x46e9b0);
    int compared = 0;
    for (int it = 0; it < 1500; ++it) {
        const int w = 1 + static_cast<int>(rng() % 48), h = 1 + static_cast<int>(rng() % 48);
        const int sizes[] = {1, 2, 4, 8, 16, 32, 64, 3, 7, 13};
        const int size = sizes[rng() % 10];
        const bool alpha = rng() % 2;
        std::vector<std::uint16_t> px(static_cast<std::size_t>(w * h));
        std::vector<unsigned char> al(static_cast<std::size_t>(w * h));
        for (auto& p : px) p = static_cast<std::uint16_t>(rng());
        for (auto& a : al) a = static_cast<unsigned char>(rng());
        std::uint32_t init[16];
        for (auto& x : init) x = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t image[16];
            std::memcpy(image, init, sizeof image);
            reinterpret_cast<std::int16_t*>(image)[2] = static_cast<std::int16_t>(w);
            reinterpret_cast<std::int16_t*>(image)[3] = static_cast<std::int16_t>(h);
            auto* p = static_cast<std::uint16_t*>(real_malloc(px.size() * 2));
            std::memcpy(p, px.data(), px.size() * 2);
            unsigned char* a = nullptr;
            if (alpha) { a = static_cast<unsigned char*>(real_malloc(al.size())); std::memcpy(a, al.data(), al.size()); }
            image[4] = addr(p);
            image[5] = addr(a);
            g_roles = {addr(p), addr(a)};
            g_log.clear();
            fn[side](image, size);
            snap[side] = g_log;
            for (int k = 0; k < 16; ++k) if (k != 4 && k != 5) snap[side].push_back(image[k]);
            const auto* np = ptr<std::uint16_t>(image[4]);
            for (int k = 0; k < size * size; ++k) snap[side].push_back(np[k]);
            if (image[5]) {
                const auto* na = ptr<unsigned char>(image[5]);
                for (int k = 0; k < size * size; ++k) snap[side].push_back(na[k]);
            } else snap[side].push_back(0xFFFFFFFFu);
            real_free(ptr<void>(image[4]));
            if (image[5]) real_free(ptr<void>(image[5]));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  Texture_ResampleSquare calls %d (images 1..48 square or not, alpha or not, sizes 1..64)\n", compared);
}

TEST(native_texture_manager_add_search_paths_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn fn[2] = {rt::original<Fn>(0x0046ebd0), reinterpret_cast<Fn>(&recoil::TextureManager_AddSearchPaths)};
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn pop[2] = {rt::original<Fn>(0x0048cb70), reinterpret_cast<Fn>(&recoil::Container_ListPopCursor)};
    const char* lists[] = {"..\\data\\common\\image", "a;b;c", ";;x;", "", "dir1;dir2", "one", "p;q;r;s;t;u"};
    std::mt19937 rng(0x46ebd0);
    int compared = 0;
    for (int it = 0; it < 1000; ++it) {
        const int calls = 1 + static_cast<int>(rng() % 4);
        std::vector<std::string> args;
        for (int c = 0; c < calls; ++c) args.push_back(lists[rng() % 7]);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            *img(side, 0x0053d794) = 0;
            for (const auto& a : args) snap[side].push_back(fn[side](addr(a.c_str()), 0));
            const std::uint32_t set = *img(side, 0x0053d794);
            snap[side].push_back(set ? 1u : 0u);
            for (std::uint32_t s; set && (s = pop[side](set, 0)) != 0;) {
                for (const char* c = ptr<char>(s); *c; ++c) snap[side].push_back(static_cast<unsigned char>(*c));
                snap[side].push_back(0xFFFFFFFFu);
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  TextureManager_AddSearchPaths sequences %d (1..4 path lists, set drained per side)\n", compared);
}

// Tex_CallWithDefaultRecord (0x0046e960, three words at ECX {a, b, c}): builds {0, 0, 0, a, b, c, 0, 1.0f} and
// returns ShadeEntry_FindExact of it - the index of the first entry of the shade table ([0x0053d780] count,
// [0x0053d784] 0x20-byte entries) whose eight floats all equal the record's, else -1. Arena floats never match an
// entry, so here: real tables of 0..6 entries holding exact copies of the record at random places and near misses
// (one field changed). Compared: the return value.
TEST(native_tex_call_with_default_record_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const float*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046e960), reinterpret_cast<Fn>(&recoil::Tex_CallWithDefaultRecord)};
    std::mt19937 rng(0x46e960);
    auto fr = [&] { return static_cast<float>(static_cast<int>(rng() % 9) - 4) * 0.25f; };
    int compared = 0, found = 0;
    for (int it = 0; it < 5000; ++it) {
        const float abc[3] = {fr(), fr(), fr()};
        const float rec[8] = {0, 0, 0, abc[0], abc[1], abc[2], 0, 1.0f};
        const int n = static_cast<int>(rng() % 7);
        std::vector<float> table(8 * 7);
        for (int e = 0; e < n; ++e) {
            for (int k = 0; k < 8; ++k) table[8 * e + k] = rec[k];
            if (rng() % 3) table[8 * e + rng() % 8] += 0.5f;  // a near miss in one field
        }
        int ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *img(side, 0x0053d780) = static_cast<std::uint32_t>(n);
            *img(side, 0x0053d784) = addr(table.data());
            ret[side] = fn[side](abc, 0);
        }
        CHECK_EQ(ret[0], ret[1]);
        found += ret[0] >= 0;
        ++compared;
    }
    rt::restore_pristine();
    CHECK(found > 500);
    std::printf("  Tex_CallWithDefaultRecord calls %d, %d found (shade tables of 0..6 entries, exact copies and near misses)\n", compared, found);
}

// TextureManager_FindSlotByName (0x0046d4d0, name ECX): the first of [0x0053d798] 0x24-byte slots (table 0x0053d79c)
// with a non-zero state [+0x1C] whose name [+8] equals the string (strcmp), as a pointer into the table, else 0.
// Real tables of 0..12 slots, names and states from a pool, queries from the pool and misses. Compared: the result as
// a slot index. TextureManager_FreeAllSlots (0x0046d5d0): per slot, state 1 -> Image_FreeUnlessDefault(+0) and the
// zVideo binding release hook [0x0056bc1c](+4), both nulled, then state 0; state 2 -> state 0; count 0; hook
// [0x0056bc20]; frees the palette-name list ([0x0053d778] count, [0x0053d77c] array) and the shade table
// [0x0053d784] (count [0x0053d780] zeroed); then TextureArchiveListA_FreeAll and ListB_FreeAll. Real slots with real
// images (or null / the default image), binding tokens, name strings, shade table and both archive lists (0..2
// records); frees, fclose and the three zVideo hooks logged. Compared: the log, the globals and every slot word.
namespace {
void __fastcall hook_release(int ecx, int) { g_log.push_back(3); g_log.push_back(static_cast<std::uint32_t>(ecx)); }
void __fastcall hook_after(int, int) { g_log.push_back(4); }
void __fastcall hook_surface(int ecx, int) { g_log.push_back(5); g_log.push_back(role(static_cast<std::uint32_t>(ecx))); }
}  // namespace

TEST(native_texture_manager_find_slot_by_name_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const char*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046d4d0), reinterpret_cast<Fn>(&recoil::TextureManager_FindSlotByName)};
    const char* names[] = {"sky.pcx", "SKY.PCX", "ground", "grnd", "", "tank01", "tank01x"};
    std::mt19937 rng(0x46d4d0);
    int compared = 0, found = 0;
    for (int it = 0; it < 4000; ++it) {
        const int n = static_cast<int>(rng() % 13);
        int nm[13];
        std::uint32_t st[13];
        for (int s = 0; s < 13; ++s) { nm[s] = static_cast<int>(rng() % 7); st[s] = rng() % 3 ? 1u + rng() % 3 : 0u; }
        const std::string q = rng() % 6 == 0 ? std::string("missing") : std::string(names[rng() % 7]);
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            auto* table = reinterpret_cast<unsigned char*>(img(side, 0x0053d79c));
            for (int s = 0; s < 13; ++s) {
                std::memset(table + 0x24 * s + 8, 0, 0x14);
                std::strcpy(reinterpret_cast<char*>(table + 0x24 * s + 8), names[nm[s]]);
                std::memcpy(table + 0x24 * s + 0x1C, &st[s], 4);
            }
            *img(side, 0x0053d798) = static_cast<std::uint32_t>(n);
            const std::uint32_t r = fn[side](q.c_str(), 0);
            ret[side] = r ? (r - addr(table)) / 0x24 : 0xFFFFFFFFu;
        }
        CHECK_EQ(ret[0], ret[1]);
        found += ret[0] != 0xFFFFFFFFu;
        ++compared;
    }
    rt::restore_pristine();
    CHECK(found > 1000);
    std::printf("  TextureManager_FindSlotByName calls %d, %d found (slot tables of 0..12)\n", compared, found);
}

TEST(native_texture_manager_free_all_slots_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(int, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046d5d0), reinterpret_cast<Fn>(&recoil::TextureManager_FreeAllSlots)};
    Hooks hooks;
    std::mt19937 rng(0x46d5d0);
    int compared = 0;
    for (int it = 0; it < 1500; ++it) {
        const int n = static_cast<int>(rng() % 7), names = static_cast<int>(rng() % 4), recsA = static_cast<int>(rng() % 3), recsB = static_cast<int>(rng() % 3);
        std::uint32_t st[7], ik[7], bind[7], w[7][9];
        for (int s = 0; s < 7; ++s) {
            st[s] = rng() % 5;  // 0 empty, 1 loaded, 2 pending, 3 reload, 4 other
            ik[s] = rng() % 4;  // 0 null image, 1 the default image, else a real one
            bind[s] = rng() % 3 ? 0x5000u + static_cast<std::uint32_t>(s) : 0u;
            for (auto& x : w[s]) x = rng() | 0x80000000u;
        }
        const bool shade = rng() % 2;
        std::uint32_t recbuf[2][2];  // drawn once: both sides build the same lists
        for (auto& l : recbuf) for (auto& r : l) r = rng() % 2;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc1c))) = reinterpret_cast<void*>(&hook_release);
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc20))) = reinterpret_cast<void*>(&hook_after);
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc28))) = reinterpret_cast<void*>(&hook_surface);
            const std::uint32_t def = addr(img(side, 0x004e06e0));
            g_roles.clear();
            auto own = [&](std::size_t bytes) { const std::uint32_t b = addr(real_malloc(bytes)); std::memset(ptr<void>(b), 0, bytes); g_roles.push_back(b); return b; };
            auto* table = img(side, 0x0053d79c);
            for (int s = 0; s < 7; ++s) {
                std::uint32_t* sl = table + 9 * s;
                for (int k = 0; k < 9; ++k) sl[k] = w[s][k];
                sl[7] = st[s];
                sl[0] = ik[s] == 0 ? 0u : ik[s] == 1 ? def : own(0x40);
                sl[1] = bind[s];
            }
            *img(side, 0x0053d798) = static_cast<std::uint32_t>(n);
            const std::uint32_t name_arr = names ? own(4 * names) : 0u;
            for (int k = 0; k < names; ++k) ptr<std::uint32_t>(name_arr)[k] = own(8);
            *img(side, 0x0053d778) = static_cast<std::uint32_t>(names);
            *img(side, 0x0053d77c) = name_arr;
            *img(side, 0x0053d784) = shade ? own(0x40) : 0u;
            *img(side, 0x0053d780) = shade ? 2u : 0u;
            const int recs[2] = {recsA, recsB};
            const std::uint32_t gc[2] = {0x0053d768, 0x0053d770}, ga[2] = {0x0053d76c, 0x0053d774};
            for (int l = 0; l < 2; ++l) {
                const std::uint32_t arr = recs[l] ? own(0xA4 * recs[l]) : 0u;
                for (int r = 0; r < recs[l]; ++r) {
                    ptr<std::uint32_t>(arr)[41 * r + 0x80 / 4] = 0x7100u + static_cast<std::uint32_t>(8 * l + r);
                    ptr<std::uint32_t>(arr)[41 * r + 0x9C / 4] = recbuf[l][r] ? own(8) : 0u;
                }
                *img(side, gc[l]) = static_cast<std::uint32_t>(recs[l]);
                *img(side, ga[l]) = arr;
            }
            g_log.clear();
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](0, 0)));
            snap[side].insert(snap[side].end(), g_log.begin(), g_log.end());
            for (std::uint32_t va : {0x0053d798u, 0x0053d778u, 0x0053d77cu, 0x0053d784u, 0x0053d780u, 0x0053d768u, 0x0053d76cu, 0x0053d770u, 0x0053d774u})
                snap[side].push_back(role(*img(side, va)));
            for (int s = 0; s < 7; ++s)
                for (int k = 0; k < 9; ++k) snap[side].push_back(k == 0 ? (table[9 * s] == def ? 0xDEFu : role(table[9 * s])) : table[9 * s + k]);
            for (std::uint32_t b : g_roles) if (b) real_free(ptr<void>(b));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  TextureManager_FreeAllSlots calls %d (0..6 slots in every state, name list, shade table, both archive lists)\n", compared);
}

// ShadeLevel_Generate (0x0046e4e0, shade entry ECX {rgbA, rgbB, weightA, weightB}, source colours EDX; stack: count,
// level 0..31, destination): each 16-bit colour split by the screen format (Pixfmt_GetFormat [0x00632158]: 5 = 555,
// else 565), blended towards the entry by level / 31 and packed back. The arena fuzz would feed denormal floats; here
// real colours, entries (channels and weights 0..1), levels 0..31, both formats (split and pack formats set together -
// with the pristine pack globals only blue survives, which is the same in both). Compared: the destination words.
TEST(native_shade_level_generate_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const float*, const std::uint16_t*, int, int, std::uint16_t*);
    const Fn fn[2] = {rt::original<Fn>(0x0046e4e0), reinterpret_cast<Fn>(&recoil::ShadeLevel_Generate)};
    std::mt19937 rng(0x46e4e0);
    auto fr = [&] { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); };
    int compared = 0;
    for (int it = 0; it < 4000; ++it) {
        float entry[8];
        for (auto& f : entry) f = fr();
        const int count = static_cast<int>(rng() % 21), level = static_cast<int>(rng() % 32);
        const std::uint32_t fmt = rng() % 2 ? 5u : 6u;
        std::vector<std::uint16_t> src(static_cast<std::size_t>(count) + 1);
        for (auto& c : src) c = static_cast<std::uint16_t>(rng());
        std::vector<std::uint16_t> dst[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            for (std::uint32_t va : {0x00632158u, 0x0063215cu, 0x00632160u}) *img(side, va) = fmt;  // Pixfmt_GetFormat's three words
            // Pixel_PackRGB packs with the screen format (0x00632170 shifts R/G/B, masks R/G): 555 or 565 to match
            const std::uint32_t pack[2][5] = {{10, 5, 3, 0x1f, 0x1f}, {11, 5, 3, 0x1f, 0x3f}};
            for (int k = 0; k < 5; ++k) *img(side, 0x00632170u + 4 * k) = pack[fmt == 5 ? 0 : 1][k];
            dst[side].assign(static_cast<std::size_t>(count) + 2, 0xC7C7);
            fn[side](entry, src.data(), count, level, dst[side].data());
        }
        CHECK(dst[0] == dst[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  ShadeLevel_Generate calls %d (real colours, entries, levels 0..31, 555 / 565)\n", compared);
}

// TextureManager_FindOrAddSlot (0x0046d810, path ECX): cuts the path at its last '.' (strrchr) for
// TextureManager_FindSlotByName, restores the '.'; found -> that slot; else appends a slot at the count, names it
// (TextureSlot_SetNameFromPath) and sets state 2. Real tables of 0..8 slots, paths with and without directories and
// extensions. Compared: the result as an index, the count, every slot's name and state, and the path after the call.
TEST(native_texture_manager_find_or_add_slot_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(char*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046d810), reinterpret_cast<Fn>(&recoil::TextureManager_FindOrAddSlot)};
    const char* names[] = {"sky", "ground", "tank01", "SKY", "a.b"};
    const char* paths[] = {"sky.pcx", "..\\tex\\sky.pcx", "ground", "data/tank01.bmp", "a.b.c", "new.pcx", "x\\y\\z", "SKY.PCX", ".pcx"};
    std::mt19937 rng(0x46d810);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const int n = static_cast<int>(rng() % 9);
        int nm[10];
        std::uint32_t st[10];
        for (int s = 0; s < 10; ++s) { nm[s] = static_cast<int>(rng() % 5); st[s] = rng() % 3 ? 1u : 0u; }
        const std::string path0 = paths[rng() % 9];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            auto* table = reinterpret_cast<unsigned char*>(img(side, 0x0053d79c));
            for (int s = 0; s < 10; ++s) {
                std::memset(table + 0x24 * s, 0, 0x24);
                std::strcpy(reinterpret_cast<char*>(table + 0x24 * s + 8), names[nm[s]]);
                std::memcpy(table + 0x24 * s + 0x1C, &st[s], 4);
            }
            *img(side, 0x0053d798) = static_cast<std::uint32_t>(n);
            std::vector<char> path(path0.begin(), path0.end());
            path.push_back(0);
            const std::uint32_t r = fn[side](path.data(), 0);
            snap[side].push_back(r ? (r - addr(table)) / 0x24 : 0xFFFFFFFFu);
            snap[side].push_back(*img(side, 0x0053d798));
            for (int s = 0; s < 10; ++s) {
                const auto* sl = reinterpret_cast<const std::uint32_t*>(table + 0x24 * s);
                for (int k = 0; k < 9; ++k) snap[side].push_back(sl[k]);
            }
            for (char c : path) snap[side].push_back(static_cast<unsigned char>(c));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  TextureManager_FindOrAddSlot calls %d (slot tables of 0..8, paths with dirs / extensions)\n", compared);
}

// TextureManager_ShutdownAll (0x0046ebb0): TextureManager_FreeAllSlots, then StringSet_Destroy of the search-path set
// [0x0053d794] (its return, 0, stored back); returns 1. Each side: empty slot table and lists, the zVideo hooks
// logged, a real search set built by its own TextureManager_AddSearchPaths on its own node pool. Compared: return,
// the log (hook calls; the set's blocks are per side, so each free is counted, not named) and the set global.
TEST(native_texture_manager_shutdown_all_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    // TextureManager_Shutdown (0x0046eb90) is ShutdownAll returning 0: the same scenario for both
    const Fn fns[2][2] = {{rt::original<Fn>(0x0046ebb0), reinterpret_cast<Fn>(&recoil::TextureManager_ShutdownAll)},
                          {rt::original<Fn>(0x0046eb90), reinterpret_cast<Fn>(&recoil::TextureManager_Shutdown)}};
    const Fn add[2] = {rt::original<Fn>(0x0046ebd0), reinterpret_cast<Fn>(&recoil::TextureManager_AddSearchPaths)};
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    Hooks hooks;
    const char* lists[] = {"a;b", "..\\data\\common\\image", "x;y;z;w", ""};
    std::mt19937 rng(0x46ebb0);
    int compared = 0;
    for (int it = 0; it < 1000; ++it) {
        const Fn* fn = fns[it % 2];
        const int calls = static_cast<int>(rng() % 3);
        std::vector<std::string> args;
        for (int c = 0; c < calls; ++c) args.push_back(lists[rng() % 4]);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc1c))) = reinterpret_cast<void*>(&hook_release);
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc20))) = reinterpret_cast<void*>(&hook_after);
            for (std::uint32_t va : {0x0053d798u, 0x0053d778u, 0x0053d77cu, 0x0053d784u, 0x0053d780u, 0x0053d768u, 0x0053d76cu, 0x0053d770u, 0x0053d774u, 0x0053d794u})
                *img(side, va) = 0;
            init[side](4, 0);
            for (const auto& a : args) add[side](addr(a.c_str()), 0);
            g_roles.clear();
            g_log.clear();
            snap[side].push_back(fn[side](0, 0));
            for (std::size_t k = 0; k + 1 < g_log.size(); k += 2) {
                snap[side].push_back(g_log[k]);
                snap[side].push_back(g_log[k] == 1 ? 0xF4EEu : g_log[k + 1]);
            }
            snap[side].push_back(*img(side, 0x0053d794));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  TextureManager_ShutdownAll / Shutdown calls %d (empty slots, real search sets from 0..2 path lists; hooks logged)\n", compared);
}

// Image_BuildShadeLevels (0x0046e8d0, buffer ECX, colour count EDX): with shade entries ([0x0053d780] count), realloc
// the buffer to count*0x4000 + 0x200 and fill 32 levels per entry after the 0x200-byte palette header
// (ShadeLevel_Generate); returns the buffer. ShadeEntry_Add (0x0046e720, 8-float entry ECX): an exactly equal entry ->
// its index; else append (realloc the table) and rebuild: each slot image with its own palette (size [+0xE] != 0, not
// shared flag 0x10 at +9) and each shared palette ([0x0053d778] / [0x0053d77c]) is realloc'd and gets the new 32
// levels; images pointing at a shared palette that moved are re-pointed. Real tables, slot images, own and shared
// palettes, both formats. Compared: return, the tables, pointer roles and palette contents (hashed).
namespace {
std::uint32_t fnv(const void* p, std::size_t n)
{
    std::uint32_t h = 2166136261u;
    for (std::size_t k = 0; k < n; ++k) h = (h ^ static_cast<const unsigned char*>(p)[k]) * 16777619u;
    return h;
}
// a shade palette's defined bytes: the 0x200 header and, per entry and level, the colours actually generated (a level
// block is 0x200 bytes; with fewer than 256 colours its tail is never written - fresh realloc memory)
std::uint32_t fnv_levels(const void* p, int entries, int colours)
{
    const auto* b = static_cast<const unsigned char*>(p);
    std::uint32_t h = fnv(b, 0x200);
    for (int e = 0; e < entries; ++e)
        for (int l = 0; l < 32; ++l) h = h * 31u + fnv(b + 0x200 + 0x4000 * e + 0x200 * l, 2 * static_cast<std::size_t>(colours));
    return h;
}
void set_formats(int side, std::uint32_t fmt)
{
    for (std::uint32_t va : {0x00632158u, 0x0063215cu, 0x00632160u}) *img(side, va) = fmt;
    const std::uint32_t pack[2][5] = {{10, 5, 3, 0x1f, 0x1f}, {11, 5, 3, 0x1f, 0x3f}};
    for (int k = 0; k < 5; ++k) *img(side, 0x00632170u + 4 * k) = pack[fmt == 5 ? 0 : 1][k];
}
}  // namespace

TEST(native_image_build_shade_levels_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046e8d0), reinterpret_cast<Fn>(&recoil::Image_BuildShadeLevels)};
    std::mt19937 rng(0x46e8d0);
    auto fr = [&] { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); };
    int compared = 0;
    for (int it = 0; it < 600; ++it) {
        const int c = static_cast<int>(rng() % 4), colours = 1 + static_cast<int>(rng() % 256);
        const std::uint32_t fmt = rng() % 2 ? 5u : 6u;
        std::vector<float> table(8 * 4);
        for (auto& f : table) f = fr();
        std::vector<unsigned char> head(0x200);
        for (auto& b : head) b = static_cast<unsigned char>(rng());
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, fmt);
            *img(side, 0x0053d780) = static_cast<std::uint32_t>(c);
            *img(side, 0x0053d784) = addr(table.data());
            auto* buf = static_cast<unsigned char*>(real_malloc(0x200));
            std::memcpy(buf, head.data(), 0x200);
            const std::uint32_t r = fn[side](buf, colours);
            snap[side].push_back(c == 0 ? (r == addr(buf) ? 1u : 0u) : 2u);
            snap[side].push_back(fnv_levels(ptr<void>(r), c, colours));
            real_free(ptr<void>(r));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Image_BuildShadeLevels calls %d (0..3 entries, 1..256 colours, 555 / 565)\n", compared);
}

TEST(native_shade_entry_add_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const float*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046e720), reinterpret_cast<Fn>(&recoil::ShadeEntry_Add)};
    std::mt19937 rng(0x46e720);
    auto fr = [&] { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); };
    int compared = 0, added = 0;
    for (int it = 0; it < 300; ++it) {
        const int c0 = static_cast<int>(rng() % 3), n = static_cast<int>(rng() % 4), m = static_cast<int>(rng() % 3);
        const std::uint32_t fmt = rng() % 2 ? 5u : 6u;
        std::vector<float> table(8 * 3);
        for (auto& f : table) f = fr();
        float entry[8];
        const bool exact = c0 && rng() % 3 == 0;
        const int pick = c0 ? static_cast<int>(rng() % c0) : 0;
        for (int k = 0; k < 8; ++k) entry[k] = exact ? table[8 * pick + k] : fr();
        const std::size_t psize = static_cast<std::size_t>(c0) * 0x4000 + 0x200;
        int kind[4], shared_ix[4], pal_n[4];
        for (int s = 0; s < 4; ++s) {
            kind[s] = static_cast<int>(rng() % 3);  // 0 no palette, 1 own palette, 2 a shared one
            if (kind[s] == 2 && m == 0) kind[s] = 1;
            shared_ix[s] = m ? static_cast<int>(rng() % m) : 0;
            pal_n[s] = 1 + static_cast<int>(rng() % 256);
        }
        std::vector<unsigned char> seed_bytes(psize * 6);
        for (auto& b : seed_bytes) b = static_cast<unsigned char>(rng());
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, fmt);
            auto* tb = static_cast<float*>(real_malloc(0x20 * (c0 ? c0 : 1)));
            std::memcpy(tb, table.data(), 0x20 * c0);
            *img(side, 0x0053d780) = static_cast<std::uint32_t>(c0);
            *img(side, 0x0053d784) = addr(tb);
            std::vector<std::uint32_t> shared(3, 0);
            for (int k = 0; k < m; ++k) {
                shared[k] = addr(real_malloc(psize));
                std::memcpy(ptr<void>(shared[k]), &seed_bytes[psize * (3 + k)], psize);
            }
            auto* sharr = static_cast<std::uint32_t*>(real_malloc(4 * 3));
            for (int k = 0; k < 3; ++k) sharr[k] = shared[k];
            *img(side, 0x0053d778) = static_cast<std::uint32_t>(m);
            *img(side, 0x0053d77c) = addr(sharr);
            auto* slots = img(side, 0x0053d79c);
            std::uint32_t images[4][16];
            for (int s = 0; s < 4; ++s) {
                std::memset(images[s], 0, sizeof images[s]);
                auto* ib = reinterpret_cast<unsigned char*>(images[s]);
                if (kind[s] == 1) {
                    images[s][6] = addr(real_malloc(psize));
                    std::memcpy(ptr<void>(images[s][6]), &seed_bytes[psize * (s % 3)], psize);
                    reinterpret_cast<std::int16_t*>(ib)[7] = static_cast<std::int16_t>(pal_n[s]);
                } else if (kind[s] == 2) {
                    images[s][6] = shared[shared_ix[s]];
                    ib[9] = 0x10;
                    reinterpret_cast<std::int16_t*>(ib)[7] = static_cast<std::int16_t>(pal_n[s]);
                }
                slots[9 * s] = addr(images[s]);
            }
            *img(side, 0x0053d798) = static_cast<std::uint32_t>(n);
            const int ret = fn[side](entry, 0);
            const std::uint32_t c1 = *img(side, 0x0053d780);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            snap[side].push_back(c1);
            snap[side].push_back(fnv(ptr<void>(*img(side, 0x0053d784)), 0x20 * c1));
            const auto* sh = ptr<std::uint32_t>(*img(side, 0x0053d77c));
            for (int k = 0; k < m; ++k) snap[side].push_back(fnv_levels(ptr<void>(sh[k]), static_cast<int>(c1), 0x100));
            for (int s = 0; s < 4; ++s) {
                int which = -1;
                for (int k = 0; k < m; ++k) if (images[s][6] && images[s][6] == sh[k]) which = k;
                snap[side].push_back(static_cast<std::uint32_t>(which));
                if (kind[s] == 1) snap[side].push_back(fnv_levels(ptr<void>(images[s][6]), static_cast<int>(s < n ? c1 : c0), pal_n[s]));
            }
            for (int s = 0; s < 4; ++s) if (kind[s] == 1) real_free(ptr<void>(images[s][6]));
            for (int k = 0; k < m; ++k) real_free(ptr<void>(sh[k]));
            real_free(sharr);
            real_free(ptr<void>(*img(side, 0x0053d784)));
        }
        CHECK_SNAP(snap[0], snap[1]);
        added += snap[0][0] == snap[0][1] - 1 && snap[0][1] > static_cast<std::uint32_t>(c0);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  ShadeEntry_Add calls %d, %d appended (0..2 entries, 0..3 slot images with own / shared palettes, 0..2 shared palettes)\n", compared, added);
}

// Image_ReadPixels (0x0046ede0, FILE ECX, image EDX, requested bytes per pixel on the stack, 0 = the image's own;
// ret 4): a request other than the image's bpp (1 + flag bit 0 at +9) reads nothing (-1 when smaller, 0 when larger);
// else fread the pixel buffer [+0x10] (Image_PixelDataSize), the alpha bytes when flag 0x08 (malloc'd into +0x14,
// flag 0x40), the palette unless shared (0x10) and non-empty (bpp x [+0xE] bytes into +0x18, flag 0x80); a 16-bit
// unshared image under a 555 screen gets its pixels / palette converted 565 -> 555; a palette gets its shade levels
// (Image_BuildShadeLevels). Real temp files (sometimes short), random flags / sizes / requests, both screen formats,
// 0 or 1 shade entries. Compared: return, the image words (pointers as set / not), and the buffers' defined bytes.
TEST(native_image_read_pixels_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*, int);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    const Fn fn[2] = {rt::original<Fn>(0x0046ede0), reinterpret_cast<Fn>(&recoil::Image_ReadPixels)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "irp", 0, path);
    std::mt19937 rng(0x46ede0);
    auto fr = [&] { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); };
    int compared = 0, full = 0;
    for (int it = 0; it < 2000; ++it) {
        const int count = 1 + static_cast<int>(rng() % 64);
        const int pals[] = {0, 0, 16, 256};
        const int pal = pals[rng() % 4];
        const unsigned char flags = static_cast<unsigned char>((rng() % 2) | (rng() % 2 ? 0x08 : 0) | (rng() % 4 == 0 ? 0x10 : 0));
        const int bpp = 1 + (flags & 1);
        const int request = rng() % 4 ? 0 : static_cast<int>(rng() % 4);
        const std::uint32_t fmt = rng() % 2 ? 5u : 6u;
        const int shades = static_cast<int>(rng() % 2);
        float shade[8];
        for (auto& f : shade) f = fr();
        const std::size_t pixbytes = pal ? static_cast<std::size_t>(count) : static_cast<std::size_t>(bpp * count);
        std::vector<unsigned char> file(pixbytes + static_cast<std::size_t>(count) + static_cast<std::size_t>(bpp * pal));
        for (auto& b : file) b = static_cast<unsigned char>(rng());
        if (rng() % 5 == 0) file.resize(rng() % file.size());
        void* w = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), w);
        m_fclose(w);
        std::uint32_t init[16];
        for (auto& x : init) x = rng() | 0x80000000u;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, fmt);
            *img(side, 0x0053d780) = static_cast<std::uint32_t>(shades);
            *img(side, 0x0053d784) = addr(shade);
            std::uint32_t image[16];
            std::memcpy(image, init, sizeof image);
            auto* ib = reinterpret_cast<unsigned char*>(image);
            image[0] = static_cast<std::uint32_t>(count);
            ib[9] = flags;
            reinterpret_cast<std::int16_t*>(ib)[7] = static_cast<std::int16_t>(pal);
            image[4] = addr(real_malloc(pixbytes));
            image[5] = 0;
            image[6] = flags & 0x10 ? 0x0BADF00Du : 0u;  // a shared palette is not touched
            void* f = m_fopen(path, "rb");
            const int ret = fn[side](f, image, request);
            m_fclose(f);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            for (int k = 0; k < 16; ++k) snap[side].push_back(k == 4 || k == 5 || (k == 6 && !(flags & 0x10)) ? (image[k] ? 1u : 0u) : image[k]);
            if (ret == 0 && (request == 0 || request == bpp)) {
                snap[side].push_back(fnv(ptr<void>(image[4]), pixbytes));
                if (image[5]) snap[side].push_back(fnv(ptr<void>(image[5]), static_cast<std::size_t>(count)));
                if (image[6] && !(flags & 0x10)) {
                    // defined bytes: the palette read (bpp x size) and, with shade entries, the generated levels
                    snap[side].push_back(fnv(ptr<void>(image[6]), static_cast<std::size_t>(bpp * pal)));
                    for (int l = 0; bpp == 2 && l < 32 * shades; ++l) /* levels: 16-bit images only */ snap[side].push_back(fnv(ptr<unsigned char>(image[6]) + 0x200 + 0x200 * l, 2 * static_cast<std::size_t>(pal)));
                }
                full += side;
            }
            real_free(ptr<void>(image[4]));
            if (image[5]) real_free(ptr<void>(image[5]));
            if (image[6] && !(flags & 0x10)) real_free(ptr<void>(image[6]));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  Image_ReadPixels calls %d, %d full reads (flags, palettes 0/16/256, requests, short files, 555 / 565, shades)\n", compared, full);
}

// TextureArchive_Open (0x0046dae0, 0xA4-byte archive record ECX, file name at +0): archives disabled ([0x004e073c]
// 0) -> 0; File_OpenOnSearchPath(name, "rb") into +0x80 (null search set: the name as given); a 24-byte header into
// +0x84 (version +0x88 must be 1, palette count +0x8C, TOC count +0x90) - a short header or a wrong version closes
// the file; the TOC (0x28 bytes each, malloc'd into +0x9C; short -> close, free); base palette index +0xA0 = the
// shared palette count; the palettes appended to the shared list ([0x0053d778] / [0x0053d77c], realloc), each 0x200
// bytes read, converted 565 -> 555 under a 555 screen, given shade levels (Image_BuildShadeLevels); returns the FILE.
// Real archive files (TOC 0..3, palettes 0..2, wrong versions, cut short), 0..1 shared palettes already listed, both
// formats, 0 or 1 shade entries. Compared: return (set / not), record words, TOC bytes, shared count, palettes.
TEST(native_texture_archive_open_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    const Fn fn[2] = {rt::original<Fn>(0x0046dae0), reinterpret_cast<Fn>(&recoil::TextureArchive_Open)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "tao", 0, path);
    std::mt19937 rng(0x46dae0);
    auto fr = [&] { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); };
    int compared = 0, opened = 0;
    for (int it = 0; it < 800; ++it) {
        const bool enabled = rng() % 8 != 0;
        const std::uint32_t version = rng() % 6 == 0 ? 2u : 1u;
        const std::uint32_t pals = rng() % 3, toc = rng() % 4;
        const std::uint32_t fmt = rng() % 2 ? 5u : 6u;
        const int shades = static_cast<int>(rng() % 2), before = static_cast<int>(rng() % 2);
        float shade[8];
        for (auto& f : shade) f = fr();
        std::vector<unsigned char> file(24 + 0x28 * toc + 0x200 * pals);
        for (auto& b : file) b = static_cast<unsigned char>(rng());
        std::memcpy(&file[4], &version, 4);
        std::memcpy(&file[8], &pals, 4);
        std::memcpy(&file[12], &toc, 4);
        if (rng() % 6 == 0) file.resize(rng() % file.size());
        // palettes present in full in the file (after the header and a complete TOC)
        const std::size_t tail = file.size() >= 24 + 0x28 * static_cast<std::size_t>(toc) ? file.size() - 24 - 0x28 * static_cast<std::size_t>(toc) : 0;
        const std::uint32_t full_pals = static_cast<std::uint32_t>(tail / 0x200 < pals ? tail / 0x200 : pals);
        const bool missing = rng() % 12 == 0;
        void* w = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), w);
        m_fclose(w);
        std::vector<unsigned char> old_pal(0x200);
        for (auto& b : old_pal) b = static_cast<unsigned char>(rng());
        std::uint32_t init[41];
        for (auto& x : init) x = rng() | 0x80000000u;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, fmt);
            *img(side, 0x004e073c) = enabled ? 1u : 0u;
            *img(side, 0x0053d780) = static_cast<std::uint32_t>(shades);
            *img(side, 0x0053d784) = addr(shade);
            auto* list = static_cast<std::uint32_t*>(real_malloc(4));
            list[0] = addr(real_malloc(0x200));
            std::memcpy(ptr<void>(list[0]), old_pal.data(), 0x200);
            *img(side, 0x0053d778) = static_cast<std::uint32_t>(before);
            *img(side, 0x0053d77c) = addr(list);
            std::uint32_t rec[41];
            std::memcpy(rec, init, sizeof rec);
            std::memset(rec, 0, 0x80);
            std::strcpy(reinterpret_cast<char*>(rec), missing ? "no_such_texture_archive.zbd" : path);
            rec[0x80 / 4] = 0;  // left alone when archives are disabled; closed below when set
            const std::uint32_t r = fn[side](rec, 0);
            snap[side].push_back(r ? 1u : 0u);
            snap[side].push_back(r == rec[0x80 / 4] ? 1u : 0u);
            for (int k = 0x80 / 4; k < 41; ++k) snap[side].push_back(k == 0x80 / 4 || k == 0x9C / 4 ? (rec[k] ? 1u : 0u) : rec[k]);
            if (r && rec[0x9C / 4]) snap[side].push_back(fnv(ptr<void>(rec[0x9C / 4]), 0x28 * static_cast<std::size_t>(rec[0x90 / 4])));
            const std::uint32_t count = *img(side, 0x0053d778);
            const auto* nl = ptr<std::uint32_t>(*img(side, 0x0053d77c));
            snap[side].push_back(count);
            // A short palette read (KG-34, original defect) leaves the count raised by every palette: the entry being
            // read holds a partly read palette and the ones after it were never written (fresh realloc memory). Only
            // the listed-before and fully read palettes are compared; the written pointers are freed.
            const std::uint32_t written = count <= static_cast<std::uint32_t>(before) ? count : static_cast<std::uint32_t>(before) + (r ? count - before : (full_pals < count - before ? full_pals + 1 : count - before));
            const std::uint32_t whole = r ? count : static_cast<std::uint32_t>(before) + (count > static_cast<std::uint32_t>(before) ? full_pals : 0u);
            for (std::uint32_t k = 0; k < whole && k < count; ++k) snap[side].push_back(k < static_cast<std::uint32_t>(before) ? fnv(ptr<void>(nl[k]), 0x200) : fnv_levels(ptr<void>(nl[k]), shades, 0x100));
            opened += side && r;
            if (rec[0x80 / 4]) m_fclose(ptr<void>(rec[0x80 / 4]));
            if (r && rec[0x9C / 4]) real_free(ptr<void>(rec[0x9C / 4]));
            for (std::uint32_t k = 0; k < written && k < count; ++k) real_free(ptr<void>(nl[k]));
            if (count == 0) real_free(ptr<void>(list[0]));
            real_free(ptr<void>(*img(side, 0x0053d77c)));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  TextureArchive_Open calls %d, %d opened (TOC 0..3, palettes 0..2, versions, short files, missing, disabled)\n", compared, opened);
}

// Image_CreateProcedural (0x0046ef70, FILE ECX): Image_Alloc, Image_ReadHeader (failure -> 0, the record leaks),
// malloc the pixel buffer (Image_PixelDataSize) into +0x10, Image_ReadPixels(0), flag 0x20; returns the image. Real
// header files (header: byte 0 flags, words 4 / 6 width / height, byte 8, word 0xC -> +0xE palette size, word 0xE ->
// +0xC), small sizes, palettes 0 / 16, data in full or cut short, and null FILE. Compared: return (set / not), the
// image words (pointers as set / not) and the pixel / alpha / palette bytes read.
TEST(native_image_create_procedural_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    const Fn fn[2] = {rt::original<Fn>(0x0046ef70), reinterpret_cast<Fn>(&recoil::Image_CreateProcedural)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "icp", 0, path);
    std::mt19937 rng(0x46ef70);
    int compared = 0, made = 0;
    for (int it = 0; it < 1500; ++it) {
        const int w = 1 + static_cast<int>(rng() % 16), h = 1 + static_cast<int>(rng() % 16);
        const int pal = rng() % 3 == 0 ? 16 : 0;
        const unsigned char flags = static_cast<unsigned char>((rng() % 2) | (rng() % 2 ? 0x08 : 0));
        const int bpp = 1 + (flags & 1), count = w * h;
        std::vector<unsigned char> file(16);
        for (auto& b : file) b = static_cast<unsigned char>(rng());
        file[0] = flags;
        file[4] = static_cast<unsigned char>(w); file[5] = 0; file[6] = static_cast<unsigned char>(h); file[7] = 0;
        file[0xC] = static_cast<unsigned char>(pal); file[0xD] = 0;
        const std::size_t pixbytes = pal ? static_cast<std::size_t>(count) : static_cast<std::size_t>(bpp * count);
        for (std::size_t k = 0; k < pixbytes + count + bpp * pal; ++k) file.push_back(static_cast<unsigned char>(rng()));
        // cut short only after the 16-byte header: a short header leaves Image_ReadHeader's locals as stack residue
        // (KG-35, original defect - each side's own return addresses), not comparable
        if (rng() % 6 == 0) file.resize(16 + rng() % (file.size() - 15));
        const bool null_file = rng() % 20 == 0;
        void* wf = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), wf);
        m_fclose(wf);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, 6);
            *img(side, 0x0053d780) = 0;
            void* f = null_file ? nullptr : m_fopen(path, "rb");
            const std::uint32_t r = fn[side](f, 0);
            if (f) m_fclose(f);
            snap[side].push_back(r ? 1u : 0u);
            if (r) {
                const auto* im = ptr<std::uint32_t>(r);
                for (int k = 0; k < 14; ++k) snap[side].push_back(k >= 4 && k <= 6 ? (im[k] ? 1u : 0u) : im[k]);
                if (im[4] && file.size() >= 16 + pixbytes) snap[side].push_back(fnv(ptr<void>(im[4]), pixbytes));  // a short read leaves the tail unset
                made += side;
                for (int k = 4; k <= 6; ++k) if (im[k]) real_free(ptr<void>(im[k]));
                real_free(ptr<void>(r));
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  Image_CreateProcedural calls %d, %d created (real headers, small sizes, palettes 0/16, short files, null FILE)\n", compared, made);
}

// TextureArchiveListB_OpenImageZbd (0x0046da40): only while list B is empty ([0x0053d770] < 1): grow the array
// [0x0053d774] by one zeroed 0xA4-byte record named "image.zbd" and TextureArchive_Open it; on failure rename it
// "rimage.zbd" (sprintf "r%s") and retry; the count rises only on success. Each side runs in a temp working directory
// holding a valid image.zbd, a valid rimage.zbd, both, a bad one or neither (valid: version 1, no TOC, no palettes).
// Compared: count, the record (name, FILE set / not, the rest), array set / not.
TEST(native_texture_archive_list_b_open_image_zbd_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(int, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046da40), reinterpret_cast<Fn>(&recoil::TextureArchiveListB_OpenImageZbd)};
    using Fclose = int(__cdecl*)(void*);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    char tmp[MAX_PATH], old_cwd[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    const std::string dir = std::string(tmp) + "recoil_listb_test";
    CreateDirectoryA(dir.c_str(), nullptr);
    GetCurrentDirectoryA(MAX_PATH, old_cwd);
    SetCurrentDirectoryA(dir.c_str());
    auto write_archive = [](const char* name, bool valid) {
        std::uint32_t hdr[6] = {0, valid ? 1u : 7u, 0, 0, 0, 0};
        FILE* f = std::fopen(name, "wb");
        std::fwrite(hdr, 4, 6, f);
        std::fclose(f);
    };
    std::mt19937 rng(0x46da40);
    int compared = 0;
    for (int it = 0; it < 200; ++it) {
        const int files = static_cast<int>(rng() % 5);  // 0 none, 1 image, 2 rimage, 3 both, 4 a bad image
        DeleteFileA("image.zbd"); DeleteFileA("rimage.zbd");
        if (files == 1 || files == 3) write_archive("image.zbd", true);
        if (files == 2 || files == 3) write_archive("rimage.zbd", true);
        if (files == 4) write_archive("image.zbd", false);
        const std::uint32_t count0 = rng() % 4 == 0 ? 1u : 0u;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *img(side, 0x004e073c) = 1;
            *img(side, 0x0053d794) = 0;
            *img(side, 0x0053d778) = 0;
            *img(side, 0x0053d77c) = 0;
            auto* arr = static_cast<std::uint32_t*>(real_malloc(0xA4));
            std::memset(arr, 0, 0xA4);
            *img(side, 0x0053d770) = count0;
            *img(side, 0x0053d774) = addr(arr);
            fn[side](0, 0);
            const std::uint32_t count = *img(side, 0x0053d770);
            auto* na = ptr<std::uint32_t>(*img(side, 0x0053d774));
            snap[side].push_back(count);
            snap[side].push_back(na ? 1u : 0u);
            for (int k = 0; na && k < 41; ++k) snap[side].push_back(k == 0x80 / 4 || k == 0x9C / 4 ? (na[k] ? 1u : 0u) : na[k]);
            if (na && na[0x80 / 4]) m_fclose(ptr<void>(na[0x80 / 4]));
            if (na && na[0x9C / 4]) real_free(ptr<void>(na[0x9C / 4]));
            if (na) real_free(na);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA("image.zbd"); DeleteFileA("rimage.zbd");
    SetCurrentDirectoryA(old_cwd);
    RemoveDirectoryA(dir.c_str());
    rt::restore_pristine();
    std::printf("  TextureArchiveListB_OpenImageZbd calls %d (image / rimage / both / bad / neither, list empty or not)\n", compared);
}

// TextureArchiveListA_OpenBySize (0x0046df50): list A non-empty -> reopen every record whose FILE is null
// (File_OpenOnSearchPath with the texture search set [0x0053d794], "rb"). Empty: the zVideo hook [0x0056bc30] reports
// texture memory; on a hardware device ([0x0056bbe8]) with the query succeeding the name is "r" "texture" MB ".zbd",
// else the detail setting *[0x005617f0] (jump table 0x0046e23c: 1..4 -> texture8/6/4/2.zbd, else texturemax.zbd);
// a record is appended and opened (TextureArchive_Open); failing that, smaller sizes down to texturemax / texture.zbd
// are tried; the count rises on success. Each side in a temp working directory holding a random subset of valid
// archives (texture8/6/4/2.zbd, texturemax.zbd, texture.zbd, rtexture2/4/8/16.zbd); detail 0..5, device flag,
// memory hook failing or reporting 2..16 MB, and the reopen path. Compared: count, every record word (FILE as set /
// not), array set / not.
namespace {
std::uint32_t g_mem_bytes = 0;
int g_mem_ok = 0;
int __fastcall fake_texture_memory(int, std::uint32_t* bytes, void*)
{
    g_log.push_back(6);
    *bytes = g_mem_bytes;
    return g_mem_ok;
}
}  // namespace

TEST(native_texture_archive_list_a_open_by_size_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(int, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046df50), reinterpret_cast<Fn>(&recoil::TextureArchiveListA_OpenBySize)};
    using Fclose = int(__cdecl*)(void*);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    char tmp[MAX_PATH], old_cwd[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    const std::string dir = std::string(tmp) + "recoil_lista_test";
    CreateDirectoryA(dir.c_str(), nullptr);
    GetCurrentDirectoryA(MAX_PATH, old_cwd);
    SetCurrentDirectoryA(dir.c_str());
    const char* names[] = {"texture8.zbd", "texture6.zbd", "texture4.zbd", "texture2.zbd", "texturemax.zbd", "texture.zbd",
                           "rtexture2.zbd", "rtexture4.zbd", "rtexture8.zbd", "rtexture16.zbd"};
    std::mt19937 rng(0x46df50);
    int compared = 0, opened = 0;
    for (int it = 0; it < 400; ++it) {
        for (const char* nm : names) {
            DeleteFileA(nm);
            if (rng() % 3 == 0) {
                const std::uint32_t hdr[6] = {0, 1, 0, 0, 0, 0};
                FILE* f = std::fopen(nm, "wb");
                std::fwrite(hdr, 4, 6, f);
                std::fclose(f);
            }
        }
        std::uint32_t detail = rng() % 6;
        const std::uint32_t device = rng() % 2, mem_ok = rng() % 3 ? 1u : 0u;
        const std::uint32_t mb[] = {2, 4, 8, 16, 5};
        const std::uint32_t mem = mb[rng() % 5] << 20;
        const bool reopen = rng() % 5 == 0;
        const int re_a = static_cast<int>(rng() % 10), re_b = static_cast<int>(rng() % 10);  // drawn once for both sides
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *img(side, 0x004e073c) = 1;
            *img(side, 0x0053d794) = 0;
            *img(side, 0x0053d778) = 0;
            *img(side, 0x0053d77c) = 0;
            *img(side, 0x0056bbe8) = device;
            *img(side, 0x005617f0) = addr(&detail);
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc30))) = reinterpret_cast<void*>(&fake_texture_memory);
            g_mem_bytes = mem;
            g_mem_ok = static_cast<int>(mem_ok);
            auto* arr = static_cast<std::uint32_t*>(real_malloc(0xA4 * 2));
            std::memset(arr, 0, 0xA4 * 2);
            std::uint32_t count0 = 0;
            if (reopen) {
                count0 = 2;
                std::strcpy(reinterpret_cast<char*>(arr), names[re_a]);
                std::strcpy(reinterpret_cast<char*>(arr + 41), names[re_b]);
            }
            *img(side, 0x0053d768) = count0;
            *img(side, 0x0053d76c) = addr(arr);
            g_log.clear();
            fn[side](0, 0);
            const std::uint32_t count = *img(side, 0x0053d768);
            auto* na = ptr<std::uint32_t>(*img(side, 0x0053d76c));
            snap[side] = g_log;
            snap[side].push_back(count);
            const std::uint32_t recs = count > (count0 ? count0 : 1u) ? count : (count0 ? count0 : 1u);
            for (std::uint32_t r = 0; na && r < recs; ++r)
                for (int k = 0; k < 41; ++k) {
                    const std::uint32_t v = na[41 * r + k];
                    snap[side].push_back(k == 0x80 / 4 || k == 0x9C / 4 ? (v ? 1u : 0u) : v);
                }
            opened += side && count > count0;
            for (std::uint32_t r = 0; na && r < recs; ++r) {
                if (na[41 * r + 0x80 / 4]) m_fclose(ptr<void>(na[41 * r + 0x80 / 4]));
                if (na[41 * r + 0x9C / 4]) real_free(ptr<void>(na[41 * r + 0x9C / 4]));
            }
            if (na) real_free(na);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    for (const char* nm : names) DeleteFileA(nm);
    SetCurrentDirectoryA(old_cwd);
    RemoveDirectoryA(dir.c_str());
    rt::restore_pristine();
    std::printf("  TextureArchiveListA_OpenBySize calls %d, %d opened (archive subsets, detail 0..5, device, memory hook, reopen)\n", compared, opened);
}

// Image_LoadFromArchive (0x0046d940, name ECX; list B, auto-opening image.zbd while the list is empty) and
// Image_LoadFromArchiveListA (0x0046dd30; list A): the first TOC entry (0x28 bytes: name, offset +0x20, palette index
// +0x24) of an open archive matching the name (_stricmp) -> fseek + Image_CreateProcedural; a palette index other than
// -1 attaches the shared palette [0x0053d77c][record +0xA0 + index] (list A first frees a palette the image read
// itself and forces its size to 256). Real archive files (TOC 1..4, image blobs with or without their own palette),
// records opened per side, 3 shared palettes, names in mixed case or missing; list B empty in a working directory
// without image.zbd. Compared: result set / not, the image words (the palette as a shared index / own / none) and the
// pixels.
TEST(native_image_load_from_archive_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const char*, int);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    // Image_Load (0x0046d900): list A first, then list B, then Image_ApplyAlphaMask when flag 0x02 and alpha are set
    const Fn fns[3][2] = {{rt::original<Fn>(0x0046d940), reinterpret_cast<Fn>(&recoil::Image_LoadFromArchive)},
                          {rt::original<Fn>(0x0046dd30), reinterpret_cast<Fn>(&recoil::Image_LoadFromArchiveListA)},
                          {rt::original<Fn>(0x0046d900), reinterpret_cast<Fn>(&recoil::Image_Load)}};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char tmp[MAX_PATH], old_cwd[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    const std::string dir = std::string(tmp) + "recoil_loadarc_test";
    CreateDirectoryA(dir.c_str(), nullptr);
    GetCurrentDirectoryA(MAX_PATH, old_cwd);
    SetCurrentDirectoryA(dir.c_str());
    GetTempFileNameA(tmp, "lfa", 0, path);
    const char* pool[] = {"sky", "tank01", "ground", "water"};
    std::mt19937 rng(0x46d940);
    int compared = 0, found = 0;
    for (int it = 0; it < 1800; ++it) {
        const int which = it % 3;
        const int toc = 1 + static_cast<int>(rng() % 4);
        std::vector<unsigned char> file(24 + 0x28 * toc);
        std::vector<int> pal_ix(toc), own_pal(toc);
        for (int e = 0; e < toc; ++e) {
            unsigned char* ent = &file[24 + 0x28 * e];
            std::memset(ent, 0, 0x28);
            std::strcpy(reinterpret_cast<char*>(ent), pool[rng() % 4]);
            pal_ix[e] = rng() % 3 == 0 ? -1 : static_cast<int>(rng() % 2);
            own_pal[e] = rng() % 2;
            const std::uint32_t off = static_cast<std::uint32_t>(file.size());
            std::memcpy(ent + 0x20, &off, 4);
            std::memcpy(ent + 0x24, &pal_ix[e], 4);
            const int w = 1 + static_cast<int>(rng() % 8), h = 1 + static_cast<int>(rng() % 8);
            unsigned char hdr[16] = {};
            const unsigned char aflags = static_cast<unsigned char>((rng() % 2 ? 0x08 : 0) | (rng() % 2 ? 0x02 : 0));  // alpha bytes, alpha mask
            hdr[0] = static_cast<unsigned char>(1 | aflags);  // 16-bit
            hdr[4] = static_cast<unsigned char>(w); hdr[6] = static_cast<unsigned char>(h);
            hdr[0xC] = static_cast<unsigned char>(own_pal[e] ? 16 : 0);
            file.insert(file.end(), hdr, hdr + 16);
            const std::size_t data = (own_pal[e] ? static_cast<std::size_t>(w * h) + 2 * 16 : static_cast<std::size_t>(2 * w * h)) + (aflags & 0x08 ? static_cast<std::size_t>(w * h) : 0u);
            for (std::size_t k = 0; k < data; ++k) file.push_back(static_cast<unsigned char>(rng()));
        }
        void* wf = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), wf);
        m_fclose(wf);
        std::string q = rng() % 5 == 0 ? std::string("missing") : std::string(pool[rng() % 4]);
        for (char& c : q) if (rng() % 3 == 0) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        const std::uint32_t count = rng() % 4 == 0 ? 0u : 1u + rng() % 2, base = rng() % 2;
        const bool second_open = rng() % 2;
        const std::uint32_t count_b = rng() % 3 == 0 ? 0u : 1u;  // Image_Load: list B beside list A
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, 6);
            *img(side, 0x0053d780) = 0;
            *img(side, 0x004e073c) = 1;
            *img(side, 0x0053d794) = 0;
            std::uint32_t shared[3];
            for (auto& s : shared) { s = addr(real_malloc(0x200)); std::memset(ptr<void>(s), 0x5A, 0x200); }
            *img(side, 0x0053d778) = 3;
            *img(side, 0x0053d77c) = addr(shared);
            auto* recs = static_cast<std::uint32_t*>(real_malloc(0xA4 * 2));
            std::memset(recs, 0, 0xA4 * 2);
            auto* tocbuf = static_cast<unsigned char*>(real_malloc(0x28 * toc));
            std::memcpy(tocbuf, &file[24], 0x28 * toc);
            for (int r = 0; r < 2; ++r) {
                // list B empty: the image.zbd auto-open reallocs the array to one zeroed record - open nothing then
                const bool auto_open = which == 0 && count == 0;  // (Image_Load gets its own list B below)
                recs[41 * r + 0x80 / 4] = !auto_open && (r == 0 || second_open) ? addr(m_fopen(path, "rb")) : 0u;
                recs[41 * r + 0x90 / 4] = static_cast<std::uint32_t>(toc);
                recs[41 * r + 0x9C / 4] = addr(tocbuf);
                recs[41 * r + 0xA0 / 4] = base;
            }
            const std::uint32_t gc = which ? 0x0053d768u : 0x0053d770u, ga = which ? 0x0053d76cu : 0x0053d774u;
            *img(side, gc) = count;
            *img(side, ga) = addr(recs);
            std::uint32_t* recs_b = nullptr;
            if (which == 2) {
                recs_b = static_cast<std::uint32_t*>(real_malloc(0xA4));
                std::memset(recs_b, 0, 0xA4);
                recs_b[0x80 / 4] = count_b ? addr(m_fopen(path, "rb")) : 0u;
                recs_b[0x90 / 4] = static_cast<std::uint32_t>(toc);
                recs_b[0x9C / 4] = addr(tocbuf);
                recs_b[0xA0 / 4] = base;
                *img(side, 0x0053d770) = count_b;
                *img(side, 0x0053d774) = addr(recs_b);
            }
            const std::uint32_t r = fns[which][side](q.c_str(), 0);
            snap[side].push_back(r ? 1u : 0u);
            snap[side].push_back(*img(side, gc));
            if (const auto* r0 = ptr<std::uint32_t>(*img(side, ga))) for (int k = 0; k < 4; ++k) snap[side].push_back(r0[k]);  // record 0's name (the image.zbd auto-open renames it)
            if (r) {
                auto* im = ptr<std::uint32_t>(r);
                for (int k = 0; k < 14; ++k) {
                    std::uint32_t v = im[k];
                    if (k == 4 || k == 5) v = v ? 1u : 0u;
                    if (k == 6) { v = 0xFFu; for (int s = 0; s < 3; ++s) if (im[6] == shared[s]) v = static_cast<std::uint32_t>(s); if (v == 0xFFu && im[6]) v = 0xAAu; if (!im[6]) v = 0xEEu; }
                    snap[side].push_back(v);
                }
                snap[side].push_back(fnv(ptr<void>(im[4]), static_cast<std::size_t>(im[0])));  // a paletted buffer holds count bytes
                found += side;
                real_free(ptr<void>(im[4]));
                if (im[5]) real_free(ptr<void>(im[5]));
                bool is_shared = false;
                for (auto s : shared) is_shared |= im[6] == s;
                if (im[6] && !is_shared) real_free(ptr<void>(im[6]));
                real_free(im);
            }
            const std::uint32_t cur = *img(side, ga);
            auto* nr = ptr<std::uint32_t>(cur);
            for (int rr = 0; nr == recs && rr < 2; ++rr) if (recs[41 * rr + 0x80 / 4]) m_fclose(ptr<void>(recs[41 * rr + 0x80 / 4]));
            if (which == 2) {
                auto* nb = ptr<std::uint32_t>(*img(side, 0x0053d774));
                if (nb == recs_b && recs_b[0x80 / 4]) m_fclose(ptr<void>(recs_b[0x80 / 4]));
                snap[side].push_back(*img(side, 0x0053d770));
                if (nb) real_free(nb);
            }
            real_free(tocbuf);
            if (nr) real_free(nr);
            for (auto s : shared) real_free(ptr<void>(s));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    SetCurrentDirectoryA(old_cwd);
    RemoveDirectoryA(dir.c_str());
    rt::restore_pristine();
    CHECK(found > 200);
    std::printf("  Image_LoadFromArchive / ListA / Image_Load calls %d, %d found (TOC 1..4, own / shared palettes, mixed-case names, empty list B)\n", compared, found);
}

// TextureSlot_LoadMipChain (0x0046e3e0, slot ECX): only for a name ending "_1" (strstr "_1" at the end): bump the
// digit and, level by level, find the slot (TextureManager_FindSlotByName) or load it (Image_Load; the chain stops at
// the first failure), append / name new slots, state 1, Image_PrepareSoftwareSampling; each level is linked from the
// previous slot's +0x20 and gets the width ratio (base width / level width) as a float at image +0x1C. Real archive
// (list A) holding a random subset of tex_2 / tex_3 / tex_4 at halving widths; base slot "tex_1" or non-matching
// names; an existing level slot pending or loaded. Compared: the slot table (names, states, links as indices,
// images by role) and each new image's words and pixels.
TEST(native_texture_slot_load_mip_chain_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    const Fn fn[2] = {rt::original<Fn>(0x0046e3e0), reinterpret_cast<Fn>(&recoil::TextureSlot_LoadMipChain)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char tmp[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    GetTempFileNameA(tmp, "mip", 0, path);
    const char* bases[] = {"tex_1", "TEX_1", "tex_1x", "tex", "a_1"};
    std::mt19937 rng(0x46e3e0);
    int compared = 0, chained = 0, faults[3] = {};
    for (int it = 0; it < 800; ++it) {
        const int bi = rng() % 4 ? 0 : static_cast<int>(rng() % 5);
        const std::string base = bases[bi];
        std::string stem = base.substr(0, base.size() >= 2 ? base.size() - 1 : 0);
        std::vector<std::string> levels;
        for (int l = 2; l <= 4; ++l) if (rng() % 4) levels.push_back(stem + std::to_string(l));
        std::vector<unsigned char> file(24 + 0x28 * (levels.size() ? levels.size() : 1));
        for (std::size_t e = 0; e < levels.size(); ++e) {
            unsigned char* ent = &file[24 + 0x28 * e];
            std::memset(ent, 0, 0x28);
            std::strcpy(reinterpret_cast<char*>(ent), levels[e].c_str());
            const std::uint32_t off = static_cast<std::uint32_t>(file.size());
            const int minus1 = -1;
            std::memcpy(ent + 0x20, &off, 4);
            std::memcpy(ent + 0x24, &minus1, 4);
            const int w = 16 >> (levels[e].back() - '1');
            unsigned char hdr[16] = {};
            hdr[0] = 1; hdr[4] = static_cast<unsigned char>(w); hdr[6] = static_cast<unsigned char>(w);
            file.insert(file.end(), hdr, hdr + 16);
            for (int k = 0; k < 2 * w * w; ++k) file.push_back(static_cast<unsigned char>(rng()));
        }
        void* wf = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), wf);
        m_fclose(wf);
        const int existing = static_cast<int>(rng() % 3);  // 0 none, 1 a pending level-2 slot, 2 a loaded level-2 slot
        std::uint32_t base_img[16];
        for (auto& x : base_img) x = rng() | 0x80000000u;
        reinterpret_cast<std::int16_t*>(base_img)[2] = 16;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, 6);
            *img(side, 0x0053d780) = 0;
            auto* recs = static_cast<std::uint32_t*>(real_malloc(0xA4));
            std::memset(recs, 0, 0xA4);
            auto* tocbuf = static_cast<unsigned char*>(real_malloc(0x28 * (levels.size() ? levels.size() : 1)));
            std::memcpy(tocbuf, &file[24], 0x28 * levels.size());
            recs[0x80 / 4] = addr(m_fopen(path, "rb"));
            recs[0x90 / 4] = static_cast<std::uint32_t>(levels.size());
            recs[0x9C / 4] = addr(tocbuf);
            *img(side, 0x0053d768) = 1;
            *img(side, 0x0053d76c) = addr(recs);
            auto* recs_b = static_cast<std::uint32_t*>(real_malloc(0xA4));
            std::memset(recs_b, 0, 0xA4);  // list B: one record, no file (skipped)
            *img(side, 0x0053d770) = 1;
            *img(side, 0x0053d774) = addr(recs_b);
            auto* table = img(side, 0x0053d79c);
            std::memset(table, 0, 0x24 * 8);
            std::uint32_t bimg[16];
            std::memcpy(bimg, base_img, sizeof bimg);
            std::strcpy(reinterpret_cast<char*>(table + 2), base.c_str());
            table[0] = addr(bimg);
            table[7] = 1;
            std::uint32_t token_img[16] = {};
            reinterpret_cast<std::int16_t*>(token_img)[2] = 8;
            int n = 1;
            if (existing) {
                std::strcpy(reinterpret_cast<char*>(table + 9 + 2), (stem + "2").c_str());
                table[9 + 7] = existing == 1 ? 2u : 1u;
                table[9 + 0] = existing == 2 ? addr(token_img) : 0u;
                table[9 + 1] = 4;  // binding word (+4): a loaded level's divisor comes from here
                n = 2;
            }
            *img(side, 0x0053d798) = static_cast<std::uint32_t>(n);
            // an already-loaded level keeps a stale EDI (KG-36): guarded, and outcomes compared
            const unsigned outcome = wd::guarded([&] { fn[side](table, 0); });
            snap[side].push_back(outcome);
            if (outcome != wd::kOk) { faults[existing] += side; m_fclose(ptr<void>(recs[0x80 / 4])); continue; }
            const std::uint32_t count = *img(side, 0x0053d798);
            snap[side].push_back(count);
            std::vector<std::uint32_t> made;
            for (std::uint32_t s = 0; s < count && s < 8; ++s) {
                const std::uint32_t* sl = table + 9 * s;
                for (int k = 2; k < 7; ++k) snap[side].push_back(sl[k]);
                snap[side].push_back(sl[7]);
                snap[side].push_back(sl[8] ? (sl[8] - addr(table)) / 0x24 : 0xFFFFFFFFu);
                if (sl[0] == addr(bimg)) snap[side].push_back(0xB0u);
                else if (sl[0] == addr(token_img)) snap[side].push_back(0x70u);
                else if (!sl[0]) snap[side].push_back(0u);
                else {
                    const auto* im = ptr<std::uint32_t>(sl[0]);
                    for (int k = 0; k < 14; ++k) snap[side].push_back(k >= 4 && k <= 6 ? (im[k] ? 1u : 0u) : im[k]);
                    snap[side].push_back(fnv(ptr<void>(im[4]), 2 * static_cast<std::size_t>(im[0])));
                    made.push_back(sl[0]);
                }
            }
            snap[side].push_back(bimg[7]);  // base image +0x1C untouched
            chained += side && count > static_cast<std::uint32_t>(n);
            for (std::uint32_t m : made) {
                const auto* im = ptr<std::uint32_t>(m);
                for (int k = 4; k <= 6; ++k) if (im[k]) real_free(ptr<void>(im[k]));
                real_free(ptr<void>(m));
            }
            m_fclose(ptr<void>(recs[0x80 / 4]));
            real_free(tocbuf);
            real_free(recs);
            real_free(recs_b);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  TextureSlot_LoadMipChain calls %d, %d chains grew; abnormal endings by existing level slot (none / pending / loaded): %d %d %d\n",
                compared, chained, faults[0], faults[1], faults[2]);
}

// TextureManager_LoadPending (0x0046de50): TextureArchiveListA_OpenBySize; for each slot in state 2 or 3: Image_Load
// into +0, else the loader callback [0x0053d788], else the default image 0x004e06e0; +0x20 cleared;
// TextureSlot_LoadMipChain; software (device [0x0056bbe8] 0, or Texture_IsSoftwareOnly) ->
// Image_PrepareSoftwareSampling; hardware: state 3 -> re-upload hook [0x0056bc18](binding +4, 0, image); state 2
// with no binding -> create hook [0x0056bc08](slot name, image, flags) into +4, then Image_FreeOwnedBuffers unless
// device 2; state 1; finally TextureArchive_CloseAllFiles. Real archive (list A, one open record) holding some slot
// names, slots in states 0..3 with or without bindings, the loader callback absent / answering / failing, devices
// 0/1/2, software-only list; hooks logged. Compared: the log, every slot word (images by role / content).
namespace {
std::uint32_t g_cb_img[16];
std::uint32_t __fastcall fake_loader(const char* name, int)
{
    g_log.push_back(7);
    return name[0] == 'c' ? addr(g_cb_img) : 0u;  // answers for names starting with 'c'
}
std::uint32_t __fastcall fake_create(std::uint32_t, std::uint32_t, int a, int b, int c)
{
    g_log.push_back(8); g_log.push_back(static_cast<std::uint32_t>(a)); g_log.push_back(static_cast<std::uint32_t>(b)); g_log.push_back(static_cast<std::uint32_t>(c));
    return 0xB1D00000u + static_cast<std::uint32_t>(g_log.size());
}
void __fastcall fake_reupload(std::uint32_t binding, int, std::uint32_t)
{
    g_log.push_back(9); g_log.push_back(binding);
}
}  // namespace

TEST(native_texture_manager_load_pending_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(int, int);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    using Fclose = int(__cdecl*)(void*);
    const Fn fn[2] = {rt::original<Fn>(0x0046de50), reinterpret_cast<Fn>(&recoil::TextureManager_LoadPending)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    char tmp[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    GetTempFileNameA(tmp, "tlp", 0, path);
    const char* names[] = {"sky", "tank", "cone", "grid", "void"};  // archive holds some; 'c...' answered by the callback
    std::mt19937 rng(0x46de50);
    int compared = 0;
    for (int it = 0; it < 600; ++it) {
        std::vector<int> in_arc;
        for (int k = 0; k < 5; ++k) if (rng() % 2) in_arc.push_back(k);
        std::vector<unsigned char> file(24 + 0x28 * (in_arc.size() ? in_arc.size() : 1));
        for (std::size_t e = 0; e < in_arc.size(); ++e) {
            unsigned char* ent = &file[24 + 0x28 * e];
            std::memset(ent, 0, 0x28);
            std::strcpy(reinterpret_cast<char*>(ent), names[in_arc[e]]);
            const std::uint32_t off = static_cast<std::uint32_t>(file.size());
            const int minus1 = -1;
            std::memcpy(ent + 0x20, &off, 4);
            std::memcpy(ent + 0x24, &minus1, 4);
            unsigned char hdr[16] = {};
            hdr[0] = static_cast<unsigned char>(1 | (rng() % 2 ? 0x02 : 0)); hdr[4] = 4; hdr[6] = 4;
            hdr[0xE] = static_cast<unsigned char>(rng() % 4);  // word +0xC: its bits 0/1 go to the create hook
            file.insert(file.end(), hdr, hdr + 16);
            for (int k = 0; k < 32; ++k) file.push_back(static_cast<unsigned char>(rng()));
        }
        void* wf = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), wf);
        m_fclose(wf);
        const int n = 1 + static_cast<int>(rng() % 5);
        int sname[5];
        std::uint32_t st[5], bind[5];
        for (int s = 0; s < 5; ++s) { sname[s] = static_cast<int>(rng() % 5); st[s] = rng() % 4; bind[s] = rng() % 3 ? 0u : 0xB100u + static_cast<std::uint32_t>(s); }
        const std::uint32_t device = rng() % 3;
        const bool loader = rng() % 2, sw_listed = rng() % 4 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, 6);
            *img(side, 0x0053d780) = 0;
            *img(side, 0x004e073c) = 1;
            *img(side, 0x0053d794) = 0;
            *img(side, 0x0056bbe8) = device;
            *img(side, 0x0053d788) = loader ? addr(reinterpret_cast<void*>(&fake_loader)) : 0u;
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc08))) = reinterpret_cast<void*>(&fake_create);
            *static_cast<void**>(static_cast<void*>(img(side, 0x0056bc18))) = reinterpret_cast<void*>(&fake_reupload);
            // Texture_IsSoftwareOnly is asked with the SLOT (EDI = slot): list slot 0 sometimes
            for (int k = 0; k < 3; ++k) *img(side, 0x0057d9a8u + 4 * k) = sw_listed && k == 0 ? addr(img(side, 0x0053d79c)) : 0u;
            std::memset(g_cb_img, 0, sizeof g_cb_img);
            reinterpret_cast<std::int16_t*>(g_cb_img)[2] = 4;
            auto* recs = static_cast<std::uint32_t*>(real_malloc(0xA4));
            std::memset(recs, 0, 0xA4);
            auto* tocbuf = static_cast<unsigned char*>(real_malloc(0x28 * (in_arc.size() ? in_arc.size() : 1)));
            std::memcpy(tocbuf, &file[24], 0x28 * in_arc.size());
            recs[0x80 / 4] = addr(m_fopen(path, "rb"));  // closed by TextureArchive_CloseAllFiles at the end
            recs[0x90 / 4] = static_cast<std::uint32_t>(in_arc.size());
            recs[0x9C / 4] = addr(tocbuf);
            *img(side, 0x0053d768) = 1;
            *img(side, 0x0053d76c) = addr(recs);
            auto* recs_b = static_cast<std::uint32_t*>(real_malloc(0xA4));
            std::memset(recs_b, 0, 0xA4);
            *img(side, 0x0053d770) = 1;
            *img(side, 0x0053d774) = addr(recs_b);
            auto* table = img(side, 0x0053d79c);
            std::memset(table, 0, 0x24 * 6);
            for (int s = 0; s < n; ++s) {
                std::strcpy(reinterpret_cast<char*>(table + 9 * s + 2), names[sname[s]]);
                table[9 * s + 7] = st[s];
                table[9 * s + 1] = bind[s];
                table[9 * s + 8] = 0x5A5Au;
            }
            *img(side, 0x0053d798) = static_cast<std::uint32_t>(n);
            g_log.clear();
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](0, 0)));
            const std::uint32_t def = addr(img(side, 0x004e06e0));
            std::vector<std::uint32_t> made;
            for (int s = 0; s < n; ++s) {
                const std::uint32_t* sl = table + 9 * s;
                for (int k = 1; k < 9; ++k) snap[side].push_back(k == 1 && sl[1] >= 0xB1D00000u ? 0xB1D0u : sl[k]);
                if (sl[0] == def) snap[side].push_back(0xDEFu);
                else if (sl[0] == addr(g_cb_img)) snap[side].push_back(0xCBu);
                else if (!sl[0]) snap[side].push_back(0u);
                else {
                    const auto* im = ptr<std::uint32_t>(sl[0]);
                    for (int k = 0; k < 14; ++k) snap[side].push_back(k >= 4 && k <= 6 ? (im[k] ? 1u : 0u) : im[k]);
                    if (im[4]) snap[side].push_back(fnv(ptr<void>(im[4]), 2 * static_cast<std::size_t>(im[0])));
                    made.push_back(sl[0]);
                }
            }
            snap[side].insert(snap[side].end(), g_log.begin(), g_log.end());
            snap[side].push_back(recs[0x80 / 4]);  // closed and nulled
            for (std::uint32_t m : made) {
                const auto* im = ptr<std::uint32_t>(m);
                for (int k = 4; k <= 6; ++k) if (im[k]) real_free(ptr<void>(im[k]));
                real_free(ptr<void>(m));
            }
            if (recs[0x80 / 4]) m_fclose(ptr<void>(recs[0x80 / 4]));
            real_free(tocbuf);
            real_free(ptr<void>(*img(side, 0x0053d76c)));
            real_free(recs_b);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  TextureManager_LoadPending calls %d (slots in states 0..3, archive hits / callback / default, devices 0..2, hooks logged)\n", compared);
}

// Font_LoadAll (0x0046efe0, config path ECX): ConfigTree_ParseFileByBasename (failure -> report, -1); the font search
// path "..\data\common\fonts" (TextureManager_AddSearchPaths); ConfigTree_FindChild(root, "FONTS") (none -> report,
// -1); one malloc'd block of (count - 1) 0x5F8-byte font records; per font name: the record goes into the texture
// table 0x0056179c, Image_Load(name) into it, and a found image gets flag 0x02 and Font_ScanGlyphs (a count other than
// 95 is reported); ConfigTree_Destroy; 0. Each side: its own reader list holding a real Zar archive (on disk, real
// SetFilePointer / ReadFile in both import tables) with fonts.zrd = {.., "FONTS", [font names]} (or without FONTS, or
// no such entry), and a texture archive in list A with some of the font images (glyph columns separated by empty
// ones). Compared: return, the texture table (entries as record indices), each record (image by role) and images.
TEST(native_font_load_all_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const char*, int);
    using C = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    using Fclose = int(__cdecl*)(void*);
    const Fn fn[2] = {rt::original<Fn>(0x0046efe0), reinterpret_cast<Fn>(&recoil::Font_LoadAll)};
    const C init[2] = {rt::original<C>(0x0048c7d0), reinterpret_cast<C>(&recoil::Container_InitNodePool)};
    const C create[2] = {rt::original<C>(0x0048c950), reinterpret_cast<C>(&recoil::Container_CreateList)};
    const C append[2] = {rt::original<C>(0x0048ca30), reinterpret_cast<C>(&recoil::Container_ListAppend)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    void** o_slot[2] = {ptr<void*>(0x004cc144), ptr<void*>(0x004cc134)};  // SetFilePointer, ReadFile: real ones
    void* const real_k[2] = {recoil::g_Iat_SetFilePointer_004cc144, recoil::g_Iat_ReadFile_004cc134};
    void* saved_k[2] = {*o_slot[0], *o_slot[1]};
    for (int i = 0; i < 2; ++i) *o_slot[i] = real_k[i];
    char tmp[MAX_PATH], zar_path[MAX_PATH], tex_path[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    GetTempFileNameA(tmp, "fnz", 0, zar_path);
    GetTempFileNameA(tmp, "fnt", 0, tex_path);
    const char* fonts[] = {"font_a.pcx", "font_b.pcx", "font_c.pcx"};
    std::mt19937 rng(0x46efe0);
    int compared = 0, loaded = 0;
    for (int it = 0; it < 300; ++it) {
        // fonts.zrd: root list {1 + ...: "FONTS", list [names]} in the ConfigTree_ReadNode stream format
        const int nf = 1 + static_cast<int>(rng() % 3);
        const int cfg = rng() % 8 == 0 ? static_cast<int>(rng() % 2) + 1 : 0;  // 1 no FONTS, 2 no fonts.zrd entry
        std::vector<unsigned char> zrd;
        auto put = [&](std::uint32_t v) { const auto* b = reinterpret_cast<const unsigned char*>(&v); zrd.insert(zrd.end(), b, b + 4); };
        auto str = [&](const char* s) { put(3); put(static_cast<std::uint32_t>(std::strlen(s))); zrd.insert(zrd.end(), s, s + std::strlen(s)); };
        put(4); put(3);
        str(cfg == 1 ? "NOTFONTS" : "FONTS");
        put(4); put(static_cast<std::uint32_t>(nf + 1));
        for (int k = 0; k < nf; ++k) str(fonts[k]);
        std::vector<unsigned char> zar(zrd);
        std::vector<unsigned char> tocent(0x94, 0);
        std::strcpy(reinterpret_cast<char*>(&tocent[8]), cfg == 2 ? "other.zrd" : "fonts.zrd");
        const std::uint32_t zsize = static_cast<std::uint32_t>(zrd.size());
        std::memcpy(&tocent[4], &zsize, 4);
        zar.insert(zar.end(), tocent.begin(), tocent.end());
        const std::uint32_t footer[2] = {1, 1};
        zar.insert(zar.end(), reinterpret_cast<const unsigned char*>(footer), reinterpret_cast<const unsigned char*>(footer) + 8);
        { FILE* f = std::fopen(zar_path, "wb"); std::fwrite(zar.data(), 1, zar.size(), f); std::fclose(f); }
        // texture archive: some fonts, 16-bit images of 95 glyphs (3 columns + 1 gap, a few gaps missing)
        std::vector<int> in_arc;
        for (int k = 0; k < nf; ++k) if (rng() % 4) in_arc.push_back(k);
        std::vector<unsigned char> tex(24 + 0x28 * (in_arc.size() ? in_arc.size() : 1));
        for (std::size_t e = 0; e < in_arc.size(); ++e) {
            unsigned char* ent = &tex[24 + 0x28 * e];
            std::memset(ent, 0, 0x28);
            std::strcpy(reinterpret_cast<char*>(ent), fonts[in_arc[e]]);
            const std::uint32_t off = static_cast<std::uint32_t>(tex.size());
            const int minus1 = -1;
            std::memcpy(ent + 0x20, &off, 4);
            std::memcpy(ent + 0x24, &minus1, 4);
            const int w = 95 * 4, h = 3;
            unsigned char hdr[16] = {};
            hdr[0] = 1; hdr[4] = static_cast<unsigned char>(w & 0xFF); hdr[5] = static_cast<unsigned char>(w >> 8); hdr[6] = h;
            tex.insert(tex.end(), hdr, hdr + 16);
            const int drop = rng() % 3 == 0 ? static_cast<int>(rng() % 95) : -1;
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    const bool gap = x % 4 == 3 && x / 4 != drop;
                    const std::uint16_t px = gap ? 0 : static_cast<std::uint16_t>(0x1000 | (rng() & 0x0FFF));
                    tex.push_back(static_cast<unsigned char>(px)); tex.push_back(static_cast<unsigned char>(px >> 8));
                }
        }
        void* wf = m_fopen(tex_path, "wb");
        m_fwrite(tex.data(), 1, tex.size(), wf);
        m_fclose(wf);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            set_formats(side, 6);
            *img(side, 0x0053d780) = 0;
            *img(side, 0x0053d794) = 0;
            init[side](4, 0);
            // the reader list: one Zar archive object on a real handle
            const HANDLE h = CreateFileA(zar_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
            auto* ztoc = static_cast<unsigned char*>(real_malloc(0x94));
            std::memcpy(ztoc, tocent.data(), 0x94);
            std::uint32_t zobj[8] = {0, addr(h), 0, 1, 1, addr(ztoc), 0, 0};
            const std::uint32_t list = create[side](0, 0);
            append[side](list, addr(zobj));
            *img(side, 0x0056b184) = list;
            // texture list A: one open record; list B: one record without a file
            auto* recs = static_cast<std::uint32_t*>(real_malloc(0xA4));
            std::memset(recs, 0, 0xA4);
            auto* tocbuf = static_cast<unsigned char*>(real_malloc(0x28 * (in_arc.size() ? in_arc.size() : 1)));
            std::memcpy(tocbuf, &tex[24], 0x28 * in_arc.size());
            recs[0x80 / 4] = addr(m_fopen(tex_path, "rb"));
            recs[0x90 / 4] = static_cast<std::uint32_t>(in_arc.size());
            recs[0x9C / 4] = addr(tocbuf);
            *img(side, 0x0053d768) = 1;
            *img(side, 0x0053d76c) = addr(recs);
            auto* recs_b = static_cast<std::uint32_t*>(real_malloc(0xA4));
            std::memset(recs_b, 0, 0xA4);
            *img(side, 0x0053d770) = 1;
            *img(side, 0x0053d774) = addr(recs_b);
            auto* table = img(side, 0x0056179c);
            std::memset(table, 0, 4 * 20);
            // prime the heap: the record block is malloc'd uninitialised and glyphs past the scanned count stay unset
            { void* prime = real_malloc(0x5F8 * static_cast<std::size_t>(nf)); std::memset(prime, 0xCD, 0x5F8 * static_cast<std::size_t>(nf)); real_free(prime); }
            snap[side].push_back(static_cast<std::uint32_t>(fn[side]("..\\data\\fonts.zrd", 0)));
            const std::uint32_t block = table[0];
            for (int k = 0; k < 20; ++k) snap[side].push_back(table[k] ? (table[k] - block) / 0x5F8 : 0xFFFFFFFFu);
            std::vector<std::uint32_t> imgs;
            for (int k = 0; k < nf && block; ++k) {
                // a font whose image is missing shares its record with the next one: follow the table
                const auto* rec = ptr<std::uint32_t>(table[k]);
                if (!table[k]) break;
                // the record: image, average advance, then 16-byte glyph entries; glyph 94 is never written when
                // the scan finds only 94 glyphs (then the entry holds heap residue), so glyphs 0..93 are compared
                // a font whose image is missing leaves its record unwritten apart from the null image
                snap[side].push_back(rec[0] ? 1u : 0u);
                for (int w = 1; rec[0] && w < (8 + 16 * 94) / 4; ++w) snap[side].push_back(rec[w]);
                if (rec[0]) {
                    const auto* im = ptr<std::uint32_t>(rec[0]);
                    for (int j = 0; j < 14; ++j) snap[side].push_back(j >= 4 && j <= 6 ? (im[j] ? 1u : 0u) : im[j]);
                    imgs.push_back(rec[0]);
                }
            }
            loaded += side && !imgs.empty();
            std::sort(imgs.begin(), imgs.end());
            imgs.erase(std::unique(imgs.begin(), imgs.end()), imgs.end());
            for (std::uint32_t m : imgs) {
                const auto* im = ptr<std::uint32_t>(m);
                for (int j = 4; j <= 6; ++j) if (im[j]) real_free(ptr<void>(im[j]));
                real_free(ptr<void>(m));
            }
            if (block) real_free(ptr<void>(block));
            CloseHandle(h);
            real_free(ztoc);
            m_fclose(ptr<void>(recs[0x80 / 4]));
            real_free(tocbuf);
            real_free(recs);
            real_free(recs_b);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    for (int i = 0; i < 2; ++i) *o_slot[i] = saved_k[i];
    DeleteFileA(zar_path);
    DeleteFileA(tex_path);
    rt::restore_pristine();
    std::printf("  Font_LoadAll calls %d, %d loaded fonts (1..3 fonts, some missing, glyph counts off by one, no FONTS, no fonts.zrd)\n", compared, loaded);
}

// TextureTable_Init (0x0046eb20, font config path ECX or 0): [0x0053d790] = 2, clears the 20-entry texture table
// 0x0056179c and [0x005617f4], calls Font_LoadAll(path) when given one, zeroes [0x005617ec], and points [0x005617f0]
// at the settings node "TextureMemory_HW" (device [0x0056bbe8] set) or "TextureMemory_SW" (Settings_FindNodeByName),
// else at 0x005617ec. Each side: random table / global contents, device 0/1, the matching node registered or not
// (Settings_RegisterNode, sometimes under the other name), path 0 or a config Font_LoadAll cannot find (no reader
// list). Compared: return, the globals (the node pointer as "the registered node" / "the local default" / other),
// the table.
TEST(native_texture_table_init_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const char*, int);
    using F4 = int(__fastcall*)(int, int, int, int);
    const Fn fn[2] = {rt::original<Fn>(0x0046eb20), reinterpret_cast<Fn>(&recoil::TextureTable_Init)};
    const F4 reg[2] = {rt::original<F4>(0x004b2e80), reinterpret_cast<F4>(&recoil::Settings_RegisterNode)};
    using F2 = int(__fastcall*)(int, int);
    const F2 shutdown[2] = {rt::original<F2>(0x004b32c0), reinterpret_cast<F2>(&recoil::Settings_Shutdown)};  // empties the node list
    // the settings node list state (named block 0x0056bcd0, 0x18 bytes) as it stands now, restored on each side
    // before every call: restore_pristine does not reset it
    std::uint32_t list_state[2][6];
    for (int side = 0; side < 2; ++side) std::memcpy(list_state[side], img(side, 0x0056bcd0), sizeof list_state[side]);
    std::mt19937 rng(0x46eb20);
    int compared = 0, found = 0;
    for (int it = 0; it < 1000; ++it) {
        const std::uint32_t device = rng() % 2;
        const int which = static_cast<int>(rng() % 3);  // 0 none, 1 the matching name, 2 the other name
        const bool with_path = rng() % 3 == 0;
        std::uint32_t junk[24];
        for (auto& x : junk) x = rng() | 0x80000000u;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(img(side, 0x0056bcd0), list_state[side], sizeof list_state[side]);
            for (int k = 0; k < 20; ++k) *img(side, 0x0056179cu + 4 * k) = junk[k];
            *img(side, 0x005617f4) = junk[20];
            *img(side, 0x005617ec) = junk[21];
            *img(side, 0x0053d790) = junk[22];
            *img(side, 0x0056bbe8) = device;
            *img(side, 0x0056b184) = 0;
            int node = 0;
            if (which) {
                const bool hw = (device != 0) == (which == 1);
                node = reg[side](addr(hw ? "TextureMemory_HW" : "TextureMemory_SW"), 1, 4, 0);
            }
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](with_path ? "..\\data\\nofonts.zrd" : nullptr, 0)));
            const std::uint32_t p = *img(side, 0x005617f0);
            snap[side].push_back(p == addr(img(side, 0x005617ec)) ? 0xDEFu : (node && p == static_cast<std::uint32_t>(node)) ? 0x40DEu : p);
            for (std::uint32_t va : {0x0053d790u, 0x005617f4u, 0x005617ecu}) snap[side].push_back(*img(side, va));
            for (int k = 0; k < 20; ++k) snap[side].push_back(*img(side, 0x0056179cu + 4 * k));
            found += side && snap[side][1] == 0x40DEu;
            shutdown[side](0, 0);  // the node list lives in named blocks: empty it for the next iteration
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    CHECK(found > 100);
    std::printf("  TextureTable_Init calls %d, %d found the node (device 0/1, node registered / other name / none, font path or not)\n", compared, found);
}

TEST(native_image_read_header_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*);
    using Fopen = void*(__cdecl*)(const char*, const char*);
    using Fclose = int(__cdecl*)(void*);
    using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
    const Fn fn[2] = {rt::original<Fn>(0x0046ed70), reinterpret_cast<Fn>(&recoil::Image_ReadHeader)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "irh", 0, path);
    std::mt19937 rng(0x46ed70);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        unsigned char hdr[16];
        for (auto& c : hdr) c = static_cast<unsigned char>(rng());
        const int kind = static_cast<int>(rng() % 10);  // 0 null FILE, 1 null image, else a real read
        void* w = m_fopen(path, "wb");
        m_fwrite(hdr, 1, 16, w);
        m_fclose(w);
        std::uint32_t init[16];  // Image_SetSize writes up to +0x34
        for (auto& x : init) x = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t image[16];
            std::memcpy(image, init, sizeof image);
            void* f = kind == 0 ? nullptr : m_fopen(path, "rb");
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](f, kind == 1 ? nullptr : image)));
            if (f) m_fclose(f);
            snap[side].insert(snap[side].end(), image, image + 16);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    std::printf("  Image_ReadHeader calls %d (real 16-byte headers, null FILE / image)\n", compared);
}
