# D25 parent review — theme transition surface

| Field | Value |
|---|---|
| Slice | D25 / ARCH-16 theme transition surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Archimedes the 2nd / Luna max; initial CONCERNS were fixed and the bounded follow-up returned PASS |
| Scope | `theme_transition_surface.py`, MainWindow animation trigger/import/state sites, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; parent source review and independent follow-up PASS |

## Outcome

`src/quillforge/presentation/theme_transition_surface.py` now owns the
appearance-change opacity effect, easing, animation lifetime, replacement of a
previous animation, and finished cleanup. MainWindow retains motion gating,
central-widget selection, and the existing post-settings-save trigger order.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: MainWindow always delegates the motion decision to
  `animate(target, enabled=...)`; disabled motion, a missing target, and a
  target with an existing external graphics effect all stop/clear the owned
  transition and then return safely.
- PASS by source reasoning: the existing 220 ms, 0.72 → 1.0, OutCubic values
  are preserved.
- PASS by source reasoning: a currently stored animation is stopped before a
  replacement is installed; the surface retains the new animation until
  completion through its field and uses `DeleteWhenStopped`.
- PASS by source reasoning: the finished callback clears only the effect it
  created and releases the matching animation reference; interrupted and
  disabled paths use `_stop_current()` instead of relying on `finished`.
- NOT RUNTIME-VERIFIED: QApplication startup, animation timing, rendered
  opacity, focus, visual hierarchy, accessibility, DPI, and screen-reader
  behavior remain unrun under the no-launch policy.

### Architecture and policy

- PASS by source reasoning: MainWindow no longer imports or constructs
  `QGraphicsOpacityEffect`/`QPropertyAnimation`; `_animate_theme_transition()`
  keeps only the motion preference, central-widget target, and trigger.
- PASS by source reasoning: ThemeTransitionSurface imports only Qt and has no
  settings, theme, editor, application-service, persistence, or TaskRunner
  dependency.
- PASS by source reasoning: `_on_settings_saved()` retains the established
  order: apply the persisted theme, retranslate, update editor settings, then
  request the transition.
- PASS by source reasoning: no second motion preference, timer, event bus,
  singleton, or speculative animation framework was introduced.

## Independent review

Archimedes the 2nd / Luna max was assigned a bounded read-only D25 review with
no write access, no Qt launch, and no test creation/run. The initial review
returned CONCERNS about disabled/interrupted cleanup, API shape, and existing
graphics-effect ownership. The parent fixed those concerns by adding the
`animate(target, *, enabled)` seam, unified `_stop_current()` cleanup, and an
existing-effect guard. The bounded follow-up returned PASS for the corrected
source. Runtime behavior remains unverified.

## Simplification assessment

The extraction removes three animation primitive imports, one stored animation
field, and an inline cleanup closure from MainWindow while preserving the
existing visual constants and trigger order. The surface is intentionally
small and target-based; no further safe behavior-preserving simplification is
required for this slice.

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
| D25 source boundary probe | PASS | ThemeTransitionSurface owns animation primitives/lifecycle; MainWindow retains motion gating, target choice, and trigger timing. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D25 portable candidate; root/dist SHA `B4500222FB63398FEF0C2E72E41FA84977A1531E3DC718A9A2ABB64A22102D19`, size `38,390,767` bytes; source `tree-sha256:0687b003f98ab8977aebc3d2adb7456b7236cf6f8287402c75065229d3ae0198`. |
| `scripts/verify_handoff.ps1` | PASS | D25 handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 79 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent follow-up review returned PASS for the corrected source; Qt
  runtime and release-level confidence still require authorized evidence.
- Runtime animation timing, rendering, visual hierarchy, accessibility, and
  cross-machine behavior remain unverified.
- Existing D7/D8 and release gates remain open and are not narrowed by D25.

**Disposition:** `accepted-with-limits`; D25 animation composition is
source-level verified by the parent with an explicit independent
no-conclusion record and all runtime/external release gates open.
