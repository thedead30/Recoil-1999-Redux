// Native L1 for the settings-node list: Settings_RegisterNode 0x004b2e80, Settings_FindNodeByName 0x004b3380,
// Settings_Shutdown 0x004b32c0 and Settings_StorePlayerName 0x00408120, ORIGINAL vs port. Random sequences of
// registrations (repeated names, every node type 0..9), lookups and player-name stores, then a shutdown, run on
// both sides; each side's list-state words (0x0056bcd0) and settings block (0x004e5d00) are registered as heap-
// graph regions, so list heads, node records, names (_strdup), value buffers (calloc) and the free sequence are
// all compared structurally (tests/heap_graph.h).
#include "test.h"
#include "native_oracle.h"
#include "heap_graph.h"
#include "unattributed/settings.h"

#include <cstdio>
#include <cstring>
#include <random>
#include <string>

namespace {
std::mt19937 nrng(0x5E7A);
using F2 = int(__fastcall*)(int, int);
using F4 = int(__fastcall*)(int, int, int, int);
int I(const void* p) { return static_cast<int>(reinterpret_cast<std::uintptr_t>(p)); }
constexpr std::uintptr_t kOrigState = 0x0056bcd0, kOrigBlock = 0x004e5d00;
const char* kNames[] = {"PlayerName", "SoundVolume", "Window", "HWAPI", "detail", "Camera", "x", "SoundLOD"};
const char* kPlayerNames[] = {"", "Ace", "Commander Keen", "A very long player name that does not fit in the buffer at all"};
}  // namespace

TEST(native_settings_node_list_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    auto oFind = rt::original<F2>(0x004b3380);
    auto oRegister = rt::original<F4>(0x004b2e80);
    auto oShutdown = rt::original<F2>(0x004b32c0);
    auto oStore = rt::original<F2>(0x00408120);
    const F2 pFind = &recoil::Settings_FindNodeByName, pShutdown = &recoil::Settings_Shutdown, pStore = &recoil::Settings_StorePlayerName;
    const F4 pRegister = &recoil::Settings_RegisterNode;
    auto* oState = reinterpret_cast<std::uint32_t*>(kOrigState);
    auto* oBlock = reinterpret_cast<std::uint32_t*>(kOrigBlock);
    hg::Recording rec;
    int bad = 0, registered = 0, found = 0, stored = 0;
    std::string first;
    for (int n = 0; n < 1500 && !bad; ++n) {
        hg::Log O, P;
        std::memset(oState, 0, 4 * recoil::kSettingsNodeStateWords);
        std::memset(recoil::g_SettingsNodeState_0056bcd0, 0, sizeof recoil::g_SettingsNodeState_0056bcd0);
        std::memset(oBlock, 0, 4 * recoil::kSettingsBlockWords);
        std::memset(recoil::g_SettingsBlock_004e5d00, 0, sizeof recoil::g_SettingsBlock_004e5d00);
        O.add(oState, 1, 4 * recoil::kSettingsNodeStateWords); P.add(recoil::g_SettingsNodeState_0056bcd0, 1, sizeof recoil::g_SettingsNodeState_0056bcd0);
        O.add(oBlock, 2, 4 * recoil::kSettingsBlockWords); P.add(recoil::g_SettingsBlock_004e5d00, 2, sizeof recoil::g_SettingsBlock_004e5d00);
        // the three owned strings and the initialised flag Settings_Shutdown looks at
        oState[1] = recoil::g_SettingsNodeState_0056bcd0[1] = nrng() % 4 ? 1u : 0u;
        for (int k = 2; k < 5; ++k)
            if (nrng() % 2) {
                hg::current() = &O; oState[k] = static_cast<std::uint32_t>(I(hg::rec_malloc(12)));
                hg::current() = &P; recoil::g_SettingsNodeState_0056bcd0[k] = static_cast<std::uint32_t>(I(hg::rec_malloc(12)));
            }
        auto step = [&](const char* what) {
            const std::string d = hg::compare(O, P);
            if (!d.empty() && !bad) { ++bad; first = std::string(what) + " #" + std::to_string(n) + ": " + d; }
        };
        const int ops = 1 + static_cast<int>(nrng() % 12);
        for (int k = 0; k < ops && !bad; ++k) {
            const char* name = kNames[nrng() % (sizeof kNames / sizeof kNames[0])];
            switch (nrng() % 4) {
            case 0:
            case 1: {
                const int type = static_cast<int>(nrng() % 10), size = static_cast<int>(nrng() % 3 == 0 ? 0 : 1 + nrng() % 40);
                const int extra = static_cast<int>(nrng() & 0xFFFF);
                hg::current() = &O; const int a = hg::call_fastcall(oRegister, I(name), type, size, extra);
                hg::current() = &P; const int b = hg::call_fastcall(pRegister, I(name), type, size, extra);
                if ((a == 0) != (b == 0) || O.find(reinterpret_cast<void*>(a)) != P.find(reinterpret_cast<void*>(b))) { ++bad; first = "register result #" + std::to_string(n); }
                registered += a != 0;
                step("register");
            } break;
            case 2: {
                hg::current() = &O; const int a = hg::call_fastcall(oFind, I(name), 0);
                hg::current() = &P; const int b = hg::call_fastcall(pFind, I(name), 0);
                if (O.find(reinterpret_cast<void*>(a)) != P.find(reinterpret_cast<void*>(b))) { ++bad; first = "find result #" + std::to_string(n); }
                found += a != 0;
            } break;
            default: {
                // store a player name into a registered node that has a value buffer (block +0x4c points at it)
                hg::current() = &O; const int a = hg::call_fastcall(oFind, I(name), 0);
                hg::current() = &P; const int b = hg::call_fastcall(pFind, I(name), 0);
                if (!a || !b || !*reinterpret_cast<std::uint32_t*>(a) || *reinterpret_cast<int*>(a + 0xc) <= 0) break;
                oBlock[0x4c / 4] = static_cast<std::uint32_t>(a);
                recoil::g_SettingsBlock_004e5d00[0x4c / 4] = static_cast<std::uint32_t>(b);
                const char* pn = kPlayerNames[nrng() % 4];
                hg::current() = &O; hg::call_fastcall(oStore, I(pn), 0);
                hg::current() = &P; hg::call_fastcall(pStore, I(pn), 0);
                ++stored;
                step("store player name");
            } break;
            }
        }
        hg::current() = &O; hg::call_fastcall(oShutdown, 0, 0);
        hg::current() = &P; hg::call_fastcall(pShutdown, 0, 0);
        step("shutdown");
        for (hg::Log* L : {&O, &P})
            for (auto& [p, blk] : L->live)
                if (blk.role >= 1000) hg::real().free_(reinterpret_cast<void*>(p));
    }
    if (bad) std::printf("  first difference: %s\n", first.c_str());
    std::printf("  registered %d, found %d, player names stored %d\n", registered, found, stored);
    CHECK_EQ(bad, 0);
    CHECK(registered > 1000 && found > 300 && stored > 100);  // each path really taken
}
