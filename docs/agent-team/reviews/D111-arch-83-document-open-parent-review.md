# D111 / ARCH-83 parent review: document-open dispatch boundary

## Scope

Reviewed D111 changes in
`src/quillforge/presentation/document_open_coordinator.py` and
`src/quillforge/presentation/main_window.py` against the D111 specification.

## Findings

| Axis | Result | Evidence |
|---|---|---|
| Contract shape | PASS | Typed open operation, success, failure, and dispatcher contracts are explicit; `submit(...)` binds the optional line number by keyword. |
| Lifecycle/order | PASS | Callbacks enter the established `complete()`/`fail()` paths; stale completion, session identity, restore continuation, document projection, and line navigation order remain unchanged. |
| Error behavior | PASS | Ordinary invalid results and worker failures retain existing error projection; session-restore invalid/failure paths still notify and continue; synchronous dispatcher errors propagate. |
| Dependency direction | PASS | The coordinator remains Qt-free and independent of TaskRunner, DocumentService, filesystem, and widgets; MainWindow retains concrete open policy. |
| Scope/control | PASS | Only duplicate open callback binding is removed; path selection, restore binding, operation IDs, service, runner, status, persistence, and close policy remain in MainWindow. |

## Simplification assessment

PASS. Extending the current open lifecycle owner is the smallest complete
change. No second coordinator, generic runner wrapper, new state, or
compatibility shim is needed. The explicit failure closure is retained for
symmetry with the typed success callback and to keep the boundary readable.

## Review limits

Native queued timing, actual editor/document interaction, filesystem decoding,
startup, visual/accessibility rendering, clean-machine, cross-machine, and
release-owner evidence were not run under the active no-launch/authorization
boundary. The delegated Luna architecture and independent review windows
returned `NO_CONCLUSION`; no child PASS is claimed.

## Applicability

Python/PyQt6 desktop code only. Embedded C/C++ assurance and manufacturer
requirements are not applicable. Public CloudWeGo material is an engineering
reference only and not a private ByteDance standard or compliance basis.
