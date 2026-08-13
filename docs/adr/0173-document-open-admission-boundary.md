# ADR-0173: document-open admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D133 / ARCH-111

## Context

`MainWindow._start_open` still combined four concerns: busy and startup
session-restore admission, operation identity/busy projection, session-restore
operation binding, and submission through the existing asynchronous
`DocumentOpenCoordinator`. That made the file-open path harder to audit even
though result classification and projection were already isolated.

## Decision

Add the Qt-free `DocumentOpenAdmissionCoordinator` and frozen/slotted
`DocumentOpenAdmissionPorts` contract. The coordinator owns only this order:

1. reject busy requests and non-restore requests during startup restore,
   retaining the existing warning text;
2. begin the existing operation with `Opening {path.name}...`;
3. bind the operation when `session_restore` is true;
4. submit the existing `DocumentOpenCoordinator` operation through the
   existing `TaskRunner` dispatcher;
5. return whether the request was admitted.

`FileDialogSurface.getOpenFileName`, `WorkspacePanel` file/folder activation,
`DocumentOpenCoordinator` result classification, duplicate-tab projection,
line navigation, session restore continuation, notifications, and close
policy remain in their existing owners.

## Alternatives rejected

- Keeping the four-step sequence in `MainWindow` leaves a user-visible file
  entry point and its async admission boundary coupled to the composition root.
- Moving result classification or tab projection into the new coordinator
  would merge admission with completion policy and widen the migration.
- A variadic `Callable[..., None]` port would hide the dispatcher contract;
  the named `OpenSubmission` type keeps operation, ID, line, and dispatcher
  positions explicit.

## Review and evidence

Laplace the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. Sartre the 4th / Luna
max was assigned the independent read-only review and also returned no
conclusion; no child PASS is claimed. Parent review is `PASS`; simplification
assessment is `PASS` because the coordinator adds one explicit contract and
does not duplicate result or application policy.

Authorized non-destructive evidence: `D133-OPEN-ADMISSION-BEHAVIOR-PROBE=PASS`,
`D133-OPEN-ADMISSION-CONTRACT-PROBE=PASS`, `D133-QT-FREE-OPEN-PROBE=PASS`,
`D133-COMPILEALL=PASS`, `D133-RUFF=PASS`, `D133-FORMAT=PASS`, project checks,
package identity, no-launch, traceability, and expected release NO-GO.
Native file dialogs, QApplication startup, queued worker timing, filesystem
durability, clean-machine, cross-machine, and release-owner evidence remain
unrun under the active boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
