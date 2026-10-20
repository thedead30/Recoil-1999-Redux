import os
import struct, sys

def parse(path, verbose=True):
    with open(path, "rb") as f:
        data = f.read()
    pos = 0

    def read(n):
        nonlocal pos
        chunk = data[pos:pos+n]
        if len(chunk) != n:
            raise EOFError(f"wanted {n} bytes at {pos}, got {len(chunk)} (file size {len(data)})")
        pos += n
        return chunk

    version, field2 = struct.unpack("<II", read(8))
    assert version == 5, f"bad version {version}"
    block24 = read(0x18)
    floats24 = struct.unpack("<6f", block24)

    print(f"=== {path} ===")
    print(f"version={version} field2={field2} block24_floats={floats24}")

    records = []
    idx = 0
    while pos < len(data):
        try:
            header3 = read(3)
        except EOFError:
            break
        (count,) = struct.unpack("<I", read(4))
        points = []
        for _ in range(count):
            pt = struct.unpack("<3f", read(0xc))
            points.append(pt)
        trailer = read(4)
        trailer_val = struct.unpack("<I", trailer)[0]
        records.append({"idx": idx, "header3": header3.hex(), "count": count, "points": points, "trailer": trailer_val})
        if verbose:
            pts_preview = points[:2] + (["..."] if len(points) > 2 else [])
            print(f"  record[{idx}] header3={header3.hex()} count={count} trailer={trailer_val} points_preview={pts_preview}")
        idx += 1

    remaining = len(data) - pos
    print(f"parsed {len(records)} records, bytes consumed={pos} of {len(data)}, remaining={remaining}")
    return records, remaining

if __name__ == "__main__":
    import glob
    files = sorted(glob.glob(os.path.join(os.environ.get("RECOIL_GAME_DIR", "game"), "maps", "*.zmap")))
    for f in files:
        try:
            parse(f, verbose=False)
        except Exception as e:
            print(f"{f}: FAILED - {e}")
        print()
