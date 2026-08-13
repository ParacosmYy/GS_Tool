# D61 parent review — session-snapshot capture boundary

| Field | Value |
|---|---|
| Delivery | `D61 / ARCH-49` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## User and architecture outcome

The session-save snapshot path now has a narrow, inspectable pure assembly
boundary. Clean path-backed tab metadata is assembled without importing Qt,
while MainWindow continues to own editor reads and application orchestration.

## Role and review record

| Role | Agent | Result |
|---|---|---|
| Architect | Jason the 2nd / Terra max | Two bounded waits returned no conclusion; no architecture PASS claimed |
| Independent reviewer | Euler the 2nd / Luna max | Two bounded waits returned no conclusion; agent was closed; no independent PASS claimed |
| Parent | Architect | Sole writer, integrated, inspected, and statically verified the slice |

No child PASS is claimed.

## Contract and boundary

`presentation.session_snapshot_builder.build_session_snapshot[TabT]` accepts
opaque tabs and callbacks for path, dirty, modified, and cursor state. It
does not import Qt or own `EditorWidget`, `SessionService`, `TaskRunner`,
timers, tab registries, or application policy. MainWindow supplies those
callbacks and retains `_build_session_snapshot()` as the coordinator-facing
method used by the existing save debounce path.

The implementation preserves path-first and dirty/modified short-circuiting,
ordered document retention, the existing `RuntimeError` / `ValueError`
per-tab skip behavior, direct `Path` equality for active selection, and the
empty snapshot default index.

## Simplification assessment

The callback-based pure function is the smallest safe boundary: it removes
duplicated assembly responsibility without adding a second DTO, a new service,
or a speculative protocol. No further behavior-preserving simplification was
identified.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code only. Embedded C/C++ assurance and
manufacturer-source applicability are `N/A`; no MISRA, ISO, certification, or
private ByteDance-standard claim is made. Public CloudWeGo material is only an
engineering reference.

## Authorized non-destructive validation

- `D61-session-snapshot-builder-boundary-probe=PASS`.
- `D61-session-snapshot-builder-behavior-probe=PASS` for ordered clean tabs,
  pathless/dirty/invalid/runtime-skipped tabs, active index, and empty output.
- Targeted compileall, Ruff, and format checks — `PASS` before final
  documentation/package synchronization.
- Full compileall, handoff, repository check, package identity, and expected
  release NO-GO evidence are recorded after final synchronization.
- No QApplication/Qt/EXE launch, screenshots, unit tests, mocks, fixtures,
  harnesses, test-only assets, deployment, or hardware operation were run.

## Limits and disposition

Static and pure-function evidence cannot prove native Qt callback timing,
editor cursor availability under real events, or rendered runtime behavior.
The slice is accepted with those limits; runtime and release gates remain
open, and delegated architecture/independent review windows are recorded as
no-conclusion.
