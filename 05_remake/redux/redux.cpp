// redux.cpp - Recoil Redux features (RX-01 switches, RX-03 native resolutions). Not Recoil code: every replacement below
// names the original function it stands in for, and is installed only when its switch in redux.ini is on.
//
// How a feature replaces original behaviour: at start-up, before the game runs, the first bytes of the ported function
// are overwritten with a jump to the Redux version (a hook). The exe on disk is unchanged, so with a switch off the
// original function runs exactly as proven.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincodec.h>
#include <shlobj.h>

#include "redux.h"
#include "GameZRecoil/zVideo/zvid_init.h"
#include "platform/image/original_data.h"
#include "unattributed/settings.h"
#include "unattributed/texture.h"
#include "unattributed/hud.h"
#include "unattributed/asset_io_misc.h"
#include "GameZRecoil/zReader/zreader.h"
#include "unattributed/view.h"
#include "unattributed/mission.h"
#include "GameZRecoil/zVideo/zvid_dd.h"
#include "GameZRecoil/zVideo/zvid_ddd3d.h"
#include "unattributed/poly_shade.h"
#include "platform/advapi.h"
#include "GameZRecoil/zInput/zin_kbd.h"
#include "Battlesport/map.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace redux {
namespace {

using namespace recoil;

const char kIni[] = ".\\redux.ini";   // PLATFORM: the settings file, in the game folder (the launcher's working directory)

// A callable copy of an original function's first `len` bytes followed by a jump back into it, so a hooked function can
// still be run. `expect` must equal those bytes (whole instructions, no relative operands) or no trampoline is made.
void* Trampoline(void* target, const unsigned char* expect, int len)
{
    if (std::memcmp(target, expect, len) != 0) return nullptr;
    auto* t = static_cast<unsigned char*>(VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!t) return nullptr;
    std::memcpy(t, target, len);
    const std::int32_t rel = static_cast<std::int32_t>((static_cast<unsigned char*>(target) + len) - (t + len + 5));
    t[len] = 0xE9;
    std::memcpy(t + len + 1, &rel, 4);
    return t;
}

// Writes a 5-byte JMP to `replacement` over the entry of `target`.
bool Hook(void* target, void* replacement, const char* name)
{
    auto* p = static_cast<unsigned char*>(target);
    DWORD old = 0;
    if (!VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &old)) {
        std::fprintf(stderr, "redux: cannot hook %s\n", name);
        return false;
    }
    const std::int32_t rel = static_cast<std::int32_t>(static_cast<unsigned char*>(replacement) - (p + 5));
    p[0] = 0xE9;   // JMP rel32
    std::memcpy(p + 1, &rel, 4);
    VirtualProtect(p, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, 5);
    return true;
}

// ------------------------------------------------------------------------------------------------ RX-03 resolutions
// The launcher's Options menu offers six video modes, 2..7 (320x200 .. 1024x768). Two original functions turn a mode
// into sizes: Settings_ApplyVideoModePreset (viewports, screen rect) and Render_ApplyResolutionPreset (render and surface
// sizes the DirectDraw mode is set from). With [Display] Resolution set, every mode uses that size instead; the game's
// Direct3D path then renders at it, and a wider-than-4:3 size shows a wider view.
int g_width = 0, g_height = 0;

std::uint32_t& Data(std::uint32_t offset)   // a dword of the .data/.bss mirror: offset from 0x004da000
{
    return *reinterpret_cast<std::uint32_t*>(g_Data_004da000 + offset);
}

// Stands in for Settings_ApplyVideoModePreset 0x00408720 (same arguments and results, mode 7's body with the Redux size)
int __fastcall ApplyVideoModePreset(int mode, int)
{
    auto* setting = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(g_SettingsBlock_004e5d00[0x30 / 4]));   // CONFIRMED-BINARY: [0x004e5d30], the stored mode
    if (mode < 2 || mode > 7) {   // CONFIRMED-BINARY: modes outside 2..7 store 0 (0x00408995)
        *setting = 0;
        return static_cast<int>(reinterpret_cast<std::uintptr_t>(setting));
    }
    *setting = static_cast<std::uint32_t>(mode);   // keep the player's menu choice, so the registry setting is unchanged
    Viewport0_SetOrigin(0, 0);
    Viewport0_SetSizeF(g_width, g_height);
    Settings_ScreenRect_SetOrigin(0, 0);
    Settings_ScreenRect_SetSize(g_width, g_height);
    Viewport1_SetOrigin(0, 0);
    Viewport1_SetSize(g_width, g_height);
    Settings_SetDisplayRect_20(16, 0);       // CONFIRMED-BINARY: 16-bit colour (mode 7 case, 0x00408930)
    return Settings_Store_004e5d6c(0, 0);    // CONFIRMED-BINARY: not pixel-doubled (mode 7 case, 0x00408930)
}

// Stands in for Render_ApplyResolutionPreset 0x004a7990 (mode 7's body with the Redux size)
int __fastcall ApplyResolutionPreset(int mode, int)
{
    if (mode < 2 || mode > 7) {
        Data(0x15814c) = 0;   // CONFIRMED-BINARY: [0x0063214c] = 0 for other indices (0x004a7acd)
        return mode - 2;
    }
    Data(0x158128) = 0;                   // CONFIRMED-BINARY: [0x00632128] pixel doubling off (mode 7 case, 0x004a7a87)
    Data(0x158240) = Data(0x158200) = Data(0x158220) = static_cast<std::uint32_t>(g_width);    // surface / render / copy width
    Data(0x158244) = Data(0x158204) = Data(0x158224) = static_cast<std::uint32_t>(g_height);   // surface / render / copy height
    Data(0x158150) = 16;                  // CONFIRMED-BINARY: [0x00632150] colour depth 16 (0x004a7ab4 tail)
    return 16;
}

// ------------------------------------------------------------------------------------------------ RX-04 wide HUD images
// At a resolution wider than 640 the game already places the score text in the centre, the timer at the right edge and the
// weapon icons in the centre, but two HUD backgrounds are fixed 640-wide images: "topstrip" (640x26, drawn at x 0) and
// "iconback" (385x84, the weapon panel, drawn at x 135). Redux builds wider versions from the player's own images when they
// load: the left part stays, the middle part moves right by half the extra width (D), the right part by D twice, and the
// gaps repeat the image's own filler. Nothing is shipped: every pixel comes from image.zbd.
// One band of rows: columns [0, cut_a) stay, [cut_a, cut_b) move right by D, [cut_b, width) by 2D; the two gaps repeat
// the source columns [fill_a, fill_a + period_a) and [fill_b, fill_b + period_b) of the same rows. MEASURED-ASSET: all
// columns and rows were read from the player's image.zbd (2026-10-08) and checked against the reference frames.
struct Band {
    int row_end;   // rows up to (not including) this one
    int cut_a, cut_b, fill_a, period_a, fill_b, period_b;
};
struct WidePlan {
    const char* name;
    int src_width;
    Band bands[3];
    int band_count;
};
const WidePlan kWide[] = {
    // topstrip: logo 0-150, filler 150-272, score box 275-380, filler 385-533, timer box 535-620, end cap 620-640.
    // The score box follows the score text (centred), the timer box the timer (right edge).
    {"topstrip", 640, {{26, 200, 460, 160, 40, 420, 40}}, 1},
    // iconback (the weapon panel): left frame 0-15, the black weapon tray with its rounded ends 15-370 (nine slots, one
    // every 38 columns, circles under them), right frame 370-385; light bar on top (rows 0-20) with the three indicator
    // squares at 322-367. The light bar's right end with the squares moves to the panel's right end (+2D, where RX-04b
    // places the game's indicator strip). Below it the tray runs the panel's full width (user, 2026-10-08: "the same colour
    // as the weapon bar"): its rounded ends go to the panel's edges, the slots (with their circles) move with the weapon
    // icons (+D), cut at 41 / 343 between circles, and the gaps repeat column 86, a clean piece of tray (black interior,
    // top and bottom edge, no slot circle).
    {"iconback", 385, {{21, 24, 318, 200, 40, 278, 40},
                       {84, 41, 343, 86, 1, 86, 1}}, 2},
};
using ImageAllocFn = void*(__cdecl*)(std::size_t);
using ImageFreeFn = void(__cdecl*)(void*);

// image record (Image_Alloc 0x0046ec00): +0 pixel count, +4 width (short), +6 height (short), +9 flags (bit 0 = 16-bit,
// 0x20 = owns pixels, 0x40 = owns alpha), +0xE palette size, +0x10 pixels, +0x14 alpha plane, +0x34 width (int)
void Widen(unsigned char* img, const WidePlan& p)
{
    const int w = *reinterpret_cast<std::int16_t*>(img + 4), h = *reinterpret_cast<std::int16_t*>(img + 6);
    const int d = (g_width - 640) / 2;
    if (w != p.src_width || d <= 0 || h <= 0) return;
    const int nw = w + 2 * d;
    const unsigned flags = img[9];
    const int bpp = *reinterpret_cast<std::uint16_t*>(img + 0xE) ? 1 : 1 + (flags & 1);   // as Image_PixelDataSize 0x0046ec40
    // the game frees these buffers with the C runtime it imports (msvcrt), so allocate them from the same heap
    static ImageAllocFn mem_alloc = reinterpret_cast<ImageAllocFn>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
    static ImageFreeFn mem_free = reinterpret_cast<ImageFreeFn>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));
    if (!mem_alloc || !mem_free) return;
    auto src_col = [&](int x, int y) {   // which source column the new column x of row y shows
        const Band* bd = &p.bands[p.band_count - 1];
        for (int i = 0; i < p.band_count; ++i)
            if (y < p.bands[i].row_end) { bd = &p.bands[i]; break; }
        if (x < bd->cut_a) return x;
        if (x < bd->cut_a + d) return bd->fill_a + (x - bd->cut_a) % bd->period_a;   // gap 1
        if (x < bd->cut_b + d) return x - d;
        if (x < bd->cut_b + 2 * d) return bd->fill_b + (x - (bd->cut_b + d)) % bd->period_b;   // gap 2
        return x - 2 * d;
    };
    auto widen_plane = [&](unsigned char* src, int size) -> unsigned char* {
        auto* dst = static_cast<unsigned char*>(mem_alloc(static_cast<std::size_t>(nw) * h * size));
        if (!dst) return nullptr;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < nw; ++x)
                std::memcpy(dst + (static_cast<std::size_t>(y) * nw + x) * size, src + (static_cast<std::size_t>(y) * w + src_col(x, y)) * size, size);
        return dst;
    };
    auto** pixels = reinterpret_cast<unsigned char**>(img + 0x10);
    auto** alpha = reinterpret_cast<unsigned char**>(img + 0x14);
    unsigned char* np = widen_plane(*pixels, bpp);
    unsigned char* na = *alpha ? widen_plane(*alpha, 1) : nullptr;
    if (!np || (*alpha && !na)) return;
    if (flags & 0x20) mem_free(*pixels);
    if (*alpha && (flags & 0x40)) mem_free(*alpha);
    *pixels = np;
    if (*alpha) *alpha = na;
    img[9] = static_cast<unsigned char>(flags | 0x20 | (na ? 0x40 : 0));   // the image now owns both buffers
    *reinterpret_cast<std::uint32_t*>(img + 0) = static_cast<std::uint32_t>(nw * h);
    *reinterpret_cast<std::int16_t*>(img + 4) = static_cast<std::int16_t>(nw);
    *reinterpret_cast<std::int32_t*>(img + 0x34) = nw;
    std::fprintf(stderr, "redux: RX-04 %s widened %d -> %d\n", p.name, w, nw);
}

// ------------------------------------------------------------------------------------------------ RX-10 texture packs
// [Textures] Pack=<folder in the game folder>: <name>.png there replaces the texture <name> when the game loads it (any
// size; RGBA - the alpha channel is used when the game's texture has an alpha plane). Only 16-bit direct-colour textures
// (the hardware path's rtexture*.zbd) are replaced; paletted ones (the software path) are left as they are.
// A texture whose picture differs between missions under the same name has its own copy in <folder>\m<N>\, which wins
// while mission N is loaded (g_mission).
char g_pack_dir[MAX_PATH] = {};
bool g_pack_on = false;
bool InstallMissionHook();   // below
int g_mission = 0;   // the mission whose data is loaded: ECX of Mission_SetDataSearchPaths 0x0042ecb0 (CONFIRMED-BINARY: zbd\m%d)
bool g_wide_on = false;
bool g_image_hooked = false;

// RGBA8 pixels of a PNG (Windows Imaging Component; no extra library)
bool LoadPng(const char* path, int& w, int& h, std::vector<unsigned char>& rgba)
{
    static IWICImagingFactory* factory = [] {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);   // RPC_E_CHANGED_MODE is fine: COM is usable either way
        IWICImagingFactory* f = nullptr;
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f));
        return f;
    }();
    if (!factory) return false;
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, MAX_PATH);
    IWICBitmapDecoder* dec = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* conv = nullptr;
    bool ok = SUCCEEDED(factory->CreateDecoderFromFilename(wpath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &dec)) &&
              SUCCEEDED(dec->GetFrame(0, &frame)) && SUCCEEDED(factory->CreateFormatConverter(&conv)) &&
              SUCCEEDED(conv->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0,
                                         WICBitmapPaletteTypeCustom));
    UINT uw = 0, uh = 0;
    if (ok) ok = SUCCEEDED(conv->GetSize(&uw, &uh)) && uw && uh && uw <= 4096 && uh <= 4096;
    if (ok) {
        rgba.resize(static_cast<std::size_t>(uw) * uh * 4);
        ok = SUCCEEDED(conv->CopyPixels(nullptr, uw * 4, static_cast<UINT>(rgba.size()), rgba.data()));
    }
    if (conv) conv->Release();
    if (frame) frame->Release();
    if (dec) dec->Release();
    w = static_cast<int>(uw);
    h = static_cast<int>(uh);
    return ok;
}

