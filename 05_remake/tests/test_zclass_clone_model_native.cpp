// Structured native L1 for ZClassNode_CloneModelIfSpecial (0x00451b20, ECX source node, EDX destination node; P2.6).
// No model at source +0x3c -> 0. Else, with the switch [0x004de4cc] off: gwNodeGetModel (0x00447f00) then
// gwNodeSetModel (0x00447e60) on the destination - the model is shared. Switch on: when the context [0x00539ca0] is
// set and Model_NeedsSpecialDraw (0x00483a60) says no, shared the same way; otherwise Model_Clone (0x00482270, EDX
// [0x00539c9c], argument the context) and the clone goes to the destination (a full model pool -> 1, nothing set).
// Each side: its own model pool [0x00576204] (2 records of 0x58 bytes, 0..2 free, linked through +0x54), a source model
// with no geometry (so the clone copies the header only; the deep copy is Model_Clone's own test) whose type word is
// 2..5 (Model_ComputeBoundsCentreRadius then only writes the centre / radius at +0x44..+0x50 - types 0 / 1 read geometry)
// and flags +4 with bit 2 (special) 1 time in 3, the destination node with no model or an old one (released), the
// switch, context and copy flag random, lists pristine. Compared by role: the return, every destination / source node
// word, every model record word (buffer pointers as null / non-null), the pool globals, list 7 and the link counter.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/cls_util.h"
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
}  // namespace

TEST(native_zclass_clone_model_if_special_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(void*, void*);
    const Fn fn[2] = {rt::original<Fn>(0x00451b20), reinterpret_cast<Fn>(&recoil::ZClassNode_CloneModelIfSpecial)};
    std::mt19937 rng(0x451b20);
    int compared = 0, per[4] = {};  // no model, shared, cloned, pool full
    for (int it = 0; it < 6000; ++it) {
        std::uint32_t src_i[49], dst_i[49], model_i[22], old_i[22], pool_i[44];
        for (auto& w : src_i) w = rng();
        for (auto& w : dst_i) w = rng();
        for (auto& w : model_i) w = rng();
        for (auto& w : old_i) w = rng();
        for (auto& w : pool_i) w = rng();
        for (std::uint32_t* m : {model_i, old_i}) {
            m[0] = 2 + rng() % 4;
            m[1] = (m[1] & ~0x2Cu) | (rng() % 3 == 0 ? 0x4u : 0u);
            m[2] = rng() % 4;  // reference count
            for (int k = 3; k <= 7; ++k) m[k] = 0;  // no polygons, vertices, normals, morph, point records
            for (int k = 12; k <= 16; ++k) m[k] = 0;
        }
        const int pool_free = static_cast<int>(rng() % 3);
        for (int k = 0; k < 2; ++k) pool_i[22 * k + 21] = k + 1 < pool_free ? k + 1 : 0xFFFFFFFFu;
        const bool has_model = rng() % 6 != 0, has_old = rng() % 2;
        const std::uint32_t sw = rng() % 4 ? 1u : 0u, ctx = rng() % 2, copy = rng() % 2;
        std::vector<std::uint32_t> snap[2];
        int path = 0;
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            static std::uint32_t src[49], dst[49], model[22], old[22], pool[44];
            std::memcpy(src, src_i, sizeof src); std::memcpy(dst, dst_i, sizeof dst);
            std::memcpy(model, model_i, sizeof model); std::memcpy(old, old_i, sizeof old); std::memcpy(pool, pool_i, sizeof pool);
            src[0x3C / 4] = has_model ? addr(model) : 0u;
            dst[0x3C / 4] = has_old ? addr(old) : 0u;
            *img(side, 0x00576204) = addr(pool);
            *img(side, 0x00576208) = static_cast<std::uint32_t>(2 - pool_free);
            *img(side, 0x0057620c) = pool_free ? 0u : 0xFFFFFFFFu;
            *img(side, 0x004de4cc) = sw;
            *img(side, 0x00539ca0) = ctx;
            *img(side, 0x00539c9c) = copy;
            const int r = fn[side](src, dst);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            role[addr(src)] = 0xA0; role[addr(dst)] = 0xA1; role[addr(model)] = 0xA2; role[addr(old)] = 0xA3;
            role[addr(pool)] = 0xA4; role[addr(pool + 22)] = 0xA5;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto q = role.find(v); return q != role.end() ? q->second : (role[v] = fresh++); };
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (int k = 0; k < 49; ++k) snap[side].push_back(k == 0x3C / 4 ? rl(dst[k]) : dst[k]);
            for (int k = 0; k < 49; ++k) snap[side].push_back(k == 0x3C / 4 ? rl(src[k]) : src[k]);
            for (const std::uint32_t* m : {static_cast<const std::uint32_t*>(model), static_cast<const std::uint32_t*>(old),
                                           static_cast<const std::uint32_t*>(pool), static_cast<const std::uint32_t*>(pool + 22)})
                for (int k = 0; k < 22; ++k) snap[side].push_back(k >= 12 && k <= 16 ? (m[k] ? 0x0B0Bu : 0u) : m[k]);
            snap[side].push_back(*img(side, 0x00576208));
            snap[side].push_back(*img(side, 0x0057620c));
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { snap[side].push_back(rl(at(p)[0])); snap[side].push_back(at(p)[3]); }
            snap[side].push_back(*img(side, 0x00539c74));
            if (side == 0) {
                const std::uint32_t now = rl(dst[0x3C / 4]);
                path = !has_model ? 0 : now == 0xA2 ? 1 : now == 0xA4 || now == 0xA5 ? 2 : r == 1 ? 3 : 1;
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++per[path];
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  ZClassNode_CloneModelIfSpecial calls %d: no model %d, shared %d, cloned %d, pool full %d\n", compared, per[0], per[1],
                per[2], per[3]);
    CHECK(per[1] > 500 && per[2] > 500 && per[3] > 100);
}
