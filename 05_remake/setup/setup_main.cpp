// setup_main.cpp - the player Setup (Setup.exe in the players' download). C++ port of tools/setup/setup_core.py +
// recoil_setup.py, which stay the reference (tools/setup/compare_setup.py checks both install the same files).
// One window (2026-10-20):
//   1  Requirements: dgVoodoo2 - link to the download page; the player selects the zip they downloaded.
//   2  Your Recoil: the player selects their copy (any form source.h reads) and an install folder; Setup checks the
//           version.
//   3  Textures: original, generated on this PC (Real-ESRGAN, upscale.cpp, from the player's own files), or an asset
//      pack zip the player selects, with
//           redux_textures\ (high-resolution textures) and redux_video\ (HD videos). Then Setup copies the game files
//           (original Recoil.exe to original\), CD music to music\trackNN.wav, the pack, payload\Recoil Redux.exe with the
//           original's resources copied in, dgVoodoo files, redux.ini, and a desktop shortcut.
// Command line (tests): Setup.exe --source COPY --dgvoodoo ZIP --dest DIR [--pack ZIP | --generate] [--esrgan DIR] [--payload EXE]
//                       [--no-shortcut]
// Nothing from Recoil is in the download: every game byte comes from the player's copy. Not Recoil code (PLATFORM).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>

#include "source.h"
#include "upscale.h"
#include "mapgen.h"
#include "zbdimg.h"
#include "quality_example.h"

#include <wincodec.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <set>
#include <string>
#include <thread>
#include <vector>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace setup;

