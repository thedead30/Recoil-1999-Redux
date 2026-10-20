// Structured native L1s for the app functions ported in the cloud that call through COM tables or KERNEL32 / msvcrt
// import slots, which the arena fuzz cannot drive (tests/fake_vtable.h, tests/vt_runner.h). Every import slot a
// function uses is bound, on both sides (the original's slot and the port's g_Iat_ variable), to a fake here that logs
// its arguments (pointers through the World normaliser, strings by content) and returns a value the case chose:
//  - Atl_InternalQueryInterface (0x0042db50; stdcall pThis, entry table {iid*, dw, func}, iid, ppv; ret 0x10): the
//    IUnknown fast path, simple-offset entries (func 1), blind / matching entries whose func is a fake-table thunk
//    returning S_OK / E_NOINTERFACE / 1, a null ppv; AddRef is fake slot 1;
//  - App_Forward_0042db50_004427d0 (0x004427d0): the same with the image's entry table 0x004d1fc8 (one blind
//    simple-offset entry);
//  - ComPtr_Release (0x0042de00) / ComPtr_StopAndReset (0x0042faa0): the holder's object's slots 2 / 8 and 7;
//  - DsBuffer_Init (0x0042dda0; InitializeCriticalSection x3), CritSec_Delete (0x00442860; DeleteCriticalSection,
//    null ECX too), App_Callback_ImportCall_00442770 (InterlockedIncrement on arg+4);
//  - Messages_LoadDll (0x004a5ad0; LoadLibraryA, GetProcAddress "ZLocGetID") and App_FreeLoadedLibrary (0x004a5b00;
//    FreeLibrary), with the handle globals 0x0056b670 / 0x0056b568;
//  - App_ExitProcess (0x004a5980): Stub_Ret, _fcloseall, ExitProcess(ECX) - the fake ExitProcess logs its code and
//    longjmps back to the case (the original never returns from it).
// Compared: the call log, EAX, every block word, the globals the functions write.
#include "test.h"
#include "vt_runner.h"
#include "platform/iat_kernel32.h"
#include "platform/iat_msvcrt.h"
#include "unattributed/app.h"

#include <csetjmp>

using namespace vtr;

namespace {
using U = std::uint32_t;

// the handle the fake LoadLibraryA returns in this call, and the ExitProcess escape
U& lib_handle()
{
    static U h = 0;
    return h;
}
std::jmp_buf& exit_jb()
{
    static std::jmp_buf jb;
    return jb;
}
void log_str(U p)
{
    const char* s = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(p));
    U h = 0;
    for (int k = 0; s && s[k] && k < 64; ++k) h = h * 31 + static_cast<unsigned char>(s[k]);
    vt::log().push_back(h);
}
void __stdcall f_init_cs(void* p) { vt::log().push_back(0xC5); vt::log().push_back(vt::norm()(ch::addr(p))); }
void __stdcall f_delete_cs(void* p) { vt::log().push_back(0xDC); vt::log().push_back(vt::norm()(ch::addr(p))); }
long __stdcall f_inc(long* p)
{
    vt::log().push_back(0x11);
    vt::log().push_back(vt::norm()(ch::addr(p)));
    return ++*p;
}
U __stdcall f_load(const char* name)
{
    vt::log().push_back(0x1B);
    vt::log().push_back(vt::norm()(ch::addr(name)));
    log_str(ch::addr(name));
    return lib_handle();
}
U __stdcall f_proc(U h, const char* name)
{
    vt::log().push_back(0x9A);
    vt::log().push_back(h);
    vt::log().push_back(vt::norm()(ch::addr(name)));
    log_str(ch::addr(name));
    return 0x7770000;
}
int __stdcall f_free_lib(U h) { vt::log().push_back(0xF1); vt::log().push_back(h); return 1; }
int __cdecl f_fcloseall() { vt::log().push_back(0xFC); return 3; }
void __stdcall f_exit(U code)
{
    vt::log().push_back(0xE7);
    vt::log().push_back(code);
    std::longjmp(exit_jb(), 1);
}

// binds one fake into the original's slot and the port's variable while alive
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

U log_globals(int side, U eax, std::initializer_list<U> vas)
{
    for (U va : vas) vt::log().push_back(*ch::img(side, va));
    return eax;
}

