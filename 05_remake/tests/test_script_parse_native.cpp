// Structured native L1 for the zinterp_parse.cpp functions ported in the cloud (P2 script). Interpreter object (0x100
// bytes here): +8 argc, +0xc tokenised flag, +0x10 error flag, +0x14 line, +0x1c line buffer, +0x20 argv[], +0x60 / +0x64
// variable pairs / count, +0x68 / +0x6c labels (0xc each) / count, +0x70 error hook, +0x74 search path list, +0x7c search
// set, +0x80 cache FILE, +0x84 / +0x88 cache header, +0x8c -> cache count, +0x90 cache index, +0x94 compiled mode,
// +0x9c / +0xa0 frames (0xc each) / depth. Each TEST states what it feeds and compares. The CRT calls reach the system
// msvcrt on both sides (the oracle resolves the original's msvcrt imports there); free and printf are swapped for
// logging fakes in both import slots where a test needs to see them. Heap and FILE pointers are compared by role.
#include "test.h"
#include "native_oracle.h"
#include "GameZRecoil/zInterp/zinterp_parse.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <windows.h>

#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace {
std::uint32_t* at(std::uint32_t p) { return reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(p)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
struct Crt {
    HMODULE m = GetModuleHandleA("msvcrt.dll");
    template <class F> F get(const char* n) const { return reinterpret_cast<F>(GetProcAddress(m, n)); }
};
const Crt& crt() { static Crt c; return c; }
void* c_malloc(std::size_t n) { return crt().get<void*(__cdecl*)(std::size_t)>("malloc")(n); }
void c_free(void* p) { crt().get<void(__cdecl*)(void*)>("free")(p); }
void* c_fopen(const char* p, const char* m) { return crt().get<void*(__cdecl*)(const char*, const char*)>("fopen")(p, m); }
int c_fclose(void* f) { return crt().get<int(__cdecl*)(void*)>("fclose")(f); }
long c_ftell(void* f) { return crt().get<long(__cdecl*)(void*)>("ftell")(f); }
std::size_t c_fwrite(const void* b, std::size_t s, std::size_t n, void* f) { return crt().get<std::size_t(__cdecl*)(const void*, std::size_t, std::size_t, void*)>("fwrite")(b, s, n, f); }
char* c_strdup(const char* s) { return crt().get<char*(__cdecl*)(const char*)>("_strdup")(s); }

std::string temp_path(const char* prefix)
{
    char dir[MAX_PATH], path[MAX_PATH];
    GetTempPathA(MAX_PATH, dir);
    GetTempFileNameA(dir, prefix, 0, path);
    return path;
}
void write_file(const std::string& path, const std::string& bytes)
{
    void* f = c_fopen(path.c_str(), "wb");
    if (!bytes.empty()) c_fwrite(bytes.data(), 1, bytes.size(), f);
    c_fclose(f);
}

// logging free in both slots (the original's import slot and the port's)
std::vector<std::uint32_t> g_freed;
void __cdecl logging_free(void* p) { g_freed.push_back(addr(p)); if (p) c_free(p); }
struct FreeHook {
    void** o = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc5b4));
    void* saved[2];
    FreeHook() { saved[0] = *o; saved[1] = recoil::g_Iat_free_004cc5b4; *o = recoil::g_Iat_free_004cc5b4 = reinterpret_cast<void*>(&logging_free); }
    ~FreeHook() { *o = saved[0]; recoil::g_Iat_free_004cc5b4 = saved[1]; }
};

const char* const kWords[] = {"NewWorld", "set", "x", "1.5", "\"q\"", "abc", "Camera", "on", "TRUE", "-3", "a_b"};

// the argv array (+0x20.., argc +8) as offsets into the line buffer +0x1c
void snap_argv(const std::uint32_t* obj, std::vector<std::uint32_t>& s)
{
    s.push_back(obj[2]);
    for (std::uint32_t k = 0; k < obj[2] && k < 16; ++k) s.push_back(obj[8 + k] - obj[7]);
}
}  // namespace

// Script_ReadLine (0x004c1160; ECX script, stack: FILE, line buffer; ret 8). Text mode (+0x94 == 0): fgetc into the
// buffer up to and including '\n' (no terminator written), 1 unless the FILE's EOF / error flags are set after it (0).
// Compiled mode: fread a 4-byte blob length (into the stack slot of the FILE argument) - 0 -> argc 0, buffer 0, return
// 0; else argc, malloc(length), the blob, argv = the blob's NUL-separated strings; tokenised flag +0xc = 1; 1.
// Files: text of 0..4 lines (some without a final newline, some empty lines), compiled files of 0..3 blocks always
// ended by a zero length (a failed length read leaves the FILE pointer in the length slot and the original then
// mallocs that and walks a stale argc - reading past the end is not a state the game produces). Up to 6 calls per file. Compared: every return, the buffer bytes, argc / argv offsets and the blob.
TEST(native_script_read_line_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t*, int, void*, char*);
    const Fn fn[2] = {rt::original<Fn>(0x004c1160), reinterpret_cast<Fn>(&recoil::Script_ReadLine)};
    const std::string path = temp_path("srl");
    std::mt19937 rng(0x4c1160);
    int compared = 0, lines = 0;
    for (int it = 0; it < 1500; ++it) {
        const bool compiled = it % 2;
        std::string bytes;
        if (!compiled) {
            for (int l = static_cast<int>(rng() % 5); l > 0; --l) {
                for (int w = static_cast<int>(rng() % 5); w > 0; --w) { bytes += kWords[rng() % 11]; bytes += rng() % 3 ? " " : ", "; }
                if (l > 1 || rng() % 2) bytes += "\n";
            }
        } else {
            for (int b = static_cast<int>(rng() % 4); b > 0; --b) {
                std::string blob;
                const std::uint32_t argc = 1 + rng() % 6;
                for (std::uint32_t a = 0; a < argc; ++a) { blob += kWords[rng() % 11]; blob += '\0'; }
                const std::uint32_t len = static_cast<std::uint32_t>(blob.size());
                bytes.append(reinterpret_cast<const char*>(&len), 4);
                bytes.append(reinterpret_cast<const char*>(&argc), 4);
                bytes += blob;
            }
            const std::uint32_t zero = 0;
            bytes.append(reinterpret_cast<const char*>(&zero), 4);
        }
        write_file(path, bytes);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {};
            obj[0x94 / 4] = compiled ? 1u : 0u;
            void* f = c_fopen(path.c_str(), "rb");
            for (int call = 0; call < 6; ++call) {
                char buf[256];
                std::memset(buf, 0xCD, sizeof buf);
                obj[7] = 0;
                const int r = fn[side](obj, 0, f, buf);
                snap[side].push_back(static_cast<std::uint32_t>(r));
                snap[side].push_back(static_cast<std::uint32_t>(c_ftell(f)));
                if (!compiled) {
                    for (int k = 0; k < 256; k += 4) { std::uint32_t w; std::memcpy(&w, buf + k, 4); snap[side].push_back(w); }
                } else {
                    snap[side].push_back(obj[3]);
                    snap[side].push_back(obj[7] ? 1u : 0u);
                    if (obj[7]) {
                        snap_argv(obj, snap[side]);
                        for (std::uint32_t k = 0; k < obj[2] && k < 16; ++k) for (const char* c = reinterpret_cast<const char*>(at(obj[8 + k])); *c; ++c) snap[side].push_back(static_cast<unsigned char>(*c));
                        c_free(at(obj[7]));
                    }
                }
                if (side == 0 && r) ++lines;
                if (!r) break;
            }
            c_fclose(f);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(path.c_str());
    rt::restore_pristine();
    std::printf("  Script_ReadLine files %d, %d lines read (text and compiled)\n", compared, lines);
    CHECK(lines > 1000);
}

