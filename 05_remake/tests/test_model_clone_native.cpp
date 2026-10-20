// Native L1 for Model_Clone (0x00482270): (source ECX, copy-materials EDX, only-special arg1; ret 4). Takes a record
// from the model pool (Model_Alloc 0x00482080: array [0x00576204] of 0x58-byte records, used count [0x00576208],
// free head [0x0057620c], linked through +0x54; pool full -> returns 0), copies the header and flag bits, and deep
// copies the 0x4C point records (+0x1C / +0x3C, each +0xC x 12 bytes at +0x2C), morph (+0x18 / +0x40), vertices
// (+0x10 / +0x34), normals (+0x14 / +0x38) and polygons (+0xC / +0x30, calloc'd 0x1C each: flags 0x100/0x200, +4,
// +0x18, index list +8, second list +0xC when 0x200, uvs +0x10 when the material is textured). Materials: EDX 0 ->
// shared; else, when arg1 is 0 or Material_HasTextureOrFlags says so, one Material_AllocCopy per distinct source
// material (a realloc'd pair map), else shared.
// Each side gets its own model pool, material state (non-cycling materials; the cycling copy is covered by
// Material_AllocCopy's own test) and source model, built from one random spec. Compared: return value (pool index or
// 0), the clone (every field; buffers by content; material pointers by material index), the pool globals, and the
// material globals and records.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t* global(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
void* m_malloc(std::size_t n) { return reinterpret_cast<void*(__cdecl*)(std::size_t)>(recoil::g_Iat_malloc_004cc5dc)(n); }
void m_free(void* p) { reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4)(p); }

struct Side {
    std::vector<std::uint32_t> pool, mats, src;
    std::vector<std::vector<std::uint32_t>> bufs;  // the source model's buffers
    std::uint32_t* make(std::size_t words, std::mt19937& fill)
    {
        bufs.emplace_back(words ? words : 1);
        for (auto& w : bufs.back()) w = fill();
        return bufs.back().data();
    }
};

// A buffer of n 32-bit words, by content (null -> marker).
void words(std::vector<std::uint32_t>& out, std::uint32_t p, std::uint32_t n)
{
    if (!p) { out.push_back(0x0B0B0B0Bu); return; }
    const auto* w = ptr<std::uint32_t>(p);
    out.insert(out.end(), w, w + n);
}

std::uint32_t mat_role(const Side& s, std::uint32_t p)
{
    const std::uint32_t base = addr(s.mats.data());
    if (p == 0x005669f0u || p == addr(recoil::ImageData_Address(0x005669f0))) return 0xDEF00000u;  // default material (array full)
    return p >= base && p < base + 4 * s.mats.size() ? 0x3A000000u + (p - base) : p;
}

// The clone, field by field (buffers by content, pointers by role); frees its buffers afterwards.
std::vector<std::uint32_t> clone_snapshot(const Side& s, std::uint32_t rec)
{
    std::vector<std::uint32_t> out;
    auto* r = ptr<std::uint32_t>(rec);
    for (int i = 0; i < 22; ++i)
        if (i < 12 || i > 16) out.push_back(r[i]);  // +0x30..+0x40 are buffers, below
    const std::uint32_t d = r[7];
    auto* recs = ptr<std::uint32_t>(r[15]);
    for (std::uint32_t k = 0; recs && k < d && k < 16; ++k) {
        for (int w = 0; w < 19; ++w) if (w != 11) out.push_back(recs[19 * k + w]);
        words(out, recs[19 * k + 11], 3 * recs[19 * k + 3]);
        m_free(ptr<void>(recs[19 * k + 11]));
    }
    m_free(recs);
    words(out, r[16], 3 * r[6]);
    words(out, r[13], 3 * r[4]);
    words(out, r[14], 3 * r[5]);
    m_free(ptr<void>(r[16])); m_free(ptr<void>(r[13])); m_free(ptr<void>(r[14]));
    auto* pl = ptr<std::uint32_t>(r[12]);
    for (std::uint32_t k = 0; pl && k < r[3] && k < 16; ++k) {
        const std::uint32_t* p = pl + 7 * k, n = p[0] & 0xFF;
        out.push_back(p[0]); out.push_back(p[1]); out.push_back(p[6]); out.push_back(mat_role(s, p[5]));
        words(out, p[2], n);
        words(out, p[3], n);
        words(out, p[4], 2 * n);  // uvs: copied for a textured material, else the calloc'd zero (marker)
        m_free(ptr<void>(p[2])); m_free(ptr<void>(p[3])); m_free(ptr<void>(p[4]));
    }
    m_free(pl);
    return out;
}
}  // namespace

