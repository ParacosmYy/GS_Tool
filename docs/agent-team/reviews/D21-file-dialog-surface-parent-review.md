# D21 parent review — file dialog surface

| Field | Value |
|---|---|
| Slice | D21 / ARCH-12 MainWindow file-dialog surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Luna path-policy audit returned REVISE before D21 registration; concerns resolved by explicit scope/acceptance. Mencius / Luna max returned CONCERNS on the startup-restore guard; the parent applied and rechecked that fix. |
| Scope | `file_dialog_surface.py`, three MainWindow dialog call sites, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; independent review evidence and source evidence recorded |

## Outcome

`src/quillforge/presentation/file_dialog_surface.py` now owns the three native
path-selection intents: `choose_document()` calls `getOpenFileName`,
`choose_workspace()` calls `getExistingDirectory`, and `choose_save_path()`
calls `getSaveFileName`. MainWindow owns the surface and delegates localized
selection while retaining every busy/startup guard, async service call, path
policy, save policy, and notification/error consequence.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: document selection uses the file-only native API;
  it is not accidentally replaced by the directory picker.
- PASS by source reasoning: workspace selection uses the directory-only native
  API and converts the selected path once before dispatch.
- PASS by source reasoning: Save As preserves the previous `Untitled.txt`
  fallback, current-path default, localized title/filter, and cancellation
  result of `None`.
- PASS by source reasoning: locale refresh reaches FileDialogSurface from the
  existing `_retranslate_ui()` path.
- PASS after independent review fix: `_open_document()` now rejects a native
  picker while startup session restoration is in flight, matching the
  workspace guard instead of discarding a user's selected path later in
  `_start_open()`.
- NOT RUNTIME-VERIFIED: QApplication startup, native dialog delivery,
  extension-filter behavior, focus, DPI, accessibility, and visual appearance
  remain unrun under the permanent no-launch policy.

### Architecture and policy

- PASS by source reasoning: MainWindow has no `QFileDialog` import or direct
  call after D21; its semantic methods still call `_start_open`,
  `_start_workspace_open`, and the existing save flow.
- PASS by source reasoning: FileDialogSurface imports only `Path`, Qt,
  `Locale`, and i18n; it has no application or infrastructure dependency.
- PASS by source reasoning: no workspace containment, session/recovery,
  document I/O, worker state, or second path-policy model was added.
- Explicit decision: direct document Open and Save As may continue to select
  arbitrary local paths. Workspace-tree and search-result containment remain
  in MainWindow/application services and were not weakened or moved.

## Independent review

A bounded Luna path-policy audit reviewed the six path-entry families and
returned **REVISE** because D21 had not yet been registered in the ADR,
acceptance, or handoff ledger and because cross-workspace policy was implicit.
It also identified that FileDialogSurface must not absorb containment or
session policy. The parent resolved the review before acceptance by adding
ADR-0046, D21-AC01/S50, the D21 handoff/register/index entries, and an explicit
decision that cross-workspace Open/Save As and session-restore policy are
unchanged and out of scope. This is recorded as a resolved review concern,
not as a claim that runtime dialogs were tested.

Mencius / Luna max then performed a focused read-only source review and
returned **CONCERNS**: `_open_document()` lacked the startup-restore guard,
which could open a picker and discard its selection after the user chose a
path. The parent added the guard, reran compile/Ruff/format and the source
boundary probe, and rebuilt the package. The concern is resolved; no Qt
runtime dialog PASS is claimed.

## Simplification assessment

The extraction removes three native dialog call blocks, repeated locale/filter
plumbing, and three `Path(selected)` conversions from MainWindow. The surface
has one locale field and three semantic methods; it adds no factory, registry,
callback graph, or duplicate path state. No further behavior-preserving
simplification is required for this bounded slice.

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
| `uv run ruff check src\quillforge` | PASS | Import/lint gate after ordering fix. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D21 source boundary probe | PASS | MainWindow has no QFileDialog; surface exposes file, directory, and save APIs; MainWindow retains async/policy call sites. |
| JSON acceptance/register/index parse | PASS | D21 acceptance, delivery register, and handoff index parse after synchronization. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 75 formatted files. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D21 candidate; root/dist identity was `4FE6A679B03A597009B8B2F310FF2493522EE4C142866F30A787A2648538BB53` / `38,386,277` bytes; source `tree-sha256:08c9c1550303dc685ce29cdf62f16462cba870a70134f5a4a50d86669ab20431`. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Native file-dialog behavior, filters, focus, accessibility, DPI, and visual
  styling remain unverified.
- The UI filter is advisory; downstream document-service limits remain the
  actual I/O boundary.
- Direct Open/Save As path policy, session restore, and symlink/reparse final
  identity remain existing boundaries and are not D21 security claims.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; D21 semantic file/folder/save dialog
separation is source-level verified, with the pre-registration review concern
resolved and runtime/external release gates explicitly open.
