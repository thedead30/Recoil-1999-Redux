# Error reporting

## `0x00404e80` — the release-build error reporter is a no-op
`CONFIRMED`, `VERIFIED-ORACLE`.

The function is the **single byte `c3` (`ret`)**, NOP-padded on both sides - read from
`Recoil.exe`. It has **278 call sites** and executes 438 times in `Recoil18` (17 in `Recoil21`).

Callers pass `(level, __FILE__, __LINE__, format, ...)` - for example
`(0x200, "D:\Proj\GameZRecoil\zClass\cls_d...", 0x587, "Unrecognized node class type...")`.
Those strings survive in the binary (they are what `03_re/ledger/source_attribution.csv` is built
from), but **nothing is displayed, logged or acted on**. After the call, callers carry on through
their own return codes (typically `return 3` or `return 5`).

**For a remake:** every error-report call site may be a no-op, or a debug log. Doing more - a
message box, an abort - would change behaviour the original never had.

Being `cdecl` with the caller cleaning the stack, a bare `ret` is safe for any argument count. It
leaves `EAX` unchanged.

## Callers that read the return value
Only one, `FUN_00464f70` (zGeometry): four error paths do `x = report(...); return x & 0xffffff00`.
That is Ghidra's rendering of **returning `false` in `AL`** with the upper 24 bits of `EAX` left as
whatever the call left there. It is harmless if every caller tests only `AL`.

**Resolved 2026-09-15 - not a defect.** `FUN_00464f70` has one caller, at `0x00464964` in
`FUN_00464810`, and the next instruction is `test al, al` (`84 c0`) then a branch. Only the low byte
is read; the junk in the upper 24 bits is never used. `FUN_00464f70` returns a boolean in `AL`.
