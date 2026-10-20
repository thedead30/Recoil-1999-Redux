// Structured native L1 test (hand-written, sonnet 2026-09-29).
// 05_remake/tests/test_arraydtor_native.cpp only together with the port (install steps in that file). Compile-checked standalone (2026-09-29); never run.
//
// Structured native L1 for ArrayDtor_Eh2 (0x004c5ec0), ArrayDtor_Eh (0x004c5f70) and ArrayCtor_Eh (0x004c6000, forward order, 5 stack args, ret 0x14): stack args array, element size, count,
// destructor (ret 0x10). Both walk the array backwards calling the destructor with ECX = each element. Eh2 takes the array
// base and computes the end itself, Eh takes the END pointer. The destructor is a fake that logs its ECX relative to the array
// base, so the log holds the call order and the this-pointers; counts 0..8, element sizes 1..64. EAX at return is not compared:
// it is whatever the last destructor left in the original (the port returns 0, INFERRED that no caller reads it).
#include "test.h"
#include "vt_runner.h"
#include "unattributed/ui_widgets.h"

using namespace vtr;

namespace {
using U = std::uint32_t;

U& base()
{
    static U b = 0;
    return b;
}
// a thiscall destructor: ECX = the element, no stack arguments
void __fastcall fake_dtor(void* ecx, void*)
{
    vt::log().push_back(0xD7);
    vt::log().push_back(static_cast<U>(reinterpret_cast<std::uintptr_t>(ecx)) - base());
}

std::vector<Case> cases()
{
    std::vector<Case> c;
    struct E { const char* name; U va; void* port; bool end_pointer; bool ctor; };
    const E es[] = {{"ArrayDtor_Eh2", 0x004c5ec0, (void*)&recoil::ArrayDtor_Eh2, false, false},
                    {"ArrayDtor_Eh", 0x004c5f70, (void*)&recoil::ArrayDtor_Eh, true, false},
                    {"ArrayCtor_Eh", 0x004c6000, (void*)&recoil::ArrayCtor_Eh, false, true}};
    for (const E& e : es) {
        // one shared seed source per case: build draws the geometry, call re-reads it from the block
        c.push_back({e.name, e.va, e.port, 400, [](std::mt19937&) {},
                     [](World& w, std::mt19937& r) {
                         const U size = 1 + r() % 64, count = r() % 9;
                         const U arr = w.add(r, size * (count ? count : 1) + 8);  // block 0: the array
                         w.add(r, 8);                                                // block 1: size, count
                         w.blocks[1].w[0] = size;
                         w.blocks[1].w[1] = count;
                         (void)arr;
                     },
                     [e](World& w, U fn, std::mt19937&) {
                         const U size = w.blocks[1].w[0], count = w.blocks[1].w[1];
                         const U arr = ch::addr(w.blocks[0].w.data());
                         base() = arr;
                         const U first = e.end_pointer ? arr + size * count : arr;
                         const U cb = ch::addr(reinterpret_cast<void*>(&fake_dtor));
                         // call_any pushes args[n-1]..args[0], so a[0] is the first stack argument (vt_runner.h); the constructor
                         // helper takes a fifth argument (the destructor, used only by its SEH unwind), passed but never called
                         U a[5] = {first, size, count, cb, cb};
                         const U eax = call_any(fn, 0, 0, a, e.ctor ? 5 : 4);
                         return eax;
                     },
                     false});
    }
    return c;
}
}  // namespace

TEST(native_arraydtor_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    CHECK_EQ(run_cases(cases(), "array destructor iterators"), 0);
}
