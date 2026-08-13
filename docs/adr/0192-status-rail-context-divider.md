# ADR-0192: status-rail context divider

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-74 / ARCH-130

## Context

The status rail already separated permanent local context from the lifecycle
phase semantically, but the context label and phase pill had no visible edge
between them. On a dense shell this made `LOCAL`/`本地` and `READY`/`WORKING`
scan as one undifferentiated block.

## Decision

Keep `presentation/theme.py` as the single visual owner and add only a
token-driven right border and compact right padding to the existing
`QLabel#statusContext` selector. The border uses `ThemeColors.border`; no new
color, widget, property, signal, locale string, timer, phase state, or layout
owner is introduced. The four `statusPhase` states and their semantic
foreground/background tokens remain unchanged.

## Alternatives rejected

- Adding a separator widget would change the status-rail object tree and add
  layout/runtime behavior for a purely presentational distinction.
- Styling the phase states more strongly would risk conflating context with
  lifecycle semantics and was broader than the reported scanability gap.
- Hard-coded colors or a local stylesheet would bypass the central theme token
  contract.

## Review and evidence

Laplace the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Hegel the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion. No
child PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because the existing selector and border token are the smallest complete
visual change.

Authorized non-destructive evidence:

- `UI74-STATUS-CONTEXT-PROBE=PASS:12 theme/accent projections`
- `UI74-QSS-CENTRALIZATION-PROBE=PASS`
- `UI74-COMPILEALL=PASS`
- `UI74-RUFF=PASS`
- `UI74-FORMAT=PASS`
- `UI74-CHECK=PASS`
- `UI74-VERIFY-HANDOFF=PASS`
- `UI74-PACKAGE-BUILD=PASS`
- `UI74-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `DEF6260736733918686BB2F7F96F454671DBB1B1B31AC02559DF999012B7D4D8`
- bytes: `38539526`
- source revision: `tree-sha256:04722069066e1396ecb316510ebf85f66c72b04871ae88c0d958815c4c903998`

Public-source applicability is Python 3.12/PyQt6 centralized QSS; embedded
C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or a certification/compliance claim.

## Limits

Static stylesheet/token probes do not prove native QSS specificity/rendering,
font/DPI metrics, accessibility, runtime startup, clean-machine or
cross-machine behavior, signing, installer/update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.

