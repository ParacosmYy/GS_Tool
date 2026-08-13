# ADR-0120: Document-tab creation coordinator boundary

- **Status:** accepted-with-limits; D95 / ARCH-70 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

MainWindow's `_add_tab` assembled an editor from the current presentation
settings, constructed the private tab record, projected it through
`DocumentTabSurface`, refreshed the title/modified marker, requested session
persistence, and synchronized status. The assembly code was repeated at the
document-open and recovery-restore call sites through one shell method.

## Decision

Extract the assembly/finalization sequence into the Qt-free generic
`DocumentTabCreationCoordinator[OpenedT, TabT, EditorT]`. It receives explicit
factories and projection callbacks for editor creation, tab construction,
surface add, title/modified calculation, title refresh, session save, and
status synchronization.

MainWindow retains document service results, editor/settings policy, the
`_DocumentTab` factory, recovery snapshot identity, and all user-facing
document/open/recovery consequences.

## Invariants

1. `presentation/document_tab_creation_coordinator.py` imports no PyQt6,
   editor implementation, DocumentTabSurface, EditorDocumentSurface, or
   concrete application result/tab type.
2. The sequence remains editor creation → tab construction → tab surface add
   (including the existing currentChanged side effect) → title refresh →
   session-save request → status synchronization.
3. `recovery_snapshot_id` is passed unchanged into the tab factory.
4. Modified state is calculated at projection time exactly as before.
5. MainWindow keeps settings/theme/editor policy and no document/open/recovery
   outcome policy moves into the coordinator.

## Alternatives considered

- **Leave `_add_tab` in MainWindow:** rejected; editor assembly and post-add
  projection were a separate presentation composition concern.
- **Move editor/settings policy into the coordinator:** rejected; those are
  concrete editor adapter and application settings concerns.
- **Make the coordinator import `_DocumentTab` or EditorWidget:** rejected;
  generic factories preserve the engine and record seams.
- **Move open/recovery event or notification consequences:** rejected; the
  coordinator only creates/projections a tab and does not own use-case outcome
  policy.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Huygens the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Fermat the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for factory/projection order, currentChanged
  timing, recovery snapshot binding, modified calculation, and retained
  document/settings policy.
- Simplification assessment: one generic assembly sequence replaces the shell
  method without a new framework or second tab state model. No further safe
  behavior-preserving reduction was identified.

## Verification target and limits

- `D95-DOCUMENT-TAB-CREATION-QT-FREE-BOUNDARY-PROBE=PASS` covers the import
  boundary, MainWindow wiring, delegation, and removal of assembly policy from
  the shell method.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D95 handoff.
- Native editor creation/currentChanged timing, accessibility, DPI, font
  metrics, runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
