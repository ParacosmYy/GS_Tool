# ADR-0094: Notification localization closure

- **Status:** accepted-with-limits; D69 / UI-42 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The shell already projected most transient messages through
`presentation.i18n.localize_message`, but three common paths could remain
English or mixed-language in the default Chinese locale: duplicate-open
feedback, plugin lifecycle failures, and known invalid-workspace results.
Plugin catalog and plugin-host summaries also contained structured English
diagnostics; the catalog dialog projected its summary directly and did not
refresh it when the locale changed.

## Decision

Keep application summaries and failure details in their stable English
diagnostic contract. Extend the presentation localization boundary with
bounded structured parsing for plugin failure, catalog-summary, and
plugin-host-summary messages, plus exact/prefix entries for known workspace
and duplicate-open messages. Preserve paths, plugin IDs, phase names, PIDs,
limits, and raw exception details as diagnostic data.

Make `PluginCatalogDialog` retain the immutable summary source and re-project
that source through `localize_message` during construction and locale refresh.
The dialog remains the owner of Qt labels; no locale or Qt dependency enters
the application services.

## Invariants

1. `en-US` returns every source message unchanged.
2. Chinese projection localizes stable labels without translating or
   discarding dynamic paths, identifiers, PIDs, limits, or error details.
3. Application-layer `summary()` methods remain locale-free and retain their
   existing diagnostic wording and data.
4. Plugin catalog entry ownership, dialog signals, governance enablement, and
   plugin execution policy are unchanged.
5. No runtime GUI launch, screenshot, or test-only asset is required or
   performed under the active no-launch policy.

## Alternatives considered

- **Translate inside application summary methods:** rejected; it would make
  application contracts depend on presentation locale and duplicate locale
  ownership.
- **Translate only fixed prefixes:** rejected; structured plugin summaries
  would remain visibly mixed-language.
- **Introduce a new localization service:** rejected; the existing
  presentation `localize_message` seam is sufficient and keeps the diff
  smaller.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- The D69 localization probe covers duplicate-open, plugin failure, known
  workspace errors, catalog summaries, host summaries, and `en-US` identity.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D69 handoff.
- Architect and independent review windows returned no conclusion; no child
  PASS is claimed.
- Native dialog rendering, runtime language switching, accessibility,
  startup, clean-machine, cross-machine, and external release evidence remain
  unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
