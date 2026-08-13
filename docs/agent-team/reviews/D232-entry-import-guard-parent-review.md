# D232 parent review — guarded entry-point application import

## Decision

`PASS` with limits.

## Findings

- `quillforge.__main__.main(argv)` remains callable and keeps the existing
  public argument shape.
- `app.main` is imported only when the entry function is called, so import
  failures occur inside the existing bottom-level startup exception boundary.
- The package-relative branch and direct-source path branch retain their prior
  responsibilities.
- Normal application dispatch still delegates the original argument object to
  `app.main`; no Qt or composition owner moved.
- The change is local to `__main__.py` and adds no dependency or second logger.

## Evidence

- `D232-LAZY-MAIN-IMPORT-CONTRACT-PROBE=PASS`
- `D232-ENTRY-IMPORT-FAILURE-GUARD-PROBE=PASS`
- `D232-COMPILEALL=PASS`
- `D232-RUFF=PASS`
- `D232-PRESENTATION-AUDIT=PASS`
- `D232-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`

## Simplification assessment

`D232-SIMPLIFICATION-ASSESSMENT=PASS`: one public wrapper preserves the
existing interface and moves only the import into the already-owned guarded
call path; no abstraction or duplicate dispatch owner was introduced.

## Limits

No EXE, QApplication, native MessageBox rendering, clean-machine, or
cross-machine launch was performed. No native runtime success is claimed.
