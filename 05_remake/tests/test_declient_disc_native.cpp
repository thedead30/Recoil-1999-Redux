// Structured native L1 for DEObjectA_BuildQuicksandDisc (0x00456010) and DEObjectB_BuildCraterDisc (0x00456c80) (ECX
// template; P2 declient, cloud port). Both create their object (DEObjectA_Create / DEObjectB_Create), find the current
// world (DEClient_GetCurrentWorld [0x00539e18]; no class data -> 0 - the object leaks, as in the original), the cell of the
// template's x / z (World_PointToCellClampedSimple, World_GetPartitionCell via [0x00539e1c]; none -> free, 0); a cell
// whose count byte +0x39 is not below the world's +0x4c -> free, 0. The centre is pulled inside the cell (cell origin +8
// / +0xc, world cell size +0x54 / +0x58, margins from .rdata), then count points (template +4) on a circle of the
// template radius (sin / cos, fsincos for large angles) at the template height, their XZ box, and each node of the cell
// (short count +0x3a, array +0x3c) whose inline name equals the string at 0x004df59c and whose +0x40 object is a type-1
// or type-3 disc with an overlapping box (margins from .rdata) -> free, 0; else the object.
// Offsets differ by one word between the two (A: points +0x30, cell +0x44, box +0x34..+0x40; B: +0x2c, +0x40, +0x30..
// +0x3c). Each call: a 2x2 grid world over [-100, 100] (cell size 100) built by hand, the cell's count byte and the world
// maximum random, 0..3 nodes in the cell (named as the string or not) holding type 1 / 3 / other objects with boxes near
// or away from the disc; templates of 1..7 points, radius, centre and height from sets. free is logged.
// Compared: the return (object by role), the object's words, its points, the free log by role.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zDEClient/zdec_crater.h"
#include "GameZRecoil/zDEClient/zdec_qsand.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

using namespace ch;

