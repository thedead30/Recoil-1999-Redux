// Structured native L1s for the DirectInput device calls (P3 input, ported in the cloud). The devices are a fake COM
// object whose vtable logs every call (slot, arguments, and the property block a pointer argument reaches) and returns
// a scripted HRESULT; the same object serves both sides.
//  - Input_Keyboard_ShutdownDevice (0x0046f420): Unacquire (+0x20) and Release (+0x8) of [0x00561cc8], free of
//    [0x00561ccc] (logged free);
//  - Input_Mouse_SetAcquired (0x00470310): by the flag [0x00561c74], Unacquire or Acquire (+0x1c) of [0x00565e78];
//    the flag flips unless the call returned 0 or 1;
//  - joystick [0x00565bd0]: Input_Joystick_Acquire (0x00471fb0), Joystick_SetDeadzone (0x004721a0: SetProperty +0x18
//    with property 5 and a 0x14-byte block), Joystick_SetAxisRange (0x004721e0: property 4, 0x18 bytes, ret 4),
//    Joystick_GetAxisRange (0x00472230: GetProperty +0x14, min/max out through EDX and the stack pointer, ret 4),
//    Input_Joystick_ShutdownDevice (0x00472280), Input_Joystick_AcquireOk (0x00472450: +0x48 with ECX, EDX, an out
//    word and 0; the out word, or 0 on a negative HRESULT);
//  - DInput_ReportError (0x00472490): message by HRESULT through sprintf to the silent reporter; 1 for S_OK, else 0.
// Compared: returns, the call logs, the globals the functions write, freed pointers by role.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zInput/zin_kbd.h"
#include "GameZRecoil/zInput/zin_init.h"
#include "unattributed/input.h"
#include "Battlesport/Briefing.h"
#include "platform/iat_kernel32.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <cstring>
#include <vector>

