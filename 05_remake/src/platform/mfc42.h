// SUBSYSTEM: platform
// The MFC42.DLL imports of Recoil.exe that ported code reaches (import table 0x004cc174.., by ordinal). The original
// links MFC's operator new (#823, slot 0x004cc2ac) and operator delete (#825, slot 0x004cc2b8), reached through the
// thunks operator_new 0x004c5b76 / operator_delete 0x004c5b6a. MFC42 sits on msvcrt's heap; the remake binds both
// slots to Platform_OperatorNew / Platform_OperatorDelete, which allocate through the msvcrt import slots
// (platform/iat_msvcrt.h), so ported code, the original under test and the heap recorder share one heap. PLATFORM.
// Difference: MFC's operator new retries through its new handler and throws CMemoryException when malloc fails;
// Platform_OperatorNew returns NULL (the game never recovers from either).
#pragma once

#include <cstddef>

namespace recoil {

void* __cdecl Platform_OperatorNew(std::size_t n);
void __cdecl Platform_OperatorDelete(void* p);

// Import-address-table slots for instruction-level ports, named after the original slot address.
extern void* g_Iat_MFC42_823_004cc2ac;  // operator new
extern void* g_Iat_MFC42_825_004cc2b8;  // operator delete

}  // namespace recoil
