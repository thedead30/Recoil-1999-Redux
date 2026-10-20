// Native L1 for the model-section readers of Model_WriteSection's layout: a header (count, used, free head), count
// 0x58-byte records (+0x54 = the model's contents offset in the file), then each model's contents in order (the layout
// Model_ReadContents 0x00481c50 reads - vertices, normals, morph, point records and lists, polygons and lists).
// Model_ReadSection (0x00481fa0, FILE ECX): header into [0x00576200] count / [0x00576208] used / [0x0057620c] free
// head (short -> -1); count 0 -> 0; allocates the array [0x00576204] or reallocs it when larger than before; reads
// the records (short -> -1), then Model_ReadContents per model in order; returns count.
// Model_ReadOne (0x00481aa0, FILE ECX, index EDX): header (short -> 0); count 0 or index >= count -> 0; seeks to the
// record, takes a pool record (Model_Alloc: [0x00576204]/[0x00576208]/[0x0057620c], full -> 0), copies 0x54 bytes,
// seeks to +0x54 and reads the contents; an error frees it again (Model_Free) and returns 0; else the model.
// Real files (some cut short); each side its own FILE, pool / previous array, material array and 565 format.
// Compared: return value (pool index for ReadOne), the globals and every model read (fields raw except its buffers,
// buffers by content; material pointers by index).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
using Fopen = void*(__cdecl*)(const char*, const char*);
using Fclose = int(__cdecl*)(void*);
using Fwrite = std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*);
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t bits(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
void* m_malloc(std::size_t n) { return reinterpret_cast<void*(__cdecl*)(std::size_t)>(recoil::g_Iat_malloc_004cc5dc)(n); }
void m_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4)(ptr<void>(p)); }
void words(std::vector<std::uint32_t>& out, std::uint32_t p, std::uint32_t n)
{
    if (!p) { out.push_back(0x0B0B0B0Bu); return; }
    const auto* w = ptr<std::uint32_t>(p);
    out.insert(out.end(), w, w + n);
}

struct Spec {  // one model: its header counts and what its contents hold
    std::uint32_t rec[22];
    std::vector<std::uint32_t> list_len, pn, pflag, pmat;
};
const std::uint32_t kMatFlags[3] = {0, 0x100, 0x100};  // untextured, textured, textured

// Appends one model's contents (Model_ReadContents layout) and fills its spec.
void emit_contents(std::vector<unsigned char>& f, Spec& s, std::mt19937& rng)
{
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    auto put = [&](std::uint32_t w) { const auto* b = reinterpret_cast<const unsigned char*>(&w); f.insert(f.end(), b, b + 4); };
    const std::uint32_t polys = s.rec[3], verts = s.rec[4], normals = s.rec[5], morph = s.rec[6], recs = s.rec[7];
    for (std::uint32_t k = 0; k < 3 * (verts + normals + morph); ++k) put(bits(fr(-100, 100)));
    s.list_len.assign(recs, 0);
    for (std::uint32_t r = 0; r < recs; ++r) {
        std::uint32_t rc[19];
        for (auto& w : rc) w = rng();
        s.list_len[r] = 1 + rng() % 3;
        rc[3] = s.list_len[r];
        rc[7] = bits(fr(0, 255)); rc[8] = bits(fr(0, 255)); rc[9] = bits(fr(0, 255));
        for (auto w : rc) put(w);
    }
    for (std::uint32_t r = 0; r < recs; ++r) for (std::uint32_t k = 0; k < 3 * s.list_len[r]; ++k) put(bits(fr(-10, 10)));
    s.pn.assign(polys, 0); s.pflag.assign(polys, 0); s.pmat.assign(polys, 0);
    for (std::uint32_t p = 0; p < polys; ++p) {
        std::uint32_t e[7];
        for (auto& w : e) w = rng();
        s.pn[p] = 1 + rng() % 5;
        s.pflag[p] = rng() % 2 ? 0x200u : 0u;
        s.pmat[p] = rng() % 3;
        e[0] = (e[0] & 0xFFFFFC00u) | s.pflag[p] | s.pn[p];
        e[5] = s.pmat[p];
        for (auto w : e) put(w);
    }
    for (std::uint32_t p = 0; p < polys; ++p) {
        for (std::uint32_t k = 0; k < s.pn[p]; ++k) put(rng() % 16);
        if (s.pflag[p]) for (std::uint32_t k = 0; k < s.pn[p]; ++k) put(rng() % 16);
        if (kMatFlags[s.pmat[p]]) for (std::uint32_t k = 0; k < 2 * s.pn[p]; ++k) put(bits(fr(-2, 2)));
    }
}

