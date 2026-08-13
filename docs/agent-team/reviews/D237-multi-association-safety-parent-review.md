# D237 parent review — multi-association safety

## Decision

`PASS` with explicit runtime limits.

## Findings

- `New-QfFileAssociation` keeps refusal of an existing external ProgId, while
  allowing only the installer's later calls to reuse the ProgId created by the
  same operation and matching the exact open command.
- `Remove-QfFileAssociation` is idempotent for an already absent owned
  extension, but still refuses a changed default value or changed command.
- `Remove-QfProgId` scans HKCU Classes references before deleting the shared
  key, so an unrecorded user reference blocks deletion.
- Install rollback deletes extension keys before the ProgId and preserves the
  target executable if cleanup is incomplete.
- Uninstall stops before deleting the executable or state when association
  cleanup is incomplete, avoiding a known dangling-target failure mode.

## Evidence

- `D237-DISTRIBUTION-STATIC-CONTRACT=PASS`
- `D237-PS51-AST=PASS`
- `D237-PS7-AST=PASS`
- `D237-COMPILEALL=PASS`
- `D237-RUFF=PASS`
- `D237-PACKAGE-IDENTITY-PROBE=PASS`
- `D237-PACKAGE-ARCHIVE=PASS`
- `D237-PYINSTALLER-WARNING-SCOPE=PASS`
- `D237-SOURCE-REVISION-BOUNDARY=PASS`

## Simplification assessment

`D237-SIMPLIFICATION-ASSESSMENT=PASS`: the fix keeps association creation and
cleanup in the existing distribution module, uses one installer ownership
marker rather than a second state store, and reuses the existing cleanup
functions. Reference scanning is the smallest necessary guard against
deleting a shared ProgId still referenced by a user key.

## Independent review

The post-fix bounded Luna/max independent review window returned
`NO_CONCLUSION` after two waits and was closed. The earlier bounded review
identified the defect that motivated D237; its finding was addressed and is
not claimed as post-fix approval.

## Limits

No installer/uninstaller script, registry provider, EXE, Qt runtime, shell
association, clean-machine, signing, or cross-machine check was executed.
No unit-test code, mock, fixture, harness, worktree, or test-only asset was
created or run.