TEST(native_model_clone_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const void*, int, int);
    const Fn fn[2] = {rt::original<Fn>(0x00482270), reinterpret_cast<Fn>(&recoil::Model_Clone)};
    std::mt19937 rng(0x482270);
    int compared = 0, shown = 0;
    for (int it = 0; it < 1000; ++it) {
        const std::uint32_t seed = rng();
        const int pool_n = 1 + static_cast<int>(rng() % 4), pool_free = static_cast<int>(rng() % (pool_n + 1));
        const int nmat = 2 + static_cast<int>(rng() % 6), mat_free = static_cast<int>(rng() % (nmat + 1));
        const int copy_mats = static_cast<int>(rng() % 2), only_special = static_cast<int>(rng() % 2);
        std::uint32_t ret[2];
        std::vector<std::uint32_t> snap[2];
        Side sides[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            Side& s = sides[side];
            std::mt19937 spec(seed), fill(seed ^ 0x33333333u);
            // model pool: records 0..pool_free-1 on the free list (linked through +0x54)
            s.pool.assign(22 * pool_n, 0);
            for (auto& w : s.pool) w = fill();
            for (int k = 0; k < pool_n; ++k) s.pool[22 * k + 21] = k + 1 < pool_free ? k + 1 : 0xFFFFFFFFu;
            *global(side, 0x00576204) = addr(s.pool.data());
            *global(side, 0x00576208) = static_cast<std::uint32_t>(pool_n - pool_free);
            *global(side, 0x0057620c) = pool_free ? 0u : 0xFFFFFFFFu;
            // material array: the last mat_free records on the free list, the rest used; non-cycling
            s.mats.assign(11 * nmat, 0);
            for (int k = 0; k < nmat; ++k) {
                std::uint32_t* m = s.mats.data() + 11 * k;
                for (int w = 0; w < 9; ++w) m[w] = spec() % 3;
                m[0] = (m[0] & ~0x0400u) | (spec() % 2 ? 0x100u : 0u) | (spec() % 3 == 0 ? 0x200u : 0u);
                m[9] = 0;
            }
            auto link = [&](int from, int to) {
                for (int k = from; k < to; ++k) {
                    auto* l = reinterpret_cast<std::int16_t*>(s.mats.data() + 11 * k + 10);
                    l[0] = static_cast<std::int16_t>(k > from ? k - 1 : -1);
                    l[1] = static_cast<std::int16_t>(k + 1 < to ? k + 1 : -1);
                }
            };
            link(0, nmat - mat_free);
            link(nmat - mat_free, nmat);
            *global(side, 0x00566a1c) = addr(s.mats.data());
            *global(side, 0x00566a18) = static_cast<std::uint32_t>(nmat);
            *global(side, 0x00566a20) = static_cast<std::uint32_t>(nmat - mat_free);
            *global(side, 0x004e1164) = nmat - mat_free ? 0u : 0xFFFFFFFFu;
            *global(side, 0x004e1160) = mat_free ? static_cast<std::uint32_t>(nmat - mat_free) : 0xFFFFFFFFu;
            *global(side, 0x00566a24) = 0;
            // source model
            s.src.assign(22, 0);
            for (auto& w : s.src) w = fill();
            const std::uint32_t polys = spec() % 5, a = spec() % 5, b = spec() % 5, c = spec() % 5, d = spec() % 4;
            s.src[3] = polys; s.src[4] = a; s.src[5] = b; s.src[6] = c; s.src[7] = d;
            s.src[13] = addr(s.make(3 * a, fill));
            s.src[14] = addr(s.make(3 * b, fill));
            s.src[16] = addr(s.make(3 * c, fill));
            std::uint32_t* recs = s.make(19 * d, fill);
            for (std::uint32_t k = 0; k < d; ++k) {
                const std::uint32_t cnt = spec() % 5;
                recs[19 * k + 3] = cnt;
                recs[19 * k + 11] = addr(s.make(3 * cnt, fill));
            }
            s.src[15] = addr(recs);
            std::uint32_t* pl = s.make(7 * polys, fill);
            for (std::uint32_t k = 0; k < polys; ++k) {
                const std::uint32_t nv = spec() % 6;
                pl[7 * k] = (spec() & 0xFFFFFC00u) | (spec() % 2 ? 0x100u : 0u) | (spec() % 2 ? 0x200u : 0u) | nv;
                pl[7 * k + 2] = addr(s.make(nv, fill));
                pl[7 * k + 3] = spec() % 3 == 0 ? 0 : addr(s.make(nv, fill));
                pl[7 * k + 4] = addr(s.make(2 * nv, fill));
                pl[7 * k + 5] = addr(s.mats.data() + 11 * (spec() % nmat));
            }
            s.src[12] = addr(pl);
            const std::uint32_t r = fn[side](s.src.data(), copy_mats, only_special);
            const std::uint32_t base = addr(s.pool.data());
            ret[side] = r ? 0x9000u + (r - base) / 0x58 : 0;
            snap[side] = {*global(side, 0x00576208), *global(side, 0x0057620c), *global(side, 0x00566a20), *global(side, 0x004e1164),
                          *global(side, 0x004e1160)};
            if (r) {
                const std::vector<std::uint32_t> c2 = clone_snapshot(s, r);
                snap[side].insert(snap[side].end(), c2.begin(), c2.end());
            }
            for (int k = 0; k < nmat; ++k) {  // material records (AllocCopy fills free ones)
                const std::uint32_t* m = s.mats.data() + 11 * k;
                snap[side].insert(snap[side].end(), m, m + 9);
                snap[side].push_back(m[10]);
            }
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        if (snap[0] != snap[1] && shown++ < 3) {
            std::size_t i = 0;
            while (i < snap[0].size() && i < snap[1].size() && snap[0][i] == snap[1][i]) ++i;
            std::printf("    it %d copy %d special %d: first difference at snapshot word %zu of %zu/%zu: %08x vs %08x\n", it, copy_mats, only_special, i, snap[0].size(), snap[1].size(),
                        i < snap[0].size() ? snap[0][i] : 0, i < snap[1].size() ? snap[1][i] : 0);
        }
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Model_Clone calls %d (own model pool, material state and source model per side)\n", compared);
}
