// Native L1 for the settings leaf accessors (src/unattributed/settings.cpp) and Preset_CompareOp, ORIGINAL vs port.
// The settings block (0x004e5d00, 52 dwords) holds pointers to value cells; each side's block points into that
// side's own cells (and, for the display/screen rectangles, one more level of pointers), so after every call the
// cells, the rectangles, the block's non-pointer words and the returned EAX are compared.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/settings.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>

namespace {
std::mt19937 srng(0x5E77);
using F1 = int(__fastcall*)(int, int);
using CmpFn = int(__fastcall*)(const char*, int, int);
constexpr std::uintptr_t kOrigBlock = 0x004e5d00;

struct Side {
    std::uint32_t cells[recoil::kSettingsBlockWords];
    std::uint32_t rects[2][16];
    std::uint32_t rect_ptrs[2];
};
Side O, P;

// Point each side's block entries at that side's cells; entries 0x84/0x88 point at a pointer to a rectangle.
void seed(std::uint32_t* block, Side& s, const std::uint32_t* raw)
{
    for (int i = 0; i < recoil::kSettingsBlockWords; ++i) block[i] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&s.cells[i]));
    for (int r = 0; r < 2; ++r) s.rect_ptrs[r] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(s.rects[r]));
    block[0x84 / 4] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&s.rect_ptrs[0]));
    block[0x88 / 4] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&s.rect_ptrs[1]));
    block[0xCC / 4] = raw[0];  // 0x004e5dcc is a value (the HW-card flag), not a pointer
    std::memcpy(s.cells, raw + 1, sizeof s.cells);
    std::memcpy(s.rects, raw + 1 + recoil::kSettingsBlockWords, sizeof s.rects);
}
}  // namespace

TEST(native_settings_accessors_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    const std::uint32_t leaves[] = {0x004076f0, 0x00407e20, 0x00407f10, 0x00407f20, 0x004080b0, 0x00408230, 0x00408240, 0x00408250,
                                    0x00408280, 0x00408290, 0x004082a0, 0x004082b0, 0x00408300, 0x00408320, 0x00408330, 0x00408340,
                                    0x004083a0, 0x004083b0, 0x00408660, 0x00408670, 0x00408680, 0x00408690, 0x004086a0, 0x004086c0,
                                    0x004086d0, 0x00408a10, 0x00408a20};
    const F1 ports[] = {reinterpret_cast<F1>(&recoil::Stub_Ret), &recoil::Settings_StoreGameCtlOptions, &recoil::Settings_StoreGameIntensity,
                        &recoil::Settings_GetGameIntensity, &recoil::RecoilApp_GetSoundAPICheckboxValue, &recoil::Settings_SetNetworkFlag,
                        &recoil::Settings_StoreNetworkModem, &recoil::Settings_StoreNetListen, &recoil::Settings_ApplyHWCardFlag,
                        &recoil::Settings_StoreHWAPI, &recoil::Settings_StoreFullScreen, &recoil::Settings_StoreHUDFlag,
                        &recoil::Settings_Store_004e5d6c, &recoil::Settings_GetHWAPI, &recoil::Settings_GetFullScreen,
                        &recoil::Settings_GetSplitScreenValue, &recoil::Settings_StoreJoystickNumAxes, &recoil::Settings_StoreJoystickNumButtons,
                        &recoil::Settings_GetDisplayRect_10, &recoil::Settings_GetDisplayRect_14, &recoil::Settings_SetDisplayRect_20,
                        &recoil::Settings_GetDisplayRect_20, &recoil::Settings_Get_004e5d70, &recoil::Settings_GetValue_004e5d88,
                        &recoil::Settings_GetScreenRect_14, &recoil::Settings_StoreWOLPasswordFlag, &recoil::Settings_GetWOLPasswordFlag};
    static_assert(sizeof leaves / sizeof leaves[0] == sizeof ports / sizeof ports[0]);
    auto* oblock = reinterpret_cast<std::uint32_t*>(kOrigBlock);
    int bad = 0;
    std::string first;
    for (int n = 0; n < 4000; ++n) {
        std::uint32_t raw[1 + recoil::kSettingsBlockWords + 32];
        for (auto& w : raw) w = srng();
        raw[0] = srng() % 3 == 0 ? 0 : raw[0];  // the HW flag selects between two cells in the HUD accessors
        seed(oblock, O, raw);
        seed(recoil::g_SettingsBlock_004e5d00, P, raw);
        const int k = static_cast<int>(srng() % (sizeof leaves / sizeof leaves[0]));
        const int ecx = static_cast<int>(srng()), edx = static_cast<int>(srng());
        const int rO = rt::original<F1>(leaves[k])(ecx, edx);
        const int rP = ports[k](ecx, edx);
        const bool setter = leaves[k] == 0x00407e20 || leaves[k] == 0x00407f10 || leaves[k] == 0x00408230 || leaves[k] == 0x00408240 ||
                            leaves[k] == 0x00408250 || leaves[k] == 0x00408280 || leaves[k] == 0x00408290 || leaves[k] == 0x004082a0 ||
                            leaves[k] == 0x004082b0 || leaves[k] == 0x00408300 || leaves[k] == 0x004083a0 || leaves[k] == 0x004083b0 ||
                            leaves[k] == 0x00408680 || leaves[k] == 0x00408a10 || leaves[k] == 0x004076f0;
        const bool same_state = std::memcmp(O.cells, P.cells, sizeof O.cells) == 0 && std::memcmp(O.rects, P.rects, sizeof O.rects) == 0 &&
                                oblock[0xCC / 4] == recoil::g_SettingsBlock_004e5d00[0xCC / 4];
        // EAX is compared for the getters; a setter's EAX holds a (side-specific) pointer and is not a result.
        // 0x004086c0 returns the screen-rectangle pointer itself: compared as "this side's rectangle".
        const bool ptr_result = leaves[k] == 0x004086c0;
        const bool same_result = ptr_result ? (rO == static_cast<int>(reinterpret_cast<std::uintptr_t>(O.rects[1])) &&
                                               rP == static_cast<int>(reinterpret_cast<std::uintptr_t>(P.rects[1])))
                                            : rO == rP;
        if (!same_state || (!setter && !same_result)) {
            if (!bad++) first = "leaf 0x" + std::to_string(leaves[k]) + " #" + std::to_string(n);
        }
    }
    if (bad) std::printf("  first difference: %s\n", first.c_str());
    CHECK_EQ(bad, 0);
}

