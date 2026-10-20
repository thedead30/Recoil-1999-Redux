// Structured native L1s for the ui_widgets functions that call virtual methods (P5 ui_widgets, ported in the cloud).
// Each case builds the same object graph on both sides from one seed - objects of random words whose vtable pointers
// (at +0 and at every embedded sub-object the listing calls through) are the fake table of tests/fake_vtable.h - sets
// the slots the listing calls with the argument counts read from its call sites (callee-clean thiscall; 0 for the
// variadic slot 0x74 / 0x88 forms whose caller pops), calls the original and the port, and compares the call log (slot,
// this, arguments), EAX, and every word of every object. Words are compared normalised: pointers into the test's
// blocks as (block, offset), the fake table as one token, stack addresses as one token, the port's image addresses as
// the original's VAs, everything else raw.
#include "watchdog.h"
#include "test.h"
#include "cloud_harness.h"
#include "fake_vtable.h"
#include "unattributed/ui_widgets.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace {

// fastcall / thiscall / cdecl call with n stack words; ESP is restored whatever the callee pops
__declspec(noinline) std::uint32_t call_any(std::uint32_t fn, std::uint32_t ecx_v, std::uint32_t edx_v, const std::uint32_t* args, int n)
{
    std::uint32_t result = 0;
    __asm {
        mov ebx, esp
        mov eax, n
        mov edx, args
    push_loop:
        test eax, eax
        jz pushed
        dec eax
        push dword ptr [edx + eax*4]
        jmp push_loop
    pushed:
        mov ecx, ecx_v
        mov edx, edx_v
        call fn
        mov esp, ebx
        mov result, eax
    }
    return result;
}

struct World {
    struct Block { std::vector<std::uint32_t> w; };
    std::vector<Block> blocks;
    int side = 0;
    // a block of n bytes filled from the seed; returns its address
    std::uint32_t add(std::mt19937& r, std::uint32_t bytes, bool small = false)
    {
        Block b;
        b.w.resize((bytes + 3) / 4);
        for (auto& x : b.w) {
            x = small ? r() % 64 : (r() % 3 ? r() % 0x10000 : r());
            // a random word that lands inside the port's data mirror would be normalised as a pointer on the port side
            // only (w.norm) - a false difference in about 1 of 200 words; keep filler out of that range
            if (recoil::ImageData_VaOf(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(x)))) x ^= 0x80000000u;
        }
        blocks.push_back(std::move(b));
        return ch::addr(blocks.back().w.data());
    }
    std::uint32_t* at(std::uint32_t p) { return ch::at(p); }
    std::uint32_t norm(std::uint32_t v) const
    {
        if (v == 0) return 0;
        if (v == vt::table()) return 0x7AB1E000;
        for (std::size_t k = 0; k < blocks.size(); ++k) {
            const std::uint32_t base = ch::addr(blocks[k].w.data()), size = static_cast<std::uint32_t>(4 * blocks[k].w.size());
            if (v >= base && v <= base + size) return 0xB0000000u + static_cast<std::uint32_t>(k) * 0x100000u + (v - base);
        }
        NT_TIB* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
        if (v >= ch::addr(tib->StackLimit) && v < ch::addr(tib->StackBase)) return 0x57AC0000;
        if (side) {
            const std::uint32_t va = recoil::ImageData_VaOf(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(v)));
            if (va) return va;
        }
        return v;
    }
};

struct Case {
    const char* name;
    std::uint32_t va;
    void* port;
    int iters;
    // slots, the graph (block 0 is ECX unless the call says otherwise) and the call; returns EAX
    std::function<void(std::mt19937&)> slots;
    std::function<void(World&, std::mt19937&)> build;
    std::function<std::uint32_t(World&, std::uint32_t fn, std::mt19937&)> call;
    bool compare_eax = true;
};

std::uint32_t fbits(float f) { return ch::fbits(f); }

// an object of `bytes` with the fake table at +0 and at each embedded offset
std::uint32_t object(World& w, std::mt19937& r, std::uint32_t bytes, std::initializer_list<std::uint32_t> embedded = {})
{
    const std::uint32_t p = w.add(r, bytes);
    w.at(p)[0] = vt::table();
    for (std::uint32_t e : embedded) w.at(p)[e / 4] = vt::table();
    return p;
}
// an image header: width / height as shorts at +4 / +6
std::uint32_t image(World& w, std::mt19937& r)
{
    const std::uint32_t p = w.add(r, 0x40);
    reinterpret_cast<std::int16_t*>(w.at(p))[2] = static_cast<std::int16_t>(r() % 400 - 50);
    reinterpret_cast<std::int16_t*>(w.at(p))[3] = static_cast<std::int16_t>(r() % 300 - 50);
    return p;
}
std::uint32_t pick(std::mt19937& r) { return r() % 3 == 0 ? r() : r() % 700; }

