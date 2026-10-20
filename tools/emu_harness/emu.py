"""emu.py - L1 emulation harness (STAGE2.md section 4, TODO_STAGE2 P0.6).

Runs an ORIGINAL function's bytes in Ghidra's p-code emulator (GhidraMCP HTTP endpoint
/emulate_function on 127.0.0.1:8089, Recoil.exe loaded) on generated inputs, and writes golden
vectors to 05_remake/tests/vectors/<name>.json. The C++ port is then tested against those vectors
bit for bit (tests/test_*.cpp). The emulator's memory is an isolated overlay: the program is never
modified.

Stack layout used by the endpoint: ESP = 0x7FFF0000 holds the return sentinel, so stack argument i
(cdecl/stdcall/thiscall/fastcall extra args) lives at 0x7FFF0004 + 4*i. Scratch data goes at
0x7FFE0000 upward.

Caveat: Ghidra's emulator evaluates x87 p-code with its own float model. Found wrong on 2026-09-25
against the native oracle (05_remake/tests/native_oracle.h, the original bytes on the real FPU):
  - FCOMP/FNSTSW: C3 is not set for equal operands (0x004727a0 with s = 0 divided instead of copying);
  - FCHS of a NaN does not flip the sign bit (0x004745c0).
It also ignores the precision-control field. So emulator vectors avoid FPU edge values, and the
native oracle is the authority whenever the two disagree.

usage (from repo root):  python tools/emu_harness/emu.py Vec3_Lerp [N]
"""
import json
import os
import random
import struct
import sys
import urllib.request

URL = "http://127.0.0.1:8089/emulate_function"
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "05_remake", "tests", "vectors")
STACK_ARG0 = 0x7FFF0004
SCRATCH = 0x7FFE0000


def f2hex(v):
    return struct.pack("<f", v).hex()


def hex2f(h):
    return struct.unpack("<f", bytes.fromhex(h))[0]


def emulate(addr, registers, memory, read_after, max_steps=20000):
    body = {
        "address": "0x%08x" % addr,
        "registers": json.dumps(registers),
        "memory": json.dumps(memory),
        "read_memory_after": json.dumps(read_after),
        "max_steps": max_steps,
        "return_registers": "EAX,ECX,EDX,ESP,ST0",
    }
    req = urllib.request.Request(URL, data=json.dumps(body).encode(), headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=60) as r:
        res = json.loads(r.read().decode())
    if not res.get("success", False) or not res.get("hit_return", False):
        raise RuntimeError("emulation failed: %s" % json.dumps(res)[:400])
    return res


def mem_after(res, addr):
    """Return hex bytes read back at addr from the emulator result."""
    for k in ("memory_after", "memory", "read_memory"):
        if k in res:
            regs = res[k]
            if isinstance(regs, dict):
                for a, v in regs.items():
                    if int(a, 16) == addr:
                        return v if isinstance(v, str) else v.get("hex") or v.get("data")
            if isinstance(regs, list):
                for e in regs:
                    if int(str(e.get("address")), 16) == addr:
                        return e.get("hex") or e.get("data")
    raise KeyError("no read-back for 0x%x in %s" % (addr, list(res.keys())))


def st0_to_f32hex(v):
    """80-bit x87 register value (hex string from the endpoint) -> float32 hex (little-endian).
    Only exact for values that are already float-representable (e.g. after FLD float)."""
    x = int(v, 16)
    sign = (x >> 79) & 1
    exp = (x >> 64) & 0x7FFF
    mant = x & ((1 << 64) - 1)
    if exp == 0 and mant == 0:
        return struct.pack("<f", -0.0 if sign else 0.0).hex()
    val = (mant / float(1 << 63)) * (2.0 ** (exp - 16383))
    return struct.pack("<f", -val if sign else val).hex()


def rand_vec(rng, lo=-1000.0, hi=1000.0):
    return [struct.unpack("<f", struct.pack("<f", rng.uniform(lo, hi)))[0] for _ in range(3)]


SCR = 0x00566420   # math3d scratch vector written by the distance helpers (global, 3 floats)


def two_vec_ret(addr, rng, xz=False):
    """fastcall (ECX a, EDX b) -> float in ST0; also returns the scratch vector 0x00566420."""
    a, b = rand_vec(rng), rand_vec(rng)
    A, B = SCRATCH, SCRATCH + 0x10
    mem = [{"address": "0x%08x" % A, "hex": "".join(f2hex(x) for x in a)},
           {"address": "0x%08x" % B, "hex": "".join(f2hex(x) for x in b)},
           {"address": "0x%08x" % SCR, "hex": "00000000" * 3}]
    res = emulate(addr, {"ECX": "0x%x" % A, "EDX": "0x%x" % B}, mem, [{"address": "0x%08x" % SCR, "length": 12}])
    scr = mem_after(res, SCR)
    return {"a": [f2hex(x) for x in a], "b": [f2hex(x) for x in b], "ret": [st0_to_f32hex(res["registers"]["ST0"])],
            "scratch": [scr[i * 8:(i + 1) * 8] for i in range(3)]}


