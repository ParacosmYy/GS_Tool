# Handoff: 2026-08-12-d257-builtin-plugin-status-name-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d257-builtin-plugin-status-name-localization` |
| Delivery / slice | `D257 / ARCH-235 Built-in plugin status-name localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

The built-in Document Statistics name now follows the selected locale in the
plugin-status presentation. English remains English, and external plugin
names remain exactly as supplied by their manifests.

## Scope and boundaries

The change adds a stable built-in-ID resolver in `presentation.i18n` and routes
the plugin-status display through it. The existing locale refresh path is
reused. The public plugin API, manifest schema, plugin lifecycle, status
contract, and startup behavior remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Hume the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Russell the 7th / Luna max` independent-review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — add the built-in status-name
  resolver.
- `src/quillforge/presentation/plugin_status_dialog.py` — use the resolver in
  the existing status projection and locale-refresh path.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The stable built-in plugin ID is the only translation key. Unknown/external
plugin names retain their manifest fallback. No EXE/Qt launch, native dialog,
registry, installer, updater, worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Built-in/external name boundary probe | `D257-PLUGIN-NAME-BOUNDARY=PASS cases=3` |
| Python compile | `D257-COMPILEALL=PASS` |
| Ruff | `D257-RUFF=PASS` |
| Ruff format | `D257-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `D0B25BFB7B849EEF25EB9FFBC0F3145B619B26FD6A383645A58E983695B54858` |
| PS7 package build | `PASS`, SHA `B466FAA1BC65862783159DD4F86493852B7B2F1D25B7AF2ACBD637D67FD076A0` |
| Package identity | root/dist match, 38,582,333 bytes |
| Source revision | `tree-sha256:cc8254f41125409b4f57d01b2f5ae29f4f49d8e603f9fa1a8e6414ab2e46a990` |
| Source startup diagnostic | `D257-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| Frozen runtime dependencies | `D257-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D257-ARCHIVE-PYZ=PASS entries=261 modules=4` |
| PyInstaller warnings | `D257-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association
launch, clean-machine startup, signing, installer, updater/rollback,
registry, cross-machine, support, and release-owner checks remain unrun under
the permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probe verifies the stable-ID projection and fallback boundary but
not native rendering, locale-switch interaction, plugin lifecycle execution,
or GUI startup. The current release dossier remains `no-go` because
artifact-bound startup/performance reports and enterprise gates are open.

## Acceptance and evidence IDs

`S305`, `ARCH-235`, `D257-PLUGIN-NAME-BOUNDARY=PASS cases=3`,
`D257-SIMPLIFICATION-ASSESSMENT=PASS`,
`D257-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D257-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `B466FAA1BC65862783159DD4F86493852B7B2F1D25B7AF2ACBD637D67FD076A0`
- Bytes: `38,582,333`
- Source revision: `tree-sha256:cc8254f41125409b4f57d01b2f5ae29f4f49d8e603f9fa1a8e6414ab2e46a990`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The built-in plugin status name is statically and
package verified; native startup and remaining release gates remain open.
