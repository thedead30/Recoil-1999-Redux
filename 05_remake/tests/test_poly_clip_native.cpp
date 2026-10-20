// Structured native L1 for the eleven polygon clippers of poly_shade (ported in the cloud). They clip the gathered
// vertex arrays in .bss in place - camera space 0x0057c5c4 (near plane) or projected 0x0057c8c4 (screen edges), stride
// 12 - with the UV pairs behind the pointer [0x0057cdc4] and the attribute arrays 0x0057d0cc / 0x0057d1cc / 0x0057d2cc,
// through stack buffers of 62..64 vertices; ECX = the clip record R (flags R[0]: 0x20 far reject against R[6], 0x10 near
// clip at R[3]; 1 / 2 / 4 / 8 the screen edges x >= R[1], x < R[7], y >= R[2], y < R[8]), EDX -> the vertex count. The
// arena fuzz cannot drive them: the count is unbounded against those buffers. Here: 3..10 vertices around a centre near
// the planes (a jittered circle, now and then a flat, duplicated or random polygon), random flag sets, the arrays filled
// from the seed; both sides from one seed. Compared: the return, the count, all four arrays (64 entries each) and the
// UV block.
#include "test.h"
#include "vt_runner.h"
#include "unattributed/poly_shade.h"

#include <cmath>

using namespace vtr;

namespace {
using U = std::uint32_t;
constexpr U kCam = 0x0057c5c4, kProj = 0x0057c8c4, kUv = 0x0057cdc4, kA0 = 0x0057d0cc, kA1 = 0x0057d1cc, kA2 = 0x0057d2cc;

float frand(std::mt19937& r, float lo, float hi) { return lo + (hi - lo) * static_cast<float>(r() % 10000) / 10000.0f; }

void build(World& w, std::mt19937& r, bool near_plane)
{
    const U rec = w.add(r, 0x24);                  // block 0: R
    const U cnt = w.add(r, 4);                     // block 1: the count
    const U uv = w.add(r, 64 * 8);                 // block 2: the UV pairs
    float* R = reinterpret_cast<float*>(w.at(rec));
    U flags = 0;
    for (U bit : {1u, 2u, 4u, 8u, 0x10u, 0x20u}) if (r() % 3) flags |= bit;
    w.at(rec)[0] = flags;
    R[1] = frand(r, -20, 40); R[2] = frand(r, -20, 40); R[3] = frand(r, 0.5f, 4);
    R[4] = frand(r, 300, 700); R[5] = frand(r, 200, 500); R[6] = frand(r, 50, 200);
    R[7] = frand(r, 300, 700); R[8] = frand(r, 200, 500);
    const U n = 3 + r() % 8;
    w.at(cnt)[0] = n;
    for (U k = 0; k < 64 * 2; ++k) w.at(uv)[k] = fbits(frand(r, 0, 1));
    *ch::img(w.side, kUv) = uv;
    for (U va : {kCam, kProj}) for (U k = 0; k < 64 * 3; ++k) *ch::img(w.side, va + 4 * k) = fbits(frand(r, -5, 5));
    for (U va : {kA0, kA1, kA2}) for (U k = 0; k < 64; ++k) *ch::img(w.side, va + 4 * k) = fbits(frand(r, 0, 255));
    // the polygon: camera space around the near / far planes, or screen space around the edges
    const int shape = static_cast<int>(r() % 8);
    const float cx = near_plane ? frand(r, -3, 3) : frand(r, -100, 700), cy = near_plane ? frand(r, -3, 3) : frand(r, -100, 550);
    const float cz = near_plane ? frand(r, -2, 8) : frand(r, 0, 1), rad = near_plane ? frand(r, 0, 6) : frand(r, 0, 400);
    const U base = near_plane ? kCam : kProj;
    for (U k = 0; k < n; ++k) {
        const float a = 6.2831853f * static_cast<float>(k) / static_cast<float>(n) + frand(r, 0, 0.3f);
        float v[3] = {cx + rad * std::cos(a), cy + rad * std::sin(a), cz + (near_plane ? rad * std::sin(a + 1.0f) : frand(r, 0, 1))};
        if (shape == 0) v[1] = cy;                                          // flat
        if (shape == 1 && k && r() % 2) {                                  // a duplicate of the previous vertex
            for (int j = 0; j < 3; ++j) *ch::img(w.side, base + 12 * k + 4 * j) = *ch::img(w.side, base + 12 * (k - 1) + 4 * j);
            continue;
        }
        if (shape == 2) for (int j = 0; j < 2; ++j) v[j] = near_plane ? frand(r, -6, 6) : frand(r, -200, 900);  // random
        for (int j = 0; j < 3; ++j) *ch::img(w.side, base + 12 * k + 4 * j) = fbits(v[j]);
    }
}
U call_and_log(World& w, U fn)
{
    const U eax = call_any(fn, ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), nullptr, 0);
    for (U va : {kCam, kProj}) for (U k = 0; k < 64 * 3; ++k) vt::log().push_back(*ch::img(w.side, va + 4 * k));
    for (U va : {kA0, kA1, kA2}) for (U k = 0; k < 64; ++k) vt::log().push_back(*ch::img(w.side, va + 4 * k));
    vt::log().push_back(w.norm(*ch::img(w.side, kUv)));
    return eax;
}

std::vector<Case> cases()
{
    struct F { const char* name; U va; void* port; bool near_plane; };
    const F fs[] = {
        {"Poly_ClipNearZ", 0x0047a200, (void*)&recoil::Poly_ClipNearZ, true},
        {"Poly_ClipNearZ_3Attr", 0x0047a4e0, (void*)&recoil::Poly_ClipNearZ_3Attr, true},
        {"Poly_ClipNearZ_UV", 0x0047aa80, (void*)&recoil::Poly_ClipNearZ_UV, true},
        {"Poly_ClipNearZ_UV_1Attr", 0x0047af60, (void*)&recoil::Poly_ClipNearZ_UV_1Attr, true},
        {"Poly_ClipNearZ_UV_3Attr", 0x0047e900, (void*)&recoil::Poly_ClipNearZ_UV_3Attr, true},
        {"Poly_ClipScreenEdges", 0x0047b540, (void*)&recoil::Poly_ClipScreenEdges, false},
        {"Poly_ClipScreenEdges_3Attr", 0x0047bd30, (void*)&recoil::Poly_ClipScreenEdges_3Attr, false},
        {"Poly_ClipScreenEdges_XY", 0x0047cdc0, (void*)&recoil::Poly_ClipScreenEdges_XY, false},
        {"Poly_ClipScreenEdges_UV", 0x0047d3f0, (void*)&recoil::Poly_ClipScreenEdges_UV, false},
        {"Poly_ClipScreenEdges_XY_1Attr", 0x0047dfb0, (void*)&recoil::Poly_ClipScreenEdges_XY_1Attr, false},
        {"Poly_ClipScreenEdges_UV_3Attr", 0x0047efd0, (void*)&recoil::Poly_ClipScreenEdges_UV_3Attr, false},
    };
    std::vector<Case> c;
    for (const F& f : fs) {
        const bool np = f.near_plane;
        c.push_back({f.name, f.va, f.port, 1500, [](std::mt19937&) {},
                     [np](World& w, std::mt19937& r) { build(w, r, np); },
                     [](World& w, U fn, std::mt19937&) { return call_and_log(w, fn); }});
    }
    return c;
}
}  // namespace

TEST(native_poly_clippers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases(), "poly_shade clippers"), 0);
}
