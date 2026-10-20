# boot_window.ps1 - list the visible windows (with child text) of the running recoil_boot.exe and save a screenshot of its main window.
# usage: powershell -File tools\boot_window.ps1 [-Out file.png]
param([string]$Out = "$env:TEMP\recoil_boot_window.png")
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @"
using System; using System.Text; using System.Collections.Generic; using System.Runtime.InteropServices;
public static class RW { public delegate bool E(IntPtr h, IntPtr l);
 [DllImport("user32.dll")] public static extern bool EnumWindows(E f, IntPtr l);
 [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr p, E f, IntPtr l);
 [DllImport("user32.dll")] public static extern int GetWindowThreadProcessId(IntPtr h, out int pid);
 [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
 [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
 [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
 [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint f);
 public struct RECT { public int L,T,R,B; }
 public static IntPtr Main = IntPtr.Zero;
 public static List<string> All(int pid) { var o = new List<string>();
  EnumWindows((h,l) => { int p; GetWindowThreadProcessId(h, out p); if (p!=pid || !IsWindowVisible(h)) return true;
   var t=new StringBuilder(256); var c=new StringBuilder(64); GetWindowText(h,t,256); GetClassName(h,c,64); RECT r; GetWindowRect(h,out r);
   o.Add(String.Format("window '{0}' class={1} {2}x{3} at {4},{5}", t, c, r.R-r.L, r.B-r.T, r.L, r.T));
   if (c.ToString()=="RecoilClass") Main = h;
   EnumChildWindows(h,(ch,ll)=>{ var tt=new StringBuilder(1024); var cc=new StringBuilder(64); GetWindowText(ch,tt,1024); GetClassName(ch,cc,64); if (tt.Length>0) o.Add("   "+cc+": "+tt); return true; },IntPtr.Zero);
   return true; }, IntPtr.Zero);
  return o; } }
"@
$p = Get-Process recoil_boot -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $p) { "recoil_boot is not running"; exit 1 }
"pid $($p.Id) cpu $([math]::Round($p.CPU,1)) s"
[RW]::All($p.Id)
if ([RW]::Main -ne [IntPtr]::Zero) {
  $r = New-Object RW+RECT; [void][RW]::GetWindowRect([RW]::Main, [ref]$r); $w = $r.R - $r.L; $h = $r.B - $r.T
  $bmp = New-Object System.Drawing.Bitmap $w, $h; $g = [System.Drawing.Graphics]::FromImage($bmp); $dc = $g.GetHdc()
  [void][RW]::PrintWindow([RW]::Main, $dc, 2); $g.ReleaseHdc($dc); $bmp.Save($Out); "screenshot -> $Out"
}
