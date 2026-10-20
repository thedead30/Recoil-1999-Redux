// Structured native L1 on real floats for float leaves ported as zclass_nodes blockers (P2.6, bottom-up); the arena
// fuzz (test_fuzz_zclass_blockers1.cpp) feeds them mostly denormals.
//   HwBlend_SetColourA (0x004a7220, ECX rgb[3]): [0x006321d0..d8] = rgb * [0x004d2f5c].
//   HwBlend_SetColourB (0x004a7250, ECX rgb[3]): [0x006321dc..e4] = rgb * [0x004d2f5c]; with [0x00632120] set, the index
//     of the largest scaled component goes to [0x00632140] (r >= g ? (r >= b ? 0 : 2) : (g >= b ? 1 : 2): ties to the
//     lower index); unset leaves it.
//   Sound_StreamCue_State3_Cooldown (0x004a5020, ECX cue): cue +0x28 += frame time [0x0056b424]; once it is not below
//     the cue definition's (+0x38) +0x1c: +0x28 = 0, state +0x34 = 4.
//   Sound_LoadFloatArg (0x0049fa00, one float stack argument): returns it in ST0.
//   RenderState_SetColour (0x004762c0, ECX rgb[3]): copies it to [0x0057d938..40]; with [0x0056bbe8] set, a tail jump
//     into HwBlend_SetColourA.
// Values from a small set with repeats (ties), zero and negatives. Compared: the returns (ST0 bits for LoadFloatArg), the
// globals written and every word of the cue.
//   Render_ComputeBoundingSphereFromCorners (0x00452650, ECX 8 corners of 3 floats, EDX centre out, one stack argument:
//     radius out): min / max per axis over the corners, centre = min + (max - min) * [0x004d23c0], radius = the fast
//     square root (bits >> 1) + 0x1fc00000 of the squared half-diagonal. Corners from boxes (the real input) and from
//     unrelated points. Compared: the centre and radius bits.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zSound/zsnd_grp.h"
#include "GameZRecoil/zSound/zsnd_parm.h"
#include "GameZRecoil/zVideo/zvid_buff.h"
#include "unattributed/render_frame.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t fbits(float f) { std::uint32_t u; std::memcpy(&u, &f, 4); return u; }
const float kF[] = {0.0f, 0.25f, 0.5f, 1.0f, 1.0f, 2.0f, 3.5f, 255.0f, -1.0f, 0.016f, 10.0f, 0.1f};
float rf(std::mt19937& rng) { return kF[rng() % 12]; }
}  // namespace

TEST(native_zclass_blockers_float_leaves_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F1 = int(__fastcall*)(const float*, int);
    using Cue = int(__fastcall*)(std::uint32_t*, int);
    using Load = float(__stdcall*)(float);  // ECX / EDX unused; the callee pops its one argument
    const F1 a[2] = {rt::original<F1>(0x004a7220), reinterpret_cast<F1>(&recoil::HwBlend_SetColourA)};
    const F1 b[2] = {rt::original<F1>(0x004a7250), reinterpret_cast<F1>(&recoil::HwBlend_SetColourB)};
    const Cue cue[2] = {rt::original<Cue>(0x004a5020), reinterpret_cast<Cue>(&recoil::Sound_StreamCue_State3_Cooldown)};
    const Load load[2] = {rt::original<Load>(0x0049fa00), reinterpret_cast<Load>(&recoil::Sound_LoadFloatArg)};
    const F1 rs[2] = {rt::original<F1>(0x004762c0), reinterpret_cast<F1>(&recoil::RenderState_SetColour)};
    std::mt19937 rng(0x4a7250);
    int compared = 0, per_index[3] = {}, cooled = 0;
    for (int it = 0; it < 20000; ++it) {
        const int f = it % 5;
        const float rgb[3] = {rf(rng), rf(rng), rf(rng)};
        const std::uint32_t flag = rng() % 4 ? 1u : 0u, prev = rng() % 5, hw = rng() % 2;
        std::uint32_t cue_init[16], def[16];
        for (auto& w : cue_init) w = rng();
        for (auto& w : def) w = rng();
        cue_init[0x28 / 4] = fbits(rf(rng));
        def[0x1c / 4] = fbits(rf(rng));
        const float dt = rf(rng);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *img(side, 0x00632120) = flag;
            *img(side, 0x00632140) = prev;
            *img(side, 0x0056bbe8) = hw;
            *reinterpret_cast<float*>(img(side, 0x0056b424)) = dt;
            std::uint32_t c[16], d[16];
            std::memcpy(c, cue_init, sizeof c);
            std::memcpy(d, def, sizeof d);
            c[0x38 / 4] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(d));
            if (f == 0) a[side](rgb, 0);
            if (f == 1) b[side](rgb, 0);
            if (f == 2) cue[side](c, 0);
            if (f == 3) { const float r = load[side](rgb[0]); snap[side].push_back(fbits(r)); }
            if (f == 4) rs[side](rgb, 0);
            for (std::uint32_t va = 0x0057d938; va <= 0x0057d940; va += 4) snap[side].push_back(*img(side, va));
            for (std::uint32_t va = 0x006321d0; va <= 0x006321e4; va += 4) snap[side].push_back(*img(side, va));
            snap[side].push_back(*img(side, 0x00632140));
            for (int k = 0; k < 16; ++k) if (k != 0x38 / 4) snap[side].push_back(c[k]);
            if (side == 0 && f == 1 && flag) ++per_index[*img(0, 0x00632140) % 3];
            if (side == 0 && f == 2) cooled += c[0x34 / 4] == 4 && cue_init[0x34 / 4] != 4;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  float leaves calls %d: SetColourB largest index 0/1/2 %d/%d/%d, cooldowns ended %d\n", compared, per_index[0],
                per_index[1], per_index[2], cooled);
    CHECK(per_index[0] > 300 && per_index[1] > 300 && per_index[2] > 300 && cooled > 1000);
}

TEST(native_zclass_bounding_sphere_from_corners_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const float*, float*, float*);
    const Fn fn[2] = {rt::original<Fn>(0x00452650), reinterpret_cast<Fn>(&recoil::Render_ComputeBoundingSphereFromCorners)};
    const float vals[] = {0.0f, 1.0f, -1.0f, 2.5f, -2.5f, 10.0f, -100.0f, 0.125f, 33.0f, -0.5f};
    std::mt19937 rng(0x452650);
    int compared = 0;
    for (int it = 0; it < 20000; ++it) {
        float c[24];
        if (rng() % 3) {  // a box: every combination of min / max per axis, in corner order
            float lo[3], hi[3];
            for (int a = 0; a < 3; ++a) { lo[a] = vals[rng() % 10]; hi[a] = vals[rng() % 10]; }
            for (int k = 0; k < 8; ++k) for (int a = 0; a < 3; ++a) c[3 * k + a] = (k >> a) & 1 ? hi[a] : lo[a];
        } else {
            for (float& v : c) v = vals[rng() % 10] * (rng() % 2 ? 1.0f : 0.37f);
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            float centre[4] = {-7.0f, -7.0f, -7.0f, -7.0f}, radius[2] = {-7.0f, -7.0f};
            fn[side](c, centre, radius);
            for (float v : centre) snap[side].push_back(fbits(v));
            for (float v : radius) snap[side].push_back(fbits(v));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  Render_ComputeBoundingSphereFromCorners calls %d (boxes and scattered points)\n", compared);
}
