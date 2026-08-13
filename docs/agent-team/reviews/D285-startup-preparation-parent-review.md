# D285 parent review: startup preparation preflight

Status: PASS (bounded source/package review)  
Delivery: D285 / ARCH-255  
Reviewer: parent architect/developer

## Scope reviewed

- `src/quillforge/composition.py`
- `src/quillforge/app.py`
- `scripts/audit_presentation_contracts.py`
- the D285 PyInstaller candidate and release manifest

## Findings

- PASS — `DesktopRuntime` remains the sole owner of plugin capability binding,
  activation, and command-refresh lifecycle stages.
- PASS — normal startup preserves the original order:
  `prepare_startup`, `restore_startup_state`, `refresh_startup_commands`,
  `show`, and `open_startup_paths`.
- PASS — the no-window probe reuses those stages without starting session or
  recovery restoration, showing a window, or entering the event loop.
- PASS — `runtime.stop()` remains in the probe `finally` path and deactivates
  any built-in plugin context after a preparation failure or success.
- PASS — the static audit guards shared-stage existence, normal ordering, and
  the no-window boundary.
- PASS — package/archive and root/dist identity evidence bind the shipped
  candidate to the D285 source snapshot.

## Simplification assessment

PASS. The two lifecycle methods represent distinct responsibilities and keep
the normal and diagnostic paths aligned. Combining them with restore state or
adding a mode flag would either change ordering or make the no-window boundary
less explicit.

## Limits

No EXE/Qt native launch, asynchronous session/recovery completion, clean-machine
startup, user interaction, native rendering, or cross-machine evidence was
run. Preparation success is not a native-startup claim.

## Public source applicability and embedded gate

Applicable public references are Python 3.12 language semantics and public Qt 6
`QApplication` lifecycle documentation, recorded in ADR-0321. No manufacturer
requirement applies. Embedded C/C++ assurance is N/A.
