// SUBSYSTEM: platform
// Layer D (STAGE2.md section 3): host services that replace DirectDraw/Direct3D 5, DirectSound device
// calls, DirectInput devices, MCI CD audio, VFW decode and the MFC shell. Every value here is PLATFORM.
// Each service has a headless implementation that records the calls it receives, so the simulation
// can run without a window and its call sequence can be compared with the original's (L3/L4).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace recoil::platform {

// One recorded call into layer D: which service, which operation, and its arguments rendered as text.
struct RecordedCall {
    std::string service;
    std::string op;
    std::string args;
};

// Headless recorder shared by all stub services.
class CallRecorder {
public:
    void record(std::string service, std::string op, std::string args)
    {
        calls_.push_back({std::move(service), std::move(op), std::move(args)});
    }
    const std::vector<RecordedCall>& calls() const { return calls_; }
    void clear() { calls_.clear(); }

private:
    std::vector<RecordedCall> calls_;
};

}  // namespace recoil::platform