namespace {

const wchar_t kTitle[] = L"Recoil Redux Setup";
const wchar_t kExeName[] = L"Recoil Redux.exe";
const char kDgvLink[] = "https://github.com/dege-diosg/dgVoodoo2/releases";
const char kDgvTested[] = "2.8.7.3 and 2.8.7.5";   // PLATFORM: the dgVoodoo2 versions the remake was tested with

// dgVoodoo.conf values the remake was tested with (PLATFORM: dgVoodoo settings from the tested conf of 2026-09-09)
struct ConfKey {
    const char *section, *key, *value;
};
const ConfKey kDgvSettings[] = {
    {"General", "OutputAPI", "bestavailable"},
    {"General", "FullScreenMode", "true"},
    {"General", "KeepWindowAspectRatio", "true"},
    {"Glide", "EnableInactiveAppState", "false"},
    {"DirectX", "VideoCard", "internal3D"},
    {"DirectX", "VRAM", "2048"},                     // Redux v1: 1024 px texture packs need it (256 MB: rainbow sky/water)
    {"DirectX", "Filtering", "16"},                  // the Redux tested look: 16x anisotropic, 4x anti-aliasing
    {"DirectX", "Antialiasing", "4x"},
    {"DirectX", "Mipmapping", "autogen_bilinear"},
    {"DirectX", "KeepFilterIfPointSampled", "false"},
    {"DirectX", "AppControlledScreenMode", "false"},
    {"DirectX", "DisableAltEnterToToggleScreenMode", "true"},
    {"DirectX", "dgVoodooWatermark", "false"},
};

// The CD's installer cabinet keeps some files at its root that the original installer puts elsewhere (CONFIRMED-DATA: the
// cabinet listing of the Recoil Classic CD vs an installed copy, 2026-10-07): sound banks go to zbd\; Windows redistributables
// and the 3dfx exe are not part of the game folder (a 1998 DLL beside the exe would be loaded instead of Windows' own).
const std::set<std::string> kCabToZbd = {"soundsh.zbd", "soundsl.zbd", "soundsm.zbd"};
const std::set<std::string> kNotGameFiles = {"mfc42.dll", "msvcirt.dll", "msvcp50.dll", "msvcrt.dll", "msvcrt20.dll",
                                             "msvcrtd.dll", "msvfw32.dll", "ws2_32.dll", "ws2help.dll", "ir50_32.dll",
                                             "eurosti.ttf", "eurostib.ttf", "recoil3dfx.exe"};

struct SetupError {
    std::string msg;
};

using Log = std::function<void(const std::string&)>;
using Progress = std::function<void(int, int)>;

std::wstring ExeDir()
{
    wchar_t p[MAX_PATH];
    GetModuleFileNameW(nullptr, p, MAX_PATH);
    std::wstring s = p;
    return s.substr(0, s.find_last_of(L'\\'));
}

std::wstring Join(const std::wstring& a, const std::string& rel_utf8)
{
    std::wstring r = Widen(rel_utf8);
    std::replace(r.begin(), r.end(), L'/', L'\\');
    return a + L"\\" + r;
}

void MakeDirs(const std::wstring& dir)
{
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
}

void WriteFileAll(const std::wstring& path, const std::vector<std::uint8_t>& d)
{
    MakeDirs(path.substr(0, path.find_last_of(L'\\')));
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    DWORD w = 0;
    const bool ok = h != INVALID_HANDLE_VALUE && (d.empty() || WriteFile(h, d.data(), DWORD(d.size()), &w, nullptr)) && w == d.size();
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    if (!ok) throw SetupError{"cannot write " + Narrow(path)};
}

bool EndsWithI(const std::string& s, const std::string& t)
{
    return s.size() >= t.size() && Lower(s.substr(s.size() - t.size())) == Lower(t);
}

std::string Basename(const std::string& p)
{
    const std::size_t i = p.find_last_of('/');
    return i == std::string::npos ? p : p.substr(i + 1);
}

// ---------------------------------------------------------------- dgVoodoo
struct DgvMembers {
    Source zip;
    long ddraw = -1, d3dimm = -1, conf = -1, cpl = -1;
};

void OpenDgv(const std::wstring& path, DgvMembers& m)
{
    const std::string link = std::string(" (") + kDgvLink + ")";
    std::string why;
    if (path.empty() || !m.zip.Open(path, why) || m.zip.Kind() != "zip")
        throw SetupError{"select the dgVoodoo2 zip you downloaded" + link};
    auto shallow = [&](const char* name) {
        long best = -1;
        std::size_t depth = 1000;
        for (std::size_t i = 0; i < m.zip.EntryCount(); ++i) {
            const std::string& p = m.zip.EntryPath(i);
            if (Lower(Basename(p)) == Lower(name)) {
                const std::size_t d = std::count(p.begin(), p.end(), '/');
                if (d < depth) best = long(i), depth = d;
            }
        }
        return best;
    };
    for (std::size_t i = 0; i < m.zip.EntryCount(); ++i) {
        const std::string p = Lower(m.zip.EntryPath(i));
        auto is = [&](const char* tail) { return p == tail || EndsWithI(p, std::string("/") + tail); };
        if (is("ms/x86/ddraw.dll")) m.ddraw = long(i);
        if (is("ms/x86/d3dimm.dll")) m.d3dimm = long(i);
    }
    if (m.ddraw < 0 || m.d3dimm < 0) throw SetupError{"this zip has no MS/x86/DDraw.dll + D3DImm.dll - is it the dgVoodoo2 zip?" + link};
    m.conf = shallow("dgVoodoo.conf");
    m.cpl = shallow("dgVoodooCpl.exe");
}

std::string PatchConf(const std::string& text)
{
    std::vector<std::string> lines;
    std::size_t s = 0;
    while (s < text.size()) {
        std::size_t e = text.find('\n', s);
        if (e == std::string::npos) e = text.size();
        std::string l = text.substr(s, e - s);
        if (!l.empty() && l.back() == '\r') l.pop_back();
        lines.push_back(l);
        s = e + 1;
    }
    auto trim = [](std::string x) {
        x.erase(0, x.find_first_not_of(" \t"));
        x.erase(x.find_last_not_of(" \t") + 1);
        return x;
    };
    for (const ConfKey& c : kDgvSettings) {
        const std::string head = Lower(std::string("[") + c.section + "]");
        std::size_t start = lines.size();
        for (std::size_t i = 0; i < lines.size(); ++i)
            if (Lower(trim(lines[i])) == head) { start = i; break; }
        if (start == lines.size()) {
            lines.push_back("");
            lines.push_back(std::string("[") + c.section + "]");
            start = lines.size() - 1;
        }
        std::size_t end = start + 1;
        while (end < lines.size() && trim(lines[end]).rfind("[", 0) != 0) ++end;
        char nl[256];
        std::snprintf(nl, sizeof nl, "%-37s= %s", c.key, c.value);
        bool hit = false;
        for (std::size_t i = start + 1; i < end; ++i) {
            const std::string t = trim(lines[i]);
            const std::size_t eq = t.find('=');
            if (eq != std::string::npos && Lower(trim(t.substr(0, eq))) == Lower(c.key)) {
                lines[i] = nl;
                hit = true;
                break;
            }
        }
        if (!hit) lines.insert(lines.begin() + long(end), nl);
    }
    std::string out;
    for (auto& l : lines) out += l + "\r\n";
    return out;
}

void InstallDgv(DgvMembers& m, const std::wstring& dest, const Log& log)
{
    std::vector<std::uint8_t> d;
    const std::pair<long, const char*> files[] = {{m.ddraw, "DDraw.dll"}, {m.d3dimm, "D3DImm.dll"}, {m.cpl, "dgVoodooCpl.exe"}};
    for (auto& [i, name] : files) {
        if (i < 0) continue;
        if (!m.zip.ReadEntry(std::size_t(i), d)) throw SetupError{std::string("cannot read ") + name + " from the dgVoodoo2 zip"};
        WriteFileAll(Join(dest, name), d);
    }
    std::string conf;
    if (m.conf >= 0 && m.zip.ReadEntry(std::size_t(m.conf), d)) conf.assign(d.begin(), d.end());
    conf = PatchConf(conf);
    WriteFileAll(Join(dest, "dgVoodoo.conf"), std::vector<std::uint8_t>(conf.begin(), conf.end()));
    log(std::string("dgVoodoo2: DDraw.dll, D3DImm.dll") + (m.cpl >= 0 ? ", dgVoodooCpl.exe" : "") + " and dgVoodoo.conf (full screen) installed");
}

// ---------------------------------------------------------------- the player's copy
const GameFile* FindExe(const Source& s)
{
    for (auto& f : s.Files())
        if (Lower(f.rel) == "recoil.exe") return &f;
    for (auto& f : s.Files())
        if (EndsWithI(f.rel, "/recoil.exe")) return &f;
    return nullptr;
}

// -> description lines; SetupError when the copy cannot be used
std::vector<std::string> CheckSource(Source& s, const std::wstring& path)
{
    std::string why;
    if (!s.Open(path, why)) throw SetupError{"cannot read " + Narrow(path) + " (" + why + ")"};
    if (!s.HasGame())
        throw SetupError{"no Recoil game files found in " + Narrow(path) + " (looked for Recoil.exe + zbd folder, or the CD installer data1.cab)"};
    const GameFile* exe = FindExe(s);
    if (!exe) throw SetupError{"Recoil.exe is missing from " + Narrow(path)};
    std::vector<std::uint8_t> d;
    if (!s.ReadGameFile(*exe, d, why)) throw SetupError{why};
    const ExeBuild b = IdentifyExe(d);
    if (!b.supported) throw SetupError{"This copy is: " + b.name + ".\r\n\r\n" + kSupportedNote};
    std::vector<std::string> lines{"format: " + s.Kind() + " - " + s.GameKind(), "Recoil.exe: " + b.name + " (supported)"};
    if (!s.Audio().empty()) lines.push_back("CD music: " + std::to_string(s.Audio().size()) + " tracks");
    else lines.push_back("CD music: none in this copy (only full disc images .nrg / .bin+.cue have it) - the game runs without music");
    return lines;
}

void InstallGame(const Source& s, const std::wstring& dest, const Log& log, const Progress& progress)
{
    std::vector<std::pair<const GameFile*, std::string>> files;
    for (auto& f : s.Files()) {
        const std::string low = Lower(f.rel);
        if (kNotGameFiles.count(low)) continue;
        files.push_back({&f, kCabToZbd.count(low) ? "zbd/" + f.rel : f.rel});
    }
    const int total = int(files.size() + s.Audio().size());
    std::vector<std::uint8_t> d;
    std::string why;
    int i = 0;
    for (auto& [f, place] : files) {
        if (!s.ReadGameFile(*f, d, why)) throw SetupError{why};
        WriteFileAll(Lower(place) == "recoil.exe" ? dest + L"\\original\\Recoil.exe" : Join(dest, place), d);
        progress(++i, total);
    }
    log("game files: " + std::to_string(files.size()) + " copied (the original Recoil.exe is in original\\)");
    if (s.Audio().empty()) return;
    MakeDirs(dest + L"\\music");
    for (auto& t : s.Audio()) {
        wchar_t name[32];
        std::swprintf(name, 32, L"\\music\\track%02d.wav", t.number);
        if (!t.SaveWav(dest + name)) throw SetupError{"cannot save CD music track " + std::to_string(t.number)};
        progress(++i, total);
    }
    log("CD music: " + std::to_string(s.Audio().size()) + " tracks saved as music\\trackNN.wav");
}

// every resource of source into target (C++ of tools/copy_resources.py)
int CopyResources(const std::wstring& target, const std::wstring& source)
{
    HMODULE h = LoadLibraryExW(source.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (!h) throw SetupError{"cannot open " + Narrow(source)};
    struct Item {
        std::wstring type, name;   // "#123" for integer ids
        WORD lang;
        std::vector<std::uint8_t> data;
    };
    std::vector<Item> items;
    auto key = [](LPCWSTR p) { return IS_INTRESOURCE(p) ? L"#" + std::to_wstring(ULONG_PTR(p)) : std::wstring(p); };
    auto arg = [](const std::wstring& k) { return k[0] == L'#' ? MAKEINTRESOURCEW(std::stoul(k.substr(1))) : k.c_str(); };
    std::vector<std::wstring> types;
    EnumResourceTypesW(h, [](HMODULE, LPWSTR t, LONG_PTR p) -> BOOL {
        auto* v = reinterpret_cast<std::vector<std::wstring>*>(p);
        v->push_back(IS_INTRESOURCE(t) ? L"#" + std::to_wstring(ULONG_PTR(t)) : std::wstring(t));
        return TRUE;
    }, LONG_PTR(&types));
    for (auto& t : types) {
        std::vector<std::wstring> names;
        EnumResourceNamesW(h, arg(t), [](HMODULE, LPCWSTR, LPWSTR n, LONG_PTR p) -> BOOL {
            auto* v = reinterpret_cast<std::vector<std::wstring>*>(p);
            v->push_back(IS_INTRESOURCE(n) ? L"#" + std::to_wstring(ULONG_PTR(n)) : std::wstring(n));
            return TRUE;
        }, LONG_PTR(&names));
        for (auto& n : names) {
            std::vector<WORD> langs;
            EnumResourceLanguagesW(h, arg(t), arg(n), [](HMODULE, LPCWSTR, LPCWSTR, WORD l, LONG_PTR p) -> BOOL {
                reinterpret_cast<std::vector<WORD>*>(p)->push_back(l);
                return TRUE;
            }, LONG_PTR(&langs));
            for (WORD l : langs) {
                HRSRC r = FindResourceExW(h, arg(t), arg(n), l);
                const DWORD size = SizeofResource(h, r);
                const auto* p = static_cast<const std::uint8_t*>(LockResource(LoadResource(h, r)));
                items.push_back({t, n, l, std::vector<std::uint8_t>(p, p + size)});
            }
        }
    }
    (void)key;
    FreeLibrary(h);
    HANDLE u = BeginUpdateResourceW(target.c_str(), FALSE);
    if (!u) throw SetupError{"cannot update " + Narrow(target)};
    for (auto& it : items)
        if (!UpdateResourceW(u, arg(it.type), arg(it.name), it.lang, it.data.data(), DWORD(it.data.size()))) {
            EndUpdateResourceW(u, TRUE);
            throw SetupError{"cannot copy the resources into " + Narrow(target)};
        }
    if (!EndUpdateResourceW(u, FALSE)) throw SetupError{"cannot finish the resources of " + Narrow(target)};
    return int(items.size());
}

std::wstring InstallRemake(const std::wstring& payload, const std::wstring& dest, const Log& log)
{
    if (GetFileAttributesW(payload.c_str()) == INVALID_FILE_ATTRIBUTES)
        throw SetupError{"the remake program (payload\\Recoil Redux.exe) is missing from this download"};
    const std::wstring target = dest + L"\\" + kExeName;
    if (!CopyFileW(payload.c_str(), target.c_str(), FALSE)) throw SetupError{"cannot write " + Narrow(target)};
    const int n = CopyResources(target, dest + L"\\original\\Recoil.exe");
    log("Recoil Redux.exe installed (" + std::to_string(n) + " menus, dialogs and icons copied from your Recoil.exe)");
    return target;
}

void MakeShortcut(const std::wstring& target, const Log& log)
{
    bool ok = false;
    PWSTR desk = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &desk))) {
        IShellLinkW* sl = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void**>(&sl)))) {
            sl->SetPath(target.c_str());
            sl->SetWorkingDirectory(target.substr(0, target.find_last_of(L'\\')).c_str());
            sl->SetIconLocation(target.c_str(), 0);
            IPersistFile* pf = nullptr;
            if (SUCCEEDED(sl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&pf)))) {
                ok = SUCCEEDED(pf->Save((std::wstring(desk) + L"\\Recoil Redux.lnk").c_str(), TRUE));
                pf->Release();
            }
            sl->Release();
        }
        CoTaskMemFree(desk);
    }
    log(ok ? "desktop shortcut: Recoil Redux" : "desktop shortcut could not be created (start Recoil Redux.exe from the install folder)");
}

