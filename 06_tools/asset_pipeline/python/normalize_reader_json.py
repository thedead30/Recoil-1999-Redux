"""
normalize_reader_json.py — turns mech3ax's raw reader-zbd JSON output into
clean, key-addressable JSON for the C++ data-loading code to consume.

Background: mech3ax's `unzbd rc reader`/generic reader-zbd extraction (used
for zrdr.zbd, which is where weapons.json/vehicle*.json/ai.json/aiv*.json/
net_NN.json/objectives.json all come from - see recoil_re_log.md's
"objectives.zrd + per-mission zrdr.zbd" entry) is explicitly a low-level,
comprehensive dump (its own README says so) - it preserves the interpreter
script's raw shape: a flat list alternating [key, value, key, value, ...],
recursively, with every "value" itself wrapped in a list (even scalars).
That is completely faithful to the source format but awkward for a C++
loader to consume directly. This module does ONE well-defined, purely
structural transform on top of mech3ax's already-validated extraction - it
does not re-derive or guess anything about the zrdr.zbd container format
itself:

  - a list is treated as a "pairs list" iff it has an even length >= 2 AND
    every even-indexed element is a string (a key). Such a list becomes a
    dict, recursively normalizing each value.
  - a normalized value that is a single-element list is unwrapped to that
    element (since mech3ax always wraps scalars in a 1-element list).
  - anything else (odd-length lists, numeric-keyed-looking lists, lists
    containing lists/non-strings at even positions - e.g. a raw
    [x,y,z]-triplet, a min/max pair, or a list of sub-lists) is left as a
    plain array and NOT reinterpreted.
  - a genuine duplicate key within one pairs list (not observed in any of
    the real files this was run against, see build_mission_assets.py's
    output log) is preserved losslessly by converting that key's value to a
    list of every occurrence, rather than the later one silently
    overwriting the earlier one - flagged via `warnings` so it's visible,
    not silent.

This heuristic was checked against every real file used by the pipeline
(weapons.json, vehicle.json/_easy/_hard, ai.json, aiv.json/_easy/_hard, all
99 net_NN.json files that exist per mission, objectives.json) - see
build_mission_assets.py's run log for the per-file duplicate-key warning
count (0 across all of them, as of this pipeline's first run against m1).
"""


def _is_pairs_list(node):
    if not isinstance(node, list):
        return False
    n = len(node)
    if n < 2 or n % 2 != 0:
        return False
    return all(isinstance(node[i], str) for i in range(0, n, 2))


def normalize(node, warnings=None, path=""):
    if warnings is None:
        warnings = []

    if isinstance(node, list):
        if _is_pairs_list(node):
            result = {}
            for i in range(0, len(node), 2):
                key = node[i]
                raw_value = node[i + 1]
                value = normalize(raw_value, warnings, f"{path}.{key}")
                if isinstance(value, list) and len(value) == 1:
                    value = value[0]
                if key in result:
                    warnings.append(f"duplicate key '{key}' at {path or '<root>'} - merged into a list")
                    if isinstance(result[key], list) and result.get(f"__is_multi_{key}"):
                        result[key].append(value)
                    else:
                        result[key] = [result[key], value]
                    result[f"__is_multi_{key}"] = True
                else:
                    result[key] = value
            # strip internal bookkeeping keys
            return {k: v for k, v in result.items() if not k.startswith("__is_multi_")}
        else:
            return [normalize(x, warnings, f"{path}[{i}]") for i, x in enumerate(node)]
    else:
        return node


def normalize_reader_file(raw):
    """raw is mech3ax's top-level output: a 1-element list wrapping the real
    pairs list (every file observed follows this shape). Returns
    (normalized_dict, warnings)."""
    warnings = []
    normalized = normalize(raw, warnings)
    # unwrap the outer single-element list mech3ax always produces
    if isinstance(normalized, list) and len(normalized) == 1:
        normalized = normalized[0]
    return normalized, warnings


if __name__ == "__main__":
    import json
    import sys
    path = sys.argv[1]
    with open(path) as f:
        raw = json.load(f)
    normalized, warnings = normalize_reader_file(raw)
    for w in warnings:
        print("WARNING:", w, file=sys.stderr)
    print(json.dumps(normalized, indent=1))
