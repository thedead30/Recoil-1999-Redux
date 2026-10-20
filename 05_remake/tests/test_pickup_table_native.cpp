// Structured native L1s for the pickup functions on the static pickup table 0x004db6e8 (40 entries of 0x30 bytes: name
// pointer +0x10, value +8, key +0x58 - 0x30, image +0x20) and on node trees (P3 pickup, ported in the cloud):
//  - PickupTable_EntryByIndexSignedLt40 (0x0041db40: the entry for any index < 40, negative ones included; else 0),
//    PickupTable_EntryByIndex (0x0041e1c0: 0..39, else 0);
//  - PickupTable_IndexOfNameOut (0x0041dd60: ECX name, EDX out; entry +8 of the first entry whose name matches, 1; else
//    out 0 and 0), PickupTable_IndexOfName (0x0041e1e0: its index or -1), PickupTable_EntryForPrimaryWeapon (0x0041e980:
//    the name of [[[ECX+4]+0x5E4]] looked up among entries 17..; the entry, else entry 0), PickupTable_LookupByKey
//    (0x0041ea00: the image word of the entry 17.. whose key word (+0x58 of the entry before) equals ECX, else 0);
//  - PickupTable_FreeImages (0x0041cca0): Image_FreeUnlessDefault on every entry's image word, then 0;
//  - Node_ClearPickupFlagsRecursive (0x0041ceb0: +0x24 &= ~0x40018) / Node_SetPickupFlagsRecursive (0x0041cef0:
//    +0x24 = (+0x24 & ~8) | 0x40010) over the node and its descendants; both return 1.
// Queries are the table's own names (read from the mapped original), case variants, prefixes and misses; image words
// are null, the default image 0x004e06e0 or a real 0x40-byte image with no buffers or surface; logged free. Compared:
// returns and outputs as image VAs (the port side's image addresses mapped back), freed images by role, the table's
// image words, every node's +0x24.
#include "test.h"
#include "cloud_harness.h"
#include "Battlesport/pickup.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
// a word as the original side would hold it: the port side's image addresses mapped back to their VAs
std::uint32_t va_of(int side, std::uint32_t w)
{
    if (!side || !w) return w;
    const std::uint32_t va = recoil::ImageData_VaOf(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(w)));
    return va ? va : w;
}
}  // namespace

TEST(native_pickup_table_lookups_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = std::uint32_t(__fastcall*)(std::uint32_t, std::uint32_t);
    const F by_index_signed[2] = {rt::original<F>(0x0041db40), reinterpret_cast<F>(&recoil::PickupTable_EntryByIndexSignedLt40)};
    const F by_index[2] = {rt::original<F>(0x0041e1c0), reinterpret_cast<F>(&recoil::PickupTable_EntryByIndex)};
    const F name_out[2] = {rt::original<F>(0x0041dd60), reinterpret_cast<F>(&recoil::PickupTable_IndexOfNameOut)};
    const F name_index[2] = {rt::original<F>(0x0041e1e0), reinterpret_cast<F>(&recoil::PickupTable_IndexOfName)};
    const F primary[2] = {rt::original<F>(0x0041e980), reinterpret_cast<F>(&recoil::PickupTable_EntryForPrimaryWeapon)};
    const F by_key[2] = {rt::original<F>(0x0041ea00), reinterpret_cast<F>(&recoil::PickupTable_LookupByKey)};
    // PickupTable_EntryByName (0x0041e1a0): the entry of PickupTable_IndexOfName's index
    const F by_name[2] = {rt::original<F>(0x0041e1a0), reinterpret_cast<F>(&recoil::PickupTable_EntryByName)};
    // the table's names and keys as the original holds them
    std::vector<std::string> names;
    std::vector<std::uint32_t> keys;
    for (int k = 0; k < 40; ++k) {
        const auto p = *reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(0x004db6f8 + 0x30 * k));
        if (p) names.push_back(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p)));
        keys.push_back(*reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(0x004db6e8 + 0x58 + 0x30 * k)));
    }
    std::mt19937 rng(0x41dd60);
    auto query = [&]() {
        std::string s = names.empty() ? std::string("x") : names[rng() % names.size()];
        switch (rng() % 5) {
            case 0: if (!s.empty()) s[0] = static_cast<char>(s[0] ^ 0x20); break;  // case variant
            case 1: s = s.substr(0, s.size() / 2); break;                           // prefix
            case 2: s += "x"; break;                                                 // longer
            default: break;
        }
        return s;
    };
    int compared = 0;
    for (int it = 0; it < 6000; ++it) {
        const int op = static_cast<int>(rng() % 7);
        const std::string q = query();
        const std::int32_t index = static_cast<std::int32_t>(rng() % 50) - 5;
        const std::uint32_t key = rng() % 3 ? keys[rng() % 40] : rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t out = 0xabcdef01, r = 0;
            if (op == 0) r = va_of(side, by_index_signed[side](static_cast<std::uint32_t>(index), 0));
            else if (op == 1) r = va_of(side, by_index[side](static_cast<std::uint32_t>(index), 0));
            else if (op == 2) { r = name_out[side](ch::addr(q.c_str()), ch::addr(&out)); out = va_of(side, out); }
            else if (op == 3) r = name_index[side](ch::addr(q.c_str()), 0);
            else if (op == 4) {
                const char* qp = q.c_str();
                std::uint32_t inner[1] = {ch::addr(&qp)};
                std::uint32_t blk[0x5e8 / 4] = {};
                blk[0x5e4 / 4] = ch::addr(inner);
                std::uint32_t obj[2] = {0, ch::addr(blk)};
                r = va_of(side, primary[side](ch::addr(obj), 0));
            } else if (op == 5) r = va_of(side, by_key[side](key, 0));
            else r = va_of(side, by_name[side](ch::addr(q.c_str()), 0));
            snap[side].push_back(r);
            snap[side].push_back(out);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  pickup table lookups %d (%zu names in the table)\n", compared, names.size());
    CHECK(!names.empty());
}

