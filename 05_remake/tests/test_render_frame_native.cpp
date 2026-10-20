// Structured native L1s for the render_frame functions ported in the cloud that the arena fuzz cannot drive - chain
// walks, counts that index static tables or stack buffers, a hook in .data - run by tests/vt_runner.h:
//  - Tex_PickLevel (0x0046e290; ECX chain head or 0, EDX level): the +0x20 chain of 0..4 nodes, levels -1..6;
//  - ObjectLights_SetPending (0x00476340; ECX object or 0, DL value): the 0x1C-byte entries [obj+0x30], count [+0xc];
//  - Clip_BuildFootprintEdges (0x00487900; ECX points, EDX count 0..66): the copies at [0x0057d984] and the normals at
//    [0x0057d988] (Vec3_NormalizeInPlace), the count [0x0057d98c];
//  - Clip_PointInFootprint (0x004879c0; ECX point, stack margin): 0..8 edges built by the previous function's layout;
//  - Light_SelectActive (0x00487a30; ECX nodes, EDX data, stack count 0..70): active nodes (flag 4 at +0x24) into the
//    0x14-byte slots at 0x0057d428 (cap 64, the reporter past it), the ambient / key-light globals 0x0057d3cc..;
//    software and hardware modes ([0x0056bbe8]);
//  - Backend_CollectActiveItems (0x0049a9c0; ECX start): the item table 0x0062ea04 (count [0x0062ea00] 0..70) into the
//    kept list 0x00631cd0 / [0x00631ccc];
//  - Backend_ItemScreenRay (0x0049aa30; ECX kept index, EDX out): zTransformScreenToWorld on a kept item;
//  - Palette_ApplyBrightness (0x004c8070; ECX palette or 0): the brightness [0x00632360] applied to 0x00632768 into a
//    stack copy handed to the hook [0x006333b8] (a fake-table thunk logging the 256 words); [0x00632154] 0 -> early out.
// Compared: the call log, EAX, every block word, the globals the functions write.
#include "test.h"
#include "vt_runner.h"
#include "GameZRecoil/zModel/gmod_light.h"
#include "unattributed/render_frame.h"

using namespace vtr;

