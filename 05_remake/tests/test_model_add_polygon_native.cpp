// Native L1 for Model_AddPolygon (0x00483650) with its mutually recursive helpers Polygon_FanTriangulate (0x00482fe0)
// and Polygon_SplitFan (0x00483240). (model ECX, vertex count EDX; positions arg1, normal source arg2 (optional),
// uvs arg3, use-offset arg4, offset xyz arg5, offset uvs arg6, material arg7, arg8 -> polygon +4, arg9 bit 0 -> flag
// 0x100, arg10 -> pointer to the dword stored at +0x18; ret 0x28; the material is never null). Counts < 3 or > 57 are rejected; collinear vertices are removed in place
// (Polygon_RemoveCollinearVertices); a non-planar polygon is fan-triangulated, one over [0x004e139c] (48) vertices
// split, both re-entering Model_AddPolygon; else vertices (dedup), offsets, normals are added, uvs quantised and a
// 0x1C polygon entry appended (then Polygon_FixupUVs). The inputs are edited in place and the model's arrays grow by
// realloc, so each side gets its own inputs and its own empty model. Each case: 1..6 adds with planar polygons of
// 3..60 vertices, non-planar ones, collinear and duplicate points, fewer than 3 vertices, textured / untextured
// materials, with and without normals and offsets. Compared: every return value and the model by content
// (vertices, offsets, normals, polygons with their lists, material role, +4 and +0x18).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"
#include "watchdog.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
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

struct Add {  // one Model_AddPolygon call, inputs shared by both sides (copied per side)
    int n;
    std::vector<float> pos, nrm, uv, off, offuv;
    bool with_nrm, with_off;
    int material;
    std::uint32_t arg8, arg9, arg10;
};

Add random_add(std::mt19937& rng)
{
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    Add a;
    const int kind = static_cast<int>(rng() % 10);
    a.n = kind == 0 ? static_cast<int>(rng() % 3) : kind == 1 ? 49 + static_cast<int>(rng() % 12) : 3 + static_cast<int>(rng() % 10);
    const int m = a.n > 0 ? a.n : 1;
    a.pos.resize(3 * m); a.nrm.resize(3 * m); a.uv.resize(2 * m); a.off.resize(3 * m); a.offuv.resize(2 * m);
    const bool planar = kind != 2;
    const float cx = fr(-50, 50), cy = fr(-50, 50), r = fr(1, 20);
    for (int i = 0; i < m; ++i) {
        const float t = 6.2831853f * i / m;
        float x = cx + r * std::cos(t), y = cy + r * std::sin(t), z = planar ? 0.25f * x - 0.5f * y + 3.0f : fr(-30, 30);
        if (kind == 3 && i % 3 == 1 && i > 0) { x = 0.5f * (a.pos[3 * (i - 1)] + cx); y = 0.5f * (a.pos[3 * (i - 1) + 1] + cy); z = planar ? 0.25f * x - 0.5f * y + 3.0f : z; }
        if (kind == 4 && i > 0 && rng() % 3 == 0) { x = a.pos[3 * (i - 1)]; y = a.pos[3 * (i - 1) + 1]; z = a.pos[3 * (i - 1) + 2]; }
        a.pos[3 * i] = x; a.pos[3 * i + 1] = y; a.pos[3 * i + 2] = z;
        a.nrm[3 * i] = fr(-1, 1); a.nrm[3 * i + 1] = fr(-1, 1); a.nrm[3 * i + 2] = fr(-1, 1);
        a.uv[2 * i] = fr(-2, 2); a.uv[2 * i + 1] = fr(-2, 2);
        a.off[3 * i] = x + fr(-1, 1); a.off[3 * i + 1] = y + fr(-1, 1); a.off[3 * i + 2] = z + fr(-1, 1);
        a.offuv[2 * i] = fr(-2, 2); a.offuv[2 * i + 1] = fr(-2, 2);
    }
    a.with_nrm = rng() % 2 != 0;
    a.with_off = rng() % 3 == 0;
    a.material = 1 + static_cast<int>(rng() % 2);  // 1 untextured, 2 textured (never null: byte +1 is always tested)
    a.arg8 = rng(); a.arg9 = rng() % 2; a.arg10 = rng();
    return a;
}
}  // namespace

