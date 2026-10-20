// SUBSYSTEM: platform
// Binds the MSVCRT.dll imports of Recoil.exe to the system msvcrt.dll (see msvcrt.h). PLATFORM.
#include "platform/msvcrt.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdexcept>

namespace recoil::crt {
namespace {

HMODULE dll()
{
    static HMODULE h = LoadLibraryA("msvcrt.dll");
    if (!h) throw std::runtime_error("msvcrt.dll not found");
    return h;
}

template <class F>
F sym(const char* name)
{
    static_assert(sizeof(F) == sizeof(FARPROC));
    FARPROC p = GetProcAddress(dll(), name);
    if (!p) throw std::runtime_error(std::string("msvcrt.dll has no ") + name);
    return reinterpret_cast<F>(p);
}

using RandFn = int(__cdecl*)();
using SrandFn = void(__cdecl*)(unsigned);
using AtofFn = double(__cdecl*)(const char*);
using AtoiFn = int(__cdecl*)(const char*);

}  // namespace

int rand()
{
    static RandFn f = sym<RandFn>("rand");
    return f();
}

void srand(unsigned seed)
{
    static SrandFn f = sym<SrandFn>("srand");
    f(seed);
}

double atof(const char* s)
{
    static AtofFn f = sym<AtofFn>("atof");
    return f(s);
}

int atoi(const char* s)
{
    static AtoiFn f = sym<AtoiFn>("atoi");
    return f(s);
}

std::int64_t ftol(double v)
{
    // _ftol takes its argument in ST0 and returns EDX:EAX; it has no C prototype.
    static void* f = reinterpret_cast<void*>(GetProcAddress(dll(), "_ftol"));
    if (!f) throw std::runtime_error("msvcrt.dll has no _ftol");
    std::uint32_t lo = 0, hi = 0;
    __asm {
        fld v
        call f
        mov lo, eax
        mov hi, edx
    }
    return static_cast<std::int64_t>((static_cast<std::uint64_t>(hi) << 32) | lo);
}

}  // namespace recoil::crt