namespace {
struct Fake {
    void** vtbl;
};
std::vector<std::uint32_t>& calls()
{
    static std::vector<std::uint32_t> v;
    return v;
}
std::uint32_t& next_hr()
{
    static std::uint32_t h = 0;
    return h;
}
std::uint32_t& out_word()
{
    static std::uint32_t w = 0;
    return w;
}
long __stdcall m1(int slot, Fake*) { calls().push_back(slot); return static_cast<long>(next_hr()); }
template <int S> long __stdcall one(Fake* f) { return m1(S, f); }
// Get/SetProperty(this, property id, DIPROPHEADER*): logs the id and the whole block (its size is its first word)
template <int S> long __stdcall prop(Fake*, std::uint32_t id, std::uint32_t* hdr)
{
    calls().push_back(S);
    calls().push_back(id);
    for (std::uint32_t k = 0; k < hdr[0] / 4 && k < 8; ++k) calls().push_back(k < 4 || S == 0x18 ? hdr[k] : 0);  // GetProperty: +0x10 and +0x14 are outputs
    if (S == 0x14) { hdr[4] = 0x1111 + out_word(); hdr[5] = 0x2222 + out_word(); }
    return static_cast<long>(next_hr());
}
// set by the force-feedback test: the GUID address by role and the effect block (b) by value, with its three pointers
// followed (two words each)
bool& ff_mode()
{
    static bool m = false;
    return m;
}
std::uint32_t& ff_guid()
{
    static std::uint32_t g = 0;
    return g;
}
long __stdcall slot48(Fake*, std::uint32_t a, std::uint32_t b, std::uint32_t* out, std::uint32_t z)
{
    calls().push_back(0x48);
    if (ff_mode()) {
        calls().push_back(a == ff_guid() ? 0x6u : 0xbad);
        const std::uint32_t* e = ch::at(b);
        for (int k = 0; k < 13; ++k) {
            if (k == 8 || k == 9 || k == 12) { calls().push_back(ch::at(e[k])[0]); calls().push_back(k == 12 ? 0 : ch::at(e[k])[1]); }
            else calls().push_back(e[k]);
        }
    } else {
        calls().push_back(a);
        calls().push_back(b);
    }
    calls().push_back(z);
    *out = out_word();
    return static_cast<long>(next_hr());
}
// GetDeviceData(this, record size, records, in/out count, flags): hands out the scripted keyboard records; after
// kb_empty() empty polls, one press record (so a waiting call ends)
struct KeyRec { std::uint32_t ofs, data, time, seq; };
std::vector<KeyRec>& kb_script()
{
    static std::vector<KeyRec> v;
    return v;
}
int& kb_polls()
{
    static int n = 0;
    return n;
}
long __stdcall slot28(Fake*, std::uint32_t size, KeyRec* recs, std::uint32_t* count, std::uint32_t flags)
{
    calls().push_back(0x28);
    calls().push_back(size);
    calls().push_back(*count);
    calls().push_back(flags);
    const int poll = kb_polls()++;
    std::uint32_t n = 0;
    if (poll == 0) for (const KeyRec& r : kb_script()) if (n < *count) recs[n++] = r;
    if (poll >= 3) recs[n++] = KeyRec{0x10u + static_cast<std::uint32_t>(poll), 0x80, 7, 9};
    *count = n;
    return static_cast<long>(poll == 0 ? next_hr() : 0);
}
// GetDeviceState(this, size, block): fills the block from a seed; Poll (+0x64) only logs
std::uint32_t& state_seed()
{
    static std::uint32_t v = 0;
    return v;
}
int& press_after()
{
    static int v = -1;
    return v;
}
int& press_button()
{
    static int v = 0;
    return v;
}
int& polls24()
{
    static int v = 0;
    return v;
}
long __stdcall slot24(Fake*, std::uint32_t size, std::uint32_t* block)
{
    calls().push_back(0x24);
    calls().push_back(size);
    for (std::uint32_t k = 0; k < size / 4; ++k) block[k] = state_seed() * 2654435761u + k * 40503u;
    if (press_after() >= 0) {  // the button-wait test: buttons up until poll press_after(), then button press_button()
        unsigned char* b = reinterpret_cast<unsigned char*>(block) + 0x30;
        for (int k = 0; k < 32; ++k) b[k] = 0;
        if (polls24()++ >= press_after()) b[press_button()] = 0x80;
    }
    return static_cast<long>(next_hr());
}
std::vector<int>& handled()
{
    static std::vector<int> v;
    return v;
}
void __fastcall log_handler(int cmd) { handled().push_back(cmd); }
long __stdcall unexpected(Fake*) { calls().push_back(0xdead); return 0; }

Fake* fake()
{
    static void* vt[40];
    static Fake f{vt};
    for (void*& p : vt) p = reinterpret_cast<void*>(&unexpected);
    vt[0x8 / 4] = reinterpret_cast<void*>(&one<0x8>);
    vt[0x14 / 4] = reinterpret_cast<void*>(&prop<0x14>);
    vt[0x18 / 4] = reinterpret_cast<void*>(&prop<0x18>);
    vt[0x1c / 4] = reinterpret_cast<void*>(&one<0x1c>);
    vt[0x20 / 4] = reinterpret_cast<void*>(&one<0x20>);
    vt[0x24 / 4] = reinterpret_cast<void*>(&slot24);
    vt[0x28 / 4] = reinterpret_cast<void*>(&slot28);
    vt[0x64 / 4] = reinterpret_cast<void*>(&one<0x64>);
    vt[0x48 / 4] = reinterpret_cast<void*>(&slot48);
    return &f;
}

std::uint32_t pick_hr(std::mt19937& rng)
{
    static const std::uint32_t v[] = {0, 1, 0x80070005u, 0x8007001eu, 0x00000002u};
    return rng() % 3 == 0 ? rng() : v[rng() % 5];
}
}  // namespace

