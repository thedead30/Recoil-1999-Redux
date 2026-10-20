// upscale.cpp - Setup's "Generate HD textures on this PC" (see upscale.h). Runs on the player's machine with nothing
// installed: archives read here, images through Windows Imaging Component, Real-ESRGAN as the bundled portable
// realesrgan-ncnn-vulkan.exe (any Vulkan graphics card), a smooth enlargement when it cannot run.
//
// Texture archives (04_spec/systems/asset_io.md, MEASURED-ASSET; the same reader as redux_assets/tools/extract_textures.py):
// header 6 dwords {0, version, palette count, entry count, 0, 0}; entry count x 0x28 bytes {name[0x20], data offset,
// palette index (-1 none)}; palettes x 512 bytes (256 x RGB565); each image: 16-byte header {byte flags, ..., word width
// +4, word height +6, word own-palette entries +12} then pixels - 1 byte per pixel with a palette, else RGB565; flag 0x10 =
// shared palette, otherwise an own palette follows the pixels (after the alpha plane when flag 8); flag 8 = alpha plane
// (one byte per pixel) after the pixels.
// What the game loads in Direct3D mode is rtexture4.zbd (names, alpha); texture6.zbd holds the same pictures at the
// highest detail (often twice the size) and gives the colour when present (maps 1-6, 13). Like
// redux_assets/tools/build_game_pack.py: colour enlarged, the archive's alpha plane enlarged smoothly, colour spread into
// transparent pixels. Pictures that differ between maps go to m<N>\ (the Redux pack layout, RX-10).
// Not Recoil code (PLATFORM).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wincodec.h>

#include "upscale.h"
#include "zbdimg.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#pragma comment(lib, "windowscodecs.lib")

namespace setup {

namespace fs = std::filesystem;

bool ReadAll(const fs::path& p, std::vector<std::uint8_t>& d)
{
    HANDLE h = CreateFileW(p.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz{};
    GetFileSizeEx(h, &sz);
    d.resize(std::size_t(sz.QuadPart));
    DWORD got = 0;
    const bool ok = d.empty() || (ReadFile(h, d.data(), DWORD(d.size()), &got, nullptr) && got == d.size());
    CloseHandle(h);
    return ok;
}

std::uint16_t U16(const std::vector<std::uint8_t>& b, std::size_t o) { return std::uint16_t(b[o] | (b[o + 1] << 8)); }
std::uint32_t U32(const std::vector<std::uint8_t>& b, std::size_t o) { return b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (std::uint32_t(b[o + 3]) << 24); }

void Put565(std::uint8_t* px, std::uint16_t v)
{
    px[2] = std::uint8_t(((v >> 11) & 31) * 255 / 31);
    px[1] = std::uint8_t(((v >> 5) & 63) * 255 / 63);
    px[0] = std::uint8_t((v & 31) * 255 / 31);
    px[3] = 255;
}

// name (lower case) -> picture; alpha planes only when want_alpha
std::map<std::string, Entry> ReadArchive(const fs::path& p, bool want_alpha)
{
    std::map<std::string, Entry> out;
    std::vector<std::uint8_t> b;
    if (!ReadAll(p, b) || b.size() < 24) return out;
    const std::uint32_t npal = U32(b, 8), n = U32(b, 12);
    const std::size_t pal_at = 24 + std::size_t(n) * 0x28;
    if (pal_at + std::size_t(npal) * 512 > b.size()) return out;
    for (std::uint32_t i = 0; i < n; ++i) {
        const std::size_t e = 24 + std::size_t(i) * 0x28;
        std::string name(reinterpret_cast<const char*>(&b[e]), strnlen(reinterpret_cast<const char*>(&b[e]), 0x20));
        const std::size_t off = U32(b, e + 0x20);
        const std::int32_t pal = std::int32_t(U32(b, e + 0x24));
        if (off + 16 > b.size()) continue;
        const std::uint8_t flags = b[off];
        const int w = U16(b, off + 4), h = U16(b, off + 6), own = U16(b, off + 12);
        if (w <= 0 || h <= 0 || w > 4096 || h > 4096) continue;
        const std::size_t d = off + 16, np = std::size_t(w) * h;
        Entry en;
        en.name = name;
        en.colour.w = w;
        en.colour.h = h;
        en.colour.bgra.resize(np * 4);
        std::uint8_t* px = en.colour.bgra.data();
        if (!(flags & 0x10) && own) {   // own palette after the pixels (and the alpha plane)
            const std::size_t po = d + np + ((flags & 8) ? np : 0);
            if (po + std::size_t(own) * 2 > b.size()) continue;
            for (std::size_t k = 0; k < np; ++k) {
                const unsigned ix = b[d + k];
                Put565(px + k * 4, ix < unsigned(own) ? U16(b, po + ix * 2) : 0);
            }
        } else if (pal >= 0 && std::uint32_t(pal) < npal) {
            if (d + np > b.size()) continue;
            const std::size_t po = pal_at + std::size_t(pal) * 512;
            for (std::size_t k = 0; k < np; ++k) Put565(px + k * 4, U16(b, po + b[d + k] * 2));
        } else {
            if (d + np * 2 > b.size()) continue;
            for (std::size_t k = 0; k < np; ++k) Put565(px + k * 4, U16(b, d + k * 2));
            if (want_alpha && (flags & 8) && d + np * 3 <= b.size()) {
                en.aw = w;
                en.ah = h;
                en.alpha.assign(b.begin() + long(d + np * 2), b.begin() + long(d + np * 3));
            }
        }
        std::string key = name;
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return char(std::tolower(c)); });
        out[key] = std::move(en);
    }
    return out;
}

