# ADR-0189: status notification pill

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-72 / ARCH-127

## Context

The shell notification label already projected explicit `info`, `success`,
`warning`, and `error` states, but its normal state was transparent text with a
hard left edge. That made transient feedback visually disappear into the
status bar and left the shell less cohesive than the newer rounded surfaces.

## Decision

Keep `theme.py` as the sole visual owner and style the existing
`QLabel#statusMessage` as a compact token-driven pill: `surface_1` base,
readable border, rounded corners, compact padding, and a semantic left color
rail. Existing success/warning/error backgrounds and contrast-safe foreground
tokens remain unchanged; no widget object, signal, timer, locale, notification
level, or application policy changes.

## Alternatives rejected

- Adding icons or a new notification widget would change the existing status
  surface and introduce extra layout/runtime risk.
- Hard-coded colors would bypass the theme/accent token contract.
- A full status-bar redesign would be broader than the reported visibility and
  hierarchy gap.

## Review and evidence

Dewey the 5th / Luna max was assigned the architecture assessment and returned
no conclusion in the bounded window. Schrodinger the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion after
two short waits. No child PASS is claimed. Parent review is `PASS`;
simplification assessment is `PASS` because the existing selector and tokens
are the smallest complete visual change.

Authorized non-destructive evidence:

- `UI72-STATUS-PILL-PROBE=PASS:12 theme/accent projections`
- `UI72-STATUS-SEMANTIC-STATE-PROBE=PASS`
- `UI72-QSS-CENTRALIZATION-PROBE=PASS`
- `UI72-COMPILEALL=PASS`
- `UI72-RUFF=PASS`
- `UI72-FORMAT=PASS`
- `UI72-CHECK=PASS`
- `UI72-VERIFY-HANDOFF=PASS`
- `UI72-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch/traceability checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `81281CCD003FF2A55E4FD1B71F308FAD947B5592BFDD9C0401A3DCA3BBC65AD3`
- bytes: `38536771`
- source revision: `tree-sha256:afb2359d6663d18f2f3f0a49bc818804ad9527982ea8119a1a96c9a6681b3c3d`

Public-source applicability is Python 3.12/PyQt6 centralized QSS; embedded
C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or a certification/compliance claim.

## Limits

Static stylesheet/token probes do not prove native QSS rendering, font/DPI
metrics, accessibility, runtime startup, clean-machine or cross-machine
behavior, signing, installer/update, legal clearance, support ownership, or
release readiness. Those gates remain open under the active no-launch/no-release
authorization boundary.
