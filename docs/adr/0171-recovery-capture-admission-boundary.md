# ADR-0171: Recovery capture admission boundary

- **Status:** accepted-with-limits; D132 / ARCH-109 bounded presentation slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow` still combined recovery autosave gates, dirty-tab selection,
snapshot identity, dirty-state normalization, content-version capture, and
the channel/editor capture pipeline. `RecoveryCaptureTracker`,
`RecoveryCaptureAbortCoordinator`, and `RecoveryWriteCoordinator` already
owned separate lifecycle seams, but the admission cluster remained embedded in
the Qt composition root.

## Decision

Introduce the Qt-free `RecoveryCaptureAdmissionCoordinator` with frozen/slotted
`RecoveryCaptureAdmissionPorts`. It owns only:

- recovery-available and busy admission;
- stable dirty-tab candidate filtering;
- document-inflight and delete-pending exclusion;
- snapshot ID reuse or generation and assignment;
- dirty-state normalization; and
- content-version capture.

The coordinator emits an immutable `RecoveryCaptureAdmission` and invokes the
existing starter before inspecting the next tab, preserving the original
per-tab sequencing. MainWindow remains responsible for channel creation and
backpressure, `EditorWidget.begin_text_capture`, `_RecoveryCaptureJob`,
tracker registration, writer dispatch, QTimer slices, abort/write lifecycle,
notifications, and close/application policy.

## Invariants

1. Recovery availability is checked before busy state, matching the original
   short-circuit order.
2. Tabs are considered in existing tab order; each admitted candidate is handed
   to its starter before the next tab is inspected.
3. Inflight documents, clean tabs, and snapshots with delete work pending are
   skipped exactly as before.
4. Existing snapshot IDs are reused; otherwise a new ID is generated and set
   before state/content capture.
5. A clean `DocumentState` is normalized dirty without mutating the original
   state object; content version is captured from the same tab.
6. The coordinator owns no Qt, editor, channel, recovery service, tracker,
   writer, timer, filesystem, or global application state.

## Alternatives considered

- **Keep admission in MainWindow:** rejected; it leaves a complete recovery
  policy cluster coupled to the composition root after lifecycle seams exist.
- **Move channel/editor capture and QTimer now:** rejected for this slice; it
  would combine backpressure, editor cursor safety, worker dispatch, and abort
  timing into a higher-risk extraction.
- **Batch all admissions before starting any capture:** rejected; it changes
  snapshot generation and starter timing relative to the existing loop.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Schrodinger the 4th / Luna max; two bounded read-only waits timed
  out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`.
- Independent review: Pasteur the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for gate/branch order, tab sequencing, snapshot/state/
  version capture, named port mapping, and unchanged channel/editor/tracker/
  writer/timer ownership.
- Simplification assessment: PASS. The coordinator extracts only candidate
  admission; no lifecycle or generic workflow abstraction was introduced.

## Verification target and limits

- Authorized evidence: recovery admission behavior/contract probes, compileall,
  Ruff, format, project checks, package identity, root/dist identity,
  no-process evidence, handoff/register/index synchronization, and expected
  release NO-GO evidence.
- Not proven: native recovery UI, channel/backpressure timing, editor capture
  timing, QTimer slices, filesystem durability, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