// Script_Tokenize (0x004c13c0; ECX script, stack: line; ret 4): compiled mode -> 1 at once. Else argc = 0, flag +0xc = 1;
// the line up to a '#' is copied (malloc) or the whole line _strdup'd into +0x1c; leading iswspace skipped; tokens split
// at strpbrk(", \t\n"), each separator overwritten with NUL, the next token's leading spaces skipped; a '\n' separator
// ends the line; a non-empty tail is the last token; 1. Lines of 0..10 words with mixed separators, comments, blank
// tails. Compared: the return, argc, argv offsets, the buffer's bytes up to its length.
TEST(native_script_tokenize_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t*, int, const char*);
    const Fn fn[2] = {rt::original<Fn>(0x004c13c0), reinterpret_cast<Fn>(&recoil::Script_Tokenize)};
    const char* const seps[] = {" ", ",", ", ", "\t", "  ", " ,"};
    std::mt19937 rng(0x4c13c0);
    int compared = 0, tokens = 0;
    for (int it = 0; it < 6000; ++it) {
        std::string line;
        if (rng() % 3 == 0) line += rng() % 2 ? "  " : "\t";
        for (int w = static_cast<int>(rng() % 11); w > 0; --w) { line += kWords[rng() % 11]; if (w > 1 || rng() % 2) line += seps[rng() % 6]; }
        if (rng() % 4 == 0) line += "# comment, here";
        if (rng() % 3 == 0) line += "\n";
        const bool compiled = rng() % 20 == 0;
        const std::size_t hash = line.find('#');
        const std::size_t buf_len = (hash == std::string::npos ? line.size() : hash) + 1;  // the copy's size
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {};
            obj[0x94 / 4] = compiled ? 1u : 0u;
            const int r = fn[side](obj, 0, line.c_str());
            snap[side].push_back(static_cast<std::uint32_t>(r));
            snap[side].push_back(obj[3]);
            if (!compiled) {
                snap_argv(obj, snap[side]);
                for (std::size_t k = 0; k < buf_len; ++k) snap[side].push_back(static_cast<unsigned char>(reinterpret_cast<const char*>(at(obj[7]))[k]));
                if (side == 0) tokens += static_cast<int>(obj[2]);
                c_free(at(obj[7]));
            }
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Script_Tokenize lines %d, %d tokens\n", compared, tokens);
    CHECK(tokens > 10000);
}

