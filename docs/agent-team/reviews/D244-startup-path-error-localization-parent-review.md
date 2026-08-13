# D244 parent review — startup-path error localization

## Decision

`PASS` with operational limits.

## Findings

- The source change is confined to the existing `localize_message()` prefix
  table in `src/quillforge/presentation/i18n.py`.
- The two new prefixes exactly match the two literals emitted by
  `MainWindow._drain_startup_paths()`; no caller or port changes are needed.
- English output is unchanged. Chinese translation is prefix-only, so paths,
  filenames, and platform exception details remain available for diagnosis.
- StatusSurface and MessageSurface already call `localize_message()`; the
  existing locale refresh path therefore covers the new messages without a
  second localization owner.
- Startup parsing, explicit-file admission, asynchronous document loading,
  folder routing, and package entrypoint behavior are unchanged.

## Evidence

- `D244-COMPILEALL=PASS`
- `D244-RUFF=PASS`
- `D244-FORMAT=PASS`
- `D244-STARTUP-PATH-LOCALIZATION=PASS`
- `D244-ARCHIVE-__main__=PASS`
- `D244-ARCHIVE-quillforge-app=PASS`
- `D244-ARCHIVE-quillforge-composition=PASS`
- `D244-ARCHIVE-qwindows-dll=PASS`
- `D244-ARCHIVE-PyQt6=PASS`
- `D244-PACKAGE-IDENTITY=PASS`; root/dist match, 38,576,104 bytes

## Simplification assessment

`D244-SIMPLIFICATION-ASSESSMENT=PASS`: the existing prefix table is the
smallest compatible extension; no new helper, locale callback, or coordinator
dependency was introduced. The two mappings remove a real leak without
reformatting or refactoring unrelated messages.

## Architecture consultation and independent review

- Pasteur the 7th / Luna max architecture window: `NO_CONCLUSION` after a
  bounded wait.
- Cicero the 7th / Luna max independent review window: `NO_CONCLUSION` after
  a bounded wait; the window was closed without treating silence as approval.
- Parent review: `PASS`; simplification: `PASS`.

## Limits

No EXE/Qt launch, updater/installer/registry operation, unit-test asset, mock,
fixture, harness, worktree, or test-only asset was executed or created. The
source localization probe does not prove native startup or Windows shell file
association behavior.
