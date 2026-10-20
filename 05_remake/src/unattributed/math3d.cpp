// SUBSYSTEM: math3d
// Functions of subsystem `math3d` with no attributed original source file (ledger orig_file empty).
// Their placement here is a layout choice, not a provenance claim (STAGE2.md section 2).
// Spec: 04_spec/systems/math3d.md
//
// x87 rule (decision D1): intermediate values that the original keeps on the x87 stack are `double`
// here (the FPU runs at 53-bit precision, Crt_SetFpuControl 0x004c6350); values the original stores
// with FSTP float are `float`. The operation order follows the listing.
#include "unattributed/math3d.h"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace recoil {

// 0x00566420 math3d scratch vector (global state of the original; 04_spec/globals/math3d.md).
float g_Vec3Scratch_00566420[3];

namespace {
// Shared body of 0x00472670 / 0x004726d0 (listings identical up to the FSQRT):
// d = a - b computed for x, y, z, then stored to the scratch vector as floats; the squares are taken
// of the STORED floats and summed as (x*x + y*y) + z*z on the x87 stack.
double scratch_sum_of_squares(const float* a, const float* b)
{
    const double dx = static_cast<double>(a[0]) - b[0];
    const double dy = static_cast<double>(a[1]) - b[1];
    const double dz = static_cast<double>(a[2]) - b[2];
    float* s = g_Vec3Scratch_00566420;
    s[0] = static_cast<float>(dx);
    s[1] = static_cast<float>(dy);
    s[2] = static_cast<float>(dz);
    const double p0 = static_cast<double>(s[0]) * s[0];
    const double p1 = static_cast<double>(s[1]) * s[1];
    const double p2 = static_cast<double>(s[2]) * s[2];
    return (p0 + p1) + p2;
}
}  // namespace

// 0x00472670 Vec3_DistanceSquared - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00472670..0x004726c7):
// the result is stored to a float local and reloaded, so the returned value is float-rounded.
float __fastcall Vec3_DistanceSquared(const float* a, const float* b)
{
    return static_cast<float>(scratch_sum_of_squares(a, b));
}

// 0x004726d0 Vec3_Distance - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x004726d0..0x00472729):
// FSQRT of the unrounded sum, then stored to a float local and reloaded.
float __fastcall Vec3_Distance(const float* a, const float* b)
{
    return static_cast<float>(std::sqrt(scratch_sum_of_squares(a, b)));
}

// 0x00472730 Vec3_DistSqXZ - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00472730..0x00472764):
// dx stored to scratch[0], then dz to scratch[2] (scratch[1] untouched); squares of the stored floats.
float __fastcall Vec3_DistSqXZ(const float* a, const float* b)
{
    float* s = g_Vec3Scratch_00566420;
    s[0] = static_cast<float>(static_cast<double>(a[0]) - b[0]);
    s[2] = static_cast<float>(static_cast<double>(a[2]) - b[2]);
    const double p0 = static_cast<double>(s[0]) * s[0];
    const double p2 = static_cast<double>(s[2]) * s[2];
    return static_cast<float>(p0 + p2);
}

// 0x00472770 Vec3_AddScaled - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00472770..0x00472798):
// per component: out[i] = (float)(s * b[i] + a[i]); out[i] is stored before b[i+1]/a[i+1] are read.
void __fastcall Vec3_AddScaled(const float* a, const float* b, float s, float* out)
{
    out[0] = static_cast<float>(static_cast<double>(s) * b[0] + a[0]);
    out[1] = static_cast<float>(static_cast<double>(b[1]) * s + a[1]);
    out[2] = static_cast<float>(static_cast<double>(b[2]) * s + a[2]);
}