// The variable / label / frame / argv helpers on a populated object:
//   Script_GetVar (0x004c15f0; name, out pair or null; ret 8) - the value of the first pair whose name matches, 0;
//   Script_FindLabel (0x004c1a40; name; ret 4) - the first 0xc-byte label entry whose name matches, or 0;
//   Script_PushFrame (0x004c18c0; three words; ret 0xc) - realloc to depth + 1 entries, append; 1;
//   Script_PopFrame (0x004c1940) - depth 0 -> 0, else depth - 1 and a pointer to that entry;
//   Script_IncLine (0x004c1b20) - +0x14 += 1;
//   Script_Argv0 (0x004c5510), Script_Argv0Equals (0x004c54b0; string; ret 4 - strcmp == 0),
//   Script_Argv0Matches (0x004c5480; string, n; ret 8 - strncmp == 0) - argc >= 1 for the last two (argc 0 hands a null
//   argv0 to the compare: a fault in the original);
//   Script_NodeGetModelIfAny (0x004c58c0; node; ret 4) - null node only: returns AL 0 (a non-null node makes
//   gwNodeGetModel write through a null out pointer - a fault in the original).
// Variables and labels from a small name pool with repeats; frames 0..4 deep in real msvcrt blocks. Compared: returns
// (pointers by role), the object's words and the frame block contents.
TEST(native_script_helpers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using GetVar = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*, std::uint32_t*);
    using Find = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*);
    using Push = int(__fastcall*)(std::uint32_t*, int, std::uint32_t, std::uint32_t, std::uint32_t);
    using Plain = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    using Match = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*, int);
    using Model = std::uint32_t(__stdcall*)(void*);
    const GetVar getvar[2] = {rt::original<GetVar>(0x004c15f0), reinterpret_cast<GetVar>(&recoil::Script_GetVar)};
    const Find find[2] = {rt::original<Find>(0x004c1a40), reinterpret_cast<Find>(&recoil::Script_FindLabel)};
    const Push push[2] = {rt::original<Push>(0x004c18c0), reinterpret_cast<Push>(&recoil::Script_PushFrame)};
    const Plain pop[2] = {rt::original<Plain>(0x004c1940), reinterpret_cast<Plain>(&recoil::Script_PopFrame)};
    const Plain inc[2] = {rt::original<Plain>(0x004c1b20), reinterpret_cast<Plain>(&recoil::Script_IncLine)};
    const Plain argv0[2] = {rt::original<Plain>(0x004c5510), reinterpret_cast<Plain>(&recoil::Script_Argv0)};
    const Find equals[2] = {rt::original<Find>(0x004c54b0), reinterpret_cast<Find>(&recoil::Script_Argv0Equals)};
    const Match matches[2] = {rt::original<Match>(0x004c5480), reinterpret_cast<Match>(&recoil::Script_Argv0Matches)};
    const Model model[2] = {rt::original<Model>(0x004c58c0), reinterpret_cast<Model>(&recoil::Script_NodeGetModelIfAny)};
    const char* const names[] = {"x", "X", "speed", "sp", "", "speedy", "label1"};
    std::mt19937 rng(0x4c15f0);
    int compared = 0, hits = 0;
    for (int it = 0; it < 20000; ++it) {
        const int f = it % 9;
        const int nv = static_cast<int>(rng() % 5), nl = static_cast<int>(rng() % 5), depth = static_cast<int>(rng() % 5);
        int vn[5], vv[5], ln[5];
        for (int k = 0; k < 5; ++k) { vn[k] = static_cast<int>(rng() % 7); vv[k] = static_cast<int>(rng() % 11); ln[k] = static_cast<int>(rng() % 7); }
        std::uint32_t frames_init[15], words[3] = {rng(), rng(), rng()}, init[0x40];
        for (auto& w : frames_init) w = rng();
        for (auto& w : init) w = rng();
        const int argc = f >= 6 && f <= 7 ? 1 + static_cast<int>(rng() % 3) : static_cast<int>(rng() % 3);
        const int a0 = static_cast<int>(rng() % 11), q = static_cast<int>(rng() % 7), n = static_cast<int>(rng() % 6);
        const char* qs = rng() % 2 ? names[q] : kWords[rng() % 11];
        std::vector<std::uint32_t> snap[2];
        const bool no_out = rng() % 3 == 0;  // drawn once for both sides (a per-side draw fed the sides different inputs)
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40];
            std::memcpy(obj, init, sizeof obj);
            std::uint32_t pairs[10], labels[15];
            for (int k = 0; k < 5; ++k) { pairs[2 * k] = addr(names[vn[k]]); pairs[2 * k + 1] = addr(kWords[vv[k]]); }
            for (int k = 0; k < 5; ++k) { labels[3 * k] = addr(names[ln[k]]); labels[3 * k + 1] = 100u + k; labels[3 * k + 2] = 200u + k; }
            obj[0x60 / 4] = addr(pairs); obj[0x64 / 4] = static_cast<std::uint32_t>(nv);
            obj[0x68 / 4] = addr(labels); obj[0x6c / 4] = static_cast<std::uint32_t>(nl);
            const std::uint32_t fb = depth ? addr(c_malloc(12 * depth)) : 0u;
            if (fb) std::memcpy(at(fb), frames_init, 12 * depth);
            obj[0x9c / 4] = fb; obj[0xa0 / 4] = static_cast<std::uint32_t>(depth);
            obj[2] = static_cast<std::uint32_t>(argc);
            obj[8] = addr(kWords[a0]);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            for (int k = 0; k < 10; ++k) role[addr(pairs + k)] = 0xA000u + k;
            for (int k = 0; k < 15; ++k) role[addr(labels + k)] = 0xB000u + k;
            for (int k = 0; k < 7; ++k) role[addr(names[k])] = 0xC000u + k;
            for (int k = 0; k < 11; ++k) role[addr(kWords[k])] = 0xD000u + k;
            auto rl = [&](std::uint32_t v) { auto x = role.find(v); return x != role.end() ? x->second : v; };
            std::uint32_t out = 0xEEEEEEEEu, r = 0;
            switch (f) {
            case 0: r = getvar[side](obj, 0, qs, no_out ? nullptr : &out); break;
            case 1: r = find[side](obj, 0, qs); break;
            case 2: r = static_cast<std::uint32_t>(push[side](obj, 0, words[0], words[1], words[2])); break;
            case 3: r = pop[side](obj, 0); break;
            case 4: inc[side](obj, 0); break;
            case 5: r = argv0[side](obj, 0); break;
            case 6: r = equals[side](obj, 0, qs); break;
            case 7: r = matches[side](obj, 0, qs, n); break;
            case 8: r = model[side](nullptr) & 0xFFu; break;
            }
            const std::uint32_t frames_now = obj[0x9c / 4];
            const bool in_frames = frames_now && r >= frames_now && r < frames_now + 12 * obj[0xa0 / 4] + 12;
            snap[side].push_back(in_frames ? 0xF000u + (r - frames_now) : rl(r));
            snap[side].push_back(rl(out));
            for (int k = 0; k < 0x40; ++k) if (k != 0x9c / 4 && k != 0x60 / 4 && k != 0x68 / 4) snap[side].push_back(rl(obj[k]));
            snap[side].push_back(frames_now == fb ? 1u : frames_now ? 2u : 0u);
            for (std::uint32_t k = 0; frames_now && k < 3 * obj[0xa0 / 4]; ++k) snap[side].push_back(at(frames_now)[k]);
            if (side == 0) hits += (f <= 1 || f >= 6) && r != 0;
            if (frames_now) c_free(at(frames_now));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  script helpers calls %d, %d lookups / compares hit\n", compared, hits);
    CHECK(hits > 1000);
}

