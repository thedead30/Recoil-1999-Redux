// Native L1 for File_ReadLengthPrefixedString (0x004a6110): it reads through KERNEL32 ReadFile, which the oracle
// leaves unresolved in the original's import table (tests/native_oracle.h), so both sides get the same fake
// ReadFile - installed in the original's slot 0x004cc134 and in the port's g_Iat_ReadFile_004cc134 - reading an
// in-memory stream (the handle points to it) and logging each request. Streams hold a 4-byte length (-1..64) and
// 0..80 body bytes (short, exact and long bodies). Compared: return value, the request log, and the returned
// buffer's contents (length + 1 bytes). Streams shorter than 4 bytes are not driven: the original then uses stack
// residue as the length (KG-28).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "GameZRecoil/zReader/zreader.h"
#include "GameZRecoil/zModel/gmod_matl.h"
#include "unattributed/asset_io_misc.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"
#include "watchdog.h"

#include <string>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
struct Stream {
    std::vector<unsigned char> bytes;
    std::size_t pos = 0;
};
std::vector<std::uint32_t> g_requests;
BOOL WINAPI fake_read_file(HANDLE h, void* buf, DWORD n, DWORD* got, void* /*overlapped*/)
{
    Stream* s = static_cast<Stream*>(h);
    g_requests.push_back(n);
    const std::size_t k = s->bytes.size() - s->pos < n ? s->bytes.size() - s->pos : n;
    if (k) std::memcpy(buf, s->bytes.data() + s->pos, k);
    s->pos += k;
    *got = static_cast<DWORD>(k);
    return TRUE;
}
}  // namespace

TEST(native_zar_read_length_prefixed_string_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Read = int(__fastcall*)(void*, void**);
    using Free = void(__cdecl*)(void*);
    auto orig = rt::original<Read>(0x004a6110);
    auto port = reinterpret_cast<Read>(&recoil::File_ReadLengthPrefixedString);
    const auto m_free = reinterpret_cast<Free>(recoil::g_Iat_free_004cc5b4);
    auto* orig_slot = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc134));
    void* const saved_orig = *orig_slot;
    void* const saved_port = recoil::g_Iat_ReadFile_004cc134;
    *orig_slot = reinterpret_cast<void*>(&fake_read_file);
    recoil::g_Iat_ReadFile_004cc134 = reinterpret_cast<void*>(&fake_read_file);
    std::mt19937 rng(0x4a6110);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        const std::int32_t len = static_cast<std::int32_t>(rng() % 66) - 1;  // -1..64
        const int body = static_cast<int>(rng() % 81);
        std::vector<unsigned char> bytes(4 + body);
        std::memcpy(bytes.data(), &len, 4);
        for (int i = 0; i < body; ++i) bytes[4 + i] = static_cast<unsigned char>(rng());
        int ret[2];
        std::vector<std::uint32_t> req[2];
        std::vector<unsigned char> out[2];
        for (int side = 0; side < 2; ++side) {
            Stream s{bytes, 0};
            g_requests.clear();
            void* p = reinterpret_cast<void*>(0xCDCDCDCD);
            ret[side] = (side ? port : orig)(&s, &p);
            req[side] = g_requests;
            const auto* q = static_cast<const unsigned char*>(p);
            if (len >= 0) out[side].assign(q, q + len + 1);
            m_free(p);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(req[0] == req[1]);
        CHECK(out[0] == out[1]);
        ++compared;
    }
    *orig_slot = saved_orig;
    recoil::g_Iat_ReadFile_004cc134 = saved_port;
    std::printf("  File_ReadLengthPrefixedString calls %d (fake ReadFile streams: length -1..64, body 0..80)\n", compared);
}

