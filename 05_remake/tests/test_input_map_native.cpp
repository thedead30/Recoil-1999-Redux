// Structured native L1s for the input map (P3 input, ported in the cloud; GameZRecoil\zInput\zin_kbd.cpp).
// Map layout as the listings use it: +4 command count, +8 packed binding word per command (primary key bits 0..10,
// secondary key 11..21, joystick button 22..25, mouse button 26..27), +0xC a block, +0x10 command-name pointers;
// reverse tables command-by-input at +0x14 (2014 primary keys), +0x1F8C (2014 secondary keys), +0x3F04 (16 joystick
// buttons), +0x3F44 (4 mouse buttons).
//  - getters 0x00470a40/a60/a80/aa0 (binding field of a command), 0x00470ac0/ad0/b00/b10 (command of an input);
//  - setters 0x00470b20/b80/bf0/c60 (input, command; ret 8): unbind the input's old command, bind, clear the command's
//    old input in the reverse table, merge the field;
//  - InputMap_FreeArrays 0x004707a0 / Bindings_Free 0x00470960 (free logged in both import slots);
//  - InputMap_CopyCommandName 0x00470f50, Input_CopyKeyName_5662bc 0x00471040 / _5662fc 0x00471070 (strncpy from a
//    name table, or the empty string 0x004e5ce0), Input_FormatKeyName 0x00470f80 (modifier prefixes by bits 8..10,
//    then the key name from 0x00565ebc by the low byte, strncat).
// Maps are consistent (every table entry a valid command, every field in range, spare top bits random); sequences of
// random getter/setter calls run on two copies. Compared: returns (pointers by role), the whole map and binding array,
// freed pointers by role, output strings.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zInput/zin_kbd.h"
#include "unattributed/input.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
constexpr int kCmds = 64;
constexpr std::uint32_t kMapWords = 0x3f60 / 4;
constexpr int kPrim = 0x14 / 4, kSec = 0x1f8c / 4, kJoy = 0x3f04 / 4, kMouse = 0x3f44 / 4;
constexpr int kKeys = (0x1f8c - 0x14) / 4;

struct Map {
    std::vector<std::uint32_t> m, b;
    void link() { m[2] = ch::addr(b.data()); }
};

Map make_map(std::mt19937& rng)
{
    Map x;
    x.m.assign(kMapWords, 0);
    x.b.assign(kCmds, 0);
    x.m[1] = kCmds;
    for (int k = 0; k < kKeys; ++k) {
        x.m[kPrim + k] = rng() % 3 ? 0 : rng() % kCmds;
        x.m[kSec + k] = rng() % 3 ? 0 : rng() % kCmds;
    }
    for (int k = 0; k < 16; ++k) x.m[kJoy + k] = rng() % kCmds;
    for (int k = 0; k < 4; ++k) x.m[kMouse + k] = rng() % kCmds;
    for (auto& w : x.b)
        w = (rng() % kKeys) | (rng() % kKeys) << 11 | (rng() % 16) << 22 | (rng() % 4) << 26 | (rng() & 0xf0000000u);
    return x;
}
}  // namespace

