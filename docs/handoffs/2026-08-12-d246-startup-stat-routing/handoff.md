# Handoff: 2026-08-12-d246-startup-stat-routing

| Field | Value |
|---|---|
| ID | `2026-08-12-d246-startup-stat-routing` |
| Delivery / slice | `D246 / ARCH-224 Startup path stat routing` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D246 / ARCH-224 handoff — startup path stat routing

## User outcome

When an explicit file or folder launch path disappears or becomes invalid
between argument parsing and admission, the shell now retains the concrete
`stat()` failure detail instead of silently treating the path as an unknown
type. Existing directory routing, file opening, special-file rejection, and
English/Chinese prefix localization remain intact.

## Scope and boundaries

The change is limited to `MainWindow._drain_startup_paths()`. It replaces two
swallowing path predicates with one `Path.stat()` call and standard-library
mode checks. Startup queue ordering, recovery/session barriers, async
document/workspace admission, error localization, and Qt composition remain
unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Boole the 7th / Luna max architecture window: `NO_CONCLUSION`.
- Maxwell the 7th / Luna max independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — use one stat-based startup
  path classification boundary.
- `docs/adr/0290-startup-stat-routing.md` and review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/RELEASE_HANDOFF.md`, `tasks/plan.md`, and `tasks/todo.md` — evidence
  binding.

## Decisions and constraints

`Path.stat()` is intentionally called once per queued path. It follows links,
matching prior predicate semantics. Only directories and regular files enter
existing admission coordinators; special files still warn. Stat failures use
the existing localized warning path with the original exception suffix.

## Verification commands and results

| Evidence | Result |
|---|---|
| Source routing contract | `D246-STARTUP-STAT-ROUTING=PASS` |
| Error-context preservation | `D246-STARTUP-ERROR-CONTEXT=PASS` |
| Source gate | `D246-SOURCE-GATE=PASS` |
| Python compile | `D246-COMPILEALL=PASS` |
| Ruff | `D246-RUFF=PASS` |
| Ruff format | `D246-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `F18088F6A2756382F7D25B2FD40283CBE0D48A4F1956FF53D4F19CDA78323E9A` |
| PS7 package build | `PASS`, final SHA `9BF18A4ABFAD6D4E27304A0AC9CF50E031A02C3514F4D3AD04FA51AFAF6F23C7` |
| Package identity | `D246-PACKAGE-IDENTITY=PASS`, 38,576,264 bytes, root/dist match |
| Release manifest | `D246-MANIFEST-PARSE=PASS`, source `tree-sha256:1db3db2eb740006e28ca3e6ca524bcec48a51bd90c85e792fdc06336e738c8ce` |
| Frozen archive | `D246-ARCHIVE-ESSENTIALS=PASS`, outer=166, inner=261; required entrypoints/PyQt6/qwindows present |
| PyInstaller warnings | `D246-PYINSTALLER-WARNING-SCOPE=PASS`, 25 non-empty lines |
| Project check PS5.1 | `D246-CHECK-PS51=PASS` |
| Project check PS7 | `D246-CHECK-PS7=PASS` |
| Handoff check PS5.1 | `D246-HANDOFF-PS51=PASS` |
| Handoff check PS7 | `D246-HANDOFF-PS7=PASS` |
| Release verifier | `D246-RELEASE-VERIFY=EXPECTED-NO-GO`, open_gates=10, mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent` |
| Final dossier binding | `D246-FINAL-DOSSIER-BINDING=PASS`, decision=no-go |
| Final JSON parse | `D246-FINAL-JSON-PARSE=PASS`, files=5 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialog, desktop association launch,
real delete/replace race, clean-machine startup, signing, installer,
updater/rollback, registry, permission/disk-pressure, cross-machine,
support, and release-owner checks remain unrun under the permanent no-launch
and non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

The single stat narrows but cannot eliminate a later filesystem replacement
race. Special files remain unsupported. Static source routing cannot prove
Windows ACL behavior, shell association timing, or native Qt rendering.

## Acceptance and evidence IDs

`S294`, `ARCH-224`, `D246-STARTUP-STAT-ROUTING=PASS`,
`D246-STARTUP-ERROR-CONTEXT=PASS`, `D246-SOURCE-GATE=PASS`,
`D246-PACKAGE-IDENTITY=PASS`, `D246-SIMPLIFICATION-ASSESSMENT=PASS`,
`D246-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D246-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next explicitly authorized native startup and real
shell/file-association evidence. This handoff does not authorize launching
the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `9BF18A4ABFAD6D4E27304A0AC9CF50E031A02C3514F4D3AD04FA51AFAF6F23C7`
- Bytes: `38,576,264`
- Source revision: `tree-sha256:1db3db2eb740006e28ca3e6ca524bcec48a51bd90c85e792fdc06336e738c8ce`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The startup path classification diagnostic gap is
closed statically and the portable candidate is rebuilt; native startup and
release evidence remain open.
