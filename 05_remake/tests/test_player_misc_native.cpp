// Structured native L1s for the player functions on trees, heap lists and search paths (P3 player, ported in the cloud):
//  - Node_AndFlags28Recursive (0x00421d60), Node_OrFlags28Recursive (0x00421da0), Node_OrFlags24Recursive (0x00421de0):
//    ECX node, EDX mask; +0x28 &= mask / +0x28 |= mask / +0x24 |= mask on the node and, depth-first, every descendant
//    (+0x5C count, +0x60 array). Real trees (the arena fuzz would recurse without bound, KG-31).
//  - NameRegistry_Add (0x00438920): mallocs a 12-byte entry {ECX, EDX, next}, appends it to the list [0x004f3344] head /
//    [0x004f3348] tail / [0x004f334c] count.
//  - Player_VehicleConfigForDifficulty (0x0041fe50; ECX search path): by the game intensity ([[0x004e5d48]]: 0, 2,
//    other) picks one of the names at 0x004dc334 / 0x004dc348 / 0x004dc35c and returns it when Reader_OpenWithSearch
//    finds it on the path, else the name at 0x004dc35c.
//  - Path_GetDirectoryOfFound (0x00421e20; ECX name, EDX out): Reader_OpenWithSearch(name, no path) against the search
//    set [0x0056b180], _fullpath, _splitpath, sprintf drive + directory ("%s%s" at 0x004dc6c8) into the out buffer.
// For the two path functions each side builds its own node pool and search set with its own functions over one temp
// tree (the config names read from the mapped original image, created in some directories only). Compared: every
// node's words; the list and entries by role; returned names by content; the out buffers.
#include "test.h"
#include "cloud_harness.h"
#include "Battlesport/pickup.h"
#include "Battlesport/player.h"
#include "GameZRecoil/zReader/zreader.h"
#include "unattributed/player.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

