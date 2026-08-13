# D239 parent review — target identity preflight

## Decision

`PASS` with operational limits.

## Findings

- Ordinary update validates the current target against `state.sha256` after
  confirmation and before moving the target to a new backup.
- Rollback validates both target and backup against `state.sha256` and
  `state.previous_sha256` before exchanging them.
- Identical target/backup paths are rejected before mutation.
- The existing D238 recovery guards, containment checks, and
  `SupportsShouldProcess` boundary remain in place.

## Evidence

- `D239-STATE-IDENTITY-STATIC-CONTRACT=PASS`
- `D239-PS51-AST=PASS`
- `D239-PS7-AST=PASS`
- `D239-COMPILEALL=PASS`
- `D239-RUFF=PASS`
- `D239-PACKAGE-IDENTITY-PROBE=PASS`
- `D239-PACKAGE-ARCHIVE=PASS`
- `D239-PYINSTALLER-WARNING-SCOPE=PASS`

## Simplification assessment

`D239-SIMPLIFICATION-ASSESSMENT=PASS`: the change reuses
`Get-QfArtifactInfo`, adds no parallel hash or state abstraction, and keeps
the identity check at the existing updater ownership boundary.

## Independent review

The post-fix Luna/max independent window returned `NO_CONCLUSION` after two
bounded waits and was closed. No independent PASS is claimed.

## Limits

No updater/rollback script, file move, registry, EXE/Qt, unit-test asset,
mock, fixture, harness, worktree, or test-only asset was executed or created.
