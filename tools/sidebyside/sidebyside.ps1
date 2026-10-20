# sidebyside.ps1 - original Recoil on the left screen, the remake on the main screen, both windowed, both straight into
# mission 1, both reading the same keyboard and mouse (shared background DirectInput). Testing tool (KG-60).
#   original: Desktop\Recoil Original Windowed (copy of the runtime folder + dgVoodoo windowed config + tools\sidebyside\dinput.dll)
#   remake  : latest build\Port\recoil_boot.exe, dgVoodoo from Desktop\Recoil dgVoodoo Windowed
# Press Space/Enter once to leave the mission briefing (both games get the key). Scroll Lock = snapshot + screenshot of both.
$ErrorActionPreference = 'Continue'
$desk = [Environment]::GetFolderPath('Desktop')
$port = Join-Path $desk 'recoil_port'
$orig = Join-Path $desk 'Recoil Original Windowed'
$dgv  = Join-Path $desk 'Recoil dgVoodoo Windowed'
$run  = Join-Path $env:TEMP 'recoil_play'
Add-Type -AssemblyName System.Windows.Forms
Add-Type -TypeDefinition @'
using System; using System.Runtime.InteropServices;
public static class SB {
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr a, int x, int y, int cx, int cy, uint f);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern int ChangeDisplaySettingsA(IntPtr dm, int f);
  [DllImport("user32.dll")] public static extern int GetWindowLong(IntPtr h, int i);
  [DllImport("user32.dll")] public static extern int SetWindowLong(IntPtr h, int i, int v);
  [DllImport("user32.dll")] public static extern bool SetWindowText(IntPtr h, string t);
  public struct RECT { public int L, T, R, B; }
  public delegate bool E(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(E f, IntPtr l);
  [DllImport("user32.dll")] public static extern int GetWindowThreadProcessId(IntPtr h, out int p);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, System.Text.StringBuilder s, int n);
  // the game's top-level window: visible, owned by pid, not a console; the biggest one
  public static IntPtr GameWindow(int pid) {
    IntPtr best = IntPtr.Zero; int area = 0;
    EnumWindows((h, l) => { int p; GetWindowThreadProcessId(h, out p); if (p != pid || !IsWindowVisible(h)) return true;
      var c = new System.Text.StringBuilder(64); GetClassName(h, c, 64); if (c.ToString() == "ConsoleWindowClass") return true;
      RECT r; GetWindowRect(h, out r); int a = (r.R - r.L) * (r.B - r.T); if (a > area) { area = a; best = h; } return true; }, IntPtr.Zero);
    return best;
  }
}
'@

Write-Host "=== Recoil side by side ===" -ForegroundColor Cyan
Get-Process Recoil, recoil_boot -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 800
[void][SB]::ChangeDisplaySettingsA([IntPtr]::Zero, 0)

# keep the original's wrapper current
Copy-Item (Join-Path $port 'tools\sidebyside\dinput.dll') (Join-Path $orig 'dinput.dll') -Force
New-Item -ItemType Directory -Force -Path $run | Out-Null
Copy-Item (Join-Path $port '05_remake\build\Port\recoil_boot.exe') (Join-Path $run 'recoil_boot.exe') -Force

# screens: main = primary; left = the other one
$screens = [System.Windows.Forms.Screen]::AllScreens
$main = ($screens | Where-Object Primary)[0].Bounds
$left = ($screens | Where-Object { -not $_.Primary } | Select-Object -First 1)
$left = if ($left) { $left.Bounds } else { $main }

# original
$env:SBS_START_MISSION = '1'
$po = Start-Process -FilePath (Join-Path $orig 'Recoil.exe') -WorkingDirectory $orig -PassThru
# remake
$env:RECOIL_NOCD = '1'; $env:RECOIL_START_MISSION = '1'; $env:RECOIL_WINDOWED = '1'
$env:RECOIL_SHARED_INPUT = '1'; $env:RECOIL_DDRAW_DIR = $dgv
$pr = Start-Process -FilePath (Join-Path $run 'recoil_boot.exe') -WorkingDirectory $run -PassThru -WindowStyle Minimized
Write-Host "original pid $($po.Id), remake pid $($pr.Id) - loading mission 1 ..."

function Place($p, $b, $label) {
    $h = [SB]::GameWindow($p.Id)
    if ($h -eq [IntPtr]::Zero) { return $false }
    $r = New-Object SB+RECT; [void][SB]::GetWindowRect($h, [ref]$r)
    $w = $r.R - $r.L; $hh = $r.B - $r.T
    $x = $b.X + [int](($b.Width - $w) / 2); $y = $b.Y + [int](($b.Height - $hh) / 2)
    # give the borderless game window a normal title bar so it can be dragged anywhere (GWL_STYLE = -16)
    $st = [SB]::GetWindowLong($h, -16)
    $want = ($st -band (-bnot 0x80000000)) -bor 0x00C00000 -bor 0x00080000 -bor 0x00020000   # -POPUP +CAPTION +SYSMENU +MINIMIZEBOX
    if ($st -ne $want) { [void][SB]::SetWindowLong($h, -16, $want); [void][SB]::SetWindowText($h, "RECOIL - $label") }
    if ($script:placed[$label]) {   # already placed once: only keep the frame, never move it again (the user may have dragged it)
        [void][SB]::SetWindowPos($h, [IntPtr]::Zero, 0, 0, 0, 0, 0x0001 -bor 0x0002 -bor 0x0004 -bor 0x0020)   # NOSIZE|NOMOVE|NOZORDER|FRAMECHANGED
        return $true
    }
    [void][SB]::SetWindowPos($h, [IntPtr]::Zero, $x, $y, 0, 0, 0x0001 -bor 0x0020)   # NOSIZE|FRAMECHANGED; HWND_TOP = bring to the front
    if ($i -ge 25) { $script:placed[$label] = $true }   # windows settle after the mission loads; stop repositioning after that
    return $true
}
# windows appear and get resized during loading; place them, then leave them where the user drags them
$script:placed = @{}
for ($i = 0; $i -lt 60; $i++) {
    Start-Sleep -Seconds 1
    if ($po.HasExited -or $pr.HasExited) { break }
    [void](Place $po $left 'ORIGINAL'); [void](Place $pr $main 'REMAKE')
}
if ($po.HasExited) { Write-Host "the original exited - see $orig\sidebyside.log" -ForegroundColor Red }
if ($pr.HasExited) { Write-Host "the remake exited - see $port\05_remake\build\game_sandbox\recoil.err" -ForegroundColor Red }
Write-Host "Ready. Press Space/Enter to start the mission in both. Scroll Lock = capture both. F12-free: close the windows to quit." -ForegroundColor Green
# snapshot listener (captures both games)
Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$(Join-Path (Split-Path $port) 'Recoil Final Attempt\01_evidence\ttd_tools\snapshot_on_hotkey.ps1')`""
