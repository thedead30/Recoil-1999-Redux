// Structured native L1s for the menus / briefing / hud functions that call virtual methods, drawing hooks or KERNEL32
// (P5 menus, ported in the cloud). The virtual-call cases run on tests/vt_runner.h (fake table, graphs from one seed,
// call log + EAX + every block word compared); the drawing hooks [0x006320f0] / [0x006320f8] are pointed at fake-table
// thunks so their calls are logged the same way; the KERNEL32 cases bind the real functions into both import slots.
#include "test.h"
#include "vt_runner.h"
#include "Battlesport/Briefing.h"
#include "Battlesport/hud.h"
#include "platform/iat_kernel32.h"
#include "unattributed/hud.h"
#include "unattributed/menus.h"
#include "unattributed/ui_widgets.h"

using namespace vtr;

namespace {
using U = std::uint32_t;
U self(World& w) { return ch::addr(w.blocks[0].w.data()); }
U call_self(World& w, U fn, std::initializer_list<U> args)
{
    std::vector<U> a(args);
    return call_any(fn, self(w), 0, a.data(), static_cast<int>(a.size()));
}
// a message panel: four embedded labels at +0x10 / +0x2b4 / +0x558 / +0x7fc with short NUL-terminated texts (+0x15c)
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

std::vector<Case> cases()
{
    std::vector<Case> c;
    // briefing steps: ECX step, stack dt (ret 4); the step's widget at +4 / +8 / +0x104
    c.push_back({"BriefStep_Show_Run", 0x004046b0, (void*)&recoil::BriefStep_Show_Run, 100, [](std::mt19937&) { vt::set(0x60, 1); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) { const U p = w.add(r, 0x10); w.at(p)[1] = object(w, r, 0xc0); },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r()}); }});
    c.push_back({"BriefStep_FadeIn_Run", 0x00404740, (void*)&recoil::BriefStep_FadeIn_Run, 300, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = w.add(r, 0x10);
                     w.at(p)[1] = object(w, r, 0xc0);
                     w.at(p)[2] = fbits(static_cast<float>(r() % 300) / 100.0f - 0.5f);
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r()}); }});
    c.push_back({"BriefStep_SetText_Run", 0x00404850, (void*)&recoil::BriefStep_SetText_Run, 100,
                 [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x78, 0); vt::set(0x60, 1); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) { const U p = w.add(r, 0x108); w.at(p)[0x104 / 4] = object(w, r, 0xc0); },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r()}); }});
    c.push_back({"BriefStep_FadeOut_Run", 0x00404960, (void*)&recoil::BriefStep_FadeOut_Run, 300,
                 [](std::mt19937&) { vt::set(0x8, 0); vt::set(0x74, 0); vt::set(0x60, 1); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = w.add(r, 0x10);
                     w.at(p)[1] = r() % 3 ? image(w, r) : 0u;
                     w.at(p)[2] = object(w, r, 0xc0);
                     w.at(p)[3] = fbits(static_cast<float>(r() % 300) / 100.0f - 0.5f);
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r()}); }});
    // HudLabel_SetTriple (three values; ret 0xc): variadic slot 0x74 (the label, the format, ...) then slot 0x78
    c.push_back({"HudLabel_SetTriple", 0x0040ef00, (void*)&recoil::HudLabel_SetTriple, 300, [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x78, 0); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x40); },
                 [](World& w, U fn, std::mt19937& r) {
                     auto v = [&] { return static_cast<U>(static_cast<int>(r() % 20) - 2); };
                     return call_self(w, fn, {v(), v(), v()});
                 },
                 false});
    // Hud_ShowStandardWidgets: the HUD's widgets in .data (0x004e657c, 0x004e6848, 0x004e6638) and behind pointers
    // (0x004e66f4, 0x004e6844), gated on [0x004e6564]; the in-image ones get the fake table here
    c.push_back({"Hud_ShowStandardWidgets", 0x00411eb0, (void*)&recoil::Hud_ShowStandardWidgets, 100, [](std::mt19937&) { vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     for (U va : {0x004e657cu, 0x004e6848u, 0x004e6638u}) *ch::img(w.side, va) = vt::table();
                     *ch::img(w.side, 0x004e66f4) = object(w, r, 0x40);
                     *ch::img(w.side, 0x004e6844) = object(w, r, 0x40);
                     *ch::img(w.side, 0x004e6564) = r() % 3;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, 0, 0, nullptr, 0); }, false});
    // Hud_ClearMessagePanels: MessagePanel_Clear and slot 4 (one argument) on the panels [0x0056bd24] / [0x0056bd20]
    c.push_back({"Hud_ClearMessagePanels", 0x00413910, (void*)&recoil::Hud_ClearMessagePanels, 100,
                 [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x60, 1); vt::set(0x4, 1); },
                 [](World& w, std::mt19937& r) {
                     const U a = panel(w, r), b = panel(w, r);
                     *ch::img(w.side, 0x0056bd24) = a;
                     *ch::img(w.side, 0x0056bd20) = b;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, 0, 0, nullptr, 0); }, false});
    // Hud_ShowMessage (ECX text, stack float; ret 4): MessagePanel_PushLine on [0x0056bd24] when its +4 is set
    c.push_back({"Hud_ShowMessage", 0x004138d0, (void*)&recoil::Hud_ShowMessage, 300,
                 [](std::mt19937&) { vt::set(0x4, 1); vt::set(0x60, 1); vt::set(0x74, 0, 2); vt::set(0x90, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = panel(w, r);
                     w.at(p)[1] = r() % 3 ? 1u : 0u;
                     *ch::img(w.side, 0x0056bd24) = p;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     static const char* const q[] = {"hello", "world", "other"};
                     U a[1] = {fbits(static_cast<float>(r() % 50) / 10.0f)};
                     return call_any(fn, ch::addr(q[r() % 3]), 0, a, 1);
                 }});
    // the net-exit host [0x004f32c0]: slot 4 (1), slot 0 with [0x0056b424] (1), slot 8 (1) and the global cleared
    auto host = [](World& w, std::mt19937& r) {
        const U o = object(w, r, 0x40);
        *ch::img(w.side, 0x004f32c0) = r() % 5 ? o : 0u;
        *ch::img(w.side, 0x0056b424) = r() % 100;
    };
    auto host_call = [](World& w, U fn, std::mt19937&) {
        const U r = call_any(fn, 0, 0, nullptr, 0);
        vt::log().push_back(w.norm(*ch::img(w.side, 0x004f32c0)));
        return r;
    };
    c.push_back({"NetExitHost_Show", 0x0041c070, (void*)&recoil::NetExitHost_Show, 50, [](std::mt19937&) { vt::set(0x4, 1); },
                 [host](World& w, std::mt19937& r) { host(w, r); *ch::img(w.side, 0x004f32c0) = self(w); }, host_call, false});
    c.push_back({"NetExitHost_Call0", 0x0041c080, (void*)&recoil::NetExitHost_Call0, 50, [](std::mt19937&) { vt::set(0x0, 1); },
                 [host](World& w, std::mt19937& r) { host(w, r); *ch::img(w.side, 0x004f32c0) = self(w); }, host_call});
    c.push_back({"NetExitHost_Release", 0x0041c0a0, (void*)&recoil::NetExitHost_Release, 100, [](std::mt19937&) { vt::set(0x8, 1); }, host, host_call, false});
    // CallGlobal004e5ee8Slot18: tail jump to slot 0x18 of [0x004e5ee8] (none: returns)
    c.push_back({"CallGlobal004e5ee8Slot18", 0x00413630, (void*)&recoil::CallGlobal004e5ee8Slot18, 50, [](std::mt19937&) { vt::set(0x18, 0); },
                 [](World& w, std::mt19937& r) { const U o = object(w, r, 0x40); *ch::img(w.side, 0x004e5ee8) = r() % 4 ? o : 0u; },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, 0, 0, nullptr, 0); }, false});
    // NewGamePanel_SyncIntensity: ScrollGroup_Select(intensity) on the group embedded at +0xaf58
    c.push_back({"NewGamePanel_SyncIntensity", 0x0041c4e0, (void*)&recoil::NewGamePanel_SyncIntensity, 200, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0xaf58 + 0x180);
                     for (U k = 0; k < 10; ++k) {
                         if (r() % 3 == 0) { w.at(p)[(0xaf58 + 0x150) / 4 + k] = 0; continue; }
                         const U ch_ = object(w, r, 0x180);
                         w.at(ch_)[0xc4 / 4] = r() % 3 ? 1u : 0u;
                         for (U off : {0x16cu, 0x170u, 0x174u, 0x178u}) w.at(ch_)[off / 4] = r() % 3 ? image(w, r) : 0u;
                         w.at(p)[(0xaf58 + 0x150) / 4 + k] = ch_;
                     }
                     const U level = w.add(r, 4);
                     w.at(level)[0] = r() % 4;
                     *ch::img(w.side, 0x004e5d48) = level;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    // SaveLoadDialog_SelectIndex (ret 4): the save records (+0xca00 / +0xca04, 0x140 bytes, name at +0x2c) fill the
    // three rows at +0xb1f0 and the six at +0xb9f4 (stride 0x2ac) around the index, and the name field +0xae7c
    // (EditField_SetText) gets the selected record's name
    c.push_back({"SaveLoadDialog_SelectIndex", 0x004353f0, (void*)&recoil::SaveLoadDialog_SelectIndex, 300,
                 [](std::mt19937&) { vt::set(0x74, 0, 3); vt::set(0x60, 1); vt::set(0x20, 0); vt::set(0x8c, 1); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0xca10);
                     for (U k = 0; k < 3; ++k) w.at(p)[(0xb1f0 + 0x2ac * k) / 4] = vt::table();
                     for (U k = 0; k < 6; ++k) w.at(p)[(0xb9f4 + 0x2ac * k) / 4] = vt::table();
                     w.at(p)[0xae7c / 4] = vt::table();
                     const U buf = w.add(r, 0x48);
                     reinterpret_cast<char*>(w.at(buf))[0] = 0;
                     w.at(p)[(0xae7c + 0x14c + 4) / 4] = buf;
                     w.at(p)[(0xae7c + 0x14c + 8) / 4] = 0x40;
                     w.at(p)[(0xae7c + 0x14c + 0xc) / 4] = 0;
                     w.at(p)[(0xae7c + 0x110) / 4] = 0;
                     w.at(p)[(0xae7c + 0x114) / 4] = 0;
                     const U n = r() % 6, recs = w.add(r, 0x140 * 5);
                     for (U k = 0; k < 5; ++k) {
                         char* t = reinterpret_cast<char*>(w.at(recs)) + 0x140 * k + 0x2c;
                         std::snprintf(t, 0x20, "save%u", k);
                     }
                     w.at(p)[0xca00 / 4] = n || r() % 2 ? recs : 0u;
                     w.at(p)[0xca04 / 4] = recs + 0x140 * n;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {static_cast<U>(static_cast<int>(r() % 8) - 1)}); }, false});
    // the drawing hooks: [0x006320f0] (line: ECX [0x00632050], EDX, four stack words) and [0x006320f8] (polyline
    // segments: five stack words each) point at fake-table thunks 30 and 31
    c.push_back({"Draw_LineViaHook", 0x00498bd0, (void*)&recoil::Draw_LineViaHook, 200, [](std::mt19937&) { vt::set(30 * 4, 4); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     *ch::img(w.side, 0x006320f0) = ch::at(vt::table())[30];
                     *ch::img(w.side, 0x00632050) = r() % 1000;
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[3] = {pick(r), pick(r), pick(r)}; return call_any(fn, pick(r), pick(r), a, 3); }, false});
    c.push_back({"Draw_PolylineViaHook", 0x00498c00, (void*)&recoil::Draw_PolylineViaHook, 200, [](std::mt19937&) { vt::set(31 * 4, 5); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 8 * 12);
                     *ch::img(w.side, 0x006320f8) = ch::at(vt::table())[31];
                     *ch::img(w.side, 0x00632050) = r() % 1000;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[2] = {pick(r), pick(r)};
                     return call_any(fn, self(w), static_cast<U>(static_cast<int>(r() % 12) - 1), a, 2);
                 },
                 false});
    return c;
}
}  // namespace

