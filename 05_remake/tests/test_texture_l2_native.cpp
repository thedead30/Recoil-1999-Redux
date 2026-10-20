// L2 for texture: every shipped texture archive (the 61 .zbd files with a version-1 texture header: image.zbd and the
// mission texture2/4/6/8 and rtexture2/4 archives) is opened by TextureArchive_Open on the port and on the original
// (565 screen, one shade entry, so every archive palette gets its 32 shade levels), and every table-of-contents entry
// is loaded by Image_LoadFromArchiveListA on both sides. Compared:
//   - the archive record and its shared palettes (palette and shade-level bytes) between the sides;
//   - each image between the sides (header words, pixel / alpha bytes, palette as shared index or own bytes);
//   - each image's pixel and alpha bytes against an independent parse of the file (16-byte header: flags byte 0,
//     width / height words 4 / 6, palette size word 0xC; pixels = palette size ? w*h : (1 + flags&1) * w*h bytes, then
//     w*h alpha bytes when flags & 8).
// Duplicate names resolve to the first TOC entry on both sides (and in the independent parse).
#include "test.h"
#include "native_oracle.h"
#include "unattributed/texture.h"
#include "platform/iat_msvcrt.h"
#include "platform/image/original_data.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

namespace {
template <class T> T* ptr(std::uint32_t v) { return reinterpret_cast<T*>(static_cast<std::uintptr_t>(v)); }
std::uint32_t addr(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }
std::uint32_t* img(int side, std::uint32_t va) { return static_cast<std::uint32_t*>(side ? recoil::ImageData_Address(va) : ptr<void>(va)); }
void real_free(void* p) { reinterpret_cast<void(__cdecl*)(void*)>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"))(p); }
std::uint32_t fnv(const void* p, std::size_t n)
{
    std::uint32_t h = 2166136261u;
    for (std::size_t k = 0; k < n; ++k) h = (h ^ static_cast<const unsigned char*>(p)[k]) * 16777619u;
    return h;
}
std::vector<unsigned char> read_file(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}
std::uint32_t u32(const std::vector<unsigned char>& b, std::size_t o) { std::uint32_t v = 0; if (o + 4 <= b.size()) std::memcpy(&v, &b[o], 4); return v; }
std::uint16_t u16(const std::vector<unsigned char>& b, std::size_t o) { std::uint16_t v = 0; if (o + 2 <= b.size()) std::memcpy(&v, &b[o], 2); return v; }
}  // namespace

TEST(native_texture_l2_shipped_archives)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using Open = std::uint32_t(__fastcall*)(void*, int);
    using Load = std::uint32_t(__fastcall*)(const char*, int);
    using Fclose = int(__cdecl*)(void*);
    const Open open[2] = {rt::original<Open>(0x0046dae0), reinterpret_cast<Open>(&recoil::TextureArchive_Open)};
    const Load load[2] = {rt::original<Load>(0x0046dd30), reinterpret_cast<Load>(&recoil::Image_LoadFromArchiveListA)};
    const auto m_fclose = reinterpret_cast<Fclose>(recoil::g_Iat_fclose_004cc5c0);
    std::string root = RECOIL_ORIGINAL_EXE;
    root = root.substr(0, root.find_last_of("/\\") + 1) + "zbd/";
    std::vector<std::string> files = {"image.zbd"};
    for (int m = 1; m <= 13; ++m)
        for (const char* n : {"texture2.zbd", "texture4.zbd", "texture6.zbd", "texture8.zbd", "rtexture2.zbd", "rtexture4.zbd"}) {
            const std::string rel = "m" + std::to_string(m) + "/" + n;
            if (GetFileAttributesA((root + rel).c_str()) != INVALID_FILE_ATTRIBUTES) files.push_back(rel);
        }
    const float shade[8] = {0.1f, 0.2f, 0.3f, 0.9f, 0.8f, 0.7f, 0.5f, 0.25f};
    int archives = 0, images = 0, palettes = 0;
    for (const std::string& rel : files) {
        const std::string path = root + rel;
        const std::vector<unsigned char> file = read_file(path);
        const std::uint32_t pals = u32(file, 8), toc = u32(file, 12);
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            // 565 screen: palettes and pixels unconverted; one shade entry: 32 levels per archive palette
            for (std::uint32_t va : {0x00632158u, 0x0063215cu, 0x00632160u}) *img(side, va) = 6;
            const std::uint32_t pack[5] = {11, 5, 3, 0x1f, 0x3f};
            for (int k = 0; k < 5; ++k) *img(side, 0x00632170u + 4 * k) = pack[k];
            *img(side, 0x0053d780) = 1;
            *img(side, 0x0053d784) = addr(shade);
            *img(side, 0x004e073c) = 1;
            *img(side, 0x0053d778) = 0;
            *img(side, 0x0053d77c) = 0;
            std::uint32_t rec[41] = {};
            std::strcpy(reinterpret_cast<char*>(rec), path.c_str());
            const std::uint32_t f = open[side](rec, 0);
            CHECK(f != 0);
            if (!f) continue;
            snap[side].push_back(rec[0x84 / 4]); snap[side].push_back(rec[0x88 / 4]); snap[side].push_back(rec[0x8C / 4]);
            snap[side].push_back(rec[0x90 / 4]); snap[side].push_back(rec[0xA0 / 4]);
            CHECK_EQ(rec[0x8C / 4], pals);
            CHECK_EQ(rec[0x90 / 4], toc);
            CHECK(std::memcmp(ptr<void>(rec[0x9C / 4]), &file[24], 0x28 * static_cast<std::size_t>(toc)) == 0);
            const std::uint32_t npal = *img(side, 0x0053d778);
            const auto* plist = ptr<std::uint32_t>(*img(side, 0x0053d77c));
            CHECK_EQ(npal, pals);
            for (std::uint32_t p = 0; p < npal; ++p) {
                // palette bytes against the file (565 screen: no conversion), shade levels between the sides
                CHECK(std::memcmp(ptr<void>(plist[p]), &file[24 + 0x28 * static_cast<std::size_t>(toc) + 0x200 * p], 0x200) == 0);
                snap[side].push_back(fnv(ptr<void>(plist[p]), 0x4000 + 0x200));
                palettes += side;
            }
            // list A = this archive; list B present but without a file
            *img(side, 0x0053d768) = 1;
            *img(side, 0x0053d76c) = addr(rec);
            std::map<std::string, bool> seen;
            for (std::uint32_t e = 0; e < toc; ++e) {
                const std::size_t ent = 24 + 0x28 * static_cast<std::size_t>(e);
                const std::string name(reinterpret_cast<const char*>(&file[ent]), strnlen(reinterpret_cast<const char*>(&file[ent]), 0x20));
                std::string key = name;
                for (char& c : key) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                if (seen.count(key)) continue;  // _stricmp: the first entry answers
                seen[key] = true;
                const std::uint32_t r = load[side](name.c_str(), 0);
                CHECK(r != 0);
                if (!r) { snap[side].push_back(0xDEADu); continue; }
                const auto* im = ptr<std::uint32_t>(r);
                // independent parse of the entry
                const std::size_t off = u32(file, ent + 0x20);
                const int pal_ix = static_cast<int>(u32(file, ent + 0x24));
                const unsigned flags = file[off];
                const std::size_t w = u16(file, off + 4), h = u16(file, off + 6), psz = u16(file, off + 0xC);
                const std::size_t bpp = 1 + (flags & 1), count = w * h;
                const std::size_t pixbytes = psz ? count : bpp * count;
                CHECK_EQ(im[0], static_cast<std::uint32_t>(count));
                CHECK(std::memcmp(ptr<void>(im[4]), &file[off + 16], pixbytes) == 0);
                if (flags & 8) CHECK(im[5] && std::memcmp(ptr<void>(im[5]), &file[off + 16 + pixbytes], count) == 0);
                for (int k = 0; k < 14; ++k) snap[side].push_back(k >= 4 && k <= 6 ? (im[k] ? 1u : 0u) : im[k]);
                snap[side].push_back(fnv(ptr<void>(im[4]), pixbytes));
                std::uint32_t pal_role = 0xFFFFFFFFu;
                for (std::uint32_t p = 0; p < npal; ++p) if (im[6] == plist[p]) pal_role = p;
                snap[side].push_back(pal_role);
                if (pal_ix != -1) CHECK_EQ(pal_role, static_cast<std::uint32_t>(pal_ix));
                images += side;
                // release: pixels, alpha, an own palette (never a shared one), the image
                real_free(ptr<void>(im[4]));
                if (im[5]) real_free(ptr<void>(im[5]));
                if (im[6] && pal_role == 0xFFFFFFFFu) real_free(ptr<void>(im[6]));
                real_free(ptr<void>(r));
            }
            m_fclose(ptr<void>(rec[0x80 / 4]));
            real_free(ptr<void>(rec[0x9C / 4]));
            for (std::uint32_t p = 0; p < npal; ++p) real_free(ptr<void>(plist[p]));
            if (plist) real_free(ptr<void>(*img(side, 0x0053d77c)));
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++archives;
    }
    rt::restore_pristine();
    std::printf("  texture L2: %d shipped archives, %d images, %d palettes with shade ramps (both sides; images against the files)\n",
                archives, images, palettes);
}
