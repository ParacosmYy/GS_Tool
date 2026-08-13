# ADR-0101: Recovery notification dynamic localization

- **Status:** accepted-with-limits; D76 / UI-49 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The shell already routes transient notifications through
`presentation.i18n.localize_message()`. The recovery-success message had a
dynamic document name followed by the stable English suffix
`; content remains unsaved`, so the default Chinese UI could still display a
mixed-language notification.

## Decision

Keep the recovery notification's source message and all recovery/document
policy unchanged. Extend the existing presentation localization boundary with
one bounded prefix/suffix shape check. For `zh-CN`, translate only the stable
suffix to `；内容仍未保存` and preserve the dynamic document name verbatim.
For `en-US`, retain the existing identity behavior and return the source
message unchanged.

## Invariants

1. Application and recovery services remain locale-free.
2. Dynamic document names, including Unicode and punctuation, remain visible
   and are not interpreted as format strings.
3. `en-US` returns the original notification exactly.
4. Notification severity, recovery state, persistence, and MainWindow policy
   remain unchanged.
5. The existing `localize_message()` seam remains the single presentation
   owner; no second translation service or parallel catalog is introduced.

## Alternatives considered

- **Change the recovery notification at the MainWindow call site:** rejected;
  it would duplicate locale policy in a coordinator and make locale refresh
  less consistent.
- **Move localization into RecoveryService:** rejected; application contracts
  must remain locale-free and diagnostic-safe.
- **Introduce a general message-template registry:** rejected; one bounded
  message shape does not justify a second abstraction or a new dependency.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Review and simplification

- Architect role: Lorentz the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Independent review: Feynman the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Parent source review covers correctness, readability, architecture,
  security, and performance and found no required change.
- Simplification assessment found no safer reduction: the two named boundary
  constants make the exact dynamic shape readable without adding a helper or
  changing the existing localization dispatch.

## Verification target and limits

- `D76-I18N-RECOVERY-PROBE=PASS` covers Chinese projection, `en-US` identity,
  Unicode names, and punctuation in a dynamic document name.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D76 handoff.
- Native Qt startup, live language switching, accessibility, DPI, font
  metrics, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner evidence remain unrun or open under the active
  no-launch/authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
