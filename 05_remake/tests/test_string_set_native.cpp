// Native L1 for StringSet_AddSemicolonTokens (0x004a5ce0): (list ECX, string EDX). For the list (when non-null) and
// then for the global string set [0x0056bb8c] (created with Container_CreateList when null): _strdup the string,
// strtok it on ";" (0x004e3004), and for each token that names an existing path (File_Exists) and is not already in
// the list (Container_FindFirstByPredicateThunk with StringSet_CompareStrings) insert a _strdup copy
// (Container_ListInsertAfterCursor). The arena fuzz cannot drive it (real paths, a real node pool and lists). Here:
// a temp tree (some directories exist, some do not), strings with duplicates and empty tokens, the caller list null
// or pre-filled, the global set null or pre-filled; each side builds its own node pool and lists with its own
// functions. Compared: every list afterwards, drained with the side's own Container_ListPopCursor, strings by
// content, and whether the global set exists.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zReader/zreader.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

TEST(native_string_set_add_tokens_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    using Dup = char*(__cdecl*)(const char*);
    const Fn add[2] = {rt::original<Fn>(0x004a5ce0), reinterpret_cast<Fn>(&recoil::StringSet_AddSemicolonTokens)};
    // then StringSet_CreateAndAddTokens (0x004a5ca0, string ECX): creates a fresh list, fills it with
    // AddSemicolonTokens(list, string) and returns it (the ledger's earlier note had this backwards) - compared too
    const Fn create_add[2] = {rt::original<Fn>(0x004a5ca0), reinterpret_cast<Fn>(&recoil::StringSet_CreateAndAddTokens)};
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn append[2] = {rt::original<Fn>(0x0048ca30), reinterpret_cast<Fn>(&recoil::Container_ListAppend)};
    const Fn pop[2] = {rt::original<Fn>(0x0048cb70), reinterpret_cast<Fn>(&recoil::Container_ListPopCursor)};
    const auto m_strdup = reinterpret_cast<Dup>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "_strdup"));
    const auto m_free = reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4);
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    const std::string root = std::string(tmp) + "recoil_ss_" + std::to_string(GetCurrentProcessId());
    CreateDirectoryA(root.c_str(), nullptr);
    std::vector<std::string> dirs;
    for (int d = 0; d < 6; ++d) {
        dirs.push_back(root + "\d" + std::to_string(d));
        if (d < 4) CreateDirectoryA(dirs.back().c_str(), nullptr);  // d4, d5 do not exist
    }
    auto g = [](int side) {
        return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x0056bb8c) : reinterpret_cast<void*>(0x0056bb8c));
    };
    int compared = 0;
    for (int fn = 0; fn < 2; ++fn) {
    std::mt19937 rng(fn ? 0x4a5ca0u : 0x4a5ce0u);
    for (int it = 0; it < 400; ++it) {
        auto tokens = [&](int n) {
            std::string s;
            for (int k = 0; k < n; ++k) {
                if (k) s += rng() % 6 == 0 ? ";;" : ";";
                s += dirs[rng() % 6];
            }
            return s;
        };
        const std::string str = rng() % 10 == 0 ? std::string() : tokens(1 + static_cast<int>(rng() % 5));
        const bool null_str = rng() % 12 == 0, own_list = rng() % 3 != 0, global_list = rng() % 2 != 0;
        const std::string own_pre = tokens(static_cast<int>(rng() % 3)), global_pre = tokens(static_cast<int>(rng() % 3));
        std::vector<std::string> out[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            auto fill = [&](std::uint32_t list, const std::string& pre) {  // pre-existing entries, one per token
                std::string s = pre;
                for (std::size_t a = 0; a <= s.size() && !s.empty();) {
                    const std::size_t b = s.find(';', a);
                    const std::string tok = s.substr(a, b == std::string::npos ? std::string::npos : b - a);
                    if (!tok.empty()) append[side](list, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_strdup(tok.c_str()))));
                    if (b == std::string::npos) break;
                    a = b + 1;
                }
            };
            const std::uint32_t list = own_list ? create[side](0, 0) : 0;
            if (list) fill(list, own_pre);
            *g(side) = 0;
            if (global_list) { *g(side) = create[side](0, 0); fill(*g(side), global_pre); }
            const std::uint32_t s_arg = null_str ? 0 : static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(str.c_str()));
            const std::uint32_t made = fn ? create_add[side](s_arg, 0) : (add[side](list, s_arg), 0u);
            for (std::uint32_t l : {list, *g(side), made}) {
                out[side].push_back(l ? "<list>" : "<none>");
                for (std::uint32_t p; l && (p = pop[side](l, 0)) != 0;) {
                    out[side].push_back(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p)));
                    m_free(reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)));
                }
            }
        }
        CHECK(out[0] == out[1]);
        ++compared;
    }
    }
    for (int d = 0; d < 4; ++d) RemoveDirectoryA(dirs[d].c_str());
    RemoveDirectoryA(root.c_str());
    rt::restore_pristine();
    std::printf("  StringSet_AddSemicolonTokens/CreateAndAddTokens calls %d (real directories, own node pool and lists per side)\n", compared);
}

