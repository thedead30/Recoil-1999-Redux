// Structured native L1s for the mission functions on images and search paths (P3 mission, ported in the cloud):
//  - MissionObjectives_Reset (0x00417f60): clears +0 and the first bytes of the three text buffers (+0x14, +0x114,
//    +0x214); a set image +0x10 goes to Image_FreeUnlessDefault and the slot is cleared. Images: null, the default
//    image 0x004e06e0, or a real 0x40-byte image with no buffers or surface; logged free.
//  - Mission_SetDataSearchPaths (0x0042ecb0; ECX mission id string): List_FreeAllPopped(0), the global string set gets
//    the path list at 0x004dcb70 (StringSet_AddSemicolonTokens), the texture set gets 0x004dcc3c
//    (TextureManager_AddSearchPaths), the search set is reset to the mission's paths (format 0x004dcbfc, the id twice;
//    SearchPath_InitOrReset) and, when [0x004f0da0] is set, the mission archive (format 0x004dcbe8) is opened
//    (Zar_OpenAndRegisterArchive, not kept). The real KERNEL32 file calls are bound into both import tables (as the L2
//    asset test); the working directory is the process's own, so the relative paths resolve the same for both sides.
//    Each side: its own node pool and reader list, 1..3 calls with ids "1".."13" / junk. Compared: the three string
//    sets (drained), whether the reader list holds archives.
#include "test.h"
#include "cloud_harness.h"
#include "Battlesport/mission.h"
#include "GameZRecoil/zReader/zreader.h"
#include "platform/iat_kernel32.h"
#include "unattributed/mission.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

TEST(native_mission_objectives_reset_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int);
    const F fn[2] = {rt::original<F>(0x00417f60), reinterpret_cast<F>(&recoil::MissionObjectives_Reset)};
    std::mt19937 rng(0x417f60);
    for (int it = 0; it < 500; ++it) {
        const int kind = static_cast<int>(rng() % 4);  // 0 null, 1 default image, else a real image
        std::vector<std::uint32_t> init(0x320 / 4);
        for (auto& w : init) w = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            const std::uint32_t def = side ? ch::addr(ch::img(1, 0x004e06e0)) : 0x004e06e0u;
            rl.set(def, 0xDEF);
            std::vector<std::uint32_t> obj = init;
            std::uint32_t image = 0;
            if (kind == 1) image = def;
            else if (kind > 1) {
                auto* w = static_cast<std::uint32_t*>(ch::c_malloc(0x40));
                std::memset(w, 0, 0x40);
                image = ch::addr(w);
                rl.set(image, 1);
            }
            obj[0x10 / 4] = image;
            ch::freed().clear();
            int ret;
            {
                ch::FreeHook hook;
                ret = fn[side](obj.data(), 0);
            }
            (void)ret;  // returns what EAX held
            for (std::size_t k = 0; k < obj.size(); ++k) snap[side].push_back(k == 0x10 / 4 ? rl(obj[k]) : obj[k]);
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            if (kind > 1 && !ch::was_freed(image)) ch::c_free(image);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

TEST(native_mission_set_data_search_paths_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn pop[2] = {rt::original<Fn>(0x0048cb70), reinterpret_cast<Fn>(&recoil::Container_ListPopCursor)};
    const Fn close_all[2] = {rt::original<Fn>(0x0048d2c0), reinterpret_cast<Fn>(&recoil::Reader_CloseAll)};
    const Fn fn[2] = {rt::original<Fn>(0x0042ecb0), reinterpret_cast<Fn>(&recoil::Mission_SetDataSearchPaths)};
    const std::uint32_t k_va[9] = {0x004cc130, 0x004cc14c, 0x004cc144, 0x004cc134, 0x004cc0e4, 0x004cc138, 0x004cc0dc, 0x004cc0f4, 0x004cc0fc};
    void* const k_port[9] = {recoil::g_Iat_CreateFileA_004cc130, recoil::g_Iat_GetFileSize_004cc14c, recoil::g_Iat_SetFilePointer_004cc144,
                             recoil::g_Iat_ReadFile_004cc134, recoil::g_Iat_CloseHandle_004cc0e4, recoil::g_Iat_WriteFile_004cc138,
                             recoil::g_Iat_GetLastError_004cc0dc, recoil::g_Iat_FormatMessageA_004cc0f4, recoil::g_Iat_LocalFree_004cc0fc};
    void* saved[9];
    for (int i = 0; i < 9; ++i) {
        void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(k_va[i]));
        saved[i] = *o;
        *o = k_port[i];
    }
    static const char* const ids[] = {"1", "2", "7", "13", "x", ""};
    std::mt19937 rng(0x42ecb0);
    int compared = 0;
    for (int it = 0; it < 200; ++it) {
        const int calls = 1 + static_cast<int>(rng() % 3);
        std::vector<int> which(calls);
        for (int& w : which) w = static_cast<int>(rng() % 6);
        const std::uint32_t open_archive = rng() % 2;
        std::vector<std::string> out[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            *ch::img(side, 0x0056b184) = create[side](0, 0);
            *ch::img(side, 0x0056b188) = 0;
            *ch::img(side, 0x004f0da0) = open_archive;
            for (int c = 0; c < calls; ++c) fn[side](ch::addr(ids[which[c]]), 0);
            for (std::uint32_t va : {0x0056b180u, 0x0056bb8cu, 0x0053d794u}) {
                const std::uint32_t l = *ch::img(side, va);
                out[side].push_back(l ? "<set>" : "<none>");
                for (std::uint32_t p; l && (p = pop[side](l, 0)) != 0;) {
                    out[side].push_back(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p)));
                    ch::c_free(p);
                }
            }
            const std::uint32_t readers = *ch::img(side, 0x0056b184);
            out[side].push_back(readers && ch::at(readers)[0] ? "<readers>" : "<no readers>");
            close_all[side](1, 0);
        }
        CHECK(out[0] == out[1]);
        ++compared;
    }
    for (int i = 0; i < 9; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(k_va[i])) = saved[i];
    rt::restore_pristine();
    std::printf("  Mission_SetDataSearchPaths sequences %d (real files, own node pool per side)\n", compared);
}
