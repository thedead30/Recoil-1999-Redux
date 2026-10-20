// source.h - the player Setup's reader for the player's own copy of Recoil (C++ port of tools/recoil_source.py, which stays
// the reference: tools/setup/compare_setup.py checks both give the same files). Reads an installed folder, a zip of one, a CD
// folder (InstallShield 5 data1.cab), .iso, raw .bin (Mode 1/2, with or without .cue), .cue and .nrg. Not Recoil code (PLATFORM).
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace setup {

// random-access bytes (a file, a data track's 2048-byte sectors, a slice, or memory)
struct Blob {
    virtual ~Blob() = default;
    virtual std::uint64_t Size() const = 0;
    virtual bool Read(std::uint64_t off, void* buf, std::size_t n) const = 0;
    bool ReadAll(std::vector<std::uint8_t>& out) const;
};

struct AudioTrack {
    int number;
    std::wstring path;
    std::uint64_t start, length;   // bytes of 2352-byte CD-DA sectors
    int Seconds() const { return int(length / 2352 / 75); }
    bool SaveWav(const std::wstring& out) const;
};

struct GameFile {
    std::string rel;   // path inside the game, '/' separated, as the copy names it
    int index;         // reader-specific
};

class Source {
public:
    // false + why on failure; on success Kind() / GameKind() / Files() / Audio() describe the copy
    bool Open(const std::wstring& path, std::string& why);
    const std::string& Kind() const { return kind_; }
    const std::string& GameKind() const { return game_kind_; }
    bool HasGame() const { return !game_.empty(); }
    const std::vector<GameFile>& Files() const { return game_; }
    const std::vector<AudioTrack>& Audio() const { return audio_; }
    bool ReadGameFile(const GameFile& f, std::vector<std::uint8_t>& out, std::string& why) const;
    // every file of the copy itself (also used to read the dgVoodoo zip)
    std::size_t EntryCount() const { return entries_.size(); }
    const std::string& EntryPath(std::size_t i) const { return entries_[i].path; }
    bool ReadEntry(std::size_t i, std::vector<std::uint8_t>& out) const;
    Source();
    ~Source();

private:
    struct Entry {
        std::string path;   // as stored, '/' separated
        std::uint64_t a = 0, b = 0, c = 0;
        int method = 0;
    };
    bool OpenAny(const std::wstring& path, std::string& why);
    bool OpenIso(std::unique_ptr<Blob> track, std::string& why);
    std::unique_ptr<Blob> OpenEntry(std::size_t i) const;
    void FindGame();

    enum class Kinds { Folder, Zip, Iso } fs_ = Kinds::Folder;
    std::wstring root_;
    std::string kind_, game_kind_;
    std::vector<Entry> entries_;
    std::unique_ptr<Blob> image_;     // zip file or data track
    std::shared_ptr<Blob> zip_file_;
    std::vector<AudioTrack> audio_;
    std::vector<GameFile> game_;
    std::string tree_root_;           // installed tree: prefix inside entries
    struct Cab;
    std::unique_ptr<Cab> cab_;
};

// SHA-256 of bytes as lowercase hex (Windows CNG)
std::string Sha256Hex(const std::vector<std::uint8_t>& data);
struct ExeBuild {
    std::string name;
    bool supported;
};
ExeBuild IdentifyExe(const std::vector<std::uint8_t>& exe);
extern const char* const kSupportedNote;

std::wstring Widen(const std::string& s);   // UTF-8 / ASCII
std::string Narrow(const std::wstring& s);  // UTF-8
std::string Lower(std::string s);

}  // namespace setup
