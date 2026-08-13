# D181 parent review: status accessibility localization

## Scope

Reviewed `presentation.i18n`, `presentation.status_bar.StatusRail`, and
`presentation.status_surface.StatusSurface`, including construction,
`set_locale`, `set_phase`, visible notification reprojection, and the
`PresentationLocaleCoordinator` call path.

## Findings

No blocking finding was identified. The new keys remain presentation-only;
the status rail normalizes locale before projecting names and phase
descriptions, and StatusSurface refreshes the notification name alongside the
existing visible message localization. Dynamic phase text is formatted at the
status boundary, so application coordinators retain their existing contracts.

The residual risk is native screen-reader output, platform accessibility tree
behavior, font/DPI metrics, and actual runtime rendering; those checks remain
unrun because GUI startup is disallowed.

## Decision

`PASS` with explicit native-runtime limits.

## Simplification assessment

`PASS`. The implementation reuses the existing locale refresh methods and the
existing `tr` catalog. It adds no helper, state store, or duplicate message
translation pipeline and does not weaken error handling.

## Gate record

Python 3.12/PyQt6 presentation contracts are applicable. Public CloudWeGo
material is a non-binding engineering reference. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable; no certification claim is
made.

## Evidence

- `D181-ACCESSIBILITY-LOCALE-PROBE=PASS locales=2 labels=4`
- `D181-SOURCE-WIRING-PROBE=PASS`
- `D181-COMPILE-RUFF-FORMAT=PASS`
- `D181-CHECK=PASS`
- `D181-PACKAGE-BUILD=PASS`
- `D181-PACKAGE-IDENTITY-PROBE=PASS`
- `D181-MANIFEST-TRACEABILITY-PROBE=PASS`
- `D181-VERIFY-HANDOFF=PASS`
- `D181-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`