// ConfigTree_ReadNode (0x0048d080): reads a 4-byte type into node +0, then by type 1 int / 2 float: 4 bytes into +4;
// 3 string: File_ReadLengthPrefixedString into +4; 4 list: a count, malloc(count * 8) into +4 whose first 8 bytes
// are a header {1, count}, and children 1..count-1 read recursively; any other type: error report, returns 0.
// Returns the bytes read. Streams hold random trees (depth <= 3, every type, sometimes an invalid type) in full (a
// short stream leaves locals as stack residue, as KG-28). Compared: return value, the ReadFile request log and the
// tree built (serialised structurally - its blocks are per side).
namespace {
void emit_node(std::vector<unsigned char>& s, std::mt19937& rng, int depth)
{
    auto put = [&](std::uint32_t v) { const auto* b = reinterpret_cast<const unsigned char*>(&v); s.insert(s.end(), b, b + 4); };
    std::uint32_t type = 1 + rng() % 4;
    if (rng() % 20 == 0) type = rng() % 2 ? 0u : 5u + rng() % 3;  // invalid: the error path ends the read
    if (type == 4 && depth >= 3) type = 1;
    put(type);
    if (type == 1 || type == 2) put(rng());
    if (type == 3) {
        const std::uint32_t n = rng() % 12;
        put(n);
        for (std::uint32_t i = 0; i < n; ++i) s.push_back(static_cast<unsigned char>('a' + rng() % 26));
    }
    if (type == 4) {
        const std::uint32_t count = 1 + rng() % 4;  // header + count-1 children
        put(count);
        for (std::uint32_t i = 1; i < count; ++i) emit_node(s, rng, depth + 1);
    }
}
void serialise(const std::uint32_t* node, std::vector<std::uint32_t>& out, int depth)
{
    out.push_back(node[0]);
    if (node[0] == 1 || node[0] == 2) out.push_back(node[1]);
    if (node[0] == 3) {
        const char* s = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(node[1]));
        for (; *s; ++s) out.push_back(static_cast<unsigned char>(*s));
        out.push_back(0xFFFFFFFFu);
    }
    if (node[0] == 4 && depth < 8) {
        const auto* arr = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(node[1]));
        out.push_back(arr[0]);
        out.push_back(arr[1]);
        for (std::uint32_t i = 1; i < arr[1]; ++i) serialise(arr + 2 * i, out, depth + 1);
    }
}
void free_tree(const std::uint32_t* node, void(__cdecl* m_free)(void*))
{
    if (node[0] == 3) m_free(reinterpret_cast<void*>(static_cast<std::uintptr_t>(node[1])));
    if (node[0] == 4) {
        auto* arr = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(node[1]));
        for (std::uint32_t i = 1; i < arr[1]; ++i) free_tree(arr + 2 * i, m_free);
        m_free(arr);
    }
}
}  // namespace

TEST(native_config_tree_read_node_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Read = int(__fastcall*)(void*, std::uint32_t*);
    using Free = void(__cdecl*)(void*);
    const Read fn[2] = {rt::original<Read>(0x0048d080), reinterpret_cast<Read>(&recoil::ConfigTree_ReadNode)};
    const auto m_free = reinterpret_cast<Free>(recoil::g_Iat_free_004cc5b4);
    auto* orig_slot = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc134));
    void* const saved_orig = *orig_slot;
    void* const saved_port = recoil::g_Iat_ReadFile_004cc134;
    *orig_slot = reinterpret_cast<void*>(&fake_read_file);
    recoil::g_Iat_ReadFile_004cc134 = reinterpret_cast<void*>(&fake_read_file);
    std::mt19937 rng(0x48d080);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        std::vector<unsigned char> bytes;
        emit_node(bytes, rng, 0);
        for (int k = 0; k < 8; ++k) bytes.push_back(static_cast<unsigned char>(rng()));  // trailing data, not read
        const std::uint32_t init1 = rng();
        int ret[2];
        std::vector<std::uint32_t> req[2], tree[2];
        std::uint32_t nodes[2][2];
        for (int side = 0; side < 2; ++side) {
            Stream s{bytes, 0};
            g_requests.clear();
            std::uint32_t* node = nodes[side];
            node[0] = 0xDEADBEEFu;
            node[1] = init1;
            ret[side] = fn[side](&s, node);
            req[side] = g_requests;
            // an invalid type anywhere stops that branch: serialise only what the call defined
            // guarded: a wrong port can leave a malformed tree (an uninitialised child), which must show as a
            // difference, not crash the test
            if (node[0] >= 1 && node[0] <= 4) {
                if (wd::guarded([&] { serialise(node, tree[side], 0); }) != wd::kOk) tree[side] = {0xFA17FA17u};
            } else {
                tree[side] = {node[0], node[1]};
            }
        }
        // free only trees that match the original's (well-formed by construction); a malformed port tree is leaked
        if (tree[0] == tree[1])
            for (auto& node : nodes)
                if (node[0] == 3 || node[0] == 4) free_tree(node, m_free);
        CHECK_EQ(ret[0], ret[1]);
        CHECK(req[0] == req[1]);
        CHECK(tree[0] == tree[1]);
        ++compared;
    }
    *orig_slot = saved_orig;
    recoil::g_Iat_ReadFile_004cc134 = saved_port;
    std::printf("  ConfigTree_ReadNode calls %d (fake ReadFile streams: random trees, invalid types)\n", compared);
}

