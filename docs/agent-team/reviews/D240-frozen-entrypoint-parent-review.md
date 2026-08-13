# D240 parent review — frozen entrypoint path hardening

## Decision

`PASS` with operational limits.

## Findings

- The normal package-relative import path remains unchanged when
  `__package__` is truthy.
- Direct source-file execution retains the existing parent-source `sys.path`
  insertion, so early source diagnostics remain usable.
- Frozen execution skips that source-style path mutation when `sys.frozen` is
  true and still imports `quillforge.app` through the packaged importer.
- The `main(argv)` contract, startup exception boundary, Qt argument/path
  routing, and application composition remain outside the changed branch.
- The one-file archive contains the frozen entrypoint, `PYZ-00.pyz`, QtCore,
  QtWidgets, QScintilla, the Windows platform plugin, and QuillForge package
  content; PyInstaller warning scope contains no missing `quillforge` module.

## Evidence

- `D240-FROZEN-ENTRYPOINT-STATIC-CONTRACT=PASS`
- `D240-COMPILEALL=PASS`
- `D240-RUFF=PASS`
- `D240-FORMAT=PASS`
- `D240-FROZEN-ARCHIVE-ESSENTIALS=PASS`
- `D240-PACKAGE-IDENTITY=PASS`
- `D240-CHECK-PS51=PASS`; `D240-CHECK-PS7=PASS`
- `D240-HANDOFF-PS51=PASS`; `D240-HANDOFF-PS7=PASS`

## Simplification assessment

`D240-SIMPLIFICATION-ASSESSMENT=PASS`: the change removes only the frozen
case of unnecessary source-path mutation. It preserves the direct-source
diagnostic convenience and the existing import contract, adds no abstraction,
and does not change application behavior outside the entrypoint branch.

## Architecture consultation and independent review

- Mendel the 7th / Luna max architecture window:
  `NO_CONCLUSION`, closed after bounded waits.
- Galileo the 7th / Terra max escalation window:
  `NO_CONCLUSION`, closed after bounded waits.
- James the 7th / Luna max independent review:
  `NO_CONCLUSION`; the bounded reviewer reported that it had not completed
  the requested call-chain/baseline comparison. No independent PASS is
  claimed.

## Limits

No EXE/Qt launch, updater/installer/registry operation, unit-test asset,
mock, fixture, harness, worktree, or test-only asset was executed or created.
Native startup and clean-machine behavior remain release gates.
