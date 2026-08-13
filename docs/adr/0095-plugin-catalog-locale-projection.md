# ADR-0095: Plugin catalog locale projection

- **Status:** accepted-with-limits; D70 / UI-43 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The extension-catalog dialog translated its title, hint, action buttons, and
summary, but list rows and tooltips still exposed stable English field names
and enum values. Changing the shell locale therefore left a visible mixed
language surface and made the language setting feel incomplete.

## Decision

Extend the existing presentation translation catalog with bounded plugin
catalog field, enum, and known execution-reason labels. Add
`catalog_value()` as a locale-aware enum projection that falls back to an
unknown raw value, preserving forward compatibility. `PluginCatalogDialog`
passes its current locale to row and tooltip formatters and re-renders both
when `set_locale()` is called, including the empty-catalog row.

Translate only stable presentation vocabulary: status, trust, approval,
execution state, loadability, field labels, and known policy reasons. Keep
file names, plugin names, IDs, versions, API values, permission values,
entrypoint metadata, hashes, and free-form validation errors unchanged.

## Invariants

1. `en-US` retains the existing English labels and all dynamic values.
2. `zh-CN` has no stable English field/enum labels in the catalog row or
   tooltip; dynamic diagnostic values remain visible.
3. Row selection, item data, approval/revoke signals, action enablement, and
   plugin trust/execution policy are unchanged.
4. Locale refresh changes only text projection; it does not rescan or mutate
   catalog entries.
5. No runtime GUI launch, screenshot, or test-only asset is required or
   performed under the active no-launch policy.

## Alternatives considered

- **Translate values inline in the dialog:** rejected; it would duplicate
  vocabulary and weaken the centralized i18n owner.
- **Translate dynamic plugin metadata:** rejected; identifiers, paths, hashes,
  and errors must remain searchable and diagnostically exact.
- **Rebuild the catalog on locale change:** rejected; locale is a projection
  concern and must not repeat filesystem scanning or alter entry ownership.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- The D70 catalog-locale probe covers zh-CN/en-US rows, tooltips, stable enum
  labels, dynamic values, and known execution reasons.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D70 handoff.
- Architect and independent review windows returned no conclusion; no child
  PASS is claimed.
- Native list rendering, live Qt locale refresh, accessibility, startup,
  clean-machine, cross-machine, and external release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