TEST(native_input_map_bindings_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using G = int(__fastcall*)(std::uint32_t*, int, int);
    using S = int(__fastcall*)(std::uint32_t*, int, int, int);
    const G get[8][2] = {
        {rt::original<G>(0x00470a40), reinterpret_cast<G>(&recoil::Input_GetPrimaryKeyBinding)},
        {rt::original<G>(0x00470a60), reinterpret_cast<G>(&recoil::Input_GetSecondaryKeyBinding)},
        {rt::original<G>(0x00470a80), reinterpret_cast<G>(&recoil::Input_GetJoystickButtonBinding)},
        {rt::original<G>(0x00470aa0), reinterpret_cast<G>(&recoil::Input_GetMouseButtonBinding)},
        {rt::original<G>(0x00470ac0), reinterpret_cast<G>(&recoil::InputMap_CommandForPrimaryKey)},
        {rt::original<G>(0x00470ad0), reinterpret_cast<G>(&recoil::InputMap_CommandForSecondaryKey)},
        {rt::original<G>(0x00470b00), reinterpret_cast<G>(&recoil::InputMap_CommandForJoystickButton)},
        {rt::original<G>(0x00470b10), reinterpret_cast<G>(&recoil::InputMap_CommandForMouseButton)},
    };
    const S set[4][2] = {
        {rt::original<S>(0x00470b20), reinterpret_cast<S>(&recoil::InputMap_SetPrimaryKeyBinding)},
        {rt::original<S>(0x00470b80), reinterpret_cast<S>(&recoil::InputMap_SetSecondaryKeyBinding)},
        {rt::original<S>(0x00470bf0), reinterpret_cast<S>(&recoil::InputMap_SetJoystickButtonBinding)},
        {rt::original<S>(0x00470c60), reinterpret_cast<S>(&recoil::InputMap_SetMouseButtonBinding)},
    };
    const int range[4] = {kKeys, kKeys, 16, 4};
    std::mt19937 rng(0x470b20);
    int calls = 0;
    for (int it = 0; it < 200; ++it) {
        const Map init = make_map(rng);
        Map map[2] = {init, init};
        map[0].link();
        map[1].link();
        for (int step = 0; step < 100; ++step) {
            const int op = static_cast<int>(rng() % 12), edx = static_cast<int>(rng());
            int ret[2];
            if (op < 8) {
                const int arg = static_cast<int>(op < 4 ? rng() % kCmds : rng() % range[op - 4]);
                for (int side = 0; side < 2; ++side) ret[side] = get[op][side](map[side].m.data(), edx, arg);
            } else {
                const int kind = op - 8;
                const int input = static_cast<int>(rng() % 4 ? rng() % range[kind] : 0);
                const int cmd = static_cast<int>(rng() % 4 ? rng() % kCmds : 0);
                for (int side = 0; side < 2; ++side) ret[side] = set[kind][side](map[side].m.data(), edx, input, cmd);
            }
            CHECK_EQ(ret[0], ret[1]);
            CHECK(map[0].b == map[1].b);
            bool same = true;
            for (std::uint32_t k = 0; k < kMapWords; ++k)
                if (k != 2 && map[0].m[k] != map[1].m[k]) same = false;
            CHECK(same);
            ++calls;
        }
    }
    std::printf("  input map calls %d\n", calls);
}

