# D286 parent review: editor-shell startup preflight

Status: PASS (bounded source/package review)  
Delivery: D286 / ARCH-256  
Reviewer: parent architect/developer

## Scope reviewed

- `src/quillforge/presentation/main_window.py`
- `src/quillforge/composition.py`
- `src/quillforge/app.py`
- `scripts/audit_presentation_contracts.py`
- the D286 PyInstaller candidate and release manifest

## Findings

- PASS — the preflight reuses the production `ensure_initial_document()` path,
  so editor, lexer, font, palette, tab projection, and signal wiring are not
  duplicated in diagnostics.
- PASS — `finally` stops both existing startup timers even if editor
  construction raises; the exception remains visible to the outer diagnostic
  probe.
- PASS — the probe never restores session state, opens paths, shows a window,
  or enters the event loop, and no file write is reachable from this path.
- PASS — normal startup remains unchanged; session/recovery restoration still
  controls initial-document creation in the real lifecycle.
- PASS — static contract and package/archive evidence bind the new code to the
  D286 candidate.

## Simplification assessment

PASS. Reusing `ensure_initial_document()` is smaller and safer than creating a
second editor assembly path. Timer cleanup belongs in the presentation method,
where the existing timer ownership is known; moving it to the application
diagnostic would leak Qt state across the layer boundary.

## Limits

No EXE/Qt native launch, asynchronous restore completion, clean-machine
startup, user interaction, native rendering, or cross-machine evidence was
run. The probe validates editor construction, not native window lifetime.

## Public source applicability and embedded gate

Applicable public references are Python 3.12 `try`/`finally` semantics and
public Qt 6 timer/application lifecycle documentation, recorded in ADR-0322.
No manufacturer requirement applies. Embedded C/C++ assurance is N/A.