namespace {
// x87 FCOMP + FNSTSW + TEST AH,0x40: taken as "equal" when C3 is set, which is equal OR unordered (NaN).
bool x87_c3_equal(double a, double b)
{
    return !(a < b || a > b);
}
std::uint32_t bits(float f)
{
    std::uint32_t d;
    std::memcpy(&d, &f, 4);
    return d;
}
float from_bits(std::uint32_t d)
{
    float f;
    std::memcpy(&f, &d, 4);
    return f;
}
}  // namespace

// 0x00446ed0 AABB_ToCorners - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00446ed0..0x00446f5b):
// 24 dword copies (no FPU, so NaN payloads pass through). box = [minx miny minz maxx maxy maxz].
void __fastcall AABB_ToCorners(const float* box, float* out)
{
    static const int k[24] = {0, 1, 5, 3, 1, 5, 3, 1, 2, 0, 1, 2, 0, 4, 5, 3, 4, 5, 3, 4, 2, 0, 4, 2};  // CONFIRMED-BINARY 0x00446ed0 (source dword index per output dword)
    auto* o = reinterpret_cast<std::uint32_t*>(out);
    const auto* b = reinterpret_cast<const std::uint32_t*>(box);
    for (int i = 0; i < 24; ++i) o[i] = b[k[i]];
}

// 0x004727a0 Vec3_ScaleByReciprocal - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x004727a0..0x004727ea):
// s compared with float 0.0 at 0x004d2918; "equal" (also NaN) -> dword copy unless dst == src.
// Otherwise r = 1.0 / s stays on the x87 stack (not rounded); dst[i] = (float)(r * src[i]).
void __fastcall Vec3_ScaleByReciprocal(const float* src, float* dst, float s)
{
    const double zero = 0.0f;  // CONFIRMED-BINARY: float 0x00000000 at 0x004d2918
    if (x87_c3_equal(s, zero)) {
        if (dst != src)
            for (int i = 0; i < 3; ++i) reinterpret_cast<std::uint32_t*>(dst)[i] = reinterpret_cast<const std::uint32_t*>(src)[i];
        return;
    }
    const double one = 1.0f;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d291c
    const double r = one / s;
    dst[0] = static_cast<float>(r * src[0]);
    dst[1] = static_cast<float>(static_cast<double>(src[1]) * r);
    dst[2] = static_cast<float>(static_cast<double>(src[2]) * r);
}

// 0x004727f0 Math_NormalizeHorizontalVector - CONFIRMED-BINARY (listing re-read 2026-09-25,
// 0x004727f0..0x0047285e): v.y is saved and set to 0 in memory; len = (float)sqrt((x*x + y*y) + z*z);
// v.y restored. len compared with double 0.0 at 0x004d2920: equal (or NaN) -> scale = len, else
// scale = 1.0 / len (unrounded). out.x = (float)(scale * v.x), out.z = (float)(v.z * scale); out.y untouched.
void __fastcall Math_NormalizeHorizontalVector(float* v, float* out)
{
    const std::uint32_t saved_y = bits(v[1]);
    v[1] = 0.0f;
    const double sum = (static_cast<double>(v[0]) * v[0] + static_cast<double>(v[1]) * v[1]) + static_cast<double>(v[2]) * v[2];
    const float len = static_cast<float>(std::sqrt(sum));
    v[1] = from_bits(saved_y);
    const double zero = 0.0;  // CONFIRMED-BINARY: double 0x0000000000000000 at 0x004d2920
    double scale = len;
    if (!x87_c3_equal(len, zero)) {
        const double one = 1.0f;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d291c
        scale = one / len;
    }
    out[0] = static_cast<float>(scale * v[0]);
    out[2] = static_cast<float>(static_cast<double>(v[2]) * scale);
}

// 0x004745c0 Vec3_PerpXZ - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x004745c0..0x004745d3):
// out.x = FCHS(in.z) stored as float; out.y = dword 0; out.z = dword copy of in.x (read after out.x is written).
void __fastcall Vec3_PerpXZ(const float* in, float* out)
{
    out[0] = static_cast<float>(-static_cast<double>(in[2]));
    const std::uint32_t x = reinterpret_cast<const std::uint32_t*>(in)[0];
    reinterpret_cast<std::uint32_t*>(out)[1] = 0;
    reinterpret_cast<std::uint32_t*>(out)[2] = x;
}