// Recoil Redux v1 settings (05_remake/redux, REDUX.md: every feature is a switch; deleting the file plays Recoil Classic).
// Must stay byte-identical to tools/setup/setup_core.py redux_ini_text (compare_setup.py checks it).
std::string ReduxIniText(bool pack, bool map)
{
    return std::string(
        "; Recoil Redux settings, written by Setup. Delete this file (or empty the values) to play Recoil Classic, the exact original.\r\n"
        "[Display]\r\n"
        "; 1 = the M map shows Redux's mission map (redux_map, made by Setup): objectives, weapons, tank upgrades, secrets, you.\r\n"
        "Map=") + (map ? "1" : "0") + "\r\n"
        "; Game resolution, width x height (Redux v1 is made for 1920x1080). Empty = the original Options menu sizes (640x480 etc.).\r\n"
        "Resolution=1920x1080\r\n"
        "; Wide HUD: the top strip and the weapon panel stretched to the screen width. 0 = off.\r\n"
        "WideHUD=1\r\n"
        "[Textures]\r\n"
        "; Folder of high-resolution textures (the asset pack). Empty = the original textures.\r\n"
        "Pack=" + (pack ? "redux_textures" : "") + "\r\n"
        "[Tools]\r\n"
        "; Screenshot key (F1-F24) for problem reports: saves to Pictures\\Recoil Redux Screenshots\\Bug Reports with a .txt of the settings.\r\n"
        "ScreenshotKey=F12\r\n"
        "; 1 = after a screenshot the game pauses and a box takes a note about it (Enter saves, Esc skips).\r\n"
        "ScreenshotNote=1\r\n"
        "; Hold for a burst of screenshots, 10 a second (burst_* folders beside the screenshots). Empty = off.\r\n"
        "RapidKey=F11\r\n"
        "[Launcher]\r\n"
        "; 1 = the start-up menu shows only what Redux supports (no multiplayer, software renderer, A3D, joystick).\r\n"
        "TrimMenu=1\r\n"
        "; 1 = settings changed in the game are kept for the next start.\r\n"
        "KeepSettings=1\r\n"
        "; Pilot name offered for a new game (instead of your Windows user name).\r\n"
        "DefaultName=User\r\n"
        "[Graphics]\r\n"
        "; 1 = the graphics options start at the game's top tier. 0 = the 1999 hardware test.\r\n"
        "MaxDetail=1\r\n"
        "; 0 = the game starts with the lighting option off (brighter). 1 = on.\r\n"
        "Lighting=0\r\n"
        "; 1 = textures get smaller copies for distance (no shimmer). 0 = off.\r\n"
        "Mipmaps=1\r\n"
        "; 1 = vehicle shadows ignore lights (no shadow flicker when enemies fire). 0 = off.\r\n"
        "ShadowFix=1\r\n";
}

void WriteReduxIni(const std::wstring& dest, bool pack, bool map, const Log& log)
{
    const std::string text = ReduxIniText(pack, map);
    WriteFileAll(dest + L"\\redux.ini", std::vector<std::uint8_t>(text.begin(), text.end()));
    log(std::string("Redux settings: 1920x1080, wide HUD, ") + (pack ? "asset pack textures" : "original textures") + " (redux.ini)");
}