TEST(native_input_map_free_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int);
    const F fn[2][2] = {
        {rt::original<F>(0x004707a0), reinterpret_cast<F>(&recoil::InputMap_FreeArrays)},
        {rt::original<F>(0x00470960), reinterpret_cast<F>(&recoil::Bindings_Free)},
    };
    std::mt19937 rng(0x4707a0);
    for (int it = 0; it < 400; ++it) {
        const int which = static_cast<int>(rng() % 2), count = static_cast<int>(rng() % 6);
        const bool has_c = rng() % 4 != 0, has_names = rng() % 4 != 0, has_b = rng() % 4 != 0;
        std::vector<bool> name_null(count);
        for (int k = 0; k < count; ++k) name_null[k] = rng() % 3 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            ch::Roles rl;
            std::uint32_t m[5] = {0x1111, static_cast<std::uint32_t>(count), 0, 0, 0};
            if (has_b) { m[2] = ch::addr(ch::c_malloc(16)); rl.set(m[2], 0xB); }
            if (has_c) { m[3] = ch::addr(ch::c_malloc(16)); rl.set(m[3], 0xC); }
            std::vector<std::uint32_t> names;
            if (has_names) {
                auto* arr = static_cast<std::uint32_t*>(ch::c_malloc(4 * (count + 1)));
                m[4] = ch::addr(arr);
                rl.set(m[4], 0xA);
                for (int k = 0; k < count; ++k) {
                    arr[k] = name_null[k] ? 0 : ch::addr(ch::c_strdup("name"));
                    if (arr[k]) rl.set(arr[k], 0x100 + k);
                }
            }
            const std::uint32_t b = m[2];
            ch::freed().clear();
            {
                ch::FreeHook hook;
                fn[which][side](m, static_cast<int>(rng()));
            }
            for (std::uint32_t w : m) snap[side].push_back(rl(w));
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            // Bindings_Free zeroes +8 without freeing it: release it here (not logged)
            if (b && !ch::was_freed(b)) ch::c_free(b);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

TEST(native_input_name_copies_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int, int, char*, int);
    using K = int(__fastcall*)(int, int, int, char*, int);
    const F cmd_name[2] = {rt::original<F>(0x00470f50), reinterpret_cast<F>(&recoil::InputMap_CopyCommandName)};
    const K key_name[3][2] = {
        {rt::original<K>(0x00471040), reinterpret_cast<K>(&recoil::Input_CopyKeyName_5662bc)},
        {rt::original<K>(0x00471070), reinterpret_cast<K>(&recoil::Input_CopyKeyName_5662fc)},
        {rt::original<K>(0x00470f80), reinterpret_cast<K>(&recoil::Input_FormatKeyName)},
    };
    static const char* const pool[] = {"", "A", "Space", "Left Shift", "Joystick Button 10", "Mouse 2", "A very long key name here"};
    std::mt19937 rng(0x470f50);
    int calls = 0;
    for (int it = 0; it < 6000; ++it) {
        rt::restore_pristine();
        const int op = static_cast<int>(rng() % 4);
        const int n = static_cast<int>(1 + rng() % 40);
        const char* s = rng() % 4 ? pool[rng() % 7] : nullptr;
        char out[2][96];
        std::uint32_t ret[2];
        if (op == 0) {
            const int idx = static_cast<int>(rng() % 8);
            std::uint32_t names[8] = {};
            names[idx] = ch::addr(s);
            std::uint32_t m[5] = {0, 8, 0, 0, ch::addr(names)};
            for (int side = 0; side < 2; ++side) {
                std::memset(out[side], 'x', sizeof out[side]);
                ret[side] = static_cast<std::uint32_t>(cmd_name[side](m, 0, idx, out[side], n));
            }
        } else {
            const int k = op - 1;
            // 0x005662bc: joystick names (index 1..8 set by the init), 0x005662fc: mouse (1..3), 0x00565ebc: key names by byte
            const std::uint32_t table = k == 0 ? 0x005662bc : k == 1 ? 0x005662fc : 0x00565ebc;
            const int idx = static_cast<int>(k == 2 ? rng() % 256 : rng() % (k == 0 ? 16 : 4));
            const int arg = k == 2 ? idx | static_cast<int>((rng() % 8) << 8) | static_cast<int>(rng() % 2 ? rng() & 0xfffff800u : 0) : idx;
            for (int side = 0; side < 2; ++side) {
                *ch::img(side, table + 4 * idx) = ch::addr(s);
                std::memset(out[side], 'x', sizeof out[side]);
                ret[side] = static_cast<std::uint32_t>(key_name[k][side](arg, 0, arg, out[side], n));
            }
        }
        for (int side = 0; side < 2; ++side) {
            ch::Roles rl;
            rl.set(out[side], 1);
            rl.set(side ? ch::addr(ch::img(1, 0x004e5ce0)) : 0x004e5ce0u, 2);
            ret[side] = rl(ret[side]);
        }
        CHECK_EQ(ret[0], ret[1]);
        CHECK(std::memcmp(out[0], out[1], sizeof out[0]) == 0);
        ++calls;
    }
    rt::restore_pristine();
    std::printf("  name copy calls %d\n", calls);
}

namespace {
std::vector<int>& handled()
{
    static std::vector<int> v;
    return v;
}
void __fastcall log_handler(int cmd) { handled().push_back(cmd); }
}  // namespace

