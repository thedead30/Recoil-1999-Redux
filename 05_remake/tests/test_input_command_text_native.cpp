// Structured native L1 for the two command-slot text lookups (P input, ported in the cloud):
// CommandSlot_Call_004a5bf0 (0x0042a4e0) and CommandSlot_Call_004a5bf0_Plus1 (0x0042a4f0) take the message id at
// [0x004f3af0 + 4*ECX] (plus one) and tail-jump into Message_GetText (0x004a5bf0: GetMessageByID into the shared
// buffer 0x0056b570). The arena fuzz cannot drive them: ECX indexes a static table without a bound. Here ECX stays
// inside the table (0..63), the slot gets an id drawn from the seed, FormatMessageA / LocalFree are bound into both
// import tables and the messages come from ntdll ([0x0056b670]), as tests/test_menus_virtual_native.cpp does for
// Message_GetText. Compared: the return as a VA and the buffer.
#include "test.h"
#include "vt_runner.h"
#include "platform/iat_kernel32.h"
#include "unattributed/input.h"

#include <string>

using vtr::call_any;
using U = std::uint32_t;

TEST(native_input_command_slot_text_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const U va[2] = {0x004cc0f4, 0x004cc0fc};
    void** port[2] = {&recoil::g_Iat_FormatMessageA_004cc0f4, &recoil::g_Iat_LocalFree_004cc0fc};
    void* saved[2];
    for (int i = 0; i < 2; ++i) { void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])); saved[i] = *o; *o = *port[i]; }
    const U fns[2][2] = {{0x0042a4e0, ch::addr(reinterpret_cast<void*>(&recoil::CommandSlot_Call_004a5bf0))},
                         {0x0042a4f0, ch::addr(reinterpret_cast<void*>(&recoil::CommandSlot_Call_004a5bf0_Plus1))}};
    std::mt19937 rng(0x42a4e0);
    for (int it = 0; it < 400; ++it) {
        const int f = static_cast<int>(rng() % 2);
        const U slot = rng() % 64;
        const U id = rng() % 3 ? rng() % 400 : 0xC0000000u | (rng() % 0x200);
        U ret[2];
        std::string out[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x0056b670) = ch::addr(GetModuleHandleA("ntdll.dll"));
            *ch::img(side, 0x004f3af0 + 4 * slot) = id;
            const U r = call_any(fns[f][side], slot, 0, nullptr, 0);
            ret[side] = side && r ? recoil::ImageData_VaOf(reinterpret_cast<void*>(static_cast<std::uintptr_t>(r))) : r;
            out[side] = std::string(reinterpret_cast<const char*>(ch::img(side, 0x0056b570)), 0x100);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(out[0] == out[1]);
    }
    for (int i = 0; i < 2; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])) = saved[i];
    rt::restore_pristine();
}
