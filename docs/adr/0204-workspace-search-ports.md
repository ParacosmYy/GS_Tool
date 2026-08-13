# ADR-0204: workspace-search Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D155 / ARCH-142

## Context

`WorkspaceSearchCoordinator` already classified asynchronous search callbacks,
but its constructor accepted four independent result callbacks. Stale and
invalidated generations were protected by the tracker while positional
wiring made the search result boundary harder to review and extend.

## Decision

Introduce the frozen/slotted Qt-free `WorkspaceSearchPorts` contract with
named callbacks for surface lookup, summary formatting, and notification
projection. Preserve the existing classification and projection behavior:

- stale callback: no projection;
- invalidated callback: `present_cancelled()` only;
- invalid result: surface error plus error notification;
- valid result: surface result plus the existing warning/success severity;
- matching failure: surface error plus error notification.

MainWindow retains search service, query/generation cancellation, result
surface, containment, notification, worker, and concrete Qt ownership. No Qt
type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would retain avoidable result-path wiring risk.
- Moving search validation, cancellation, or severity policy into the Ports
  contract would broaden a presentation composition boundary.
- Introducing an event bus or search state machine would add lifecycle
  abstraction without changing behavior.

## Review and evidence

Aquinas the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Boole the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the immutable named contract removes positional coupling
without adding behavior or policy.

The first inline branch probe reproduced an `AttributeError` in `fail()` from
one stale `_get_surface` reference. The parent fixed that root-cause omission
by routing the failure path through `WorkspaceSearchPorts.get_surface`; the
complete branch probe then passed.

Authorized non-destructive evidence:

- `D155-WORKSPACE-SEARCH-BRANCH-PROBE=PASS`
- `D155-STALE-INVALIDATED-INVALID-VALID-FAILURE-PROBE=PASS`
- `D155-PORTS-IMMUTABILITY-PROBE=PASS`
- `D155-SOURCE-WIRING-PROBE=PASS`
- `D155-QT-FREE-CONTRACT-PROBE=PASS`
- `D155-PRESENTATION-AUDIT=PASS`
- `D155-COMPILEALL=PASS`
- `D155-RUFF=PASS`
- `D155-FORMAT=PASS`
- `D155-PACKAGE-BUILD=PASS`
- `D155-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `54B05D3C49B6D36FBBBFD6574F7DDA1794457896D145C72FF3CEFD685FF1556E`
- bytes: `38545481`
- source revision: `tree-sha256:bb8d9f11d89541898d2564e5646e7e01596d8b3a8f643a5cca5c7a1d23de14b9`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The inline and static/package checks do not prove native Qt worker/event
timing, filesystem traversal, runtime startup, clean-machine or cross-machine
behavior, signing, installer, update, legal clearance, support ownership, or
release readiness. Those gates remain open under the active no-launch/no-
release authorization boundary.
