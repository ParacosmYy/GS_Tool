# D235 parent review — startup-path edge routing

## Decision

`PASS` with limits.

## Findings

- `_takes_qt_value()` normalizes only leading-dash spelling and preserves the
  original Qt option/value pair, so `--platform windows` and `-platform
  windows` do not become startup paths.
- `_drain_startup_paths()` still returns for a temporary busy or session
  restore barrier, allowing the existing completion/finish callbacks to retry.
- A non-temporary admission rejection now produces an explicit warning instead
  of silently discarding the requested path.
- No document/workspace service, registry operation, or second startup queue
  was introduced.

## Evidence

- `D235-DESKTOP-LAUNCH-PARSER-PROBE=PASS`
- `D235-COMPILEALL=PASS`
- `D235-RUFF=PASS`
- `D235-PRESENTATION-AUDIT=PASS`
- `D235-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`
- `D235-PACKAGE-ARCHIVE-PROBE=PASS entries=166 pyz=PYZ.pyz`
- `D235-PYZ-MODULE-PROBE=PASS modules=261`
- `D235-PYINSTALLER-WARNING-SCOPE-PROBE=PASS`
- `D235-SOURCE-REVISION-BOUNDARY-PROBE=PASS`
- `D235-PACKAGE-IDENTITY-PROBE=PASS hash=CBAF4460C5162BE71A86881C032B8DE6835CFE923A5D5473F758A85388674A07 bytes=38574557`

## Simplification assessment

`D235-SIMPLIFICATION-ASSESSMENT=PASS`: the existing parser and startup drain
remain the single owners. The two local branches are clearer and safer than a
new abstraction or a second error/queue policy.

## Limits

Native EXE/Qt, shell drag-and-drop, registry association, installer, and
clean-machine behavior were not run under the active policy.