// Script_FreeStringPairs (0x004c1670), Script_FreeLabels (0x004c16c0), Script_FreeFrames (0x004c1960): free every
// pair's name and value / every label's name, then the array (when set), and zero the pointer and count; frames: free
// the array, zero it and the depth. 0..5 real msvcrt entries. Compared: the free log by role and the object's words.
TEST(native_script_free_lists_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = int(__fastcall*)(std::uint32_t*, int);
    const Fn fns[3][2] = {{rt::original<Fn>(0x004c1670), reinterpret_cast<Fn>(&recoil::Script_FreeStringPairs)},
                          {rt::original<Fn>(0x004c16c0), reinterpret_cast<Fn>(&recoil::Script_FreeLabels)},
                          {rt::original<Fn>(0x004c1960), reinterpret_cast<Fn>(&recoil::Script_FreeFrames)}};
    FreeHook hook;
    std::mt19937 rng(0x4c1670);
    int compared = 0, freed = 0;
    for (int it = 0; it < 3000; ++it) {
        const int f = it % 3, n = static_cast<int>(rng() % 6);
        const bool has_array = n > 0 || rng() % 2;
        std::uint32_t init[0x40];
        for (auto& w : init) w = rng();
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40];
            std::memcpy(obj, init, sizeof obj);
            std::map<std::uint32_t, std::uint32_t> role{{0, 0}};
            const int stride = f == 0 ? 2 : 3;
            const std::uint32_t arr = has_array ? addr(c_malloc(4 * stride * (n ? n : 1))) : 0u;
            role[arr] = 0xA0;
            for (int k = 0; k < n; ++k) {
                if (f == 2) { at(arr)[stride * k] = 0; at(arr)[stride * k + 1] = 1; at(arr)[stride * k + 2] = 2; continue; }
                at(arr)[stride * k] = addr(c_strdup("name"));
                role[at(arr)[stride * k]] = 0xB0u + 2 * k;
                if (f == 0) { at(arr)[stride * k + 1] = addr(c_strdup("value")); role[at(arr)[stride * k + 1]] = 0xB1u + 2 * k; }
                else { at(arr)[stride * k + 1] = 1; at(arr)[stride * k + 2] = 2; }
            }
            if (f == 0) { obj[0x60 / 4] = arr; obj[0x64 / 4] = static_cast<std::uint32_t>(n); }
            if (f == 1) { obj[0x68 / 4] = arr; obj[0x6c / 4] = static_cast<std::uint32_t>(n); }
            if (f == 2) { obj[0x9c / 4] = arr; obj[0xa0 / 4] = static_cast<std::uint32_t>(n); }
            // the blocks this side allocated, recorded before the call: the call frees the array, so reading the string
            // pointers out of it afterwards read freed memory (the 0xc0000374 heap corruption)
            std::vector<std::uint32_t> mine;
            if (arr) { for (int k = 0; k < n && f < 2; ++k) { mine.push_back(at(arr)[stride * k]); if (f == 0) mine.push_back(at(arr)[stride * k + 1]); } }
            g_freed.clear();
            fns[f][side](obj, 0);
            auto rl = [&](std::uint32_t v) { auto x = role.find(v); return x != role.end() ? x->second : 0xDEADu; };
            for (std::uint32_t p : g_freed) snap[side].push_back(rl(p));
            snap[side].push_back(0xF0F0F0F0u);
            snap[side].insert(snap[side].end(), obj, obj + 0x40);
            if (side == 0) freed += static_cast<int>(g_freed.size());
            // anything the call did not free (frames hold no allocated words)
            for (std::uint32_t p : mine) { bool gone = false; for (std::uint32_t g : g_freed) gone |= g == p; if (!gone) c_free(at(p)); }
            bool arr_gone = false;
            for (std::uint32_t g : g_freed) arr_gone |= g == arr;
            if (arr && !arr_gone) c_free(at(arr));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Script free lists calls %d, %d frees\n", compared, freed);
    CHECK(freed > 3000);
}

namespace {
std::vector<std::string> g_printed;
int __cdecl logging_printf(const char* fmt, ...)
{
    char buf[512];
    va_list va;
    va_start(va, fmt);
    const int n = std::vsnprintf(buf, sizeof buf, fmt, va);
    va_end(va);
    g_printed.push_back(buf);
    return n;
}
std::vector<std::uint32_t> g_hook_log;
void __cdecl logging_hook(const char* fmt, const std::uint32_t* va)
{
    g_hook_log.push_back(addr(fmt));
    g_hook_log.push_back(va[0]);
    g_hook_log.push_back(va[1]);
}
}  // namespace

// Script_DumpArgs (0x004c1870): printf("%s"-style format 0x004e4920, argv[k]) for each argument (a null one past argc is
// never reached), then printf(0x004dd1c4). printf is swapped for a logging fake in both import slots; the printed
// strings are compared. Script_CallErrorHook (0x004c1b30; cdecl: script, format, ...): the hook +0x70 (when set) gets
// (format, pointer to the variadic arguments). Script_Fail (0x004c5520; same shape) also sets the error flag +0x10.
// Hook present or absent; two variadic words. Compared: printed strings, hook calls, the object's words.
TEST(native_script_dump_and_error_hooks_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Dump = int(__fastcall*)(std::uint32_t*, int);
    using Hook = int(__cdecl*)(std::uint32_t*, const char*, ...);
    const Dump dump[2] = {rt::original<Dump>(0x004c1870), reinterpret_cast<Dump>(&recoil::Script_DumpArgs)};
    const Hook call_hook[2] = {rt::original<Hook>(0x004c1b30), reinterpret_cast<Hook>(&recoil::Script_CallErrorHook)};
    const Hook fail[2] = {rt::original<Hook>(0x004c5520), reinterpret_cast<Hook>(&recoil::Script_Fail)};
    void** o_printf = reinterpret_cast<void**>(static_cast<std::uintptr_t>(0x004cc4dc));
    void* const saved[2] = {*o_printf, recoil::g_Iat_printf_004cc4dc};
    *o_printf = recoil::g_Iat_printf_004cc4dc = reinterpret_cast<void*>(&logging_printf);
    std::mt19937 rng(0x4c1870);
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        const int f = it % 3, argc = static_cast<int>(rng() % 6);
        int words[6];
        for (int& w : words) w = static_cast<int>(rng() % 11);
        const bool has_hook = rng() % 4 != 0;
        const std::uint32_t v0 = rng(), v1 = rng();
        std::vector<std::uint32_t> snap[2];
        std::vector<std::string> printed[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {};
            obj[2] = static_cast<std::uint32_t>(argc);
            for (int k = 0; k < argc; ++k) obj[8 + k] = addr(kWords[words[k]]);
            obj[0x70 / 4] = has_hook ? addr(reinterpret_cast<void*>(&logging_hook)) : 0u;
            g_printed.clear();
            g_hook_log.clear();
            if (f == 0) dump[side](obj, 0);
            if (f == 1) call_hook[side](obj, "fmt %d %d", v0, v1);
            if (f == 2) fail[side](obj, "fmt %d %d", v0, v1);
            printed[side] = g_printed;
            snap[side] = g_hook_log;
            snap[side].insert(snap[side].end(), obj, obj + 0x40);
        }
        CHECK(printed[0] == printed[1]);
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    *o_printf = saved[0];
    recoil::g_Iat_printf_004cc4dc = saved[1];
    rt::restore_pristine();
    std::printf("  Script_DumpArgs / CallErrorHook / Fail calls %d\n", compared);
}

