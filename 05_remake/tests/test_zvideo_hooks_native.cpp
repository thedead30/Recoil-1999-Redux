// Structured native L1 for the zvideo surface wrappers ported in the cloud that call the video back end through the
// function pointers in .data ([0x006333c0] / [0x006333c4] / [0x006333c8] per-surface slots, [0x006333b4] present),
// which the arena fuzz cannot follow. Every hook is a fake-table thunk (tests/fake_vtable.h) logging ECX (the surface
// record 0x00632200 / 0x00632220 / 0x00632240, normalised to its original VA) and its stack arguments; the 0x3C8 slot
// is a small hook of this test that also logs EDX. Functions:
//  - zVideo_NotifyScreenResize (0x004a6770) / zVideo_NotifyScreenResizeFromSurface (0x004a6840): slot 0x3C4 (the
//    latter only when [0x00632120] or [0x00632128]), then zRndr_SetFramebuffer, SetGlobalRect_0056b1c4 and
//    SetRect_Object0056bd58 from the record's size / pitch / base, then slot 0x3C0 when [0x00632128];
//  - the tail jumps zVideo_CallSurfaceSlot3C0_Back / 3C8 / 3C0_Front / 3C4_Record240 / 3C0_Record240;
//  - zVideo_PresentFrame (0x004a6900; two stack words; ret 8): the present hook and the frame counter [0x0056bbd8]
//    unless [0x00632144] > 0.
//  - zVideo_CaptureSurfaceToImage (0x004a6e80; ECX record index 0..3): lock (slot 0x3C4 of record 0x00632240), a new
//    image (Image_Alloc, Image_SetSize) with an owned 2-byte-per-pixel buffer, the pixels copied in one block or row by
//    row by pitch, unlock (slot 0x3C0);
//  - zVideo_CopySurfaceRectToImage (0x004a6fe0; ECX record index, EDX rect, stack image or 0; ret 4): the rect clipped
//    to the record (zVideo_ClampValueReturnDelta), a new image when none is given, the rows copied.
//    For these two the records hold sizes up to 16 x 12 with a pixel block of this test; malloc is bound to a fake
//    that hands out test blocks.
//  - AnimImageWidget_CaptureFrame (0x004bfba0; ui_widgets; ECX widget, stack x, y; ret 8): with a frame image +0xbc,
//    zVideo_CopySurfaceRectToImage of the widget-sized rect at (x, y) from the record [+0xc4] into it, then the clip
//    image (slot 0x18: image and {0, 0, w, h}, or 0, 0 when the copy fails).
//  - ScreenHost_CloseModalKeep (0x00408fa0) / MainMenuScreen_OnActivate (0x00415370) (menus; ECX host, stack flag;
//    ret 4): with a screen [+4] (and, for the latter, a zero flag): NotifyScreenResizeFromSurface, the screen's slot 4(1),
//    WidgetContainer_RedrawChildren, slot 0(0), the front-surface slot, then PresentFrame with the setting
//    [[0x004e5d88]] twice.
// The surface records are filled from the seed. Compared: the call log, EAX, the globals the callees write.
#include "test.h"
#include "vt_runner.h"
#include "GameZRecoil/zVideo/zvid_buff.h"
#include "unattributed/ui_widgets.h"
#include "unattributed/menus.h"
#include "platform/iat_msvcrt.h"

using namespace vtr;

