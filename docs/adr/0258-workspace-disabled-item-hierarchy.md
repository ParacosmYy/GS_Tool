# ADR-0258: Workspace disabled-item hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D211 / UI-113 / ARCH-196

## Context

`WorkspacePanel` marks inaccessible entries and the overflow marker as
disabled `QTreeWidgetItem` instances. The shared stylesheet supplied only
muted text for ordinary disabled items, so those rows could still scan like
available workspace entries. The populated tree already had a more explicit
selected-disabled state and a separate disabled container state during
provider refresh.

## Decision

Add one scoped `QTreeWidget#workspaceTree::item:disabled` rule in the central
presentation stylesheet. Reuse `ThemeColors.surface_2`, `border`,
`border_strong`, and `text_muted` to give ordinary disabled rows a subdued
surface, neutral left boundary, and semibold readable text. Keep the existing
`QTreeWidget#workspaceTree::item:selected:disabled` rule more specific so a
selected disabled row retains its selected hierarchy.

## Preserved invariants

- `WorkspacePanel._item_for()` remains the owner of the existing disabled
  state for inaccessible entries; the overflow marker remains disabled.
- File first-click, folder double-click, Enter/Return activation, item roles,
  locale refresh, icon refresh, loading/cancellation, and provider diagnostics
  remain unchanged.
- The disabled tree container rule remains separate from row-level disabled
  styling, and ordinary `QListWidget` rows do not inherit the new scoped rule.
- No delegate, workspace service, filesystem policy, application coordinator,
  or second styling system is introduced.

## Review and applicability

The architecture consultation (`Maxwell the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Kuhn the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one existing scoped selector is the smallest complete presentation correction.

The [Qt 6 Style Sheets Reference](https://doc.qt.io/qt-6/stylesheet-reference.html)
is the applicable public first-party source for stateful QSS selector
projection. This is a Python 3.12 / PyQt6 presentation-only change, not
embedded C/C++, MCU, RTOS, or manufacturer-requirement work; the mandatory
embedded enterprise workflow is therefore not applicable to this source
slice. Public CloudWeGo material is an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Evidence and limits

- `D211-WORKSPACE-DISABLED-SOURCE-PROBE=PASS`
- `D211-QSS-SPECIFICITY-SOURCE-PROBE=PASS`
- `D211-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`
- `D211-COMPILEALL=PASS`
- `D211-RUFF=PASS`
- `D211-FORMAT=PASS`
- `D211-PRESENTATION-AUDIT=PASS`
- `D211-PACKAGE-BUILD-PS51=PASS`
- `D211-PACKAGE-BUILD-PS7=PASS`
- `D211-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native item painting,
accessibility tree, DPI, live workspace refresh, clean-machine,
cross-machine, signing, installer, updater, legal, support, or release-owner
evidence was performed. No unit-test asset was created or run. Native QSS
specificity/painting and accessibility behavior remain open.