TEST(native_settings_preset_compare_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    auto oCmp = rt::original<CmpFn>(0x00407220);
    const char* ops[] = {"==", "!=", "<", ">", "<=", ">=", "~=", "=", "<>", "", "=>", "==x"};
    int bad = 0, trues = 0;
    for (int n = 0; n < 20000; ++n) {
        const char* op = ops[srng() % (sizeof ops / sizeof ops[0])];
        const int a = static_cast<int>(srng() % 2000) - 1000;
        const int b = srng() % 4 == 0 ? a + static_cast<int>(srng() % 21) - 10 : static_cast<int>(srng() % 2000) - 1000;
        const int r1 = oCmp(op, a, b), r2 = recoil::Preset_CompareOp(reinterpret_cast<int>(op), a, b);
        if (r1 != r2) ++bad;
        trues += r1 != 0;
    }
    CHECK_EQ(bad, 0);
    CHECK(trues > 2000);
}

// Preset_EvaluateCondition (0x00407470, config node ECX): a string node equal to "DEFAULT" (0x004da658) -> 1; a list
// node whose child array has count 4 {name, op, value}: the name selects the measured value - CPU_CLASS [0x004e5da8],
// CPU_MHZ [0x004e5dac], VIDEO_KB [0x004e5db8], RAM_KB [0x004e5db4], HW_ACCEL bit 6 of [0x004e5db0], any other name 0 -
// and the result is Preset_CompareOp(op, measured, ConfigValue_ToInt(value)); anything else 0. Real config nodes
// (DEFAULT and near misses, lists over the six names and others, every operator, int / float / string values, a wrong
// count) and random measured values in each side's settings block. Compared: the return value.
TEST(native_settings_preset_evaluate_condition_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00407470), reinterpret_cast<Fn>(&recoil::Preset_EvaluateCondition)};
    const char* strs[] = {"DEFAULT", "default", "DEFAULTS", "DEFAUL", ""};
    const char* names[] = {"CPU_CLASS", "CPU_MHZ", "VIDEO_KB", "RAM_KB", "HW_ACCEL", "cpu_mhz", "CPU", "RAM_KBX", ""};
    const char* ops[] = {"==", "!=", "<", ">", "<=", ">=", "~=", "=", ""};
    std::mt19937 rng(0x407470);
    int compared = 0, trues = 0;
    for (int it = 0; it < 20000; ++it) {
        std::uint32_t node[2], arr[8];
        const int kind = static_cast<int>(rng() % 6);  // 0 string, 1 other type, else list
        const char* nm = names[rng() % 9];
        const char* op = ops[rng() % 9];
        float fv = static_cast<float>(static_cast<int>(rng() % 4000)) * 0.75f;
        if (kind == 0) { node[0] = 3; node[1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(strs[rng() % 5])); }
        else if (kind == 1) { node[0] = rng() % 2 ? 1u : 5u; node[1] = rng(); }
        else {
            node[0] = 4;
            node[1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(arr));
            arr[0] = 1; arr[1] = rng() % 8 == 0 ? 3u : 4u;
            arr[2] = 3; arr[3] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(nm));
            arr[4] = 3; arr[5] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(op));
            const int vt = static_cast<int>(rng() % 3);
            arr[6] = vt == 0 ? 1u : vt == 1 ? 2u : 3u;
            if (vt == 0) arr[7] = rng() % 4000;
            else if (vt == 1) std::memcpy(&arr[7], &fv, 4);
            else arr[7] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(names[rng() % 9]));
        }
        const std::uint32_t meas[5] = {rng() % 8, rng() % 4000, rng() % 4000, rng() % 4000, rng()};
        int ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const std::uint32_t va[5] = {0x004e5da8, 0x004e5dac, 0x004e5db8, 0x004e5db4, 0x004e5db0};
            for (int k = 0; k < 5; ++k)
                *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va[k]) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va[k]))) = meas[k];
            ret[side] = fn[side](node, 0);
        }
        CHECK_EQ(ret[0], ret[1]);
        trues += ret[0] != 0;
        ++compared;
    }
    rt::restore_pristine();
    CHECK(trues > 1000);
    std::printf("  Preset_EvaluateCondition calls %d, %d true (DEFAULT, condition lists over the measured values)\n", compared, trues);
}

