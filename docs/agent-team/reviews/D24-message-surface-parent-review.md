# D24 parent review — message surface

| Field | Value |
|---|---|
| Slice | D24 / ARCH-15 MainWindow message surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Rawls the 2nd / Luna max; bounded read-only review window returned no conclusion; no PASS is claimed |
| Scope | `message_surface.py`, MainWindow message call/retranslate/close sites, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; parent source review and explicit independent no-conclusion record |

## Outcome

`src/quillforge/presentation/message_surface.py` now owns localized common
message-dialog composition for save-before-close, About, and recoverable
errors. MainWindow retains dirty-state, Save As, asynchronous save, tab-close,
error-phase, and status synchronization policy.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: `ask_save_before_close()` maps Save to `save`,
  Discard to `discard`, and Cancel or any unexpected result to `cancel`.
- PASS by source reasoning: the dirty close path returns immediately on
  `cancel`, starts the existing asynchronous save path on `save`, and removes
  the tab only after the existing save callback; discard keeps the existing
  direct removal path.
- PASS by source reasoning: `show_about()` and `show_error()` use the current
  normalized locale and the existing parent window.
- PASS by source reasoning: `_show_error()` still sets the error phase before
  the modal and synchronizes the status surface after it returns.
- NOT RUNTIME-VERIFIED: QApplication startup, modal button delivery, focus,
  visual hierarchy, accessibility, DPI, and screen-reader behavior remain
  unrun under the no-launch policy.

### Architecture and policy

- PASS by source reasoning: MainWindow no longer imports or calls
  `QMessageBox`; it consumes only the surface's semantic decision and keeps
  Save As/path selection and save consequences in the coordinator.
- PASS by source reasoning: MessageSurface imports only Qt, the domain
  Locale, and the existing i18n adapter; it does not import application
  services, document state, TaskRunner, or infrastructure.
- PASS by source reasoning: `_retranslate_ui()` routes locale changes through
  `MessageSurface.set_locale()` along with the other presentation surfaces.
- PASS by source reasoning: no second close guard, save state, error phase
  model, event bus, singleton, or speculative abstraction was added.

## Independent review

Rawls the 2nd / Luna max was assigned a bounded read-only D24 review with no
write access, no Qt launch, and no test creation/run. Two bounded wait windows
returned no conclusion before the reviewer was closed. No child PASS or FAIL
is claimed. The parent records this exact limitation and accepts D24 only with
static source reasoning plus authorized deterministic checks. A later
independent review is required before any release-level confidence claim.

## Simplification assessment

The extraction removes three direct QMessageBox composition paths and their
locale plumbing from MainWindow while preserving the existing policy and
consequence code. The typed save decision is smaller and clearer than
coordinating raw QMessageBox flags at every call site. No further safe
behavior-preserving simplification is required for this bounded slice.

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
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D24 source boundary probe | PASS | MessageSurface owns QMessageBox composition; MainWindow owns close/save/error/status consequences and locale routing. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D24 portable candidate; root/dist SHA `FFAE5D29F4C6C2540EC10C66EE86C54A8B249C9E260C63467836E670571D435E`, size `38,390,152` bytes; source `tree-sha256:8b6221c3d30f518b1f975afb3994ca96a1105cc4532bf22511f8f7f99af27f46`. |
| `scripts/verify_handoff.ps1` | PASS | D24 handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 78 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent review has no conclusion and must be revisited for future
  release-level confidence; this is explicitly not a PASS claim.
- Runtime message behavior, visual hierarchy, accessibility, and
  cross-machine rendering remain unverified.
- Existing D7/D8 and release gates remain open and are not narrowed by D24.

**Disposition:** `accepted-with-limits`; D24 message composition is
source-level verified by the parent with an explicit independent
no-conclusion record and all runtime/external release gates open.
