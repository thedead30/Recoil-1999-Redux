// The port passes the Windows SDK's DirectInput data formats (platform/dinput_formats.h) where the original passes the copies
// dinput.lib linked into its .text (03_re/ledger/text_data_symbols.csv). This checks they are the same tables: the DIDATAFORMAT
// header words and, for every object, the offset, type and flags words and the GUID it points to (or both null). Addresses inside
// the tables differ by construction (each side's own copy), so pointers are compared by what they point at.
#include "test.h"
#include "native_oracle.h"
#include "platform/dinput_formats.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
bool same_format(const char* name, std::uint32_t orig_va, const unsigned int* sdk)
{
    const auto* o = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(orig_va));
    for (int k = 0; k < 5; ++k)
        if (o[k] != sdk[k]) { std::printf("  %s header word %d: original %08x sdk %08x\n", name, k, o[k], sdk[k]); return false; }
    const auto* oo = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(o[5]));
    const auto* so = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(sdk[5]));
    for (std::uint32_t i = 0; i < o[4]; ++i) {
        const std::uint32_t* a = oo + 4 * i;  // DIOBJECTDATAFORMAT: pguid, dwOfs, dwType, dwFlags
        const std::uint32_t* b = so + 4 * i;
        if (a[1] != b[1] || a[2] != b[2] || a[3] != b[3]) {
            std::printf("  %s object %u: original %08x %08x %08x sdk %08x %08x %08x\n", name, i, a[1], a[2], a[3], b[1], b[2], b[3]);
            return false;
        }
        if (!a[0] != !b[0] || (a[0] && std::memcmp(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(a[0])),
                                                    reinterpret_cast<const void*>(static_cast<std::uintptr_t>(b[0])), 16))) {
            std::printf("  %s object %u: GUIDs differ\n", name, i);
            return false;
        }
    }
    std::printf("  %s: header and %u objects equal\n", name, o[4]);
    return true;
}
}  // namespace

TEST(native_dinput_formats_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK(same_format("c_dfDIKeyboard", 0x004c73f0u, c_dfDIKeyboard));
    CHECK(same_format("c_dfDIMouse", 0x004c7480u, c_dfDIMouse));
    CHECK(same_format("c_dfDIJoystick2", 0x004c7ee0u, c_dfDIJoystick2));
}