// ScriptCache_OpenIndex (0x004c5550; ECX script, stack: cache file name; ret 4): already open (+0x80) -> 1. Creates the
// search set from +0x74 when +0x7c is null (StringSet_CreateAndAddTokens), opens the name through it
// (File_OpenOnSearchPath, "rb"); none -> 0. Reads the 8-byte header (short -> close, 0), needs magic 0x08971119 (else
// close, 0) and version 7 (else close, 0), reads the count (short -> close, 0), reallocs (count + 1) * 0x80 bytes and
// reads count entries (short -> close, 0). Then each entry's source (+0) is _stat'ed: an existing file whose time differs
// from the entry's +0x78 invalidates the cache (close, free, 0 - checked until the first mismatch). Else +0x84 / +0x88 =
// the header, *[+0x8c] = count, +0x90 = the entries; 1.
// ScriptCache_OpenCompiled (0x004c5740; name; ret 4): no cache open -> 0; the first entry whose name matches
// case-insensitively; none -> 0; the source _stat'ed: when it exists and difftime(source, entry time) > 0 -> 0 (stale);
// else fseek to the entry's +0x7c; that fails -> 0; else the cache FILE.
// Real temp files: an index with 0..3 entries naming two real source files (times right or wrong) and a missing one,
// bad magic / version, cut short; OpenCompiled asked for each entry name (case varied) and a missing one. Compared: the
// returns (FILE by role), the header words, the count, the entries' bytes, the FILE position.
TEST(native_script_cache_open_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*);
    const Fn open_index[2] = {rt::original<Fn>(0x004c5550), reinterpret_cast<Fn>(&recoil::ScriptCache_OpenIndex)};
    const Fn open_compiled[2] = {rt::original<Fn>(0x004c5740), reinterpret_cast<Fn>(&recoil::ScriptCache_OpenCompiled)};
    const std::string cache = temp_path("scc"), src[2] = {temp_path("sa"), temp_path("sb")};
    write_file(src[0], "NewWorld\n");
    write_file(src[1], "set x 1\n");
    struct Stat32 { std::uint32_t dev; std::uint16_t ino, mode; std::int16_t nlink, uid, gid; std::uint32_t rdev, size; std::int32_t atime, mtime, ctime; };
    auto c_stat = crt().get<int(__cdecl*)(const char*, Stat32*)>("_stat");
    std::int32_t mtime[2];
    for (int k = 0; k < 2; ++k) { Stat32 st{}; c_stat(src[k].c_str(), &st); mtime[k] = st.mtime; }
    std::mt19937 rng(0x4c5550);
    int compared = 0, opened = 0, found = 0;
    for (int it = 0; it < 1500; ++it) {
        const int n = static_cast<int>(rng() % 4);
        std::string bytes, names[3];
        const std::uint32_t magic = rng() % 8 ? 0x08971119u : 0x08971118u, version = rng() % 8 ? 7u : 6u, cnt = static_cast<std::uint32_t>(n);
        bytes.append(reinterpret_cast<const char*>(&magic), 4);
        bytes.append(reinterpret_cast<const char*>(&version), 4);
        bytes.append(reinterpret_cast<const char*>(&cnt), 4);
        for (int k = 0; k < n; ++k) {
            char e[0x80] = {};
            const int which = static_cast<int>(rng() % 3);
            names[k] = which < 2 ? src[which] : src[0] + ".missing";
            std::strncpy(e, names[k].c_str(), 0x77);
            const std::int32_t t = which < 2 ? mtime[which] + (rng() % 4 == 0 ? static_cast<std::int32_t>(rng() % 3) - 1 : 0) : 12345;
            std::memcpy(e + 0x78, &t, 4);
            const std::uint32_t off = rng() % 40;
            std::memcpy(e + 0x7c, &off, 4);
            bytes.append(e, 0x80);
        }
        if (rng() % 6 == 0) bytes.resize(rng() % bytes.size());
        write_file(cache, bytes);
        std::string query = n && rng() % 4 ? names[rng() % n] : std::string("missing.gs");
        if (rng() % 2) for (char& c : query) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {}, count_store = 0xEEEEEEEEu;
            const char* paths[] = {".", nullptr};
            obj[0x74 / 4] = addr(paths);
            obj[0x8c / 4] = addr(&count_store);
            const std::uint32_t r = open_index[side](obj, 0, cache.c_str());
            snap[side].push_back(r);
            snap[side].push_back(obj[0x80 / 4] ? 1u : 0u);
            snap[side].push_back(obj[0x7c / 4] ? 1u : 0u);
            snap[side].push_back(obj[0x84 / 4]);
            snap[side].push_back(obj[0x88 / 4]);
            snap[side].push_back(count_store);
            const std::uint32_t entries = obj[0x90 / 4];
            if (r == 1 && entries) for (std::uint32_t k = 0; k < 0x20 * count_store; ++k) snap[side].push_back(at(entries)[k]);
            if (r == 1) {
                const std::uint32_t c = open_compiled[side](obj, 0, query.c_str());
                snap[side].push_back(c == obj[0x80 / 4] && c ? 0xCAFEu : c);
                if (c) snap[side].push_back(static_cast<std::uint32_t>(c_ftell(at(c))));
                if (side == 0) { ++opened; found += c != 0; }
            }
            if (obj[0x80 / 4]) c_fclose(at(obj[0x80 / 4]));
            if (r == 1 && entries) c_free(at(entries));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    DeleteFileA(cache.c_str());
    DeleteFileA(src[0].c_str());
    DeleteFileA(src[1].c_str());
    rt::restore_pristine();
    std::printf("  ScriptCache_OpenIndex / OpenCompiled calls %d, %d indexes opened, %d compiled scripts found\n", compared, opened, found);
    CHECK(opened > 300 && found > 100);
}

