# Handoff: 2026-08-12-d249-workspace-error-locale-refresh

| Field | Value |
|---|---|
| ID | `2026-08-12-d249-workspace-error-locale-refresh` |
| Delivery / slice | `D249 / ARCH-227 Workspace error locale refresh` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

When a workspace open/list operation fails and the user changes language, the
same error remains visible at error severity and is retranslated for the new
locale. A successful directory result or a new load clears the obsolete error.

## Scope and boundaries

The change is limited to `WorkspacePanel`'s existing presentation state. It
retains the original `str | Exception` source, reuses the shared typed/string
localizers, and does not move navigation, service, session, notification, or
last-good-page policy into the panel.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Epicurus the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Feynman the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — retain and reproject the
  current error source; clear it when success/new loading supersedes it.
- `docs/adr/0293-workspace-error-locale-refresh.md` and review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/RELEASE_HANDOFF.md`, `tasks/plan.md`, and `tasks/todo.md` — evidence
  binding.

## Decisions and constraints

The retained source is transient UI state: it is not serialized, re-raised, or
passed to application services. Source retention avoids stale translated text
and preserves typed filesystem/codec metadata. No EXE/Qt launch, native dialog,
registry, installer, updater, worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Error source retention | `D249-ERROR-SOURCE-RETENTION=PASS` |
| Locale error reprojection | `D249-LOCALE-ERROR-REPROJECTION=PASS` |
| Success/load clearing | `D249-SUCCESS-AND-LOAD-CLEARING=PASS` |
| Python compile | `D249-COMPILEALL=PASS` |
| Ruff | `D249-RUFF=PASS` |
| Ruff format | `D249-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `A63D05AADFD6E8F40D10493EBA1E0A33DF09A435A1E54E1AF1A3FD62BC021044` |
| PS7 package build | `PASS`, SHA `41DF3416DEBC5C2E59D6E091BB36BA7F9C954D7A3952D6F6E009F5999D6B5E9D` |
| Package identity | root/dist match, 38,578,143 bytes |
| Source revision | `tree-sha256:ff124786da0879a1fa867ae79293297ed029ab4ce3175a6ee2acc94579a726a7` |
| Frozen archive | `D249-ARCHIVE-OUTER=PASS`, outer=166, inner=261; entry/workspace/PyQt6/qwindows present |
| PyInstaller warnings | `D249-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |
| JSON records | `D249-JSON-PARSE=PASS` for acceptance, delivery, handoff, and manifest records |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
real filesystem permission timing, clean-machine startup, signing, installer,
updater/rollback, registry, permission/disk-pressure, cross-machine, support,
and release-owner checks remain unrun under the permanent no-launch and
non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

This slice preserves and retranslates the error state but does not make
workspace access succeed, change ACLs, alter retry policy, or eliminate
filesystem races. Frozen archive evidence cannot prove native rendering or
Windows startup behavior.

## Acceptance and evidence IDs

`S297`, `ARCH-227`, `D249-ERROR-SOURCE-RETENTION=PASS`,
`D249-LOCALE-ERROR-REPROJECTION=PASS`,
`D249-SUCCESS-AND-LOAD-CLEARING=PASS`, `D249-PACKAGE-IDENTITY=PASS`,
`D249-SIMPLIFICATION-ASSESSMENT=PASS`,
`D249-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D249-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup, native
folder/file dialog, and permission evidence. This handoff does not authorize
launching the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `41DF3416DEBC5C2E59D6E091BB36BA7F9C954D7A3952D6F6E009F5999D6B5E9D`
- Bytes: `38,578,143`
- Source revision: `tree-sha256:ff124786da0879a1fa867ae79293297ed029ab4ce3175a6ee2acc94579a726a7`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Workspace error locale refresh is packaged and recorded;
native startup, native dialogs, and the remaining release gates remain open.