// ---------------------------------------------------------------- WIC
IWICImagingFactory* Wic()
{
    static IWICImagingFactory* f = [] {
        IWICImagingFactory* x = nullptr;
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&x));
        return x;
    }();
    return f;
}

bool SavePng(const fs::path& p, const Img& im, bool alpha)
{
    IWICImagingFactory* f = Wic();
    if (!f) return false;
    IWICStream* st = nullptr;
    IWICBitmapEncoder* enc = nullptr;
    IWICBitmapFrameEncode* fr = nullptr;
    bool ok = SUCCEEDED(f->CreateStream(&st)) && SUCCEEDED(st->InitializeFromFilename(p.c_str(), GENERIC_WRITE)) &&
              SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &enc)) && SUCCEEDED(enc->Initialize(st, WICBitmapEncoderNoCache)) &&
              SUCCEEDED(enc->CreateNewFrame(&fr, nullptr)) && SUCCEEDED(fr->Initialize(nullptr)) && SUCCEEDED(fr->SetSize(UINT(im.w), UINT(im.h)));
    if (ok) {
        if (alpha) {
            WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
            ok = SUCCEEDED(fr->SetPixelFormat(&fmt)) && fmt == GUID_WICPixelFormat32bppBGRA &&
                 SUCCEEDED(fr->WritePixels(UINT(im.h), UINT(im.w * 4), UINT(im.bgra.size()), const_cast<BYTE*>(im.bgra.data())));
        } else {
            std::vector<std::uint8_t> bgr(std::size_t(im.w) * im.h * 3);
            for (std::size_t k = 0, n = std::size_t(im.w) * im.h; k < n; ++k)
                std::copy_n(&im.bgra[k * 4], 3, &bgr[k * 3]);
            WICPixelFormatGUID fmt = GUID_WICPixelFormat24bppBGR;
            ok = SUCCEEDED(fr->SetPixelFormat(&fmt)) && fmt == GUID_WICPixelFormat24bppBGR &&
                 SUCCEEDED(fr->WritePixels(UINT(im.h), UINT(im.w * 3), UINT(bgr.size()), bgr.data()));
        }
        ok = ok && SUCCEEDED(fr->Commit()) && SUCCEEDED(enc->Commit());
    }
    if (fr) fr->Release();
    if (enc) enc->Release();
    if (st) st->Release();
    return ok;
}

bool LoadPng(const fs::path& p, Img& im)
{
    IWICImagingFactory* f = Wic();
    if (!f) return false;
    IWICBitmapDecoder* dec = nullptr;
    IWICBitmapFrameDecode* fr = nullptr;
    IWICFormatConverter* cv = nullptr;
    UINT w = 0, h = 0;
    bool ok = SUCCEEDED(f->CreateDecoderFromFilename(p.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &dec)) &&
              SUCCEEDED(dec->GetFrame(0, &fr)) && SUCCEEDED(f->CreateFormatConverter(&cv)) &&
              SUCCEEDED(cv->Initialize(fr, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom)) &&
              SUCCEEDED(cv->GetSize(&w, &h)) && w && h;
    if (ok) {
        im.w = int(w);
        im.h = int(h);
        im.bgra.resize(std::size_t(w) * h * 4);
        ok = SUCCEEDED(cv->CopyPixels(nullptr, w * 4, UINT(im.bgra.size()), im.bgra.data()));
    }
    if (cv) cv->Release();
    if (fr) fr->Release();
    if (dec) dec->Release();
    return ok;
}

