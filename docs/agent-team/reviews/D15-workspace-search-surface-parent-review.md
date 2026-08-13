# D15 parent review — workspace-search surface coordinator

| Field | Value |
|---|---|
| Slice | D15 / ARCH-06 MainWindow workspace-search surface coordinator |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Schrodinger / Luna max; bounded read-only review returned PASS at source level |
| Scope | `workspace_search_surface.py`, `workspace_search_dialog.py`, `workspace_panel.py`, `main_window.py`, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits |

## Outcome

`src/quillforge/presentation/workspace_search_surface.py` now owns creation
and parent lifetime of `WorkspaceSearchDialog`, the three external semantic
signal routes, non-modal activation, root/locale projection, busy/cancel
feedback, and result/error/cancelled projection. `MainWindow` stores the
surface and no longer constructs or directly wires the search dialog.

The application boundary remains in `MainWindow`: it constructs and validates
`WorkspaceSearchQuery`, submits `WorkspaceSearchService` work through
`TaskRunner`, retains operation IDs/generations/cancellation events, rejects
stale completions, rechecks workspace containment before opening a result, and
owns notifications and session/workspace ordering.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: the surface creates one dialog with the existing
  MainWindow parent, connects search `(str, bool)`, cancel `()`, and file
  `(object, int)` routes to the same MainWindow callbacks, then preserves the
  existing show/raise/activate sequence.
- PASS by source reasoning: `_show_workspace_search()` compares the surface's
  current root, invalidates an active search before calling `set_root()`, and
  then shows the same surface. `set_root()` still clears only stale rows and
  diagnostics while retaining the explicit root.
- PASS by source reasoning: busy, cancellation, result, recoverable error, and
  stale/cancelled completion calls are delegated without changing the
  operation ID or generation branches.
- PASS: locale refresh now has one workspace surface owner; the old duplicate
  direct panel locale call was removed. `WorkspacePanel.set_locale()` also
  refreshes the existing truncated-entry marker without replacing the current
  directory page.
- PASS: the defensive invalid-directory branch no longer dereferences a null
  panel while still notifying the user; the normal panel projection path is
  unchanged.
- NOT RUNTIME-VERIFIED: QApplication startup, signal delivery, dialog
  parenting/destruction, focus/activation, search interaction, result
  double-click, visual locale, and file opening remain unrun under the
  permanent no-launch policy.

### Architecture and security

- PASS: the surface imports Qt, domain `Locale`, and the existing presentation
  dialog only; it selects no service, touches no infrastructure, owns no
  worker, and contains no path-containment or execution policy.
- PASS: `WorkspaceSearchSurfaceCallbacks` is a small frozen/slots contract with
  exactly three semantic routes. No event bus, service locator, singleton,
  widget registry, network path, plugin execution path, or second search state
  model was introduced.
- PASS: user-selected result paths still cross the existing
  `WorkspaceService.contains()` check before `MainWindow` opens a document.

## Independent review

Schrodinger / Luna max was assigned a bounded read-only D15 review with no
write access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** and reported no blocking FAIL.
The review also records Qt signal delivery, parent lifetime, focus/activation,
double-click, and compile/runtime limitations as unverified.

## Simplification assessment

The slice removes dialog construction, three external signal-connect lines,
and repeated activation calls from `MainWindow`, replacing them with one
explicit callback record and one surface. It keeps all async state in the
existing coordinator, avoids a second state model, and removes a duplicate
workspace locale projection discovered during review. The invalid-directory
guard is now explicit instead of relying on an unreachable normal composition
path. The independent review found no further safe simplification required
for this bounded change.

## Public-source applicability and embedded gate

This is a Python/PyQt6 desktop change, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded simplifier are therefore **N/A** for
vendor/MCU constraints: no MCU, SDK, RTOS, ISR, DMA, driver, boot, Flash,
power, or hardware target was changed. The public architecture references are
engineering references only: [CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They are not private ByteDance standards, manufacturer requirements,
certification evidence, or a release-readiness claim.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS after final source repair |
| `uv run ruff check src\quillforge` | PASS after final source repair |
| `uv run ruff format --check src\quillforge` | PASS after final source repair |
| D15 AST/source boundary and lifecycle probe | PASS by source reasoning; final exact probe recorded with package evidence |
| JSON parse for acceptance/register/index | PASS after D15 entries were added |
| `scripts/verify_handoff.ps1` | PASS | D15 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | D15 portable candidate rebuilt; root/dist identity is `9CA0ACC937DDA93AF5F440A3994C840F226C6A3FDE7A02C2645C84B0797D3977` / `38,378,840` bytes. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Dossier refreshed; the known three report failures and ten open release gates remain. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- MainWindow still owns workspace async/search/session orchestration; recovery,
  session, plugin, and remaining coordinator slices are future work.
- Runtime dialog activation, native metrics, accessibility, fonts, DPI,
  search timing, and file-result activation remain unverified.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`, conditional on final handoff/index,
static, package, artifact, and release-no-go evidence.
