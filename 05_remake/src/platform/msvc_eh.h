// SUBSYSTEM: platform
// The MSVC 5 C++ exception-frame tables (FuncInfo magic 0x19930520) that __CxxFrameHandler reads. PLATFORM: compiler / CRT ABI,
// not Recoil data - the layouts are the ones the original's tables have (decoded from Recoil.exe .rdata by tools/gen_eh_frames.py:
// FuncInfo 5 dwords, unwind map entries 2 dwords, try block map entries 5 dwords, handler types 4 dwords). The generated tables
// in the ported files point at the port's funclets and catch blocks instead of the original's.
#pragma once

namespace recoil {

struct EhUnwindMapEntry { int to_state; void* action; };
struct EhHandlerType { unsigned adjectives; const void* type_descriptor; int disp_catch_obj; void* handler; };
struct EhTryBlockMapEntry { int try_low; int try_high; int catch_high; int n_catches; EhHandlerType* handlers; };
struct EhFuncInfo { unsigned magic; int max_state; const EhUnwindMapEntry* unwind_map; unsigned n_try; const EhTryBlockMapEntry* try_map; };
static_assert(sizeof(EhUnwindMapEntry) == 8 && sizeof(EhHandlerType) == 16 && sizeof(EhTryBlockMapEntry) == 20 && sizeof(EhFuncInfo) == 20,
              "MSVC 5 EH table layout (PLATFORM)");

}  // namespace recoil