TEST(native_player_node_flag_trees_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, std::uint32_t);
    const F fn[3][2] = {
        {rt::original<F>(0x00421d60), reinterpret_cast<F>(&recoil::Node_AndFlags28Recursive)},
        {rt::original<F>(0x00421da0), reinterpret_cast<F>(&recoil::Node_OrFlags28Recursive)},
        {rt::original<F>(0x00421de0), reinterpret_cast<F>(&recoil::Node_OrFlags24Recursive)},
    };
    std::mt19937 rng(0x421d60);
    for (int it = 0; it < 2000; ++it) {
        const int n = 1 + static_cast<int>(rng() % 30);
        std::vector<int> parent(n, -1);
        for (int k = 1; k < n; ++k) parent[k] = static_cast<int>(rng() % k);
        std::vector<std::uint32_t> f24(n), f28(n);
        for (int k = 0; k < n; ++k) { f24[k] = rng(); f28[k] = rng(); }
        const int which = static_cast<int>(rng() % 3), root = static_cast<int>(rng() % n);
        const std::uint32_t mask = rng() % 3 ? 1u << (rng() % 32) : rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<std::vector<std::uint32_t>> node(n, std::vector<std::uint32_t>(0xc4 / 4, 0)), kids(n);
            for (int k = 1; k < n; ++k) kids[parent[k]].push_back(ch::addr(node[k].data()));
            for (int k = 0; k < n; ++k) {
                node[k][0x24 / 4] = f24[k];
                node[k][0x28 / 4] = f28[k];
                node[k][0x5c / 4] = static_cast<std::uint32_t>(kids[k].size());
                node[k][0x60 / 4] = kids[k].empty() ? 0u : ch::addr(kids[k].data());
            }
            fn[which][side](node[root].data(), mask);  // returns what EAX held
            for (int k = 0; k < n; ++k) {
                snap[side].push_back(node[k][0x24 / 4]);
                snap[side].push_back(node[k][0x28 / 4]);
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

TEST(native_player_name_registry_add_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const F fn[2] = {rt::original<F>(0x00438920), reinterpret_cast<F>(&recoil::NameRegistry_Add)};
    std::mt19937 rng(0x438920);
    for (int it = 0; it < 300; ++it) {
        const int adds = 1 + static_cast<int>(rng() % 5);
        const bool preexisting = rng() % 2 != 0;
        std::vector<std::uint32_t> a(adds), b(adds);
        for (int k = 0; k < adds; ++k) { a[k] = rng(); b[k] = rng(); }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            std::uint32_t old[3] = {0x1111, 0x2222, 0x3333};
            rl.set(old, 1);
            *ch::img(side, 0x004f3344) = preexisting ? ch::addr(old) : 0u;
            *ch::img(side, 0x004f3348) = preexisting ? ch::addr(old) : 0u;
            *ch::img(side, 0x004f334c) = preexisting ? 1u : 0u;
            std::vector<std::uint32_t> got;
            for (int k = 0; k < adds; ++k) {
                const std::uint32_t e = fn[side](a[k], b[k]);
                got.push_back(e);
                snap[side].push_back(rl(e));
            }
            for (std::uint32_t va : {0x004f3344u, 0x004f3348u, 0x004f334cu}) snap[side].push_back(rl(*ch::img(side, va)));
            for (std::uint32_t w : old) snap[side].push_back(rl(w));
            for (std::uint32_t e : got)
                for (int w = 0; w < 3; ++w) snap[side].push_back(rl(ch::at(e)[w]));
            for (std::uint32_t e : got) ch::c_free(e);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

TEST(native_player_search_path_functions_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn reset[2] = {rt::original<Fn>(0x0048cca0), reinterpret_cast<Fn>(&recoil::SearchPath_InitOrReset)};
    const Fn pop[2] = {rt::original<Fn>(0x0048cb70), reinterpret_cast<Fn>(&recoil::Container_ListPopCursor)};
    const Fn config[2] = {rt::original<Fn>(0x0041fe50), reinterpret_cast<Fn>(&recoil::Player_VehicleConfigForDifficulty)};
    const Fn dir_of[2] = {rt::original<Fn>(0x00421e20), reinterpret_cast<Fn>(&recoil::Path_GetDirectoryOfFound)};
    // Pickups_ConfigFileForDifficulty (0x0041ddf0): the same choice over the names at 0x004dc22c / 0x004dc240 / 0x004dc254
    const Fn pickup_config[2] = {rt::original<Fn>(0x0041ddf0), reinterpret_cast<Fn>(&recoil::Pickups_ConfigFileForDifficulty)};
    // the six config names, as the original image holds them
    const std::string names[6] = {reinterpret_cast<const char*>(0x004dc334), reinterpret_cast<const char*>(0x004dc348),
                                  reinterpret_cast<const char*>(0x004dc35c), reinterpret_cast<const char*>(0x004dc22c),
                                  reinterpret_cast<const char*>(0x004dc240), reinterpret_cast<const char*>(0x004dc254)};
    const std::string root = ch::temp_path("rcp");
    DeleteFileA(root.c_str());
    CreateDirectoryA(root.c_str(), nullptr);
    std::vector<std::string> dirs;
    for (int d = 0; d < 4; ++d) {
        dirs.push_back(root + "\\d" + std::to_string(d));
        if (d < 3) {
            CreateDirectoryA(dirs.back().c_str(), nullptr);
            for (int k = 0; k < 6; ++k)
                if ((d + k) % 2 == 0) ch::write_file(dirs.back() + "\\" + names[k], "x");
            ch::write_file(dirs.back() + "\\other.txt", "y");
        }
    }
    std::mt19937 rng(0x41fe50);
    auto tokens = [&](int n) {
        std::string s;
        for (int k = 0; k < n; ++k) { if (k) s += ";"; s += dirs[rng() % 4]; }
        return s;
    };
    int compared = 0;
    for (int it = 0; it < 400; ++it) {
        const int cfg = static_cast<int>(rng() % 3);  // 0 directory of, 1 vehicle config, 2 pickup config
        const std::int32_t intensity = static_cast<std::int32_t>(rng() % 4);
        const std::string path = rng() % 5 == 0 ? std::string() : tokens(1 + static_cast<int>(rng() % 3));
        const std::string set = tokens(1 + static_cast<int>(rng() % 3));
        const int q = static_cast<int>(rng() % 5);
        const std::string query = q < 3 ? names[q + 3 * (rng() % 2)] : q == 3 ? std::string("other.txt") : std::string("missing.bin");
        std::vector<std::string> out[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            std::int32_t level = intensity;
            *ch::img(side, 0x004e5d48) = ch::addr(&level);
            if (cfg) {
                const std::uint32_t r = (cfg == 1 ? config : pickup_config)[side](ch::addr(path.c_str()), 0);
                out[side].push_back(r ? reinterpret_cast<const char*>(static_cast<std::uintptr_t>(r)) : "<null>");
            } else {
                reset[side](ch::addr(set.c_str()), 0);
                char buf[0x200];
                std::memset(buf, 'z', sizeof buf);
                buf[sizeof buf - 1] = 0;
                dir_of[side](ch::addr(query.c_str()), ch::addr(buf));
                out[side].push_back(buf);
            }
            for (std::uint32_t va : {0x0056b180u, 0x0056bb8cu}) {
                const std::uint32_t l = *ch::img(side, va);
                out[side].push_back(l ? "<set>" : "<none>");
                for (std::uint32_t p; l && (p = pop[side](l, 0)) != 0;) {
                    out[side].push_back(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p)));
                    ch::c_free(p);
                }
            }
        }
        CHECK(out[0] == out[1]);
        ++compared;
    }
    for (int d = 0; d < 3; ++d) {
        for (int k = 0; k < 6; ++k) DeleteFileA((dirs[d] + "\\" + names[k]).c_str());
        DeleteFileA((dirs[d] + "\\other.txt").c_str());
        RemoveDirectoryA(dirs[d].c_str());
    }
    RemoveDirectoryA(root.c_str());
    rt::restore_pristine();
    std::printf("  Player_VehicleConfigForDifficulty / Pickups_ConfigFileForDifficulty / Path_GetDirectoryOfFound calls %d (real tree, own node pool per side)\n", compared);
}
