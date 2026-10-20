// Native L1 for SearchPath_Resolve (0x004a5e50) and its predicate SearchPath_EntryLacksFile (0x004a5f20, a function
// the Stage 1 export missed - see its ledger row). The resolver splits a path (_splitpath into 0x0056bb88 drive,
// 0x0056ba88 dir, 0x0056b678 name, 0x0056b778 ext), formats name+ext into 0x0056b980, walks a search list (ECX, or
// the global list [0x0056bb8c] when ECX is null; list +0 non-empty flag, +0x10 head; circular nodes +0 directory
// string, +4 next) with the predicate until a directory holds the file, strips a trailing backslash from that
// directory string in place, formats the full path into 0x0056b980 and returns it; otherwise it retries once with
// the global list, then returns 0. Both need real files: a temp tree with files in some of its directories.
// Each side gets its own lists and directory strings (the resolver edits them) and its own image globals.
// Compared: return value (0x0056b980 on each side, or 0), the result buffer, every directory string after the
// call; for the predicate its return value and its path buffer 0x0056b878.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "unattributed/asset_io_misc.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
char* image(int side, std::uint32_t va)
{
    return static_cast<char*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }

struct List {  // one side's search list over its own copies of the directory strings
    std::vector<std::vector<char>> dirs;
    std::vector<std::uint32_t> nodes;
    std::uint32_t head[8] = {};
    void build(const std::vector<std::string>& names)
    {
        dirs.clear();
        for (const auto& s : names) dirs.emplace_back(s.c_str(), s.c_str() + s.size() + 1);
        nodes.assign(2 * (names.empty() ? 1 : names.size()), 0);
        for (std::size_t i = 0; i < names.size(); ++i) {
            nodes[2 * i] = addr(dirs[i].data());
            nodes[2 * i + 1] = addr(&nodes[2 * ((i + 1) % names.size())]);
        }
        head[0] = names.empty() ? 0 : 1;
        head[4] = names.empty() ? 0 : addr(nodes.data());
    }
};
}  // namespace

