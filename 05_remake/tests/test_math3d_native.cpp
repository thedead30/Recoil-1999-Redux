// Native L1: port vs the ORIGINAL function bytes executed on the real x87 (tests/native_oracle.h),
// on random inputs plus the edge values (0, -0, NaN, inf) the listings branch on.
#include "test.h"
#include "vectors.h"
#include "native_oracle.h"
#include "unattributed/math3d.h"
#include "Battlesport/ai_net.h"

#include <cmath>
#include <limits>
#include <random>

namespace {
std::mt19937 rng(0x5EC0);
float rnd(float lo = -1000.0f, float hi = 1000.0f)
{
    const float edges[] = {0.0f, -0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity(), 1e-40f, -1e-40f};
    if (rng() % 10 == 0) return edges[rng() % 7];
    return std::uniform_real_distribution<float>(lo, hi)(rng);
}
// Number of values on the x87 register stack (8 - count of empty tag pairs).
int x87_depth()
{
    unsigned short env[14];
    __asm fnstenv env
    __asm fldenv env
    const unsigned tag = env[4];
    int used = 0;
    for (int i = 0; i < 8; ++i)
        if (((tag >> (2 * i)) & 3) != 3) ++used;
    return used;
}
bool same_bits(const float* a, const float* b, int n)
{
    for (int i = 0; i < n; ++i)
        if (rt::float_bits(a[i]) != rt::float_bits(b[i])) return false;
    return true;
}
}  // namespace

TEST(native_oracle_maps_original_image)
{
    CHECK(rt::map_original());
}

TEST(native_math3d_batch_matches_original_on_real_fpu)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Scale = void(__fastcall*)(const float*, float*, float);
    using Perp = void(__fastcall*)(const float*, float*);
    using Norm = void(__fastcall*)(float*, float*);
    using DistF = float(__fastcall*)(const float*, const float*);
    using Lerp = void(__fastcall*)(float*, const float*, float);
    using AddS = void(__fastcall*)(const float*, const float*, float, float*);
    using Corners = void(__fastcall*)(const float*, float*);
    auto oScale = rt::original<Scale>(0x004727a0);
    auto oPerp = rt::original<Perp>(0x004745c0);
    auto oNorm = rt::original<Norm>(0x004727f0);
    auto oDsq = rt::original<DistF>(0x00472670);
    auto oDst = rt::original<DistF>(0x004726d0);
    auto oDxz = rt::original<DistF>(0x00472730);
    auto oLerp = rt::original<Lerp>(0x00472960);
    auto oAdd = rt::original<AddS>(0x00472770);
    auto oCorn = rt::original<Corners>(0x00446ed0);
    auto* oScratch = reinterpret_cast<float*>(0x00566420);  // the original's own scratch global

    int bad = 0;
    for (int n = 0; n < 4000; ++n) {
        float a[6], b[3];
        for (float& x : a) x = rnd();
        for (float& x : b) x = rnd();
        const float s = rnd(-10.0f, 10.0f);
        {   // Vec3_ScaleByReciprocal (both dst != src and dst == src)
            float d1[3] = {7, 7, 7}, d2[3] = {7, 7, 7}, s1[3], s2[3];
            std::memcpy(s1, a, 12); std::memcpy(s2, a, 12);
            oScale(a, d1, s); recoil::Vec3_ScaleByReciprocal(a, d2, s);
            oScale(s1, s1, s); recoil::Vec3_ScaleByReciprocal(s2, s2, s);
            if (!same_bits(d1, d2, 3) || !same_bits(s1, s2, 3)) ++bad;
        }
        {   float o1[3] = {1, 2, 3}, o2[3] = {1, 2, 3};
            oPerp(a, o1); recoil::Vec3_PerpXZ(a, o2);
            if (!same_bits(o1, o2, 3)) ++bad;
        }
        {   float v1[3], v2[3], o1[3] = {5, 5, 5}, o2[3] = {5, 5, 5};
            std::memcpy(v1, a, 12); std::memcpy(v2, a, 12);
            oNorm(v1, o1); recoil::Math_NormalizeHorizontalVector(v2, o2);
            if (!same_bits(o1, o2, 3) || !same_bits(v1, v2, 3)) ++bad;
        }
        for (auto [o, p] : {std::pair<DistF, DistF>{oDsq, recoil::Vec3_DistanceSquared}, {oDst, recoil::Vec3_Distance}, {oDxz, recoil::Vec3_DistSqXZ}}) {
            for (int i = 0; i < 3; ++i) oScratch[i] = recoil::g_Vec3Scratch_00566420[i] = 0.0f;
            const float r1 = o(a, b), r2 = p(a, b);
            if (rt::float_bits(r1) != rt::float_bits(r2) || !same_bits(oScratch, recoil::g_Vec3Scratch_00566420, 3)) ++bad;
        }
        {   float l1[3], l2[3];
            std::memcpy(l1, a, 12); std::memcpy(l2, a, 12);
            oLerp(l1, b, s); recoil::Vec3_Lerp(l2, b, s);
            if (!same_bits(l1, l2, 3)) ++bad;
        }
        {   float o1[3], o2[3];
            oAdd(a, b, s, o1); recoil::Vec3_AddScaled(a, b, s, o2);
            if (!same_bits(o1, o2, 3)) ++bad;
        }
        {   float c1[24], c2[24];
            oCorn(a, c1); recoil::AABB_ToCorners(a, c2);
            if (!same_bits(c1, c2, 24)) ++bad;
        }
    }
    CHECK_EQ(bad, 0);
}

