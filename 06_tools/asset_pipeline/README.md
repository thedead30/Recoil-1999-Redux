# tools/asset_pipeline

Offline `.zbd`/`.zmap` → engine-ready asset importer, per `ARCHITECTURE.md` §6: **import
once, at build/dev time, into an intermediate engine-native format — the shipped runtime never
parses `.zbd` at all.** This is the only place in the project that speaks the original
container formats, and it does so entirely by wrapping already-validated RE-phase tooling, not
by re-deriving any parsing logic.

Two things live in this directory with different roles:

- **`src/` + `CMakeLists.txt`** — the `recoil_asset_pipeline` C++ stub declared in the root
  build (target name reserved for later; still just a placeholder `main.cpp`, untouched by
  this pass).
- **`python/`** — the actual, working pipeline (this pass's deliverable). Python, per
  `CLAUDE.md`'s existing-tooling convention and this task's own instructions — there is no
  reason to reimplement byte-perfect, already-validated parsers in C++ just to convert assets
  offline on a dev machine.

## What it does

`python/build_mission_assets.py <mission>` takes one mission's original files from
`RECOIL files after setup/zbd/<mission>/` (plus the shared, game-wide `zbd/zrdr.zbd`) and
produces a complete, engine-ready asset set under `output/<mission>/`:

| Output | Format | Built by | Source |
|---|---|---|---|
| `geometry/<m>_gamez.gltf` + `.bin` | glTF 2.0 | `gamez_export.py` | `gamez.zbd` model3d buffer |
| `geometry/<m>_scene_nodes.json` | JSON | `gamez_export.py` | `gamez.zbd` node buffer (now mesh-linked and hierarchical — see schema below) |
| `geometry/<m>_materials.json` | JSON | `gamez_export.py` | `gamez.zbd` material buffer + texture directory (texture-linked — see schema below) |
| `textures/<archive>/*.png` | PNG | mech3ax `unzbd.exe` (`rc textures`) | `texture2/4/6.zbd`, `rtexture2/4.zbd` |
| `data/weapon_mounts.json` | JSON | `normalize_reader_json.py` | shared `zrdr.zbd`: `weapons.json` |
| `data/vehicle_classes[_easy\|_hard].json` | JSON | ” | shared `zrdr.zbd`: `vehicle*.json` |
| `data/ai_stats.json` | JSON | ” | mission `zrdr.zbd`: `ai.json` |
| `data/ai_vehicle_spawns[_easy\|_hard].json` | JSON | ” | mission `zrdr.zbd`: `aiv*.json` |
| `data/objectives.json` | JSON | ” | mission `zrdr.zbd`: `objectives.json` |
| `data/net_graphs/net_NN.json` | JSON | ” | mission `zrdr.zbd`: `net_NN.json` (N=1..99, variable count) |
| `manifest.json` | JSON | orchestrator | run log + per-stage counts, for CI/spot-check use |

Run: `python python/build_mission_assets.py m1` (defaults to `m1`; pass a mission name, and
optionally `--game-dir`/`--mech3ax-dir`/`--out` to override the default paths, which assume
this project's standard directory layout under the game folder given by RECOIL_GAME_DIR).

## Built on (RE-phase tools this pipeline reuses, not re-derives)

- **`tools/scripts/parse_gamez.py`** — byte-perfect `gamez.zbd` header/material/model3d/node
  section walk, full-coverage validated across all 13,036 real models in all 13 missions. Every
  offset and section-order fact in `gamez_export.py` is copied from this script's confirmed
  layout, cited inline.
- **`tools/scripts/export_gamez_obj.py`** / **`validate_gamez_full.py`** — the same walk,
  extended to *capture* vertex/polygon/UV data instead of discarding it (proven correct via
  OBJ export + exhaustive sanity checks: no OOB indices, no degenerate bboxes, across every
  real model). `gamez_export.py`'s model3d capture loop is a direct extension of this — the only
  difference is capturing *all* models instead of the first N, and emitting glTF instead of OBJ.
- **`recoil_confirmed_logic.md`**'s `Object3DBlock` struct (confirmed offsets: `flags` @0x00,
  `scale[3]` @0x24, `localTransform` (3x3 rotation + translate) @0x30) — used to extract each
  object3d node's local transform into `<m>_scene_nodes.json`. `worldTransform` @0x60 is
  **not** exported; the confirmed doc's own wording ("written by concatenation, read as parent
  by children") reads as runtime-computed, not a reliable static value to trust from the file.
- **mech3ax** (`tools/mech3ax/mech3ax-v0.6.1-x86_64-pc-windows-msvc/unzbd.exe`) — confirmed
  working for Recoil per `CLAUDE.md`'s tools table: `rc textures` for all 5 texture archive
  kinds, `rc reader` for `zrdr.zbd` (weapon/vehicle/AI/net-graph/objectives data). Invoked
  directly via `subprocess`, output unzipped into the pipeline's own directories — this
  pipeline does not depend on or read from the ad-hoc `extracted/` directory from earlier RE
  sessions; it re-extracts from the original `.zbd` files itself so the whole path is
  reproducible standalone.
- **`recoil_confirmed_logic.md`**'s `GamezNode.meshIndex`/`.childArray` and
  `MaterialRecord.texIndex`/`TextureDirEntry.name` fields (added 2026-09-02, via GhidraMCP
  decompile+disassembly of `recoil.exe`'s `gamez.zbd` loader, cross-checked against real
  `m1/gamez.zbd` bytes) — used to wire mesh↔node, node parent/child, and material↔texture
  linkage into this pipeline's output. See "Closed gaps" below.

## Geometry format choice: glTF 2.0, not a bespoke chunked binary

`ARCHITECTURE.md` §6 describes the eventual target as "an engine-native chunked binary
(vertex/index buffers... plus the node-hierarchy/transform data)". This pass ships glTF 2.0
instead, deliberately:

- It is a real, complete, industry-standard format — not a placeholder — directly loadable by
  well-known small libraries (`cgltf`, `tinygltf`) that produce exactly the vertex/index
  buffers `src/render`'s D3D11 path needs, with no bespoke binary-format design/versioning work
  needed before `render`/`scene` can start consuming real data.
- It keeps geometry and (once wired up) materials/node placement in one file — the same "one
  read" property §6 asks for — without inventing and versioning a new on-disk struct layout
  during this foundation pass.
- Converting glTF → a final bespoke packed binary later (if the `render`/`scene` owners decide
  they want one) is a mechanical, low-risk step once those modules' actual buffer-layout needs
  are known; going the other direction — discovering the bespoke format was wrong — would not
  be.

This is a scoping decision for this pass, not a rejection of §6's intent — flag it for
revisiting once `src/render`/`src/engine/scene` have real consumption code.

## Scene-node schema (`geometry/<m>_scene_nodes.json`)

Each entry in `"nodes"` is one `GamezNode`, indexed by its position in that array (`"index"` ==
array position == what `"children"`/`"mesh_index"` elsewhere refer to). Fields added 2026-09-02
(closing the mesh-link and hierarchy gaps below) are marked **NEW**:

| Field | Type | Meaning |
|---|---|---|
| `index` | int | position in this array = the node's id, referenced by other nodes' `children` |
| `name` | string | `GamezNode.name` (`+0x00`) |
| `type` / `type_name` | int / string | `GamezNode.nodeType` (`+0x34`) |
| `active_flags` | int | `GamezNode.activeFlags` (`+0x24`) |
| **`mesh_index`** | int or `null` | **NEW.** Index into `<m>_gamez.gltf`'s `meshes[]` (== a model3d `model_index`), or `null` if this node has no mesh. Confirmed field: `GamezNode+0x3c`, see `recoil_confirmed_logic.md`. |
| `child_count` | int | `GamezNode.childCount` (`+0x5c`) |
| **`children`** | list[int] | **NEW**, present iff `child_count > 0`. Node indices (into this same `"nodes"` array) of this node's children — reconstruct the full hierarchy by following these from the `world`-type root node(s). Confirmed field: `GamezNode+0x60`, see `recoil_confirmed_logic.md`. |
| `bbox_min` / `bbox_max` | [float,float,float] | cached AABB (`+0x74`/`+0x80`) |
| `object3d` | object, type-5 nodes only | `flags`, `scale`, `local_rotation_3x3`, `local_translation` (`Object3DBlock`) |

## Materials schema (`geometry/<m>_materials.json`) — NEW output, 2026-09-02

```json
{
  "mission": "m1", "tex_count": 471, "mat_count": 5000,
  "materials": [
    {"material_index": 0, "textured": true, "texture_index": 4, "texture_name": "dockp01k"},
    ...
  ]
}
```

`materials` lists only the *live* materials (the ones reachable from the material buffer's own
free-list walk — 512/5000 for m1, matching `parse_gamez.py`'s existing linked-list logic).
`texture_name` is the texture's base name with no extension or archive/tier qualifier — a name
resolves to up to 5 files (`textures/{texture2,texture4,texture6,rtexture2,rtexture4}/<name>.png`,
not all 5 necessarily present for every mission — see `build_mission_assets.py`'s
`TEXTURE_ARCHIVES`); which tier to actually load at runtime is a separate, not-yet-traced
question (see `recoil_re_log.md`), so the pipeline hands back the name and lets the consumer
choose rather than guessing one archive. `texture_index`/`texture_name` are both `null` when
`textured` is `false` (the material has no texture at all, per the confirmed flag-bit test).

## Mesh material-index linkage (`geometry/<m>_gamez.gltf` primitive `extras`) — NEW, 2026-09-03

Each polygon's real, on-disk `matIndex` (model3d polygon record `+0x14`) was already being read
by `parse_gamez_full()` into `parsed["models"][i]["polys"][j]["matIndex"]`, but `build_gltf()`
previously discarded it — no output file recorded which material(s) a given mesh actually uses.
Closed by attaching it to each mesh's (single, non-indexed) primitive as glTF's standard
`extras` object (arbitrary per-object app data, valid on any glTF object per the 2.0 spec) —
not a bespoke `"material"` field, since glTF's real `primitive.material` must index into a glTF
materials array (a PBR material definition this pipeline doesn't build); this pipeline's
material indices key into `<m>_materials.json` instead, so `extras` is the correct place for
them, not a repurposed spec field. Chosen over `<m>_materials.json` (organized by material, not
by mesh) or `<m>_scene_nodes.json` (a node is a geometry *instance* — material data belongs with
the geometry it's instancing, in the gltf itself, not duplicated per node):

```json
"primitives": [{
  "attributes": {"POSITION": 0, "TEXCOORD_0": 1},
  "mode": 4,
  "extras": {"material_indices": [0, 3, 4, 5, 6]}
}]
```

- `material_indices` — always present: the sorted, deduped list of every real `matIndex >= 0`
  used by any of the mesh's polygons (`matIndex < 0` = "no material" on that polygon, confirmed
  by `tools/scripts/parse_gamez.py`'s identical uv-presence test — excluded, not treated as
  index `0`). Empty `[]` is possible in principle (no polygon in the mesh has a material) but
  did not occur in m1 (0/1338 exported meshes). A mesh commonly spans several materials — m1
  `model_4` spans 5 (`[0, 3, 4, 5, 6]`, a multi-textured dock-piling prop) across its 19
  polygons.
- `material_index` — convenience scalar, present **iff** `material_indices` has exactly one
  element (equal to that element). Lets a single-texture consumer (e.g. the
  `src/render` textured-mesh demo) read one field without special-casing length-1 lists.
- Each `material_index` value is a `material_index` key directly into that same mission's
  `<m>_materials.json`'s `"materials"` list, which resolves it to `texture_name` (see schema
  above). Cross-checked (m1): every `matIndex` referenced by any polygon in any model (488
  distinct values) falls inside the "live" 512-entry materials list — never an orphaned
  reference to a freed/dead material slot.
- This is per-*mesh*, not per-*triangle*: a multi-material mesh's single primitive still mixes
  triangles from different materials in one vertex/index buffer with no per-triangle material
  tag. Correctly rendering such a mesh (as opposed to just knowing which materials it touches)
  would need splitting it into one primitive per material — not done here, flagged as a
  follow-up if/when `src/render` needs to draw a genuinely multi-material mesh (the current demo
  sidesteps this by using a single-material mesh, `model_1198`).

## Closed gaps (2026-09-02)

The three gaps below were closed via GhidraMCP decompile+disassembly of `recoil.exe`'s
`gamez.zbd` loader (node loader `FUN_00455350`, material loader `FUN_004808c0`, and 3 small
index→pointer resolver functions), cross-checked against real `m1/gamez.zbd` bytes with zero
out-of-range results. Full evidence: `recoil_confirmed_logic.md` (`GamezNode`/`MaterialRecord`/
`TextureDirEntry` struct sections) and `recoil_re_log.md`'s "gamez.zbd mesh/hierarchy/
material-texture linkage" entry (2026-09-02).

- ~~**No mesh ↔ scene-node mapping.**~~ **CLOSED.** `GamezNode+0x3c` is, on disk, a plain
  0-based model3d index (confirmed via the node loader's resolver call). Exported as
  `mesh_index` on every scene node (2,170/4,199 m1 nodes have one) — see schema above.
- ~~**No parent/child edges in the node list.**~~ **CLOSED.** `GamezNode+0x5c`/`+0x60`'s raw
  file entries are, on disk, plain node indices into this same node array (confirmed via the
  node loader's per-element resolver call in `FUN_00454bf0`). Exported as `children` on every
  scene node with `child_count > 0` (1,860/4,199 m1 nodes) — see schema above. Sample check: the
  `world1` root node has 28 children; `object3d` nodes named `g6` resolve their 6 children to
  nodes named `m1`..`m6` — a coherent group/sub-part structure, not noise.
- ~~**No material/texture linkage.**~~ **CLOSED.** `MaterialRecord+0x10` is, on disk, a plain
  0-based texture-directory index, valid when the material's flag byte has the UV/textured bit
  set (confirmed via the material loader's resolver call). The texture directory's own `+0x08`
  field is a null-terminated name, hex-dump-confirmed to match this pipeline's mech3ax-extracted
  PNG basenames exactly. New `<m>_materials.json` output, 465/465 m1 textured materials resolve
  to a real texture name (`dockp01k`, `m1gate01`, `nanite`, ... — see schema above).
- **Non-indexed geometry.** Each glTF primitive duplicates a vertex per triangle corner rather
  than sharing/indexing vertices, because UV is authored per-polygon-corner (a shared vertex
  can carry different UVs in different polygons) — this is a real fan-out from the source
  format, not a bug, but it means the `.bin` buffers are larger than a fully-indexed mesh would
  be. Left as a follow-up optimization since correctness (this pass's goal) doesn't depend on
  it.
- **`weapons.json`/`vehicle*.json` are shared, not per-mission.** Confirmed via
  `recoil_re_log.md`: these live only in the shared root `zbd/zrdr.zbd`, not any per-mission
  one. The pipeline extracts them once per mission run (cheap, `zrdr.zbd` is <1MB) rather than
  hoisting them to a separate "shared assets" pass — revisit if/when the pipeline is extended
  to build all 13 missions in one invocation, to avoid redundant work.
- ~~**No per-mesh material-index export.**~~ **CLOSED 2026-09-03.** `matIndex` was already
  parsed per-polygon (model3d polygon record `+0x14`) but discarded before reaching any output
  file — found and worked around ad hoc by the `src/render` texture-loading task (re-parsing
  `gamez.zbd` directly via a one-off script, see `DEVLOG.md`'s "first real texture" entry).
  Closed for real here: exported as `material_indices`/`material_index` in each mesh's primitive
  `extras` in `<m>_gamez.gltf` — see schema above.

## Verified run (mission 1, updated 2026-09-03 — mesh material-index linkage added)

```
geometry:  1338/1350 models exported, 20130 triangles, 4199 scene nodes
           (2170 mesh-linked, 1860 with children), 5566096/5566096 bytes
           consumed (full coverage — matches parse_gamez.py's validated
           byte-exact parse of m1/gamez.zbd; the 12 unexported models have
           zero captured vertices, same as the OOB/degenerate-mesh
           convention validate_gamez_full.py already established)
materials: 512/5000 live materials, 465 textured, 465/465 resolved to a
           real texture name (0 unresolved) -> m1_materials.json
textures:  576 PNGs each from texture2/4/6.zbd and rtexture2/4.zbd (2,880 PNGs total)
data:      weapon_mounts.json (19 weapons, full confirmed WeaponMountEntry field
           vocabulary — NAME/DESC/DAMAGE/FIRE_RATE/RANGE/VELOCITY/FIRE/FLYOUT/IMPACT/...),
           3 vehicle-class files (25 vehicle classes each, easy/normal/hard),
           ai_stats.json, 3 ai_vehicle_spawns files, objectives.json,
           91 net_graphs/net_NN.json patrol-graph files
           — 0 key-collision warnings across every file normalized
```

Spot-checked real entries (see `recoil_re_log.md`'s 2026-09-02 entry for the full evidence):
- Node `g6` (index 4) has `children: [24, 20, 15, 14, 16, 8]`, resolving to nodes named
  `m1`..`m6` — a coherent named sub-part group, not noise.
- Node `m2` (index 5) has `mesh_index: 1254`, a valid index into `m1_gamez.gltf`'s `meshes[]`.
- Material 0 has `texture_name: "dockp01k"`; `output/m1/textures/texture2/dockp01k.png` exists
  on disk (produced independently by the mech3ax texture step, in a different container).

Mesh material-index linkage (2026-09-03), cross-checked against a direct, independent
`parse_gamez_full()` call over the same source `gamez.zbd` (not just "the exporter ran without
error"):
- Every one of the 1338 exported meshes' `extras.material_indices` matches the ground-truth set
  built directly from `parsed["models"][i]["polys"]` — **0 mismatches across all 1338**.
- `mesh_4` (multi-material case): `extras == {"material_indices": [0, 3, 4, 5, 6]}` — matches
  the 5 distinct materials the `src/render` task found by hand across mesh 4's 19 polygons.
- gltf array index `1186` (`model_1198`, the mesh the `src/render` texture-loading task actually
  used): `extras == {"material_indices": [492], "material_index": 492}`; `material_index: 492`
  resolves in `m1_materials.json` to `{"textured": true, "texture_index": 452, "texture_name":
  "ammolock"}` — matches the render task's independently-discovered `ammolock.png` linkage
  exactly.
- Of 1338 exported meshes: 952 single-material, 386 multi-material, 0 with no material at all.

Total output: ~24 MB under `output/m1/`. Re-run any time with
`python python/build_mission_assets.py m1` — it re-extracts from the original `.zbd` files
each time (no dependency on any previous session's `extracted/` directory), so the whole path
is reproducible from a clean checkout of the original game files.
