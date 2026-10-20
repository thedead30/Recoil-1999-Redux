// Shared helpers for the structured native tests written in cloud sessions (03_re/CLOUD.md): addresses as 32-bit
// words, the two sides' copies of an image address, the system msvcrt (the CRT both sides' import slots reach), a
// logging free hooked into both import slots, list snapshots by role.
#pragma once

#include "native_oracle.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <malloc.h>

#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace ch {

inline std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
inline std::uint32_t* at(std::uint32_t p) { return reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(p)); }
// the word at image address va on a side: 0 the mapped original, 1 the port's data image
inline std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
inline std::uint32_t fbits(float f) { std::uint32_t u; std::memcpy(&u, &f, 4); return u; }

inline HMODULE crt() { return GetModuleHandleA("msvcrt.dll"); }
template <class F> F crt_fn(const char* name) { return reinterpret_cast<F>(GetProcAddress(crt(), name)); }
inline void* c_malloc(std::size_t n) { return crt_fn<void*(__cdecl*)(std::size_t)>("malloc")(n); }
inline void c_free(void* p) { crt_fn<void(__cdecl*)(void*)>("free")(p); }
inline void c_free(std::uint32_t p) { c_free(at(p)); }
inline char* c_strdup(const char* s) { return crt_fn<char*(__cdecl*)(const char*)>("_strdup")(s); }
inline void* c_fopen(const char* p, const char* m) { return crt_fn<void*(__cdecl*)(const char*, const char*)>("fopen")(p, m); }
inline int c_fclose(void* f) { return crt_fn<int(__cdecl*)(void*)>("fclose")(f); }
inline long c_ftell(void* f) { return crt_fn<long(__cdecl*)(void*)>("ftell")(f); }
inline std::size_t c_fwrite(const void* b, std::size_t s, std::size_t n, void* f)
{
    return crt_fn<std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*)>("fwrite")(b, s, n, f);
}
inline std::string temp_path(const char* prefix)
{
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, prefix, 0, path);
    return path;
}
inline void write_file(const std::string& path, const std::string& bytes)
{
    void* f = c_fopen(path.c_str(), "wb");
    if (!bytes.empty()) c_fwrite(bytes.data(), 1, bytes.size(), f);
    c_fclose(f);
}

// Fills the stack below the caller with one pattern, so a function that writes out a local it never fully initialised
// (a record built on its stack) sees the same bytes on both sides (as tests/arena_fuzz.h does for the fuzz).
__declspec(noinline) inline void wipe_stack(std::uint32_t bytes = 0x8000)
{
    volatile unsigned char* p = static_cast<volatile unsigned char*>(_alloca(bytes));
    for (std::uint32_t k = 0; k < bytes; ++k) p[k] = 0xCD;
}

// free logged in both import slots while alive
inline std::vector<std::uint32_t>& freed()
{
    static std::vector<std::uint32_t> v;
    return v;
}
inline void __cdecl logging_free(void* p) { freed().push_back(addr(p)); if (p) c_free(p); }
struct FreeHook {
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* saved[2];
    FreeHook()
    {
        saved[0] = *o; saved[1] = recoil::g_Iat_free_004cc5b4;
        *o = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free);
    }
    ~FreeHook() { *o = saved[0]; recoil::g_Iat_free_004cc5b4 = saved[1]; }
};
inline bool was_freed(std::uint32_t p)
{
    for (std::uint32_t f : freed()) if (f == p) return true;
    return false;
}

// pointers compared by role: known addresses get their given role, others a fresh number in order of appearance
struct Roles {
    std::map<std::uint32_t, std::uint32_t> m{{0, 0}};
    std::uint32_t fresh = 0xC000;
    void set(std::uint32_t p, std::uint32_t r) { m[p] = r; }
    void set(const void* p, std::uint32_t r) { m[addr(p)] = r; }
    std::uint32_t operator()(std::uint32_t v)
    {
        auto q = m.find(v);
        return q != m.end() ? q->second : (m[v] = fresh++);
    }
};

// list l (0..15): links {node, prev, next, mark} from the head holder [0x004ddef8 + 4l], plus the dirty word, the
// free-link head and the link counter
inline void snap_list(int side, int l, std::vector<std::uint32_t>& s, Roles& rl)
{
    std::uint32_t p = *at(*img(side, 0x004ddef8u + 4 * l));
    s.push_back(rl(p));
    for (int guard = 0; p && guard < 64; ++guard, p = at(p)[2]) { s.push_back(rl(at(p)[0])); s.push_back(at(p)[3]); }
    s.push_back(*img(side, 0x00539bb4u + 12 * l));
}
inline void snap_link_state(int side, std::vector<std::uint32_t>& s, Roles& rl)
{
    s.push_back(rl(*img(side, 0x00539c6c)));
    s.push_back(*img(side, 0x00539c74));
}

}  // namespace ch
