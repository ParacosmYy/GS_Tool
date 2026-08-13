# Handoff: 2026-08-12-d278-command-surface-startup-contract

| Field | Value |
|---|---|
| ID | `2026-08-12-d278-command-surface-startup-contract` |
| Delivery / slice | `D278 / ARCH-248 Command-surface startup contract` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The known startup crash caused by the missing `CommandSurface._locale` seam is
present in the local historical log as the root cause, and the current source
candidate retains the D225 fix. D278 adds a static guard for the provider,
accessor, and startup ordering so the shell cannot regress silently at the
next packaging iteration.

## Scope and boundaries

- Added `_audit_startup_surface_contract()` to the existing Qt-free
  presentation audit.
- Checked `CommandSurface` locale-provider storage/accessor delegation and
  the four MainWindow startup ordering anchors.
- No runtime command, menu, toolbar, locale, settings, or Qt behavior changed.
- No EXE/Qt launch, native dialog, updater, installer, registry operation,
  unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Nietzsche consultation | Boundary, ownership, and integration review |
| Developer | parent | Qt-free startup contract audit |
| QA | parent | Static, source-diagnostic, archive, package, and handoff verification |
| Independent reviewer | Kepler consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `scripts/audit_presentation_contracts.py`
- D278 ADR, review, plan, roadmap, register, acceptance, and handoff records

## Decisions and constraints

- Keep locale state composition-owned by `MainWindow` and keep menu/toolbar
  projection in `CommandSurface`.
- Guard the existing provider/accessor startup seam with a Qt-free source
  contract; do not duplicate locale policy or instantiate Qt in the audit.
- Shared checkout writer: parent agent, limited to the listed audit and
  delivery-record files.
- Runtime launch policy: EXE/Qt startup was not allowed in this turn; only
  non-destructive source, archive, and package evidence was authorized.

## Public-source applicability

Python 3.12 standard-library `str.find`, `zip(strict=True)`, and source/AST
inspection behavior are applicable references; Qt 6 `QMainWindow` remains the
public UI composition reference. No vendor/manufacturer requirement applies.
No private ByteDance standard, certification, MISRA, ISO 26262, ASPICE, or
embedded claim is made. Embedded workflow and embedded simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Startup surface contract | `D278-STARTUP-SURFACE-CONTRACT=PASS provider=1 accessor=1 order=4` |
| CommandSurface locale probe | `D278-COMMAND-SURFACE-LOCALE-PROBE=PASS locale=zh-CN` |
| Presentation audit | `PASS` |
| Compileall | `PASS` |
| Ruff and formatting | `PASS` |
| Source startup diagnostic | `D278-SOURCE-DIAGNOSTIC=PASS exit=0 status=passed failed=0` |
| PE header | `D278-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI` |
| Frozen archive | `D278-PE-ARCHIVE=PASS outer_entries=166 pyz_entries=261 required=8 embedded_command_surface=True` |
| Root/dist identity | `D278-MANIFEST-COPY=PASS bytes=38583521 sha=7ACD072EA6E0778C468C04958CC4DAC18E7FEB095FFCC14323C69726ACDB52BC` |
| Source revision | `tree-sha256:e62e62c51c7d698128bbe1418934af482ac419f1bbf39b40d488b673885e8d5e` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded wait and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, native menu rendering, native file activation, native
font rendering/fallback, accessibility, clean-machine behavior, signing,
installer/update, registry, cross-machine repeatability, and release-owner
acceptance remain unrun under the active no-launch and non-destructive policy.

## Known risks and limits

The current source/archive evidence proves that the startup fix and its static
guard are present, but it does not prove Windows can construct the native Qt
window on this machine. The historical user-local log is from the pre-D225
candidate; a future authorized native run should attach a fresh startup log
bound to the current SHA-256.

## Acceptance and evidence IDs

- Acceptance: `S318`.
- Architecture slice: `ARCH-248`.
- Evidence: `D278-STARTUP-SURFACE-CONTRACT=PASS`, `D278-CHECK=PASS`,
  `D278-MANIFEST-IDENTITY=PASS`,
  `D278-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D278-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D278-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: on an authorized desktop run, launch the current candidate once,
  confirm the shell appears, and attach a fresh artifact-bound startup report
  if it does not.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size:
  `7ACD072EA6E0778C468C04958CC4DAC18E7FEB095FFCC14323C69726ACDB52BC` /
  `38,583,521` bytes.
- Source revision:
  `tree-sha256:e62e62c51c7d698128bbe1418934af482ac419f1bbf39b40d488b673885e8d5e`.

## Disposition

`accepted-with-limits`: the known startup root cause is fixed in the current
source and protected by a static contract; native launch and enterprise
release gates remain open.