TEST(native_input_devices_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F2 = int(__fastcall*)(int, int);
    using F3 = int(__fastcall*)(int, int, int);
    const F2 kbd_shut[2] = {rt::original<F2>(0x0046f420), reinterpret_cast<F2>(&recoil::Input_Keyboard_ShutdownDevice)};
    const F2 mouse_acq[2] = {rt::original<F2>(0x00470310), reinterpret_cast<F2>(&recoil::Input_Mouse_SetAcquired)};
    const F2 joy_acq[2] = {rt::original<F2>(0x00471fb0), reinterpret_cast<F2>(&recoil::Input_Joystick_Acquire)};
    const F2 dead[2] = {rt::original<F2>(0x004721a0), reinterpret_cast<F2>(&recoil::Joystick_SetDeadzone)};
    const F3 set_range[2] = {rt::original<F3>(0x004721e0), reinterpret_cast<F3>(&recoil::Joystick_SetAxisRange)};
    const F3 get_range[2] = {rt::original<F3>(0x00472230), reinterpret_cast<F3>(&recoil::Joystick_GetAxisRange)};
    const F2 joy_shut[2] = {rt::original<F2>(0x00472280), reinterpret_cast<F2>(&recoil::Input_Joystick_ShutdownDevice)};
    const F2 acq_ok[2] = {rt::original<F2>(0x00472450), reinterpret_cast<F2>(&recoil::Input_Joystick_AcquireOk)};
    Fake* dev = fake();
    std::mt19937 rng(0x472450);
    int n = 0;
    for (int it = 0; it < 4000; ++it) {
        const int op = static_cast<int>(rng() % 8);
        const bool present = rng() % 5 != 0;
        const std::uint32_t hr = pick_hr(rng), word = rng(), flag = rng() % 3 == 0 ? rng() : rng() % 2;
        const int a = static_cast<int>(rng() % 3 ? rng() % 20000 : rng()), b = static_cast<int>(rng() % 3 ? rng() % 20000 : rng());
        const bool has_buf = rng() % 3 != 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            rl.set(dev, 1);
            calls().clear();
            next_hr() = hr;
            out_word() = word;
            const std::uint32_t d = present ? ch::addr(dev) : 0;
            int ret = 0;
            std::uint32_t o1 = 0xabababab, o2 = 0xcdcdcdcd;
            if (op == 0) {
                const std::uint32_t buf = has_buf ? ch::addr(ch::c_malloc(32)) : 0;
                rl.set(buf, 2);
                *ch::img(side, 0x00561cc8) = d;
                *ch::img(side, 0x00561ccc) = buf;
                ch::freed().clear();
                {
                    ch::FreeHook hook;
                    ret = kbd_shut[side](a, b);
                }
                for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            } else if (op == 1) {
                *ch::img(side, 0x00565e78) = d;
                *ch::img(side, 0x00561c74) = flag;
                mouse_acq[side](a, b);  // returns whatever EAX held
                snap[side].push_back(*ch::img(side, 0x00561c74));
            } else {
                *ch::img(side, 0x00565bd0) = op == 2 || op == 6 || op == 7 ? d : ch::addr(dev);
                if (op == 2) ret = joy_acq[side](a, b);
                else if (op == 3) ret = dead[side](a, b);
                else if (op == 4) ret = set_range[side](a, b, static_cast<int>(word));
                else if (op == 5) { ret = get_range[side](a, static_cast<int>(ch::addr(&o1)), static_cast<int>(ch::addr(&o2))); }
                else if (op == 6) ret = joy_shut[side](a, b);
                else ret = acq_ok[side](a, b);
                snap[side].push_back(rl(*ch::img(side, 0x00565bd0)));
            }
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            snap[side].push_back(o1);
            snap[side].push_back(o2);
            snap[side].insert(snap[side].end(), calls().begin(), calls().end());
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++n;
    }
    rt::restore_pristine();
    std::printf("  DirectInput device calls %d\n", n);
}

TEST(native_dinput_report_error_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t, const char*, const char*);
    const F fn[2] = {rt::original<F>(0x00472490), reinterpret_cast<F>(&recoil::DInput_ReportError)};
    static const std::uint32_t known[] = {0, 1, 0x80040110u, 0x80004001u, 0x80004002u, 0x80004005u, 0x80070002u, 0x80040154u,
                                          0x8007000cu, 0x80070005u, 0x80070015u, 0x8007000eu, 0x80070057u, 0x8007001eu,
                                          0x800704dfu, 0x80040111u, 0x8007000au, 0x7fffffffu, 0xffffffffu};
    std::mt19937 rng(0x472490);
    for (int it = 0; it < 2000; ++it) {
        const std::uint32_t hr = it < 19 ? known[it] : rng() % 2 ? known[rng() % 19] + (rng() % 3) - 1 : rng();
        int ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::wipe_stack();
            ret[side] = fn[side](hr, "context", "Input.c");
        }
        CHECK_EQ(ret[0], ret[1]);
    }
    rt::restore_pristine();
}

