# ADR-0254: Workspace tree disabled-state hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D207 / UI-109 / ARCH-192

## Context

When Workspace refreshes an already-open directory, `WorkspacePanel` keeps
the populated tree visible while `set_loading(True)` disables it. The tree
had normal, focus, hover, selected, and selected-disabled item rules, but no
disabled projection for the visible tree container. During loading, the
content could therefore retain an actionable surface even though navigation
was temporarily owned by the provider request.

## Decision

Add one scoped `QTreeWidget#workspaceTree:disabled` rule after the existing
workspace-tree focus rule. It uses `surface_2`, `border`, and `text_muted`
while inheriting the existing radius, padding, item rules, and layout. The
change is presentation-only; loading, tree data, selection, navigation,
cancellation, signals, and provider policy remain unchanged.

## Preserved invariants

- `WorkspacePanel.set_loading()` still disables the same controls and keeps
  the existing populated tree visible while an already-open directory is
  refreshed.
- Normal tree, focus, hover, selected, selected-inactive, and item-disabled
  projections remain unchanged when the tree is available.
- The selected-disabled item selector remains the more specific row-level
  state; the new rule only establishes the container surface and text.
- Theme/accent resolution remains the only color source, with the existing
  12 projections retaining at least 4.5:1 disabled text/surface contrast.

## Review and applicability

The architecture consultation (`Boyle the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Parfit the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one container-level rule closes the visible loading gap without duplicating
row rules or adding a workspace state service.

The applicable public first-party source is Qt's Style Sheets Reference,
which documents selector pseudo-states such as `:disabled`:
https://doc.qt.io/qt-6/stylesheet-reference.html. This is a Python 3.12 /
PyQt6 presentation-only change, not embedded C/C++, MCU, RTOS, or
manufacturer-requirement work; the mandatory embedded enterprise workflow is
therefore not applicable to this source slice. Public CloudWeGo material
remains engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Evidence and limits

- `D207-DISABLED-STATE-SOURCE-PROBE=PASS`
- `D207-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`
- `D207-COMPILEALL=PASS`
- `D207-RUFF=PASS`
- `D207-FORMAT=PASS`
- `D207-PRESENTATION-AUDIT=PASS`
- `D207-PACKAGE-BUILD-PS51=PASS`
- `D207-PACKAGE-BUILD-PS7=PASS`
- `D207-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native QSS painting,
accessibility tree, DPI, live filesystem/provider refresh, clean-machine,
cross-machine, signing, installer, updater, legal, support, or release-owner
evidence was performed. Native Qt selector parsing/painting and live loading
interleavings remain open. No unit-test asset was created or run.

