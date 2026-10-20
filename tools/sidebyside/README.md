# Side-by-side testing (KG-60)

`Desktop\Recoil Side by Side.bat` starts the original on the left screen and the remake on the main screen. Both run in a window and go straight into mission 1. Both read the same keyboard and mouse.

- Press Space or Enter once to leave the mission briefing. Both games get the key.
- Press Scroll Lock to save a memory snapshot and a screenshot of each game into `Recoil Final Attempt\01_evidence\snapshots`.

| part | where | what |
|---|---|---|
| `sidebyside.ps1` | here | starts both games, places the windows, opens the snapshot listener |
| `dinput.dll` (`rcl_dinput.cpp`, `build.bat`) | copied into `Desktop\Recoil Original Windowed` | wrapper inside the original: shared background input, direct start of mission 1, start-up clock, second-instance bypass |
| remake switches | `05_remake/boot/boot_main.cpp` | `RECOIL_SHARED_INPUT=1`: shared background input and second-instance bypass |
| windowed dgVoodoo | `Desktop\Recoil dgVoodoo Windowed`, `Desktop\Recoil Original Windowed` | `OutputAPI = bestavailable` (the runtime copy's `direct3d11` is invalid, KG-59) |

The games start the same way and get the same input, but they are not locked frame by frame, so they drift apart over time.