// ---------------------------------------------------------------- asset pack
// A pack is a zip holding pack.txt (its name on the first line), redux_textures\ and/or redux_video\. Only those two folders
// are installed; anything else in the zip is ignored.
std::string PackName(const std::wstring& path)
{
    Source z;
    std::string why;
    if (!z.Open(path, why) || z.Kind() != "zip") return "";
    std::vector<std::uint8_t> d;
    for (std::size_t i = 0; i < z.EntryCount(); ++i)
        if (Lower(Basename(z.EntryPath(i))) == "pack.txt" && z.ReadEntry(i, d)) {
            std::string t(d.begin(), d.end());
            t = t.substr(0, t.find_first_of("\r\n"));
            return t.empty() ? "asset pack" : t;
        }
    for (std::size_t i = 0; i < z.EntryCount(); ++i)
        if (Lower(z.EntryPath(i)).find("redux_textures/") != std::string::npos) return "asset pack";
    return "";
}

bool InstallPack(const std::wstring& path, const std::wstring& dest, const Log& log, const Progress& progress)
{
    Source z;
    std::string why;
    if (!z.Open(path, why) || z.Kind() != "zip") throw SetupError{"cannot read the asset pack " + Narrow(path)};
    std::vector<std::pair<std::size_t, std::string>> files;
    for (std::size_t i = 0; i < z.EntryCount(); ++i) {
        const std::string p = z.EntryPath(i), low = Lower(p);
        for (const char* top : {"redux_textures/", "redux_video/"}) {
            const std::size_t k = low.find(top);
            if (k != std::string::npos && (k == 0 || low[k - 1] == '/')) files.push_back({i, p.substr(k)});
        }
    }
    if (files.empty()) throw SetupError{Narrow(path) + " is not a Recoil Redux asset pack (no redux_textures or redux_video folder)"};
    std::vector<std::uint8_t> d;
    int n = 0, tex = 0, vid = 0;
    for (auto& [i, rel] : files) {
        if (!z.ReadEntry(i, d)) throw SetupError{"cannot read " + rel + " from the asset pack"};
        WriteFileAll(Join(dest, rel), d);
        (Lower(rel).rfind("redux_textures/", 0) == 0 ? tex : vid)++;
        progress(++n, int(files.size()));
    }
    log("asset pack " + PackName(path) + ": " + std::to_string(tex) + " textures, " + std::to_string(vid) + " videos");
    return tex > 0;
}

// payload\Recoil Redux.exe next to Setup (the download); recoil_boot.exe beside it (Setup run from the build folder)
std::wstring DefaultPayload()
{
    const std::wstring p = ExeDir() + L"\\payload\\" + kExeName, b = ExeDir() + L"\\recoil_boot.exe";
    return GetFileAttributesW(p.c_str()) == INVALID_FILE_ATTRIBUTES && GetFileAttributesW(b.c_str()) != INVALID_FILE_ATTRIBUTES ? b : p;
}
std::wstring DefaultEsrgan() { return ExeDir() + L"\\payload\\esrgan"; }

// Explorer runs an exe opened inside a zip from a temporary copy without the rest of the zip
bool RunFromZip()
{
    std::wstring d = ExeDir();
    std::transform(d.begin(), d.end(), d.begin(), ::towlower);
    return d.find(L"\\appdata\\local\\temp\\") != std::wstring::npos;
}

// ---------------------------------------------------------------- an existing install: upgrade or clean install
// (user 2026-10-20: "upgrade keeps screenshots and save games; clean install is full clean")
const char kReduxVersion[] = "1.03";            // written to redux_version.txt by this Setup
const wchar_t kVersionFile[] = L"redux_version.txt";

bool IsReduxInstall(const std::wstring& dir)
{
    return !dir.empty() && GetFileAttributesW((dir + L"\\" + kExeName).c_str()) != INVALID_FILE_ATTRIBUTES &&
           GetFileAttributesW((dir + L"\\original\\Recoil.exe").c_str()) != INVALID_FILE_ATTRIBUTES;
}

std::string InstalledVersion(const std::wstring& dir)   // "1.0 or 1.01" before version files
{
    std::vector<std::uint8_t> d;
    if (!ReadAll(std::filesystem::path(dir) / kVersionFile, d) || d.empty()) return "1.0 or 1.01";
    std::string v(d.begin(), d.end());
    v = v.substr(0, v.find_first_of("\r\n"));
    return v.empty() ? "1.0 or 1.01" : v;
}

void WriteVersion(const std::wstring& dest)
{
    const std::string v = std::string(kReduxVersion) + "\r\n";
    WriteFileAll(dest + L"\\" + kVersionFile, std::vector<std::uint8_t>(v.begin(), v.end()));
}

// the folder of the "Recoil Redux" desktop shortcut that Setup makes, if it is a Redux install
std::wstring FindExisting()
{
    std::wstring found;
    PWSTR desk = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &desk))) {
        IShellLinkW* sl = nullptr;
        IPersistFile* pf = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void**>(&sl))) &&
            SUCCEEDED(sl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&pf))) &&
            SUCCEEDED(pf->Load((std::wstring(desk) + L"\\Recoil Redux.lnk").c_str(), STGM_READ))) {
            wchar_t dir[MAX_PATH] = {};
            if (SUCCEEDED(sl->GetWorkingDirectory(dir, MAX_PATH)) && IsReduxInstall(dir)) found = dir;
        }
        if (pf) pf->Release();
        if (sl) sl->Release();
        CoTaskMemFree(desk);
    }
    if (found.empty()) {   // the default install folder
        wchar_t home[MAX_PATH] = L"C:";
        GetEnvironmentVariableW(L"USERPROFILE", home, MAX_PATH);
        const std::wstring d = std::wstring(home) + L"\\Games\\Recoil Redux";
        if (IsReduxInstall(d)) found = d;
    }
    return found;
}

// adds the settings this version has and the file lacks; every value already there stays (the player's choices)
void MergeReduxIni(const std::wstring& dest, bool map, const Log& log)
{
    const std::wstring ini = dest + L"\\redux.ini";
    wchar_t pack[MAX_PATH] = {};
    GetPrivateProfileStringW(L"Textures", L"Pack", L"", pack, MAX_PATH, ini.c_str());
    const std::string text = ReduxIniText(pack[0] != 0, map);
    std::wstring section;
    int added = 0;
    std::size_t at = 0;
    while (at < text.size()) {
        std::size_t e = text.find('\n', at);
        if (e == std::string::npos) e = text.size();
        std::string line = text.substr(at, e - at);
        at = e + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == ';') continue;
        if (line[0] == '[') { section = Widen(line.substr(1, line.find(']') - 1)); continue; }
        const std::size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::wstring key = Widen(line.substr(0, eq)), value = Widen(line.substr(eq + 1));
        wchar_t got[8] = {};
        GetPrivateProfileStringW(section.c_str(), key.c_str(), L"\x01", got, 8, ini.c_str());
        if (got[0] == L'\x01') {
            WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), ini.c_str());
            ++added;
        }
    }
    log("Redux settings kept (redux.ini): " + std::to_string(added) + " new setting" + (added == 1 ? "" : "s") + " added");
}

