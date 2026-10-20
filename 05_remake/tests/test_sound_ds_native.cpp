// Structured native L1s for the sound functions ported in the cloud that the arena fuzz cannot drive - calls through
// DirectSound / A3D object tables (fake-table objects here, tests/fake_vtable.h), a settings-list walk, a search of the
// bank vector, a RIFF chunk walk over a buffer the header sizes - run by tests/vt_runner.h:
//  - SoundVoice_AdjustAndReplay_DS (0x004a0400; ECX resource, EDX voice, stack restart flag, delta; ret 8): SetVolume
//    (slot 0x3c) with the accumulated attenuation [v+0x24], SetCurrentPosition (0x34), Play (0x30) with the looping bit
//    of [R+8]; each HRESULT drawn from the seed (a failure goes to Sound_ReportDirectSoundError);
//  - SoundBank_FindByName (0x004a0920; ECX name): the provider vector [0x0056b294, 0x0056b298) of banks named by
//    [bank+0], names from a small pool with repeats and near misses;
//  - zSndInit (0x004a12c0; ECX stored): the guard [0x0056b2a0], the mode [0x0056b2ac], the settings list [0x0056bcd0]
//    holding 0..3 of the three names it looks up (the image's strings 0x004da798 / 0x004da7b0 / 0x004da7a4) among
//    others; every global it writes compared;
//  - Sound_SetListenerFromMatrix (0x004a2950; ECX matrix or 0, EDX velocity or 0): modes 0 / 1 / 2, the listener
//    [0x0056b2b4] a fake object or 0 (slots 0xc, 0x2c, 0x3c);
//  - SoundBuffer_GetPlayPosition (0x004a3620; ECX resource): busy / idle, modes 0 / 1 / 2, the buffer's
//    GetCurrentPosition (slot 0x10) or the A3D slot 0x4c writing the positions and returning an HRESULT;
//  - Wav_ParseRiffChunks (0x004a5460; ECX record, [+0xc] the file image): RIFF/WAVE images with fmt / data / cue /
//    other chunks, odd sizes, a short fmt, repeated chunks, a bad tag now and then;
//  - SoundResource_ReleaseBuffers (0x004a3690; ECX resource or 0): the six owned strings / arrays, the duplicate-voice
//    records [R+0x84] (count [R+0x80]) with their buffers, the main buffer [R+0x4c]; modes 0 / 1 / 2; the A3D slot
//    0x18 HRESULT drawn from the seed;
//  - SoundFile_Destroy (0x004a5440): SoundFile_Unload, then the path [+4];
//  - SoundBank_ReleaseAllBuffers (0x004a0e40; ECX bank or 0): ReleaseBuffers on each 0xB8-byte descriptor [bank+8]
//    below the count [bank+4] when the loaded flag [bank+0xc] is set; SoundVoiceSet_Destroy (0x004a3910):
//    ReleaseBuffers, then free(ECX); SoundBank_Destroy (0x004a0c00): ReleaseAllBuffers, free the descriptors and the
//    name; SoundBank_FindAndReleaseBuffers (0x004a0870; ECX name): FindByName over a vector of such banks, then
//    ReleaseAllBuffers on the match (or 0).
//  - SoundBuffer_Create_DirectSound (0x004a3180; ECX resource, EDX descriptor): CreateSoundBuffer on the device
//    [0x0056b2b0] (slot 0xc; the DSBUFFERDESC logged) with the flags built from [R+0xc], GetCaps (0x24; lost flag
//    drawn), Restore (0x50), Lock (0x2c; one or two pieces), the copy of the data [desc+0x20], Unlock (0x4c),
//    SetCurrentPosition (0x34), then the marker arrays from the cue records [desc+0x1c] (count [desc+0x18]) against
//    the format [desc+0x14]; each HRESULT drawn from the seed; malloc bound to a fake that hands out test blocks.
// free is bound in both import slots to a fake that logs the pointer (normalised) and frees nothing.
// Compared: the call log, EAX, every block word (pointers into blocks as (block, offset)), the globals written.
#include "test.h"
#include "vt_runner.h"
#include "GameZRecoil/zSound/zsnd_cd.h"
#include "GameZRecoil/zSound/zsnd_create.h"
#include "GameZRecoil/zSound/zsnd_grp.h"
#include "GameZRecoil/zSound/zsnd_parm.h"
#include "platform/iat_msvcrt.h"

