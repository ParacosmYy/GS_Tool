# ADR-0174: document-save admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D134 / ARCH-112

## Context

`MainWindow._start_save` still combined save admission, duplicate-target
rejection, immutable state/text capture, editor read-only protection, operation
begin, and asynchronous `DocumentSaveCoordinator` submission. This made the
save path harder to compare with the newly explicit open-admission boundary.

## Decision

Add the Qt-free `DocumentSaveAdmissionCoordinator` and frozen/slotted
`DocumentSaveAdmissionPorts` contract. The coordinator owns only this order:

1. reject busy or startup-restore requests, retaining the existing warning;
2. reject a target already represented by another tab;
3. capture the tab's state and text;
4. set the editor read-only;
5. begin the existing operation with `Saving {target.name}...`;
6. submit the existing save operation with the original `after` continuation
   through `DocumentSaveCoordinator` and `TaskRunner`.

`_save_document`, `_save_as_document`, `FileDialogSurface.choose_save_path`,
target identity policy, `DocumentSaveCoordinator` result classification,
recovery/session persistence, notifications, and close policy remain in their
existing owners.

## Alternatives rejected

- Keeping the sequence in `MainWindow` leaves save admission coupled to the
  composition root and asymmetric with document-open admission.
- Moving result classification, recovery projection, or filesystem policy
  would merge admission with completion and durability boundaries.
- A generic event bus or variadic callback port would weaken ordering and
  observability for a single synchronous admission path.

## Review and evidence

Cicero the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. Epicurus the 4th /
Luna max was assigned the independent read-only review and also returned no
conclusion; no child PASS is claimed. Parent review is `PASS`; simplification
assessment is `PASS` because one explicit typed boundary replaces one
composition-root sequence without duplicating save completion policy.

Authorized non-destructive evidence: `D134-SAVE-ADMISSION-BEHAVIOR-PROBE=PASS`,
`D134-SAVE-ADMISSION-CONTRACT-PROBE=PASS`, `D134-QT-FREE-SAVE-PROBE=PASS`,
`D134-COMPILEALL=PASS`, `D134-RUFF=PASS`, `D134-FORMAT=PASS`, project checks,
package identity, no-launch, traceability, and expected release NO-GO.
Native dialogs, QApplication startup, queued worker timing, filesystem
durability, clean-machine, cross-machine, and release-owner evidence remain
unrun under the active boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