// Input_Keyboard_WaitForKeyPress (0x0046fa10): ECX = wait flag; polls GetDeviceData (+0x28, 16-byte records, count 1,
// into [0x00561ccc]); DIERR_INPUTLOST re-acquires, other errors report and return 0; per record: Alt / Ctrl / Shift
// (0x38/0xb8, 0x1d/0x9d, 0x2a/0x36) track the modifier bits of [0x00561cd0], other keys get the modifier flags ORed
// into the record and update the keyboard table entry (key | modifiers); returns the last pressed key code, looping
// while none and the flag is set. Scripted: a first poll with 0..3 records (its HRESULT from the list below), then
// empty polls until the fourth, which presses a key. Compared: return, call log, the record buffer, [0x00561cd0] and
// the whole keyboard table.
TEST(native_input_keyboard_wait_for_key_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F fn[2] = {rt::original<F>(0x0046fa10), reinterpret_cast<F>(&recoil::Input_Keyboard_WaitForKeyPress)};
    static const std::uint32_t special[] = {0x38, 0xb8, 0x1d, 0x9d, 0x2a, 0x36};
    Fake* dev = fake();
    std::mt19937 rng(0x46fa10);
    for (int it = 0; it < 3000; ++it) {
        const int wait = static_cast<int>(rng() % 3 == 0);
        const std::uint32_t hr = rng() % 4 ? 0 : rng() % 2 ? 0x8007001eu : 0x80070005u, mods = (rng() % 8) << 8;
        std::vector<KeyRec> script(rng() % 4);
        for (KeyRec& r : script)
            r = KeyRec{rng() % 3 == 0 ? special[rng() % 6] : rng() % 0xd0, rng() % 2 ? 0x80u : rng() % 0x80, rng(), rng()};
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            calls().clear();
            next_hr() = hr;
            kb_script() = script;
            kb_polls() = 0;
            std::uint32_t buf[4 * 8];
            for (std::uint32_t& w : buf) w = 0x5a5a5a5a;
            *ch::img(side, 0x00561cc8) = ch::addr(dev);
            *ch::img(side, 0x00561ccc) = ch::addr(buf);
            *ch::img(side, 0x00561cd0) = mods;
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](wait, 0)));
            snap[side].insert(snap[side].end(), calls().begin(), calls().end());
            snap[side].insert(snap[side].end(), buf, buf + 32);
            for (std::uint32_t k = 0; k < 2 * 2014 + 2; ++k) snap[side].push_back(*ch::img(side, 0x00561cd0 + 4 * k));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// ForceFeedback_CreateConstantEffect (0x00430070): builds a constant-force effect block on the stack (magnitude 10000,
