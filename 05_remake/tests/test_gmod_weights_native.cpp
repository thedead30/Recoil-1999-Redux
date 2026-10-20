// Native L1 for Model_BuildVertexWeightTable (0x00483f80). It indexes heap tables with vertex indices read from
// the model, so random arena words (tests/arena_fuzz.h) turn into wild writes through the test process's heap;
// here each call gets a well-formed model instead: vertex count +0x10, parts (+0xC count, +0x30 array of 0x1C-byte
// entries: low byte of +0 = index count, +8 = index list, indices < vertex count), pinned index list (EDX,
// arg2 entries), value (arg1, float), minimum usage (arg3, -2..4). Each side gets its own identical copy; compared:
// return value, the model words, and the 0xC-byte table the call reallocates at +0x40 (by content).
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zModel/gmod_const.h"
#include "platform/iat_msvcrt.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

TEST(native_gmod_vertex_weight_table_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Build = int(__fastcall*)(void*, const std::uint32_t*, std::uint32_t, int, int);
    using Free = void(__cdecl*)(void*);
    auto orig = rt::original<Build>(0x00483f80);
    auto port = reinterpret_cast<Build>(&recoil::Model_BuildVertexWeightTable);
    const auto m_free = reinterpret_cast<Free>(recoil::g_Iat_free_004cc5b4);
    std::mt19937 rng(0x483f80);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        const int nverts = rng() % 40;
        const int nparts = rng() % 6;
        std::vector<std::vector<std::uint32_t>> lists(nparts);
        for (auto& l : lists) {
            l.resize(nverts ? rng() % 12 : 0);
            for (auto& v : l) v = rng() % nverts;
        }
        std::vector<std::uint32_t> pinned(nverts ? rng() % 6 : 0);
        for (auto& v : pinned) v = rng() % nverts;
        const float value = static_cast<float>(static_cast<int>(rng() % 2000) - 1000) / 7.0f;
        std::uint32_t vbits;
        std::memcpy(&vbits, &value, 4);
        const int min_use = static_cast<int>(rng() % 7) - 2;
        std::uint32_t model[2][32];
        std::vector<std::uint32_t> parts[2];
        std::vector<std::uint32_t> table[2];
        int ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::mt19937 fill(it);
            for (auto& w : model[side]) w = fill() & 0xFFFF;
            parts[side].assign(nparts * 7, 0);
            for (int p = 0; p < nparts; ++p) {
                parts[side][p * 7] = (fill() & 0xFFFFFF00u) | static_cast<std::uint32_t>(lists[p].size());
                parts[side][p * 7 + 2] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(lists[p].data()));
            }
            model[side][0x0c / 4] = nparts;
            model[side][0x10 / 4] = nverts;
            model[side][0x30 / 4] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(parts[side].data()));
            model[side][0x40 / 4] = 0;
            ret[side] = (side ? port : orig)(model[side], pinned.data(), vbits, static_cast<int>(pinned.size()), min_use);
            const auto* t = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(model[side][0x40 / 4]));
            table[side].assign(t, t + 3 * nverts);
            m_free(const_cast<std::uint32_t*>(t));
            model[side][0x40 / 4] = 0;
            model[side][0x30 / 4] = 0;  // per-side buffer address
        }
        CHECK_EQ(ret[0], ret[1]);
        for (int i = 0; i < 32; ++i) CHECK_EQ(model[0][i], model[1][i]);
        CHECK(parts[0] == parts[1]);
        CHECK(table[0] == table[1]);
        ++compared;
    }
    std::printf("  Model_BuildVertexWeightTable calls %d (well-formed models)\n", compared);
}