std::vector<Case> cases()
{
    using U = std::uint32_t;
    std::vector<Case> c;
    auto self_call = [](int nargs) {
        return [nargs](World& w, U fn, std::mt19937& r) {
            U a[8];
            for (int k = 0; k < nargs; ++k) a[k] = pick(r);
            return call_any(fn, ch::addr(w.blocks[0].w.data()), r(), a, nargs);
        };
    };
    auto obj_only = [](U bytes) { return [bytes](World& w, std::mt19937& r) { object(w, r, bytes); }; };

    c.push_back({"Widget_SetPosition", 0x00404cd0, (void*)&recoil::Widget_SetPosition, 300, [](std::mt19937&) { vt::set(0x20, 0); }, obj_only(0x40), self_call(2)});
    c.push_back({"Widget_SetX", 0x00404cf0, (void*)&recoil::Widget_SetX, 300, [](std::mt19937&) { vt::set(0x20, 0); }, obj_only(0x40), self_call(1)});
    c.push_back({"Widget_SetY", 0x00404d00, (void*)&recoil::Widget_SetY, 300, [](std::mt19937&) { vt::set(0x20, 0); }, obj_only(0x40), self_call(1)});
    c.push_back({"Widget_SetVisible", 0x00404d20, (void*)&recoil::Widget_SetVisible, 300, [](std::mt19937&) { vt::set(0x20, 0); }, obj_only(0x40),
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r() % 3 ? r() % 2 : r()}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); }});
    c.push_back({"Widget_SetField14CAndRedraw", 0x004ba3c0, (void*)&recoil::Widget_SetField14CAndRedraw, 300, [](std::mt19937&) { vt::set(0x20, 0); }, obj_only(0x160), self_call(1)});
    c.push_back({"Widget_SetField1CAndRect", 0x004bcd40, (void*)&recoil::Widget_SetField1CAndRect, 300, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x40); w.add(r, 0x10); },
                 [](World& w, U fn, std::mt19937& r) { U a[2] = {pick(r), r() % 4 ? ch::addr(w.blocks[1].w.data()) : 0u}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 2); }});
    c.push_back({"Widget_SetField1CAndNotify", 0x004b4190, (void*)&recoil::Widget_SetField1CAndNotify, 300, [](std::mt19937&) { vt::set(0x1c, 1); }, obj_only(0x40), self_call(2)});
    c.push_back({"ImageWidget_SetImageNoOwn", 0x004b3e70, (void*)&recoil::ImageWidget_SetImageNoOwn, 300, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x50); image(w, r); },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r() % 4 ? ch::addr(w.blocks[1].w.data()) : 0u}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); }});
    c.push_back({"Widget_SetRectAndParams", 0x004bdc00, (void*)&recoil::Widget_SetRectAndParams, 300, [](std::mt19937&) { vt::set(0xc, 2); }, obj_only(0x50), self_call(7)});
    c.push_back({"ImageWidget_SetPositionAndAnchor", 0x004bffb0, (void*)&recoil::ImageWidget_SetPositionAndAnchor, 300, [](std::mt19937&) { vt::set(0xc, 2); }, obj_only(0x40), self_call(4)});
    c.push_back({"LineWidget_SetPoint", 0x004bf8b0, (void*)&recoil::LineWidget_SetPoint, 300, [](std::mt19937&) { vt::set(0xc, 2); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0xe0); w.at(p)[0xdc / 4] = r() % 20; },
                 [](World& w, U fn, std::mt19937& r) { U a[3] = {r() % 16, pick(r), pick(r)}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 3); }});
    // the rect slot 0x2c hands out a rect block of this graph, or 0
    c.push_back({"Widget_HitTestRect", 0x004b4030, (void*)&recoil::Widget_HitTestRect, 400, [](std::mt19937&) { vt::set(0x2c, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x40);
                     const U rect = w.add(r, 0x10);
                     U* q = w.at(rect);
                     q[0] = r() % 300; q[1] = r() % 300; q[2] = q[0] + r() % 300; q[3] = q[1] + r() % 300;
                     vt::slots()[0x2c / 4].ret = r() % 5 ? rect : 0u;
                     w.at(p)[0xc / 4] = r() % 2 ? 0x10u : r();
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[2] = {r() % 700, r() % 700}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 2); }});
    c.push_back({"Widget_GetExtentRect", 0x004b42c0, (void*)&recoil::Widget_GetExtentRect, 300,
                 [](std::mt19937& r) { vt::set(0x64, 0, 0, 0, r() % 500); vt::set(0x68, 0, 0, 0, r() % 500); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x40); w.add(r, 0x10); },
                 [](World& w, U fn, std::mt19937&) { U a[1] = {ch::addr(w.blocks[1].w.data())}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); }});
    c.push_back({"Widget_Tick", 0x004b41e0, (void*)&recoil::Widget_Tick, 600, [](std::mt19937&) { vt::set(0x4, 0); vt::set(0x8, 0); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x40); w.at(p)[0xc / 4] = r() % 64; w.at(p)[0x10 / 4] = fbits(static_cast<float>(r() % 100) / 10.0f - 2.0f); },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {fbits(static_cast<float>(r() % 50) / 10.0f)}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); }, false});
    c.push_back({"ImageWidget_Slot_UpdateBoundsFromImage", 0x00404e10, (void*)&recoil::ImageWidget_Slot_UpdateBoundsFromImage, 300, [](std::mt19937&) { vt::set(0x1c, 1, 1, 4); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x40); const U img = image(w, r); w.at(p)[0x3c / 4] = r() % 4 ? img : 0u; },
                 self_call(0), false});
    c.push_back({"ImageWidget_SetPosition", 0x004b3dd0, (void*)&recoil::ImageWidget_SetPosition, 300, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x50); const U img = image(w, r); w.at(p)[0x3c / 4] = r() % 4 ? img : 0u; w.at(p)[0x48 / 4] = r() % 2; },
                 self_call(2), false});
    c.push_back({"ImageWidget_Slot_ComputeBounds", 0x004bff00, (void*)&recoil::ImageWidget_Slot_ComputeBounds, 400,
                 [](std::mt19937& r) { vt::set(0x64, 0, 0, 0, r() % 3 ? r() % 200 : static_cast<U>(-5)); vt::set(0x68, 0, 0, 0, r() % 3 ? r() % 200 : 0u); vt::set(0x1c, 1, 1, 4); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x40);
                     const U a = image(w, r), b = image(w, r);
                     w.at(p)[0x34 / 4] = r() % 4 ? a : 0u;
                     w.at(p)[0x1c / 4] = r() % 2 ? b : 0u;
                 },
                 self_call(0), false});
    c.push_back({"Widget_AddDirtyRect", 0x004b3e90, (void*)&recoil::Widget_AddDirtyRect, 500,
                 [](std::mt19937& r) { vt::set(0x64, 0, 0, 0, r() % 50); vt::set(0x68, 0, 0, 0, r() % 50); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0xc0);
                     const U img = image(w, r);
                     w.at(p)[0x3c / 4] = r() % 5 ? img : 0u;
                     for (int k = 0; k < 4; ++k) w.at(p)[(0x4c + 0x1c * k) / 4] = r() % 3 ? 1u : 0u;
                     const U rect = w.add(r, 0x10);
                     U* q = w.at(rect);
                     q[0] = r() % 400; q[1] = r() % 300; q[2] = q[0] + r() % 200 - 20; q[3] = q[1] + r() % 200 - 20;
                     *ch::img(w.side, 0x004e4870) = r() % 2 ? 4u : 0xcu;
                 },
                 [](World& w, U fn, std::mt19937&) { U a[1] = {ch::addr(w.blocks[2].w.data())}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); }, false});
    c.push_back({"ListLabel_GetText", 0x004bb440, (void*)&recoil::ListLabel_GetText, 200, [](std::mt19937&) { vt::set(0x90, 0); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x280); w.at(p)[0x270 / 4] = r() % 2; }, self_call(0)});
    c.push_back({"ListLabel_GetLineHeight", 0x004bb710, (void*)&recoil::ListLabel_GetLineHeight, 200, [](std::mt19937&) { vt::set(0x90, 0); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x280); w.at(p)[0x270 / 4] = r() % 2; }, self_call(0)});
    // tail jumps through the table: the slot's own cleanup applies
    c.push_back({"Widget_Slot_CallVirtual08", 0x00404ca0, (void*)&recoil::Widget_Slot_CallVirtual08, 100, [](std::mt19937&) { vt::set(0x8, 0); }, obj_only(0x40), self_call(0), false});
    c.push_back({"Widget_Slot_TailVirtual8C", 0x0041a290, (void*)&recoil::Widget_Slot_TailVirtual8C, 100, [](std::mt19937&) { vt::set(0x8c, 0); }, obj_only(0x40), self_call(0), false});
    c.push_back({"UiScreen_Slot_TailOwnerSlot0C", 0x004353e0, (void*)&recoil::UiScreen_Slot_TailOwnerSlot0C, 100, [](std::mt19937&) { vt::set(0xc, 2); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0xd0); const U o = object(w, r, 0x40); w.at(p)[0xc8 / 4] = o; }, self_call(2), false});
    c.push_back({"ScrollGroup_ActivateChild", 0x00409010, (void*)&recoil::ScrollGroup_ActivateChild, 300, [](std::mt19937&) { vt::set(0x78, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x170);
                     const U n = r() % 5;
                     w.at(p)[0x14c / 4] = n;
                     for (U k = 0; k < 4; ++k) { const U ch_ = object(w, r, 0xd0); w.at(p)[0x150 / 4 + k] = ch_; }
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r() % 6}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); }, false});
    c.push_back({"UiScreen_Slot_NotifyVisibleChildren", 0x0040db90, (void*)&recoil::UiScreen_Slot_NotifyVisibleChildren, 200, [](std::mt19937&) { vt::set(0x4, 0); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x120, {0x48, 0x104}); w.at(p)[0x54 / 4] = r() % 64; w.at(p)[0x110 / 4] = r() % 64; }, self_call(0), false});
    c.push_back({"UiScreen_Slot_Virtual08_NotifyThreeChildren", 0x0040f400, (void*)&recoil::UiScreen_Slot_Virtual08_NotifyThreeChildren, 200,
                 [](std::mt19937&) { vt::set(0x8, 0); vt::set(0x4, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x1d0, {0x1b4, 0xf8, 0x3c});
                     w.at(p)[0x1c0 / 4] = r() % 64; w.at(p)[0x104 / 4] = r() % 64; w.at(p)[0x48 / 4] = r() % 64;
                 },
                 self_call(0), false});
    c.push_back({"Widget_Slot_ForwardToMember34Slot0", 0x0040fa10, (void*)&recoil::Widget_Slot_ForwardToMember34Slot0, 100, [](std::mt19937&) { vt::set(0x0, 1); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x40); w.at(p)[0x34 / 4] = object(w, r, 0x40); }, self_call(1), false});
    c.push_back({"DeleteWidgetPtr", 0x004b52f0, (void*)&recoil::DeleteWidgetPtr, 100, [](std::mt19937&) { vt::set(0x0, 1); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x40); },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r() % 4 ? ch::addr(w.blocks[0].w.data()) : 0u}; return call_any(fn, r(), 0, a, 1); }});
    c.push_back({"ListItem_Slot_ForwardValueToTarget", 0x004b9520, (void*)&recoil::ListItem_Slot_ForwardValueToTarget, 200, [](std::mt19937&) { vt::set(0x84, 1); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x2b0); const U t = object(w, r, 0x40); w.at(p)[0x2a8 / 4] = r() % 4 ? t : 0u; }, self_call(0), false});
    c.push_back({"WidgetContainer_RedrawChildren", 0x004ba3a0, (void*)&recoil::WidgetContainer_RedrawChildren, 200, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x20);
                     const int n = static_cast<int>(r() % 5);
                     U prev = 0;
                     for (int k = 0; k < n; ++k) { const U o = object(w, r, 0x20); w.at(o)[1] = prev; prev = o; }
                     w.at(p)[2] = prev;
                 },
                 self_call(0), false});
    c.push_back({"Manager_BroadcastVirtual24", 0x004bc900, (void*)&recoil::Manager_BroadcastVirtual24, 200, [](std::mt19937&) { vt::set(0x24, 1); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x20);
                     const int n = static_cast<int>(r() % 5);
                     U prev = 0;
                     for (int k = 0; k < n; ++k) { const U o = object(w, r, 0x20); w.at(o)[1] = prev; prev = o; }
                     w.at(p)[2] = prev;
                     w.at(p)[1] = r() % 4 ? 1u : 0u;
                 },
                 self_call(1), false});
    c.push_back({"Widget_ForwardSlot88", 0x004ba3e0, (void*)&recoil::Widget_ForwardSlot88, 100, [](std::mt19937&) { vt::set(0x88, 0); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x120); w.at(p)[0x110 / 4] = object(w, r, 0x40); }, self_call(0), false});
    c.push_back({"WidgetArray_SetRectAll", 0x004bd110, (void*)&recoil::WidgetArray_SetRectAll, 200, [](std::mt19937&) { vt::set(0x80, 7); },
                 [](World& w, std::mt19937& r) { object(w, r, 0xab0, {0x10, 0x2b4, 0x558, 0x7fc}); }, self_call(4), false});
    c.push_back({"MessagePanel_Clear", 0x004bd2a0, (void*)&recoil::MessagePanel_Clear, 100, [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) { object(w, r, 0xab0, {0x10, 0x2b4, 0x558, 0x7fc}); }, self_call(0), false});
    c.push_back({"MessagePanel_SetCentreX", 0x004bd410, (void*)&recoil::MessagePanel_SetCentreX, 100, [](std::mt19937&) { vt::set(0x10, 1); },
                 [](World& w, std::mt19937& r) { object(w, r, 0xab0, {0x10, 0x2b4, 0x558, 0x7fc}); }, self_call(1), false});
    c.push_back({"MessagePanel_SetTopY", 0x004bd440, (void*)&recoil::MessagePanel_SetTopY, 100, [](std::mt19937&) { vt::set(0x14, 1); },
                 [](World& w, std::mt19937& r) { object(w, r, 0xab0, {0x10, 0x2b4, 0x558, 0x7fc}); }, self_call(1), false});
    c.push_back({"ScreenFX_SetPrimaryRect", 0x004bed50, (void*)&recoil::ScreenFX_SetPrimaryRect, 200, [](std::mt19937&) { vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x80, {0x28}); }, self_call(3), false});
    c.push_back({"Widget_Slot_DrawWithStyle", 0x004bdb60, (void*)&recoil::Widget_Slot_DrawWithStyle, 300, [](std::mt19937&) { vt::set(0x8, 0); vt::set(0x74, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x40);
                     const U style = w.add(r, 0x28);
                     U* s = w.at(style);
                     s[4] = r() % 3 ? r() : 0; s[5] = r() % 3 ? r() : 0; s[6] = r() % 2;
                     w.at(p)[2] = r() % 4 ? style : 0u;
                 },
                 [](World& w, U fn, std::mt19937&) {
                     const U r = call_any(fn, ch::addr(w.blocks[0].w.data()), 0, nullptr, 0);
                     for (U k = 0; k < 5; ++k) vt::log().push_back(*ch::img(w.side, 0x0056b1c4 + 4 * k));
                     return r;
                 },
                 false});
    c.push_back({"ListWidget_BroadcastIfVisible", 0x004bb980, (void*)&recoil::ListWidget_BroadcastIfVisible, 200, [](std::mt19937&) { vt::set(0x24, 1); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x2c0);
                     const U n = r() % 4;
                     const U arr = w.add(r, 0x2c0 * 3 + 4);
                     for (U k = 0; k < 3; ++k) w.at(arr)[0x2c0 / 4 * k] = vt::table();
                     w.at(p)[0x2ac / 4] = n || r() % 2 ? arr : 0u;
                     w.at(p)[0x2b0 / 4] = arr + 0x2c0 * n;
                     w.at(p)[0xc / 4] = r() % 2 ? 0u : 0x10u;
                 },
                 self_call(1), false});
    c.push_back({"ListWidget_PrintfCurrent", 0x004bbaa0, (void*)&recoil::ListWidget_PrintfCurrent, 100, [](std::mt19937&) { vt::set(0x88, 0, 3); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x40); },
                 [](World& w, U fn, std::mt19937& r) { U a[4] = {ch::addr(w.blocks[0].w.data()), 0x1234, pick(r), pick(r)}; return call_any(fn, 0, 0, a, 4); }, false});
    c.push_back({"ListWidget_ForwardToCurrentItem", 0x004bbac0, (void*)&recoil::ListWidget_ForwardToCurrentItem, 100,
                 [](std::mt19937&) { vt::set(0x88, 0, 3); vt::set(0x60, 1); vt::set(0x94, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x2c0);
                     const U arr = w.add(r, 0x2c0 * 3);
                     for (U k = 0; k < 3; ++k) w.at(arr)[0x2c0 / 4 * k] = vt::table();
                     w.at(p)[0x2ac / 4] = arr;
                     w.at(p)[0x2a4 / 4] = r() % 3;
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[3] = {ch::addr(w.blocks[0].w.data()), pick(r), pick(r)}; return call_any(fn, 0, 0, a, 3); }, false});
    c.push_back({"ListWidget_ClearRange", 0x004bbed0, (void*)&recoil::ListWidget_ClearRange, 300, [](std::mt19937&) { vt::set(0x74, 0, 2); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x2c0);
                     const U n = r() % 4;
                     const U arr = w.add(r, 0x2c0 * 3 + 4);
                     for (U k = 0; k < 3; ++k) w.at(arr)[0x2c0 / 4 * k] = vt::table();
                     w.at(p)[0x2ac / 4] = n || r() % 2 ? arr : 0u;
                     w.at(p)[0x2b0 / 4] = arr + 0x2c0 * n;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[2] = {static_cast<U>(static_cast<int>(r() % 6) - 1), static_cast<U>(static_cast<int>(r() % 6) - 1)};
                     return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 2);
                 },
                 false});
    c.push_back({"ListScreen_ScrollTo", 0x004b9330, (void*)&recoil::ListScreen_ScrollTo, 300,
                 [](std::mt19937&) { vt::set(0x74, 0, 3); vt::set(0x60, 1); vt::set(0x8, 0); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     static const char* const texts[] = {"alpha", "beta", "gamma", "delta", "epsilon"};
                     const U p = object(w, r, 0x440, {0x16c});
                     const U vis = r() % 4, total = vis + r() % 3;
                     w.at(p)[0x168 / 4] = vis;
                     w.at(p)[0x164 / 4] = total;
                     const U arr = w.add(r, 0x2ac * 6);
                     for (U k = 0; k < 6; ++k) w.at(arr)[0x2ac / 4 * k] = vt::table();
                     w.at(p)[0x418 / 4] = arr;
                     const U n = r() % 6;
                     const U items = w.add(r, 4 * 6), rec = w.add(r, 4 * 6);
                     for (U k = 0; k < 6; ++k) { w.at(rec)[k] = ch::addr(texts[k % 5]); w.at(items)[k] = rec + 4 * k; }
                     w.at(p)[0x420 / 4] = n || r() % 2 ? items : 0u;
                     w.at(p)[0x424 / 4] = items + 4 * n;
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {static_cast<U>(static_cast<int>(r() % 8) - 1)}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); },
                 false});
    c.push_back({"Cycler_SetItemFont", 0x004b8100, (void*)&recoil::Cycler_SetItemFont, 300, [](std::mt19937&) { vt::set(0x80, 7); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x1c0);
                     w.at(p)[0x150 / 4] = r() % 22;
                     w.at(p)[0x158 / 4] = r() % 22;
                     const U owner = w.add(r, 0x1cec + 0x24 * 4);
                     for (U k = 0; k < 4; ++k) w.at(owner)[(0x1cec + 0x24 * k) / 4] = r() % 3 ? 1u + r() % 9 : 0u;
                     w.at(p)[0xc8 / 4] = owner;
                     for (U k = 0; k < 20; ++k) w.at(p)[0x168 / 4 + k] = object(w, r, 0x2a4);
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[2] = {r() % 20, r() % 4}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 2); }, false});
    c.push_back({"EditField_SetEnabled", 0x004b4ba0, (void*)&recoil::EditField_SetEnabled, 300, [](std::mt19937&) { vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x370, {0x260});
                     const U child = object(w, r, 0x40);
                     const U vec = w.add(r, 8);
                     w.at(vec)[0] = r() % 4 ? child : 0u;
                     const U n = r() % 3;
                     w.at(p)[0x110 / 4] = n || r() % 2 ? vec : 0u;
                     w.at(p)[0x114 / 4] = vec + 4 * (n ? 1 : 0);
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {r() % 3 ? r() % 2 : r()}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); }});
    c.push_back({"Widget_MarkDirtyAndRedrawChildren", 0x004b5310, (void*)&recoil::Widget_MarkDirtyAndRedrawChildren, 200, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x120);
                     const U n = r() % 4;
                     const U vec = w.add(r, 4 * 3);
                     for (U k = 0; k < 3; ++k) w.at(vec)[k] = object(w, r, 0x40);
                     w.at(p)[0x110 / 4] = n || r() % 2 ? vec : 0u;
                     w.at(p)[0x114 / 4] = vec + 4 * n;
                     *ch::img(w.side, 0x004e4870) = r() % 2 ? 4u : 0xcu;
                 },
                 self_call(0), false});
    // the text buffer: +4 characters (NUL-terminated), +8 capacity, +0xC cursor; a full buffer only redraws
    c.push_back({"EditField_InsertChar", 0x004b44e0, (void*)&recoil::EditField_InsertChar, 600, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x40);
                     const U buf = w.add(r, 0x48);
                     char* t = reinterpret_cast<char*>(w.at(buf));
                     const U cap = 1 + r() % 0x40, len = r() % (cap + 1 < 0x40 ? cap + 1 : 0x40);
                     for (U k = 0; k < len; ++k) t[k] = static_cast<char>('a' + r() % 26);
                     t[len] = 0;
                     w.at(p)[1] = buf;
                     w.at(p)[2] = cap;
                     w.at(p)[3] = r() % (len + 1);
                 },
                 [](World& w, U fn, std::mt19937& r) { U a[1] = {static_cast<U>('A' + r() % 26) | (r() % 2 ? 0u : r() & 0xffffff00u)}; return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a, 1); },
                 false});
    // a config tree of list nodes {4, array} (array: header {0, count}, then {type, value} children) whose first child is
    // the name string: the root holds the section named by the string at 0x004e4838, which holds the buttons
    c.push_back({"ScreenBase_BindButtonImpl", 0x004ba070, (void*)&recoil::ScreenBase_BindButtonImpl, 400,
                 [](std::mt19937&) { vt::set(0x7c, 2); vt::set(0x80, 0); },
                 [](World& w, std::mt19937& r) {
                     static std::string key;
                     key = reinterpret_cast<const char*>(0x004e4838);
                     static const char* const names[] = {"ok", "cancel", "back", "next"};
                     const U self = object(w, r, 0x40);
                     const U button = object(w, r, 0x40);
                     auto list = [&](const char* name, std::vector<U> kids) {
                         const U arr = w.add(r, 8 * (kids.size() + 2));
                         U* a = w.at(arr);
                         a[0] = 0; a[1] = static_cast<U>(kids.size() + 2);
                         a[2] = 3; a[3] = ch::addr(name);
                         for (std::size_t k = 0; k < kids.size(); ++k) { a[4 + 2 * k] = 4; a[5 + 2 * k] = kids[k]; }
                         return arr;
                     };
                     std::vector<U> buttons;
                     for (int k = 0; k < 4; ++k) if (r() % 2) buttons.push_back(list(names[k], {}));
                     const U section = list(r() % 5 ? key.c_str() : "other", buttons);
                     const U root = list("root", {section});
                     const U node = w.add(r, 8);
                     w.at(node)[0] = 4; w.at(node)[1] = root;
                     (void)self; (void)button;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     static const char* const names[] = {"ok", "cancel", "back", "next", "none"};
                     const U node = ch::addr(w.blocks.back().w.data());
                     U a[2] = {ch::addr(names[r() % 5]), ch::addr(w.blocks[1].w.data())};
                     return call_any(fn, ch::addr(w.blocks[0].w.data()), r() % 8 ? node : 0u, a, 2) & 0xff;
                 }});
    return c;
}

}  // namespace

