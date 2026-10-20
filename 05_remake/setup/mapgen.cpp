// mapgen.cpp - Setup's RX-47 mission maps (see mapgen.h): the C++ form of redux_assets/tools/map_geo.py and
// map_render.py --ingame, so a player's Setup makes them from the player's own copy with nothing installed.
//
// Sources, all in the installed game folder:
//   zbd\m<N>\gamez.zbd   the world: textures, materials, models, nodes (layout from the game's own loaders, as
//                        texture_roles.py: GameZ_LoadZbd 0x00455520, Material_ReadSection 0x004808c0,
//                        Model_ReadContents 0x00481c50, GameZ_ReadNodeBuffer 0x00455350; object3d class data
//                        Object3D_Create 0x0044daa0: local matrix +0x30 (Object3D_GetLocalMatrix 0x0044e5b0), position +0x54
//                        (Object3D_GetPosition 0x0044e270))
//   zbd\m<N>\zrdr.zbd    config archive (Zar trailer) of .zrd trees (asset_io.md: node types 1 int, 2 float, 3 string,
//                        4 list): objectives.zrd, location.zrd, puppies.zrd (Pickups_ConfigFileForDifficulty 0x0041ddf0)
//   maps\m<N>.zmap       the original map outlines (MapScreen_ReadFile 0x004168d0: version 5, then colour, point list,
//                        state per marker)
//   messages.dll         objective titles (message table; MSG_M<m>_O<k>_SUMMARY = 2560 + (m-1)*256 + (k-1)*3,
//                        MEASURED-ASSET from 02_extracted/messages/messages.json)
//   zbd\m<N>\texture6/4/2.zbd, rtexture4/2.zbd   the pictures (zbdimg.h)
// Picture orientation is the game's own (MapScreen_WorldToMap 0x00416c90 with heading (1, 0)): world +x up, +z right.
// Not Recoil code (PLATFORM).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "mapgen.h"
#include "zbdimg.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <functional>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace setup {
namespace {

namespace fs = std::filesystem;
using Bytes = std::vector<std::uint8_t>;

std::uint32_t Rd32(const Bytes& b, std::size_t o) { return o + 4 <= b.size() ? b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (std::uint32_t(b[o + 3]) << 24) : 0; }
std::int32_t Ri32(const Bytes& b, std::size_t o) { return std::int32_t(Rd32(b, o)); }
std::int16_t Ri16(const Bytes& b, std::size_t o) { return o + 2 <= b.size() ? std::int16_t(b[o] | (b[o + 1] << 8)) : 0; }
float Rf(const Bytes& b, std::size_t o) { const std::uint32_t v = Rd32(b, o); float f; std::memcpy(&f, &v, 4); return f; }
std::string Cstr(const Bytes& b, std::size_t o, std::size_t n)
{
    std::string s;
    for (std::size_t i = o; i < o + n && i < b.size() && b[i]; ++i) s += char(b[i]);
    return s;
}
std::string Low(std::string s) { for (char& c : s) c = char(std::tolower(static_cast<unsigned char>(c))); return s; }

// ---------------------------------------------------------------- zrdr.zbd and .zrd trees
std::map<std::string, Bytes> ReadZar(const fs::path& p)
{
    std::map<std::string, Bytes> out;
    Bytes b;
    if (!ReadAll(p, b) || b.size() < 8) return out;
    const std::uint32_t cnt = Rd32(b, b.size() - 4);
    if (std::size_t(cnt) * 0x94 + 8 > b.size()) return out;
    const std::size_t toc = b.size() - 8 - std::size_t(cnt) * 0x94;
    for (std::uint32_t i = 0; i < cnt; ++i) {
        const std::size_t e = toc + std::size_t(i) * 0x94;
        const std::uint32_t off = Rd32(b, e), sz = Rd32(b, e + 4);
        if (std::size_t(off) + sz > b.size()) continue;
        out[Low(Cstr(b, e + 8, 0x8c))] = Bytes(b.begin() + off, b.begin() + off + sz);
    }
    return out;
}

struct Z {
    int type = 0;   // 1 int, 2 float, 3 string, 4 list
    double num = 0;
    std::string str;
    std::vector<Z> list;
};

Z ZNode(const Bytes& b, std::size_t& p, int depth = 0)
{
    Z z;
    if (p + 4 > b.size() || depth > 64) { p = b.size(); return z; }
    z.type = int(Rd32(b, p));
    p += 4;
    if (z.type == 1) { z.num = Ri32(b, p); p += 4; }
    else if (z.type == 2) { z.num = Rf(b, p); p += 4; }
    else if (z.type == 3) { const std::uint32_t n = Rd32(b, p); z.str = Cstr(b, p + 4, n); p += 4 + n; }
    else if (z.type == 4) {
        const std::uint32_t n = Rd32(b, p);
        p += 4;
        for (std::uint32_t i = 1; i < n && p < b.size(); ++i) z.list.push_back(ZNode(b, p, depth + 1));
    } else p = b.size();
    return z;
}

const Z* Tree(const std::map<std::string, Bytes>& zar, const char* name, Z& keep)
{
    const auto it = zar.find(name);
    if (it == zar.end()) return nullptr;
    std::size_t p = 0;
    keep = ZNode(it->second, p);
    return keep.type == 4 && !keep.list.empty() && keep.list[0].type == 4 ? &keep.list[0] : nullptr;
}

// ---------------------------------------------------------------- gamez.zbd
struct Poly { std::string tex; std::vector<std::array<double, 3>> w; std::vector<std::array<double, 2>> uv; };
struct Xform { double m[9]; double t[3]; };
struct World {
    std::vector<std::string> names;
    std::vector<std::uint32_t> cls;
    std::vector<int> model;
    std::vector<std::vector<int>> parents, children;
    std::map<int, Xform> xform;
    std::vector<std::vector<Poly>> model_geo;   // local space
};

bool ParseGamez(const fs::path& p, World& g)
{
    Bytes b;
    if (!ReadAll(p, b) || b.size() < 0x24) return false;
    const std::uint32_t tcount = Rd32(b, 8), tex_off = Rd32(b, 12), mat_off = Rd32(b, 16), mdl_off = Rd32(b, 20),
                        ncount = Rd32(b, 24), node_off = Rd32(b, 32);
    std::vector<std::string> textures;
    for (std::uint32_t i = 0; i < tcount; ++i) textures.push_back(Cstr(b, tex_off + i * 0x24 + 8, 0x1c));
    const std::int32_t size = Ri32(b, mat_off), head = Ri32(b, mat_off + 12);
    if (size < 0 || size > 100000) return false;
    std::size_t q = mat_off + 16;
    const std::size_t mats = q;
    q += std::size_t(size) * 0x2C;
    std::map<int, int> mat_tex;
    for (int i = head, guard = 0; i >= 0 && i < size && guard < size + 1; ++guard) {
        const std::size_t m = mats + std::size_t(i) * 0x2C;
        if (b[m + 1] & 1) mat_tex[i] = Ri32(b, m + 0x10);
        if (b[m + 1] & 4) {
            const std::int32_t n = Ri32(b, q + 0x10);
            if (n > 0 && !mat_tex.count(i)) mat_tex[i] = Ri32(b, q + 0x1C);
            q += 0x1C + 4 * std::size_t(std::max(n, 0));
        }
        i = Ri16(b, m + 0x2A);
    }
    if (q != mdl_off) return false;
    const std::uint32_t mcount = Rd32(b, mdl_off);
    q = mdl_off + 12;
    const std::size_t models = q;
    q += std::size_t(mcount) * 0x58;
    g.model_geo.resize(mcount);
    for (std::uint32_t k = 0; k < mcount; ++k) {
        const std::size_t m = models + std::size_t(k) * 0x58;
        const std::int32_t polys = Ri32(b, m + 0x0C), verts = Ri32(b, m + 0x10), norms = Ri32(b, m + 0x14), morph = Ri32(b, m + 0x18),
                           points = Ri32(b, m + 0x1C);
        const std::size_t v = q;
        q += 12 * std::size_t(std::max(verts, 0)) + 12 * std::size_t(std::max(norms, 0)) + 12 * std::size_t(std::max(morph, 0));
        std::vector<std::int32_t> prec;
        for (std::int32_t i = 0; i < points; ++i) prec.push_back(Ri32(b, q + std::size_t(i) * 0x4C + 0xC));
        q += 0x4C * std::size_t(std::max(points, 0));
        for (std::int32_t c : prec) q += 12 * std::size_t(std::max(c, 0));
        const std::size_t recs = q;
        q += 0x1C * std::size_t(std::max(polys, 0));
        for (std::int32_t r = 0; r < polys; ++r) {
            const std::size_t rr = recs + std::size_t(r) * 0x1C;
            const std::uint32_t d0 = Rd32(b, rr);
            const std::int32_t mi = Ri32(b, rr + 0x14);
            const int vc = int(d0 & 0xFF);
            const std::size_t idx = q;
            q += 4 * std::size_t(vc);
            if (d0 & 0x200) q += 4 * std::size_t(vc);
            const bool textured = mi >= 0 && mi < size && (b[mats + std::size_t(mi) * 0x2C + 1] & 1);
            const std::size_t uvs = q;
            if (textured) q += 8 * std::size_t(vc);
            if (q > b.size()) return false;
            const auto t = mat_tex.find(mi);
            if (!textured || t == mat_tex.end() || t->second < 0 || std::uint32_t(t->second) >= tcount || vc < 3) continue;
            Poly pl;
            pl.tex = Low(textures[t->second]);
            bool ok = true;
            for (int i = 0; i < vc; ++i) {
                const std::int32_t ix = Ri32(b, idx + 4 * std::size_t(i));
                if (ix < 0 || ix >= verts) { ok = false; break; }
                pl.w.push_back({Rf(b, v + 12 * std::size_t(ix)), Rf(b, v + 12 * std::size_t(ix) + 4), Rf(b, v + 12 * std::size_t(ix) + 8)});
                pl.uv.push_back({Rf(b, uvs + 8 * std::size_t(i)), Rf(b, uvs + 8 * std::size_t(i) + 4)});
            }
            if (ok) g.model_geo[k].push_back(std::move(pl));
        }
    }
    if (q != node_off) return false;
    g.names.resize(ncount);
    g.cls.resize(ncount);
    g.model.resize(ncount);
    g.parents.resize(ncount);
    g.children.resize(ncount);
    q = node_off + std::size_t(ncount) * 0xC4;
    for (std::uint32_t k = 0; k < ncount; ++k) {
        const std::size_t n = node_off + std::size_t(k) * 0xC4;
        g.names[k] = Cstr(b, n, 0x20);
        const std::uint32_t c = Rd32(b, n + 0x34);
        g.cls[k] = c;
        g.model[k] = Ri32(b, n + 0x3C);
        if (c == 5) {
            Xform x;
            for (int i = 0; i < 9; ++i) x.m[i] = Rf(b, q + 0x30 + 4 * i);
            for (int i = 0; i < 3; ++i) x.t[i] = Rf(b, q + 0x54 + 4 * i);
            g.xform[int(k)] = x;
        }
        switch (c) {   // class data sizes (GameZ_ReadNodeData 0x00454c60)
        case 1: q += 0x1E8; break;
        case 3: q += 0xF8; break;
        case 4: q += 0x1C; break;
        case 5: q += 0x90; break;
        case 6: q += 0x50; break;
        case 2: {
            const std::size_t w = q;
            q += 0xAC;
            const std::int32_t n90 = Ri32(b, w + 0x90), n9c = Ri32(b, w + 0x9C), n78 = Ri32(b, w + 0x78), n7c = Ri32(b, w + 0x7C);
            q += 4 * std::size_t(std::max(n90, 0)) + 4 * std::size_t(std::max(n9c, 0));
            for (std::int64_t i = 0; i < std::int64_t(std::max(n7c, 0)) * std::max(n78, 0) && q < b.size(); ++i)
                q += 0x40 + 4 * std::size_t(std::max<int>(Ri16(b, q + 0x3A), 0));
            break;
        }
        case 9: q += 0xE4 + 4 * std::size_t(std::max(Ri32(b, q + 0xDC), 0)); break;
        case 10: q += 0x94 + 4 * std::size_t(std::max(Ri32(b, q + 0x8C), 0)); break;
        default: break;
        }
        const std::int32_t np = Ri32(b, n + 0x54), nc = Ri32(b, n + 0x5C);
        for (std::int32_t i = 0; i < np; ++i) g.parents[k].push_back(Ri32(b, q + 4 * std::size_t(i)));
        q += 4 * std::size_t(std::max(np, 0));
        for (std::int32_t i = 0; i < nc; ++i) g.children[k].push_back(Ri32(b, q + 4 * std::size_t(i)));
        q += 4 * std::size_t(std::max(nc, 0));
        if (q > b.size()) return false;
    }
    return true;
}

std::array<double, 3> Apply(const Xform& x, const std::array<double, 3>& v)
{
    return {v[0] * x.m[0] + v[1] * x.m[3] + v[2] * x.m[6] + x.t[0], v[0] * x.m[1] + v[1] * x.m[4] + v[2] * x.m[7] + x.t[1],
            v[0] * x.m[2] + v[1] * x.m[5] + v[2] * x.m[8] + x.t[2]};
}

// the node's transforms from itself up to (not including) the world node
std::vector<const Xform*> Chain(const World& g, int k)
{
    std::vector<const Xform*> c;
    for (int j = k, guard = 0; guard < 64; ++guard) {
        const auto x = g.xform.find(j);
        if (x != g.xform.end()) c.push_back(&x->second);
        if (g.parents[j].empty()) break;
        const int p = g.parents[j][0];
        if (p < 0 || p >= int(g.names.size()) || g.cls[p] == 2) break;
        j = p;
    }
    return c;
}

bool Skip(const std::string& name)
{
    const std::string l = Low(name);
    for (const char* w : {"shad", "sky", "cloud", "horizon"})
        if (l.find(w) != std::string::npos) return true;
    return false;
}

// every model in world space (lower LOD levels left out), as map_geo.world_polygons
std::vector<Poly> WorldPolys(const World& g)
{
    std::vector<Poly> out;
    const int n = int(g.names.size());
    for (int k = 0; k < n; ++k) {
        const int mi = g.model[k];
        if (g.cls[k] != 5 || mi < 0 || mi >= int(g.model_geo.size()) || g.model_geo[mi].empty()) continue;
        bool ok = !Skip(g.names[k]);
        for (int j = k, guard = 0; ok && guard < 64; ++guard) {
            if (g.parents[j].empty()) break;
            const int p = g.parents[j][0];
            if (p < 0 || p >= n) break;
            if (g.cls[p] == 6 && (g.children[p].empty() || g.children[p][0] != j)) ok = false;
            if (g.cls[p] == 2) break;
            if (Skip(g.names[p])) ok = false;
            j = p;
        }
        if (!ok) continue;
        const auto ch = Chain(g, k);
        for (const Poly& pl : g.model_geo[mi]) {
            Poly w = pl;
            for (auto& v : w.w)
                for (const Xform* x : ch) v = Apply(*x, v);
            out.push_back(std::move(w));
        }
    }
    return out;
}

bool NodeCentre(const World& g, int k, double& x, double& z)
{
    double x0 = 1e30, x1 = -1e30, z0 = 1e30, z1 = -1e30;
    std::vector<int> stack{k};
    std::vector<char> seen(g.names.size(), 0);
    int visited = 0;
    while (!stack.empty() && visited < 4000) {
        const int j = stack.back();
        stack.pop_back();
        if (j < 0 || j >= int(g.names.size()) || seen[j]) continue;
        seen[j] = 1;
        ++visited;
        const int mi = g.model[j];
        if (g.cls[j] == 5 && mi >= 0 && mi < int(g.model_geo.size())) {
            const auto ch = Chain(g, j);
            for (const Poly& pl : g.model_geo[mi])
                for (auto v : pl.w) {
                    for (const Xform* t : ch) v = Apply(*t, v);
                    x0 = std::min(x0, v[0]); x1 = std::max(x1, v[0]);
                    z0 = std::min(z0, v[2]); z1 = std::max(z1, v[2]);
                }
        }
        for (int c : g.children[j]) stack.push_back(c);
    }
    if (x0 > x1) {
        std::array<double, 3> v{0, 0, 0};
        for (const Xform* t : Chain(g, k)) v = Apply(*t, v);
        x = v[0];
        z = v[2];
    } else {
        x = (x0 + x1) / 2;
        z = (z0 + z1) / 2;
    }
    return std::fabs(x) + std::fabs(z) > 1;
}

int FindNode(const World& g, const std::string& name)
{
    const std::string l = Low(name);
    for (std::size_t k = 0; k < g.names.size(); ++k)
        if (Low(g.names[k]) == l) return int(k);
    return -1;
}

// ---------------------------------------------------------------- textures
struct Tex { int w = 0, h = 0; const std::uint8_t* px = nullptr; };

struct TexSet {
    std::vector<std::map<std::string, Entry>> arch;
    std::map<std::string, Tex> cache;
    Tex Get(const std::string& name)
    {
        const auto c = cache.find(name);
        if (c != cache.end()) return c->second;
        Tex t;
        for (auto& a : arch) {
            const auto e = a.find(name);
            if (e != a.end() && e->second.colour.w > 0) {
                t = {e->second.colour.w, e->second.colour.h, e->second.colour.bgra.data()};
                break;
            }
        }
        return cache[name] = t;
    }
};

void Sample(const Tex& t, double u, double v, double& r, double& gr, double& bl)
{
    u = u - std::floor(u);
    v = v - std::floor(v);
    const double fx = u * t.w - 0.5, fy = v * t.h - 0.5;
    const int x0 = int(std::floor(fx)), y0 = int(std::floor(fy));
    const double ax = fx - x0, ay = fy - y0;
    r = gr = bl = 0;
    for (int dy = 0; dy < 2; ++dy)
        for (int dx = 0; dx < 2; ++dx) {
            const int xx = ((x0 + dx) % t.w + t.w) % t.w, yy = ((y0 + dy) % t.h + t.h) % t.h;
            const double wgt = (dx ? ax : 1 - ax) * (dy ? ay : 1 - ay);
            const std::uint8_t* p = t.px + (std::size_t(yy) * t.w + xx) * 4;
            bl += p[0] * wgt;
            gr += p[1] * wgt;
            r += p[2] * wgt;
        }
}

// ---------------------------------------------------------------- drawing
struct Canvas {
    int w = 0, h = 0;
    std::vector<std::uint8_t> px;   // BGRA
    std::uint8_t* At(int x, int y) { return &px[(std::size_t(y) * w + x) * 4]; }
};

void FillTriangle(Canvas& c, const double (*p)[2], const double (*uv)[2], const Tex& t, double shade)
{
    const double x0 = std::min({p[0][0], p[1][0], p[2][0]}), x1 = std::max({p[0][0], p[1][0], p[2][0]});
    const double y0 = std::min({p[0][1], p[1][1], p[2][1]}), y1 = std::max({p[0][1], p[1][1], p[2][1]});
    const int ix0 = std::max(0, int(std::floor(x0))), ix1 = std::min(c.w - 1, int(std::ceil(x1)));
    const int iy0 = std::max(0, int(std::floor(y0))), iy1 = std::min(c.h - 1, int(std::ceil(y1)));
    const double det = (p[1][0] - p[0][0]) * (p[2][1] - p[0][1]) - (p[2][0] - p[0][0]) * (p[1][1] - p[0][1]);
    if (std::fabs(det) < 1e-9) return;
    for (int y = iy0; y <= iy1; ++y)
        for (int x = ix0; x <= ix1; ++x) {
            const double px = x + 0.5, py = y + 0.5;
            const double b1 = ((px - p[0][0]) * (p[2][1] - p[0][1]) - (p[2][0] - p[0][0]) * (py - p[0][1])) / det;
            const double b2 = ((p[1][0] - p[0][0]) * (py - p[0][1]) - (px - p[0][0]) * (p[1][1] - p[0][1])) / det;
            const double b0 = 1 - b1 - b2;
            if (b0 < -1e-6 || b1 < -1e-6 || b2 < -1e-6) continue;
            double r, gr, bl;
            Sample(t, b0 * uv[0][0] + b1 * uv[1][0] + b2 * uv[2][0], b0 * uv[0][1] + b1 * uv[1][1] + b2 * uv[2][1], r, gr, bl);
            std::uint8_t* d = c.At(x, y);
            d[0] = std::uint8_t(std::min(255.0, bl * shade));
            d[1] = std::uint8_t(std::min(255.0, gr * shade));
            d[2] = std::uint8_t(std::min(255.0, r * shade));
        }
}

// a 2-pixel line blended at alpha/255 (each pixel once per line)
void Polyline(Canvas& c, const std::vector<std::array<double, 2>>& pts, std::uint8_t r, std::uint8_t gr, std::uint8_t bl, int alpha)
{
    std::vector<std::uint8_t> mask(std::size_t(c.w) * c.h, 0);
    for (std::size_t i = 1; i < pts.size(); ++i) {
        const double dx = pts[i][0] - pts[i - 1][0], dy = pts[i][1] - pts[i - 1][1];
        const int steps = std::max(1, int(std::sqrt(dx * dx + dy * dy) * 2));
        for (int s = 0; s <= steps; ++s) {
            const double x = pts[i - 1][0] + dx * s / steps, y = pts[i - 1][1] + dy * s / steps;
            for (int oy = 0; oy < 2; ++oy)
                for (int ox = 0; ox < 2; ++ox) {
                    const int xx = int(std::floor(x - 0.5)) + ox, yy = int(std::floor(y - 0.5)) + oy;
                    if (xx >= 0 && yy >= 0 && xx < c.w && yy < c.h) mask[std::size_t(yy) * c.w + xx] = 1;
                }
        }
    }
    for (std::size_t i = 0; i < mask.size(); ++i)
        if (mask[i]) {
            std::uint8_t* d = &c.px[i * 4];
            d[0] = std::uint8_t((bl * alpha + d[0] * (255 - alpha)) / 255);
            d[1] = std::uint8_t((gr * alpha + d[1] * (255 - alpha)) / 255);
            d[2] = std::uint8_t((r * alpha + d[2] * (255 - alpha)) / 255);
        }
}

std::wstring ObjectiveTitle(HMODULE msgs, int mission, int k, const std::string& key)
{
    int km = 0, kk = 0;
    std::wstring t;
    if (msgs && std::sscanf(key.c_str(), "MSG_M%d_O%d_SUMMARY", &km, &kk) == 2 && km == mission) {
        wchar_t* buf = nullptr;
        const DWORD id = DWORD(2560 + (km - 1) * 256 + (kk - 1) * 3);
        if (FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, msgs, id,
                           MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), reinterpret_cast<wchar_t*>(&buf), 0, nullptr) && buf) {
            t = buf;
            LocalFree(buf);
        }
    }
    while (!t.empty() && (t.back() == L'\n' || t.back() == L'\r' || t.back() == L' ')) t.pop_back();
    if (t.size() > 3 && t[0] == L'M') {   // "M1: Operation Short Circuit" -> "Operation Short Circuit"
        const std::size_t c = t.find(L": ");
        if (c != std::wstring::npos && c < 4) t = t.substr(c + 2);
    }
    return t.empty() ? L"Objective " + std::to_wstring(k) : t;
}

