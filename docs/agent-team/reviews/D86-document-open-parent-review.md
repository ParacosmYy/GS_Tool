# D86 / ARCH-61 parent review: document-open coordinator

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** Generic completion remains first; stale callbacks
  return without consuming restore state. Current callbacks preserve ordinary
  and session invalid-result/failure behavior and pass line-number context to
  the existing valid policy.
- **Readability — PASS:** One Qt-free coordinator owns completion
  classification; MainWindow's valid open method visibly owns tab/editor and
  session consequences.
- **Architecture — PASS:** The coordinator depends only on application/domain
  result types, callable seams, and the notification contract. It has no Qt,
  tab surface, editor, service, filesystem, or event-bus authority.
- **Security/data safety — PASS:** No new path resolution, file access, or
  trust boundary exists. Document service and existing path identity checks are
  unchanged.
- **Performance — PASS:** No worker, decode, tab, or editor work was added;
  callback dispatch and session queue progression remain unchanged.

## Behavior review

1. `_start_open` still begins one generic operation and submits the same
   `DocumentService.open_document` call. Session restores bind their operation
   before submission; ordinary opens do not.
2. `DocumentOpenCoordinator.complete` rejects stale IDs before reading the
   restore tracker. Current restore callbacks consume the matching
   `SessionDocument` once, matching the former callback order.
3. Ordinary invalid results still call `_show_error("Open failed", ...)`;
   restore invalid results keep the warning and continue the queue.
4. Valid `OpenedDocument` results delegate line-number and restore context to
   MainWindow. Duplicate-path rejection, `_add_tab`, editor line/cursor
   projection, restored-tab recording, `DocumentOpened`, success notification,
   and restore continuation remain there.
5. Ordinary failures still show `Operation failed`; restore failures consume
   the bound document, warn with its filename when present, and continue.
6. Save completion/failure remains untouched, including tab liveness, editor
   read-only restoration, language projection, recovery cleanup, and
   `DocumentSaved` policy.

## Simplification assessment

The extraction removes duplicated open/failure lifecycle code without hiding
tab policy behind a broad interface. Keeping `DocumentOpenCoordinator` separate
from save is simpler than a generic document-operation abstraction because the
two callbacks have different state and recovery semantics. No further safe
simplification was identified.

## Review-role evidence

Copernicus the 3rd / Luna max architecture and Hubble the 3rd / Luna max
independent review both returned `NO_CONCLUSION` after bounded windows. No
child PASS is claimed.

## Verification and limits

The D86 Qt-free boundary probe, targeted compileall, Ruff, format, package
identity, handoff, repository checks, and expected release no-go evidence are
required before delivery. Native tab/editor/session timing, runtime startup,
accessibility, DPI, clean-machine, cross-machine, and external release gates
remain unrun or open.
