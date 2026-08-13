# Handoff: 2026-08-12-d283-startup-error-log-lifecycle

| Field | Value |
|---|---|
| ID | `2026-08-12-d283-startup-error-log-lifecycle` |
| Delivery / slice | `D283 / ARCH-253 Startup-error log lifecycle` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

QuillForge no longer leaves a previous startup exception visible as if it were
the current failure. Each new entry attempt clears only the prior app-owned
startup log; if the attempt fails, the existing guarded recorder writes a
fresh traceback.

## Scope and boundaries

- Added fail-open cleanup at the existing `main()` entry boundary.
- Added a static lifecycle contract for cleanup-before-dispatch and the
  existing failure writer.
- No settings, recovery, session, registry, installer, updater, or arbitrary
  user paths are touched.
- No EXE/Qt launch, native dialog, unit-test asset, or worktree was used.
- The old local D225 log was observed before this change and is now absent
  after a successful source preflight; no other user files were removed.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Bohr consultation | Entry boundary, file scope, and lifecycle review |
| Developer | parent | Startup log cleanup and static contract |
| QA | parent | Static, source-diagnostic, stale-log, archive, package, and handoff verification |
| Independent reviewer | Banach consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/__main__.py` — fail-open stale-log cleanup.
- `scripts/audit_presentation_contracts.py` — startup-log lifecycle contract.
- D283 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Keep `_record_startup_failure()` as the only failure-log writer.
- Delete only the fixed app-owned path, never a directory or user-selected
  path.
- Prefer no stale evidence over a misleading historical traceback; native
  startup evidence remains separately authorized.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, and package evidence was authorized.

## Public-source applicability

Python 3.12 first-party `pathlib.Path.unlink(missing_ok=True)` behavior is the
applicable reference. No manufacturer requirement applies; this delivery has
no embedded C/C++, MCU, BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or firmware
change. Embedded workflow and simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Startup-log lifecycle | `D283-STARTUP-LOG-CONTRACT=PASS clear_before_dispatch=1 fail_open=1 writer_preserved=1` |
| Stale-log behavior | `D283-STALE-LOG-CLEAR=PASS path_absent_after_successful_preflight` |
| Presentation audit | `D283-PRESENTATION-AUDIT=PASS` |
| Source startup diagnostic | `D283-SOURCE-DIAGNOSTIC=PASS exit=0 status=passed settings_preflight=passed` |
| Compileall | `D283-COMPILEALL=PASS` |
| Ruff/formatting | `D283-RUFF=PASS`; `D283-FORMAT=PASS` |
| Project checks | `D283-CHECK=PASS` |
| PE header | `D283-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI` |
| Frozen archive | `D283-PE-ARCHIVE=PASS outer_entries=166 pyz_entries=261 required=7 embedded_startup_module=True` |
| Root/dist identity | `D283-MANIFEST-COPY=PASS bytes=38585719 sha=5BFE08923B9EC9DAECE95808E707D637843BA13E19DC473509E11D081DF0AE9D` |
| Source revision | `tree-sha256:b89d57b08c0a1199b983029543231f235d0348b917a670c6e43ce819dc10cd92` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded wait and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, clean-machine behavior, signing, installer/update,
registry, cross-machine repeatability, and release-owner acceptance remain
unrun under the active no-launch/non-destructive policy. Release verification
is expected to remain `NO-GO` with artifact-bound mechanical failures and
remaining enterprise gates open.

## Known risks and limits

If a process is killed after cleanup and before the guarded failure writer,
there may be no startup log for that attempt. The documented no-window
preflight remains available, and stale diagnostics are intentionally not
retained as current evidence. Concurrent launches may replace a failure log,
which matches the existing single-path diagnostic design.

## Acceptance and evidence IDs

- Acceptance: `S323`.
- Architecture slice: `ARCH-253`.
- Evidence: `D283-STARTUP-LOG-CONTRACT=PASS`, `D283-CHECK=PASS`,
  `D283-MANIFEST-IDENTITY=PASS`,
  `D283-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D283-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D283-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: if the candidate still shows no window, run the documented
  no-window preflight and attach the fresh JSON report; attach
  `%LOCALAPPDATA%\QuillForge\startup-error.log` only when the current attempt
  actually recreated it.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256 / size:
  `5BFE08923B9EC9DAECE95808E707D637843BA13E19DC473509E11D081DF0AE9D` /
  `38,585,719` bytes.
- Source revision:
  `tree-sha256:b89d57b08c0a1199b983029543231f235d0348b917a670c6e43ce819dc10cd92`.

## Disposition

`accepted-with-limits`: stale startup evidence is cleared at each entry
attempt and current failures remain recordable; native startup and enterprise
release gates remain open.
