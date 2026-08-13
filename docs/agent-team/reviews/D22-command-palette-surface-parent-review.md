# D22 parent review — command palette surface

| Field | Value |
|---|---|
| Slice | D22 / ARCH-13 MainWindow command-palette surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Arendt / Luna max; bounded read-only review returned CONCERNS; stale-command locale concern resolved by parent |
| Scope | `command_palette_surface.py`, `command_palette.py`, MainWindow command-palette call/retranslate sites, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; independent review and source evidence recorded |

## Outcome

`src/quillforge/presentation/command_palette_surface.py` now owns command
palette modal composition, QWidget parentage, locale, and Accepted/cancelled
projection. MainWindow receives only a stable command ID and retains live
registry lookup, command execution, stale-ID notification, menu refresh, and
command ownership. The stale-command notification now uses the existing
localized message adapter, so the Chinese locale does not leak English on this
error path.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: `choose()` calls `dialog.exec()` and returns an ID
  only for `QDialog.DialogCode.Accepted`; cancel, close, Escape, and empty
  selection return `None`.
- PASS by source reasoning: the existing dialog stores the stable command ID in
  `Qt.ItemDataRole.UserRole`; MainWindow resolves the ID again through the
  current registry before executing.
- PASS by source reasoning: locale reaches the surface through
  `_retranslate_ui()` and is passed into the dialog at construction.
- PASS after review fix: stale-command notification uses
  `localize_message()` and retains the command ID, preserving localized shell
  feedback in English and Simplified Chinese.
- NOT RUNTIME-VERIFIED: QApplication startup, modal acceptance/cancel,
  keyboard/focus, visual hierarchy, accessibility, DPI, and screen-reader
  behavior remain unrun under the permanent no-launch policy.

### Architecture and policy

- PASS by source reasoning: MainWindow has no `CommandPaletteDialog` import or
  direct modal execution after D22.
- PASS by source reasoning: command execution and stale-command recovery remain
  in MainWindow; the surface has no executor, registry, event bus, or refresh
  policy.
- PASS by source reasoning: `CommandPaletteDialog` and the new surface accept a
  `QWidget` parent, matching the actual MainWindow object tree.
- Accepted limitation: both existing palette classes consume the application
  `Command` contract for title/ID projection. A metadata-only descriptor is a
  possible future boundary, but introducing it here would enlarge D22 without
  a user-visible requirement.

## Independent review

Arendt / Luna max performed a bounded read-only source review and returned
**CONCERNS**, with no blocking modal or ID-handoff failure. The review
confirmed Accepted/cancel semantics, stable-ID resolution, QWidget parent
typing, locale construction, and MainWindow execution/refresh ownership. It
identified one user-visible issue: the stale-command notification was
hardcoded English. The parent fixed that path with `localize_message()`, then
reran compile/Ruff/format and the source boundary probe. The metadata coupling
observation is recorded as a nonblocking future boundary, not hidden as a
completed refactor.

## Simplification assessment

The extraction removes dialog construction, modal result interpretation, and
parent/locale plumbing from MainWindow while preserving a single stable-ID
handoff. The surface has one locale field and one semantic method; no factory,
registry, callback graph, or duplicate command state was introduced. No further
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
| `uv run python -m compileall -q src\quillforge` | PASS | Non-destructive compile gate after the localized-message fix. |
| `uv run ruff check src\quillforge` | PASS | Import/lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D22 source boundary/locale probe | PASS | MainWindow has no palette dialog import/direct exec; surface route, locale route, execution/refresh ownership, and localized stale-message route are present. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D22 candidate; root/dist identity was `7F1D2E18C1AE24B8E113228260A0ADEA46E8F8B6223C5675FCDD1C380A2B6CB0` / `38,386,100` bytes; source `tree-sha256:712c8f17fb4cf308837d60769c3bb26a3f336ff89a650b0fb7c59f506f4c04f0`. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 76 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Native palette interaction, keyboard/focus, visual hierarchy, accessibility,
  DPI, and cross-machine rendering remain unverified.
- Application `Command` metadata coupling remains a documented future boundary;
  D22 does not claim a DTO/protocol migration.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; command-palette modal composition and
localized stale-command feedback are source-level verified, with runtime and
external release gates explicitly open.
