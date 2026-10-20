// Tooling test for the L4 lockstep support (synthetic stream; no game code yet).
#include "test.h"
#include "platform/lockstep.h"

#include <cstdio>
#include <fstream>

TEST(lockstep_load_feed_compare_synthetic)
{
    const char* path = "lockstep_test_stream.jsonl";
    {
        std::ofstream f(path);
        f << R"({"frame": 0, "timer": [1000], "kbd": [{"hr": 0, "events": [["1e000000", "80000000", "40e20100", "07000000"]]}], "mouse": [{"hr": 0, "state": "0100000002000000000000000080"}], "joy": [], "check": {"dt": "0ad7233c"}})" "\n";
        f << R"({"frame": 1, "timer": [1016, 1017], "kbd": [], "mouse": [], "joy": [], "check": {"dt": "8fc2753c"}})" "\n";
    }
    auto frames = recoil::platform::LoadLockstep(path);
    CHECK_EQ(frames.size(), 2u);
    CHECK_EQ(frames[1].timer.size(), 2u);

    recoil::platform::ReplayTimer t;
    recoil::platform::ReplayKeyboard k;
    recoil::platform::ReplayStateDevice m("mouse"), j("joystick");
    recoil::platform::Feed(frames[0], t, k, m, j);
    CHECK_EQ(t.TickCount(), 1000u);
    std::vector<recoil::platform::KeyEvent> ev;
    CHECK_EQ(k.GetDeviceData(ev, 0x80), 0);
    CHECK_EQ(ev.size(), 1u);
    CHECK_EQ(ev[0].ofs, 0x1eu);         // scan code from little-endian hex
    CHECK_EQ(ev[0].data, 0x80u);
    unsigned char ms[0x10] = {};
    CHECK_EQ(m.GetDeviceState(ms, sizeof ms), 0);
    CHECK_EQ(ms[0], 1);
    CHECK_EQ(ms[4], 2);

    bool threw = false;
    try { t.TickCount(); } catch (const recoil::platform::ReplayExhausted& e) { threw = (e.seam == "timer"); }
    CHECK(threw);  // calling a seam more often than recorded is a desync

    std::map<std::string, recoil::platform::Probe> probes{{"dt", [] { return std::string("0ad7233c"); }}};
    CHECK_EQ(recoil::platform::Compare(frames[0], probes).size(), 0u);
    CHECK_EQ(recoil::platform::Compare(frames[1], probes).size(), 1u);
    std::remove(path);
}
