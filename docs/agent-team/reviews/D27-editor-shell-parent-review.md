# D27 parent review — editor shell surface

| Field | Value |
|---|---|
| Slice | D27 / ARCH-18 central editor shell surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Ampere the 2nd / Luna max; bounded read-only review window returned no conclusion; no PASS is claimed |
| Scope | `editor_shell_surface.py`, MainWindow central-shell construction/retranslate sites, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; parent source review and explicit independent no-conclusion record |

## Outcome

`EditorShellSurface` now owns the central `editorShell` QWidget, zero-margin
vertical layout, DocumentTabSurface and FindSurface composition, initial hidden
FindBar state, and FindBar locale routing. MainWindow retains all semantic
callbacks, document/editor state, operation policy, tab lifecycle, and command
behavior.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: the surface preserves the `editorShell` object
  name, zero layout margins, tab-first/FindBar-second ordering, and initial
  hidden FindBar state.
- PASS by source reasoning: DocumentTabSurface keeps the MainWindow close and
  current-tab callbacks; FindSurface keeps all six existing semantic routes.
- PASS by source reasoning: the central shell owns the child QWidget parentage
  and is installed exactly once through `setCentralWidget()`.
- NOT RUNTIME-VERIFIED: QApplication startup, tab/FindBar interaction,
  focus, visual hierarchy, accessibility, DPI, and screen-reader behavior
  remain unrun under the no-launch policy.

### Architecture and policy

- PASS by source reasoning: MainWindow no longer imports or constructs
  `QWidget`, `QVBoxLayout`, `DocumentTabSurface`, or `FindSurface` directly;
  it consumes the existing semantic child-surface APIs through one composite.
- PASS by source reasoning: EditorShellSurface imports only Qt, domain Locale,
  and existing presentation surfaces; it has no application, document,
  persistence, or TaskRunner dependency.
- PASS by source reasoning: MainWindow still owns `_close_tab`,
  `_on_current_tab_changed`, Find/Replace operations, Replace All lifecycle,
  tab records, and all document/session/recovery consequences.
- PASS by source reasoning: locale refresh routes through
  `EditorShellSurface.set_locale()` without introducing a second locale model.
- PASS by source reasoning: no general layout framework, service locator,
  event bus, singleton, or speculative coordinator was introduced.

## Independent review

Ampere the 2nd / Luna max was assigned a bounded read-only D27 architecture
review with no write access, no Qt launch, and no test creation/run. Two
bounded wait windows returned no conclusion before the reviewer was closed. No
child PASS or FAIL is claimed. The parent records this exact limitation and
accepts D27 only with static source reasoning plus authorized deterministic
checks. A later independent review is required before any release-level
confidence claim.

## Simplification assessment

The extraction removes central QWidget/layout composition and four concrete
presentation imports from MainWindow while preserving the existing semantic
child-surface APIs and callback ownership. The composite has one clear reason
to change—central editor-shell layout/parentage—and no further safe
behavior-preserving simplification is required for this slice.

## Public-source applicability and embedded gate

This is a Python/PyQt6 desktop change, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded code-review simplifier are **N/A**
for MCU/vendor constraints: no MCU, SDK, RTOS, ISR, DMA, driver, boot,
Flash/NVM, power, or hardware target was changed. Public architecture
references are engineering references only: [CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They are not private ByteDance standards, manufacturer requirements,
certification evidence, or a release-readiness claim.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Non-destructive compile gate. |
| `uv run ruff check src\quillforge` | PASS | Import/lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate; 78 source files were already formatted. |
| D27 source boundary probe | PASS | EditorShellSurface owns central shell/layout/child composition; MainWindow retains callbacks and policy. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D27 portable candidate; root/dist SHA `8EFBFD85EF378E0A23DC55E6E33468F52EBEF5F72AB4944DDFFB102110E53655`, size `38,392,493` bytes; source `tree-sha256:d648b1e3b3ebb9fdd461934fe94e7a64673ee227642221ee30c75fda3afff174`. |
| `scripts/verify_handoff.ps1` | PASS | D27 handoff/index/register/acceptance status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | D27 source, formatting, inventory, and static project checks passed. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent review has no conclusion and must be revisited for future
  release-level confidence; this is explicitly not a PASS claim.
- Runtime central-shell parentage, tab/find interaction, visual hierarchy,
  accessibility, and cross-machine rendering remain unverified.
- Existing D7/D8 and release gates remain open and are not narrowed by D27.

**Disposition:** `accepted-with-limits`; D27 editor-shell composition is
source-level verified by the parent with an explicit independent
no-conclusion record and all runtime/external release gates open.