namespace {
using U = std::uint32_t;

extern "C" void __cdecl zvh_log_edx(U edx, U ecx)
{
    vt::log().push_back(0xED);
    vt::log().push_back(vt::norm()(edx));
    vt::log().push_back(vt::norm()(ecx));
}
__declspec(naked) void hook_edx()
{
    __asm {
        push ecx
        push edx
        call zvh_log_edx
        add esp, 8
        mov eax, 0x3c8
        ret
    }
}

void setup(World& w, std::mt19937& r)
{
    w.add(r, 4);
    const U* t = ch::at(vt::table());
    *ch::img(w.side, 0x006333c0) = t[0];
    *ch::img(w.side, 0x006333c4) = t[1];
    *ch::img(w.side, 0x006333b4) = t[2];
    *ch::img(w.side, 0x006333c8) = ch::addr(reinterpret_cast<void*>(&hook_edx));
    for (U va = 0x00632200; va < 0x00632260; va += 4) *ch::img(w.side, va) = r() % 4 ? r() % 1024 : r();
    *ch::img(w.side, 0x00632120) = r() % 2;
    *ch::img(w.side, 0x00632128) = r() % 2;
    *ch::img(w.side, 0x00632144) = static_cast<U>(static_cast<int>(r() % 5) - 2);
}
U log_globals(World& w, U eax)
{
    for (U va = 0x00632050; va < 0x00632080; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
    for (U va = 0x0056b1c4; va < 0x0056b1d4; va += 4) vt::log().push_back(*ch::img(w.side, va));
    for (U va = 0x0056bd58; va < 0x0056bd98; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
    vt::log().push_back(*ch::img(w.side, 0x0056bbd8));
    return eax;
}
void slots(std::mt19937& r)
{
    vt::set(0, 0, 0, 0, r() % 3);
    vt::set(4, 0, 0, 0, r() % 3);
    vt::set(8, 2, 2, 0, r() % 3);
}

World*& cur_world()
{
    static World* w = nullptr;
    return w;
}
void* __cdecl f_malloc(std::size_t n)
{
    std::mt19937 r(static_cast<U>(n));
    vt::log().push_back(0x3A11);
    vt::log().push_back(static_cast<U>(n));
    return ch::at(cur_world()->add(r, static_cast<U>(n) + 4));
}
struct BindMalloc {
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5dc));
    void* saved[2];
    BindMalloc() { saved[0] = *o; saved[1] = recoil::g_Iat_malloc_004cc5dc; *o = recoil::g_Iat_malloc_004cc5dc = reinterpret_cast<void*>(&f_malloc); }
    ~BindMalloc() { *o = saved[0]; recoil::g_Iat_malloc_004cc5dc = saved[1]; }
};
// the three surface records {width, height, pitch bytes, -, pixels} with small sizes and a pixel block each
void small_records(World& w, std::mt19937& r)
{
    for (U rec : {0x00632200u, 0x00632220u, 0x00632240u}) {
        const U wd = r() % 17, ht = r() % 13, pitch = 2 * wd + 2 * (r() % 3 == 0 ? r() % 4 : 0);
        *ch::img(w.side, rec) = wd;
        *ch::img(w.side, rec + 4) = ht;
        *ch::img(w.side, rec + 8) = pitch;
        *ch::img(w.side, rec + 0x10) = w.add(r, pitch * ht + 8);
    }
}

std::vector<Case> cases()
{
    struct F { const char* name; U va; void* port; int nstack; };
    const F fs[] = {
        {"zVideo_NotifyScreenResize", 0x004a6770, (void*)&recoil::zVideo_NotifyScreenResize, 0},
        {"zVideo_NotifyScreenResizeFromSurface", 0x004a6840, (void*)&recoil::zVideo_NotifyScreenResizeFromSurface, 0},
        {"zVideo_CallSurfaceSlot3C0_Back", 0x004a67d0, (void*)&recoil::zVideo_CallSurfaceSlot3C0_Back, 0},
        {"zVideo_CallSurfaceSlot3C8", 0x004a6830, (void*)&recoil::zVideo_CallSurfaceSlot3C8, 0},
        {"zVideo_CallSurfaceSlot3C0_Front", 0x004a68d0, (void*)&recoil::zVideo_CallSurfaceSlot3C0_Front, 0},
        {"zVideo_CallSurfaceSlot3C4_Record240", 0x004a68e0, (void*)&recoil::zVideo_CallSurfaceSlot3C4_Record240, 0},
        {"zVideo_CallSurfaceSlot3C0_Record240", 0x004a68f0, (void*)&recoil::zVideo_CallSurfaceSlot3C0_Record240, 0},
        {"zVideo_PresentFrame", 0x004a6900, (void*)&recoil::zVideo_PresentFrame, 2},
    };
    std::vector<Case> c;
    for (const F& f : fs) {
        const int ns = f.nstack;
        c.push_back({f.name, f.va, f.port, 200, slots, setup,
                     [ns](World& w, U fn, std::mt19937& r) {
                         U a[2] = {r(), r()};
                         return log_globals(w, call_any(fn, r() % 0x1000, r() % 0x1000, a, ns));
                     }});
    }
    c.push_back({"zVideo_CaptureSurfaceToImage", 0x004a6e80, (void*)&recoil::zVideo_CaptureSurfaceToImage, 400, slots,
                 [](World& w, std::mt19937& r) { setup(w, r); small_records(w, r); cur_world() = &w; },
                 [](World& w, U fn, std::mt19937& r) {
                     BindMalloc bm;
                     const U eax = call_any(fn, r() % 4, 0, nullptr, 0);
                     cur_world() = nullptr;
                     return eax;
                 }});
    c.push_back({"zVideo_CopySurfaceRectToImage", 0x004a6fe0, (void*)&recoil::zVideo_CopySurfaceRectToImage, 600, slots,
                 [](World& w, std::mt19937& r) {
                     setup(w, r);
                     small_records(w, r);
                     const U rect = w.add(r, 16);
                     for (U k = 0; k < 4; ++k) w.at(rect)[k] = static_cast<U>(static_cast<int>(r() % 24) - 4);
                     const U img = w.add(r, 0x38);           // a destination image big enough for any rect
                     // a valid 24 x 24 image record (Image_SetSize layout: +0 pixel count, +4 / +6 width / height shorts,
                     // +0x34 width): the copy steps rows by the image's own width, so random size fields sent it far
                     // past the 24 x 24 buffer on both sides (heap corruption 0xc0000374)
                     w.at(img)[0] = 24 * 24;
                     w.at(img)[1] = 24u | (24u << 16);
                     w.at(img)[0x34 / 4] = 24;
                     w.at(img)[4] = w.add(r, 2 * 24 * 24);
                     cur_world() = &w;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     BindMalloc bm;
                     const U n = static_cast<U>(w.blocks.size());
                     const bool own = r() % 2 != 0;
                     const U idx = r() % 4;
                     const U rect = ch::addr(w.blocks[n - 3].w.data());
                     // KG-42 (original defect): with no destination image the function allocates the CLIPPED size but
                     // steps rows by the UNCLIPPED rect width, overrunning its own buffer when the rect sticks out of the
                     // surface - a real heap overflow on the original side too. So the allocate path only gets rects
                     // inside the record; the destination-image path keeps every rect.
                     bool inside = idx < 3;
                     if (inside) {
                         const U rec = 0x00632200u + 0x20u * idx;
                         const int wd = static_cast<int>(*ch::img(w.side, rec)), ht = static_cast<int>(*ch::img(w.side, rec + 4));
                         const int* q = reinterpret_cast<const int*>(ch::at(rect));
                         inside = q[0] >= 0 && q[1] >= 0 && q[2] <= wd && q[3] <= ht;
                     }
                     U a[1] = {own || !inside ? ch::addr(w.blocks[n - 2].w.data()) : 0u};
                     const U eax = call_any(fn, idx, rect, a, 1);
                     cur_world() = nullptr;
                     return eax;
                 }});
    c.push_back({"AnimImageWidget_CaptureFrame", 0x004bfba0, (void*)&recoil::AnimImageWidget_CaptureFrame, 400,
                 [](std::mt19937& r) {
                     slots(r);
                     vt::set(0x18, 2);
                     vt::slots()[6].fn = [](U, const U* a) -> U {
                         vt::log().push_back(vt::norm()(a[0]));
                         for (int k = 0; a[1] && k < 4; ++k) vt::log().push_back(ch::at(a[1])[k]);
                         return 0;
                     };
                 },
                 [](World& w, std::mt19937& r) {
                     setup(w, r);
                     small_records(w, r);
                     const U wd = object(w, r, 0xd0);          // the widget
                     const U im = w.add(r, 0x40);              // its image (size)
                     reinterpret_cast<std::int16_t*>(w.at(im))[2] = static_cast<std::int16_t>(r() % 20);
                     reinterpret_cast<std::int16_t*>(w.at(im))[3] = static_cast<std::int16_t>(r() % 14);
                     w.at(wd)[0x3c / 4] = im;
                     const U frame = w.add(r, 0x38);           // the frame image with a big pixel block
                     w.at(frame)[4] = w.add(r, 2 * 24 * 24);
                     w.at(wd)[0xbc / 4] = r() % 5 ? frame : 0u;
                     w.at(wd)[0xc4 / 4] = r() % 4;
                     w.add(r, 4);
                     w.blocks.back().w[0] = wd;
                     cur_world() = &w;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     BindMalloc bm;
                     U a[2] = {static_cast<U>(static_cast<int>(r() % 24) - 4), static_cast<U>(static_cast<int>(r() % 18) - 4)};
                     const U eax = call_any(fn, w.blocks.back().w[0], 0, a, 2);
                     cur_world() = nullptr;
                     return eax;
                 }, false});
    struct M { const char* name; U va; void* port; };
    const M ms[] = {{"ScreenHost_CloseModalKeep", 0x00408fa0, (void*)&recoil::ScreenHost_CloseModalKeep},
                    {"MainMenuScreen_OnActivate", 0x00415370, (void*)&recoil::MainMenuScreen_OnActivate}};
    for (const M& m : ms)
        c.push_back({m.name, m.va, m.port, 200,
                     [](std::mt19937& r) { slots(r); vt::set(0x20, 0); },
                     [](World& w, std::mt19937& r) {
                         setup(w, r);
                         const U host = w.add(r, 0x10);
                         const U scr = object(w, r, 0x20);
                         U head = 0;
                         for (U k = 0, n = r() % 3; k < n; ++k) { const U ch_ = object(w, r, 0x20); w.at(ch_)[1] = head; head = ch_; }
                         w.at(scr)[2] = head;
                         w.at(host)[1] = r() % 5 ? scr : 0u;
                         const U v = w.add(r, 4);
                         w.at(v)[0] = r() % 5;
                         *ch::img(w.side, 0x004e5d88) = v;
                         w.add(r, 4);
                         w.blocks.back().w[0] = host;
                     },
                     [](World& w, U fn, std::mt19937& r) {
                         U a[1] = {r() % 3 ? 0u : 1u};
                         return log_globals(w, call_any(fn, w.blocks.back().w[0], 0, a, 1));
                     }, false});
    return c;
}
}  // namespace

TEST(native_zvideo_surface_hooks_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases(), "zvideo surface hooks"), 0);
}