using namespace vtr;

namespace {
using U = std::uint32_t;

U hres(std::mt19937& r) { return r() % 4 ? 0u : (r() % 2 ? 0x88780078u : 0x80004005u); }
void log_globals(World& w, U lo, U hi)
{
    for (U va = lo; va < hi; va += 4) vt::log().push_back(w.norm(*ch::img(w.side, va)));
}
// a string block
U str(World& w, std::mt19937& r, const char* s)
{
    const U p = w.add(r, 0x20);
    std::memset(w.at(p), 0, 0x20);
    std::memcpy(w.at(p), s, std::strlen(s) + 1);
    return p;
}
const char* pool(std::mt19937& r)
{
    static const char* names[] = {"engine", "Engine", "engines", "weapons", "voice", "amb", "", "weapon"};
    return names[r() % 8];
}

void __cdecl f_free(void* p) { vt::log().push_back(0xF4EE); vt::log().push_back(vt::norm()(ch::addr(p))); }
struct BindFree {
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* saved[2];
    BindFree() { saved[0] = *o; saved[1] = recoil::g_Iat_free_004cc5b4; *o = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&f_free); }
    ~BindFree() { *o = saved[0]; recoil::g_Iat_free_004cc5b4 = saved[1]; }
};

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
    return ch::at(cur_world()->add(r, static_cast<U>(n)));
}
struct BindMalloc {
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5dc));
    void* saved[2];
    BindMalloc() { saved[0] = *o; saved[1] = recoil::g_Iat_malloc_004cc5dc; *o = recoil::g_Iat_malloc_004cc5dc = reinterpret_cast<void*>(&f_malloc); }
    ~BindMalloc() { *o = saved[0]; recoil::g_Iat_malloc_004cc5dc = saved[1]; }
};
// the Lock pieces this call hands out: {p1, n1, p2, n2}
U* lock_pieces()
{
    static U p[4];
    return p;
}