TEST(native_menus_virtual_calls_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases(), "menus virtual-call cases"), 0);
}

// The KERNEL32 users, with the real functions bound into both import slots:
//  - SaveRecord_IsNewer (0x00434660): CompareFileTime of the FILETIMEs at +0x14 of ECX and EDX > 0;
//  - CDCheck_FindDiscDrive (0x004a59e0; ECX drive type 3 / 5 / other, EDX file name; ret 4): for each logical drive
//    of the given type, sprintf "%s%s" (drive, file) into 0x0056b438 and _stat it; the buffer (as a VA) on the first
//    hit, else 0 - this machine's drives, the same for both sides;
//  - GetMessageByID (0x004a5b60; cdecl dest, size, id, ...): FormatMessageA from the module [0x0056b670] (ntdll here),
//    CR/LF trimmed, strncpy, LocalFree; returns the length.
// Compared: returns (the buffer as a VA), output buffers.
TEST(native_menus_kernel32_users_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    struct Bind { U va; void** port; };
    const Bind binds[] = {{0x004cc108, &recoil::g_Iat_CompareFileTime_004cc108}, {0x004cc160, &recoil::g_Iat_GetDriveTypeA_004cc160},
                          {0x004cc164, &recoil::g_Iat_GetLogicalDriveStringsA_004cc164}, {0x004cc0f4, &recoil::g_Iat_FormatMessageA_004cc0f4},
                          {0x004cc0fc, &recoil::g_Iat_LocalFree_004cc0fc}};
    void* saved[5];
    for (int i = 0; i < 5; ++i) {
        void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(binds[i].va));
        saved[i] = *o;
        *o = *binds[i].port;
    }
    std::mt19937 rng(0x434660);
    // SaveRecord_IsNewer
    for (int it = 0; it < 2000; ++it) {
        U a[8], b[8];
        for (auto& x : a) x = rng() % 4 ? rng() % 8 : rng();
        for (auto& x : b) x = rng() % 4 ? rng() % 8 : rng();
        if (rng() % 4 == 0) { a[5] = b[5]; a[6] = b[6]; }
        const U fn[2] = {0x00434660, ch::addr(reinterpret_cast<void*>(&recoil::SaveRecord_IsNewer))};
        CHECK_EQ(call_any(fn[0], ch::addr(a), ch::addr(b), nullptr, 0), call_any(fn[1], ch::addr(a), ch::addr(b), nullptr, 0));
    }
    // CDCheck_FindDiscDrive
    static const char* const files[] = {"", "Windows", "missing_recoil_file.xyz", "."};
    for (int it = 0; it < 40; ++it) {
        const U type = it % 4 == 0 ? 7u : (it % 2 ? 3u : 5u);
        const char* file = files[rng() % 4];
        std::string out[2];
        U ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const U fn = side ? ch::addr(reinterpret_cast<void*>(&recoil::CDCheck_FindDiscDrive)) : 0x004a59e0u;
            U a[1] = {0};
            const U r = call_any(fn, type, ch::addr(file), a, 1);
            ret[side] = side && r ? recoil::ImageData_VaOf(reinterpret_cast<void*>(static_cast<std::uintptr_t>(r))) : r;
            out[side] = reinterpret_cast<const char*>(ch::img(side, 0x0056b438));
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(out[0] == out[1]);
    }
    // GetMessageByID
    for (int it = 0; it < 200; ++it) {
        const U id = rng() % 3 ? rng() % 400 : 0xC0000000u | (rng() % 0x200);
        const U size = 1 + rng() % 200;
        char out[2][256];
        U ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x0056b670) = ch::addr(GetModuleHandleA("ntdll.dll"));
            std::memset(out[side], 'q', sizeof out[side]);
            const U fn = side ? ch::addr(reinterpret_cast<void*>(&recoil::GetMessageByID)) : 0x004a5b60u;
            U a[5] = {ch::addr(out[side]), size, id, 0, 0};
            ret[side] = call_any(fn, 0, 0, a, 5);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(std::memcmp(out[0], out[1], sizeof out[0]) == 0);
    }
    for (int i = 0; i < 5; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(binds[i].va)) = saved[i];
    rt::restore_pristine();
}

