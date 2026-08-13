# D257 parent review — built-in plugin status-name localization

## Decision

`PASS` with operational limits.

## Findings

- The change is isolated to the existing presentation localization boundary
  and plugin-status projection.
- Only the stable built-in plugin ID is translated; external plugin metadata
  retains its supplied fallback name.
- The existing `set_locale` path reuses `_format_status`, so locale refresh
  includes the plugin name without adding a second refresh mechanism.
- The public plugin manifest/status contract and built-in plugin behavior are
  unchanged.

## Evidence

- `D257-PLUGIN-NAME-BOUNDARY=PASS cases=3`
- `D257-COMPILEALL=PASS`, `D257-RUFF=PASS`, `D257-FORMAT=PASS`
- `D257-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166`
- `D257-ARCHIVE-PYZ=PASS entries=261 modules=4`
- `D257-PACKAGE-IDENTITY=PASS`; root/dist match, 38,582,333 bytes

## Simplification assessment

`D257-SIMPLIFICATION-ASSESSMENT=PASS`: one stable-ID resolver at the existing
presentation boundary is smaller and safer than changing plugin manifests,
adding a translated metadata field, or duplicating locale policy in the
built-in plugin. The explicit external fallback keeps the boundary narrow.

## Review roles

- `Hume the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Russell the 7th / Luna max` independent-review window: `NO_CONCLUSION`
  after a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, registry, installer, updater, unit-test
asset, mock, fixture, harness, worktree, or clean-machine run was performed.
