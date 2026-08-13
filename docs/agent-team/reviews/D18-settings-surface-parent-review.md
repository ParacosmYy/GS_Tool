# D18 parent review — Settings surface coordinator

| Field | Value |
|---|---|
| Slice | D18 / ARCH-09 MainWindow settings surface coordinator |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Peirce / Luna max; bounded read-only review returned PASS (source-level, accepted-with-limits) |
| Scope | `settings_surface.py`, `settings_dialog.py`, `main_window.py`, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; independent review and source evidence recorded |

## Outcome

`src/quillforge/presentation/settings_surface.py` now owns the concrete
`SettingsDialog` parent relationship and modal edit lifecycle. It returns the
existing immutable `SettingsSnapshot` only after Save, or `None` after cancel
or dismissal. MainWindow delegates that presentation operation and retains all
settings persistence and application policy.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: `SettingsSurface.edit()` constructs one dialog with
  the MainWindow parent, preserves the existing `QDialog.DialogCode.Accepted`
  branch, and returns the dialog's existing snapshot projection.
- PASS by source reasoning: cancel/dismissal returns `None`, so the existing
  save-in-flight flag, worker submission, and settings state are untouched.
- PASS by source reasoning: MainWindow still validates the worker result,
  applies the theme, retranslates the shell, projects editor settings,
  animates the transition, and reports success/failure.
- NOT RUNTIME-VERIFIED: QApplication startup, modal interaction, focus,
  keyboard, native metrics, QSS refresh, and settings save timing remain
  unrun under the permanent no-launch policy.

### Architecture and security

- PASS by source reasoning: MainWindow has no `SettingsDialog(...)`
  construction and depends on the semantic `SettingsSurface.edit()` method.
- PASS by source reasoning: SettingsSurface imports only Qt, the domain
  snapshot, and the existing presentation dialog; it does not import
  application or infrastructure modules.
- PASS by source reasoning: no settings persistence, TaskRunner, theme,
  editor, event bus, singleton, service locator, or speculative state model
  was moved into the surface.

## Independent review

Peirce / Luna max was assigned a bounded read-only D18 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The
follow-up confirmed the `QWidget | None` parent contract, Accepted-only snapshot
return, retained MainWindow persistence/application policy, and minimal safe
delegation. Qt runtime delivery remains unverified.

## Simplification assessment

The extraction removes one concrete dialog construction and one modal
execution/detail block from MainWindow. A single semantic `edit()` delegate
returns the existing domain snapshot without introducing callback state or a
second settings model. No further safe behavior-preserving simplification is
required for this bounded slice.

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
| D18 SettingsSurface boundary/return-semantics probe | PASS by source reasoning |
| JSON parse for acceptance/register/index | PASS | D18 entries and evidence strings parse after synchronization. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D18 portable candidate; root/dist identity is `E49590F24B192D39C57237B6FF9659CEE4E96913DF54057721273449307AC4EF` / `38,382,078` bytes. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO; runtime/report freshness and open release gates remain policy-required |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices remain bounded.
- Runtime settings interaction, accessibility, native metrics, fonts, DPI, and
  cross-machine appearance remain unverified.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; independent source review, handoff,
static, and current package evidence are recorded. Runtime and external release
gates remain open as stated.
