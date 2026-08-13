# D36a parent review — Workspace and operation notification severity

- **Delivery:** D36a / UI-22 / ARCH-26
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D36a adds explicit levels to the existing MainWindow Workspace and Find in
Files notification calls. The permanent status phase shows long-running
operation progress while its transient copy uses explicit `info`; clean
workspace-open and complete searches use `success`; cancellation,
startup restore guards, missing roots, containment rejections, no-match,
truncation/limit, and typed diagnostics use `warning`; unavailable services,
invalid results, and failures use `error`.

The existing `_begin_operation` seam carries the progress metadata for the
document, workspace, save, and Replace All operations already owned by that
coordinator. It does not add state or change the operation ID, busy, phase,
completion, stale-generation, cooperative-cancellation, containment,
session-restore, or file-opening policies. An invalid search result is now
projected to both the existing inline error surface and the shell notification
surface using the already localized message.

## Review evidence

The parent source review found no behavior change outside presentation metadata
and the explicit invalid-result shell projection. The source probe found legal
explicit levels on all targeted workspace/search notifications and all eleven
startup restore guard notifications. The summary mapping reads only typed
`WorkspaceSearchResult` fields.

The required architecture consultation was attempted with Noether the 2nd /
Luna max using a read-only workspace/coordinator boundary question. After two
bounded waits it returned no conclusion and was closed; no architecture PASS
is claimed. An independent read-only review was attempted with Nietzsche the
2nd / Luna max. Its two bounded waits also returned no conclusion and it was
closed; no child PASS is claimed.

## Simplification assessment

Reusing `_begin_operation` for progress avoids duplicating a notification call
at every asynchronous operation without introducing a new policy object.
Typed result fields are used directly instead of parsing localized summaries.
The existing inline surfaces remain responsible for detailed presentation,
and no second Workspace state model is introduced. No further safe
simplification is required for D36a.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code; embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, and motor-control
requirements are not applicable. Public CloudWeGo material remains a
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim. The applicability
record is carried in ADR-0061.

## Authorized non-destructive validation

- `uv run python -m compileall -q src/quillforge/presentation/main_window.py` — PASS.
- Ruff check and format check on `main_window.py` — PASS; Ruff cache emitted a
  non-fatal access warning while returning exit code 0.
- D36a workspace-level source probe — PASS: targeted methods have explicit
  legal levels.
- D36a restore-guard probe — PASS: all eleven matching startup restore
  notifications have explicit levels.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, interactive workspace,
  search, or file activation, screen-reader, DPI, clean-machine, deployment,
  or hardware operation was authorized or performed.

## Handoff and limits

The D36a handoff records package identity after the synchronized package build.
The release verifier remains expected NO-GO because runtime, clean-machine,
report-freshness, and release-owner gates remain open. Settings and unrelated
document/session notification calls remain future bounded work.