// ConfigValue_ToInt (0x004071f0): node type 1 -> the int at +4; 2 -> the float at +4 truncated by the CRT _ftol
// (a tail jump through the import thunk 0x004c60a6); 3 -> NameTable_LookupValue(string at +4); else 0. The arena
// fuzz reaches the float path rarely and with denormal bits (both truncate to 0), so here: ints, floats across
// magnitudes and signs (fractions, halves, large values), strings (names that may or may not be in the table) and
// other types. Compared: the return value.
TEST(native_config_value_to_int_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004071f0), reinterpret_cast<Fn>(&recoil::ConfigValue_ToInt)};
    std::mt19937 rng(0x4071f0);
    const char* names[] = {"", "on", "off", "true", "false", "yes", "no", "high", "low", "medium", "x", "none", "all"};
    int compared = 0;
    for (int it = 0; it < 5000; ++it) {
        std::uint32_t node[2] = {1 + rng() % 5, rng()};
        if (node[0] == 2) {
            const float scales[] = {1.0f, 0.5f, 10.0f, 1000.0f, 1e6f, 3e9f};
            float f = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng) * scales[rng() % 6];
            if (rng() % 8 == 0) f = static_cast<float>(static_cast<int>(rng() % 200) - 100) + 0.5f;  // exact halves
            std::memcpy(&node[1], &f, 4);
        }
        if (node[0] == 3) node[1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(names[rng() % 13]));
        int ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ret[side] = fn[side](node, 0);
        }
        CHECK_EQ(ret[0], ret[1]);
        ++compared;
    }
    std::printf("  ConfigValue_ToInt calls %d (int, float, string and other nodes)\n", compared);
}

// ConfigTree_ParseFileByBasename (0x0048cdc0, path at ECX, one ignored stack word; ret 4): _splitpath keeps only the
// name (0x0056af80) and extension (0x0056ae80), the extension is appended to the name in place, and
// Zar_FindEntryInArchives looks the basename up in the reader list [0x0056b184]; found: a two-word child array
// (ConfigTree_AllocChildArray(4, 1)) is filled by ConfigTree_ReadNode from the archive handle and returned, else 0.
// Each side: its own node pool and reader list of 0..3 archives; an archive's handle is an in-memory stream (fake
// SetFilePointer moves it, logged; the ReadFile fake reads it) holding a random config tree at a TOC entry's offset.
// Paths: drive/dir + name + ext, bare names, missing names, no extension. Compared: return (null or the tree,
// serialised structurally), the name buffer, the seek and read logs.
namespace {
std::vector<std::uint32_t> g_seeks;
DWORD WINAPI stream_set_file_pointer(HANDLE h, LONG dist, LONG* high, DWORD method)
{
    g_seeks.push_back(static_cast<std::uint32_t>(dist));
    g_seeks.push_back(high ? 1u : 0u);
    g_seeks.push_back(method);
    static_cast<Stream*>(h)->pos = static_cast<std::size_t>(dist);
    return static_cast<DWORD>(dist);
}
}  // namespace

