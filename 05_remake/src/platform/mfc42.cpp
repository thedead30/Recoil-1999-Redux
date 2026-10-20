// SUBSYSTEM: platform
// MFC42 operator new / delete for ported code (see mfc42.h). PLATFORM.
#include "platform/mfc42.h"

#include "platform/iat_msvcrt.h"

namespace recoil {

// Through the slots, not direct calls: tests redirect the msvcrt slots to the heap recorder (tests/heap_graph.h).
void* __cdecl Platform_OperatorNew(std::size_t n)
{
    return reinterpret_cast<void*(__cdecl*)(std::size_t)>(g_Iat_malloc_004cc5dc)(n);
}

void __cdecl Platform_OperatorDelete(void* p)
{
    reinterpret_cast<void(__cdecl*)(void*)>(g_Iat_free_004cc5b4)(p);
}

void* g_Iat_MFC42_823_004cc2ac = reinterpret_cast<void*>(&Platform_OperatorNew);
void* g_Iat_MFC42_825_004cc2b8 = reinterpret_cast<void*>(&Platform_OperatorDelete);

}  // namespace recoil
