// Structured native L1 for the zclass_nodes class-data accessors (P2.6): each checks the node (ECX; null -> report, 5),
// its class data (+0x38; null -> report, 5) and, for some, the class type (+0x34; wrong -> report, 3) - except the two
// Node_SetClassData* of Seq.c, which check nothing and are never given nulls - then reads or
// writes words of the class data through EDX and the stack arguments. The arena fuzz (test_fuzz_zclass*) reaches the
// success path only when a random +0x34 happens to equal the class, about 1 call in 80; here every call gets a real
// node (0xC4 bytes) whose class is the expected one 3 times in 4, real class data (0x200 bytes, random words), null
// node / null data now and then, and per argument either a value (random word, or a float from a small set with
// repeats, negatives and zero) or an out pointer to a per-side slot. Compared: the return, every node and class-data
// word and every out slot, with the slot addresses replaced by their role.
#include "test.h"
#include "native_oracle.h"
#include "platform/image/original_data.h"
#include "GameZRecoil/zClass/Camera.h"
#include "GameZRecoil/zClass/Display.h"
#include "GameZRecoil/zClass/Light.h"
#include "GameZRecoil/zClass/Seq.h"
#include "GameZRecoil/zClass/Sound.h"
#include "GameZRecoil/zClass/Window_nodes.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }

// args: one character per argument, EDX first, then the stack arguments in order:
//   p = out pointer, v = random word, f = float, - = unused (EDX not read)
// global: a data address the function may write (compared over 128 bytes on both sides), or 0
// unchecked: the function has no null checks (never given a null node / class data)
// setup: per-side global state the function reads (after the pristine restore), or null
struct Accessor {
    std::uint32_t va; void* port; const char* name; int cls; const char* args; std::uint32_t global; bool unchecked;
    void (*setup)(int side);
};

std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
// a 565 screen format: Pixel_PackRGB reads red shift, green shift, blue right shift, red mask, green mask at 0x00632170
void screen_565(int side)
{
    const std::uint32_t fmt[5] = {11, 5, 3, 0x1f, 0x3f};
    for (int k = 0; k < 5; ++k) *img(side, 0x00632170u + 4 * k) = fmt[k];
}

std::vector<Accessor> accessors()
{
    return {
        {0x0044f8b0, reinterpret_cast<void*>(&recoil::WindowClass_SetSize), "WindowClass_SetSize", 3, "vv", 0},
        {0x0044f930, reinterpret_cast<void*>(&recoil::Class_GetCameraFields_8_C), "Class_GetCameraFields_8_C", 3, "pp", 0},
        {0x0044f9c0, reinterpret_cast<void*>(&recoil::WindowClass_SetOrigin), "WindowClass_SetOrigin", 3, "vv", 0},
        {0x0044fa40, reinterpret_cast<void*>(&recoil::Class_GetCameraFields_0_4), "Class_GetCameraFields_0_4", 3, "pp", 0},
        {0x0044fe90, reinterpret_cast<void*>(&recoil::DisplayClass_SetSize), "DisplayClass_SetSize", 4, "vv", 0},
        {0x0044ff10, reinterpret_cast<void*>(&recoil::DisplayClass_SetOrigin), "DisplayClass_SetOrigin", 4, "vv", 0},
        {0x00453200, reinterpret_cast<void*>(&recoil::Light_SetFieldA8), "Light_SetFieldA8", 9, "-v", 0},
        {0x00453250, reinterpret_cast<void*>(&recoil::Light_SetFieldA4), "Light_SetFieldA4", 9, "-v", 0},
        {0x004532a0, reinterpret_cast<void*>(&recoil::Light_SetFieldB8), "Light_SetFieldB8", 9, "v", 0},
        {0x004532f0, reinterpret_cast<void*>(&recoil::Light_SetModeBC), "Light_SetModeBC", 9, "-", 0},
        {0x00453350, reinterpret_cast<void*>(&recoil::Light_SetModeC0), "Light_SetModeC0", 9, "-", 0},
        {0x004533b0, reinterpret_cast<void*>(&recoil::Light_SetFieldC4), "Light_SetFieldC4", 9, "v", 0},
        {0x00453400, reinterpret_cast<void*>(&recoil::Light_SetFalloffRange), "Light_SetFalloffRange", 9, "-ff", 0x00575de0},
        {0x00453500, reinterpret_cast<void*>(&recoil::Light_GetFalloffRange), "Light_GetFalloffRange", 9, "pp", 0},
        {0x00453560, reinterpret_cast<void*>(&recoil::Light_SetVec14), "Light_SetVec14", 9, "-vvv", 0},
        {0x004535c0, reinterpret_cast<void*>(&recoil::Light_SetVec08), "Light_SetVec08", 9, "-vvv", 0},
        {0x00453a40, reinterpret_cast<void*>(&recoil::Light_GetColor), "Light_GetColor", 9, "ppp", 0},
        {0x00452d00, reinterpret_cast<void*>(&recoil::Sound_SetVec30), "Sound_SetVec30", 10, "-vvv", 0},
        {0x00452d60, reinterpret_cast<void*>(&recoil::Sound_GetVec30), "Sound_GetVec30", 10, "ppp", 0},
        {0x004540c0, reinterpret_cast<void*>(&recoil::Seq_SetField0), "Seq_SetField0", 7, "v", 0},
        {0x00454100, reinterpret_cast<void*>(&recoil::Seq_SetField4), "Seq_SetField4", 7, "v", 0},
        {0x00454140, reinterpret_cast<void*>(&recoil::Seq_SetField8), "Seq_SetField8", 7, "v", 0},
        {0x00454180, reinterpret_cast<void*>(&recoil::Seq_SetFieldC), "Seq_SetFieldC", 7, "v", 0},
        {0x00454330, reinterpret_cast<void*>(&recoil::Node_SetClassDataField0_NoCheck), "Node_SetClassDataField0_NoCheck", 6, "v", 0, true},
        {0x00454340, reinterpret_cast<void*>(&recoil::Node_SetClassDataRangeCheck), "Node_SetClassDataRangeCheck", 6, "vf", 0, true},
        {0x0044a410, reinterpret_cast<void*>(&recoil::Camera_SetAspectRatio), "Camera_SetAspectRatio", 1, "-ff", 0},
        {0x0044a610, reinterpret_cast<void*>(&recoil::Camera_SetFOV), "Camera_SetFOV", 1, "-ff", 0},
        {0x0044ff90, reinterpret_cast<void*>(&recoil::Display_SetColour), "Display_SetColour", 4, "-fff", 0x006321cc, false, &screen_565},
    };
}

