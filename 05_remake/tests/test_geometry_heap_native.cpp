// Native L1 for geometry functions that allocate or free: port vs the ORIGINAL bytes (tests/native_oracle.h).
// Both sides' MSVCRT import slots (the port's platform/msvcrt.h variables and the mapped original's IAT) are
// pointed at recorders around the real msvcrt.dll functions, so besides the resulting data the exact sequence
// of heap calls is compared: kind, sizes and which block (by role, not by address) each call touches.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zGeometry/zgeo_model.h"
#include "GameZRecoil/zGeometry/zgeo_weiler.h"
#include "platform/msvcrt.h"
#include "unattributed/geometry.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <random>
#include <vector>

namespace {
struct Ev {
    char kind;
    std::uint32_t a, b;
    int role;
    bool operator==(const Ev& o) const { return kind == o.kind && a == o.a && b == o.b && role == o.role; }
};
struct Log {
    std::vector<Ev> ev;
    std::map<const void*, int> roles;
    int next = 100;
    int role(const void* p) const
    {
        if (!p) return -1;
        auto it = roles.find(p);
        return it == roles.end() ? -2 : it->second;
    }
};
Log* g_log = nullptr;

using MallocFn = void*(__cdecl*)(std::size_t);
using CallocFn = void*(__cdecl*)(std::size_t, std::size_t);
using ReallocFn = void*(__cdecl*)(void*, std::size_t);
using FreeFn = void(__cdecl*)(void*);
MallocFn real_malloc;
CallocFn real_calloc;
ReallocFn real_realloc;
FreeFn real_free;

void* __cdecl rec_malloc(std::size_t n)
{
    void* p = real_malloc(n);
    g_log->ev.push_back({'M', static_cast<std::uint32_t>(n), 0, g_log->next});
    g_log->roles[p] = g_log->next++;
    return p;
}
void* __cdecl rec_calloc(std::size_t n, std::size_t s)
{
    void* p = real_calloc(n, s);
    g_log->ev.push_back({'C', static_cast<std::uint32_t>(n), static_cast<std::uint32_t>(s), g_log->next});
    g_log->roles[p] = g_log->next++;
    return p;
}
void* __cdecl rec_realloc(void* old, std::size_t n)
{
    g_log->ev.push_back({'R', static_cast<std::uint32_t>(n), static_cast<std::uint32_t>(g_log->next), g_log->role(old)});
    void* p = real_realloc(old, n);
    g_log->roles[p] = g_log->next++;
    return p;
}
void __cdecl rec_free(void* p)
{
    g_log->ev.push_back({'F', 0, 0, g_log->role(p)});
    real_free(p);
}

constexpr std::uintptr_t kOrigCalloc = 0x004cc4ac, kOrigRealloc = 0x004cc4ec, kOrigFree = 0x004cc5b4, kOrigMalloc = 0x004cc5dc;
void set_slot(std::uintptr_t va, const void* f) { const auto v = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(f)); std::memcpy(reinterpret_cast<void*>(va), &v, 4); }

// Install the recorders on both sides (idempotent); the real functions come from the port's slots, which
// platform/msvcrt.cpp resolved from msvcrt.dll.
void install_recorders()
{
    if (!real_malloc) {
        real_malloc = reinterpret_cast<MallocFn>(recoil::g_Iat_malloc_004cc5dc);
        real_calloc = reinterpret_cast<CallocFn>(recoil::g_Iat_calloc_004cc4ac);
        real_realloc = reinterpret_cast<ReallocFn>(recoil::g_Iat_realloc_004cc4ec);
        real_free = reinterpret_cast<FreeFn>(recoil::g_Iat_free_004cc5b4);
    }
    recoil::g_Iat_malloc_004cc5dc = reinterpret_cast<void*>(&rec_malloc);
    recoil::g_Iat_calloc_004cc4ac = reinterpret_cast<void*>(&rec_calloc);
    recoil::g_Iat_realloc_004cc4ec = reinterpret_cast<void*>(&rec_realloc);
    recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&rec_free);
    set_slot(kOrigMalloc, reinterpret_cast<void*>(&rec_malloc));
    set_slot(kOrigCalloc, reinterpret_cast<void*>(&rec_calloc));
    set_slot(kOrigRealloc, reinterpret_cast<void*>(&rec_realloc));
    set_slot(kOrigFree, reinterpret_cast<void*>(&rec_free));
}
void remove_recorders()
{
    recoil::g_Iat_malloc_004cc5dc = reinterpret_cast<void*>(real_malloc);
    recoil::g_Iat_calloc_004cc4ac = reinterpret_cast<void*>(real_calloc);
    recoil::g_Iat_realloc_004cc4ec = reinterpret_cast<void*>(real_realloc);
    recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(real_free);
    set_slot(kOrigMalloc, reinterpret_cast<void*>(real_malloc));
    set_slot(kOrigCalloc, reinterpret_cast<void*>(real_calloc));
    set_slot(kOrigRealloc, reinterpret_cast<void*>(real_realloc));
    set_slot(kOrigFree, reinterpret_cast<void*>(real_free));
}

