// Native L1 for Pixel_PackRGB (0x004a6d40): three floats at ECX, each minus the constant at 0x004d2f58 and truncated
// by the CRT _ftol (through the thunk 0x004c60a6), packed with the screen format globals: (R & [0x0063217c]) <<
// [0x00632170] | (G & [0x00632180]) << [0x00632174] | B >> [0x00632178], returned in AX. Arena words read as floats are
// denormals that truncate to 0 (a wrong shift then goes unseen), so here: real channel values (0..255, fractions,
// just outside the range) under the real 16-bit formats (565, 555) and random masks/shifts. Compared: AX.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zVideo/zvid_buff.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>

TEST(native_pixel_pack_rgb_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const float*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004a6d40), reinterpret_cast<Fn>(&recoil::Pixel_PackRGB)};
    std::mt19937 rng(0x4a6d40);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0;
    for (int it = 0; it < 20000; ++it) {
        // format: [0x00632170] shift R, [0x00632174] shift G, [0x00632178] shift B, [0x0063217c] mask R, [0x00632180] mask G
        std::uint32_t fmt[5];
        const int kind = static_cast<int>(rng() % 3);
        if (kind == 0) { const std::uint32_t f[5] = {11, 5, 3, 0x1f, 0x3f}; std::memcpy(fmt, f, sizeof fmt); }       // 565
        else if (kind == 1) { const std::uint32_t f[5] = {10, 5, 3, 0x1f, 0x1f}; std::memcpy(fmt, f, sizeof fmt); }  // 555
        else { fmt[0] = rng() % 16; fmt[1] = rng() % 16; fmt[2] = rng() % 8; fmt[3] = rng() & 0xFF; fmt[4] = rng() & 0xFF; }
        float rgb[3];
        for (float& c : rgb) c = rng() % 5 == 0 ? static_cast<float>(rng() % 256) : fr(-2.0f, 258.0f);
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            for (int k = 0; k < 5; ++k) {
                const std::uint32_t va = 0x00632170u + 4 * k;
                std::memcpy(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)), &fmt[k], 4);
            }
            ret[side] = fn[side](rgb, 0) & 0xFFFFu;
        }
        CHECK_EQ(ret[0], ret[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Pixel_PackRGB calls %d (real channel values, 565 / 555 / random formats)\n", compared);
}