// two axes, direction {ECX, 0}) and hands it to Input_Joystick_AcquireOk with ECX = the GUID at 0x004d2850.
// Input_Mouse_ShutdownDevice (0x00470360): flag 0, Input_Mouse_SetAcquired, Release, both globals cleared, returns 1.
// Joystick_ApplyAxisRanges (0x00471fd0): per axis (X at 0, Y at 4, then 8 and 0x14 when [0x00565e14] > 2 / > 3)
// SetAxisRange, GetAxisRange on failure, the centre and 2/range as floats, SetDeadzone; returns the AND of the
// deadzone results. Compared: returns, call logs, the globals, the whole config block.
TEST(native_input_device_composites_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    using FP = int(__fastcall*)(std::uint32_t*, int);
    const F ff[2] = {rt::original<F>(0x00430070), reinterpret_cast<F>(&recoil::ForceFeedback_CreateConstantEffect)};
    const F mouse_shut[2] = {rt::original<F>(0x00470360), reinterpret_cast<F>(&recoil::Input_Mouse_ShutdownDevice)};
    const FP apply[2] = {rt::original<FP>(0x00471fd0), reinterpret_cast<FP>(&recoil::Joystick_ApplyAxisRanges)};
    Fake* dev = fake();
    std::mt19937 rng(0x471fd0);
    for (int it = 0; it < 3000; ++it) {
        const int op = static_cast<int>(rng() % 3);
        const std::uint32_t hr = pick_hr(rng), word = rng(), flag = rng() % 2, axes = rng() % 6;
        const int dir = static_cast<int>(rng() % 3 ? rng() % 20000 - 10000 : rng());
        const bool present = rng() % 5 != 0, null_cfg = rng() % 10 == 0;
        std::uint32_t cfg0[32];
        for (std::uint32_t& w : cfg0) w = rng() % 2 ? rng() % 2000 : rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            calls().clear();
            next_hr() = hr;
            out_word() = word;
            ch::wipe_stack();
            int ret = 0;
            if (op == 0) {
                ff_mode() = true;
                ff_guid() = side ? ch::addr(ch::img(1, 0x004d2850)) : 0x004d2850u;
                *ch::img(side, 0x00565bd0) = present ? ch::addr(dev) : 0;
                ret = ff[side](dir, 0);
                ff_mode() = false;
            } else if (op == 1) {
                *ch::img(side, 0x00565e78) = present ? ch::addr(dev) : 0;
                *ch::img(side, 0x00561c74) = flag;
                ret = mouse_shut[side](0, 0);
                snap[side].push_back(*ch::img(side, 0x00565e78));
                snap[side].push_back(*ch::img(side, 0x00565e74));
                snap[side].push_back(*ch::img(side, 0x00561c74));
            } else {
                std::uint32_t cfg[32];
                std::memcpy(cfg, cfg0, sizeof cfg);
                *ch::img(side, 0x00565bd0) = ch::addr(dev);
                *ch::img(side, 0x00565e14) = axes;
                ret = apply[side](null_cfg ? nullptr : cfg, 0);
                snap[side].insert(snap[side].end(), cfg, cfg + 32);
            }
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            snap[side].insert(snap[side].end(), calls().begin(), calls().end());
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// Joystick_EnableAndSetRanges (0x0042e170): ECX = enable; when enabled and present ([0x00565bcc] == 1) takes an
// acquire count (resetting the button state on the first), fills the axis config block 0x004f3350 (ranges +-1000,
// deadzones, saturation) and applies it (Joystick_ApplyAxisRanges); otherwise drops a count; returns 1 when applied.
// zInShutdown (0x00471c10): with a window ([0x00561cb4]) shuts the joystick, keyboard and mouse devices down, releases
// the DirectInput object [0x00561cb0], clears the window; 1 without one, else 0.
// Input_Joystick_Poll (0x004722c0): CL = dispatch flag; when present, Poll (+0x64) and GetDeviceState (+0x24, 0x110
// bytes into 0x00566310), zeroing the axes past [0x00565e14]; DIERR_INPUTLOST re-acquires and returns 0, other errors 0;
// on success the current state 0x00565bd4 moves to 0x00565ce4, the new one to 0x00565bd4, the button handlers run
// when the flag is set, and it returns 0x00565bd4.
// Compared: returns (addresses by role), call logs, handler calls, freed pointers by role, and the globals written.
TEST(native_input_device_lifecycle_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F enable[2] = {rt::original<F>(0x0042e170), reinterpret_cast<F>(&recoil::Joystick_EnableAndSetRanges)};
    const F shut[2] = {rt::original<F>(0x00471c10), reinterpret_cast<F>(&recoil::zInShutdown)};
    const F poll[2] = {rt::original<F>(0x004722c0), reinterpret_cast<F>(&recoil::Input_Joystick_Poll)};
    Fake* dev = fake();
    std::mt19937 rng(0x4722c0);
    for (int it = 0; it < 3000; ++it) {
        const int op = static_cast<int>(rng() % 3), flag = static_cast<int>(rng() % 2 ? rng() % 2 : rng());
        const std::uint32_t hr = rng() % 2 ? 0 : pick_hr(rng), word = rng(), present = rng() % 4 ? 1 : rng() % 3;
        const std::uint32_t count = rng() % 3 ? rng() % 3 : rng() & 0xffff, gate = rng() % 4 ? 0x100 : 0, axes = rng() % 6;
        const bool window = rng() % 4 != 0, kb = rng() % 3 != 0, has_buf = rng() % 2 != 0, di = rng() % 3 != 0, joy = rng() % 2 != 0;
        // a map for the dispatch: every joystick button bound to a command with a logging handler
        std::vector<std::uint32_t> handlers(64, ch::addr(reinterpret_cast<void*>(&log_handler)));
        std::vector<std::uint32_t> map(0x3f60 / 4, 0);
        map[3] = ch::addr(handlers.data());
        for (int b = 0; b < 16; ++b) map[0x3f04 / 4 + b] = rng() % 64;
        const std::uint32_t seed = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::Roles rl;
            rl.set(dev, 1);
            rl.set(side ? ch::addr(ch::img(1, 0x00565bd4)) : 0x00565bd4u, 2);
            calls().clear();
            handled().clear();
            next_hr() = hr;
            out_word() = word;
            state_seed() = seed;
            ch::wipe_stack();
            int ret = 0;
            if (op == 0) {
                *ch::img(side, 0x00565bcc) = present;
                *ch::img(side, 0x00565bd0) = ch::addr(dev);
                *ch::img(side, 0x00565e14) = axes;
                reinterpret_cast<std::uint16_t*>(ch::img(side, 0x00561cbc))[1] = static_cast<std::uint16_t>(count);
                *ch::img(side, 0x00561cb8) = (*ch::img(side, 0x00561cb8) & ~0x100u) | gate;
                ret = enable[side](flag, 0);
                snap[side].push_back(reinterpret_cast<std::uint16_t*>(ch::img(side, 0x00561cbc))[1]);
                for (std::uint32_t k = 0; k < 0x50 / 4; ++k) snap[side].push_back(*ch::img(side, 0x004f3350 + 4 * k));
                for (std::uint32_t k = 0; k < 0x44; ++k) snap[side].push_back(*ch::img(side, 0x00565bd4 + 4 * k));
            } else if (op == 1) {
                const std::uint32_t buf = has_buf ? ch::addr(ch::c_malloc(32)) : 0;
                rl.set(buf, 3);
                *ch::img(side, 0x00561cb4) = window ? 0x1234 : 0;
                *ch::img(side, 0x00561cb0) = di ? ch::addr(dev) : 0;
                *ch::img(side, 0x00565bd0) = joy ? ch::addr(dev) : 0;
                *ch::img(side, 0x00561cc8) = kb ? ch::addr(dev) : 0;
                *ch::img(side, 0x00561ccc) = buf;
                *ch::img(side, 0x00565e78) = ch::addr(dev);
                ch::freed().clear();
                {
                    ch::FreeHook hook;
                    ret = shut[side](0, 0);
                }
                if (buf && !ch::was_freed(buf)) ch::c_free(buf);
                for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
                for (std::uint32_t va : {0x00561cb4u, 0x00565bd0u, 0x00565e78u, 0x00565e74u, 0x00561c74u}) snap[side].push_back(rl(*ch::img(side, va)));
            } else {
                *ch::img(side, 0x00565bcc) = present;
                *ch::img(side, 0x00565bd0) = ch::addr(dev);
                *ch::img(side, 0x00565e14) = axes;
                *ch::img(side, 0x00565ea0) = ch::addr(map.data());
                ret = poll[side](flag, 0);
                for (std::uint32_t k = 0; k < 0x44; ++k) {
                    snap[side].push_back(*ch::img(side, 0x00565bd4 + 4 * k));
                    snap[side].push_back(*ch::img(side, 0x00565ce4 + 4 * k));
                    snap[side].push_back(*ch::img(side, 0x00566310 + 4 * k));
                }
            }
            snap[side].push_back(rl(static_cast<std::uint32_t>(ret)));
            snap[side].insert(snap[side].end(), calls().begin(), calls().end());
            for (int c : handled()) snap[side].push_back(static_cast<std::uint32_t>(c));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// Input_WaitJoystickButton (0x004723d0): ECX = wait flag; polls the joystick with dispatch (Input_Joystick_Poll, CL = 1),
// takes the first button 1..10 whose edge state is 1, resets the button state; loops while none and the flag is set;
// returns the button (0 for none). Scripted device: buttons up for 0..3 polls, then one of rgbButtons[0..11] down. Compared:
// return, call log, handler calls, the three state blocks.
TEST(native_input_wait_joystick_button_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F fn[2] = {rt::original<F>(0x004723d0), reinterpret_cast<F>(&recoil::Input_WaitJoystickButton)};
    Fake* dev = fake();
    std::mt19937 rng(0x4723d0);
    for (int it = 0; it < 1000; ++it) {
        const int button = static_cast<int>(rng() % 12), after = static_cast<int>(rng() % 4);
        // rgbButtons[k] is edge button k + 1: wait only when one of edge buttons 1..10 will come down
        const int wait = button <= 9 && rng() % 2 ? 1 : 0;
        const std::uint32_t seed = rng(), axes = rng() % 6;
        std::vector<std::uint32_t> handlers(64, ch::addr(reinterpret_cast<void*>(&log_handler)));
        std::vector<std::uint32_t> map(0x3f60 / 4, 0);
        map[3] = ch::addr(handlers.data());
        for (int b = 0; b < 16; ++b) map[0x3f04 / 4 + b] = rng() % 64;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            calls().clear();
            handled().clear();
            next_hr() = 0;
            state_seed() = seed;
            press_after() = after;
            press_button() = button;
            polls24() = 0;
            *ch::img(side, 0x00565bcc) = 1;
            *ch::img(side, 0x00565bd0) = ch::addr(dev);
            *ch::img(side, 0x00565e14) = axes;
            *ch::img(side, 0x00565ea0) = ch::addr(map.data());
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](wait, 0)));
            snap[side].insert(snap[side].end(), calls().begin(), calls().end());
            for (int c : handled()) snap[side].push_back(static_cast<std::uint32_t>(c));
            for (std::uint32_t k = 0; k < 0x44; ++k) {
                snap[side].push_back(*ch::img(side, 0x00565bd4 + 4 * k));
                snap[side].push_back(*ch::img(side, 0x00565ce4 + 4 * k));
                snap[side].push_back(*ch::img(side, 0x00566310 + 4 * k));
            }
        }
        press_after() = -1;
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// Briefing_WaitKeyOrTimeout (0x00404140; menus, ported in the cloud; ECX milliseconds): while time is left, one
// non-waiting Input_Keyboard_WaitForKeyPress poll, then Sleep(100) (a logging fake over both import slots) and 100 ms
// less; 1 as soon as a poll returns a key, else 0. Keyboard scripted as the wait test above (a key on the fourth poll
// at the latest). Compared: return, the device call log (with the sleeps), the keyboard globals.
namespace {
void __stdcall fake_sleep(std::uint32_t ms) { calls().push_back(0x51EE); calls().push_back(ms); }
}  // namespace
TEST(native_briefing_wait_key_or_timeout_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(int, int);
    const F fn[2] = {rt::original<F>(0x00404140), reinterpret_cast<F>(&recoil::Briefing_WaitKeyOrTimeout)};
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc0b0));
    void* const saved[2] = {*o, recoil::g_Iat_Sleep_004cc0b0};
    *o = recoil::g_Iat_Sleep_004cc0b0 = reinterpret_cast<void*>(&fake_sleep);
    Fake* dev = fake();
    std::mt19937 rng(0x404140);
    for (int it = 0; it < 600; ++it) {
        const int ms = static_cast<int>(rng() % 5 == 0 ? -(static_cast<int>(rng() % 50)) : rng() % 600);
        const std::uint32_t hr = rng() % 4 ? 0 : 0x8007001eu;
        std::vector<KeyRec> script(rng() % 3);
        for (KeyRec& r : script) r = KeyRec{rng() % 0xd0, rng() % 2 ? 0x80u : rng() % 0x80, rng(), rng()};
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            calls().clear();
            next_hr() = hr;
            kb_script() = script;
            kb_polls() = 0;
            std::uint32_t buf[4 * 8];
            for (std::uint32_t& w : buf) w = 0x5a5a5a5a;
            *ch::img(side, 0x00561cc8) = ch::addr(dev);
            *ch::img(side, 0x00561ccc) = ch::addr(buf);
            *ch::img(side, 0x00561cd0) = 0;
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](ms, 0)));
            snap[side].insert(snap[side].end(), calls().begin(), calls().end());
            snap[side].insert(snap[side].end(), buf, buf + 32);
            for (std::uint32_t k = 0; k < 2 * 2014 + 2; ++k) snap[side].push_back(*ch::img(side, 0x00561cd0 + 4 * k));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    *o = saved[0];
    recoil::g_Iat_Sleep_004cc0b0 = saved[1];
    rt::restore_pristine();
}
