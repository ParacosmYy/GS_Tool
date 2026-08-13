# ADR-0199: inactive-selection contrast closure

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-76 / ARCH-137

## Context

The inactive selected-row selector for the workspace tree and list views used
`text_secondary` on the shared `pressed` background. In the Paper-Sand theme
that combination measured only `3.69:1`, weakening the very highlight users
need to locate the selected row when focus is elsewhere.

## Decision

Keep `presentation/theme.py` as the centralized QSS owner and change only the
inactive selected-row foreground from `colors.text_secondary` to
`colors.text_primary` for:

`QTreeWidget#workspaceTree::item:selected:!active` and
`QListWidget::item:selected:!active`.

The existing `pressed` background, strong border, accent-left cue, disabled
state, selection model, focus/activation behavior, row geometry, widget IDs,
locale, font, motion, token values, and application policy remain unchanged.

## Alternatives rejected

- Changing the `pressed` token would affect many unrelated interaction states.
- Adding a new inactive-selection color token would broaden the palette for a
  one-selector contrast defect.
- Adding a widget marker or signal would change behavior for a presentation-
  only foreground problem.

## Review and evidence

Beauvoir the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Popper the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion. No
child PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because one existing selector endpoint changes to the already-authored
primary text token.

Authorized non-destructive evidence:

- `UI76-INACTIVE-SELECTION-PROBE=PASS:12 theme/accent projections`
- `UI76-STATE-PRESERVATION-PROBE=PASS`
- `UI76-PRESENTATION-AUDIT=PASS`
- `UI76-COMPILEALL=PASS`
- `UI76-RUFF=PASS`
- `UI76-FORMAT=PASS`
- `UI76-PACKAGE-BUILD=PASS`
- `UI76-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `0B4A8AC8BDFD0EDE6751B29F6F9A3B5EE05C3FB3CB0D2D83F54F6B3F33B19A03`
- bytes: `38544446`
- source revision: `tree-sha256:90cd6205ab6d0c8862f8f09c0d39e3ecd67a3ec04434bff11771834345b870a6`

Public-source applicability is Python 3.12/PyQt6 centralized QSS;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

Static QSS and contrast projections do not prove native style-engine rendering,
focus state under every Windows style/DPI combination, accessibility,
runtime startup, clean-machine or cross-machine behavior, signing, installer,
update, legal clearance, support ownership, or release readiness. Those gates
remain open under the active no-launch/no-release authorization boundary.
