# Handoff: 2026-08-12-d260-font-size-unit-locale-refresh

| Field | Value |
|---|---|
| ID | `2026-08-12-d260-font-size-unit-locale-refresh` |
| Delivery / slice | `D260 / ARCH-238 Font-size unit locale refresh` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

The interface and editor font-size controls now show `pt` in English and
`磅` in Simplified Chinese. Switching language in Settings refreshes both
controls without changing the stored numeric font sizes.

## Scope and boundaries

The change adds one catalog key and applies it in the existing
`SettingsDialog.set_locale` path to both `QSpinBox` controls. Numeric ranges,
selected values, settings snapshots, persistence, preview behavior, and
startup remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Darwin the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Harvey the 7th / Luna max` independent-review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — add the locale-specific font-size
  suffix.
- `src/quillforge/presentation/settings_dialog.py` — refresh both suffixes in
  the existing locale projection.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The suffix is display-only; the settings model continues to store validated
integers. No EXE/Qt launch, native dialog, registry, installer, updater,
worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Font suffix catalog probe | `D260-FONT-SUFFIX-CATALOG=PASS locales=2` |
| Font suffix wiring/order probe | `D260-FONT-SUFFIX-WIRING=PASS spinboxes=2 locale_refresh=1` |
| Python compile | `D260-COMPILEALL=PASS` |
| Ruff | `D260-RUFF=PASS` |
| Ruff format | `D260-FORMAT=PASS` |
| Source startup diagnostic | `D260-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| PS5.1 package build | `PASS`, SHA `8CFCBDF715C8AAE79743A6BEF072B19F3DFB185EE0E8F98545184FC6C3123074` |
| PS7 package build | `PASS`, SHA `8E27D208781476ED1A6BD2A8D291563295A7524CAF9897BBA5F9CE324E5C079B` |
| Package identity | root/dist match, 38,582,507 bytes |
| Source revision | `tree-sha256:2816e34d35d8406d90e11d62a0b396af7695b9f268d9667ed43d2f15bbb387e4` |
| Frozen runtime dependencies | `D260-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D260-ARCHIVE-PYZ=PASS entries=261 modules=4` |
| PyInstaller warnings | `D260-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association
launch, clean-machine startup, signing, installer, updater/rollback,
registry, cross-machine, support, and release-owner checks remain unrun under
the permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probes verify catalog values and refresh wiring but not native
SpinBox rendering, font metrics, DPI scaling, or GUI startup. The current
release dossier remains `no-go` because artifact-bound startup/performance
reports and enterprise gates are open.

## Acceptance and evidence IDs

`S308`, `ARCH-238`, `D260-FONT-SUFFIX-CATALOG=PASS locales=2`,
`D260-SIMPLIFICATION-ASSESSMENT=PASS`,
`D260-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D260-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `8E27D208781476ED1A6BD2A8D291563295A7524CAF9897BBA5F9CE324E5C079B`
- Bytes: `38,582,507`
- Source revision: `tree-sha256:2816e34d35d8406d90e11d62a0b396af7695b9f268d9667ed43d2f15bbb387e4`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Font-size units are statically and package verified;
native rendering and remaining release gates remain open.
