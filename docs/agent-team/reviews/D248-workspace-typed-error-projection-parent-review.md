# D248 parent review — workspace typed-error projection

## Decision

`PASS` with operational limits.

## Findings

- The change is limited to the existing workspace presentation error seam and
  one coordinator forwarding line.
- Typed navigation exceptions now reach the same i18n category resolver used
  by document errors; invalid-result string messages retain their prior path.
- `en-US` remains equivalent to the prior string conversion.
- Operation tracking, loading state, session-restore completion, workspace
  service ownership, notifications, and page-retention policy are unchanged.
- No new service, callback, persistence field, translator, or cross-layer
  dependency was introduced.

## Evidence

- `D248-WORKSPACE-TYPED-ERROR-ROUTING=PASS`
- `D248-WORKSPACE-CHINESE-ERROR=PASS`
- `D248-WORKSPACE-ENGLISH-COMPATIBILITY=PASS`
- `D248-COMPILEALL=PASS`
- `D248-RUFF=PASS`
- `D248-FORMAT=PASS`
- `D248-ARCHIVE-ESSENTIALS=PASS outer=166 inner=261`
- `D248-PACKAGE-IDENTITY=PASS`; root/dist match, 38,576,424 bytes

## Simplification assessment

`D248-SIMPLIFICATION-ASSESSMENT=PASS`: forwarding the existing exception and
branching once at the workspace presentation endpoint is the smallest
behavior-preserving correction. A workspace-specific error catalog or new
error adapter would duplicate the shared boundary and increase coupling.

## Review roles

- `Dewey the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Beauvoir the 7th / Luna max` independent review window: `NO_CONCLUSION`
  after a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, unit-test asset, mock, fixture, harness,
worktree, registry, installer, updater, or clean-machine run was performed.
Static and packaged archive checks cannot prove native Windows rendering,
permissions, or startup behavior.