TEST(native_search_path_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    const std::string root = std::string(tmp) + "recoil_sp_" + std::to_string(GetCurrentProcessId());
    const char* sub[4] = {"a", "b", "cc", "d d"};
    const char* file[6] = {"one.zbd", "two.txt", "three", "four.x", "five.zbd", "six.dat"};
    CreateDirectoryA(root.c_str(), nullptr);
    for (int d = 0; d < 4; ++d) {
        const std::string dir = root + "\\" + sub[d];
        CreateDirectoryA(dir.c_str(), nullptr);
        for (int f = 0; f < 6; ++f)
            if ((d + f) % 3 == 0) {  // each file in some directories only
                const std::string full = dir + "\\" + file[f];  // content = its own path, so an opened FILE names its file
                HANDLE h = CreateFileA(full.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
                DWORD w = 0;
                WriteFile(h, full.c_str(), static_cast<DWORD>(full.size()), &w, nullptr);
                CloseHandle(h);
            }
    }
    using Fn = std::uint32_t(__fastcall*)(void*, const char*);
    const Fn resolve[2] = {rt::original<Fn>(0x004a5e50), reinterpret_cast<Fn>(&recoil::SearchPath_Resolve)};
    const Fn pred[2] = {rt::original<Fn>(0x004a5f20), reinterpret_cast<Fn>(&recoil::SearchPath_EntryLacksFile)};
    std::mt19937 rng(0x4a5e50);
    auto dir_name = [&]() {
        std::string d = rng() % 6 == 0 ? root + "\\nowhere" : root + "\\" + sub[rng() % 4];
        if (rng() % 2) d += "\\";  // a trailing backslash, which the resolver strips in place
        return d;
    };
    int compared = 0;
    for (int it = 0; it < 400; ++it) {
        std::vector<std::string> own(rng() % 4), global(rng() % 4);
        for (auto& d : own) d = dir_name();
        for (auto& d : global) d = dir_name();
        const bool null_list = rng() % 4 == 0, no_global = rng() % 4 == 0;
        std::string path = std::string(rng() % 3 == 0 ? "C:\\some\\where\\" : rng() % 2 ? "rel\\" : "") + file[rng() % 6];
        if (rng() % 8 == 0) path = "missing.zbd";
        std::uint32_t ret[2];
        std::string out[2];
        std::vector<std::vector<char>> after[2];
        List lists[2][2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            lists[side][0].build(own);
            lists[side][1].build(global);
            std::uint32_t g = no_global ? 0 : addr(lists[side][1].head);
            std::memcpy(image(side, 0x0056bb8c), &g, 4);
            ret[side] = resolve[side](null_list ? nullptr : lists[side][0].head, path.c_str());
            out[side] = image(side, 0x0056b980);
            for (int l = 0; l < 2; ++l) after[side].insert(after[side].end(), lists[side][l].dirs.begin(), lists[side][l].dirs.end());
        }
        CHECK_EQ(ret[0] == 0, ret[1] == 0);
        if (ret[0]) {
            CHECK_EQ(ret[0], 0x0056b980u);
            CHECK_EQ(ret[1], addr(image(1, 0x0056b980)));
        }
        CHECK(out[0] == out[1]);
        if (out[0] != out[1] && compared < 4)
            std::printf("    path '%s' ret %x/%x: original '%s' port '%s'\n", path.c_str(), ret[0], ret[1], out[0].c_str(), out[1].c_str());
        CHECK(after[0] == after[1]);
        ++compared;
    }
    for (int it = 0; it < 200; ++it) {
        const std::string d = dir_name(), n = file[rng() % 6];
        std::uint32_t ret[2];
        std::string buf[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            using P = std::uint32_t(__fastcall*)(const char*, const char*);
            ret[side] = reinterpret_cast<P>(pred[side])(d.c_str(), n.c_str());
            buf[side] = image(side, 0x0056b878);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(buf[0] == buf[1]);
        ++compared;
    }
    // File_OpenOnSearchPath (0x004a5f50): SearchPath_Resolve, then fopen(resolved path, or the name itself, mode)
    {
        using Open = void*(__fastcall*)(void*, const char*, const char*);
        using Fread = std::size_t(__cdecl*)(void*, std::size_t, std::size_t, void*);
        using Fclose = int(__cdecl*)(void*);
        const Open open[2] = {rt::original<Open>(0x004a5f50), reinterpret_cast<Open>(&recoil::File_OpenOnSearchPath)};
        const auto m_fread = reinterpret_cast<Fread>(recoil::g_Iat_fread_004cc4e0);
        const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
        for (int it = 0; it < 300; ++it) {
            std::vector<std::string> own(rng() % 4), global(rng() % 4);
            for (auto& d : own) d = dir_name();
            for (auto& d : global) d = dir_name();
            const bool null_list = rng() % 4 == 0, no_global = rng() % 4 == 0;
            std::string path = std::string(rng() % 3 == 0 ? "rel\\" : "") + file[rng() % 6];
            if (rng() % 8 == 0) path = root + "\\" + sub[rng() % 4] + "\\" + file[rng() % 6];  // unresolvable, opened as given
            const char* mode = rng() % 2 ? "rb" : "r";
            bool opened[2];
            std::string content[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                List lists[2];
                lists[0].build(own);
                lists[1].build(global);
                std::uint32_t g = no_global ? 0 : addr(lists[1].head);
                std::memcpy(image(side, 0x0056bb8c), &g, 4);
                void* f = open[side](null_list ? nullptr : lists[0].head, path.c_str(), mode);
                opened[side] = f != nullptr;
                if (f) {
                    char buf[400] = {};
                    const std::size_t k = m_fread(buf, 1, sizeof buf - 1, f);
                    content[side].assign(buf, k);
                    m_fclose(f);
                }
            }
            CHECK_EQ(opened[0], opened[1]);
            CHECK(content[0] == content[1]);
            ++compared;
        }
    }
    for (int d = 0; d < 4; ++d) {
        for (int f = 0; f < 6; ++f) DeleteFileA((root + "\\" + sub[d] + "\\" + file[f]).c_str());
        RemoveDirectoryA((root + "\\" + sub[d]).c_str());
    }
    RemoveDirectoryA(root.c_str());
    rt::restore_pristine();
    std::printf("  SearchPath_Resolve/EntryLacksFile/File_OpenOnSearchPath calls %d (temp directory tree, own lists per side)\n", compared);
}
