// Structured native L1 for SoundBank_FindLoadedDescriptorByName (0x004a0ec0; ECX bank, one stack argument: the name;
// ret 4; ported in the cloud as a zclass_nodes blocker, P2.6). Null bank -> 0. The sound mode [0x0056b2ac] must be 0 or
// 1 (else 0; both modes run the same scan): the bank's descriptors ([bank+8], 0xB8 bytes each, count [bank+4]) are
// compared by name ([d+8] -> string, inlined case-sensitive strcmp) and the first match with a created buffer
// ([d+0x4C] != 0) is returned; a match without one is skipped. The arena fuzz would feed random counts (a scan of
// up to 255 records of 0xB8 bytes through the harness), so real banks here: 0..6 descriptors, names from a small pool
// with repeats and case variants, buffers present or not, modes 0 / 1 / 2, null bank now and then. Compared: the
// result as a descriptor index.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zSound/zsnd_parm.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

TEST(native_sound_bank_find_loaded_descriptor_by_name_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const std::uint32_t*, int, const char*);
    const Fn fn[2] = {rt::original<Fn>(0x004a0ec0), reinterpret_cast<Fn>(&recoil::SoundBank_FindLoadedDescriptorByName)};
    const char* names[] = {"engine", "Engine", "gun", "gun2", "", "explode"};
    std::mt19937 rng(0x4a0ec0);
    int compared = 0, found = 0;
    for (int it = 0; it < 6000; ++it) {
        const int n = static_cast<int>(rng() % 7);
        std::vector<std::uint32_t> desc(46 * (n ? n : 1));
        for (auto& w : desc) w = rng();
        for (int k = 0; k < n; ++k) {
            desc[46 * k + 2] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(names[rng() % 6]));
            desc[46 * k + 0x4C / 4] = rng() % 3 ? rng() | 1u : 0u;
        }
        const std::uint32_t bank[3] = {rng(), static_cast<std::uint32_t>(n), static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(desc.data()))};
        const char* q = rng() % 5 == 0 ? "missing" : names[rng() % 6];
        const std::uint32_t mode = rng() % 5 == 0 ? 2u : rng() % 2;
        const bool null_bank = rng() % 30 == 0;
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(0x0056b2ac) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056b2ac))) = mode;
            const std::uint32_t r = fn[side](null_bank ? nullptr : bank, 0, q);
            const std::uint32_t base = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(desc.data()));
            ret[side] = r == 0 ? 0xFFFFFFFFu : (r - base) % 0xB8 ? 0xBADu : (r - base) / 0xB8;
        }
        CHECK_EQ(ret[0], ret[1]);
        found += ret[0] != 0xFFFFFFFFu;
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  SoundBank_FindLoadedDescriptorByName calls %d, %d found\n", compared, found);
    CHECK(found > 1000);
}
