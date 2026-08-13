# D246 parent review — startup path stat routing

## Decision

`PASS` with operational limits.

## Findings

- The change is confined to the existing startup-path routing method and its
  standard-library import.
- A single `Path.stat()` preserves missing/broken-link errors for the existing
  `OSError` warning boundary instead of silently treating them as a generic
  false predicate result.
- `S_ISDIR` and `S_ISREG` preserve the prior directory/regular-file split;
  `stat()` follows symlinks as the old predicates did.
- Special files remain rejected, and the existing admission, busy barrier,
  startup queue order, and async document/workspace boundaries are unchanged.
- No new service, callback, worker, storage format, Qt API, or file-open path
  was introduced.

## Evidence

- `D246-STARTUP-STAT-ROUTING=PASS`
- `D246-STARTUP-ERROR-CONTEXT=PASS`
- `D246-SOURCE-GATE=PASS`
- `D246-COMPILEALL=PASS`
- `D246-RUFF=PASS`
- `D246-FORMAT=PASS`
- `D246-ARCHIVE-ESSENTIALS=PASS` with outer=166 and inner=261 entries
- `D246-PACKAGE-IDENTITY=PASS`; root/dist match, 38,576,264 bytes

## Simplification assessment

`D246-SIMPLIFICATION-ASSESSMENT=PASS`: one `stat()` call and two standard
library mode predicates are the smallest compatible correction. Adding a new
path-classification service or duplicating error handling would increase
coupling without improving the startup boundary.

## Review roles

- `Boole the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Maxwell the 7th / Luna max` independent review window: `NO_CONCLUSION`
  after a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, unit-test asset, mock, fixture, harness,
worktree, registry, installer, updater, or clean-machine run was performed.
Static mode routing cannot prove a native Windows delete/replace race.
