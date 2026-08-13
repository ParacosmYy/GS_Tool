# Handoff: 2026-08-11-d186-workspace-file-entry

| Field | Value |
|---|---|
| ID | `2026-08-11-d186-workspace-file-entry` |
| Delivery / slice | `D186 / UI-97 / ARCH-173 Workspace file-entry closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

The workspace dock now exposes a clearly labeled Open file action in addition
to Open project folder. Selecting it reuses the existing file picker and
asynchronous document-open flow, so users no longer need to discover the
toolbar/menu command to open a file.

## Scope and boundaries

### In scope

- WorkspacePanel file-action button, localized label, icon, signal, and loading
  enablement.
- WorkspaceSurface semantic callback forwarding and MainWindow binding.
- Existing FileDialogSurface/DocumentPickerAdmissionCoordinator reuse and
  scoped QSS.

### Out of scope

- New document service, picker, state store, file format policy, folder/tree
  activation semantics, GUI startup, native dialog behavior, or screenshots.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependencies, risks, and status |
| Product | `user outcome` | Discoverable file opening beside folder opening |
| Developer 1 | `parent` | WorkspacePanel/Surface semantic edge |
| Developer 2 | `parent` | MainWindow/i18n/QSS integration and packaging |
| QA | `parent` | Read-only source/call-chain/package verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — explicit file action.
- `src/quillforge/presentation/workspace_surface.py` — callback forwarding.
- `src/quillforge/presentation/main_window.py` — reuse `_open_document`.
- `src/quillforge/presentation/i18n.py` — English/Simplified Chinese labels.
- `src/quillforge/presentation/theme.py` — scoped file-action affordance.

## Decisions and constraints

- Shared checkout writer: `parent`, the bounded presentation/integration files.
- Runtime launch policy: not allowed; no GUI, EXE, native dialog, screenshot,
  or test-only asset was run/created.
- Architecture window: Arendt the 5th / Luna max — `NO_CONCLUSION` after timeout.
- Independent review: Cicero the 6th / Luna max — `NO_CONCLUSION` after timeout
  and closure; no child PASS is claimed.
- Python/PyQt6 presentation only; embedded C/C++, MCU, RTOS, and manufacturer
  requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Workspace file-action/source probes | `PASS` | `D186-WORKSPACE-FILE-ACTION-PROBE=PASS`, surface/call-chain/i18n/QSS probes pass. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src` and `ruff format --check src` | `PASS` | `D186-COMPILE-RUFF-FORMAT=PASS`. |
| `scripts/package.ps1` | `PASS` | Portable candidate rebuilt; EXE not launched. |

## Unrun checks and reason

- Native file picker selection and actual document opening — no GUI/EXE policy.
- Screenshot, DPI, accessibility, clean-machine, cross-machine, and release
  gates — external/runtime evidence is not authorized or available.

## Known risks and limits

- Native toolbar/button width may require visual adjustment at narrow DPI or
  with large user-selected fonts.
- Existing release remains `no-go` with ten open gates.

## Acceptance and evidence IDs

- Acceptance: `S239`, `D186-AC01`
- Evidence: `D186-WORKSPACE-FILE-ACTION-PROBE=PASS`,
  `D186-SURFACE-CALLBACK-PROBE=PASS`,
  `D186-REUSE-OPEN-BOUNDARY-PROBE=PASS`, `D186-I18N-PROBE=PASS en+zh-CN`,
  `D186-FILE-DIALOG-CHAIN-PROBE=PASS`,
  `D186-FILE-ACTION-QSS-PROBE=PASS`, `D186-COMPILE-RUFF-FORMAT=PASS`,
  `D186-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize a native file-picker/opening smoke pass and verify the
  discoverability fix under supported DPI and font-size settings.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `0EA9BFCC1C4B6FC84F98FCFE0765A4078C60F1B957B551791DAD349622523562` / `38,556,329` bytes
- Source revision: `tree-sha256:108a06394c548ac4178826f0271def6b989195b711461b6900de59e106f91b79`
- Packaging note: portable PyInstaller one-file candidate rebuilt; no installer
  or updater artifact is claimed.

## Disposition

`accepted-with-limits`: the discoverable file-entry boundary and static source
evidence are recorded; native dialog/opening and release gates remain open.
