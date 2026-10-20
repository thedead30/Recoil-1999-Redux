// Structured native L1 for DEClient_ResolveMaterial (0x00455dd0; ECX out material pointer, EDX texture name; P2
// declient, cloud port): TextureManager_FindSlotByName(name) - found -> *out = Material_FindByTexture(slot) (the used
// material whose +0x10 is that slot, or 0), return 0; not found -> TextureManager_FindOrAddSlot(name) (cuts at the last
// '.', appends a slot at 0x0053d79c + 0x24 * count, count [0x0053d798] + 1, name via TextureSlot_SetNameFromPath, state
// 2), *out = 0, return 1. When *out is 0 a local material is initialised (Material_Init), marked textured (flag 0x100)
// with the slot at +0x10, and *out = Material_FindOrCreate(it) (the cached [0x00566a24] when equal, else the first equal
// used material, else Material_AllocCopy into the array).
// Each side: its own texture slot table (0..3 slots with short names, states 0..3) in the image, its own material array
// (4..6 records, used / free lists linked through the short pair at +0x28, some pointing at the slots, flag words
// random but non-cycling), cache set or not. Names from a pool with extensions, case variants and repeats. Compared: the
// return, *out as a material index, every slot word (slot pointers as indices), every material word, the material
// globals.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zDEClient/zdec_init.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

using namespace ch;

TEST(native_declient_resolve_material_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t*, char*);
    const Fn fn[2] = {rt::original<Fn>(0x00455dd0), reinterpret_cast<Fn>(&recoil::DEClient_ResolveMaterial)};
    const char* const names[] = {"rock", "rock.tga", "Rock.tga", "sand.bmp", "sand", "lava.x.tga", "grass"};
    std::mt19937 rng(0x455dd0);
    int compared = 0, added = 0, found = 0;
    for (int it = 0; it < 4000; ++it) {
        const int nslots = static_cast<int>(rng() % 4), nmat = 4 + static_cast<int>(rng() % 3), nused = static_cast<int>(rng() % (nmat + 1));
        int slot_name[4], slot_state[4], mat_slot[6];
        std::uint32_t mat_words[6][9];
        for (int k = 0; k < 4; ++k) { slot_name[k] = static_cast<int>(rng() % 7); slot_state[k] = static_cast<int>(rng() % 4); }
        for (int k = 0; k < 6; ++k) {
            mat_slot[k] = rng() % 3 ? static_cast<int>(rng() % 4) : -1;
            for (auto& w : mat_words[k]) w = rng() % 3;
            mat_words[k][0] = (mat_words[k][0] & ~0x0400u) | (rng() % 2 ? 0x100u : 0u);
        }
        const int cache = rng() % 3 == 0 && nused ? static_cast<int>(rng() % nused) : -1;
        const std::string name = names[rng() % 7];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t* slots = img(side, 0x0053d79c);
            *img(side, 0x0053d798) = static_cast<std::uint32_t>(nslots);
            std::memset(slots, 0, 0x24 * 5);
            for (int k = 0; k < nslots; ++k) {
                std::string n = names[slot_name[k]];
                const std::size_t dot = n.rfind('.');
                if (dot != std::string::npos) n.resize(dot);
                std::strncpy(reinterpret_cast<char*>(slots + 9 * k + 2), n.c_str(), 19);
                slots[9 * k + 7] = static_cast<std::uint32_t>(slot_state[k]);
            }
            std::vector<std::uint32_t> mats(11 * nmat, 0);
            for (int k = 0; k < nmat; ++k) {
                std::memcpy(&mats[11 * k], mat_words[k], sizeof mat_words[k]);
                mats[11 * k + 4] = mat_slot[k] >= 0 && mat_slot[k] < nslots ? addr(slots + 9 * mat_slot[k]) : 0u;
            }
            auto link = [&](int from, int to) {
                for (int k = from; k < to; ++k) {
                    auto* l = reinterpret_cast<std::int16_t*>(&mats[11 * k + 10]);
                    l[0] = static_cast<std::int16_t>(k > from ? k - 1 : -1);
                    l[1] = static_cast<std::int16_t>(k + 1 < to ? k + 1 : -1);
                }
            };
            link(0, nused);
            link(nused, nmat);
            *img(side, 0x00566a1c) = addr(mats.data());
            *img(side, 0x00566a18) = static_cast<std::uint32_t>(nmat);
            *img(side, 0x00566a20) = static_cast<std::uint32_t>(nused);
            *img(side, 0x004e1164) = nused ? 0u : 0xFFFFFFFFu;
            *img(side, 0x004e1160) = nused < nmat ? static_cast<std::uint32_t>(nused) : 0xFFFFFFFFu;
            *img(side, 0x00566a24) = cache >= 0 ? addr(&mats[11 * cache]) : 0u;
            char buf[32];
            std::strcpy(buf, name.c_str());
            std::uint32_t out = 0xEEEEEEEEu;
            const int r = fn[side](&out, buf);
            auto role = [&](std::uint32_t p) -> std::uint32_t {
                if (!p) return 0;
                if (p >= addr(mats.data()) && p < addr(mats.data()) + 44 * nmat) return 0xA000u + (p - addr(mats.data()));
                if (p >= addr(slots) && p < addr(slots) + 0x24 * 5) return 0xB000u + (p - addr(slots));
                const std::uint32_t va = recoil::ImageData_VaOf(at(p));
                return va ? va : p;
            };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            snap[side].push_back(role(out));
            snap[side].push_back(*img(side, 0x0053d798));
            for (int k = 0; k < 9 * 5; ++k) snap[side].push_back(slots[k]);
            for (int k = 0; k < 11 * nmat; ++k) snap[side].push_back(k % 11 == 4 ? role(mats[k]) : mats[k]);
            for (std::uint32_t va : {0x00566a18u, 0x00566a20u, 0x004e1164u, 0x004e1160u}) snap[side].push_back(*img(side, va));
            snap[side].push_back(role(*img(side, 0x00566a24)));
            snap[side].insert(snap[side].end(), buf, buf + 8);
            if (side == 0) { added += r == 1; found += r == 0 && out != 0; }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  DEClient_ResolveMaterial calls %d, %d slots added, %d existing materials found\n", compared, added, found);
    CHECK(added > 300 && found > 100);
}
