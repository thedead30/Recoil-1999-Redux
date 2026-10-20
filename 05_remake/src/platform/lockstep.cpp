// SUBSYSTEM: platform
// L4 lockstep stream loader, seam feeder and state comparer (tools/lockstep/FORMAT.md).
// Minimal JSON reader for the fixed stream layout; no external dependency.
#include "platform/lockstep.h"

#include <cctype>
#include <fstream>
#include <stdexcept>

namespace recoil::platform {
namespace {

// Tiny JSON value for the stream's subset: objects, arrays, strings, integers.
struct J {
    enum K { Null, Num, Str, Arr, Obj } k = Null;
    long long n = 0;
    std::string s;
    std::vector<J> a;
    std::vector<std::pair<std::string, J>> o;
    const J* get(const std::string& key) const
    {
        for (auto& kv : o)
            if (kv.first == key) return &kv.second;
        return nullptr;
    }
};

struct P {
    const std::string& t;
    size_t i = 0;
    void ws() { while (i < t.size() && std::isspace(static_cast<unsigned char>(t[i]))) ++i; }
    J parse()
    {
        ws();
        J v;
        if (t[i] == '{') {
            v.k = J::Obj;
            ++i;
            ws();
            if (t[i] == '}') { ++i; return v; }
            for (;;) {
                ws();
                std::string key = parse().s;
                ws();
                ++i;  // ':'
                v.o.emplace_back(key, parse());
                ws();
                if (t[i++] == '}') break;
            }
        } else if (t[i] == '[') {
            v.k = J::Arr;
            ++i;
            ws();
            if (t[i] == ']') { ++i; return v; }
            for (;;) {
                v.a.push_back(parse());
                ws();
                if (t[i++] == ']') break;
            }
        } else if (t[i] == '"') {
            v.k = J::Str;
            size_t j = t.find('"', i + 1);
            v.s = t.substr(i + 1, j - i - 1);
            i = j + 1;
        } else {
            v.k = J::Num;
            size_t j = i;
            while (j < t.size() && (std::isdigit(static_cast<unsigned char>(t[j])) || t[j] == '-')) ++j;
            v.n = std::stoll(t.substr(i, j - i));
            i = j;
        }
        return v;
    }
};

std::uint32_t hex_le_dword(const std::string& h)
{
    std::uint32_t v = 0;
    for (int b = 3; b >= 0; --b) v = (v << 8) | static_cast<std::uint32_t>(std::stoul(h.substr(b * 2, 2), nullptr, 16));
    return v;
}

std::vector<std::uint8_t> hex_bytes(const std::string& h)
{
    std::vector<std::uint8_t> out;
    for (size_t i = 0; i + 1 < h.size(); i += 2) out.push_back(static_cast<std::uint8_t>(std::stoul(h.substr(i, 2), nullptr, 16)));
    return out;
}

}  // namespace

std::vector<LockstepFrame> LoadLockstep(const std::string& path)
{
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open lockstep stream " + path);
    std::vector<LockstepFrame> out;
    std::string line;
    while (std::getline(f, line)) {
        if (line.find('{') == std::string::npos) continue;
        P p{line};
        J j = p.parse();
        LockstepFrame fr;
        if (auto* v = j.get("frame")) fr.frame = static_cast<int>(v->n);
        if (auto* v = j.get("timer"))
            for (auto& e : v->a) fr.timer.push_back(static_cast<std::uint32_t>(e.n));
        if (auto* v = j.get("kbd"))
            for (auto& c : v->a) {
                ReplayKeyboard::Call call{static_cast<std::int32_t>(c.get("hr")->n), {}};
                for (auto& ev : c.get("events")->a)
                    call.events.push_back({hex_le_dword(ev.a[0].s), hex_le_dword(ev.a[1].s), hex_le_dword(ev.a[2].s), hex_le_dword(ev.a[3].s)});
                fr.kbd.push_back(call);
            }
        for (auto [name, dst] : {std::pair{"mouse", &fr.mouse}, std::pair{"joy", &fr.joy}})
            if (auto* v = j.get(name))
                for (auto& c : v->a) dst->push_back({static_cast<std::int32_t>(c.get("hr")->n), hex_bytes(c.get("state")->s)});
        if (auto* v = j.get("check"))
            for (auto& kv : v->o) fr.check[kv.first] = kv.second.s;
        out.push_back(std::move(fr));
    }
    return out;
}

void Feed(const LockstepFrame& f, ReplayTimer& t, ReplayKeyboard& k, ReplayStateDevice& m, ReplayStateDevice& j)
{
    for (auto v : f.timer) t.ticks.push_back(v);
    for (auto& c : f.kbd) k.calls.push_back(c);
    for (auto& c : f.mouse) m.calls.push_back(c);
    for (auto& c : f.joy) j.calls.push_back(c);
}

std::vector<std::string> Compare(const LockstepFrame& f, const std::map<std::string, Probe>& probes)
{
    std::vector<std::string> bad;
    for (auto& [key, want] : f.check) {
        auto it = probes.find(key);
        if (it == probes.end() || it->second() != want) bad.push_back(key);
    }
    return bad;
}

}  // namespace recoil::platform
