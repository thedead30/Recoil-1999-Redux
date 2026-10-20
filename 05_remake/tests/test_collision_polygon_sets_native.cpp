// Structured native L1s for the polygon queries ported in the cloud (P3 collision), on real geometry:
//  - Collision_PointInPolygonXZ_Height (0x004856d0): ECX result record (normal +0..+8, height +0x10), EDX vertices,
//    stack x, z, vertex count; ret 0xC. One winding only, -0.0001 tolerance.
//  - Collision_PointSetVsPolygon (0x00484b70): ECX hit node, EDX per-point results (0x504 bytes: count + 32 entries of
//    0x28), stack points (x,y,z), active mask, count, height ceiling, vertices, polygon record ([rec] & 0xff = vertex
//    count, +0x14/+0x18 recorded); ret 0x18. Upward-facing polygons only.
//  - Collision_SegmentSetVsPolygon (0x00486290): ECX hit node, EDX per-segment results, stack endpoints (6 floats per
//    segment), mask, count, vertices, polygon record (bit 0x100 two-sided); ret 0x14.
// Convex polygons of 3..6 vertices (regular, some jittered) on planes of random orientation (axis-aligned and upward
// ones included), both windings; points/segments inside, near the edge and outside; result counts seeded from 0 to 33
// so the 32-entry limit is hit; masks with inactive entries. Compared: the return, the result record / every result
// array word, the caller's mask.
#include "test.h"
#include "cloud_harness.h"
#include "unattributed/collision.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

namespace {
struct Poly {
    int n;
    float v[18];
    double nx, ny, nz, ox, oy, oz, ax, ay, az, bx, by, bz, r;
};

// a convex polygon on a random plane; up biases the normal towards +Y
Poly make_poly(std::mt19937& rng, bool up)
{
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    Poly p;
    p.n = 3 + static_cast<int>(rng() % 4);
    if (rng() % 3 == 0) {
        const int a = up ? 1 : static_cast<int>(rng() % 3);
        p.nx = a == 0; p.ny = a == 1; p.nz = a == 2;
    } else {
        p.nx = u(rng); p.ny = up ? 0.2 + (u(rng) + 1) : u(rng); p.nz = u(rng);
    }
    if (rng() % 4 == 0) { p.nx = -p.nx; p.ny = -p.ny; p.nz = -p.nz; }  // some facing down / the other way
    const double nl = std::sqrt(p.nx * p.nx + p.ny * p.ny + p.nz * p.nz) + 1e-12;
    p.nx /= nl; p.ny /= nl; p.nz /= nl;
    const double tx = std::fabs(p.nx) < 0.9 ? 1 : 0, ty = std::fabs(p.nx) < 0.9 ? 0 : 1, tz = 0;
    p.ax = p.ny * tz - p.nz * ty; p.ay = p.nz * tx - p.nx * tz; p.az = p.nx * ty - p.ny * tx;
    const double al = std::sqrt(p.ax * p.ax + p.ay * p.ay + p.az * p.az);
    p.ax /= al; p.ay /= al; p.az /= al;
    p.bx = p.ny * p.az - p.nz * p.ay; p.by = p.nz * p.ax - p.nx * p.az; p.bz = p.nx * p.ay - p.ny * p.ax;
    p.ox = 20 * u(rng); p.oy = 20 * u(rng); p.oz = 20 * u(rng);
    p.r = 0.5 + 5 * (u(rng) + 1);
    const double rot = 3.14159 * u(rng);
    const bool reverse = rng() % 2 != 0;
    for (int k = 0; k < p.n; ++k) {
        const int kk = reverse ? p.n - 1 - k : k;
        const double ang = rot + 2 * 3.14159265358979 * kk / p.n, jr = rng() % 4 == 0 ? p.r * (0.8 + 0.1 * (u(rng) + 1)) : p.r;
        const double px = jr * std::cos(ang), py = jr * std::sin(ang);
        p.v[3 * k] = static_cast<float>(p.ox + px * p.ax + py * p.bx);
        p.v[3 * k + 1] = static_cast<float>(p.oy + px * p.ay + py * p.by);
        p.v[3 * k + 2] = static_cast<float>(p.oz + px * p.az + py * p.bz);
    }
    return p;
}

// a point of the plane: inside, near the edge or outside; returned in plane coordinates
void plane_point(std::mt19937& rng, const Poly& p, double& x, double& y, double& z)
{
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    const int kind = static_cast<int>(rng() % 3);
    const double s = kind == 0 ? 0.3 * p.r * (u(rng) + 1) / 2 : kind == 1 ? p.r * (0.9 + 0.1 * (u(rng) + 1)) : 2 * p.r * (1 + (u(rng) + 1));
    const double ang = 3.14159 * u(rng), qx = s * std::cos(ang), qy = s * std::sin(ang);
    x = p.ox + qx * p.ax + qy * p.bx; y = p.oy + qx * p.ay + qy * p.by; z = p.oz + qx * p.az + qy * p.bz;
}

std::vector<std::uint32_t> seed_results(std::mt19937& rng, int count)
{
    const int max_entries = 32;
    std::vector<std::uint32_t> r(max_entries * (0x504 / 4));
    for (std::size_t k = 0; k < r.size(); ++k) r[k] = 0xA5A50000u + static_cast<std::uint32_t>(k);
    for (int i = 0; i < count && i < max_entries; ++i) {
        const std::uint32_t c = rng() % 8 == 0 ? 30 + rng() % 4 : rng() % 6;
        r[i * (0x504 / 4)] = c;
    }
    return r;
}

std::vector<std::uint32_t> seed_mask(std::mt19937& rng, int count)
{
    std::vector<std::uint32_t> m(count);
    for (auto& w : m) w = rng() % 5 == 0 ? 0 : (rng() % 3 == 0 ? 1 + rng() % 7 : 1);
    return m;
}
}  // namespace

