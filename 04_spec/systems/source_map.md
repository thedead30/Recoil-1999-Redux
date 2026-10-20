# The original engine's source layout, recovered from the binary
`CONFIRMED-BINARY` 2026-09-10. Ledger: `03_re/ledger/source_attribution.csv`.

The binary embeds **55 original source paths**, all under `D:\Proj\GameZRecoil\`, passed to
an error reporter alongside a line number. They name the developers' own file and module
structure for the GameZ engine.

## The 18 modules
| module | what it is |
|---|---|
| `zClass` | the object/scene core - `Class.c`, `Object3d.c`, `Camera.c`, `Light.c`, `Seq.c`, `Animate.c`, `Sound.c`, `Switch.c`, `Window.c`, `Display.c`, `List.c`, `cls_world.c`, `cls_zbd.c`, `cls_di.c`, `cls_util.c` |
| `zVideo` | DirectDraw / D3D bring-up - `zvid_dd.c`, `zvid_ddd3d.c`, `zvid_buff.c`, `zvid_init.c` |
| `zSound` | `zsnd_play.cpp`, `zsnd_init.cpp`, `zsnd_create.cpp`, `zsnd_cd.cpp`, `zsnd_grp.cpp`, `zsnd_parm.cpp`, `zsnd_3d.cpp` |
| `zModel` | `gmod_init.c`, `gmod_const.c`, `gmod_matl.c`, `gmod_light.c` |
| `zGeometry` | `zgeo_weiler.cpp`, `zgeo_model.cpp`, `zgeo_convexify.cpp` |
| `zEffect` | `zeff_init.c`, `zeff_anim_init.c`, `zeff_anim_run.c`, `zeff_anim_save.c` |
| `zNetwork` | `znet_dplay.cpp` |
| `zRender` | `zrndr_draw.c` |
| `zInput` | `zin_init.cpp`, `zin_kbd.cpp` |
| `zFMV` | `fmv_main.cpp`, `fmv_script.cpp`, `fmv_stream.cpp` |
| `zImage` | `zimg_texture.cpp`, `zimg_fonts.cpp` |
| `zDEClient` | `zdec_init.cpp`, `zdec_crater.cpp`, `zdec_qsand.cpp` |
| `zWeapon` | `zwep_init.c` |
| `zInterp` | `zinterp_parse.cpp` |
| `zMath` `zReader` `zUtil` `zError` | `zmth_main.c`, `zreader.cpp`, `zutl_zar.cpp`, `zerr_old.c` |

Note `zDEClient` holds `zdec_crater.cpp` and `zdec_qsand.cpp` - **craters and quicksand**,
matching the `quicksand_slowdown` key already found in `VehicleClassConfig.md`.
`zGeometry\zgeo_weiler.cpp` names the **Weiler-Atherton** clipping algorithm.

## Attribution - 562 functions, 18.0% of the engine
Each path string is referenced from the function that reports the error, so a reference
attributes that function to that file.

| evidence | count | meaning |
|---|---|---|
| **`anchor`** | **336** | the function directly references the path string. `CONFIRMED-BINARY` |
| **`span`** | **226** | the function lies between two anchors of the same file. `INFERRED` |
| total | **562** | |

### Why `span` is trustworthy, and where it is not
36 source files have two or more anchors. Their address ranges are **completely disjoint -
zero overlaps between different files**. That is what a linker emitting one object file's
functions contiguously produces, and it is not what noise produces: with 336 scattered
attributions across 55 files, random assignment would collide constantly.

Independently validated on the one file established by other means: `zsnd_play.cpp` picks up
`0x0049fbb0` - proven in `04_spec/systems/sound.md` to call `IDirectSoundBuffer::Play` and
then the error reporter - along with `Sound_StartVoice_Mode1`, `Sound_StopOrRestoreVoice` and
`Sound_DuplicateA3DVoice`. All four are genuinely voice-playback code.

**The limits, stated plainly:**
- A function that never calls the error reporter is not an anchor, so files with one anchor
  contribute no span, and the 19 single-anchor files are barely mapped.
- Functions in the **gaps between** file ranges are unattributed and must not be guessed at.
- A span attribution is `INFERRED`. It is good enough to decide **what to read next**; it is
  not evidence about behaviour and must never be cited as provenance for a constant.

## Anchored spans, largest first
| file | range | anchors |
|---|---|---|
| `Class.c` | `0x004478c0 - 0x00449af0` | 40 |
| `Camera.c` | `0x00449be0 - 0x0044d260` | 26 |
| `Object3d.c` | `0x0044d9e0 - 0x0044e5b0` | 19 |
| `zvid_dd.c` | `0x004a7b40 - 0x004a95e0` | 18 |
| `Light.c` | `0x00452fd0 - 0x00453aa0` | 16 |
| `znet_dplay.cpp` | `0x0048a0d0 - 0x0048b730` | 16 |
| `cls_util.c` | `0x004518b0 - 0x00452560` | 14 |
| `zgeo_weiler.cpp` | `0x00464680 - 0x0046a1f0` | 14 |
| `gmod_const.c` | `0x004815c0 - 0x00484250` | 12 |
| `zvid_ddd3d.c` | `0x004a9ac0 - 0x004ad250` | 12 |
| `Window.c` | `0x0044f7a0 - 0x0044fcf0` | 10 |
| `cls_zbd.c` | `0x004543f0 - 0x004557a0` | 10 |
| `cls_di.c` | `0x00443f80 - 0x00446f60` | 9 |
| `Sound.c` | `0x004529c0 - 0x00452dc0` | 8 |
| `Seq.c` | `0x00453ee0 - 0x004541c0` | 8 |
| `zsnd_play.cpp` | `0x0049f6f0 - 0x004a0400` | 7 |
| `cls_world.c` | `0x004502b0 - 0x00451640` | 7 |
| `gmod_matl.c` | `0x00480600 - 0x004812c0` | 7 |
| `zeff_anim_init.c` | `0x0045e210 - 0x0045fb30` | 7 |
| `zgeo_model.cpp` | `0x0046a770 - 0x0046bb90` | 6 |
| `zsnd_cd.cpp` | `0x004a20d0 - 0x004a2880` | 6 |
| `zrndr_draw.c` | `0x00499a20 - 0x00499ec0` | 3 |

## What this is for
1. **Prioritising.** `zrndr_draw.c` sits at `0x00499a20-0x00499ec0`, and `zvid_dd.c` /
   `zvid_ddd3d.c` cover `0x004a7b40-0x004ad250` with 30 anchors between them. That is where
   the render and D3D bring-up work lives, and `04_spec/systems/render.md` lists exactly what
   is still missing from it. Previously the draw path had to be hunted through a call graph
   defeated by function-pointer dispatch tables.
2. **Structure for Stage 2.** The remake can mirror the original's module boundaries instead
   of inventing its own, which keeps the correspondence ledger legible.
3. **Sanity-checking.** A function whose behaviour looks like sound but which sits inside
   `zGeometry`'s range is a signal that a reading is wrong.

**277 of the 562 are still `UNTOUCHED`**, so this is a map of where to dig, not a substitute
for digging.