// high-quality (cubic) resize through WIC
Img Resize(const Img& src, int tw, int th)
{
    Img out;
    out.w = tw;
    out.h = th;
    out.bgra.assign(std::size_t(tw) * th * 4, 0);
    IWICImagingFactory* f = Wic();
    IWICBitmap* bmp = nullptr;
    IWICBitmapScaler* sc = nullptr;
    if (f && SUCCEEDED(f->CreateBitmapFromMemory(UINT(src.w), UINT(src.h), GUID_WICPixelFormat32bppBGRA, UINT(src.w * 4),
                                                 UINT(src.bgra.size()), const_cast<BYTE*>(src.bgra.data()), &bmp)) &&
        SUCCEEDED(f->CreateBitmapScaler(&sc)) && SUCCEEDED(sc->Initialize(bmp, UINT(tw), UINT(th), WICBitmapInterpolationModeHighQualityCubic)))
        sc->CopyPixels(nullptr, UINT(tw * 4), UINT(out.bgra.size()), out.bgra.data());
    if (sc) sc->Release();
    if (bmp) bmp->Release();
    return out;
}

namespace {

// alpha plane enlarged bilinearly to tw x th
std::vector<std::uint8_t> ScaleAlpha(const std::vector<std::uint8_t>& a, int aw, int ah, int tw, int th)
{
    std::vector<std::uint8_t> out(std::size_t(tw) * th);
    for (int y = 0; y < th; ++y) {
        const float fy = std::max(0.0f, (y + 0.5f) * ah / th - 0.5f);
        const int y0 = std::min(int(fy), ah - 1), y1 = std::min(y0 + 1, ah - 1);
        const float wy = fy - y0;
        for (int x = 0; x < tw; ++x) {
            const float fx = std::max(0.0f, (x + 0.5f) * aw / tw - 0.5f);
            const int x0 = std::min(int(fx), aw - 1), x1 = std::min(x0 + 1, aw - 1);
            const float wx = fx - x0;
            const float v = (a[y0 * aw + x0] * (1 - wx) + a[y0 * aw + x1] * wx) * (1 - wy) + (a[y1 * aw + x0] * (1 - wx) + a[y1 * aw + x1] * wx) * wy;
            out[std::size_t(y) * tw + x] = std::uint8_t(std::clamp(v + 0.5f, 0.0f, 255.0f));
        }
    }
    return out;
}

// spread opaque colours into transparent pixels (alpha < 8) so texture filtering does not bleed the old key colour
void PadColours(Img& im, const std::vector<std::uint8_t>& alpha)
{
    const int w = im.w, h = im.h;
    std::vector<std::uint8_t> filled(std::size_t(w) * h);
    for (std::size_t k = 0; k < filled.size(); ++k) filled[k] = alpha[k] >= 8;
    for (int it = 0; it < 12; ++it) {
        std::vector<std::uint8_t> next = filled;
        std::vector<std::uint8_t> px = im.bgra;
        bool any = false;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                const std::size_t k = std::size_t(y) * w + x;
                if (filled[k]) continue;
                int s[3] = {0, 0, 0}, n = 0;
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx) {
                        const int xx = x + dx, yy = y + dy;
                        if (xx < 0 || yy < 0 || xx >= w || yy >= h) continue;
                        const std::size_t j = std::size_t(yy) * w + xx;
                        if (!filled[j]) continue;
                        for (int c = 0; c < 3; ++c) s[c] += im.bgra[j * 4 + c];
                        ++n;
                    }
                if (!n) continue;
                for (int c = 0; c < 3; ++c) px[k * 4 + c] = std::uint8_t(s[c] / n);
                next[k] = 1;
                any = true;
            }
        im.bgra.swap(px);
        filled.swap(next);
        if (!any) break;
    }
    for (std::size_t k = 0; k < filled.size(); ++k) im.bgra[k * 4 + 3] = alpha[k];
}

