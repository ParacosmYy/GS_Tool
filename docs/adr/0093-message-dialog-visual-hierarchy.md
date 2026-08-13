# ADR-0093: Message-dialog visual hierarchy

- **Status:** accepted-with-limits; D68 / UI-41 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The shell's About, error, and unsaved-document prompts used Qt static
`QMessageBox` helpers, so they could not receive stable presentation identity
or the project's theme hierarchy. Recovery already used an instance but its
three choices were visually generic. This left important user decisions with
weak highlighting even though the rest of the shell had semantic action roles.

## Decision

Keep message composition inside `MessageSurface` and use equivalent
instance-based `QMessageBox` projections for question, About, and error flows.
Each receives a stable object name and the same icon, localized title/text,
standard-button set, default Save button, modal `exec()` boundary, and return
mapping as its former static helper.

Give `RecoveryPromptSurface` the `recoveryPrompt` identity and reuse existing
presentation roles: restore is `primaryAction`, discard is `warningAction`,
and later is `quietAction`. Centralized `theme.py` adds the message-box
surface/edge/label/button hierarchy. No application policy, signal, service,
locale, persistence, or recovery decision moves.

## Invariants

1. Unsaved close still offers Save, Discard, and Cancel in the same order,
   defaults to Save, and maps only the same returned standard buttons.
2. About and error remain blocking parent-owned dialogs with the same
   localized strings and icon intent.
3. Recovery still returns only restore/discard/later from the same clicked
   button identity; only presentation metadata changes.
4. All message labels use the theme's primary text endpoint and role-specific
   button selectors reuse existing contrast-safe tokens.
5. No runtime GUI launch, screenshot, or test-only asset is required or
   performed under the active policy.

## Alternatives considered

- **Leave static helpers unchanged:** rejected; they cannot receive the
  presentation identity needed for the shared theme hierarchy.
- **Move message decisions into MainWindow:** rejected; it would increase
  coupling and violate the existing MessageSurface boundary.
- **Create a new message-dialog widget class:** rejected; equivalent
  `QMessageBox` composition plus centralized QSS is sufficient and smaller.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- Source probe confirms equivalent message-box configuration, recovery action
  roles, and centralized selectors.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D68 handoff.
- Architect and independent review windows returned no conclusion; no child
  PASS is claimed.
- Native message-box rendering, keyboard traversal, accessibility, runtime
  startup, clean-machine, cross-machine, and external release evidence remain
  unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
