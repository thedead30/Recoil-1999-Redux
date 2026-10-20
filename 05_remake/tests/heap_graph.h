// Heap-graph oracle support for native L1 tests of code that builds linked structures.
//
// Both sides' MSVCRT import slots (the port's platform/msvcrt.h variables and the mapped original's IAT) are
// pointed at recorders around the real msvcrt.dll functions. Each side has a Log: the sequence of heap calls,
// and every live block with a role number (allocation order) and size. Two sides are then compared as graphs:
// blocks with the same role must have the same size and the same bytes, where any 32-bit word that points into
// a known block (or a registered caller region) is compared as (role, offset) instead of as an address. So the
// comparison is deep and layout-agnostic: it needs no knowledge of the structures' fields.
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "platform/image/original_data.h"
#include "platform/msvcrt.h"
#include "watchdog.h"

namespace hg {

struct Ev {
    char kind;       // M malloc, C calloc, R realloc, F free, P fprintf
    std::uint32_t a, b;
    int role;
    std::string text;  // fprintf: the formatted message
    bool operator==(const Ev& o) const { return kind == o.kind && a == o.a && b == o.b && role == o.role && text == o.text; }
};

struct Block {
    int role;
    std::uint32_t size;
};

struct Log {
    std::vector<Ev> ev;
    std::map<std::uintptr_t, Block> live;  // start -> block
    std::map<std::uintptr_t, Block> dead;  // freed (or realloc'd away) blocks: dangling pointers keep their role
    int next = 1000;
    std::uint32_t call_esp = 0;  // ESP at the side's last call_fastcall (0: stack words compare raw)
    bool port = false;           // side P: addresses in the data image / named blocks / ported functions map back to original VAs
    // role of the block containing p (and the offset), or -1 for null / -2 for unknown; live blocks first,
    // then freed ones (a dangling pointer is compared by the role it used to point to)
    int find(const void* p, std::uint32_t* off = nullptr) const
    {
        if (!p) return -1;
        const auto v = reinterpret_cast<std::uintptr_t>(p);
        for (const auto* m : {&live, &dead}) {
            auto it = m->upper_bound(v);
            if (it == m->begin()) continue;
            --it;
            if (v >= it->first + (it->second.size ? it->second.size : 1)) continue;
            if (off) *off = static_cast<std::uint32_t>(v - it->first);
            return it->second.role;
        }
        return -2;
    }
    void add(const void* p, int role, std::uint32_t size) { live[reinterpret_cast<std::uintptr_t>(p)] = {role, size}; }
    void kill(const void* p)
    {
        auto it = live.find(reinterpret_cast<std::uintptr_t>(p));
        if (it == live.end()) return;
        dead[it->first] = it->second;
        live.erase(it);
    }
    // Blocks the code frees are kept allocated until this log ends (quarantine): the two sides share one heap, and
    // if a freed address were reused, a stale pointer on one side could land in a new block while its twin on the
    // other side does not - an order-dependent false difference.
    std::vector<void*> quarantine;
    std::size_t quarantined_bytes = 0;  // capped (kQuarantineCap): a runaway loop must not exhaust the address space
    // Start the log over (a re-run of the same input, tests/arena_fuzz.h): the blocks the recorder allocated so
    // far join the quarantine, so they stay allocated (no address reuse) until the log ends.
    void reset()
    {
        for (auto& [p, blk] : live)
            if (blk.role >= 1000) quarantine.push_back(reinterpret_cast<void*>(p));
        ev.clear();
        live.clear();
        dead.clear();
        next = 1000;
        call_esp = 0;
    }
    Log() = default;
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    ~Log();
};

inline Log*& current()
{
    static Log* l = nullptr;
    return l;
}

using MallocFn = void*(__cdecl*)(std::size_t);
using CallocFn = void*(__cdecl*)(std::size_t, std::size_t);
using ReallocFn = void*(__cdecl*)(void*, std::size_t);
using FreeFn = void(__cdecl*)(void*);
using VfprintfFn = int(__cdecl*)(void*, const char*, va_list);
struct Real {
    MallocFn malloc_;
    CallocFn calloc_;
    ReallocFn realloc_;
    FreeFn free_;
    VfprintfFn vfprintf_;
    void* iob;
};
inline Real& real()
{
    static Real r = {
        reinterpret_cast<MallocFn>(recoil::g_Iat_malloc_004cc5dc), reinterpret_cast<CallocFn>(recoil::g_Iat_calloc_004cc4ac),
        reinterpret_cast<ReallocFn>(recoil::g_Iat_realloc_004cc4ec), reinterpret_cast<FreeFn>(recoil::g_Iat_free_004cc5b4),
        reinterpret_cast<VfprintfFn>(GetProcAddress(LoadLibraryA("msvcrt.dll"), "vfprintf")), recoil::g_Iat__iob_004cc4f8};
    return r;
}

constexpr std::size_t kQuarantineCap = 64u << 20;  // bytes held per log before freed blocks are really released
inline void quarantine_block(Log* L, void* p, std::size_t n)
{
    if (L->quarantined_bytes + n > kQuarantineCap) { real().free_(p); return; }
    L->quarantined_bytes += n;
    L->quarantine.push_back(p);
}

inline Log::~Log()
{
    for (void* p : quarantine) real().free_(p);
}

// New memory from malloc/realloc is uninitialised: fill it with one pattern on both sides, so bytes the code
// never writes (spare capacity, padding) compare equal instead of comparing whatever the allocator left there.
constexpr int kFill = 0xCD;
// Requests above 64 MB fail (NULL) on both sides. Fuzzed sizes can be pointer-sized (~1-2 GB): the original's block
// is still held when the port asks for the same amount, and in a 32-bit address space the second request fails -
// NULL on one side only (seen in Zar_GrowTocBuffer 0x004a62f0). Real inputs never ask for that much.
constexpr std::size_t kMaxAlloc = 64u << 20;

inline void* __cdecl rec_malloc(std::size_t n)
{
    wd::NoEscape harness;  // not abandoned mid-update (tests/watchdog.h)
    void* p = n > kMaxAlloc ? nullptr : real().malloc_(n);
    if (p) std::memset(p, kFill, n);
    Log* L = current();
    L->ev.push_back({'M', static_cast<std::uint32_t>(n), 0, L->next, {}});
    if (p) L->add(p, L->next, static_cast<std::uint32_t>(n));  // a failed allocation (NULL) is not a block
    ++L->next;
    return p;
}
inline void* __cdecl rec_calloc(std::size_t n, std::size_t s)
{
    wd::NoEscape harness;  // not abandoned mid-update (tests/watchdog.h)
    void* p = (s && n > kMaxAlloc / s) ? nullptr : real().calloc_(n, s);
    Log* L = current();
    L->ev.push_back({'C', static_cast<std::uint32_t>(n), static_cast<std::uint32_t>(s), L->next, {}});
    if (p) L->add(p, L->next, static_cast<std::uint32_t>(n * s));
    ++L->next;
    return p;
}
inline void* __cdecl rec_realloc(void* old, std::size_t n)
{
    wd::NoEscape harness;  // not abandoned mid-update (tests/watchdog.h)
    Log* L = current();
    L->ev.push_back({'R', static_cast<std::uint32_t>(n), 0, L->find(old), {}});
    std::uint32_t old_size = 0;
    if (old) {
        auto it = L->live.find(reinterpret_cast<std::uintptr_t>(old));
        if (it != L->live.end()) old_size = it->second.size;
    }
    // realloc as allocate + copy + quarantine the old block (see Log::quarantine); realloc(p, 0) frees and
    // returns NULL, as msvcrt does
    void* p = n && n <= kMaxAlloc ? real().malloc_(n) : nullptr;
    if (p) {
        std::memcpy(p, old ? old : p, old ? (old_size < n ? old_size : n) : 0);
        if (n > old_size) std::memset(static_cast<char*>(p) + old_size, kFill, n - old_size);
    }
    if (old && (p || !n)) {
        auto it = L->live.find(reinterpret_cast<std::uintptr_t>(old));
        if (it != L->live.end() && it->second.role >= 1000) {  // only blocks this recorder allocated
            L->kill(old);
            quarantine_block(L, old, old_size);
        }
    }
    if (p) L->add(p, L->next, static_cast<std::uint32_t>(n));  // a failed allocation (NULL) is not a block
    ++L->next;
    return p;
}
inline void __cdecl rec_free(void* p)
{
    wd::NoEscape harness;  // not abandoned mid-update (tests/watchdog.h)
    Log* L = current();
    L->ev.push_back({'F', 0, 0, L->find(p), {}});
    if (p) {
        auto it = L->live.find(reinterpret_cast<std::uintptr_t>(p));
        if (it != L->live.end() && it->second.role >= 1000) {  // only blocks this recorder allocated
            const std::size_t n = it->second.size;
            L->kill(p);
            quarantine_block(L, p, n);  // freed when the log ends (see Log::quarantine)
        }
        // anything else (a caller's or test's memory) is recorded but never really freed
    }
}
// _strdup allocates inside msvcrt: record it as an allocation of strlen + 1 bytes.
using DupFn = char*(__cdecl*)(const char*);
inline DupFn& real_strdup()
{
    static DupFn f = reinterpret_cast<DupFn>(GetProcAddress(LoadLibraryA("msvcrt.dll"), "_strdup"));
    return f;
}
inline char* __cdecl rec_strdup(const char* s)
{
    wd::NoEscape harness;  // not abandoned mid-update (tests/watchdog.h)
    char* p = real_strdup()(s);
    Log* L = current();
    const auto n = static_cast<std::uint32_t>(s ? std::strlen(s) + 1 : 0);
    L->ev.push_back({'D', n, 0, L->next, {}});
    if (p) L->add(p, L->next, n);
    ++L->next;
    return p;
}
// fprintf: the message is recorded (formatted) instead of written; the stream is recorded as its _iob index.
inline int __cdecl rec_fprintf(void* stream, const char* fmt, ...)
{
    wd::NoEscape harness;  // not abandoned mid-update (tests/watchdog.h)
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    Log* L = current();
    const auto idx = static_cast<std::uint32_t>((reinterpret_cast<char*>(stream) - static_cast<char*>(real().iob)) / 0x20);
    L->ev.push_back({'P', idx, 0, 0, buf});
    return static_cast<int>(std::strlen(buf));
}

constexpr std::uintptr_t kOrigCalloc = 0x004cc4ac, kOrigRealloc = 0x004cc4ec, kOrigFree = 0x004cc5b4, kOrigMalloc = 0x004cc5dc,
                         kOrigFprintf = 0x004cc5bc, kOrigStrdup = 0x004cc5e4;
inline void set_slot(std::uintptr_t va, const void* f)
{
    const auto v = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(f));
    std::memcpy(reinterpret_cast<void*>(va), &v, 4);
}