TEST(native_collision_point_in_polygon_xz_height_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(float*, const float*, float, float, int);
    const Fn fn[2] = {rt::original<Fn>(0x004856d0), reinterpret_cast<Fn>(&recoil::Collision_PointInPolygonXZ_Height)};
    std::mt19937 rng(0x4856d0);
    int compared = 0, hits = 0;
    for (int it = 0; it < 20000; ++it) {
        const Poly p = make_poly(rng, rng() % 2 != 0);
        double x, y, z;
        plane_point(rng, p, x, y, z);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::wipe_stack();
            float rec[8];
            for (float& f : rec) f = -777.0f;
            const int ret = fn[side](rec, p.v, static_cast<float>(x), static_cast<float>(z), p.n);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            for (float f : rec) snap[side].push_back(ch::fbits(f));
            if (side == 0) hits += ret == 1;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Collision_PointInPolygonXZ_Height calls %d, %d hits\n", compared, hits);
    CHECK(hits > 1500);
}

TEST(native_collision_point_set_vs_polygon_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t, std::uint32_t*, const float*, std::uint32_t*, int, float, const float*, const std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x00484b70), reinterpret_cast<Fn>(&recoil::Collision_PointSetVsPolygon)};
    std::mt19937 rng(0x484b70);
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    int compared = 0, recorded = 0;
    for (int it = 0; it < 10000; ++it) {
        const Poly p = make_poly(rng, rng() % 4 != 0);
        const int count = 1 + static_cast<int>(rng() % 12);
        std::vector<float> pts(3 * count);
        for (int i = 0; i < count; ++i) {
            double x, y, z;
            plane_point(rng, p, x, y, z);
            pts[3 * i] = static_cast<float>(x);
            pts[3 * i + 1] = static_cast<float>(y + 5 * u(rng));
            pts[3 * i + 2] = static_cast<float>(z);
        }
        const float ceiling = static_cast<float>(p.oy + 8 * u(rng));
        std::uint32_t rec[8];
        for (auto& w : rec) w = rng();
        rec[0] = (rec[0] & ~0xffu) | static_cast<std::uint32_t>(p.n);
        const std::uint32_t node = 0x10000000u | (rng() & 0xfffff0u);
        const std::vector<std::uint32_t> results0 = seed_results(rng, count), mask0 = seed_mask(rng, count);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::wipe_stack();
            std::vector<std::uint32_t> results = results0, mask = mask0;
            const int ret = fn[side](node, results.data(), pts.data(), mask.data(), count, ceiling, p.v, rec);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            snap[side].insert(snap[side].end(), results.begin(), results.end());
            snap[side].insert(snap[side].end(), mask.begin(), mask.end());
            if (side == 0)
                for (int i = 0; i < count; ++i) recorded += results[i * (0x504 / 4)] != results0[i * (0x504 / 4)];
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Collision_PointSetVsPolygon calls %d, %d points recorded\n", compared, recorded);
    CHECK(recorded > 1000);
}