// keeps everything in the folder - game files, textures, saved games, screenshots, settings - and replaces the program
std::wstring Upgrade(const std::wstring& dest, std::wstring payload, const Log& log)
{
    if (payload.empty()) payload = DefaultPayload();
    if (!IsReduxInstall(dest)) throw SetupError{"no Recoil Redux install in " + Narrow(dest)};
    log("upgrading Recoil Redux " + InstalledVersion(dest) + " in " + Narrow(dest) + " to " + kReduxVersion);
    const std::wstring target = InstallRemake(payload, dest, log);
    const bool map = GenerateMaps(dest, log) > 0;
    MergeReduxIni(dest, map, log);
    WriteVersion(dest);
    log(std::string("done - Recoil Redux ") + kReduxVersion + "; saved games, screenshots and settings kept");
    return target;
}

// full clean (user 2026-10-20): everything in the install folder and the game's saved options are removed first
void CleanOut(const std::wstring& dest, const Log& log)
{
    if (!IsReduxInstall(dest)) throw SetupError{Narrow(dest) + " is not a Recoil Redux install - nothing was deleted"};
    std::error_code ec;
    int n = 0;
    for (const auto& e : std::filesystem::directory_iterator(dest, ec)) {
        std::filesystem::remove_all(e.path(), ec);
        if (ec) throw SetupError{"cannot delete " + Narrow(e.path().wstring()) + " (is the game still running?)"};
        ++n;
    }
    // the options the game saves (Settings_SaveToRegistry; with [Launcher] KeepSettings they are read back from here)
    RegDeleteTreeW(HKEY_CURRENT_USER, L"SOFTWARE\\Zipper\\RECOIL");
    log("clean install: " + std::to_string(n) + " items removed from " + Narrow(dest) + ", saved game options reset");
}

// pack: an asset pack zip; generate: HD textures made here with Real-ESRGAN (esrgan dir); neither: the original textures
std::wstring Install(const std::wstring& src, const std::wstring& dgv, const std::wstring& dest, const std::wstring& pack,
                     bool generate, std::wstring esrgan, std::wstring payload, bool shortcut, const Log& log, const Progress& progress)
{
    if (payload.empty()) payload = DefaultPayload();
    if (GetFileAttributesW(payload.c_str()) == INVALID_FILE_ATTRIBUTES)   // fail before copying anything
        throw SetupError{RunFromZip() ? "Setup is running from inside the zip. Extract the whole zip first (right-click it, Extract All...), "
                                        "then run Setup.exe from the extracted folder."
                                      : "the remake program (payload\\Recoil Redux.exe) is missing - keep Setup.exe in the folder it came in, "
                                        "next to its payload folder"};
    DgvMembers m;
    OpenDgv(dgv, m);
    if (!pack.empty() && PackName(pack).empty())   // fail before copying anything
        throw SetupError{Narrow(pack) + " is not a Recoil Redux asset pack"};
    Source s;
    for (auto& l : CheckSource(s, src)) log(l);
    MakeDirs(dest);
    InstallGame(s, dest, log, progress);
    InstallDgv(m, dest, log);
    if (esrgan.empty()) esrgan = DefaultEsrgan();
    const bool tex = generate ? GenerateTextures(dest, esrgan, log, progress) > 0 : !pack.empty() && InstallPack(pack, dest, log, progress);
    const bool map = GenerateMaps(dest, log) > 0;
    WriteReduxIni(dest, tex, map, log);
    const std::wstring target = InstallRemake(payload, dest, log);
    WriteVersion(dest);
    if (shortcut) MakeShortcut(target, log);
    log("done - start Recoil Redux from the desktop shortcut or " + Narrow(target));
    return target;
}

// ---------------------------------------------------------------- command line
int RunCli(int argc, wchar_t** argv)
{
    std::wstring src, dgv, dest, payload, pack, esrgan, maps_only, upgrade;
    bool shortcut = true, generate = false, clean = false;
    for (int i = 1; i < argc; ++i) {
        const std::wstring a = argv[i];
        auto next = [&]() { return i + 1 < argc ? std::wstring(argv[++i]) : std::wstring(); };
        if (a == L"--source") src = next();
        else if (a == L"--dgvoodoo") dgv = next();
        else if (a == L"--dest") dest = next();
        else if (a == L"--payload") payload = next();
        else if (a == L"--pack") pack = next();
        else if (a == L"--generate") generate = true;
        else if (a == L"--esrgan") esrgan = next();
        else if (a == L"--no-shortcut") shortcut = false;
        else if (a == L"--maps") maps_only = next();
        else if (a == L"--upgrade") upgrade = next();
        else if (a == L"--clean") clean = true;
    }
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) AllocConsole();
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    auto print = [&](const std::string& s) {
        DWORD w;
        const std::string l = s + "\r\n";
        WriteFile(out, l.data(), DWORD(l.size()), &w, nullptr);
    };
    if (!maps_only.empty()) return GenerateMaps(maps_only, print) > 0 ? 0 : 3;   // test: the maps of an installed copy
    if (!upgrade.empty()) {
        try {
            Upgrade(upgrade, payload, print);
            return 0;
        } catch (const SetupError& e) {
            print("setup: " + e.msg);
            return 2;
        }
    }
    if (src.empty() || dgv.empty() || dest.empty()) {
        print("usage: Setup.exe --source COPY --dgvoodoo ZIP --dest DIR [--clean] [--pack ZIP | --generate] [--esrgan DIR] [--payload EXE] "
              "[--no-shortcut]\r\n       Setup.exe --upgrade DIR [--payload EXE]");
        return 1;
    }
    try {
        if (clean) CleanOut(dest, print);
        Install(src, dgv, dest, pack, generate, esrgan, payload, shortcut, print, [](int, int) {});
        return 0;
    } catch (const SetupError& e) {
        print("setup: " + e.msg);
        return 2;
    }
}

// ---------------------------------------------------------------- the window
enum : int {
    IdNext = 100, IdBack, IdDgvEdit, IdDgvBrowse, IdLink, IdSrcEdit, IdSrcBrowse, IdSrcFolder, IdDestEdit, IdDestBrowse, IdShortcut,
    IdPackList, IdPackBrowse, IdTexOriginal, IdTexGenerate, IdTexPack,
};
const UINT WmLog = WM_APP + 1, WmProgress = WM_APP + 2, WmDone = WM_APP + 3;

struct Ui {
    HWND wnd, next, info, bar, log, dgv, src, dest, shortcut, packs, packinfo, packbrowse, tex_original, tex_generate, tex_pack, example;
    std::vector<HWND> page1;
    HFONT font, bold;
    bool source_ok = false, busy = false, finished = false;
    std::wstring clean_dir;   // a Redux install to wipe before installing (the player chose "Clean install")
} ui;

std::wstring Text(HWND h)
{
    std::wstring s(GetWindowTextLengthW(h) + 1, L'\0');
    GetWindowTextW(h, s.data(), int(s.size()));
    s.resize(s.size() - 1);
    return s;
}

