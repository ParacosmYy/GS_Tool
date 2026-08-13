# ADR-0177: document-creation admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D137 / ARCH-115

## Context

`MainWindow._new_document` still combined busy/startup-restore admission,
document creation, tab projection, lifecycle publication, and success
notification. The existing `DocumentTabCreationCoordinator` owns tab assembly,
while `DocumentService` owns creation of the immutable application document.
The remaining composition-root sequence needed a small, explicit boundary.

## Decision

Add the Qt-free `DocumentCreationAdmissionCoordinator` and frozen/slotted
`DocumentCreationAdmissionPorts` contract. The coordinator owns only this
sequence:

1. read the current busy and startup-restore state;
2. reject busy work, or reject startup restore unless the existing explicit
   `allow_during_startup` policy is supplied;
3. preserve the existing restore warning text;
4. create one new `OpenedDocument` through the composition-bound
   `DocumentService`;
5. delegate tab assembly to the existing `DocumentTabCreationCoordinator`
   adapter through `MainWindow._add_tab`;
6. publish the existing `DocumentOpened` event; and
7. publish the existing success notification.

The coordinator does not own document persistence, editor construction, tab
title/status/session-save policy, event-bus implementation, notification
rendering, or startup restore orchestration. Initial-document restore retains
its explicit `allow_during_startup=True` call path.

## Alternatives rejected

- Keeping all admission and projection calls in `MainWindow` leaves new
  document creation inconsistent with the existing typed admission boundaries.
- Moving `DocumentService`, tab assembly, or `EventBus` behavior into the new
  coordinator would duplicate application or presentation policy.
- A generic event/command abstraction would hide the required ordering for one
  bounded creation path and weaken the typed ports contract.

## Review and evidence

Socrates the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. Helmholtz the 4th /
Luna max was assigned the independent read-only review and also returned no
conclusion; no child PASS is claimed. Parent review is `PASS`; simplification
assessment is `PASS` because one focused typed admission boundary preserves
the existing document service, tab coordinator, event bus, and notification
owners without duplicating their policies.

Authorized non-destructive evidence: `D137-DOCUMENT-CREATION-BEHAVIOR-
PROBE=PASS`, `D137-DOCUMENT-CREATION-CONTRACT-PROBE=PASS`,
`D137-QT-FREE-CREATION-PROBE=PASS`, `D137-COMPILEALL=PASS`, `D137-RUFF=PASS`,
`D137-FORMAT=PASS`, project checks, package identity, no-launch,
traceability, and expected release NO-GO. QApplication startup, initial
restore timing, native editor/tab rendering, clean-machine behavior, and
release-owner evidence remain unrun under the active authorization boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
