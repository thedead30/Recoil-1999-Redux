// Native L1 for Model_ReadContents (0x00481c50): (FILE ECX, model EDX), counts already in the model (+0xC polygons,
// +0x10 vertices, +0x14 normals, +0x18 morph, +0x1C point records). Reads, each into a new block: vertices -> +0x34,
// normals -> +0x38, morph -> +0x40 (12 bytes each); point records -> +0x3C (0x4C each), then per record its colour
// word from the floats +0x1C/+0x20/+0x24 (_ftol(x - 0.5) -> Pixel_FromRGBBytes) and its point list (+0xC x 12 bytes
// -> +0x2C); polygons -> +0x30 (0x1C each, +0x14 material index -> Material_FromIndex, base [0x00566a1c]); per
// polygon (n = low byte of +0) indices -> +8, normal indices -> +0xC when flag 0x200, uvs (8 bytes) -> +0x10 when the
// material is textured (byte +1 bit 0). A short read reports and returns -1.
// Real files written in that layout (and cut short at a random point for the error paths); each side reads its own
// open FILE with its own material array and screen format. Compared: return value; on success the model by content,
// on an error which buffers were allocated (a short fread leaves the rest of its malloc'd block uninitialised).
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
void m_free(std::uint32_t p) { reinterpret_cast<void(__cdecl*)(void*)>(recoil::g_Iat_free_004cc5b4)(ptr<void>(p)); }
void words(std::vector<std::uint32_t>& out, std::uint32_t p, std::uint32_t n)
{
    if (!p) { out.push_back(0x0B0B0B0Bu); return; }
    const auto* w = ptr<std::uint32_t>(p);
    out.insert(out.end(), w, w + n);
}
}  // namespace

TEST(native_model_read_contents_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x00481c50), reinterpret_cast<Fn>(&recoil::Model_ReadContents)};
    const auto m_fopen = reinterpret_cast<Fopen>(recoil::g_Iat_fopen_004cc5b8);
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    const auto m_fwrite = reinterpret_cast<Fwrite>(recoil::g_Iat_fwrite_004cc594);
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, "mrc", 0, path);
    std::mt19937 rng(0x481c50);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0;
    for (int it = 0; it < 800; ++it) {
        const std::uint32_t polys = rng() % 5, verts = rng() % 6, normals = rng() % 6, morph = rng() % 4, recs = rng() % 4;
        const int nmat = 3;
        std::vector<std::uint32_t> mat_flags(nmat);
        for (auto& f : mat_flags) f = rng() % 2 ? 0x100u : 0u;
        std::vector<unsigned char> file;
        auto put = [&](std::uint32_t w) { const auto* b = reinterpret_cast<const unsigned char*>(&w); file.insert(file.end(), b, b + 4); };
        for (std::uint32_t k = 0; k < 3 * (verts + normals + morph); ++k) put(bits(fr(-100, 100)));
        std::vector<std::uint32_t> list_len(recs);
        for (std::uint32_t r = 0; r < recs; ++r) {
            std::uint32_t rec[19];
            for (auto& w : rec) w = rng();
            list_len[r] = rng() % 4;
            rec[3] = list_len[r];
            rec[7] = bits(fr(0, 255)); rec[8] = bits(fr(0, 255)); rec[9] = bits(fr(0, 255));
            for (auto w : rec) put(w);
        }
        for (std::uint32_t r = 0; r < recs; ++r)
            for (std::uint32_t k = 0; k < 3 * list_len[r]; ++k) put(bits(fr(-10, 10)));
        std::vector<std::uint32_t> pn(polys), pflag(polys), pmat(polys);
        for (std::uint32_t p = 0; p < polys; ++p) {
            std::uint32_t e[7];
            for (auto& w : e) w = rng();
            pn[p] = 1 + rng() % 5;
            pflag[p] = rng() % 2 ? 0x200u : 0u;
            pmat[p] = rng() % nmat;
            e[0] = (e[0] & 0xFFFFFC00u) | pflag[p] | pn[p];
            e[5] = pmat[p];
            for (auto w : e) put(w);
        }
        for (std::uint32_t p = 0; p < polys; ++p) {
            for (std::uint32_t k = 0; k < pn[p]; ++k) put(rng() % 16);
            if (pflag[p]) for (std::uint32_t k = 0; k < pn[p]; ++k) put(rng() % 16);
            if (mat_flags[pmat[p]]) for (std::uint32_t k = 0; k < 2 * pn[p]; ++k) put(bits(fr(-2, 2)));
        }
        if (rng() % 4 == 0 && !file.empty()) file.resize(rng() % file.size());  // short file: an error path
        void* w = m_fopen(path, "wb");
        m_fwrite(file.data(), 1, file.size(), w);
        m_fclose(w);
        int ret[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const std::uint32_t fmt[5] = {11, 5, 3, 0x1f, 0x3f};  // 565
            for (int k = 0; k < 5; ++k) *img(side, 0x00632170u + 4 * k) = fmt[k];
            std::uint32_t mats[3][11] = {};
            for (int k = 0; k < nmat; ++k) mats[k][0] = mat_flags[k];
            *img(side, 0x00566a1c) = addr(mats);
            std::uint32_t model[22] = {};
            model[3] = polys; model[4] = verts; model[5] = normals; model[6] = morph; model[7] = recs;
            void* f = m_fopen(path, "rb");
            ret[side] = fn[side](f, model);
            m_fclose(f);
            snap[side].push_back(static_cast<std::uint32_t>(ret[side]));
            const int ptrs[5] = {12, 13, 14, 15, 16};
            for (int k : ptrs) snap[side].push_back(model[k] ? 1u : 0u);
            if (ret[side] == 0) {
                words(snap[side], model[13], 3 * verts);
                words(snap[side], model[14], 3 * normals);
                words(snap[side], model[16], 3 * morph);
                const auto* rr = ptr<std::uint32_t>(model[15]);
                for (std::uint32_t r = 0; rr && r < recs; ++r) {
                    for (int k = 0; k < 19; ++k) if (k != 11) snap[side].push_back(rr[19 * r + k]);
                    words(snap[side], rr[19 * r + 11], 3 * list_len[r]);
                    if (list_len[r]) m_free(rr[19 * r + 11]);  // an empty list is not read: +0x2C keeps the file's word
                }
                const auto* pl = ptr<std::uint32_t>(model[12]);
                for (std::uint32_t p = 0; pl && p < polys; ++p) {
                    const std::uint32_t* e = pl + 7 * p;
                    snap[side].push_back(e[0]); snap[side].push_back(e[1]); snap[side].push_back(e[6]);
                    snap[side].push_back(e[5] ? (e[5] - addr(mats)) / 0x2C : 0xFFFFFFFFu);
                    words(snap[side], e[2], pn[p]);
                    words(snap[side], e[3], pflag[p] ? pn[p] : 0);
                    words(snap[side], e[4], mat_flags[pmat[p]] ? 2 * pn[p] : 0);
                    m_free(e[2]);  // +0xC / +0x10 keep the file's raw words unless their list was read
                    if (pflag[p]) m_free(e[3]);
                    if (mat_flags[pmat[p]]) m_free(e[4]);
                }
            }
            for (int k : ptrs) m_free(model[k]);  // on an error the inner lists of a partial read are leaked
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path);
    rt::restore_pristine();
    std::printf("  Model_ReadContents calls %d (real files in the model layout, short files for the error paths)\n", compared);
}
