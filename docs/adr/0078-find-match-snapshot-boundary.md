# ADR-0078: Find Match snapshot boundary

- **Status:** accepted-with-limits; D53 / ARCH-43 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

MainWindow kept the result of the last Find request as a positional tuple.
Single replacement compared that tuple against the current tab, query,
case-sensitivity, selection, and content version. The invariant was correct
but implicit, and the tuple could be invalidated from several document,
criteria, and tab lifecycle callbacks.

## Decision

Add the Qt-free `presentation.find_match_tracker.FindMatchTracker` and its
typed immutable `FindMatch` snapshot. The tracker records a match only when a
selection exists, matches all five identity dimensions (tab, query, case
sensitivity, selection bounds, and content version), and exposes explicit
`clear()` invalidation. MainWindow remains responsible for invoking the
editor's literal find operation, reading the live selection, comparing the
selected text, performing replacement, busy gating, and projecting feedback.

## Invariants

1. A failed find or missing selection clears the stored match.
2. A single replacement is allowed only when the current editor state exactly
   matches the recorded tab/query/case/selection/content-version snapshot.
3. Criteria changes, editor modification, tab changes, and Find-bar close keep
   using the existing MainWindow invalidation routes.
4. The tracker imports no Qt, editor widget, service, timer, theme, locale,
   status, or notification type.

## Alternatives considered

- **Keep the anonymous tuple in MainWindow:** rejected because the matching
  invariant remains duplicated and undocumented at the replacement call site.
- **Move find or replacement into the tracker:** rejected because QScintilla
  search and user-facing replacement policy belong to EditorWidget/MainWindow.
- **Track only query text:** rejected because it permits stale tabs, selections,
  or content versions to pass the replacement precondition.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code, not embedded C/C++ or
firmware; MCU, BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and vendor-manufacturer requirements are not applicable.
Public CloudWeGo material remains transferable engineering reference only and
does not establish a private ByteDance standard, certification, or compliance
claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable project references are `EditorEngine.selection_bounds`,
`EditorWidget.find_literal`, `MainWindow` document versioning, and the
enterprise architecture migration specification.

## Verification target and limits

- A Qt-free behavior probe covers a valid match and each stale dimension,
  failed-find/missing-selection clearing, and explicit invalidation.
- A source probe confirms the tracker is Qt-free and MainWindow retains the
  editor, selected-text, busy, and feedback policy.
- Compile, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D53 handoff.
- Native QScintilla selection timing, editor rendering, accessibility, DPI,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner evidence remain unrun under the active no-launch and
  external-authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
