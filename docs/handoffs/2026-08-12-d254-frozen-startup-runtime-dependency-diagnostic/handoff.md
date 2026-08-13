# Handoff: 2026-08-12-d254-frozen-startup-runtime-dependency-diagnostic

| Field | Value |
|---|---|
| ID | `2026-08-12-d254-frozen-startup-runtime-dependency-diagnostic` |
| Delivery / slice | `D254 / ARCH-232 Frozen-startup runtime dependency diagnostic` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

The existing no-window startup diagnostic now reports missing Qt Core/Gui/
Widgets, QScintilla, and platform-bundle dependencies individually, making a
portable EXE startup failure easier to diagnose without launching the GUI.

## Scope and boundaries

Only `--diagnose-startup` frozen-bundle reporting changed. The helper checks
fixed paths with `Path.is_file()` and reports missing entries; it does not load
DLLs, change environment variables, alter normal startup, or mutate user data.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Bacon the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Aristotle the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/app.py` — add frozen Qt/QScintilla dependency presence
  reporting to the existing startup diagnostic.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The new check is diagnostic-only and preserves normal application startup,
qwindows detection, import checks, and failure exit semantics. No EXE/Qt
launch, native dialog, registry, installer, updater, worktree, or test-only
asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Source startup diagnostic | `D254-SOURCE-DIAGNOSTIC-EXIT=0` |
| Source dependency status | `D254-DIAGNOSTIC-DEPENDENCIES-STATUS=not_applicable` |
| Python compile | `D254-COMPILEALL=PASS` |
| Ruff | `D254-RUFF=PASS` |
| Ruff format | `D254-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `6D22D31EF3A0FF48BBAF87FB4E399AEAC363B2C2B35A409E72C3F94232BBBCED` |
| PS7 package build | `PASS`, SHA `3925F642D38536DAA59420FAF4F1CCA1A9D6A26BA4CD89900EE052AE01C68CFA` |
| Package identity | root/dist match, 38,581,732 bytes |
| Source revision | `tree-sha256:6f73e761de1f8eac8e1d5575a7c7085cc63e92fbaf435a25e730d4700ee93009` |
| Frozen runtime dependencies | `D254-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7`, outer=166, PYZ=261 |
| PyInstaller warnings | `D254-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
clean-machine startup, signing, installer, updater/rollback, registry,
cross-machine, support, and release-owner checks remain unrun under the
permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The diagnostic proves frozen archive path presence, not DLL loadability or
native GUI startup. It does not repair missing dependencies; it reports them.
Current archive evidence shows all required paths present.

## Acceptance and evidence IDs

`S302`, `ARCH-232`, `D254-SOURCE-DIAGNOSTIC-EXIT=0`,
`D254-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7`,
`D254-SIMPLIFICATION-ASSESSMENT=PASS`,
`D254-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D254-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `3925F642D38536DAA59420FAF4F1CCA1A9D6A26BA4CD89900EE052AE01C68CFA`
- Bytes: `38,581,732`
- Source revision: `tree-sha256:6f73e761de1f8eac8e1d5575a7c7085cc63e92fbaf435a25e730d4700ee93009`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The diagnostic is packaged and records runtime
dependency presence; native startup and remaining release gates remain open.