void ReplaceFromPack(unsigned char* img, const char* name)
{
    const unsigned flags = img[9];
    if (!(flags & 1) || *reinterpret_cast<std::uint16_t*>(img + 0xE)) return;   // 16-bit direct colour only
    char base[64] = {};
    std::snprintf(base, sizeof base, "%s", name);
    if (char* dot = std::strrchr(base, '.')) *dot = 0;
    char path[MAX_PATH];
    std::snprintf(path, sizeof path, "%s\\m%d\\%s.png", g_pack_dir, g_mission, base);   // this mission's own version
    if (g_mission <= 0 || GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
        std::snprintf(path, sizeof path, "%s\\%s.png", g_pack_dir, base);
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) return;
    int w = 0, h = 0;
    std::vector<unsigned char> rgba;
    if (!LoadPng(path, w, h, rgba)) {
        std::fprintf(stderr, "redux: RX-10 cannot read %s\n", path);
        return;
    }
    static ImageAllocFn mem_alloc = reinterpret_cast<ImageAllocFn>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
    static ImageFreeFn mem_free = reinterpret_cast<ImageFreeFn>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));
    if (!mem_alloc || !mem_free) return;
    const std::size_t n = static_cast<std::size_t>(w) * h;
    auto* px = static_cast<std::uint16_t*>(mem_alloc(n * 2));
    auto** alpha = reinterpret_cast<unsigned char**>(img + 0x14);
    unsigned char* na = (flags & 0x8) || *alpha ? static_cast<unsigned char*>(mem_alloc(n)) : nullptr;   // the texture has an alpha plane
    if (!px || ((flags & 0x8 || *alpha) && !na)) return;
    const bool rgb555 = Data(0x15815c) == 5;   // CONFIRMED-BINARY: green bits [0x0063215c] = 5 -> 555, as Image_ReadPixels 0x0046eeba
    for (std::size_t i = 0; i < n; ++i) {
        const unsigned r = rgba[i * 4], g = rgba[i * 4 + 1], b = rgba[i * 4 + 2];
        px[i] = static_cast<std::uint16_t>(rgb555 ? ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)
                                                  : ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
        if (na) na[i] = rgba[i * 4 + 3];
    }
    auto** pixels = reinterpret_cast<unsigned char**>(img + 0x10);
    if (flags & 0x20) mem_free(*pixels);
    if (*alpha && (flags & 0x40)) mem_free(*alpha);
    *pixels = reinterpret_cast<unsigned char*>(px);
    if (na) *alpha = na;
    img[9] = static_cast<unsigned char>(flags | 0x20 | (na ? 0x40 : 0));
    *reinterpret_cast<std::uint32_t*>(img + 0) = static_cast<std::uint32_t>(n);
    *reinterpret_cast<std::int16_t*>(img + 4) = static_cast<std::int16_t>(w);
    *reinterpret_cast<std::int16_t*>(img + 6) = static_cast<std::int16_t>(h);
    *reinterpret_cast<std::int32_t*>(img + 0x34) = w;
    static int count = 0;
    if (++count <= 5 || count % 100 == 0) std::fprintf(stderr, "redux: RX-10 %s from the pack (%dx%d), %d so far\n", base, w, h, count);
}

// ------------------------------------------------------------------------------------------------ RX-34 maximum detail
// The game picks its graphics defaults from detail.zrd presets (CONFIRMED-DATA: zrdr.zbd detail.zrd): for each option the
// first entry whose hardware condition holds (CPU_MHZ / CPU_CLASS / RAM_KB / VIDEO_KB), listed best first. On a modern PC
// the 1999 probes misjudge it (the CPU speed probe returns 0 on non-Intel CPUs), so it lands on low tiers. With
// [Graphics] MaxDetail=1 each option takes its first (top) entry: texture memory MAX, sound HIGH,
// global lighting and perspective on, effects HIGH, object detail HIGH, HUD type I.
// The HUD type is the exception: its top tier is HUD_TYPEI, the minimal HUD ("Full HUD" off), which hides the panels the
// wide HUD (RX-04) builds - so HUDType_* takes its last entry, DEFAULT = HUD_TYPEII (the user, 2026-10-09: keep the full HUD).
//
// Stands in for Settings_GetHardwarePresetOrDefault 0x00407680 (ECX preset node, EDX option name, 1 stack arg = default,
// ret 4): the same walk over the option's entries 1..n-1 (CONFIRMED-BINARY 0x004076a1: entry i at [[child+4] + i*8 + 4],
// condition at +8, value at +0x10), picking an entry outright instead of testing the hardware.
int __fastcall PresetTopTier(int node, int name_arg, int fallback)
{
    if (!node) return fallback;
    const int child = ConfigTree_FindChild(node, name_arg);
    if (!child) return fallback;
    auto* list = *reinterpret_cast<unsigned char**>(static_cast<std::uintptr_t>(child) + 4);
    const int n = *reinterpret_cast<int*>(list + 4);
    const char* name = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(name_arg));
    if (n <= 1) return fallback;
    // HUDType_*: the last entry (DEFAULT = HUD_TYPEII, the full HUD) on every PC; every other option: the first (top) entry
    const int pick = name && _strnicmp(name, "HUDType", 7) == 0 ? n - 1 : 1;
    auto* item = *reinterpret_cast<unsigned char**>(list + pick * 8 + 4);
    int v = ConfigValue_ToInt(static_cast<int>(reinterpret_cast<std::uintptr_t>(item + 0x10)), 0);
    // [Graphics] Lighting=0 (user 2026-10-17: start with the lighting option off - lit scenes are much darker). The
    // Options-menu lighting switch is the GlobalLight_SW / GlobalLight_HW preset (bit 0x10 of GfxFlags_*; option names
    // CONFIRMED-DATA: the strings RecoilApp_LoadUserSettings 0x00407700 passes, .data 0x004da850 / 0x004da824)
    if (name && _strnicmp(name, "GlobalLight", 11) == 0 && GetPrivateProfileIntA("Graphics", "Lighting", 1, kIni) == 0) v = 0;
    return v;
}

bool InitMaxDetail()
{
    if (GetPrivateProfileIntA("Graphics", "MaxDetail", 0, kIni) != 1) return false;
    const bool ok = Hook(reinterpret_cast<void*>(&Settings_GetHardwarePresetOrDefault), reinterpret_cast<void*>(&PresetTopTier),
                         "Settings_GetHardwarePresetOrDefault");
    if (ok) std::fprintf(stderr, "redux: RX-34 graphics defaults at the game's top tier\n");
    return ok;
}

// ------------------------------------------------------------------------------------------------ screenshot key
// [Tools] ScreenshotKey=F12: the key saves the screen to <game folder>\screenshots\redux_<date>_<time>.png (with a short
// beep), for reporting problems. A watcher thread polls the key; it does not touch the game (no hook). PLATFORM only.
// The folder (user 2026-10-20): Pictures\Recoil Redux Screenshots\Bug Reports, or [Tools] ScreenshotFolder= (a full
// path, or relative to the game folder; "screenshots" = the old place in the game folder).
int g_shot_vk = 0;

const wchar_t* ShotDir()
{
    static wchar_t dir[MAX_PATH] = {};
    if (*dir) return dir;
    char cfg[MAX_PATH] = {};
    GetPrivateProfileStringA("Tools", "ScreenshotFolder", "", cfg, sizeof cfg, kIni);
    if (*cfg) {
        wchar_t w[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, cfg, -1, w, MAX_PATH);
        GetFullPathNameW(w, MAX_PATH, dir, nullptr);
    } else {
        PWSTR pics = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &pics))) {
            swprintf_s(dir, L"%s\\Recoil Redux Screenshots\\Bug Reports", pics);
            CoTaskMemFree(pics);
        } else {
            GetFullPathNameW(L"screenshots", MAX_PATH, dir, nullptr);
        }
    }
    SHCreateDirectoryExW(nullptr, dir, nullptr);
    return dir;
}
wchar_t g_note_txt[MAX_PATH] = {};   // the last screenshot's .txt (RX-38 appends the note)

bool SavePng(const wchar_t* path, HBITMAP bmp, int w, int h)
{
    IWICImagingFactory* f = nullptr;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f)))) return false;
    IWICBitmap* wb = nullptr;
    IWICStream* st = nullptr;
    IWICBitmapEncoder* enc = nullptr;
    IWICBitmapFrameEncode* fr = nullptr;
    bool ok = SUCCEEDED(f->CreateBitmapFromHBITMAP(bmp, nullptr, WICBitmapIgnoreAlpha, &wb)) &&
              SUCCEEDED(f->CreateStream(&st)) && SUCCEEDED(st->InitializeFromFilename(path, GENERIC_WRITE)) &&
              SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &enc)) &&
              SUCCEEDED(enc->Initialize(st, WICBitmapEncoderNoCache)) && SUCCEEDED(enc->CreateNewFrame(&fr, nullptr)) &&
              SUCCEEDED(fr->Initialize(nullptr)) && SUCCEEDED(fr->SetSize(w, h));
    WICPixelFormatGUID fmt = GUID_WICPixelFormat24bppBGR;
    if (ok) ok = SUCCEEDED(fr->SetPixelFormat(&fmt)) && SUCCEEDED(fr->WriteSource(wb, nullptr)) && SUCCEEDED(fr->Commit()) &&
                 SUCCEEDED(enc->Commit());
    if (fr) fr->Release();
    if (enc) enc->Release();
    if (st) st->Release();
    if (wb) wb->Release();
    f->Release();
    return ok;
}

void TakeScreenshot()
{
    // the game's own window when it is in front, else the whole primary screen (full screen: the same thing)
    RECT r{0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    if (fg && pid == GetCurrentProcessId()) GetWindowRect(fg, &r);
    const int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0) return;
    HDC screen = GetDC(nullptr);
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateCompatibleBitmap(screen, w, h);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, r.left, r.top, SRCCOPY | CAPTUREBLT);
    SelectObject(mem, old);
    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t path[MAX_PATH];
    const wchar_t* dir = ShotDir();
    swprintf_s(path, L"%s\\redux_%04d-%02d-%02d_%02d-%02d-%02d.png", dir, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    const bool ok = SavePng(path, bmp, w, h);
    // beside the picture: the game's settings (name = value), so a report says what the options were
    wchar_t txt[MAX_PATH];
    swprintf_s(txt, L"%.*s.txt", static_cast<int>(wcslen(path) - 4), path);
    if (FILE* f = _wfopen(txt, L"w")) {
        std::fprintf(f, "Recoil Redux screenshot %04d-%02d-%02d %02d:%02d:%02d  %dx%d\n", t.wYear, t.wMonth, t.wDay, t.wHour,
                     t.wMinute, t.wSecond, w, h);
        // settings block 0x004e5d00 (0x26 dwords, CONFIRMED-BINARY 0x00407700): each entry points at a settings node
        // (Settings_RegisterNode 0x004b2e80: value at +0 for types 0-2, name at +0x10)
        for (int i = 0; i < 0x26; ++i) {
            auto* node = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(g_SettingsBlock_004e5d00[i]));
            if (!node || IsBadReadPtr(node, 0x20)) continue;
            const char* name = *reinterpret_cast<const char* const*>(node + 0x10);
            if (!name || IsBadStringPtrA(name, 64)) continue;
            const int type = *reinterpret_cast<const int*>(node + 8);   // +8 type: 3-7 keep a buffer at +0 (settings.md)
            const char* text = type >= 3 ? *reinterpret_cast<const char* const*>(node) : nullptr;
            if (text && !IsBadStringPtrA(text, 64) && text[0] >= 32 && text[0] < 127)
                std::fprintf(f, "%-24s = \"%.40s\"\n", name, text);
            else
                std::fprintf(f, "%-24s = %d\n", name, *reinterpret_cast<const int*>(node));
        }
        std::fclose(f);
    }
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
    Beep(ok ? 1400 : 300, 70);
    std::fprintf(stderr, "redux: screenshot %ls%s\n", path, ok ? "" : " FAILED");
    if (ok) wcscpy_s(g_note_txt, txt);
}