TEST(native_math3d_batch3_matches_original_on_real_fpu)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Rot = void(__fastcall*)(float*, const float*, float);
    using QMul = void(__fastcall*)(const float*, const float*, float*);
    using QMat = void(__fastcall*)(const float*, float*);
    using Solve = void(__fastcall*)(float*, float*, float, float, float, float, float, float, float, float, float);
    auto oRx = rt::original<Rot>(0x00474ec0);
    auto oRy = rt::original<Rot>(0x00474f40);
    auto oQm = rt::original<QMul>(0x00475910);
    auto oQx = rt::original<QMat>(0x00475a80);
    auto oSv = rt::original<Solve>(0x00475130);
    int bad[5] = {};
    for (int n = 0; n < 4000; ++n) {
        float v[4], w[4], p[9];
        for (float& x : v) x = rnd();
        for (float& x : w) x = rnd(-2, 2);
        for (float& x : p) x = rnd(-50, 50);
        if (n % 97 == 0) { p[4] = p[2]; p[5] = p[3]; }  // det == 0 path
        float ang = (n % 50 == 0) ? 1e19f : rnd(-20, 20);
        {   float o1[3] = {9, 9, 9}, o2[3] = {9, 9, 9};
            oRx(o1, v, ang); recoil::Vec3_RotateX(o2, v, ang);
            if (!same_bits(o1, o2, 3)) ++bad[0];
            oRy(o1, v, ang); recoil::Vec3_RotateY(o2, v, ang);
            if (!same_bits(o1, o2, 3)) ++bad[1];
        }
        {   float o1[4], o2[4];
            oQm(v, w, o1); recoil::Quat_Multiply(v, w, o2);
            if (!same_bits(o1, o2, 4)) ++bad[2];
        }
        {   float m1[9], m2[9];
            oQx(w, m1); recoil::Quat_ToMatrix3(w, m2);
            if (!same_bits(m1, m2, 9)) ++bad[3];
        }
        {   float a1 = 5, b1 = 5, a2 = 5, b2 = 5;
            oSv(&a1, &b1, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8]);
            recoil::Math_Solve2x2(&a2, &b2, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8]);
            if (rt::float_bits(a1) != rt::float_bits(a2) || rt::float_bits(b1) != rt::float_bits(b2)) ++bad[4];
        }
    }
    for (int i = 0; i < 5; ++i) CHECK_EQ(bad[i], 0);
}

TEST(native_math3d_batch4_matches_original_on_real_fpu)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using SetY = void(__fastcall*)(float*, int, float);
    using Box = void(__fastcall*)(const float*, float*, float*);
    using Refl = void(__fastcall*)(const float*, const float*, float*);
    using Perp2 = void(__fastcall*)(const float*, float*);
    using Head = void(__fastcall*)(float*, int, float);
    auto oSetY = rt::original<SetY>(0x0043b500);
    auto oBox = rt::original<Box>(0x004525d0);
    auto oRefl = rt::original<Refl>(0x00472860);
    auto oPerp = rt::original<Perp2>(0x00472cc0);
    auto oHead = rt::original<Head>(0x00474580);
    int bad[5] = {};
    for (int n = 0; n < 4000; ++n) {
        float v[6], d[3];
        for (float& x : v) x = rnd(-50, 50);
        for (float& x : d) x = rnd(-50, 50);
        if (n % 40 == 0) { v[0] = 0; v[2] = 0; }
        const float y = (n % 60 == 0) ? 0.0f : rnd(-1.5f, 1.5f);
        {   float a[3], b[3];
            std::memcpy(a, v, 12); std::memcpy(b, v, 12);
            oSetY(a, 0, y); recoil::Vec3_SetYKeepXZDir(b, 0, y);
            if (!same_bits(a, b, 3)) ++bad[0];
        }
        {   float c1[3], c2[3], r1, r2;
            oBox(v, c1, &r1); recoil::Math_BoxToSphere(v, c2, &r2);
            if (!same_bits(c1, c2, 3) || rt::float_bits(r1) != rt::float_bits(r2)) ++bad[1];
        }
        {   float o1[3], o2[3];
            oRefl(v, d, o1); recoil::Vec3_Reflect(v, d, o2);
            if (!same_bits(o1, o2, 3)) ++bad[2];
        }
        {   float o1[3] = {3, 3, 3}, o2[3] = {3, 3, 3};
            oPerp(v, o1); recoil::Vec2_PerpNormalized(v, o2);
            if (!same_bits(o1, o2, 3)) ++bad[3];
        }
        {   float o1[3], o2[3];
            const float ang = rnd(-20, 20);
            oHead(o1, 0, ang); recoil::Vec3_FromHeading(o2, 0, ang);
            if (!same_bits(o1, o2, 3)) ++bad[4];
        }
    }
    for (int i = 0; i < 5; ++i) CHECK_EQ(bad[i], 0);
}

