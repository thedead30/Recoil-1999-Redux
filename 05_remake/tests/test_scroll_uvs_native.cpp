// Native L1 for Model_ScrollUVs (0x004791c0): (texture record ECX - log2 sizes at bytes +0xA/+0xB; uv pairs EDX;
// rate pointer arg1 - two floats; count arg2; ret 8). uv += rate * dt [0x0056b424] (both axes, or one when the other
// rate is 0), then floor(min)/ceil(max) via _ftol and, if the range leaves +-R >> shift (R = [0x0056bbe8] ? 0x80 :
// 0x800), every pair is shifted back by an integer offset. Here: real uv arrays of 0..40 pairs placed near and past
// the wrap range, rates with either axis zero, dt values, both range flags, log2 sizes 0..10. Compared: every uv (as
// bits); EAX is not (the function returns nothing).
#include "test.h"
#include "native_oracle.h"
#include "unattributed/asset_io_misc.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

TEST(native_model_scroll_uvs_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const unsigned char*, float*, const float*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004791c0), reinterpret_cast<Fn>(&recoil::Model_ScrollUVs)};
    std::mt19937 rng(0x4791c0);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0;
    for (int it = 0; it < 5000; ++it) {
        unsigned char tex[16] = {};
        tex[0xA] = static_cast<unsigned char>(rng() % 11);
        tex[0xB] = static_cast<unsigned char>(rng() % 11);
        const std::uint32_t flag = rng() % 2;
        const float dt = std::vector<float>{0.0f, 0.016f, 0.05f, 0.1f, 1.0f}[rng() % 5];
        float rate[2] = {rng() % 4 ? fr(-20, 20) : 0.0f, rng() % 4 ? fr(-20, 20) : 0.0f};
        const int n = static_cast<int>(rng() % 41);
        const float range = static_cast<float>((flag ? 0x80 : 0x800) >> (rng() % 11));
        const float centre = rng() % 3 == 0 ? fr(-1.5f, 1.5f) * range : fr(-4, 4);
        std::vector<float> uv(2 * (n ? n : 1));
        for (auto& x : uv) x = centre + fr(-3, 3);
        std::vector<float> out[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(side ? recoil::ImageData_Address(0x0056bbe8) : reinterpret_cast<void*>(0x0056bbe8), &flag, 4);
            std::memcpy(side ? recoil::ImageData_Address(0x0056b424) : reinterpret_cast<void*>(0x0056b424), &dt, 4);
            out[side] = uv;
            fn[side](tex, out[side].data(), rate, n);  // EAX: leftover (no return value)
        }
        CHECK(std::memcmp(out[0].data(), out[1].data(), 4 * out[0].size()) == 0);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Model_ScrollUVs calls %d (uv arrays near the wrap range, rates, dt, range flag)\n", compared);
}