// ------------------------------------------------------------------------------------------------ RX-38 screenshot note
// [Tools] ScreenshotNote=1: after the screenshot key, Redux presses Escape for the player so the game pauses in its own
// menu (user 2026-10-16: "use the escape function as it pauses the game"), then draws a note box over it. The first
// line is filled in: map (data folder zbd\m%d of Mission_SetDataSearchPaths 0x0042ecb0; not the mission number), player tank position and heading
// (V = [[0x004f36a8]+4], world position V+0x3ec..+0x3f4, yaw V+0x3c0, CONFIRMED-BINARY per vehicle.md). Enter saves the
// line and the typed comment into the screenshot's .txt, Esc skips; the box closes and the player resumes from the pause menu.
// Typing: while the box is open, Input_Keyboard_PollAndTranslate 0x0046f690 (trampoline) does not run; Redux reads the
// game's DirectInput keyboard itself (device [0x00561cc8], flag [0x00561cc4], GetDeviceData +0x28, Acquire +0x1c:
// CONFIRMED-BINARY 0x0046f450), so the menu never sees the letters; on closing, Input_Keyboard_InitialPollAndClear
// 0x0046f450 empties the buffer and key table. Drawing: zVideo_FlipFrame 0x004a9b70 (trampoline) draws the box with GDI
// on the render target [0x0063223c] before presenting.
volatile LONG g_note_stage = 0;   // 0 none, 1 Escape sent (waiting for the menu), 2 box open
volatile DWORD g_note_t = 0;
char g_note_head[200] = {};
std::string g_note_text;
bool g_note_shift = false;
using FlipFn = int(__fastcall*)(int, int, int, int);
FlipFn g_flip = nullptr;
using KbFn = int(__fastcall*)(int, int);
KbFn g_kb_poll = nullptr;
int g_note_draw_fail = 0;

void NoteHeader()
{
    char pos[120] = "tank position unknown";
    const std::uint32_t holder = Data(0x196a8);   // [0x004f36a8]
    if (holder && !IsBadReadPtr(reinterpret_cast<void*>(static_cast<std::uintptr_t>(holder)), 8)) {
        const auto* v = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(*reinterpret_cast<const std::uint32_t*>(
            static_cast<std::uintptr_t>(holder) + 4)));
        if (v && !IsBadReadPtr(v, 0x3f8)) {
            const float* p = reinterpret_cast<const float*>(v + 0x3ec);
            double deg = *reinterpret_cast<const float*>(v + 0x3c0) * 57.29577951308232;
            deg = std::fmod(std::fmod(deg, 360.0) + 360.0, 360.0);
            std::snprintf(pos, sizeof pos, "tank at X %.0f  Y %.0f  Z %.0f  heading %.0f deg", p[0], p[1], p[2], deg);
        }
    }
    SYSTEMTIME t;
    GetLocalTime(&t);
    std::snprintf(g_note_head, sizeof g_note_head, "Map %d  |  %s  |  %02d:%02d:%02d", g_mission, pos, t.wHour, t.wMinute,
                  t.wSecond);
}

RECT NoteBoxRect(int w, int h);

void DrawNoteBox(HDC dc, int w, int h)
{
    const int s = (std::max)(1, h / 540);   // 2 at 1080p
    RECT box = NoteBoxRect(w, h);
    HBRUSH bg = CreateSolidBrush(RGB(16, 24, 20)), edge = CreateSolidBrush(RGB(200, 160, 40));
    FillRect(dc, &box, edge);
    RECT in{box.left + 2 * s, box.top + 2 * s, box.right - 2 * s, box.bottom - 2 * s};
    FillRect(dc, &in, bg);
    HFONT f = CreateFontA(-11 * s, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, "Arial");
    HFONT fb = CreateFontA(-12 * s, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, "Arial");
    HGDIOBJ old = SelectObject(dc, fb);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(240, 200, 60));
    RECT t{in.left + 8 * s, in.top + 6 * s, in.right - 8 * s, in.bottom - 6 * s};
    DrawTextA(dc, "SCREENSHOT SAVED - describe the problem.    Enter = save note     Esc = no note", -1, &t, DT_SINGLELINE);
    SelectObject(dc, f);
    SetTextColor(dc, RGB(150, 200, 160));
    t.top += 20 * s;
    DrawTextA(dc, g_note_head, -1, &t, DT_SINGLELINE | DT_NOPREFIX);
    SetTextColor(dc, RGB(240, 240, 240));
    t.top += 18 * s;
    const std::string shown = g_note_text + (((GetTickCount() / 500) & 1) ? " " : "_");
    DrawTextA(dc, shown.c_str(), -1, &t, DT_WORDBREAK | DT_NOPREFIX);
    SelectObject(dc, old);
    DeleteObject(f);
    DeleteObject(fb);
    DeleteObject(bg);
    DeleteObject(edge);
}

void FinishNote(bool save)
{
    if (save && g_note_txt[0]) {
        if (FILE* f = _wfopen(g_note_txt, L"a")) {
            std::fprintf(f, "\n%s\nNOTE: %s\n", g_note_head, g_note_text.c_str());
            std::fclose(f);
        }
    }
    std::fprintf(stderr, "redux: RX-38 note %s\n", save ? "saved" : "skipped");
    g_note_text.clear();
    g_note_stage = 0;
    Input_Keyboard_InitialPollAndClear(0, 0);   // nothing typed reaches the menu; no key left held
}

// stand-in for Input_Keyboard_PollAndTranslate while the box is open: the keys edit the note
int __fastcall KbPollWithNote(int ecx, int edx)
{
    if (g_note_stage == 1 && GetTickCount() - g_note_t > 400) g_note_stage = 2;   // the game has taken the Escape
    if (g_note_stage != 2) return g_kb_poll(ecx, edx);
    void* kb = Data(0x87cc4) ? reinterpret_cast<void*>(static_cast<std::uintptr_t>(Data(0x87cc8))) : nullptr;
    if (!kb) { FinishNote(false); return 0; }
    using GetDataFn = long(__stdcall*)(void*, DWORD, void*, DWORD*, DWORD);
    using AcqFn = long(__stdcall*)(void*);
    unsigned char ev[32][16];   // DIDEVICEOBJECTDATA: offset (scan code) +0, data +4 (bit 0x80 = down)
    DWORD n = 32;
    const long hr = reinterpret_cast<GetDataFn>((*reinterpret_cast<void***>(kb))[10])(kb, 16, ev, &n, 0);   // +0x28
    if (hr == static_cast<long>(0x8007001e)) { reinterpret_cast<AcqFn>((*reinterpret_cast<void***>(kb))[7])(kb); return 0; }
    if (hr < 0) return 0;
    for (DWORD i = 0; i < n; ++i) {
        const DWORD sc = *reinterpret_cast<DWORD*>(ev[i]);
        const bool down = (*reinterpret_cast<DWORD*>(ev[i] + 4) & 0x80) != 0;
        if (sc == 0x2a || sc == 0x36) { g_note_shift = down; continue; }   // DIK_LSHIFT / DIK_RSHIFT (PLATFORM)
        if (!down) continue;
        if (sc == 0x1c || sc == 0x9c) { FinishNote(true); return 0; }    // DIK_RETURN / DIK_NUMPADENTER
        if (sc == 0x01) { FinishNote(false); return 0; }                 // DIK_ESCAPE
        if (sc == 0x0e) { if (!g_note_text.empty()) g_note_text.pop_back(); continue; }   // DIK_BACK
        BYTE ks[256] = {};
        if (g_note_shift) ks[VK_SHIFT] = 0x80;
        WORD out = 0;
        const UINT vk = MapVirtualKeyA(sc, MAPVK_VSC_TO_VK);
        if (vk && ToAscii(vk, sc, ks, &out, 0) == 1 && (out & 0xff) >= 32 && g_note_text.size() < 600)
            g_note_text += static_cast<char>(out & 0xff);
    }
    return 0;
}

// The pause menu does not redraw the whole screen, so the box would stay after Enter (user 2026-10-16): what is under
// the box is kept before the first draw and put back on the next flips after the note closes (both buffers of a flip
// chain).
HDC g_under_dc = nullptr;
HBITMAP g_under_bmp = nullptr;
HGDIOBJ g_under_old = nullptr;
RECT g_under{};
int g_restore_left = 0;

RECT NoteBoxRect(int w, int h)
{
    const int s = (std::max)(1, h / 540);
    return RECT{w / 8, h / 3, w - w / 8, h / 3 + 160 * s};
}

// ------------------------------------------------------------------------------------------------ RX-46 texture probe
// F12 reports name the textures under the aiming reticle (user 2026-10-20: players report texture bugs for regeneration;
// "only if it won't affect game performance"). Textured polygons reach Direct3D through the draw-mode slots
// [0x0056bc5c] SingleTextured, [0x0056bc60] SingleTexturedLit, [0x0056bc64] SingleTexturedLitGradient (render.md
// draw-mode table, CONFIRMED-BINARY; every caller reads the slot each time). For ONE frame after F12 Redux puts recorders
// in those three slots and restores them at the next flip - no cost in any other frame. A recorder tests the reticle
// point (cursor [0x00561c80]/[0x00561c84], Input_Mouse_UpdateCursorPosition 0x004704f0; the screen centre if out of
// range) against the polygon's screen vertices (ECX, 0xC bytes each: x, y, z) and keeps the material (SingleTextured
// stack arg 2, the Lit pair stack arg 5) - the material is the 28-byte D3D texture binding (render.md "Texture handles").
// Names: bindings are made only by D3DTextureBinding_CreateFromImage 0x004aa0f0 through hook [0x0056bc08] (__fastcall,
// texture name in ECX, image record in EDX, 3 stack args, returns the binding; zvideo.md, CONFIRMED-BINARY). Redux keeps
// that slot pointed at a recorder that notes binding -> name (texture loading only, never per frame). Failures return the
// shared default binding [0x006333a8], which is not recorded.
std::unordered_map<std::uint32_t, std::string> g_binding_names;   // binding -> texture name (game thread)
std::uint32_t g_create_binding = 0;                                 // the original [0x0056bc08]
volatile LONG g_probe_request = 0;
int g_probe_state = 0;                     // 0 idle, 1 recorders in place for this frame
std::uint32_t g_probe_slots[3] = {};
float g_probe_x = 0, g_probe_y = 0;
struct ProbeHit { std::uint32_t material; float z; };
std::vector<ProbeHit> g_probe_hits;
std::string g_probe_result;               // waiting to be written into the screenshot's .txt
DWORD g_probe_result_t = 0;

using CreateBindingFn = int(__fastcall*)(int, int, int, int, int);
int __fastcall RecordBinding(int name, int image, int a1, int a2, int a3)
{
    const int b = reinterpret_cast<CreateBindingFn>(static_cast<std::uintptr_t>(g_create_binding))(name, image, a1, a2, a3);
    if (b && std::uint32_t(b) != Data(0x1593a8)) {   // [0x006333a8] the shared default binding
        const char* n = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(name));
        g_binding_names[std::uint32_t(b)] = n && !IsBadStringPtrA(n, 260) && *n ? n : "(no name)";
    }
    return b;
}

void ProbePolygon(int verts, int count, int material)
{
    if (!verts || !material || count < 3 || count > 64) return;
    const float* v = reinterpret_cast<const float*>(static_cast<std::uintptr_t>(verts));
    bool inside = false;
    float z = 0;
    for (int i = 0, j = count - 1; i < count; j = i++) {
        const float xi = v[i * 3], yi = v[i * 3 + 1], xj = v[j * 3], yj = v[j * 3 + 1];
        if ((yi > g_probe_y) != (yj > g_probe_y) && g_probe_x < (xj - xi) * (g_probe_y - yi) / (yj - yi) + xi) inside = !inside;
        z += v[i * 3 + 2];
    }
    if (inside) g_probe_hits.push_back({std::uint32_t(material), z / count});
}

using Tex5Fn = int(__fastcall*)(int, int, int, int, int, int, int);
using Tex8Fn = int(__fastcall*)(int, int, int, int, int, int, int, int, int, int);
int __fastcall ProbeTextured(int ecx, int edx, int count, int material, int a3, int a4, int a5)
{
    ProbePolygon(ecx, count, material);
    return reinterpret_cast<Tex5Fn>(static_cast<std::uintptr_t>(g_probe_slots[0]))(ecx, edx, count, material, a3, a4, a5);
}
int __fastcall ProbeTexturedLit(int ecx, int edx, int a1, int a2, int a3, int count, int material, int a6, int a7, int a8)
{
    ProbePolygon(ecx, count, material);
    return reinterpret_cast<Tex8Fn>(static_cast<std::uintptr_t>(g_probe_slots[1]))(ecx, edx, a1, a2, a3, count, material, a6, a7, a8);
}
int __fastcall ProbeTexturedLitGradient(int ecx, int edx, int a1, int a2, int a3, int count, int material, int a6, int a7, int a8)
{
    ProbePolygon(ecx, count, material);
    return reinterpret_cast<Tex8Fn>(static_cast<std::uintptr_t>(g_probe_slots[2]))(ecx, edx, a1, a2, a3, count, material, a6, a7, a8);
}

