# ROADMAP — live status board

Updated at the end of every work unit, **before** reporting. Numbers come from
`python 03_re/scripts/re_status.py` — never typed from memory.

Last updated: 2026-09-30 (auto: roadmap_sync.py)

---

## Where things stand

| | |
|---|---|
| Stage | **2 - translation** |
| Engine functions CONFIRMED | **3603 / 3603   100.0%** |
| Subsystem gates | **41 / 41 OPEN** |
| Closed gates | none |
| Stage 2 progress | P1 143/150 verified (150 translated); P2 517/741 verified (651 translated); P3 292/894 verified (569 translated); P4 304/607 verified (533 translated); P5 283/1009 verified (575 translated) |
| Known gaps open | 26 (`03_re/ledger/known_gaps.csv`) |
| Remake source | 224 files, 220561 lines |
| Drift verdict | CLEAN |
`harvest.py` is validated against known ground truth — on `0x00452ec0` it flags both traps that
cost a full pass to find by hand, on `0x00487c50` it locates the fast sqrt at `0x00487d51`
exactly, on `0x00429870` all four `ftol`→`expApprox` pairs.