// 0x004744f0 Vec3Array_AddScaled - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x004744f0..0x00474574):
// for i < count (count 0 -> nothing; decremented to 0, so an unsigned loop): t = (float)(scaled[i] * s) per
// component into float locals; then dst[i] = (float)(src[i] + t) for x, y, z computed first, stored after.
void __fastcall Vec3Array_AddScaled(float* dst, const float* src, const float* scaled, unsigned count, float s)
{
    for (; count != 0; --count, dst += 3, src += 3, scaled += 3) {
        const float t0 = static_cast<float>(static_cast<double>(scaled[0]) * s);
        const float t1 = static_cast<float>(static_cast<double>(scaled[1]) * s);
        const float t2 = static_cast<float>(static_cast<double>(scaled[2]) * s);
        const double x = static_cast<double>(src[0]) + t0;
        const double y = static_cast<double>(src[1]) + t1;
        const double z = static_cast<double>(src[2]) + t2;
        dst[0] = static_cast<float>(x);
        dst[1] = static_cast<float>(y);
        dst[2] = static_cast<float>(z);
    }
}

// ---- 0x00474ec0 / 0x00474f40: rotations. x87 inline assembly, because the cosine stays on the x87 stack
// at 64-bit precision on one path (fsin/fcos ignore the precision-control field), which C++ cannot hold.
// Shared prologue, CONFIRMED-BINARY (listings re-read 2026-09-25): |angle| compared with the double
// 9.22e18 at 0x004d2968; if |angle| <= limit (C0|C3, also NaN): FSINCOS on the angle, sin and cos both
// stored as floats and cos reloaded (float precision); else FSIN stored as float, FCOS kept unrounded.
namespace {
const double kSinCosLimit = 9.22e18;  // CONFIRMED-BINARY: double 0x43dffd01499f4680 at 0x004d2968
}

// 0x00474ec0 Vec3_RotateX - CONFIRMED-BINARY: out.x = in.x (dword); out.y = c*y - s*z; out.z = s*y + c*z.
void __fastcall Vec3_RotateX(float* dst, const float* src, float angle)
{
    float s, c;
    double a;
    __asm {
        mov ecx, dst
        mov edx, src
        fld dword ptr angle
        fst qword ptr a
        fld st(0)
        fabs
        fcomp qword ptr kSinCosLimit
        fnstsw ax
        test ah, 0x41
        jnz small_x
        fld st(0)
        fsin
        fstp dword ptr s
        fcos
        jmp body_x
    small_x:
        fstp st(0)
        fld qword ptr a
        fsincos
        fstp dword ptr c
        fstp dword ptr s
        fld dword ptr c
    body_x:
        mov eax, dword ptr [edx]
        fld st(0)
        mov dword ptr [ecx], eax
        fmul dword ptr [edx + 4]
        fld dword ptr s
        fmul dword ptr [edx + 8]
        fsubp st(1), st(0)
        fld dword ptr s
        fmul dword ptr [edx + 4]
        fxch st(2)
        fmul dword ptr [edx + 8]
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [ecx + 8]
        fstp dword ptr [ecx + 4]
    }
}

