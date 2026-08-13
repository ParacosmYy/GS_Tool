# Handoff: 2026-08-12-d289-file-open-preflight

| Field | Value |
|---|---|
| ID | `2026-08-12-d289-file-open-preflight` |
| Delivery / slice | `D289 / ARCH-259 File-open preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The explicit file-open diagnostic now proves a real regular file can travel
through the production startup-path queue, asynchronous document-open
admission, file decoding, and tab projection. This directly covers the
reported folder-versus-file failure boundary without changing normal launch.

## Scope and boundaries

- Added `--diagnose-file-open <path> --report <path>`.
- Reused `DesktopRuntime` restore/command ordering and
  `MainWindow.open_startup_paths()`.
- Kept file opening in `DocumentOpenAdmissionCoordinator` and
  `DocumentService`; no second open path was introduced.
- Rejected non-regular files in the diagnostic and safely skipped modal
  recovery candidates.
- No EXE/Qt launch, native dialog, unit-test asset, installer/updater/registry
  operation, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Luna/max consultation | File-open boundary and architecture review |
| Developer | parent | Diagnostic/runtime seam implementation |
| QA | parent | Source, static, compile, package, PE/archive, and identity checks |
| Independent reviewer | Luna/max consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/app.py` — explicit file-open diagnostic dispatcher.
- `src/quillforge/composition.py` — startup-path preflight ordering seam.
- `src/quillforge/presentation/main_window.py` — queued path drain result and
  shared bounded wait.
- `scripts/audit_presentation_contracts.py` — file-open diagnostic contract.
- `README.md`, architecture/roadmap/plan records, ADR, reviews, acceptance,
  delivery register, and handoff index.

## Decisions and constraints

- Preserve one document-open boundary for picker, workspace activation, session
  restore, and explicit startup paths.
- Do not let a diagnostic open a modal recovery decision or write user state.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, package, and static evidence was authorized.

## Public-source applicability

Public Python 3.12 path/argument behavior and public Qt 6 event/lifecycle
references are applicable. No manufacturer requirement applies. Embedded
workflow and simplifier: `N/A` because this is Python/Qt desktop code. No
private ByteDance standard, MISRA, ISO 26262, ASPICE, or certification claim is
made.

## Verification commands and results

| Evidence | Result |
|---|---|
| File-open source diagnostic | `D289-FILE-OPEN-DIAGNOSTIC-EXIT=0`; `status=passed`, `file_present=true`, `startup_paths_total=1`, `startup_paths_open=1` |
| Async completion | `startup_restore_completed=true`, `pending_work=false`, `pending_startup_paths=0`, `tab_count=2` |
| No-window boundary | `window_shown=false`; `event_loop_entered=false` |
| User-state write boundary | session/settings timestamps unchanged after the preflight |
| File-open contract | `D289-FILE-OPEN-CONTRACT=PASS` |
| Compile/lint/format | `D289-COMPILEALL=PASS`; `D289-RUFF=PASS`; `D289-FORMAT=PASS` |
| Project checks | `D289-CHECK=PASS`; presentation contract audit passed |
| PE header | `D289-PE-HEADER=PASS`; AMD64 / PE32+ / WINDOWS_GUI |
| Frozen archive | `D289-PE-ARCHIVE=PASS`; 9 required entries present |
| Package identity | `D289-MANIFEST-COPY=PASS`; 38,590,986 bytes; SHA-256 `B663169E9BBD9D3B1A0830CDF060AD3945CF22C0DF0B464C95E71E2965AFE7A8` |
| Release verifier | `EXPECTED-NO-GO`; 10 open gates; mechanical failures are the three artifact-bound startup-report consistency checks |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded waits and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE startup, Windows shell drag/drop, file association invocation,
frozen asynchronous file opening, native dialogs, clean-machine behavior,
cross-machine repeatability, signing, installer/update, registry, and release
owner acceptance remain unrun under the active no-launch policy. Release
verification is expected to remain `NO-GO` with historical artifact-bound
reports and enterprise gates open.

## Known risks and limits

- The source diagnostic proves one explicit regular file; it does not prove all
  codecs, permissions, network shares, drag/drop shell behavior, or native
  rendering.
- Valid recovery candidates are intentionally skipped because their decision is
  user-owned and modal.
- The independent review and architecture consultation returned no conclusion;
  no independent approval is claimed.

## Acceptance and evidence IDs

- Acceptance: `S329`.
- Architecture slice: `ARCH-259`.
- Evidence: `D289-FILE-OPEN-CONTRACT=PASS`,
  `D289-FILE-OPEN-DIAGNOSTIC=PASS`, `D289-CHECK=PASS`,
  `D289-PE-HEADER=PASS`, `D289-PE-ARCHIVE=PASS`,
  `D289-MANIFEST-COPY=PASS`, `D289-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: `architect`.
- Action: retain the candidate identity and keep native startup unrun until an
  authorized Windows runtime evidence window is available.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `B663169E9BBD9D3B1A0830CDF060AD3945CF22C0DF0B464C95E71E2965AFE7A8` /
  `38,590,986` bytes; root copy matches.
- Source revision: `tree-sha256:b9e5decb47ab8ce3e3d440f73b344f88ba8b1c0bffa44e066ee5867bc7392bdf`.
- Packaging note: portable candidate rebuilt and archive-bound; no native launch.

## Disposition

`accepted-with-limits`: source file-open evidence, package identity, and
non-destructive release checks are recorded; native startup and enterprise
release gates remain open.
