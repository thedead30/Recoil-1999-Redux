import os
import struct, sys

def parse(path, verbose=False, max_nodes=None):
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

    magic, const, manifestCount = struct.unpack("<III", read(12))
    assert magic == 0x08170616, f"bad magic 0x{magic:x}"
    assert const == 0x1c, f"bad const 0x{const:x}"

    manifest = []
    for i in range(manifestCount):
        rec = read(0x54)
        path_str = rec[:0x54-8].split(b'\x00', 1)[0]
        manifest.append(path_str)

    world_header = read(0x3c)
    animNodeCount = struct.unpack_from("<H", world_header, 0xa)[0]
    trailingCount = struct.unpack_from("<H", world_header, 0x10)[0]

    print(f"=== {path} ===")
    print(f"manifestCount={manifestCount} animNodeCount={animNodeCount} trailingCount={trailingCount}")
    if verbose and manifest:
        print("sample manifest paths:", manifest[:3])

    nodes = []
    n_to_parse = animNodeCount if max_nodes is None else min(animNodeCount, max_nodes)
    for node_idx in range(n_to_parse):
        node_start = pos
        rec = bytearray(read(0x134))
        counts = {
            0x104: struct.unpack_from("<B", rec, 0x104)[0],
            0x105: struct.unpack_from("<B", rec, 0x105)[0],
            0x106: struct.unpack_from("<B", rec, 0x106)[0],
            0x107: struct.unpack_from("<B", rec, 0x107)[0],
            0x108: struct.unpack_from("<B", rec, 0x108)[0],
            0x109: struct.unpack_from("<B", rec, 0x109)[0],
            0x10a: struct.unpack_from("<B", rec, 0x10a)[0],
            0x10b: struct.unpack_from("<B", rec, 0x10b)[0],
            0x10d: struct.unpack_from("<B", rec, 0x10d)[0],
        }
        # name-ish printable run in the fixed struct (for eyeballing)
        printable = bytes(b if 32 <= b < 127 else 0 for b in rec[:0x40])

        arr_0x105 = [read(0x60) for _ in range(counts[0x105])]
        arr_0x106 = [read(0x28) for _ in range(counts[0x106])]
        arr_0x107 = [read(0x2c) for _ in range(counts[0x107])]
        arr_0x108 = [read(0x2c) for _ in range(counts[0x108])]
        arr_0x109 = [read(0x24) for _ in range(counts[0x109])]
        arr_0x10a = [read(0x24) for _ in range(counts[0x10a])]
        arr_0x10b = [read(0x30) for _ in range(counts[0x10b])]
        arr_0x10d = [read(0x48) for _ in range(counts[0x10d])]
        block_0xc4 = read(0x40)
        var_size = struct.unpack_from("<i", rec, 0x100)[0]
        var_buf = read(var_size) if var_size > 0 else b""
        arr_0x104 = []
        for _ in range(counts[0x104]):
            elem = read(0x40)
            sub_size = struct.unpack_from("<i", elem, 0x3c)[0]
            sub = read(sub_size) if sub_size > 0 else b""
            arr_0x104.append((elem, sub))

        nodes.append({
            "idx": node_idx, "start": node_start, "counts": counts,
            "printable": printable, "rec": bytes(rec),
            "arr_0x104": arr_0x104, "var_buf": var_buf,
        })
        if verbose and node_idx < 5:
            print(f"  node[{node_idx}] @0x{node_start:x} counts={counts} printable={printable!r}")

    trailing = [read(0x24) for _ in range(trailingCount)] if animNodeCount == n_to_parse else []

    remaining = len(data) - pos
    print(f"parsed {len(nodes)} of {animNodeCount} nodes, {len(trailing)} trailing records")
    print(f"bytes consumed: {pos} of {len(data)}, remaining: {remaining}")
    return nodes, remaining

if __name__ == "__main__":
    for m in ["m1", "m2", "m6"]:
        try:
            parse(os.path.join(os.environ.get("RECOIL_GAME_DIR", "game"), "zbd", m, "anim.zbd"), verbose=(m=="m1"))
        except Exception as e:
            print(f"{m}: FAILED - {e}")
        print()
