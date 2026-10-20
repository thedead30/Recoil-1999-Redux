// Verifies 0x004c6350 Crt_SetFpuControl: after it runs, the x87 control word must equal the value the
// original shows during gameplay, 0x027F (VERIFIED-ORACLE, trace Recoil22, zvideo.md section 1):
// 53-bit precision, round to nearest, all exceptions masked.
#include "test.h"
#include "unattributed/app.h"

#include <cstdint>

static std::uint16_t read_x87_control_word()
{
    std::uint16_t cw = 0;
    __asm fnstcw cw
    return cw;
}

TEST(app_Crt_SetFpuControl_sets_0x027F)
{
    // Start from 64-bit precision so the test proves the function changes it.
    std::uint16_t cw64 = 0x037F;
    __asm fldcw cw64
    CHECK_EQ(read_x87_control_word(), 0x037F);

    recoil::Crt_SetFpuControl();
    CHECK_EQ(read_x87_control_word(), 0x027F);
}

TEST(platform_is_32bit)
{
    CHECK_EQ(sizeof(void*), 4u);  // decision D1
}
