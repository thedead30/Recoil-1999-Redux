// SUBSYSTEM: platform
// L4 lockstep support (tools/lockstep/FORMAT.md): load a recorded stream into the replay seams, and
// compare the remake's per-frame state samples against the original's.
#pragma once

#include "platform/seams.h"

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace recoil::platform {

// One frame of a stream: the seam values in call order, and the original's state samples.
struct LockstepFrame {
    int frame = 0;
    std::vector<std::uint32_t> timer;
    std::vector<ReplayKeyboard::Call> kbd;
    std::vector<ReplayStateDevice::Call> mouse, joy;
    std::map<std::string, std::string> check;  // key -> hex of the original's bytes
};

// Parses a JSON-lines stream in the FORMAT.md layout.
std::vector<LockstepFrame> LoadLockstep(const std::string& path);

// Queues one frame's seam values into the replay devices.
void Feed(const LockstepFrame& f, ReplayTimer& t, ReplayKeyboard& k, ReplayStateDevice& m, ReplayStateDevice& j);

// State probes: check key -> function returning the remake's current bytes as hex (layout-faithful
// structs make this a straight memory read). Returns the keys that differ.
using Probe = std::function<std::string()>;
std::vector<std::string> Compare(const LockstepFrame& f, const std::map<std::string, Probe>& probes);

}  // namespace recoil::platform