TEST(native_collision_segment_set_vs_polygon_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t, std::uint32_t*, const float*, std::uint32_t*, int, const float*, const std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x00486290), reinterpret_cast<Fn>(&recoil::Collision_SegmentSetVsPolygon)};
    std::mt19937 rng(0x486290);
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    int compared = 0, recorded = 0;
    for (int it = 0; it < 10000; ++it) {
        const Poly p = make_poly(rng, false);
        const int count = 1 + static_cast<int>(rng() % 12);
        std::vector<float> seg(6 * count);
        for (int i = 0; i < count; ++i) {
            double x, y, z;
            plane_point(rng, p, x, y, z);
            const int kind = static_cast<int>(rng() % 5);
            double d0 = 0.1 + 5 * (u(rng) + 1), d1 = -(0.1 + 5 * (u(rng) + 1));
            if (kind == 2) d1 = -d1;                   // both on one side
            if (kind == 3) { d0 = 0; d1 = 0; }         // in the plane
            if (kind == 4 && rng() % 2) d1 = 0;        // ends on the plane
            if (rng() % 2) { const double t = d0; d0 = d1; d1 = t; }
            const double sx = 3 * u(rng), sy = 3 * u(rng);
            seg[6 * i] = static_cast<float>(x + d0 * p.nx + sx * p.ax);
            seg[6 * i + 1] = static_cast<float>(y + d0 * p.ny + sx * p.ay);
            seg[6 * i + 2] = static_cast<float>(z + d0 * p.nz + sx * p.az);
            seg[6 * i + 3] = static_cast<float>(x + d1 * p.nx - sy * p.ax);
            seg[6 * i + 4] = static_cast<float>(y + d1 * p.ny - sy * p.ay);
            seg[6 * i + 5] = static_cast<float>(z + d1 * p.nz - sy * p.az);
        }
        std::uint32_t rec[8];
        for (auto& w : rec) w = rng();
        rec[0] = (rec[0] & ~0x1ffu) | static_cast<std::uint32_t>(p.n) | (rng() % 2 ? 0x100u : 0u);
        const std::uint32_t node = 0x10000000u | (rng() & 0xfffff0u);
        const std::vector<std::uint32_t> results0 = seed_results(rng, count), mask0 = seed_mask(rng, count);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::wipe_stack();
            std::vector<std::uint32_t> results = results0, mask = mask0;
            const int ret = fn[side](node, results.data(), seg.data(), mask.data(), count, p.v, rec);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            snap[side].insert(snap[side].end(), results.begin(), results.end());
            snap[side].insert(snap[side].end(), mask.begin(), mask.end());
            if (side == 0)
                for (int i = 0; i < count; ++i) recorded += results[i * (0x504 / 4)] != results0[i * (0x504 / 4)];
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Collision_SegmentSetVsPolygon calls %d, %d segments recorded\n", compared, recorded);
    CHECK(recorded > 1000);
}

