// SUBSYSTEM: geometry
// Declarations for src/unattributed/geometry.cpp (geometry functions with no attributed original file).
// Spec: 04_spec/systems/geometry.md, 04_spec/systems/poly_shade.md (back-face cull)
#pragma once

namespace recoil {

// 0x004e0fc0 back-face cull epsilon (initial 0.005, CONFIRMED-DATA Recoil.exe .data)
extern float g_BFETolerance_004e0fc0;

// 0x00476460 Geom_SetBFETolerance (stack value; ret 4): back-face cull epsilon := value.
void __stdcall Geom_SetBFETolerance(float);
// 0x00476470 Geom_GetBFETolerance -> ST0: the back-face cull epsilon.
long double Geom_GetBFETolerance();
// 0x0046c720 Alloc3Ptr_Free (ECX block or null): frees [+0x4] and [+0xc] if set, then the block.
void __fastcall Alloc3Ptr_Free(void*);

// 0x0046a690
int __fastcall Material_CreateRandomColour(int ecx, int edx);
}  // namespace recoil