def vec3_add_scaled(rng):
    """0x00472770 Vec3_AddScaled: out(arg2) = ECX + EDX * s(arg1); ret 8."""
    a, b = rand_vec(rng), rand_vec(rng)
    s = struct.unpack("<f", struct.pack("<f", rng.uniform(-10, 10)))[0]
    A, B, O = SCRATCH, SCRATCH + 0x10, SCRATCH + 0x20
    mem = [{"address": "0x%08x" % A, "hex": "".join(f2hex(x) for x in a)},
           {"address": "0x%08x" % B, "hex": "".join(f2hex(x) for x in b)},
           {"address": "0x%08x" % STACK_ARG0, "hex": f2hex(s) + struct.pack("<I", O).hex()}]
    res = emulate(0x00472770, {"ECX": "0x%x" % A, "EDX": "0x%x" % B}, mem, [{"address": "0x%08x" % O, "length": 12}])
    out = mem_after(res, O)
    return {"a": [f2hex(x) for x in a], "b": [f2hex(x) for x in b], "s": [f2hex(s)], "out": [out[i * 8:(i + 1) * 8] for i in range(3)]}


def edge_float(rng, lo=-100.0, hi=100.0):
    """Mostly random, sometimes the edge values the listings branch on."""
    c = rng.random()
    if c < 0.08:
        return 0.0
    if c < 0.12:
        return -0.0
    if c < 0.15:
        return float("nan")
    return struct.unpack("<f", struct.pack("<f", rng.uniform(lo, hi)))[0]


def hexs(vals):
    return "".join(f2hex(x) for x in vals)


def aabb_to_corners(rng):
    box = rand_vec(rng) + rand_vec(rng)
    A, O = SCRATCH, SCRATCH + 0x40
    res = emulate(0x00446ed0, {"ECX": "0x%x" % A, "EDX": "0x%x" % O}, [{"address": "0x%08x" % A, "hex": hexs(box)}],
                  [{"address": "0x%08x" % O, "length": 96}])
    out = mem_after(res, O)
    return {"box": [f2hex(x) for x in box], "out": [out[i * 8:(i + 1) * 8] for i in range(24)]}


def scale_by_reciprocal(rng):
    src = rand_vec(rng)
    # no 0 / NaN here: Ghidra's emulator does not set C3 after FCOMP (see caveat); the native oracle covers them
    s = struct.unpack("<f", struct.pack("<f", rng.choice([-1, 1]) * rng.uniform(0.01, 10)))[0]
    same = rng.random() < 0.15
    A, O = SCRATCH, (SCRATCH if same else SCRATCH + 0x10)
    mem = [{"address": "0x%08x" % A, "hex": hexs(src)}, {"address": "0x%08x" % STACK_ARG0, "hex": f2hex(s)}]
    if not same:
        mem.append({"address": "0x%08x" % O, "hex": "11111111" * 3})
    res = emulate(0x004727a0, {"ECX": "0x%x" % A, "EDX": "0x%x" % O}, mem, [{"address": "0x%08x" % O, "length": 12}])
    out = mem_after(res, O)
    return {"src": [f2hex(x) for x in src], "s": [f2hex(s)], "same": ["00000001" if same else "00000000"],
            "out": [out[i * 8:(i + 1) * 8] for i in range(3)]}


def normalize_horizontal(rng):
    v = rand_vec(rng)
    if rng.random() < 0.1:
        v[0] = 0.0
        v[2] = 0.0
    A, O = SCRATCH, SCRATCH + 0x10
    mem = [{"address": "0x%08x" % A, "hex": hexs(v)}, {"address": "0x%08x" % O, "hex": "22222222" * 3}]
    res = emulate(0x004727f0, {"ECX": "0x%x" % A, "EDX": "0x%x" % O}, mem,
                  [{"address": "0x%08x" % O, "length": 12}, {"address": "0x%08x" % A, "length": 12}])
    out, vin = mem_after(res, O), mem_after(res, A)
    return {"v": [f2hex(x) for x in v], "out": [out[i * 8:(i + 1) * 8] for i in range(3)],
            "v_after": [vin[i * 8:(i + 1) * 8] for i in range(3)]}


