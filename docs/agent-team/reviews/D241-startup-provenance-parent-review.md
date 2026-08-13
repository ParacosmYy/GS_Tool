# D241 parent review — startup provenance diagnostic

## Decision

`PASS` with operational limits.

## Findings

- The change is confined to `_run_startup_diagnostic` and two diagnostic
  helpers in `src/quillforge/app.py`; the default `QApplication` path is
  unchanged.
- The application import probe uses the existing `importlib` boundary and
  reports module/package/origin/loader metadata through the existing fail-soft
  `probe` wrapper.
- The frozen plugin helper checks the first existing PyQt6 Qt6/legacy Qt
  plugin root and the `platforms/qwindows.dll` file without changing
  `QT_PLUGIN_PATH`; only a boolean configured state is reported for the
  environment.
- The existing `qwindows_platform_plugin` result now reuses the helper's
  selected path and status, eliminating the Qt6-versus-legacy-Qt diagnostic
  mismatch.
- Source mode explicitly marks frozen/plugin checks not applicable, while
  source Qt/PyQt/import checks remain active.

## Evidence

- `D241-AST=PASS`
- `D241-COMPILEALL=PASS`
- `D241-RUFF=PASS`
- `D241-FORMAT=PASS`
- `D241-SOURCE-STARTUP-DIAGNOSTIC=PASS`
- `D241-FROZEN-ARCHIVE-ENTRYPOINT=PASS`
- `D241-PACKAGE-IDENTITY=PASS`

## Simplification assessment

`D241-SIMPLIFICATION-ASSESSMENT=PASS`: the helper is a single diagnostic
ownership boundary and the platform-plugin result reuses its selected path
instead of maintaining a second hard-coded path. No runtime abstraction,
environment mutation, or duplicated plugin-resolution policy was introduced.

## Architecture consultation and independent review

- Helmholtz the 7th / Luna max initial D241 architecture window:
  `NO_CONCLUSION` after bounded wait.
- Pauli the 7th / Luna max follow-up architecture review for the final
  hard-coded-path correction: `PASS`.
- Confucius the 7th / Luna max final independent review: `PASS` with the
  explicit limit that EXE/Qt was not started.

## Limits

No EXE/Qt launch, updater/installer/registry operation, unit-test asset,
mock, fixture, harness, worktree, or test-only asset was executed or created.
The diagnostic verifies presence and provenance only; it does not prove Qt
platform plugin loading or native window creation.
