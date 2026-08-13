# ADR-0073: document-tab state clarity

- **Status:** accepted-with-limits; D48 / ARCH-38 / UI-34 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

Modified documents were represented by an asterisk in the tab title. That
preserved the text contract but made the unsaved state easy to miss in a dense
tab rail and left the tab surface without a theme-refresh path for state
markers. The visual cue must remain presentation-only: dirty state, save
policy, close guards, recovery, and document lifecycle belong to MainWindow and
the application services.

## Decision

Add an authored `IconKey.MODIFIED` marker and let
`DocumentTabSurface` project a clean-document or modified-document icon for
each tab. The surface keeps a parallel, index-aligned modified projection and
offers `set_modified()` plus `refresh_icons()`. It obtains foreground/accent
colors from the current palette, so theme/accent changes retint existing tabs
and future tabs inherit the current theme.

`MainWindow` remains the only owner that interprets dirty state. It passes the
initial dirty projection when adding a tab and updates the surface whenever the
existing title/dirty projection changes. The existing asterisk remains in the
title as a non-color text cue; tab close/current signals, title semantics,
document save/recovery, and close policy remain unchanged.

## Invariants

1. A tab's modified marker is presentation state, not a new document or
   persistence state owner.
2. Tab and marker lists are updated together on add/remove, so index reuse
   cannot leave a stale marker on a different document.
3. Theme changes refresh all existing markers; new tabs use the current
   `QTabWidget` palette.
4. The title asterisk and existing `set_title()` route remain intact, so the
   unsaved state is not conveyed by color alone.
5. Tab close requests, current-tab changes, document dirty/save/recovery policy,
   and MainWindow operation guards remain unchanged.

## Alternatives considered

- **Only change the title text:** rejected because the current asterisk is easy
  to miss and provides no visual hierarchy in the tab rail.
- **Add a custom tab delegate:** rejected as a larger rendering surface with
  higher DPI/accessibility and native-style risk.
- **Put a widget or dirty policy inside each tab:** rejected because it would
  couple document state and application behavior to the presentation surface.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable project references are the existing authored icon contract in
`docs/adr/0037-authored-icon-surface.md`, the document-tab boundary in
`docs/adr/0039-main-window-document-tab-surface.md`, and the UI visual-quality
skill's contrast/state matrix.

## Verification target

- A source probe proves the modified icon path, add/remove index alignment,
  theme refresh route, and MainWindow dirty-policy ownership.
- Compile, Ruff, format, JSON, handoff, package, and release no-go evidence are
  recorded.
- Parent and independent reviews record conclusions or explicit no-conclusion
  states.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active no-launch policy.

## Limits and simplification

This is the smallest complete tab-state presentation improvement: one authored
vector glyph, one surface-owned projection list, and one theme refresh route.
No document model, dirty flag, signal, custom delegate, animation, persistence,
or new coordinator was introduced. Native icon metrics, screen-reader output,
runtime tab interaction, DPI, cross-machine appearance, and release-owner
gates remain unrun.
