# D35a parent review — plugin and extension notification severity

- **Delivery:** D35a / UI-21 / ARCH-25
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D35a keeps all existing plugin and extension outcome interpretation in
`MainWindow` and adds explicit `StatusMessageLevel` metadata at the current
notification call sites. Catalog scans, governance mutations, runtime
enablement, and isolated host diagnostics now distinguish in-progress info,
success, user-attention, and failure states. The permanent TaskRunner/phase
projection still communicates `WORKING`. A valid catalog with truncation or
typed scan/approval diagnostics is a warning; a ready host is success, a
policy rejection is warning, and typed host failures are errors.

The independent source review also identified and the parent integrated two
D34a corrections: invalid or failed session persistence is `error`, and the
recovery `later` branch that retains a snapshot for review is `warning`.

No service, trust boundary, execution policy, worker state, operation ID,
stale guard, governance lock, or command-refresh policy moved. The existing
one-argument `notify(message)` path remains available.

## Review evidence

The parent source review found no behavior or security-boundary regression in
the changed coordinator calls. Static probes found legal explicit levels for
all targeted plugin/extension notification calls and the corrected D34a
branches. Conditional result mapping uses only the existing typed snapshot and
host state fields.

The required architecture consultation was attempted with Fermat the 2nd /
Luna max using a read-only plugin/coordinator boundary question. After two
bounded waits it returned no conclusion and was closed; no architecture PASS
is claimed. An independent read-only review was attempted with Cicero the 2nd
/ Luna max. Its two bounded waits also returned no conclusion and it was
closed; no child PASS is claimed.

## Simplification assessment

Explicit call-site metadata is the smallest behavior-preserving change. It
avoids parsing localized messages, avoids a second plugin notification policy
service, and keeps typed result interpretation next to the existing outcome
owner. The conditional warning for catalog diagnostics and host-state mapping
does not introduce new state; it projects fields already present in immutable
application results. No further safe simplification is required for D35a.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code; embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, and motor-control
requirements are not applicable. Public CloudWeGo material remains a
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim. The applicability
record is carried in ADR-0060.

## Authorized non-destructive validation

- `uv run python -m compileall -q src/quillforge/presentation/main_window.py` — PASS.
- Ruff check and format check on `main_window.py` — PASS.
- D35a plugin-level source probe — PASS: all targeted calls have explicit legal
  level metadata.
- D34a correction source probe — PASS: invalid/failed persistence is `error`
  and deferred recovery is `warning`.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, interactive plugin/session
  flows, screen-reader, DPI, clean-machine, deployment, or hardware operation
  was authorized or performed.

## Handoff and limits

The D35a handoff records package identity after the synchronized package build.
The release verifier remains expected NO-GO because runtime, clean-machine,
report-freshness, and release-owner gates are still open. Document/workspace
notification calls outside this slice remain future bounded work.
