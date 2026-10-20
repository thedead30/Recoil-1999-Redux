// Native L1 for Settings_LoadFromRegistry 0x004b2960 and Settings_SaveToRegistry 0x004b2bf0 (and Crt_ChkStk
// 0x004c6100, which both call to allocate the key path on the stack), ORIGINAL vs port. Both sides' ADVAPI32 slots
// (the port's platform/advapi.h variables and the mapped original's IAT) are pointed at an in-memory fake registry
// that logs every call - key paths, value names, types, sizes, data - so nothing touches the real registry. Nodes
// are registered with the (verified) Settings_RegisterNode; after a load the nodes' value buffers are compared as
// heap graphs, after a save the fake registries' contents; the call logs must be identical throughout.
#include "test.h"
#include "native_oracle.h"
#include "heap_graph.h"
#include "platform/advapi.h"
#include "unattributed/settings.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace {
std::mt19937 grng(0x4E61);
using F2 = int(__fastcall*)(int, int);
using F4 = int(__fastcall*)(int, int, int, int);
int I(const void* p) { return static_cast<int>(reinterpret_cast<std::uintptr_t>(p)); }

struct Value { std::uint32_t type; std::string data; };
struct FakeReg {
    std::map<std::string, std::map<std::string, Value>> keys;  // "hive\\path" -> values
    std::map<std::uint32_t, std::string> open;                  // handle -> key
    std::uint32_t next = 0x100;
    std::vector<std::string> log;
};
FakeReg* g_reg = nullptr;

std::string hive(std::uint32_t h) { return h == 0x80000001u ? "HKCU" : h == 0x80000002u ? "HKLM" : "H" + std::to_string(h); }
std::string key_of(std::uint32_t h) { auto it = g_reg->open.find(h); return it != g_reg->open.end() ? it->second : hive(h); }

long __stdcall fake_open(std::uint32_t h, const char* sub, std::uint32_t, std::uint32_t sam, std::uint32_t* out)
{
    const std::string k = key_of(h) + "\\" + (sub ? sub : "");
    g_reg->log.push_back("open " + k + " sam " + std::to_string(sam));
    if (!g_reg->keys.count(k)) return 2;  // ERROR_FILE_NOT_FOUND
    *out = g_reg->next++;
    g_reg->open[*out] = k;
    return 0;
}
long __stdcall fake_create(std::uint32_t h, const char* sub, std::uint32_t, char*, std::uint32_t opts, std::uint32_t sam, void*,
                           std::uint32_t* out, std::uint32_t* disp)
{
    const std::string k = key_of(h) + "\\" + (sub ? sub : "");
    g_reg->log.push_back("create " + k + " opts " + std::to_string(opts) + " sam " + std::to_string(sam));
    const bool existed = g_reg->keys.count(k) != 0;
    g_reg->keys[k];
    *out = g_reg->next++;
    g_reg->open[*out] = k;
    if (disp) *disp = existed ? 2 : 1;
    return 0;
}
long __stdcall fake_query(std::uint32_t h, const char* name, std::uint32_t*, std::uint32_t* type, unsigned char* data, std::uint32_t* cb)
{
    const std::string k = key_of(h);
    // *cb is an input only when a buffer is passed (a size query leaves it uninitialised in the original too)
    g_reg->log.push_back("query " + k + " " + (name ? name : "") + (data ? " data cb " + std::to_string(cb ? *cb : 0) : std::string(" nodata")));
    auto kit = g_reg->keys.find(k);
    if (kit == g_reg->keys.end() || !kit->second.count(name ? name : "")) return 2;
    const Value& v = kit->second[name ? name : ""];
    if (type) *type = v.type;
    if (data) {
        if (!cb || *cb < v.data.size()) { if (cb) *cb = static_cast<std::uint32_t>(v.data.size()); return 234; }  // ERROR_MORE_DATA
        std::memcpy(data, v.data.data(), v.data.size());
    }
    if (cb) *cb = static_cast<std::uint32_t>(v.data.size());
    return 0;
}
long __stdcall fake_set(std::uint32_t h, const char* name, std::uint32_t, std::uint32_t type, const unsigned char* data, std::uint32_t cb)
{
    const std::string k = key_of(h);
    std::string bytes(reinterpret_cast<const char*>(data), cb);
    g_reg->log.push_back("set " + k + " " + (name ? name : "") + " type " + std::to_string(type) + " cb " + std::to_string(cb));
    g_reg->keys[k][name ? name : ""] = {type, bytes};
    return 0;
}
long __stdcall fake_close(std::uint32_t h)
{
    g_reg->log.push_back("close " + key_of(h));
    g_reg->open.erase(h);
    return 0;
}