bool IsWeapon(const std::string& t) { return t.size() > 7 && t.compare(t.size() - 7, 7, "_WEAPON") == 0; }
bool IsUpgrade(const std::string& t) { return t == "PUP_HOVER" || t == "PUP_AMPHIB" || t == "PUP_SUB"; }

struct Objective { int n; std::wstring title; bool placed; double x, z; };

bool MakeMap(const fs::path& game, int m, const fs::path& out_dir, HMODULE msgs, std::string& why)
{
    const fs::path zbd = game / L"zbd" / (L"m" + std::to_wstring(m));
    Bytes zmap;
    if (!ReadAll(game / L"maps" / (L"m" + std::to_wstring(m) + L".zmap"), zmap) || zmap.size() < 0x20 || Rd32(zmap, 0) != 5) {
        why = "no map outlines";
        return false;
    }
    struct Outline { std::uint8_t r, g, b; std::vector<std::array<double, 3>> pts; };
    std::vector<Outline> outlines;
    for (std::size_t p = 0x20; p + 7 <= zmap.size();) {
        Outline o{zmap[p], zmap[p + 1], zmap[p + 2], {}};
        const std::uint32_t n = Rd32(zmap, p + 3);
        p += 7;
        if (p + std::size_t(n) * 12 + 4 > zmap.size()) break;
        for (std::uint32_t i = 0; i < n; ++i) o.pts.push_back({Rf(zmap, p + 12 * i), Rf(zmap, p + 12 * i + 4), Rf(zmap, p + 12 * i + 8)});
        p += std::size_t(n) * 12 + 4;
        outlines.push_back(std::move(o));
    }
    World g;
    if (!ParseGamez(zbd / L"gamez.zbd", g)) { why = "cannot read gamez.zbd"; return false; }
    const auto zar = ReadZar(zbd / L"zrdr.zbd");
    Z keep_p, keep_o, keep_l;
    const Z* pick = Tree(zar, "puppies.zrd", keep_p);
    const Z* objz = Tree(zar, "objectives.zrd", keep_o);
    const Z* loc = Tree(zar, "location.zrd", keep_l);

    // objectives: place = location.zrd tour stop "#n", else the centre of the ACTIVE / INACTIVE object
    std::map<int, std::array<double, 2>> tour;
    if (loc)
        for (std::size_t i = 0; i + 1 < loc->list.size(); i += 2) {
            const Z& nm = loc->list[i];
            const std::size_t h = nm.str.find('#');
            if (nm.type != 3 || h == std::string::npos) continue;
            const Z& v = loc->list[i + 1];
            if (v.type == 4 && !v.list.empty() && v.list[0].type == 4 && v.list[0].list.size() >= 3)
                tour[std::atoi(nm.str.c_str() + h + 1)] = {v.list[0].list[0].num, v.list[0].list[2].num};
        }
    std::vector<Objective> objs;
    if (objz)
        for (std::size_t i = 0; i + 1 < objz->list.size(); ++i) {
            const Z& k = objz->list[i];
            int n = 0;
            if (k.type != 3 || std::sscanf(k.str.c_str(), "OBJECTIVE%d", &n) != 1 || k.str.find_first_not_of("0123456789", 9) != std::string::npos)
                continue;
            const Z& o = objz->list[i + 1];
            Objective ob{n, ObjectiveTitle(msgs, m, n, o.list.size() > 1 ? o.list[1].str : ""), false, 0, 0};
            if (tour.count(n)) {
                ob.placed = true;
                ob.x = tour[n][0];
                ob.z = tour[n][1];
            } else {
                for (std::size_t j = 0; j + 1 < o.list.size() && !ob.placed; ++j)
                    if (o.list[j].str == "ACTIVE" || o.list[j].str == "INACTIVE")
                        for (const Z& nm : o.list[j + 1].list) {
                            const int node = FindNode(g, nm.str);
                            if (node >= 0 && NodeCentre(g, node, ob.x, ob.z)) { ob.placed = true; break; }
                        }
            }
            objs.push_back(ob);
        }
    std::sort(objs.begin(), objs.end(), [](const Objective& a, const Objective& b) { return a.n < b.n; });

    // bounds: outlines, every pickup, objectives, padded (as map_render.py)
    double bx0 = 1e30, bx1 = -1e30, bz0 = 1e30, bz1 = -1e30;
    auto grow = [&](double x, double z) { bx0 = std::min(bx0, x); bx1 = std::max(bx1, x); bz0 = std::min(bz0, z); bz1 = std::max(bz1, z); };
    for (const Outline& o : outlines)
        for (const auto& p : o.pts) grow(p[0], p[2]);
    int weapons = 0, upgrades = 0;
    if (pick)
        for (const Z& q : pick->list)
            if (q.type == 4 && q.list.size() >= 3 && q.list[2].type == 4 && q.list[2].list.size() >= 3) {
                grow(q.list[2].list[0].num, q.list[2].list[2].num);
                weapons += IsWeapon(q.list[0].str);
                upgrades += IsUpgrade(q.list[0].str);
            }
    for (const Objective& o : objs)
        if (o.placed) grow(o.x, o.z);
    if (bx0 > bx1) { why = "empty map"; return false; }
    const double pad = 120, ppu = 0.6;   // INVENTED (map_render.py defaults, chosen by eye): margin and pixels per unit
    const double x0 = bx0 - pad, z0 = bz0 - pad, x1 = bx1 + pad, z1 = bz1 + pad;
    const int W = int((z1 - z0) * ppu), H = int((x1 - x0) * ppu), ss = 2;
    if (W < 16 || H < 16 || W > 6000 || H > 6000) { why = "unexpected size"; return false; }

    TexSet tex;
    for (const wchar_t* a : {L"texture6.zbd", L"texture4.zbd", L"texture2.zbd", L"rtexture4.zbd", L"rtexture2.zbd"})
        tex.arch.push_back(ReadArchive(zbd / a, false));
    std::vector<Poly> polys = WorldPolys(g);
    Canvas big;
    big.w = W * ss;
    big.h = H * ss;
    big.px.assign(std::size_t(big.w) * big.h * 4, 255);
    for (std::size_t i = 0; i < big.px.size(); i += 4) { big.px[i] = 48; big.px[i + 1] = 34; big.px[i + 2] = 24; }
    // open areas are the sea (the game draws its water plane separately): the mission's water, tiled, darker
    for (const char* wt : {"ocean1", "oceantop", "wat2", "clwatr01"}) {
        const bool used = std::any_of(polys.begin(), polys.end(), [&](const Poly& p) { return p.tex == wt; });
        const Tex t = used ? tex.Get(wt) : Tex{};
        if (!t.px) continue;
        const int tile = std::max(8, int(64 * ppu * ss));
        for (int y = 0; y < big.h; ++y)
            for (int x = 0; x < big.w; ++x) {
                double r, gr, bl;
                Sample(t, double(x % tile) / tile, double(y % tile) / tile, r, gr, bl);
                std::uint8_t* d = big.At(x, y);
                d[0] = std::uint8_t(bl * 0.8);
                d[1] = std::uint8_t(gr * 0.8);
                d[2] = std::uint8_t(r * 0.8);
            }
        break;
    }
    struct Draw { double top; const Poly* p; double shade; };
    std::vector<Draw> order;
    for (const Poly& p : polys) {
        const auto& w = p.w;
        const double ax = w[1][0] - w[0][0], ay = w[1][1] - w[0][1], az = w[1][2] - w[0][2];
        const double bx = w[2][0] - w[0][0], by = w[2][1] - w[0][1], bz = w[2][2] - w[0][2];
        const double nx = ay * bz - az * by, ny = az * bx - ax * bz, nz = ax * by - ay * bx;
        double ln = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (ln == 0) ln = 1;
        if (std::fabs(ny) / ln < 0.15) continue;   // walls, edge-on from above
        const double light = std::max(0.0, (-nx * 0.4 + std::fabs(ny) * 0.8 + nz * 0.4) / ln);
        double top = -1e30;
        for (const auto& v : w) top = std::max(top, v[1]);
        order.push_back({top, &p, 0.55 + 0.45 * std::min(1.0, light)});
    }
    std::stable_sort(order.begin(), order.end(), [](const Draw& a, const Draw& b) { return a.top < b.top; });
    for (const Draw& d : order) {
        const Tex t = tex.Get(d.p->tex);
        if (!t.px) continue;
        const auto& w = d.p->w;
        std::vector<std::array<double, 2>> sp;
        for (const auto& v : w) sp.push_back({(v[2] - z0) * ppu * ss, (x1 - v[0]) * ppu * ss});
        for (std::size_t i = 1; i + 1 < w.size(); ++i) {
            const double tp[3][2] = {{sp[0][0], sp[0][1]}, {sp[i][0], sp[i][1]}, {sp[i + 1][0], sp[i + 1][1]}};
            const double tu[3][2] = {{d.p->uv[0][0], d.p->uv[0][1]}, {d.p->uv[i][0], d.p->uv[i][1]}, {d.p->uv[i + 1][0], d.p->uv[i + 1][1]}};
            FillTriangle(big, tp, tu, t, d.shade);
        }
    }
    // down to W x H (2 x 2 average)
    Canvas map;
    map.w = W;
    map.h = H;
    map.px.assign(std::size_t(W) * H * 4, 255);
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
            for (int c = 0; c < 3; ++c)
                map.At(x, y)[c] = std::uint8_t((big.At(2 * x, 2 * y)[c] + big.At(2 * x + 1, 2 * y)[c] + big.At(2 * x, 2 * y + 1)[c] +
                                                big.At(2 * x + 1, 2 * y + 1)[c] + 2) / 4);
    big.px.clear();
    big.px.shrink_to_fit();
    auto P = [&](double x, double z) { return std::array<double, 2>{(z - z0) * ppu, (x1 - x) * ppu}; };
    for (const Outline& o : outlines) {
        std::vector<std::array<double, 2>> pts;
        for (const auto& p : o.pts) pts.push_back(P(p[0], p[2]));
        if (pts.size() > 1) Polyline(map, pts, o.r, o.g, o.b, 110);   // INVENTED (by eye): the outlines faint under the new map
    }

    // the picture with its legend panel, drawn with GDI on a DIB
    const int PW = 420, FW = W + PW, FH = std::max(H, 760);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = FW;
    bi.bmiHeader.biHeight = -FH;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bmp = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!dc || !bmp) { why = "out of memory"; if (dc) DeleteDC(dc); return false; }
    HGDIOBJ old = SelectObject(dc, bmp);
    auto* fp = static_cast<std::uint8_t*>(bits);
    for (int y = 0; y < FH; ++y)
        for (int x = 0; x < FW; ++x) {
            std::uint8_t* d = fp + (std::size_t(y) * FW + x) * 4;
            if (x < W && y < H) std::memcpy(d, map.At(x, y), 4);
            else { d[0] = 28; d[1] = 20; d[2] = 16; d[3] = 255; }
        }
    GdiFlush();
    SetBkMode(dc, TRANSPARENT);
    auto font = [](int px, bool bold) {
        return CreateFontW(-px, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"Arial");
    };
    auto text = [&](int x, int y, const std::wstring& s, int px, bool bold, COLORREF col) {
        HFONT f = font(px, bold);
        HGDIOBJ of = SelectObject(dc, f);
        SetTextColor(dc, col);
        TextOutW(dc, x, y, s.c_str(), int(s.size()));
        SelectObject(dc, of);
        DeleteObject(f);
    };
    auto shape = [&](const std::vector<POINT>& pts, COLORREF fill, COLORREF edge, int pen) {
        HBRUSH br = CreateSolidBrush(fill);
        HPEN pn = CreatePen(PS_SOLID, pen, edge);
        HGDIOBJ ob = SelectObject(dc, br), op = SelectObject(dc, pn);
        Polygon(dc, pts.data(), int(pts.size()));
        SelectObject(dc, ob);
        SelectObject(dc, op);
        DeleteObject(br);
        DeleteObject(pn);
    };
    const double PI = 3.14159265358979;
    auto star = [&](double x, double y, double s) {
        std::vector<POINT> pts;
        for (int i = 0; i < 10; ++i) {
            const double r = s * (i % 2 ? 0.6 : 1.3), a = PI / 5 * i - PI / 2;
            pts.push_back({LONG(x + r * std::cos(a)), LONG(y + r * std::sin(a))});
        }
        shape(pts, RGB(200, 110, 255), RGB(255, 255, 255), 1);
    };
    auto objective = [&](double x, double y, double s, int n) {
        HBRUSH br = CreateSolidBrush(RGB(255, 60, 60));
        HPEN pn = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HGDIOBJ ob = SelectObject(dc, br), op = SelectObject(dc, pn);
        const double r = s * 1.3;
        Ellipse(dc, int(x - r), int(y - r), int(x + r), int(y + r));
        SelectObject(dc, ob);
        SelectObject(dc, op);
        DeleteObject(br);
        DeleteObject(pn);
        HFONT f = font(int(s * 1.5), true);
        HGDIOBJ of = SelectObject(dc, f);
        SetTextColor(dc, RGB(255, 255, 255));
        RECT rc{int(x - r), int(y - r), int(x + r), int(y + r)};
        const std::wstring t = std::to_wstring(n);
        DrawTextW(dc, t.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, of);
        DeleteObject(f);
    };
    // secrets named by the mission scripts: m1 gstrut.zrd opens the breakable door 'nudedoor' to the hidden area 'nbsecret'
    std::vector<std::wstring> secrets;
    if (m == 1) {
        const int node = FindNode(g, "nbsecret");
        double sx, sz;
        if (node >= 0 && NodeCentre(g, node, sx, sz)) {
            const auto q = P(sx, sz);
            star(q[0], q[1], 11);
            secrets.push_back(L"Hidden area behind a breakable door");
        }
    }
    const COLORREF txt = RGB(225, 228, 235), dim = RGB(150, 156, 168);
    int X = W + 24, Y = 24;
    text(X, Y, L"Mission " + std::to_wstring(m), 30, true, txt);
    Y += 52;
    text(X, Y, L"Objectives", 20, true, RGB(255, 60, 60));
    Y += 32;
    for (const Objective& o : objs) {
        objective(X + 12, Y + 12, 9, o.n);
        text(X + 34, Y + 1, o.title + (o.placed ? L"" : L"  (no fixed place)"), 18, false, txt);
        Y += 32;
    }
    Y += 16;
    text(X, Y, L"On this map", 20, true, txt);
    Y += 32;
    auto row = [&](const std::wstring& label) {
        text(X + 34, Y + 1, label, 18, false, txt);
        Y += 34;
    };
    if (weapons) {
        shape({{X + 12, Y + 4}, {X + 20, Y + 12}, {X + 12, Y + 20}, {X + 4, Y + 12}}, RGB(255, 150, 30), RGB(0, 0, 0), 1);
        row(L"Weapons");
    }
    if (upgrades) {
        std::vector<POINT> hx;
        for (int i = 0; i < 6; ++i) hx.push_back({LONG(X + 12 + 9.2 * std::cos(PI / 3 * i)), LONG(Y + 12 + 9.2 * std::sin(PI / 3 * i))});
        shape(hx, RGB(60, 200, 255), RGB(255, 255, 255), 1);
        row(L"Tank upgrades");
    }
    if (!secrets.empty()) {
        star(X + 12, Y + 12, 8);
        text(X + 34, Y + 1, L"Secrets", 18, false, txt);
        Y += 26;
        for (const auto& s : secrets) { text(X + 34, Y, s, 15, false, dim); Y += 20; }
        Y += 8;
    }
    shape({{X + 12, Y + 2}, {X + 20, Y + 22}, {X + 12, Y + 17}, {X + 4, Y + 22}}, RGB(255, 255, 255), RGB(0, 0, 0), 1);
    row(L"You");
    GdiFlush();
    Img out;
    out.w = FW;
    out.h = FH;
    out.bgra.assign(fp, fp + std::size_t(FW) * FH * 4);
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
    std::error_code ec;
    fs::create_directories(out_dir, ec);
    if (!SavePng(out_dir / (L"m" + std::to_wstring(m) + L".png"), out, false)) { why = "cannot write the picture"; return false; }
    // read by Redux (RX-47): map area size, world bounds, pixels per unit; then one line per objective (marker position)
    std::FILE* f = _wfopen((out_dir / (L"m" + std::to_wstring(m) + L".txt")).c_str(), L"w");
    if (!f) { why = "cannot write the map data"; return false; }
    std::fprintf(f, "%d %d %.3f %.3f %.3f %.3f %.5f\n", W, H, x0, z0, x1, z1, ppu);
    for (const Objective& o : objs) {
        const auto q = o.placed ? P(o.x, o.z) : std::array<double, 2>{-1, -1};
        std::fprintf(f, "obj %d %.1f %.1f\n", o.n, q[0], q[1]);
    }
    std::fclose(f);
    return true;
}

}  // namespace

int GenerateMaps(const std::wstring& dest, const std::function<void(const std::string&)>& log)
{
    const fs::path game(dest);
    HMODULE msgs = LoadLibraryExW((game / L"messages.dll").c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    int made = 0;
    for (int m = 1; m <= 6; ++m) {   // the campaign maps (m7-m13 are the multiplayer arenas)
        std::string why;
        try {
            if (MakeMap(game, m, game / L"redux_map", msgs, why)) ++made;
            else log("map " + std::to_string(m) + ": not made (" + why + ")");
        } catch (const std::exception& e) {
            log("map " + std::to_string(m) + ": not made (" + e.what() + ")");
        }
    }
    if (msgs) FreeLibrary(msgs);
    log("mission maps: " + std::to_string(made) + " of 6 (redux_map)");
    return made;
}

}  // namespace setup
