// P1.1: the CRT equivalents (system msvcrt.dll, decision D5).
#include "test.h"
#include "platform/msvcrt.h"

#include <cstdint>

// rand must follow the LCG disassembled from the traced msvcrt (04_spec/formulas/crt_rand.md).
TEST(crt_rand_matches_traced_lcg)
{
    for (unsigned seed : {1u, 12345u, 0xdeadbeefu, 1727226000u /* a time() value */}) {
        recoil::crt::srand(seed);
        std::uint32_t x = seed;
        for (int i = 0; i < 1000; ++i) {
            x = x * 214013u + 2531011u;
            CHECK_EQ(recoil::crt::rand(), static_cast<int>((x >> 16) & 0x7FFF));
        }
    }
}

// _ftol truncates toward zero; the low dword is what Recoil uses (e.g. expApprox's ftol).
TEST(crt_ftol_truncates)
{
    CHECK_EQ(recoil::crt::ftol(2.9), 2);
    CHECK_EQ(recoil::crt::ftol(-2.9), -2);
    CHECK_EQ(recoil::crt::ftol(0.99999), 0);
    CHECK_EQ(recoil::crt::ftol(3000000000.0), 3000000000ll);           // beyond int32: 64-bit result
    CHECK_EQ(static_cast<std::uint32_t>(recoil::crt::ftol(-1.5)), 0xffffffffu);
}

TEST(crt_atof_atoi)
{
    CHECK(recoil::crt::atof("1.5") == 1.5);
    CHECK(recoil::crt::atof("-0.25x") == -0.25);
    CHECK(recoil::crt::atof("") == 0.0);
    CHECK_EQ(recoil::crt::atoi("42abc"), 42);
    CHECK_EQ(recoil::crt::atoi("  -7"), -7);
}