constexpr std::uintptr_t kIat[] = {0x004cc004, 0x004cc008, 0x004cc00c, 0x004cc010, 0x004cc014};
void install_fake_registry()
{
    void* fns[] = {reinterpret_cast<void*>(&fake_close), reinterpret_cast<void*>(&fake_create), reinterpret_cast<void*>(&fake_open),
                   reinterpret_cast<void*>(&fake_query), reinterpret_cast<void*>(&fake_set)};
    for (int i = 0; i < 5; ++i) hg::set_slot(kIat[i], fns[i]);
    recoil::g_Iat_RegCloseKey_004cc004 = fns[0];
    recoil::g_Iat_RegCreateKeyExA_004cc008 = fns[1];
    recoil::g_Iat_RegOpenKeyExA_004cc00c = fns[2];
    recoil::g_Iat_RegQueryValueExA_004cc010 = fns[3];
    recoil::g_Iat_RegSetValueExA_004cc014 = fns[4];
}

const char* kParts[3][3] = {{"Rebellion", "Zipper", "X"}, {"Recoil", "Game", ""}, {"1.0", "Settings", "Current"}};
const char* kNames[] = {"SoundVolume", "PlayerName", "HWAPI", "FullScreen", "detail", "Window"};
constexpr std::uintptr_t kOrigState = 0x0056bcd0;
}  // namespace