namespace {
// runs every case on both sides; returns the number of functions that differed
int run_cases(const std::vector<Case>& list, const char* what)
{
    int failed = 0, total = 0;
    for (const Case& k : list) {
        std::mt19937 seq(k.va);
        int bad = 0;
        for (int it = 0; it < k.iters; ++it) {
            const std::uint32_t seed = seq();
            std::vector<std::uint32_t> snap[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                std::mt19937 r(seed);
                World w;
                w.side = side;
                w.blocks.reserve(64);
                vt::reset();
                k.slots(r);
                k.build(w, r);
                vt::norm() = [&w](std::uint32_t v) { return w.norm(v); };
                ch::wipe_stack();
                const std::uint32_t fn = side ? ch::addr(k.port) : k.va;
                // the original faults on some generated inputs; how the call ended must match, and a faulted call's
                // half-written state is not compared (as in arena_fuzz.h)
                std::uint32_t eax = 0;
                const unsigned outcome = wd::guarded([&] { eax = k.call(w, fn, r); });
                if (outcome != wd::kOk) {
                    snap[side] = {0xFA017000u, outcome == wd::kHung ? 1u : outcome};
                    rt::restore_pristine();
                    continue;
                }
                snap[side] = vt::log();
                snap[side].push_back(k.compare_eax ? w.norm(eax) : 0u);
                for (const auto& b : w.blocks)
                    for (std::uint32_t x : b.w) snap[side].push_back(w.norm(x));
            }
            if (snap[0] != snap[1]) {
                if (!bad) {  // evidence for the first differing case: where the snapshots part and with what
                    std::size_t i = 0;
                    while (i < snap[0].size() && i < snap[1].size() && snap[0][i] == snap[1][i]) ++i;
                    std::printf("  %-44s first diff at word %u of %u/%u (log %u): original %08x port %08x\n", k.name,
                                static_cast<unsigned>(i), static_cast<unsigned>(snap[0].size()), static_cast<unsigned>(snap[1].size()),
                                static_cast<unsigned>(snap[0].size() - (snap[0].size() - i)),
                                i < snap[0].size() ? snap[0][i] : 0u, i < snap[1].size() ? snap[1][i] : 0u);
                }
                ++bad;
            }
            ++total;
        }
        if (bad) ++failed;
        // every function it ran, 0 for a match: tools/verify_batch.py takes the line as the evidence the function was exercised
        std::printf("  %-44s %d of %d differ\n", k.name, bad, k.iters);
    }
    vt::norm() = [](std::uint32_t v) { return v; };
    rt::restore_pristine();
    std::printf("  %s: %d calls, %d functions differ\n", what, total, failed);
    return failed;
}
}  // namespace

