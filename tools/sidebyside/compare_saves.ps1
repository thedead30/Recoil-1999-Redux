# compare_saves.ps1 - KG-61 compare-save replay (testing tool). For every Scroll Lock save the remake wrote
# (05_remake\build\game_sandbox\savedgames\cmp_*), start the ORIGINAL (Desktop\Recoil Original Windowed + dinput.dll wrapper)
# with that save loaded (SBS_LOAD_SAVE): full memory dump the moment gameplay starts (orig_frame0.dmp, written by the wrapper),
# then a reference screenshot (orig.png) a few seconds later, then close it.
# Output: Recoil Final Attempt\01_evidence\snapshots\compare\<save>\orig_frame0.dmp / orig.png. Saves already done are skipped
# (-Redo to repeat). Only touches the original it starts itself - never attach to a game someone is playing.
param([switch]$Redo, [double]$Settle = 8)
$ErrorActionPreference = 'Continue'
$desk = [Environment]::GetFolderPath('Desktop')
$port = Join-Path $desk 'recoil_port'
$orig = Join-Path $desk 'Recoil Original Windowed'
$saves = Join-Path $port '05_remake\build\game_sandbox\savedgames'
$out = Join-Path $desk 'Recoil Final Attempt\01_evidence\snapshots\compare'
$cdb = Join-Path $env:LOCALAPPDATA 'Microsoft\WindowsApps\cdbX86.exe'
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
Add-Type -TypeDefinition @'
using System; using System.Runtime.InteropServices;
public static class CS {
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern int ChangeDisplaySettingsA(IntPtr dm, int f);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  public struct RECT { public int L, T, R, B; }
}
'@
New-Item -ItemType Directory -Force -Path $out, (Join-Path $orig 'SavedGames') | Out-Null
Get-Process Recoil -ErrorAction SilentlyContinue | Stop-Process -Force; Start-Sleep -Milliseconds 800
Copy-Item (Join-Path $port 'tools\sidebyside\dinput.dll') (Join-Path $orig 'dinput.dll') -Force
$list = Get-ChildItem $saves -Filter 'cmp_*' -ErrorAction SilentlyContinue | Sort-Object Name
if (-not $list) { Write-Host "no cmp_* saves in $saves (press Scroll Lock in the remake started with RECOIL_COMPARE=1)"; exit }
foreach ($s in $list) {
    $dir = Join-Path $out $s.Name
    if (-not $Redo -and (Test-Path (Join-Path $dir 'orig_frame0.dmp'))) { Write-Host "$($s.Name): done already"; continue }
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    Copy-Item $s.FullName (Join-Path $orig 'SavedGames') -Force
    Get-Process Recoil -ErrorAction SilentlyContinue | Stop-Process -Force; Start-Sleep -Milliseconds 800
    $env:SBS_START_MISSION = '0'; $env:SBS_LOAD_SAVE = $s.Name
    $f0 = Join-Path $env:TEMP 'cmp_frame0.dmp'; Remove-Item $f0 -ErrorAction SilentlyContinue; $env:SBS_DUMP = $f0   # written by the wrapper the moment gameplay starts
    $p = Start-Process (Join-Path $orig 'Recoil.exe') -WorkingDirectory $orig -PassThru
    $log = Join-Path $orig 'sidebyside.log'; $ok = $false
    for ($i = 0; $i -lt 90 -and -not $p.HasExited; $i++) {
        Start-Sleep 1
        if ((Test-Path $log) -and (Select-String -Path $log -SimpleMatch "load $($s.Name) -> 1" -Quiet)) { $ok = $true; break }
    }
    if (-not $ok) { Write-Host "$($s.Name): original did not load it (see $log)" -ForegroundColor Red; Copy-Item $log $dir -ErrorAction SilentlyContinue; Get-Process Recoil -ErrorAction SilentlyContinue | Stop-Process -Force; continue }
    Start-Sleep 10   # mission load -> briefing screen
    $p.Refresh(); $h = $p.MainWindowHandle
    if ($h -ne [IntPtr]::Zero) { [void][CS]::SetForegroundWindow($h); Start-Sleep -Milliseconds 500; [System.Windows.Forms.SendKeys]::SendWait('{ENTER}') }   # leave the briefing
    for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) {   # frame-0 dump (save applied, nothing played yet)
        if (Select-String -Path $log -SimpleMatch 'frame-0 dump' -Quiet) { break }; Start-Sleep -Milliseconds 500 }
    if (Test-Path $f0) { Move-Item $f0 (Join-Path $dir 'orig_frame0.dmp') -Force }
    Start-Sleep -Milliseconds ([int]($Settle * 1000))   # seconds of play after the briefing before the capture (the player can die on hard levels)
    if ($h -ne [IntPtr]::Zero) {
        [void][CS]::SetForegroundWindow($h); 
        $r = New-Object CS+RECT; [void][CS]::GetWindowRect($h, [ref]$r)
        $bmp = New-Object System.Drawing.Bitmap ([Math]::Max(1, $r.R - $r.L)), ([Math]::Max(1, $r.B - $r.T))
        [System.Drawing.Graphics]::FromImage($bmp).CopyFromScreen($r.L, $r.T, 0, 0, $bmp.Size)
        $bmp.Save((Join-Path $dir 'orig.png')); $bmp.Dispose()
    }
    Copy-Item $log $dir -ErrorAction SilentlyContinue
    Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue; Start-Sleep -Milliseconds 800
    [void][CS]::ChangeDisplaySettingsA([IntPtr]::Zero, 0)
    Write-Host "$($s.Name): orig_frame0.dmp + orig.png -> $dir" -ForegroundColor Green
}
Remove-Item Env:SBS_LOAD_SAVE -ErrorAction SilentlyContinue
