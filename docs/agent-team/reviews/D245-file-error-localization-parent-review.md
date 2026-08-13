# D245 parent review — typed file-error localization

## Decision

`PASS` with operational limits.

## Findings

- The change keeps exception ownership in the existing document open/save
  coordinators and uses the existing `MessageSurface` boundary for projection.
- `localize_exception()` is presentation-only and preserves the English
  `str(error)` contract; it does not catch, mutate, or replace exceptions.
- Common filesystem errors retain their path, encode/decode errors retain
  encoding and position, and generic Unicode/unknown details fall back to the
  existing message mapper without a second exception.
- The external document-change prefix is routed through the existing ordered
  prefix table, so the prior conflict diagnostic remains localizable.
- No new service, worker, callback, storage format, Qt API, or file-open path
  was introduced.

## Evidence

- `D245-EXCEPTION-LOCALIZATION=PASS`
- `D245-UNICODE-FALLBACK=PASS`
- `D245-CONFLICT-LOCALIZATION=PASS`
- `D245-SOURCE-GATE=PASS`
- `D245-COMPILEALL=PASS`
- `D245-RUFF=PASS`
- `D245-FORMAT=PASS`
- `D245-ARCHIVE-ESSENTIALS=PASS` with outer=166 and inner=261 entries
- `D245-PACKAGE-IDENTITY=PASS`; root/dist match, 38,576,075 bytes

## Simplification assessment

`D245-SIMPLIFICATION-ASSESSMENT=PASS`: one typed exception projection helper
and one existing modal-surface branch are the smallest compatible extension.
Moving translations into the file store or duplicating them across open/save
coordinators would increase coupling and create multiple localization owners.

## Review roles

- `Ohm the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Boyle the 7th / Luna max` independent review window: `NO_CONCLUSION` after
  a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, unit-test asset, mock, fixture, harness,
worktree, registry, installer, updater, or clean-machine run was performed.
Static exception projection cannot prove native Windows error text or dialog
layout.
