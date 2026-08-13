# ADR-0195: active-document tab emphasis

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-75 / ARCH-133

## Context

The generic `QTabBar::tab:selected` selector carries a left accent cue, but
the more specific `QTabBar#documentTabBar::tab:selected` selector replaces
`border-color` and therefore suppresses that left-edge color for the actual
document tab rail. The selected tab still has a bottom accent, but active
document scanning is weaker than the authored state hierarchy intends.

## Decision

Keep `presentation/theme.py` as the single QSS owner and add only
`border-left-color: {colors.accent_alt}` to the existing selected and selected-
hover document-tab selectors. The existing one-pixel border width, surface,
bottom accent, text, font weight, focus, disabled, close-button, tab metrics,
signals, locale, motion, and application policy remain unchanged.

## Alternatives rejected

- Adding a new active-tab widget, marker, or layout item would change the tab
  object tree for a specificity-only visual gap.
- Increasing border width or changing padding could alter tab geometry and
  close-button alignment; the fix keeps existing widths.
- Hard-coded colors or a local stylesheet would bypass the centralized theme
  and accent token contract.

## Review and evidence

Kuhn the 5th / Luna max was assigned the architecture assessment and returned
no conclusion in the bounded window. Pascal the 5th / Luna max was assigned
the independent read-only review and also returned no conclusion. No child
PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because two existing selectors receive one token-derived color property
and no new visual structure.

Authorized non-destructive evidence:

- `UI75-TAB-HIGHLIGHT-PROBE=PASS:12 theme/accent projections`
- `UI75-STATE-PRESERVATION-PROBE=PASS`
- `UI75-PRESENTATION-AUDIT=PASS`
- `UI75-COMPILEALL=PASS`
- `UI75-RUFF=PASS`
- `UI75-FORMAT=PASS`
- `UI75-PACKAGE-BUILD=PASS`
- `UI75-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `0479DD015FE188C66759C9B7B3BDC828C0BEAF6BD80F3B87501065ECA8E57250`
- bytes: `38543443`
- source revision: `tree-sha256:0d06b3c925489a64dc62091eea8c1b463662cd21c315bd954a993305c2f1bb5d`

Public-source applicability is Python 3.12/PyQt6 centralized QSS; embedded
C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or certification/compliance claim.

## Limits

Static QSS projection does not prove native style-engine specificity, final
tab metrics, font/DPI rendering, accessibility, runtime startup, clean-machine
or cross-machine behavior, signing, installer/update, legal clearance,
support ownership, or release readiness. Those gates remain open under the
active no-launch/no-release authorization boundary.
