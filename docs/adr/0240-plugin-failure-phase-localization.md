# ADR-0240: Plugin failure phase localization boundary

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D192 / UI-102 / ARCH-178

## Context

The plugin failure notification already translated its stable shell through
`presentation.i18n`, but phase values emitted by the existing runtime seam
(`activate`, `deactivate`, `command`, and `event`) appeared in English inside
Simplified Chinese errors. Plugin IDs and exception details are intentionally
diagnostic data and must remain intact.

## Decision

Keep the translation at `_localize_plugin_failure`, the existing presentation
adapter selected by the message shape. Add one bounded phase map for the four
known runtime values. Unknown phase values fall back to the original text so
new runtime diagnostics remain visible and are never guessed or silently
discarded.

## Preserved invariants

- English (`en-US`) returns the original message unchanged.
- Plugin IDs, delimiters, and error details remain verbatim.
- Only the known stable phase vocabulary is translated; arbitrary details are
  not parsed as user-facing labels.
- Plugin manager lifecycle, event bus publication, trust/enablement,
  containment, command refresh, notification severity, and persistence are
  unchanged.
- No second locale service or styling system is introduced.

## Review and applicability

The architecture consultation (`Bohr the 6th / Luna max`) and independent
review (`Ohm the 6th / Luna max`) both timed out within their bounded windows;
both are recorded as `NO_CONCLUSION`. Parent review is `PASS`, and the
behavior-preserving simplification assessment is `PASS`.

This is a Python 3.12/PyQt6 presentation change. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable; the mandatory embedded
enterprise workflow is therefore recorded as not applicable to this source
slice. Public CloudWeGo material is an engineering reference only; this ADR
makes no private ByteDance standard, certification, or compliance claim.

## Evidence and limits

- `D192-PLUGIN-PHASE-I18N-PROBE=PASS`
- `D192-COMPILE-RUFF-FORMAT=PASS`
- `D192-PRESENTATION-AUDIT=PASS`
- `D192-PACKAGE-BUILD=PASS`
- `D192-PACKAGE-IDENTITY-PROBE=PASS`

Native Qt rendering, accessibility-tree output, DPI, screenshot review,
GUI/EXE startup, clean-machine, cross-machine, legal, signing, installer,
updater, support, and release-owner evidence remain open. Release verification
remains `no-go` under the current no-launch and external-gate policy.