namespace {
// error hook for the second group: logs the format and four variadic words, image addresses as original VAs and the
// given tree nodes by index
std::vector<std::uint32_t> g_hook2;
std::vector<std::uint32_t> g_nodes;
std::uint32_t norm(std::uint32_t v)
{
    // any address inside a node (its name at +0, fields) by node index and offset: the nodes are heap vectors whose
    // addresses differ between the two sides
    for (std::size_t k = 0; k < g_nodes.size(); ++k)
        if (v >= g_nodes[k] && v < g_nodes[k] + 49 * 4) return 0xA0000000u + static_cast<std::uint32_t>(k) * 0x100u + (v - g_nodes[k]);
    const std::uint32_t va = recoil::ImageData_VaOf(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(v)));
    return va ? va : v;
}
void __cdecl logging_hook2(const char* fmt, const std::uint32_t* va)
{
    g_hook2.push_back(norm(addr(fmt)));
    // only the arguments the format consumes: the slots after them are whatever the caller's stack held (they
    // rotated between iterations - a value from one side reappeared on the other side one iteration later)
    int used = 0;
    for (const char* c = fmt; c && *c; ++c)
        if (*c == '%') { if (c[1] == '%') ++c; else ++used; }
    for (int k = 0; k < 4; ++k) g_hook2.push_back(k < used ? norm(va[k]) : 0x5107u);
}
}  // namespace

// Script_ExpandVars (0x004c1250; string; ret 4): null -> 0; no "%name%" pair -> the string itself; else the global
// buffer [0x0056c378] gets the text before each pair (strncat) and each known variable's value (Script_GetVar on the
// name copied with strncpy into a 64-byte local), unknown names dropped, then the tail; returns the buffer.
// Script_VarIsTrue (0x004c1710; name; ret 4): 0 for an unknown variable, else value == "TRUE" (0x004da634).
// Script_SetVar (0x004c1780; name, value; ret 8): either null -> 0; a known name: its value is realloc'd to the new
// length and copied; else the pair array grows by one (realloc) with two _strdup'd strings; 1.
// Variables: 0..4 pairs in real msvcrt blocks, names and values from small pools ("TRUE", "true", ...). Strings with
// 0..3 "%name%" references (known, unknown, empty "%%", unmatched "%"). Compared: returns (the buffer / input by role),
// the buffer text, and every pair's strings.
TEST(native_script_variables_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Expand = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*);
    using IsTrue = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*);
    using Set = std::uint32_t(__fastcall*)(std::uint32_t*, int, const char*, const char*);
    const Expand expand[2] = {rt::original<Expand>(0x004c1250), reinterpret_cast<Expand>(&recoil::Script_ExpandVars)};
    const IsTrue is_true[2] = {rt::original<IsTrue>(0x004c1710), reinterpret_cast<IsTrue>(&recoil::Script_VarIsTrue)};
    const Set set[2] = {rt::original<Set>(0x004c1780), reinterpret_cast<Set>(&recoil::Script_SetVar)};
    const char* const names[] = {"x", "speed", "Map", "a_long_variable_name", "q"};
    const char* const values[] = {"TRUE", "true", "1.5", "", "FALSE", "world01"};
    const char* const pieces[] = {"%x%", "%speed%", "%nope%", "%%", "abc ", " ", "%Map%/", "%", "tail"};
    std::mt19937 rng(0x4c1250);
    int compared = 0, expanded = 0;
    for (int it = 0; it < 6000; ++it) {
        const int f = it % 3, nv = static_cast<int>(rng() % 5);
        int vn[4], vv[4];
        for (int k = 0; k < 4; ++k) { vn[k] = static_cast<int>(rng() % 5); vv[k] = static_cast<int>(rng() % 6); }
        std::string s;
        for (int k = static_cast<int>(rng() % 5); k > 0; --k) s += pieces[rng() % 9];
        const char* name = rng() % 10 == 0 ? nullptr : names[rng() % 5];
        const char* value = rng() % 10 == 0 ? nullptr : values[rng() % 6];
        const bool null_str = rng() % 20 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {};
            const std::uint32_t pairs = nv ? addr(c_malloc(8 * nv)) : 0u;
            for (int k = 0; k < nv; ++k) { at(pairs)[2 * k] = addr(c_strdup(names[vn[k]])); at(pairs)[2 * k + 1] = addr(c_strdup(values[vv[k]])); }
            obj[0x60 / 4] = pairs; obj[0x64 / 4] = static_cast<std::uint32_t>(nv);
            char* const buf = reinterpret_cast<char*>(side ? recoil::ImageData_Address(0x0056c378) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056c378)));
            std::uint32_t r = 0;
            if (f == 0) r = expand[side](obj, 0, null_str ? nullptr : s.c_str());
            if (f == 1) r = is_true[side](obj, 0, name ? name : "x");
            if (f == 2) r = set[side](obj, 0, name, value);
            snap[side].push_back(f == 0 ? (r == addr(buf) ? 0xB0Fu : r == addr(s.c_str()) ? 0x1Du : r) : r);
            if (f == 0 && r == addr(buf)) { for (const char* c = buf; *c; ++c) snap[side].push_back(static_cast<unsigned char>(*c)); if (side == 0) ++expanded; }
            snap[side].push_back(obj[0x64 / 4]);
            for (std::uint32_t k = 0; k < obj[0x64 / 4]; ++k)
                for (int w = 0; w < 2; ++w) {
                    for (const char* c = reinterpret_cast<const char*>(at(at(obj[0x60 / 4])[2 * k + w])); *c; ++c) snap[side].push_back(static_cast<unsigned char>(*c));
                    snap[side].push_back(0x100u);
                    c_free(at(at(obj[0x60 / 4])[2 * k + w]));
                }
            if (obj[0x60 / 4]) c_free(at(obj[0x60 / 4]));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Script_ExpandVars / VarIsTrue / SetVar calls %d, %d expanded into the buffer\n", compared, expanded);
    CHECK(expanded > 700);
}

