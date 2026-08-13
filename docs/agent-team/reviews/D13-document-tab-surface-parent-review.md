# D13 parent review — document-tab surface coordinator

| Field | Value |
|---|---|
| Slice | D13 / ARCH-04 MainWindow document-tab surface coordinator |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Kierkegaard / Luna max; bounded read-only review returned no conclusion and was closed |
| Scope | `document_tab_surface.py`, `main_window.py`, architecture/acceptance/handoff projections |
| Decision | accepted-with-limits |

## Outcome

The document-tab Qt projection is now isolated in
`src/quillforge/presentation/document_tab_surface.py`. MainWindow delegates
the tab widget composition at lines 203–220 and no longer owns `_tabs_widget`
or a second tab-record list. Existing document behavior remains in MainWindow:
`_add_tab` (line 941), `_close_tab` (line 1919), `_remove_tab` (line 1953),
editor callbacks, recovery/session state, path uniqueness, and lifecycle event
publication.

The surface is intentionally a bounded projection coordinator, not a document
service. Its structural `DocumentTabLike` contract (line 13) requires only an
`EditorWidget`; the coordinator owns the Qt widget, record collection, current
index, identity lookup, title sync, and tab-bar enablement (lines 22–110).

## Review findings

### Boundary and ownership

- PASS: `DocumentTabSurface` imports only Qt and presentation `EditorWidget`;
  it has no application-service or infrastructure dependency.
- PASS: MainWindow still decides dirty/save/close/recovery/session behavior and
  supplies the close/current callbacks; the surface does not interpret them.
- PASS: `MainWindow._active_tab`, `_find_tab`, `_contains_tab`, and
  `_update_tab_title` delegate to the surface rather than duplicating indexes.
- PASS: the workspace, recovery, session, plugin, and command behavior
  boundaries are not moved in this slice.

### Ordering and lifecycle reasoning

- PASS by source reasoning: `add_tab` appends the record before calling
  `addTab`/`setCurrentIndex`, so a first-tab `currentChanged` callback can
  resolve the active record.
- PASS by source reasoning: `remove_tab` removes the record before
  `QTabWidget.removeTab`, so a resulting current-change callback observes the
  remaining collection. MainWindow performs recovery cancellation, event
  publication, editor `deleteLater`, and replacement-tab policy around that
  projection call.
- PASS: membership uses object identity, preventing equal-looking records from
  resolving to the wrong tab.
- NOT RUNTIME-VERIFIED: Qt signal emission timing, widget ownership/destruction,
  visual title rendering, tab-bar state, and callback delivery were not
  exercised because the project no-launch policy prohibits QApplication/Qt
  startup and interactive validation.

## Independent review

Kierkegaard / Luna max was assigned a read-only review with no write access,
no Qt launch, and no test creation/run. Two bounded waits expired while the
reviewer remained running; the reviewer was closed. No child conclusion is
treated as PASS. This is recorded as an independent-review limitation rather
than substituted evidence.

## Simplification assessment

The extraction removes direct `QTabWidget` assembly and duplicated list/index
operations from MainWindow while adding one small, explicit coordinator. It
does not add a container, service locator, singleton, event bus, duplicated
document source of truth, or speculative adapter hierarchy. The generic
structural contract is limited to the editor projection needed by the tab rail.
The `set_current`/`set_title` boolean results keep ownership failures explicit
without raising new policy into the surface. No further behavior-preserving
simplification is required for this slice.

## Public-source applicability

This software slice is not embedded firmware. The repository's enterprise
architecture baseline uses public CloudWeGo/ByteDance-adjacent engineering
references only where applicable: [CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They support transferable principles such as explicit extension boundaries,
composable interfaces, and incremental verified delivery; they are not private
ByteDance standards, manufacturer requirements, certification evidence, or a
claim that QuillForge conforms to an internal corporate architecture.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS |
| `uv run ruff check src\quillforge` | PASS |
| `uv run ruff format --check src\quillforge` | PASS after formatter correction |
| D13 AST/source boundary and ordering probe | PASS |
| JSON parse for acceptance/register/handoff index | PASS |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status equality. |
| `scripts/check.ps1` | PASS | NOTICE, handoff, source, and formatting gates. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Portable candidate rebuilt after D13 source edit. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1 with exactly three mechanical failures and ten open gates; no release approval inferred. |

The probe confirmed that MainWindow contains no `QTabWidget`, `_tabs_widget`, or
`self._tabs` ownership and that the surface exposes the required projection
methods without application/infrastructure imports. No unit tests, mocks,
fixtures, harnesses, QApplication launch, screenshots, deployment, or hardware
operation were created or run.

## Open risks and disposition

- MainWindow remains a large coordinator; this is one incremental extraction,
  not completion of the enterprise migration.
- Runtime Qt signal/lifecycle behavior and visual acceptance remain unverified.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`, with handoff/index, static-check, and
package evidence refreshed after the source change; release approval remains a
separate no-go dossier.