TEST(native_ui_widgets_virtual_calls_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases(), "ui_widgets virtual-call cases"), 0);
}

// ImageWidget_SetImage (0x004b3e30; stack name; ret 4): null name -> 0. Else releases an owned image (+0x34 set, +0x3C
// through Image_FreeUnlessDefault, which clears it), loads the name (Image_Load; the names here are not found, so it
// returns 0), stores the result in +0x3C, sets +0x34 when loaded, redraws (slot 0x20) and returns +0x3C. Images: null,
// the default image 0x004e06e0, or a real 0x40-byte CRT block with no buffers or surface; free logged. Compared: the
// return, the call log, the widget's words (image by role), freed blocks by role.
TEST(native_ui_image_widget_set_image_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const std::uint32_t fn[2] = {0x004b3e30, ch::addr(reinterpret_cast<void*>(&recoil::ImageWidget_SetImage))};
    std::mt19937 rng(0x4b3e30);
    for (int it = 0; it < 300; ++it) {
        const int kind = static_cast<int>(rng() % 4);
        const std::uint32_t own = rng() % 2, name_kind = rng() % 4;
        std::uint32_t init[0x50 / 4];
        for (auto& w : init) w = rng() % 0x10000;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            vt::reset();
            vt::set(0x20, 0);
            ch::Roles rl;
            std::uint32_t obj[0x50 / 4];
            std::memcpy(obj, init, sizeof obj);
            obj[0] = vt::table();
            rl.set(obj, 1);
            rl.set(vt::table(), 2);
            const std::uint32_t def = side ? ch::addr(ch::img(1, 0x004e06e0)) : 0x004e06e0u;
            rl.set(def, 3);
            std::uint32_t img = 0;
            if (kind == 1) img = def;
            else if (kind > 1) {
                auto* p = static_cast<std::uint32_t*>(ch::c_malloc(0x40));
                std::memset(p, 0, 0x40);
                img = ch::addr(p);
                rl.set(img, 4);
            }
            obj[0x3c / 4] = img;
            obj[0x34 / 4] = own;
            vt::norm() = [&rl](std::uint32_t v) { return rl(v); };
            const char* name = name_kind == 0 ? nullptr : name_kind == 1 ? "no_such_image.tga" : "missing\\thing.pcx";
            std::uint32_t a[1] = {ch::addr(name)};
            ch::freed().clear();
            std::uint32_t ret;
            {
                ch::FreeHook hook;
                ret = call_any(fn[side], ch::addr(obj), 0, a, 1);
            }
            snap[side] = vt::log();
            snap[side].push_back(rl(ret));
            for (std::uint32_t w : obj) snap[side].push_back(rl(w));
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            if (kind > 1 && !ch::was_freed(img)) ch::c_free(img);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    vt::norm() = [](std::uint32_t v) { return v; };
    rt::restore_pristine();
}

// ScreenParticle_Respawn (0x004bdee0; stack index; ret 8): picks particle `index` (12 bytes) in the current buffer
// ([ECX+0x58 + 4*[ECX+0x60]]) and the previous one ([ECX+0x58 + 4*[ECX+0x64]]), respawns it from three rand() calls
// (msvcrt on both sides, seeded the same with srand before each side) with the depth clamps of the listing, and copies it
// to the previous buffer. Compared: both buffers.
TEST(native_ui_screen_particle_respawn_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int, std::uint32_t, std::uint32_t);
    const F fn[2] = {rt::original<F>(0x004bdee0), reinterpret_cast<F>(&recoil::ScreenParticle_Respawn)};
    const auto srand_ = ch::crt_fn<void(__cdecl*)(unsigned)>("srand");
    std::mt19937 rng(0x4bdee0);
    for (int it = 0; it < 2000; ++it) {
        const std::uint32_t cur = rng() % 2, prev = rng() % 2, index = rng() % 16, seed = rng();
        std::vector<float> init(2 * 16 * 3);
        for (float& f : init) f = static_cast<float>(static_cast<int>(rng() % 2000) - 1000) / 10.0f;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<float> buf = init;
            std::uint32_t obj[0x68 / 4] = {};
            obj[0x58 / 4] = ch::addr(&buf[0]);
            obj[0x5c / 4] = ch::addr(&buf[48]);
            obj[0x60 / 4] = cur;
            obj[0x64 / 4] = prev;
            srand_(seed);
            fn[side](obj, 0, index, 0);
            for (float f : buf) snap[side].push_back(ch::fbits(f));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

// Second wave: button / list / panel widgets built on the first wave's functions (P5 ui_widgets, cloud port). The
// same runner and fake table; child widgets sit in begin/end pointer vectors (+0x110, +0x120, +0x130, +0x140) or in
// arrays of embedded objects (stride 0x2ac / 0x2c0 / 0x4c) as the listings index them.
namespace {
using U = std::uint32_t;
// a vector of 0..3 child widget pointers at +off / +off+4 of p
void child_vector(World& w, std::mt19937& r, U p, U off, U child_bytes = 0x2a0)
{
    const U n = r() % 4;
    const U vec = w.add(r, 4 * 3);
    for (U k = 0; k < 3; ++k) {
        const U c = object(w, r, child_bytes);
        w.at(c)[0x144 / 4] = r() % 4;
        w.at(c)[0x270 / 4] = r() % 2;
        w.at(vec)[k] = c;
    }
    w.at(p)[off / 4] = n || r() % 2 ? vec : 0u;
    w.at(p)[off / 4 + 1] = vec + 4 * n;
}
// config-tree list node array: header {0, count}, then {type, value} children; the first child is the name string
U cfg_list(World& w, std::mt19937& r, const char* name, const std::vector<std::pair<U, U>>& kids)
{
    const U arr = w.add(r, 8 * (kids.size() + 2));
    U* a = w.at(arr);
    a[0] = 0; a[1] = static_cast<U>(kids.size() + 2);
    a[2] = 3; a[3] = ch::addr(name);
    for (std::size_t k = 0; k < kids.size(); ++k) { a[4 + 2 * k] = kids[k].first; a[5 + 2 * k] = kids[k].second; }
    return arr;
}
std::uint32_t call_self(World& w, U fn, std::initializer_list<U> args)
{
    std::vector<U> a(args);
    return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, a.data(), static_cast<int>(a.size()));
}

// the object pointer call_self passes (block 0), for cases that build their own argument list
U self(World& w) { return ch::addr(w.blocks[0].w.data()); }

std::vector<Case> cases2()
{
    std::vector<Case> c;
    auto tick_slots = [] { vt::set(0x4, 0); vt::set(0x8, 0); vt::set(0x60, 1); };
    c.push_back({"UiScreen_Slot_BroadcastToGroups", 0x00409410, (void*)&recoil::UiScreen_Slot_BroadcastToGroups, 300,
                 [tick_slots](std::mt19937&) { tick_slots(); vt::set(0x24, 1); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x160);
                     w.at(p)[0xc / 4] = r() % 64;
                     w.at(p)[0x10 / 4] = fbits(static_cast<float>(r() % 40) / 10.0f);
                     const U groups = r() % 3, g = w.add(r, 0x10 * 2);
                     for (U k = 0; k < 2; ++k) {
                         const U n = r() % 3, arr = w.add(r, 0x2ac * 2);
                         for (U e = 0; e < 2; ++e) w.at(arr)[0x2ac / 4 * e] = vt::table();
                         w.at(g)[4 * k + 1] = arr;
                         w.at(g)[4 * k + 2] = arr + 0x2ac * n;
                     }
                     w.at(p)[0x150 / 4] = g;
                     w.at(p)[0x154 / 4] = g + 0x10 * groups;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {fbits(static_cast<float>(r() % 30) / 10.0f)}); }, false});
    c.push_back({"Button_GetHitRect", 0x004b5350, (void*)&recoil::Button_GetHitRect, 400,
                 [](std::mt19937& r) { vt::set(0x64, 0, 0, 0, r() % 300); vt::set(0x68, 0, 0, 0, r() % 200); vt::set(0x90, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x120);
                     w.at(p)[0xc4 / 4] = r() % 4 ? 1u : 0u;
                     const U img = image(w, r);
                     w.at(p)[0x3c / 4] = r() % 3 ? 0u : img;
                     child_vector(w, r, p, 0x110);
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }});
    auto button_graph = [](World& w, std::mt19937& r) {
        const U p = object(w, r, 0x170);
        w.at(p)[0xc4 / 4] = r() % 2;
        for (U off : {0x110u, 0x120u, 0x130u, 0x140u}) child_vector(w, r, p, off, 0x80);
        for (U off : {0xdcu, 0xe0u, 0x158u, 0x15cu}) w.at(p)[off / 4] = r() % 3 ? image(w, r) : 0u;
        w.at(p)[0x14c / 4] = r() % 2;
        w.at(p)[0x150 / 4] = r() % 3 ? 0u : image(w, r);
        w.at(p)[0x154 / 4] = r() % 3 ? 0u : image(w, r);
        w.at(p)[0x160 / 4] = r() % 3 ? object(w, r, 0x40) : 0u;
        w.at(p)[0xec / 4] = r() % 2;
    };
    auto button_slots = [](std::mt19937&) { vt::set(0x60, 1); vt::set(0x20, 0); };
    c.push_back({"ToggleButton_Slot_RefreshVisuals", 0x004b5740, (void*)&recoil::ToggleButton_Slot_RefreshVisuals, 300, button_slots, button_graph,
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    c.push_back({"Button_Reset", 0x004b5860, (void*)&recoil::Button_Reset, 300, button_slots, button_graph,
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    c.push_back({"ToggleImageButton_Slot_RefreshVisuals", 0x004b70c0, (void*)&recoil::ToggleImageButton_Slot_RefreshVisuals, 300, button_slots, button_graph,
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    c.push_back({"Toggle_SetState", 0x004b72c0, (void*)&recoil::Toggle_SetState, 300, button_slots, button_graph,
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r() % 3 ? r() % 2 : r()}); }});
    c.push_back({"RadioGroup_Slot_UpdateButtons", 0x004b7e60, (void*)&recoil::RadioGroup_Slot_UpdateButtons, 300,
                 [tick_slots](std::mt19937&) { tick_slots(); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x210);
                     w.at(p)[0xc / 4] = r() % 64;
                     w.at(p)[0x10 / 4] = fbits(static_cast<float>(r() % 40) / 10.0f);
                     w.at(p)[0x150 / 4] = r() % 5;
                     w.at(p)[0x14c / 4] = r() % 5;
                     for (U k = 0; k < 4; ++k) {
                         w.at(p)[0x168 / 4 + k] = r() % 3 ? object(w, r, 0x40) : 0u;
                         w.at(p)[0x1b8 / 4 + k] = r() % 3 ? object(w, r, 0x40) : 0u;
                     }
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {fbits(static_cast<float>(r() % 30) / 10.0f)}); }, false});
    auto list_label = [](World& w, std::mt19937& r) {
        const U p = object(w, r, 0x2c0);
        w.at(p)[0xc / 4] = r() % 2 ? 0u : 0x10u;
        w.at(p)[0x270 / 4] = r() % 2;
        return p;
    };
    c.push_back({"ListWidget_HitTest", 0x004bb3d0, (void*)&recoil::ListWidget_HitTest, 400, [](std::mt19937&) { vt::set(0x90, 0); },
                 [list_label](World& w, std::mt19937& r) { list_label(w, r); },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r() % 0x10000, r() % 0x10000}); }});
    c.push_back({"ListWidget_GetRect", 0x004bb740, (void*)&recoil::ListWidget_GetRect, 300,
                 [](std::mt19937& r) { vt::set(0x64, 0, 0, 0, r() % 300); vt::set(0x68, 0, 0, 0, r() % 200); vt::set(0x90, 0); },
                 [list_label](World& w, std::mt19937& r) { list_label(w, r); w.add(r, 0x10); },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {ch::addr(w.blocks[1].w.data())}); }, false});
    auto list_rows = [list_label](World& w, std::mt19937& r) {
        const U p = list_label(w, r);
        const U n = r() % 4, arr = w.add(r, 0x2c0 * 4);
        for (U k = 0; k < 4; ++k) { w.at(arr)[0x2c0 / 4 * k] = vt::table(); w.at(arr)[(0x2c0 * k + 0x270) / 4] = r() % 2; }
        w.at(p)[0x2ac / 4] = n || r() % 2 ? arr : 0u;
        w.at(p)[0x2b0 / 4] = arr + 0x2c0 * n;
        w.at(p)[0x2a4 / 4] = r() % 4;
    };
    c.push_back({"ListWidget_SetPosition", 0x004bb9f0, (void*)&recoil::ListWidget_SetPosition, 300,
                 [](std::mt19937& r) { vt::set(0x20, 0); vt::set(0x90, 0); vt::set(0x64, 0, 0, 0, r() % 300); vt::set(0x68, 0, 0, 0, r() % 200); vt::set(0xc, 2); },
                 list_rows, [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {pick(r), pick(r)}); }, false});
    c.push_back({"ListWidget_Slot_AppendRow", 0x004bbb20, (void*)&recoil::ListWidget_Slot_AppendRow, 300,
                 [](std::mt19937&) { vt::set(0x20, 0); vt::set(0x90, 0); vt::set(0x8c, 1); }, list_rows,
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    // the panel: the self object calls slot 4 with one argument; four embedded labels at +0x10 / +0x2b4 / +0x558 /
    // +0x7fc whose text (+0x15c) is a short NUL-terminated string
    c.push_back({"MessagePanel_PushLine", 0x004bd160, (void*)&recoil::MessagePanel_PushLine, 300,
                 [](std::mt19937&) { vt::set(0x4, 1); vt::set(0x60, 1); vt::set(0x74, 0, 2); vt::set(0x90, 0); },
                 [](World& w, std::mt19937& r) {
                     static const char* const texts[] = {"hello", "world", "", "hello"};
                     const U p = object(w, r, 0xab0, {0x10, 0x2b4, 0x558, 0x7fc});
                     w.at(p)[0x1c / 4] = r() % 2 ? 0u : 0x10u;
                     for (U base : {0x10u, 0x2b4u, 0x558u, 0x7fcu}) {
                         char* t = reinterpret_cast<char*>(w.at(p)) + base + 0x15c;
                         std::strcpy(t, texts[r() % 4]);
                         w.at(p)[(base + 0xc) / 4] = r() % 2 ? 0u : 0x10u;
                         w.at(p)[(base + 0x10) / 4] = fbits(static_cast<float>(r() % 40) / 10.0f);
                         w.at(p)[(base + 0x270) / 4] = 0;
                     }
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     static const char* const q[] = {"hello", "world", "other"};
                     return call_self(w, fn, {ch::addr(q[r() % 3]), fbits(static_cast<float>(r() % 50) / 10.0f)});
                 }});
    c.push_back({"ScreenFX_AllocateRectSlot", 0x004bed90, (void*)&recoil::ScreenFX_AllocateRectSlot, 300, [](std::mt19937&) { vt::set(0xc, 2); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x200);
                     w.at(p)[0x1ec / 4] = r() % 6;
                     for (U k = 0; k < 5; ++k) w.at(p)[(0x70 + 0x4c * k) / 4] = vt::table();
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {pick(r), pick(r), pick(r), pick(r), pick(r), pick(r), pick(r)}); }, false});
    // the text: +0x14c text buffer (+4 characters, +8 capacity, +0xc cursor), then the first row (+0x110 vector) gets
    // slot 0x8c with the text and the widget redraws
    c.push_back({"EditField_SetText", 0x004b4e60, (void*)&recoil::EditField_SetText, 300, [](std::mt19937&) { vt::set(0x8c, 1); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x170);
                     const U buf = w.add(r, 0x48);
                     reinterpret_cast<char*>(w.at(buf))[0] = 0;
                     w.at(p)[(0x14c + 4) / 4] = buf;
                     w.at(p)[(0x14c + 8) / 4] = 1 + r() % 0x40;
                     w.at(p)[(0x14c + 0xc) / 4] = 0;
                     child_vector(w, r, p, 0x110, 0x40);
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     static const char* const q[] = {"", "abc", "a longer line of text for the field"};
                     return call_self(w, fn, {ch::addr(q[r() % 3])});
                 }, false});
    // ScreenBase_BindWidget (ret 0xc: unused, widget, name): the tree [+0xa940] -> section named at 0x004e4864 -> the
    // widget's section (by name) -> keys (the strings at 0x004e4708 / 4710 / 4858 / 475c / 0x004db428 / 0x004e484c /
    // 4840) each a list whose children 1 and 2 are ints; the widget is appended to the screen's list (+8 / +0xc),
    // positioned, sized, given a font from the screen's table (+0x1cec) and a colour, then slot 0x18 with the
    // screen's +0x118 and its extent. The image key's value is a name that is not found (Image_Load returns 0).
    c.push_back({"ScreenBase_BindWidget", 0x004ba0e0, (void*)&recoil::ScreenBase_BindWidget, 300,
                 [](std::mt19937& r) {
                     vt::set(0xc, 2); vt::set(0x6c, 1, 1, 4); vt::set(0x80, 7); vt::set(0x18, 2, 2);
                     vt::set(0x64, 0, 0, 0, r() % 300); vt::set(0x68, 0, 0, 0, r() % 200); vt::set(0x20, 0);
                 },
                 [](World& w, std::mt19937& r) {
                     static const char* const wnames[] = {"title", "okbtn", "list"};
                     auto s = [](U va) { return reinterpret_cast<const char*>(static_cast<std::uintptr_t>(va)); };
                     const U self = object(w, r, 0xa960);
                     const U widget = object(w, r, 0x2a4);
                     w.at(widget)[0x34 / 4] = 0;
                     w.at(widget)[0x3c / 4] = 0;
                     w.at(self)[2] = 0; w.at(self)[3] = 0;
                     for (U k = 0; k < 4; ++k) w.at(self)[(0x1cec + 0x24 * k) / 4] = r() % 3 ? 1u : 0u;
                     std::vector<std::pair<U, U>> keys;
                     const U keyvas[] = {0x004e4708, 0x004e4710, 0x004e4858, 0x004e475c, 0x004db428, 0x004e484c, 0x004e4840};
                     for (U va : keyvas) {
                         if (r() % 3 == 0) continue;
                         const U v1 = va == 0x004e4708 ? ch::addr("no_such_image.tga") : (va == 0x004e475c ? r() % 4 : r() % 300);
                         keys.push_back({4, cfg_list(w, r, s(va), {{va == 0x004e4708 ? 3u : 1u, v1}, {1, r() % 300}, {1, r() % 300}})});
                     }
                     const U wsec = cfg_list(w, r, wnames[r() % 3], keys);
                     const U sec = cfg_list(w, r, r() % 5 ? s(0x004e4864) : "other", {{4, wsec}});
                     const U root = cfg_list(w, r, "root", {{4, sec}});
                     const U node = w.add(r, 8);
                     w.at(node)[0] = 4; w.at(node)[1] = root;
                     w.at(self)[0xa940 / 4] = r() % 8 ? node : 0u;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     static const char* const wnames[] = {"title", "okbtn", "list", "none"};
                     return call_self(w, fn, {r(), ch::addr(w.blocks[1].w.data()), ch::addr(wnames[r() % 4])});
                 }});
    // wrappers over first-wave functions, on the same graphs
    auto first = [](const char* name) {
        for (const Case& k : cases()) if (std::strcmp(k.name, name) == 0) return k;
        return Case{};
    };
    {
        Case k = first("ListScreen_ScrollTo");
        k.name = "Thunk_004b9330"; k.va = 0x004b9320; k.port = (void*)&recoil::Thunk_004b9330;
        c.push_back(k);
    }
    {
        Case k = first("ListWidget_ClearRange");
        k.name = "ListWidget_ClearAll"; k.va = 0x004bbe90; k.port = (void*)&recoil::ListWidget_ClearAll;
        k.call = [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); };
        c.push_back(k);
    }
    {
        Case k = first("Manager_BroadcastVirtual24");
        k.name = "Manager_0056bd58_ResetTimer"; k.va = 0x004bed30; k.port = (void*)&recoil::Manager_0056bd58_ResetTimer;
        auto build = k.build;
        k.build = [build](World& w, std::mt19937& r) { build(w, r); w.blocks[0].w.resize(0x200 / 4, 0x1234); };
        c.push_back(k);
    }
    c.push_back({"EditField_SetCaretShape", 0x004b4810, (void*)&recoil::EditField_SetCaretShape, 300, [](std::mt19937&) { vt::set(0xc, 2); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) { const U p = object(w, r, 0x100); w.at(p)[0xdc / 4] = r() % 8; },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {pick(r), pick(r), r() % 40, r() % 40}); }, false});
    c.push_back({"ScrollGroupChild_SetSelected", 0x004b8a90, (void*)&recoil::ScrollGroupChild_SetSelected, 300, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x180);
                     w.at(p)[0xc4 / 4] = r() % 3 ? 1u : 0u;
                     for (U off : {0x16cu, 0x170u, 0x174u, 0x178u}) w.at(p)[off / 4] = r() % 3 ? image(w, r) : 0u;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r() % 3 ? r() % 2 : r()}); }, false});
    // ScreenBase_BindButton (ret 0xc: unused, name, button): ScreenBase_BindButtonImpl on the screen's tree [+0xa940]
    c.push_back({"ScreenBase_BindButton", 0x004ba0c0, (void*)&recoil::ScreenBase_BindButton, 300, [](std::mt19937&) { vt::set(0x7c, 2); vt::set(0x80, 0); },
                 [](World& w, std::mt19937& r) {
                     static const char* const names[] = {"ok", "cancel", "back", "next"};
                     auto s = [](U va) { return reinterpret_cast<const char*>(static_cast<std::uintptr_t>(va)); };
                     const U self = object(w, r, 0xa950);
                     object(w, r, 0x40);
                     std::vector<std::pair<U, U>> buttons;
                     for (int k = 0; k < 4; ++k) if (r() % 2) buttons.push_back({4, cfg_list(w, r, names[k], {})});
                     const U sec = cfg_list(w, r, r() % 5 ? s(0x004e4838) : "other", buttons);
                     const U root = cfg_list(w, r, "root", {{4, sec}});
                     const U node = w.add(r, 8);
                     w.at(node)[0] = 4; w.at(node)[1] = root;
                     w.at(self)[0xa940 / 4] = r() % 8 ? node : 0u;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     static const char* const names[] = {"ok", "cancel", "back", "next", "none"};
                     return call_self(w, fn, {r(), ch::addr(names[r() % 5]), ch::addr(w.blocks[1].w.data())});
                 }});
    // ScreenFX_QueueFade (ECX a short, stack two words; ret 8): ScreenFX_SetPrimaryRect on the global panel 0x0056bd58,
    // whose embedded widget +0x28 gets the fake table here; the panel's words are logged after the call
    c.push_back({"ScreenFX_QueueFade", 0x004beee0, (void*)&recoil::ScreenFX_QueueFade, 200, [](std::mt19937&) { vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) { w.add(r, 4); *ch::img(w.side, 0x0056bd58 + 0x28) = vt::table(); },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[2] = {pick(r), pick(r)};
                     const U ret = call_any(fn, r() & 0xffff, 0, a, 2);
                     for (U k = 0; k < 0x80 / 4; ++k) vt::log().push_back(w.norm(*ch::img(w.side, 0x0056bd58 + 4 * k)));
                     return ret;
                 },
                 false});
    return c;
}
}  // namespace

