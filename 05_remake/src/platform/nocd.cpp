// SUBSYSTEM: platform
// No-CD mode (KG-45): see nocd.h. PLATFORM - every value here is a Win32 API value, none is from Recoil.
#include "platform/nocd.h"
#include "platform/iat_kernel32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstring>

namespace recoil {
namespace {
char g_dir[MAX_PATH];  // the game directory, with a trailing backslash
using DriveStringsFn = DWORD(WINAPI*)(DWORD, LPSTR);
using DriveTypeFn = UINT(WINAPI*)(LPCSTR);
DriveStringsFn g_real_strings;
DriveTypeFn g_real_type;

// the real drive list with the game directory appended as one more root (same double-NUL-terminated format)
DWORD WINAPI NoCd_GetLogicalDriveStringsA(DWORD size, LPSTR buf)
{
    const DWORD n = g_real_strings(size, buf);
    const DWORD extra = static_cast<DWORD>(std::strlen(g_dir)) + 1;
    if (n == 0 || n + extra + 1 > size) return n;
    std::memcpy(buf + n, g_dir, extra);  // after the last root's NUL
    buf[n + extra] = '\0';
    return n + extra;
}

// the game directory is a CD-ROM; everything else as Windows says
UINT WINAPI NoCd_GetDriveTypeA(LPCSTR root)
{
    if (root && _strnicmp(root, g_dir, std::strlen(g_dir)) == 0) return DRIVE_CDROM;
    return g_real_type(root);
}
}  // namespace

void Platform_EnableNoCd(const char* dir)
{
    std::strncpy(g_dir, dir, MAX_PATH - 2);
    const size_t len = std::strlen(g_dir);
    if (len && g_dir[len - 1] != '\\' && g_dir[len - 1] != '/') { g_dir[len] = '\\'; g_dir[len + 1] = '\0'; }
    for (char* p = g_dir; *p; ++p)
        if (*p == '/') *p = '\\';
    g_real_strings = reinterpret_cast<DriveStringsFn>(g_Iat_GetLogicalDriveStringsA_004cc164);
    g_real_type = reinterpret_cast<DriveTypeFn>(g_Iat_GetDriveTypeA_004cc160);
    g_Iat_GetLogicalDriveStringsA_004cc164 = reinterpret_cast<void*>(&NoCd_GetLogicalDriveStringsA);
    g_Iat_GetDriveTypeA_004cc160 = reinterpret_cast<void*>(&NoCd_GetDriveTypeA);
}

}  // namespace recoil
