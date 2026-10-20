// Native L1 for ZbdResource_Construct (0x004c0d20, this ECX, two stack words; ret 8) and its C++ EH frame (handler
// 0x004cb9be, unwind action 0x004cb9b0 = List_DestroyNodes on this+0xB0).
// Construct: byte +0xB0 = low byte of the second stack word; +0xB4 = a new self-linked 12-byte list sentinel, +0xB8 =
// 0; vtable 0x004d42c0 at +0; +0xA4 = malloc(0x1D8) zeroed; +0x74 / +0x78 = _strdup of the stack words; +0x8C =
// malloc(4) holding 0; a set of fields zeroed. Each side: a random-filled object, two strings. Compared: every object
// word, with the vtable as "the side's own 0x004d42c0" (the port's is the .rdata mirror), heap blocks by content (the
// sentinel as self-linked, the buffers' bytes, the strings).
// EH: both handlers called directly as the dispatcher would (the OS would not dispatch to the original's handler in
// its private mapping - KG-32): registration node {next, handler, state} with [EBP-0x10] = the object, whose list at
// +0xB0 holds 0..4 real heap nodes; unwinding or search-phase record, states 0 / -1. Compared: disposition, the free
// log (blocks by role) and the list words / state afterwards.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/asset_io_misc.h"
#include "unattributed/hud.h"
#include "cloud_harness.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
using Free = void(__cdecl*)(void*);
using Malloc = void*(__cdecl*)(std::size_t);
Free real_free() { return reinterpret_cast<Free>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free")); }
Malloc real_malloc() { return reinterpret_cast<Malloc>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc")); }
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::vector<std::uint32_t> g_frees, g_roles;
void __cdecl logging_free(void* p)
{
    std::uint32_t r = addr(p);
    for (std::size_t k = 0; k < g_roles.size(); ++k) if (g_roles[k] == addr(p)) r = 0xA0000000u + static_cast<std::uint32_t>(k);
    g_frees.push_back(r);
    real_free()(p);
}
}  // namespace

TEST(native_zbd_resource_construct_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int, const char*, const char*);
    const Fn fn[2] = {rt::original<Fn>(0x004c0d20), reinterpret_cast<Fn>(&recoil::ZbdResource_Construct)};
    const char* names[] = {"interp", "anim", "m1.zbd", "", "a_rather_longer_resource_name"};
    std::mt19937 rng(0x4c0d20);
    int compared = 0;
    for (int it = 0; it < 2000; ++it) {
        std::uint32_t init[64];
        for (auto& w : init) w = rng();
        const std::string a = names[rng() % 5], b = names[rng() % 5];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t obj[64];
            std::memcpy(obj, init, sizeof obj);
            // prime the heap: a freed block of the buffer's size full of 0xCD, so an incomplete zero fill shows
            void* prime = real_malloc()(0x1D8);
            std::memset(prime, 0xCD, 0x1D8);
            real_free()(prime);
            const std::uint32_t ret = fn[side](obj, 0, a.c_str(), b.c_str());
            const std::uint32_t vt = side ? addr(recoil::ImageData_Address(0x004d42c0)) : 0x004d42c0u;
            snap[side].push_back(ret == addr(obj) ? 1u : ret);
            for (int k = 0; k < 64; ++k) {
                std::uint32_t v = obj[k];
                if (k == 0) v = v == vt ? 0x0D42C0u : v;
                else if (k == 0xB4 / 4 || k == 0xA4 / 4 || k == 0x74 / 4 || k == 0x78 / 4 || k == 0x8C / 4) v = v ? 1u : 0u;
                snap[side].push_back(v);
            }
            if (obj[0xB4 / 4]) {
                const auto* s = ptr<std::uint32_t>(obj[0xB4 / 4]);
                snap[side].push_back(s[0] == obj[0xB4 / 4]);
                snap[side].push_back(s[1] == obj[0xB4 / 4]);
            }
            if (obj[0xA4 / 4]) {
                const auto* bb = ptr<unsigned char>(obj[0xA4 / 4]);
                snap[side].insert(snap[side].end(), bb, bb + 0x1D8);
            }
            for (int k : {0x74 / 4, 0x78 / 4})
                if (obj[k]) for (const char* c = ptr<char>(obj[k]); ; ++c) { snap[side].push_back(static_cast<unsigned char>(*c)); if (!*c) break; }
            if (obj[0x8C / 4]) snap[side].push_back(*ptr<std::uint32_t>(obj[0x8C / 4]));
            for (int k : {0xB4 / 4, 0xA4 / 4, 0x74 / 4, 0x78 / 4, 0x8C / 4}) if (obj[k]) real_free()(ptr<void>(obj[k]));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    std::printf("  ZbdResource_Construct calls %d (random objects, name / extension strings)\n", compared);
}

TEST(native_eh_handler_zbd_resource_construct_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Handler = int(__cdecl*)(EXCEPTION_RECORD*, void*, CONTEXT*, void*);
    const Handler h[2] = {rt::original<Handler>(0x004cb9be), reinterpret_cast<Handler>(&recoil::EH_Handler_ZbdResource_Construct)};
    void* const saved_free = recoil::g_Iat_free_004cc5b4;
    recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    std::mt19937 rng(0x4cb9be);
    int compared = 0;
    for (int it = 0; it < 200; ++it) {
        const int kind = it % 4;  // state 0 / -1 x unwinding / search phase
        const std::int32_t state = kind / 2 == 0 ? 0 : -1;
        const bool unwinding = kind % 2 == 0;
        const int n = static_cast<int>(rng() % 5);
        const std::uint32_t count = static_cast<std::uint32_t>(n);
        int disp[2];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::uint32_t obj[64] = {};
            g_roles.clear();
            for (int k = 0; k <= n; ++k) g_roles.push_back(addr(real_malloc()(12)));
            for (int k = 0; k <= n; ++k) {
                auto* node = ptr<std::uint32_t>(g_roles[k]);
                node[0] = g_roles[(k + 1) % (n + 1)];
                node[1] = g_roles[(k + n) % (n + 1)];
            }
            obj[0xB4 / 4] = g_roles[0];
            obj[0xB8 / 4] = count;
            std::uint32_t frame[16] = {};
            std::uint32_t* node = frame + 4;
            node[0] = 0xFFFFFFFFu;
            node[1] = side ? addr(reinterpret_cast<void*>(h[1])) : 0x004cb9beu;
            node[2] = static_cast<std::uint32_t>(state);
            frame[3] = addr(obj);  // [EBP - 0x10] with EBP = node + 0xC
            EXCEPTION_RECORD rec = {};
            rec.ExceptionCode = unwinding ? 0xE06D7363u : 0xC0000005u;
            rec.ExceptionFlags = unwinding ? EXCEPTION_UNWINDING : 0;
            CONTEXT ctx = {};
            g_frees.clear();
            disp[side] = h[side](&rec, node, &ctx, nullptr);
            snap[side] = g_frees;
            snap[side].push_back(obj[0xB4 / 4] ? 1u : 0u);
            snap[side].push_back(obj[0xB8 / 4]);
            snap[side].push_back(node[2]);
            if (g_frees.empty()) for (std::uint32_t b : g_roles) real_free()(ptr<void>(b));
        }
        CHECK_EQ(disp[0], disp[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    recoil::g_Iat_free_004cc5b4 = saved_free;
    std::printf("  EH_Handler/Unwind of ZbdResource_Construct calls %d (lists of 0..4 nodes; unwinding / search phase, states 0 / -1)\n", compared);
}

// InterpZbd_Construct (0x00414ab0; hud, ported in the cloud; ECX this): ZbdResource_Construct(this, [0x004e48f4], the
// extension 0x004dae4c), then the vtable 0x004ce9c8 over the base's. Compared as the construct test above, with
// [0x004e48f4] a name of this test on both sides.
TEST(native_interp_zbd_construct_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(void*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00414ab0), reinterpret_cast<Fn>(&recoil::InterpZbd_Construct)};
    const char* names[] = {"interp", "m1", "", "a_rather_longer_resource_name"};
    std::mt19937 rng(0x414ab0);
    for (int it = 0; it < 300; ++it) {
        std::uint32_t init[64];
        for (auto& w : init) w = rng();
        const char* nm = names[rng() % 4];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x004e48f4) = addr(nm);
            std::uint32_t obj[64];
            std::memcpy(obj, init, sizeof obj);
            const std::uint32_t ret = fn[side](obj, 0);
            const std::uint32_t vt = side ? addr(recoil::ImageData_Address(0x004ce9c8)) : 0x004ce9c8u;
            snap[side].push_back(ret == addr(obj) ? 1u : ret);
            for (int k = 0; k < 64; ++k) {
                std::uint32_t v = obj[k];
                if (k == 0) v = v == vt ? 0x0CE9C8u : v;
                else if (k == 0xB4 / 4 || k == 0xA4 / 4 || k == 0x74 / 4 || k == 0x78 / 4 || k == 0x8C / 4) v = v ? 1u : 0u;
                snap[side].push_back(v);
            }
            for (int k : {0x74 / 4, 0x78 / 4})
                if (obj[k]) for (const char* c = ptr<char>(obj[k]); ; ++c) { snap[side].push_back(static_cast<unsigned char>(*c)); if (!*c) break; }
            if (obj[0xA4 / 4]) { const auto* bb = ptr<unsigned char>(obj[0xA4 / 4]); snap[side].insert(snap[side].end(), bb, bb + 0x1D8); }
            for (int k : {0xB4 / 4, 0xA4 / 4, 0x74 / 4, 0x78 / 4, 0x8C / 4}) if (obj[k]) real_free()(ptr<void>(obj[k]));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// Hud_StaticInit_Construct_004edb78 (0x00414a70; hud): InterpZbd_Construct on the object at 0x004edb78 in .data.
// Compared as above on each side's copy of that object (the original's .data / the port's data image).
TEST(native_hud_static_interp_zbd_construct_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(int, int);
    const Fn fn[2] = {rt::original<Fn>(0x00414a70), reinterpret_cast<Fn>(&recoil::Hud_StaticInit_Construct_004edb78)};
    const char* names[] = {"interp", "m1", ""};
    std::mt19937 rng(0x414a70);
    for (int it = 0; it < 60; ++it) {
        const char* nm = names[rng() % 3];
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x004e48f4) = addr(nm);
            std::uint32_t* obj = ch::img(side, 0x004edb78);
            const std::uint32_t ret = fn[side](0, 0);
            const std::uint32_t vt = side ? addr(recoil::ImageData_Address(0x004ce9c8)) : 0x004ce9c8u;
            snap[side].push_back(ret == addr(obj) ? 1u : ret);
            for (int k = 0; k < 64; ++k) {
                std::uint32_t v = obj[k];
                if (k == 0) v = v == vt ? 0x0CE9C8u : v;
                else if (k == 0xB4 / 4 || k == 0xA4 / 4 || k == 0x74 / 4 || k == 0x78 / 4 || k == 0x8C / 4) v = v ? 1u : 0u;
                snap[side].push_back(v);
            }
            for (int k : {0x74 / 4, 0x78 / 4})
                if (obj[k]) for (const char* c = ptr<char>(obj[k]); ; ++c) { snap[side].push_back(static_cast<unsigned char>(*c)); if (!*c) break; }
            for (int k : {0xB4 / 4, 0xA4 / 4, 0x74 / 4, 0x78 / 4, 0x8C / 4}) if (obj[k]) real_free()(ptr<void>(obj[k]));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}