// ECX, EDX and up to 4 stack arguments; the callee pops its own (RET n)
std::uint32_t call(void* f, std::uint32_t c, std::uint32_t d, const std::uint32_t* s, int ns)
{
    std::uint32_t r = 0, saved = 0;
    int n = ns;
    __asm {
        mov saved, esp
        mov esi, s
        mov eax, n
    push_loop:
        test eax, eax
        jz pushed
        dec eax
        push dword ptr [esi + eax * 4]
        jmp push_loop
    pushed:
        mov ecx, c
        mov edx, d
        call f
        mov r, eax
        mov esp, saved
    }
    return r;
}
}  // namespace

TEST(native_zclass_accessors_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const float floats[] = {0.0f, 1.0f, -1.0f, 0.5f, 2.0f, 100.0f, 1000.0f, -250.0f, 3.25f, 1e-3f};
    int failed = 0;
    for (const Accessor& a : accessors()) {
        std::mt19937 rng(a.va);
        const int nargs = static_cast<int>(std::strlen(a.args));
        int compared = 0, ok = 0;
        for (int it = 0; it < 3000; ++it) {
            std::uint32_t node_init[49], data_init[128], vals[6];
            for (auto& w : node_init) w = rng();
            for (auto& w : data_init) w = rng();
            node_init[0x34 / 4] = rng() % 4 ? static_cast<std::uint32_t>(a.cls) : rng() % 14;
            for (int k = 0; k < nargs; ++k)
                vals[k] = a.args[k] == 'f' ? [&] { float f = floats[rng() % 10]; std::uint32_t u; std::memcpy(&u, &f, 4); return u; }()
                                           : rng() % 8 == 0 ? 0u : rng();  // zero now and then: some test the value
            const unsigned null_kind = !a.unchecked && rng() % 12 == 0 ? 1 + rng() % 2 : 0;  // 1 node, 2 class data
            std::vector<std::uint32_t> snap[2];
            for (int side = 0; side < 2; ++side) {
                rt::restore_pristine();
                if (a.setup) a.setup(side);
                static std::uint32_t node[49], data[128], out[6];
                std::memcpy(node, node_init, sizeof node);
                std::memcpy(data, data_init, sizeof data);
                for (auto& w : out) w = 0xA5A5A5A5u;
                node[0x38 / 4] = null_kind == 2 ? 0u : addr(data);
                std::uint32_t arg[6];
                for (int k = 0; k < nargs; ++k) arg[k] = a.args[k] == 'p' ? addr(&out[k]) : vals[k];
                void* f = side ? a.port : rt::original<void*>(a.va);
                const std::uint32_t edx = nargs ? arg[0] : 0u;
                const std::uint32_t r = call(f, null_kind == 1 ? 0u : addr(node), edx, arg + 1, nargs > 1 ? nargs - 1 : 0);
                auto role = [&](std::uint32_t v) -> std::uint32_t {
                    for (int k = 0; k < 6; ++k) if (v == addr(&out[k])) return 0xB0000000u + k;
                    return v == addr(data) ? 0xB1000000u : v;
                };
                snap[side].push_back(r);
                for (std::uint32_t w : node) snap[side].push_back(role(w));
                for (std::uint32_t w : data) snap[side].push_back(role(w));
                for (std::uint32_t w : out) snap[side].push_back(w);
                if (a.global) {
                    const auto* g = static_cast<const std::uint32_t*>(side ? recoil::ImageData_Address(a.global)
                                                                          : reinterpret_cast<void*>(static_cast<std::uintptr_t>(a.global)));
                    snap[side].insert(snap[side].end(), g, g + 32);
                }
                if (side == 0 && r == 0) ++ok;
            }
            if (snap[0] != snap[1]) { if (!failed++) std::printf("  DIFF %s call %d\n", a.name, it); }
            ++compared;
        }
        std::printf("  %-36s calls %d, %d returned 0\n", a.name, compared, ok);
        if (ok < 500) ++failed;
    }
    rt::restore_pristine();
    CHECK_EQ(failed, 0);
}