// The composites over the map: InputMap_CommandForKey (0x00470ae0: primary, else secondary), InputMap_SetCommand
// (0x00470cd0: strncpy of a non-empty name into the command's name buffer (0x4f), then the four setters; ret 0x18),
// Input_QueryActionState (0x00470eb0: consumes the edges of both keys in the keyboard table 0x00561cd0, ORs the
// joystick button edge state (buttons 1..10) and the mouse edge word 0x00561c80 +0x20/+0x24/+0x28 of its button),
// Input_Mouse_DispatchButtonEdgeHandlers (0x00470d40: for each mouse button whose edge word is 1, calls the handler of
// its command from +0xC, ECX = command), and the InputMgr_ wrappers (0x004716d0..0x00471840) on the map in [0x00565ea0].
TEST(native_input_map_composites_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using G = int(__fastcall*)(std::uint32_t*, int, int);
    using SC = int(__fastcall*)(std::uint32_t*, int, int, const char*, int, int, int, int);
    using Q = int(__fastcall*)(std::uint32_t*, int, int);
    using D = int(__fastcall*)(std::uint32_t*, int);
    using W2 = int(__fastcall*)(int, int);
    using W3 = int(__fastcall*)(int, int, int);
    const G for_key[2] = {rt::original<G>(0x00470ae0), reinterpret_cast<G>(&recoil::InputMap_CommandForKey)};
    const SC set_cmd[2] = {rt::original<SC>(0x00470cd0), reinterpret_cast<SC>(&recoil::InputMap_SetCommand)};
    const Q query[2] = {rt::original<Q>(0x00470eb0), reinterpret_cast<Q>(&recoil::Input_QueryActionState)};
    const D dispatch[2] = {rt::original<D>(0x00470d40), reinterpret_cast<D>(&recoil::Input_Mouse_DispatchButtonEdgeHandlers)};
    // wrapper, the input range of its first argument (0 = command)
    struct Wrap { std::uint32_t va; void* port; int range; bool set; };
    const Wrap wraps[] = {
        {0x004716d0, reinterpret_cast<void*>(&recoil::InputMgr_GetPrimaryKeyBinding), 0, false},
        {0x004716e0, reinterpret_cast<void*>(&recoil::InputMgr_GetSecondaryKeyBinding), 0, false},
        {0x004716f0, reinterpret_cast<void*>(&recoil::InputMgr_GetJoystickButtonBinding), 0, false},
        {0x00471700, reinterpret_cast<void*>(&recoil::InputMgr_GetMouseButtonBinding), 0, false},
        {0x00471710, reinterpret_cast<void*>(&recoil::InputMgr_CommandForPrimaryKey), kKeys, false},
        {0x00471720, reinterpret_cast<void*>(&recoil::InputMgr_CommandForSecondaryKey), kKeys, false},
        {0x00471730, reinterpret_cast<void*>(&recoil::InputMgr_CommandForJoystickButton), 16, false},
        {0x00471740, reinterpret_cast<void*>(&recoil::InputMgr_CommandForMouseButton), 4, false},
        {0x00471750, reinterpret_cast<void*>(&recoil::InputMgr_SetPrimaryKeyBinding), kKeys, true},
        {0x00471760, reinterpret_cast<void*>(&recoil::InputMgr_SetSecondaryKeyBinding), kKeys, true},
        {0x00471770, reinterpret_cast<void*>(&recoil::InputMgr_SetJoystickButtonBinding), 16, true},
    };
    const W3 name_wraps[4][2] = {
        {rt::original<W3>(0x004717e0), reinterpret_cast<W3>(&recoil::InputMgr_Call_00470f50)},
        {rt::original<W3>(0x00471800), reinterpret_cast<W3>(&recoil::InputMgr_Call_00470f80)},
        {rt::original<W3>(0x00471820), reinterpret_cast<W3>(&recoil::InputMgr_Call_00471040)},
        {rt::original<W3>(0x00471840), reinterpret_cast<W3>(&recoil::InputMgr_Call_00471070)},
    };
    static const char* const pool[] = {"", "Fire", "Turn Left", "A name of more than seventy-nine characters that strncpy cuts at 0x4f for the buffer"};
    std::mt19937 rng(0x470cd0);
    int calls = 0;
    for (int it = 0; it < 300; ++it) {
        const Map init = make_map(rng);
        Map map[2] = {init, init};
        std::vector<std::string> names[2];
        std::vector<char> name_buf[2];
        std::vector<std::uint32_t> name_ptr[2], handlers(kCmds);
        for (int k = 0; k < kCmds; ++k) handlers[k] = rng() % 3 ? ch::addr(reinterpret_cast<void*>(&log_handler)) : 0;
        for (int side = 0; side < 2; ++side) {
            map[side].link();
            name_buf[side].assign(kCmds * 0x60, 'q');
            name_ptr[side].resize(kCmds);
            for (int k = 0; k < kCmds; ++k) name_ptr[side][k] = ch::addr(&name_buf[side][k * 0x60]);
            map[side].m[3] = ch::addr(handlers.data());
            map[side].m[4] = ch::addr(name_ptr[side].data());
        }
        for (int step = 0; step < 40; ++step) {
            rt::restore_pristine();
            const int op = static_cast<int>(rng() % 6), edx = static_cast<int>(rng());
            std::uint32_t ret[2] = {0, 0};
            std::vector<std::uint32_t> extra[2];
            if (op == 0) {
                const int key = static_cast<int>(rng() % kKeys);
                for (int side = 0; side < 2; ++side) ret[side] = static_cast<std::uint32_t>(for_key[side](map[side].m.data(), edx, key));
            } else if (op == 1) {
                const int cmd = static_cast<int>(rng() % kCmds);
                const char* name = rng() % 4 ? pool[rng() % 4] : nullptr;
                const int a = static_cast<int>(rng() % 3 ? rng() % kKeys : 0), b = static_cast<int>(rng() % 3 ? rng() % kKeys : 0);
                const int j = static_cast<int>(rng() % 3 ? rng() % 16 : 0), mo = static_cast<int>(rng() % 3 ? rng() % 4 : 0);
                for (int side = 0; side < 2; ++side) ret[side] = static_cast<std::uint32_t>(set_cmd[side](map[side].m.data(), edx, cmd, name, a, b, j, mo));
            } else if (op == 2) {
                const int cmd = static_cast<int>(rng() % kCmds);
                const std::uint32_t w = map[0].b[cmd], k1 = w & 0x7ff, k2 = (w >> 11) & 0x7ff, jb = (w >> 22) & 0xf;
                std::uint32_t st1 = rng() % 8, st2 = rng() % 8, edge[3] = {rng() % 3, rng() % 3, rng() % 3};
                const unsigned char ja = static_cast<unsigned char>(rng() % 2), jc = static_cast<unsigned char>(rng() % 2);
                for (int side = 0; side < 2; ++side) {
                    *ch::img(side, 0x00561cd4 + 8 * k1) = st1;
                    *ch::img(side, 0x00561cd4 + 8 * k2) = st2;
                    for (int e = 0; e < 3; ++e) *ch::img(side, 0x00561ca0 + 4 * e) = edge[e];
                    reinterpret_cast<unsigned char*>(ch::img(side, 0x00565c03 + jb))[0] = ja;
                    reinterpret_cast<unsigned char*>(ch::img(side, 0x00565d13 + jb))[0] = jc;
                    ret[side] = static_cast<std::uint32_t>(query[side](map[side].m.data(), edx, cmd));
                    extra[side].push_back(*ch::img(side, 0x00561cd4 + 8 * k1));
                    extra[side].push_back(*ch::img(side, 0x00561cd4 + 8 * k2));
                }
            } else if (op == 3) {
                std::uint32_t edge[3] = {rng() % 3, rng() % 3, rng() % 3};
                for (int side = 0; side < 2; ++side) {
                    for (int e = 0; e < 3; ++e) *ch::img(side, 0x00561ca0 + 4 * e) = edge[e];
                    handled().clear();
                    dispatch[side](map[side].m.data(), edx);
                    for (int c : handled()) extra[side].push_back(static_cast<std::uint32_t>(c));
                }
            } else if (op == 4) {
                const Wrap& w = wraps[rng() % 11];
                const int a = static_cast<int>(w.range ? rng() % 4 ? rng() % w.range : 0 : rng() % kCmds);
                const int b = static_cast<int>(w.set ? rng() % 4 ? rng() % kCmds : 0 : edx);
                const W2 f[2] = {rt::original<W2>(w.va), reinterpret_cast<W2>(w.port)};
                for (int side = 0; side < 2; ++side) {
                    *ch::img(side, 0x00565ea0) = ch::addr(map[side].m.data());
                    ret[side] = static_cast<std::uint32_t>(f[side](a, b));
                }
            } else {
                const int k = static_cast<int>(rng() % 4), n = static_cast<int>(1 + rng() % 40);
                const char* s = rng() % 4 ? pool[rng() % 3] : nullptr;
                const int idx = static_cast<int>(k == 0 ? rng() % kCmds : k == 1 ? rng() % 256 | (rng() % 8) << 8 : k == 2 ? rng() % 16 : rng() % 4);
                const std::uint32_t table = k == 1 ? 0x00565ebc : k == 2 ? 0x005662bc : 0x005662fc;
                char out[2][96];
                for (int side = 0; side < 2; ++side) {
                    *ch::img(side, 0x00565ea0) = ch::addr(map[side].m.data());
                    if (k) *ch::img(side, table + 4 * (idx & 0xff)) = ch::addr(s);
                    std::memset(out[side], 'x', sizeof out[side]);
                    const std::uint32_t r = static_cast<std::uint32_t>(name_wraps[k][side](idx, ch::addr(out[side]), n));
                    ret[side] = r == ch::addr(out[side]) ? 1 : r == (side ? ch::addr(ch::img(1, 0x004e5ce0)) : 0x004e5ce0u) ? 2 : r ? 3 : 0;
                    extra[side].assign(out[side], out[side] + sizeof out[side]);
                }
            }
            CHECK_EQ(ret[0], ret[1]);
            CHECK(extra[0] == extra[1]);
            CHECK(map[0].b == map[1].b);
            bool same = true;
            for (std::uint32_t k = 0; k < kMapWords; ++k)
                if ((k < 2 || k > 4) && map[0].m[k] != map[1].m[k]) same = false;
            CHECK(same);
            CHECK(name_buf[0] == name_buf[1]);
            ++calls;
        }
    }
    rt::restore_pristine();
    std::printf("  input map composite calls %d\n", calls);
}