TEST(native_model_add_polygon_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int, const float*, const float*, float*, int, const float*, float*, void*, std::uint32_t,
                                std::uint32_t, std::uint32_t);
    const Fn fn[2] = {rt::original<Fn>(0x00483650), reinterpret_cast<Fn>(&recoil::Model_AddPolygon)};
    // Model_Call483650_Arg2Zero (0x00483610): the same call with the normal source forced to 0 (9 stack args, ret 0x24)
    using Wrap = int(__fastcall*)(void*, int, const float*, float*, int, const float*, float*, void*, std::uint32_t, std::uint32_t,
                                  std::uint32_t);
    const Wrap wrap[2] = {rt::original<Wrap>(0x00483610), reinterpret_cast<Wrap>(&recoil::Model_Call483650_Arg2Zero)};
    std::mt19937 rng(0x483650);
    int compared = 0, faults = 0;
    for (int it = 0; it < 600; ++it) {
        std::vector<Add> adds(1 + rng() % 6);
        for (auto& a : adds) a = random_add(rng);
        // one offset mode per model: mixing leaves offset entries (+0x40) of non-offset vertices as realloc residue (KG-30)
        const bool with_off = rng() % 3 == 0;
        const bool via_wrap = rng() % 4 == 0;  // this sequence through the wrapper
        for (auto& a : adds) a.with_off = with_off;
        std::vector<std::uint32_t> snap[2];
        std::vector<std::pair<std::size_t, int>> marks[2];  // diagnostics: (snapshot index, section)
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t model[22] = {};
            std::uint32_t mats[2][11] = {};
            mats[0][0] = 0x0000u;  // untextured
            mats[1][0] = 0x0100u;  // textured (byte +1 bit 0)
            for (const Add& a0 : adds) {
                Add a = a0;  // inputs are edited in place (collinear removal)
                void* mat = mats[a.material - 1];
                std::uint32_t src = a.arg10;  // arg10 points to the dword stored at polygon +0x18
                int r = 0;
                const unsigned o = wd::guarded([&] {
                    const auto srcp = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&src));
                    r = via_wrap ? wrap[side](model, a.n, a.pos.data(), a.uv.data(), a.with_off ? 1 : 0, a.off.data(), a.offuv.data(), mat,
                                              a.arg8, a.arg9, srcp)
                                 : fn[side](model, a.n, a.pos.data(), a.with_nrm ? a.nrm.data() : nullptr, a.uv.data(), a.with_off ? 1 : 0,
                                            a.off.data(), a.offuv.data(), mat, a.arg8, a.arg9, srcp);
                });
                if (o != wd::kOk && faults++ < 5)
                    std::printf("    it %d side %d: outcome %08x (n %d nrm %d off %d mat %d)\n", it, side, o, a.n, a.with_nrm, a.with_off, a.material);
                marks[side].push_back({snap[side].size(), 100 + (int)marks[side].size()});
                snap[side].push_back(static_cast<std::uint32_t>(r));
                for (float f : a.pos) snap[side].push_back(bits(f));
                for (float f : a.uv) snap[side].push_back(bits(f));
            }
            // the model
            marks[side].push_back({snap[side].size(), 1});
            for (int i = 0; i < 12; ++i) snap[side].push_back(model[i]);
            marks[side].push_back({snap[side].size(), 2});
            words(snap[side], model[13], 3 * model[4]);
            marks[side].push_back({snap[side].size(), 3});
            words(snap[side], model[14], 3 * model[5]);
            marks[side].push_back({snap[side].size(), 4});
            words(snap[side], model[16], 3 * model[6]);
            marks[side].push_back({snap[side].size(), 5});
            const auto* pl = ptr<std::uint32_t>(model[12]);
            for (std::uint32_t k = 0; pl && k < model[3] && k < 512; ++k) {
                const std::uint32_t* p = pl + 7 * k, n = p[0] & 0xFF;
                marks[side].push_back({snap[side].size(), 1000 + (int)k});
                snap[side].push_back(p[0]); snap[side].push_back(p[1]); snap[side].push_back(p[6]);
                snap[side].push_back(p[5] == addr(mats[0]) ? 1u : p[5] == addr(mats[1]) ? 2u : p[5] ? 0xBADu : 0u);
                words(snap[side], p[2], n);
                words(snap[side], p[3], n);
                words(snap[side], p[4], 2 * n);
                m_free(p[2]); m_free(p[3]); m_free(p[4]);
            }
            m_free(model[12]); m_free(model[13]); m_free(model[14]); m_free(model[16]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        if (snap[0] != snap[1] && compared < 400) {
            std::size_t i = 0;
            while (i < snap[0].size() && i < snap[1].size() && snap[0][i] == snap[1][i]) ++i;
            int sec = -1; std::size_t base = 0;
            for (const auto& [at, s] : marks[0]) if (at <= i) { sec = s; base = at; }
            std::printf("    it %d: first difference at word %zu (section %d +%zu) of %zu/%zu: %08x vs %08x\n", it, i, sec, i - base,
                        snap[0].size(), snap[1].size(), i < snap[0].size() ? snap[0][i] : 0, i < snap[1].size() ? snap[1][i] : 0);
            compared += 100;  // report at most a few
        }
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Model_AddPolygon (+ FanTriangulate, SplitFan) sequences done (own model and inputs per side)\n");
}