// on the game thread at a flip: keep the binding recorder in place, start a probe (recorders in) or finish one
void ProbeStep()
{
    std::uint32_t& create = Data(0x91c08);   // [0x0056bc08], refilled whenever the dispatch table is installed
    const std::uint32_t rec = std::uint32_t(reinterpret_cast<std::uintptr_t>(&RecordBinding));
    if (create && create != rec) { g_create_binding = create; create = rec; }
    std::uint32_t* slots = &Data(0x91c5c);   // [0x0056bc5c..64]
    if (g_probe_state == 1) {
        for (int i = 0; i < 3; ++i) slots[i] = g_probe_slots[i];
        g_probe_state = 0;
        // INFERRED: smaller screen z is nearer (D3D z buffer convention); the alternative ordering only changes the list order
        std::sort(g_probe_hits.begin(), g_probe_hits.end(), [](const ProbeHit& a, const ProbeHit& b) { return a.z < b.z; });
        std::string names;
        int listed = 0;
        std::vector<std::uint32_t> seen;
        for (const ProbeHit& h : g_probe_hits) {
            if (listed >= 5 || std::find(seen.begin(), seen.end(), h.material) != seen.end()) continue;
            seen.push_back(h.material);
            const auto it = g_binding_names.find(h.material);
            names += (listed ? ", " : "") + (it != g_binding_names.end() ? it->second : std::string("(unnamed)"));
            ++listed;
        }
        char head[96];
        std::snprintf(head, sizeof head, "Textures under the reticle (%d, %d), nearest first: ", int(g_probe_x), int(g_probe_y));
        g_probe_result = head + (listed ? names : std::string("none found"));
        g_probe_result_t = GetTickCount();
        std::fprintf(stderr, "redux: RX-46 %s\n", g_probe_result.c_str());
        return;
    }
    if (InterlockedExchange(&g_probe_request, 0)) {
        const int cx = std::int32_t(Data(0x87c80)), cy = std::int32_t(Data(0x87c84));   // [0x00561c80] / [0x00561c84]
        const int w = g_width > 0 ? g_width : GetSystemMetrics(SM_CXSCREEN), h = g_height > 0 ? g_height : GetSystemMetrics(SM_CYSCREEN);
        const bool on = cx > 0 && cy > 0 && cx < w && cy < h;
        g_probe_x = float(on ? cx : w / 2);
        g_probe_y = float(on ? cy : h / 2);
        g_probe_hits.clear();
        for (int i = 0; i < 3; ++i) g_probe_slots[i] = slots[i];
        if (!g_probe_slots[0] || !g_probe_slots[1] || !g_probe_slots[2]) return;
        slots[0] = std::uint32_t(reinterpret_cast<std::uintptr_t>(&ProbeTextured));
        slots[1] = std::uint32_t(reinterpret_cast<std::uintptr_t>(&ProbeTexturedLit));
        slots[2] = std::uint32_t(reinterpret_cast<std::uintptr_t>(&ProbeTexturedLitGradient));
        g_probe_state = 1;
    }
}

// writes a finished probe into the screenshot's .txt once the watcher has written it
void ProbeWriteResult()
{
    if (g_probe_result.empty()) return;
    if (GetTickCount() - g_probe_result_t > 5000) { g_probe_result.clear(); return; }
    if (!g_note_txt[0] || GetFileAttributesW(g_note_txt) == INVALID_FILE_ATTRIBUTES) return;
    if (FILE* f = _wfopen(g_note_txt, L"a")) {
        std::fprintf(f, "\n%s\n", g_probe_result.c_str());
        std::fclose(f);
    }
    g_probe_result.clear();
}

// ------------------------------------------------------------------------------------------------ RX-47 mission map
// [Display] Map=1 (user 2026-10-20: "give the in-game map a redo, higher quality, with the weapon pickups and tank
// upgrades and mission objectives"; pickups shown from the start, collected ones disappear; the player's position).
// redux_map\m<N>.png is a top-down render of map N (made from the player's own copy: terrain, the original outlines,
// numbered objectives, secrets and a legend); m<N>.txt gives its map area size and world bounds - image x =
// (z - z0) * ppu, image y = (x1 - x) * ppu (world +x up, +z right, the game's own orientation, MapScreen_WorldToMap
// 0x00416c90). While the game's map is open (the Map command 0x1b of Mission_HandleCommand 0x00417ca0 toggles
// MapScreen_PushVisible; open = map object 0x004f0cc0 +0x68, set by MapScreen_BeginZoomToFit 0x00416a30) during play
// ([0x004f36b0] = 1, set by Game_HandleStateEvent 0x0042bb30 event 15; MEASURED 2026-10-20 RX-47 log: 1 while driving
// in mission 1, 0 before play starts; that it is 0 in the Escape menu is not yet checked), Redux draws at
// each flip, over the frame: the picture fitted to the screen, then every pickup still in the live list (head
// [0x004f331c], next +0x50, position +0xC, type entry +4 with its id at +8 - pickup.md, CONFIRMED-BINARY) that is a
// weapon (ids 0x11-0x21) or a tank upgrade (0x25-0x27), then the tank (V = [[0x004f36a8]+4], position +0x3EC, forward
// vector +0x580: the map focus heading of MapScreen_SetFocusEntity 0x00417220). No cost while the map is closed.
bool g_map_on = false;
int g_map_mission = -1;                     // whose picture is loaded (-1 none tried)
int g_map_w = 0, g_map_h = 0;               // the picture
int g_map_area_w = 0, g_map_area_h = 0;     // its map part (the legend is to the right)
double g_map_x0 = 0, g_map_z0 = 0, g_map_x1 = 0, g_map_z1 = 0, g_map_ppu = 1;
std::vector<unsigned char> g_map_bgra;
struct MapObjective { int n; double x, y; };   // number and marker position in the picture
std::vector<MapObjective> g_map_objectives;
// user 2026-10-20: "not full screen and slightly transparent as the game continues to run in the background"
constexpr double kMapFill = 0.75;           // INVENTED (user preference, not from Recoil): share of the screen
constexpr int kMapAlpha = 190;              // INVENTED (user preference): opacity out of 256 (about 75 %)
HDC g_map_dc = nullptr, g_map_tmp_dc = nullptr;   // the picture scaled for the screen; the frame area being composed
HBITMAP g_map_bmp = nullptr, g_map_tmp_bmp = nullptr;
HGDIOBJ g_map_old = nullptr, g_map_tmp_old = nullptr;
unsigned char* g_map_bits = nullptr;
unsigned char* g_map_tmp_bits = nullptr;
int g_map_sw = 0, g_map_sh = 0, g_map_ox = 0, g_map_oy = 0, g_map_dw = 0, g_map_dh = 0;
double g_map_scale = 1;

void MapFreeScaled()
{
    if (g_map_dc) {
        SelectObject(g_map_dc, g_map_old);
        DeleteObject(g_map_bmp);
        DeleteDC(g_map_dc);
        SelectObject(g_map_tmp_dc, g_map_tmp_old);
        DeleteObject(g_map_tmp_bmp);
        DeleteDC(g_map_tmp_dc);
        g_map_dc = g_map_tmp_dc = nullptr;
        g_map_bmp = g_map_tmp_bmp = nullptr;
    }
}

bool MapLoad(int mission)
{
    g_map_mission = mission;
    g_map_bgra.clear();
    MapFreeScaled();
    char path[MAX_PATH];
    std::snprintf(path, sizeof path, "redux_map\\m%d.txt", mission);
    FILE* f = std::fopen(path, "r");
    if (!f) return false;
    const int got = std::fscanf(f, "%d %d %lf %lf %lf %lf %lf", &g_map_area_w, &g_map_area_h, &g_map_x0, &g_map_z0, &g_map_x1,
                                &g_map_z1, &g_map_ppu);
    g_map_objectives.clear();
    MapObjective o{};
    while (std::fscanf(f, " obj %d %lf %lf", &o.n, &o.x, &o.y) == 3)
        if (o.n >= 1 && o.x >= 0) g_map_objectives.push_back(o);
    std::fclose(f);
    std::snprintf(path, sizeof path, "redux_map\\m%d.png", mission);
    std::vector<unsigned char> rgba;
    if (got != 7 || g_map_ppu <= 0 || !LoadPng(path, g_map_w, g_map_h, rgba)) return false;
    g_map_bgra.resize(rgba.size());
    for (std::size_t i = 0; i < rgba.size(); i += 4) {
        g_map_bgra[i] = rgba[i + 2];
        g_map_bgra[i + 1] = rgba[i + 1];
        g_map_bgra[i + 2] = rgba[i];
        g_map_bgra[i + 3] = 255;
    }
    std::fprintf(stderr, "redux: RX-47 map %d loaded (%dx%d)\n", mission, g_map_w, g_map_h);
    return true;
}

HDC MapDib(int w, int h, HBITMAP& bmp, HGDIOBJ& old, unsigned char*& bits)
{
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    void* p = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    bmp = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &p, nullptr, 0);
    old = SelectObject(dc, bmp);
    bits = static_cast<unsigned char*>(p);
    return dc;
}

void MapScaleTo(int w, int h)
{
    MapFreeScaled();
    g_map_sw = w;
    g_map_sh = h;
    g_map_scale = (std::min)(w * kMapFill / g_map_w, h * kMapFill / g_map_h);
    g_map_dw = (std::max)(1, int(g_map_w * g_map_scale));
    g_map_dh = (std::max)(1, int(g_map_h * g_map_scale));
    g_map_ox = (w - g_map_dw) / 2;
    g_map_oy = (h - g_map_dh) / 2;
    g_map_dc = MapDib(g_map_dw, g_map_dh, g_map_bmp, g_map_old, g_map_bits);
    g_map_tmp_dc = MapDib(g_map_dw, g_map_dh, g_map_tmp_bmp, g_map_tmp_old, g_map_tmp_bits);
    BITMAPINFO src{};
    src.bmiHeader.biSize = sizeof src.bmiHeader;
    src.bmiHeader.biWidth = g_map_w;
    src.bmiHeader.biHeight = -g_map_h;
    src.bmiHeader.biPlanes = 1;
    src.bmiHeader.biBitCount = 32;
    SetStretchBltMode(g_map_dc, HALFTONE);
    SetBrushOrgEx(g_map_dc, 0, 0, nullptr);
    StretchDIBits(g_map_dc, 0, 0, g_map_dw, g_map_dh, 0, 0, g_map_w, g_map_h, g_map_bgra.data(), &src, DIB_RGB_COLORS, SRCCOPY);
    GdiFlush();
}

POINT MapPoint(double x, double z)   // in the composed area (0, 0 = the picture's top-left)
{
    return {int((z - g_map_z0) * g_map_ppu * g_map_scale), int((g_map_x1 - x) * g_map_ppu * g_map_scale)};
}