// SearchPath_InitOrReset (0x0048cca0) and SearchPath_InitOrAdd (0x0048cce0), string ECX: create the search-path set
// [0x0056b180] from the string (StringSet_CreateAndAddTokens) or, when it exists, reset it first (List_FreeAllPopped;
// InitOrReset only) and add the string's tokens. AddSemicolonTokens also feeds the global set [0x0056bb8c]. Here: 1..3
// random calls on each side from a pristine state (own node pool), then both sets drained and compared.
// Reader_OpenWithSearch (0x0048cd40, name ECX, path EDX): the name itself when it exists; else, for a non-empty path, a
// temporary set from it, SearchPath_Resolve against it (result in 0x0056b980), the set destroyed; else resolve
// against [0x0056b180]; 0 when nothing is found. Real files; compared: the return by role (name / resolved path by
// content / 0) and the sets afterwards.
TEST(native_search_path_sets_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn reset[2] = {rt::original<Fn>(0x0048cca0), reinterpret_cast<Fn>(&recoil::SearchPath_InitOrReset)};
    const Fn add[2] = {rt::original<Fn>(0x0048cce0), reinterpret_cast<Fn>(&recoil::SearchPath_InitOrAdd)};
    const Fn open[2] = {rt::original<Fn>(0x0048cd40), reinterpret_cast<Fn>(&recoil::Reader_OpenWithSearch)};
    // Archive_InitRegistry (0x0048cc70, string ECX): creates the reader list [0x0056b184] when missing, then
    // SearchPath_InitOrReset(string) and clears the kept archive [0x0056b188]; nothing when the list exists
    const Fn registry[2] = {rt::original<Fn>(0x0048cc70), reinterpret_cast<Fn>(&recoil::Archive_InitRegistry)};
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn pop[2] = {rt::original<Fn>(0x0048cb70), reinterpret_cast<Fn>(&recoil::Container_ListPopCursor)};
    const auto m_free = reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4);
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    const std::string root = std::string(tmp) + "recoil_sp2_" + std::to_string(GetCurrentProcessId());
    CreateDirectoryA(root.c_str(), nullptr);
    std::vector<std::string> dirs;
    const char* files[3] = {"a.zbd", "b.zbd", "c.txt"};
    for (int d = 0; d < 5; ++d) {
        dirs.push_back(root + "\p" + std::to_string(d));
        if (d < 4) {
            CreateDirectoryA(dirs.back().c_str(), nullptr);
            for (int f = 0; f < 3; ++f)
                if ((d + f) % 2 == 0) CloseHandle(CreateFileA((dirs.back() + "\\" + files[f]).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr));
        }
    }
    auto img = [](int side, std::uint32_t va) {
        return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
    };
    std::mt19937 rng(0x48cca0);
    auto tokens = [&](int n) {
        std::string s;
        for (int k = 0; k < n; ++k) { if (k) s += ";"; s += dirs[rng() % 5]; }
        return s;
    };
    int compared = 0;
    for (int it = 0; it < 600; ++it) {
        const int calls = 1 + static_cast<int>(rng() % 3);
        std::vector<int> kinds;
        std::vector<std::string> strs;
        for (int c = 0; c < calls; ++c) { kinds.push_back(static_cast<int>(rng() % 3)); strs.push_back(tokens(1 + static_cast<int>(rng() % 3))); }
        const bool do_open = rng() % 2 != 0;
        const int which = static_cast<int>(rng() % 4);
        std::string name = which == 0 ? dirs[rng() % 4] + "\\" + files[rng() % 3] : std::string(files[rng() % 3]);
        if (which == 3) name = "missing.zbd";
        const std::string path = rng() % 3 == 0 ? std::string() : tokens(1 + static_cast<int>(rng() % 3));
        const bool null_path = path.empty() && rng() % 2 != 0;  // an empty path, or none at all
        std::vector<std::string> out[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            for (int c = 0; c < calls; ++c)
                (kinds[c] == 2 ? registry : kinds[c] ? add : reset)[side](static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(strs[c].c_str())), 0);
            if (do_open) {
                const auto np = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(name.c_str()));
                const std::uint32_t r = open[side](np, null_path ? 0u : static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(path.c_str())));
                out[side].push_back(r == 0 ? "<null>" : r == np ? "<name>" : std::string("<resolved>") + reinterpret_cast<const char*>(static_cast<std::uintptr_t>(r)));
            }
            out[side].push_back(*img(side, 0x0056b184) ? "<readers>" : "<no readers>");
            out[side].push_back(*img(side, 0x0056b188) ? "<kept>" : "<no kept>");
            for (std::uint32_t va : {0x0056b180u, 0x0056bb8cu}) {
                const std::uint32_t l = *img(side, va);
                out[side].push_back(l ? "<set>" : "<none>");
                for (std::uint32_t p; l && (p = pop[side](l, 0)) != 0;) {
                    out[side].push_back(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p)));
                    m_free(reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)));
                }
            }
        }
        CHECK(out[0] == out[1]);
        ++compared;
    }
    for (int d = 0; d < 4; ++d) {
        for (int f = 0; f < 3; ++f) DeleteFileA((dirs[d] + "\\" + files[f]).c_str());
        RemoveDirectoryA(dirs[d].c_str());
    }
    RemoveDirectoryA(root.c_str());
    rt::restore_pristine();
    std::printf("  SearchPath_InitOrReset/InitOrAdd/Reader_OpenWithSearch calls %d (real tree, own node pool per side)\n", compared);
}
