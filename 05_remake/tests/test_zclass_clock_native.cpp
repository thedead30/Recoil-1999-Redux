// Structured native L1 for the zclass_nodes clocks (P2.6), on real float state. The arena fuzz feeds mostly small
// integers read as floats (denormals), so the time comparisons barely vary there (PointToCellClampedSimple's mutant
// survived it); here times, lengths and steps come from a small set of real values with repeats, zero and negatives.
//   Anim_AdvanceClock (0x00453c90, ECX channel, one float stack argument dt; returns a 16-bit status in AX):
//     state short +0x64 == 2 -> 2. t = +0x58 + dt, stored; t > end +0x50: loop short +0x6a == -1 -> +0x58 = 0,
//     state 2, return 1; else +0x58 = t - end + loop start +0x5c, 1. t <= end: +0x6a != -1 and t > +0x60 ->
//     +0x58 = t - +0x60 + +0x5c; return 1.
//     Compared: the return (AX only - the upper half of EAX is whatever the caller left on the state-2 path) and every
//     word of the channel.
//   Seq_AdvanceKeyframes (0x004541c0, ECX node, d = class data +0x38): null node / data -> report, 5. Paused d[3] == 0
//     and active d[0]: time d[6] += frame time [0x0056b424]; while time > the duration of entry d[5] (entries at +0x20,
//     8 bytes: node, float duration): time -= it, cursor += step d[4]; past the end: loop d[2] -> 0, else last entry and
//     the step negated; below 0: loop -> last, else 0 and the step negated; either end with d[1] == 0 deactivates.
//     Tables of 1..5 entries with positive durations (so the loop ends), steps -1 / 0 / 1 / 2, cursor in range.
//     Compared: the return and every class-data word.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Animate.h"
#include "GameZRecoil/zClass/Seq.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t fbits(float f) { std::uint32_t u; std::memcpy(&u, &f, 4); return u; }
const float kF[] = {0.0f, 0.1f, 0.25f, 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 10.0f, -0.5f, 0.033333f, 100.0f};
float rf(std::mt19937& rng) { return kF[rng() % 12]; }
}  // namespace

TEST(native_zclass_anim_advance_clock_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int, float);
    const Fn fn[2] = {rt::original<Fn>(0x00453c90), reinterpret_cast<Fn>(&recoil::Anim_AdvanceClock)};
    std::mt19937 rng(0x453c90);
    int compared = 0, per_path[4] = {};
    for (int it = 0; it < 20000; ++it) {
        std::uint32_t init[0x70 / 4];
        for (auto& w : init) w = rng();
        init[0x50 / 4] = fbits(rf(rng));
        init[0x58 / 4] = fbits(rf(rng));
        init[0x5c / 4] = fbits(rf(rng));
        init[0x60 / 4] = fbits(rf(rng));
        const std::uint16_t state = rng() % 5 == 0 ? 2 : static_cast<std::uint16_t>(rng() % 4);
        const std::uint16_t loop = rng() % 2 ? 0xFFFFu : static_cast<std::uint16_t>(rng() % 8);
        std::memcpy(reinterpret_cast<char*>(init) + 0x64, &state, 2);
        std::memcpy(reinterpret_cast<char*>(init) + 0x6a, &loop, 2);
        const float dt = rf(rng);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t ch[0x70 / 4];
            std::memcpy(ch, init, sizeof ch);
            const std::uint32_t r = fn[side](ch, 0, dt);
            snap[side].push_back(r & 0xFFFFu);
            snap[side].insert(snap[side].end(), ch, ch + 0x70 / 4);
            if (side == 0) {
                const bool wrapped = ch[0x58 / 4] != init[0x58 / 4];
                ++per_path[state == 2 ? 0 : (r & 0xFFFFu) == 1 && !wrapped ? 1 : wrapped ? 2 : 3];
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  Anim_AdvanceClock calls %d: state 2 %d, returned 1 with the time unchanged %d, time changed %d, other %d\n", compared, per_path[0],
                per_path[1], per_path[2], per_path[3]);
    CHECK(per_path[0] > 1000 && per_path[2] > 1000);
}

TEST(native_zclass_seq_advance_keyframes_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x004541c0), reinterpret_cast<Fn>(&recoil::Seq_AdvanceKeyframes)};
    const float durations[] = {0.1f, 0.25f, 0.5f, 1.0f, 2.0f};
    const float dts[] = {0.0f, 0.016f, 0.1f, 0.5f, 1.0f, 3.0f, 10.0f, -0.5f};
    const std::int32_t steps[] = {-1, 0, 1, 2};
    std::mt19937 rng(0x4541c0);
    int compared = 0, moved = 0, stopped = 0;
    for (int it = 0; it < 20000; ++it) {
        const int count = 1 + static_cast<int>(rng() % 5);
        std::uint32_t init[0x20 / 4 + 2 * 5];
        for (auto& w : init) w = rng();
        init[0] = rng() % 4 ? 1u : 0u;
        init[1] = rng() % 2;
        init[2] = rng() % 2;
        init[3] = rng() % 5 ? 0u : 1u;
        init[4] = static_cast<std::uint32_t>(steps[rng() % 4]);
        init[5] = rng() % count;
        init[6] = fbits(durations[rng() % 5] * (rng() % 3) * 0.5f);
        init[7] = static_cast<std::uint32_t>(count);
        for (int e = 0; e < count; ++e) init[0x24 / 4 + 2 * e] = fbits(durations[rng() % 5]);
        const float dt = dts[rng() % 8];
        const unsigned null_kind = rng() % 20 == 0 ? 1 + rng() % 2 : 0;  // 1 node, 2 class data
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t node[49] = {}, data[0x20 / 4 + 2 * 5];
            std::memcpy(data, init, sizeof data);
            node[0x38 / 4] = null_kind == 2 ? 0u : static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(data));
            *static_cast<float*>(side ? recoil::ImageData_Address(0x0056b424) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056b424))) = dt;
            const int r = fn[side](null_kind == 1 ? nullptr : node, 0);
            snap[side].push_back(static_cast<std::uint32_t>(r));
            snap[side].insert(snap[side].end(), data, data + 0x20 / 4 + 2 * count);
            if (side == 0) { moved += data[5] != init[5]; stopped += data[0] != init[0]; }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Seq_AdvanceKeyframes calls %d, %d moved the cursor, %d deactivated\n", compared, moved, stopped);
    CHECK(moved > 2000 && stopped > 200);
}
