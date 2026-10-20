// Loader for golden vectors produced by tools/emu_harness/emu.py (L1: the original function's bytes
// run in Ghidra's emulator). Minimal parser for the fixed JSON layout the harness writes: every value
// is a hex string of a little-endian float or dword.
#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace rt {

#ifndef RECOIL_VECTORS_DIR
#define RECOIL_VECTORS_DIR "tests/vectors"
#endif

// One case: field name -> list of hex strings (a scalar field is a list of one).
using VecCase = std::map<std::string, std::vector<std::string>>;

inline std::vector<VecCase> load_vectors(const std::string& name)
{
    std::ifstream f(std::string(RECOIL_VECTORS_DIR) + "/" + name + ".json");
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string s = ss.str();
    std::vector<VecCase> out;
    size_t p = s.find("\"cases\"");
    if (p == std::string::npos) return out;
    p = s.find('[', p);
    VecCase cur;
    std::string key;
    int depth = 0;
    for (size_t i = p + 1; i < s.size(); ++i) {
        char c = s[i];
        if (c == '{') { ++depth; cur.clear(); }
        else if (c == '}') { --depth; out.push_back(cur); }
        else if (c == ']' && depth == 0) break;
        else if (c == '"') {
            size_t j = s.find('"', i + 1);
            std::string tok = s.substr(i + 1, j - i - 1);
            size_t k = j + 1;
            while (k < s.size() && (s[k] == ' ' || s[k] == '\n' || s[k] == '\r')) ++k;
            if (k < s.size() && s[k] == ':') key = tok;
            else cur[key].push_back(tok);
            i = j;
        }
    }
    return out;
}

inline std::uint32_t hex_dword(const std::string& h)
{
    std::uint32_t v = 0;
    for (int i = 3; i >= 0; --i) v = (v << 8) | static_cast<std::uint32_t>(std::stoul(h.substr(i * 2, 2), nullptr, 16));
    return v;
}

inline float hex_float(const std::string& h)
{
    std::uint32_t d = hex_dword(h);
    float f;
    std::memcpy(&f, &d, 4);
    return f;
}

inline std::uint32_t float_bits(float f)
{
    std::uint32_t d;
    std::memcpy(&d, &f, 4);
    return d;
}

}  // namespace rt