void MapDraw(HDC screen, int w, int h)
{
    if (w != g_map_sw || h != g_map_sh || !g_map_dc) MapScaleTo(w, h);
    // the frame under the map, blended with the picture, then the live markers, then back in one copy
    BitBlt(g_map_tmp_dc, 0, 0, g_map_dw, g_map_dh, screen, g_map_ox, g_map_oy, SRCCOPY);
    GdiFlush();
    const std::size_t bytes = std::size_t(g_map_dw) * g_map_dh * 4;
    for (std::size_t i = 0; i < bytes; ++i)
        g_map_tmp_bits[i] = static_cast<unsigned char>((g_map_bits[i] * kMapAlpha + g_map_tmp_bits[i] * (256 - kMapAlpha)) >> 8);
    HDC dc = g_map_tmp_dc;
    const int s = (std::max)(4, int(9 * g_map_scale * 1.6));   // icon size, as the legend's at this scale
    HPEN black = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)), white = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HBRUSH weapon = CreateSolidBrush(RGB(255, 150, 30)), upgrade = CreateSolidBrush(RGB(60, 200, 255)),
           you = CreateSolidBrush(RGB(255, 255, 255));
    HGDIOBJ op = SelectObject(dc, black), ob = SelectObject(dc, weapon);
    HRGN clip = CreateRectRgn(0, 0, int(g_map_area_w * g_map_scale), int(g_map_area_h * g_map_scale));
    SelectClipRgn(dc, clip);
    // objectives still to do: mission object 0x004f0cc0, count +0x12c, objective i's done flag at +0x53c + i * 0x31c
    // (objective_triggers.md, VERIFIED-ORACLE; slot i is OBJECTIVE<i+1>, Mission_LoadObjectiveTriggers 0x00418230)
    {
        HBRUSH red = CreateSolidBrush(RGB(255, 60, 60));
        HPEN ring = CreatePen(PS_SOLID, (std::max)(2, s / 4), RGB(255, 255, 255));
        HFONT num = CreateFontA(-int(s * 1.6), 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, "Arial");
        HGDIOBJ of = SelectObject(dc, num);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));
        const int count = int(Data(0x16dec));
        for (const MapObjective& o : g_map_objectives) {
            const int i = o.n - 1;
            if (i < count && Data(0x171fc + i * 0x31c)) continue;   // done: the marker goes
            const int x = int(o.x * g_map_scale), y = int(o.y * g_map_scale), r = int(s * 1.3);
            SelectObject(dc, red);
            SelectObject(dc, ring);
            Ellipse(dc, x - r, y - r, x + r, y + r);
            char t[8];
            std::snprintf(t, sizeof t, "%d", o.n);
            RECT tr{x - r, y - r, x + r, y + r};
            DrawTextA(dc, t, -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
        SelectObject(dc, of);
        SelectObject(dc, black);
        DeleteObject(num);
        DeleteObject(ring);
        DeleteObject(red);
    }
    std::uint32_t node = Data(0x1931c);   // [0x004f331c] live pickups
    for (int guard = 0; node && guard < 4096; ++guard) {
        const auto* n = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(node));
        if (IsBadReadPtr(n, 0x54)) break;
        const auto* entry = reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(*reinterpret_cast<const std::uint32_t*>(n + 4)));
        const std::uint32_t id = entry && !IsBadReadPtr(entry, 0xc) ? *reinterpret_cast<const std::uint32_t*>(entry + 8) : 0;
        const float* p = reinterpret_cast<const float*>(n + 0xc);
        const POINT c = MapPoint(p[0], p[2]);
        if (id >= 0x11 && id <= 0x21) {   // weapons: orange diamond
            SelectObject(dc, weapon);
            SelectObject(dc, black);
            POINT d[4] = {{c.x, c.y - s}, {c.x + s, c.y}, {c.x, c.y + s}, {c.x - s, c.y}};
            Polygon(dc, d, 4);
        } else if (id >= 0x25 && id <= 0x27) {   // tank upgrades: blue hexagon
            SelectObject(dc, upgrade);
            SelectObject(dc, white);
            POINT hx[6];
            for (int i = 0; i < 6; ++i)
                hx[i] = {c.x + int(s * 1.15 * std::cos(3.14159265 / 3 * i)), c.y + int(s * 1.15 * std::sin(3.14159265 / 3 * i))};
            Polygon(dc, hx, 6);
        }
        node = *reinterpret_cast<const std::uint32_t*>(n + 0x50);
    }
    const std::uint32_t holder = Data(0x196a8);   // [0x004f36a8]
    const auto* hv = holder && !IsBadReadPtr(reinterpret_cast<void*>(static_cast<std::uintptr_t>(holder)), 8)
                         ? reinterpret_cast<const unsigned char*>(static_cast<std::uintptr_t>(*reinterpret_cast<const std::uint32_t*>(
                               static_cast<std::uintptr_t>(holder) + 4)))
                         : nullptr;
    if (hv && !IsBadReadPtr(hv, 0x590)) {
        const float* p = reinterpret_cast<const float*>(hv + 0x3ec);
        const float* fwd = reinterpret_cast<const float*>(hv + 0x580);
        double fx = fwd[0], fz = fwd[2];
        const double len = std::sqrt(fx * fx + fz * fz);
        if (len > 1e-6) { fx /= len; fz /= len; } else { fx = 1; fz = 0; }
        const POINT c = MapPoint(p[0], p[2]);
        const double ux = fz, uy = -fx;           // forward on screen (+x up, +z right)
        const double vx = -uy, vy = ux;           // its right
        const double L = s * 2.2, W = s * 1.3;
        POINT a[4] = {{c.x + int(ux * L), c.y + int(uy * L)}, {c.x - int(ux * L * 0.7 - vx * W), c.y - int(uy * L * 0.7 - vy * W)},
                      {c.x - int(ux * L * 0.3), c.y - int(uy * L * 0.3)}, {c.x - int(ux * L * 0.7 + vx * W), c.y - int(uy * L * 0.7 + vy * W)}};
        HPEN thick = CreatePen(PS_SOLID, (std::max)(2, s / 3), RGB(0, 0, 0));
        SelectObject(dc, thick);
        SelectObject(dc, you);
        Polygon(dc, a, 4);
        SelectObject(dc, black);
        DeleteObject(thick);
    }
    SelectClipRgn(dc, nullptr);
    DeleteObject(clip);
    SelectObject(dc, op);
    SelectObject(dc, ob);
    DeleteObject(black);
    DeleteObject(white);
    DeleteObject(weapon);
    DeleteObject(upgrade);
    DeleteObject(you);
    GdiFlush();
    BitBlt(screen, g_map_ox, g_map_oy, g_map_dw, g_map_dh, dc, 0, 0, SRCCOPY);
}

// at a flip: the map over the frame while it is open
void MapStep(bool copy)
{
    if (!g_map_on || g_note_stage != 0) return;
    static DWORD last_log = 0;
    static std::uint32_t last_state = 0xffffffff;
    const std::uint32_t state = Data(0x16d28) | Data(0x16d30) << 8 | Data(0x196b0) << 16 | Data(0x16c98) << 24;
    if (state != last_state || GetTickCount() - last_log > 5000) {   // diagnostic while RX-47 is unverified
        std::fprintf(stderr, "redux: RX-47 state open+68=%u anim+70=%u menu=%u pushcount=%u mission=%d\n", Data(0x16d28), Data(0x16d30),
                     Data(0x196b0), Data(0x16c98), g_mission);
        std::fflush(stderr);
        last_state = state;
        last_log = GetTickCount();
    }
    if (!Data(0x16d28) || !Data(0x196b0)) return;   // map +0x68 open; [0x004f36b0] playing
    // the Escape menu leaves [0x004f36b0] at 1 (RX-47 log): draw only while the game clock [0x0056b430] (Pickup respawn
    // time base, pickup.md) moves - it stands still in the pause menu
    static std::uint32_t last_clock = 0;
    static int still = 0;
    const std::uint32_t clock = Data(0x91430);
    still = clock == last_clock ? still + 1 : 0;
    last_clock = clock;
    if (still >= 3) {
        // the pause menu came up with the map open: close it with the game's own Map command step
        // (MapScreen_PushVisible 0x00416b30 with 0, as Mission_HandleCommand 0x00417ca0 does), because after the menu the
        // Map key no longer closed it (user 2026-10-20; RX-47 log: +0x68 stayed 1 with push count 1)
        MapScreen_PushVisible(static_cast<int>(reinterpret_cast<std::uintptr_t>(&Data(0x16cc0))), 0, 0);
        std::fprintf(stderr, "redux: RX-47 map closed for the pause menu\n");
        return;
    }
    if (g_mission != g_map_mission) MapLoad(g_mission);
    if (g_map_bgra.empty()) return;
    // zVideo_FlipFrame (render.md) copies [0x0063223c] into the back buffer [0x0063221c] only when its copy flag (stack
    // arg 2) is set - the 2D screens; in play the 3D device draws straight into the back buffer (Render_CreateD3DDevice
    // 0x004a9c20 creates it on [0x0063221c]), so the map goes where the frame is
    void* rt = reinterpret_cast<void*>(static_cast<std::uintptr_t>(Data(copy ? 0x15823c : 0x15821c)));
    if (!rt) return;
    using GetDcFn = long(__stdcall*)(void*, HDC*);
    using RelDcFn = long(__stdcall*)(void*, HDC);
    using GetDescFn = long(__stdcall*)(void*, void*);
    unsigned char desc[0x6c] = {};
    *reinterpret_cast<std::uint32_t*>(desc) = 0x6c;
    if (reinterpret_cast<GetDescFn>((*reinterpret_cast<void***>(rt))[22])(rt, desc) != 0) return;   // GetSurfaceDesc +0x58
    const int h = *reinterpret_cast<std::int32_t*>(desc + 8), w = *reinterpret_cast<std::int32_t*>(desc + 0xc);
    HDC dc = nullptr;
    if (w <= 0 || h <= 0 || reinterpret_cast<GetDcFn>((*reinterpret_cast<void***>(rt))[17])(rt, &dc) != 0) return;   // GetDC +0x44
    MapDraw(dc, w, h);
    static int drawn = 0;
    if (drawn++ == 0) std::fprintf(stderr, "redux: RX-47 map drawn on the %s surface %dx%d\n", copy ? "2D" : "back", w, h);
    reinterpret_cast<RelDcFn>((*reinterpret_cast<void***>(rt))[26])(rt, dc);   // ReleaseDC +0x68
}

int __fastcall FlipWithNote(int ecx, int edx, int a, int b)
{
    ProbeStep();
    ProbeWriteResult();
    MapStep(b != 0);
    if (g_note_stage == 2 || g_restore_left > 0) {
        void* rt = reinterpret_cast<void*>(static_cast<std::uintptr_t>(Data(0x15823c)));   // [0x0063223c] render target
        using GetDcFn = long(__stdcall*)(void*, HDC*);
        using RelDcFn = long(__stdcall*)(void*, HDC);
        using GetDescFn = long(__stdcall*)(void*, void*);
        HDC dc = nullptr;
        long hr = -1;
        if (rt) {
            unsigned char desc[0x6c] = {};
            *reinterpret_cast<std::uint32_t*>(desc) = 0x6c;
            reinterpret_cast<GetDescFn>((*reinterpret_cast<void***>(rt))[22])(rt, desc);   // GetSurfaceDesc +0x58
            const int h = *reinterpret_cast<std::int32_t*>(desc + 8), w = *reinterpret_cast<std::int32_t*>(desc + 0xc);
            hr = reinterpret_cast<GetDcFn>((*reinterpret_cast<void***>(rt))[17])(rt, &dc);   // GetDC +0x44
            if (hr == 0) {
                if (g_note_stage == 2) {
                    if (!g_under_bmp) {   // first frame of the box: keep what is under it
                        g_under = NoteBoxRect(w, h);
                        const int bw = g_under.right - g_under.left, bh = g_under.bottom - g_under.top;
                        g_under_dc = CreateCompatibleDC(dc);
                        g_under_bmp = CreateCompatibleBitmap(dc, bw, bh);
                        g_under_old = SelectObject(g_under_dc, g_under_bmp);
                        BitBlt(g_under_dc, 0, 0, bw, bh, dc, g_under.left, g_under.top, SRCCOPY);
                    }
                    DrawNoteBox(dc, w, h);
                    g_restore_left = 3;
                } else if (g_under_bmp) {   // the box has closed: put the screen back
                    BitBlt(dc, g_under.left, g_under.top, g_under.right - g_under.left, g_under.bottom - g_under.top, g_under_dc, 0, 0,
                           SRCCOPY);
                    if (--g_restore_left == 0) {
                        SelectObject(g_under_dc, g_under_old);
                        DeleteObject(g_under_bmp);
                        DeleteDC(g_under_dc);
                        g_under_bmp = nullptr;
                        g_under_dc = nullptr;
                    }
                } else {
                    g_restore_left = 0;
                }
                reinterpret_cast<RelDcFn>((*reinterpret_cast<void***>(rt))[26])(rt, dc);   // ReleaseDC +0x68
            }
        }
        if (hr != 0 && g_note_draw_fail++ == 0)
            std::fprintf(stderr, "redux: RX-38 cannot draw the note box (surface %p, GetDC 0x%08lx)\n", rt, static_cast<unsigned long>(hr));
    }
    return g_flip(ecx, edx, a, b);
}

// called by the screenshot watcher after the picture is saved
void StartNote()
{
    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    if (!g_flip || g_note_stage != 0 || !g_note_txt[0] || pid != GetCurrentProcessId()) return;
    NoteHeader();
    g_note_text.clear();
    g_note_shift = false;
    INPUT in[2] = {};
    in[0].type = in[1].type = INPUT_KEYBOARD;
    in[0].ki.wScan = in[1].ki.wScan = 0x01;   // Escape: the game's pause menu
    in[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    in[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    SendInput(1, &in[0], sizeof(INPUT));
    Sleep(60);
    SendInput(1, &in[1], sizeof(INPUT));
    g_note_t = GetTickCount();
    g_note_stage = 1;
}

// the zVideo_FlipFrame 0x004a9b70 trampoline shared by RX-38 / RX-46 / RX-47 (its first 7 bytes copied)
bool InstallFlipHook()
{
    if (g_flip) return true;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(&zVideo_FlipFrame);
    if (p[0] != 0xA1 || p[5] != 0x53 || p[6] != 0x55) return false;   // MOV EAX,[abs] / PUSH EBX / PUSH EBP
    if (!(g_flip = reinterpret_cast<FlipFn>(Trampoline(const_cast<unsigned char*>(p), p, 7))) ||
        !Hook(const_cast<unsigned char*>(p), reinterpret_cast<void*>(&FlipWithNote), "zVideo_FlipFrame")) {
        g_flip = nullptr;
        return false;
    }
    return true;
}

bool InitScreenshotNote()
{
    if (GetPrivateProfileIntA("Tools", "ScreenshotNote", 0, kIni) != 1 || !g_shot_vk) return false;
    const unsigned char* k = reinterpret_cast<const unsigned char*>(&Input_Keyboard_PollAndTranslate);
    if (k[0] != 0x83 || k[1] != 0xEC || k[2] != 0x08 || k[3] != 0xA1) {   // SUB ESP,8 / MOV EAX,[abs]
        std::fprintf(stderr, "redux: RX-38 screenshot note not installed (unexpected code)\n");
        return false;
    }
    if (!InstallFlipHook() || !InstallMissionHook() ||
        !(g_kb_poll = reinterpret_cast<KbFn>(Trampoline(const_cast<unsigned char*>(k), k, 8))) ||
        !Hook(const_cast<unsigned char*>(k), reinterpret_cast<void*>(&KbPollWithNote), "Input_Keyboard_PollAndTranslate")) {
        std::fprintf(stderr, "redux: RX-38 screenshot note not installed\n");
        return false;
    }
    std::fprintf(stderr, "redux: RX-38 screenshot key pauses (Escape) for a note\n");
    return true;
}

bool InitMap()
{
    if (GetPrivateProfileIntA("Display", "Map", 0, kIni) != 1) return false;
    const DWORD a = GetFileAttributesA("redux_map");
    if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY)) {
        std::fprintf(stderr, "redux: RX-47 map not installed (no redux_map folder)\n");
        return false;
    }
    if (!InstallFlipHook() || !InstallMissionHook()) {
        std::fprintf(stderr, "redux: RX-47 map not installed (unexpected code)\n");
        return false;
    }
    g_map_on = true;
    std::fprintf(stderr, "redux: RX-47 mission map (redux_map)\n");
    return true;
}

