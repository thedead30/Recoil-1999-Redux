// L2 for asset_io: every shipped Zar archive (the 17 .zbd files with a Zar footer - 00_original/game_install/zbd:
// zrdr.zbd, m1..m13/zrdr.zbd, soundsl/m/h.zbd; the other 88 .zbd files are texture / gamez / anim formats of later
// subsystems) is opened through the ported functions and through the original, with the real KERNEL32 file calls
// bound into both import tables:
//   Zar_OpenAndRegisterArchive(path, keep 1) -> the kept archive's TOC is compared with the file's own footer and TOC
//   (read independently here) and between the sides;
//   zrdr archives: ConfigTree_ParseFileByBasename(entry name) for every .zrd entry -> the tree (serialised
//   structurally) is compared between the sides and with an independent parse of the entry's bytes here
//   (type dword; 1 int / 2 float: 4 bytes; 3 string: length + bytes; 4 list: count, children 1..count-1);
//   sound archives: Zar_FindEntryInArchives(entry name) for every .wav entry -> the returned handle is positioned at
//   the entry: its first bytes (up to 4 KB, read here) equal the file's bytes at the TOC offset, on both sides.
// Then Reader_CloseAll(1) closes and frees each side's archives.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zReader/zreader.h"
#include "platform/iat_kernel32.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }

std::vector<unsigned char> read_file(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}
std::uint32_t u32(const std::vector<unsigned char>& b, std::size_t o)
{
    std::uint32_t v = 0;
    if (o + 4 <= b.size()) std::memcpy(&v, &b[o], 4);
    return v;
}
// independent parse of a config node at o, in the serialisation below; returns false on a malformed stream
bool parse_zrd(const std::vector<unsigned char>& b, std::size_t& o, std::vector<std::uint32_t>& out, int depth)
{
    if (o + 4 > b.size() || depth > 16) return false;
    const std::uint32_t type = u32(b, o);
    o += 4;
    out.push_back(type);
    if (type == 1 || type == 2) { out.push_back(u32(b, o)); o += 4; return o <= b.size(); }
    if (type == 3) {
        const std::uint32_t n = u32(b, o);
        o += 4;
        if (o + n > b.size()) return false;
        for (std::uint32_t i = 0; i < n; ++i) out.push_back(b[o + i]);
        out.push_back(0xFFFFFFFFu);
        o += n;
        return true;
    }
    if (type == 4) {
        const std::uint32_t count = u32(b, o);
        o += 4;
        out.push_back(1);
        out.push_back(count);
        for (std::uint32_t i = 1; i < count; ++i) if (!parse_zrd(b, o, out, depth + 1)) return false;
        return true;
    }
    return false;
}
void serialise(const std::uint32_t* node, std::vector<std::uint32_t>& out, int depth)
{
    out.push_back(node[0]);
    if (node[0] == 1 || node[0] == 2) out.push_back(node[1]);
    if (node[0] == 3) {
        for (const char* s = ptr<char>(node[1]); *s; ++s) out.push_back(static_cast<unsigned char>(*s));
        out.push_back(0xFFFFFFFFu);
    }
    if (node[0] == 4 && depth < 16) {
        const auto* arr = ptr<std::uint32_t>(node[1]);
        out.push_back(arr[0]);
        out.push_back(arr[1]);
        for (std::uint32_t i = 1; i < arr[1]; ++i) serialise(arr + 2 * i, out, depth + 1);
    }
}
}  // namespace

