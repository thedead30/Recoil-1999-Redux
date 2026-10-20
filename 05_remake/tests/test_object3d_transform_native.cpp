// Structured native L1 for the Object3D transform setters (ECX node; null or no class data +0x38 -> report, 5):
// Object3D_ResetTransform (0x0044d9e0: scale 1 / rotation 0 / identity matrix, data flags +0 bits 0 and 3 set, 4
// cleared), Object3D_SetScale (0x0044df00: +0x24..+0x2c, bit 3 kept only when all three are 1.0), Object3D_SetRotation
// (0x0044e030: +0x18..+0x20), Object3D_AddRotation (0x0044e170), Object3D_SetPosition (0x0044e300: +0x54..+0x5c),
// Object3D_AddPosition (0x0044e3d0: class +0x34 must be 5, else report, 3) - these five take three stack floats and keep
// bit 3 only while the values are all 0 - and Object3D_SetLocalMatrix (0x0044e4f0: EDX 12 floats copied to +0x30
// unless they are already there; class 5 required; bit 3 cleared, bits 0 and 4 set). Each then marks the subtree dirty
// (gwNodeMarkSubtreeDirty 0x0044d990: class 5 data bit 5, +0x2c bit 2, +0x24 bit 25, recursing into children +0x5c /
// +0x60 without bit 25), pushes a node without +0x24 bit 0 on list 7 (NodeRegistry_Insert) and sets +0x24 bits 0 and 1.
// A graph of 1..5 nodes (class 5 mostly, each with class data), child and parent references among them (cycles
// allowed); floats from {0, 1, -1, 0.5, random} so the all-zero / all-one tests go both ways. The same node storage is
// re-initialised for each side, so node words compare directly; the list-7 links by role.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zClass/Object3d.h"
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

TEST(native_object3d_transform_setters_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F3 = int(__fastcall*)(void*, int, float, float, float);
    using F0 = int(__fastcall*)(void*, void*);
    const std::uint32_t va3[5] = {0x0044df00, 0x0044e030, 0x0044e170, 0x0044e300, 0x0044e3d0};
    const F3 port3[5] = {reinterpret_cast<F3>(&recoil::Object3D_SetScale), reinterpret_cast<F3>(&recoil::Object3D_SetRotation),
                         reinterpret_cast<F3>(&recoil::Object3D_AddRotation), reinterpret_cast<F3>(&recoil::Object3D_SetPosition),
                         reinterpret_cast<F3>(&recoil::Object3D_AddPosition)};
    const F0 reset[2] = {rt::original<F0>(0x0044d9e0), reinterpret_cast<F0>(&recoil::Object3D_ResetTransform)};
    const F0 matrix[2] = {rt::original<F0>(0x0044e4f0), reinterpret_cast<F0>(&recoil::Object3D_SetLocalMatrix)};
    const char* names[7] = {"SetScale", "SetRotation", "AddRotation", "SetPosition", "AddPosition", "ResetTransform", "SetLocalMatrix"};
    std::mt19937 rng(0x44d9e0);
    auto pick = [&]() -> float {
        switch (rng() % 6) {
        case 0: case 1: return 0.0f;
        case 2: return 1.0f;
        case 3: return -1.0f;
        case 4: return 0.5f;
        default: return std::uniform_real_distribution<float>(-500, 500)(rng);
        }
    };
    int compared = 0, per_fn[7] = {};
    // one storage for both sides: pointers inside it are then the same on both
    static std::uint32_t node[5][0xC4 / 4], data[5][0x60 / 4], refs[5][2], kids[5][2], mat[12];
    for (int it = 0; it < 14000; ++it) {
        const int f = it % 7;
        const int n = 1 + static_cast<int>(rng() % 5);
        std::uint32_t node_init[5][0xC4 / 4], data_init[5][0x60 / 4], refs_init[5][2], kids_init[5][2], mat_init[12];
        for (int i = 0; i < n; ++i) {
            for (auto& w : node_init[i]) w = rng();
            for (auto& w : data_init[i]) w = bits(pick());
            data_init[i][0] = rng();
            node_init[i][0x34 / 4] = rng() % 5 ? 5u : rng() % 8;
            node_init[i][0x38 / 4] = rng() % 25 == 0 && i == 0 ? 0u : addr(data[i]);
            node_init[i][0x24 / 4] &= ~(1u | 0x2000000u);
            if (rng() % 4 == 0) node_init[i][0x24 / 4] |= 1u;
            if (i && rng() % 4 == 0) node_init[i][0x24 / 4] |= 0x2000000u;
            const std::uint32_t np = rng() % 3, nk = rng() % 3;
            for (int k = 0; k < 2; ++k) { refs_init[i][k] = addr(node[rng() % n]); kids_init[i][k] = addr(node[rng() % n]); }
            node_init[i][0x54 / 4] = np; node_init[i][0x58 / 4] = addr(refs[i]);
            node_init[i][0x5C / 4] = nk; node_init[i][0x60 / 4] = addr(kids[i]);
        }
        for (auto& w : mat_init) w = bits(pick());
        const bool null_node = rng() % 40 == 0, in_place = rng() % 6 == 0;
        const float a = pick(), b = pick(), c = pick();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::memcpy(node, node_init, sizeof node_init[0] * n);
            std::memcpy(data, data_init, sizeof data_init[0] * n);
            std::memcpy(refs, refs_init, sizeof refs_init[0] * n);
            std::memcpy(kids, kids_init, sizeof kids_init[0] * n);
            std::memcpy(mat, mat_init, sizeof mat);
            void* const target = null_node ? nullptr : node[0];
            int r;
            if (f < 5) r = (side ? port3[f] : rt::original<F3>(va3[f]))(target, 0, a, b, c);
            else if (f == 5) r = reset[side](target, nullptr);
            else r = matrix[side](target, in_place ? static_cast<void*>(&data[0][0x30 / 4]) : static_cast<void*>(mat));
            snap[side].push_back(static_cast<std::uint32_t>(r));
            for (int i = 0; i < n; ++i) {
                snap[side].insert(snap[side].end(), node[i], node[i] + 0xC4 / 4);
                snap[side].insert(snap[side].end(), data[i], data[i] + 0x60 / 4);
            }
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int i = 0; i < n; ++i) role[addr(node[i])] = 0x100u + i;
            std::uint32_t fresh = 0xC000;
            auto rl = [&](std::uint32_t v) { auto x = role.find(v); return x != role.end() ? x->second : (role[v] = fresh++); };
            std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * 7));
            snap[side].push_back(rl(p));
            snap[side].push_back(rl(*at(*img(side, 0x004ddf38u + 4 * 7))));
            for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) {
                snap[side].push_back(rl(at(p)[0])); snap[side].push_back(rl(at(p)[1]));
                snap[side].push_back(rl(at(p)[2])); snap[side].push_back(at(p)[3]);
            }
            snap[side].push_back(*img(side, 0x00539c74));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++per_fn[f];
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Object3D transform setters calls %d:", compared);
    for (int f = 0; f < 7; ++f) std::printf(" %s %d", names[f], per_fn[f]);
    std::printf(" (node graphs of 1..5, floats 0 / 1 / -1 / 0.5 / random)\n");
}