std::uint64_t Hash(const Entry& e)
{
    std::uint64_t x = 1469598103934665603ull;
    auto add = [&](const std::uint8_t* p, std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) x = (x ^ p[i]) * 1099511628211ull;
    };
    const int dims[4] = {e.colour.w, e.colour.h, e.aw, e.ah};
    add(reinterpret_cast<const std::uint8_t*>(dims), sizeof dims);
    add(e.colour.bgra.data(), e.colour.bgra.size());
    add(e.alpha.data(), e.alpha.size());
    return x;
}

// runs realesrgan-ncnn-vulkan on a folder (x4, realesrgan-x4plus), reporting progress from the output count
bool RunEsrgan(const std::wstring& exe_dir, const fs::path& in, const fs::path& out, int total, const std::function<void(int, int)>& progress,
               int done_before, int grand_total)
{
    fs::create_directories(out);
    std::wstring cmd = L"\"" + exe_dir + L"\\realesrgan-ncnn-vulkan.exe\" -i \"" + in.wstring() + L"\" -o \"" + out.wstring() +
                       L"\" -n realesrgan-x4plus -s 4 -t 32 -f png";
    STARTUPINFOW si{sizeof si};
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, exe_dir.c_str(), &si, &pi)) return false;
    while (WaitForSingleObject(pi.hProcess, 500) == WAIT_TIMEOUT) {
        int n = 0;
        for (auto& f : fs::directory_iterator(out)) n += f.is_regular_file();
        progress(done_before + std::min(n, total), grand_total);
    }
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    int n = 0;
    for (auto& f : fs::directory_iterator(out)) n += f.is_regular_file();
    return code == 0 && n > 0;
}

}  // namespace

