// SUBSYSTEM: platform
// Layer-D seams (tools/lockstep/FORMAT.md): the only points where the outside world enters the game.
// The live implementations (Win32 GetTickCount, DirectInput devices) and the replay implementations
// (a recorded lockstep stream) share these interfaces, so the translated game code cannot tell which
// one it is talking to. Values are PLATFORM.
#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace recoil::platform {

// Seam at 0x004a56d3 (frame timer 0x004a56d0 -> GetTickCount).
struct ITimer {
    virtual ~ITimer() = default;
    virtual std::uint32_t TickCount() = 0;
};

// Seam at 0x0046f6c0 (IDirectInputDevice::GetDeviceData, keyboard). One element = DIDEVICEOBJECTDATA.
struct KeyEvent {
    std::uint32_t ofs, data, timestamp, sequence;
};
struct IKeyboardDevice {
    virtual ~IKeyboardDevice() = default;
    virtual std::int32_t GetDeviceData(std::vector<KeyEvent>& out, std::uint32_t max) = 0;  // HRESULT
};

// Seams at 0x00470419 (mouse, 0x10 bytes) and 0x004722ef (joystick, 0x110 bytes): GetDeviceState.
struct IStateDevice {
    virtual ~IStateDevice() = default;
    virtual std::int32_t GetDeviceState(void* buf, std::uint32_t size) = 0;  // HRESULT
};

// Raised when a replayed seam is called more often than the recording (a desync by itself).
struct ReplayExhausted {
    std::string seam;
};

// ---------------------------------------------------------------- replay implementations
class ReplayTimer : public ITimer {
public:
    std::deque<std::uint32_t> ticks;
    std::uint32_t TickCount() override
    {
        if (ticks.empty()) throw ReplayExhausted{"timer"};
        std::uint32_t t = ticks.front();
        ticks.pop_front();
        return t;
    }
};

class ReplayKeyboard : public IKeyboardDevice {
public:
    struct Call {
        std::int32_t hr;
        std::vector<KeyEvent> events;
    };
    std::deque<Call> calls;
    std::int32_t GetDeviceData(std::vector<KeyEvent>& out, std::uint32_t max) override
    {
        if (calls.empty()) throw ReplayExhausted{"keyboard"};
        Call c = std::move(calls.front());
        calls.pop_front();
        out = std::move(c.events);
        if (out.size() > max) out.resize(max);
        return c.hr;
    }
};

class ReplayStateDevice : public IStateDevice {
public:
    explicit ReplayStateDevice(std::string name) : name_(std::move(name)) {}
    struct Call {
        std::int32_t hr;
        std::vector<std::uint8_t> state;
    };
    std::deque<Call> calls;
    std::int32_t GetDeviceState(void* buf, std::uint32_t size) override
    {
        if (calls.empty()) throw ReplayExhausted{name_};
        Call c = std::move(calls.front());
        calls.pop_front();
        auto* b = static_cast<std::uint8_t*>(buf);
        for (std::uint32_t i = 0; i < size; ++i) b[i] = i < c.state.size() ? c.state[i] : 0;
        return c.hr;
    }

private:
    std::string name_;
};

}  // namespace recoil::platform
