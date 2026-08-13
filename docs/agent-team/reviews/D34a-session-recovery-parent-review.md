# D34a parent review — Session/Recovery notification severity closure

- **Delivery:** D34a / UI-20 / ARCH-24
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D34a adds explicit `StatusMessageLevel` metadata to 19 existing Session and
Recovery notifications in MainWindow. The change reuses D31's `notify`
contract and centralized StatusSurface styling. It does not create a service,
move state, or change any SessionService/RecoveryService/TaskRunner callback,
snapshot, stale-generation, close-guard, or persistence behavior.

Failures and invalid persistence results are error; attention-required
fallback/deferred/postponed/already-running outcomes are warning; successful
restore/discard outcomes are success; neutral results are explicitly info.
MainWindow remains the use-case outcome owner and StatusSurface remains the
presentation owner.

## Review evidence

The parent source review found no must-fix behavior regression. A static AST
probe found 19 notify calls in the targeted Session/Recovery methods, each with
an explicit legal level. The one-argument `notify(message)` compatibility path
was not removed or changed.

The required architecture consultation was attempted with Herschel the 2nd /
Luna max using a read-only coordinator/typed-contract question. After two
bounded waits and a concise-result request it returned no conclusion; this
record does not claim an architecture PASS. An independent read-only review
was attempted with Bernoulli the 2nd / Luna max; its bounded review window
also returned no conclusion and was closed. No child PASS is claimed; the
parent review is the integration decision.

## Simplification assessment

Explicit call-site levels are a safe behavior-preserving simplification of
notification semantics: they remove hidden dependence on the default info
level without introducing another policy layer. Parsing copy or introducing a
RecoveryNotificationPolicy service would add coupling for no demonstrated
benefit. No further safe simplification is required for D34a.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code; embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, and motor-control
concerns are not applicable. The public CloudWeGo pages recorded in ADR-0059
are transferable engineering references only. They do not establish a
private ByteDance standard, certification, or compliance claim.

## Authorized non-destructive validation

- `uv run python -m compileall -q src` — PASS.
- Ruff check on changed presentation modules — PASS.
- Ruff format check on changed presentation modules — PASS.
- D34a Session/Recovery level probe — PASS: 19 targeted notification calls,
  all with legal explicit levels; policy-boundary probe PASS.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, interactive recovery or
  session flows, screen-reader, DPI, clean-machine, deployment, or hardware
  operation was authorized or performed.

## Handoff and release limits

The D34a handoff and package manifest are synchronized after packaging. The
release verifier remains expected NO-GO because release-owner, clean-machine,
interactive, and fresh packaged-report evidence are still open. Other
MainWindow notification call sites remain outside this slice.

The synchronized D35a package identity is root/dist SHA-256
`A6579E0DF3E6FFA4F4540EF12686562FB93E1AB33F718E9A50859EA6974B6EC3`,
`38,405,768` bytes, with source revision
`tree-sha256:168a5e6f9843edb39ee86c71e7dcb93687f099672bed06f91614539100fdc24d`.

## Follow-up correction

The later D35a independent source audit found two classification corrections
outside the original parent review window: invalid/failed session persistence
must be `error`, and the recovery `later` branch that retains a snapshot for
review must be `warning`. Those local levels are now corrected in
`main_window.py`; D35a is the authoritative follow-up record and does not
change the Session/Recovery state machine.
