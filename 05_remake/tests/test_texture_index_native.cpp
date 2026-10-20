// Native L1 for Texture_ToIndex (0x0046d310) and Texture_FromIndex (0x0046d340): both do arithmetic on the address
// of the texture table in .bss (0x0053d79c, 0x24-byte entries), so random arena pointers (tests/arena_fuzz.h) give
// address-relative results that cannot agree. Each side gets pointers into its own table instead: the original's
// at 0x0053d79c, the port's where the data image keeps it (recoil::ImageData_Address). ToIndex: null, and entry
// offsets -3..600 with byte offsets 0..0x23 - results must be equal. FromIndex: indices -2..700 - the port's result
// must be the original's mapped into the data image (0 for -1).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zImage/zimg_texture.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>

TEST(native_texture_index_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t, int);
    auto o_to = rt::original<Fn>(0x0046d310);
    auto o_from = rt::original<Fn>(0x0046d340);
    auto p_to = reinterpret_cast<Fn>(&recoil::Texture_ToIndex);
    auto p_from = reinterpret_cast<Fn>(&recoil::Texture_FromIndex);
    const std::uint32_t o_table = 0x0053d79c;
    const auto p_table = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(recoil::ImageData_Address(o_table)));
    CHECK(p_table != 0);
    int compared = 0;
    CHECK_EQ(o_to(0, 0), p_to(0, 0));
    ++compared;
    for (int k = -3; k <= 600; ++k)
        for (int off = 0; off < 0x24; off += 5) {
            const std::uint32_t d = static_cast<std::uint32_t>(k * 0x24 + off);
            CHECK_EQ(o_to(o_table + d, 0), p_to(p_table + d, 0));
            ++compared;
        }
    for (int i = -2; i <= 700; ++i) {
        const std::uint32_t o = o_from(static_cast<std::uint32_t>(i), 0);
        const std::uint32_t p = p_from(static_cast<std::uint32_t>(i), 0);
        CHECK_EQ(o == 0 ? 0u : p_table + (o - o_table), p);
        ++compared;
    }
    std::printf("  Texture_ToIndex/FromIndex calls %d (pointers into each side's own table; indices -2..700)\n", compared);
}
