"""capture_state.py - state-tagged reference capture of the ORIGINAL Recoil (TODO item H1).

Tooling, not remake code (lives outside 05_remake/src). Attaches to a running Recoil.exe with
ReadProcessMemory, suspends the process for the few ms a capture takes, grabs a screenshot of the
game window and writes a JSON of the fields in fields.json (each field cites its source).

usage:
  python tools/capture_state/capture_state.py --out captures/run1 [--every 0.5] [--count 100]

Output per capture: NNNNN.png + NNNNN.json {t_wall, globals, player, vehicles[]}.
Status: IMPLEMENTED-UNVERIFIED until run against the game (TODO H2).
"""
import argparse
import ctypes
import ctypes.wintypes as wt
import json
import os
import struct
import time

HERE = os.path.dirname(os.path.abspath(__file__))
k32 = ctypes.WinDLL("kernel32", use_last_error=True)
u32 = ctypes.WinDLL("user32", use_last_error=True)
ntdll = ctypes.WinDLL("ntdll")

PROCESS_VM_READ, PROCESS_QUERY_INFORMATION, PROCESS_SUSPEND_RESUME = 0x10, 0x400, 0x800
TH32CS_SNAPPROCESS = 0x2


class PROCESSENTRY32(ctypes.Structure):
    _fields_ = [("dwSize", wt.DWORD), ("cntUsage", wt.DWORD), ("th32ProcessID", wt.DWORD),
                ("th32DefaultHeapID", ctypes.c_void_p), ("th32ModuleID", wt.DWORD),
                ("cntThreads", wt.DWORD), ("th32ParentProcessID", wt.DWORD),
                ("pcPriClassBase", ctypes.c_long), ("dwFlags", wt.DWORD), ("szExeFile", ctypes.c_char * 260)]


def find_pid(name=b"recoil.exe"):
    snap = k32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    e = PROCESSENTRY32()
    e.dwSize = ctypes.sizeof(e)
    ok = k32.Process32First(snap, ctypes.byref(e))
    while ok:
        if e.szExeFile.lower() == name:
            k32.CloseHandle(snap)
            return e.th32ProcessID
        ok = k32.Process32Next(snap, ctypes.byref(e))
    k32.CloseHandle(snap)
    return None


class Proc:
    def __init__(self, pid):
        self.h = k32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_SUSPEND_RESUME, False, pid)
        if not self.h:
            raise OSError("OpenProcess failed: %d" % ctypes.get_last_error())

    def read(self, addr, n):
        buf = ctypes.create_string_buffer(n)
        got = ctypes.c_size_t()
        if not k32.ReadProcessMemory(self.h, ctypes.c_void_p(addr), buf, n, ctypes.byref(got)) or got.value != n:
            return None
        return buf.raw

    def u32(self, a):
        b = self.read(a, 4)
        return struct.unpack("<I", b)[0] if b else None

    def suspend(self):
        ntdll.NtSuspendProcess(self.h)

    def resume(self):
        ntdll.NtResumeProcess(self.h)


def read_typed(p, addr, typ):
    n = {"u32": 4, "f32": 4, "f32x3": 12}[typ]
    b = p.read(addr, n)
    if b is None:
        return None
    if typ == "u32":
        return struct.unpack("<I", b)[0]
    if typ == "f32":
        return struct.unpack("<f", b)[0]
    return list(struct.unpack("<3f", b))


def read_inst(p, node, fields):
    inst = p.u32(node + 4) if node else None
    if not inst:
        return None
    out = {"node": hex(node), "inst": hex(inst)}
    for k, f in fields.items():
        if not k.startswith("_"):
            out[k] = read_typed(p, inst + int(f["off"], 16), f["type"])
    return out


def capture(p, F):
    rec = {"t_wall": time.time(), "globals": {}}
    for k, g in F["globals"].items():
        a = int(g["addr"], 16)
        if g.get("deref"):
            a = p.u32(a) or 0
        rec["globals"][k] = read_typed(p, a, g["type"]) if a else None
    vf = F["vehicle_inst_fields"]
    rec["player"] = read_inst(p, p.u32(int(F["player_node"]["addr"], 16)), vf)
    vs, node, guard = [], p.u32(int(F["vehicle_list"]["addr"], 16)), 0
    while node and guard < 256:
        vs.append(read_inst(p, node, vf))
        node, guard = p.u32(node), guard + 1
    rec["vehicles"] = vs
    return rec


def screenshot(path):
    from PIL import ImageGrab
    hwnd = u32.FindWindowW("RecoilClass", None)   # class name from app.md (single-instance check)
    r = wt.RECT()
    if hwnd and u32.GetWindowRect(hwnd, ctypes.byref(r)):
        ImageGrab.grab(bbox=(r.left, r.top, r.right, r.bottom)).save(path)
    else:
        ImageGrab.grab().save(path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    ap.add_argument("--every", type=float, default=0.5)
    ap.add_argument("--count", type=int, default=1)
    a = ap.parse_args()
    F = json.load(open(os.path.join(HERE, "fields.json")))
    pid = find_pid()
    if not pid:
        raise SystemExit("Recoil.exe not running")
    p = Proc(pid)
    os.makedirs(a.out, exist_ok=True)
    for i in range(a.count):
        p.suspend()
        try:
            rec = capture(p, F)
            screenshot(os.path.join(a.out, "%05d.png" % i))
        finally:
            p.resume()
        json.dump(rec, open(os.path.join(a.out, "%05d.json" % i), "w"), indent=1)
        time.sleep(a.every)
    print("captured", a.count, "->", a.out)


if __name__ == "__main__":
    main()
