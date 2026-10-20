// SUBSYSTEM: platform
// The CRT functions Recoil.exe imports from MSVCRT.dll (import table 0x004cc000..): rand 0x004cc5d8,
// srand 0x004cc488, _ftol 0x004cc5ac, and the others added as translations need them.
// Decision D5 / 04_spec/formulas/crt_rand.md: the reference captures ran against the Windows 11
// system msvcrt.dll, so the remake calls the SAME DLL functions (resolved with GetProcAddress)
// instead of re-implementing them. This keeps rand's per-thread state, _ftol's rounding and
// atof's conversion identical by construction. PLATFORM.
#pragma once

#include <cstdint>

namespace recoil::crt {

// MSVCRT.dll rand / srand: per-thread LCG, x = x*214013 + 2531011, result (x >> 16) & 0x7FFF.
int rand();
void srand(unsigned seed);

// MSVCRT.dll _ftol: x87 value truncated toward zero, 64-bit result in EDX:EAX. Recoil uses EAX.
std::int64_t ftol(double v);

// MSVCRT.dll atof / atoi, as Script_ArgFloat 0x004c1a00 and Script_ArgInt 0x004c1a20 call them.
double atof(const char* s);
int atoi(const char* s);

}  // namespace recoil::crt

// Import slots for every MSVCRT.dll import (g_Iat_<name>_<slot>), generated from the import table.
#include "platform/iat_msvcrt.h"