def perp_xz(rng):
    # no NaN here: Ghidra's emulator does not flip a NaN's sign in FCHS (see caveat); the native oracle covers it
    v = [x if x == x else 1.0 for x in (edge_float(rng) for _ in range(3))]
    A, O = SCRATCH, SCRATCH + 0x10
    res = emulate(0x004745c0, {"ECX": "0x%x" % A, "EDX": "0x%x" % O},
                  [{"address": "0x%08x" % A, "hex": hexs(v)}, {"address": "0x%08x" % O, "hex": "33333333" * 3}],
                  [{"address": "0x%08x" % O, "length": 12}])
    out = mem_after(res, O)
    return {"v": [f2hex(x) for x in v], "out": [out[i * 8:(i + 1) * 8] for i in range(3)]}


def array_add_scaled(rng):
    n = rng.choice([0, 1, 2, 3, 5])
    src = [rand_vec(rng) for _ in range(max(n, 1))]
    scl = [rand_vec(rng) for _ in range(max(n, 1))]
    s = struct.unpack("<f", struct.pack("<f", rng.uniform(-5, 5)))[0]
    D, S, P = SCRATCH, SCRATCH + 0x100, SCRATCH + 0x200
    mem = [{"address": "0x%08x" % D, "hex": "44444444" * 3 * max(n, 1)},
           {"address": "0x%08x" % S, "hex": "".join(hexs(v) for v in src)},
           {"address": "0x%08x" % P, "hex": "".join(hexs(v) for v in scl)},
           {"address": "0x%08x" % STACK_ARG0, "hex": struct.pack("<I", P).hex() + struct.pack("<I", n).hex() + f2hex(s)}]
    res = emulate(0x004744f0, {"ECX": "0x%x" % D, "EDX": "0x%x" % S}, mem, [{"address": "0x%08x" % D, "length": 12 * max(n, 1)}])
    out = mem_after(res, D)
    return {"n": ["%08x" % n], "src": [f2hex(x) for v in src for x in v], "scl": [f2hex(x) for v in scl for x in v],
            "s": [f2hex(s)], "out": [out[i * 8:(i + 1) * 8] for i in range(3 * max(n, 1))]}


# ---------------------------------------------------------------- function drivers
def vec3_lerp(rng):
    """0x00472960 Vec3_Lerp: ECX = t*ECX + (1-t)*EDX, ret 4 (t on the stack)."""
    a = [rng.uniform(-1000, 1000) for _ in range(3)]
    b = [rng.uniform(-1000, 1000) for _ in range(3)]
    t = rng.choice([0.0, 1.0, 0.5, rng.uniform(-0.5, 1.5)])
    a = [struct.unpack("<f", struct.pack("<f", x))[0] for x in a]
    b = [struct.unpack("<f", struct.pack("<f", x))[0] for x in b]
    t = struct.unpack("<f", struct.pack("<f", t))[0]
    A, B = SCRATCH, SCRATCH + 0x10
    mem = [{"address": "0x%08x" % A, "hex": "".join(f2hex(x) for x in a)},
           {"address": "0x%08x" % B, "hex": "".join(f2hex(x) for x in b)},
           {"address": "0x%08x" % STACK_ARG0, "hex": f2hex(t)}]
    res = emulate(0x00472960, {"ECX": "0x%x" % A, "EDX": "0x%x" % B}, mem, [{"address": "0x%08x" % A, "length": 12}])
    out = mem_after(res, A)
    return {"a": [f2hex(x) for x in a], "b": [f2hex(x) for x in b], "t": f2hex(t),
            "out": [out[i * 8:(i + 1) * 8] for i in range(3)]}


DRIVERS = {"Vec3_Lerp": (0x00472960, vec3_lerp),
           "Vec3_DistanceSquared": (0x00472670, lambda r: two_vec_ret(0x00472670, r)),
           "Vec3_Distance": (0x004726d0, lambda r: two_vec_ret(0x004726d0, r)),
           "Vec3_DistSqXZ": (0x00472730, lambda r: two_vec_ret(0x00472730, r)),
           "Vec3_AddScaled": (0x00472770, vec3_add_scaled),
           "AABB_ToCorners": (0x00446ed0, aabb_to_corners),
           "Vec3_ScaleByReciprocal": (0x004727a0, scale_by_reciprocal),
           "Math_NormalizeHorizontalVector": (0x004727f0, normalize_horizontal),
           "Vec3_PerpXZ": (0x004745c0, perp_xz),
           "Vec3Array_AddScaled": (0x004744f0, array_add_scaled)}


def main():
    name = sys.argv[1]
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 64
    addr, drv = DRIVERS[name]
    rng = random.Random(0x5EC0)
    cases = [drv(rng) for _ in range(n)]
    os.makedirs(OUT, exist_ok=True)
    doc = {"function": name, "address": "0x%08x" % addr, "source": "Ghidra p-code emulation of the original bytes (tools/emu_harness/emu.py)",
           "cases": cases}
    json.dump(doc, open(os.path.join(OUT, name + ".json"), "w"), indent=1)
    print(name, len(cases), "vectors written")


if __name__ == "__main__":
    main()