std::mt19937 hrng(0x4EA9);
std::uint32_t u(std::uint32_t n) { return static_cast<std::uint32_t>(hrng() % n); }
float hf() { return std::uniform_real_distribution<float>(-100, 100)(hrng); }
std::uint32_t P(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
template <class T> T* at(void* base, int off) { return reinterpret_cast<T*>(static_cast<char*>(base) + off); }
// A heap block owned by a log (registered under a role both sides share).
void* block(Log& L, int role, std::size_t n)
{
    void* p = real_malloc(n);
    std::memset(p, 0xCD, n);
    L.roles[p] = role;
    return p;
}
}  // namespace

TEST(native_geometry_dynarray_and_frees_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    install_recorders();
    using InitFn = void(__fastcall*)(void*, int, int);
    using PushFn = void(__fastcall*)(void*, int, void**);
    using P1 = void(__fastcall*)(void*);
    using AllocFn = void*(__cdecl*)();
    auto oInit = rt::original<InitFn>(0x00467600);
    auto oPush = rt::original<PushFn>(0x00467660);
    auto oFree = rt::original<P1>(0x00467630);
    auto oDestroy = rt::original<P1>(0x004647d0);
    auto oFreeBuf = rt::original<P1>(0x00464b30);
    auto oResFree = rt::original<P1>(0x0046ab10);
    auto oA16 = rt::original<AllocFn>(0x0046af00);
    auto oA16Free = rt::original<P1>(0x0046af20);
    auto oA3Free = rt::original<P1>(0x0046c720);
    int bad[8] = {};
    for (int n = 0; n < 1500; ++n) {
        Log LO, LP;
        {   // DynArray: init, a run of pushes, free - header compared after each step (buffer fields by role)
            std::uint32_t hO[5], hP[5];
            for (int i = 0; i < 5; ++i) hO[i] = hP[i] = hrng();
            const int cap = 1 + static_cast<int>(u(6)), size = 4 * (1 + static_cast<int>(u(6)));
            g_log = &LO; oInit(hO, cap, size);
            g_log = &LP; recoil::DynArray_Init(hP, cap, size);
            auto same = [&] {
                if (hO[0] != hP[0] || hO[1] != hP[1] || hO[2] != hP[2]) return false;
                if (LO.role(reinterpret_cast<void*>(static_cast<std::uintptr_t>(hO[3]))) != LP.role(reinterpret_cast<void*>(static_cast<std::uintptr_t>(hP[3])))) return false;
                return hO[4] - hO[3] == hP[4] - hP[3];
            };
            bool ok = same() && std::memcmp(reinterpret_cast<void*>(static_cast<std::uintptr_t>(hO[3])), reinterpret_cast<void*>(static_cast<std::uintptr_t>(hP[3])), cap * size) == 0;
            const int pushes = static_cast<int>(u(6));
            for (int k = 0; ok && k < pushes; ++k) {
                const int m = 1 + static_cast<int>(u(4));
                void* old1 = reinterpret_cast<void*>(1);
                void* old2 = reinterpret_cast<void*>(1);
                const bool want = u(2);
                g_log = &LO; oPush(hO, m, want ? &old1 : nullptr);
                g_log = &LP; recoil::DynArray_Push(hP, m, want ? &old2 : nullptr);
                ok = same() && (old1 == reinterpret_cast<void*>(1)) == (old2 == reinterpret_cast<void*>(1))
                     && (old1 == reinterpret_cast<void*>(1) || LO.role(old1) == LP.role(old2));
            }
            g_log = &LO; oFree(hO);
            g_log = &LP; recoil::DynArray_Free(hP);
            if (!ok || std::memcmp(hO, hP, sizeof hO) || !(LO.ev == LP.ev)) ++bad[0];
        }
        {   // WeilerClip_Destroy: four dynamic arrays (+0x34, +0x48, +0x5c, +0xc) then the clip itself; or null
            Log O2, P2;
            auto make = [&](Log& L) -> void* {
                void* c = block(L, 1, 0x70);
                const int offs[4] = {0x34, 0x48, 0x5c, 0xc};
                for (int i = 0; i < 4; ++i) *at<std::uint32_t>(c, offs[i] + 0xc) = (i + n) % 3 ? P(block(L, 10 + i, 16)) : 0;
                return c;
            };
            void* cO = n % 9 ? make(O2) : nullptr;
            void* cP = n % 9 ? make(P2) : nullptr;
            g_log = &O2; oDestroy(cO);
            g_log = &P2; recoil::WeilerClip_Destroy(cP);
            if (!(O2.ev == P2.ev)) ++bad[1];
        }
        {   // WeilerClip_FreeBuffers: frees and nulls four fields; the clip itself stays
            Log O2, P2;
            std::uint32_t cO[8], cP[8];
            for (int i = 0; i < 8; ++i) cO[i] = cP[i] = hrng();
            const int offs[4] = {0x1c, 0x4, 0xc, 0x14};
            for (int i = 0; i < 4; ++i) {
                const bool set = (i + n) % 4 != 0;
                *at<std::uint32_t>(cO, offs[i]) = set ? P(block(O2, 20 + i, 8)) : 0;
                *at<std::uint32_t>(cP, offs[i]) = set ? P(block(P2, 20 + i, 8)) : 0;
            }
            g_log = &O2; oFreeBuf(cO);
            g_log = &P2; recoil::WeilerClip_FreeBuffers(cP);
            if (std::memcmp(cO, cP, sizeof cO) || !(O2.ev == P2.ev)) ++bad[2];
        }
        {   // ClipResult_Free: [+0x4] buffer (or null), [+0x0] clip destroyed, then the result block
            Log O2, P2;
            auto make = [&](Log& L) -> void* {
                void* r = block(L, 30, 8);
                void* c = block(L, 31, 0x70);
                for (int off : {0x34, 0x48, 0x5c, 0xc}) *at<std::uint32_t>(c, off + 0xc) = 0;
                *at<std::uint32_t>(r, 0) = P(c);
                *at<std::uint32_t>(r, 4) = n % 2 ? P(block(L, 32, 12)) : 0;
                return r;
            };
            void* rO = make(O2);
            void* rP = make(P2);
            g_log = &O2; oResFree(rO);
            g_log = &P2; recoil::ClipResult_Free(rP);
            if (!(O2.ev == P2.ev)) ++bad[3];
        }
        {   // Alloc16Zeroed / Alloc16_Free
            Log O2, P2;
            g_log = &O2; void* a = oA16();
            g_log = &P2; void* b = recoil::Alloc16Zeroed();
            const bool same16 = std::memcmp(a, b, 16) == 0;
            const bool child = n % 2;
            if (child) { *at<std::uint32_t>(a, 0xc) = P(block(O2, 40, 4)); *at<std::uint32_t>(b, 0xc) = P(block(P2, 40, 4)); }
            g_log = &O2; oA16Free(a);
            g_log = &P2; recoil::Alloc16_Free(b);
            if (!same16) ++bad[4];
            if (!(O2.ev == P2.ev)) ++bad[5];
        }
        {   // Alloc3Ptr_Free: [+0x4], [+0xc] if set, then the block; or null
            Log O2, P2;
            auto make = [&](Log& L) -> void* {
                void* b = block(L, 50, 16);
                *at<std::uint32_t>(b, 4) = n % 2 ? P(block(L, 51, 4)) : 0;
                *at<std::uint32_t>(b, 0xc) = n % 3 ? P(block(L, 52, 4)) : 0;
                return b;
            };
            void* bO = n % 7 ? make(O2) : nullptr;
            void* bP = n % 7 ? make(P2) : nullptr;
            g_log = &O2; oA3Free(bO);
            g_log = &P2; recoil::Alloc3Ptr_Free(bP);
            if (!(O2.ev == P2.ev)) ++bad[6];
        }
    }
    remove_recorders();
    for (int i = 0; i < 7; ++i) CHECK_EQ(bad[i], 0);
}

