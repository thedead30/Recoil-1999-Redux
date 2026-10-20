// Structured native L1s for three rasteriser functions ported in the cloud that the arena fuzz cannot drive:
//  - Raster_InitNoiseAndBackBuffer (0x0048d340): malloc of 25 x the surface width [0x00632220] filled from rand, the
//    2-byte back buffer of width x height [0x00632224], the tint rect 0x0056b1c4.. zeroed, the tint routine
//    [0x0056b1d8] = Tint_Row555; sizes up to 20 x 12, srand from the seed before each side;
//  - SwRender_Init (0x00490520; ECX span capacity 0..64): calloc of the span tables [0x0057dae4] / [0x0057dae8],
//    zRndr_BeginFrame (no clip polygons), the span hooks [0x006320a4] / [0x006320a8] = Span_InsertOccluding /
//    Span_ClipVisible;
//  - Raster_PickTextureLevel (0x00499130; ECX material chain or 0, EDX plane points, stack n, UV pairs, z / U' / V'
//    gradients; ret 0x14): [0x0063209c] off -> the base texture; else the level from the largest-z vertex's texel
//    footprint through Tex_PickLevel; 1..6 vertices, chains of 0..4 levels.
//  - Blur16_Full / Blur16_Vertical / Blur16_Horizontal (0x0048e380 / 0x0048e670 / 0x0048e870; menus; ECX rect or 0):
//    3x3 / vertical / horizontal box blurs of the 16-bit buffer [0x0056b1c4] (width [0x0056b1c8], height [0x0056b1cc],
//    pitch [0x0056b1d4] pixels) into [0x0056b1c0] with the channel masks of the screen format (0x00632164..6c, 565 or
//    555), the rect clamped to the buffer; buffers up to 16 x 12 with slack around them.
// malloc / calloc are bound to fakes that hand out test blocks. Code addresses the functions store are compared as
// their original VAs. Compared: EAX, every block word, the globals written.
#include "test.h"
#include "vt_runner.h"
#include "platform/iat_msvcrt.h"
#include "unattributed/rasteriser.h"
#include "unattributed/menus.h"

using namespace vtr;

