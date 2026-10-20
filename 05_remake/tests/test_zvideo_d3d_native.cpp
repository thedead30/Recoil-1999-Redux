// Structured native L1s for the zvideo DirectDraw / Direct3D helpers ported in the cloud that the arena fuzz cannot
// drive - hooks in .data, COM objects, a power-of-two loop that never ends past 2^30 - run by tests/vt_runner.h with
// fake-table objects and hooks (tests/fake_vtable.h):
//  - zVideo_SetFlag0063212c (0x004a71c0; ECX value): the refusal while pixel-doubled, the hook [0x0056bc00] when
//    turning the flag off on a non-DirectDraw path;
//  - HwBlend_ApplyColourIfChanged (0x004a7330) / HwBlend_ApplyColourBIfChanged (0x004a73a0): the shared colour cache
//    [0x006321f4..fc] against [0x006321d0..d8] / [0x006321e8..f0] (equal or not, component by component), the tail
//    jump through [0x006333d4];
//  - Render_FlipToGDISurface (0x004a7d70): DirectDraw [0x006333d8] (or 0) slot 0x28 when [0x00632134];
//  - DDraw_CreateSurface3 (0x004a88b0; ECX DirectDraw, EDX description, stack out, unused; ret 8): CreateSurface
//    (slot 0x18), QueryInterface for Surface3 (slot 0), Release (slot 2), HRESULTs from the seed;
//  - Math_FloorPowerOfTwo (0x004ad680; ECX): values up to 2^30 (above it the original loops forever), 0 and negatives.
//  - D3DTextureBinding_Free (0x004aa980; ECX record): the three interfaces released (slot 2), free - unless the record
//    is the shared default [0x006333a8];
//  - Render_SetFogEnabled (0x004aa9e0; ECX flag) / Render_FlushQuadDrawQueue (0x004ad120): the device [0x006333f0]'s
//    SetRenderState (slot 0x5c), SetLightState (0x64), DrawPrimitive (0x74, 0..6 quads at 0x0063363c) through the
//    state caches 0x00633408..0x00633434;
//  - D3DTextureBinding_ConvertImageToArgb16 (0x004aa6f0; ECX destination, EDX image, stack pitch, flag; ret 8): 1555
//    or, with an alpha plane, 4444 (565 / 555 by [0x0063215c]); images up to 12 x 8 with random masks;
//  - ZVid_LoadPalette (0x004c7fd0; ECX path or 0 - the last path at 0x00632260): fopen / fread (a temp file of 768
//    bytes, or a missing one: fprintf to stderr, 0x800), then Palette_ApplyBrightness with its hook [0x006333b8] a fake.
// free is bound to a fake that logs the pointer and frees nothing.
// Compared: the call log, EAX, every block word, the globals the functions write.
#include "test.h"
#include "vt_runner.h"
#include "GameZRecoil/zVideo/zvid_buff.h"
#include "GameZRecoil/zVideo/zvid_dd.h"
#include "GameZRecoil/zVideo/zvid_ddd3d_zvideo.h"
#include "unattributed/zvideo.h"
#include "platform/iat_msvcrt.h"

#include <string>

using namespace vtr;

