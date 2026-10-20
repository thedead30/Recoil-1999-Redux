# CRT random numbers (`rand` / `srand`) - decision D5

## Where they come from
- `rand` and `srand` are **imports from `MSVCRT.dll`** (IAT `0x004cc5d8` rand, `0x004cc488` srand).
  They are not linked into `Recoil.exe`. `CONFIRMED-BINARY`, import table.
- The behaviour is therefore the system `msvcrt.dll`'s. The copy recorded in trace Recoil22
  (Windows 11, `msvcrt!rand` at `0x774be300`) was disassembled on 2026-09-25 (`CONFIRMED-BINARY`,
  TTD):

```
rand:  call _getptd ; imul ecx,[eax+14h],343FDh ; add ecx,269EC3h ; mov [eax+14h],ecx
       shr ecx,10h ; and ecx,7FFFh ; mov eax,ecx ; ret
srand: call _getptd ; mov ecx,[ebp+8] ; mov [eax+14h],ecx ; ret
```

- **Generator:** `state = state * 214013 + 2531011` (mod 2^32); the result is `(state >> 16) & 0x7FFF`.
- **The state is per thread** (the `_tiddata` field at `+0x14`). A thread that never calls `srand`
  starts from 1. The remake must keep one state per original thread that calls `rand`.

## Seeding - both seeds come from `time()`
| site | function | code |
|---|---|---|
| `0x00455958` | `0x004558f0` (DEClient init) | `push 0; call [0x004cc48c]` (time) `; push eax; call srand` |
| `0x0045e16d` | `0x0045e100` (`Anim_ResetGlobals`) | the same, after `[0x00575dbc] = 0xc11ccccd` |

The seed is wall-clock time, so every run of the original differs. For lockstep (L4), the return
values of `time()` at `0x0045594e` and `0x0045e163` are **seams**: they are recorded and replayed
(`tools/lockstep/FORMAT.md`).

## Call sites
All `rand` call sites (21, including register-cached calls through `[0x004cc5d8]`) are listed in
`03_re/ledger/rand_call_sites.csv`. The order in which they run each frame is part of the
behaviour: one extra or missing call shifts every later value. L4 checks it by comparing the per
thread state against the trace.
