// Structured native L1 for Vehicle_BuildActiveContactList (0x0042cf60; P3 vehicle, ported in the cloud): for the
// [0x004f3bc8] contact slots, writes the index of every slot whose flag word (0x004f3bd0 + 4i) is non-zero into the
// list at 0x004f3bf8, in order. The arena fuzz cannot drive it (a random count walks off the ten slots), so the count
// stays in 0..10 here and the flags are seeded the same on both sides. Compared: the flags, the list and the words
// after it.
#include "test.h"
#include "cloud_harness.h"
#include "unattributed/vehicle.h"

#include <cstdint>
#include <random>
#include <vector>

TEST(native_vehicle_build_active_contact_list_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F fn[2] = {rt::original<F>(0x0042cf60), reinterpret_cast<F>(&recoil::Vehicle_BuildActiveContactList)};
    std::mt19937 rng(0x42cf60);
    for (int it = 0; it < 2000; ++it) {
        const std::uint32_t count = it < 20 ? (it % 2 ? 0xffffffffu : 0) : rng() % 11;
        std::uint32_t flags[10], list[12];
        for (auto& w : flags) w = rng() % 2 ? 0 : (rng() % 2 ? 1 : rng());
        for (auto& w : list) w = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x004f3bc8) = count;
            for (int k = 0; k < 10; ++k) *ch::img(side, 0x004f3bd0 + 4 * k) = flags[k];
            for (int k = 0; k < 12; ++k) *ch::img(side, 0x004f3bf8 + 4 * k) = list[k];
            fn[side](static_cast<int>(rng()), static_cast<int>(rng()));  // returns the loop counter
            for (std::uint32_t k = 0; k < 0x40 / 4; ++k) snap[side].push_back(*ch::img(side, 0x004f3bc8 + 4 * k));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// SurfaceMask_AndThree (0x0042cbd0): ANDs the words of the .rdata table 0x004d0870 at three contact indices (ECX, EDX,
// stack; ret 4). Indices -1..9 (the -1 an unfilled pick of Vehicle_SelectTopContacts, reading the word before the
// table). Not in the arena fuzz: a random index reads outside the table.
TEST(native_vehicle_surface_mask_and_three_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int, int);
    const F fn[2] = {rt::original<F>(0x0042cbd0), reinterpret_cast<F>(&recoil::SurfaceMask_AndThree)};
    for (int a = -1; a < 10; ++a)
        for (int b = -1; b < 10; ++b)
            for (int c = -1; c < 10; ++c) CHECK_EQ(fn[0](a, b, c), fn[1](a, b, c));
}

// Vehicle_SelectTopContacts (0x0042cc00): ECX = a direction, EDX = a block whose floats +0x24 + 4i replace the Y of
// contact i (positions 0x004f3c20 + 12i); keeps the four active contacts ([0x004f3bc8] slots, flags 0x004f3bd0) with
// the largest dot product with the direction, clearing the flags of the others; drops the third or the fourth pick by
// the surface mask of picks 1..3 (SurfaceMask_AndThree), then rebuilds the active list (0x004f3bf8). Real floats,
// counts 0..10, ties included. Compared: the return and the whole contact block 0x004f3bc8..0x004f3c98.
TEST(native_vehicle_select_top_contacts_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(const float*, const float*);
    const F fn[2] = {rt::original<F>(0x0042cc00), reinterpret_cast<F>(&recoil::Vehicle_SelectTopContacts)};
    std::mt19937 rng(0x42cc00);
    std::uniform_real_distribution<float> u(-10.0f, 10.0f);
    for (int it = 0; it < 5000; ++it) {
        const std::uint32_t count = rng() % 11;
        float dir[3] = {u(rng), u(rng), u(rng)}, block[20], pos[30];
        std::uint32_t flags[10];
        for (float& f : block) f = rng() % 5 == 0 ? 1.0f : u(rng);  // repeated values give ties
        for (float& f : pos) f = rng() % 5 == 0 ? 0.0f : u(rng);
        for (auto& w : flags) w = rng() % 4 ? 1 : 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x004f3bc8) = count;
            for (int k = 0; k < 10; ++k) *ch::img(side, 0x004f3bd0 + 4 * k) = flags[k];
            for (int k = 0; k < 30; ++k) *ch::img(side, 0x004f3c20 + 4 * k) = ch::fbits(pos[k]);
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](dir, block)));
            for (std::uint32_t k = 0; k < (0x004f3c98 - 0x004f3bc8) / 4; ++k) snap[side].push_back(*ch::img(side, 0x004f3bc8 + 4 * k));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}
