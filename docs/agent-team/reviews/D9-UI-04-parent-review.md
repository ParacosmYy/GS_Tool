# D9 / UI-04 parent review

## Scope

- **Delivery:** D9 Modern UI iteration
- **Slice:** UI-04 Explicit shell status rail
- **Owner:** Architect (parent)
- **Change type:** presentation-only status projection

## Product and architecture decision

The shell needs a compact, legible lifecycle signal for long-running work and
recoverable failures. `StatusRail` owns only visual state rendering. It does
not inspect or parse notification strings, access application services, or
replace TaskRunner/close guards.

## Implementation record

- `status_bar.py` defines the bounded `ready/working/attention/error` phase
  contract and maps it to explicit labels.
- `MainWindow` projects `working` at the existing document-operation boundary,
  returns to `ready` at the existing completion boundary, and marks `error`
  when it presents a recoverable modal error. The active document's unsaved
  state projects `attention`; switching to a clean tab returns to `ready`.
- `theme.py` styles the phase through the explicit `state` property so color
  semantics remain centralized and independently replaceable.

## Verification and limits

- Ruff check/format and `scripts/check.ps1` are the required static gates.
- `.\scripts\package.ps1` completed after the attention-state refinement;
  root/dist are 38,324,566 bytes
  (recorded in `dist/QuillForge.release.json`) with SHA-256
  `AC8F5D7C0A8FE0C189D58D0E2CFBF1A1BCD18930709DE92F669B686D5B1B1FDD`.
- No EXE, Qt window, interactive startup, or screenshot is launched because
  the user explicitly prohibited starting the software.
- Independent background operations, native accessibility rendering, DPI,
  font availability, and runtime visual contrast remain open for a permitted
  visual review.

## Disposition

`PROCEED WITH LIMITS`: the status rail is a bounded, low-coupling source slice;
runtime visual acceptance remains pending.