// Install the recorders on both sides for the lifetime of the object.
struct Recording {
    Recording()
    {
        real();
        recoil::g_Iat_malloc_004cc5dc = reinterpret_cast<void*>(&rec_malloc);
        recoil::g_Iat_calloc_004cc4ac = reinterpret_cast<void*>(&rec_calloc);
        recoil::g_Iat_realloc_004cc4ec = reinterpret_cast<void*>(&rec_realloc);
        recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&rec_free);
        recoil::g_Iat_fprintf_004cc5bc = reinterpret_cast<void*>(&rec_fprintf);
        recoil::g_Iat__strdup_004cc5e4 = reinterpret_cast<void*>(&rec_strdup);
        set_slot(kOrigStrdup, reinterpret_cast<void*>(&rec_strdup));
        set_slot(kOrigMalloc, reinterpret_cast<void*>(&rec_malloc));
        set_slot(kOrigCalloc, reinterpret_cast<void*>(&rec_calloc));
        set_slot(kOrigRealloc, reinterpret_cast<void*>(&rec_realloc));
        set_slot(kOrigFree, reinterpret_cast<void*>(&rec_free));
        set_slot(kOrigFprintf, reinterpret_cast<void*>(&rec_fprintf));
    }
    ~Recording()
    {
        Real& r = real();
        recoil::g_Iat_malloc_004cc5dc = reinterpret_cast<void*>(r.malloc_);
        recoil::g_Iat_calloc_004cc4ac = reinterpret_cast<void*>(r.calloc_);
        recoil::g_Iat_realloc_004cc4ec = reinterpret_cast<void*>(r.realloc_);
        recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(r.free_);
        recoil::g_Iat_fprintf_004cc5bc = GetProcAddress(LoadLibraryA("msvcrt.dll"), "fprintf");
        set_slot(kOrigMalloc, reinterpret_cast<void*>(r.malloc_));
        set_slot(kOrigCalloc, reinterpret_cast<void*>(r.calloc_));
        set_slot(kOrigRealloc, reinterpret_cast<void*>(r.realloc_));
        set_slot(kOrigFree, reinterpret_cast<void*>(r.free_));
        set_slot(kOrigFprintf, recoil::g_Iat_fprintf_004cc5bc);
        recoil::g_Iat__strdup_004cc5e4 = reinterpret_cast<void*>(real_strdup());
        set_slot(kOrigStrdup, reinterpret_cast<void*>(real_strdup()));
    }
};

