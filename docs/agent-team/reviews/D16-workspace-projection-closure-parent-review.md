# D16 parent review — workspace projection-boundary closure

| Field | Value |
|---|---|
| Slice | D16 / ARCH-07 MainWindow workspace projection-boundary closure |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Curie / Luna max; bounded read-only review returned PASS (source-level, accepted-with-limits) |
| Scope | `workspace_surface.py`, `workspace_panel.py`, `main_window.py`, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; evidence synchronized |

## Outcome

`WorkspaceSurface` now owns the semantic projection methods for the existing
workspace panel: `current_path`, `set_loading`, `set_directory`, and
`show_error`. MainWindow no longer imports `WorkspacePanel`, stores a panel
compatibility property, or calls widget methods directly.

The workspace service boundary remains unchanged. MainWindow still owns
workspace open/list calls, `TaskRunner` submission, global busy/operation
coordination, generation and cancellation, session-restore barriers, root
containment, document activation, notifications, and recoverable error policy.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: the four new surface methods delegate one-to-one to
  the existing panel methods and preserve `Path`, `WorkspaceDirectory`,
  `bool`, and `str` shapes.
- PASS by source reasoning: workspace start/cancel/navigation paths retain the
  same busy checks and loading/error feedback; only the receiver changed from
  the panel compatibility property to the surface.
- PASS by source reasoning: opened-directory and listed-directory completions
  still clear loading before generation checks, reject stale results, preserve
  session-restore completion ordering, and project current data only after
  type/root checks.
- PASS: `_go_workspace_parent()` reads the same panel-owned current path via
  the surface; containment, document activation, and notifications remain in
  MainWindow.
- NOT RUNTIME-VERIFIED: QApplication startup, Qt object lifetime, signal
  delivery, panel rendering, loading/cancel timing, and interactive
  folder/file activation remain unrun under the permanent no-launch policy.

### Architecture and security

- PASS: MainWindow has no `WorkspacePanel` import, `_workspace_panel`
  property, or direct panel method call after the extraction.
- PASS: the surface imports only Qt, domain models, i18n, and the existing
  presentation panel; it does not import application/infrastructure or own
  async policy.
- PASS: no second workspace state model, service locator, event bus, singleton,
  widget registry, or plugin execution path was added.
- PASS: the existing user-selected file containment check remains in
  `MainWindow` before document opening.

## Independent review

Curie / Luna max was assigned a bounded read-only D16 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The review
confirmed the four semantic projection delegates, retained MainWindow policy,
and found no unsafe simplification opportunity. Qt runtime delivery and
interactive behavior remain unverified.

## Simplification assessment

The slice removes one MainWindow widget import, one compatibility property, and
all direct panel projection calls, replacing them with four explicit semantic
surface methods. It does not add state or move policy; the methods are thin
one-to-one delegations. No further safe behavior-preserving simplification is
required for this bounded boundary closure.

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
| `uv run python -m compileall -q src\quillforge` | PASS before final docs/package refresh |
| `uv run ruff check src\quillforge` | PASS before final docs/package refresh |
| `uv run ruff format --check src\quillforge` | PASS before final docs/package refresh |
| D16 dependency/lifecycle probe | PASS by source reasoning; final exact probe recorded with package evidence |
| JSON parse for acceptance/register/index | PASS after D16 entries were added |
| `scripts/verify_handoff.ps1` | PASS | D16 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D16 portable candidate; root/dist identity is `22AC2146C2499B0994BF3E93B6705D586C85B1905995214893350DF1F88220E3` / `38,378,407` bytes. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Dossier refreshed; the known three report failures and ten open release gates remain. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- MainWindow still owns workspace async/session coordination and remains a large
  coordinator; future slices must keep contracts explicit.
- Runtime panel lifetime, native metrics, accessibility, fonts, DPI, and
  interactive workspace behavior remain unverified.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; D16 evidence is synchronized and the
historical package identity is preserved. Runtime and external release gates
remain open as recorded.
