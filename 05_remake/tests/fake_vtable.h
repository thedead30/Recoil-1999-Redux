// A fake C++ vtable for structured native tests of functions that call virtual methods on objects the test builds
// (03_re/CLOUD.md). Every slot is a thunk into one logger: it records the slot offset, `this` and the stack arguments
// the test configured for that slot, returns the configured value, and removes the configured number of argument bytes
// (thiscall callee-clean; 0 for the variadic slots whose caller cleans). The same table serves both sides; words are
// logged through a normaliser the test sets (object pointers by role, the port's image addresses as original VAs).
// Configure the slots each function uses before the calls: a slot left unconfigured logs 0xDEAD and cleans nothing.
#pragma once

#include <cstdint>
#include <functional>
#include <vector>

extern "C" {
inline std::uint32_t vt_clean_bytes = 0;  // read by vt_common after the logger returns
}

namespace vt {

struct Slot {
    bool used = false;
    int clean = 0;           // stack argument words the method removes
    int logged = 0;          // stack argument words logged (a variadic slot logs what its caller pushed)
    int deref = 0;           // words logged behind the first argument (a pointer to a rect or a record)
    std::uint32_t ret = 0;   // EAX
    std::function<std::uint32_t(std::uint32_t self, const std::uint32_t* args)> fn;  // optional EAX by call
};
constexpr int kSlots = 40;
inline Slot* slots()
{
    static Slot s[kSlots];
    return s;
}
inline std::vector<std::uint32_t>& log()
{
    static std::vector<std::uint32_t> v;
    return v;
}
inline std::function<std::uint32_t(std::uint32_t)>& norm()
{
    static std::function<std::uint32_t(std::uint32_t)> f = [](std::uint32_t v) { return v; };
    return f;
}
inline void reset()
{
    for (int k = 0; k < kSlots; ++k) slots()[k] = Slot{};
    log().clear();
}
// offset in bytes, argument words removed, argument words logged, words logged behind argument 0, EAX
inline void set(int offset, int clean, int logged = -1, int deref = 0, std::uint32_t ret = 0)
{
    Slot& s = slots()[offset / 4];
    s.used = true;
    s.clean = clean;
    s.logged = logged < 0 ? clean : logged;
    s.deref = deref;
    s.ret = ret;
}

extern "C" inline std::uint32_t __cdecl vt_log(std::uint32_t index, std::uint32_t self, const std::uint32_t* args)
{
    Slot& s = slots()[index];
    log().push_back(0x5100 + index * 4);
    if (!s.used) {
        log().push_back(0xDEAD);
        vt_clean_bytes = 0;
        return 0;
    }
    log().push_back(norm()(self));
    for (int k = 0; k < s.logged; ++k) log().push_back(norm()(args[k]));
    if (s.deref && s.logged) {
        const auto* p = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(args[0]));
        for (int k = 0; k < s.deref; ++k) log().push_back(p ? norm()(p[k]) : 0xDEADu);
    }
    vt_clean_bytes = static_cast<std::uint32_t>(4 * s.clean);
    return s.fn ? s.fn(self, args) : s.ret;
}

// [esp] = slot index, [esp+4] = return address, [esp+8] = the arguments
__declspec(naked) inline void vt_common()
{
    __asm {
        push ecx
        lea eax, [esp + 0xc]
        push eax
        push ecx
        push dword ptr [esp + 0xc]
        call vt_log
        add esp, 0xc
        pop ecx
        mov edx, vt_clean_bytes
        push eax
        mov eax, [esp + 8]
        mov [esp + 8 + edx], eax
        pop eax
        lea esp, [esp + 4 + edx]
        ret
    }
}

#define VT_THUNK(n) \
    __declspec(naked) inline void vt_thunk_##n() { __asm { push n } __asm { jmp vt_common } }
VT_THUNK(0) VT_THUNK(1) VT_THUNK(2) VT_THUNK(3) VT_THUNK(4) VT_THUNK(5) VT_THUNK(6) VT_THUNK(7) VT_THUNK(8) VT_THUNK(9)
VT_THUNK(10) VT_THUNK(11) VT_THUNK(12) VT_THUNK(13) VT_THUNK(14) VT_THUNK(15) VT_THUNK(16) VT_THUNK(17) VT_THUNK(18)
VT_THUNK(19) VT_THUNK(20) VT_THUNK(21) VT_THUNK(22) VT_THUNK(23) VT_THUNK(24) VT_THUNK(25) VT_THUNK(26) VT_THUNK(27)
VT_THUNK(28) VT_THUNK(29) VT_THUNK(30) VT_THUNK(31) VT_THUNK(32) VT_THUNK(33) VT_THUNK(34) VT_THUNK(35) VT_THUNK(36)
VT_THUNK(37) VT_THUNK(38) VT_THUNK(39)
#undef VT_THUNK

inline std::uint32_t table()
{
    static void* t[kSlots] = {
        &vt_thunk_0, &vt_thunk_1, &vt_thunk_2, &vt_thunk_3, &vt_thunk_4, &vt_thunk_5, &vt_thunk_6, &vt_thunk_7,
        &vt_thunk_8, &vt_thunk_9, &vt_thunk_10, &vt_thunk_11, &vt_thunk_12, &vt_thunk_13, &vt_thunk_14, &vt_thunk_15,
        &vt_thunk_16, &vt_thunk_17, &vt_thunk_18, &vt_thunk_19, &vt_thunk_20, &vt_thunk_21, &vt_thunk_22, &vt_thunk_23,
        &vt_thunk_24, &vt_thunk_25, &vt_thunk_26, &vt_thunk_27, &vt_thunk_28, &vt_thunk_29, &vt_thunk_30, &vt_thunk_31,
        &vt_thunk_32, &vt_thunk_33, &vt_thunk_34, &vt_thunk_35, &vt_thunk_36, &vt_thunk_37, &vt_thunk_38, &vt_thunk_39,
    };
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(t));
}

}  // namespace vt
