# D253 parent review — workspace-entry error locale refresh

## Decision

`PASS` with operational limits.

## Findings

- The change remains inside `WorkspacePanel`, the existing workspace
  presentation boundary.
- The raw `WorkspaceEntry.error` source is retained per item and translated
  only while composing the visible tooltip.
- Disabled inaccessible rows now refresh their tooltip on locale changes;
  their disabled state, kind, path, and non-activation behavior are preserved.
- Stable provider-owned reasons are covered and unknown filesystem/provider
  text remains unchanged, preserving diagnostic information.
- File first-click activation, folder double-click/Enter activation, provider
  ordering/bounds, and application workspace policy are untouched.

## Evidence

- `D253-WORKSPACE-ERROR-SOURCE-RETENTION=PASS`
- `D253-WORKSPACE-ERROR-TOOLTIP-LOCALIZATION=PASS`
- `D253-WORKSPACE-DISABLED-LOCALE-REFRESH=PASS`
- `D253-WORKSPACE-UNKNOWN-ERROR-FALLBACK=PASS`
- `D253-COMPILEALL=PASS`, `D253-RUFF=PASS`, `D253-FORMAT=PASS`
- `D253-ARCHIVE-OUTER=PASS entries=166`,
  `D253-ARCHIVE-PYZ=PASS entries=261`
- `D253-PACKAGE-IDENTITY=PASS`; root/dist match, 38,580,637 bytes

## Simplification assessment

`D253-SIMPLIFICATION-ASSESSMENT=PASS`: one private item source role, one
shared localizer call at creation, and one refresh branch are the smallest
behavior-preserving correction. Moving translation into the provider or
creating a second workspace error model would increase coupling.

## Review roles

- `Hypatia the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Peirce the 7th / Luna max` independent review window: `NO_CONCLUSION` after
  a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native dialog, registry, installer, updater, unit-test
asset, mock, fixture, harness, worktree, or clean-machine run was performed.
Static and frozen-archive checks cannot prove native rendering or startup.