// Settings_GetHardwarePresetOrDefault (0x00407680; ECX config tree, EDX key, one stack argument: default; ret 4;
// cloud port, P2 settings): null tree -> the default; ConfigTree_FindChild(tree, key) (the item after the matching
// string) none -> default; that item's list (count +4) entries 1.. are lists {header, condition, value}: the first entry
// whose condition passes Preset_EvaluateCondition returns ConfigValue_ToInt(value); none -> the default.
// Real trees: the key present or not, 0..4 presets whose conditions are DEFAULT, near misses or measured-value
// comparisons, values int / float / string; measured values random in each side's settings block. Compared: the return.
TEST(native_settings_hardware_preset_or_default_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(const std::uint32_t*, const char*, int);
    const Fn fn[2] = {rt::original<Fn>(0x00407680), reinterpret_cast<Fn>(&recoil::Settings_GetHardwarePresetOrDefault)};
    const char* keys[] = {"TEXTURE_DETAIL", "SOUND", "MISSING"};
    const char* conds[] = {"DEFAULT", "DEFAUL", "CPU_MHZ", "RAM_KB"};
    const char* ops[] = {">=", "<", "==", "!="};
    auto p = [](const void* x) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(x)); };
    std::mt19937 rng(0x407680);
    int compared = 0, picked = 0;
    for (int it = 0; it < 10000; ++it) {
        static std::uint32_t root[2], rootarr[8], presets[2 * 5], entries[4][6], condarr[4][8];
        const int n = static_cast<int>(rng() % 5);
        const char* want = keys[rng() % 3];
        root[0] = 4; root[1] = p(rootarr);
        rootarr[0] = 1; rootarr[1] = 4;
        rootarr[2] = 3; rootarr[3] = p(keys[rng() % 2]);
        rootarr[4] = 4; rootarr[5] = p(presets);
        rootarr[6] = 1; rootarr[7] = 7;
        presets[0] = 1; presets[1] = static_cast<std::uint32_t>(n + 1);
        for (int k = 0; k < n && k < 4; ++k) {
            presets[2 + 2 * k] = 4; presets[3 + 2 * k] = p(entries[k]);
            entries[k][0] = 1; entries[k][1] = 3;
            const int c = static_cast<int>(rng() % 4);
            if (c < 2) { entries[k][2] = 3; entries[k][3] = p(conds[c]); }
            else {
                entries[k][2] = 4; entries[k][3] = p(condarr[k]);
                condarr[k][0] = 1; condarr[k][1] = 4;
                condarr[k][2] = 3; condarr[k][3] = p(conds[c]);
                condarr[k][4] = 3; condarr[k][5] = p(ops[rng() % 4]);
                condarr[k][6] = 1; condarr[k][7] = rng() % 1000;
            }
            const int vt = static_cast<int>(rng() % 3);
            entries[k][4] = vt == 0 ? 1u : vt == 1 ? 2u : 3u;
            if (vt == 0) entries[k][5] = rng() % 100;
            else if (vt == 1) { const float f = static_cast<float>(rng() % 400) * 0.25f; std::memcpy(&entries[k][5], &f, 4); }
            else entries[k][5] = p("12");
        }
        const std::uint32_t meas[5] = {rng() % 8, rng() % 1000, rng() % 1000, rng() % 1000, rng()};
        const int def = static_cast<int>(rng() % 50) - 10;
        const bool null_tree = rng() % 30 == 0;
        int ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            const std::uint32_t va[5] = {0x004e5da8, 0x004e5dac, 0x004e5db8, 0x004e5db4, 0x004e5db0};
            for (int k = 0; k < 5; ++k)
                *static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va[k]) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va[k]))) = meas[k];
            ret[side] = fn[side](null_tree ? nullptr : root, want, def);
        }
        CHECK_EQ(ret[0], ret[1]);
        picked += ret[0] != def;
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Settings_GetHardwarePresetOrDefault calls %d, %d took a preset value\n", compared, picked);
    CHECK(picked > 1000);
}
