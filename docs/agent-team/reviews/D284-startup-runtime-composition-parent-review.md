# D284 parent review: startup runtime-composition preflight

Status: PASS (bounded source/package review)  
Delivery: D284 / ARCH-254  
Reviewer: parent architect/developer

## Scope reviewed

- `src/quillforge/app.py`
- `scripts/audit_presentation_contracts.py`
- the D284 PyInstaller candidate and release manifest

## Findings

- PASS — the new probe reuses `build_desktop_runtime()` and does not create a
  second composition graph or move application policy into diagnostics.
- PASS — the probe creates a temporary `QApplication` only when none exists,
  does not call `DesktopRuntime.start()`, `show()`, or `exec()`, and reports
  explicit no-window facts.
- PASS — runtime cleanup is guarded in `finally`; `runtime.stop()` is attempted
  only after successful construction and the temporary application is quit only
  when owned by the probe. Cleanup exceptions cannot replace the diagnostic
  failure being reported by the outer `probe()` boundary.
- PASS — the static contract requires the probe ordering, cleanup anchors, and
  no-window/event-loop invariants.
- PASS — D284 source and package evidence binds the candidate to the same
  root/dist identity and includes the runtime-composition code in the archive.

## Simplification assessment

PASS. No safe behavior-preserving reduction was identified. Removing the
ownership flag would risk quitting a caller-owned QApplication; merging the
cleanup into the probe wrapper would obscure the runtime/application lifetime
boundary and could mask cleanup failures.

## Limits

No EXE/Qt native launch, clean-machine startup, user interaction, native
rendering, or cross-machine evidence was run. The probe validates construction,
not native window lifetime or platform-driver behavior.

## Public source applicability and embedded gate

The applicable public references are Python 3.12 `try`/`finally` semantics and
the public Qt 6 `QApplication` lifecycle documentation, recorded in
`docs/adr/0320-startup-runtime-composition-preflight.md`. No manufacturer
requirement applies. Embedded C/C++ assurance is N/A.
