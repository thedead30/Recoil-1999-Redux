// mapgen.h - Setup's RX-47 mission maps (user 2026-10-20: the M map redone with the objectives, weapons, tank upgrades
// and secrets). Made from the player's own installed copy into <dest>\redux_map\m<N>.png + m<N>.txt, which Redux draws
// while the game's map is open ([Display] Map=1). Not Recoil code (PLATFORM).
#pragma once

#include <functional>
#include <string>

namespace setup {

// maps 1-6 from <dest>\zbd, <dest>\maps and <dest>\messages.dll; returns how many were made
int GenerateMaps(const std::wstring& dest, const std::function<void(const std::string&)>& log);

}  // namespace setup
