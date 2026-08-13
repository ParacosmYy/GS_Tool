# ADR-0275: Keep startup diagnostics fail-open at the path boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D229 / ARCH-211
- Scope: `src/quillforge/__main__.py`

## Context

D228 made the early startup log more actionable, but `_record_startup_failure`
resolved the log path before entering its write-error boundary. A broken or
unavailable `LOCALAPPDATA`, home directory, or path provider could therefore
raise a second exception while the original application startup exception was
being handled.

## Decision

Keep the existing entry-boundary ownership and make all diagnostic stages
fail-open:

1. resolve the user-local report path inside a protected boundary;
2. retain D228's `execution_context: unavailable` fallback when context
   collection fails; and
3. protect payload construction and report writing so the helper returns
   `None` rather than replacing the original startup failure.

No Qt import, application composition, MessageBox policy, normal GUI entry,
or exit-code policy changes.

## Boundary and privacy

The helper remains a write-only diagnostic edge. It records no arguments,
environment dump, settings, secrets, or document contents. The existing
diagnostic path tradeoff remains explicit: a path may contain a username or
network root. Native EXE startup is not asserted under the no-launch policy.

## Review and simplification

The parent review is `PASS`; the simplification assessment is `PASS`. The
`Boole the 6th / Luna max` architecture window returned `NO_CONCLUSION`; the
`Confucius the 6th / Luna max` independent source review returned `PASS`. No
additional abstraction or diagnostics service is justified for this one
entry-boundary failure mode.

## Public-source applicability

Python 3.12 public `pathlib`, `os`, `traceback`, and exception behavior are the
applicable first-party engineering references. No dependency changed. Public
CloudWeGo material is engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable.
