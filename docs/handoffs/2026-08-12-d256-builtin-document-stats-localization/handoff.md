# Handoff: 2026-08-12-d256-builtin-document-stats-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d256-builtin-document-stats-localization` |
| Delivery / slice | `D256 / ARCH-234 Built-in document-statistics localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

The built-in Document Statistics command and its notifications now use
Simplified Chinese in the Chinese shell, including dynamic line/character
counts. English output and unknown external plugin fallbacks remain unchanged.

## Scope and boundaries

Only `presentation.i18n` changed. The public plugin API, built-in plugin, core
command registration, notification routing, counters, and startup behavior
remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Einstein the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Lovelace the 7th / Luna max` independent-review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — add the built-in command title and
  exact/dynamic notification mappings.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

Core command IDs use the existing catalog. Built-in notification text is
localized at the existing presentation boundary; unknown plugin text remains
unchanged. No EXE/Qt launch, native dialog, registry, installer, updater,
worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Built-in localization probe | `D256-BUILTIN-PLUGIN-LOCALIZATION=PASS cases=3` |
| Python compile | `D256-COMPILEALL=PASS` |
| Ruff | `D256-RUFF=PASS` |
| Ruff format | `D256-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `E0E6F3AEC1FA4927FCE023FFB4AA54AE2B3D134F10859D0C8CCE4E44E36468CE` |
| PS7 package build | `PASS`, SHA `8E53E9DF668E6651156AEB243BBDDC173A32A7FCD5D679700469B94471AEA713` |
| Package identity | root/dist match, 38,580,380 bytes |
| Source revision | `tree-sha256:4ce202e572231e7ccc394fe637fcc95e1c0233ad14d54869eb87d499b928fe56` |
| Frozen runtime dependencies | `D256-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7`, outer=166, PYZ=261 |
| PyInstaller warnings | `D256-PYINSTALLER-WARNING-SCOPE=PASS`, lines=29 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
clean-machine startup, signing, installer, updater/rollback, registry,
cross-machine, support, and release-owner checks remain unrun under the
permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probe verifies the i18n transformation but not native rendering,
locale-switch interaction, plugin lifecycle execution, or GUI startup. The
current release dossier remains `no-go` because artifact-bound startup/
performance reports and enterprise gates are open.

## Acceptance and evidence IDs

`S304`, `ARCH-234`, `D256-BUILTIN-PLUGIN-LOCALIZATION=PASS cases=3`,
`D256-SIMPLIFICATION-ASSESSMENT=PASS`,
`D256-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D256-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `8E53E9DF668E6651156AEB243BBDDC173A32A7FCD5D679700469B94471AEA713`
- Bytes: `38,580,380`
- Source revision: `tree-sha256:4ce202e572231e7ccc394fe637fcc95e1c0233ad14d54869eb87d499b928fe56`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The built-in Document Statistics user-visible strings
are packaged and statically verified; native startup and remaining release
gates remain open.