TEST(native_settings_registry_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    auto oRegister = rt::original<F4>(0x004b2e80);
    auto oLoad = rt::original<F2>(0x004b2960);
    auto oSave = rt::original<F2>(0x004b2bf0);
    auto oShutdown = rt::original<F2>(0x004b32c0);
    const F4 pRegister = &recoil::Settings_RegisterNode;
    const F2 pLoad = &recoil::Settings_LoadFromRegistry, pSave = &recoil::Settings_SaveToRegistry, pShutdown = &recoil::Settings_Shutdown;
    auto* oState = reinterpret_cast<std::uint32_t*>(kOrigState);
    install_fake_registry();
    hg::Recording rec;
    int bad = 0, loads_ok = 0, values_read = 0, saves = 0;
    std::string first;
    for (int n = 0; n < 800 && !bad; ++n) {
        hg::Log O, P;
        FakeReg rO, rP;
        std::memset(oState, 0, 4 * recoil::kSettingsNodeStateWords);
        std::memset(recoil::g_SettingsNodeState_0056bcd0, 0, sizeof recoil::g_SettingsNodeState_0056bcd0);
        O.add(oState, 1, 4 * recoil::kSettingsNodeStateWords); P.add(recoil::g_SettingsNodeState_0056bcd0, 1, sizeof recoil::g_SettingsNodeState_0056bcd0);
        // the three key-path parts (heap strings owned by the state)
        std::string path = "SOFTWARE\\";
        for (int k = 0; k < 3; ++k) {
            const char* part = kParts[k][grng() % 3];
            path += part; path += "\\";
            hg::current() = &O; oState[2 + k] = static_cast<std::uint32_t>(I(hg::rec_strdup(part)));
            hg::current() = &P; recoil::g_SettingsNodeState_0056bcd0[2 + k] = static_cast<std::uint32_t>(I(hg::rec_strdup(part)));
        }
        oState[1] = recoil::g_SettingsNodeState_0056bcd0[1] = 1;
        // nodes: types 0..9, hive selector 0 (HKLM) / 1 (HKCU) / other (skipped)
        const int nodes = 1 + static_cast<int>(grng() % 6);
        for (int k = 0; k < nodes; ++k) {
            const char* name = kNames[grng() % 6];
            // types 0..7 only: the load/save/register switches (tables 0x004b2bc4 / 0x004b2e58 / 0x004b2f28) define
            // those; a type above 7 reads uninitialised stack in the load (KG-27)
            const int type = static_cast<int>(grng() % 8), size = static_cast<int>(1 + grng() % 24), sel = static_cast<int>(grng() % 3);
            hg::current() = &O; hg::call_fastcall(oRegister, I(name), type, size, sel);
            hg::current() = &P; hg::call_fastcall(pRegister, I(name), type, size, sel);
        }
        // the fake registries: the two keys exist or not, with values of matching or mismatching sizes
        for (const char* hv : {"HKCU\\", "HKLM\\"}) {
            if (grng() % 5 == 0) continue;
            auto& vals = rO.keys[hv + path.substr(0, path.size() - 1)];
            for (const char* name : kNames)
                if (grng() % 2) {
                    // mostly the sizes nodes expect (4, 8) so reads really happen; the rest arbitrary
                    std::string d(grng() % 5 < 2 ? 4 : grng() % 3 == 0 ? 8 : 1 + grng() % 24, '\0');
                    for (char& c : d) c = static_cast<char>(grng());
                    vals[name] = {static_cast<std::uint32_t>(grng() % 4), d};
                }
        }
        rP.keys = rO.keys;
        auto step = [&](const char* what) {
            std::string d = hg::compare(O, P);
            if (d.empty() && rO.log != rP.log) {
                d = "registry call logs differ";
                if (!bad) {
                    for (std::size_t i = 0; i < rO.log.size() || i < rP.log.size(); ++i) {
                        const std::string& lo = i < rO.log.size() ? rO.log[i] : std::string("-");
                        const std::string& lp = i < rP.log.size() ? rP.log[i] : std::string("-");
                        if (lo != lp) { std::printf("    [%zu] original: %s\n    [%zu] port:     %s\n", i, lo.c_str(), i, lp.c_str()); break; }
                    }
                }
            }
            if (d.empty() && rO.keys.size() != rP.keys.size()) d = "registry contents differ";
            if (d.empty())
                for (auto& [k, v] : rO.keys) {
                    auto it = rP.keys.find(k);
                    if (it == rP.keys.end() || it->second.size() != v.size()) { d = "registry key " + k; break; }
                    for (auto& [nm, val] : v) {
                        auto jt = it->second.find(nm);
                        if (jt == it->second.end() || jt->second.type != val.type || jt->second.data != val.data) { d = "registry value " + k + " " + nm; break; }
                    }
                }
            if (!d.empty() && !bad) { ++bad; first = std::string(what) + " #" + std::to_string(n) + ": " + d; }
        };
        g_reg = &rO; hg::current() = &O; const int a = hg::call_fastcall(oLoad, 0, 0);
        g_reg = &rP; hg::current() = &P; const int b = hg::call_fastcall(pLoad, 0, 0);
        if (a != b) { ++bad; first = "load result #" + std::to_string(n); }
        loads_ok += a;
        for (auto& s : rO.log) values_read += s.rfind("query", 0) == 0 && s.find(" data") != std::string::npos;
        step("load");
        g_reg = &rO; hg::current() = &O; const int c = hg::call_fastcall(oSave, 0, 0);
        g_reg = &rP; hg::current() = &P; const int d = hg::call_fastcall(pSave, 0, 0);
        if (c != d) { ++bad; first = "save result #" + std::to_string(n); }
        saves += c;
        step("save");
        hg::current() = &O; hg::call_fastcall(oShutdown, 0, 0);
        hg::current() = &P; hg::call_fastcall(pShutdown, 0, 0);
        step("shutdown");
        for (hg::Log* L : {&O, &P})
            for (auto& [p, blk] : L->live)
                if (blk.role >= 1000) hg::real().free_(reinterpret_cast<void*>(p));
    }
    g_reg = nullptr;
    if (bad) std::printf("  first difference: %s\n", first.c_str());
    std::printf("  loads that opened both keys %d, value reads %d, saves %d\n", loads_ok, values_read, saves);
    CHECK_EQ(bad, 0);
    CHECK(loads_ok > 300 && values_read > 50 && saves > 300);  // each path really taken
}
