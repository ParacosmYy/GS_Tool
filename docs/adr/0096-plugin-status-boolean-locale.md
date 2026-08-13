# ADR-0096: Plugin status boolean locale projection

- **Status:** accepted-with-limits; D71 / UI-44 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

Plugin Status already localized its labels and lifecycle words, but its
tooltip rendered `enabled` and `active` booleans through Python's `True` and
`False`. This left a small but visible English/runtime-value leak after the
user selected Chinese.

## Decision

Add two bounded presentation strings for boolean values: `true`/`false` in
`en-US` and `是`/`否` in `zh-CN`. `PluginStatusDialog` uses those strings only
for tooltip projection. The immutable `PluginRuntimeStatus` contract, row
format, selection, signals, trust/enablement predicates, and lifecycle policy
remain unchanged.

## Invariants

1. `en-US` keeps the diagnostic values `true` and `false`.
2. `zh-CN` shows `是` and `否` instead of Python representation text.
3. Plugin status data, trust, enablement, active state, permissions, errors,
   row selection, and button enablement are untouched.
4. No runtime GUI launch, screenshot, or test-only asset is required or
   performed under the active no-launch policy.

## Alternatives considered

- **Leave Python booleans visible:** rejected; it is a clear language-switch
  inconsistency in a user-facing tooltip.
- **Translate the application status contract:** rejected; application
  contracts remain locale-free and typed.
- **Use catalog-only boolean keys:** rejected; boolean projection is shared
  plugin-status vocabulary, not extension-catalog metadata.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- The D71 plugin-status locale probe covers both boolean states, both locales,
  and preserved dynamic status values.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D71 handoff.
- Architect and independent review windows returned no conclusion; no child
  PASS is claimed.
- Native tooltip rendering, runtime language switching, accessibility,
  startup, clean-machine, cross-machine, and external release evidence remain
  unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