// Collision_SegmentSetVsPolygonWithUV (0x004869a0): as the plain segment set, plus per-vertex UVs and an out UV (stack
// args 5 and 6, ret 0x1C); the UV work is gated on [0x0057d9a0] (seeded 0 or 1 on both sides) and each recorded hit's
// UV goes through Collision_StoreLastHitUV ([0x0057d9b4] / [0x0057d9b8], compared).
TEST(native_collision_segment_set_vs_polygon_with_uv_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t, std::uint32_t*, const float*, std::uint32_t*, int, const float*, const float*, float*, const std::uint32_t*);
    const Fn fn[2] = {rt::original<Fn>(0x004869a0), reinterpret_cast<Fn>(&recoil::Collision_SegmentSetVsPolygonWithUV)};
    std::mt19937 rng(0x4869a0);
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    int compared = 0, recorded = 0;
    for (int it = 0; it < 10000; ++it) {
        const Poly p = make_poly(rng, false);
        const int count = 1 + static_cast<int>(rng() % 12);
        std::vector<float> seg(6 * count);
        for (int i = 0; i < count; ++i) {
            double x, y, z;
            plane_point(rng, p, x, y, z);
            const int kind = static_cast<int>(rng() % 5);
            double d0 = 0.1 + 5 * (u(rng) + 1), d1 = -(0.1 + 5 * (u(rng) + 1));
            if (kind == 2) d1 = -d1;
            if (kind == 3) { d0 = 0; d1 = 0; }
            if (kind == 4 && rng() % 2) d1 = 0;
            if (rng() % 2) { const double t = d0; d0 = d1; d1 = t; }
            const double sx = 3 * u(rng), sy = 3 * u(rng);
            seg[6 * i] = static_cast<float>(x + d0 * p.nx + sx * p.ax);
            seg[6 * i + 1] = static_cast<float>(y + d0 * p.ny + sx * p.ay);
            seg[6 * i + 2] = static_cast<float>(z + d0 * p.nz + sx * p.az);
            seg[6 * i + 3] = static_cast<float>(x + d1 * p.nx - sy * p.ax);
            seg[6 * i + 4] = static_cast<float>(y + d1 * p.ny - sy * p.ay);
            seg[6 * i + 5] = static_cast<float>(z + d1 * p.nz - sy * p.az);
        }
        float uvs[36]; // 6 vertices * 6 floats/vertex (u,v,w, s,t,r or similar stride)
        for (float& f : uvs) f = static_cast<float>(4 * u(rng));
        std::uint32_t rec[8];
        for (auto& w : rec) w = rng();
        rec[0] = (rec[0] & ~0x1ffu) | static_cast<std::uint32_t>(p.n) | (rng() % 2 ? 0x100u : 0u);
        const std::uint32_t node = 0x10000000u | (rng() & 0xfffff0u), uv_on = rng() % 4 != 0;
        const std::vector<std::uint32_t> results0 = seed_results(rng, count), mask0 = seed_mask(rng, count);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x0057d9a0) = uv_on;
            ch::wipe_stack();
            std::vector<std::uint32_t> results = results0, mask = mask0;
            float out_uv[3] = {-777.0f, -777.0f, -777.0f};
            const int ret = fn[side](node, results.data(), seg.data(), mask.data(), count, p.v, uvs, out_uv, rec);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            snap[side].insert(snap[side].end(), results.begin(), results.end());
            snap[side].insert(snap[side].end(), mask.begin(), mask.end());
            for (float f : out_uv) snap[side].push_back(ch::fbits(f));
            snap[side].push_back(*ch::img(side, 0x0057d9b4));
            snap[side].push_back(*ch::img(side, 0x0057d9b8));
            if (side == 0)
                for (int i = 0; i < count; ++i) recorded += results[i * (0x504 / 4)] != results0[i * (0x504 / 4)];
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Collision_SegmentSetVsPolygonWithUV calls %d, %d segments recorded\n", compared, recorded);
    CHECK(recorded > 1000);
}

