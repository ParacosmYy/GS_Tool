# Handoff: 2026-08-12-d248-workspace-typed-error-projection

| Field | Value |
|---|---|
| ID | `2026-08-12-d248-workspace-typed-error-projection` |
| Delivery / slice | `D248 / ARCH-226 Workspace typed-error projection` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D248 / ARCH-226 handoff — workspace typed-error projection

## User outcome

When opening a workspace folder fails, the workspace panel now retains the
typed filesystem exception long enough for Chinese permission, missing-path,
directory, or codec wording to be shown. English output and ordinary invalid
result messages remain compatible.

## Scope and boundaries

The change is limited to the existing workspace navigation presentation seam:
the coordinator forwards `Exception`, the view/surface protocols accept the
union type, and `WorkspacePanel` delegates typed errors to the shared i18n
resolver. Workspace services, operation tracking, session restoration,
loading state, notifications, and visible page policy are unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Dewey the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Beauvoir the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/workspace_navigation_coordinator.py` — forward
  the typed failure object and widen the view protocol.
- `src/quillforge/presentation/workspace_surface.py` — preserve the widened
  presentation contract.
- `src/quillforge/presentation/workspace_panel.py` — select typed or string
  localization at the existing status projection endpoint.
- `docs/adr/0292-workspace-typed-error-projection.md` and review records.
- `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, `docs/handoffs/index.json`,
  `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, `docs/RELEASE_HANDOFF.md`,
  `tasks/plan.md`, and `tasks/todo.md` — evidence binding.

## Decisions and constraints

Typed errors are used only during the existing presentation call; no error is
stored or re-raised. Invalid-result strings still use `localize_message`, and
English typed errors still render as `str(error)`. No runtime startup or native
dialog claim is made.

## Verification commands and results

| Evidence | Result |
|---|---|
| Typed workspace error route | `D248-WORKSPACE-TYPED-ERROR-ROUTING=PASS` |
| Chinese filesystem error | `D248-WORKSPACE-CHINESE-ERROR=PASS` |
| English compatibility | `D248-WORKSPACE-ENGLISH-COMPATIBILITY=PASS` |
| Python compile | `D248-COMPILEALL=PASS` |
| Ruff | `D248-RUFF=PASS` |
| Ruff format | `D248-FORMAT=PASS` |
| PS5.1 package build | `PASS`, intermediate SHA `F1BCBF3FD4FE663C9E266CB5536751458473DA7E14AA4359E0DD00FED10BEFF3` |
| PS7 package build | `PASS`, final SHA `E016DF4FE44370527CA916C531F8E595FDB796D8792ABE468925FF0139D2D57B` |
| Package identity | root/dist match, 38,576,424 bytes |
| Source revision | `tree-sha256:368041dba7a50b9cef404c4036fd1337b76d253c370d099ec3f7c25cf295be11` |
| Frozen archive | `D248-ARCHIVE-ESSENTIALS=PASS`, outer=166, inner=261; workspace modules/PyQt6/qwindows present |
| PyInstaller warnings | `D248-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialog, desktop association launch,
real filesystem permission timing, clean-machine startup, signing, installer,
updater/rollback, registry, permission/disk-pressure, cross-machine, support,
and release-owner checks remain unrun under the permanent no-launch and
non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

The typed projection improves error comprehension but does not make workspace
access succeed, change ACLs, alter retry policy, or eliminate filesystem races.
The status notification still follows the existing generic notification
contract; this slice specifically closes the workspace panel error surface.

## Acceptance and evidence IDs

`S296`, `ARCH-226`, `D248-WORKSPACE-TYPED-ERROR-ROUTING=PASS`,
`D248-WORKSPACE-CHINESE-ERROR=PASS`,
`D248-WORKSPACE-ENGLISH-COMPATIBILITY=PASS`, `D248-PACKAGE-IDENTITY=PASS`,
`D248-SIMPLIFICATION-ASSESSMENT=PASS`,
`D248-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D248-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next explicitly authorized native startup, native
folder/file dialog, and permission evidence. This handoff does not authorize
launching the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `E016DF4FE44370527CA916C531F8E595FDB796D8792ABE468925FF0139D2D57B`
- Bytes: `38,576,424`
- Source revision: `tree-sha256:368041dba7a50b9cef404c4036fd1337b76d253c370d099ec3f7c25cf295be11`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Workspace navigation now preserves typed failures at the
panel boundary and the portable candidate is rebuilt; native startup and
release evidence remain open.
