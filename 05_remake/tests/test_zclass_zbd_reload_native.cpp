// Structured native L1 for GameZ_ReloadNodeModel (0x004557a0; ECX FILE, EDX GameZ header, stack: node, recurse; P2.6).
// index = ClsRecord_ToIndex(node) (pool [0x00539c94]); negative or >= header node count +0x18 -> 1. fseek to the node
// record header +0x20 + 0xC4 * index, fread it (short -> report, 1); fseek to the model section header +0x14; the old
// model (gwNodeGetModel) is taken off the node (gwNodeSetModel 0); Model_ReadOne(FILE, the record's +0x3c) - success:
// the new model goes on the node and the old one is freed (Model_Free: still referenced -> kept); failure: the old model
// goes back. With recurse set, the node's children (+0x5c / +0x60) are reloaded the same way, whatever happened; 0.
// Each call: its own node pool (6 records; the node a tree of up to 6 of them, depth <= 2; model +0x3c none or an old
// model; +0x24 random; no parents), its own model pool [0x00576204] (6 records of 0x58 bytes: old models used with
// reference count 1 or 2, the rest free, [0x00576208] / [0x0057620c]), a real file: 0x24 header bytes, 6 node
// records (+0x3c = a model index, some out of range), a model section (count 0..3, used, free head, 0x58-byte records
// of models with no geometry and type 2..5 - Model_ReadContents is its own test), cut short now and then; header node
// count 0..6. Compared by role: the return, every node word, every model record word, the model pool globals, list 7
// and the link counter, and the file position.
// GameZ_LoadWorldFile (0x00455730; ECX node, EDX recurse): the GameZ path [0x00539ca8] empty -> 1; GameZ_OpenAndCheckHeader
// on it (missing file / bad magic or version -> 1); else GameZ_ReloadNodeModel(FILE, the header just read, node,
// recurse), fclose, its result. Driven by the same harness with the header as the file's first 0x24 bytes (magic and
// version right, or either wrong 1 time in 8), the path a short relative name (the path buffer is small) or empty, the
// file sometimes missing; the file position is not compared (the FILE is closed inside).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_zbd.h"
#include "platform/image/original_data.h"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <random>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t* at(std::uint32_t p) { return reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(p)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
void put(std::vector<unsigned char>& f, std::uint32_t w) { const auto* b = reinterpret_cast<const unsigned char*>(&w); f.insert(f.end(), b, b + 4); }
}  // namespace

