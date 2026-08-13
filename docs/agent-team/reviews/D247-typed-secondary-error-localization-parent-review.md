# D247 parent review — typed secondary error localization

## Decision

`PASS` with operational limits.

## Findings

- The two edits are limited to presentation error forwarding in
  `MainWindow`; application services and contracts are unchanged.
- Passing `Exception` preserves the existing `MessageSurface` type contract
  and enables the already-established Chinese filesystem/codec localization
  path.
- English output remains equivalent because `localize_exception()` returns
  `str(error)` for `en-US`.
- No new translator, callback, service, state field, or ownership edge was
  introduced.

## Evidence

- `D247-TYPED-ERROR-BOUNDARY=PASS`
- `D247-SETTINGS-ERROR-LOCALIZATION=PASS`
- `D247-REPLACE-ERROR-LOCALIZATION=PASS`
- `D247-COMPILEALL=PASS`
- `D247-RUFF=PASS`
- `D247-FORMAT=PASS`
- `D247-ARCHIVE-ESSENTIALS=PASS outer=166 inner=261`
- `D247-PACKAGE-IDENTITY=PASS`; root/dist match, 38,577,691 bytes

## Simplification assessment

`D247-SIMPLIFICATION-ASSESSMENT=PASS`: removing the two premature
`str(error)` conversions is the smallest behavior-preserving correction. A
new typed error adapter or duplicated localization table would add coupling
without improving the existing boundary.

## Review roles

- `Volta the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Euclid the 7th / Luna max` independent review window: `NO_CONCLUSION`
  after a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, unit-test asset, mock, fixture, harness,
worktree, registry, installer, updater, or clean-machine run was performed.
Static and packaged archive checks cannot prove native Windows rendering or
startup behavior.