TEST(native_geometry_model_buffers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    install_recorders();
    using CopyFn = int(__fastcall*)(const void*, int*, float**);
    using GatherFn = void*(__fastcall*)(const void*, const void*, void*);
    using AppendFn = bool(__fastcall*)(void*, int);
    auto oCopy = rt::original<CopyFn>(0x0046aab0);
    auto oGather = rt::original<GatherFn>(0x0046b650);
    auto oAppend = rt::original<AppendFn>(0x00468700);
    int bad[3] = {};
    for (int n = 0; n < 2000; ++n) {
        {   // ClipPolygon_CopyOutRotated: polygon [+0x4] points, [+0x8] count
            Log LO, LP;
            float pts[3 * 8];
            for (float& x : pts) x = hf();
            std::uint32_t poly[3] = {hrng(), P(pts), u(9)};
            int cO = -1, cP = -1;
            float* bO = n % 3 ? static_cast<float*>(block(LO, 60, 4)) : nullptr;
            float* bP = n % 3 ? static_cast<float*>(block(LP, 60, 4)) : nullptr;
            g_log = &LO; const int rO = oCopy(poly, &cO, &bO);
            g_log = &LP; const int rP = recoil::ClipPolygon_CopyOutRotated(poly, &cP, &bP);
            const bool ok = rO == rP && cO == cP && LO.ev == LP.ev && LO.role(bO) == LP.role(bP)
                         && (poly[2] == 0 || std::memcmp(bO, bP, 12 * poly[2]) == 0);
            if (!ok) ++bad[0];
            if (bO) real_free(bO);
            if (bP) real_free(bP);
        }
        {   // ModelPolygon_GatherPoints: model [+0x34] vertices; polygon low byte of [+0x0] count, [+0x8] indices
            Log LO, LP;
            float verts[3 * 16];
            for (float& x : verts) x = hf();
            std::uint32_t model[16];
            for (auto& w : model) w = hrng();
            model[0x34 / 4] = P(verts);
            std::uint32_t idx[8];
            for (auto& i : idx) i = u(16);
            std::uint32_t poly[3] = {(hrng() & 0xFFFFFF00u) | u(9), hrng(), P(idx)};
            void* bO = n % 3 ? block(LO, 70, 4) : nullptr;
            void* bP = n % 3 ? block(LP, 70, 4) : nullptr;
            g_log = &LO; void* rO = oGather(model, poly, bO);
            g_log = &LP; void* rP = recoil::ModelPolygon_GatherPoints(model, poly, bP);
            const std::uint32_t cnt = poly[0] & 0xFF;
            const bool ok = LO.ev == LP.ev && LO.role(rO) == LP.role(rP) && (cnt == 0 || std::memcmp(rO, rP, 12 * cnt) == 0);
            if (!ok) ++bad[1];
            if (rO) real_free(rO);
            if (rP) real_free(rP);
        }
        {   // ClipContour_AppendToOutput: contour [+0] flags (bit 0), [+0x8] output; pass at +0x20 (EDX == 3) or +0xc
            //   pass: [+0x8] count, [+0xc] points; output: [+0x4] -> header pair, [+0x18] count, [+0x1c] buffer
            Log LO, LP;
            float pts[3 * 70];
            for (float& x : pts) x = hf();
            auto make = [&](Log& L, std::uint32_t* contour, std::uint32_t* out, std::uint32_t* hdr, std::uint32_t outCount) {
                std::memset(contour, 0, 0x40);
                contour[0] = n % 5 ? 1u : 0u;
                contour[2] = P(out);
                const std::uint32_t passCount = u(70);
                contour[(0x20 + 0x8) / 4] = passCount; contour[(0x20 + 0xc) / 4] = P(pts);
                contour[(0xc + 0x8) / 4] = passCount; contour[(0xc + 0xc) / 4] = P(pts);
                std::memset(out, 0, 0x20);
                out[1] = P(hdr);
                out[0x18 / 4] = outCount;
                // the output buffer holds 0x80 points until grown (the listing reallocs above 0x80)
                out[0x1c / 4] = P(block(L, 80, 12 * 0x80 + 12 * outCount));
                hdr[0] = hdr[1] = 0xABABABAB;
            };
            std::uint32_t conO[16], conP[16], outO[8], outP[8], hdrO[2], hdrP[2];
            const std::uint32_t outCount = u(70);
            const int pass = n % 2 ? 3 : 1;
            make(LO, conO, outO, hdrO, outCount);
            std::mt19937 save = hrng;
            make(LP, conP, outP, hdrP, outCount);
            hrng = save;
            conP[(0x20 + 0x8) / 4] = conO[(0x20 + 0x8) / 4];
            conP[(0xc + 0x8) / 4] = conO[(0xc + 0x8) / 4];
            g_log = &LO; const bool rO = oAppend(conO, pass);
            g_log = &LP; const bool rP = recoil::ClipContour_AppendToOutput(conP, pass);
            const std::uint32_t total = outO[0x18 / 4];
            bool ok = rO == rP && LO.ev == LP.ev && outO[0] == outP[0] && total == outP[0x18 / 4]
                   && hdrO[0] == hdrP[0] && hdrO[1] == hdrP[1]
                   && LO.role(reinterpret_cast<void*>(static_cast<std::uintptr_t>(outO[7]))) == LP.role(reinterpret_cast<void*>(static_cast<std::uintptr_t>(outP[7])));
            ok = ok && std::memcmp(reinterpret_cast<void*>(static_cast<std::uintptr_t>(outO[7])), reinterpret_cast<void*>(static_cast<std::uintptr_t>(outP[7])), 12 * total) == 0;
            if (!ok) ++bad[2];
            real_free(reinterpret_cast<void*>(static_cast<std::uintptr_t>(outO[7])));
            real_free(reinterpret_cast<void*>(static_cast<std::uintptr_t>(outP[7])));
        }
    }
    remove_recorders();
    for (int i = 0; i < 3; ++i) CHECK_EQ(bad[i], 0);
}