// Collision_SegmentSetVsTransformedBox (0x00487540): ECX node, EDX results, stack endpoints, mask, count, 8 box corners
// (24 floats); ret 0x10. Six quads through the static face buffer 0x0057c2c4 into Collision_SegmentSetVsPolygon with a
// stack record (vertex count 4, one-sided). Boxes: bottom ring then top ring, rotated and placed at random (some with
// the rings swapped, so the faces wind the other way); segments from outside towards the box, through it, beside it.
// Compared: the return, every result word, the mask and the 12 words of the face buffer.
TEST(native_collision_segment_set_vs_transformed_box_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t, std::uint32_t*, const float*, std::uint32_t*, int, const float*);
    const Fn fn[2] = {rt::original<Fn>(0x00487540), reinterpret_cast<Fn>(&recoil::Collision_SegmentSetVsTransformedBox)};
    std::mt19937 rng(0x487540);
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    int compared = 0, recorded = 0;
    for (int it = 0; it < 10000; ++it) {
        // rotation from a random unit quaternion
        double qw = u(rng), qx = u(rng), qy = u(rng), qz = u(rng);
        const double ql = std::sqrt(qw * qw + qx * qx + qy * qy + qz * qz) + 1e-12;
        qw /= ql; qx /= ql; qy /= ql; qz /= ql;
        if (rng() % 3 == 0) { qw = 1; qx = qy = qz = 0; }
        const double m[9] = {1 - 2 * (qy * qy + qz * qz), 2 * (qx * qy - qz * qw), 2 * (qx * qz + qy * qw),
                             2 * (qx * qy + qz * qw), 1 - 2 * (qx * qx + qz * qz), 2 * (qy * qz - qx * qw),
                             2 * (qx * qz - qy * qw), 2 * (qy * qz + qx * qw), 1 - 2 * (qx * qx + qy * qy)};
        const double cx = 20 * u(rng), cy = 20 * u(rng), cz = 20 * u(rng);
        const double hx = 0.5 + 3 * (u(rng) + 1), hy = 0.5 + 3 * (u(rng) + 1), hz = 0.5 + 3 * (u(rng) + 1);
        const bool swap = rng() % 4 == 0;
        float corners[24];
        static const int ring[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
        for (int k = 0; k < 8; ++k) {
            const double lx = hx * ring[k % 4][0], ly = hy * ((k < 4) != swap ? -1 : 1), lz = hz * ring[k % 4][1];
            corners[3 * k] = static_cast<float>(cx + m[0] * lx + m[1] * ly + m[2] * lz);
            corners[3 * k + 1] = static_cast<float>(cy + m[3] * lx + m[4] * ly + m[5] * lz);
            corners[3 * k + 2] = static_cast<float>(cz + m[6] * lx + m[7] * ly + m[8] * lz);
        }
        const int count = 1 + static_cast<int>(rng() % 12);
        std::vector<float> seg(6 * count);
        for (int i = 0; i < count; ++i) {
            const double r = 2 + 10 * (u(rng) + 1);
            double dx = u(rng), dy = u(rng), dz = u(rng);
            const double dl = std::sqrt(dx * dx + dy * dy + dz * dz) + 1e-12;
            dx /= dl; dy /= dl; dz /= dl;
            const double ox = (rng() % 3 == 0 ? 2 : 0.6) * hx * u(rng), oy = 0.6 * hy * u(rng), oz = 0.6 * hz * u(rng);
            const double tail = rng() % 3 == 0 ? r : -(0.2 + r * (u(rng) + 1) / 2);  // through, into or short of the box
            seg[6 * i] = static_cast<float>(cx + ox + r * dx);
            seg[6 * i + 1] = static_cast<float>(cy + oy + r * dy);
            seg[6 * i + 2] = static_cast<float>(cz + oz + r * dz);
            seg[6 * i + 3] = static_cast<float>(cx + ox - tail * dx);
            seg[6 * i + 4] = static_cast<float>(cy + oy - tail * dy);
            seg[6 * i + 5] = static_cast<float>(cz + oz - tail * dz);
        }
        const std::uint32_t node = 0x10000000u | (rng() & 0xfffff0u);
        const std::vector<std::uint32_t> results0 = seed_results(rng, count), mask0 = seed_mask(rng, count);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            ch::wipe_stack();
            std::vector<std::uint32_t> results = results0, mask = mask0;
            const int ret = fn[side](node, results.data(), seg.data(), mask.data(), count, corners);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            snap[side].insert(snap[side].end(), results.begin(), results.end());
            snap[side].insert(snap[side].end(), mask.begin(), mask.end());
            for (std::uint32_t k = 0; k < 12; ++k) snap[side].push_back(*ch::img(side, 0x0057c2c4 + 4 * k));
            if (side == 0)
                for (int i = 0; i < count; ++i) recorded += results[i * (0x504 / 4)] != results0[i * (0x504 / 4)];
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Collision_SegmentSetVsTransformedBox calls %d, %d segments recorded\n", compared, recorded);
    CHECK(recorded > 1000);
}
