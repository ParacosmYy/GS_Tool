# Handoff: 2026-08-12-d245-file-error-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d245-file-error-localization` |
| Delivery / slice | `D245 / UI-27 Typed file-error localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D245 / UI-27 handoff — typed file-error localization

## User outcome

When opening or saving a file fails, the Chinese interface now provides a
readable category for common filesystem and text-codec failures instead of
showing only an English Python exception body. Paths, encodings, positions,
and unknown diagnostic detail remain available. English output remains
unchanged.

## Scope and boundaries

The change is limited to typed exception projection through the existing
presentation error dialog. `i18n.py` owns translation; the open/save
coordinators only preserve the exception object across their existing ports.
File decoding, filesystem access, async dispatch, tab state, save policy,
startup routing, and Qt composition remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Ohm the 7th / Luna max architecture window: `NO_CONCLUSION`.
- Boyle the 7th / Luna max independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — typed filesystem/codec error
  localization and external-change prefix coverage.
- `src/quillforge/presentation/message_surface.py` — project typed failures
  through the existing modal error surface.
- `src/quillforge/presentation/main_window.py` — widen the existing error
  callback type without changing policy.
- `src/quillforge/presentation/document_open_coordinator.py` and
  `src/quillforge/presentation/document_save_coordinator.py` — preserve the
  exception object across existing error ports.
- `docs/adr/0289-file-error-localization.md` and review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/RELEASE_HANDOFF.md`, `tasks/plan.md`, and `tasks/todo.md` — evidence
  binding.

## Decisions and constraints

`en-US` returns the original exception string. `zh-CN` translates only stable
typed categories and known codec reasons; unknown detail is retained. The
presentation layer does not catch or replace the exception. No new
localization service, application error taxonomy, worker, or file-open path
was introduced.

## Verification commands and results

| Evidence | Result |
|---|---|
| Focused exception projection | `D245-EXCEPTION-LOCALIZATION=PASS` |
| External-change prefix | `D245-CONFLICT-LOCALIZATION=PASS` |
| Source gate | `D245-SOURCE-GATE=PASS` |
| Python compile | `D245-COMPILEALL=PASS` |
| Ruff | `D245-RUFF=PASS` |
| Ruff format | `D245-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `B357BDD4088BB3D6B2528FFC0FC1892288528C6A17DD899E89B5A8277027E377` |
| PS7 package build | `PASS`, final SHA `C0DB69ED66CF38F9CA26F23C60335E0419449D064F7ED9A4FE8235F343DF3B63` |
| Package identity | `D245-PACKAGE-IDENTITY=PASS`, 38,576,075 bytes, root/dist match |
| Release manifest | `D245-MANIFEST-PARSE=PASS`, source `tree-sha256:2440482877f350212b122953dfab7e1f0111a363eacfc6db2d7cb3c2393c3e7f` |
| Frozen archive | `D245-ARCHIVE-ESSENTIALS=PASS`, outer=166, inner=261; required entrypoints/PyQt6/qwindows present |
| PyInstaller warnings | `D245-PYINSTALLER-WARNING-SCOPE=PASS`, 25 non-empty lines |
| Project check PS5.1 | `D245-CHECK-PS51=PASS` |
| Project check PS7 | `D245-CHECK-PS7=PASS` |
| Handoff check PS5.1 | `D245-HANDOFF-PS51=PASS` |
| Handoff check PS7 | `D245-HANDOFF-PS7=PASS` |
| Release verifier | `D245-RELEASE-VERIFY=EXPECTED-NO-GO`, open_gates=10, mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent` |
| Final dossier binding | `D245-FINAL-DOSSIER-BINDING=PASS`, decision=no-go |
| Final JSON parse | `D245-FINAL-JSON-PARSE=PASS`, files=5 |

## Unrun checks and reason

Native EXE/Qt startup, native error-dialog rendering, real GUI file open/save,
desktop association launch, clean-machine startup, signing, installer,
updater/rollback, registry, permission/disk-pressure, cross-machine, support,
and release-owner checks remain unrun under the permanent no-launch and
non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

This slice improves error comprehension but cannot make a missing, locked,
directory, or undecodable file open successfully. Unknown OS wording may
remain in its original language. Static probes cannot prove Windows-native
exception construction, filesystem ACL behavior, or Qt dialog layout.

## Acceptance and evidence IDs

`S293`, `UI-27`, `D245-EXCEPTION-LOCALIZATION=PASS`,
`D245-CONFLICT-LOCALIZATION=PASS`, `D245-SOURCE-GATE=PASS`,
`D245-PACKAGE-IDENTITY=PASS`, `D245-SIMPLIFICATION-ASSESSMENT=PASS`,
`D245-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D245-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next explicitly authorized native startup and real
file-open/file-save evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `C0DB69ED66CF38F9CA26F23C60335E0419449D064F7ED9A4FE8235F343DF3B63`
- Bytes: `38,576,075`
- Source revision: `tree-sha256:2440482877f350212b122953dfab7e1f0111a363eacfc6db2d7cb3c2393c3e7f`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The typed file-error localization gap is closed
statically and the portable candidate is rebuilt; native startup and release
evidence remain open.