namespace {
void run_reload(bool via_path)
{
    using Fn = int(__fastcall*)(void*, const std::uint32_t*, void*, int);
    using Load = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004557a0), reinterpret_cast<Fn>(&recoil::GameZ_ReloadNodeModel)};
    const Load load[2] = {rt::original<Load>(0x00455730), reinterpret_cast<Load>(&recoil::GameZ_LoadWorldFile)};
    HMODULE crt = GetModuleHandleA("msvcrt.dll");
    auto c_fopen = reinterpret_cast<void*(__cdecl*)(const char*, const char*)>(GetProcAddress(crt, "fopen"));
    auto c_fclose = reinterpret_cast<int(__cdecl*)(void*)>(GetProcAddress(crt, "fclose"));
    auto c_fwrite = reinterpret_cast<std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*)>(GetProcAddress(crt, "fwrite"));
    auto c_ftell = reinterpret_cast<long(__cdecl*)(void*)>(GetProcAddress(crt, "ftell"));
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "rnm", 0, path);
    if (via_path) std::strcpy(path, "zcls_lw.zbd");  // relative (working directory): fits the GameZ path buffer
    std::mt19937 rng(via_path ? 0x455730u : 0x4557a0u);
    int missing = 0;
    int compared = 0, replaced = 0, restored = 0;
    for (int it = 0; it < 3000; ++it) {
        // the tree: record 0 the root (or a random record), children among the other records, depth <= 2
        std::vector<std::vector<int>> kids(6);
        const int root = rng() % 4 ? 0 : static_cast<int>(rng() % 6);
        std::vector<int> order{root};
        for (int r = 0; r < 6; ++r) if (r != root && rng() % 3) order.push_back(r);
        for (std::size_t k = 1; k < order.size(); ++k) kids[k < 3 ? order[0] : order[rng() % 3]].push_back(order[k]);  // depth <= 2
        std::uint32_t node_i[6][49], model_i[6][22], rec_i[6][49];
        for (auto& n : node_i) for (auto& w : n) w = rng();
        for (auto& m : model_i) for (auto& w : m) w = rng();
        for (auto& r : rec_i) for (auto& w : r) w = rng();
        int old_model[6];  // per node: model pool record or -1
        const int n_old = static_cast<int>(rng() % 4);
        for (int r = 0; r < 6; ++r) old_model[r] = rng() % 2 && r < n_old ? r : -1;
        for (int m = 0; m < 6; ++m) {
            model_i[m][0] = 2 + rng() % 4;
            model_i[m][2] = m < n_old ? 1 + rng() % 2 : 0;  // reference count
            for (int k = 3; k <= 7; ++k) model_i[m][k] = 0;
            for (int k = 12; k <= 16; ++k) model_i[m][k] = 0;
            model_i[m][21] = m >= n_old && m + 1 < 6 ? static_cast<std::uint32_t>(m + 1) : 0xFFFFFFFFu;  // free list n_old..5
        }
        const std::uint32_t free_head = n_old < 6 ? static_cast<std::uint32_t>(n_old) : 0xFFFFFFFFu;
        // the file
        const std::uint32_t nmodels = rng() % 4;
        std::vector<unsigned char> file;
        const std::size_t header_at = file.size();
        for (int k = 0; k < 9; ++k) put(file, rng());
        const std::uint32_t rec_off = static_cast<std::uint32_t>(file.size());
        for (int r = 0; r < 6; ++r) {
            rec_i[r][0x3C / 4] = rng() % (nmodels + 2);
            for (std::uint32_t w : rec_i[r]) put(file, w);
        }
        const std::uint32_t model_off = static_cast<std::uint32_t>(file.size());
        put(file, nmodels); put(file, rng() % 8); put(file, rng());
        for (std::uint32_t m = 0; m < nmodels; ++m) {
            std::uint32_t mr[22];
            for (auto& w : mr) w = rng();
            mr[0] = 2 + rng() % 4;
            for (int k = 3; k <= 7; ++k) mr[k] = 0;
            for (int k = 12; k <= 16; ++k) mr[k] = 0;
            mr[21] = model_off;  // contents offset (nothing to read)
            for (std::uint32_t w : mr) put(file, w);
        }
        if (rng() % 6 == 0) file.resize(rng() % file.size());
        std::uint32_t header[9];
        for (auto& w : header) w = rng();
        header[0x14 / 4] = model_off;
        header[0x18 / 4] = rng() % 4 ? 6u : rng() % 6;
        header[0x20 / 4] = rec_off;
        const bool recurse = rng() % 3 != 0, null_like = rng() % 30 == 0;  // null_like: a node below the pool
        const unsigned path_kind = via_path ? rng() % 12 : 0;              // 0 empty path, 1 missing file, else a file
        if (via_path) {
            header[0] = rng() % 8 ? 0x02971222u : rng();
            header[1] = rng() % 8 ? 15u : rng() % 32;
            if (file.size() >= header_at + sizeof header) std::memcpy(&file[header_at], header, sizeof header);
        }
        DeleteFileA(path);
        if (path_kind != 1) {
            void* w = c_fopen(path, "wb");
            c_fwrite(file.data(), 1, file.size(), w);
            c_fclose(w);
        }
        missing += via_path && path_kind <= 1;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t area[7 * 49], models[6 * 22], kid_arr[6][6];
            std::uint32_t* const outside = area;       // one record below the pool: index -1 -> 1
            std::uint32_t* const pool = area + 49;
            for (int r = 0; r < 6; ++r) {
                std::uint32_t* n = pool + 49 * r;
                std::memcpy(n, node_i[r], sizeof node_i[r]);
                n[0x3C / 4] = old_model[r] < 0 ? 0u : addr(models + 22 * old_model[r]);
                n[0x54 / 4] = 0;
                for (std::size_t k = 0; k < kids[r].size(); ++k) kid_arr[r][k] = addr(pool + 49 * kids[r][k]);
                n[0x5C / 4] = static_cast<std::uint32_t>(kids[r].size());
                n[0x60 / 4] = kids[r].empty() ? 0u : addr(kid_arr[r]);
            }
            std::memcpy(outside, node_i[0], 49 * 4);
            outside[0x5C / 4] = 0;
            for (int m = 0; m < 6; ++m) std::memcpy(models + 22 * m, model_i[m], sizeof model_i[m]);
            *img(side, 0x00539c94) = addr(pool);
            *img(side, 0x00576204) = addr(models);
            *img(side, 0x00576208) = static_cast<std::uint32_t>(n_old);
            *img(side, 0x0057620c) = free_head;
            void* const node = null_like ? outside : pool + 49 * root;
            void* f = nullptr;
            int r;
            if (via_path) {
                char* gz = reinterpret_cast<char*>(img(side, 0x00539ca8));
                std::strcpy(gz, path_kind == 0 ? "" : path);
                r = load[side](node, recurse ? 1 : 0);
            } else {
                f = c_fopen(path, "rb");
                r = fn[side](f, header, node, recurse ? 1 : 0);
            }
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int k = 0; k < 6; ++k) { role[addr(pool + 49 * k)] = 0xA0u + k; role[addr(models + 22 * k)] = 0xB0u + k; role[addr(kid_arr[k])] = 0xC0u + k; }
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            if (f) {
                snap[side].push_back(static_cast<std::uint32_t>(c_ftell(f)));
                c_fclose(f);
            }
            for (int k = 0; k < 6 * 49; ++k) {
                const int w = k % 49;
                snap[side].push_back(w == 0x3C / 4 || w == 0x60 / 4 ? rl(pool[k]) : pool[k]);
            }
            for (int k = 0; k < 6; ++k) for (int j = 0; j < 6; ++j) snap[side].push_back(rl(kid_arr[k][j]));
            snap[side].insert(snap[side].end(), models, models + 6 * 22);
            snap[side].insert(snap[side].end(), outside, outside + 49);
            snap[side].push_back(*img(side, 0x00576208));
            snap[side].push_back(*img(side, 0x0057620c));
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0) {
                const std::uint32_t now = pool[49 * root + 0x3C / 4], was = old_model[root] < 0 ? 0u : addr(models + 22 * old_model[root]);
                replaced += !null_like && now != was;
                restored += !null_like && r == 0 && now == was;
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  %s calls %d: root model replaced %d, kept / put back %d, empty path / missing file %d\n",
                via_path ? "GameZ_LoadWorldFile" : "GameZ_ReloadNodeModel", compared, replaced, restored, missing);
    CHECK(replaced > 400 && restored > 200);
}
}  // namespace

TEST(native_zclass_gamez_reload_node_model_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    run_reload(false);
}

TEST(native_zclass_gamez_load_world_file_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    run_reload(true);
}