std::vector<Case> cases()
{
    std::vector<Case> c;
    c.push_back({"Atl_InternalQueryInterface", 0x0042db50, (void*)&recoil::Atl_InternalQueryInterface, 800,
                 [](std::mt19937& r) {
                     vt::set(4, 1, 1, 0, 2);  // AddRef
                     const U rets[3] = {0, 0x80004002, 1};
                     for (int k = 10; k < 13; ++k) vt::set(k * 4, 4, 4, 0, rets[r() % 3]);
                 },
                 [](World& w, std::mt19937& r) {
                     const U obj = w.add(r, 0x40);                // block 0: pThis, the fake table in every word
                     for (int k = 0; k < 16; ++k) w.at(obj)[k] = vt::table();
                     const U iids = w.add(r, 16 * 5, true);       // block 1: five GUIDs, the first IUnknown
                     w.at(iids)[0] = 0; w.at(iids)[1] = 0; w.at(iids)[2] = 0xc0; w.at(iids)[3] = 0x46000000;
                     if (r() % 4 == 0) std::memcpy(w.at(iids) + 8, w.at(iids) + 4, 16);  // two equal GUIDs
                     const U n = 1 + r() % 4;
                     const U ent = w.add(r, 12 * (n + 1));        // block 2: the entry table
                     for (U e = 0; e < n; ++e) {
                         U* p = w.at(ent) + 3 * e;
                         p[0] = r() % 6 == 0 ? 0 : iids + 16 * (1 + r() % 4);
                         p[1] = 4 * (r() % 16);
                         const U f = r() % 4;
                         p[2] = f == 0 ? 1 : ch::at(vt::table())[10 + f - 1];
                         if (p[0] == 0 && p[2] == 1) p[2] = ch::at(vt::table())[10];
                     }
                     w.at(ent)[3 * n + 2] = 0;
                     if (r() % 5 == 0) w.at(ent)[2] = 0;          // an empty table
                     w.add(r, 8);                                 // block 3: ppv
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U iids = ch::addr(w.blocks[1].w.data());
                     const U k = r() % 7;
                     const U iid = k < 5 ? iids + 16 * k : ch::addr(w.blocks[1].w.data()) + 4;
                     U a[4] = {ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[2].w.data()), iid,
                               r() % 8 == 0 ? 0u : ch::addr(w.blocks[3].w.data())};
                     return call_any(fn, r(), r(), a, 4);
                 }});
    c.push_back({"App_Forward_0042db50_004427d0", 0x004427d0, (void*)&recoil::App_Forward_0042db50_004427d0, 200,
                 [](std::mt19937&) { vt::set(4, 1, 1, 0, 2); },
                 [](World& w, std::mt19937& r) {
                     const U obj = w.add(r, 0x10);                // block 0: pThis
                     w.at(obj)[0] = vt::table();
                     const U iid = w.add(r, 16, true);            // block 1: IUnknown or another GUID
                     if (r() % 2) { w.at(iid)[0] = 0; w.at(iid)[1] = 0; w.at(iid)[2] = 0xc0; w.at(iid)[3] = 0x46000000; }
                     w.add(r, 4);                                 // block 2: ppv
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[3] = {ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), r() % 8 == 0 ? 0u : ch::addr(w.blocks[2].w.data())};
                     return call_any(fn, r(), r(), a, 3);
                 }});
    c.push_back({"ComPtr_Release", 0x0042de00, (void*)&recoil::ComPtr_Release, 200,
                 [](std::mt19937& r) { vt::set(8, 1, 1, 0, r() % 5); },
                 [](World& w, std::mt19937& r) {
                     const U h = w.add(r, 4);
                     const U o = object(w, r, 0x20);
                     w.at(h)[0] = r() % 4 == 0 ? 0 : o;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_any(fn, ch::addr(w.blocks[0].w.data()), r(), nullptr, 0); }});
    c.push_back({"ComPtr_StopAndReset", 0x0042faa0, (void*)&recoil::ComPtr_StopAndReset, 200,
                 [](std::mt19937& r) { vt::set(0x20, 1, 1, 0, r() % 5); vt::set(0x1c, 3, 3, 0, r() % 5); },
                 [](World& w, std::mt19937& r) {
                     const U h = w.add(r, 4);
                     const U o = object(w, r, 0x20);
                     w.at(h)[0] = r() % 4 == 0 ? 0 : o;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_any(fn, ch::addr(w.blocks[0].w.data()), r(), nullptr, 0); }});
    c.push_back({"DsBuffer_Init", 0x0042dda0, (void*)&recoil::DsBuffer_Init, 300, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U p = w.add(r, 0x70);
                     w.at(p)[0] = r() % 2 ? r() % 0x100 : r();
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[3] = {r() % 6 == 0 ? 0u : ch::addr(w.blocks[0].w.data()), r(), r()};
                     return call_any(fn, r(), r(), a, 3);
                 }});
    c.push_back({"CritSec_Delete", 0x00442860, (void*)&recoil::CritSec_Delete, 100, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) { w.add(r, 0x20); },
                 [](World& w, U fn, std::mt19937& r) {
                     return call_any(fn, r() % 4 == 0 ? 0u : ch::addr(w.blocks[0].w.data()), r(), nullptr, 0);
                 },
                 false});
    c.push_back({"App_Callback_ImportCall_00442770", 0x00442770, (void*)&recoil::App_Callback_ImportCall_00442770, 100,
                 [](std::mt19937&) {}, [](World& w, std::mt19937& r) { w.add(r, 0x10); },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[1] = {ch::addr(w.blocks[0].w.data())};
                     return call_any(fn, r(), r(), a, 1);
                 }});
    c.push_back({"Messages_LoadDll", 0x004a5ad0, (void*)&recoil::Messages_LoadDll, 100, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U s = w.add(r, 0x20);
                     const char* nm = r() % 2 ? "messages.dll" : "loc\\msg_de.dll";
                     std::memcpy(w.at(s), nm, std::strlen(nm) + 1);
                     lib_handle() = r() % 3 == 0 ? 0 : 0x10000000 + (r() % 0x100) * 0x10000;
                     *ch::img(w.side, 0x0056b670) = r();
                     *ch::img(w.side, 0x0056b568) = r();
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     return log_globals(w.side, call_any(fn, ch::addr(w.blocks[0].w.data()), r(), nullptr, 0), {0x0056b670, 0x0056b568});
                 }});
    c.push_back({"App_FreeLoadedLibrary", 0x004a5b00, (void*)&recoil::App_FreeLoadedLibrary, 100, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     w.add(r, 4);
                     *ch::img(w.side, 0x0056b670) = r() % 3 == 0 ? 0 : 0x10000000 + (r() % 0x100) * 0x10000;
                 },
                 [](World& w, U fn, std::mt19937& r) { return log_globals(w.side, call_any(fn, r(), r(), nullptr, 0), {0x0056b670}); },
                 false});
    c.push_back({"App_ExitProcess", 0x004a5980, (void*)&recoil::App_ExitProcess, 50, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) { w.add(r, 4); },
                 [](World&, U fn, std::mt19937& r) -> U {
                     const U code = r();
                     if (setjmp(exit_jb()) == 0) call_any(fn, code, r(), nullptr, 0);
                     return 0;
                 },
                 false});
    return c;
}
}  // namespace

