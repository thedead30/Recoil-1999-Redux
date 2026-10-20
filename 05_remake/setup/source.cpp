// source.cpp - reader for the player's own copy of Recoil (port of tools/recoil_source.py; that file is the reference).
// Format notes: ISO 9660 (ECMA-119), raw CD sectors (2352 bytes, sync + header), cue sheets, Nero NER5/NERO footers, zip
// (PKWARE APPNOTE), InstallShield 5 cabinets (after the open-source unshield project's notes). PLATFORM, not Recoil code.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>

#include "source.h"
#include "inflate.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <map>

namespace setup {

// ---------------------------------------------------------------- strings
std::wstring Widen(const std::string& s)
{
    if (s.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), w.data(), n);
    return w;
}

std::string Narrow(const std::wstring& w)
{
    if (w.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), int(w.size()), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), int(w.size()), s.data(), n, nullptr, nullptr);
    return s;
}

std::string Lower(std::string s)
{
    for (auto& c : s)
        if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
    return s;
}

static std::string Latin1ToUtf8(const char* p, std::size_t n)
{
    std::wstring w;
    for (std::size_t i = 0; i < n; ++i) w.push_back(wchar_t(std::uint8_t(p[i])));
    return Narrow(w);
}

static std::uint32_t Le32(const std::uint8_t* p) { return p[0] | p[1] << 8 | p[2] << 16 | std::uint32_t(p[3]) << 24; }
static std::uint16_t Le16(const std::uint8_t* p) { return std::uint16_t(p[0] | p[1] << 8); }
static std::uint32_t Be32(const std::uint8_t* p) { return std::uint32_t(p[0]) << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static std::uint16_t Be16(const std::uint8_t* p) { return std::uint16_t(p[0] << 8 | p[1]); }
static std::uint64_t Be64(const std::uint8_t* p) { return std::uint64_t(Be32(p)) << 32 | Be32(p + 4); }

// ---------------------------------------------------------------- blobs
bool Blob::ReadAll(std::vector<std::uint8_t>& out) const
{
    out.resize(std::size_t(Size()));
    return out.empty() || Read(0, out.data(), out.size());
}

namespace {

class FileBlob : public Blob {
public:
    explicit FileBlob(const std::wstring& p)
    {
        h_ = CreateFileW(p.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);
        LARGE_INTEGER sz{};
        if (h_ != INVALID_HANDLE_VALUE && GetFileSizeEx(h_, &sz)) size_ = std::uint64_t(sz.QuadPart);
    }
    ~FileBlob() override
    {
        if (h_ != INVALID_HANDLE_VALUE) CloseHandle(h_);
    }
    bool Ok() const { return h_ != INVALID_HANDLE_VALUE; }
    std::uint64_t Size() const override { return size_; }
    bool Read(std::uint64_t off, void* buf, std::size_t n) const override
    {
        if (off + n > size_) return false;
        auto* p = static_cast<std::uint8_t*>(buf);
        while (n) {
            OVERLAPPED ov{};
            ov.Offset = DWORD(off);
            ov.OffsetHigh = DWORD(off >> 32);
            const DWORD want = DWORD(std::min<std::size_t>(n, 1u << 24));
            DWORD got = 0;
            if (!ReadFile(h_, p, want, &got, &ov) || got == 0) return false;
            p += got;
            off += got;
            n -= got;
        }
        return true;
    }

private:
    HANDLE h_ = INVALID_HANDLE_VALUE;
    std::uint64_t size_ = 0;
};

// the 2048-byte user data of a data track's sectors, as one byte stream
class SectorBlob : public Blob {
public:
    SectorBlob(std::shared_ptr<Blob> f, std::uint64_t start, unsigned ss, unsigned doff)
        : f_(std::move(f)), start_(start), ss_(ss), doff_(doff)
    {
        n_ = f_->Size() > start_ ? (f_->Size() - start_) / ss_ : 0;
    }
    std::uint64_t Size() const override { return n_ * 2048; }
    bool Read(std::uint64_t off, void* buf, std::size_t n) const override
    {
        auto* p = static_cast<std::uint8_t*>(buf);
        while (n) {
            const std::uint64_t sec = off / 2048;
            const unsigned in = unsigned(off % 2048);
            const std::size_t k = std::min<std::size_t>(n, 2048 - in);
            if (sec >= n_) {   // past the end: zeros, as a short read in the Python reader
                std::memset(p, 0, k);
            } else if (ss_ == 2048) {   // contiguous
                const std::size_t run = std::size_t(std::min<std::uint64_t>(n, (n_ - sec) * 2048 - in));
                if (!f_->Read(start_ + off, p, run)) return false;
                p += run;
                off += run;
                n -= run;
                continue;
            } else if (!f_->Read(start_ + sec * ss_ + doff_ + in, p, k)) {
                return false;
            }
            p += k;
            off += k;
            n -= k;
        }
        return true;
    }

private:
    std::shared_ptr<Blob> f_;
    std::uint64_t start_, n_;
    unsigned ss_, doff_;
};

class SubBlob : public Blob {
public:
    SubBlob(const Blob* b, std::uint64_t off, std::uint64_t size) : b_(b), off_(off), size_(size) {}
    std::uint64_t Size() const override { return size_; }
    bool Read(std::uint64_t off, void* buf, std::size_t n) const override
    {
        return off + n <= size_ && b_->Read(off_ + off, buf, n);
    }

private:
    const Blob* b_;
    std::uint64_t off_, size_;
};

class MemBlob : public Blob {
public:
    explicit MemBlob(std::vector<std::uint8_t> d) : d_(std::move(d)) {}
    std::uint64_t Size() const override { return d_.size(); }
    bool Read(std::uint64_t off, void* buf, std::size_t n) const override
    {
        if (off + n > d_.size()) return false;
        std::memcpy(buf, d_.data() + off, n);
        return true;
    }

private:
    std::vector<std::uint8_t> d_;
};

const std::uint8_t kSync[12] = {0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0};   // CD sector sync (ECMA-130)

bool EndsWith(const std::string& s, const char* t)
{
    const std::size_t n = std::strlen(t);
    return s.size() >= n && s.compare(s.size() - n, n, t) == 0;
}

// raw 2352 image: data track mode + where its data sectors end
bool RawBinLayout(const FileBlob& f, int& mode, std::uint64_t& end, std::uint64_t& total)
{
    std::uint8_t h[16];
    if (f.Size() < 16 || !f.Read(0, h, 16) || std::memcmp(h, kSync, 12) != 0) return false;
    mode = h[15];
    total = f.Size() / 2352;
    auto is_data = [&](std::uint64_t s) {
        std::uint8_t b[12];
        return f.Read(s * 2352, b, 12) && std::memcmp(b, kSync, 12) == 0;
    };
    end = total;
    if (!is_data(total - 1)) {
        std::uint64_t lo = 0, hi = total - 1;
        while (lo + 1 < hi) {
            const std::uint64_t m = (lo + hi) / 2;
            if (is_data(m)) lo = m;
            else hi = m;
        }
        end = hi;
    }
    return true;
}

// CD audio without a cue sheet: tracks separated by digital silence (2 s pregap = 150 sectors of zeros)
std::vector<std::pair<std::uint64_t, std::uint64_t>> SplitAudioBySilence(const FileBlob& f, std::uint64_t start, std::uint64_t end_bytes)
{
    const std::uint64_t min_gap = 150;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> tracks;
    const std::uint64_t n_sec = (end_bytes - start) / 2352;
    std::uint64_t zero_run = 0, cur = start;
    std::vector<std::uint8_t> b(2352);
    for (std::uint64_t k = 0; k < n_sec; ++k) {
        f.Read(start + k * 2352, b.data(), 2352);
        if (std::all_of(b.begin(), b.end(), [](std::uint8_t x) { return x == 0; })) {
            ++zero_run;
        } else {
            if (zero_run >= min_gap && k - zero_run > (cur - start) / 2352) {
                tracks.push_back({cur, start + (k - zero_run) * 2352});
                cur = start + k * 2352;
            } else if (tracks.empty() && zero_run && cur == start) {
                cur = start + k * 2352;   // leading pregap
            }
            zero_run = 0;
        }
    }
    tracks.push_back({cur, start + (n_sec - zero_run) * 2352});
    std::vector<std::pair<std::uint64_t, std::uint64_t>> out;
    for (auto& t : tracks)
        if (t.second > t.first && t.second - t.first > 2352 * 75) out.push_back(t);   // drop slivers under 1 s
    return out;
}

struct CueTrack {
    int n;
    std::string type;
    std::wstring file;
    long long index1 = -1;
};

std::vector<CueTrack> ParseCue(const std::wstring& cue)
{
    std::vector<CueTrack> out;
    std::ifstream in(cue, std::ios::binary);
    std::wstring base = cue.substr(0, cue.find_last_of(L"\\/") + 1);
    std::wstring cur;
    std::string line;
    while (std::getline(in, line)) {
        std::vector<std::string> t;
        std::size_t i = 0;
        while (i < line.size()) {
            while (i < line.size() && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i;
            std::size_t j = i;
            while (j < line.size() && line[j] != ' ' && line[j] != '\t' && line[j] != '\r') ++j;
            if (j > i) t.push_back(line.substr(i, j - i));
            i = j;
        }
        if (t.empty()) continue;
        std::string k0 = t[0];
        std::transform(k0.begin(), k0.end(), k0.begin(), ::toupper);
        if (k0 == "FILE" && t.size() > 1) {
            std::string name = t[1];
            const std::size_t q = line.find('"');
            if (q != std::string::npos) name = line.substr(q + 1, line.find('"', q + 1) - q - 1);
            cur = base + Widen(Latin1ToUtf8(name.data(), name.size()));
        } else if (k0 == "TRACK" && t.size() > 2) {
            std::string ty = t[2];
            std::transform(ty.begin(), ty.end(), ty.begin(), ::toupper);
            out.push_back({std::atoi(t[1].c_str()), ty, cur});
        } else if (k0 == "INDEX" && t.size() > 2 && t[1] == "01" && !out.empty()) {
            int mm = 0, ss = 0, ff = 0;
            std::sscanf(t[2].c_str(), "%d:%d:%d", &mm, &ss, &ff);
            out.back().index1 = (mm * 60LL + ss) * 75 + ff;
        }
    }
    return out;
}

struct NrgTrack {
    int n;
    unsigned sector, mode;
    std::uint64_t start, end;
};

bool NrgTracks(const FileBlob& f, std::vector<NrgTrack>& out)
{
    if (f.Size() < 12) return false;
    std::uint8_t tail[12];
    f.Read(f.Size() - 12, tail, 12);
    std::uint64_t off;
    if (std::memcmp(tail, "NER5", 4) == 0) {
        off = Be64(tail + 4);
    } else if (std::memcmp(tail + 4, "NERO", 4) == 0) {
        off = Be32(tail + 8);
    } else {
        return false;
    }
    if (off >= f.Size()) return false;
    std::vector<std::uint8_t> d(std::size_t(std::min<std::uint64_t>(f.Size() - off, 1 << 20)));
    f.Read(off, d.data(), d.size());
    std::size_t k = 0;
    while (k + 8 <= d.size()) {
        const std::uint8_t* c = d.data() + k;
        const std::uint32_t ln = Be32(c + 4);
        if (k + 8 + ln > d.size()) break;
        const std::uint8_t* body = c + 8;
        const bool daox = std::memcmp(c, "DAOX", 4) == 0;
        if (daox || std::memcmp(c, "DAOI", 4) == 0) {
            const int first = body[20], last = body[21];
            const int esz = daox ? 42 : 30;
            for (int t = 0; t <= last - first; ++t) {
                const std::uint8_t* e = body + 22 + esz * t;
                if (e + esz > body + ln) break;
                NrgTrack tr{first + t, Be16(e + 12), Be16(e + 14)};
                tr.start = daox ? Be64(e + 26) : Be32(e + 22);
                tr.end = daox ? Be64(e + 34) : Be32(e + 26);
                out.push_back(tr);
            }
        }
        if (std::memcmp(c, "END!", 4) == 0) break;
        k += 8 + ln;
    }
    return !out.empty();
}

void ListFolder(const std::wstring& root, const std::wstring& rel, std::vector<std::string>& out)
{
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((root + rel + L"*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::wstring n = fd.cFileName;
        if (n == L"." || n == L"..") continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ListFolder(root, rel + n + L"\\", out);
        else {
            std::string p = Narrow(rel + n);
            std::replace(p.begin(), p.end(), '\\', '/');
            out.push_back(p);
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

}  // namespace

// ---------------------------------------------------------------- InstallShield 5 cabinet
struct Source::Cab {
    std::unique_ptr<Blob> blob;
    struct File {
        std::string path;
        unsigned flags;
        std::uint32_t exp, comp, doff;
    };
    std::vector<File> files;

    bool Open(std::unique_ptr<Blob> b, std::string& why)
    {
        blob = std::move(b);
        std::uint8_t h[20];
        if (blob->Size() < 20 || !blob->Read(0, h, 20) || std::memcmp(h, "ISc(", 4) != 0) {
            why = "data1.cab is not an InstallShield cabinet";
            return false;
        }
        const std::uint32_t cdo = Le32(h + 12), cds = Le32(h + 16);
        // the descriptor (file table included) - read generously, bounded by the file
        const std::uint64_t want = std::min<std::uint64_t>(blob->Size() - cdo, std::max<std::uint64_t>(cds, 1u << 22));
        std::vector<std::uint8_t> d(static_cast<std::size_t>(want));
        if (cdo >= blob->Size() || !blob->Read(cdo, d.data(), d.size())) {
            why = "data1.cab: unreadable descriptor";
            return false;
        }
        auto at = [&](std::size_t o) -> const std::uint8_t* { return o + 4 <= d.size() ? d.data() + o : nullptr; };
        if (!at(0x2c)) { why = "data1.cab: short descriptor"; return false; }
        const std::size_t ft = Le32(at(0x0c));
        const std::uint32_t ndirs = Le32(at(0x1c)), nfiles = Le32(at(0x28));
        if (ft + 4ull * (ndirs + nfiles) > d.size()) { why = "data1.cab: file table out of range"; return false; }
        auto cstr = [&](std::size_t o) {
            std::size_t p = ft + o, e = p;
            while (e < d.size() && d[e]) ++e;
            return p < d.size() ? Latin1ToUtf8(reinterpret_cast<const char*>(d.data() + p), e - p) : std::string();
        };
        std::vector<std::string> dirs;
        for (std::uint32_t i = 0; i < ndirs; ++i) dirs.push_back(cstr(Le32(d.data() + ft + 4 * i)));
        for (std::uint32_t i = 0; i < nfiles; ++i) {
            const std::size_t p = ft + Le32(d.data() + ft + 4 * (ndirs + i));
            if (p + 42 > d.size()) continue;
            const std::uint8_t* e = d.data() + p;
            const std::uint32_t name_off = Le32(e), dir_idx = Le32(e + 4);
            const unsigned flags = Le16(e + 8);
            if (flags & 8) continue;   // invalid entry
            File f{"", flags, Le32(e + 10), Le32(e + 14), Le32(e + 38)};
            const std::string dir = dir_idx < dirs.size() ? dirs[dir_idx] : std::string();
            f.path = (dir.empty() ? "" : dir + "/") + cstr(name_off);
            std::replace(f.path.begin(), f.path.end(), '\\', '/');
            files.push_back(f);
        }
        return true;
    }

    bool Read(const File& f, std::vector<std::uint8_t>& out, std::string& why) const
    {
        if (f.flags & 2) { why = "obfuscated cabinet file (not supported): " + f.path; return false; }
        std::vector<std::uint8_t> raw(f.comp);
        if (!raw.empty() && !blob->Read(f.doff, raw.data(), raw.size())) { why = "cannot read " + f.path; return false; }
        out.clear();
        if (!(f.flags & 4)) {
            raw.resize(std::min<std::size_t>(raw.size(), f.exp));
            out = std::move(raw);
            return true;
        }
        std::size_t k = 0;
        while (k + 2 <= raw.size() && out.size() < f.exp) {   // chunks: uint16 length + raw deflate data
            const std::size_t n = Le16(raw.data() + k);
            k += 2;
            if (k + n > raw.size() || !Inflate(raw.data() + k, n, out)) { why = "corrupt compressed data in " + f.path; return false; }
            k += n;
        }
        if (out.size() < f.exp) { why = "short data in " + f.path; return false; }
        out.resize(f.exp);
        return true;
    }
};

Source::Source() = default;
Source::~Source() = default;

// ---------------------------------------------------------------- opening a copy
bool Source::Open(const std::wstring& path, std::string& why)
{
    if (!OpenAny(path, why)) return false;
    FindGame();
    return true;
}

bool Source::OpenIso(std::unique_ptr<Blob> track, std::string& why)
{
    image_ = std::move(track);
    fs_ = Kinds::Iso;
    std::uint8_t pvd[2048];
    if (image_->Size() < 17 * 2048 || !image_->Read(16 * 2048, pvd, 2048) || std::memcmp(pvd + 1, "CD001", 5) != 0) {
        why = "no ISO 9660 file system found (not a Recoil disc image?)";
        return false;
    }
    // walk the directory tree (iterative)
    struct Dir {
        std::uint32_t extent, size;
        std::string prefix;
    };
    std::vector<Dir> todo{{Le32(pvd + 156 + 2), Le32(pvd + 156 + 10), ""}};
    int guard = 0;
    while (!todo.empty() && ++guard < 100000) {
        Dir dir = todo.back();
        todo.pop_back();
        std::vector<std::uint8_t> data((std::size_t(dir.size) + 2047) / 2048 * 2048);
        if (!image_->Read(std::uint64_t(dir.extent) * 2048, data.data(), data.size())) continue;
        std::size_t k = 0;
        while (k < dir.size) {
            const std::uint8_t ln = data[k];
            if (ln == 0) {
                k = (k / 2048 + 1) * 2048;
                continue;
            }
            if (k + ln > data.size()) break;
            const std::uint8_t* r = data.data() + k;
            const unsigned flags = r[25], nl = r[32];
            if (!(nl == 1 && (r[33] == 0 || r[33] == 1))) {
                std::string nm = Latin1ToUtf8(reinterpret_cast<const char*>(r + 33), nl);
                nm = nm.substr(0, nm.find(';'));
                while (!nm.empty() && nm.back() == '.') nm.pop_back();
                const std::string p = dir.prefix + nm;
                if (flags & 2) todo.push_back({Le32(r + 2), Le32(r + 10), p + "/"});
                else {
                    Entry e;
                    e.path = p;
                    e.a = Le32(r + 2);
                    e.b = Le32(r + 10);
                    entries_.push_back(e);
                }
            }
            k += ln;
        }
    }
    return true;
}

bool Source::OpenAny(const std::wstring& path, std::string& why)
{
    root_ = path;
    const std::string low = Lower(Narrow(path));
    const DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) { why = "not found: " + Narrow(path); return false; }
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        if (root_.back() != L'\\' && root_.back() != L'/') root_ += L'\\';
        std::vector<std::string> files;
        ListFolder(root_, L"", files);
        for (auto& f : files) {
            Entry e;
            e.path = f;
            entries_.push_back(e);
        }
        fs_ = Kinds::Folder;
        kind_ = "folder";
        return true;
    }
    auto file = std::make_shared<FileBlob>(path);
    if (!file->Ok()) { why = "cannot open " + Narrow(path); return false; }
    std::uint8_t magic[4] = {};
    file->Read(0, magic, std::min<std::uint64_t>(4, file->Size()) == 4 ? 4 : 0);
    // zip: end-of-central-directory record in the last 64 KB
    if (std::memcmp(magic, "PK\x03\x04", 4) == 0 || EndsWith(low, ".zip")) {
        const std::size_t tail = std::size_t(std::min<std::uint64_t>(file->Size(), 65557));
        std::vector<std::uint8_t> t(tail);
        file->Read(file->Size() - tail, t.data(), tail);
        std::size_t eocd = std::string::npos;
        for (std::size_t i = tail >= 22 ? tail - 22 : 0; i != std::string::npos && i + 4 <= tail; --i) {
            if (std::memcmp(t.data() + i, "PK\x05\x06", 4) == 0) { eocd = i; break; }
            if (i == 0) break;
        }
        if (eocd != std::string::npos) {
            const std::uint32_t cd_size = Le32(t.data() + eocd + 12), cd_off = Le32(t.data() + eocd + 16);
            std::vector<std::uint8_t> cd(cd_size);
            if (!file->Read(cd_off, cd.data(), cd.size())) { why = "zip: unreadable central directory"; return false; }
            std::size_t k = 0;
            while (k + 46 <= cd.size() && std::memcmp(cd.data() + k, "PK\x01\x02", 4) == 0) {
                const std::uint8_t* c = cd.data() + k;
                const unsigned nl = Le16(c + 28), xl = Le16(c + 30), cl = Le16(c + 32);
                Entry e;
                e.method = Le16(c + 10);
                e.b = Le32(c + 20);
                e.c = Le32(c + 24);
                e.a = Le32(c + 42);
                // zip64 extra field (id 1): the 64-bit values of the fields stored as 0xFFFFFFFF, in the order
                // uncompressed size, compressed size, local header offset (PKWARE APPNOTE 4.5.3, PLATFORM). Python writes
                // it for entries past 2 GB, e.g. in a Redux asset pack.
                for (unsigned x = 0; x + 4 <= xl;) {
                    const std::uint8_t* f = c + 46 + nl + x;
                    const unsigned id = Le16(f), sz = Le16(f + 2);
                    if (id == 1) {
                        unsigned o = 4;
                        auto take = [&](std::uint64_t& v) {
                            if (v == 0xFFFFFFFFu && o + 8 <= 4 + sz) {
                                v = std::uint64_t(Le32(f + o)) | (std::uint64_t(Le32(f + o + 4)) << 32);
                                o += 8;
                            }
                        };
                        take(e.c);
                        take(e.b);
                        take(e.a);
                    }
                    x += 4 + sz;
                }
                const unsigned gp = Le16(c + 8);
                std::string name(reinterpret_cast<const char*>(c + 46), nl);
                if (!(gp & 0x800)) name = Latin1ToUtf8(name.data(), name.size());   // not UTF-8 flagged: cp437/latin-1
                e.path = name;
                if (!name.empty() && name.back() != '/') entries_.push_back(e);
                k += 46 + nl + xl + cl;
            }
            image_ = std::unique_ptr<Blob>(new SubBlob(file.get(), 0, file->Size()));
            zip_file_ = file;
            fs_ = Kinds::Zip;
            kind_ = "zip";
            return true;
        }
    }
    if (EndsWith(low, ".cue")) {
        auto tr = ParseCue(path);
        const CueTrack* data = nullptr;
        for (auto& t : tr)
            if (t.type != "AUDIO") { data = &t; break; }
        if (!data) { why = "the cue sheet has no data track"; return false; }
        const unsigned ss = data->type.find("2352") != std::string::npos ? 2352 : 2048;
        const unsigned doff = ss == 2048 ? 0 : (data->type.rfind("MODE2", 0) == 0 ? 24 : 16);
        auto df = std::make_shared<FileBlob>(data->file);
        if (!df->Ok()) { why = "cannot open " + Narrow(data->file); return false; }
        if (!OpenIso(std::make_unique<SectorBlob>(df, std::uint64_t(data->index1) * ss, ss, doff), why)) return false;
        for (std::size_t i = 0; i < tr.size(); ++i) {
            const CueTrack& t = tr[i];
            if (t.type != "AUDIO") continue;
            const CueTrack* nxt = i + 1 < tr.size() && tr[i + 1].file == t.file ? &tr[i + 1] : nullptr;
            const std::uint64_t st = std::uint64_t(t.index1) * 2352;
            std::uint64_t end;
            if (nxt) end = std::uint64_t(nxt->index1) * 2352;
            else {
                FileBlob af(t.file);
                end = af.Size();
            }
            audio_.push_back({t.n, t.file, st, end - st});
        }
        kind_ = "cue/bin";
        return true;
    }
    std::vector<NrgTrack> nt;
    if (EndsWith(low, ".nrg") && NrgTracks(*file, nt)) {
        const NrgTrack* d = nullptr;
        for (auto& t : nt)
            if (t.mode != 0x0700) { d = &t; break; }
        if (!d) { why = "the Nero image has no data track"; return false; }
        unsigned doff = d->sector == 2048 ? 0 : ((d->sector == 2336 || d->sector == 2352) && (d->mode == 2 || d->mode == 3) ? 24 : 16);
        if (!OpenIso(std::make_unique<SectorBlob>(file, d->start, d->sector, doff), why)) return false;
        for (auto& t : nt)
            if (t.mode == 0x0700) audio_.push_back({t.n, path, t.start, t.end - t.start});
        kind_ = "nrg";
        return true;
    }
    int mode;
    std::uint64_t end, total;
    if (RawBinLayout(*file, mode, end, total)) {
        if (!OpenIso(std::make_unique<SectorBlob>(file, 0, 2352, mode == 2 ? 24 : 16), why)) return false;
        std::wstring cue = path.substr(0, path.find_last_of(L'.')) + L".cue";
        if (GetFileAttributesW(cue.c_str()) != INVALID_FILE_ATTRIBUTES) {
            Source c;
            std::string w2;
            if (c.OpenAny(cue, w2)) audio_ = c.audio_;
        } else if (end < total) {
            int n = 2;
            for (auto& ab : SplitAudioBySilence(*file, end * 2352, total * 2352)) audio_.push_back({n++, path, ab.first, ab.second - ab.first});
        }
        kind_ = mode == 2 ? "bin (raw, Mode 2)" : "bin (raw, Mode 1)";
        return true;
    }
    if (!OpenIso(std::make_unique<SectorBlob>(file, 0, 2048, 0), why)) return false;   // plain .iso
    kind_ = "iso";
    return true;
}

std::unique_ptr<Blob> Source::OpenEntry(std::size_t i) const
{
    const Entry& e = entries_[i];
    if (fs_ == Kinds::Iso) return std::make_unique<SubBlob>(image_.get(), e.a * 2048, e.b);
    if (fs_ == Kinds::Folder) {
        std::wstring p = Widen(e.path);
        std::replace(p.begin(), p.end(), L'/', L'\\');
        auto f = std::make_unique<FileBlob>(root_ + p);
        return f->Ok() ? std::move(f) : nullptr;
    }
    std::vector<std::uint8_t> d;
    if (!ReadEntry(i, d)) return nullptr;
    return std::make_unique<MemBlob>(std::move(d));
}

bool Source::ReadEntry(std::size_t i, std::vector<std::uint8_t>& out) const
{
    const Entry& e = entries_[i];
    if (fs_ != Kinds::Zip) {
        auto b = OpenEntry(i);
        return b && b->ReadAll(out);
    }
    std::uint8_t lh[30];
    if (!image_->Read(e.a, lh, 30) || std::memcmp(lh, "PK\x03\x04", 4) != 0) return false;
    const std::uint64_t data = e.a + 30 + Le16(lh + 26) + Le16(lh + 28);
    std::vector<std::uint8_t> comp(static_cast<std::size_t>(e.b));
    if (!comp.empty() && !image_->Read(data, comp.data(), comp.size())) return false;
    if (e.method == 0) {
        out = std::move(comp);
        return true;
    }
    if (e.method != 8) return false;
    out.clear();
    out.reserve(std::size_t(e.c));
    return Inflate(comp.data(), comp.size(), out) && out.size() == e.c;
}

void Source::FindGame()
{
    std::map<std::string, std::size_t> low;
    for (std::size_t i = 0; i < entries_.size(); ++i) low[Lower(entries_[i].path)] = i;
    for (auto& [k, i] : low) {
        if (!EndsWith(k, "recoil.exe")) continue;
        const std::string root = k.substr(0, k.size() - 10);
        auto it = low.lower_bound(root + "zbd/");
        if (it != low.end() && it->first.rfind(root + "zbd/", 0) == 0) {
            tree_root_ = root;
            for (auto& [k2, j] : low)
                if (k2.rfind(root, 0) == 0) game_.push_back({entries_[j].path.substr(root.size()), int(j)});
            game_kind_ = "installed game files";
            return;
        }
    }
    for (auto& [k, i] : low) {
        if (!EndsWith(k, "data1.cab")) continue;
        auto cab = std::make_unique<Cab>();
        std::string why;
        auto b = OpenEntry(i);
        if (!b || !cab->Open(std::move(b), why)) continue;
        for (std::size_t j = 0; j < cab->files.size(); ++j) game_.push_back({cab->files[j].path, int(j)});
        cab_ = std::move(cab);
        game_kind_ = "InstallShield cabinet (CD installer)";
        return;
    }
}

bool Source::ReadGameFile(const GameFile& f, std::vector<std::uint8_t>& out, std::string& why) const
{
    if (cab_) return cab_->Read(cab_->files[std::size_t(f.index)], out, why);
    if (!ReadEntry(std::size_t(f.index), out)) {
        why = "cannot read " + f.rel;
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- audio
bool AudioTrack::SaveWav(const std::wstring& out) const
{
    FileBlob in(path);
    HANDLE h = CreateFileW(out.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (!in.Ok() || h == INVALID_HANDLE_VALUE) {
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        return false;
    }
    const std::uint64_t len = std::min<std::uint64_t>(length, in.Size() > start ? in.Size() - start : 0) & ~3ull;
    std::uint8_t hd[44];
    auto put32 = [&](int o, std::uint32_t v) { std::memcpy(hd + o, &v, 4); };
    auto put16 = [&](int o, std::uint16_t v) { std::memcpy(hd + o, &v, 2); };
    std::memcpy(hd, "RIFF", 4);
    put32(4, std::uint32_t(36 + len));
    std::memcpy(hd + 8, "WAVEfmt ", 8);
    put32(16, 16);
    put16(20, 1);         // PCM
    put16(22, 2);         // CD-DA: stereo
    put32(24, 44100);     // 44.1 kHz
    put32(28, 176400);
    put16(32, 4);
    put16(34, 16);
    std::memcpy(hd + 36, "data", 4);
    put32(40, std::uint32_t(len));
    DWORD w;
    bool ok = WriteFile(h, hd, 44, &w, nullptr) != 0;
    std::vector<std::uint8_t> buf(2352 * 75 * 10);
    for (std::uint64_t done = 0; ok && done < len;) {
        const std::size_t n = std::size_t(std::min<std::uint64_t>(buf.size(), len - done));
        ok = in.Read(start + done, buf.data(), n) && WriteFile(h, buf.data(), DWORD(n), &w, nullptr);
        done += n;
    }
    CloseHandle(h);
    return ok;
}

// ---------------------------------------------------------------- the exe fingerprint
std::string Sha256Hex(const std::vector<std::uint8_t>& data)
{
    unsigned char h[32] = {};
    BCRYPT_ALG_HANDLE alg = nullptr;
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) return {};
    BCryptHash(alg, nullptr, 0, const_cast<PUCHAR>(data.data()), ULONG(data.size()), h, sizeof h);
    BCryptCloseAlgorithmProvider(alg, 0);
    static const char* hex = "0123456789abcdef";
    std::string s;
    for (unsigned char c : h) {
        s += hex[c >> 4];
        s += hex[c & 15];
    }
    return s;
}

// CONFIRMED-DATA: SHA-256 of the known Recoil.exe builds (the same table as tools/recoil_source.py KNOWN_EXE)
ExeBuild IdentifyExe(const std::vector<std::uint8_t>& exe)
{
    const std::string h = Sha256Hex(exe);
    if (h == "22423e0bb090b6be6e10bca10a7c2d851da7e3708c30d27687369d40f93987d0")
        return {"Recoil 1999-01-29 build (Recoil Classic / patched)", true};
    if (h == "98f306c21ceab1cebdd492a93766118460293cf1da0f31f89aa030a281786c2f")
        return {"Recoil 1998-11-04 build (original 1998 CD)", false};
    if (h == "20eb9378af7be02af6e97689bde1703ba710bf2eb132c32254c8fc152158d50b")
        return {"Recoil 1998-11-04 build, modified copy (likely a no-CD patch)", false};
    return {"an unknown build of Recoil.exe", false};
}

const char* const kSupportedNote =
    "The remake only accepts the version it was made from: the 1999-01-29 build (as on the \"Recoil Classic\" CD). The "
    "1998-11-04 release differs in its program code and in its game data (35 data files differ and missions 10-13 are "
    "packaged differently), so it cannot be used.";

}  // namespace setup
