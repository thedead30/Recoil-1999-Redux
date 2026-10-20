// Structured native L1 for gwNodeUpdateBoxAndPropagate (0x00448e90, ECX node): null -> report, 5; class 2 -> 0. The box
// by the node's flags: 0x200 and 0x400 -> per-axis union of the model box +0x8c and the children box +0xa4, only one ->
// that box, neither -> clear 0x100 and return; else copy it to +0x74, set 0x100 and +0x2c bit 2. Then for each parent:
// a world (class 2) -> the node's cell from its corners (gwNodeGetLocalCorners, World_BoxToCell; flag 0x80 -> no cell);
// the same cell -> World_RefreshCell, else World_DetachChild + World_InsertChildInCell; another parent -> +0x2c bit 1
// and, without +0x24 bit 0, a push on list 7.
// Each side builds the same scene with verified functions: a pool of 8 nodes, World_Create + World_BuildGrid (cells of
// 64, up to 6 x 6), an Object3D node (Object3D_Create: identity matrix) filed in the world (World_AttachChild) and
// attached to 0..2 other Object3D parents (gwNodeAttachChild); then the node's flags (0x80 / 0x200 / 0x400), model box
// and children box are set at random (inside, across or outside the grid) and the call is made - sometimes twice, so
// the second finds the node already in its cell. Compared by role: returns, every pool word, every class data block,
// the grid cells and their arrays, the queue, the lists and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Class.h"
#include "GameZRecoil/zClass/Object3d.h"
#include "GameZRecoil/zClass/cls_world.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <random>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t* at(std::uint32_t p) { return reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(p)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t bits(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
}  // namespace