// 0x00474f40 Vec3_RotateY - CONFIRMED-BINARY: out.y = in.y (dword); out.x = s*z + c*x; out.z = c*z - s*x.
void __fastcall Vec3_RotateY(float* dst, const float* src, float angle)
{
    float s, c;
    double a;
    __asm {
        mov ecx, dst
        mov edx, src
        fld dword ptr angle
        fst qword ptr a
        fld st(0)
        fabs
        fcomp qword ptr kSinCosLimit
        fnstsw ax
        test ah, 0x41
        jnz small_y
        fld st(0)
        fsin
        fstp dword ptr s
        fcos
        jmp body_y
    small_y:
        fstp st(0)
        fld qword ptr a
        fsincos
        fstp dword ptr c
        fstp dword ptr s
        fld dword ptr c
    body_y:
        fld dword ptr s
        fmul dword ptr [edx + 8]
        mov eax, dword ptr [edx + 4]
        fld st(1)
        fmul dword ptr [edx]
        mov dword ptr [ecx + 4], eax
        faddp st(1), st(0)
        fxch st(1)
        fmul dword ptr [edx + 8]
        fld dword ptr s
        fmul dword ptr [edx]
        fsubp st(1), st(0)
        fstp dword ptr [ecx + 8]
        fstp dword ptr [ecx]
    }
}

// 0x00475910 Quat_Multiply - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00475910..0x004759bf).
// Products of two floats are exact in double; each sum is rounded at 53 bits in this order; each
// component is stored before the next is read. (a0..a3 = ECX w,x,y,z; b0..b3 = EDX.)
void __fastcall Quat_Multiply(const float* a, const float* b, float* out)
{
    auto m = [](float x, float y) { return static_cast<double>(x) * y; };
    out[0] = static_cast<float>(((m(b[0], a[0]) - m(a[1], b[1])) - m(a[2], b[2])) - m(a[3], b[3]));
    out[1] = static_cast<float>(((m(b[0], a[1]) + m(a[0], b[1])) + m(b[3], a[2])) - m(a[3], b[2]));
    out[2] = static_cast<float>(((m(b[0], a[2]) + m(a[0], b[2])) + m(a[3], b[1])) - m(b[3], a[1]));
    out[3] = static_cast<float>(((m(b[0], a[3]) + m(a[0], b[3])) + m(b[2], a[1])) - m(a[2], b[1]));
}

// 0x00475a80 Quat_ToMatrix3 - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00475a80..0x00475b79).
// v = 2*(x, y, z) stored as float locals; xx = v.x*x, yy = v.y*y, zz = v.z*z, xy = v.y*x, wz = v.z*w kept
// on the stack; yz, xz, wx, wy stored as float locals. 1.0 = float at 0x004d29c0.
void __fastcall Quat_ToMatrix3(const float* q, float* m)
{
    const float v0 = static_cast<float>(static_cast<double>(q[1]) + q[1]);
    const float v1 = static_cast<float>(static_cast<double>(q[2]) + q[2]);
    const float v2 = static_cast<float>(static_cast<double>(q[3]) + q[3]);
    const double xx = static_cast<double>(v0) * q[1];
    const double yy = static_cast<double>(v1) * q[2];
    const double zz = static_cast<double>(v2) * q[3];
    const double xy = static_cast<double>(v1) * q[1];
    const float yz = static_cast<float>(static_cast<double>(v2) * q[2]);
    const float xz = static_cast<float>(static_cast<double>(v0) * q[3]);
    const float wx = static_cast<float>(static_cast<double>(v0) * q[0]);
    const float wy = static_cast<float>(static_cast<double>(v1) * q[0]);
    const double wz = static_cast<double>(v2) * q[0];
    const double one = 1.0f;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d29c0
    m[0] = static_cast<float>((one - yy) - zz);
    m[1] = static_cast<float>(wz + xy);
    m[2] = static_cast<float>(static_cast<double>(xz) - wy);
    m[3] = static_cast<float>(xy - wz);
    m[4] = static_cast<float>((one - zz) - xx);
    m[5] = static_cast<float>(static_cast<double>(wx) + yz);
    m[6] = static_cast<float>(static_cast<double>(wy) + xz);
    m[7] = static_cast<float>(static_cast<double>(yz) - wx);
    m[8] = static_cast<float>((one - xx) - yy);
}