TEST(native_declient_build_disc_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2][2] = {{rt::original<Fn>(0x00456010), reinterpret_cast<Fn>(&recoil::DEObjectA_BuildQuicksandDisc)},
                         {rt::original<Fn>(0x00456c80), reinterpret_cast<Fn>(&recoil::DEObjectB_BuildCraterDisc)}};
    const char* const disc_name = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(0x004df59c));  // original image
    const float coords[] = {-120.0f, -99.0f, -50.0f, -1.0f, 0.0f, 3.5f, 49.0f, 50.0f, 98.0f, 150.0f};
    const float radii[] = {0.0f, 1.0f, 5.0f, 20.0f, 60.0f};
    FreeHook hook;
    std::mt19937 rng(0x456010);
    int compared = 0, built = 0, rejected = 0;
    for (int it = 0; it < 4000; ++it) {
        const int f = it % 2;  // 0 A (quicksand), 1 B (crater)
        const int tw = f ? 10 : 11, pts_off = f ? 0x2c : 0x30, cell_off = f ? 0x40 : 0x44;
        std::uint32_t tmpl[11];
        for (auto& w : tmpl) w = rng();
        tmpl[0] = rng() & ~0x1008u;  // no table lookups in Create
        tmpl[1] = 1 + rng() % 7;     // points (template +4 = object +8)
        // radius / x / y / z: object +0x1c.. (A) / +0x18.. (B) = template words 6.. (A) / 5.. (B)
        const int r0 = f ? 5 : 6;
        tmpl[r0] = fbits(radii[rng() % 5]);
        tmpl[r0 + 1] = fbits(coords[rng() % 10]);
        tmpl[r0 + 2] = fbits(coords[rng() % 10] / 10);
        tmpl[r0 + 3] = fbits(coords[rng() % 10]);
        const bool no_data = rng() % 20 == 0;
        const std::uint32_t max_count = 2 + rng() % 3, cell_count = rng() % 5;
        const int nodes = static_cast<int>(rng() % 4);
        int node_named[3], obj_type[3];
        float box[3][4];
        for (int k = 0; k < 3; ++k) {
            node_named[k] = rng() % 4 != 0;
            obj_type[k] = rng() % 5 == 0 ? 2 : rng() % 2 ? 1 : 3;
            for (float& b : box[k]) b = coords[rng() % 10];
        }
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t world[49], wdata[0xac / 4], cells[2][2][16], objs[3][20], cnodes[3][49];
            std::memset(world, 0, sizeof world);
            std::memset(wdata, 0, sizeof wdata);
            world[0x38 / 4] = no_data ? 0u : addr(wdata);
            wdata[0x34 / 4] = fbits(-100.0f); wdata[0x38 / 4] = fbits(-100.0f);
            wdata[0x44 / 4] = fbits(100.0f); wdata[0x48 / 4] = fbits(100.0f);
            wdata[0x54 / 4] = fbits(100.0f); wdata[0x58 / 4] = fbits(100.0f);
            wdata[0x64 / 4] = fbits(0.01f); wdata[0x68 / 4] = fbits(0.01f);
            wdata[0x4c / 4] = max_count;
            static std::uint32_t rows[2];
            rows[0] = addr(cells[0]); rows[1] = addr(cells[1]);
            wdata[0x80 / 4] = addr(rows);
            std::uint32_t arr[3];
            for (int z = 0; z < 2; ++z)
                for (int x = 0; x < 2; ++x) {
                    std::uint32_t* c = cells[z][x];
                    std::memset(c, 0, 64);
                    c[2] = fbits(-100.0f + 100 * x);
                    c[3] = fbits(-100.0f + 100 * z);
                    c[0x38 / 4] = (cell_count & 0xFF) << 8 | (static_cast<std::uint32_t>(nodes) << 16);  // +0x39 byte, +0x3a short
                    c[0x3c / 4] = addr(arr);
                }
            for (int k = 0; k < 3; ++k) {
                std::memset(cnodes[k], 0, sizeof cnodes[k]);
                std::strcpy(reinterpret_cast<char*>(cnodes[k]), node_named[k] ? disc_name : "other");
                for (int w = 0; w < 20; ++w) objs[k][w] = 0;
                objs[k][0] = static_cast<std::uint32_t>(obj_type[k]);
                for (int b = 0; b < 4; ++b) objs[k][0x30 / 4 + b] = fbits(box[k][b]);
                objs[k][0x40 / 4] = fbits(box[k][(k + 1) % 4]);
                cnodes[k][0x40 / 4] = addr(objs[k]);
                arr[k] = addr(cnodes[k]);
            }
            *img(side, 0x00539e18) = addr(world);
            *img(side, 0x00539e1c) = no_data ? 0u : addr(wdata);
            freed().clear();
            const std::uint32_t r = fn[f][side](tmpl, 0);
            Roles rl;
            for (int z = 0; z < 2; ++z) for (int x = 0; x < 2; ++x) rl.set(cells[z][x], 0xA0u + 2 * z + x);
            snap[side].push_back(rl(r));
            // the object: every word (the points array and the cell by role) and the points; freed ones only by role
            const std::uint32_t o = r;
            if (o) {
                for (int k = 0; k < 20; ++k) snap[side].push_back(k == pts_off / 4 || k == cell_off / 4 ? rl(at(o)[k]) : at(o)[k]);
                for (std::uint32_t k = 0; k < 3 * tmpl[1]; ++k) snap[side].push_back(at(at(o)[pts_off / 4])[k]);
            }
            for (std::uint32_t p : freed()) snap[side].push_back(rl(p));
            if (side == 0) { built += o != 0; rejected += o == 0; }
            if (o) {  // free what the call returned
                c_free(at(o)[pts_off / 4]);
                const std::uint32_t block = at(o)[(f ? 0x44 : 0x48) / 4];
                c_free(block);
                c_free(o);
            }
            (void)tw;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  DEObjectA / DEObjectB disc builders calls %d, %d built, %d refused\n", compared, built, rejected);
    CHECK(built > 300 && rejected > 300);
}