// A model after a successful read, by content; frees its buffers.
void model_snapshot(std::vector<std::uint32_t>& out, const std::uint32_t* m, const Spec& s, std::uint32_t mats)
{
    for (int k = 0; k < 22; ++k) if (k < 12 || k > 16) out.push_back(m[k]);
    words(out, m[4] ? m[13] : 0, 3 * m[4]);
    words(out, m[5] ? m[14] : 0, 3 * m[5]);
    words(out, m[6] ? m[16] : 0, 3 * m[6]);
    const auto* rr = m[7] ? ptr<std::uint32_t>(m[15]) : nullptr;
    for (std::uint32_t r = 0; rr && r < s.list_len.size(); ++r) {
        for (int k = 0; k < 19; ++k) if (k != 11) out.push_back(rr[19 * r + k]);
        words(out, rr[19 * r + 11], 3 * s.list_len[r]);
        m_free(rr[19 * r + 11]);
    }
    const auto* pl = m[3] ? ptr<std::uint32_t>(m[12]) : nullptr;
    for (std::uint32_t p = 0; pl && p < s.pn.size(); ++p) {
        const std::uint32_t* e = pl + 7 * p;
        out.push_back(e[0]); out.push_back(e[1]); out.push_back(e[6]);
        out.push_back(e[5] ? (e[5] - mats) / 0x2C : 0xFFFFFFFFu);
        words(out, e[2], s.pn[p]);
        words(out, e[3], s.pflag[p] ? s.pn[p] : 0);
        words(out, e[4], kMatFlags[s.pmat[p]] ? 2 * s.pn[p] : 0);
        m_free(e[2]);
        if (s.pflag[p]) m_free(e[3]);
        if (kMatFlags[s.pmat[p]]) m_free(e[4]);
    }
    // a buffer pointer is replaced only when its count is non-zero: an empty one keeps the file's raw word
    if (m[3]) m_free(m[12]);
    if (m[4]) m_free(m[13]);
    if (m[5]) m_free(m[14]);
    if (m[7]) m_free(m[15]);
    if (m[6]) m_free(m[16]);
}

// A whole section file: header, records (offsets filled in), contents in order.
std::vector<unsigned char> build_file(std::vector<Spec>& specs, std::mt19937& rng)
{
    std::vector<unsigned char> head, contents;
    auto put = [](std::vector<unsigned char>& f, std::uint32_t w) { const auto* b = reinterpret_cast<const unsigned char*>(&w); f.insert(f.end(), b, b + 4); };
    const auto n = static_cast<std::uint32_t>(specs.size());
    put(head, n); put(head, rng() % 20); put(head, rng());
    const std::uint32_t base = 12 + 0x58 * n;
    for (Spec& s : specs) {
        for (auto& w : s.rec) w = rng();
        s.rec[3] = rng() % 4; s.rec[4] = rng() % 5; s.rec[5] = rng() % 5; s.rec[6] = rng() % 3; s.rec[7] = rng() % 3;
        s.rec[21] = base + static_cast<std::uint32_t>(contents.size());
        emit_contents(contents, s, rng);
    }
    for (const Spec& s : specs) for (auto w : s.rec) put(head, w);
    head.insert(head.end(), contents.begin(), contents.end());
    return head;
}

struct World {  // one side's globals: 565 format, material array
    std::uint32_t mats[3][11] = {};
    void install(int side)
    {
        const std::uint32_t fmt[5] = {11, 5, 3, 0x1f, 0x3f};
        for (int k = 0; k < 5; ++k) *img(side, 0x00632170u + 4 * k) = fmt[k];
        for (int k = 0; k < 3; ++k) mats[k][0] = kMatFlags[k];
        *img(side, 0x00566a1c) = addr(mats);
    }
};
}  // namespace

