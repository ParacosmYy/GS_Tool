# D26 parent review — status surface host projection

| Field | Value |
|---|---|
| Slice | D26 / ARCH-17 status surface host projection |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Leibniz the 2nd / Luna max; bounded read-only review window returned no conclusion; no PASS is claimed |
| Scope | `status_surface.py`, MainWindow status-bar initialization/notification sites, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; parent source review and explicit independent no-conclusion record |

## Outcome

`StatusSurface` now owns status-bar attachment, size-grip configuration,
localized transient notification projection, locale state, and the existing
status-rail phase projection. MainWindow retains status phase precedence,
TaskRunner/document policy, notification call sites, and the single host
selection at initialization.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: `attach_to()` is idempotent for the same host and
  removes the owned rail from an earlier host before reattaching it.
- PASS by source reasoning: the existing `StatusRail` instance remains the
  permanent widget and phase/locale calls still reach it.
- PASS by source reasoning: `show_message()` uses the normalized surface
  locale, preserves the 5000 ms default, and clamps negative custom timeouts
  to zero.
- NOT RUNTIME-VERIFIED: QApplication startup, QStatusBar host behavior,
  transient message timing, focus, visual hierarchy, accessibility, DPI, and
  screen-reader behavior remain unrun under the no-launch policy.

### Architecture and policy

- PASS by source reasoning: MainWindow no longer calls
  `setSizeGripEnabled()`, `addPermanentWidget()`, or `showMessage()` directly;
  it only attaches the status surface and delegates notifications.
- PASS by source reasoning: StatusSurface imports only Qt, the domain Locale,
  the existing i18n adapter, and StatusRail; it imports no application service,
  document state, TaskRunner, persistence adapter, or operation policy.
- PASS by source reasoning: `_retranslate_ui()` continues to route locale
  changes through the same `StatusSurface.set_locale()` seam.
- PASS by source reasoning: no second phase precedence model, event bus,
  singleton, or speculative notification service was introduced.

## Independent review

Leibniz the 2nd / Luna max was assigned a bounded read-only D26 review with no
write access, no Qt launch, and no test creation/run. Two bounded wait windows
returned no conclusion before the reviewer was closed. No child PASS or FAIL is
claimed. The parent records this exact limitation and accepts D26 only with
static source reasoning plus authorized deterministic checks. A later
independent review is required before any release-level confidence claim.

## Simplification assessment

The extraction removes direct status-bar host calls and notification
localization from MainWindow while preserving all call sites, phase policy, and
the existing 5000 ms behavior. The API is limited to attach, locale, message,
phase, and the existing rail widget projection; no further safe
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
| `uv run ruff format --check src\quillforge` | PASS | Format gate; 77 source files were already formatted. |
| D26 source boundary probe | PASS | StatusSurface owns attach/size-grip/permanent rail/showMessage/localization; MainWindow retains phase/policy and delegates notify. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D26 portable candidate; root/dist SHA `34337021DF82E9144F97F68CEC79292DD08749DBC406D6BECA4F54F074B62ED8`, size `38,392,052` bytes; source `tree-sha256:ed91c1474dfb6b9dadc6ed1cfb606bd14f6315096b08d7151f6e97c26a06a831`. |
| `scripts/verify_handoff.ps1` | PASS | D26 handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 79 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent review has no conclusion and must be revisited for future
  release-level confidence; this is explicitly not a PASS claim.
- Runtime status-bar attachment, notification timing, visual hierarchy,
  accessibility, and cross-machine rendering remain unverified.
- Existing D7/D8 and release gates remain open and are not narrowed by D26.

**Disposition:** `accepted-with-limits`; D26 status host composition is
source-level verified by the parent with an explicit independent
no-conclusion record and all runtime/external release gates open.