TEST(native_app_com_and_imports_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    Bind b[] = {
        {0x004cc0e8, recoil::g_Iat_InitializeCriticalSection_004cc0e8, reinterpret_cast<void*>(&f_init_cs)},
        {0x004cc0f0, recoil::g_Iat_DeleteCriticalSection_004cc0f0, reinterpret_cast<void*>(&f_delete_cs)},
        {0x004cc0d8, recoil::g_Iat_InterlockedIncrement_004cc0d8, reinterpret_cast<void*>(&f_inc)},
        {0x004cc0b8, recoil::g_Iat_LoadLibraryA_004cc0b8, reinterpret_cast<void*>(&f_load)},
        {0x004cc0bc, recoil::g_Iat_GetProcAddress_004cc0bc, reinterpret_cast<void*>(&f_proc)},
        {0x004cc16c, recoil::g_Iat_FreeLibrary_004cc16c, reinterpret_cast<void*>(&f_free_lib)},
        {0x004cc520, recoil::g_Iat__fcloseall_004cc520, reinterpret_cast<void*>(&f_fcloseall)},
        {0x004cc0ec, recoil::g_Iat_ExitProcess_004cc0ec, reinterpret_cast<void*>(&f_exit)},
    };
    (void)b;
    CHECK_EQ(run_cases(cases(), "app COM / import callers"), 0);
}
