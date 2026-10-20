// Structured native L1s for the input functions that index fixed tables in .data by a key / button number (P3 input,
// ported in the cloud). The arena fuzz cannot drive these: a random index lands outside the table. Here the index stays
// in range and the table words it reaches are seeded the same on both sides.
//  - keyboard table 0x00561cd0: 2014 entries of 8 bytes (+4 state, +8 handler; the clear loop runs 0x00561cd8 to
//    0x00565bc8): Input_Keyboard_ConsumeKeyEdge (0x0046f980), Input_Keyboard_SetKeyHandler (0x0046f9b0, ret 4),
//    Input_ClearBindingSlot (0x0046f9d0), Input_Keyboard_ClearHeldTable (0x0046f9f0);
//  - mouse button bytes 0x00565e87 / 0x00565e97: Input_GetMouseButtonEdgeState (0x004702e0);
//  - joystick button bytes 0x00565c03 / 0x00565d13: Input_GetJoystickButtonEdgeState (0x004723a0);
//  - InputState_Ctor (0x00471ab0) on a 0x4210-byte block.
// Compared: the return and every table word the function can reach.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zInput/zin_init.h"
#include "GameZRecoil/zInput/zin_kbd.h"
#include "unattributed/input.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

namespace {
constexpr std::uint32_t kKeys = 0x00561cd0, kKeyCount = (0x00565bc8 - 0x00561cd8) / 8;

std::uint32_t pick_state(std::mt19937& rng)
{
    static const std::uint32_t v[] = {0, 1, 2, 3, 4, 5, 6, 0x80000004u};
    return rng() % 4 == 0 ? rng() : v[rng() % 8];
}

// seeds n words from va on both sides (the same values)
void seed_both(std::mt19937& rng, std::uint32_t va, std::uint32_t n, bool states)
{
    for (std::uint32_t k = 0; k < n; ++k) {
        const std::uint32_t w = states ? pick_state(rng) : (rng() % 2 ? 0 : rng());
        *ch::img(0, va + 4 * k) = w;
        *ch::img(1, va + 4 * k) = w;
    }
}

void snap_words(std::vector<std::uint32_t>& s, int side, std::uint32_t va, std::uint32_t n)
{
    for (std::uint32_t k = 0; k < n; ++k) s.push_back(*ch::img(side, va + 4 * k));
}
}  // namespace

TEST(native_input_keyboard_table_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F0 = int(__fastcall*)(int, int);
    using F1 = int(__fastcall*)(int, int, int);
    const F0 consume[2] = {rt::original<F0>(0x0046f980), reinterpret_cast<F0>(&recoil::Input_Keyboard_ConsumeKeyEdge)};
    const F1 set_handler[2] = {rt::original<F1>(0x0046f9b0), reinterpret_cast<F1>(&recoil::Input_Keyboard_SetKeyHandler)};
    const F0 clear_slot[2] = {rt::original<F0>(0x0046f9d0), reinterpret_cast<F0>(&recoil::Input_ClearBindingSlot)};
    const F0 clear_all[2] = {rt::original<F0>(0x0046f9f0), reinterpret_cast<F0>(&recoil::Input_Keyboard_ClearHeldTable)};
    std::mt19937 rng(0x46f980);
    int compared = 0;
    for (int it = 0; it < 20000; ++it) {
        rt::restore_pristine();
        const int op = static_cast<int>(rng() % 4);
        const int key = static_cast<int>(rng() % kKeyCount);
        const int edx = static_cast<int>(rng() % 2 ? 0 : rng()), arg = static_cast<int>(rng());
        // the entry itself, its neighbours (to catch a stride error) and, for the clear-all, a sample of the table
        const std::uint32_t lo = key > 0 ? key - 1 : 0, hi = key + 2 < static_cast<int>(kKeyCount) ? key + 2 : kKeyCount;
        seed_both(rng, kKeys + 8 * lo, 2 * (hi - lo), true);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            int ret = 0;
            if (op == 0) ret = consume[side](key, edx);
            else if (op == 1) ret = set_handler[side](key, edx, arg);
            else if (op == 2) ret = clear_slot[side](key, edx);
            else { clear_all[side](key, edx); ret = 0; }  // returns whatever EAX held
            snap[side].push_back(op == 3 ? 0u : static_cast<std::uint32_t>(ret));
            snap_words(snap[side], side, kKeys + 8 * lo, 2 * (hi - lo));
            if (op == 3) snap_words(snap[side], side, kKeys, 2 * kKeyCount + 2);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  keyboard table calls %d\n", compared);
    CHECK(compared == 20000);
}

TEST(native_input_button_edge_state_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F mouse[2] = {rt::original<F>(0x004702e0), reinterpret_cast<F>(&recoil::Input_GetMouseButtonEdgeState)};
    const F joy[2] = {rt::original<F>(0x004723a0), reinterpret_cast<F>(&recoil::Input_GetJoystickButtonEdgeState)};
    std::mt19937 rng(0x4702e0);
    int compared = 0;
    for (int it = 0; it < 4000; ++it) {
        rt::restore_pristine();
        const bool is_joy = rng() % 2 != 0;
        const int button = static_cast<int>(rng() % 16);
        const std::uint32_t now = is_joy ? 0x00565c03 : 0x00565e87, then = is_joy ? 0x00565d13 : 0x00565e97;
        const unsigned char a = static_cast<unsigned char>(rng() % 3 == 0 ? rng() : rng() % 2);
        const unsigned char b = static_cast<unsigned char>(rng() % 3 == 0 ? rng() : rng() % 2);
        for (int side = 0; side < 2; ++side) {
            reinterpret_cast<unsigned char*>(ch::img(side, now + button))[0] = a;
            reinterpret_cast<unsigned char*>(ch::img(side, then + button))[0] = b;
        }
        const int r0 = (is_joy ? joy : mouse)[0](button, static_cast<int>(rng()));
        const int r1 = (is_joy ? joy : mouse)[1](button, static_cast<int>(rng()));
        CHECK_EQ(r0, r1);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  button edge state calls %d\n", compared);
}

TEST(native_input_state_ctor_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int);
    const F fn[2] = {rt::original<F>(0x00471ab0), reinterpret_cast<F>(&recoil::InputState_Ctor)};
    std::mt19937 rng(0x471ab0);
    for (int it = 0; it < 50; ++it) {
        std::vector<std::uint32_t> init(0x4210 / 4);
        for (auto& w : init) w = rng();
        std::vector<std::uint32_t> blk[2] = {init, init};
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) ret[side] = static_cast<std::uint32_t>(fn[side](blk[side].data(), static_cast<int>(rng())));
        CHECK(blk[0] == blk[1]);
        CHECK_EQ(ret[0] - ch::addr(blk[0].data()), ret[1] - ch::addr(blk[1].data()));  // returns the block
    }
}
