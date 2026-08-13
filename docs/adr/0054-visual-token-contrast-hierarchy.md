# ADR-0054: Harden visual endpoint contrast and interaction hierarchy

- Status: Accepted with limits
- Date: 2026-08-10
- Decision owners: Architect / fixed six-role workflow
- Delivery: D29 / UI-15

## Context

The shell already centralizes its palette and QSS, but the same foreground
token is reused for different high-saturation backgrounds. That is fragile for
the amber/砂金 warning action and makes a bright hover state depend on unrelated
accent choices. The surface hierarchy also benefits from a more intentional
tab-pane and command-rail rhythm rather than adding ad-hoc widget styles.

## Decision

Keep `ThemeColors` as the single visual-token source and derive a dedicated
foreground for each accent endpoint used as a filled background. In
particular, `on_accent_gold` is selected against `accent_gold` and is used by
the warning action hover state. The existing shared gradient foreground remains
conservative across the accent/pink endpoints.

Refine only the existing QSS contracts for the command rail and document-tab
surface: preserve semantic roles, focus states, keyboard behavior, and widget
object names while making spacing, border weight, and selected-tab hierarchy
more deliberate. Do not add a theme engine, runtime color mutation, or a
second stylesheet source.

## Invariants

1. Every filled accent endpoint used for text has a deterministic foreground
   selected by the existing contrast calculation.
2. The amber warning action keeps its warning role, label, callback, and
   disabled/pressed/focus semantics; only its filled-hover foreground token is
   refined.
3. Existing theme IDs, accent IDs, persisted settings, localization, and
   editor syntax roles remain unchanged.
4. All QSS remains generated from `ThemeColors`; no widget-local stylesheet or
   hard-coded theme branch is introduced.
5. Runtime screenshots, native rendering, installed fonts, DPI, and screen
   reader behavior remain explicit limits under the no-launch policy.

## Alternatives rejected

- Using one hard-coded white or dark foreground for every accent would re-open
  the reported gold readability bug in another theme.
- Adding a runtime contrast-repair pass would make visual output depend on
  widget order and hide palette errors instead of fixing token ownership.
- Replacing the existing stylesheet wholesale would broaden a bounded visual
  correction into an unreviewable UI rewrite.

## Public-source applicability

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded code-review simplifier are N/A for
MCU/vendor constraints. CloudWeGo public material is an engineering reference
for explicit boundaries only, not a private ByteDance standard or certification
claim.

## Verification target

Run the existing compile, Ruff, format, handoff, project, and package gates;
add a non-destructive static contrast probe for every theme/accent endpoint and
the warning-action pair. Do not launch Qt, create tests, or claim runtime
visual acceptance.
