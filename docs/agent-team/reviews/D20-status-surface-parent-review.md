# D20 parent review — status surface coordinator

| Field | Value |
|---|---|
| Slice | D20 / ARCH-11 MainWindow status surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Carson / Luna max; bounded read-only review returned PASS (source-level, accepted-with-limits) |
| Scope | `status_surface.py`, `status_bar.py`, `main_window.py`, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; independent review and source evidence recorded |

## Outcome

`src/quillforge/presentation/status_surface.py` now owns `StatusRail` creation,
parent lifetime, widget exposure, locale projection, and phase projection.
MainWindow retains the phase precedence and all operation/document/error policy;
its semantic synchronizer is named `_sync_status_surface()`.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: the same `StatusRail` remains inserted into the
  native status bar with MainWindow as the parent; only construction is moved
  behind the surface.
- PASS by source reasoning: working, attention, ready, and error calls are
  routed one-to-one through `StatusSurface.set_phase()`.
- PASS by source reasoning: busy/TaskRunner precedence remains ahead of dirty
  document attention, and locale refresh reaches the surface at the same shell
  retranslation point.
- NOT RUNTIME-VERIFIED: QApplication startup, widget lifetime, status-bar
  rendering, QSS refresh, native metrics, accessibility, and DPI remain unrun
  under the permanent no-launch policy.

### Architecture and security

- PASS by source reasoning: MainWindow has no `StatusRail(...)` construction,
  `_status_rail` field, or direct StatusRail import.
- PASS by source reasoning: StatusSurface imports only Qt, domain Locale, and
  the existing presentation rail; it does not import application or
  infrastructure modules.
- PASS by source reasoning: no phase state model, event bus, singleton,
  service locator, or speculative registry was added.

## Independent review

Carson / Luna max was assigned a bounded read-only D20 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The review
confirmed status-bar parent/lifetime, locale preservation, phase precedence,
error/close policy, and no unsafe simplification. Returning the concrete rail
type from `widget` is recorded as an optional narrowing improvement only. Qt
runtime rendering remains unverified.

## Simplification assessment

The extraction removes one widget construction, status-bar insertion detail,
locale call, and direct phase receiver from MainWindow. The surface provides
three thin semantic methods and no state beyond the existing rail. No further
safe behavior-preserving simplification is required for this bounded slice.

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
| `uv run ruff check src\quillforge` | PASS | Lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D20 StatusSurface boundary/phase/locale probe | PASS | MainWindow has no StatusRail construction or direct field. |
| JSON parse for acceptance/register/index | PASS | D20 entries and evidence strings parse after synchronization. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D20 portable candidate; root/dist identity was `F9FB9D0DB12F03B2A040FBEA05D4BFE913BCDCB3A3AFCAF620B09B6E79542233` / `38,381,994` bytes. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO; runtime/report freshness and open release gates remain policy-required |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices remain bounded.
- Runtime status rendering, accessibility, native metrics, fonts, DPI, and
  cross-machine appearance remain unverified.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; independent source review, handoff,
static, and current package evidence are recorded. Runtime and external release
gates remain open as stated.