// ------------------------------------------------------------------------------------------------ RX-39 rapid capture
// [Tools] RapidKey=F11: a burst of screenshots for problems too quick to catch - 10 a second while the key is held,
// at least 3 seconds, at most 20, into screenshots\burst_<date>_<time>\NNN_<ms>.png. The capture thread only copies the
// screen; a second thread writes the PNGs (at most 12 frames waiting, extra frames are dropped and counted). No hook.
int g_rapid_vk = 0;
struct BurstFrame { HBITMAP bmp; int w, h; std::wstring path; };
std::vector<BurstFrame> g_burst_q;
CRITICAL_SECTION g_burst_cs;
HANDLE g_burst_ev = nullptr;

DWORD WINAPI BurstWriter(void*)
{
    for (;;) {
        WaitForSingleObject(g_burst_ev, INFINITE);
        for (;;) {
            EnterCriticalSection(&g_burst_cs);
            if (g_burst_q.empty()) { LeaveCriticalSection(&g_burst_cs); break; }
            BurstFrame fr = g_burst_q.front();
            g_burst_q.erase(g_burst_q.begin());
            LeaveCriticalSection(&g_burst_cs);
            SavePng(fr.path.c_str(), fr.bmp, fr.w, fr.h);
            DeleteObject(fr.bmp);
        }
    }
}

void RunBurst()
{
    RECT r{0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    if (fg && pid == GetCurrentProcessId()) GetWindowRect(fg, &r);
    const int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t dir[MAX_PATH], base[MAX_PATH];
    wcscpy_s(base, ShotDir());
    swprintf_s(dir, L"%s\\burst_%04d-%02d-%02d_%02d-%02d-%02d", base, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    CreateDirectoryW(dir, nullptr);
    Beep(1800, 40);
    HDC screen = GetDC(nullptr);
    const DWORD start = GetTickCount();
    int n = 0, dropped = 0;
    for (;;) {
        const DWORD el = GetTickCount() - start;
        const bool held = (GetAsyncKeyState(g_rapid_vk) & 0x8000) != 0;
        if (el >= 20000 || (el >= 3000 && !held)) break;
        EnterCriticalSection(&g_burst_cs);
        const bool full = g_burst_q.size() >= 12;
        LeaveCriticalSection(&g_burst_cs);
        if (full) ++dropped;
        else {
            HDC mem = CreateCompatibleDC(screen);
            HBITMAP bmp = CreateCompatibleBitmap(screen, w, h);
            HGDIOBJ old = SelectObject(mem, bmp);
            BitBlt(mem, 0, 0, w, h, screen, r.left, r.top, SRCCOPY | CAPTUREBLT);
            SelectObject(mem, old);
            DeleteDC(mem);
            wchar_t p[MAX_PATH];
            swprintf_s(p, L"%s\\%03d_%05lums.png", dir, n++, el);
            EnterCriticalSection(&g_burst_cs);
            g_burst_q.push_back({bmp, w, h, p});
            LeaveCriticalSection(&g_burst_cs);
            SetEvent(g_burst_ev);
        }
        const DWORD next = (static_cast<DWORD>(n + dropped)) * 100;
        const DWORD now = GetTickCount() - start;
        if (next > now) Sleep(next - now);
    }
    ReleaseDC(nullptr, screen);
    Beep(1000, 40);
    std::fprintf(stderr, "redux: RX-39 burst %ls: %d frames (%d dropped)\n", dir, n, dropped);
}

DWORD WINAPI ScreenshotWatcher(void*)
{
    bool down = false, rdown = false;
    for (;;) {
        const bool now = g_shot_vk && (GetAsyncKeyState(g_shot_vk) & 0x8000) != 0;
        if (now && !down) {
            g_note_txt[0] = 0;
            if (g_flip) InterlockedExchange(&g_probe_request, 1);   // RX-46: the textures under the reticle, next frame
            TakeScreenshot();
            StartNote();
        }
        down = now;
        const bool rnow = g_rapid_vk && (GetAsyncKeyState(g_rapid_vk) & 0x8000) != 0;
        if (rnow && !rdown) RunBurst();
        rdown = rnow;
        Sleep(30);
    }
}

int ParseFKey(const char* k)
{
    if ((k[0] == 'F' || k[0] == 'f') && std::atoi(k + 1) >= 1 && std::atoi(k + 1) <= 24) return VK_F1 + std::atoi(k + 1) - 1;
    return 0;
}

bool InitScreenshotKey()
{
    char k[16] = {}, rk[16] = {};
    GetPrivateProfileStringA("Tools", "ScreenshotKey", "", k, sizeof k, kIni);
    GetPrivateProfileStringA("Tools", "RapidKey", "", rk, sizeof rk, kIni);
    g_shot_vk = ParseFKey(k);
    g_rapid_vk = ParseFKey(rk);
    if (g_rapid_vk == g_shot_vk) g_rapid_vk = 0;
    if (!g_shot_vk && !g_rapid_vk) return false;
    if (g_rapid_vk) {
        InitializeCriticalSection(&g_burst_cs);
        g_burst_ev = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        CloseHandle(CreateThread(nullptr, 0, BurstWriter, nullptr, 0, nullptr));
        std::fprintf(stderr, "redux: RX-39 rapid capture key %s -> %ls\\burst_*\\\n", rk, ShotDir());
    }
    CloseHandle(CreateThread(nullptr, 0, ScreenshotWatcher, nullptr, 0, nullptr));
    if (g_shot_vk) std::fprintf(stderr, "redux: screenshot key %s -> %ls\\\n", k, ShotDir());
    return true;
}

// Stands in for Image_Load 0x0046d900 (the same three steps), then widens the two HUD backgrounds
int __fastcall ImageLoad(int name_arg, int)
{
    int img = Image_LoadFromArchiveListA(name_arg, 0);
    if (!img) img = Image_LoadFromArchive(name_arg, 0);
    if (!img) return 0;
    auto* rec = reinterpret_cast<unsigned char*>(static_cast<std::uintptr_t>(img));
    if (g_pack_on && name_arg)
        ReplaceFromPack(rec, reinterpret_cast<const char*>(static_cast<std::uintptr_t>(name_arg)));
    if ((rec[9] & 0x2) && *reinterpret_cast<std::uint32_t*>(rec + 0x14))   // CONFIRMED-BINARY: alpha flag + plane -> mask (0x0046d91c)
        Image_ApplyAlphaMask(img, 0);
    const char* name = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(name_arg));
    for (const WidePlan& p : kWide)
        if (g_wide_on && name && _stricmp(name, p.name) == 0) Widen(rec, p);
    return img;
}

// RX-04b: the three indicator squares at the weapon panel's right end. HudTriple_LayoutFromConfig 0x0040f2e0 places them at
// half the HUD width [0x004e61f0] plus a config offset, which is the panel's right end only at 640 wide. While it runs,
// Redux adds the extra width to [0x004e61f0], so the strip keeps its distance from the right edge as in the original.
using HudTripleFn = int(__fastcall*)(int, int, int);
HudTripleFn g_hud_triple = nullptr;

int __fastcall HudTripleLayout(int ecx, int edx, int arg)
{
    std::uint32_t& half_base = Data(0xc1f0);   // CONFIRMED-BINARY: [0x004e61f0], read and halved at 0x0040f2e3
    const std::uint32_t saved = half_base;
    half_base = saved + static_cast<std::uint32_t>(g_width - 640);
    const int r = g_hud_triple(ecx, edx, arg);
    half_base = saved;
    return r;
}

bool HookImageLoad()   // one Image_Load stand-in serves RX-04 and RX-10
{
    if (!g_image_hooked)
        g_image_hooked = Hook(reinterpret_cast<void*>(&Image_Load), reinterpret_cast<void*>(&ImageLoad), "Image_Load");
    return g_image_hooked;
}

// Records the mission number, then runs the original (its first 7 bytes SUB ESP,0x100 / PUSH ESI copied to a trampoline)
using MissionPathsFn = int(__fastcall*)(int, int);
MissionPathsFn g_mission_paths = nullptr;

int __fastcall MissionPaths(int mission, int edx)
{
    g_mission = mission;
    return g_mission_paths(mission, edx);
}

// every feature that needs the mission number installs this (once); it was only installed with a texture pack, so
// without one the RX-47 map and the RX-38 note never knew the mission (found 2026-10-24)
bool InstallMissionHook()
{
    if (g_mission_paths) return true;
    const unsigned char head[] = {0x81, 0xEC, 0x00, 0x01, 0x00, 0x00, 0x56};   // SUB ESP,0x100 / PUSH ESI
    void* target = reinterpret_cast<void*>(&Mission_SetDataSearchPaths);
    if ((g_mission_paths = reinterpret_cast<MissionPathsFn>(Trampoline(target, head, sizeof head))) != nullptr &&
        Hook(target, reinterpret_cast<void*>(&MissionPaths), "Mission_SetDataSearchPaths"))
        return true;
    g_mission_paths = nullptr;
    std::fprintf(stderr, "redux: mission number unknown (unexpected code at Mission_SetDataSearchPaths)\n");
    return false;
}

bool InitTexturePack()
{
    char dir[MAX_PATH] = {};
    GetPrivateProfileStringA("Textures", "Pack", "", dir, sizeof dir, kIni);
    if (!*dir) return false;
    GetFullPathNameA(dir, MAX_PATH, g_pack_dir, nullptr);
    const DWORD a = GetFileAttributesA(g_pack_dir);
    if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY)) {
        std::fprintf(stderr, "redux: RX-10 texture pack folder %s not found\n", g_pack_dir);
        return false;
    }
    g_pack_on = HookImageLoad();
    if (g_pack_on) std::fprintf(stderr, "redux: RX-10 texture pack %s\n", g_pack_dir);
    if (g_pack_on && InstallMissionHook())
        std::fprintf(stderr, "redux: RX-10 per-mission pack folders (m<N>)\n");
    else if (g_pack_on)
        std::fprintf(stderr, "redux: RX-10 per-mission folders off (unexpected code at Mission_SetDataSearchPaths)\n");
    return g_pack_on;
}

bool InitWideHud()
{
    // on by default with a custom resolution (the HUD looks broken without it); WideHUD=0 turns it off
    if (g_width <= 640 || GetPrivateProfileIntA("Display", "WideHUD", 1, kIni) == 0)
        return false;
    const bool ok = HookImageLoad();
    g_wide_on = ok;
    if (ok) std::fprintf(stderr, "redux: RX-04 wide HUD backgrounds\n");
    // the indicator strip: its first instructions are SUB ESP,0x18 / MOV EAX,[0x004e61f0] (8 bytes, copied to the trampoline)
    const unsigned char head[] = {0x83, 0xEC, 0x18, 0xA1};
    void* target = reinterpret_cast<void*>(&HudTriple_LayoutFromConfig);
    if (std::memcmp(target, head, sizeof head) == 0 &&
        (g_hud_triple = reinterpret_cast<HudTripleFn>(Trampoline(target, static_cast<unsigned char*>(target), 8))) != nullptr &&
        Hook(target, reinterpret_cast<void*>(&HudTripleLayout), "HudTriple_LayoutFromConfig"))
        std::fprintf(stderr, "redux: RX-04b indicator squares kept at the panel's right end\n");
    else
        std::fprintf(stderr, "redux: RX-04b not installed (unexpected code at HudTriple_LayoutFromConfig)\n");
    return ok;
}

bool InitResolution()
{
    char v[64] = {};
    GetPrivateProfileStringA("Display", "Resolution", "", v, sizeof v, kIni);
    int w = 0, h = 0;
    if (!*v || std::sscanf(v, "%dx%d", &w, &h) != 2)
        return false;
    if (w < 320 || h < 200 || w > 7680 || h > 4320) {   // PLATFORM: sanity bounds (8K)
        std::fprintf(stderr, "redux: [Display] Resolution=%s ignored (expected e.g. 1920x1080)\n", v);
        return false;
    }
    g_width = w;
    g_height = h;
    const bool ok = Hook(reinterpret_cast<void*>(&Settings_ApplyVideoModePreset), reinterpret_cast<void*>(&ApplyVideoModePreset),
                         "Settings_ApplyVideoModePreset") &&
                    Hook(reinterpret_cast<void*>(&Render_ApplyResolutionPreset), reinterpret_cast<void*>(&ApplyResolutionPreset),
                         "Render_ApplyResolutionPreset");
    if (ok) std::fprintf(stderr, "redux: RX-03 resolution %dx%d (every Options video mode uses it)\n", w, h);
    return ok;
}