// BitmapFont_MeasureString (0x0046f260; ECX text, EDX font index, stack out width, out height; ret 8): the font from
// TextureTable_GetOrDefault (the 20-entry table 0x0056179c, entry 0 as the fallback - its index is not range-checked,
// so the arena fuzz cannot drive it: indices stay 0..19 here); a missing font leaves the outputs untouched; else width
// = the widest line (spaces +4, glyph c at +8 / +0x10 of record entry c - 0x21, CR skipped, LF starts a line) and
// height = lines x the image height (short +6). Fonts: 0x610-byte records of small random words with an image header.
// Compared: return and both outputs.
TEST(native_menus_bitmap_font_measure_string_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(const char*, U, U*, U*);
    const F fn[2] = {rt::original<F>(0x0046f260), reinterpret_cast<F>(&recoil::BitmapFont_MeasureString)};
    std::mt19937 rng(0x46f260);
    for (int it = 0; it < 3000; ++it) {
        std::vector<U> fonts(3 * 0x610 / 4), img(4 * 3);
        for (auto& x : fonts) x = rng() % 64;
        for (auto& x : img) x = rng() % 64;
        std::vector<int> slot(20);
        for (int& s : slot) s = rng() % 3 ? static_cast<int>(rng() % 3) : -1;
        const U index = rng() % 20;
        std::string text;
        for (int k = static_cast<int>(rng() % 30); k > 0; --k) {
            const U c = rng() % 10;
            text += c == 0 ? ' ' : c == 1 ? '\n' : c == 2 ? '\r' : static_cast<char>(0x21 + rng() % 0x60);
        }
        std::vector<U> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<U> f = fonts, im = img;
            for (int k = 0; k < 3; ++k) f[k * 0x610 / 4] = ch::addr(&im[4 * k]);
            for (int k = 0; k < 20; ++k) *ch::img(side, 0x0056179c + 4 * k) = slot[k] < 0 ? 0u : ch::addr(&f[slot[k] * 0x610 / 4]);
            U w = 0xaaaa, h = 0xbbbb;
            snap[side].push_back(static_cast<U>(fn[side](text.c_str(), index, &w, &h)));
            snap[side].push_back(w);
            snap[side].push_back(h);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// The save-record sorts over 0x140-byte records ordered by the FILETIME at +0x14 (SaveRecord_IsNewer, so CompareFileTime
// is bound into both import slots): SaveRecordSort_Quick (0x004362f0; ECX first, EDX end, one unused stack word; ret 4: quicksort-style quicksort for
// ranges over 16 records, median of three), SaveRecordSort_Partition (0x00436580; ECX first, EDX last, the pivot record
// by value on the stack; ret 0x140; returns the split), SaveRecordSort_InsertOne (0x00436530; ECX position, the record
// by value; ret 0x140; unguarded - the array starts with a newest-possible sentinel). Filetimes with many ties, the rest
// of each record random. Compared: every record word afterwards (and the split as an index).
TEST(native_menus_save_record_sorts_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc108));
    void* const saved = *o;
    *o = recoil::g_Iat_CompareFileTime_004cc108;
    const U quick[2] = {0x004362f0, ch::addr(reinterpret_cast<void*>(&recoil::SaveRecordSort_Quick))};
    const U part[2] = {0x00436580, ch::addr(reinterpret_cast<void*>(&recoil::SaveRecordSort_Partition))};
    const U insert[2] = {0x00436530, ch::addr(reinterpret_cast<void*>(&recoil::SaveRecordSort_InsertOne))};
    constexpr U W = 0x140 / 4;
    std::mt19937 rng(0x4362f0);
    for (int it = 0; it < 600; ++it) {
        const int op = static_cast<int>(rng() % 3), n = 2 + static_cast<int>(rng() % 60);
        std::vector<U> init(W * (n + 1));
        for (auto& x : init) x = rng();
        for (int k = 0; k <= n; ++k) { init[k * W + 5] = rng() % 6; init[k * W + 6] = rng() % 3; }
        init[5] = 0xffffffff; init[6] = 0x7fffffff;  // record 0: the sentinel
        const int pos = 1 + static_cast<int>(rng() % n), piv = 1 + static_cast<int>(rng() % n);
        std::vector<U> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<U> a = init;
            U ret = 0;
            if (op == 0) { U z[1] = {0}; ret = call_any(quick[side], ch::addr(&a[W]), ch::addr(&a[W * (n + 1)]), z, 1); }
            else if (op == 1) {
                std::vector<U> pivot(a.begin() + W * piv, a.begin() + W * (piv + 1));
                const U r = call_any(part[side], ch::addr(&a[W]), ch::addr(&a[W * (n + 1)]), pivot.data(), W);
                ret = (r - ch::addr(a.data())) / 0x140;
            } else {
                std::vector<U> rec(W);
                for (U k = 0; k < W; ++k) rec[k] = init[(pos * W + k) % init.size()] ^ 0x5a5a;
                rec[5] = init[pos * W + 5] ^ 1; rec[6] = init[pos * W + 6];
                call_any(insert[side], ch::addr(&a[W * pos]), 0, rec.data(), W);
            }
            snap[side] = a;
            snap[side].push_back(op == 1 ? ret : 0u);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    *o = saved;
}

// Message_GetText (0x004a5bf0; ECX id): GetMessageByID into 0x0056b570 (0x100 bytes); the buffer or 0. FormatMessageA /
// LocalFree bound into both slots, messages from ntdll ([0x0056b670]). Compared: the return as a VA and the buffer.
TEST(native_menus_message_get_text_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const U va[2] = {0x004cc0f4, 0x004cc0fc};
    void** port[2] = {&recoil::g_Iat_FormatMessageA_004cc0f4, &recoil::g_Iat_LocalFree_004cc0fc};
    void* saved[2];
    for (int i = 0; i < 2; ++i) { void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])); saved[i] = *o; *o = *port[i]; }
    std::mt19937 rng(0x4a5bf0);
    for (int it = 0; it < 200; ++it) {
        const U id = rng() % 3 ? rng() % 400 : 0xC0000000u | (rng() % 0x200);
        U ret[2];
        std::string out[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x0056b670) = ch::addr(GetModuleHandleA("ntdll.dll"));
            const U fn = side ? ch::addr(reinterpret_cast<void*>(&recoil::Message_GetText)) : 0x004a5bf0u;
            const U r = call_any(fn, id, 0, nullptr, 0);
            ret[side] = side && r ? recoil::ImageData_VaOf(reinterpret_cast<void*>(static_cast<std::uintptr_t>(r))) : r;
            out[side] = std::string(reinterpret_cast<const char*>(ch::img(side, 0x0056b570)), 0x100);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(out[0] == out[1]);
    }
    for (int i = 0; i < 2; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])) = saved[i];
    rt::restore_pristine();
}

