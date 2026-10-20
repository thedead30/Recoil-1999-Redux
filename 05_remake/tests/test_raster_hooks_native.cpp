// Structured native L1s for the scan converters that hand their spans to the renderer's hooks (P4 rasteriser / P5
// menus, ported in the cloud). The arena fuzz cannot drive them: the hooks are function pointers in .data
// ([0x006320a4] / [0x006320a8] span visibility, [0x006320b0] / [0x006320b4] span fill, [0x00632100] pixel plot). Here
// every hook is a fake-table thunk (tests/fake_vtable.h) that logs its call; the visibility hooks also emulate their
// contract - ECX = the output array of visible-piece pointers, one stack word = the count - by splitting the span the
// caller left in [0x0057dae0] (+4 x0, +8 x1) into 0..2 pieces from records of this test. Functions:
//  - Raster_FillFlatPoly (0x00492000; ECX screen vertices (stride 12), EDX depth-plane points, stack n, pixel; ret 8);
//  - Raster_FillFlatPolyBlend (0x00492f00; also a level; ret 0xc);
//  - zRndr_FillClipPoly (0x004927d0; ECX vertices, EDX n);
//  - Draw_FillConvexPolygon2D (0x004936d0; ECX vertices, EDX n, stack pixel; ret 4);
//  - Draw_CirclePlot8 (0x00499020; ECX x, EDX y, stack colour; ret 4) and Draw_CircleMidpoint (0x00498fb0; ECX / EDX
//    centre, stack radius, colour, unused; ret 0xc);
//  - Queue_FlatPolygon_SW (0x00499a20; ECX vertices, EDX 9-dword record, stack a1, alpha, n, overwrite; ret 0x10):
//    appends to the overwrite queue (count [0x005cb270], 0x48C-byte records at 0x005cb274) or, opaque, draws through
//    Raster_FillFlatPoly, or appends to the translucent queue (count [0x0057de7c], 0x384-byte records at 0x0057de80);
//    counts up to the cap 350 (full -> the no-op reporter). The queue record written is compared too;
//  - zRndr_BeginFrame (0x00490590; render_frame): zeroes [0x0057dafc] words at [0x0057dae4], resets the span state
//    ([0x0057dae0] = [0x0057dae8]) and fills each of the [0x0057de30] (0..7) clip polygons at 0x0057db10 (stride 0x64,
//    count +0x60) through zRndr_FillClipPoly - so through the hooks;
//  - zRndr_AddClipPoly (0x00490710; ECX vertices, EDX count 0..12): appends to that table (ignored at 7; copies all
//    n vertices, stores min(n, 8)). The table 0x0057db10..0x0057de34 is compared for both;
//  - Raster_FillTexturedPoly (0x00493df0; ECX material, EDX vertices, stack plane, uv, n, shade; ret 0x10) and
//    Raster_FillTexturedPolyBlend (0x00494af0; also the level v; ret 0x14): the texture record (size, texels, alpha /
//    shade planes present or not), the level pick through a +0x20 chain now and then, the fill pairs 0x006320c8..d4 /
//    0x006320e0..ec (fake thunks taking u, v in ECX / EDX and n, [0x004e21f0] on the stack), spans through the
//    visibility hook; the texel globals 0x0056b260..0x0056b27c, 0x004e21f0..f8, 0x0057da48 compared too.
// Convex polygons of 3..8 vertices on a 640x480 screen (some degenerate, duplicated, off-screen), both windings
// ([0x0057dac8]); random depth scales, pitch and row base. Compared: the hook log (with the span record at each
// visibility call), the globals the functions write, EAX.
#include "test.h"
#include "vt_runner.h"
#include "unattributed/menus.h"
#include "unattributed/rasteriser.h"
#include "GameZRecoil/zRender/zrndr_draw.h"
#include "unattributed/render_frame.h"

#include <cmath>

using namespace vtr;