// ------------------------------------------------------------------------------------------------ RX-37 mipmaps
// The game creates every Direct3D texture as ONE level (DDraw_CreateTextureSurface 0x004a83d0, CONFIRMED-BINARY: a
// DDSURFACEDESC of 0x6C bytes, flags 0x10007, caps EDX | 0x40, CreateSurface on [0x006333d8], QueryInterface for the
// IID at 0x004cc000+0x6fd8 into record +0x30, then Texture_UploadPixels 0x004a8500 copies record +0x10 (w +4, h +6,
// 16-bit) into it and frees the source). With no smaller copies, a large texture drawn small samples a few texels per
// pixel that change as the view moves - the shimmer that sharp texture packs make visible (user 2026-10-16).
// [Graphics] Mipmaps=1: Redux creates the same surface with a full mip chain (DDSD_MIPMAPCOUNT 0x20000,
// DDSCAPS_MIPMAP 0x400000 | DDSCAPS_COMPLEX 0x8, PLATFORM) and, after each upload, fills every smaller level with a
// 2x2 box average of the one above. Colour key 0 (the game zeroes transparent pixels, Image_ApplyAlphaMask) is kept:
// a texel stays transparent only when most of its four sources are, and transparent sources never darken the rest.
using TexFn = int(__fastcall*)(int, int);
TexFn g_upload = nullptr;

void FillMips(void* top, const std::uint16_t* src, int w, int h)
{
    const bool r555 = Data(0x15815c) == 5;   // CONFIRMED-BINARY: green bits [0x0063215c] = 5 -> 555
    const int rs = r555 ? 10 : 11, gb = r555 ? 5 : 6, gm = (1 << gb) - 1;
    std::vector<std::uint16_t> cur(src, src + static_cast<std::size_t>(w) * h), next;
    void* surf = top;
    using GetAttachedFn = long(__stdcall*)(void*, std::uint32_t*, void**);
    using LockFn = long(__stdcall*)(void*, void*, void*, std::uint32_t, void*);
    using UnlockFn = long(__stdcall*)(void*, void*);
    using ReleaseFn = unsigned long(__stdcall*)(void*);
    bool first = true;
    while (w > 1 || h > 1) {
        std::uint32_t caps = 0x400000;   // DDSCAPS_MIPMAP
        void* child = nullptr;
        auto vt = *reinterpret_cast<void***>(surf);
        if (reinterpret_cast<GetAttachedFn>(vt[12])(surf, &caps, &child) != 0 || !child) break;   // +0x30
        if (!first) reinterpret_cast<ReleaseFn>(vt[2])(surf);
        first = false;
        surf = child;
        const int nw = w > 1 ? w / 2 : 1, nh = h > 1 ? h / 2 : 1;
        next.assign(static_cast<std::size_t>(nw) * nh, 0);
        for (int y = 0; y < nh; ++y)
            for (int x = 0; x < nw; ++x) {
                unsigned r = 0, g = 0, b = 0, n = 0, keyed = 0;
                for (int dy = 0; dy < 2; ++dy)
                    for (int dx = 0; dx < 2; ++dx) {
                        const int sx = (std::min)(x * 2 + dx, w - 1), sy = (std::min)(y * 2 + dy, h - 1);
                        const std::uint16_t c = cur[static_cast<std::size_t>(sy) * w + sx];
                        if (c == 0) { ++keyed; continue; }
                        r += c >> rs; g += (c >> 5) & gm; b += c & 0x1f; ++n;
                    }
                if (n == 0 || keyed > 2) continue;   // mostly transparent: stays key colour 0
                std::uint16_t v = static_cast<std::uint16_t>(((r / n) << rs) | (((g + n / 2) / n) << 5) | ((b + n / 2) / n));
                next[static_cast<std::size_t>(y) * nw + x] = v ? v : 1;   // never turn an opaque texel into the key
            }
        unsigned char desc[0x6c] = {};
        *reinterpret_cast<std::uint32_t*>(desc) = 0x6c;
        auto cvt = *reinterpret_cast<void***>(surf);
        if (reinterpret_cast<LockFn>(cvt[25])(surf, nullptr, desc, 1, nullptr) == 0) {   // Lock +0x64, DDLOCK_WAIT
            const int pitch = *reinterpret_cast<std::int32_t*>(desc + 0x10);
            auto* dst = *reinterpret_cast<unsigned char**>(desc + 0x24);
            for (int y = 0; y < nh; ++y)
                std::memcpy(dst + y * pitch, &next[static_cast<std::size_t>(y) * nw], static_cast<std::size_t>(nw) * 2);
            reinterpret_cast<UnlockFn>(cvt[32])(surf, nullptr);   // Unlock +0x80
        }
        cur.swap(next);
        w = nw;
        h = nh;
    }
    if (!first) reinterpret_cast<ReleaseFn>((*reinterpret_cast<void***>(surf))[2])(surf);
}

int __fastcall UploadWithMips(int rec_arg, int edx)
{
    auto* rec = reinterpret_cast<unsigned char*>(static_cast<std::uintptr_t>(rec_arg));
    const int w = *reinterpret_cast<std::int16_t*>(rec + 4), h = *reinterpret_cast<std::int16_t*>(rec + 6);
    const auto* src = *reinterpret_cast<const std::uint16_t* const*>(rec + 0x10);
    std::vector<std::uint16_t> copy;
    if (src && w > 1 && h > 1) {   // the original frees the source: keep a copy for the smaller levels
        const int stride = *reinterpret_cast<std::int32_t*>(rec + 0x34) > 0 ? *reinterpret_cast<std::int32_t*>(rec + 0x34) : w;
        copy.resize(static_cast<std::size_t>(w) * h);
        for (int y = 0; y < h; ++y) std::memcpy(&copy[static_cast<std::size_t>(y) * w], src + static_cast<std::size_t>(y) * stride, w * 2u);
    }
    const int r = g_upload(rec_arg, edx);
    void* surf = *reinterpret_cast<void**>(rec + 0x30);
    if (r && surf && !copy.empty()) FillMips(surf, copy.data(), w, h);
    return r;
}

// Same as the original (byte-for-byte behaviour above), plus the mip chain request
int __fastcall CreateTextureWithMips(int rec_arg, int caps_in)
{
    auto* rec = reinterpret_cast<unsigned char*>(static_cast<std::uintptr_t>(rec_arg));
    const std::int16_t h = *reinterpret_cast<std::int16_t*>(rec + 6), w = *reinterpret_cast<std::int16_t*>(rec + 4);
    if (*reinterpret_cast<std::uint32_t*>(rec + 0x14) != 0 || *reinterpret_cast<std::uint32_t*>(rec + 0x10) == 0 || !h || !w)
        return 0;
    int levels = 1;
    for (int s = (std::max)(w, h); s > 1; s >>= 1) ++levels;
    unsigned char desc[0x6c] = {};
    *reinterpret_cast<std::uint32_t*>(desc + 0x00) = 0x6c;
    *reinterpret_cast<std::uint32_t*>(desc + 0x04) = 0x10007 | (levels > 1 ? 0x20000u : 0u);
    *reinterpret_cast<std::int32_t*>(desc + 0x08) = h;
    *reinterpret_cast<std::int32_t*>(desc + 0x0c) = w;
    *reinterpret_cast<std::uint32_t*>(desc + 0x18) = static_cast<std::uint32_t>(levels);
    *reinterpret_cast<std::uint32_t*>(desc + 0x68) = static_cast<std::uint32_t>(caps_in) | 0x40u | (levels > 1 ? 0x400008u : 0u);
    *reinterpret_cast<std::uint32_t*>(rec + 0x30) = 0;
    void* dd = reinterpret_cast<void*>(static_cast<std::uintptr_t>(Data(0x1593d8)));   // [0x006333d8]
    using CreateFn = long(__stdcall*)(void*, void*, void**, void*);
    void* surf = nullptr;
    long hr = reinterpret_cast<CreateFn>((*reinterpret_cast<void***>(dd))[6])(dd, desc, &surf, nullptr);   // +0x18
    if (hr != 0 && levels > 1) {   // the device refuses mip chains: the original single level
        *reinterpret_cast<std::uint32_t*>(desc + 0x04) = 0x10007;
        *reinterpret_cast<std::uint32_t*>(desc + 0x68) = static_cast<std::uint32_t>(caps_in) | 0x40u;
        hr = reinterpret_cast<CreateFn>((*reinterpret_cast<void***>(dd))[6])(dd, desc, &surf, nullptr);
    }
    if (hr != 0 || !surf) { zVideo_ReportError(static_cast<int>(hr), static_cast<int>(reinterpret_cast<std::uintptr_t>(&Data(0x90e8))), 0x2ed); return *reinterpret_cast<int*>(rec + 0x30); }
    using QiFn = long(__stdcall*)(void*, const void*, void**);
    void* tex = nullptr;
    hr = reinterpret_cast<QiFn>((*reinterpret_cast<void***>(surf))[0])(surf, static_cast<const void*>(g_RData_004cc000 + 0x6fd8), &tex);
    if (hr != 0) { zVideo_ReportError(static_cast<int>(hr), static_cast<int>(reinterpret_cast<std::uintptr_t>(&Data(0x90e8))), 0x2ed); return *reinterpret_cast<int*>(rec + 0x30); }
    *reinterpret_cast<void**>(rec + 0x30) = tex;
    UploadWithMips(rec_arg, 0);
    return *reinterpret_cast<int*>(rec + 0x30);
}

// The mip chain is only sampled when the minification filter uses it. Render_CreateD3DDevice 0x004a9c20 sets
// D3DRENDERSTATE_TEXTUREMAG (0x11) and TEXTUREMIN (0x12) to D3DFILTER_LINEAR 2 once (CONFIRMED-BINARY, its
// SetRenderState [vtable+0x5c] calls; nothing else changes 0x12). Redux sets TEXTUREMIN to D3DFILTER_LINEARMIPLINEAR 6
// (PLATFORM) after the device is made: trilinear, smaller levels blended by distance.
TexFn g_create_device = nullptr;

int __fastcall CreateDeviceWithMipFilter(int ecx, int edx)
{
    const int r = g_create_device(ecx, edx);
    void* dev = reinterpret_cast<void*>(static_cast<std::uintptr_t>(Data(0x1593f0)));   // [0x006333f0] the device
    if (dev) {
        using SetRsFn = long(__stdcall*)(void*, std::uint32_t, std::uint32_t);
        reinterpret_cast<SetRsFn>((*reinterpret_cast<void***>(dev))[23])(dev, 0x12, 6);   // +0x5c SetRenderState
    }
    return r;
}

bool InitMipmaps()
{
    if (GetPrivateProfileIntA("Graphics", "Mipmaps", 0, kIni) != 1) return false;
    const unsigned char dhead[] = {0x81, 0xEC, 0xF0, 0x00, 0x00, 0x00};   // Render_CreateD3DDevice: SUB ESP,0xF0
    void* cd = reinterpret_cast<void*>(&Render_CreateD3DDevice);
    if (!(g_create_device = reinterpret_cast<TexFn>(Trampoline(cd, dhead, sizeof dhead))) ||
        !Hook(cd, reinterpret_cast<void*>(&CreateDeviceWithMipFilter), "Render_CreateD3DDevice")) {
        std::fprintf(stderr, "redux: RX-37 mip filter not installed (unexpected code)\n");
        return false;
    }
    const unsigned char head[] = {0x83, 0xEC, 0x74, 0x53, 0x55, 0x56};   // Texture_UploadPixels: SUB ESP,0x74 / PUSH EBX/EBP/ESI
    void* up = reinterpret_cast<void*>(&Texture_UploadPixels);
    if (!(g_upload = reinterpret_cast<TexFn>(Trampoline(up, head, sizeof head)))) {
        std::fprintf(stderr, "redux: RX-37 mipmaps not installed (unexpected code)\n");
        return false;
    }
    const bool ok = Hook(up, reinterpret_cast<void*>(&UploadWithMips), "Texture_UploadPixels") &&
                    Hook(reinterpret_cast<void*>(&DDraw_CreateTextureSurface), reinterpret_cast<void*>(&CreateTextureWithMips),
                         "DDraw_CreateTextureSurface");
    if (ok) std::fprintf(stderr, "redux: RX-37 textures get full mip chains\n");
    return ok;
}

// ------------------------------------------------------------------------------------------------ shadow flicker log
// [Debug] ShadowLog=1 (user 2026-10-17: with the game's lighting option off, an enemy's shadow vanishes for a split
// second when it fires). Diagnostic only: Render_ShadeAndQueueObjectPolygons 0x00477b30 (the hardware per-object
// shader, called through [0x0057d9e0] with ECX = the scene node; node name at node+0, CONFIRMED-BINARY
// Node_FindChildByNameRecursive 0x00452770) is wrapped (trampoline). For every node whose name contains "shad" it
// writes one line to shadow_log.txt: frame stamp [0x0056bbd8], the node drawn just before it, and the draw state the
// shadow starts with - the Direct3D state cache [0x006333f0+0x10..+0x5c] (texture handle, shade mode, map blend ...),
// the HW colour [0x006321d0..d8] / last pushed [0x006321f4..fc] (HwBlend_ApplyColourIfChanged 0x004a7330) and the
// blend preset block [0x0057de40..54], the transparent-queue count - and the same after it is drawn. Every 300th frame it also lists the names of
// all nodes drawn, so shadows with other names can be found. Nothing is changed.
using ObjDrawFn = int(__fastcall*)(int, int);
ObjDrawFn g_obj_draw = nullptr;
FILE* g_shadow_log = nullptr;
char g_prev_obj[40] = "-";
std::uint32_t g_names_frame = 0;
int g_shadow_lines = 0;
// RX-41 [Graphics] ShadowFix=1: shadows ignore lights. ShadowLog runs 2-3 (user, lighting option off, 2026-10-17):
// an enemy's "shadow" node reached the shader every frame but emitted no polygon for 5-13 frames at a time, and in
// most of those frames a light was active ([0x0057d418] light count, the test in Shade_SetupObjectLighting
// 0x00476a50 before Light_ComputeObjectInfluence 0x00487c50) - the enemy's gun-flash light. While a node named
// *shad* is shaded, Redux sets that count to 0, so the shadow is never lit (it is a shadow); it is restored after.
bool g_shadow_fix = false;