// 0x00475130 Math_Solve2x2 - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00475130..0x0047520d).
// A = p4-p2, B = p5-p3, C = p0-p2, D = p1-p3 (unrounded); Bf, Cf, Df stored as floats.
// det = B*Cf - A*Df, compared (unrounded) with double 0.0 at 0x004d2970; equal (or NaN) -> both outs = 0.
// Else inv = (float)(1.0 / (float)det) (1.0 float at 0x004d297c), E = p8-p7, F = p6-p7 (Ff float):
// out0 = -(float)((E*Df - Bf*Ff) * inv) ; out1 = -(float)((A*Ff - E*Cf) * inv).
void __fastcall Math_Solve2x2(float* out0, float* out1, float p0, float p1, float p2, float p3, float p4,
                              float p5, float p6, float p7, float p8)
{
    const double A = static_cast<double>(p4) - p2;
    const double B = static_cast<double>(p5) - p3;
    const double C = static_cast<double>(p0) - p2;
    const double D = static_cast<double>(p1) - p3;
    const float Bf = static_cast<float>(B);
    const float Cf = static_cast<float>(C);
    const double BC = B * Cf;
    const float Df = static_cast<float>(D);
    const double det = BC - A * Df;
    const float detf = static_cast<float>(det);
    const double zero = 0.0;  // CONFIRMED-BINARY: double 0x0000000000000000 at 0x004d2970
    if (!(det < zero || det > zero)) {
        *reinterpret_cast<std::uint32_t*>(out0) = 0;
        *reinterpret_cast<std::uint32_t*>(out1) = 0;
        return;
    }
    const double one = 1.0f;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d297c
    const float inv = static_cast<float>(one / detf);
    const double E = static_cast<double>(p8) - p7;
    const float Ff = static_cast<float>(static_cast<double>(p6) - p7);
    const double BfFf = static_cast<double>(Bf) * Ff;
    const double AFf = A * Ff;
    const double EDf = E * Df;
    const double ECf = E * Cf;
    const double r0 = (EDf - BfFf) * inv;
    const double r1 = (AFf - ECf) * inv;
    *out0 = static_cast<float>(-r0);
    *out1 = static_cast<float>(-r1);
}

// ---- batch 4 (listings re-read 2026-09-25)
namespace {
// The "fast sqrt" bit trick used across the engine: bits(r) = (bits(x) >> 1, arithmetic) + 0x1fc00000.
std::uint32_t fast_sqrt_bits(float x)
{
    const std::int32_t i = static_cast<std::int32_t>(bits(x));
    return static_cast<std::uint32_t>((i >> 1) + 0x1fc00000);  // CONFIRMED-BINARY: SAR 1; ADD 0x1fc00000 (e.g. 0x0043b56c)
}
// FLD dword / FSTP dword through the FPU (a signalling NaN comes out quiet, as in the original).
float fpu_copy(std::uint32_t d)
{
    float src = from_bits(d), dst;
    __asm {
        fld dword ptr src
        fstp dword ptr dst
    }
    return dst;
}
}  // namespace

float g_DefaultDir_004dc998[3] = {0.0f, 0.0f, -1.0f};  // CONFIRMED-BINARY: dwords 0, 0, 0xbf800000 at 0x004dc998

