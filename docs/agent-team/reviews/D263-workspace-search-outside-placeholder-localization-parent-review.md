# D263 parent review — workspace-search outside-workspace placeholder localization

## Decision

`PASS` with operational limits.

## Findings

- The change is isolated to the existing workspace-search presentation
  catalog and path-display fallback.
- English retains `<outside selected workspace>` and Chinese projects
  `<位于所选工作区之外>` with the same angle-bracket placeholder semantics.
- The existing `_refresh_diagnostic_items()` locale-refresh path reprojects the
  placeholder; no translated string is stored in the application result.
- Path containment, result data, diagnostic reasons, ordering, and expansion
  state are unchanged.

## Evidence

- `D263-OUTSIDE-WORKSPACE-LOCALIZATION=PASS locales=2 placeholder=preserved`
- `D263-DIAGNOSTIC-REFRESH-WIRING=PASS locale_refresh=1`
- `D263-COMPILEALL=PASS`, `D263-RUFF=PASS`, `D263-FORMAT=PASS`
- `D263-SOURCE-DIAGNOSTIC-EXIT=0`
- `D263-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166`
- `D263-ARCHIVE-PYZ=PASS entries=261 modules=4`
- `D263-PYINSTALLER-WARNING-SCOPE=PASS lines=29`
- `D263-PACKAGE-IDENTITY=PASS`; root/dist match, 38,583,245 bytes

## Simplification assessment

`D263-SIMPLIFICATION-ASSESSMENT=PASS`: one catalog value at the existing
fallback call is smaller than adding translated state to search results or
duplicating the diagnostic refresh path. The safety boundary remains owned by
the existing path projection.

## Review roles

- `Herschel the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- Independent-review window: `NO_CONCLUSION` after the bounded architecture
  review workflow; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native search dialog, registry, installer, updater, unit-test
asset, mock, fixture, harness, worktree, or clean-machine run was performed.

