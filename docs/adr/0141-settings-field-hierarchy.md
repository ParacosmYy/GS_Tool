# ADR-0141: Settings field hierarchy

- **Status:** accepted-with-limits; UI-55 bounded visual slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The settings dialog already exposes language, theme, accent, interface font
and size, editor font and size, wrapping, line numbers, and motion controls,
plus the pending-appearance preview. Its controls did not all have stable
semantic identities, so centralized QSS could not consistently distinguish
field labels and compact editor-option rows.

## Decision

Give the settings controls stable presentation identities and mark form labels
with the `settingsRole="fieldLabel"` semantic property. Add only
`QDialog#settingsDialog`-scoped QSS for field-label hierarchy, language and
appearance/editor accent grouping, and option-row hover/checked/disabled
states. All colors remain derived from the canonical theme tokens.

## Invariants

1. `settings_snapshot()`, `set_locale()`, preview signals, Save/Cancel,
   SettingsService, SettingsSaveCoordinator, and MainWindow ownership are
   unchanged.
2. The new selectors are scoped to `settingsDialog`; other dialogs and
   application controls do not consume the settings option-row styles.
3. Foregrounds/backgrounds use existing readable text and surface tokens;
   disabled rows remain muted and selected rows remain distinguishable.
4. No settings value, domain contract, persistence, animation policy, or
   editor behavior moves across a module boundary.

## Alternatives considered

- **Style all QCheckBox/QLabel controls globally:** rejected; it would alter
  unrelated dialogs and weaken local ownership.
- **Add inline per-widget styles:** rejected; it would bypass centralized
  theme token resolution and make theme changes inconsistent.
- **Change settings layout or data flow:** rejected; the issue is semantic
  presentation identity, not settings behavior or form composition.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance gate is `N/A`; the embedded workflow and simplifier were reviewed
for applicability and no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect: Kant the 4th / Luna max; bounded read-only wait timed out and the
  agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS is
  claimed.
- Initial independent review: Popper the 4th / Luna max returned `CONCERNS`,
  identifying missing explicit scoped focus/checked-focus selectors for the
  option rows and missing named-control disabled/focus guarantees.
- Parent remediation: added settings-scoped checkbox focus/checked-focus
  selectors and appearance/editor combo/spin hover/focus/disabled selectors,
  retaining accent rails, popup rules, and token-only colors.
- Follow-up independent review: Euclid the 4th / Luna max; bounded read-only
  wait timed out and the agent was closed. Status is `NO_CONCLUSION`; no
  follow-up child PASS is claimed.
- Parent source review after remediation: PASS for semantic identity coverage,
  selector scope, token-only colors, focus/disabled closure, and preservation
  of settings value/signal behavior.
- Simplification assessment: PASS. The smallest complete change is semantic
  identities plus centralized scoped selectors; no new visual subsystem,
  layout abstraction, or runtime state is introduced.

## Verification target and limits

- Required and authorized here: source/selector identity probe, focus/disabled
  closure probe, compileall, Ruff, format, package identity, handoff/register/
  index synchronization, and expected release NO-GO evidence.
- Not proven: native Qt rendering, actual contrast measurement, installed-font
  fallback, DPI, screen-reader output, QApplication startup, clean-machine/
  cross-machine behavior, signing, installer, legal, support, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
