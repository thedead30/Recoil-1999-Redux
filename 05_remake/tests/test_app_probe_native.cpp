// Structured native L1s for two app start-up functions ported in the cloud that only talk to the system, driven
// through fake import functions bound into both sides' slots (the original's slot and the port's g_Iat_ variable):
//  - RecoilApp_ProbeDirectXCapabilities (0x0040c370; ECX -> version out, EDX -> platform out): GetVersionExA (fake
//    fills platform 1/2, major 3..5 or fails), LoadLibraryA / GetProcAddress / FreeLibrary / OutputDebugStringA fakes,
//    DirectDrawCreate a fake that hands out a fake-table object (tests/fake_vtable.h). The objects' QueryInterface
//    (slot 0; the IID picks the object handed out), Release (slot 2), CreateSurface (slot 6; the surface description
//    logged) and SetCooperativeLevel (slot 0x14) take their HRESULTs from a per-call script drawn from the seed, as do
//    the library and proc lookups, so every exit of the probe is reached;
//  - App_RedirectStdioToLogs (0x004a5780; ECX = base path or 0): freopen (fake: 0 or a fake stream, by script),
//    GetTempPathA (fake: a fixed directory or failure), fprintf / fflush fakes, _iob a static array of this test.
// Compared: the call log (pointers normalised, strings by content), EAX, every block word, the global 0x004f3eec.
#include "test.h"
#include "vt_runner.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"
#include "unattributed/app.h"

using namespace vtr;

namespace {
using U = std::uint32_t;

// results drawn from the seed, handed out in call order
std::vector<U>& script()
{
    static std::vector<U> v;
    return v;
}
U& script_at()
{
    static U i = 0;
    return i;
}
U next(U n) { const U v = script().empty() ? 0 : script()[script_at()++ % script().size()]; return v % n; }
bool fail() { return next(5) == 0; }  // one call in five fails
void new_script(std::mt19937& r)
{
    script().clear();
    script_at() = 0;
    for (int k = 0; k < 64; ++k) script().push_back(r());
}
// the objects handed out: DirectDraw, DirectDraw2, surface, surface3, surface4
U& obj(int k)
{
    static U o[5];
    return o[k];
}
U hash_str(U p)
{
    const char* s = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p));
    U h = 0;
    for (int k = 0; s && s[k] && k < 128; ++k) h = h * 31 + static_cast<unsigned char>(s[k]);
    return h;
}
void lg(U tag, U v) { vt::log().push_back(tag); vt::log().push_back(v); }

// --- the probe's system functions
int __stdcall f_version(OSVERSIONINFOA* p)
{
    lg(0x7E, p->dwOSVersionInfoSize);
    if (next(10) == 0) return 0;
    p->dwMajorVersion = 3 + next(3);
    p->dwMinorVersion = next(2);
    p->dwBuildNumber = 1381;
    p->dwPlatformId = 1 + next(2);
    return 1;
}
U __stdcall f_load(const char* name)
{
    lg(0x1B, vt::norm()(ch::addr(name)));
    vt::log().push_back(hash_str(ch::addr(name)));
    return fail() ? 0 : 0x12340000;
}
U __stdcall f_ddcreate(void* guid, U* pp, void* outer)
{
    lg(0xDD, vt::norm()(ch::addr(guid)));
    vt::log().push_back(vt::norm()(ch::addr(pp)));
    vt::log().push_back(vt::norm()(ch::addr(outer)));
    if (fail()) return 0x80004005;
    *pp = obj(0);
    return 0;
}
U __stdcall f_proc(U h, const char* name)
{
    lg(0x9A, h);
    vt::log().push_back(vt::norm()(ch::addr(name)));
    vt::log().push_back(hash_str(ch::addr(name)));
    if (fail()) return 0;
    return ch::addr(reinterpret_cast<void*>(&f_ddcreate));
}
int __stdcall f_free_lib(U h) { lg(0xF1, h); return 1; }
void __stdcall f_debug(const char* s) { lg(0xDB, vt::norm()(ch::addr(s))); vt::log().push_back(hash_str(ch::addr(s))); }

void probe_slots()
{
    // QueryInterface: the IID (its original VA) picks the object
    vt::set(0, 3);
    vt::slots()[0].fn = [](U, const U* a) -> U {
        if (fail()) return 0x80004002;
        const U iid = vt::norm()(a[1]);
        *ch::at(a[2]) = iid == 0x004d2f98 ? obj(1) : iid == 0x004d2fd8 ? obj(3) : obj(4);
        return 0;
    };
    vt::set(8, 1, 1, 0, 0);  // Release
    // CreateSurface (this, description, out, outer): the description's 27 words
    vt::set(0x18, 4);
    vt::slots()[6].fn = [](U, const U* a) -> U {
        for (int k = 0; k < 27; ++k) vt::log().push_back(ch::at(a[1])[k]);
        if (fail()) return 0x80004005;
        *ch::at(a[2]) = obj(2);
        return 0;
    };
    vt::set(0x50, 3);  // SetCooperativeLevel
    vt::slots()[0x14].fn = [](U, const U*) -> U { return fail() ? 0x80004005 : 0; };
}

