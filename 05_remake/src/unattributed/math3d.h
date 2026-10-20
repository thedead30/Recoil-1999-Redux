// SUBSYSTEM: math3d
// Declarations for src/unattributed/math3d.cpp. Spec: 04_spec/systems/math3d.md
#pragma once

#include "Battlesport/ai_net.h"  // Vec3_NormalizeInPlace (0x00402f60), a callee of math3d functions

namespace recoil {

// 0x00566420: 3-float scratch vector written by the distance helpers (a - b, stored as floats).
extern float g_Vec3Scratch_00566420[3];

// 0x00472670 Vec3_DistanceSquared (ECX a, EDX b) -> |a-b|^2 as float; writes the scratch vector
float __fastcall Vec3_DistanceSquared(const float* a, const float* b);
// 0x004726d0 Vec3_Distance (ECX a, EDX b) -> |a-b| as float; writes the scratch vector
float __fastcall Vec3_Distance(const float* a, const float* b);
// 0x00472730 Vec3_DistSqXZ (ECX a, EDX b) -> dx^2+dz^2 as float; writes scratch x and z only
float __fastcall Vec3_DistSqXZ(const float* a, const float* b);
// 0x00472770 Vec3_AddScaled (ECX a, EDX b, stack s, stack out; ret 8): out = a + b*s
void __fastcall Vec3_AddScaled(const float* a, const float* b, float s, float* out);

// 0x00446ed0 AABB_ToCorners (ECX box = min xyz, max xyz; EDX out 8 corners x 3 floats), dword copies
void __fastcall AABB_ToCorners(const float* box, float* out);
// 0x004727a0 Vec3_ScaleByReciprocal (ECX src, EDX dst, stack s; ret 4): dst = src / s, copy when s == 0
void __fastcall Vec3_ScaleByReciprocal(const float* src, float* dst, float s);
// 0x004727f0 Math_NormalizeHorizontalVector (ECX v, EDX out): out.xz = v.xz / |v.xz|; out.y untouched
void __fastcall Math_NormalizeHorizontalVector(float* v, float* out);
// 0x004745c0 Vec3_PerpXZ (ECX in, EDX out): out = (-in.z, 0, in.x)
void __fastcall Vec3_PerpXZ(const float* in, float* out);
// 0x004744f0 Vec3Array_AddScaled (ECX dst, EDX src, stack scaled, count, s; ret 0xc)
void __fastcall Vec3Array_AddScaled(float* dst, const float* src, const float* scaled, unsigned count, float s);

// 0x00474ec0 Vec3_RotateX (ECX out, EDX in, stack angle; ret 4): out = (x, c*y - s*z, s*y + c*z)
void __fastcall Vec3_RotateX(float* out, const float* in, float angle);
// 0x00474f40 Vec3_RotateY (ECX out, EDX in, stack angle; ret 4): out = (s*z + c*x, y, c*z - s*x)
void __fastcall Vec3_RotateY(float* out, const float* in, float angle);
// 0x00475910 Quat_Multiply (ECX a, EDX b, stack out; ret 4), quaternions (w, x, y, z)
void __fastcall Quat_Multiply(const float* a, const float* b, float* out);
// 0x00475a80 Quat_ToMatrix3 (ECX q = (w, x, y, z), EDX out 9 floats)
void __fastcall Quat_ToMatrix3(const float* q, float* m);
// 0x00475130 Math_Solve2x2 (ECX out0, EDX out1, 9 stack floats; ret 0x24): Cramer's rule, 0/0 when det == 0
void __fastcall Math_Solve2x2(float* out0, float* out1, float p0, float p1, float p2, float p3, float p4,
                              float p5, float p6, float p7, float p8);

// 0x004dc998: default direction used by Vec3_SetYKeepXZDir when x = z = 0 and y = 0.
extern float g_DefaultDir_004dc998[3];

// 0x0043b500 Vec3_SetYKeepXZDir (thiscall ECX v, stack y; ret 4)
void __fastcall Vec3_SetYKeepXZDir(float* v, int /*edx unused*/, float y);
// 0x004525d0 Math_BoxToSphere (ECX box min/max, EDX centre out, stack radius out; ret 4)
void __fastcall Math_BoxToSphere(const float* box, float* centre, float* radius);
// 0x00472860 Vec3_Reflect (ECX n, EDX d, stack out; ret 4)
void __fastcall Vec3_Reflect(const float* n, const float* d, float* out);
// 0x00472cc0 Vec2_PerpNormalized (ECX in, EDX out)
void __fastcall Vec2_PerpNormalized(const float* in, float* out);
// 0x00474580 Vec3_FromHeading (ECX out, stack angle; ret 4): RotateY((0, 0, -1), angle)
void __fastcall Vec3_FromHeading(float* out, int /*edx unused*/, float angle);

// ---- instruction-level x87 ports (src/unattributed/math3d_x87.cpp)
// 0x00474d10 Math_ComputeLookAtPitchYaw (ECX a, EDX b, stack out; ret 4): out = (pitch, yaw, 0)
void __fastcall Math_ComputeLookAtPitchYaw(const float* a, const float* b, float* out);
// 0x00474d90 Math_LookAtPitch (ECX a, EDX b) -> ST0 (unrounded fpatan result)
long double __fastcall Math_LookAtPitch(const float* a, const float* b);
// 0x00474de0 Mat3_YawFromRows (ECX m) -> ST0: atan2(m[6], m[8]) or 0
long double __fastcall Mat3_YawFromRows(const float* m);
// 0x00474260 Mat3_FromEuler (ECX out 12 floats, stack a, b, g; ret 0xc)
void __fastcall Mat3_FromEuler(float* out, int /*edx unused*/, float a, float b, float g);
// 0x00474870 Box8_TransformByMatrix (ECX m, EDX box, stack out 24 floats; ret 4)
void __fastcall Box8_TransformByMatrix(const float* m, const float* box, float* out);

// 0x004729b0 Vec3_SubNormalize (ECX a, EDX b, stack out; ret 4): out = b - a, then Vec3_NormalizeInPlace(out).
float __fastcall Vec3_SubNormalize(const float* a, const float* b, float* out);  // returns the length
// 0x004729f0 Vec3_LerpNormalize (ECX v, EDX other, stack t; ret 4): Vec3_Lerp(v, other, t); Vec3_NormalizeInPlace(v).
void __fastcall Vec3_LerpNormalize(float*, const float*, float);
// 0x00475070 Tri_Normal (ECX a, EDX b, stack c, stack out; ret 8): out = normalise((b - a) x (c - a)).
void __fastcall Tri_Normal(const float*, const float*, const float*, float*);
// 0x00475b80 Quat_FromRotationVector (ECX v, EDX out): a = |v|; a == 0 -> (1, 0, 0, 0); else (cos a, v * sin(a) / a).
void __fastcall Quat_FromRotationVector(const float*, float*);
// 0x00475210 Math_RaySphereNormal (ECX, EDX, stack r, dir, out; ret 0xc) -> EAX 0/1: stable quadratic with the fast-sqrt
int __fastcall Math_RaySphereNormal(const float*, const float*, float, const float*, float*);
// 0x00474e10 Mat3_ToEuler (ECX matrix, EDX out pitch/yaw/roll): yaw = Mat3_YawFromRows; pitch = atan2(-m7, hypot);
void __fastcall Mat3_ToEuler(const float*, float*);
// 0x00472a10 Vec3_Slerp (ECX from, EDX to, stack t, stack out; ret 8): t == 0 / t == 1 copies; dot < -0.95 builds a
void __fastcall Vec3_Slerp(const float*, const float*, float, float*);

// 0x00472960 Vec3_Lerp (ECX = a in/out, EDX = b, stack t; ret 4)
void __fastcall Vec3_Lerp(float* a, const float* b, float t);

}  // namespace recoil
