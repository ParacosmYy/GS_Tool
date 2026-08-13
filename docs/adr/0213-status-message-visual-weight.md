# ADR-0213: Status-message visual weight

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D164 / UI-77

## Context

The status-message surface already exposes localized info, success, warning,
and error states through one `QLabel#statusMessage` object-name contract. Its
base rule inherited the ordinary label weight, so transient feedback competed
with the status context and phase labels in the status rail.

## Decision

Set `font-weight: 600` on the base `QLabel#statusMessage` selector in the
centralized presentation stylesheet. Keep the existing state selectors,
foreground derivation, text and locale projection, timer cleanup, visibility,
tooltip, margins, padding, and status-rail layout unchanged.

This is a presentation-only refinement. `StatusSurface` remains responsible
for message text, state projection, and lifecycle; `MainWindow` and the
application layer retain notification policy.

## Alternatives considered

- Change every status state selector independently: rejected because it would
  duplicate a shared visual role and increase selector drift.
- Increase font size or change status-rail geometry: rejected because it could
  alter size hints, truncation, and layout at different UI font settings.
- Add a new widget or notification contract: rejected because the existing
  object-name and state contract is sufficient.

## Review and simplification

- Architect role: Faraday the 5th / Luna max; `PASS` for the minimal scoped
  presentation change.
- Independent review: Hubble the 5th / Luna max; `PASS` for state, lifecycle,
  localization, and QSS-scope preservation.
- Parent review: `PASS`; the single declaration is in the shared base rule and
  no behavior owner or state mapping changed.
- Simplification assessment: `PASS`; one shared declaration is the smallest
  implementation and introduces no new token, selector, adapter, or state.

## Authorized evidence

- `D164-AST-PROBE=PASS` for `theme.py` and `status_surface.py`.
- `D164-STATUS-MESSAGE-QSS-PROBE=PASS` for the base rule and all four state
  selectors.
- `D164-SINGLE-SCOPE-PROBE=PASS` confirming the declaration is confined to
  `QLabel#statusMessage`.
- `D164-INDEPENDENT-REVIEW=PASS`.
- `scripts/check.ps1` and package identity are recorded in the D164 handoff.
- No QApplication, EXE, screenshot, or native rendering was authorized.

## Public-source applicability

This is Python 3.12/PyQt6 presentation styling. Embedded C/C++, MCU/RTOS,
BSP/HAL, and manufacturer requirements are not applicable. Public CloudWeGo
material remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Limits

Static source evidence cannot establish native font availability, DPI metrics,
status-bar size hints, text elision, or rendered appearance across machines.
The change does not close runtime, clean-machine, signing, installer, legal,
support, or other release gates.