TEST(native_model_read_section_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00481fa0), reinterpret_cast<Fn>(&recoil::Model_ReadSection)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "mrs", 0, path);
    std::mt19937 rng(0x481fa0);
    int compared = 0;
    for (int it = 0; it < 500; ++it) {
        std::vector<Spec> specs(it % 10 == 0 ? 0 : 1 + rng() % 4);
        std::vector<unsigned char> file = build_file(specs, rng);
        const bool cut = rng() % 4 == 0 && !file.empty();
        if (cut) file.resize(rng() % file.size());
        const int prev = static_cast<int>(rng() % 3);
        const int n = static_cast<int>(specs.size());
        const int prev_n = prev == 1 ? (n > 1 ? n - 1 : 1) : n + 2;
        void* w = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), w);
        m_fclose(w);
        int ret[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            World wd;
            wd.install(side);
            *img(side, 0x00576204) = prev ? addr(m_malloc(0x58 * prev_n)) : 0;
            *img(side, 0x00576200) = prev ? static_cast<std::uint32_t>(prev_n) : 0;
            void* f = m_fopen(path, "rb");
            ret[side] = fn[side](f, 0);
            m_fclose(f);
            const std::uint32_t arr = *img(side, 0x00576204);
            snap[side] = {static_cast<std::uint32_t>(ret[side]), *img(side, 0x00576200), *img(side, 0x00576208), *img(side, 0x0057620c), arr ? 1u : 0u};
            // Model_ReadSection ignores Model_ReadContents' result: from a short file it still returns the count with
            // later models only partly read, so their contents are compared only for complete files
            if (ret[side] > 0 && !cut)
                for (int k = 0; k < n; ++k) model_snapshot(snap[side], ptr<std::uint32_t>(arr) + 22 * k, specs[k], addr(wd.mats));
            m_free(arr);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  Model_ReadSection calls %d (real multi-model files, previous array none/smaller/larger, short files)\n", compared);
}

TEST(native_model_read_one_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00481aa0), reinterpret_cast<Fn>(&recoil::Model_ReadOne)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "mro", 0, path);
    std::mt19937 rng(0x481aa0);
    int compared = 0;
    for (int it = 0; it < 500; ++it) {
        std::vector<Spec> specs(it % 10 == 0 ? 0 : 1 + rng() % 4);
        std::vector<unsigned char> file = build_file(specs, rng);
        if (rng() % 4 == 0 && !file.empty()) file.resize(rng() % file.size());
        const int index = static_cast<int>(rng() % 6);  // may be out of range
        const int pool_n = 1 + static_cast<int>(rng() % 3), pool_free = static_cast<int>(rng() % (pool_n + 1));
        void* w = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), w);
        m_fclose(w);
        std::uint32_t ret[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            World wd;
            wd.install(side);
            std::vector<std::uint32_t> pool(22 * pool_n, 0);
            for (int k = 0; k < pool_n; ++k) pool[22 * k + 21] = k + 1 < pool_free ? k + 1 : 0xFFFFFFFFu;
            *img(side, 0x00576204) = addr(pool.data());
            *img(side, 0x00576208) = static_cast<std::uint32_t>(pool_n - pool_free);
            *img(side, 0x0057620c) = pool_free ? 0u : 0xFFFFFFFFu;
            void* f = m_fopen(path, "rb");
            const std::uint32_t r = fn[side](f, index);
            m_fclose(f);
            ret[side] = r ? 0x9000u + (r - addr(pool.data())) / 0x58 : 0;
            snap[side] = {*img(side, 0x00576208), *img(side, 0x0057620c)};
            if (r) model_snapshot(snap[side], ptr<std::uint32_t>(r), specs[index], addr(wd.mats));
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  Model_ReadOne calls %d (real files, indices in and out of range, pool free / full, short files)\n", compared);
}
