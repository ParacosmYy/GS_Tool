# D249 parent review — workspace error locale refresh

## Decision

`PASS` with operational limits.

## Findings

- The change is confined to `WorkspacePanel`'s existing presentation state.
- The original error source is retained, so a locale change can translate it
  again instead of retaining stale English text.
- A successful directory result and the beginning of a new load clear the
  obsolete error; the last good page is still not cleared by `show_error()`.
- Error status has explicit precedence over loading and normal directory
  status only while an error source is retained.
- No coordinator, domain, service, notification, persistence, or Qt policy
  moved across the existing boundary.

## Evidence

- `D249-ERROR-SOURCE-RETENTION=PASS`
- `D249-LOCALE-ERROR-REPROJECTION=PASS`
- `D249-SUCCESS-AND-LOAD-CLEARING=PASS`
- `D249-COMPILEALL=PASS`, `D249-RUFF=PASS`, `D249-FORMAT=PASS`
- `D249-ARCHIVE-OUTER=PASS entries=166`,
  `D249-ARCHIVE-INNER=PASS entries=261`
- `D249-PACKAGE-IDENTITY=PASS`;
  root/dist match, 38,578,143 bytes

## Simplification assessment

`D249-SIMPLIFICATION-ASSESSMENT=PASS`: one presentation-local source field
and one projection helper are the smallest behavior-preserving correction.
They avoid a second translator, a coordinator state machine, or a translated
text cache that would increase coupling and create stale-locale behavior.

## Review roles

- `Epicurus the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Feynman the 7th / Luna max` independent review window: `NO_CONCLUSION`
  after a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, registry, installer, updater, unit-test
asset, mock, fixture, harness, worktree, or clean-machine run was performed.
Static and frozen-archive checks cannot prove native rendering, filesystem
permission behavior, or Windows startup success.
