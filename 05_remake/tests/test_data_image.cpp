// The port's data image (src/platform/image, decision D7) against the ORIGINAL image mapped by the oracle:
//  - every mirror byte equals the original's, except import slots, relocated words and bytes inside named blocks;
//  - every relocated word holds what its kind says: the mirror/named-block address of its target, the ported
//    function, or the unported-code trap.
#include "test.h"
#include "native_oracle.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstring>
#include <set>

TEST(data_image_matches_original_and_relocations_resolve)
{
    if (!rt::map_original()) { CHECK(false); return; }
    std::set<std::uint32_t> reloc_words;
    int bad_reloc = 0;
    for (std::uint32_t i = 0; i < recoil::g_ImageDataRelocationCount; ++i) {
        const auto& r = recoil::g_ImageDataRelocations[i];
        for (int k = 0; k < 4; ++k) reloc_words.insert(r.va + k);
        const std::uint32_t got = *static_cast<const std::uint32_t*>(recoil::ImageData_Address(r.va));
        std::uint32_t want = 0;
        if (r.kind == 0 || r.kind == 1) want = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(recoil::ImageData_Address(r.target)));
        else if (r.kind == 3) want = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&recoil::ImageData_Unported));
        if (r.kind != 2 && got != want) ++bad_reloc;
        if (r.kind == 2 && (got == 0 || got == static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&recoil::ImageData_Unported)))) ++bad_reloc;
        // the original word itself must be the target address
        if (*reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(r.va)) != r.target) ++bad_reloc;
    }
    // byte comparison over both sections, skipping import slots (zero in the mirror), relocations and named blocks
    int bad_bytes = 0, compared = 0;
    const struct { std::uint32_t va, size; const unsigned char* mirror; } secs[] = {
        {recoil::kRDataVa, recoil::kRDataSize, recoil::g_RData_004cc000}, {recoil::kDataVa, recoil::kDataSize, recoil::g_Data_004da000}};
    for (const auto& s : secs)
        for (std::uint32_t o = 0; o < s.size; ++o) {
            const std::uint32_t va = s.va + o;
            if (reloc_words.count(va)) continue;
            if (recoil::ImageData_Address(va) != s.mirror + o) continue;          // inside a named block
            if (va >= 0x004cc000 && va < 0x004cc000 + 0x1000 && s.mirror[o] == 0) continue;  // import slots zeroed
            ++compared;
            if (s.mirror[o] != *reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(va))) ++bad_bytes;
        }
    std::printf("  %u relocations, %d bytes compared\n", recoil::g_ImageDataRelocationCount, compared);
    CHECK_EQ(bad_reloc, 0);
    CHECK_EQ(bad_bytes, 0);
    CHECK(compared > 2700000);
}