// 0x0043b500 Vec3_SetYKeepXZDir - CONFIRMED-BINARY (0x0043b500..0x0043b5c5).
// sum = z*z + x*x (unrounded) compared with float 0.0 at 0x004d1794:
//  sum == 0 (or NaN): y == 0 (or NaN) -> v = default direction (dword copies); else
//     r = fastsqrt((float)((1.0 - y*y) * 0.5)) -> v = (r, y, r) as dwords.
//  else: r = fastsqrt((float)((1.0 - y*y) / sum)) -> v.x = (float)(r*x), v.y = y (dword), v.z = (float)(r*z).
void __fastcall Vec3_SetYKeepXZDir(float* v, int, float y)
{
    const double zero = 0.0f;  // CONFIRMED-BINARY: float 0x00000000 at 0x004d1794
    const double one = 1.0f;   // CONFIRMED-BINARY: float 0x3f800000 at 0x004d17b8
    const double half = 0.5f;  // CONFIRMED-BINARY: float 0x3f000000 at 0x004d17e0
    const double sum = static_cast<double>(v[2]) * v[2] + static_cast<double>(v[0]) * v[0];
    auto* w = reinterpret_cast<std::uint32_t*>(v);
    if (x87_c3_equal(sum, zero)) {
        if (x87_c3_equal(y, zero)) {
            const auto* d = reinterpret_cast<const std::uint32_t*>(g_DefaultDir_004dc998);
            w[0] = d[0];
            w[1] = d[1];
            w[2] = d[2];
            return;
        }
        const std::uint32_t r = fast_sqrt_bits(static_cast<float>((one - static_cast<double>(y) * y) * half));
        w[2] = r;
        w[0] = r;
        w[1] = bits(y);
        return;
    }
    const float r = from_bits(fast_sqrt_bits(static_cast<float>((one - static_cast<double>(y) * y) / sum)));
    const double rx = static_cast<double>(r) * v[0];
    w[1] = bits(y);
    v[0] = static_cast<float>(rx);
    v[2] = static_cast<float>(static_cast<double>(r) * v[2]);
}

// 0x004525d0 Math_BoxToSphere - CONFIRMED-BINARY (0x004525d0..0x00452640). h = (max - min) * 0.5 per axis
// (unrounded, 0.5 = float at 0x004d23c0); centre = (float)(h + min); r2 = (float)((hx*hx + hy*hy) + hz*hz);
// radius = fastsqrt(r2) passed through FLD/FSTP.
void __fastcall Math_BoxToSphere(const float* box, float* centre, float* radius)
{
    const double half = 0.5f;  // CONFIRMED-BINARY: float 0x3f000000 at 0x004d23c0
    const double hx = (static_cast<double>(box[3]) - box[0]) * half;
    const double hy = (static_cast<double>(box[4]) - box[1]) * half;
    const double hz = (static_cast<double>(box[5]) - box[2]) * half;
    centre[0] = static_cast<float>(hx + box[0]);
    centre[1] = static_cast<float>(hy + box[1]);
    centre[2] = static_cast<float>(hz + box[2]);
    const float r2 = static_cast<float>((hx * hx + hy * hy) + hz * hz);
    *radius = fpu_copy(fast_sqrt_bits(r2));
}

// 0x00472860 Vec3_Reflect - CONFIRMED-BINARY (0x00472860..0x00472951). k = (float)((n.x*d.x + n.y*d.y) + n.z*d.z);
// k == 0 (or NaN, vs float 0.0 at 0x004d2918): out = d * -1.0 (float at 0x004d2928) per component.
// Else m = -k; t = (float)(n*m) per component (locals); u = (float)(d + t); out = (float)(t + u).
void __fastcall Vec3_Reflect(const float* n, const float* d, float* out)
{
    const float k = static_cast<float>((static_cast<double>(n[0]) * d[0] + static_cast<double>(n[1]) * d[1]) +
                                       static_cast<double>(n[2]) * d[2]);
    const double zero = 0.0f;       // CONFIRMED-BINARY: float 0x00000000 at 0x004d2918
    if (x87_c3_equal(k, zero)) {
        const double neg1 = -1.0f;  // CONFIRMED-BINARY: float 0xbf800000 at 0x004d2928
        out[0] = static_cast<float>(d[0] * neg1);
        out[1] = static_cast<float>(d[1] * neg1);
        out[2] = static_cast<float>(d[2] * neg1);
        return;
    }
    const double m = -static_cast<double>(k);
    float tt[3], uu[3];
    tt[0] = static_cast<float>(m * n[0]);
    tt[1] = static_cast<float>(static_cast<double>(n[1]) * m);
    tt[2] = static_cast<float>(static_cast<double>(n[2]) * m);
    const double u0 = static_cast<double>(d[0]) + tt[0], u1 = static_cast<double>(d[1]) + tt[1], u2 = static_cast<double>(d[2]) + tt[2];
    uu[0] = static_cast<float>(u0);
    uu[1] = static_cast<float>(u1);
    uu[2] = static_cast<float>(u2);
    const double o0 = static_cast<double>(tt[0]) + uu[0], o1 = static_cast<double>(tt[1]) + uu[1], o2 = static_cast<double>(tt[2]) + uu[2];
    out[0] = static_cast<float>(o0);
    out[1] = static_cast<float>(o1);
    out[2] = static_cast<float>(o2);
}