// --- the redirect's CRT functions
U s_iob[24];
U& stream()
{
    static U s = 0;
    return s;
}
U __cdecl f_freopen(const char* path, const char* mode, U f)
{
    lg(0xF0, hash_str(ch::addr(path)));
    vt::log().push_back(vt::norm()(ch::addr(mode)));
    vt::log().push_back(f - ch::addr(s_iob));
    return next(2) ? 0 : stream();
}
U __stdcall f_temp(U n, char* buf)
{
    lg(0x7A, n);
    if (next(2)) return 0;
    std::memcpy(buf, "C:\\TEMP\\", 9);
    return 8;
}
int __cdecl f_fprintf(U f, const char* fmt)
{
    lg(0xFF, vt::norm()(f));
    vt::log().push_back(vt::norm()(ch::addr(fmt)));
    vt::log().push_back(hash_str(ch::addr(fmt)));
    return 1;
}
int __cdecl f_fflush(U f) { lg(0xF5, vt::norm()(f)); return 0; }

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

std::vector<Case> cases()
{
    std::vector<Case> c;
    c.push_back({"RecoilApp_ProbeDirectXCapabilities", 0x0040c370, (void*)&recoil::RecoilApp_ProbeDirectXCapabilities, 1500,
                 [](std::mt19937& r) { new_script(r); probe_slots(); },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);  // block 0: version out
                     w.add(r, 4);  // block 1: platform out
                     for (int k = 0; k < 5; ++k) obj(k) = object(w, r, 0x10);
                 },
                 [](World& w, U fn, std::mt19937&) {
                     return call_any(fn, ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), nullptr, 0);
                 },
                 false});
    c.push_back({"App_RedirectStdioToLogs", 0x004a5780, (void*)&recoil::App_RedirectStdioToLogs, 400,
                 [](std::mt19937& r) { new_script(r); },
                 [](World& w, std::mt19937& r) {
                     const U s = w.add(r, 0x20);
                     const char* names[3] = {"C:\\Recoil\\", "D:\\", "logs\\run1\\"};
                     const char* nm = names[r() % 3];
                     std::memcpy(w.at(s), nm, std::strlen(nm) + 1);
                     stream() = w.add(r, 0x20);
                     *ch::img(w.side, 0x004f3eec) = r();
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r() % 6 == 0 ? 0u : ch::addr(w.blocks[0].w.data()), r(), nullptr, 0);
                     vt::log().push_back(*ch::img(w.side, 0x004f3eec));
                     return eax;
                 },
                 false});
    return c;
}
}  // namespace

TEST(native_app_probe_and_stdio_redirect_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    static void* iob = s_iob;
    Bind b[] = {
        {0x004cc0c0, recoil::g_Iat_GetVersionExA_004cc0c0, reinterpret_cast<void*>(&f_version)},
        {0x004cc0b8, recoil::g_Iat_LoadLibraryA_004cc0b8, reinterpret_cast<void*>(&f_load)},
        {0x004cc0bc, recoil::g_Iat_GetProcAddress_004cc0bc, reinterpret_cast<void*>(&f_proc)},
        {0x004cc16c, recoil::g_Iat_FreeLibrary_004cc16c, reinterpret_cast<void*>(&f_free_lib)},
        {0x004cc0b4, recoil::g_Iat_OutputDebugStringA_004cc0b4, reinterpret_cast<void*>(&f_debug)},
        {0x004cc51c, recoil::g_Iat_freopen_004cc51c, reinterpret_cast<void*>(&f_freopen)},
        {0x004cc13c, recoil::g_Iat_GetTempPathA_004cc13c, reinterpret_cast<void*>(&f_temp)},
        {0x004cc5bc, recoil::g_Iat_fprintf_004cc5bc, reinterpret_cast<void*>(&f_fprintf)},
        {0x004cc4fc, recoil::g_Iat_fflush_004cc4fc, reinterpret_cast<void*>(&f_fflush)},
        {0x004cc4f8, recoil::g_Iat__iob_004cc4f8, iob},
    };
    (void)b;
    CHECK_EQ(run_cases(cases(), "app probe / stdio redirect"), 0);
}