namespace {
using U = std::uint32_t;
constexpr int kVis = 32, kVisBlend = 33, kFill = 34, kFillBlend = 35, kPlot = 36, kTexA = 37, kTexB = 38;

// visible-piece records {0, x0, x1} handed out by the visibility hook (the same addresses on both sides)
U* pieces()
{
    static U p[2][3];
    return &p[0][0];
}
U& vis_calls()
{
    static U n = 0;
    return n;
}
// the span record the caller filled, as the side's image holds the pointer
U span_block(int side) { return *ch::img(side, 0x0057dae0); }
int& cur_side()
{
    static int s = 0;
    return s;
}

void set_hooks(int side)
{
    const U* t = ch::at(vt::table());
    *ch::img(side, 0x006320a4) = t[kVis];
    *ch::img(side, 0x006320a8) = t[kVisBlend];
    *ch::img(side, 0x006320b0) = t[kFill];
    *ch::img(side, 0x006320b4) = t[kFillBlend];
    *ch::img(side, 0x00632100) = t[kPlot];
    // the fill pairs {c8, d0} / {cc, d4} and {e0, e4} / {e8, ec}: each pair (A, B) or (B, A), so every choice shows
    for (U va : {0x006320c8u, 0x006320d4u, 0x006320e0u, 0x006320ecu}) *ch::img(side, va) = t[kTexA];
    for (U va : {0x006320ccu, 0x006320d0u, 0x006320e4u, 0x006320e8u}) *ch::img(side, va) = t[kTexB];
}
void hook_slots()
{
    auto vis = [](U out, const U* args) -> U {
        const U* span = ch::at(span_block(cur_side()));
        for (int k = 0; k < 6; ++k) vt::log().push_back(span[k]);
        const U x0 = span[1], x1 = span[2], n = (vis_calls()++ + x0) % 3;
        U* rec = pieces();
        U* arr = ch::at(out);
        for (U k = 0; k < n && k < 2; ++k) {
            rec[3 * k] = 0;
            rec[3 * k + 1] = x0 + k * ((x1 - x0) / 2);
            rec[3 * k + 2] = k + 1 < n ? x0 + (x1 - x0) / 2 : x1;
            arr[k] = ch::addr(&rec[3 * k]);
        }
        *ch::at(args[0]) = n;
        return n;
    };
    vt::set(kVis * 4, 1, 0);
    vt::slots()[kVis].fn = vis;
    vt::set(kVisBlend * 4, 1, 0);
    vt::slots()[kVisBlend].fn = vis;
    vt::set(kFill * 4, 0);
    vt::set(kFillBlend * 4, 1);
    vt::set(kPlot * 4, 2);
    vt::set(kTexA * 4, 2);
    vt::set(kTexB * 4, 2);
}
// vertices (x, y, z floats) of a convex polygon, n of them, possibly with duplicates / degenerate / off-screen
U polygon(World& w, std::mt19937& r, int n)
{
    const U p = w.add(r, 12 * 10);
    float* v = reinterpret_cast<float*>(w.at(p));
    const float cx = static_cast<float>(r() % 800) - 80.0f, cy = static_cast<float>(r() % 600) - 60.0f;
    const float rad = r() % 8 == 0 ? 0.0f : static_cast<float>(1 + r() % 200) + static_cast<float>(r() % 100) / 100.0f;
    const bool rev = r() % 2 != 0;
    for (int k = 0; k < n; ++k) {
        const int kk = rev ? n - 1 - k : k;
        const float a = 6.2831853f * static_cast<float>(kk) / static_cast<float>(n) + static_cast<float>(r() % 100) / 300.0f;
        v[3 * k] = cx + rad * std::cos(a);
        v[3 * k + 1] = cy + rad * std::sin(a) * (r() % 6 == 0 ? 0.0f : 1.0f);
        v[3 * k + 2] = static_cast<float>(r() % 1000) / 1000.0f;
        if (k && r() % 8 == 0) { v[3 * k] = v[3 * k - 3]; v[3 * k + 1] = v[3 * k - 2]; }
    }
    return p;
}
void raster_globals(World& w, std::mt19937& r)
{
    const U span = w.add(r, 0x40);
    *ch::img(w.side, 0x0057dae0) = span;
    *ch::img(w.side, 0x0057dac0) = fbits(static_cast<float>(r() % 100) / 10.0f);
    *ch::img(w.side, 0x0057dac4) = fbits(static_cast<float>(1 + r() % 100) / 7.0f);
    *ch::img(w.side, 0x0057dac8) = r() % 2;
    *ch::img(w.side, 0x00632050) = r() % 0x10000;
    *ch::img(w.side, 0x0063205c) = 640 * 2;
    *ch::img(w.side, 0x00632060) = 2;
    set_hooks(w.side);
    cur_side() = w.side;
    vis_calls() = 0;
}
U log_globals(World& w, U eax)
{
    for (U va : {0x0056b270u, 0x0056b23cu, 0x0056b240u, 0x0056b244u}) vt::log().push_back(*ch::img(w.side, va));
    return eax;
}

std::vector<Case> cases()
{
    std::vector<Case> c;
    c.push_back({"Raster_FillFlatPoly", 0x00492000, (void*)&recoil::Raster_FillFlatPoly, 600, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) {
                     polygon(w, r, 3 + static_cast<int>(r() % 6));
                     const U plane = w.add(r, 0x24);
                     for (int k = 0; k < 9; ++k) w.at(plane)[k] = fbits(static_cast<float>(r() % 1000) / 10.0f);
                     raster_globals(w, r);
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U n = 3 + r() % 6;
                     U a[2] = {n, r() % 0x10000};
                     return log_globals(w, call_any(fn, ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), a, 2));
                 },
                 false});
    c.push_back({"Raster_FillFlatPolyBlend", 0x00492f00, (void*)&recoil::Raster_FillFlatPolyBlend, 600, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) {
                     polygon(w, r, 3 + static_cast<int>(r() % 6));
                     const U plane = w.add(r, 0x24);
                     for (int k = 0; k < 9; ++k) w.at(plane)[k] = fbits(static_cast<float>(r() % 1000) / 10.0f);
                     raster_globals(w, r);
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U n = 3 + r() % 6;
                     U a[3] = {n, r() % 32, r() % 0x10000};
                     return log_globals(w, call_any(fn, ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), a, 3));
                 },
                 false});
    c.push_back({"zRndr_FillClipPoly", 0x004927d0, (void*)&recoil::zRndr_FillClipPoly, 600, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) { polygon(w, r, 3 + static_cast<int>(r() % 6)); raster_globals(w, r); },
                 [](World& w, U fn, std::mt19937& r) {
                     return log_globals(w, call_any(fn, ch::addr(w.blocks[0].w.data()), static_cast<U>(r() % 9), nullptr, 0));
                 },
                 false});
    c.push_back({"Draw_FillConvexPolygon2D", 0x004936d0, (void*)&recoil::Draw_FillConvexPolygon2D, 600, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) { polygon(w, r, 3 + static_cast<int>(r() % 6)); raster_globals(w, r); *ch::img(w.side, 0x0056b270) = 0; },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[1] = {r() % 0x10000};
                     return log_globals(w, call_any(fn, ch::addr(w.blocks[0].w.data()), static_cast<U>(r() % 9), a, 1));
                 },
                 false});
    c.push_back({"Draw_CirclePlot8", 0x00499020, (void*)&recoil::Draw_CirclePlot8, 300, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     raster_globals(w, r);
                     *ch::img(w.side, 0x0056b23c) = r() % 640;
                     *ch::img(w.side, 0x0056b240) = r() % 480;
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r() % 0x10000}; return log_globals(w, call_any(fn, r() % 100, r() % 100, a, 1)); }, false});
    c.push_back({"Draw_CircleMidpoint", 0x00498fb0, (void*)&recoil::Draw_CircleMidpoint, 300, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) { w.add(r, 4); raster_globals(w, r); *ch::img(w.side, 0x0056b244) = r() % 0x10000; },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[3] = {r() % 60, r() % 0x10000, r()};
                     return log_globals(w, call_any(fn, r() % 640, r() % 480, a, 3));
                 },
                 false});
    c.push_back({"Queue_FlatPolygon_SW", 0x00499a20, (void*)&recoil::Queue_FlatPolygon_SW, 800, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) {
                     polygon(w, r, 3 + static_cast<int>(r() % 6));
                     const U plane = w.add(r, 0x24);
                     for (int k = 0; k < 9; ++k) w.at(plane)[k] = fbits(static_cast<float>(r() % 1000) / 10.0f);
                     raster_globals(w, r);
                     *ch::img(w.side, 0x005cb270) = r() % 8 == 0 ? 350 : r() % 350;
                     *ch::img(w.side, 0x0057de7c) = r() % 8 == 0 ? 350 : r() % 350;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U n = 3 + r() % 6;
                     U a[4] = {r() % 0x10000, r() % 3 ? r() % 255 : 255 + r() % 10, n, r() % 3 == 0 ? 1u : 0u};
                     const U q0 = *ch::img(w.side, 0x005cb270), q1 = *ch::img(w.side, 0x0057de7c);
                     const U eax = log_globals(w, call_any(fn, ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), a, 4));
                     vt::log().push_back(*ch::img(w.side, 0x005cb270));
                     vt::log().push_back(*ch::img(w.side, 0x0057de7c));
                     if (q0 < 350) for (U k = 0; k < 0x48c / 4; ++k) vt::log().push_back(*ch::img(w.side, 0x005cb274 + 0x48c * q0 + 4 * k));
                     if (q1 < 350) for (U k = 0; k < 0x384 / 4; ++k) vt::log().push_back(*ch::img(w.side, 0x0057de80 + 0x384 * q1 + 4 * k));
                     return eax;
                 },
                 false});
    c.push_back({"zRndr_BeginFrame", 0x00490590, (void*)&recoil::zRndr_BeginFrame, 500, [](std::mt19937&) { hook_slots(); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     raster_globals(w, r);
                     *ch::img(w.side, 0x0057dafc) = r() % 17;
                     *ch::img(w.side, 0x0057dae4) = w.add(r, 4 * 16);
                     *ch::img(w.side, 0x0057dae8) = w.add(r, 0x40);   // the span record BeginFrame makes current
                     const U n = r() % 8;
                     *ch::img(w.side, 0x0057de30) = n;
                     for (U k = 0; k < n; ++k) {
                         const U m = 3 + r() % 6;
                         const float cx = static_cast<float>(r() % 800) - 80.0f, cy = static_cast<float>(r() % 600) - 60.0f;
                         const float rad = static_cast<float>(r() % 200);
                         for (U j = 0; j < m; ++j) {
                             const float a = 6.2831853f * static_cast<float>(j) / static_cast<float>(m);
                             *ch::img(w.side, 0x0057db10 + 0x64 * k + 12 * j) = fbits(cx + rad * std::cos(a));
                             *ch::img(w.side, 0x0057db10 + 0x64 * k + 12 * j + 4) = fbits(cy + rad * std::sin(a));
                             *ch::img(w.side, 0x0057db10 + 0x64 * k + 12 * j + 8) = fbits(static_cast<float>(r() % 1000) / 1000.0f);
                         }
                         *ch::img(w.side, 0x0057db10 + 0x64 * k + 0x60) = m;
                     }
                 },
                 [](World& w, U fn, std::mt19937&) {
                     cur_side() = w.side;
                     const U eax = log_globals(w, call_any(fn, 0, 0, nullptr, 0));
                     for (U va : {0x0057dae0u, 0x0057daf0u, 0x0057daf4u}) vt::log().push_back(w.norm(*ch::img(w.side, va)));
                     return eax;
                 },
                 false});
    c.push_back({"zRndr_AddClipPoly", 0x00490710, (void*)&recoil::zRndr_AddClipPoly, 300, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     polygon(w, r, 10);
                     *ch::img(w.side, 0x0057de30) = r() % 8;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, ch::addr(w.blocks[0].w.data()), r() % 11, nullptr, 0);
                     for (U va = 0x0057db10; va < 0x0057de34; va += 4) vt::log().push_back(*ch::img(w.side, va));
                     return eax;
                 },
                 false});
    // the textured fillers: block 0 vertices, block 1 plane, block 2 uv, block 3 material, then the texture records
    auto textured = [](World& w, std::mt19937& r) {
        polygon(w, r, 3 + static_cast<int>(r() % 6));
        const U plane = w.add(r, 0x24);
        for (int k = 0; k < 9; ++k) w.at(plane)[k] = fbits(k % 3 == 2 ? static_cast<float>(1 + r() % 1000) / 1000.0f : static_cast<float>(r() % 640));
        if (r() % 8 == 0) for (int k = 0; k < 3; ++k) w.at(plane)[6 + k] = w.at(plane)[k];  // a degenerate plane
        const U uv = w.add(r, 0x18);
        for (int k = 0; k < 6; ++k) w.at(uv)[k] = fbits(static_cast<float>(r() % 1000) / 1000.0f);
        const U mat = w.add(r, 0x30);
        auto tex = [&w, &r]() {
            const U t = w.add(r, 0x30);
            const U tw = 8u << (r() % 4), th = 8u << (r() % 4);
            reinterpret_cast<std::int16_t*>(w.at(t))[2] = static_cast<std::int16_t>(tw);
            reinterpret_cast<std::int16_t*>(w.at(t))[3] = static_cast<std::int16_t>(th);
            // texel blocks as big as the record says (16-bit texels) - they were 16 bytes for up to 64 x 64 textures, so
            // the filler read heap garbage past them and its texel pointers left every known block
            w.at(t)[4] = w.add(r, 2 * tw * th + 16);
            w.at(t)[5] = r() % 2 ? w.add(r, 2 * tw * th + 16) : 0u;
            w.at(t)[6] = r() % 2 ? w.add(r, 2 * tw * th + 16) : 0u;
            return t;
        };
        w.at(mat)[0] = tex();
        w.at(mat)[8] = 0;
        if (r() % 3 == 0) { const U node = w.add(r, 0x24); w.at(node)[0] = tex(); w.at(node)[8] = 0; w.at(mat)[8] = node; }
        raster_globals(w, r);
        *ch::img(w.side, 0x0063209c) = r() % 2;
    };
    auto tex_globals = [](World& w, U eax) {
        for (U va = 0x0056b260; va < 0x0056b280; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
        for (U va = 0x004e21f0; va < 0x004e21fc; va += 4) vt::log().push_back(*ch::img(w.side, va));
        vt::log().push_back(*ch::img(w.side, 0x0057da48));
        for (U va = 0x006320c0; va < 0x006320c8; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
        return log_globals(w, eax);
    };
    c.push_back({"Raster_FillTexturedPoly", 0x00493df0, (void*)&recoil::Raster_FillTexturedPoly, 600, [](std::mt19937&) { hook_slots(); },
                 textured,
                 [tex_globals](World& w, U fn, std::mt19937& r) {
                     U a[4] = {ch::addr(w.blocks[1].w.data()), ch::addr(w.blocks[2].w.data()), 3 + r() % 6,
                               r() % 3 ? static_cast<U>(-1) : r() % 4};
                     return tex_globals(w, call_any(fn, ch::addr(w.blocks[3].w.data()), ch::addr(w.blocks[0].w.data()), a, 4));
                 },
                 false});
    c.push_back({"Raster_FillTexturedPolyBlend", 0x00494af0, (void*)&recoil::Raster_FillTexturedPolyBlend, 600, [](std::mt19937&) { hook_slots(); },
                 textured,
                 [tex_globals](World& w, U fn, std::mt19937& r) {
                     U a[5] = {ch::addr(w.blocks[1].w.data()), ch::addr(w.blocks[2].w.data()), 3 + r() % 6,
                               fbits(static_cast<float>(r() % 256) / 255.0f), r() % 3 ? static_cast<U>(-1) : r() % 4};
                     return tex_globals(w, call_any(fn, ch::addr(w.blocks[3].w.data()), ch::addr(w.blocks[0].w.data()), a, 5));
                 },
                 false});
    return c;
}
}  // namespace

TEST(native_raster_hook_scan_converters_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases(), "raster hook scan converters"), 0);
}
