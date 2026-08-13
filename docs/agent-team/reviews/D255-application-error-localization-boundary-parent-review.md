# D255 parent review — application-error localization boundary

## Decision

`PASS` with operational limits.

## Findings

- The source change is isolated to the shared presentation localizer and does
  not alter application validation, worker scheduling, or UI ownership.
- English locale behavior remains an early return, preserving existing
  diagnostics for English users.
- Exact application messages and finite wrapper prefixes avoid translating
  internal invariants by guesswork.
- Nested details are localized through the same mapper while paths, document
  names, plugin IDs, and unknown text remain present.
- The open-path and autosave patterns retain their dynamic portions; no
  executable or Qt startup path was added.

## Evidence

- `D255-LOCALIZATION-BOUNDARY=PASS cases=7`
- `D255-COMPILEALL=PASS`, `D255-RUFF=PASS`, `D255-FORMAT=PASS`
- `D255-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166`
- `D255-ARCHIVE-PYZ=PASS entries=261`
- `D255-PACKAGE-IDENTITY=PASS`; root/dist match, 38,581,105 bytes

## Simplification assessment

`D255-SIMPLIFICATION-ASSESSMENT=PASS`: keeping the behavior in one existing
presentation localizer avoids a cross-coordinator contract change. A larger
exception hierarchy or a translation dependency in application/domain would
increase coupling without improving this user-visible boundary.

## Review roles

- `Zeno the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Ptolemy the 7th / Luna max` independent-review window: `NO_CONCLUSION`
  after a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, registry, installer, updater, unit-test
asset, mock, fixture, harness, worktree, or clean-machine run was performed.
Static localizer and archive evidence cannot prove native startup success.
