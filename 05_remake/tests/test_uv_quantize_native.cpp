// Native L1 for UV_QuantizeAndRebase (0x00483510): (count ECX, uv pairs EDX) - each u, v = ftol((x + 1/512) * 256) /
// 256 (CRT _ftol through the thunk 0x004c60a6, floor through its import), then floor(min u) / floor(min v) is
// subtracted from every pair. Random arena words read as denormals or NaNs, so here: real uv arrays of 0..40 pairs -
// fractions, negatives, large magnitudes, exact quantisation steps and values just off them. Compared: every uv (as
// bits). EAX is not compared: it holds what msvcrt's floor left there - the function returns nothing.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

TEST(native_uv_quantize_and_rebase_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(int, float*);
    const Fn fn[2] = {rt::original<Fn>(0x00483510), reinterpret_cast<Fn>(&recoil::UV_QuantizeAndRebase)};
    std::mt19937 rng(0x483510);
    int compared = 0;
    for (int it = 0; it < 5000; ++it) {
        const int n = static_cast<int>(rng() % 41);
        std::vector<float> uv(2 * (n ? n : 1));
        const float scale = std::vector<float>{1.0f, 4.0f, 0.01f, 100.0f, 3000.0f}[rng() % 5];
        const float shift = static_cast<float>(static_cast<int>(rng() % 9) - 4);
        for (auto& x : uv) {
            x = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng) * scale + shift;
            if (rng() % 6 == 0) x = static_cast<float>(static_cast<int>(rng() % 2048) - 1024) / 256.0f;  // exact steps
            if (rng() % 10 == 0) x -= 1.0f / 512.0f;  // just below the rounding bias
        }
        std::vector<float> out[2];
        for (int side = 0; side < 2; ++side) {
            out[side] = uv;
            fn[side](n, out[side].data());  // EAX: whatever msvcrt's floor left there (the function returns nothing)
        }
        CHECK(std::memcmp(out[0].data(), out[1].data(), 4 * out[0].size()) == 0);
        ++compared;
    }
    std::printf("  UV_QuantizeAndRebase calls %d (uv arrays of 0..40 pairs)\n", compared);
}
