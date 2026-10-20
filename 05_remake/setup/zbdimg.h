// zbdimg.h - the game's texture archives and PNG files, shared by Setup's texture generator (upscale.cpp) and map
// generator (mapgen.cpp). Archive format in upscale.cpp's header. Not Recoil code (PLATFORM).
#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace setup {

struct Img {
    int w = 0, h = 0;
    std::vector<std::uint8_t> bgra;   // w*h*4
};

struct Entry {
    std::string name;
    Img colour;
    int aw = 0, ah = 0;
    std::vector<std::uint8_t> alpha;   // aw*ah, empty = opaque
};

bool ReadAll(const std::filesystem::path& p, std::vector<std::uint8_t>& d);
// name (lower case) -> picture; alpha planes only when want_alpha
std::map<std::string, Entry> ReadArchive(const std::filesystem::path& p, bool want_alpha);
bool SavePng(const std::filesystem::path& p, const Img& im, bool alpha);
bool LoadPng(const std::filesystem::path& p, Img& im);
Img Resize(const Img& src, int tw, int th);   // high-quality (cubic), through WIC

}  // namespace setup
