# D17 parent review — Find surface coordinator

| Field | Value |
|---|---|
| Slice | D17 / ARCH-08 MainWindow Find surface coordinator |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Gödel / Luna max; bounded read-only review returned PASS (source-level, accepted-with-limits) |
| Scope | `find_surface.py`, `find_bar.py`, `main_window.py`, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; evidence synchronized |

## Outcome

`src/quillforge/presentation/find_surface.py` now owns `FindBar` creation and
parent lifetime, six semantic signal routes, editor-shell placement, hide/show
mode, locale, query/replacement/case access, status, operation-active, and
session-reset projection. MainWindow no longer creates or directly wires the
FindBar widget.

MainWindow remains the behavior owner: it resolves the active tab, calls the
editor literal find/replace APIs, owns `_find_match`, creates and advances the
Replace All session, performs content-version and cancellation/rollback
checks, locks tabs, and projects operation/error policy.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: `FindSurfaceCallbacks` preserves the six existing
  signal shapes: `(bool)` for find and no-argument callbacks for replace,
  replace-all, cancel, close, and criteria changes.
- PASS by source reasoning: FindBar is created with the editor-shell parent,
  hidden before the central widget is installed, and the same widget is added
  to the shell layout. `show_find()` remains the existing focus/select-all and
  mode projection path.
- PASS: query, replacement, case, status, locale, operation-active, and reset
  methods are one-to-one delegations; no query or editor state is duplicated.
- PASS: MainWindow's find invalidation, Replace All lifecycle, cancellation,
  rollback, tab locking, and error/status branches remain in place.
- NOT RUNTIME-VERIFIED: QApplication startup, signal delivery, focus/keyboard
  behavior, QSS refresh, native layout metrics, and interactive Replace All
  behavior remain unrun under the permanent no-launch policy.

### Architecture and security

- PASS: MainWindow has no `FindBar(...)` construction or six external signal
  bindings; it depends on `FindSurface` and its semantic API only.
- PASS: FindSurface imports only Qt, domain `Locale`, and presentation
  `FindBar`; it does not import application/infrastructure or own editor
  policy.
- PASS: no event bus, singleton, widget registry, service locator, plugin
  execution path, or speculative state container was introduced.

## Independent review

Gödel / Luna max was assigned a bounded read-only D17 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The review
confirmed all six callback routes, shell lifetime/placement, and MainWindow's
retained editor and Replace All policy; it found no unsafe simplification
opportunity. Qt runtime delivery and interactive behavior remain unverified.

## Simplification assessment

The extraction removes FindBar construction, six signal-connect operations,
hide/placement details, and direct widget projection from MainWindow. One
explicit frozen callback record and thin semantic delegates replace those
details; all editor and Replace All policy stays in the existing coordinator.
No further safe behavior-preserving simplification is required for this slice.

## Public-source applicability and embedded gate

This is a Python/PyQt6 desktop change, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded simplifier are **N/A** for MCU/vendor
constraints: no MCU, SDK, RTOS, ISR, DMA, driver, boot, Flash/NVM, power, or
hardware target was changed. Public architecture references are engineering
references only: [CloudWeGo About](https://www.cloudwego.io/about/),
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
| D17 FindSurface boundary and six-route probe | PASS | MainWindow has no FindBar construction or external six-route wiring. |
| JSON parse for acceptance/register/index | PASS | D17 entries and evidence strings parse after synchronization. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D17 portable candidate; root/dist identity is `CFD8510A81A3E3AC0BB851125EE5DD573344C8F29ECF843984BE654A93C5E4CD` / `38,380,906` bytes. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO; runtime/report freshness failures and open release gates remain policy-required |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices remain bounded.
- Runtime keyboard/focus behavior, native metrics, fonts, DPI, accessibility,
  and Replace All interaction remain unverified.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; D17 evidence is synchronized and the
current package identity is recorded. Runtime and external release gates remain
open as recorded.
