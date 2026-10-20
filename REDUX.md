# Recoil Redux

Recoil Redux is the enhanced version of the remake, built on **Recoil Classic** (the exact, verified rebuild on branch
`port-work`). It lives on branch `redux`, in the working folder `recoil_redux`.

## The rule

**Every Redux feature is a switch. With every switch off, Redux is Recoil Classic.** It must then still pass
`tools/prove_identity.py` and the oracle tests. An improvement can never quietly change the original game.

- Each change to original behaviour is logged in `03_re/ledger/redux_backlog.csv` (id `RX-nn`), the same way the
  original's constants are tagged.
- Fixes to Classic are made on `port-work` and merged into `redux`, never the other way round.

## Backlog

`03_re/ledger/redux_backlog.csv` lists every idea with its approach, dependencies, effort (S / M / L) and status.

## Thanks

DustTheory's [recoil-patch](https://github.com/DustTheory/recoil-patch) (2026) showed that the game's own renderer
handles widescreen resolutions once its resolution presets are extended (RX-03). That repository has no licence, so
Redux uses none of its code or images, only the knowledge that the approach works.