namespace {
using U = std::uint32_t;

World*& cur_world()
{
    static World* w = nullptr;
    return w;
}
void* __cdecl f_malloc(std::size_t n)
{
    std::mt19937 r(static_cast<U>(n));
    vt::log().push_back(0x3A11);
    vt::log().push_back(static_cast<U>(n));
    return ch::at(cur_world()->add(r, static_cast<U>(n) + 4));
}
void* __cdecl f_calloc(std::size_t n, std::size_t size)
{
    std::mt19937 r(0);
    vt::log().push_back(0xCA11);
    vt::log().push_back(static_cast<U>(n));
    vt::log().push_back(static_cast<U>(size));
    const U p = cur_world()->add(r, static_cast<U>(n * size) + 4);
    std::memset(ch::at(p), 0, n * size + 4);
    return ch::at(p);
}
struct Bind {
    void** o;
    void** p;
    void* saved[2];
    Bind(U va, void*& port, void* fn) : o(reinterpret_cast<void**>(static_cast<std::uintptr_t>(va))), p(&port)
    {
        saved[0] = *o; saved[1] = *p;
        *o = *p = fn;
    }
    ~Bind() { *o = saved[0]; *p = saved[1]; }
};
// a global word: code addresses of the port as the original's VA, then the World's normaliser
U code_norm(World& w, U v)
{
    const std::pair<void*, U> code[] = {{(void*)&recoil::Tint_Row555, 0x0048d450}, {(void*)&recoil::Span_InsertOccluding, 0x00490ae0},
                                        {(void*)&recoil::Span_ClipVisible, 0x00491840}};
    for (const auto& c : code) if (v == ch::addr(c.first)) return c.second;
    return w.norm(v);
}
void log_globals(World& w, U lo, U hi)
{
    for (U va = lo; va < hi; va += 4) vt::log().push_back(code_norm(w, *ch::img(w.side, va)));
}
float frand(std::mt19937& r, float lo, float hi) { return lo + (hi - lo) * static_cast<float>(r() % 10000) / 10000.0f; }

std::vector<Case> cases()
{
    std::vector<Case> c;
    c.push_back({"Raster_InitNoiseAndBackBuffer", 0x0048d340, (void*)&recoil::Raster_InitNoiseAndBackBuffer, 200, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     *ch::img(w.side, 0x00632220) = r() % 21;
                     *ch::img(w.side, 0x00632224) = r() % 13;
                     w.blocks[0].w[0] = r();            // the srand seed
                     cur_world() = &w;
                 },
                 [](World& w, U fn, std::mt19937&) {
                     ch::crt_fn<void(__cdecl*)(unsigned)>("srand")(w.blocks[0].w[0]);
                     const U eax = call_any(fn, 0, 0, nullptr, 0);
                     log_globals(w, 0x0056b1b8, 0x0056b1dc);
                     cur_world() = nullptr;
                     return eax;
                 },
                 false});
    c.push_back({"SwRender_Init", 0x00490520, (void*)&recoil::SwRender_Init, 200, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     *ch::img(w.side, 0x0057de30) = 0;
                     cur_world() = &w;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 65, 0, nullptr, 0);
                     for (U va : {0x0057dae0u, 0x0057dae4u, 0x0057dae8u, 0x0057daf0u, 0x0057daf4u, 0x0057dafcu, 0x0057db00u, 0x0057de30u,
                                  0x006320a4u, 0x006320a8u})
                         vt::log().push_back(code_norm(w, *ch::img(w.side, va)));
                     cur_world() = nullptr;
                     return eax;
                 }});
    c.push_back({"Raster_PickTextureLevel", 0x00499130, (void*)&recoil::Raster_PickTextureLevel, 800, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U n = 1 + r() % 6;
                     const U pts = w.add(r, 12 * 6);          // block 0: plane points
                     for (U k = 0; k < 18; ++k) w.at(pts)[k] = fbits(frand(r, 0.01f, 4.0f));
                     const U uv = w.add(r, 8 * 6);            // block 1: U'V' pairs
                     for (U k = 0; k < 12; ++k) w.at(uv)[k] = fbits(frand(r, -2, 2));
                     for (int g = 0; g < 3; ++g) {            // blocks 2..4: the z / U' / V' gradients
                         const U b = w.add(r, 8);
                         for (U k = 0; k < 2; ++k) w.at(b)[k] = fbits(frand(r, -0.01f, 0.01f));
                     }
                     U next = 0;                              // the level chain
                     for (U k = 0, m = r() % 5; k < m; ++k) {
                         const U node = w.add(r, 0x24);
                         w.at(node)[8] = next;
                         next = node;
                     }
                     w.add(r, 8);
                     w.blocks.back().w[0] = r() % 6 ? next : 0u;
                     w.blocks.back().w[1] = n;
                     *ch::img(w.side, 0x0063209c) = r() % 5 ? 1u : 0u;
                 },
                 [](World& w, U fn, std::mt19937&) {
                     const U mat = w.blocks.back().w[0], n = w.blocks.back().w[1];
                     U a[5] = {n, ch::addr(w.blocks[1].w.data()), ch::addr(w.blocks[2].w.data()), ch::addr(w.blocks[3].w.data()),
                               ch::addr(w.blocks[4].w.data())};
                     return call_any(fn, mat, ch::addr(w.blocks[0].w.data()), a, 5);
                 }});
    struct B { const char* name; U va; void* port; };
    const B bs[] = {{"Blur16_Full", 0x0048e380, (void*)&recoil::Blur16_Full},
                    {"Blur16_Vertical", 0x0048e670, (void*)&recoil::Blur16_Vertical},
                    {"Blur16_Horizontal", 0x0048e870, (void*)&recoil::Blur16_Horizontal}};
    for (const B& b : bs)
        c.push_back({b.name, b.va, b.port, 500, [](std::mt19937&) {},
                     [](World& w, std::mt19937& r) {
                         const U rect = w.add(r, 16);             // block 0: the rect
                         for (U k = 0; k < 4; ++k) w.at(rect)[k] = static_cast<U>(static_cast<int>(r() % 22) - 3);
                         const U wd = 1 + r() % 16, ht = 2 + r() % 11, pitch = wd + r() % 4;  // >= 2 rows: y is clamped to 1..h-1
                         // the blur clamps x0 >= 0, y0 >= 1, x1 <= w-1, y1 <= h-1 but never checks that the start is on
                         // the surface or before the end: an inverted rect, or one starting past the surface, runs the row
                         // loop with a negative count and overran the destination on both sides (0xc0000374). Callers pass
                         // ordered rects that start on the surface; negative starts and ends past the edge stay (clamped).
                         int* q = reinterpret_cast<int*>(w.at(rect));
                         if (q[0] > q[2]) std::swap(q[0], q[2]);
                         if (q[1] > q[3]) std::swap(q[1], q[3]);
                         if (q[0] > static_cast<int>(wd) - 1) q[0] = static_cast<int>(wd) - 1;
                         if (q[1] > static_cast<int>(ht) - 1) q[1] = static_cast<int>(ht) - 1;
                         if (q[2] < q[0]) q[2] = q[0];
                         if (q[3] < q[1]) q[3] = q[1];
                         if (q[3] < 1) q[3] = 1;  // y0 is forced to >= 1, so an end row of 0 inverts too
                         // two spare rows (and 32 bytes) either side of the surface: the 3x3 blur touches the rows and
                         // columns next to the clamped rect (the heap check caught the original writing past a 16-byte
                         // margin); the margin bytes are still compared
                         const U margin = 2 * 2 * pitch + 32;
                         const U bytes = 2 * pitch * ht + 2 * margin;
                         const U src = w.add(r, bytes), dst = w.add(r, bytes);
                         *ch::img(w.side, 0x0056b1c4) = src + margin;
                         *ch::img(w.side, 0x0056b1c0) = dst + margin;
                         *ch::img(w.side, 0x0056b1c8) = wd;
                         *ch::img(w.side, 0x0056b1cc) = ht;
                         *ch::img(w.side, 0x0056b1d4) = pitch;
                         const bool g6 = r() % 2 != 0;
                         *ch::img(w.side, 0x00632164) = g6 ? 0xf800u : 0x7c00u;
                         *ch::img(w.side, 0x00632168) = g6 ? 0x07e0u : 0x03e0u;
                         *ch::img(w.side, 0x0063216c) = 0x001fu;
                     },
                     [](World& w, U fn, std::mt19937& r) { return call_any(fn, r() % 4 ? ch::addr(w.blocks[0].w.data()) : 0u, 0, nullptr, 0); },
                     false});
    return c;
}
}  // namespace

TEST(native_rasteriser_init_and_level_pick_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    Bind m(0x004cc5dc, recoil::g_Iat_malloc_004cc5dc, reinterpret_cast<void*>(&f_malloc));
    Bind cl(0x004cc4ac, recoil::g_Iat_calloc_004cc4ac, reinterpret_cast<void*>(&f_calloc));
    CHECK_EQ(run_cases(cases(), "rasteriser init / level pick"), 0);
}
