# ADR-0197: settings-save projection Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D149 / ARCH-135

## Context

`SettingsSaveProjectionCoordinator` already preserved a stable five-step
projection order, but its constructor accepted five positional callbacks.
That made call-site order part of an implicit contract and made future
extension easy to get wrong during the enterprise architecture migration.

## Decision

Introduce the Qt-free frozen/slotted `SettingsSaveProjectionPorts` contract
with named callbacks for `apply_snapshot`, `retranslate`,
`apply_editor_settings`, `animate_transition`, and `notify_saved`.
`SettingsSaveProjectionCoordinator.project` owns only the existing order:

`apply_snapshot -> retranslate -> apply_editor_settings -> animate_transition -> notify_saved`.

MainWindow retains the settings service, dialog, concrete editor iteration,
QApplication/theme application, animation surface, notification policy, and
asynchronous result classification. No Qt type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would preserve an avoidable wiring hazard.
- Moving settings validation, persistence, or animation implementation into
  the coordinator would cross existing service and presentation boundaries.
- Adding a generic event bus or registry would be broader than a named
  contract for five already-stable callbacks.

## Review and evidence

Avicenna the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Nietzsche the 5th / Luna max
was assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the frozen/slotted named contract removes positional
coupling without adding behavior or policy.

Authorized non-destructive evidence:

- `D149-PROJECTION-ORDER-PROBE=PASS`
- `D149-SOURCE-WIRING-PROBE=PASS`
- `D149-PRESENTATION-AUDIT=PASS`
- `D149-COMPILEALL=PASS`
- `D149-RUFF=PASS`
- `D149-FORMAT=PASS`
- `D149-PACKAGE-BUILD=PASS`
- `D149-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `4B4591A074B985F36ABEFFBF1B1B4948FC030A028131D233F97264A5EB2BD328`
- bytes: `38543804`
- source revision: `tree-sha256:cc9dd570ec8b4877f365c4d7c5e08136b86a4520714e186f342fd7ed628aeb75`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The order/contract probe and static/package checks do not prove native Qt
event timing, theme rendering, editor font/DPI metrics, accessibility,
runtime startup, clean-machine or cross-machine behavior, signing, installer,
update, legal clearance, support ownership, or release readiness. Those gates
remain open under the active no-launch/no-release authorization boundary.