// Calls through call_fastcall record ESP just before the target runs, so a pointer the code keeps into its own
// (or its caller's) stack frame can be compared as an offset from that ESP: the two sides reach the call through
// different template instantiations whose frames differ, but below this point the frames are identical.
template <class... A>
__declspec(noinline) int call_fastcall(int(__fastcall* f)(A...), A... a)
{
    std::uint32_t stack_now;
    __asm mov stack_now, esp
    current()->call_esp = stack_now;
    return f(a...);
}
inline bool on_stack(std::uint32_t w)
{
    const NT_TIB* tib = reinterpret_cast<const NT_TIB*>(NtCurrentTeb());
    return w >= reinterpret_cast<std::uintptr_t>(tib->StackLimit) && w < reinterpret_cast<std::uintptr_t>(tib->StackBase);
}

// Normalised view of one 32-bit word: a pointer into a known block becomes (role, offset); a pointer into the
// stack becomes its offset from the ESP recorded at the side's last call_fastcall.
inline std::uint64_t norm(const Log& L, std::uint32_t w)
{
    std::uint32_t off = 0;
    const int r = L.find(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(w)), &off);
    if (r >= 0) return (1ull << 63) | (static_cast<std::uint64_t>(r) << 32) | off;
    if (L.call_esp && on_stack(w)) return (3ull << 62) | static_cast<std::uint32_t>(w - L.call_esp);
    // image addresses: the original side holds original VAs, the port side the port's copies of them
    if (!L.port && w >= 0x00401000u && w < 0x007c9000u) return (5ull << 60) | w;
    if (L.port) {
        const std::uint32_t va = recoil::ImageData_VaOf(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(w)));
        if (va) return (5ull << 60) | va;
    }
    return w;
}
// Deep comparison of every live block (and every word of it) on the two sides. Returns "" when equal.
inline std::string compare(const Log& O, const Log& P)
{
    if (!(O.ev == P.ev)) return "heap call sequence differs";
    std::map<int, std::pair<std::uintptr_t, std::uint32_t>> ob, pb;
    for (auto& [p, blk] : O.live) ob[blk.role] = {p, blk.size};
    for (auto& [p, blk] : P.live) pb[blk.role] = {p, blk.size};
    if (ob.size() != pb.size()) return "live block sets differ";
    for (auto& [role, o] : ob) {
        auto it = pb.find(role);
        if (it == pb.end() || it->second.second != o.second) return "block " + std::to_string(role) + " missing or resized";
        const auto* a = reinterpret_cast<const unsigned char*>(o.first);
        const auto* b = reinterpret_cast<const unsigned char*>(it->second.first);
        const std::uint32_t n = o.second;
        for (std::uint32_t i = 0; i + 4 <= n; i += 4) {
            std::uint32_t wa, wb;
            std::memcpy(&wa, a + i, 4);
            std::memcpy(&wb, b + i, 4);
            // bit-identical words are equal (a port can never hold the original's own addresses: the converter
            // refuses them), else compare what they point to
            if (wa != wb && norm(O, wa) != norm(P, wb)) {
                char msg[160];
                std::snprintf(msg, sizeof msg, "block %d (size %u) differs at +%u: original %08x port %08x (roles %d / %d)",
                              role, n, i, wa, wb, O.find(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(wa))),
                              P.find(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(wb))));
                return msg;
            }
        }
        if (std::memcmp(a + (n & ~3u), b + (n & ~3u), n & 3u)) return "block " + std::to_string(role) + " tail differs";
    }
    return "";
}

}  // namespace hg
