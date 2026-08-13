# D19 parent review — recovery prompt surface

| Field | Value |
|---|---|
| Slice | D19 / ARCH-10 MainWindow recovery prompt surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Linnaeus / Luna max; bounded read-only review returned PASS (source-level, accepted-with-limits) |
| Scope | `recovery_prompt_surface.py`, `main_window.py`, recovery models/i18n, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; independent review and source evidence recorded |

## Outcome

`src/quillforge/presentation/recovery_prompt_surface.py` now owns the recovery
message-box parentage, locale-aware path/time/source text, button roles, and
typed three-state decision. MainWindow delegates prompt presentation and keeps
the recovery service, snapshot, session, document-event, cleanup, and
notification branches.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: restore and discard button identity maps to the same
  `_restore_snapshot()` and `_discard_snapshot()` calls; dismissal and the
  explicit Later button return `"later"` and preserve deferred-path behavior.
- PASS by source reasoning: untitled paths, timestamp formatting, source-status
  fallback, translated details, and locale refresh preserve the old prompt
  inputs and text keys.
- PASS by source reasoning: MainWindow still checks recovery availability,
  restores snapshots through RecoveryService, schedules deletion, appends
  deferred session paths, and notifies the user.
- NOT RUNTIME-VERIFIED: QApplication startup, modal button delivery, locale
  rendering, focus, native metrics, and recovery timing remain unrun under the
  permanent no-launch policy.

### Architecture and security

- PASS by source reasoning: MainWindow no longer constructs the recovery
  prompt or formats its timestamp/source explanation; it consumes a typed
  semantic decision.
- PASS by source reasoning: RecoveryPromptSurface imports only Qt, domain
  Locale, Path, standard-library formatting, and presentation i18n; it does
  not import RecoveryService, application policy, or infrastructure.
- PASS by source reasoning: no recovery service call, snapshot mutation,
  session state, event bus, singleton, service locator, or second state model
  moved into the surface.

## Independent review

Linnaeus / Luna max was assigned a bounded read-only D19 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The review
confirmed three-state mapping, dismissal-as-later, retained MainWindow recovery
branches, text/locale invariants, and no unsafe simplification. A non-blocking
concern is recorded: `source_status` remains a defensive `str` boundary rather
than a closed type; existing diagnostic-runner layering is explicitly outside
D19 scope. Qt runtime delivery remains unverified.

## Simplification assessment

The extraction removes prompt construction, translation mapping, timestamp
formatting, and button identity details from MainWindow. One typed decision
surface replaces those details without duplicating recovery state or policy.
No further safe behavior-preserving simplification is required for this
bounded slice.

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
| `uv run python -m compileall -q src\quillforge` | PASS |
| `uv run ruff check src\quillforge` | PASS |
| `uv run ruff format --check src\quillforge` | PASS |
| D19 recovery-prompt decision/text/ownership probe | PASS | Final static boundary/decision probe passed. |
| JSON parse for acceptance/register/index | PASS | D19 entries and evidence strings parse after synchronization. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D19 portable candidate; root/dist identity is `826BD6F2D3CDCB25CCF488908A37ABABE1720C8C3D2ABA8E44204B39A963D800` / `38,382,128` bytes. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO; runtime/report freshness and open release gates remain policy-required |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices remain bounded.
- Runtime recovery prompt interaction, accessibility, native metrics, fonts,
  DPI, and recovery timing remain unverified.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; independent source review, handoff,
static, and current package evidence are recorded. Runtime and external release
gates remain open as stated.