TEST(native_ui_widgets_virtual_calls_wave2_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases2(), "ui_widgets wave-2 virtual-call cases"), 0);
}

// Third wave: wrappers over the second wave and the global panels (P5 ui_widgets, cloud port).
namespace {
Case find_case(const std::vector<Case>& list, const char* name)
{
    for (const Case& k : list) if (std::strcmp(k.name, name) == 0) return k;
    return Case{};
}
Case rename(Case k, const char* name, U va, void* port)
{
    k.name = name; k.va = va; k.port = port;
    return k;
}

std::vector<Case> cases3()
{
    const std::vector<Case> w2 = cases2();
    std::vector<Case> c;
    {
        Case k = rename(find_case(w2, "Button_Reset"), "Toggle_OnReleaseIfOff", 0x004b87e0, (void*)&recoil::Toggle_OnReleaseIfOff);
        c.push_back(k);  // +0x14c (the toggle state) is random in the button graph: set -> returns, clear -> Button_Reset
    }
    {
        Case k = rename(find_case(w2, "Button_GetHitRect"), "Cycler_GetLabel", 0x004b8af0, (void*)&recoil::Cycler_GetLabel);
        auto build = k.build;
        k.build = [build](World& w, std::mt19937& r) { build(w, r); w.blocks[0].w.resize(0x180 / 4, 0); w.blocks[0].w[0x158 / 4] = r() % 2; };
        c.push_back(k);
    }
    // ScrollGroup_Select (ret 4): +0x178 = the index; each of the ten children +0x150.. (or null) is selected when its
    // index matches (ScrollGroupChild_SetSelected)
    c.push_back({"ScrollGroup_Select", 0x004b8cf0, (void*)&recoil::ScrollGroup_Select, 300, [](std::mt19937&) { vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x180);
                     for (U k = 0; k < 10; ++k) {
                         if (r() % 3 == 0) { w.at(p)[0x150 / 4 + k] = 0; continue; }
                         const U ch_ = object(w, r, 0x180);
                         w.at(ch_)[0xc4 / 4] = r() % 3 ? 1u : 0u;
                         for (U off : {0x16cu, 0x170u, 0x174u, 0x178u}) w.at(ch_)[off / 4] = r() % 3 ? image(w, r) : 0u;
                         w.at(p)[0x150 / 4 + k] = ch_;
                     }
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r() % 12}); }});
    // IntEditField_ParseClamped: atoi of the field's text (EditField_GetText) or the minimum +0x374 for an empty one,
    // clamped to +0x374..+0x378, written back with sprintf("%d") and EditField_SetText; returns the clamped value
    c.push_back({"IntEditField_ParseClamped", 0x0041a2d0, (void*)&recoil::IntEditField_ParseClamped, 400, [](std::mt19937&) { vt::set(0x8c, 1); vt::set(0x20, 0); },
                 [](World& w, std::mt19937& r) {
                     static const char* const texts[] = {"", "0", "42", "-17", "99999", "12abc", "abc"};
                     const U p = object(w, r, 0x380);
                     const U buf = w.add(r, 0x48);
                     std::strcpy(reinterpret_cast<char*>(w.at(buf)), texts[r() % 7]);
                     w.at(p)[(0x14c + 4) / 4] = buf;
                     w.at(p)[(0x14c + 8) / 4] = 0x40;
                     w.at(p)[(0x14c + 0xc) / 4] = 0;
                     const int lo = static_cast<int>(r() % 200) - 100;
                     w.at(p)[0x374 / 4] = static_cast<U>(lo);
                     w.at(p)[0x378 / 4] = static_cast<U>(lo + static_cast<int>(r() % 300));
                     child_vector(w, r, p, 0x110, 0x40);
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }});
    // MessageOverlay_Show (ECX text, stack float; ret 4): MessagePanel_PushLine on the panel [0x0056bd24]
    {
        Case k = rename(find_case(w2, "MessagePanel_PushLine"), "MessageOverlay_Show", 0x004bd280, (void*)&recoil::MessageOverlay_Show);
        auto build = k.build;
        k.build = [build](World& w, std::mt19937& r) { build(w, r); *ch::img(w.side, 0x0056bd24) = ch::addr(w.blocks[0].w.data()); };
        k.call = [](World& w, U fn, std::mt19937& r) {
            static const char* const q[] = {"hello", "world", "other"};
            U a[1] = {fbits(static_cast<float>(r() % 50) / 10.0f)};
            return call_any(fn, ch::addr(q[r() % 3]), 0, a, 1);
        };
        c.push_back(k);
    }
    // the global panel 0x0056bd58: its rect slots (+0x70 + 0x4c*k) and child list (+4 / +8) get this test's table
    c.push_back({"ScreenFX_QueueFlashRect", 0x004bef10, (void*)&recoil::ScreenFX_QueueFlashRect, 200, [](std::mt19937&) { vt::set(0xc, 2); vt::set(0x60, 1); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     for (U k = 0; k < 5; ++k) *ch::img(w.side, 0x0056bd58 + 0x70 + 0x4c * k) = vt::table();
                     *ch::img(w.side, 0x0056bd58 + 0x1ec) = r() % 6;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[5] = {pick(r), pick(r), pick(r), pick(r), pick(r)};
                     const U ret = call_any(fn, pick(r), pick(r), a, 5);
                     for (U k = 0; k < 0x1f0 / 4; ++k) vt::log().push_back(w.norm(*ch::img(w.side, 0x0056bd58 + 4 * k)));
                     return ret;
                 },
                 false});
    c.push_back({"Manager_0056bd58_Tick", 0x004bef70, (void*)&recoil::Manager_0056bd58_Tick, 200, [](std::mt19937&) { vt::set(0x24, 1); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     const int n = static_cast<int>(r() % 4);
                     U prev = 0;
                     for (int k = 0; k < n; ++k) { const U o = object(w, r, 0x20); w.at(o)[1] = prev; prev = o; }
                     *ch::img(w.side, 0x0056bd58 + 8) = prev;
                     *ch::img(w.side, 0x0056bd58 + 4) = r() % 4 ? 1u : 0u;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[1] = {pick(r)};
                     const U ret = call_any(fn, 0, 0, a, 1);
                     vt::log().push_back(*ch::img(w.side, 0x0056bd58 + 0x1ec));
                     return ret;
                 },
                 false});
    return c;
}
}  // namespace