TEST(native_config_tree_parse_file_by_basename_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const char*, int, int);
    using C = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    using Free = void(__cdecl*)(void*);
    const Fn fn[2] = {rt::original<Fn>(0x0048cdc0), reinterpret_cast<Fn>(&recoil::ConfigTree_ParseFileByBasename)};
    const C init[2] = {rt::original<C>(0x0048c7d0), reinterpret_cast<C>(&recoil::Container_InitNodePool)};
    const C create[2] = {rt::original<C>(0x0048c950), reinterpret_cast<C>(&recoil::Container_CreateList)};
    const C append[2] = {rt::original<C>(0x0048ca30), reinterpret_cast<C>(&recoil::Container_ListAppend)};
    const auto m_free = reinterpret_cast<Free>(recoil::g_Iat_free_004cc5b4);
    void** o_slot[2] = {reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc134)), reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc144))};
    void** p_slot[2] = {&recoil::g_Iat_ReadFile_004cc134, &recoil::g_Iat_SetFilePointer_004cc144};
    void* const fakes[2] = {reinterpret_cast<void*>(&fake_read_file), reinterpret_cast<void*>(&stream_set_file_pointer)};
    void* saved[4];
    for (int i = 0; i < 2; ++i) {
        saved[i] = *o_slot[i];
        saved[2 + i] = *p_slot[i];
        *o_slot[i] = *p_slot[i] = fakes[i];
    }
    const char* bases[] = {"detail.cfg", "SOUND.CFG", "keys.cfg", "noext", "x.y"};
    const char* dirs[] = {"", "C:\\Recoil\\", "data\\cfg\\", "D:/a/b/", "..\\"};
    std::mt19937 rng(0x48cdc0);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        const int n = static_cast<int>(rng() % 4);
        std::vector<std::vector<unsigned char>> arch(n), tocs(n);
        std::vector<std::uint32_t> counts(n);
        for (int a = 0; a < n; ++a) {
            counts[a] = rng() % 4;
            for (int k = static_cast<int>(rng() % 16); k > 0; --k) arch[a].push_back(static_cast<unsigned char>(rng()));
            tocs[a].resize(0x94 * (counts[a] ? counts[a] : 1));
            for (auto& b : tocs[a]) b = static_cast<unsigned char>(rng());
            for (std::uint32_t e = 0; e < counts[a]; ++e) {
                unsigned char* ent = &tocs[a][0x94 * e];
                const std::uint32_t off = static_cast<std::uint32_t>(arch[a].size());
                std::memcpy(ent, &off, 4);
                char* name = reinterpret_cast<char*>(ent + 8);
                std::memset(name, 0, 0x40);
                std::strcpy(name, bases[rng() % 5]);
                emit_node(arch[a], rng, 0);
            }
        }
        std::string path = std::string(dirs[rng() % 5]) + (rng() % 6 == 0 ? std::string("missing.cfg") : std::string(bases[rng() % 5]));
        std::uint32_t ret[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            init[side](4, 0);
            const std::uint32_t list = create[side](0, 0);
            std::vector<Stream> streams(n);
            std::vector<std::vector<std::uint32_t>> objs(n, std::vector<std::uint32_t>(8));
            for (int a = 0; a < n; ++a) {
                streams[a] = Stream{arch[a], 0};
                std::uint32_t* o = objs[a].data();
                o[0] = 0; o[1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&streams[a])); o[2] = 0;
                o[3] = counts[a]; o[4] = counts[a];
                o[5] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(tocs[a].data()));
                append[side](list, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(o)));
            }
            std::uint32_t* head = static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x0056b184) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056b184)));
            *head = list;
            g_requests.clear();
            g_seeks.clear();
            ret[side] = fn[side](path.c_str(), 0, 0x5A5A5A5A);
            snap[side] = g_requests;
            snap[side].insert(snap[side].end(), g_seeks.begin(), g_seeks.end());
            const char* nm = static_cast<const char*>(side ? recoil::ImageData_Address(0x0056af80) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056af80)));
            for (const char* c = nm; *c; ++c) snap[side].push_back(static_cast<unsigned char>(*c));
            snap[side].push_back(ret[side] ? 1u : 0u);
            if (ret[side]) {
                const auto* node = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(ret[side]));
                if (wd::guarded([&] { serialise(node, snap[side], 0); }) != wd::kOk) snap[side].push_back(0xFA17FA17u);
            }
        }
        const bool same = snap[0] == snap[1];
        for (int side = 0; side < 2 && same; ++side)
            if (ret[side]) {
                auto* node = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(ret[side]));
                if (node[0] >= 1 && node[0] <= 4) free_tree(node, m_free);
                m_free(node);
            }
        CHECK(same);
        ++compared;
    }
    for (int i = 0; i < 2; ++i) {
        *o_slot[i] = saved[i];
        *p_slot[i] = saved[2 + i];
    }
    rt::restore_pristine();
    std::printf("  ConfigTree_ParseFileByBasename calls %d (paths with / without dirs, reader lists of 0..3 stream archives, config trees)\n", compared);
}