TEST(native_gw_node_update_box_and_propagate_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F0 = std::uint32_t(__fastcall*)(int, int);
    using F2 = int(__fastcall*)(std::uint32_t, std::uint32_t);
    using Build = int(__fastcall*)(std::uint32_t, int, float, float);
    const F0 wcreate[2] = {rt::original<F0>(0x004501c0), reinterpret_cast<F0>(&recoil::World_Create)};
    const F0 ocreate[2] = {rt::original<F0>(0x0044daa0), reinterpret_cast<F0>(&recoil::Object3D_Create)};
    const Build build[2] = {rt::original<Build>(0x00450c60), reinterpret_cast<Build>(&recoil::World_BuildGrid)};
    const F2 wattach[2] = {rt::original<F2>(0x004510e0), reinterpret_cast<F2>(&recoil::World_AttachChild)};
    const F2 attach[2] = {rt::original<F2>(0x004484d0), reinterpret_cast<F2>(&recoil::gwNodeAttachChild)};
    const F2 update[2] = {rt::original<F2>(0x00448e90), reinterpret_cast<F2>(&recoil::gwNodeUpdateBoxAndPropagate)};
    std::mt19937 rng(0x448e90);
    auto fr = [&](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };
    int compared = 0, celled = 0;
    for (int it = 0; it < 3000; ++it) {
        const int nx = 1 + static_cast<int>(rng() % 6), nz = 1 + static_cast<int>(rng() % 6), parents = static_cast<int>(rng() % 3);
        const float ox = fr(-1000, 1000), oz = fr(-1000, 1000);
        const std::uint32_t flag_bits = (rng() % 5 == 0 ? 0x80u : 0u) | (rng() % 3 ? 0x200u : 0u) | (rng() % 3 ? 0x400u : 0u);
        float box[2][6];
        {  // model box: mostly small and inside the grid (a cell is 64 with 8-unit margins); children box inside it or sticking out
            const float half = rng() % 4 == 0 ? fr(64, 200) : fr(1, 6);
            const bool inside = rng() % 4 != 0;
            const float cx = inside ? fr(ox + 8, ox + 64 * nx - 8) : fr(ox - 64, ox + 64 * nx + 64);
            const float cz = inside ? fr(oz - 64 * nz + 8, oz - 8) : fr(oz - 64 * nz - 64, oz + 64), cy = fr(-20, 20);
            const float v[6] = {cx - half, cy - 3, cz - half, cx + half, cy + 3, cz + half};
            std::memcpy(box[0], v, sizeof v);
            const float s = fr(0.1f, 1.0f);  // half the time the children box sticks out (the union then differs from both)
            const bool out = rng() % 2 == 0;
            for (int k = 0; k < 3; ++k) {
                const float m = (v[k] + v[k + 3]) / 2 + (out ? fr(-3, 3) : 0.0f), h = (v[k + 3] - v[k]) / 2 * s;
                box[1][k] = m - h; box[1][k + 3] = m + h;
            }
        }
        const bool twice = rng() % 3 == 0, null_node = rng() % 50 == 0, on_world = rng() % 50 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> pool(49 * 8, 0);
            for (int i = 0; i < 8; ++i) pool[49 * i + 0xC0 / 4] = i + 1 < 8 ? static_cast<std::uint32_t>(i + 1) : 0xFFFFFFu;
            *img(side, 0x00539c94) = addr(pool.data());
            *img(side, 0x004de4c8) = 0;
            const std::uint32_t world = wcreate[side](0, 0);
            std::uint32_t* wd = at(at(world)[0x38 / 4]);
            wd[0x34 / 4] = bits(ox); wd[0x38 / 4] = bits(oz); wd[0x3C / 4] = bits(64.0f * nx); wd[0x40 / 4] = bits(-64.0f * nz);
            build[side](world, 0, 64.0f, -64.0f);
            const std::uint32_t node = ocreate[side](0, 0);
            wattach[side](world, node);
            for (int p = 0; p < parents; ++p) attach[side](ocreate[side](0, 0), node);
            std::uint32_t* nw = at(node);
            nw[0x24 / 4] = (nw[0x24 / 4] & ~0x680u) | flag_bits;
            for (int k = 0; k < 6; ++k) { nw[0x8c / 4 + k] = bits(box[0][k]); nw[0xa4 / 4 + k] = bits(box[1][k]); }
            const std::uint32_t target = on_world ? world : null_node ? 0u : node;
            snap[side].push_back(static_cast<std::uint32_t>(update[side](target, 0)));
            if (twice) snap[side].push_back(static_cast<std::uint32_t>(update[side](target, 0)));
            // roles: pool records, class data blocks, cells
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int i = 0; i < 8; ++i) role[addr(&pool[49 * i])] = 0x100u + i;
            const int gx = static_cast<int>(wd[0x78 / 4]), gz = static_cast<int>(wd[0x7C / 4]);
            for (int r = 0; r < gz; ++r) for (int c = 0; c < gx; ++c) role[at(wd[0x80 / 4])[r] + 64u * c] = 0x1000u + 16 * r + c;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            for (int i = 0; i < 8 * 49; ++i) {
                const int w = i % 49;
                snap[side].push_back(w == 0x38 / 4 || w == 0x58 / 4 || w == 0x60 / 4 ? rl(pool[i]) : pool[i]);
            }
            for (int i = 0; i < 8; ++i) {
                const std::uint32_t* p = &pool[49 * i];
                if (!p[0x38 / 4]) continue;
                const int words = p[0x34 / 4] == 2 ? 0xac / 4 : 0x90 / 4;
                for (int k = 0; k < words; ++k) snap[side].push_back(p[0x34 / 4] == 2 && (k == 3 || k == 0x80 / 4) ? rl(at(p[0x38 / 4])[k]) : at(p[0x38 / 4])[k]);
                for (std::uint32_t k = 0; k < p[0x54 / 4] && k < 8; ++k) snap[side].push_back(rl(at(p[0x58 / 4])[k]));
                for (std::uint32_t k = 0; k < p[0x5C / 4] && k < 8; ++k) snap[side].push_back(rl(at(p[0x60 / 4])[k]));
            }
            for (std::uint32_t k = 0; k < wd[1] && k < 64; ++k) snap[side].push_back(rl(at(wd[3])[k]));
            for (int r = 0; r < gz; ++r)
                for (int c = 0; c < gx; ++c) {
                    const std::uint32_t* cw = at(at(wd[0x80 / 4])[r] + 64u * c);
                    for (int k = 0; k < 15; ++k) snap[side].push_back(cw[k]);
                    for (int k = 0; k < reinterpret_cast<const std::int16_t*>(cw)[0x3a / 2] && k < 8; ++k) snap[side].push_back(rl(at(cw[15])[k]));
                }
            for (int l = 0; l < 16; ++l) {
                std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
                snap[side].push_back(*img(side, 0x00539bb4u + 12 * l));
                for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            }
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0 && static_cast<int>(nw[0x4C / 4]) >= 0) ++celled;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    CHECK(celled > 200);
    std::printf("  gwNodeUpdateBoxAndPropagate calls %d, %d left in a cell (flags 0x80/0x200/0x400, 0..2 other parents, twice)\n", compared, celled);
}
