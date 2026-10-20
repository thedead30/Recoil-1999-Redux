// Structured native L1 for the rasteriser buffer frees (P4 rasteriser, ported in the cloud):
//  - Raster_FreeNoiseAndBackBuffer (0x0048d3e0): frees [0x0056b1bc] and [0x0056b1c0] when set, clearing both;
//  - SwRender_FreeBuffers (0x00490780): frees [0x0057dae4] and [0x0057dae8] when set, clearing each; returns 0;
//  - zRndrShutdown (0x0048ff60; render_frame): Raster_FreeNoiseAndBackBuffer, returns 0.
// Each global holds a real CRT block or null; free is logged in both import slots. Compared: freed blocks by role and
// the four globals afterwards.
#include "test.h"
#include "cloud_harness.h"
#include "unattributed/rasteriser.h"
#include "unattributed/render_frame.h"

#include <cstdint>
#include <random>
#include <vector>

TEST(native_rasteriser_free_buffers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F fn[3][2] = {
        {rt::original<F>(0x0048d3e0), reinterpret_cast<F>(&recoil::Raster_FreeNoiseAndBackBuffer)},
        {rt::original<F>(0x00490780), reinterpret_cast<F>(&recoil::SwRender_FreeBuffers)},
        {rt::original<F>(0x0048ff60), reinterpret_cast<F>(&recoil::zRndrShutdown)},
    };
    static const std::uint32_t globals[3][2] = {{0x0056b1bc, 0x0056b1c0}, {0x0057dae4, 0x0057dae8}, {0x0056b1bc, 0x0056b1c0}};
    std::mt19937 rng(0x48d3e0);
    for (int it = 0; it < 300; ++it) {
        const int which = static_cast<int>(rng() % 3);
        const bool set[2] = {rng() % 3 != 0, rng() % 3 != 0};
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            std::uint32_t blocks[2] = {0, 0};
            for (int g = 0; g < 2; ++g) {
                if (set[g]) { blocks[g] = ch::addr(ch::c_malloc(0x40)); rl.set(blocks[g], 1 + g); }
                *ch::img(side, globals[which][g]) = blocks[g];
            }
            ch::freed().clear();
            int ret;
            {
                ch::FreeHook hook;
                ret = fn[which][side](0, 0);
            }
            snap[side].push_back(which ? static_cast<std::uint32_t>(ret) : 0u);  // the first returns what EAX held
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            for (int g = 0; g < 2; ++g) snap[side].push_back(rl(*ch::img(side, globals[which][g])));
            for (int g = 0; g < 2; ++g) if (blocks[g] && !ch::was_freed(blocks[g])) ch::c_free(blocks[g]);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}
