// L2-style native test for NetGraph_LoadFromZrdByIndex (0x00403040; P3 ai_net, ported in the cloud) and
// NetGraph_FreeAll (0x00403870) on the shipped mission archives (00_original/game_install/zbd/m1..m13/zrdr.zbd), with
// the real KERNEL32 file calls bound into both import tables (as tests/test_asset_io_l2_native.cpp):
// LoadFromZrdByIndex(i) sprintf's the entry name from the formats at 0x004da23c / 0x004da234, parses it
// (ConfigTree_ParseFileByBasename; 0 when missing), checks the type key, appends a graph (NetGraph_AllocAndAppend) and
// fills it from the tree's keys (name +4, mode +0x18, floats +0x1C..+0x48 with their defaults), builds its nodes and
// links them (NetGraph_LinkNodes); FreeAll frees every graph (NetGraph_Free: nodes, edges, the graph) and clears the
// list head. For each archive the graphs are loaded on each side from a pristine state with its own node pool and
// reader list; NetGraph_LoadAllForMission (0x00402fd0) loads indices 1..99 after index 0 on its own. Compared: the returns, every graph (words, with the node chain and next graph by role), every node
// (words, links by role, edges' 15 words), the list globals, and the number of blocks FreeAll frees (logged free).
#include "test.h"
#include "cloud_harness.h"
#include "Battlesport/ai_net.h"
#include "Battlesport/player.h"
#include "GameZRecoil/zReader/zreader.h"
#include "platform/iat_kernel32.h"

#include <cstdint>
#include <random>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

TEST(native_ai_net_load_graphs_from_mission_archives)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn open[2] = {rt::original<Fn>(0x0048d210), reinterpret_cast<Fn>(&recoil::Zar_OpenAndRegisterArchive)};
    const Fn close_all[2] = {rt::original<Fn>(0x0048d2c0), reinterpret_cast<Fn>(&recoil::Reader_CloseAll)};
    const Fn load[2] = {rt::original<Fn>(0x00403040), reinterpret_cast<Fn>(&recoil::NetGraph_LoadFromZrdByIndex)};
    const Fn free_all[2] = {rt::original<Fn>(0x00403870), reinterpret_cast<Fn>(&recoil::NetGraph_FreeAll)};
    const Fn load_all[2] = {rt::original<Fn>(0x00402fd0), reinterpret_cast<Fn>(&recoil::NetGraph_LoadAllForMission)};
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
    std::string dir = RECOIL_ORIGINAL_EXE;
    dir = dir.substr(0, dir.find_last_of("/\\") + 1) + "zbd/";
    int graphs = 0, nodes_seen = 0;
    for (int m = 1; m <= 13; ++m) {
        const std::string path = dir + "m" + std::to_string(m) + "/zrdr.zbd";
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            *ch::img(side, 0x0056b184) = create[side](0, 0);
            *ch::img(side, 0x0056b188) = 0;
            open[side](ch::addr(path.c_str()), 1);
            *ch::img(side, 0x004e5c58) = 0;
            *ch::img(side, 0x004e5c5c) = 0;
            // index 0 on its own, then NetGraph_LoadAllForMission (0x00402fd0: indices 1..99)
            ch::wipe_stack();
            snap[side].push_back(load[side](0, 0) ? 1u : 0u);
            ch::wipe_stack();
            load_all[side](0, 0);
            // the graphs and their nodes, pointers by role
            ch::Roles rl;
            std::vector<std::uint32_t> gs;
            for (std::uint32_t g = *ch::img(side, 0x004e5c58), guard = 0; g && guard < 64; g = ch::at(g)[0x54 / 4], ++guard) {
                rl.set(g, 0x1000 + static_cast<std::uint32_t>(gs.size()));
                gs.push_back(g);
            }
            std::uint32_t nid = 0;
            for (std::uint32_t g : gs)
                for (std::uint32_t n = ch::at(g)[0x50 / 4], guard = 0; n && guard < 4096; n = ch::at(n)[0x2c / 4], ++guard) rl.set(n, 0x2000 + nid++);
            for (std::uint32_t g : gs) {
                for (int w = 0; w < 0x58 / 4; ++w) snap[side].push_back(w == 0x50 / 4 || w == 0x54 / 4 ? rl(ch::at(g)[w]) : ch::at(g)[w]);
                for (std::uint32_t n = ch::at(g)[0x50 / 4], guard = 0; n && guard < 4096; n = ch::at(n)[0x2c / 4], ++guard) {
                    const std::uint32_t* w = ch::at(n);
                    for (int c = 0; c < 12; ++c) snap[side].push_back(c >= 3 && c < 6 || c == 11 ? rl(w[c]) : c >= 6 && c < 9 ? (w[c] ? 1u : 0u) : w[c]);
                    for (int s = 0; s < 3; ++s)
                        if (w[6 + s]) for (int e = 0; e < 15; ++e) snap[side].push_back(ch::at(w[6 + s])[e]);
                    if (side == 0) ++nodes_seen;
                }
                if (side == 0) ++graphs;
            }
            snap[side].push_back(rl(*ch::img(side, 0x004e5c5c)));
            ch::freed().clear();
            {
                ch::FreeHook hook;
                free_all[side](0, 0);
            }
            snap[side].push_back(static_cast<std::uint32_t>(ch::freed().size()));
            snap[side].push_back(*ch::img(side, 0x004e5c58));
            close_all[side](1, 0);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    for (int i = 0; i < 9; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(k_va[i])) = saved[i];
    rt::restore_pristine();
    std::printf("  NetGraph_LoadFromZrdByIndex / LoadAllForMission: 13 mission archives x indices 0..99, %d graphs, %d nodes (both sides)\n", graphs, nodes_seen);
    CHECK(graphs > 0);
}

