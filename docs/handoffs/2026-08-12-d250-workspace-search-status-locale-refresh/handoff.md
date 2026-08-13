# Handoff: 2026-08-12-d250-workspace-search-status-locale-refresh

| Field | Value |
|---|---|
| ID | `2026-08-12-d250-workspace-search-status-locale-refresh` |
| Delivery / slice | `D250 / ARCH-228 Workspace search status locale refresh` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

The workspace-search dialog now reprojects its current status in the newly
selected language. Search failures, loading/cancellation prompts, and dynamic
completion/limit summaries no longer remain in the previous locale.

## Scope and boundaries

The change is limited to `WorkspaceSearchDialog` presentation state. It keeps
catalog keys, raw failure strings, or immutable `WorkspaceSearchResult`
sources and regenerates text in the existing i18n boundary. Search services,
operation lifecycle, result rows, diagnostics, query, cancellation, and
application policy are unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Godel the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Lorentz the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/workspace_search_dialog.py` — retain structured
  status sources and rebuild them after locale changes.
- `docs/adr/0294-workspace-search-status-locale-refresh.md` and review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/RELEASE_HANDOFF.md`, `tasks/plan.md`, and `tasks/todo.md` — evidence
  binding.

## Decisions and constraints

The source state is transient UI data; it is not serialized or passed into
application services. No EXE/Qt launch, native dialog, registry, installer,
updater, worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Status source retention | `D250-STATUS-SOURCE-RETENTION=PASS` |
| Error reprojection | `D250-LOCALE-ERROR-REPROJECTION=PASS` |
| Summary reprojection | `D250-LOCALE-SUMMARY-REPROJECTION=PASS` |
| Catalog reprojection | `D250-CATALOG-STATUS-REPROJECTION=PASS` |
| Python compile | `D250-COMPILEALL=PASS` |
| Ruff | `D250-RUFF=PASS` |
| Ruff format | `D250-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `2F1938E3DC1860F2090F76C0D1228D953800D965E77697D39D66F7AD6A9C3798` |
| PS7 package build | `PASS`, SHA `65345D8C6484C57A272501E68BCDBC82873A400D02539B582FD2733686A7BA34` |
| Package identity | root/dist match, 38,578,545 bytes |
| Source revision | `tree-sha256:fd5630adf006bccea7c69226e598d7384a3f8bfa8bf1453349ec50eb15196075` |
| Frozen archive | `D250-ARCHIVE-OUTER=PASS`, outer=166, inner=261; entry/search/PyQt6/qwindows present |
| PyInstaller warnings | `D250-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
real filesystem permission timing, clean-machine startup, signing, installer,
updater/rollback, registry, cross-machine, support, and release-owner checks
remain unrun under the permanent no-launch and non-destructive policy. No
unit-test assets were created or run.

## Known risks and limits

The status source improves language switching but does not alter search
execution, result ordering, filesystem limits, or cancellation timing. Static
and archive evidence cannot prove native rendering or Windows startup.

## Acceptance and evidence IDs

`S298`, `ARCH-228`, `D250-STATUS-SOURCE-RETENTION=PASS`,
`D250-LOCALE-ERROR-REPROJECTION=PASS`,
`D250-LOCALE-SUMMARY-REPROJECTION=PASS`,
`D250-SIMPLIFICATION-ASSESSMENT=PASS`,
`D250-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D250-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup, native
dialog, and clean-machine evidence. This handoff does not authorize launching
the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `65345D8C6484C57A272501E68BCDBC82873A400D02539B582FD2733686A7BA34`
- Bytes: `38,578,545`
- Source revision: `tree-sha256:fd5630adf006bccea7c69226e598d7384a3f8bfa8bf1453349ec50eb15196075`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Workspace search status locale refresh is packaged and
recorded; native startup, native dialogs, and remaining release gates remain
open.
