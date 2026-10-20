// Structured native L1s for the hud functions ported in the cloud that call virtual methods - on widgets behind
// pointers, embedded in objects, or living in .data (whose vtable word gets the fake table here) - through hooks in
// .data or KERNEL32, which the arena fuzz cannot drive. Run by tests/vt_runner.h: graphs from one seed, the call log,
// EAX, every block word and the globals the functions write compared. Slots used (thiscall unless noted): 0x4 show(1),
// 0xc move(2), 0x18 rect(2), 0x20 redraw(0), 0x24 update(1), 0x60 visible(1), 0x64 / 0x68 x / y(0), 0x74 printf
// (cdecl: widget, format, ...), 0x78 refresh(0), 0x84 progress(1).
#include "test.h"
#include "vt_runner.h"
#include "Battlesport/hud_hud.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"
#include "unattributed/hud.h"
#include "Battlesport/hud.h"

using namespace vtr;

namespace {
using U = std::uint32_t;
U self(World& w) { return ch::addr(w.blocks[0].w.data()); }
U call_self(World& w, U fn, std::initializer_list<U> args, U edx = 0)
{
    std::vector<U> a(args);
    return call_any(fn, self(w), edx, a.data(), static_cast<int>(a.size()));
}
// the fake table into widgets in .data
void in_image(World& w, std::initializer_list<U> vas)
{
    for (U va : vas) *ch::img(w.side, va) = vt::table();
}
void log_globals(World& w, U lo, U hi)
{
    for (U va = lo; va < hi; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
}
// widget words the SetImage family writes: owned flag +0x34, image +0x3c
void log_widget(World& w, U va)
{
    for (U off : {0xcu, 0x34u, 0x3cu}) vt::log().push_back(w.norm(*ch::img(w.side, va + off)));
}
// x / y slots answering by the widget (as its original VA) so both sides see the same numbers
void position_slots(std::mt19937& r)
{
    const U dx = r() % 300, dy = r() % 200;
    vt::set(0x64, 0);
    vt::slots()[0x19].fn = [dx](U self, const U*) -> U { return (vt::norm()(self) >> 2) % 97 + dx; };
    vt::set(0x68, 0);
    vt::slots()[0x1a].fn = [dy](U self, const U*) -> U { return (vt::norm()(self) >> 3) % 89 + dy; };
}
// a message panel: four embedded labels at +0x10 / +0x2b4 / +0x558 / +0x7fc with short NUL-terminated texts (+0x15c)
// (as tests/test_menus_virtual_native.cpp)
U panel(World& w, std::mt19937& r)
{
    static const char* const texts[] = {"hello", "world", "", "hello"};
    const U p = object(w, r, 0xab0, {0x10, 0x2b4, 0x558, 0x7fc});
    w.at(p)[0x1c / 4] = r() % 2 ? 0u : 0x10u;
    for (U base : {0x10u, 0x2b4u, 0x558u, 0x7fcu}) {
        std::strcpy(reinterpret_cast<char*>(w.at(p)) + base + 0x15c, texts[r() % 4]);
        w.at(p)[(base + 0xc) / 4] = r() % 2 ? 0u : 0x10u;
        w.at(p)[(base + 0x10) / 4] = fbits(static_cast<float>(r() % 40) / 10.0f);
        w.at(p)[(base + 0x270) / 4] = 0;
    }
    return p;
}
// an image record for Image_FreeUnlessDefault: the side's default image 0x004e06e0 now and then, else a 0x38-byte
// record with owned buffers +0x10 / +0x14 (flags +9) and a hook flag +0x30
U free_image(World& w, std::mt19937& r)
{
    const U k = r() % 5;
    if (k == 0) return 0;
    if (k == 1) return ch::addr(ch::img(w.side, 0x004e06e0));
    const U im = w.add(r, 0x38);
    reinterpret_cast<unsigned char*>(w.at(im))[9] = static_cast<unsigned char>(r());
    w.at(im)[4] = r() % 3 ? w.add(r, 8) : 0u;
    w.at(im)[5] = r() % 3 ? w.add(r, 8) : 0u;
    w.at(im)[0x30 / 4] = r() % 3 == 0 ? 1u : 0u;
    return im;
}
// a config list {0, count} {entries...} as the zReader trees hold them; entries {type, value}: 1 int, 3 string, 4 list
U cfg(World& w, std::mt19937& r, const std::vector<std::pair<U, U>>& entries, U count = 0)
{
    const U arr = w.add(r, 8 * (entries.size() + 1));
    U* a = w.at(arr);
    a[0] = 0;
    a[1] = count ? count : static_cast<U>(entries.size() + 1);
    for (std::size_t k = 0; k < entries.size(); ++k) { a[2 + 2 * k] = entries[k].first; a[3 + 2 * k] = entries[k].second; }
    return arr;
}
U str_block(World& w, std::mt19937& r, const char* s)
{
    const U b = w.add(r, 0x20);
    std::memset(w.at(b), 0, 0x20);
    std::memcpy(w.at(b), s, std::strlen(s) + 1);
    return b;
}
// the image readers: list A empty, list B one record with no TOC, so Image_Load finds nothing without touching a file
U* empty_record()
{
    static U rec[0xa4 / 4];
    return rec;
}
void no_images(World& w)
{
    *ch::img(w.side, 0x0053d768) = 0;
    *ch::img(w.side, 0x0053d770) = 1;
    std::memset(empty_record(), 0, 0xa4);
    *ch::img(w.side, 0x0053d774) = ch::addr(empty_record());
}
void __cdecl f_free(void* p) { vt::log().push_back(0xF4EE); vt::log().push_back(vt::norm()(ch::addr(p))); }
void sleep_log(U ms) { vt::log().push_back(0x51EE); vt::log().push_back(ms); }
void __stdcall f_sleep(U ms) { sleep_log(ms); }
U s_iob[24];
int __cdecl f_puts(const char* t)
{
    vt::log().push_back(0x9075);
    U h = 0;
    for (int k = 0; t && t[k] && k < 64; ++k) h = h * 31 + static_cast<unsigned char>(t[k]);
    vt::log().push_back(h);
    return 1;
}
int __cdecl f_fflush(U f) { vt::log().push_back(0xF5); vt::log().push_back(f - ch::addr(s_iob)); return 0; }

std::vector<Case> cases()
{
    std::vector<Case> c;
    // Loading_SetProgress (stack value; ret 4): the loader [0x004e5cb4]'s embedded widget +0xa960 slot 0x84, Sleep(100)
    c.push_back({"Loading_SetProgress", 0x00404c50, (void*)&recoil::Loading_SetProgress, 100, [](std::mt19937&) { vt::set(0x84, 1); },
                 [](World& w, std::mt19937& r) {
                     const U o = object(w, r, 0xa970, {0xa960});
                     *ch::img(w.side, 0x004e5cb4) = r() % 4 ? o : 0u;
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r() % 101}; return call_any(fn, 0, 0, a, 1); }, false});
    // HudPanel_ForwardUpdate (stack value; ret 4): slot 0x24 of [0x004ed4e0]
    c.push_back({"HudPanel_ForwardUpdate", 0x0040eae0, (void*)&recoil::HudPanel_ForwardUpdate, 100, [](std::mt19937& r) { vt::set(0x24, 1, 1, 0, r()); },
                 [](World& w, std::mt19937& r) { *ch::img(w.side, 0x004ed4e0) = object(w, r, 0x40); },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r()}; return call_any(fn, 0, 0, a, 1); }});
    // HudModeIcon_Layout: move (slot 0xc) to [+0xd8 / +0xdc] + the offsets [0x004e61f0 / f4], then the bounds +0xc8..
    // from the image +0xbc
    c.push_back({"HudModeIcon_Layout", 0x0040f130, (void*)&recoil::HudModeIcon_Layout, 200, [](std::mt19937&) { vt::set(0xc, 2); },
                 [](World& w, std::mt19937& r) {
                     const U o = object(w, r, 0xe0);
                     w.at(o)[0xbc / 4] = image(w, r);
                     *ch::img(w.side, 0x004e61f0) = r() % 100;
                     *ch::img(w.side, 0x004e61f4) = r() % 100;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    // HudModeIcon_Select (ECX icon 0..3, EDX state 0..3): state 2 first puts the previous icon [0x004ea65c] back to its
    // image +0xc0; then the icon's image for the state; the four icons (0x004ea660, stride 0xe0) are in .data
    c.push_back({"HudModeIcon_Select", 0x0040f1a0, (void*)&recoil::HudModeIcon_Select, 300, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     in_image(w, {0x004ea660, 0x004ea740, 0x004ea820, 0x004ea900});
                     *ch::img(w.side, 0x004ea65c) = r() % 4;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 4, r() % 4, nullptr, 0);
                     for (U k = 0; k < 4; ++k) log_widget(w, 0x004ea660 + 0xe0 * k);
                     log_globals(w, 0x004ea65c, 0x004ea660);
                     return eax;
                 }, false});
    // HudPips_SetCount (stack count; ret 4): clamped to 0..4 into +0x34 (unchanged -> nothing); the three pips at
    // +0x3c + 0xbc*k shown (slot 0x60) below the count; redraw (slot 0x20)
    c.push_back({"HudPips_SetCount", 0x0040f460, (void*)&recoil::HudPips_SetCount, 300, [](std::mt19937&) { vt::set(0x60, 1); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U o = object(w, r, 0x3c + 3 * 0xbc, {0x3c, 0x3c + 0xbc, 0x3c + 2 * 0xbc});
                     w.at(o)[0x34 / 4] = r() % 5;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {static_cast<U>(static_cast<int>(r() % 9) - 2)}); }, false});
    // Hud_LayoutFromFontHeight: the font height (slot 0x64 of the widget 0x004e657c) less two constants into four floats
    c.push_back({"Hud_LayoutFromFontHeight", 0x004118b0, (void*)&recoil::Hud_LayoutFromFontHeight, 100, position_slots,
                 [](World& w, std::mt19937& r) { w.add(r, 4); in_image(w, {0x004e657c}); },
                 [](World& w, U fn, std::mt19937&) {
                     const U eax = call_any(fn, 0, 0, nullptr, 0);
                     log_globals(w, 0x004e6730, 0x004e6758);
                     return eax;
                 }, false});
    // Hud_PrintToConsole (ECX text): the console [0x004e6a98] printf (slot 0x74, "%s"-style format 0x004dacbc) and refresh
    c.push_back({"Hud_PrintToConsole_004e6a98", 0x00412050, (void*)&recoil::Hud_PrintToConsole_004e6a98, 100, [](std::mt19937&) { vt::set(0x74, 0, 3); vt::set(0x78, 0); },
                 [](World& w, std::mt19937& r) { w.add(r, 0x10); *ch::img(w.side, 0x004e6a98) = object(w, r, 0x40); },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, self(w), 0, nullptr, 0); }, false});
    // Hud_HideIfOwner (ECX owner): the picked marker [0x004e6af0]'s target record [+0x38] owner [+4] == ECX -> hide the
    // marker widget 0x004e6c6c
    c.push_back({"Hud_HideIfOwner", 0x00412620, (void*)&recoil::Hud_HideIfOwner, 200, [](std::mt19937&) { vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     const U m = w.add(r, 0x40), t = w.add(r, 8);
                     w.at(m)[0x38 / 4] = t;
                     w.at(t)[1] = r() % 3;
                     *ch::img(w.side, 0x004e6af0) = r() % 5 ? m : 0u;
                     in_image(w, {0x004e6c6c});
                 },
                 [](World&, U fn, std::mt19937& r) { return call_any(fn, r() % 3, 0, nullptr, 0); }, false});
    // one-line virtual forwards: slot 4 with 1 / 0
    struct V { const char* name; U va; void* port; };
    const V vs[] = {{"VMethod_CallSlot1_True_00412bf0", 0x00412bf0, (void*)&recoil::VMethod_CallSlot1_True_00412bf0},
                    {"VMethod_CallSlot1_False_00412c00", 0x00412c00, (void*)&recoil::VMethod_CallSlot1_False_00412c00},
                    {"VMethod_CallSlot1_False_004135f0", 0x004135f0, (void*)&recoil::VMethod_CallSlot1_False_004135f0}};
    for (const V& v : vs)
        c.push_back({v.name, v.va, v.port, 50, [](std::mt19937& r) { vt::set(0x4, 1, 1, 0, r()); },
                     [](World& w, std::mt19937& r) { object(w, r, 0x40); },
                     [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }});
    // --- the popup: in-image widgets 0x004e6bb0 / 0x004e6638 / 0x004e657c / 0x004e6848, the texts behind 0x004e66f4 /
    // 0x004e6844, the state 0x004e6560..0x004e6578 and the lock [0x004e6984]
    auto popup = [](World& w, std::mt19937& r) {
        w.add(r, 0x40);                                   // block 0: the image argument (a header)
        reinterpret_cast<std::int16_t*>(w.blocks[0].w.data())[2] = static_cast<std::int16_t>(r() % 200);
        in_image(w, {0x004e6bb0, 0x004e6638, 0x004e657c, 0x004e6848});
        *ch::img(w.side, 0x004e66f4) = object(w, r, 0x40);
        *ch::img(w.side, 0x004e6844) = object(w, r, 0x40);
        *ch::img(w.side, 0x004e6984) = r() % 6 == 0 ? 1u : 0u;
        *ch::img(w.side, 0x004e6564) = r() % 4;
        *ch::img(w.side, 0x004e65b8) = r() % 3 ? image(w, r) : 0u;
        *ch::img(w.side, 0x004e656c) = fbits(static_cast<float>(r() % 100) / 10.0f);
        *ch::img(w.side, 0x004e6568) = fbits(static_cast<float>(r() % 100) / 10.0f);
        w.add(r, 0x10);                                   // the two texts (the last two blocks)
        w.add(r, 0x10);
    };
    auto popup_log = [](World& w, U eax) {
        log_globals(w, 0x004e6560, 0x004e657c);
        log_globals(w, 0x0057da2c, 0x0057da30);
        log_widget(w, 0x004e6638);
        return eax;
    };
    auto popup_slots = [](std::mt19937& r) { vt::set(0x74, 0, 2); vt::set(0x60, 1); vt::set(0x20, 0); position_slots(r); };
    // Hud_ShowPopup (ECX image, EDX text, stack second text, value; ret 8)
    c.push_back({"Hud_ShowPopup", 0x00411900, (void*)&recoil::Hud_ShowPopup, 400, popup_slots, popup,
                 [popup_log](World& w, U fn, std::mt19937& r) {
                     const U n = static_cast<U>(w.blocks.size());
                     const U t1 = r() % 6 ? ch::addr(w.blocks[n - 2].w.data()) : 0u, t2 = r() % 6 ? ch::addr(w.blocks[n - 1].w.data()) : 0u;
                     U a[2] = {t2, r()};
                     return popup_log(w, call_any(fn, self(w), t1, a, 2));
                 }});
    // Hud_ClosePopup: state 2 -> hides the texts and the image, state 3, timer 0; state 1 -> state 3 with the remaining time
    c.push_back({"Hud_ClosePopup", 0x00411a20, (void*)&recoil::Hud_ClosePopup, 300, popup_slots, popup,
                 [popup_log](World& w, U fn, std::mt19937&) { return popup_log(w, call_any(fn, 0, 0, nullptr, 0)); }, false});
    // Hud_PickTargetMarker (ECX mode 0..2, EDX out pairs): the nearest (by the x / y slots against [0x004e621c / 20])
    // free marker of [0x004e6dc8] (0x004e6dcc, stride 0x1c0, in .data) into [0x004e6af0]; mode 2 collects target
    // pairs; mode 1 within [0x004ea5cc] shows the marker image [0x004e6db4] on the marker's widget +0x104, centred
    c.push_back({"Hud_PickTargetMarker", 0x004122c0, (void*)&recoil::Hud_PickTargetMarker, 600,
                 [](std::mt19937& r) { position_slots(r); vt::set(0x20, 0); vt::set(0xc, 2); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 8 * 4);                     // block 0: the out pairs
                     const U n = r() % 5;
                     *ch::img(w.side, 0x004e6dc8) = n;
                     for (U k = 0; k < 4; ++k) {
                         const U m = 0x004e6dcc + 0x1c0 * k;
                         in_image(w, {m, m + 0x104});
                         *ch::img(w.side, m + 0x34) = r() % 4 == 0 ? 1u : 0u;
                         // record sizes with room: the function forms pointers past an 8-byte / 4-byte block (first
                         // diff was a raw address 0x80 past one), which the comparison cannot map to a role
                         const U t = w.add(r, 0x40), o = w.add(r, 0x100);
                         w.at(t)[0] = 1 + r() % 3;
                         w.at(t)[1] = o;
                         w.at(o)[1] = w.add(r, 0x200);
                         *ch::img(w.side, m + 0x38) = t;
                         *ch::img(w.side, m + 0x140) = image(w, r);
                     }
                     *ch::img(w.side, 0x004e621c) = r() % 300;
                     *ch::img(w.side, 0x004e6220) = r() % 200;
                     *ch::img(w.side, 0x004ea5cc) = r() % 20000;
                     *ch::img(w.side, 0x004e6db4) = image(w, r);
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 3, self(w), nullptr, 0);
                     log_globals(w, 0x004e6af0, 0x004e6af4);
                     for (U k = 0; k < 4; ++k) log_widget(w, 0x004e6dcc + 0x1c0 * k + 0x104);
                     return eax;
                 }});
    // the player slots (0x004ea9e8, stride 0x44c, in .data): widget at +0, second widget +0x390, text widget +0xe0
    auto slots_in_image = [](World& w, std::mt19937& r) {
        w.add(r, 4);
        for (U k = 0; k < 10; ++k) {
            const U s0 = 0x004ea9e8 + 0x44c * k;
            in_image(w, {s0, s0 + 0x390, s0 + 0xe0});
        }
    };
    auto slot_log = [](World& w, U k) {
        const U s0 = 0x004ea9e8 + 0x44c * k;
        log_widget(w, s0);
        log_widget(w, s0 + 0x390);
        for (U off : {0xd0u, 0xd4u, 0x384u}) vt::log().push_back(*ch::img(w.side, s0 + off));
    };
    c.push_back({"HudPlayerSlot_SetMode", 0x004126e0, (void*)&recoil::HudPlayerSlot_SetMode, 400, [](std::mt19937&) { vt::set(0x20, 0); },
                 slots_in_image,
                 [slot_log](World& w, U fn, std::mt19937& r) {
                     const U k = r() % 10;
                     const U eax = call_any(fn, k, r() % 7, nullptr, 0);
                     slot_log(w, k);
                     return eax;
                 }, false});
    c.push_back({"HudPlayerSlot_SelectEntry", 0x00412790, (void*)&recoil::HudPlayerSlot_SelectEntry, 200, [](std::mt19937&) { vt::set(0x20, 0); },
                 slots_in_image,
                 [slot_log](World& w, U fn, std::mt19937& r) {
                     const U k = r() % 10;
                     const U eax = call_any(fn, k, r() % 2, nullptr, 0);
                     slot_log(w, k);
                     return eax;
                 }, false});
    c.push_back({"HudPlayerSlot_Clear", 0x004127d0, (void*)&recoil::HudPlayerSlot_Clear, 100, [](std::mt19937&) { vt::set(0x20, 0); vt::set(0x74, 0, 2); },
                 slots_in_image,
                 [slot_log](World& w, U fn, std::mt19937& r) {
                     const U k = r() % 10;
                     const U eax = call_any(fn, k, 0, nullptr, 0);
                     slot_log(w, k);
                     return eax;
                 }, false});
    // Hud_ForwardTo_004bc900 (stack value; ret 4): Manager_BroadcastVirtual24 on ECX - slot 0x24 of each listed node
    // when the manager is active
    c.push_back({"Hud_ForwardTo_004bc900", 0x00412be0, (void*)&recoil::Hud_ForwardTo_004bc900, 200, [](std::mt19937&) { vt::set(0x24, 1); },
                 [](World& w, std::mt19937& r) {
                     const U m = w.add(r, 0x10);
                     w.at(m)[1] = r() % 4 ? 1u : 0u;
                     U head = 0;
                     for (U k = 0, n = r() % 4; k < n; ++k) {
                         const U node = object(w, r, 0x10);
                         w.at(node)[1] = head;
                         head = node;
                     }
                     w.at(m)[2] = head;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r()}); }, false});
    // Hud_SetOverlayVisible (ECX flag): slot 0x60 of [0x004ea658]; off -> CallGlobal004e5ee8Slot18
    c.push_back({"Hud_SetOverlayVisible", 0x00413770, (void*)&recoil::Hud_SetOverlayVisible, 100, [](std::mt19937&) { vt::set(0x60, 1); vt::set(0x18, 0); },
                 [](World& w, std::mt19937& r) {
                     *ch::img(w.side, 0x004ea658) = object(w, r, 0x40);
                     const U g = object(w, r, 0x40);
                     *ch::img(w.side, 0x004e5ee8) = r() % 4 ? g : 0u;
                 },
                 [](World&, U fn, std::mt19937& r) { return call_any(fn, r() % 2, 0, nullptr, 0); }, false});
    // Hud_ShowPanel_004e6db0 (ECX flag): slot 4 of [0x004e6db0] with 1 / 0
    c.push_back({"Hud_ShowPanel_004e6db0", 0x004137a0, (void*)&recoil::Hud_ShowPanel_004e6db0, 100, [](std::mt19937& r) { vt::set(0x4, 1, 1, 0, r()); },
                 [](World& w, std::mt19937& r) { *ch::img(w.side, 0x004e6db0) = object(w, r, 0x40); },
                 [](World&, U fn, std::mt19937& r) { return call_any(fn, r() % 3, 0, nullptr, 0); }});
    // Hud_SetMessageLine (ECX mode 0..3, EDX line 0..3, stack text; ret 4): the panel [0x004e6db0]'s labels at +0x20 +
    // 0x2a4*line: 1 -> text and show, 0 -> hide, 2 -> text and show unless empty (then hide)
    c.push_back({"Hud_SetMessageLine", 0x004137f0, (void*)&recoil::Hud_SetMessageLine, 300, [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     const U t = w.add(r, 0x10);
                     if (r() % 3 == 0) w.at(t)[0] = 0;
                     *ch::img(w.side, 0x004e6db0) = object(w, r, 0x20 + 4 * 0x2a4, {0x20, 0x20 + 0x2a4, 0x20 + 2 * 0x2a4, 0x20 + 3 * 0x2a4});
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {self(w)}; return call_any(fn, r() % 4, r() % 4, a, 1); }, false});
    // --- image releases: Image_FreeUnlessDefault on the fields, then zeroed; the image hook [0x0056bc28] a fake thunk
    struct R { const char* name; U va; void* port; std::vector<U> offs; };
    const R rs[] = {{"HudTriple_ReleaseImages", 0x0040f0f0, (void*)&recoil::HudTriple_ReleaseImages, {0xbc, 0xc0, 0xc4}},
                    {"HudImages_Release", 0x00413080, (void*)&recoil::HudImages_Release, {0x1ac, 0x1b0, 0x274, 0x278}},
                    {"HudScoreImages_Release", 0x00413ff0, (void*)&recoil::HudScoreImages_Release, {0xbc, 0xc0, 0xc4, 0xc8, 0xcc, 0xd8, 0xdc}}};
    for (const R& rr : rs) {
        const std::vector<U> offs = rr.offs;
        c.push_back({rr.name, rr.va, rr.port, 300, [](std::mt19937&) { vt::set(0x28, 0); },
                     [offs](World& w, std::mt19937& r) {
                         const U o = w.add(r, 0x280);
                         for (U off : offs) w.at(o)[off / 4] = free_image(w, r);
                         *ch::img(w.side, 0x0056bc28) = ch::at(vt::table())[10];
                     },
                     [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }});
    }
    // HudMessage_LayoutRect: the dirty rect of the widget +0x1b4 from the font widget 0x004e657c's position / size and
    // the image [0x004e65b8]; then the screen 0x004e62f0 (in .data) gets the global flags and notifies its children
    c.push_back({"HudMessage_LayoutRect", 0x004132b0, (void*)&recoil::HudMessage_LayoutRect, 300,
                 [](std::mt19937& r) { position_slots(r); vt::set(0x20, 0); vt::set(0x8, 0); vt::set(0x4, 0); },
                 [](World& w, std::mt19937& r) {
                     const U o = object(w, r, 0x1b4 + 0xc0, {0x1b4});
                     w.at(o)[(0x1b4 + 0x3c) / 4] = r() % 5 ? image(w, r) : 0u;
                     for (U k = 0; k < 4; ++k) w.at(o)[(0x1b4 + 0x4c + 0x1c * k) / 4] = r() % 3 ? 1u : 0u;
                     for (U off : {0x14u, 0x18u}) w.at(o)[(0x1b4 + off) / 4] = r() % 300;
                     in_image(w, {0x004e657c, 0x004e62f0, 0x004e62f0 + 0x1b4, 0x004e62f0 + 0xf8, 0x004e62f0 + 0x3c});
                     for (U off : {0x1c0u, 0x104u, 0x48u}) *ch::img(w.side, 0x004e62f0 + off) = r() % 64;
                     *ch::img(w.side, 0x004e65b8) = r() % 3 ? image(w, r) : 0u;
                     *ch::img(w.side, 0x004e4870) = r() % 2 ? 4u : 0xcu;
                     *ch::img(w.side, 0x004e6578) = r() % 300;
                 },
                 [](World& w, U fn, std::mt19937&) {
                     const U eax = call_self(w, fn, {});
                     log_globals(w, 0x004e62f0 + 0xc, 0x004e62f0 + 0x10);
                     return eax;
                 }, false});
    // Hud_DrawOverlay_00413500 (ECX manager, stack value; ret 4): the overlay hook [0x0056bbfc] (ECX 0x004e6aa0, EDX
    // 0x004e6ab0) when the HUD [0x004e5ed4], the setting [[0x004e5d6c]] and no popup [0x004e6564]; then the broadcast
    c.push_back({"Hud_DrawOverlay_00413500", 0x00413500, (void*)&recoil::Hud_DrawOverlay_00413500, 300,
                 [](std::mt19937&) { vt::set(0x2c, 0); vt::set(0x24, 1); },
                 [](World& w, std::mt19937& r) {
                     const U m = w.add(r, 0x10);
                     w.at(m)[1] = r() % 4 ? 1u : 0u;
                     U head = 0;
                     for (U k = 0, n = r() % 3; k < n; ++k) { const U node = object(w, r, 0x10); w.at(node)[1] = head; head = node; }
                     w.at(m)[2] = head;
                     const U v = w.add(r, 4);
                     w.at(v)[0] = r() % 3 ? 1u : 0u;
                     *ch::img(w.side, 0x004e5d6c) = v;
                     *ch::img(w.side, 0x004e5ed4) = r() % 4 ? 1u : 0u;
                     *ch::img(w.side, 0x004e6564) = r() % 3 == 0 ? 1u : 0u;
                     *ch::img(w.side, 0x0056bbfc) = ch::at(vt::table())[11];
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r()}); }, false});
    // Hud_ResetAllVisibility: WidgetContainer_SetChildFlags(0xe) on 0x004e5ed0 and on ECX, the hidden-bit masks of the
    // HUD widgets' flags (0x004e6588, 0x004e6708, [[0x004e66f8] + 0xc], 0x004e6bbc, the player-slot table entries with
    // an image), then slot 4(1) of ECX
    c.push_back({"Hud_ResetAllVisibility", 0x00413540, (void*)&recoil::Hud_ResetAllVisibility, 200, [](std::mt19937&) { vt::set(0x4, 1); },
                 [](World& w, std::mt19937& r) {
                     auto list = [&w, &r]() {
                         U head = 0;
                         for (U k = 0, n = r() % 4; k < n; ++k) { const U node = w.add(r, 0x10); w.at(node)[1] = head; head = node; }
                         return head;
                     };
                     const U o = object(w, r, 0x290);
                     w.at(o)[2] = list();
                     *ch::img(w.side, 0x004e5ed8) = list();
                     *ch::img(w.side, 0x004e66f8) = w.add(r, 0x10);
                     for (U va : {0x004e6588u, 0x004e6708u, 0x004e6bbcu}) *ch::img(w.side, va) = r();
                     for (U e = 0x004eb1d0; e < 0x004ed87c; e += 0x44c) { *ch::img(w.side, e) = r(); *ch::img(w.side, e + 0x30) = r() % 2; }
                 },
                 [](World& w, U fn, std::mt19937&) {
                     const U eax = call_self(w, fn, {});
                     for (U va : {0x004e6588u, 0x004e6708u, 0x004e6bbcu}) vt::log().push_back(*ch::img(w.side, va));
                     for (U e = 0x004eb1d0; e < 0x004ed87c; e += 0x44c) vt::log().push_back(*ch::img(w.side, e));
                     return eax;
                 }, false});
    // Hud_ForwardIfActive_0056bd20 (ECX text, stack float; ret 4): MessagePanel_PushLine on [0x0056bd20] when its +4
    c.push_back({"Hud_ForwardIfActive_0056bd20", 0x004138f0, (void*)&recoil::Hud_ForwardIfActive_0056bd20, 300,
                 [](std::mt19937&) { vt::set(0x4, 1); vt::set(0x60, 1); vt::set(0x74, 0, 2); vt::set(0x90, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = panel(w, r);
                     w.at(p)[1] = r() % 3 ? 1u : 0u;
                     *ch::img(w.side, 0x0056bd20) = p;
                 },
                 [](World&, U fn, std::mt19937& r) {
                     static const char* const q[] = {"hello", "world", "other"};
                     U a[1] = {fbits(static_cast<float>(r() % 50) / 10.0f)};
                     return call_any(fn, ch::addr(q[r() % 3]), 0, a, 1);
                 }});
    // Hud_HideBothPanels: MessagePanel_Clear and slot 4(0) on [0x0056bd24] then [0x0056bd20]
    c.push_back({"Hud_HideBothPanels", 0x00413950, (void*)&recoil::Hud_HideBothPanels, 100,
                 [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x60, 1); vt::set(0x4, 1); },
                 [](World& w, std::mt19937& r) {
                     *ch::img(w.side, 0x0056bd24) = panel(w, r);
                     *ch::img(w.side, 0x0056bd20) = panel(w, r);
                 },
                 [](World&, U fn, std::mt19937&) { return call_any(fn, 0, 0, nullptr, 0); }, false});
    // ConfigValue_PlaceTextWidget (ECX config value, EDX widget, stack dx, dy, offset or 0; ret 0xc): type 4 only -
    // move (slot 0xc) to the record's +0x14 / +0x1c plus the deltas and offset, then the text +0xc (or 0x004e5ce0)
    c.push_back({"ConfigValue_PlaceTextWidget", 0x00413990, (void*)&recoil::ConfigValue_PlaceTextWidget, 300,
                 [](std::mt19937&) { vt::set(0xc, 2); vt::set(0x74, 0, 2); },
                 [](World& w, std::mt19937& r) {
                     const U cfg = w.add(r, 8), rec = w.add(r, 0x28);
                     w.at(cfg)[0] = r() % 5 ? 4u : 3u;
                     w.at(cfg)[1] = rec;
                     w.at(rec)[3] = r() % 3 ? w.add(r, 0x10) : 0u;
                     object(w, r, 0x40);                  // the widget (block 3 or 4)
                     w.add(r, 8);                         // the offset (last block)
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U n = static_cast<U>(w.blocks.size());
                     U a[3] = {r() % 50, r() % 50, r() % 3 ? ch::addr(w.blocks[n - 1].w.data()) : 0u};
                     return call_any(fn, self(w), ch::addr(w.blocks[n - 2].w.data()), a, 3);
                 }});
    // ConfigValue_PlaceImage (ECX config value, EDX widget, stack dx, dy, offset or 0, image, out rect or 0; ret 0x14):
    // type 4 only; record type 6 compares +0x24 / +0x2c with "TRUE" (centre flags); the image given (a null image would
    // load one by name through the asset readers - not driven here) via ImageWidget_SetImageNoOwn; move (slot 0xc),
    // the centring flag word +0x40, redraw (slot 0x20), the out rect
    c.push_back({"ConfigValue_PlaceImage", 0x00413d30, (void*)&recoil::ConfigValue_PlaceImage, 400,
                 [](std::mt19937&) { vt::set(0xc, 2); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U cfg = w.add(r, 8), rec = w.add(r, 0x30);
                     w.at(cfg)[0] = r() % 6 ? 4u : 2u;
                     w.at(cfg)[1] = rec;
                     w.at(rec)[1] = r() % 3 ? 6u : 5u;
                     for (U off : {0x24u, 0x2cu}) {
                         const U t = w.add(r, 8);
                         std::memcpy(w.at(t), r() % 2 ? "TRUE" : "FALSE", 6);
                         w.at(rec)[off / 4] = t;
                     }
                     object(w, r, 0x50);                  // the widget
                     w.add(r, 8);                         // the offset
                     image(w, r);                         // the image
                     w.add(r, 0x10);                      // the out rect (last block)
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U n = static_cast<U>(w.blocks.size());
                     U a[5] = {r() % 50, r() % 50, r() % 3 ? ch::addr(w.blocks[n - 3].w.data()) : 0u, ch::addr(w.blocks[n - 2].w.data()),
                               r() % 3 ? ch::addr(w.blocks[n - 1].w.data()) : 0u};
                     return call_any(fn, self(w), ch::addr(w.blocks[n - 4].w.data()), a, 5);
                 }});
    // HudStatusBox_Layout: from the font widget 0x004ed8cc's x / y and the box images (+0xbc frame, +0xd8 icon): moves
    // (slot 0xc) and rects (slot 0x18: 0, rect - the rect's four words logged) of the box, its text +0xe0 and its icon
    // widget +0x390
    c.push_back({"HudStatusBox_Layout", 0x00414070, (void*)&recoil::HudStatusBox_Layout, 300,
                 [](std::mt19937& r) {
                     position_slots(r);
                     vt::set(0xc, 2);
                     vt::set(0x18, 2);
                     vt::slots()[6].fn = [](U, const U* a) -> U { for (int k = 0; k < 4; ++k) vt::log().push_back(ch::at(a[1])[k]); return 0; };
                 },
                 [](World& w, std::mt19937& r) {
                     const U o = object(w, r, 0x3a0, {0xe0, 0x390});
                     w.at(o)[0xbc / 4] = image(w, r);
                     w.at(o)[0xd8 / 4] = image(w, r);
                     w.at(o)[0x388 / 4] = r() % 400;
                     w.at(o)[0x38c / 4] = r() % 300;
                     in_image(w, {0x004ed8cc});
                     *ch::img(w.side, 0x004e61f0) = r() % 100;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    // Hud_ShowKillMessage (ECX victim, EDX killer or 0, stack other; ret 4): the killer's name [+0x160] (or message
    // 0x253 via Message_GetText), sprintf with the format 0x004dae34 and the two names at +0x24, then Hud_ShowMessage
    // (2.0) on the panel [0x0056bd24]; FormatMessageA / LocalFree bound, messages from ntdll
    c.push_back({"Hud_ShowKillMessage", 0x00414330, (void*)&recoil::Hud_ShowKillMessage, 200,
                 [](std::mt19937&) { vt::set(0x4, 1); vt::set(0x60, 1); vt::set(0x74, 0, 2); vt::set(0x90, 0); },
                 [](World& w, std::mt19937& r) {
                     static const char* const names[] = {"Ace", "rook", "", "a_longer_name_here"};
                     auto who = [&w, &r]() {
                         const U o = w.add(r, 0x170);
                         const char* nm = names[r() % 4];
                         std::memcpy(reinterpret_cast<char*>(w.at(o)) + 0x24, nm, std::strlen(nm) + 1);
                         return o;
                     };
                     who();                               // block 0: victim
                     const U k = who(), other = who();
                     const U nm = w.add(r, 0x10);
                     std::memcpy(w.at(nm), "Killer", 7);
                     w.at(k)[0x160 / 4] = r() % 3 ? nm : 0u;
                     w.blocks[0].w[0] = r() % 4 ? k : 0u;  // the killer argument, kept in the victim's first word
                     w.blocks[0].w[1] = other;
                     const U p = panel(w, r);
                     w.at(p)[1] = r() % 4 ? 1u : 0u;
                     *ch::img(w.side, 0x0056bd24) = p;
                     *ch::img(w.side, 0x0056b670) = ch::addr(GetModuleHandleA("ntdll.dll"));
                 },
                 [](World& w, U fn, std::mt19937&) {
                     U a[1] = {w.blocks[0].w[1]};
                     return call_any(fn, self(w), w.blocks[0].w[0], a, 1);
                 }, false});
    // Array50_CopyRange (stack first, last, dest; ret 0xc): 0x50-byte records [first, last) to dest (null dest skips
    // the copies but advances); Array50_FillCopy (stack dest, count, value; ret 0xc): count copies of *value
    c.push_back({"Array50_CopyRange", 0x004146a0, (void*)&recoil::Array50_CopyRange, 200, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) { w.add(r, 0x50 * 6); w.add(r, 0x50 * 6); },
                 [](World& w, U fn, std::mt19937& r) {
                     const U src = ch::addr(w.blocks[0].w.data()), n = r() % 7, lo = r() % (7 - n > 0 ? 7 - n : 1);
                     U a[3] = {src + 0x50 * lo, src + 0x50 * (lo + n) , r() % 6 ? ch::addr(w.blocks[1].w.data()) : 0u};
                     if (a[1] > src + 0x50 * 6) a[1] = src + 0x50 * 6;
                     return call_any(fn, r(), r(), a, 3);
                 }});
    c.push_back({"Array50_FillCopy", 0x004146e0, (void*)&recoil::Array50_FillCopy, 200, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) { w.add(r, 0x50 * 6); w.add(r, 0x50); },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[3] = {r() % 6 ? ch::addr(w.blocks[0].w.data()) : 0u, r() % 7, ch::addr(w.blocks[1].w.data())};
                     return call_any(fn, r(), r(), a, 3);
                 }});
    // ByteSet_Intersects (ECX set, EDX set; [0x004dd90c] enables): sets are a count byte then members, 0xff ends early;
    // 1 when a member of the first is found in the second before either's 0xff, or when the gate / an empty set says so
    c.push_back({"ByteSet_Intersects", 0x00476370, (void*)&recoil::ByteSet_Intersects, 600, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     for (int k = 0; k < 2; ++k) {
                         const U b = w.add(r, 0x20);
                         unsigned char* p = reinterpret_cast<unsigned char*>(w.at(b));
                         p[0] = static_cast<unsigned char>(r() % 8);
                         for (int j = 1; j < 0x20; ++j) p[j] = static_cast<unsigned char>(r() % 6 == 0 ? 0xff : r() % 12);
                     }
                     *ch::img(w.side, 0x004dd90c) = r() % 5 ? 1u : 0u;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, self(w), ch::addr(w.blocks[1].w.data()), nullptr, 0); }});
    // Hud_ForwardToGlobal_004e62f0 (ECX count): HudPips_SetCount on the pips object 0x004e62f0 in .data
    c.push_back({"Hud_ForwardToGlobal_004e62f0", 0x00411750, (void*)&recoil::Hud_ForwardToGlobal_004e62f0, 200,
                 [](std::mt19937&) { vt::set(0x60, 1); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     in_image(w, {0x004e62f0, 0x004e62f0 + 0x3c, 0x004e62f0 + 0xf8, 0x004e62f0 + 0x1b4});
                     *ch::img(w.side, 0x004e62f0 + 0x34) = r() % 5;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, static_cast<U>(static_cast<int>(r() % 9) - 2), 0, nullptr, 0);
                     log_globals(w, 0x004e62f0 + 0x34, 0x004e62f0 + 0x38);
                     return eax;
                 }, false});
    // Hud_ResetKeyBindings: the 23 lines of the panel [0x004e6db0] set to the empty text (mode 2) then hidden (mode 0)
    c.push_back({"Hud_ResetKeyBindings", 0x004137c0, (void*)&recoil::Hud_ResetKeyBindings, 30, [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     const U o = w.add(r, 0x20 + 23 * 0x2a4);
                     for (U k = 0; k < 23; ++k) w.at(o)[(0x20 + 0x2a4 * k) / 4] = vt::table();
                     *ch::img(w.side, 0x004e6db0) = o;
                 },
                 [](World&, U fn, std::mt19937&) { return call_any(fn, 0, 0, nullptr, 0); }, false});
    // Loading_Checkpoint (ECX text or 0): the next checkpoint fraction of the table 0x004ed560 (index [0x004ed5c8], last
    // [0x004ed5c4]; past it the no-op reporter) into [0x004ed5d0]; a text is puts'd and stderr flushed (fakes over
    // the msvcrt slots, _iob a static array of this test); Stub_Ret; Loading_SetProgress with the fraction
    c.push_back({"Loading_Checkpoint", 0x00414180, (void*)&recoil::Loading_Checkpoint, 200, [](std::mt19937&) { vt::set(0x84, 1); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 0x20);
                     std::memcpy(w.at(ch::addr(w.blocks[0].w.data())), "Loading level", 14);
                     const U o = object(w, r, 0xa970, {0xa960});
                     *ch::img(w.side, 0x004e5cb4) = r() % 4 ? o : 0u;
                     *ch::img(w.side, 0x004ed5c4) = 0x12;
                     *ch::img(w.side, 0x004ed5c8) = r() % 0x16;
                     for (U k = 0; k < 0x19; ++k) *ch::img(w.side, 0x004ed560 + 4 * k) = fbits(static_cast<float>(r() % 1000) / 1000.0f);
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 4 ? self(w) : 0u, 0, nullptr, 0);
                     log_globals(w, 0x004ed5c8, 0x004ed5cc);
                     log_globals(w, 0x004ed5d0, 0x004ed5d4);
                     return eax;
                 }, false});
    // HudRect_LoadFromConfig (ECX this, stack config node; ret 4): the child named by 0x004dae0c (ConfigTree_FindChild)
    // -> ConfigValue_GetRectOffset of its first entry into +0x10.. and copied to +0x20..; missing -> untouched
    c.push_back({"HudRect_LoadFromConfig", 0x00412c10, (void*)&recoil::HudRect_LoadFromConfig, 300, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     w.add(r, 0x30);                          // block 0: this
                     const U rect = cfg(w, r, {{1, r() % 300}, {1, r() % 300}, {1, r() % 300}, {1, r() % 300}});
                     const U inner = cfg(w, r, {{r() % 5 ? 4u : 1u, rect}});
                     const char* key = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(0x004dae0c));
                     const U name = str_block(w, r, r() % 4 ? key : "other");
                     const U root = cfg(w, r, {{3, str_block(w, r, "hud")}, {3, name}, {4, inner}});
                     const U node = w.add(r, 8);
                     w.at(node)[0] = 4; w.at(node)[1] = root;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {ch::addr(w.blocks.back().w.data())}); }});
    // HudTriple_LayoutFromConfig (ECX this, stack config value; ret 4): the rect offset of entry 1, then
    // ConfigValue_PlaceImage of entries 2..4 on the label widgets 0x004e632c / 0x004e63e8 / 0x004e64a4 (in .data) with no
    // image given - so ImageWidget_SetImage by name, which finds nothing here (no image archives: Image_Load returns 0)
    // - then the text widget +0x1b4's position (slots 0x68 / 0x64), move (slot 0xc) and rect (slot 0x18) of this
    c.push_back({"HudTriple_LayoutFromConfig", 0x0040f2e0, (void*)&recoil::HudTriple_LayoutFromConfig, 200,
                 [](std::mt19937& r) {
                     position_slots(r);
                     vt::set(0xc, 2);
                     vt::set(0x20, 0);
                     vt::set(0x18, 2);
                     vt::slots()[6].fn = [](U, const U* a) -> U { for (int k = 0; k < 4; ++k) vt::log().push_back(ch::at(a[1])[k]); return 0; };
                 },
                 [](World& w, std::mt19937& r) {
                     object(w, r, 0x1b4 + 0xc0, {0x1b4});     // block 0: this
                     no_images(w);
                     in_image(w, {0x004ed8cc, 0x004e632c, 0x004e63e8, 0x004e64a4});
                     for (U va : {0x004e632cu, 0x004e63e8u, 0x004e64a4u}) *ch::img(w.side, va + 0x34) = 0;
                     *ch::img(w.side, 0x004e61f0) = r() % 100;
                     const U rect = cfg(w, r, {{1, r() % 300}, {1, r() % 300}, {1, r() % 300}, {1, r() % 300}});
                     std::vector<std::pair<U, U>> top = {{4, rect}};
                     for (int k = 0; k < 3; ++k) {
                         const U rec = cfg(w, r, {{3, str_block(w, r, "icon")}, {1, r() % 300}, {1, r() % 300}, {3, str_block(w, r, "TRUE")},
                                                  {3, str_block(w, r, r() % 2 ? "TRUE" : "FALSE")}}, r() % 2 ? 6u : 5u);
                         top.push_back({4, rec});
                     }
                     const U list = cfg(w, r, top);
                     const U v = w.add(r, 8);
                     w.at(v)[0] = 4; w.at(v)[1] = list;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {ch::addr(w.blocks.back().w.data())}); }, false});
    // Inset_Enable (menus): slot 4(1) of the container 0x004e5ed0 (in .data), slot 0x10 of the HUD [0x004e5ee8],
    // Hud_ShowStandardWidgets, RenderInset_SetDestinationRect(0x004e6ad0), the inset pass flag [0x0057da2c] = 1;
    // returns the old [0x004e5ed4]
    c.push_back({"Inset_Enable", 0x00410e90, (void*)&recoil::Inset_Enable, 200, [](std::mt19937&) { vt::set(0x4, 1); vt::set(0x10, 0); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     in_image(w, {0x004e5ed0, 0x004e657c, 0x004e6848, 0x004e6638});
                     *ch::img(w.side, 0x004e5ee8) = object(w, r, 0x40);
                     *ch::img(w.side, 0x004e66f4) = object(w, r, 0x40);
                     *ch::img(w.side, 0x004e6844) = object(w, r, 0x40);
                     *ch::img(w.side, 0x004e6564) = r() % 3;
                     *ch::img(w.side, 0x004e5ed4) = r();
                     for (U k = 0; k < 4; ++k) *ch::img(w.side, 0x004e6ad0 + 4 * k) = fbits(static_cast<float>(r() % 640));
                 },
                 [](World& w, U fn, std::mt19937&) {
                     const U eax = call_any(fn, 0, 0, nullptr, 0);
                     log_globals(w, 0x0057da2c, 0x0057da30);
                     log_globals(w, 0x0057628c, 0x005762a4);
                     log_globals(w, 0x00576254, 0x00576258);
                     return eax;
                 }});
    return c;
}
}  // namespace