// The manager-level wrappers over the map in [0x00565ea0]: InputMgr_Call_00470cd0 (0x00471790: InputMap_SetCommand
// with ECX command, EDX name and four stack inputs, ret 0x10, returns ECX), Input_QueryAction (0x004717d0: the action
// state of command ECX), Input_Joystick_DispatchButtonHandlers (0x00470db0: for joystick buttons 1..10 whose edge
// state is 1, the handler of the button's command from ECX +0xC, called with ECX = command).
TEST(native_input_manager_wrappers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using SC = int(__fastcall*)(int, const char*, int, int, int, int);
    using Q = int(__fastcall*)(int, int);
    using D = int(__fastcall*)(std::uint32_t*, int);
    const SC set_cmd[2] = {rt::original<SC>(0x00471790), reinterpret_cast<SC>(&recoil::InputMgr_Call_00470cd0)};
    const Q query[2] = {rt::original<Q>(0x004717d0), reinterpret_cast<Q>(&recoil::Input_QueryAction)};
    const D dispatch[2] = {rt::original<D>(0x00470db0), reinterpret_cast<D>(&recoil::Input_Joystick_DispatchButtonHandlers)};
    static const char* const pool[] = {"", "Fire", "Look Up"};
    std::mt19937 rng(0x471790);
    int calls = 0;
    for (int it = 0; it < 200; ++it) {
        const Map init = make_map(rng);
        Map map[2] = {init, init};
        std::vector<char> name_buf[2];
        std::vector<std::uint32_t> name_ptr[2], handlers(kCmds);
        for (int k = 0; k < kCmds; ++k) handlers[k] = rng() % 3 ? ch::addr(reinterpret_cast<void*>(&log_handler)) : 0;
        for (int side = 0; side < 2; ++side) {
            map[side].link();
            name_buf[side].assign(kCmds * 0x60, 'q');
            name_ptr[side].resize(kCmds);
            for (int k = 0; k < kCmds; ++k) name_ptr[side][k] = ch::addr(&name_buf[side][k * 0x60]);
            map[side].m[3] = ch::addr(handlers.data());
            map[side].m[4] = ch::addr(name_ptr[side].data());
        }
        for (int step = 0; step < 30; ++step) {
            rt::restore_pristine();
            const int op = static_cast<int>(rng() % 3);
            std::uint32_t ret[2] = {0, 0};
            std::vector<std::uint32_t> extra[2];
            if (op == 0) {
                const int cmd = static_cast<int>(rng() % kCmds);
                const char* name = rng() % 4 ? pool[rng() % 3] : nullptr;
                const int a = static_cast<int>(rng() % 3 ? rng() % kKeys : 0), b = static_cast<int>(rng() % 3 ? rng() % kKeys : 0);
                const int j = static_cast<int>(rng() % 3 ? rng() % 16 : 0), mo = static_cast<int>(rng() % 3 ? rng() % 4 : 0);
                for (int side = 0; side < 2; ++side) {
                    *ch::img(side, 0x00565ea0) = ch::addr(map[side].m.data());
                    ret[side] = static_cast<std::uint32_t>(set_cmd[side](cmd, name, a, b, j, mo));
                }
            } else if (op == 1) {
                const int cmd = static_cast<int>(rng() % kCmds);
                const std::uint32_t w = map[0].b[cmd], k1 = w & 0x7ff, k2 = (w >> 11) & 0x7ff, jb = (w >> 22) & 0xf;
                const std::uint32_t st1 = rng() % 8, st2 = rng() % 8, edge[3] = {rng() % 3, rng() % 3, rng() % 3};
                const unsigned char ja = static_cast<unsigned char>(rng() % 2), jc = static_cast<unsigned char>(rng() % 2);
                for (int side = 0; side < 2; ++side) {
                    *ch::img(side, 0x00565ea0) = ch::addr(map[side].m.data());
                    *ch::img(side, 0x00561cd4 + 8 * k1) = st1;
                    *ch::img(side, 0x00561cd4 + 8 * k2) = st2;
                    for (int e = 0; e < 3; ++e) *ch::img(side, 0x00561ca0 + 4 * e) = edge[e];
                    reinterpret_cast<unsigned char*>(ch::img(side, 0x00565c03 + jb))[0] = ja;
                    reinterpret_cast<unsigned char*>(ch::img(side, 0x00565d13 + jb))[0] = jc;
                    ret[side] = static_cast<std::uint32_t>(query[side](cmd, 0));
                    extra[side].push_back(*ch::img(side, 0x00561cd4 + 8 * k1));
                    extra[side].push_back(*ch::img(side, 0x00561cd4 + 8 * k2));
                }
            } else {
                unsigned char now[11], then[11];
                for (int b = 0; b < 11; ++b) { now[b] = static_cast<unsigned char>(rng() % 2); then[b] = static_cast<unsigned char>(rng() % 2); }
                for (int side = 0; side < 2; ++side) {
                    *ch::img(side, 0x00565ea0) = ch::addr(map[side].m.data());
                    for (int b = 0; b < 11; ++b) {
                        reinterpret_cast<unsigned char*>(ch::img(side, 0x00565c03 + b))[0] = now[b];
                        reinterpret_cast<unsigned char*>(ch::img(side, 0x00565d13 + b))[0] = then[b];
                    }
                    handled().clear();
                    dispatch[side](map[side].m.data(), 0);
                    for (int c : handled()) extra[side].push_back(static_cast<std::uint32_t>(c));
                }
            }
            CHECK_EQ(ret[0], ret[1]);
            CHECK(extra[0] == extra[1]);
            CHECK(map[0].b == map[1].b);
            bool same = true;
            for (std::uint32_t k = 0; k < kMapWords; ++k)
                if ((k < 2 || k > 4) && map[0].m[k] != map[1].m[k]) same = false;
            CHECK(same);
            CHECK(name_buf[0] == name_buf[1]);
            ++calls;
        }
    }
    rt::restore_pristine();
    std::printf("  input manager wrapper calls %d\n", calls);
}