// Movers_LoadFromConfig (0x00420be0; P3 player, cloud port): parses the config named at 0x004dc4d4
// (ConfigTree_ParseFileByBasename); for each string of its first list looks the node up in registry list 6
// (NodeRegistry_LookupByName) and, when found, ORs flag 1 into its +0x28 tree (Node_OrFlags28Recursive), sets owner =
// itself and flag 0x200000 over its tree (Node_SetOwnerAndFlagsRecursive) and keeps it in [0x004f3ab4]; destroys the
// tree. On the shipped mission archives, with registry list 6 holding nodes for a random part of the config's names
// (read from the original's parse of the same file) plus others, each with 0..2 children. Compared: every node's
// +0x24 / +0x28 / +0x40 words and [0x004f3ab4] by node index.
TEST(native_player_movers_load_from_config_on_mission_archives)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    using Fn3 = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t, std::uint32_t);
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn open[2] = {rt::original<Fn>(0x0048d210), reinterpret_cast<Fn>(&recoil::Zar_OpenAndRegisterArchive)};
    const Fn close_all[2] = {rt::original<Fn>(0x0048d2c0), reinterpret_cast<Fn>(&recoil::Reader_CloseAll)};
    const Fn movers[2] = {rt::original<Fn>(0x00420be0), reinterpret_cast<Fn>(&recoil::Movers_LoadFromConfig)};
    const Fn3 parse = rt::original<Fn3>(0x0048cdc0);
    const Fn destroy = rt::original<Fn>(0x0048ce40);
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
    std::string dir = RECOIL_ORIGINAL_EXE;
    dir = dir.substr(0, dir.find_last_of("/\\") + 1) + "zbd/";
    const std::string cfg = reinterpret_cast<const char*>(0x004dc4d4);
    std::mt19937 rng(0x420be0);
    int configs = 0, bound = 0;
    for (int m = 1; m <= 13; ++m) {
        const std::string path = dir + "m" + std::to_string(m) + "/zrdr.zbd";
        // the config's names, from the original's own parse
        std::vector<std::string> names;
        rt::restore_pristine();
        init[0](4, 0);
        *ch::img(0, 0x0056b184) = create[0](0, 0);
        *ch::img(0, 0x0056b188) = 0;
        open[0](ch::addr(path.c_str()), 1);
        if (const std::uint32_t tree = parse(ch::addr(cfg.c_str()), 0, 0)) {
            // root list value -> child 1 {type, value} at +8: a list whose children 1.. are the names
            const std::uint32_t* root = ch::at(ch::at(tree)[1]);
            const std::uint32_t list = root[2] == 4 ? root[3] : 0u;
            if (list)
                for (std::uint32_t i = 0; i + 1 < ch::at(list)[1]; ++i) {
                    const std::uint32_t s = ch::at(list)[2 * i + 3];
                    if (s) names.push_back(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(s)));
                }
            destroy(tree, 0);
            ++configs;
        }
        close_all[0](1, 0);
        for (int round = 0; round < 10; ++round) {
            std::vector<std::string> reg;
            for (const std::string& s : names) if (rng() % 2) reg.push_back(s);
            reg.push_back("not_a_mover");
            std::vector<int> kids(reg.size());
            for (int& k : kids) k = static_cast<int>(rng() % 3);
            std::vector<std::uint32_t> f24(3 * reg.size()), f28(3 * reg.size());
            for (auto& f : f24) f = rng();
            for (auto& f : f28) f = rng();
            std::vector<std::uint32_t> snap[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                init[side](4, 0);
                *ch::img(side, 0x0056b184) = create[side](0, 0);
                *ch::img(side, 0x0056b188) = 0;
                open[side](ch::addr(path.c_str()), 1);
                // each registered node with its children (3 node slots per name), and the registry links
                std::vector<std::vector<std::uint32_t>> node(3 * reg.size(), std::vector<std::uint32_t>(0xc4 / 4, 0));
                std::vector<std::vector<std::uint32_t>> arr(reg.size());
                std::vector<std::uint32_t> links(4 * reg.size(), 0);
                for (std::size_t r = 0; r < reg.size(); ++r) {
                    std::strncpy(reinterpret_cast<char*>(node[3 * r].data()), reg[r].c_str(), 0x20);
                    for (int c = 0; c < 3; ++c) {
                        node[3 * r + c][0x24 / 4] = f24[3 * r + c];
                        node[3 * r + c][0x28 / 4] = f28[3 * r + c];
                    }
                    for (int c = 1; c <= kids[r]; ++c) arr[r].push_back(ch::addr(node[3 * r + c].data()));
                    node[3 * r][0x5c / 4] = static_cast<std::uint32_t>(arr[r].size());
                    node[3 * r][0x60 / 4] = arr[r].empty() ? 0u : ch::addr(arr[r].data());
                    std::uint32_t* l = &links[4 * r];
                    l[0] = ch::addr(node[3 * r].data());
                    l[1] = r ? ch::addr(&links[4 * (r - 1)]) : 0u;
                    l[2] = r + 1 < reg.size() ? ch::addr(&links[4 * (r + 1)]) : 0u;
                }
                std::uint32_t* holder = ch::at(*ch::img(side, 0x004ddef8 + 4 * 6));
                const std::uint32_t saved_head = *holder;
                *holder = ch::addr(links.data());
                *ch::img(side, 0x004f3ab4) = 0;
                ch::wipe_stack();
                movers[side](0, 0);
                *holder = saved_head;
                for (auto& n : node) {
                    snap[side].push_back(n[0x24 / 4]);
                    snap[side].push_back(n[0x28 / 4]);
                    std::uint32_t owner = 0;
                    for (std::size_t k = 0; k < node.size(); ++k) if (n[0x40 / 4] == ch::addr(node[k].data())) owner = static_cast<std::uint32_t>(k + 1);
                    snap[side].push_back(n[0x40 / 4] ? owner : 0u);
                }
                const std::uint32_t kept = *ch::img(side, 0x004f3ab4);
                std::uint32_t kept_i = kept ? 0xbad : 0;
                for (std::size_t k = 0; k < node.size(); ++k) if (kept == ch::addr(node[k].data())) kept_i = static_cast<std::uint32_t>(k + 1);
                snap[side].push_back(kept_i);
                if (side == 0) bound += kept != 0;
                close_all[side](1, 0);
            }
            CHECK_SNAP(snap[0], snap[1]);
        }
    }
    for (int i = 0; i < 9; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(k_va[i])) = saved[i];
    rt::restore_pristine();
    std::printf("  Movers_LoadFromConfig: 13 mission archives x 10 registries, %d configs found, %d rounds with a mover bound\n", configs, bound);
}
