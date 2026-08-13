# D47 parent review — session-restore state boundary

## Scope and decision

- **Delivery:** D47 / ARCH-37 / UI-33
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

`SessionRestoreTracker` is a framework-neutral value-state boundary. It owns
the immutable session snapshot reference, ordered cursor, active path,
workspace barrier, deferred recovery paths, and the one pending document/open
operation binding. `MainWindow` still owns `SessionService`,
`RecoveryService`, `WorkspaceService`, `DocumentService`, `TaskRunner`,
generic operation/busy/status state, tab widgets, notifications, startup/close
guards, and result projection.

The architecture consultation was attempted with Huygens the 2nd / Luna max
and escalated to Dirac the 2nd / Terra max for the cross-module restore chain;
both bounded windows returned no conclusion. No architecture PASS is claimed.

## Source findings

- The tracker has no Qt import and no service, filesystem, TaskRunner, widget,
  notification, or persistence dependency.
- `has_remaining_documents` is read-only; `next_document()` is the sole
  ordered cursor consumer. The first session path therefore cannot be skipped
  by a completion check.
- `set_pending_document()` precedes `_start_open(..., session_restore=True)`;
  `_start_open` binds the returned operation ID before submission.
- `_on_opened` and `_on_session_restore_open_failed` consume only the matching
  tracker binding, then continue the existing serial queue.
- Workspace restore still sets and clears the tracker barrier through the
  existing workspace lifecycle callbacks. Recovery deferral is retained until
  the next explicit startup restore, matching the prior behavior.
- Existing-tab reuse, active-tab selection, caret projection, initial-document
  fallback, notifications, session save, and close guards remain in the Qt
  coordinator.

## Simplification assessment

The extraction is the smallest complete state-only slice. A generic mapping
would weaken the queue/callback contract; moving service calls or tab lists
would make the new boundary framework- or policy-coupled. No new service,
signal, persistence format, timer, thread, or test-only asset was introduced.

## Authorized non-destructive validation

- Session-restore boundary source probe — **PASS**.
- Qt-free tracker / legacy-field / queue-order / callback-binding probe —
  **PASS**.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- `pwsh -NoProfile -File scripts\package.ps1` — **PASS**; root/dist portable
  candidates match at 38,417,701 bytes with the recorded SHA-256.
- `scripts\verify_handoff.ps1` — **PASS**.
- `scripts\check.ps1` — **PASS**.
- `scripts\verify_release_handoff.ps1` — **EXPECTED NO-GO**; 10 open gates and
  three known report-binding failures remain.
- Independent Luna review window — **NO_CONCLUSION** after two bounded waits;
  no independent PASS is claimed.
- No unit tests, mocks, fixtures, test-only assets, QApplication, Qt/EXE
  startup, screenshots, deployment, or hardware operation were created or
  run.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; MCU, BSP/HAL, RTOS, ISR/DMA, driver,
boot, Flash/NVM, power, motor-control, and vendor-manufacturer requirements
are not applicable. Public CloudWeGo sources are engineering references only;
no private ByteDance standard, certification, or compliance claim is made.

## Limits and disposition

The tracker contract does not prove native Qt callback timing, recovery/file
I/O, thread interleavings, visual rendering, accessibility, clean-machine,
cross-machine, or release-owner behavior. The bounded slice is accepted with
those limits and remains subject to the open runtime/release gates.
