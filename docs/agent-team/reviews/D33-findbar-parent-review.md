# D33 parent review — FindBar semantic feedback projection

- **Delivery:** D33 / UI-19 / ARCH-23
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

The existing `FindSurface` seam now forwards an optional keyword
`FeedbackLevel` to `FindBar.set_status`. The default remains `info`, so the
existing one-argument call contract is preserved. FindBar owns only the raw
message, locale projection, current level, and QSS refresh; it does not know
about editors, tabs, services, operation IDs, rollback, or application policy.

MainWindow maps the existing Find/Replace/Replace All outcomes at their
current call sites: successful matches/replacements are success; input,
stale-match, busy-conflict, policy/limit, and cancellation outcomes are
warning; Replace All progress is working; and Replace All failure/partial
change outcomes are error. Keyboard routes, editor calls, operation locking,
cancellation, rollback, and notification/error behavior stay in their prior
owners.

## Review evidence

The parent source review found no must-fix behavior regression in the changed
files. The static AST probe found 17 FindSurface status calls in MainWindow and
each supplies an explicit level. The default compatibility path remains
available for callers that do not need semantic styling.

The required architecture consultation was attempted with Aquinas the 2nd /
Luna max using a read-only D33 call-chain and ownership question. After two
bounded waits and a concise-result request it returned no conclusion; this
record does not claim an architecture PASS. An independent read-only review
was attempted with Volta the 2nd / Luna max. It also returned no conclusion
after bounded waits and a concise-result request, then was closed. No child
PASS is claimed; the parent review is the integration decision.

## Simplification assessment

The change is a behavior-preserving simplification of repeated status-only
presentation work: one shared level contract now drives FindBar, Workspace,
Search, and StatusSurface QSS projection. The optional keyword avoids an API
break. Adding a second FindBar state model, parsing localized strings, or
moving outcome mapping into the widget would increase coupling and was not
necessary. No further safe simplification is required in D33.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control concerns are not
applicable. The public CloudWeGo pages recorded in ADR-0058 are transferable
engineering references only. They do not establish a private ByteDance
standard, certification, or compliance claim.

## Authorized non-destructive validation

- `uv run python -m compileall -q src` — PASS.
- Ruff check on the changed presentation modules — PASS.
- Ruff format check on the changed presentation modules — PASS.
- D33 API/boundary probe — PASS: one-argument compatibility, thin
  FindSurface forwarding, 17 explicit MainWindow semantic calls, and all five
  `findStatus` selectors.
- D33 contrast probe — PASS: 60 pairs (3 themes × 4 accents × 5 states), each
  at or above 4.5:1; minimum observed 4.525:1.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, interactive Find/Replace,
  screen-reader, DPI, clean-machine, deployment, or hardware operation was
  authorized or performed.

## Handoff and release limits

The D33 handoff and package manifest are synchronized after the package build.
The release verifier remains expected NO-GO because release-owner,
clean-machine, interactive, and fresh packaged-report evidence are still open.

The D33 package identity is root/dist SHA-256
`C10278494D9424BA47614936CE5A3C88B7A656A1AF210C734D9FC88871098C8E`,
`38,402,282` bytes, with source revision
`tree-sha256:07cb6f3b2e9d9130ab97ca10a14b7a962bec3efcfbfb4ebc7554d10c129d4023`.