namespace {
using U = std::uint32_t;

U hres(std::mt19937& r) { return r() % 4 ? 0u : 0x80004005u; }
void log_globals(World& w, U lo, U hi)
{
    for (U va = lo; va < hi; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
}
U arg0(World& w) { return ch::addr(w.blocks[0].w.data()); }
float frand(std::mt19937& r) { return static_cast<float>(r() % 4) / 4.0f; }

void __cdecl f_free(void* p) { vt::log().push_back(0xF4EE); vt::log().push_back(vt::norm()(ch::addr(p))); }
struct BindFree {
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* saved[2];
    BindFree() { saved[0] = *o; saved[1] = recoil::g_Iat_free_004cc5b4; *o = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&f_free); }
    ~BindFree() { *o = saved[0]; recoil::g_Iat_free_004cc5b4 = saved[1]; }
};
std::string& palette_path()
{
    static std::string p;
    return p;
}

std::vector<Case> cases()
{
    std::vector<Case> c;
    c.push_back({"zVideo_SetFlag0063212c", 0x004a71c0, (void*)&recoil::zVideo_SetFlag0063212c, 300,
                 [](std::mt19937& r) { vt::set(0, 0, 0, 0, r()); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     *ch::img(w.side, 0x0056bc00) = ch::at(vt::table())[0];
                     *ch::img(w.side, 0x0063212c) = r() % 3;
                     *ch::img(w.side, 0x00632128) = r() % 3 == 0 ? 1u : 0u;
                     *ch::img(w.side, 0x00632120) = r() % 2;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 3, r(), nullptr, 0);
                     log_globals(w, 0x0063212c, 0x00632130);
                     return eax;
                 }});
    for (int b = 0; b < 2; ++b) {
        const U src = b ? 0x006321e8 : 0x006321d0;
        c.push_back({b ? "HwBlend_ApplyColourBIfChanged" : "HwBlend_ApplyColourIfChanged", b ? 0x004a73a0u : 0x004a7330u,
                     b ? (void*)&recoil::HwBlend_ApplyColourBIfChanged : (void*)&recoil::HwBlend_ApplyColourIfChanged, 300,
                     [](std::mt19937& r) { vt::set(4, 0, 0, 0, r()); },
                     [src](World& w, std::mt19937& r) {
                         w.add(r, 4);
                         *ch::img(w.side, 0x006333d4) = ch::at(vt::table())[1];
                         for (U k = 0; k < 3; ++k) {
                             const U v = fbits(frand(r));
                             *ch::img(w.side, src + 4 * k) = v;
                             *ch::img(w.side, 0x006321f4 + 4 * k) = r() % 4 ? v : fbits(frand(r));
                         }
                     },
                     [](World& w, U fn, std::mt19937& r) {
                         const U eax = call_any(fn, r(), r(), nullptr, 0);
                         log_globals(w, 0x006321d0, 0x00632200);
                         return eax;
                     }});
    }
    c.push_back({"Render_FlipToGDISurface", 0x004a7d70, (void*)&recoil::Render_FlipToGDISurface, 200,
                 [](std::mt19937& r) { vt::set(0x28, 1, 1, 0, r() % 3); },
                 [](World& w, std::mt19937& r) {
                     const U dd = object(w, r, 0x10);
                     *ch::img(w.side, 0x006333d8) = r() % 4 ? dd : 0u;
                     *ch::img(w.side, 0x00632134) = r() % 3 ? 1u : 0u;
                 },
                 [](World&, U fn, std::mt19937& r) { return call_any(fn, r(), r(), nullptr, 0); },
                 false});
    c.push_back({"DDraw_CreateSurface3", 0x004a88b0, (void*)&recoil::DDraw_CreateSurface3, 300,
                 [](std::mt19937&) { vt::set(0x18, 4); vt::set(0, 3); vt::set(8, 1, 1, 0, 0); },
                 [](World& w, std::mt19937& r) {
                     object(w, r, 0x10);                   // block 0: DirectDraw
                     w.add(r, 0x6c);                       // block 1: the description
                     const U surf = object(w, r, 0x10);    // block 2: the surface handed out
                     const U s3 = object(w, r, 0x10);      // block 3: the Surface3 handed out
                     w.add(r, 4);                          // block 4: the out pointer
                     const U hc = hres(r), hq = hres(r);
                     // CreateSurface (this, desc, out, outer) writes the surface; QueryInterface (this, iid, out) the
                     // Surface3 (the IID logged as its original VA)
                     vt::slots()[6].fn = [surf, hc](U, const U* a) -> U { if (!hc) *ch::at(a[2]) = surf; return hc; };
                     vt::slots()[0].fn = [s3, hq](U, const U* a) -> U {
                         vt::log().push_back(vt::norm()(a[1]));
                         if (!hq) *ch::at(a[2]) = s3;
                         return hq;
                     };
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[2] = {ch::addr(w.blocks[4].w.data()), r()};
                     return call_any(fn, arg0(w), ch::addr(w.blocks[1].w.data()), a, 2);
                 }});
    c.push_back({"Math_FloorPowerOfTwo", 0x004ad680, (void*)&recoil::Math_FloorPowerOfTwo, 400, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) { w.add(r, 4); },
                 [](World&, U fn, std::mt19937& r) {
                     const U k = r() % 4;
                     const U v = k == 0 ? static_cast<U>(static_cast<int>(r() % 5) - 3) : k == 1 ? 1u << (r() % 31) : r() % 0x40000001u;
                     return call_any(fn, v, r(), nullptr, 0);
                 }});
    c.push_back({"D3DTextureBinding_Free", 0x004aa980, (void*)&recoil::D3DTextureBinding_Free, 300,
                 [](std::mt19937&) { vt::set(8, 1, 1, 0, 0); },
                 [](World& w, std::mt19937& r) {
                     const U rec = w.add(r, 0x1c);         // block 0: the record
                     for (U k = 0; k < 3; ++k) w.at(rec)[k] = r() % 3 ? object(w, r, 0x10) : 0u;
                     *ch::img(w.side, 0x006333a8) = r() % 6 == 0 ? rec : 0u;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, arg0(w), 0, nullptr, 0); },
                 false});
    c.push_back({"Render_SetFogEnabled", 0x004aa9e0, (void*)&recoil::Render_SetFogEnabled, 300,
                 [](std::mt19937& r) { vt::set(0x5c, 3, 3, 0, hres(r)); vt::set(0x64, 3, 3, 0, hres(r)); },
                 [](World& w, std::mt19937& r) {
                     *ch::img(w.side, 0x006333f0) = object(w, r, 0x10);
                     *ch::img(w.side, 0x00633430) = r() % 3;
                     *ch::img(w.side, 0x00633434) = r() % 2 ? 3u : r() % 5;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 3, 0, nullptr, 0);
                     log_globals(w, 0x00633430, 0x00633438);
                     return eax;
                 },
                 false});
    c.push_back({"Render_FlushQuadDrawQueue", 0x004ad120, (void*)&recoil::Render_FlushQuadDrawQueue, 300,
                 [](std::mt19937& r) { vt::set(0x5c, 3, 3, 0, hres(r)); vt::set(0x74, 6, 6, 0, hres(r)); },
                 [](World& w, std::mt19937& r) {
                     *ch::img(w.side, 0x006333f0) = object(w, r, 0x10);
                     *ch::img(w.side, 0x00633638) = r() % 4 == 0 ? 0u : r() % 7;
                     for (U va : {0x00633408u, 0x0063340cu, 0x00633424u, 0x00633428u}) *ch::img(w.side, va) = r() % 3;
                     for (U va = 0x0063363c; va < 0x0063363c + 0x80 * 6; va += 4) *ch::img(w.side, va) = r();
                 },
                 [](World& w, U fn, std::mt19937&) {
                     const U eax = call_any(fn, 0, 0, nullptr, 0);
                     log_globals(w, 0x00633408, 0x00633430);
                     log_globals(w, 0x00633638, 0x0063363c);
                     return eax;
                 },
                 false});
    c.push_back({"D3DTextureBinding_ConvertImageToArgb16", 0x004aa6f0, (void*)&recoil::D3DTextureBinding_ConvertImageToArgb16, 600,
                 [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U wd = r() % 13, ht = r() % 9;
                     w.add(r, 32 * 10);                    // block 0: the destination
                     const U img = w.add(r, 0x18);         // block 1: the image
                     reinterpret_cast<std::int16_t*>(w.at(img))[2] = static_cast<std::int16_t>(r() % 10 == 0 ? -1 : static_cast<int>(wd));
                     reinterpret_cast<std::int16_t*>(w.at(img))[3] = static_cast<std::int16_t>(ht);
                     const U px = w.add(r, 2 * 12 * 8 + 4);
                     for (U k = 0; k < 12 * 8 / 2; ++k) if (r() % 5 == 0) w.at(px)[k] &= 0xffff0000u;  // some zero pixels
                     w.at(img)[4] = px;
                     w.at(img)[5] = r() % 2 ? w.add(r, 12 * 8 + 4) : 0u;
                     *ch::img(w.side, 0x0063215c) = r() % 2 ? 6u : 5u;
                     for (U va = 0x00632164; va < 0x00632170; va += 4) *ch::img(w.side, va) = r() % 0x10000;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[2] = {24 + 2 * (r() % 4), r()};
                     return call_any(fn, arg0(w), ch::addr(w.blocks[1].w.data()), a, 2);
                 },
                 false});
    c.push_back({"ZVid_LoadPalette", 0x004c7fd0, (void*)&recoil::ZVid_LoadPalette, 60,
                 [](std::mt19937&) { vt::set(0, 1, 1, 256, 0); },
                 [](World& w, std::mt19937& r) {
                     const U s = w.add(r, 0x100);          // block 0: the path
                     const std::string p = r() % 4 ? palette_path() : palette_path() + ".missing";
                     std::memcpy(w.at(s), p.c_str(), p.size() + 1);
                     const std::string last = palette_path();
                     std::memcpy(ch::img(w.side, 0x00632260), last.c_str(), last.size() + 1);
                     *ch::img(w.side, 0x00632154) = r() % 4 ? 1u : 0u;
                     *ch::img(w.side, 0x00632360) = r() % 256;
                     *ch::img(w.side, 0x006333b8) = ch::at(vt::table())[0];
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 5 ? arg0(w) : 0u, 0, nullptr, 0);
                     log_globals(w, 0x00632260, 0x00632368 + 0x300);
                     log_globals(w, 0x00632768, 0x00632768 + 0x400);
                     return eax;
                 }});
    return c;
}
}  // namespace

TEST(native_zvideo_d3d_helpers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    palette_path() = ch::temp_path("pal");
    std::string bytes(768, '\0');
    for (int k = 0; k < 768; ++k) bytes[k] = static_cast<char>((k * 37 + 11) & 0xff);
    ch::write_file(palette_path(), bytes);
    BindFree bf;
    CHECK_EQ(run_cases(cases(), "zvideo DirectDraw / Direct3D helpers"), 0);
    DeleteFileA(palette_path().c_str());
}
