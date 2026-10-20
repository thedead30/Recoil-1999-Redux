// L1 tests for math3d: the port against golden vectors from the original's bytes (tools/emu_harness).
#include "test.h"
#include "vectors.h"
#include "unattributed/math3d.h"

TEST(math3d_Vec3_Lerp_matches_original_bytes)
{
    auto cases = rt::load_vectors("Vec3_Lerp");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float a[3], b[3];
        for (int i = 0; i < 3; ++i) {
            a[i] = rt::hex_float(c["a"][i]);
            b[i] = rt::hex_float(c["b"][i]);
        }
        recoil::Vec3_Lerp(a, b, rt::hex_float(c["t"][0]));
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(a[i]), rt::hex_dword(c["out"][i]));
    }
}

// L3: the same function against 64 real calls recorded in trace Recoil22 (tools/trace_vectors).
TEST(math3d_Vec3_Lerp_matches_trace_Recoil22)
{
    auto cases = rt::load_vectors("Vec3_Lerp.trace");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float a[3], b[3];
        for (int i = 0; i < 3; ++i) {
            a[i] = rt::hex_float(c["in_ecx"][i]);
            b[i] = rt::hex_float(c["in_edx"][i]);
        }
        recoil::Vec3_Lerp(a, b, rt::hex_float(c["s0"][0]));
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(a[i]), rt::hex_dword(c["out_ecx"][i]));
    }
}

static void check_two_vec_ret(const char* name, float (__fastcall *fn)(const float*, const float*), bool xz)
{
    auto cases = rt::load_vectors(name);
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float a[3], b[3];
        for (int i = 0; i < 3; ++i) {
            a[i] = rt::hex_float(c["a"][i]);
            b[i] = rt::hex_float(c["b"][i]);
            recoil::g_Vec3Scratch_00566420[i] = 0.0f;  // the harness zeroes the scratch before each call
        }
        float r = fn(a, b);
        CHECK_EQ(rt::float_bits(r), rt::hex_dword(c["ret"][0]));
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(recoil::g_Vec3Scratch_00566420[i]), rt::hex_dword(c["scratch"][i]));
        (void)xz;
    }
}

TEST(math3d_Vec3_DistanceSquared_matches_original_bytes) { check_two_vec_ret("Vec3_DistanceSquared", recoil::Vec3_DistanceSquared, false); }
TEST(math3d_Vec3_Distance_matches_original_bytes) { check_two_vec_ret("Vec3_Distance", recoil::Vec3_Distance, false); }
TEST(math3d_Vec3_DistSqXZ_matches_original_bytes) { check_two_vec_ret("Vec3_DistSqXZ", recoil::Vec3_DistSqXZ, true); }

TEST(math3d_Vec3_AddScaled_matches_original_bytes)
{
    auto cases = rt::load_vectors("Vec3_AddScaled");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float a[3], b[3], out[3];
        for (int i = 0; i < 3; ++i) {
            a[i] = rt::hex_float(c["a"][i]);
            b[i] = rt::hex_float(c["b"][i]);
        }
        recoil::Vec3_AddScaled(a, b, rt::hex_float(c["s"][0]), out);
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(out[i]), rt::hex_dword(c["out"][i]));
    }
}

// ---- batch 2 (AABB_ToCorners, ScaleByReciprocal, NormalizeHorizontal, PerpXZ, ArrayAddScaled)
static void load3(float* d, const std::vector<std::string>& h, size_t off = 0)
{
    for (int i = 0; i < 3; ++i) d[i] = rt::hex_float(h[off + i]);
}

TEST(math3d_AABB_ToCorners_matches_original_bytes)
{
    auto cases = rt::load_vectors("AABB_ToCorners");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float box[6], out[24];
        for (int i = 0; i < 6; ++i) box[i] = rt::hex_float(c["box"][i]);
        recoil::AABB_ToCorners(box, out);
        for (int i = 0; i < 24; ++i) CHECK_EQ(rt::float_bits(out[i]), rt::hex_dword(c["out"][i]));
    }
}

TEST(math3d_Vec3_ScaleByReciprocal_matches_original_bytes)
{
    auto cases = rt::load_vectors("Vec3_ScaleByReciprocal");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float src[3], dst[3];
        load3(src, c["src"]);
        const bool same = rt::hex_dword(c["same"][0]) != 0;
        float* d = same ? src : dst;
        if (!same)
            for (int i = 0; i < 3; ++i) dst[i] = rt::hex_float("11111111");
        recoil::Vec3_ScaleByReciprocal(src, d, rt::hex_float(c["s"][0]));
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(d[i]), rt::hex_dword(c["out"][i]));
    }
}

TEST(math3d_Math_NormalizeHorizontalVector_matches_original_bytes)
{
    auto cases = rt::load_vectors("Math_NormalizeHorizontalVector");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float v[3], out[3];
        load3(v, c["v"]);
        for (int i = 0; i < 3; ++i) out[i] = rt::hex_float("22222222");
        recoil::Math_NormalizeHorizontalVector(v, out);
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(out[i]), rt::hex_dword(c["out"][i]));
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(v[i]), rt::hex_dword(c["v_after"][i]));
    }
}

TEST(math3d_Vec3_PerpXZ_matches_original_bytes)
{
    auto cases = rt::load_vectors("Vec3_PerpXZ");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        float v[3], out[3];
        load3(v, c["v"]);
        recoil::Vec3_PerpXZ(v, out);
        for (int i = 0; i < 3; ++i) CHECK_EQ(rt::float_bits(out[i]), rt::hex_dword(c["out"][i]));
    }
}

TEST(math3d_Vec3Array_AddScaled_matches_original_bytes)
{
    auto cases = rt::load_vectors("Vec3Array_AddScaled");
    CHECK(cases.size() >= 32);
    for (auto& c : cases) {
        const unsigned n = rt::hex_dword(c["n"][0]) == 0 ? 0u : 0u;  // "n" is written big-endian text
        const unsigned count = static_cast<unsigned>(std::stoul(c["n"][0], nullptr, 16));
        (void)n;
        const size_t m = count ? count : 1;
        std::vector<float> dst(3 * m), src(3 * m), scl(3 * m);
        for (size_t i = 0; i < 3 * m; ++i) {
            dst[i] = rt::hex_float("44444444");
            src[i] = rt::hex_float(c["src"][i]);
            scl[i] = rt::hex_float(c["scl"][i]);
        }
        recoil::Vec3Array_AddScaled(dst.data(), src.data(), scl.data(), count, rt::hex_float(c["s"][0]));
        for (size_t i = 0; i < 3 * m; ++i) CHECK_EQ(rt::float_bits(dst[i]), rt::hex_dword(c["out"][i]));
    }
}
