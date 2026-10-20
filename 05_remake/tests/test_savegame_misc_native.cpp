// Structured native L1 for savegame helpers ported in the cloud (P2 savegame).
//   TempFile_FromBuffer (0x004c0780; stack: buffer, size; ret 8): tmpfile(), fwrite the buffer, fflush, rewind; returns
//     the FILE. Compared: its contents read back and the position.
//   TempFile_RemoveAll (0x004c07c0; one ignored stack word; ret 4): _rmtmp() - the count of temporary files it closed.
//     0..3 tmpfiles open before the call. Compared: the return.
//   SaveHandler_CallSave (0x004c06a0; ECX handler, one stack argument; ret 4): the save callback +4 (when set) with
//     ECX = the argument, EDX = the user word +0x10; its return; else 1.
//   SaveHandler_CallLoad (0x004c06c0; ECX handler, four stack arguments; ret 0x10): the load callback +8 (when set) with
//     ECX / EDX = the first two arguments and the third, the fourth and the user word on the stack; its return (else
//     whatever EAX held: the callback pointer, 0).
// Callbacks log what they receive. Compared: the returns and the logs.
#include "test.h"
#include "cloud_harness.h"
#include "unattributed/savegame.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

using namespace ch;

TEST(native_savegame_temp_files_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using From = void*(__fastcall*)(int, int, const void*, std::uint32_t);
    using Remove = int(__fastcall*)(int, int, int);
    const From from[2] = {rt::original<From>(0x004c0780), reinterpret_cast<From>(&recoil::TempFile_FromBuffer)};
    const Remove remove[2] = {rt::original<Remove>(0x004c07c0), reinterpret_cast<Remove>(&recoil::TempFile_RemoveAll)};
    auto c_tmpfile = crt_fn<void*(__cdecl*)()>("tmpfile");
    auto c_fread = crt_fn<std::size_t(__cdecl*)(void*, std::size_t, std::size_t, void*)>("fread");
    auto c_rmtmp = crt_fn<int(__cdecl*)()>("_rmtmp");
    std::mt19937 rng(0x4c0780);
    for (int it = 0; it < 400; ++it) {
        const int f = it % 2;
        std::vector<unsigned char> buf(rng() % 300);
        for (auto& b : buf) b = static_cast<unsigned char>(rng());
        const int open = static_cast<int>(rng() % 4);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            c_rmtmp();
            if (f == 0) {
                void* t = from[side](0, 0, buf.data(), static_cast<std::uint32_t>(buf.size()));
                snap[side].push_back(static_cast<std::uint32_t>(c_ftell(t)));
                std::vector<unsigned char> back(400, 0xCD);
                snap[side].push_back(static_cast<std::uint32_t>(c_fread(back.data(), 1, back.size(), t)));
                snap[side].insert(snap[side].end(), back.begin(), back.end());
            } else {
                for (int k = 0; k < open; ++k) c_tmpfile();
                snap[side].push_back(static_cast<std::uint32_t>(remove[side](0, 0, 0)));
            }
            c_rmtmp();
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

namespace {
std::vector<std::uint32_t> g_log;
int __fastcall save_cb(std::uint32_t a, std::uint32_t user) { g_log.push_back(a); g_log.push_back(user); return 0x51; }
int __fastcall load_cb(std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d, std::uint32_t user)
{
    g_log.push_back(a); g_log.push_back(b); g_log.push_back(c); g_log.push_back(d); g_log.push_back(user);
    return 0x1d;
}
}  // namespace

TEST(native_savegame_handler_calls_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Save = std::uint32_t(__fastcall*)(const std::uint32_t*, int, std::uint32_t);
    using Load = std::uint32_t(__fastcall*)(const std::uint32_t*, int, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
    const Save save[2] = {rt::original<Save>(0x004c06a0), reinterpret_cast<Save>(&recoil::SaveHandler_CallSave)};
    const Load load[2] = {rt::original<Load>(0x004c06c0), reinterpret_cast<Load>(&recoil::SaveHandler_CallLoad)};
    std::mt19937 rng(0x4c06a0);
    for (int it = 0; it < 2000; ++it) {
        const int f = it % 2;
        const bool set = rng() % 4 != 0;
        std::uint32_t h[5] = {rng(), set ? addr(reinterpret_cast<void*>(&save_cb)) : 0u, set ? addr(reinterpret_cast<void*>(&load_cb)) : 0u, rng(), rng()};
        const std::uint32_t a[4] = {rng(), rng(), rng(), rng()};
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            g_log.clear();
            const std::uint32_t r = f ? load[side](h, 0, a[0], a[1], a[2], a[3]) : save[side](h, 0, a[0]);
            snap[side] = g_log;
            snap[side].push_back(r);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

// The save-system wrappers on the active save object [0x0056bf70] (null -> nothing): SaveGame_FinishIfActive
// (0x004c0070: SaveGame_SetDirty30, +0x30 = 1), SaveSystem_WriteToTempFile (0x004c00c0; ECX buffer, EDX size:
// TempFile_FromBuffer, else 0), SaveSystem_RemoveTempFiles (0x004c00e0: TempFile_RemoveAll; EAX is the argument when
// there is no object). Compared: the returns, the object's words, the temp file's contents.
TEST(native_savegame_system_wrappers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const void*, std::uint32_t);
    const Fn finish[2] = {rt::original<Fn>(0x004c0070), reinterpret_cast<Fn>(&recoil::SaveGame_FinishIfActive)};
    const Fn write[2] = {rt::original<Fn>(0x004c00c0), reinterpret_cast<Fn>(&recoil::SaveSystem_WriteToTempFile)};
    const Fn remove[2] = {rt::original<Fn>(0x004c00e0), reinterpret_cast<Fn>(&recoil::SaveSystem_RemoveTempFiles)};
    auto c_fread = crt_fn<std::size_t(__cdecl*)(void*, std::size_t, std::size_t, void*)>("fread");
    auto c_rmtmp = crt_fn<int(__cdecl*)()>("_rmtmp");
    auto c_tmpfile = crt_fn<void*(__cdecl*)()>("tmpfile");
    std::mt19937 rng(0x4c0070);
    for (int it = 0; it < 600; ++it) {
        const int f = it % 3;
        const bool active = rng() % 4 != 0;
        std::vector<unsigned char> buf(rng() % 100);
        for (auto& b : buf) b = static_cast<unsigned char>(rng());
        std::uint32_t obj_init[16];
        for (auto& w : obj_init) w = rng();
        const int open = static_cast<int>(rng() % 3);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            c_rmtmp();
            std::uint32_t obj[16];
            std::memcpy(obj, obj_init, sizeof obj);
            *img(side, 0x0056bf70) = active ? addr(obj) : 0u;
            if (f == 0) { finish[side](nullptr, 0); snap[side].assign(obj, obj + 16); }
            if (f == 1) {
                void* t = reinterpret_cast<void*>(static_cast<std::uintptr_t>(write[side](buf.data(), static_cast<std::uint32_t>(buf.size()))));
                snap[side].push_back(t ? 1u : 0u);
                if (t) {
                    std::vector<unsigned char> back(128, 0xCD);
                    snap[side].push_back(static_cast<std::uint32_t>(c_fread(back.data(), 1, back.size(), t)));
                    snap[side].insert(snap[side].end(), back.begin(), back.end());
                }
            }
            if (f == 2) {
                for (int k = 0; k < open; ++k) c_tmpfile();
                snap[side].push_back(active ? remove[side](nullptr, 0) : 0u);
            }
            c_rmtmp();
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// List_MergeByLess (0x004c0bd0; ECX destination list, one stack argument: source list; ret 4): a std::list merge of
// two circular doubly linked lists (list object +4 head sentinel, +8 size; node +0 next, +4 prev, data from +8) ordered
// by SaveHandler_Less (key at data +0xc, signed <): source nodes are spliced before the first destination node that is
// not less; the rest are appended; the sizes are summed and the source's zeroed. The same list twice -> nothing.
// Sorted lists of 0..5 nodes with equal keys (stability). Compared: the node order (by identity) and keys of both lists,
// the sizes.
TEST(native_savegame_list_merge_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t*, int, std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x004c0bd0), reinterpret_cast<Fn>(&recoil::List_MergeByLess)};
    std::mt19937 rng(0x4c0bd0);
    for (int it = 0; it < 3000; ++it) {
        int n[2] = {static_cast<int>(rng() % 6), static_cast<int>(rng() % 6)};
        std::vector<int> keys[2];
        for (int l = 0; l < 2; ++l) { for (int k = 0; k < n[l]; ++k) keys[l].push_back(static_cast<int>(rng() % 7) - 2); std::sort(keys[l].begin(), keys[l].end()); }
        const bool same = rng() % 15 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            static std::uint32_t nodes[2][7][8], lists[2][3];
            for (int l = 0; l < 2; ++l) {
                std::uint32_t* head = nodes[l][6];
                std::uint32_t* prev = head;
                for (int k = 0; k < n[l]; ++k) {
                    std::uint32_t* nd = nodes[l][k];
                    nd[2 + 3] = static_cast<std::uint32_t>(keys[l][k]);
                    nd[7] = 0x100u * l + k;  // identity
                    prev[0] = addr(nd); nd[1] = addr(prev); prev = nd;
                }
                prev[0] = addr(head); head[1] = addr(prev);
                head[7] = 0x999;
                lists[l][0] = 0; lists[l][1] = addr(head); lists[l][2] = static_cast<std::uint32_t>(n[l]);
            }
            fn[side](lists[0], 0, same ? lists[0] : lists[1]);
            for (int l = 0; l < 2; ++l) {
                snap[side].push_back(lists[l][2]);
                std::uint32_t p = at(lists[l][1])[0];
                for (int guard = 0; guard < 16 && p != lists[l][1]; ++guard, p = at(p)[0]) { snap[side].push_back(at(p)[7]); snap[side].push_back(at(p)[5]); }
                p = at(lists[l][1])[1];  // backwards too
                for (int guard = 0; guard < 16 && p != lists[l][1]; ++guard, p = at(p)[1]) snap[side].push_back(at(p)[7]);
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}
