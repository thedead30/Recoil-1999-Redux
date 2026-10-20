// Structured native L1 for WildcardName_NextDigitCombination (0x004a6070, no arguments). It used to be arena fuzzed
// (test_fuzz_zutl_zar), which set the digit count [0x0056bba8] and the digits [0x0056bb90..] to random words. The
// function sprintf's each digit ("%d", 0x004dacbc) into a 2-byte stack slot at esp+0xa, so a digit above 9 (or a count
// above 5, reading past the 5 digits) wrote over the saved registers and the return address. On the port side the
// return then landed in test code: the shard crashed under load with 0xc0000409 and printed CHECK failures from
// unrelated files (KG-31). In the game WildcardName_InitDigitExpansion (0x004a5f90) caps the count at 5 and zeroes the
// digits, and this function keeps them 0..9, so that state never occurs.
// Here, per side: InitDigitExpansion on a real name (0..7 '*' at random places, so the cap of 5 is reached), then up to
// 120 NextDigitCombination calls (odometer: a digit below 9 goes up, a 9 wraps to 0 and carries; overflow of the last
// digit returns 0, else the digits are written into the '*' slots and the name is returned). Compared: every return
// (as name / null), the name text and the digit words after each call.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zUtil/zutl_zar.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
}  // namespace

TEST(native_wildcard_name_next_digit_combination_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Init = std::uint32_t(__fastcall*)(char*, int);
    using Next = std::uint32_t(__fastcall*)(int, int);
    const Init init[2] = {rt::original<Init>(0x004a5f90), reinterpret_cast<Init>(&recoil::WildcardName_InitDigitExpansion)};
    const Next next[2] = {rt::original<Next>(0x004a6070), reinterpret_cast<Next>(&recoil::WildcardName_NextDigitCombination)};
    std::mt19937 rng(0x4a6070);
    int compared = 0, calls = 0, overflowed = 0;
    for (int it = 0; it < 600; ++it) {
        std::string name = "mesh";
        const int stars = static_cast<int>(rng() % 8);
        for (int s = 0; s < stars; ++s) name.insert(rng() % (name.size() + 1), 1, '*');
        const int n_calls = 1 + static_cast<int>(rng() % 120);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<char> buf(name.begin(), name.end());
            buf.push_back('\0');
            snap[side].push_back(init[side](buf.data(), 0) == addr(buf.data()) ? 1u : 0u);
            for (int c = 0; c < n_calls; ++c) {
                const std::uint32_t r = next[side](0, 0);
                snap[side].push_back(r == 0 ? 0u : r == addr(buf.data()) ? 1u : 0xBADu);
                for (char ch : buf) snap[side].push_back(static_cast<unsigned char>(ch));
                for (int d = 0; d < 5; ++d) snap[side].push_back(*img(side, 0x0056bb90u + 4 * d));
                snap[side].push_back(*img(side, 0x0056bba8));
                if (side == 0) { ++calls; overflowed += r == 0; }
                if (r == 0) break;
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    CHECK(overflowed > 20);
    std::printf("  WildcardName_NextDigitCombination: %d names, %d calls, %d overflowed (0..7 '*', count capped at 5)\n", compared, calls, overflowed);
}