int GenerateTextures(const std::wstring& dest, const std::wstring& esrgan_dir, const std::function<void(const std::string&)>& log,
                     const std::function<void(int, int)>& progress)
{
    const fs::path root(dest), work = root / L"_hd_work", pack = root / L"redux_textures";
    std::error_code ec;
    fs::remove_all(work, ec);
    fs::create_directories(work / L"in", ec);
    fs::create_directories(pack, ec);
    // 1. the pictures of every map, each different picture once
    struct Job {
        std::wstring rel;   // "name.png" or "m5\name.png"
        Entry e;
        bool thin = false;
    };
    std::vector<Job> jobs;
    struct Seen {
        std::uint64_t id;   // the Direct3D picture (rtexture4 colour + alpha): the same in every map that shares it
        std::size_t job;
        bool from_t6;
    };
    std::map<std::string, Seen> shared;
    std::map<std::pair<std::string, std::uint64_t>, std::size_t> versions;   // other maps' versions, made once each
    for (int m = 1; m <= 13; ++m) {
        const fs::path dir = root / L"zbd" / (L"m" + std::to_wstring(m));
        auto rt = ReadArchive(dir / L"rtexture4.zbd", true);
        if (rt.empty()) continue;
        auto t6 = ReadArchive(dir / L"texture6.zbd", false);
        for (auto& [key, e] : rt) {
            const std::uint64_t id = Hash(e);
            auto it = t6.find(key);
            const bool has6 = it != t6.end() && it->second.colour.w >= e.colour.w && it->second.colour.h >= e.colour.h;
            Entry job;
            job.name = e.name;
            job.colour = has6 ? it->second.colour : e.colour;
            job.aw = e.aw;
            job.ah = e.ah;
            job.alpha = e.alpha;
            std::string file = e.name;
            std::replace(file.begin(), file.end(), '/', '_');
            std::replace(file.begin(), file.end(), '\\', '_');
            const std::wstring wfile(file.begin(), file.end());
            auto s = shared.find(key);
            if (s == shared.end()) {
                shared[key] = {id, jobs.size(), has6};
                jobs.push_back({wfile + L".png", std::move(job)});
            } else if (s->second.id == id) {
                if (has6 && !s->second.from_t6) {   // the same picture, now with its high-detail colour
                    jobs[s->second.job].e.colour = job.colour;
                    s->second.from_t6 = true;
                }
            } else if (!versions.count({key, id})) {
                versions[{key, id}] = jobs.size();
                jobs.push_back({L"m" + std::to_wstring(m) + L"\\" + wfile + L".png", std::move(job)});
            }
        }
    }
    if (jobs.empty()) {
        log("HD textures: no texture archives found in the install folder");
        return 0;
    }
    const int total = int(jobs.size());
    log("HD textures: " + std::to_string(total) + " pictures from your game files");
    // 2. inputs (thin pictures tiled 3x3 so the upscaler sees enough context, as the pack tools do)
    for (int i = 0; i < total; ++i) {
        Job& j = jobs[std::size_t(i)];
        Img in = j.e.colour;
        j.thin = std::min(in.w, in.h) < 16;
        if (j.thin) {
            Img t;
            t.w = in.w * 3;
            t.h = in.h * 3;
            t.bgra.resize(std::size_t(t.w) * t.h * 4);
            for (int y = 0; y < t.h; ++y)
                for (int x = 0; x < t.w; ++x)
                    std::copy_n(&in.bgra[(std::size_t(y % in.h) * in.w + x % in.w) * 4], 4, &t.bgra[(std::size_t(y) * t.w + x) * 4]);
            in = std::move(t);
        }
        SavePng(work / L"in" / (std::to_wstring(i) + L".png"), in, false);
    }
    // 3. Real-ESRGAN x4, and once more for small pictures (up to 64 px -> up to 1024 px)
    const bool have = GetFileAttributesW((esrgan_dir + L"\\realesrgan-ncnn-vulkan.exe").c_str()) != INVALID_FILE_ATTRIBUTES;
    bool esrgan = have && RunEsrgan(esrgan_dir, work / L"in", work / L"out1", total, progress, 0, total * 2);
    if (!esrgan) log("HD textures: Real-ESRGAN could not run (it needs a Vulkan graphics card) - using a smooth enlargement instead");
    if (esrgan) {
        fs::create_directories(work / L"in2", ec);
        int second = 0;
        for (int i = 0; i < total; ++i) {
            const Job& j = jobs[std::size_t(i)];
            if (j.thin || std::max(j.e.colour.w, j.e.colour.h) > 64) continue;
            fs::copy_file(work / L"out1" / (std::to_wstring(i) + L".png"), work / L"in2" / (std::to_wstring(i) + L".png"), ec);
            ++second;
        }
        if (second) RunEsrgan(esrgan_dir, work / L"in2", work / L"out2", second, progress, total, total * 2);
    }
    // 4. final size (long side up to 1024, at most x16), alpha, write
    int written = 0, with_alpha = 0;
    for (int i = 0; i < total; ++i) {
        const Job& j = jobs[std::size_t(i)];
        const int w = j.e.colour.w, h = j.e.colour.h;
        const int k = std::clamp(1024 / std::max(w, h), 1, 16);
        const int tw = w * k, th = h * k;
        Img up;
        const std::wstring f = std::to_wstring(i) + L".png";
        bool got = esrgan && (LoadPng(work / L"out2" / f, up) || LoadPng(work / L"out1" / f, up));
        if (got && j.thin) {   // the middle tile of the 3x3
            const int s = up.w / (w * 3);
            Img c;
            c.w = w * s;
            c.h = h * s;
            c.bgra.resize(std::size_t(c.w) * c.h * 4);
            for (int y = 0; y < c.h; ++y)
                std::copy_n(&up.bgra[(std::size_t(y + h * s) * up.w + w * s) * 4], std::size_t(c.w) * 4, &c.bgra[std::size_t(y) * c.w * 4]);
            up = std::move(c);
        }
        if (!got) up = j.e.colour;
        Img out = (up.w == tw && up.h == th) ? up : Resize(up, tw, th);
        const bool alpha = !j.e.alpha.empty();
        if (alpha) {
            PadColours(out, ScaleAlpha(j.e.alpha, j.e.aw, j.e.ah, tw, th));
            ++with_alpha;
        }
        fs::create_directories((pack / j.rel).parent_path(), ec);
        if (SavePng(pack / j.rel, out, alpha)) ++written;
        progress(total + i + 1, total * 2);
    }
    fs::remove_all(work, ec);
    log("HD textures: " + std::to_string(written) + " written (" + std::to_string(with_alpha) + " with transparency)" +
        (esrgan ? ", Real-ESRGAN" : ", smooth enlargement"));
    return written;
}

}  // namespace setup