TEST(native_ui_widgets_virtual_calls_wave3_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases3(), "ui_widgets wave-3 virtual-call cases"), 0);
}

// Fourth wave: option refreshers over the settings getters, text measurement, draw slots (P5 ui_widgets, cloud port).
// Settings: every pointer in 0x004e5d00..0x004e5d5c points at a cell of this test (small ints; 0x004e5d44 a float)
// and [0x004e5dcc] picks the live or the stored set. Fonts: the 20-entry table 0x0056179c holds 0x610-byte records of
// small words. Drawing hooks ([0x00632100] plot, [0x006320b0] span fill, [0x006320f0] line, [0x006320f8] polyline)
// are fake-table thunks 36 / 34 / 30 / 31; the span-visibility hooks are not reached by these.
namespace {
void settings_cells(World& w, std::mt19937& r)
{
    const U cells = w.add(r, 4 * 24, true);
    for (U k = 0; k < 24; ++k) *ch::img(w.side, 0x004e5d00 + 4 * k) = cells + 4 * k;
    w.at(cells)[0x44 / 4] = fbits(static_cast<float>(r() % 100) / 10.0f);
    *ch::img(w.side, 0x004e5dcc) = r() % 2;
}
void fonts(World& w, std::mt19937& r)
{
    const U f = w.add(r, 3 * 0x610, true), im = w.add(r, 3 * 0x10, true);
    for (U k = 0; k < 3; ++k) w.at(f)[k * 0x610 / 4] = im + 0x10 * k;
    for (U k = 0; k < 20; ++k) *ch::img(w.side, 0x0056179c + 4 * k) = r() % 4 ? f + 0x610 * (r() % 3) : 0u;
}
void draw_hooks(World& w, std::mt19937& r)
{
    const U* t = ch::at(vt::table());
    *ch::img(w.side, 0x00632100) = t[36];
    *ch::img(w.side, 0x006320b0) = t[34];
    *ch::img(w.side, 0x006320f0) = t[30];
    *ch::img(w.side, 0x006320f8) = t[31];
    *ch::img(w.side, 0x00632050) = r() % 0x10000;
    *ch::img(w.side, 0x0063205c) = 1280;
    *ch::img(w.side, 0x00632060) = 2;
    *ch::img(w.side, 0x0057dac8) = r() % 2;
}
void draw_slots() { vt::set(36 * 4, 2); vt::set(34 * 4, 0); vt::set(30 * 4, 4); vt::set(31 * 4, 5); vt::set(0x8, 0); }

std::vector<Case> cases4()
{
    const std::vector<Case> w2 = cases2();
    std::vector<Case> c;
    const Case toggle = find_case(w2, "Toggle_SetState");
    struct T { const char* name; U va; void* port; };
    const T toggles[] = {
        {"OptionToggle_Refresh_Bit10", 0x0040c9c0, (void*)&recoil::OptionToggle_Refresh_Bit10},
        {"OptionToggle_Refresh_Bit08", 0x0040ca20, (void*)&recoil::OptionToggle_Refresh_Bit08},
        {"OptionToggle_Refresh_00408360Is2", 0x0040ca80, (void*)&recoil::OptionToggle_Refresh_00408360Is2},
        {"OptionToggle_Refresh_Not00408060", 0x0040cb90, (void*)&recoil::OptionToggle_Refresh_Not00408060},
        {"OptionToggle_Refresh_From00408220", 0x0040cc60, (void*)&recoil::OptionToggle_Refresh_From00408220},
    };
    for (const T& t : toggles) {
        Case k = rename(toggle, t.name, t.va, t.port);
        auto build = k.build;
        k.build = [build](World& w, std::mt19937& r) { build(w, r); settings_cells(w, r); };
        k.call = [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); };
        k.compare_eax = false;
        c.push_back(k);
    }
    const T cyclers[] = {
        {"OptionCycler_Refresh_From00408030", 0x0040cab0, (void*)&recoil::OptionCycler_Refresh_From00408030},
        {"OptionCycler_Refresh_From00408100", 0x0040caf0, (void*)&recoil::OptionCycler_Refresh_From00408100},
        {"OptionCycler_Refresh_HWCardGated", 0x0040cb30, (void*)&recoil::OptionCycler_Refresh_HWCardGated},
        {"OptionCycler_Refresh_From004080d0", 0x0040cbd0, (void*)&recoil::OptionCycler_Refresh_From004080d0},
    };
    for (const T& t : cyclers)
        c.push_back({t.name, t.va, t.port, 300, [](std::mt19937&) {},
                     [](World& w, std::mt19937& r) {
                         const U p = object(w, r, 0x160);
                         w.at(p)[0x14c / 4] = r() % 8; w.at(p)[0x150 / 4] = r() % 8; w.at(p)[0x154 / 4] = r() % 8; w.at(p)[0x158 / 4] = r() % 8;
                         settings_cells(w, r);
                     },
                     [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    c.push_back({"OptionControl_Refresh_FloatFrom00408090", 0x0040cc10, (void*)&recoil::OptionControl_Refresh_FloatFrom00408090, 100,
                 [](std::mt19937&) { vt::set(0x84, 1); },
                 [](World& w, std::mt19937& r) { object(w, r, 0x40); settings_cells(w, r); },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    c.push_back({"Widget_Slot_Virtual08_Then00498fb0", 0x004bc4c0, (void*)&recoil::Widget_Slot_Virtual08_Then00498fb0, 200, [](std::mt19937&) { draw_slots(); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x40);
                     w.at(p)[0x14 / 4] = r() % 640; w.at(p)[0x18 / 4] = r() % 480; w.at(p)[0x34 / 4] = r() % 40; w.at(p)[0x3c / 4] = r() % 0x10000;
                     draw_hooks(w, r);
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    auto text_widget = [](World& w, std::mt19937& r) {
        static const char* const texts[] = {"", "Hi", "Two words", "line one\nline two", "A!{}~"};
        const U p = object(w, r, 0x140);
        std::strcpy(reinterpret_cast<char*>(w.at(p)) + 0x34, texts[r() % 5]);
        w.at(p)[0x134 / 4] = r() % 20;
        w.at(p)[0xc / 4] = r() % 2 ? 0u : 0x10u;
        fonts(w, r);
    };
    c.push_back({"TextWidget_UpdateBoundsFromText", 0x004bcd80, (void*)&recoil::TextWidget_UpdateBoundsFromText, 300, [](std::mt19937&) {}, text_widget,
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    c.push_back({"TextWidget_MeasureWidth", 0x004bcdc0, (void*)&recoil::TextWidget_MeasureWidth, 300, [](std::mt19937&) {}, text_widget,
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }});
    c.push_back({"TextWidget_HitTest", 0x004bcea0, (void*)&recoil::TextWidget_HitTest, 300, [](std::mt19937&) {}, text_widget,
                 [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {r() % 0x1000, r() % 0x1000}); }});
    c.push_back({"Widget_Slot_Virtual08_Then004936d0IfField4C", 0x004bcff0, (void*)&recoil::Widget_Slot_Virtual08_Then004936d0IfField4C, 300,
                 [](std::mt19937&) { draw_slots(); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0x140);
                     float* v = reinterpret_cast<float*>(w.at(p)) + 0x34 / 4;
                     const int n = 3 + static_cast<int>(r() % 6);
                     for (int k = 0; k < n * 3 && k < 0xf0 / 4; ++k) v[k] = static_cast<float>(r() % 600);
                     w.at(p)[0x130 / 4] = r() % 3 ? static_cast<U>(n) : 0u;
                     w.at(p)[0x134 / 4] = r() % 0x10000;
                     draw_hooks(w, r);
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    c.push_back({"LineWidget_Slot_Draw", 0x004bf900, (void*)&recoil::LineWidget_Slot_Draw, 300, [](std::mt19937&) { draw_slots(); },
                 [](World& w, std::mt19937& r) {
                     const U p = object(w, r, 0xf0);
                     for (U k = 0; k < 20; ++k) w.at(p)[0x34 / 4 + k] = r() % 600;
                     w.at(p)[0xdc / 4] = r() % 11;
                     w.at(p)[0xe4 / 4] = r() % 2;
                     w.at(p)[0xe0 / 4] = r() % 0x10000;
                     draw_hooks(w, r);
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    // ListScreen_Slot_ReselectIfActive: the dialog [+8] (when set) re-selects index [+0x2a4] (SaveLoadDialog_SelectIndex)
    c.push_back({"ListScreen_Slot_ReselectIfActive", 0x00435a10, (void*)&recoil::ListScreen_Slot_ReselectIfActive, 200,
                 [](std::mt19937&) { vt::set(0x74, 0, 3); vt::set(0x60, 1); vt::set(0x20, 0); vt::set(0x8c, 1); },
                 [](World& w, std::mt19937& r) {
                     const U row = w.add(r, 0x2a8);
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
                     for (U k = 0; k < 5; ++k) std::snprintf(reinterpret_cast<char*>(w.at(recs)) + 0x140 * k + 0x2c, 0x20, "save%u", k);
                     w.at(p)[0xca00 / 4] = n || r() % 2 ? recs : 0u;
                     w.at(p)[0xca04 / 4] = recs + 0x140 * n;
                     w.at(row)[2] = r() % 5 ? p : 0u;
                     w.at(row)[0x2a4 / 4] = r() % 7;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_self(w, fn, {}); }, false});
    return c;
}
}  // namespace

TEST(native_ui_widgets_virtual_calls_wave4_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases4(), "ui_widgets wave-4 cases"), 0);
}

// Fifth wave: Thunk_Widget_Slot_Virtual08_Then00498fb0 (0x00403c80, a jump to 0x004bc4c0), LineWidget_Slot_TickBlink
// (0x004b47b0; stack dt; ret 4: counts the blink timer +0x100 down by dt, flips +0x104 and reloads from +0xfc when it
// runs out, draws while +0x104 is 1 or blinking is off (+0xf8 0); nothing when hidden), TextWidget_CentreInBox
// (0x004bcdf0: x = centre of +0x13c..+0x140 minus half the text width; with +0x1c set, the bounds are refreshed).
TEST(native_ui_widgets_virtual_calls_wave5_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const std::vector<Case> w4 = cases4();
    std::vector<Case> c;
    c.push_back(rename(find_case(w4, "Widget_Slot_Virtual08_Then00498fb0"), "Thunk_Widget_Slot_Virtual08_Then00498fb0", 0x00403c80,
                       (void*)&recoil::Thunk_Widget_Slot_Virtual08_Then00498fb0));
    {
        Case k = rename(find_case(w4, "LineWidget_Slot_Draw"), "LineWidget_Slot_TickBlink", 0x004b47b0, (void*)&recoil::LineWidget_Slot_TickBlink);
        auto build = k.build;
        k.build = [build](World& w, std::mt19937& r) {
            build(w, r);
            U* p = w.blocks[0].w.data();
            w.blocks[0].w.resize(0x110 / 4, 0);
            p = w.blocks[0].w.data();
            p[0xc / 4] = r() % 3 ? 0u : 0x10u;
            p[0xf8 / 4] = r() % 3 ? 1u : 0u;
            p[0xfc / 4] = fbits(static_cast<float>(r() % 20) / 10.0f);
            p[0x100 / 4] = fbits(static_cast<float>(r() % 20) / 10.0f - 0.5f);
            p[0x104 / 4] = r() % 2 ? 1u : static_cast<U>(-1);
        };
        k.call = [](World& w, U fn, std::mt19937& r) { return call_self(w, fn, {fbits(static_cast<float>(r() % 10) / 10.0f)}); };
        c.push_back(k);
    }
    {
        Case k = rename(find_case(w4, "TextWidget_UpdateBoundsFromText"), "TextWidget_CentreInBox", 0x004bcdf0, (void*)&recoil::TextWidget_CentreInBox);
        auto build = k.build;
        k.build = [build](World& w, std::mt19937& r) {
            build(w, r);
            w.blocks[0].w.resize(0x148 / 4, 0);
            U* p = w.blocks[0].w.data();
            p[0x13c / 4] = r() % 300;
            p[0x140 / 4] = p[0x13c / 4] + r() % 400;
            p[0x1c / 4] = r() % 2;
        };
        c.push_back(k);
    }
    CHECK_EQ(run_cases(c, "ui_widgets wave-5 cases"), 0);
}

// Sixth wave: the label text setters (the slot-0x74 implementations). ListLabel_Printf (0x004bb540; cdecl label, format,
// ...), ListLabel_VPrintf (0x004bb5e0; cdecl label, format, va_list), TextWidget_Printf (0x004bccf0; cdecl label,
// format, ...), ListLabel_SetText (0x004bb680; thiscall text; ret 4): a null format / text clears the 0x100-byte text
// +0x34; else the text is formatted (_vsnprintf 0x100 / vsprintf) or copied (strncpy 0x100) and, when it differs from
// the last shown text +0x15c (strncmp), the widget is re-centred (TextWidget_CentreInBox, when +0x138), redrawn (slot
// 0x20), marked (+0x270) and the text remembered. Formats with %d / %s arguments; texts equal to the last one or not.
TEST(native_ui_widgets_label_text_setters_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    std::vector<Case> c;
    auto label = [](World& w, std::mt19937& r) {
        static const char* const last[] = {"", "Score 5", "Hello", "Lives: 3 of 7"};
        const U p = object(w, r, 0x280);
        std::memset(reinterpret_cast<char*>(w.at(p)) + 0x34, 0, 0x100);
        std::strcpy(reinterpret_cast<char*>(w.at(p)) + 0x15c, last[r() % 4]);
        w.at(p)[0x134 / 4] = r() % 20;
        w.at(p)[0x138 / 4] = r() % 2;
        w.at(p)[0x13c / 4] = r() % 300;
        w.at(p)[0x140 / 4] = w.at(p)[0x13c / 4] + r() % 400;
        w.at(p)[0x1c / 4] = r() % 2;
        fonts(w, r);
    };
    static const char* const formats[] = {"Score %d", "Hello", "Lives: %d of %d", "%s", nullptr};
    static const char* const words[] = {"x", "Hello", "a longer argument string"};
    auto printf_call = [](World& w, U fn, std::mt19937& r) {
        const char* f = formats[r() % 5];
        U a[4] = {self(w), ch::addr(f), r() % 3 == 0 ? ch::addr(words[r() % 3]) : r() % 10, r() % 10};
        if (f && std::strcmp(f, "%s") != 0 && a[2] > 0xffff) a[2] = 5;
        if (f && std::strcmp(f, "%s") == 0) a[2] = ch::addr(words[r() % 3]);
        return call_any(fn, 0, 0, a, 4);
    };
    c.push_back({"ListLabel_Printf", 0x004bb540, (void*)&recoil::ListLabel_Printf, 400, [](std::mt19937&) { vt::set(0x20, 0); }, label, printf_call, false});
    c.push_back({"TextWidget_Printf", 0x004bccf0, (void*)&recoil::TextWidget_Printf, 400, [](std::mt19937&) { vt::set(0x20, 0); }, label, printf_call, false});
    c.push_back({"ListLabel_VPrintf", 0x004bb5e0, (void*)&recoil::ListLabel_VPrintf, 400, [](std::mt19937&) { vt::set(0x20, 0); }, label,
                 [](World& w, U fn, std::mt19937& r) {
                     const char* f = formats[r() % 5];
                     static U va[2];
                     va[0] = f && std::strcmp(f, "%s") == 0 ? ch::addr(words[r() % 3]) : r() % 10;
                     va[1] = r() % 10;
                     U a[3] = {self(w), ch::addr(f), ch::addr(va)};
                     return call_any(fn, 0, 0, a, 3);
                 },
                 false});
    c.push_back({"ListLabel_SetText", 0x004bb680, (void*)&recoil::ListLabel_SetText, 400, [](std::mt19937&) { vt::set(0x20, 0); }, label,
                 [](World& w, U fn, std::mt19937& r) {
                     static const char* const texts[] = {"", "Score 5", "Hello", "New text", nullptr};
                     return call_self(w, fn, {ch::addr(texts[r() % 5])});
                 },
                 false});
    CHECK_EQ(run_cases(c, "label text setters"), 0);
}
