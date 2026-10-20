#!/usr/bin/env python3
"""Export a map's world triangles as a flat binary soup for the runtime's ground queries.

WHY A TRIANGLE SOUP AND NOT A HEIGHTFIELD
-----------------------------------------
A 2D heightmap cannot represent map 1. The map has tunnels and a multi-level dock structure, so a
single height per (x,z) is wrong by construction — a max-height field would put the tank on the
tunnel ROOF, and a min-height field would drop it through the dock. The runtime therefore keeps
the real triangles and answers "what surface is directly below this point" exactly, which handles
tunnels, overhangs and ramps because it never collapses the third dimension.

The cost is trivial: map 1's drawn world is ~15.5k triangles, i.e. about 560 KB of positions.

WHAT THIS IS NOT
----------------
This is the RENDER geometry, used as ground. The original also ships real collision data — 160
`bvol` / `collide01..10` nodes, 40 of them mesh-bearing — which is almost certainly what the
engine actually collides against, and which is deliberately NOT rendered. Those are per-object
collision volumes rather than terrain, and wiring them up is a separate job. Using render geometry
for the ground is a defensible approximation for driving on terrain; it is not a claim about how
the original does collision.

Format (little-endian):
    magic   'RCTS'          4 bytes
    version u32 = 1
    count   u32             triangle count
    then `count` * 9 float32: ax ay az bx by bz cx cy cz
"""

import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from render_turntable import load_obj  # the same OBJ reader the renderers use


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--obj", required=True, help="world OBJ produced by export_bft_obj.py --world")
    ap.add_argument("--out", required=True)
    ap.add_argument("--skip-sky", action="store_true", default=True,
                    help="drop anything absurdly large (the horizon shell), which is not ground")
    args = ap.parse_args()

    verts, uvs, tris, mtl = load_obj(args.obj)
    print("loaded %d triangles from %s" % (len(tris), os.path.basename(args.obj)))

    kept = []
    dropped_huge = 0
    for (i0, i1, i2, _, _, _, _) in tris:
        a, b, c = verts[i0], verts[i1], verts[i2]
        if args.skip_sky:
            # A triangle spanning more than 1500 units is scenery-scale backdrop, not something
            # a tank drives on. The horizon shell is excluded from the world export already; this
            # is a belt-and-braces guard so a stray backdrop can never become "the ground".
            span = max(max(a[0], b[0], c[0]) - min(a[0], b[0], c[0]),
                       max(a[2], b[2], c[2]) - min(a[2], b[2], c[2]))
            if span > 1500.0:
                dropped_huge += 1
                continue
        kept.append((a, b, c))

    with open(args.out, "wb") as f:
        f.write(b"RCTS")
        f.write(struct.pack("<II", 1, len(kept)))
        for a, b, c in kept:
            f.write(struct.pack("<9f", a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2]))

    size = os.path.getsize(args.out)
    print("dropped %d backdrop-scale triangles (span > 1500 units)" % dropped_huge)
    print("wrote %s: %d triangles, %.0f KB" % (args.out, len(kept), size / 1024.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
