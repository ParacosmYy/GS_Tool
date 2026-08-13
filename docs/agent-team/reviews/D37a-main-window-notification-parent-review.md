# D37a parent review — MainWindow notification contract closure

- **Delivery:** D37a / UI-23 / ARCH-27
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D37a closes the remaining MainWindow notification call sites without moving
outcome ownership or introducing another coordinator service. The current
81 unique MainWindow `notify` calls now carry explicit transient
`info`/`success`/`warning`/`error` metadata. New-document completion is
success; invalid or skipped session documents and stale/settings guard paths
are warning; settings unavailability is error; active-tab projection is info.

Long-running operation progress is intentionally not represented as a fifth
transient `StatusMessageLevel`: the permanent status phase remains `WORKING`
and the transient copy is explicit `info`. This preserves the existing
four-level shell contract while keeping the visual hierarchy readable.

D34a–D36a mappings remain intact. MainWindow remains the outcome and policy
owner, StatusSurface remains the presentation owner, and the external
one-argument `notify(message)` compatibility default remains available.
Document/session/settings/command/tab policy, operation IDs, close guards,
locale routing, and service boundaries do not move.

## Review evidence

The parent source review verified, with an AST probe, that all 81 unique
MainWindow `notify` calls pass a literal legal level and that no transient
MainWindow `notify` call uses the invalid `working` value. The separate
FindSurface `FeedbackLevel` contract may still use `working`; it is an inline
surface contract and is not the shell `StatusMessageLevel` API.

The required architecture consultation was attempted with Wegener the 2nd /
Luna max using a read-only MainWindow contract question. It returned no
conclusion; no architecture PASS is claimed. An independent read-only review
was completed by Curie the 2nd / Luna max with **PASS with limits**: the AST
count, level legality, key branch mappings, ownership boundaries, safety, and
performance checks were sound. A follow-up review window with Mill the 2nd /
Luna max timed out twice and was closed while running; it is recorded as no
conclusion rather than a PASS.

## Simplification assessment

Explicit call-site metadata is the smallest behavior-preserving change. It
avoids message parsing, a new notification policy service, and a second
status state model. Keeping progress in the existing permanent phase while
using transient `info` matches the actual `StatusMessageLevel` contract and
avoids adding an unsupported level. No further safe simplification is
required for D37a.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code; embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, and motor-control
requirements are not applicable. Public CloudWeGo material remains a
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim. The applicability
record is carried in ADR-0062.

## Authorized non-destructive validation

- MainWindow AST notification-contract probe — PASS: 81 unique calls,
  explicit legal levels, zero transient `working` levels.
- D36a search summary probe — PASS: the normal `limit_reason == "none"`
  path is not classified as a warning; warning is reserved for cancellation,
  truncation, limits, issues, or no matches.
- Startup restore guard probe — PASS: all eleven matching guard notifications
  carry explicit levels.
- `uv run python -m compileall -q src/quillforge/presentation/main_window.py`
  — PASS.
- Ruff check and format check on `main_window.py` — PASS; any cache access
  warning is non-fatal and the commands return zero.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, interactive flows,
  screen-reader, DPI, clean-machine, deployment, or hardware operation was
  authorized or performed.

## Handoff and limits

The D37a handoff records the rebuilt package identity and repository-backed
checks. The release verifier remains expected NO-GO because runtime/report
freshness, clean-machine, and release-owner gates remain open. Runtime QSS
specificity, actual font/color geometry, accessibility, and visual acceptance
remain unrun under the active no-launch policy.