HWND Make(std::vector<HWND>& page, const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id = 0,
          HFONT f = nullptr)
{
    HWND c = CreateWindowExW(std::wstring(cls) == L"EDIT" ? WS_EX_CLIENTEDGE : 0, cls, text, WS_CHILD | style, x, y, w, h, ui.wnd,
                             HMENU(INT_PTR(id)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(c, WM_SETFONT, WPARAM(f ? f : ui.font), TRUE);
    page.push_back(c);
    return c;
}

// One page (user 2026-10-20): dgVoodoo2 zip, the Recoil copy, the install folder and an optional asset pack together.
void UpdateInstallButton()
{
    EnableWindow(ui.next, !ui.busy && !ui.finished ? ui.source_ok : ui.finished);
}

std::wstring PickFile(const wchar_t* title, const wchar_t* filter)
{
    wchar_t buf[MAX_PATH] = {};
    OPENFILENAMEW o{sizeof o};
    o.hwndOwner = ui.wnd;
    o.lpstrFilter = filter;
    o.lpstrFile = buf;
    o.nMaxFile = MAX_PATH;
    o.lpstrTitle = title;
    o.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    return GetOpenFileNameW(&o) ? buf : L"";
}

std::wstring PickFolder(const wchar_t* title)
{
    std::wstring r;
    IFileOpenDialog* d = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d)))) {
        DWORD o = 0;
        d->GetOptions(&o);
        d->SetOptions(o | FOS_PICKFOLDERS);
        d->SetTitle(title);
        IShellItem* it = nullptr;
        if (SUCCEEDED(d->Show(ui.wnd)) && SUCCEEDED(d->GetResult(&it))) {
            PWSTR p = nullptr;
            if (SUCCEEDED(it->GetDisplayName(SIGDN_FILESYSPATH, &p))) {
                r = p;
                CoTaskMemFree(p);
            }
            it->Release();
        }
        d->Release();
    }
    return r;
}

void SetSource(std::wstring path)
{
    if (path.empty()) return;
    if (EndsWithI(Narrow(path), "recoil.exe")) path = path.substr(0, path.find_last_of(L'\\'));
    SetWindowTextW(ui.src, path.c_str());
    SetWindowTextW(ui.info, L"checking...");
    UpdateWindow(ui.info);
    HCURSOR old = SetCursor(LoadCursor(nullptr, IDC_WAIT));
    try {
        Source s;
        std::string t;
        for (auto& l : CheckSource(s, path)) t += l + "   ";
        SetWindowTextW(ui.info, Widen(t).c_str());
        ui.source_ok = true;
    } catch (const SetupError& e) {
        SetWindowTextW(ui.info, Widen(e.msg).c_str());
        ui.source_ok = false;
    }
    SetCursor(old);
    UpdateInstallButton();
}

// the embedded quality example (PNG) as a bitmap for a STATIC control, through Windows Imaging Component
HBITMAP ExampleBitmap()
{
    IWICImagingFactory* f = nullptr;
    IWICStream* st = nullptr;
    IWICBitmapDecoder* dec = nullptr;
    IWICBitmapFrameDecode* fr = nullptr;
    IWICFormatConverter* cv = nullptr;
    HBITMAP bmp = nullptr;
    UINT w = 0, h = 0;
    if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f))) &&
        SUCCEEDED(f->CreateStream(&st)) &&
        SUCCEEDED(st->InitializeFromMemory(const_cast<BYTE*>(kQualityExamplePng), DWORD(sizeof kQualityExamplePng))) &&
        SUCCEEDED(f->CreateDecoderFromStream(st, nullptr, WICDecodeMetadataCacheOnDemand, &dec)) && SUCCEEDED(dec->GetFrame(0, &fr)) &&
        SUCCEEDED(f->CreateFormatConverter(&cv)) &&
        SUCCEEDED(cv->Initialize(fr, GUID_WICPixelFormat32bppBGR, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom)) &&
        SUCCEEDED(cv->GetSize(&w, &h))) {
        BITMAPINFO bi{};
        bi.bmiHeader.biSize = sizeof bi.bmiHeader;
        bi.bmiHeader.biWidth = LONG(w);
        bi.bmiHeader.biHeight = -LONG(h);
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        void* bits = nullptr;
        bmp = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (bmp) cv->CopyPixels(nullptr, w * 4, w * h * 4, static_cast<BYTE*>(bits));
    }
    if (cv) cv->Release();
    if (fr) fr->Release();
    if (dec) dec->Release();
    if (st) st->Release();
    if (f) f->Release();
    return bmp;
}

// the asset pack field and its Browse button are only active with "Asset pack" chosen
void UpdateTextureChoice()
{
    const bool pack = SendMessageW(ui.tex_pack, BM_GETCHECK, 0, 0) == BST_CHECKED;
    EnableWindow(ui.packs, pack);
    EnableWindow(ui.packbrowse, pack);
    EnableWindow(ui.packinfo, pack);
}

void SetPack(const std::wstring& path)
{
    if (path.empty()) return;
    const std::string name = PackName(path);
    if (name.empty()) {
        MessageBoxW(ui.wnd, (path + L" is not a Recoil Redux asset pack.").c_str(), kTitle, MB_ICONWARNING);
        return;
    }
    SetWindowTextW(ui.packs, path.c_str());
    SetWindowTextW(ui.packinfo, Widen(name).c_str());
}

void AppendLog(const std::string& s)
{
    const std::wstring w = Widen(s + "\r\n");
    const int n = GetWindowTextLengthW(ui.log);
    SendMessageW(ui.log, EM_SETSEL, n, n);
    SendMessageW(ui.log, EM_REPLACESEL, FALSE, LPARAM(w.c_str()));
}

