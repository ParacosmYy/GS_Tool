# D14 parent review — workspace surface coordinator

| Field | Value |
|---|---|
| Slice | D14 / ARCH-05 MainWindow workspace surface coordinator |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Kuhn / Luna max; bounded read-only review expired twice with no conclusion; reviewer closed |
| Scope | `workspace_surface.py`, `main_window.py`, `workspace_panel.py`, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits, pending final evidence refresh |

## Outcome

`src/quillforge/presentation/workspace_surface.py` now owns the workspace dock
and panel composition. `WorkspaceSurfaceCallbacks` at lines 17–24 provides the
five semantic intents already emitted by `WorkspacePanel`; `WorkspaceSurface`
at lines 27–56 creates the panel/dock, preserves the `WorkspaceDock` object
name and left-dock placement, connects the existing payloads, and retranslates
the dock and panel together.

MainWindow composes the surface at lines 225–242 and exposes only a read-only
panel projection at line 286 for applying current application results. It
continues to own `WorkspaceService`, TaskRunner submission, operation IDs,
generation/stale-result guards, cooperative cancellation, root containment,
workspace-search dialog state, session-restore barrier, notifications, and
error policy.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: the panel is created before signal connections and
  is parented through the existing MainWindow/dock ownership path; the dock is
  created with MainWindow as parent and receives the panel through
  `setWidget`.
- PASS by source reasoning: all five routes remain explicit and payload-stable:
  folder/no-argument, directory/object, file/object, back/no-argument, and
  cancel/no-argument.
- PASS: `set_locale` updates the dock title and delegates panel labels without
  replacing the panel or clearing its directory page.
- PASS: MainWindow still controls every async workspace/search/session state
  transition and keeps root containment before opening a file.
- NOT RUNTIME-VERIFIED: Qt signal delivery, QWidget reparenting/destruction,
  dock placement, visual labels, and first-click file activation remain unrun
  under the permanent no-launch policy.

### Architecture and security

- PASS: the surface imports only Qt, domain `Locale`, i18n, and the existing
  presentation `WorkspacePanel`; no application or infrastructure dependency
  was introduced.
- PASS: callbacks are explicit semantic callables; no service container,
  widget registry, event bus, singleton, network access, or plugin execution
  path was added.
- PASS: user-selected paths still cross the existing MainWindow containment
  check before document opening; this extraction does not widen trust.

## Independent review

Kuhn / Luna max was assigned a bounded read-only review with no write access,
no Qt launch, and no test creation/run. The initial two waits expired without a
conclusion, so D14 was first recorded as no-conclusion. A delayed completion
then returned **CONCERNS** (no blocking FAIL): it identified duplicate direct
panel locale projection in MainWindow, a stale-language truncated-directory
marker after locale changes, and a defensive null-panel dereference in an
invalid-directory branch. D15 removes the duplicate call, reprojects the
marker, and hardens the null guard; the D15 review and handoff carry the
remediation evidence.

## Simplification assessment

The change removes dock construction and five repeated signal-connect lines
from MainWindow, while keeping one small explicit callback record. It does not
duplicate workspace state or move async policy into presentation. The
read-only `_workspace_panel` property is a compatibility projection over the
surface's single panel owner; it adds no second state source. No further
behavior-preserving simplification is required for this bounded slice.

## Public-source applicability and embedded gate

This is a Python/PyQt6 desktop change, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded simplifier are therefore **N/A** for
vendor/MCU constraints: no MCU, SDK, RTOS, ISR, DMA, driver, boot, Flash, or
hardware target was changed. The enterprise architecture references remain
public engineering references only: [CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They support explicit boundaries and composable extension points; they are not
private ByteDance standards, manufacturer requirements, certification, or a
release claim.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS |
| `uv run ruff check src\quillforge` | PASS |
| `uv run ruff format --check src\quillforge` | PASS |
| D14 AST/source boundary and signal-route probe | PASS |
| JSON parse for acceptance/register/handoff index | PASS | D14 IDs, index, register, and release manifest parse. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status equality. |
| `scripts/check.ps1` | PASS | NOTICE, handoff, source, and formatting gates. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Portable candidate rebuilt after D14 source edit. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; three known mechanical report failures and ten open gates. |

The boundary probe confirms MainWindow contains no `QDockWidget` construction,
`WorkspacePanel(...)` construction, or `_workspace_dock` state, while the new
surface owns the five routes and has no application/infrastructure import.
No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- MainWindow still owns workspace async/search/session orchestration; those are
  future bounded slices.
- Runtime dock parenting, accessibility, native metrics, DPI, fonts, and file
  activation remain unverified; the delayed source concerns are remediated in
  D15 and do not change the D14 artifact's historical identity.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`, conditional on final independent-review
record, handoff/index, static, package, artifact, and release-no-go evidence.