TEST(native_geometry_model_leaves_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using RectFn = void(__fastcall*)(float*, const float*, int);
    using UseFn = int(__fastcall*)(int, int, const void*, int*);
    using EdgeFn = const void*(__fastcall*)(int, int, int, const void*);
    using RevFn = void(__fastcall*)(int, float*);
    auto oRect = rt::original<RectFn>(0x0046a9c0);
    auto oUse = rt::original<UseFn>(0x0046bf30);
    auto oEdge = rt::original<EdgeFn>(0x0046bf70);
    auto oRev = rt::original<RevFn>(0x0046c5b0);
    int bad[4] = {};
    for (int n = 0; n < 4000; ++n) {
        {   float pts[3 * 8], r1[4] = {1, 2, 3, 4}, r2[4] = {1, 2, 3, 4};
            for (float& x : pts) x = hf();
            if (n % 13 == 0) pts[4] = std::numeric_limits<float>::quiet_NaN();
            const int c = 1 + static_cast<int>(u(8));
            oRect(r1, pts, c); recoil::Rect_FromPoints2D(r2, pts, c);
            if (std::memcmp(r1, r2, 16)) ++bad[0];
        }
        std::uint32_t edges[3 * 10];
        for (int i = 0; i < 10; ++i) { edges[3 * i] = u(6); edges[3 * i + 1] = u(6); edges[3 * i + 2] = u(3) == 0 ? 0 : hrng() | 1; }
        const int count = static_cast<int>(u(11)) - (n % 17 == 0 ? 11 : 0);
        {   int o1[10] = {}, o2[10] = {};
            const int v = static_cast<int>(u(6));
            if (oUse(v, count, edges, o1) != recoil::EdgeList_FindUsingVertex(v, count, edges, o2) || std::memcmp(o1, o2, sizeof o1)) ++bad[1];
        }
        {   const int a = static_cast<int>(u(6)), b = static_cast<int>(u(6));
            if (oEdge(a, b, count, edges) != recoil::EdgeList_FindEdge(a, b, count, edges)) ++bad[2];
        }
        {   float v1[3 * 9], v2[3 * 9];
            for (int i = 0; i < 27; ++i) v1[i] = v2[i] = hf();
            const int c = static_cast<int>(u(10));
            oRev(c, v1); recoil::Vec3Array_ReverseKeepFirst(c, v2);
            if (std::memcmp(v1, v2, sizeof v1)) ++bad[3];
        }
    }
    for (int i = 0; i < 4; ++i) CHECK_EQ(bad[i], 0);
}
