# Handoff: 2026-08-12-d255-application-error-localization-boundary

| Field | Value |
|---|---|
| ID | `2026-08-12-d255-application-error-localization-boundary` |
| Delivery / slice | `D255 / ARCH-233 Application-error localization boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

Chinese users now receive localized application validation and coordinator
failure details for common document, workspace, search, plugin, recovery, and
autosave paths. Dynamic paths, names, IDs, and unknown details remain visible.

## Scope and boundaries

Only the shared `presentation.i18n` boundary changed. It adds a bounded
application-message catalog and known wrapper localization; it does not change
application policy, exception construction, worker scheduling, filesystem
behavior, normal startup, or the English locale.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Zeno the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Ptolemy the 7th / Luna max` independent-review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — localize bounded application errors
  and nested presentation failure details.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The existing presentation localizer remains the single translation boundary.
Finite exact/prefix/regex mappings preserve dynamic diagnostics and leave
unknown text unchanged. No EXE/Qt launch, native dialog, registry, installer,
updater, worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Localization boundary probe | `D255-LOCALIZATION-BOUNDARY=PASS cases=7` |
| Source startup diagnostic | `D255-SOURCE-DIAGNOSTIC-EXIT=0` |
| Source dependency status | `D255-DIAGNOSTIC-DEPENDENCIES-STATUS=not_applicable` |
| Python compile | `D255-COMPILEALL=PASS` |
| Ruff | `D255-RUFF=PASS` |
| Ruff format | `D255-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `D17A37B57B86AC2EAC6ABAC3603FDD1359F9051D43B477F8F5988FDB39351CE1` |
| PS7 package build | `PASS`, SHA `C9BB0C6B8271CA3534DA6DA98C34D87C8CCDB6B96E2BB52D75E88BB077CD63B9` |
| Package identity | root/dist match, 38,581,105 bytes |
| Source revision | `tree-sha256:30a23d226f08c89b797e3ecb4fbc4dfb57941cd8b8077d9bc78f4c771732d07d` |
| Frozen runtime dependencies | `D255-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7`, outer=166, PYZ=261 |
| PyInstaller warnings | `D255-PYINSTALLER-WARNING-SCOPE=PASS`, lines=29 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
clean-machine startup, signing, installer, updater/rollback, registry,
cross-machine, support, and release-owner checks remain unrun under the
permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The localizer proves presentation text transformation for the bounded probe
cases, not native rendering, locale switching, or a successful GUI startup.
Unknown provider and infrastructure text intentionally remains available as a
diagnostic fallback. The current release dossier remains `no-go` because
artifact-bound startup/performance reports and enterprise gates are open.

## Acceptance and evidence IDs

`S303`, `ARCH-233`, `D255-LOCALIZATION-BOUNDARY=PASS cases=7`,
`D255-SIMPLIFICATION-ASSESSMENT=PASS`,
`D255-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D255-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `C9BB0C6B8271CA3534DA6DA98C34D87C8CCDB6B96E2BB52D75E88BB077CD63B9`
- Bytes: `38,581,105`
- Source revision: `tree-sha256:30a23d226f08c89b797e3ecb4fbc4dfb57941cd8b8077d9bc78f4c771732d07d`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The user-visible application-error localization boundary
is packaged and statically verified; native startup and remaining release
gates remain open.
