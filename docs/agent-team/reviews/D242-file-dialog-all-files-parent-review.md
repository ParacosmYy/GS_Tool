# D242 parent review — file dialog all-files default

## Decision

`PASS` with operational limits.

## Findings

- The source change is confined to the two localized values of the existing
  `dialog.text_filter` key in `src/quillforge/presentation/i18n.py`.
- Qt's documented `;;` filter-list contract is preserved. `All files (*)` is
  now the first/default filter, so files without an extension and files whose
  extension is not in the finite text/source list are visible without a user
  needing to discover a secondary filter.
- `FileDialogSurface` remains the single owner of native open/folder/save
  selection. The file-vs-folder distinction, `getOpenFileName` call,
  `getExistingDirectory` call, async admission, document decoding, and error
  policy are unchanged.
- The change broadens what the user can select; it does not bypass path
  containment, execute content, or change the document service's decoding
  behavior.

## Evidence

- `D242-AST=PASS`
- `D242-COMPILEALL=PASS`
- `D242-RUFF=PASS`
- `D242-FORMAT=PASS`
- `D242-FILE-FILTER-CONTRACT=PASS`
- `D242-QT-WILDCARD-CONTRACT=PASS`
- `D242-EXTENSIONLESS-REGRESSION-PROBE=PASS`
- `D242-SOURCE-STARTUP-DIAGNOSTIC=PASS`
- `D242-PACKAGE-IDENTITY=PASS`
- `D242-FROZEN-RESOURCES=PASS`
- `D242-PYINSTALLER-WARNING-SCOPE=PASS`

## Simplification assessment

`D242-SIMPLIFICATION-ASSESSMENT=PASS`: reordering one canonical localized
filter list removes the visibility defect without adding a picker wrapper,
extension registry, or duplicate filter policy. The existing shared key keeps
open/save localization cohesive. No unrelated cleanup or error handling was
removed.

## Architecture consultation and independent review

- Plato the 7th / Luna max D242 architecture window: `NO_CONCLUSION` after a
  bounded wait.
- Poincare the 7th / Luna max narrowed architecture window: `NO_CONCLUSION`
  after a bounded wait.
- Raman the 7th / Luna max independent review window: `NO_CONCLUSION` after a
  bounded wait; the window was closed without treating silence as approval.
- Parent review: `PASS`; simplification: `PASS`.

## Limits

No EXE/Qt launch, updater/installer/registry operation, unit-test asset,
mock, fixture, harness, worktree, or test-only asset was executed or created.
The QtCore wildcard probe does not prove native dialog painting, selection,
or Windows shell behavior.