// Script_DumpValue (0x004c1ab0; value record {name, type, data}; ret 4): null -> nothing; type 0 -> hook("%s = %d"-style
// 0x004e4924, name, *(int*)data), 1 -> (0x004e4934, name, (double)*(float*)data), 2 -> (0x004e4944, name, (char)*data);
// other types nothing. Script_DumpTree (0x004c2030; node, indent; ret 8): null -> nothing; hook(0x004e4a28, indent,
// "" 0x004dad08, node) then each child with indent + 2 (recursion over +0x5c / +0x60; real trees of depth <= 3).
// Script_CheckArgs (0x004c5820; min, class, node; ret 0xc): class set and node null -> Script_Fail(0x004e5ac8, argv0),
// AL 0; class set and node class +0x34 differs -> Fail(0x004e5a8c, argv0, node, its class, class), 0; then argc < min + 1
// -> Fail(0x004e5a50, argv0, min, argc - 1), 0; else AL 1. The hook logs its format and four variadic words (image
// addresses as original addresses, tree nodes by index). Compared: returns (AL for CheckArgs), the hook log, the error flag.
TEST(native_script_dump_value_tree_check_args_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using One = int(__fastcall*)(std::uint32_t*, int, const void*);
    using Tree = int(__fastcall*)(std::uint32_t*, int, const void*, int);
    using Check = std::uint32_t(__fastcall*)(std::uint32_t*, int, int, int, const void*);
    const One value[2] = {rt::original<One>(0x004c1ab0), reinterpret_cast<One>(&recoil::Script_DumpValue)};
    const Tree tree[2] = {rt::original<Tree>(0x004c2030), reinterpret_cast<Tree>(&recoil::Script_DumpTree)};
    const Check check[2] = {rt::original<Check>(0x004c5820), reinterpret_cast<Check>(&recoil::Script_CheckArgs)};
    std::mt19937 rng(0x4c1ab0);
    int compared = 0, passed = 0;
    for (int it = 0; it < 6000; ++it) {
        const int f = it % 3;
        const std::uint32_t type = rng() % 4, word = rng();
        const float fl = static_cast<float>(static_cast<int>(rng() % 2000) - 1000) / 8.0f;
        const int argc = static_cast<int>(rng() % 5), min = static_cast<int>(rng() % 4), cls = rng() % 3 ? 0 : 1 + static_cast<int>(rng() % 3);
        const int node_cls = 1 + static_cast<int>(rng() % 3);
        const bool null_node = rng() % 4 == 0;
        // a tree: node 0 the root, each node 0..2 children while the depth allows
        std::vector<int> parent{-1}, depth{0};
        for (std::size_t i = 0; i < parent.size() && parent.size() < 12; ++i)
            if (depth[i] < 3) for (int c = static_cast<int>(rng() % 3); c > 0; --c) { parent.push_back(static_cast<int>(i)); depth.push_back(depth[i] + 1); }
        const int indent = static_cast<int>(rng() % 5);
        const bool null_rec = rng() % 20 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {};
            obj[0x70 / 4] = addr(reinterpret_cast<void*>(&logging_hook2));
            obj[2] = static_cast<std::uint32_t>(argc);
            obj[8] = addr(kWords[1]);
            std::vector<std::vector<std::uint32_t>> nodes(parent.size(), std::vector<std::uint32_t>(49, 0)), kids(parent.size());
            g_nodes.clear();
            for (auto& n : nodes) g_nodes.push_back(addr(n.data()));
            for (std::size_t i = 1; i < parent.size(); ++i) kids[parent[i]].push_back(addr(nodes[i].data()));
            for (std::size_t i = 0; i < parent.size(); ++i) {
                std::strcpy(reinterpret_cast<char*>(nodes[i].data()), kWords[i % 11]);
                nodes[i][0x34 / 4] = static_cast<std::uint32_t>(node_cls);
                nodes[i][0x5c / 4] = static_cast<std::uint32_t>(kids[i].size());
                nodes[i][0x60 / 4] = kids[i].empty() ? 0u : addr(kids[i].data());
            }
            std::uint32_t datum = word;
            if (type == 1) std::memcpy(&datum, &fl, 4);
            const std::uint32_t rec[3] = {addr(kWords[2]), type, addr(&datum)};
            g_hook2.clear();
            std::uint32_t r = 0;
            if (f == 0) value[side](obj, 0, null_rec ? nullptr : static_cast<const void*>(rec));
            if (f == 1) tree[side](obj, 0, null_node ? nullptr : nodes[0].data(), indent);
            if (f == 2) r = check[side](obj, 0, min, cls, null_node ? nullptr : nodes[0].data()) & 0xFFu;
            snap[side] = g_hook2;
            snap[side].push_back(r);
            snap[side].push_back(obj[0x10 / 4]);
            if (side == 0) passed += f == 2 && r == 1;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Script_DumpValue / DumpTree / CheckArgs calls %d, %d argument checks passed\n", compared, passed);
    CHECK(passed > 200);
}

// Script_NextArg (0x004c1990): the argument cursor +0xc moves on by one; past argc (+8) -> 0; else
// Script_ExpandVars(that argument) (the argument itself or the expansion buffer [0x0056c378]).
// Script_EvalCondition (0x004c1b50): argc 1 -> 0; argc 2 -> Script_VarIsTrue(argv[1]); else folds argv[1..] left to
// right: operand, then an operator ("||" 0x004e495c / "&&" 0x004e4958 compared with strncmp n 2 - so "||x" counts), an
// unknown operator ends the evaluation with the value so far. Variables: 0..4 real pairs, values "TRUE" / "true" /
// other. Compared: the returns (NextArg's by role), the cursor, the expansion buffer text.
TEST(native_script_next_arg_eval_condition_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Fn = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    const Fn next[2] = {rt::original<Fn>(0x004c1990), reinterpret_cast<Fn>(&recoil::Script_NextArg)};
    const Fn eval[2] = {rt::original<Fn>(0x004c1b50), reinterpret_cast<Fn>(&recoil::Script_EvalCondition)};
    const char* const names[] = {"a", "b", "c", "%a%", "d"};
    const char* const values[] = {"TRUE", "true", "0", "a"};
    const char* const ops[] = {"||", "&&", "||x", "&", "or"};
    std::mt19937 rng(0x4c1990);
    int compared = 0, truths = 0;
    for (int it = 0; it < 12000; ++it) {
        const int f = it % 2, nv = static_cast<int>(rng() % 5);
        int vn[4], vv[4];
        for (int k = 0; k < 4; ++k) { vn[k] = static_cast<int>(rng() % 5); vv[k] = static_cast<int>(rng() % 4); }
        std::vector<const char*> argv{"ifdef"};
        const int terms = static_cast<int>(rng() % 5);
        for (int k = 0; k < terms; ++k) {
            argv.push_back(names[rng() % 5]);
            if (k + 1 < terms || rng() % 4 == 0) argv.push_back(rng() % 6 ? ops[rng() % 2] : ops[2 + rng() % 3]);
        }
        const std::uint32_t cursor = rng() % (argv.size() + 2);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {};
            const std::uint32_t pairs = nv ? addr(c_malloc(8 * nv)) : 0u;
            for (int k = 0; k < nv; ++k) { at(pairs)[2 * k] = addr(names[vn[k]]); at(pairs)[2 * k + 1] = addr(values[vv[k]]); }
            obj[0x60 / 4] = pairs; obj[0x64 / 4] = static_cast<std::uint32_t>(nv);
            obj[2] = static_cast<std::uint32_t>(argv.size());
            for (std::size_t k = 0; k < argv.size() && k < 16; ++k) obj[8 + k] = addr(argv[k]);
            obj[3] = cursor;
            const char* buf = reinterpret_cast<const char*>(side ? recoil::ImageData_Address(0x0056c378) : reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x0056c378)));
            const std::uint32_t r = (f ? eval : next)[side](obj, 0);
            std::uint32_t rr = r;
            if (!f && r == addr(buf)) rr = 0xB0Fu;
            for (std::size_t k = 0; !f && k < argv.size(); ++k) if (r == addr(argv[k])) rr = 0xA0u + static_cast<std::uint32_t>(k);
            snap[side].push_back(rr);
            snap[side].push_back(obj[3]);
            if (rr == 0xB0Fu) for (const char* c = buf; *c; ++c) snap[side].push_back(static_cast<unsigned char>(*c));
            if (side == 0) truths += f && r;
            if (pairs) c_free(at(pairs));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Script_NextArg / EvalCondition calls %d, %d conditions true\n", compared, truths);
    CHECK(truths > 300);
}