// Scoreboard_Refresh (0x0040e140): sorts the score rows (+0x80 / +0x84, 0x50 bytes: name +4, values +0x44 / +0x48,
// colour +0x4c) with ScoreRows_InsertionSort / QuickSort, then fills up to eight rows of three labels ([+0x1c + 12*row]
// .. ) - colour, visibility (slot 0x60), font (slot 0x80, seven words), position (slot 0xc, two words) from the layout
// +0x8c..+0xa4, text (variadic slot 0x74) - hides the unused rows, and fills the header labels +0x10 / +0x14 / +0x18,
// whose texts come from Message_GetText (FormatMessageA / LocalFree bound into both slots, messages from ntdll) and the
// flag [0x004f3128] / value [0x004f3130]. Rows 0..10 with names, values with ties, labels as fake-table widgets.
// Also Scoreboard_UpdateEntry / Scoreboard_RemoveEntry (hud, ported in the cloud) on the same boards.
TEST(native_menus_scoreboard_refresh_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const U va[2] = {0x004cc0f4, 0x004cc0fc};
    void** port[2] = {&recoil::g_Iat_FormatMessageA_004cc0f4, &recoil::g_Iat_LocalFree_004cc0fc};
    void* saved[2];
    for (int i = 0; i < 2; ++i) { void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])); saved[i] = *o; *o = *port[i]; }
    std::vector<Case> c;
    auto slots = [](std::mt19937&) { vt::set(0x60, 1); vt::set(0x80, 7); vt::set(0xc, 2); vt::set(0x74, 0, 3); };
    auto board = [](World& w, std::mt19937& r) {
        const U p = w.add(r, 0xb0);
        for (U k = 0; k < 3; ++k) w.at(p)[4 + k] = object(w, r, 0x2a4);
        for (U k = 0; k < 24; ++k) w.at(p)[7 + k] = object(w, r, 0x2a4);
        const U n = r() % 11, rows = w.add(r, 0x50 * 10);
        for (U k = 0; k < 10; ++k) {
            U* row = w.at(rows) + 0x14 * k;
            row[0] = r() % 12;
            std::snprintf(reinterpret_cast<char*>(row + 1), 0x20, "player%u", static_cast<unsigned>(r() % 20));
            row[0x44 / 4] = r() % 5; row[0x48 / 4] = static_cast<U>(static_cast<int>(r() % 5) - 1); row[0x4c / 4] = r() % 0x10000;
        }
        w.at(p)[0x80 / 4] = n || r() % 2 ? rows : 0u;
        w.at(p)[0x84 / 4] = rows + 0x50 * n;
        for (U off = 0x8c; off <= 0xa4; off += 4) w.at(p)[off / 4] = r() % 400;
        *ch::img(w.side, 0x004f3128) = r() % 2;
        *ch::img(w.side, 0x004f3130) = r() % 50;
        *ch::img(w.side, 0x0056b670) = ch::addr(GetModuleHandleA("ntdll.dll"));
        const U e = w.add(r, 0x14);                      // the entry for Update / Remove: id, -, colour, values
        w.at(e)[0] = r() % 12;
    };
    // the entry block is the last one
    auto entry_call = [](World& w, U fn, std::mt19937&) {
        U a[1] = {ch::addr(w.blocks.back().w.data())};
        return call_any(fn, self(w), 0, a, 1);
    };
    c.push_back({"Scoreboard_Refresh", 0x0040e140, (void*)&recoil::Scoreboard_Refresh, 300, slots, board,
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, self(w), 0, nullptr, 0); }, false});
    // Scoreboard_UpdateEntry / Scoreboard_RemoveEntry (0x0040e800 / 0x0040e880; stack entry; ret 4, hud): the row whose
    // id matches the entry's (among the first 8) takes its colour and values (the second value only when [0x004f3128],
    // else -1) / is removed by moving the rows behind it down; then Scoreboard_Refresh
    c.push_back({"Scoreboard_UpdateEntry", 0x0040e800, (void*)&recoil::Scoreboard_UpdateEntry, 300, slots, board, entry_call, false});
    c.push_back({"Scoreboard_RemoveEntry", 0x0040e880, (void*)&recoil::Scoreboard_RemoveEntry, 300, slots, board, entry_call, false});
    // Hud_ForwardToPanel_0040e800 / _0040e880 (ECX entry): the same on the board at [[0x004ed4e0] + 0x34]
    auto panel_board = [board](World& w, std::mt19937& r) {
        board(w, r);
        const U holder = w.add(r, 0x40);
        w.at(holder)[0x34 / 4] = self(w);
        *ch::img(w.side, 0x004ed4e0) = holder;
    };
    auto entry_ecx = [](World& w, U fn, std::mt19937&) {
        // the entry is the block before the holder
        return call_any(fn, ch::addr(w.blocks[w.blocks.size() - 2].w.data()), 0, nullptr, 0);
    };
    c.push_back({"Hud_ForwardToPanel_0040e800", 0x004143b0, (void*)&recoil::Hud_ForwardToPanel_0040e800, 200, slots, panel_board, entry_ecx, false});
    c.push_back({"Hud_ForwardToPanel_0040e880", 0x004143c0, (void*)&recoil::Hud_ForwardToPanel_0040e880, 200, slots, panel_board, entry_ecx, false});
    CHECK_EQ(run_cases(c, "scoreboard refresh"), 0);
    for (int i = 0; i < 2; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])) = saved[i];
    rt::restore_pristine();
}