namespace {
// Call a function returning its result in ST0 and store it with FSTP qword (80-bit -> 64-bit rounding):
// enough to compare, since both sides round the same extended value the same way.
using St0Fn2 = long double(__fastcall*)(const float*, const float*);
using St0Fn1 = long double(__fastcall*)(const float*);
double st0_to_double2(St0Fn2 f, const float* a, const float* b)
{
    double r;
    __asm {
        mov ecx, a
        mov edx, b
        call f
        fstp qword ptr r
    }
    return r;
}
double st0_to_double1(St0Fn1 f, const float* a)
{
    double r;
    __asm {
        mov ecx, a
        call f
        fstp qword ptr r
    }
    return r;
}
bool same_d(double x, double y)
{
    std::uint64_t a, b;
    std::memcpy(&a, &x, 8);
    std::memcpy(&b, &y, 8);
    return a == b;
}
}  // namespace

TEST(native_math3d_batch5_matches_original_on_real_fpu)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Look = void(__fastcall*)(const float*, const float*, float*);
    using Euler = void(__fastcall*)(float*, int, float, float, float);
    using BoxT = void(__fastcall*)(const float*, const float*, float*);
    using Norm = float(__fastcall*)(float*);
    auto oLook = rt::original<Look>(0x00474d10);
    auto oPitch = rt::original<St0Fn2>(0x00474d90);
    auto oYaw = rt::original<St0Fn1>(0x00474de0);
    auto oEuler = rt::original<Euler>(0x00474260);
    auto oBox = rt::original<BoxT>(0x00474870);
    auto oNorm = rt::original<Norm>(0x00402f60);
    int bad[6] = {};
    for (int n = 0; n < 4000; ++n) {
        float a[12], b[6];
        for (float& x : a) x = rnd(-100, 100);
        for (float& x : b) x = rnd(-100, 100);
        if (n % 50 == 0) { a[6] = 0; a[8] = 0; }
        {   float o1[3] = {1, 1, 1}, o2[3] = {1, 1, 1};
            oLook(a, b, o1); recoil::Math_ComputeLookAtPitchYaw(a, b, o2);
            if (!same_bits(o1, o2, 3)) ++bad[0];
        }
        if (!same_d(st0_to_double2(oPitch, a, b), st0_to_double2(recoil::Math_LookAtPitch, a, b))) ++bad[1];
        if (!same_d(st0_to_double1(oYaw, a), st0_to_double1(recoil::Mat3_YawFromRows, a))) ++bad[2];
        {   float m1[12], m2[12];
            const float x = rnd(-10, 10), y = (n % 70 == 0) ? 1e19f : rnd(-10, 10), z = rnd(-10, 10);
            oEuler(m1, 0, x, y, z); recoil::Mat3_FromEuler(m2, 0, x, y, z);
            if (!same_bits(m1, m2, 12)) ++bad[3];
        }
        {   float o1[24], o2[24];
            oBox(a, b, o1); recoil::Box8_TransformByMatrix(a, b, o2);
            if (!same_bits(o1, o2, 24)) ++bad[4];
        }
        {   float v1[3], v2[3];
            std::memcpy(v1, b, 12); std::memcpy(v2, b, 12);
            if (n % 30 == 0) { v1[0] = v1[1] = v1[2] = v2[0] = v2[1] = v2[2] = 0.0f; }
            const float l1 = oNorm(v1), l2 = recoil::Vec3_NormalizeInPlace(v2);
            if (!same_bits(v1, v2, 3) || rt::float_bits(l1) != rt::float_bits(l2)) ++bad[5];
        }
    }
    for (int i = 0; i < 6; ++i) CHECK_EQ(bad[i], 0);
}