// Script_ArgBool (0x004c19c0): Script_NextArg; 1 when it is "on" or "true" (_stricmp), else 0 (none -> 0).
// Script_ArgFloat (0x004c1a00): atof(next) in ST0, or the float constant it loads from .rdata when there is none.
// Script_ArgInt (0x004c1a20): atoi(next) or 0. Arguments from a pool (case variants, numbers, junk), cursor anywhere.
// Compared: the return (ST0 as a double for ArgFloat) and the cursor.
TEST(native_script_arg_readers_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using I = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    using D = double(__fastcall*)(std::uint32_t*, int);
    const I arg_bool[2] = {rt::original<I>(0x004c19c0), reinterpret_cast<I>(&recoil::Script_ArgBool)};
    const D arg_float[2] = {rt::original<D>(0x004c1a00), reinterpret_cast<D>(&recoil::Script_ArgFloat)};
    const I arg_int[2] = {rt::original<I>(0x004c1a20), reinterpret_cast<I>(&recoil::Script_ArgInt)};
    const char* const pool[] = {"on", "ON", "true", "True", "off", "1", "-3", "2.75", "1e3", "x", "", "0x10", " 7"};
    std::mt19937 rng(0x4c19c0);
    int compared = 0;
    for (int it = 0; it < 6000; ++it) {
        const int f = it % 3, argc = static_cast<int>(rng() % 5);
        int a[4];
        for (int& x : a) x = static_cast<int>(rng() % 13);
        const std::uint32_t cursor = rng() % (argc + 2);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t obj[0x40] = {};
            obj[2] = static_cast<std::uint32_t>(argc);
            for (int k = 0; k < argc; ++k) obj[8 + k] = addr(pool[a[k]]);
            obj[3] = cursor;
            if (f == 1) {
                const double d = arg_float[side](obj, 0);
                std::uint32_t w[2];
                std::memcpy(w, &d, 8);
                snap[side].push_back(w[0]); snap[side].push_back(w[1]);
            } else {
                snap[side].push_back((f ? arg_int : arg_bool)[side](obj, 0));
            }
            snap[side].push_back(obj[3]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Script_ArgBool / ArgFloat / ArgInt calls %d\n", compared);
}
