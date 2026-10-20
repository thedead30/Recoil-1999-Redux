# Caveat list - functions CONFIRMED with parts not itemised

Source: the list in `LOOP.md` under "STAGE 1 COMPLETE", plus caveats added while writing the specs.
This file is the single tracker. Status: **RESOLVED** (bytes re-read and itemised),
**DEFERRED** (out of v1 scope), **OPEN**.

## Resolved
| address | name | resolution (2026-09-24) |
|---|---|---|
| 0x00429b40 / 0x00429d30 | Vehicle_ComputeLateralSlipAccel / Vehicle_IntegrateSkid | the fields are a skid model: `+0xb0` is lateral velocity and `+0x20` the skid flag (`vehicle.md` 4.2) |
| 0x0040b460 / 0x0040b4e0 / 0x0040b560 | CommandsDialog_OnCaptureKey / JoystickButton / MouseButton | set A/B/C are keyboard, joystick and mouse capture handlers (`input.md`) |
| 0x00434980 / 0x00434dd0 | Save/LoadGameDialog scalar-deleting dtors | pairing confirmed from each constructor's vtable slot 2 |
| 0x00435f80 (+0x00435f50) | LoadGameScreen_Open (+SaveGameScreen_Open) | selector `[0x004f3fb8]` is 1 = save, 0 = load; mode 2 = opened from inside a game |
| 0x00409380 | CreditsPanel_Tick | won game → pop, then push screen `0x004f3e78`; otherwise pop |
| 0x00443032 | AppRun_CatchFileException | all 15 causes itemised, caption "File Error" |
| 0x00405040 | Camera_Mode1_Follow | the EAX at 0x004052b0 is the vehicle skid flag `+0x20`: the direction smoother uses base 2.0 while skidding, 3.0 otherwise |
| 0x00408720 | Settings_ApplyVideoModePreset | every call and value itemised; **found** that `0x004086e0`/`0x00408700` had swapped names (SetSize/SetOrigin), now fixed |

| 0x0042bb30 | Game_HandleStateEvent | every event itemised (0, 1, 2, 10, 11, 14, 15, 16, 17, 20, 25-27, 99, 911-914); the earlier note misnamed callees and is replaced |

| 0x004b4810 | EditField_SetCaretShape | 13-point I-beam caret outline itemised; was misnamed BevelFrame_SetRect |

| 0x00407700 | RecoilApp_LoadUserSettings | per-key fallbacks itemised; real values come from the detail.zrd presets |

| 0x004626b0 | Seq_Load | every FMV action mapped from bytes; **fixed** wrong mapping plus 3 wrong names (SeqFade/SeqBlur/SeqMci_Ctor) |

| 0x00428d60 | Vehicle_GatherGroundContacts | all five steps itemised: sample points pre-offset by vy*dt except in SUB, zone swap around the 500.0 query, point-0 record kept at +0x44c |

| 0x0044f1d0 | ZClass_DestroyNodeData | all 11 class types itemised (detach loop, reference-count gate, per-type destroy) |

| 0x0048ed60 | Raster_BlendLine | it is a thick alpha-ramped line (renamed from Raster_BlendSpan); clipping, alpha ramp (start ×255<<16, step ×(2^24-1): a quirk) and 555/565 blend itemised |

| 0x0048f560 | Raster_BlitImageColorKey | all 8 paths (16-bit/paletted × copy/key/alpha, 555/565) itemised; one-row vertical clip quirk found |

| 0x0048daf0 | Raster_RippleDistort | displacement formula itemised: radial `(x,y)/r * amp*sin(r/λ+φ)` with bit-trick sqrt; fast path = same maths without clipping |
| 0x004c20a0 | Script_DispatchCommand | argument order for every command in `03_re/decomp/0x004c20a0_interp_args.csv`; first-char switch and prefix-match quirks byte-read |
| 0x0042e5da | RecoilApp_BootstrapAndCDCheck | every branch itemised with IAT names and message IDs; **fixed**: zrdr.zbd is opened only when the first settings load fails |
| 0x0042eed0 | App_EnterGameplay | all steps itemised; **fixed**: the first frame is `App_RenderFrameAndPresent(0)`, a normal frame, not a reset; CD track formula recorded |
| 0x0042f280 | App_RenderFrameAndPresent | all 3 paths itemised; **fixed**: the argument is a present flag, not "reset only"; the mission tick and HUD tick run inside the render frame |

## Deferred - out of v1 scope
| address | name | why |
|---|---|---|
| 0x0048a9c0, 0x0048a520, 0x0048c250, 0x0048afe0, 0x0048bbe0 | DirectPlay layer | multiplayer |
| 0x0043d2e0, 0x0043dcd0, 0x0043cf90, 0x004407e0, 0x00440a30 | Zone lobby | multiplayer |
| 0x00430250, 0x00430c30 | MainWnd_Ctor, MainWnd_OnAbout | Windows menu bar and About box are dropped in v1 |
| 0x0040c370 | RecoilApp_ProbeDirectXCapabilities | `PLATFORM` probe; the remake does not use DirectX 5 |

## Open (in scope)
| address | name | what is not itemised |
|---|---|---|
