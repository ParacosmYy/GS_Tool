# Handoff: 2026-08-12-d247-typed-secondary-error-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d247-typed-secondary-error-localization` |
| Delivery / slice | `D247 / ARCH-225 Typed secondary error localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D247 / ARCH-225 handoff — typed secondary error localization

## User outcome

Chinese error dialogs for settings persistence and Replace All now retain the
typed exception long enough for the shared localization surface to describe
permission, missing-path, directory, and codec failures in Chinese. English
output and existing diagnostic detail remain compatible.

## Scope and boundaries

The change is limited to two `MainWindow` presentation handlers. They now pass
the existing `Exception` object to `_show_error()` instead of converting it to
text early. `MessageSurface` and `i18n` remain the sole error projection and
translation boundary; services, persistence, async jobs, and document policy
are unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Volta the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Euclid the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — preserve typed exceptions at
  settings-save and Replace All error forwarding.
- `docs/adr/0291-typed-secondary-error-localization.md` and review records.
- `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, `docs/handoffs/index.json`,
  `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, `docs/RELEASE_HANDOFF.md`,
  `tasks/plan.md`, and `tasks/todo.md` — evidence binding.

## Decisions and constraints

`Exception` remains the existing `_show_error` message type and is not stored
or re-raised by the UI. The English localizer still returns `str(error)`;
Chinese typed branches retain the error class and codec/path metadata. No
runtime startup or native dialog claim is made.

## Verification commands and results

| Evidence | Result |
|---|---|
| Typed exception boundary | `D247-TYPED-ERROR-BOUNDARY=PASS` |
| Settings failure localization | `D247-SETTINGS-ERROR-LOCALIZATION=PASS` |
| Replace All failure localization | `D247-REPLACE-ERROR-LOCALIZATION=PASS` |
| Python compile | `D247-COMPILEALL=PASS` |
| Ruff | `D247-RUFF=PASS` |
| Ruff format | `D247-FORMAT=PASS` |
| PS5.1 package build | `PASS`, intermediate SHA `6F7E66D70F0B5BA53DB21788753305CA9F6B22C741CD173975AF46F5385391CA` |
| PS7 package build | `PASS`, final SHA `C7D8AD797BA785C033CCD205156CF79E21F228E664CBB91CD02C2C6F82B92729` |
| Package identity | root/dist match, 38,577,691 bytes |
| Source revision | `tree-sha256:761f83cf30977e6cf45f9fb6e8210db5fec49c77ac8e96aad2240561841b5447` |
| Frozen archive | `D247-ARCHIVE-ESSENTIALS=PASS`, outer=166, inner=261; required entrypoints/PyQt6/qwindows present |
| PyInstaller warnings | `D247-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialog, desktop association launch,
real filesystem error timing, clean-machine startup, signing, installer,
updater/rollback, registry, permission/disk-pressure, cross-machine, support,
and release-owner checks remain unrun under the permanent no-launch and
non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

Preserving the exception object only improves the presentation boundary; it
does not make a failed settings or Replace All operation successful, alter
rollback policy, or prove native dialog rendering. Unknown exception classes
still use the existing localized message fallback.

## Acceptance and evidence IDs

`S295`, `ARCH-225`, `D247-TYPED-ERROR-BOUNDARY=PASS`,
`D247-SETTINGS-ERROR-LOCALIZATION=PASS`,
`D247-REPLACE-ERROR-LOCALIZATION=PASS`, `D247-PACKAGE-IDENTITY=PASS`,
`D247-SIMPLIFICATION-ASSESSMENT=PASS`,
`D247-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D247-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `C7D8AD797BA785C033CCD205156CF79E21F228E664CBB91CD02C2C6F82B92729`
- Bytes: `38,577,691`
- Source revision: `tree-sha256:761f83cf30977e6cf45f9fb6e8210db5fec49c77ac8e96aad2240561841b5447`
- Portable manifest association status: `not-configured`

## Next owner and next action

Project Manager owns the next explicitly authorized native startup and native
dialog evidence. This handoff does not authorize launching the application.

## Disposition

Accepted with limits. Typed error context is preserved at the two remaining
presentation forwarding sites and the portable candidate is rebuilt; native
startup and release evidence remain open.
