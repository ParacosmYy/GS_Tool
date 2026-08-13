# ADR-0200: session-load Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D151 / ARCH-138

## Context

`SessionLoadCoordinator` already owned session-load result classification and
recovery-first continuation, but its constructor accepted four independent
projection callbacks. The callback meanings were stable, yet positional
wiring made the baseline and startup contract harder to review as the
enterprise architecture migration continues.

## Decision

Introduce the frozen/slotted Qt-free `SessionLoadPorts` contract with named
callbacks for `set_last_saved`, `set_snapshot`, `schedule_recovery_scan`, and
`notify`. The coordinator keeps the existing behavior:

- absent or valid results project the loaded snapshot (or the default) and
  then schedule recovery scanning;
- invalid results project the baseline, notify the existing error, and then
  schedule recovery scanning;
- malformed and failed results use the default baseline, notify the existing
  error, and then schedule recovery scanning;
- baseline callbacks remain ordered as `set_last_saved -> set_snapshot`.

MainWindow retains the session service, TaskRunner, startup admission,
restore tracker, notification policy, and all concrete Qt/application
ownership. No Qt type enters the coordinator.

## Alternatives rejected

- Keeping four positional callbacks would preserve an avoidable wiring hazard.
- Moving validation, persistence, or recovery decisions into the Ports
  contract would cross existing application and restore boundaries.
- Adding an event bus or generic projection registry would be broader than a
  named contract for four existing callbacks.

## Review and evidence

Lagrange the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Mill the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the immutable named contract removes positional coupling
without adding behavior or policy.

Authorized non-destructive evidence:

- `D151-LOAD-PROJECTION-PROBE=PASS`
- `D151-LOAD-ORDER-PROBE=PASS`
- `D151-SOURCE-WIRING-PROBE=PASS`
- `D151-QT-FREE-CONTRACT-PROBE=PASS`
- `D151-PRESENTATION-AUDIT=PASS`
- `D151-COMPILEALL=PASS`
- `D151-RUFF=PASS`
- `D151-FORMAT=PASS`
- `D151-PACKAGE-BUILD=PASS`
- `D151-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `FA57BF8BBA8CF33D4E54F656DC615A0909B7C77A8CB410C1F58CB5FEC3264A95`
- bytes: `38544714`
- source revision: `tree-sha256:c028c83405fb6d1c2acb387c910847cc1e5d749aa093c4b86447490ee8891fd2`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The result/order probes and static/package checks do not prove native Qt event
timing, startup scheduling, session filesystem behavior, theme rendering,
font/DPI metrics, accessibility, runtime startup, clean-machine or
cross-machine behavior, signing, installer, update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.