// 0x00472cc0 Vec2_PerpNormalized - CONFIRMED-BINARY (0x00472cc0..0x00472d2a). out.z = 0 (dword) first;
// in.x == 0 (or NaN, vs float 0.0 at 0x004d2918): out = (1.0, 0) as dwords. Else sq = (float)(y*y + x*x),
// r = fastsqrt(sq), inv = 1.0 / r (unrounded); out.x = (float)(inv*in.y); then out.y = (float)-(inv*in.x)
// (in.x is read AFTER out.x is written).
void __fastcall Vec2_PerpNormalized(const float* in, float* out)
{
    auto* w = reinterpret_cast<std::uint32_t*>(out);
    w[2] = 0;
    const double zero = 0.0f;  // CONFIRMED-BINARY: float 0x00000000 at 0x004d2918
    if (x87_c3_equal(in[0], zero)) {
        w[0] = 0x3f800000;  // CONFIRMED-BINARY: MOV [EDX], 0x3f800000 at 0x00472cdc (1.0f)
        w[1] = 0;
        return;
    }
    const float sq = static_cast<float>(static_cast<double>(in[1]) * in[1] + static_cast<double>(in[0]) * in[0]);
    const double one = 1.0f;   // CONFIRMED-BINARY: float 0x3f800000 at 0x004d291c
    const double inv = one / from_bits(fast_sqrt_bits(sq));
    out[0] = static_cast<float>(inv * in[1]);
    out[1] = static_cast<float>(-(inv * in[0]));
}

// 0x00474580 Vec3_FromHeading - CONFIRMED-BINARY (0x00474580..0x004745b6): out zeroed (dwords), then
// Vec3_RotateY(out, local (0, 0, -1.0), angle).
void __fastcall Vec3_FromHeading(float* out, int, float angle)
{
    auto* w = reinterpret_cast<std::uint32_t*>(out);
    w[0] = w[1] = w[2] = 0;
    const std::uint32_t local[3] = {0, 0, 0xbf800000};  // CONFIRMED-BINARY: 0x0047459e..0x004745a6 (0, 0, -1.0f)
    Vec3_RotateY(out, reinterpret_cast<const float*>(local), angle);
}

// 0x00472960 Vec3_Lerp - CONFIRMED-BINARY (listing re-read 2026-09-25, 0x00472960..0x004729a4):
//   u = 1.0 - t kept on the x87 stack; for each component in order x, y, z:
//   a[i] = (float)(t * a[i] + u * b[i]); a[i+1] is read before a[i] is stored. ret 4.
void __fastcall Vec3_Lerp(float* a, const float* b, float t)
{
    const double one = 1.0f;  // CONFIRMED-BINARY: float 0x3f800000 at 0x004d291c
    const double u = one - static_cast<double>(t);
    const double ta0 = static_cast<double>(t) * a[0];
    const double x = ta0 + u * b[0];
    const double ta1 = static_cast<double>(t) * a[1];
    a[0] = static_cast<float>(x);
    const double y = ta1 + u * b[1];
    const double ta2 = static_cast<double>(t) * a[2];
    a[1] = static_cast<float>(y);
    const double z = ta2 + u * b[2];
    a[2] = static_cast<float>(z);
}

}  // namespace recoil