std::vector<Case> cases()
{
    std::vector<Case> c;
    c.push_back({"SoundVoice_AdjustAndReplay_DS", 0x004a0400, (void*)&recoil::SoundVoice_AdjustAndReplay_DS, 400,
                 [](std::mt19937& r) {
                     vt::set(0x3c, 2, 2, 0, hres(r));
                     vt::set(0x34, 2, 2, 0, hres(r));
                     vt::set(0x30, 4, 4, 0, hres(r));
                 },
                 [](World& w, std::mt19937& r) {
                     w.add(r, 0x10);                      // block 0: resource ([+8] flags)
                     const U v = w.add(r, 0x30);          // block 1: voice
                     w.at(v)[0x24 / 4] = static_cast<U>(-static_cast<int>(r() % 10000));
                     w.at(v)[2] = object(w, r, 0x10);     // block 2: the buffer
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     U a[2] = {r() % 2, r() % 3 == 0 ? 0u : static_cast<U>(static_cast<int>(r() % 4000) - 3000)};
                     return call_any(fn, ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), a, 2);
                 }});
    c.push_back({"SoundBank_FindByName", 0x004a0920, (void*)&recoil::SoundBank_FindByName, 400, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U n = r() % 6;
                     const U vec = w.add(r, 4 * 8);       // block 0: the vector storage
                     for (U k = 0; k < n; ++k) {
                         const U bank = w.add(r, 0x10);
                         w.at(bank)[0] = str(w, r, pool(r));
                         w.at(vec)[k] = bank;
                     }
                     str(w, r, pool(r));                  // the name searched for (last block)
                     *ch::img(w.side, 0x0056b294) = n || r() % 2 ? vec : 0;
                     *ch::img(w.side, 0x0056b298) = *ch::img(w.side, 0x0056b294) + 4 * n;
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     return call_any(fn, ch::addr(w.blocks.back().w.data()), r(), nullptr, 0);
                 }});
    c.push_back({"zSndInit", 0x004a12c0, (void*)&recoil::zSndInit, 300, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     static const U names[3] = {0x004da798, 0x004da7b0, 0x004da7a4};
                     // settings nodes: value +0, name +0x10, next +0x18
                     U head = 0;
                     const U n = r() % 6;
                     for (U k = 0; k < n; ++k) {
                         const U node = w.add(r, 0x20);
                         w.at(node)[0x10 / 4] = r() % 3 ? names[r() % 3] : str(w, r, pool(r));
                         w.at(node)[0x18 / 4] = head;
                         head = node;
                     }
                     *ch::img(w.side, 0x0056bcd0) = head;
                     *ch::img(w.side, 0x0056b2a0) = r() % 5 == 0 ? 1u : 0u;
                     *ch::img(w.side, 0x0056b2ac) = r() % 3;
                     for (U va = 0x0056b2a4; va < 0x0056b3d4; va += 4) *ch::img(w.side, va) = r();
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U eax = call_any(fn, r(), r(), nullptr, 0);
                     log_globals(w, 0x0056b2a0, 0x0056b3d4);
                     return eax;
                 }});
    c.push_back({"Sound_SetListenerFromMatrix", 0x004a2950, (void*)&recoil::Sound_SetListenerFromMatrix, 400,
                 [](std::mt19937&) { vt::set(0xc, 4); vt::set(0x2c, 7); vt::set(0x3c, 4); },
                 [](World& w, std::mt19937& r) {
                     const U m = w.add(r, 0x30);           // block 0: matrix
                     for (int k = 0; k < 12; ++k) w.at(m)[k] = fbits(static_cast<float>(static_cast<int>(r() % 2000) - 1000) / 7.0f);
                     const U v = w.add(r, 0xc);            // block 1: velocity
                     for (int k = 0; k < 3; ++k) w.at(v)[k] = fbits(static_cast<float>(static_cast<int>(r() % 200) - 100) / 3.0f);
                     const U l = object(w, r, 0x10);       // block 2: listener
                     *ch::img(w.side, 0x0056b2ac) = r() % 3;
                     *ch::img(w.side, 0x0056b2b4) = r() % 4 ? l : 0;
                     for (U va = 0x0056b36c; va < 0x0056b3ac; va += 4) *ch::img(w.side, va) = r();
                 },
                 [](World& w, U fn, std::mt19937& r) {
                     const U m = r() % 5 ? ch::addr(w.blocks[0].w.data()) : 0u, v = r() % 5 ? ch::addr(w.blocks[1].w.data()) : 0u;
                     const U eax = call_any(fn, m, v, nullptr, 0);
                     log_globals(w, 0x0056b36c, 0x0056b3ac);
                     return eax;
                 }});
    c.push_back({"SoundBuffer_GetPlayPosition", 0x004a3620, (void*)&recoil::SoundBuffer_GetPlayPosition, 400,
                 [](std::mt19937& r) {
                     const U h = hres(r), p0 = r() % 0x10000, p1 = r() % 0x10000;
                     vt::set(0x10, 3);
                     vt::slots()[4].fn = [h, p0, p1](U, const U* a) -> U { *ch::at(a[1]) = p0; *ch::at(a[2]) = p1; return h; };
                     vt::set(0x4c, 2);
                     vt::slots()[0x13].fn = [h, p0](U, const U* a) -> U { *ch::at(a[1]) = p0; return h; };
                 },
                 [](World& w, std::mt19937& r) {
                     const U res = w.add(r, 0x50);         // block 0: resource
                     w.at(res)[0] = r() % 4 == 0 ? 1u : 0u;
                     w.at(res)[0x4c / 4] = object(w, r, 0x10);
                     *ch::img(w.side, 0x0056b2ac) = r() % 3;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, nullptr, 0); }});
    c.push_back({"Wav_ParseRiffChunks", 0x004a5460, (void*)&recoil::Wav_ParseRiffChunks, 800, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U rec = w.add(r, 0x24);         // block 0: the record
                     const U img = w.add(r, 0x400);        // block 1: the file image
                     unsigned char* b = reinterpret_cast<unsigned char*>(w.at(img));
                     auto put = [b](U at, U v) { std::memcpy(b + at, &v, 4); };
                     U at = 12;
                     const U nch = r() % 6;
                     static const U tags[5] = {0x20746d66, 0x61746164, 0x20657563, 0x5453494c, 0x6b6e756a};  // fmt data cue LIST junk
                     for (U k = 0; k < nch && at < 0x300; ++k) {
                         const U tag = tags[r() % 5];
                         U size = tag == 0x20746d66 ? (r() % 4 ? 0x10u : r() % 0x14) : r() % 0x60;
                         put(at, tag);
                         put(at + 4, size);
                         at += 8 + ((size + 1) & ~1u);
                     }
                     put(0, r() % 10 ? 0x46464952u : 0x58464952u);  // RIFF / RIFX
                     put(4, r() % 8 ? at - 8 : r() % 0x100);
                     put(8, r() % 10 ? 0x45564157u : 0x20495641u);  // WAVE / AVI
                     w.at(rec)[3] = r() % 10 ? img : 0u;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, nullptr, 0); }});
    c.push_back({"SoundResource_ReleaseBuffers", 0x004a3690, (void*)&recoil::SoundResource_ReleaseBuffers, 500,
                 [](std::mt19937& r) { vt::set(8, 1, 1, 0, r() % 3); vt::set(0x18, 1, 1, 0, r() % 3 ? 0u : 0x80004005u); },
                 [](World& w, std::mt19937& r) {
                     const U res = w.add(r, 0xb0);         // block 0: resource
                     w.at(res)[0] = r() % 5 == 0 ? 1u : 0u;
                     for (U off : {0x38u, 0x3cu, 0x40u, 0x88u, 0x98u, 0xa8u}) w.at(res)[off / 4] = r() % 3 ? w.add(r, 8) : 0u;
                     const U n = r() % 4;
                     w.at(res)[0x80 / 4] = n;
                     const U arr = w.add(r, 16);
                     w.at(res)[0x84 / 4] = n || r() % 2 ? arr : 0u;
                     for (U k = 0; k < n; ++k) {
                         const U rec = w.add(r, 0x3c);
                         w.at(rec)[2] = r() % 4 ? object(w, r, 0x10) : 0u;
                         w.at(arr)[k] = rec;
                     }
                     w.at(res)[0x4c / 4] = r() % 4 ? object(w, r, 0x10) : 0u;
                     *ch::img(w.side, 0x0056b2ac) = r() % 3;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_any(fn, r() % 10 ? ch::addr(w.blocks[0].w.data()) : 0u, 0, nullptr, 0); }});
    c.push_back({"SoundFile_Destroy", 0x004a5440, (void*)&recoil::SoundFile_Destroy, 200, [](std::mt19937&) {},
                 [](World& w, std::mt19937& r) {
                     const U rec = w.add(r, 0x24);         // block 0: the record
                     w.at(rec)[0] = r() % 3 ? 1u : 0u;
                     w.at(rec)[1] = r() % 4 ? w.add(r, 0x10) : 0u;
                     w.at(rec)[3] = r() % 4 ? w.add(r, 0x10) : 0u;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, nullptr, 0); }});
    auto descriptor = [](World& w, std::mt19937& r, U d) {
        w.at(d)[0] = r() % 6 == 0 ? 1u : 0u;
        for (U off : {0x38u, 0x3cu, 0x40u, 0x88u, 0x98u, 0xa8u}) w.at(d)[off / 4] = r() % 3 == 0 ? w.add(r, 8) : 0u;
        const U n = r() % 3;
        w.at(d)[0x80 / 4] = n;
        const U arr = n ? w.add(r, 12) : 0u;
        w.at(d)[0x84 / 4] = arr;
        for (U k = 0; k < n; ++k) {
            const U rec = w.add(r, 0x3c);
            w.at(rec)[2] = r() % 2 ? object(w, r, 0x10) : 0u;
            w.at(arr)[k] = rec;
        }
        w.at(d)[0x4c / 4] = r() % 2 ? object(w, r, 0x10) : 0u;
    };
    c.push_back({"SoundBank_ReleaseAllBuffers", 0x004a0e40, (void*)&recoil::SoundBank_ReleaseAllBuffers, 300,
                 [](std::mt19937& r) { vt::set(8, 1, 1, 0, r() % 3); vt::set(0x18, 1, 1, 0, r() % 3 ? 0u : 0x80004005u); },
                 [descriptor](World& w, std::mt19937& r) {
                     const U bank = w.add(r, 0x10);        // block 0: bank
                     const U n = r() % 3;
                     const U ds = w.add(r, 0xb8 * 2);      // block 1: descriptors
                     for (U k = 0; k < n; ++k) {
                         for (U i = 0; i < 0xb8 / 4; ++i) w.at(ds)[k * 0xb8 / 4 + i] = 0;
                         descriptor(w, r, ds + k * 0xb8);
                     }
                     w.at(bank)[1] = n;
                     w.at(bank)[2] = ds;
                     w.at(bank)[3] = r() % 4 ? 1u : 0u;
                     *ch::img(w.side, 0x0056b2ac) = r() % 3;
                 },
                 [](World& w, U fn, std::mt19937& r) { return call_any(fn, r() % 10 ? ch::addr(w.blocks[0].w.data()) : 0u, 0, nullptr, 0); }});
    c.push_back({"SoundVoiceSet_Destroy", 0x004a3910, (void*)&recoil::SoundVoiceSet_Destroy, 200,
                 [](std::mt19937& r) { vt::set(8, 1, 1, 0, r() % 3); vt::set(0x18, 1, 1, 0, r() % 3 ? 0u : 0x80004005u); },
                 [descriptor](World& w, std::mt19937& r) {
                     const U d = w.add(r, 0xb8);           // block 0: the voice set
                     for (U i = 0; i < 0xb8 / 4; ++i) w.at(d)[i] = 0;
                     descriptor(w, r, d);
                     *ch::img(w.side, 0x0056b2ac) = r() % 3;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, nullptr, 0); },
                 false});
    // a bank: name [+0], count [+4], descriptors [+8], loaded [+0xc]
    auto bank = [descriptor](World& w, std::mt19937& r) {
        const U b = w.add(r, 0x10);
        const U n = r() % 3;
        const U ds = w.add(r, 0xb8 * 2);
        for (U k = 0; k < n; ++k) {
            for (U i = 0; i < 0xb8 / 4; ++i) w.at(ds)[k * 0xb8 / 4 + i] = 0;
            descriptor(w, r, ds + k * 0xb8);
        }
        w.at(b)[0] = r() % 5 ? str(w, r, pool(r)) : 0u;
        w.at(b)[1] = n;
        w.at(b)[2] = r() % 5 || n ? ds : 0u;
        w.at(b)[3] = r() % 4 ? 1u : 0u;
        return b;
    };
    c.push_back({"SoundBank_Destroy", 0x004a0c00, (void*)&recoil::SoundBank_Destroy, 300,
                 [](std::mt19937& r) { vt::set(8, 1, 1, 0, r() % 3); vt::set(0x18, 1, 1, 0, r() % 3 ? 0u : 0x80004005u); },
                 [bank](World& w, std::mt19937& r) { bank(w, r); *ch::img(w.side, 0x0056b2ac) = r() % 3; },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, ch::addr(w.blocks[0].w.data()), 0, nullptr, 0); },
                 false});
    c.push_back({"SoundBank_FindAndReleaseBuffers", 0x004a0870, (void*)&recoil::SoundBank_FindAndReleaseBuffers, 300,
                 [](std::mt19937& r) { vt::set(8, 1, 1, 0, r() % 3); vt::set(0x18, 1, 1, 0, r() % 3 ? 0u : 0x80004005u); },
                 [bank](World& w, std::mt19937& r) {
                     const U vec = w.add(r, 4 * 4);        // block 0: the vector storage
                     const U n = r() % 4;
                     for (U k = 0; k < n; ++k) {
                         const U b = bank(w, r);
                         if (!w.at(b)[0]) w.at(b)[0] = str(w, r, pool(r));
                         w.at(vec)[k] = b;
                     }
                     str(w, r, pool(r));                   // the name searched for (last block)
                     *ch::img(w.side, 0x0056b294) = vec;
                     *ch::img(w.side, 0x0056b298) = vec + 4 * n;
                     *ch::img(w.side, 0x0056b2ac) = r() % 3;
                 },
                 [](World& w, U fn, std::mt19937&) { return call_any(fn, ch::addr(w.blocks.back().w.data()), 0, nullptr, 0); }});
    c.push_back({"SoundBuffer_Create_DirectSound", 0x004a3180, (void*)&recoil::SoundBuffer_Create_DirectSound, 800,
                 [](std::mt19937& r) {
                     // CreateSoundBuffer (this, desc, out, outer): the description's 5 words, the buffer handed out
                     const U hc = hres(r);
                     vt::set(0xc, 4);
                     vt::slots()[3].fn = [hc](U, const U* a) -> U {
                         for (int k = 0; k < 5; ++k) vt::log().push_back(vt::norm()(ch::at(a[1])[k]));
                         if (!hc) *ch::at(a[2]) = ch::addr(cur_world()->blocks[2].w.data());
                         return hc;
                     };
                     const U caps = r() % 4 == 0 ? 2u : 0u, hg = r() % 6 ? 0u : 0x80004005u;
                     vt::set(0x24, 2);
                     vt::slots()[9].fn = [caps, hg](U, const U* a) -> U { *ch::at(a[1]) = caps; return hg; };
                     vt::set(0x50, 1, 1, 0, r() % 5 ? 0u : 0x88780096u);
                     const U hl = r() % 6 ? 0u : 0x80004005u;
                     vt::set(0x2c, 8);
                     vt::slots()[0xb].fn = [hl](U, const U* a) -> U {
                         for (int k = 0; k < 4; ++k) *ch::at(a[3 + k]) = lock_pieces()[k];
                         return hl;
                     };
                     vt::set(0x4c, 5, 5, 0, r() % 6 ? 0u : 0x80004005u);
                     vt::set(0x34, 2, 2, 0, r() % 6 ? 0u : 0x80004005u);
                 },
                 [](World& w, std::mt19937& r) {
                     cur_world() = &w;
                     const U res = w.add(r, 0x50);         // block 0: resource
                     w.at(res)[0] = r() % 6 == 0 ? 1u : 0u;
                     const U d = w.add(r, 0x24);           // block 1: descriptor
                     object(w, r, 0x10);                   // block 2: the buffer
                     const U dev = object(w, r, 0x10);     // block 3: the device
                     *ch::img(w.side, 0x0056b2b0) = dev;
                     const U size = 4 + r() % 60;
                     w.at(d)[1] = str(w, r, "boom.wav");
                     w.at(d)[4] = size;
                     const U fmt = w.add(r, 0x14);         // WAVEFORMATEX: channels +2, rate +4, avg +8, bits +0xe
                     reinterpret_cast<std::uint16_t*>(w.at(fmt))[1] = static_cast<std::uint16_t>(1 + r() % 2);
                     w.at(fmt)[1] = 8000 + r() % 40000;
                     w.at(fmt)[2] = 1 + r() % 200000;
                     reinterpret_cast<std::uint16_t*>(w.at(fmt))[7] = static_cast<std::uint16_t>(r() % 2 ? 16 : 8);
                     w.at(d)[5] = fmt;
                     const U ncue = r() % 4;
                     w.at(d)[6] = r() % 5 ? ncue : 0u;
                     w.at(d)[7] = w.add(r, 0x18 * 4 + 4);  // the cue records
                     w.at(d)[8] = w.add(r, size);          // the data
                     const U n1 = r() % 3 ? size : r() % size;
                     lock_pieces()[0] = w.add(r, n1 + 4);
                     lock_pieces()[1] = n1;
                     lock_pieces()[2] = n1 < size ? w.add(r, size - n1 + 4) : 0u;
                     lock_pieces()[3] = size - n1;
                 },
                 [](World& w, U fn, std::mt19937&) {
                     BindMalloc bm;
                     const U eax = call_any(fn, ch::addr(w.blocks[0].w.data()), ch::addr(w.blocks[1].w.data()), nullptr, 0);
                     cur_world() = nullptr;
                     return eax;
                 }});
    return c;
}
}  // namespace

TEST(native_sound_ds_listener_riff_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    BindFree bf;
    CHECK_EQ(run_cases(cases(), "sound DirectSound / listener / RIFF"), 0);
}
