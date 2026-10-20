import os
import struct

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

    header = struct.unpack("<9I", read(0x24))
    magic, const = header[0], header[1]
    assert magic == 0x02971222, f"bad magic 0x{magic:x}"
    assert const == 0xf, f"bad const 0x{const:x}"
    texCount = header[2]
    print(f"=== {path} ===")
    print(f"header={header}")
    print(f"texCount={texCount}")

    # Section 1: texture directory, texCount * 0x24 bytes, no extra reads
    tex_start = pos
    tex_data = read(texCount * 0x24)
    print(f"texture directory: {tex_start}..{pos} ({len(tex_data)} bytes)")

    # Section 2: material buffer
    mat_start = pos
    matCount, matField2, matField3, matStartIdx = struct.unpack("<4I", read(0x10))
    matStartIdx = struct.unpack("<i", struct.pack("<I", matStartIdx))[0]  # as signed
    print(f"material header: count={matCount} field2={matField2} field3={matField3} startIdx={matStartIdx}")
    mat_array_start = pos
    mat_array = read(matCount * 0x2c)
    n_cycled = 0
    idx = matStartIdx
    visited = set()
    while idx >= 0:
        if idx in visited or idx >= matCount:
            print(f"  WARNING: linked-list issue at idx={idx} (visited={idx in visited}, matCount={matCount})")
            break
        visited.add(idx)
        mat = mat_array[idx*0x2c:(idx+1)*0x2c]
        flag_byte = mat[1]
        if flag_byte & 4:
            cycle_header = read(0x1c)
            cycleCount = struct.unpack_from("<I", cycle_header, 0x10)[0]
            read(cycleCount * 4)
            n_cycled += 1
        next_idx = struct.unpack_from("<h", mat, 0x2a)[0]  # signed 16-bit
        idx = next_idx
    print(f"material buffer: {mat_start}..{pos}, {n_cycled} materials had cycling data, visited {len(visited)}/{matCount}")

    # Section 3: model3d buffer
    model_start = pos
    modelCount, modelField2, modelField3 = struct.unpack("<3I", read(0xc))
    print(f"model3d header: count={modelCount} field2={modelField2} field3={modelField3}")
    model_array = read(modelCount * 0x58)
    for mi in range(modelField2):
        rec = model_array[mi*0x58:(mi+1)*0x58]
        vertCount, normCount, morphCount, pointCount = struct.unpack_from("<4I", rec, 0x10)
        polyCount = struct.unpack_from("<I", rec, 0xc)[0]
        if vertCount > 0:
            read(vertCount * 0xc)
        if normCount > 0:
            read(normCount * 0xc)
        if morphCount > 0:
            read(morphCount * 0xc)
        if pointCount > 0:
            # ground truth (FUN_00481c50): ONE fread of pointCount*0x4c contiguous
            # fixed records, THEN a separate sequential pass appending each
            # point's variable tail (subCount*0xc) in index order - not interleaved.
            points = read(pointCount * 0x4c)
            for pi in range(pointCount):
                prec = points[pi*0x4c:(pi+1)*0x4c]
                subCount = struct.unpack_from("<I", prec, 0xc)[0]
                if subCount > 0:
                    read(subCount * 0xc)
        if polyCount > 0:
            polys = read(polyCount * 0x1c)
            for pi in range(polyCount):
                prec = polys[pi*0x1c:(pi+1)*0x1c]
                flags = struct.unpack_from("<I", prec, 0)[0]
                idxCount = flags & 0xff
                matIndex = struct.unpack_from("<i", prec, 0x14)[0]
                if idxCount != 0:
                    read(idxCount * 4)
                    if flags & 0x200:
                        read(idxCount * 4)
                # ground truth (FUN_00481c50 + FUN_004805e0): UV block present iff
                # mat_array[matIndex].flagByte (offset 1, same field as material
                # cycling) has bit 0 set. matIndex<0 => no material => no UV.
                uv_present = False
                if matIndex >= 0 and matIndex < matCount:
                    mflag = mat_array[matIndex*0x2c + 1]
                    uv_present = (mflag & 1) != 0
                if uv_present:
                    read(idxCount * 8)
    print(f"model3d buffer: {model_start}..{pos}")

    # Section 4: node buffer. Ground truth (FUN_00455520 -> FUN_00455350 -> FUN_00454c60):
    # the fixed node array is CAPACITY-sized (header[6], constant 16000 across all
    # files - same reserved-capacity convention already confirmed for materials
    # (header field matCount=5000) and models (modelCount=6000): one contiguous
    # fread of capacity*0xc4 bytes) - NOT sized by the per-file "used" count.
    # header[7] is normally the actual used-node count (confirmed byte-exact on
    # 12 of 13 real mission files this way), and is the loop bound for the second
    # pass, where FUN_00454c60(node, file) does the REAL per-node-type
    # variable-length file reads (previously missed entirely - the multi-MB
    # "trailing data" mystery was this, not a separate section).
    # KNOWN ANOMALY (m6 specifically): header[7]=309 undershoots badly (691KB of
    # real trailing node data unaccounted for) even though the raw header bytes
    # are unambiguous and correctly positioned (confirmed via hex dump, no
    # misalignment). The true used count for m6 was found empirically (exhaustive
    # walk to find where pos lands exactly on EOF) to be 4955, not 309 - source
    # of the discrepancy within the file/engine not yet identified (open
    # question, see recoil_re_log.md 2026-08-31 node-buffer entries). Since gamez.zbd
    # has no section after node data (confirmed via exhaustive "Error reading
    # GameZ ..." string enumeration), this parser self-verifies instead of
    # trusting header[7] blindly: it keeps walking nodes past header[7] if bytes
    # remain, stopping only when the file is exactly exhausted.
    node_start = pos
    nodeCap = header[6]
    nodeCount_header = header[7]
    print(f"node buffer: start={node_start} capacity(header[6])={nodeCap} header[7]={nodeCount_header}")
    nodes = read(nodeCap * 0xc4)
    world_records = []
    type_counts = {}
    ni = 0
    while pos < len(data) and ni < nodeCap:
        rec = nodes[ni*0xc4:(ni+1)*0xc4]
        ntype = struct.unpack_from("<I", rec, 0x34)[0]
        type_counts[ntype] = type_counts.get(ntype, 0) + 1
        if ntype == 2:
            world_records.append(ni)

        # FUN_00454c60 per-type fixed block + conditional extras
        if ntype == 0:
            pass
        elif ntype == 1:  # camera
            read(0x1e8)
        elif ntype == 2:  # world
            world = read(0xac)
            cnt1 = struct.unpack_from("<i", world, 0x90)[0]
            if cnt1 > 0:
                read(cnt1 * 4)
            cnt2 = struct.unpack_from("<i", world, 0x9c)[0]
            if cnt2 > 0:
                read(cnt2 * 4)
            rows = struct.unpack_from("<i", world, 0x7c)[0]
            cols = struct.unpack_from("<i", world, 0x78)[0]
            print(f"  [node {ni}] WORLD: cnt1={cnt1} cnt2={cnt2} rows={rows} cols={cols}")
            for _r in range(max(rows, 0)):
                for _c in range(max(cols, 0)):
                    cell = read(0x40)
                    cellSub = struct.unpack_from("<h", cell, 0x3a)[0]
                    if cellSub > 0:
                        read(cellSub * 4)
        elif ntype == 3:  # window
            read(0xf8)
        elif ntype == 4:  # display
            read(0x1c)
        elif ntype == 5:  # object3d
            read(0x90)
        elif ntype == 6:  # LOD
            read(0x50)
        elif ntype == 9:  # light
            light = read(0xe4)
            cnt = struct.unpack_from("<i", light, 0xdc)[0]
            if cnt > 0:
                read(cnt * 4)
        elif ntype == 10:  # sound
            snd = read(0x94)
            cnt = struct.unpack_from("<i", snd, 0x8c)[0]
            if cnt > 0:
                read(cnt * 4)
        else:
            raise ValueError(f"node[{ni}]: unrecognized node type {ntype} (pos={pos})")

        # common to ALL types: two more arrays at fixed-record offsets 0x54/0x5c
        cnt54 = struct.unpack_from("<i", rec, 0x54)[0]
        if cnt54 >= 1:
            read(cnt54 * 4)
        cnt5c = struct.unpack_from("<i", rec, 0x5c)[0]
        if cnt5c > 0:
            read(cnt5c * 4)

        ni += 1
        if ni == nodeCount_header and pos != len(data):
            print(f"  NOTE: header[7]={nodeCount_header} reached but {len(data)-pos} bytes remain - "
                  f"continuing past header[7] (self-verifying against EOF, see m6 anomaly)")

    nodeCount_actual = ni
    if nodeCount_actual != nodeCount_header:
        print(f"  ANOMALY: actual used node count ({nodeCount_actual}) != header[7] ({nodeCount_header})")
    print(f"node buffer end: {pos}, used_count={nodeCount_actual}, world-type(2) records at indices: {world_records}")
    print(f"node type histogram: {type_counts}")

    remaining = len(data) - pos
    print(f"bytes consumed so far: {pos} of {len(data)}, remaining={remaining}")
    return pos, remaining

if __name__ == "__main__":
    for i in range(1, 14):
        m = f"m{i}"
        try:
            pos, remaining = parse(os.path.join(os.environ.get("RECOIL_GAME_DIR", "game"), "zbd", m, "gamez.zbd"), verbose=False)
            print(f"{m}: OK remaining={remaining}")
        except Exception as e:
            print(f"{m}: FAILED - {e}")
        print()