void StartInstall()
{
    try {   // the dgVoodoo zip and the pack are checked here, before anything is copied
        DgvMembers m;
        OpenDgv(Text(ui.dgv), m);
    } catch (const SetupError& e) {
        MessageBoxW(ui.wnd, Widen(e.msg).c_str(), kTitle, MB_ICONWARNING);
        return;
    }
    const bool use_pack = SendMessageW(ui.tex_pack, BM_GETCHECK, 0, 0) == BST_CHECKED;
    const bool generate = SendMessageW(ui.tex_generate, BM_GETCHECK, 0, 0) == BST_CHECKED;
    const std::wstring dest = Text(ui.dest), pack = use_pack ? Text(ui.packs) : L"";
    if (use_pack && pack.empty()) {
        MessageBoxW(ui.wnd, L"Select the asset pack zip with Browse..., or choose another texture option.", kTitle, MB_ICONWARNING);
        return;
    }
    if (dest.empty()) {
        MessageBoxW(ui.wnd, L"Choose an install folder.", kTitle, MB_ICONWARNING);
        return;
    }
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((dest + L"\\*").c_str(), &fd);
    bool empty = true;
    if (h != INVALID_HANDLE_VALUE) {
        do
            if (wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L"..")) empty = false;
        while (empty && FindNextFileW(h, &fd));
        FindClose(h);
    }
    const bool clean = !ui.clean_dir.empty() && _wcsicmp(ui.clean_dir.c_str(), dest.c_str()) == 0;
    if (clean) {
        if (MessageBoxW(ui.wnd, (L"Clean install: everything in " + dest + L" will be deleted, including saved games and the game's saved "
                                 L"options, and Recoil Redux installed fresh.\r\n\r\nContinue?").c_str(),
                        kTitle, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
            return;
    } else if (!empty && MessageBoxW(ui.wnd, (dest + L" is not empty. Install into it anyway?").c_str(), kTitle, MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;
    ui.busy = true;
    UpdateInstallButton();
    const std::wstring src = Text(ui.src), dgv = Text(ui.dgv);
    const bool sc = SendMessageW(ui.shortcut, BM_GETCHECK, 0, 0) == BST_CHECKED;
    HWND wnd = ui.wnd;
    std::thread([=] {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        auto log = [wnd](const std::string& s) { PostMessageW(wnd, WmLog, 0, LPARAM(new std::string(s))); };
        bool ok = false;
        try {
            if (clean) CleanOut(dest, log);
            Install(src, dgv, dest, pack, generate, L"", L"", sc, log, [wnd](int i, int n) { PostMessageW(wnd, WmProgress, WPARAM(i), LPARAM(n)); });
            ok = true;
        } catch (const SetupError& e) {
            log("FAILED: " + e.msg);
        } catch (const std::exception& e) {
            log(std::string("FAILED: ") + e.what());
        }
        PostMessageW(wnd, WmDone, ok, 0);
        CoUninitialize();
    }).detach();
}

// Setup found Recoil Redux already installed: upgrade it (nothing else to fill in) or clean install (the normal page, the
// folder emptied first). Returns false when the player cancels.
bool AskExisting()
{
    const std::wstring dir = FindExisting();
    if (dir.empty()) return true;
    const std::wstring head = L"Recoil Redux " + Widen(InstalledVersion(dir)) + L" is already installed";
    const std::wstring body = L"in " + dir + L"\r\n\r\nThis Setup installs Recoil Redux " + Widen(kReduxVersion) + L".";
    const std::wstring up = L"Upgrade to " + Widen(kReduxVersion) +
                            L"\nKeeps your saved games, screenshots, settings and textures. Takes a few seconds.";
    TASKDIALOG_BUTTON buttons[] = {
        {1000, up.c_str()},
        {1001, L"Clean install\nDeletes everything in the folder, including saved games and the game's saved options, then installs "
               L"Recoil Redux fresh (you choose the dgVoodoo2 zip and your Recoil copy as for a first install)."},
    };
    TASKDIALOGCONFIG c{sizeof c};
    c.hwndParent = ui.wnd;
    c.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION;
    c.dwCommonButtons = TDCBF_CANCEL_BUTTON;
    c.pszWindowTitle = kTitle;
    c.pszMainIcon = TD_INFORMATION_ICON;
    c.pszMainInstruction = head.c_str();
    c.pszContent = body.c_str();
    c.cButtons = 2;
    c.pButtons = buttons;
    c.nDefaultButton = 1000;
    int pressed = IDCANCEL;
    if (FAILED(TaskDialogIndirect(&c, &pressed, nullptr, nullptr)) || pressed == IDCANCEL) return false;
    SetWindowTextW(ui.dest, dir.c_str());
    if (pressed == 1001) {
        ui.clean_dir = dir;
        AppendLog("Clean install into " + Narrow(dir) + ": choose the dgVoodoo2 zip, your Recoil copy and the textures, then Install.");
        return true;
    }
    // upgrade: the page is not needed
    for (HWND h : ui.page1)
        if (h != ui.log && h != ui.bar && h != ui.next) EnableWindow(h, FALSE);
    ui.busy = true;
    UpdateInstallButton();
    HWND wnd = ui.wnd;
    std::thread([=] {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        auto log = [wnd](const std::string& s) { PostMessageW(wnd, WmLog, 0, LPARAM(new std::string(s))); };
        bool ok = false;
        try {
            Upgrade(dir, L"", log);
            ok = true;
        } catch (const SetupError& e) {
            log("FAILED: " + e.msg);
        } catch (const std::exception& e) {
            log(std::string("FAILED: ") + e.what());
        }
        PostMessageW(wnd, WmDone, ok, 1);
        CoUninitialize();
    }).detach();
    return true;
}

LRESULT CALLBACK WndProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IdNext:
            if (ui.finished) DestroyWindow(w);
            else if (!ui.busy) StartInstall();
            break;
        case IdPackBrowse: SetPack(PickFile(L"Select a Recoil Redux asset pack", L"asset packs\0*.zip\0all files\0*.*\0")); break;
        case IdTexOriginal:
        case IdTexGenerate:
        case IdTexPack: UpdateTextureChoice(); break;
        case IdLink: ShellExecuteA(w, "open", kDgvLink, nullptr, nullptr, SW_SHOWNORMAL); break;
        case IdDgvBrowse: {
            const std::wstring p = PickFile(L"Select the dgVoodoo2 zip", L"zip files\0*.zip\0all files\0*.*\0");
            if (!p.empty()) SetWindowTextW(ui.dgv, p.c_str());
            break;
        }
        case IdSrcBrowse:
            SetSource(PickFile(L"Select your Recoil copy", L"Recoil copy\0*.zip;*.iso;*.bin;*.cue;*.nrg;*.exe\0all files\0*.*\0"));
            break;
        case IdSrcFolder: SetSource(PickFolder(L"Select the Recoil folder or CD")); break;
        case IdSrcEdit:
            if (HIWORD(wp) == EN_KILLFOCUS && !Text(ui.src).empty()) SetSource(Text(ui.src));
            break;
        case IdDestBrowse: {
            const std::wstring p = PickFolder(L"Install folder");
            if (!p.empty()) SetWindowTextW(ui.dest, p.c_str());
            break;
        }
        }
        return 0;
    case WmLog: {
        auto* s = reinterpret_cast<std::string*>(lp);
        AppendLog(*s);
        delete s;
        return 0;
    }
    case WmProgress:
        SendMessageW(ui.bar, PBM_SETRANGE32, 0, lp);
        SendMessageW(ui.bar, PBM_SETPOS, wp, 0);
        return 0;
    case WmDone:
        ui.busy = false;
        if (wp) {
            ui.finished = true;
            SetWindowTextW(ui.next, L"Close");
            MessageBoxW(w, lp ? L"Recoil Redux is upgraded." : L"Recoil Redux is installed.", kTitle, MB_ICONINFORMATION);
        }
        UpdateInstallButton();
        return 0;
    case WM_CTLCOLORSTATIC:
        SetBkMode(HDC(wp), TRANSPARENT);
        return LRESULT(GetSysColorBrush(COLOR_WINDOW));
    case WM_CLOSE:
        if (ui.busy && MessageBoxW(w, L"Setup is still copying files. Stop it?", kTitle, MB_YESNO) != IDYES) return 0;
        DestroyWindow(w);
        return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(w, msg, wp, lp);
}

int RunGui(HINSTANCE inst)
{
    if (GetFileAttributesW(DefaultPayload().c_str()) == INVALID_FILE_ATTRIBUTES) {   // tell the player before they fill anything in
        MessageBoxW(nullptr,
                    RunFromZip() ? L"Setup is running from inside the zip.\r\n\r\nExtract the whole zip first (right-click it, Extract All...), "
                                   L"then run Setup.exe from the extracted folder."
                                 : L"The payload folder is missing.\r\n\r\nKeep Setup.exe in the folder it came in, next to its payload folder.",
                    kTitle, MB_ICONWARNING);
        return 1;
    }
    INITCOMMONCONTROLSEX icc{sizeof icc, ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    wc.lpszClassName = L"RecoilReduxSetup";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassW(&wc);
    const UINT dpi = GetDpiForSystem();
    auto S = [dpi](int v) { return MulDiv(v, int(dpi), 96); };
    RECT r{0, 0, S(660), S(700)};
    AdjustWindowRect(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    ui.wnd = CreateWindowW(wc.lpszClassName, kTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT,
                           CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr, nullptr, inst, nullptr);
    ui.font = CreateFontW(-S(12), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    ui.bold = CreateFontW(-S(14), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    auto& p = ui.page1;
    const int L = S(20), W = S(620), LW = S(110), EX = L + S(110), EW = S(410), BX = L + S(528), BW = S(92);
    const DWORD V = WS_VISIBLE;
    // 1. dgVoodoo2
    Make(p, L"STATIC", L"1. dgVoodoo2", V, L, S(12), W, S(22), 0, ui.bold);
    Make(p, L"STATIC", Widen(std::string("Recoil uses DirectDraw, which modern Windows needs help with: download the free dgVoodoo2 zip "
                                         "(tested with ") + kDgvTested + ") and select it.").c_str(),
         V, L, S(36), S(440), S(34));
    Make(p, L"BUTTON", L"Open the download page", V | BS_PUSHBUTTON, L + S(460), S(36), S(160), S(26), IdLink);
    Make(p, L"STATIC", L"dgVoodoo2 zip:", V, L, S(80), LW, S(20));
    ui.dgv = Make(p, L"EDIT", L"", V | ES_AUTOHSCROLL, EX, S(77), EW, S(24), IdDgvEdit);
    Make(p, L"BUTTON", L"Browse...", V, BX, S(76), BW, S(26), IdDgvBrowse);
    // 2. the player's copy and the install folder
    Make(p, L"STATIC", L"2. Your copy of Recoil", V, L, S(120), W, S(22), 0, ui.bold);
    Make(p, L"STATIC", L"The installed game folder, a zip of it, the CD, or a disc image (.iso, .bin/.cue, .nrg) of the 1999-01-29 "
                       L"build (\"Recoil Classic\" CD). A full disc image also gives the CD music.",
         V, L, S(144), W, S(34));
    Make(p, L"STATIC", L"Recoil copy:", V, L, S(188), LW, S(20));
    ui.src = Make(p, L"EDIT", L"", V | ES_AUTOHSCROLL, EX, S(185), EW, S(24), IdSrcEdit);
    Make(p, L"BUTTON", L"Browse...", V, BX, S(184), BW, S(26), IdSrcBrowse);
    Make(p, L"BUTTON", L"Select a folder instead...", V, BX - S(98), S(214), S(190), S(26), IdSrcFolder);
    ui.info = Make(p, L"STATIC", L"", V, L, S(218), S(410), S(34));
    Make(p, L"STATIC", L"Install to:", V, L, S(262), LW, S(20));
    wchar_t home[MAX_PATH] = L"C:";
    GetEnvironmentVariableW(L"USERPROFILE", home, MAX_PATH);
    ui.dest = Make(p, L"EDIT", (std::wstring(home) + L"\\Games\\Recoil Redux").c_str(), V | ES_AUTOHSCROLL, EX, S(259), EW, S(24), IdDestEdit);
    Make(p, L"BUTTON", L"Browse...", V, BX, S(258), BW, S(26), IdDestBrowse);
    // 3. textures: choices on the left, the quality example (tank track) on the right
    const int CW = S(296);
    Make(p, L"STATIC", L"3. Textures", V, L, S(300), W, S(22), 0, ui.bold);
    ui.tex_original = Make(p, L"BUTTON", L"Original textures", V | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, L, S(328), CW, S(22), IdTexOriginal);
    ui.tex_generate = Make(p, L"BUTTON", L"Generate HD textures on this PC (Real-ESRGAN)", V | BS_AUTORADIOBUTTON, L, S(354), CW, S(22),
                           IdTexGenerate);
    Make(p, L"STATIC", L"Upscaled from your own game files. Takes about 15 minutes on a modern PC; needs a graphics card "
                       L"with Vulkan, otherwise a smooth enlargement is made.",
         V, L + S(18), S(376), CW - S(18), S(50));
    ui.tex_pack = Make(p, L"BUTTON", L"Asset pack (high-resolution textures and HD videos)", V | BS_AUTORADIOBUTTON, L, S(428), CW, S(22),
                       IdTexPack);
    SendMessageW(ui.tex_original, BM_SETCHECK, BST_CHECKED, 0);
    ui.example = Make(p, L"STATIC", L"", V | SS_BITMAP | SS_REALSIZECONTROL, L + S(310), S(330), S(310), S(119));
    if (HBITMAP bmp = ExampleBitmap()) SendMessageW(ui.example, STM_SETIMAGE, IMAGE_BITMAP, LPARAM(bmp));
    ui.packs = Make(p, L"EDIT", L"", V | ES_AUTOHSCROLL | ES_READONLY, L + S(18), S(474), S(492), S(24), IdPackList);
    ui.packbrowse = Make(p, L"BUTTON", L"Browse...", V, BX, S(473), BW, S(26), IdPackBrowse);
    ui.packinfo = Make(p, L"STATIC", L"", V, L + S(18), S(502), S(492), S(20));
    UpdateTextureChoice();
    // install
    ui.shortcut = Make(p, L"BUTTON", L"Create a desktop shortcut", V | BS_AUTOCHECKBOX | WS_GROUP, L, S(528), S(300), S(22), IdShortcut);
    SendMessageW(ui.shortcut, BM_SETCHECK, BST_CHECKED, 0);
    ui.bar = Make(p, PROGRESS_CLASSW, L"", V, L, S(556), W, S(14));
    ui.log = Make(p, L"EDIT", L"", V | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL, L, S(576), W, S(74));
    ui.next = Make(p, L"BUTTON", L"Install", V | BS_DEFPUSHBUTTON, S(544), S(660), S(96), S(28), IdNext);
    UpdateInstallButton();
    ShowWindow(ui.wnd, SW_SHOW);
    if (!AskExisting()) {
        DestroyWindow(ui.wnd);
        return 0;
    }
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0)) {
        if (IsDialogMessageW(ui.wnd, &m)) continue;
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    return 0;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    const int rc = argc > 1 ? RunCli(argc, argv) : RunGui(inst);
    LocalFree(argv);
    CoUninitialize();
    return rc;
}
