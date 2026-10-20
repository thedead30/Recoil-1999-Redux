// SUBSYSTEM: platform
// No-CD mode (KG-45, a documented deviation, not original behaviour): the original insists on its disc - CDCheck_FindDiscDrive
// 0x004a59e0 looks for a CD-ROM drive holding video\intro_01.avi and later reads the movies from it. In no-CD mode the game
// directory is presented as that drive, through the two import slots the finder uses (GetLogicalDriveStringsA 0x004cc164,
// GetDriveTypeA 0x004cc160); the ported code is unchanged. PLATFORM.
#pragma once

namespace recoil {

// Rebind the two slots so that `dir` (ending in a backslash) is listed as an extra drive root and reported as DRIVE_CDROM.
void Platform_EnableNoCd(const char* dir);

}  // namespace recoil