TEST(native_asset_io_l2_shipped_archives)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    using Fn3 = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t, std::uint32_t);
    const Fn init[2] = {rt::original<Fn>(0x0048c7d0), reinterpret_cast<Fn>(&recoil::Container_InitNodePool)};
    const Fn create[2] = {rt::original<Fn>(0x0048c950), reinterpret_cast<Fn>(&recoil::Container_CreateList)};
    const Fn open[2] = {rt::original<Fn>(0x0048d210), reinterpret_cast<Fn>(&recoil::Zar_OpenAndRegisterArchive)};
    const Fn3 parse[2] = {rt::original<Fn3>(0x0048cdc0), reinterpret_cast<Fn3>(&recoil::ConfigTree_ParseFileByBasename)};
    const Fn destroy[2] = {rt::original<Fn>(0x0048ce40), reinterpret_cast<Fn>(&recoil::ConfigTree_Destroy)};
    const Fn find[2] = {rt::original<Fn>(0x0048d1c0), reinterpret_cast<Fn>(&recoil::Zar_FindEntryInArchives)};
    const Fn close_all[2] = {rt::original<Fn>(0x0048d2c0), reinterpret_cast<Fn>(&recoil::Reader_CloseAll)};
    // the original's KERNEL32 slots are unresolved: give them the real functions the port's slots hold
    const std::uint32_t k_va[9] = {0x004cc130, 0x004cc14c, 0x004cc144, 0x004cc134, 0x004cc0e4, 0x004cc138, 0x004cc0dc, 0x004cc0f4, 0x004cc0fc};
    void* const k_port[9] = {recoil::g_Iat_CreateFileA_004cc130, recoil::g_Iat_GetFileSize_004cc14c, recoil::g_Iat_SetFilePointer_004cc144,
                             recoil::g_Iat_ReadFile_004cc134, recoil::g_Iat_CloseHandle_004cc0e4, recoil::g_Iat_WriteFile_004cc138,
                             recoil::g_Iat_GetLastError_004cc0dc, recoil::g_Iat_FormatMessageA_004cc0f4, recoil::g_Iat_LocalFree_004cc0fc};
    void* saved[9];
    for (int i = 0; i < 9; ++i) {
        void** o = ptr<void*>(k_va[i]);
        saved[i] = *o;
        *o = k_port[i];
    }
    std::string dir = RECOIL_ORIGINAL_EXE;
    dir = dir.substr(0, dir.find_last_of("/\\") + 1) + "zbd/";
    std::vector<std::string> archives = {"zrdr.zbd", "soundsl.zbd", "soundsm.zbd", "soundsh.zbd"};
    for (int m = 1; m <= 13; ++m) archives.push_back("m" + std::to_string(m) + "/zrdr.zbd");
    int n_archives = 0, n_trees = 0, n_waves = 0;
    for (const std::string& rel : archives) {
        const std::string path = dir + rel;
        const std::vector<unsigned char> file = read_file(path);
        CHECK(file.size() >= 8);
        if (file.size() < 8) continue;
        const std::uint32_t count = u32(file, file.size() - 4);
        const std::size_t toc = file.size() - 8 - static_cast<std::size_t>(count) * 0x94;
        CHECK_EQ(u32(file, file.size() - 8), 1u);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            auto g = [&](std::uint32_t va) { return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : ptr<void>(va)); };
            init[side](4, 0);
            *g(0x0056b184) = create[side](0, 0);
            *g(0x0056b188) = 0;
            snap[side].push_back(open[side](addr(path.c_str()), 1));
            const std::uint32_t arc = *g(0x0056b188);
            CHECK(arc != 0);
            if (!arc) continue;
            const auto* o = ptr<std::uint32_t>(arc);
            // TOC against the file
            CHECK_EQ(o[3], count);
            CHECK(o[5] != 0 && std::memcmp(ptr<void>(o[5]), &file[toc], static_cast<std::size_t>(count) * 0x94) == 0);
            snap[side].push_back(o[3]);
            for (std::uint32_t e = 0; e < count; ++e) {
                const std::size_t rec = toc + e * 0x94;
                const std::uint32_t off = u32(file, rec), size = u32(file, rec + 4);
                const std::string name(reinterpret_cast<const char*>(&file[rec + 8]), strnlen(reinterpret_cast<const char*>(&file[rec + 8]), 0x40));
                const bool is_zrd = name.size() > 4 && _stricmp(name.c_str() + name.size() - 4, ".zrd") == 0;
                if (is_zrd) {
                    const std::uint32_t tree = parse[side](addr(name.c_str()), 0, 0);
                    CHECK(tree != 0);
                    std::vector<std::uint32_t> got, want;
                    if (tree) serialise(ptr<std::uint32_t>(tree), got, 0);
                    std::size_t pos = off;
                    const bool ok = parse_zrd(file, pos, want, 0);
                    CHECK(ok);
                    CHECK(pos <= static_cast<std::size_t>(off) + size);
                    CHECK(got == want);
                    snap[side].insert(snap[side].end(), got.begin(), got.end());
                    if (tree) destroy[side](tree, 0);
                    n_trees += side;
                } else {
                    const std::uint32_t h = find[side](addr(name.c_str()), 0);
                    CHECK(h != 0xFFFFFFFFu);
                    std::vector<unsigned char> buf(size < 4096 ? size : 4096);
                    DWORD got = 0;
                    ReadFile(reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(h)), buf.data(), static_cast<DWORD>(buf.size()), &got, nullptr);
                    CHECK_EQ(got, static_cast<DWORD>(buf.size()));
                    CHECK(std::memcmp(buf.data(), &file[off], buf.size()) == 0);
                    snap[side].push_back(got);
                    n_waves += side;
                }
            }
            close_all[side](1, 0);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++n_archives;
    }
    for (int i = 0; i < 9; ++i) *ptr<void*>(k_va[i]) = saved[i];
    rt::restore_pristine();
    std::printf("  asset_io L2: %d shipped archives, %d .zrd config trees, %d .wav entries (both sides; trees and bytes against the files)\n",
                n_archives, n_trees, n_waves);
}