// MaterialName_LoadFromConfig (0x00481460, path at ECX): null -> nothing; ConfigTree_ParseFileByBasename(path); its
// first child list's strings 1..count-1: a name MaterialName_FindIndex does not know is copied (malloc) into the
// table 0x004e0fd0 at the count [0x004e0fc8] while the count is below 100; ConfigTree_Destroy. Each side: its own
// name table (count 0..100, names from a pool with prefixes of each other) and one stream archive "mat.cfg" holding
// root {list: strings}; paths null, missing or "..\\x\\mat.cfg". Compared: the count and every table name.
TEST(native_material_name_load_from_config_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const char*, int);
    using C = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const Fn fn[2] = {rt::original<Fn>(0x00481460), reinterpret_cast<Fn>(&recoil::MaterialName_LoadFromConfig)};
    const C init[2] = {rt::original<C>(0x0048c7d0), reinterpret_cast<C>(&recoil::Container_InitNodePool)};
    const C create[2] = {rt::original<C>(0x0048c950), reinterpret_cast<C>(&recoil::Container_CreateList)};
    const C append[2] = {rt::original<C>(0x0048ca30), reinterpret_cast<C>(&recoil::Container_ListAppend)};
    using Dup = char*(__cdecl*)(const char*);
    using Free = void(__cdecl*)(void*);
    const auto m_strdup = reinterpret_cast<Dup>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "_strdup"));
    const auto m_free = reinterpret_cast<Free>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));
    void** o_slot[2] = {reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc134)), reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc144))};
    void** p_slot[2] = {&recoil::g_Iat_ReadFile_004cc134, &recoil::g_Iat_SetFilePointer_004cc144};
    void* const fakes[2] = {reinterpret_cast<void*>(&fake_read_file), reinterpret_cast<void*>(&stream_set_file_pointer)};
    void* saved[4];
    for (int i = 0; i < 2; ++i) {
        saved[i] = *o_slot[i];
        saved[2 + i] = *p_slot[i];
        *o_slot[i] = *p_slot[i] = fakes[i];
    }
    const char* pool[] = {"default", "metal", "wood", "glass", "Metal", "met", "metalic", "rock", "water", "sand", "", "w"};
    std::mt19937 rng(0x481460);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        const std::uint32_t count0 = rng() % 4 == 0 ? 95 + rng() % 6 : rng() % 8;
        std::vector<int> table0(count0);
        const int big_pool[3] = {0, 2, 3};  // near the limit: few distinct names, so config names are new
        for (auto& k : table0) k = count0 >= 95 ? big_pool[rng() % 3] : static_cast<int>(rng() % 12);
        if (count0) table0[0] = 0;
        const std::uint32_t k = rng() % 7;
        std::vector<unsigned char> file;
        auto put = [&](std::uint32_t v) { const auto* b = reinterpret_cast<const unsigned char*>(&v); file.insert(file.end(), b, b + 4); };
        put(4); put(2); put(4); put(k + 1);
        for (std::uint32_t s = 0; s < k; ++s) {
            const char* nm = pool[rng() % 12];
            put(3); put(static_cast<std::uint32_t>(std::strlen(nm)));
            file.insert(file.end(), nm, nm + std::strlen(nm));
        }
        std::vector<unsigned char> toc(0x94, 0);
        std::strcpy(reinterpret_cast<char*>(&toc[8]), "mat.cfg");
        const int pk = static_cast<int>(rng() % 6);
        const char* path = pk == 0 ? nullptr : pk == 1 ? "other.cfg" : "..\\x\\mat.cfg";
        std::vector<std::string> snap[2];
        std::uint32_t count_after[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            auto g = [&](std::uint32_t va) {
                return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
            };
            init[side](4, 0);
            const std::uint32_t list = create[side](0, 0);
            Stream st{file, 0};
            std::uint32_t obj[8] = {0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&st)), 0, 1, 1,
                                    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(toc.data())), 0, 0};
            append[side](list, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(obj)));
            *g(0x0056b184) = list;
            for (std::uint32_t i = 0; i < 100; ++i) *g(0x004e0fd0u + 4 * i) = 0;
            for (std::uint32_t i = 0; i < count0; ++i)
                *g(0x004e0fd0u + 4 * i) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(m_strdup(pool[table0[i]])));
            *g(0x004e0fc8) = count0;
            fn[side](path, 0);
            count_after[side] = *g(0x004e0fc8);
            for (std::uint32_t i = 0; i < 100; ++i) {
                const std::uint32_t p = *g(0x004e0fd0u + 4 * i);
                snap[side].push_back(p ? std::string(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p))) : std::string("<null>"));
                if (p) m_free(reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)));
            }
        }
        CHECK_EQ(count_after[0], count_after[1]);
        CHECK(snap[0] == snap[1]);
        ++compared;
    }
    for (int i = 0; i < 2; ++i) {
        *o_slot[i] = saved[i];
        *p_slot[i] = saved[2 + i];
    }
    rt::restore_pristine();
    std::printf("  MaterialName_LoadFromConfig calls %d (name tables of 0..100, config string lists, null / missing paths)\n", compared);
}