TEST(native_pickup_table_free_images_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F fn[2] = {rt::original<F>(0x0041cca0), reinterpret_cast<F>(&recoil::PickupTable_FreeImages)};
    std::mt19937 rng(0x41cca0);
    for (int it = 0; it < 100; ++it) {
        std::vector<int> kind(40);
        for (int& k : kind) k = static_cast<int>(rng() % 4);  // 0 null, 1 default image, else a real image
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            const std::uint32_t def = side ? ch::addr(ch::img(1, 0x004e06e0)) : 0x004e06e0u;
            rl.set(def, 0xDEF);
            std::vector<std::uint32_t> imgs(40, 0);
            for (int k = 0; k < 40; ++k) {
                if (kind[k] == 1) imgs[k] = def;
                else if (kind[k] > 1) {
                    auto* w = static_cast<std::uint32_t*>(ch::c_malloc(0x40));
                    std::memset(w, 0, 0x40);
                    imgs[k] = ch::addr(w);
                    rl.set(imgs[k], 0x100 + k);
                }
                *ch::img(side, 0x004db708 + 0x30 * k) = imgs[k];
            }
            ch::freed().clear();
            int ret;
            {
                ch::FreeHook hook;
                ret = fn[side](0, 0);
            }
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            for (int k = 0; k < 40; ++k) snap[side].push_back(rl(*ch::img(side, 0x004db708 + 0x30 * k)));
            for (int k = 0; k < 40; ++k)
                if (kind[k] > 1 && !ch::was_freed(imgs[k])) ch::c_free(imgs[k]);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

TEST(native_pickup_flag_trees_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int);
    const F fn[2][2] = {
        {rt::original<F>(0x0041ceb0), reinterpret_cast<F>(&recoil::Node_ClearPickupFlagsRecursive)},
        {rt::original<F>(0x0041cef0), reinterpret_cast<F>(&recoil::Node_SetPickupFlagsRecursive)},
    };
    std::mt19937 rng(0x41ceb0);
    for (int it = 0; it < 2000; ++it) {
        const int n = 1 + static_cast<int>(rng() % 30);
        std::vector<int> parent(n, -1);
        for (int k = 1; k < n; ++k) parent[k] = static_cast<int>(rng() % k);
        std::vector<std::uint32_t> f24(n);
        for (auto& f : f24) f = rng();
        const int which = static_cast<int>(rng() % 2), root = static_cast<int>(rng() % n);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<std::vector<std::uint32_t>> node(n, std::vector<std::uint32_t>(0xc4 / 4, 0)), kids(n);
            for (int k = 1; k < n; ++k) kids[parent[k]].push_back(ch::addr(node[k].data()));
            for (int k = 0; k < n; ++k) {
                node[k][0x24 / 4] = f24[k];
                node[k][0x5c / 4] = static_cast<std::uint32_t>(kids[k].size());
                node[k][0x60 / 4] = kids[k].empty() ? 0u : ch::addr(kids[k].data());
            }
            snap[side].push_back(static_cast<std::uint32_t>(fn[which][side](node[root].data(), 0)));
            for (int k = 0; k < n; ++k) snap[side].push_back(node[k][0x24 / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}