TEST(native_math3d_batch6_matches_original_on_real_fpu)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F3 = float(__fastcall*)(const float*, const float*, float*);
    using LN = void(__fastcall*)(float*, const float*, float);
    using TN = void(__fastcall*)(const float*, const float*, const float*, float*);
    using QR = void(__fastcall*)(const float*, float*);
    using RS = int(__fastcall*)(const float*, const float*, float, const float*, float*);
    using SL = void(__fastcall*)(const float*, const float*, float, float*);
    auto oSub = rt::original<F3>(0x004729b0);
    auto oLN = rt::original<LN>(0x004729f0);
    auto oTN = rt::original<TN>(0x00475070);
    auto oQR = rt::original<QR>(0x00475b80);
    auto oRS = rt::original<RS>(0x00475210);
    auto oEu = rt::original<QR>(0x00474e10);
    auto oSl = rt::original<SL>(0x00472a10);
    int bad[7] = {}, leaks = 0;
    for (int n = 0; n < 4000; ++n) {
        if (x87_depth() != 0) ++leaks;
        float a[12], b[12], c[3];
        for (float& x : a) x = rnd(-10, 10);
        for (float& x : b) x = rnd(-10, 10);
        for (float& x : c) x = rnd(-10, 10);
        if (n % 40 == 0) std::memcpy(b, a, 12);
        {   float o1[3], o2[3];
            const float l1 = oSub(a, b, o1), l2 = recoil::Vec3_SubNormalize(a, b, o2);
            if (!same_bits(o1, o2, 3) || rt::float_bits(l1) != rt::float_bits(l2)) ++bad[0];
        }
        {   float v1[3], v2[3];
            std::memcpy(v1, a, 12); std::memcpy(v2, a, 12);
            const float t = rnd(-1, 2);
            oLN(v1, b, t); recoil::Vec3_LerpNormalize(v2, b, t);
            if (!same_bits(v1, v2, 3)) ++bad[1];
        }
        {   float o1[3], o2[3];
            oTN(a, b, c, o1); recoil::Tri_Normal(a, b, c, o2);
            if (!same_bits(o1, o2, 3)) {
                if (!bad[2]) std::printf("  TriNormal a=(%g %g %g) b=(%g %g %g) c=(%g %g %g) orig=(%.9g %.9g %.9g) port=(%.9g %.9g %.9g)\n",
                    a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2], o1[0], o1[1], o1[2], o2[0], o2[1], o2[2]);
                ++bad[2];
            }
        }
        {   float v[3], o1[4] = {7, 7, 7, 7}, o2[4] = {7, 7, 7, 7};
            std::memcpy(v, a, 12);
            if (n % 30 == 0) v[0] = v[1] = v[2] = 0.0f;
            oQR(v, o1); recoil::Quat_FromRotationVector(v, o2);
            if (!same_bits(o1, o2, 4)) ++bad[3];
        }
        {   float o1[8] = {}, o2[8] = {};
            const float r = rnd(0, 20);
            const int k1 = oRS(a, b, r, c, o1), k2 = recoil::Math_RaySphereNormal(a, b, r, c, o2);
            if (k1 != k2 || !same_bits(o1, o2, 8)) ++bad[4];
        }
        {   float m[12], o1[3], o2[3];
            recoil::Mat3_FromEuler(m, 0, rnd(-3, 3), rnd(-3, 3), rnd(-3, 3));
            if (n % 25 == 0) for (float& x : m) x = rnd(-1, 1);
            oEu(m, o1); recoil::Mat3_ToEuler(m, o2);
            if (!same_bits(o1, o2, 3)) ++bad[5];
        }
        {   float f[3], g[3], o1[3] = {4, 4, 4}, o2[3] = {4, 4, 4};
            std::memcpy(f, a, 12); std::memcpy(g, b, 12);
            recoil::Vec3_NormalizeInPlace(f); recoil::Vec3_NormalizeInPlace(g);
            if (n % 20 == 0) { g[0] = -f[0]; g[1] = -f[1]; g[2] = -f[2]; }
            if (n % 21 == 0) std::memcpy(g, f, 12);
            const float t = (n % 17 == 0) ? 0.0f : (n % 19 == 0) ? 1.0f : rnd(0, 1);
            oSl(f, g, t, o1); recoil::Vec3_Slerp(f, g, t, o2);
            if (!same_bits(o1, o2, 3)) ++bad[6];
        }
    }
    CHECK_EQ(leaks, 0);
    for (int i = 0; i < 7; ++i) {
        if (bad[i]) std::printf("  batch6 function %d: %d mismatches\n", i, bad[i]);
        CHECK_EQ(bad[i], 0);
    }
}
