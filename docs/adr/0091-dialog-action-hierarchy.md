# ADR-0091: Dialog action hierarchy

- **Status:** accepted-with-limits; D66 / UI-40 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The extension catalog, plugin status, workspace search, and settings dialogs
used the same generic button presentation even when their actions had
different consequences. This made primary, warning, and close/cancel actions
harder to scan and left the workspace-search/settings button boxes without a
stable visual rail.

## Decision

Keep action semantics in the existing presentation widgets and express their
visual roles through stable Qt `objectName` metadata:

- `primaryAction` marks approve, enable, and save actions;
- `warningAction` marks revoke and disable actions;
- `quietAction` marks close/cancel actions; and
- `QDialogButtonBox#dialogActions` provides the shared dialog action rail for
  workspace search and settings.

The centralized `presentation/theme.py` stylesheet owns the quiet-action
normal/hover/focus/pressed/disabled states and the action-rail boundary.
Existing primary and warning tokens are reused. No new widget, signal,
business state, i18n key, persistence field, or service boundary is added.

## Invariants

1. Dialog signals, button ordering, enablement predicates, locale refresh, and
   modal/non-modal behavior remain unchanged.
2. `primaryAction` and `warningAction` keep their existing contrast-safe token
   contracts, including the gold foreground correction from D55.
3. Quiet actions use `text_secondary` only on the default surface and
   `text_primary` on hover/focus/pressed surfaces; all three themes remain at
   least 4.5:1 in the source contrast probe.
4. The action rail is presentation-only; no dialog acquires application or
   persistence policy.
5. No runtime GUI launch, screenshot, accessibility-driver run, or test-only
   asset is required or performed under the active policy.

## Alternatives considered

- **Leave all dialogs on generic buttons:** rejected; it preserves the
  hierarchy defect reported by the user.
- **Create a shared dialog-action widget:** rejected; the existing Qt controls
  and centralized QSS are sufficient, and a new component would add coupling
  for one visual concern.
- **Move action decisions into theme or a dialog base class:** rejected;
  enablement, signals, and policy belong to each existing dialog.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- Source object-name/QSS contract probe and real-state contrast probe pass.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D66 handoff.
- The architect and independent review windows returned no conclusion; no
  child PASS is claimed.
- Native Qt rendering, keyboard traversal, accessibility, DPI/font metrics,
  runtime startup, clean-machine, cross-machine, and external release
  evidence remain unrun under the no-launch policy.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
