# D57 parent review — session-restore tab projection boundary

| Field | Value |
|---|---|
| Delivery | `D57 / ARCH-46` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## User and architecture outcome

The restore tracker now owns the ordered output references and active-tab
selection input for the session restore lifecycle. MainWindow no longer owns a
parallel tab list and retains all application, service, async, startup, close,
and notification policy.

## Role and review record

| Role | Agent | Result |
|---|---|---|
| Architect | Erdos the 2nd / Terra max | Two bounded waits returned no conclusion; no architecture PASS claimed |
| Independent reviewer | Bohr the 2nd / Luna max | Two bounded waits returned no conclusion; no independent PASS claimed |
| Parent | Architect | Sole writer, integrated, inspected, and statically verified the bounded change |

No child PASS is claimed.

## Contract and boundary

`SessionRestoreTracker[TabT]` stores opaque tab records, appends them through
`record_restored_tab()`, and selects the canonical active-path match or first
record through `select_restored_tab(path_of)`. `path_of` is the only record
shape supplied by MainWindow; the tracker imports no Qt, editor, service, or
task-runner module.

MainWindow records the unchanged existing/new-tab outcomes and delegates only
the final restored-tab target. It retains workspace restoration barriers,
document/recovery/session services, TaskRunner dispatch, tab-surface changes,
notifications, startup/close guards, and initial-document fallback policy.

## Simplification assessment

The slice removes one coordinator-owned list and one inline active-path scan.
No second tracker, path-index cache, `_DocumentTab` protocol, or new callback
state machine was added. The generic opaque record plus one path callback is the
smallest boundary that keeps the tracker Qt-free; no further safe simplification
was identified without returning state ownership to MainWindow or coupling the
tracker to presentation records.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code only. Embedded C/C++ assurance and
manufacturer-source applicability are `N/A`; no MISRA, ISO, certification, or
private ByteDance-standard claim is made. Public CloudWeGo material is only an
engineering reference.

## Authorized non-destructive validation

- `D57-session-restore-tab-projection-boundary-probe=PASS`.
- `D57-session-restore-order-active-fallback-cleanup-probe=PASS`.
- Targeted and full compileall, Ruff, and format checks — `PASS` for the
  source slice before final documentation/package synchronization.
- Package identity, handoff, repository check, and expected release NO-GO
  evidence are recorded after the final package.
- No QApplication/Qt/EXE launch, screenshots, unit tests, mocks, fixtures,
  harnesses, test-only assets, deployment, or hardware operation were run.

## Limits and disposition

Static source evidence cannot prove native Qt event ordering, queued callback
timing, or filesystem behavior on every Windows volume. The slice is accepted
with those limits; runtime and release gates remain open, and the two child
reviews are recorded as no-conclusion.
