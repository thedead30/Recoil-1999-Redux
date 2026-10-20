// inflate.h - raw DEFLATE decoder (RFC 1951) for the player Setup: zip members and InstallShield cabinet chunks.
// Not Recoil code (PLATFORM): a plain implementation of the public format.
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace setup {
// Appends the decoded bytes of one raw deflate stream (no zlib header) to out. Returns false on corrupt data.
bool Inflate(const std::uint8_t* in, std::size_t in_size, std::vector<std::uint8_t>& out);
}