void LogDrawState(const char* tag)
{
    // transparent queue count [0x00632fa0], capacity 256: CONFIRMED-BINARY, the cmp ...,0x100 in every Render_QueuePolygon_*
    std::fprintf(g_shadow_log, " %s q %u d3d", tag, Data(0x158fa0));
    for (std::uint32_t o = 0x159400; o <= 0x15944c; o += 4) std::fprintf(g_shadow_log, " %x", Data(o));
    std::fprintf(g_shadow_log, " col");
    for (std::uint32_t o = 0x1581d0; o <= 0x1581d8; o += 4) { float f; std::memcpy(&f, &Data(o), 4); std::fprintf(g_shadow_log, " %.3f", f); }
    std::fprintf(g_shadow_log, " pushed");
    for (std::uint32_t o = 0x1581f4; o <= 0x1581fc; o += 4) { float f; std::memcpy(&f, &Data(o), 4); std::fprintf(g_shadow_log, " %.3f", f); }
    std::fprintf(g_shadow_log, " fade");   // [0x0057d930] fade on, [0x0057d944..58] ranges A/B (render.md), [0x0057d960] overwrite, [0x0057d964] fade
    for (std::uint32_t o : {0xa3930u, 0xa3944u, 0xa3948u, 0xa394cu, 0xa3950u, 0xa3954u, 0xa3958u, 0xa3960u, 0xa3964u}) {
        float f; std::memcpy(&f, &Data(o), 4); std::fprintf(g_shadow_log, " %g", f);
    }
    std::fprintf(g_shadow_log, " lightsel %x %x", Data(0xa39c0) ? *reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(Data(0xa39c0))) : 0u, Data(0xa3418));
    std::fprintf(g_shadow_log, " blend");
    for (std::uint32_t o = 0xa3e40; o <= 0xa3e54; o += 4) std::fprintf(g_shadow_log, " %x", Data(o));
}

int __fastcall ObjDrawLogged(int ecx, int edx)
{
    const char* name = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(ecx));
    char nm[40] = "?";
    if (name && !IsBadReadPtr(name, 32)) {
        std::size_t i = 0;
        for (; i < sizeof nm - 1 && name[i] >= 32 && name[i] < 127; ++i) nm[i] = name[i];
        nm[i] = 0;
    }
    const std::uint32_t frame = Data(0x91bd8);   // [0x0056bbd8] frame stamp
    char low[40];
    std::size_t i = 0;
    for (; nm[i]; ++i) low[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(nm[i])));
    low[i] = 0;
    const bool shadow = std::strstr(low, "shad") != nullptr;
    if (g_shadow_log && frame % 300 == 0) {
        if (g_names_frame != frame) { g_names_frame = frame; std::fprintf(g_shadow_log, "\nNAMES frame %u:", frame); }
        std::fprintf(g_shadow_log, " %s", nm);
    }
    if (g_shadow_log && shadow && g_shadow_lines < 200000) {
        std::fprintf(g_shadow_log, "\nSH %u %s after=%s", frame, nm, g_prev_obj);
        LogDrawState("before");
    }
    const std::uint32_t lights = Data(0xa3418);   // [0x0057d418] active light count
    if (g_shadow_fix && shadow) Data(0xa3418) = 0;
    const int r = g_obj_draw(ecx, edx);
    if (g_shadow_fix && shadow) Data(0xa3418) = lights;
    if (g_shadow_log && shadow && g_shadow_lines < 200000) {
        LogDrawState("after");
        ++g_shadow_lines;
        std::fflush(g_shadow_log);
    }
    std::memcpy(g_prev_obj, nm, sizeof g_prev_obj);
    return r;
}

bool InitShadowLog()   // [Debug] ShadowLog and RX-41 [Graphics] ShadowFix share the hook
{
    const bool log = GetPrivateProfileIntA("Debug", "ShadowLog", 0, kIni) == 1;
    g_shadow_fix = GetPrivateProfileIntA("Graphics", "ShadowFix", 0, kIni) == 1;
    if (!log && !g_shadow_fix) return false;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(&Render_ShadeAndQueueObjectPolygons);
    if (p[0] != 0x55 || p[1] != 0x8B || p[2] != 0xEC || p[3] != 0xB8) {   // PUSH EBP / MOV EBP,ESP / MOV EAX,imm32
        std::fprintf(stderr, "redux: shadow hook not installed (unexpected code)\n");
        return false;
    }
    if (log) g_shadow_log = std::fopen("shadow_log.txt", "w");
    if (!(g_obj_draw = reinterpret_cast<ObjDrawFn>(Trampoline(const_cast<unsigned char*>(p), p, 8))) ||
        !Hook(const_cast<unsigned char*>(p), reinterpret_cast<void*>(&ObjDrawLogged), "Render_ShadeAndQueueObjectPolygons"))
        return false;
    if (g_shadow_log) std::fprintf(stderr, "redux: shadow log -> shadow_log.txt\n");
    if (g_shadow_fix) std::fprintf(stderr, "redux: RX-41 shadows ignore lights\n");
    return true;
}

// ------------------------------------------------------------------------------------------------ RX-42 launcher menu
// [Launcher] TrimMenu=1 (user 2026-10-17, Redux v1): the start-up window's menu offers only what Redux v1 supports.
// The menu is the exe's MENU resource "MYMENU" (copied into recoil_boot.exe; command IDs read from the resource):
// Game: Multi Player Start 40019, Westwood Online 40062 (multiplayer is out of release 1); Options: Software - GameZ
// 40067, A3D 40066, Joystick 40061, and the resolutions 320x200 518, 320x240 519, 640x400 520, 800x600 40049,
// 1024x768 528. Those are removed; 640x480 521 stays and is renamed to the Redux resolution - with [Display]
// Resolution set every video mode uses it (RX-03), so the item only labels it - and is kept checked. The game edits the
// same menu at start-up and when options change (CheckMenuItem by ID), so a watcher thread re-applies the edits every
// half second; removing an item twice is harmless. No game code is changed. PLATFORM only.
const UINT kMenuRemove[] = {40019, 40062, 40067, 40066, 40061, 518, 519, 520, 40049, 528};
const UINT kMenuResItem = 521;

BOOL CALLBACK FindGameWindow(HWND w, LPARAM out)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (pid == GetCurrentProcessId() && GetMenu(w)) {
        *reinterpret_cast<HWND*>(out) = w;
        return FALSE;
    }
    return TRUE;
}

DWORD WINAPI MenuWatcher(void*)
{
    char label[32] = "1920 x 1080";
    if (g_width > 0 && g_height > 0) std::snprintf(label, sizeof label, "%d x %d", g_width, g_height);
    for (;;) {
        HWND w = nullptr;
        EnumWindows(FindGameWindow, reinterpret_cast<LPARAM>(&w));
        if (HMENU m = w ? GetMenu(w) : nullptr) {
            bool changed = false;
            for (UINT id : kMenuRemove)
                if (DeleteMenu(m, id, MF_BYCOMMAND)) changed = true;
            char cur[32] = {};
            if (GetMenuStringA(m, kMenuResItem, cur, sizeof cur, MF_BYCOMMAND) > 0 && std::strcmp(cur, label) != 0) {
                ModifyMenuA(m, kMenuResItem, MF_BYCOMMAND | MF_STRING, kMenuResItem, label);
                changed = true;
            }
            if (GetMenuState(m, kMenuResItem, MF_BYCOMMAND) != static_cast<UINT>(-1) &&
                !(GetMenuState(m, kMenuResItem, MF_BYCOMMAND) & MF_CHECKED)) {
                CheckMenuItem(m, kMenuResItem, MF_BYCOMMAND | MF_CHECKED);
                changed = true;
            }
            if (changed) DrawMenuBar(w);
        }
        Sleep(500);
    }
}

bool InitTrimMenu()
{
    if (GetPrivateProfileIntA("Launcher", "TrimMenu", 0, kIni) != 1) return false;
    CloseHandle(CreateThread(nullptr, 0, MenuWatcher, nullptr, 0, nullptr));
    std::fprintf(stderr, "redux: RX-42 launcher menu trimmed for Redux v1\n");
    return true;
}

// ------------------------------------------------------------------------------------------------ RX-44 settings kept
// [Launcher] KeepSettings=1 (user 2026-10-17: changed settings should load next time). The game saves its options to
// HKCU\SOFTWARE\Zipper\RECOIL\1.0, but Settings_LoadFromRegistry 0x004b2960 loads nothing unless the same key also
// opens under HKEY_LOCAL_MACHINE (CONFIRMED-BINARY, settings.md) - a key only the 1999 installer, run as administrator,
// created. So every start fell back to the hardware presets. Redux wraps the game's RegOpenKeyExA / RegCreateKeyExA
// import slots (0x004cc00c / 0x004cc008, PLATFORM) and opens HKEY_CURRENT_USER where the game asks for
// HKEY_LOCAL_MACHINE: both opens succeed, the saved values load after the presets (RecoilApp_LoadUserSettings
// 0x00407700 calls the load last) and saves need no administrator rights.
using RegOpenFn = LONG(WINAPI*)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
using RegCreateFn = LONG(WINAPI*)(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, const LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
RegOpenFn g_reg_open = nullptr;
RegCreateFn g_reg_create = nullptr;

LONG WINAPI RegOpenUser(HKEY root, LPCSTR sub, DWORD opt, REGSAM sam, PHKEY out)
{
    return g_reg_open(root == HKEY_LOCAL_MACHINE ? HKEY_CURRENT_USER : root, sub, opt, sam, out);
}

LONG WINAPI RegCreateUser(HKEY root, LPCSTR sub, DWORD res, LPSTR cls, DWORD opt, REGSAM sam, const LPSECURITY_ATTRIBUTES sa,
                          PHKEY out, LPDWORD disp)
{
    return g_reg_create(root == HKEY_LOCAL_MACHINE ? HKEY_CURRENT_USER : root, sub, res, cls, opt, sam, sa, out, disp);
}

// [Launcher] DefaultName=User (user 2026-10-17: no personal names in the game): RecoilApp_LoadUserSettings 0x00407700
// fills the pilot name (setting PlayerName, prefilled in the New Game panel, NewGamePanel_Ctor 0x0041c290) from
// GetUserNameA - the Windows logon name. Redux answers the game's GetUserNameA import slot (0x004cc000, PLATFORM) with this
// name instead. A name the player types is saved and loaded as before.
char g_default_name[32] = {};
using GetUserFn = BOOL(WINAPI*)(LPSTR, LPDWORD);

BOOL WINAPI DefaultUserName(LPSTR buf, LPDWORD size)
{
    const DWORD need = static_cast<DWORD>(std::strlen(g_default_name)) + 1;
    if (!buf || !size || *size < need) {
        if (size) *size = need;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    std::memcpy(buf, g_default_name, need);
    *size = need;
    return TRUE;
}

bool InitDefaultName()
{
    GetPrivateProfileStringA("Launcher", "DefaultName", "", g_default_name, sizeof g_default_name, kIni);
    if (!*g_default_name) return false;
    g_default_name[15] = 0;   // the game keeps 22 bytes for the name; 15 characters fit the New Game field
    g_Iat_GetUserNameA_004cc000 = reinterpret_cast<void*>(&DefaultUserName);
    std::fprintf(stderr, "redux: default pilot name '%s'\n", g_default_name);
    return true;
}

bool InitKeepSettings()
{
    if (GetPrivateProfileIntA("Launcher", "KeepSettings", 0, kIni) != 1) return false;
    g_reg_open = reinterpret_cast<RegOpenFn>(g_Iat_RegOpenKeyExA_004cc00c);
    g_reg_create = reinterpret_cast<RegCreateFn>(g_Iat_RegCreateKeyExA_004cc008);
    if (!g_reg_open || !g_reg_create) return false;
    g_Iat_RegOpenKeyExA_004cc00c = reinterpret_cast<void*>(&RegOpenUser);
    g_Iat_RegCreateKeyExA_004cc008 = reinterpret_cast<void*>(&RegCreateUser);
    std::fprintf(stderr, "redux: RX-44 game settings are kept between runs (HKCU)\n");
    return true;
}

}  // namespace

int Init()
{
    if (GetFileAttributesA(kIni) == INVALID_FILE_ATTRIBUTES) {
        std::fprintf(stderr, "redux: no redux.ini - Recoil Classic\n");
        return 0;
    }
    int n = 0;
    n += InitResolution();
    n += InitWideHud();
    n += InitTexturePack();
    n += InitScreenshotKey();
    n += InitScreenshotNote();
    n += InitMap();
    n += InitMaxDetail();
    n += InitMipmaps();
    n += InitShadowLog();
    n += InitTrimMenu();
    n += InitKeepSettings();
    n += InitDefaultName();
    n += InitHdVideo();
    std::fprintf(stderr, n ? "redux: %d feature(s) on\n" : "redux: every feature off - Recoil Classic\n", n);
    return n;
}

}  // namespace redux
