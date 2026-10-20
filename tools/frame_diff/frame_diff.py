"""frame_diff.py - L5(b) pixel comparison (STAGE2.md section 4, TODO_STAGE2 P0.8).

Compares a remake frame with a reference frame from the ORIGINAL (captures/, 01_evidence/frames_index)
and reports whether they match within a per-channel tolerance. Decision D2: 2D screens and the HUD
must match exactly (tolerance 0); 3D frames get a small tolerance set from measured rasteriser
differences. Optionally writes a diff image (differing pixels in red over a dimmed reference).

usage: python tools/frame_diff/frame_diff.py REF.png TEST.png [--tol N] [--max-bad-frac F] [--diff OUT.png]
exit code 0 = within tolerance, 1 = differs, 2 = size mismatch
"""
import argparse
import sys

from PIL import Image, ImageChops


def compare(ref, test, tol=0):
    a, b = ref.convert("RGB"), test.convert("RGB")
    if a.size != b.size:
        return None
    d = ImageChops.difference(a, b)
    px = d.get_flattened_data() if hasattr(d, "get_flattened_data") else d.getdata()
    bad = sum(1 for p in px if max(p) > tol)
    worst = max((max(p) for p in px), default=0)
    return {"pixels": a.size[0] * a.size[1], "bad": bad, "worst": worst, "diff": d}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ref")
    ap.add_argument("test")
    ap.add_argument("--tol", type=int, default=0, help="max per-channel difference counted as equal")
    ap.add_argument("--max-bad-frac", type=float, default=0.0, help="allowed fraction of differing pixels")
    ap.add_argument("--diff", help="write a diff visualisation")
    a = ap.parse_args()
    ref, test = Image.open(a.ref), Image.open(a.test)
    r = compare(ref, test, a.tol)
    if r is None:
        print("SIZE MISMATCH", ref.size, test.size)
        return 2
    frac = r["bad"] / r["pixels"]
    ok = frac <= a.max_bad_frac
    print("%s  differing %d / %d (%.4f%%), worst channel diff %d, tol %d" % (
        "MATCH" if ok else "DIFFERS", r["bad"], r["pixels"], 100 * frac, r["worst"], a.tol))
    if a.diff:
        base = ref.convert("RGB").point(lambda v: v // 3)
        mask = r["diff"].convert("L").point(lambda v: 255 if v > a.tol else 0)
        base.paste((255, 0, 0), mask=mask)
        base.save(a.diff)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