TEST(native_hud_virtual_calls_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc0b0));
    void* const saved[2] = {*o, recoil::g_Iat_Sleep_004cc0b0};
    *o = recoil::g_Iat_Sleep_004cc0b0 = reinterpret_cast<void*>(&f_sleep);
    static void* iob = s_iob;
    const U crt_va[3] = {0x004cc4f4, 0x004cc4fc, 0x004cc4f8};
    void** crt_port[3] = {&recoil::g_Iat_puts_004cc4f4, &recoil::g_Iat_fflush_004cc4fc, &recoil::g_Iat__iob_004cc4f8};
    void* const crt_fake[3] = {reinterpret_cast<void*>(&f_puts), reinterpret_cast<void*>(&f_fflush), iob};
    void* crt_saved[6];
    for (int i = 0; i < 3; ++i) {
        void** q = reinterpret_cast<void**>(static_cast<std::uintptr_t>(crt_va[i]));
        crt_saved[i] = *q; crt_saved[3 + i] = *crt_port[i];
        *q = *crt_port[i] = crt_fake[i];
    }
    const U fm_va[2] = {0x004cc0f4, 0x004cc0fc};
    void** fm_port[2] = {&recoil::g_Iat_FormatMessageA_004cc0f4, &recoil::g_Iat_LocalFree_004cc0fc};
    void* fm_saved[2];
    for (int i = 0; i < 2; ++i) { void** q = reinterpret_cast<void**>(static_cast<std::uintptr_t>(fm_va[i])); fm_saved[i] = *q; *q = *fm_port[i]; }
    void** of = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* const saved_free[2] = {*of, recoil::g_Iat_free_004cc5b4};
    *of = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&f_free);
    CHECK_EQ(run_cases(cases(), "hud virtual calls"), 0);
    *o = saved[0];
    recoil::g_Iat_Sleep_004cc0b0 = saved[1];
    for (int i = 0; i < 2; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(fm_va[i])) = fm_saved[i];
    for (int i = 0; i < 3; ++i) { *reinterpret_cast<void**>(static_cast<std::uintptr_t>(crt_va[i])) = crt_saved[i]; *crt_port[i] = crt_saved[3 + i]; }
    *of = saved_free[0];
    recoil::g_Iat_free_004cc5b4 = saved_free[1];
}
