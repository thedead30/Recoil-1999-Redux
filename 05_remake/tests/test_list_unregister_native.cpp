// Native L1 for List_UnregisterNode (0x0044f000, node at ECX): by class [+0x34] (jump table 0x0044f0e8, 0..11)
// NodeList_MarkNode on the class list (1 -> 8, 2 -> 13, 3 -> 14, 4 -> 15, 7 -> 11, 8 -> 12, 9 -> 9, 10 -> 10; 0, 5, 6,
// 11 none); a class above 11 formats the error text into 0x00575de0 and reports it (zerr_old_Report 3). Then list 7
// when byte +0x24 bit 0, list [+0x44] when [+0x48] is set and 0 <= [+0x44] < 6, and always list 6.
// NodeList_MarkNode(list, node): the list head holder [0x004ddef8 + 4*list] -> first element {node, ?, next, mark};
// the first element for the node with mark 0 gets mark 1 and the list's dirty word (0x00539bb4 + 12*list) is set.
// Each side: its own 16 list chains (0..3 elements each, some for the node, marks random) hung on its own holders, and
// its own node (class 0..13, flags, index -1..7). Compared: the 16 dirty words, every element's mark and, on the
// error path, the text buffer.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/List.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
}  // namespace

TEST(native_list_unregister_node_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x0044f000), reinterpret_cast<Fn>(&recoil::List_UnregisterNode)};
    std::mt19937 rng(0x44f000);
    int compared = 0;
    for (int it = 0; it < 6000; ++it) {
        std::uint32_t len[16], is_node[16][3], mark[16][3], other[16][3], el_w1[16][3];
        for (int l = 0; l < 16; ++l) {
            len[l] = rng() % 4;
            for (int e = 0; e < 3; ++e) {
                is_node[l][e] = rng() % 2;
                mark[l][e] = rng() % 3 == 0 ? 1u : 0u;
                other[l][e] = rng() | 1u;
                el_w1[l][e] = rng();
            }
        }
        std::uint32_t node_init[20];
        for (auto& w : node_init) w = rng();
        node_init[13] = rng() % 14;                                        // class +0x34
        node_init[17] = static_cast<std::uint32_t>(static_cast<int>(rng() % 9) - 1);  // index +0x44
        if (rng() % 3 == 0) node_init[18] = 0;                             // +0x48
        int ret[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t node[20];
            std::memcpy(node, node_init, sizeof node);
            std::uint32_t el[16][3][4];
            for (int l = 0; l < 16; ++l) {
                for (int e = 0; e < 3; ++e) {
                    el[l][e][0] = is_node[l][e] ? addr(node) : other[l][e];
                    el[l][e][1] = el_w1[l][e];
                    el[l][e][2] = static_cast<std::uint32_t>(e + 1) < len[l] ? addr(el[l][e + 1]) : 0u;
                    el[l][e][3] = mark[l][e];
                }
                const std::uint32_t holder = *img(side, 0x004ddef8u + 4 * l);
                *reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(holder)) = len[l] ? addr(el[l][0]) : 0u;
            }
            ret[side] = fn[side](node, 0);
            snap[side].push_back(static_cast<std::uint32_t>(ret[side]));
            for (int l = 0; l < 16; ++l) {
                snap[side].push_back(*img(side, 0x00539bb4u + 12 * l));
                for (int e = 0; e < 3; ++e) snap[side].push_back(el[l][e][3]);
            }
            if (node_init[13] > 11) {
                const auto* t = reinterpret_cast<const unsigned char*>(img(side, 0x00575de0));
                snap[side].insert(snap[side].end(), t, t + 128);
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  List_UnregisterNode calls %d (classes 0..13, 16 list chains, flags, index -1..7)\n", compared);
}