namespace {
using U = std::uint32_t;

float frand(std::mt19937& r, float lo, float hi) { return lo + (hi - lo) * static_cast<float>(r() % 10000) / 10000.0f; }
void log_globals(World& w, U lo, U hi)
{
    for (U va = lo; va < hi; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
}
U arg0(World& w) { return ch::addr(w.blocks[0].w.data()); }

std::vector<Case> cases()
{
    std::vector<Case> c;
    c.push_back({"Tex_PickLevel", 0x0046e290, (void*)&recoil::Tex_PickLevel, 300, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U n = r() % 5;
                     U next = 0;
                     for (U k = 0; k < n; ++k) {
                         const U node = w.add(r, 0x24);
                         w.at(node)[8] = next;
                         next = node;
                     }
                     w.add(r, 4);                           // a block for the null-head case
                     w.blocks.back().w[0] = next;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     return call_any(fn, r() % 6 ? w.blocks.back().w[0] : 0u, static_cast<U>(static_cast<int>(r() % 8) - 1), nullptr, 0);
                 }});
    c.push_back({"ObjectLights_SetPending", 0x00476340, (void*)&recoil::ObjectLights_SetPending, 300, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U obj = w.add(r, 0x40);
                     const U n = r() % 6;
                     w.at(obj)[3] = r() % 8 ? n : static_cast<U>(-1);
                     const U arr = w.add(r, 0x1c * 6);
                     for (U k = 0; k < 6; ++k) reinterpret_cast<unsigned char*>(w.at(arr))[0x1c * k + 0x18] = static_cast<unsigned char>(r() % 3 ? 0 : 1);
                     w.at(obj)[0x30 / 4] = arr;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_any(fn, r() % 8 ? arg0(w) : 0u, r(), nullptr, 0); },
                 false});
    c.push_back({"Clip_BuildFootprintEdges", 0x00487900, (void*)&recoil::Clip_BuildFootprintEdges, 400, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U pts = w.add(r, 12 * 66);       // block 0: the points
                     for (U k = 0; k < 66 * 3; ++k) w.at(pts)[k] = fbits(frand(r, -500, 500));
                     *ch::img(w.side, 0x0057d984) = w.add(r, 12 * 64);
                     *ch::img(w.side, 0x0057d988) = w.add(r, 12 * 64);
                     *ch::img(w.side, 0x0057d98c) = r();
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U n = r() % 8 == 0 ? 60 + r() % 7 : r() % 8;
                     const U eax = call_any(fn, arg0(w), n, nullptr, 0);
                     log_globals(w, 0x0057d984, 0x0057d990);
                     return eax;
                 }});
    c.push_back({"Clip_PointInFootprint", 0x004879c0, (void*)&recoil::Clip_PointInFootprint, 600, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U p = w.add(r, 12);              // block 0: the point
                     for (U k = 0; k < 3; ++k) w.at(p)[k] = fbits(frand(r, -300, 300));
                     const U pts = w.add(r, 12 * 8), nrm = w.add(r, 12 * 8);
                     for (U k = 0; k < 24; ++k) { w.at(pts)[k] = fbits(frand(r, -200, 200)); w.at(nrm)[k] = fbits(frand(r, -1, 1)); }
                     *ch::img(w.side, 0x0057d984) = pts;
                     *ch::img(w.side, 0x0057d988) = nrm;
                     *ch::img(w.side, 0x0057d98c) = r() % 9;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[1] = {fbits(r() % 3 ? frand(r, -200, 20) : 0.0f)};
                     return call_any(fn, arg0(w), r(), a, 1);
                 }});
    c.push_back({"Light_SelectActive", 0x00487a30, (void*)&recoil::Light_SelectActive, 500, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U n = r() % 4 == 0 ? 60 + r() % 11 : r() % 8;
                     const U nodes = w.add(r, 4 * 72), data = w.add(r, 4 * 72);   // blocks 0 / 1: the arrays
                     const bool dense = r() % 4 == 0;
                     for (U k = 0; k < n; ++k) {
                         const U node = w.add(r, 0x28);
                         reinterpret_cast<unsigned char*>(w.at(node))[0x24] = static_cast<unsigned char>(dense || r() % 2 ? (r() % 256) | 4 : (r() % 256) & ~4u);
                         const U d = w.add(r, 0xc0);
                         w.at(d)[0xbc / 4] = r() % 3 ? 0u : 1u;
                         for (U j = 0; j < 4; ++j) w.at(d)[0xa8 / 4 + j] = fbits(frand(r, 0, 1));
                         w.at(nodes)[k] = node;
                         w.at(data)[k] = d;
                     }
                     w.blocks[0].w[71] = n;                 // the count, kept for the call
                     *ch::img(w.side, 0x0056bbe8) = r() % 4 == 0 ? 1u : 0u;
                 },
                 [](World& w, U fn, std::mt19937&) {
                     U a[1] = {w.blocks[0].w[71]};
                     const U eax = call_any(fn, arg0(w), ch::addr(w.blocks[1].w.data()), a, 1);
                     log_globals(w, 0x0057d3cc, 0x0057d428 + 0x14 * 64);
                     return eax;
                 },
                 false});
    c.push_back({"Backend_CollectActiveItems", 0x0049a9c0, (void*)&recoil::Backend_CollectActiveItems, 400, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U n = r() % 71;
                     const U zero = *reinterpret_cast<const U*>(static_cast<std::uintptr_t>(0x004d2e44));
                     for (U k = 0; k < n; ++k) {
                         U* it = ch::img(w.side, 0x0062ea04 + 0x14 * k);
                         it[2] = r() % 4 ? fbits(frand(r, -5, 5)) : zero;
                         U obj = 0;
                         if (r() % 4) { obj = w.add(r, 0x10); w.at(obj)[3] = r() % 4 ? 1u : 0u; }
                         it[4] = obj;
                     }
                     *ch::img(w.side, 0x0062ea00) = n;
                     w.add(r, 4);
                     w.blocks.back().w[0] = r() % 5 ? r() % (n + 3) : 0u;  // the start index
                 },
                 [](World& w, U fn, std::mt19937&) {
                     const U eax = call_any(fn, w.blocks.back().w[0], 0, nullptr, 0);
                     log_globals(w, 0x00631ccc, 0x00631cd0 + 4 * 64);
                     return eax;
                 }});
    c.push_back({"Backend_ItemScreenRay", 0x0049aa30, (void*)&recoil::Backend_ItemScreenRay, 300, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U out = w.add(r, 0x40);          // block 0: the output
                     for (U k = 0; k < 4; ++k) {
                         const U item = w.add(r, 0x14);
                         for (U j = 0; j < 3; ++j) w.at(item)[j] = fbits(frand(r, 0, 600));
                         *ch::img(w.side, 0x00631cd0 + 4 * k) = item;
                     }
                     (void)out;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_any(fn, r() % 4, arg0(w), nullptr, 0); }});
    c.push_back({"Palette_ApplyBrightness", 0x004c8070, (void*)&recoil::Palette_ApplyBrightness, 400,
                 [](std::mt19937&) { vt::set(0, 1, 1, 256, 0); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 0x400);                       // block 0: the palette
                     *ch::img(w.side, 0x00632154) = r() % 6 ? 1u : 0u;
                     *ch::img(w.side, 0x00632360) = r() % 256;
                     *ch::img(w.side, 0x006333b8) = ch::at(vt::table())[0];
                     for (U k = 0; k < 256; ++k) *ch::img(w.side, 0x00632768 + 4 * k) = r();
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 4 ? arg0(w) : 0u, 0, nullptr, 0);
                     log_globals(w, 0x00632768, 0x00632768 + 0x400);
                     return eax;
                 }});
    return c;
}
}  // namespace

TEST(native_render_frame_structured_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases(), "render_frame structured"), 0);
}
