// SUBSYSTEM: transform
// Declarations for src/unattributed/transform.cpp (the transform stack; no attributed original file).
// Spec: 04_spec/systems/transform_stack.md, globals 04_spec/globals/transform.md
#pragma once

#include <cstdint>

namespace recoil {

// ---- state (original globals; all zero at load except the two stack pointers)
// 0x00566868 matrix-pointer stack, 0x00566950 identity-flag stack: one 4-byte slot per level.
constexpr int kTransformStackSlots = 32;  // INFERRED: 0x00566868 + 32*4 = 0x005668e8, where matrix A starts;
                                          // the original has no bounds check (alternative: the flag stack
                                          // alone could hold 34 slots up to 0x005669d8 - not needed, depth is shared)
extern float* g_TransformMatrixStack_00566868[kTransformStackSlots];
extern std::uint32_t g_TransformFlagStack_00566950[kTransformStackSlots];
extern float** g_TransformMatrixSP_004e0e88;         // initial value = base (CONFIRMED-DATA Recoil.exe .data 0x004e0e88)
extern std::uint32_t* g_TransformFlagSP_004e0e84;   // initial value = base (CONFIRMED-DATA Recoil.exe .data 0x004e0e84)
extern float g_ViewMatrixA_005668e8[12];             // view matrix = inverse of B (written by 0x00473e60)
extern float g_CameraWorldB_00566920[12];            // camera world matrix, axis-converted (written by 0x00473e60)

// Reset the stack pointers to their load-time values (the original's .data initialisers).
void Transform_ResetState();

// ---- primitives
// 0x00472ef0 push; current := ECX; flag inherited; parent's 12 floats copied into ECX
void __fastcall zTransformStackPushAndCopy(float* m);
// 0x00472f30 push; current := ECX; flag := 0
void __fastcall zTransformStackPushRef(float* m);
// 0x00472f60 pop (writes nothing)
void zTransformStackPop();
// 0x00473250 current matrix := 12 floats at ECX; flag := 0
void __fastcall zTransformLoad(const float* src);
// 0x00472f90 zTransformLoad(A = view)
void zTransformLoadView();
// 0x00472fa0 zTransformLoad(B = camera world)
void zTransformLoadCameraWorld();
// 0x00473210 copy the current matrix to the stack argument (ret 4)
float* __stdcall zTransformStore(float* out);   // returns out in EAX (callers copy from it, KG-51)
// 0x00473230 current matrix pointer
float* zTransformGetCurrent();
// 0x00473240 current identity flag
std::uint32_t zTransformIsIdentity();
// 0x00473280 current rotation (floats 0..8) := 9 floats at ECX; flag := 0
void __fastcall zTransformSetRotation(const float* src);
// 0x004732f0 current := identity; flag := 1
void zTransformLoadIdentity();
// 0x004731f0 current := A (view), then concatenate the parent level's matrix into it, mode 1
void zTransformLoadViewConcatParent();

// ---- instruction-level x87 ports (src/unattributed/transform_x87.cpp)
// 0x00473370 (ECX src, EDX mode). Mode 2 keeps the current translation: deliberate deviation KG-25.
void __fastcall zTransformConcatenateLocal(const float* src, int mode);
// 0x00473690 (stack sx, sy, sz; ret 0xc)
void __stdcall zTransformScaleLocal(float sx, float sy, float sz);
// 0x004737e0 (stack x, y, z; ret 0xc)
void __stdcall Transform_TranslateLocal(float x, float y, float z);
// 0x00473970 / 0x00473b10 / 0x00473cc0 (stack angle in radians; ret 4)
void __stdcall Transform_RotateXLocal(float angle);
void __stdcall Transform_RotateYLocal(float angle);
void __stdcall Transform_RotateZLocal(float angle);
// 0x004745e0 v := v . R in place
void __fastcall zTransformRotateVectors(float* v, int count);
// 0x00474670 v := R . v in place
void __fastcall zTransformRotateVectorsTransposed(float* v, int count);
// 0x00474710 dst := src . R (stack count; ret 4)
void __fastcall zTransformRotateVectorsTo(const float* src, float* dst, int count);
// 0x004747d0 p := p . R + t in place
void __fastcall zTransformPoints(float* p, int count);
// 0x00474010 local translate-rotate-scale concatenated, mode 1 (stack scale; ret 4)
void __fastcall zTransformTRSLocal(const float* angles, const float* translation, const float* scale);
// 0x004757c0 Euler angles -> quaternion (w, x, y, z) (ECX out, EDX unused, stack angles; ret 0xc)
void __fastcall Quat_FromEuler(float* out, int /*edx unused*/, float a, float b, float c);
// 0x004759d0 quaternion product (stack out; ret 4)
void __fastcall Quat_Product(const float* a, const float* b, float* out);
// 0x00474bc0 screen points -> view space, using the view state (stack count; ret 4)
void __fastcall zTransformScreenToView(const float* screen, float* out, int count);
// 0x00474c20 screen points -> world space through camera matrix B (stack count; ret 4). Callers pass 1.
void __fastcall zTransformScreenToWorld(const float* screen, float* out, int count);
// 0x00472fb0 yaw billboard (stack yaw; ret 4)
void __stdcall zTransformBillboardYaw(float yaw);
// 0x00473060 full-orientation billboard
void zTransformBillboardFull();

}  // namespace recoil
