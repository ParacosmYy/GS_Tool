# Handoff: 2026-08-10-d9-ui-08-localization-appearance

| Field | Value |
|---|---|
| ID | `2026-08-10-d9-ui-08-localization-appearance` |
| Delivery / slice | `D9 / UI-08 localization, appearance preferences, and workspace file activation` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; parent is the sole writer |
| Created | `2026-08-10T01:39:06+08:00` |

## User outcome

QuillForge now has a persisted settings surface for English/Simplified
Chinese, interface and editor font families/sizes, line wrapping, line
numbers, light/dark theme surfaces, four accent colors, and optional smooth
transitions. The shell reprojects menus, dialogs, toolbar, workspace, find,
status, and plugin surfaces after saving. A workspace file can be activated
through the existing asynchronous document-open path; folders remain directory
navigation intents.

## Scope and boundaries

### In scope

- Settings schema v2, schema-1 migration, bounded defaults, and atomic JSON
  persistence.
- Presentation-only i18n catalog, theme tokens, settings dialog controls, and
  one motion-gated opacity transition.
- File/directory signal mapping in the workspace panel and menu/dialog/status
  localization.
- Architecture review, simplification assessment, static validation, handoff
  traceability, and portable package refresh.

### Out of scope

- Launching QuillForge, QApplication/window creation, screenshots, visual
  acceptance, screen-reader/DPI/cross-machine validation, or native dialog
  measurement.
- Installing fonts, signing, installer/updater/legal/support decisions, or
  target hardware operations.
- Unit tests, mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | `parent` + Plato read-only review | Architecture, integration, final diff review, disposition |
| Project Manager | `repository policy` | Delivery/register dependency tracking |
| Product | `user request` | Language, appearance, animation, and file activation outcome |
| Developer 1 | `parent` | Domain/application/infrastructure settings and composition-root changes |
| Developer 2 | `parent` | Presentation, workspace activation, docs, and packaging integration |
| QA | `parent read-only validation` | Compile, lint, formatting, handoff/check/package evidence; no runtime launch |

## Changed files and modules

- `src/quillforge/domain/models.py` — locale/theme/accent and appearance value
  objects; expanded editor preferences.
- `src/quillforge/application/settings.py` — schema v2 normalization and
  schema-1 migration.
- `src/quillforge/infrastructure/settings_store.py` — JSON v2 mapping.
- `src/quillforge/app.py` and `src/quillforge/presentation/main_window.py` —
  single initial settings snapshot injection and UI application.
- `src/quillforge/presentation/i18n.py`, `theme.py`, `settings_dialog.py`,
  `find_bar.py`, `workspace_search_dialog.py`, `workspace_panel.py`,
  `editor_widget.py`, `status_bar.py`, `command_palette.py`,
  `plugin_catalog_dialog.py`, and `plugin_status_dialog.py` — bounded
  presentation projection.
- `docs/adr/0005-versioned-settings-and-command-palette.md` and
  `docs/adr/0035-localization-and-appearance-boundary.md` — contract and
  architecture decisions.
- `docs/agent-team/reviews/D9-UI-08-localization-appearance-parent-review.md` —
  independent review, applicability, simplification, and limits.

## Decisions and constraints

- The workspace panel does not perform file I/O. A file activation signal is
  revalidated by `WorkspaceService.contains()` and opened by `DocumentService`
  through `TaskRunner`.
- The user-visible workspace contract is single-click file activation and
  double-click folder navigation. A second activation is rejected by the
  existing busy/path guards; Qt/platform click timing remains a runtime limit.
- Settings are applied only after persistence reports a valid normalized
  snapshot. The initial composition-root load is reused by the window.
- Embedded C/C++ assurance was assessed as not applicable; no vendor rule or
  certification claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src/quillforge` | `PASS` | Static bytecode compilation only. |
| `uv run ruff check` | `PASS` | All changed Python modules. |
| `uv run ruff format --check` | `PASS` | All changed Python modules. |
| `scripts/verify_handoff.ps1` | `PASS` | Index, status, required sections, and repository-backed paths validated. |
| `scripts/check.ps1` | `PASS` | NOTICE, formatting, static, metadata, acceptance, and project checks passed. |
| `scripts/package.ps1` | `PASS` | PowerShell 7 rebuilt the root/dist portable candidate and release manifest. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Current identity matches; packaged/runtime freshness and external gates remain open. |

## Unrun checks and reason

- EXE/QApplication/window startup, interactive file opening, settings switching,
  native dialogs, visual screenshots, font availability, DPI, screen-reader,
  click timing, clean-machine, cross-machine, pressure, and hard-power checks —
  prohibited or unavailable under the active local policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under repository policy.
- Signing, installer, updater, legal, support, and external release approval —
  outside the current checkout and authority.

## Known risks and limits

- Workspace root containment is rechecked before opening but remains a
  pre-open TOCTOU check; it is not claimed as a strong security boundary.
- Installed font availability, native QFileDialog metrics, DPI, and Qt click
  timing can change the runtime presentation.
- Package/runtime reports may remain bound to earlier artifacts until the
  release verifier refreshes its current no-go dossier.

## Acceptance and evidence IDs

- Acceptance: `D9-AC06`, `S33`
- Related: `D9-AC01`, `D9-AC02`, `D9-AC04`
- Evidence: source paths above, ADR-0035, the parent review, static checks,
  handoff verifier, package manifest, and current release dossier.

## Next owner and next action

- Owner: `authorized runtime/visual reviewer`
- Action: when launch is explicitly authorized, verify settings persistence,
  language retranslation, installed-font fallbacks, native file dialogs, and
  single-click/double-click workspace behavior; then refresh release evidence.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `8BEC911FF4DE159EE81D3D9D176197E84E4A46AE42E066E37C4F8328632ED10D` / `38,359,398` bytes
- Source snapshot: `tree-sha256:01434ec04dea71a7aee1455356affb420696945bd5629e52bc48be5549941560`
- Packaging note: PowerShell 7 portable candidate rebuilt; runtime evidence is
  intentionally not regenerated.

## Disposition

`accepted-with-limits`: the requested source features and architecture
boundaries are implemented and statically verified; runtime visual, native
environment, and external release gates remain open.