// Call004a5bf0_If004a5b20 (0x004a5b40; ui_widgets, ported in the cloud; ECX key): the id lookup hook [0x0056b568]
// (CallOptionalHook_0056b568; a fake cdecl lookup here, or none) -> Message_GetText(id) into the shared buffer
// 0x0056b570, else the key itself. FormatMessageA / LocalFree bound into both slots, messages from ntdll. Compared:
// the return (a VA, or the key), the buffer and the lookup's argument.
namespace {
std::vector<U>& lookups()
{
    static std::vector<U> v;
    return v;
}
U& lookup_result()
{
    static U v = 0;
    return v;
}
U __cdecl fake_lookup(U key) { lookups().push_back(key); return lookup_result(); }
}  // namespace
TEST(native_ui_message_lookup_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const U va[2] = {0x004cc0f4, 0x004cc0fc};
    void** port[2] = {&recoil::g_Iat_FormatMessageA_004cc0f4, &recoil::g_Iat_LocalFree_004cc0fc};
    void* saved[2];
    for (int i = 0; i < 2; ++i) { void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])); saved[i] = *o; *o = *port[i]; }
    std::mt19937 rng(0x4a5b40);
    for (int it = 0; it < 300; ++it) {
        const bool hook = rng() % 4 != 0;
        const U key = rng();
        lookup_result() = rng() % 4 == 0 ? 0u : (rng() % 3 ? rng() % 400 : 0xC0000000u | (rng() % 0x200));
        U ret[2];
        std::string out[2];
        std::vector<U> seen[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x0056b670) = ch::addr(GetModuleHandleA("ntdll.dll"));
            *ch::img(side, 0x0056b568) = hook ? ch::addr(reinterpret_cast<void*>(&fake_lookup)) : 0u;
            lookups().clear();
            const U fn = side ? ch::addr(reinterpret_cast<void*>(&recoil::Call004a5bf0_If004a5b20)) : 0x004a5b40u;
            const U r = call_any(fn, key, 0, nullptr, 0);
            const U img_va = side ? recoil::ImageData_VaOf(reinterpret_cast<void*>(static_cast<std::uintptr_t>(r))) : r;
            ret[side] = side && img_va ? img_va : r;
            out[side] = std::string(reinterpret_cast<const char*>(ch::img(side, 0x0056b570)), 0x100);
            seen[side] = lookups();
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(out[0] == out[1]);
        CHECK(seen[0] == seen[1]);
    }
    for (int i = 0; i < 2; ++i) *reinterpret_cast<void**>(static_cast<std::uintptr_t>(va[i])) = saved[i];
    rt::restore_pristine();
}
