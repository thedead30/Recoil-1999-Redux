// Structured native L1 for Collision_RayPolygonTestWithUV (0x00485d10; ported in the cloud as a zclass_nodes blocker,
// P2.6): ECX hit record (normal +0..+8, hit point +0xC..+0x14), EDX segment end, stack: segment start, polygon vertices
// (x, y, z each), per-vertex UVs (u, v each), out UV, vertex count, two-sided flag; ret 0x18. Normal from the first three
// vertices (Tri_Normal), plane distances of both ends, one-sided cull (start on or in front -> 0 unless two-sided), no
// straddle -> 0, hit point on the plane, dominant-axis inside test with a -0.0001 tolerance; on a hit the planar UV
// gradients are solved from the first three vertices (Math_Solve2x2, once for U and once for V), the hit's UV goes to
// the out pointer and to Collision_StoreLastHitUV ([0x0057d9b4] / [0x0057d9b8]); 1 for any geometric hit.
// Real geometry: convex polygons of 3..6 vertices (regular, with a random radius and rotation, some jittered) placed on a
// plane of random orientation (axis-aligned ones included, so each dominant axis occurs), random UVs; segments that
// cross the plane inside the polygon, outside it, near its edge, stay on one side, lie in the plane or end on it.
// Compared: the return, the hit record (8 words), the out UV and the two globals.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/collision.h"
#include "platform/image/original_data.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

namespace {
std::uint32_t* img(int side, std::uint32_t va)
{
    return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(va)));
}
std::uint32_t fbits(float f) { std::uint32_t u; std::memcpy(&u, &f, 4); return u; }
}  // namespace

TEST(native_collision_ray_polygon_test_with_uv_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(float*, const float*, const float*, const float*, const float*, float*, int, int);
    const Fn fn[2] = {rt::original<Fn>(0x00485d10), reinterpret_cast<Fn>(&recoil::Collision_RayPolygonTestWithUV)};
    std::mt19937 rng(0x485d10);
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    int compared = 0, hits = 0;
    for (int it = 0; it < 20000; ++it) {
        const int n = 3 + static_cast<int>(rng() % 4);
        // plane basis: a random or axis-aligned normal, two tangents
        double nx, ny, nz;
        if (rng() % 3 == 0) { const int a = static_cast<int>(rng() % 3); nx = a == 0; ny = a == 1; nz = a == 2; if (rng() % 2) { nx = -nx; ny = -ny; nz = -nz; } }
        else { nx = u(rng); ny = u(rng); nz = u(rng); }
        const double nl = std::sqrt(nx * nx + ny * ny + nz * nz) + 1e-12;
        nx /= nl; ny /= nl; nz /= nl;
        double tx = std::fabs(nx) < 0.9 ? 1 : 0, ty = std::fabs(nx) < 0.9 ? 0 : 1, tz = 0;
        double ax = ny * tz - nz * ty, ay = nz * tx - nx * tz, az = nx * ty - ny * tx;  // a = n x t
        const double al = std::sqrt(ax * ax + ay * ay + az * az);
        ax /= al; ay /= al; az /= al;
        const double bx = ny * az - nz * ay, by = nz * ax - nx * az, bz = nx * ay - ny * ax;  // b = n x a
        const double ox = 20 * u(rng), oy = 20 * u(rng), oz = 20 * u(rng), r = 0.5 + 5 * (u(rng) + 1), rot = 3.14159 * u(rng);
        const bool reverse = rng() % 2 != 0;  // winding (which side is the front)
        float verts[18], uvs[12];
        for (int k = 0; k < n; ++k) {
            const int kk = reverse ? n - 1 - k : k;
            const double ang = rot + 2 * 3.14159265358979 * kk / n, jr = rng() % 4 == 0 ? r * (0.8 + 0.2 * (u(rng) + 1) / 2) : r;
            const double px = jr * std::cos(ang), py = jr * std::sin(ang);
            verts[3 * k] = static_cast<float>(ox + px * ax + py * bx);
            verts[3 * k + 1] = static_cast<float>(oy + px * ay + py * by);
            verts[3 * k + 2] = static_cast<float>(oz + px * az + py * bz);
            uvs[2 * k] = static_cast<float>(u(rng) * 4);
            uvs[2 * k + 1] = static_cast<float>(u(rng) * 4);
        }
        // the segment: a point of the plane (inside, near the edge or outside), offset along the normal both ways
        const int kind = static_cast<int>(rng() % 6);
        const double s = kind == 0 ? 0.3 * r * (u(rng) + 1) / 2 : kind == 1 ? r * (0.9 + 0.2 * (u(rng) + 1) / 2) : 2 * r * (1 + (u(rng) + 1));
        const double ang = 3.14159 * u(rng), qx = s * std::cos(ang), qy = s * std::sin(ang);
        const double hx = ox + qx * ax + qy * bx, hy = oy + qx * ay + qy * by, hz = oz + qx * az + qy * bz;
        double d0 = 0.1 + 5 * (u(rng) + 1), d1 = -(0.1 + 5 * (u(rng) + 1));
        if (kind == 3) d1 = -d1;                      // both on one side
        if (kind == 4) { d0 = 0; d1 = 0; }             // in the plane
        if (kind == 5 && rng() % 2) d1 = 0;           // ends on the plane
        if (rng() % 2) { const double t = d0; d0 = d1; d1 = t; }
        const double sx = 3 * u(rng), sy = 3 * u(rng);  // a slant
        const float start[3] = {static_cast<float>(hx + d0 * nx + sx * ax), static_cast<float>(hy + d0 * ny + sx * ay), static_cast<float>(hz + d0 * nz + sx * az)};
        const float end[3] = {static_cast<float>(hx + d1 * nx - sy * ax), static_cast<float>(hy + d1 * ny - sy * ay), static_cast<float>(hz + d1 * nz - sy * az)};
        const int two_sided = static_cast<int>(rng() % 2);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            float hit[8], out_uv[3];
            for (float& h : hit) h = -777.0f;
            for (float& h : out_uv) h = -777.0f;
            const int ret = fn[side](hit, end, start, verts, uvs, out_uv, n, two_sided);
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            for (float h : hit) snap[side].push_back(fbits(h));
            for (float h : out_uv) snap[side].push_back(fbits(h));
            snap[side].push_back(*img(side, 0x0057d9b4));
            snap[side].push_back(*img(side, 0x0057d9b8));
            if (side == 0) hits += ret == 1;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Collision_RayPolygonTestWithUV calls %d, %d hits\n", compared, hits);
    CHECK(hits > 1500);
}
