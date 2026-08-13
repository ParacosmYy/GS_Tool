# Handoff: 2026-08-12-d287-startup-state-preflight

| Field | Value |
|---|---|
| ID | `2026-08-12-d287-startup-state-preflight` |
| Delivery / slice | `D287 / ARCH-257 Startup state preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The explicit no-window startup report now exposes whether the same user-local
session and recovery manifests used by normal startup are readable. It reports
session state, missing remembered files, and valid versus malformed recovery
counts without returning document content or a user document path list.

## Scope and boundaries

- Reused `SessionService.load()` for the production session normalization and
  absent/valid/invalid state contract.
- Reused `JsonRecoverySnapshotStore.list_snapshots()` for the production
  recovery manifest read and derived malformed-file count.
- Kept both probes behind `--diagnose-startup`; normal asynchronous recovery
  and session restore order is unchanged.
- No EXE/Qt launch, native dialog, unit-test asset, installer/updater/registry
  operation, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Luna/max consultation | Diagnostic boundary and module ownership review |
| Developer | parent | Read-only session/recovery probe implementation |
| QA | parent | Source, static, compile, package, PE/archive, and identity checks |
| Independent reviewer | Luna/max consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/app.py` — session and recovery startup-state probes.
- `scripts/audit_presentation_contracts.py` — probe order, store reuse,
  non-destructive, and no-content contract.
- `README.md` — diagnostic field and boundary guidance.
- D287 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Keep diagnostic-only state inspection at the application entry-point
  boundary, matching the existing settings preflight.
- Do not add a second application service for reporting metadata.
- Do not serialize snapshot text or user document path lists into the report.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, package, and static evidence was authorized.

## Public-source applicability

Public Python 3.12 standard-library path/JSON behavior and public Qt 6
lifecycle references are applicable. No manufacturer requirement applies.
Embedded workflow and simplifier: `N/A` because this is Python/Qt desktop code,
not embedded C/C++, MCU, BSP/HAL, RTOS, or firmware. No private ByteDance
standard, MISRA, ISO 26262, ASPICE, or certification claim is made.

## Verification commands and results

| Evidence | Result |
|---|---|
| Startup state source diagnostic | `D287-SOURCE-DIAGNOSTIC=PASS exit=0`; runtime composition, editor shell, settings, session, and recovery probes passed |
| User-state result | `session=valid`, `document_count=1`, `missing_document_count=0`, `recovery_directory_present=false`, `valid_snapshot_count=0` |
| Startup-state contract | `D287-STARTUP-STATE-CONTRACT=PASS` |
| Presentation audit | `D287-PRESENTATION-AUDIT=PASS` |
| Compileall | `D287-COMPILEALL=PASS` |
| Ruff/formatting | `D287-RUFF=PASS`; `D287-FORMAT=PASS` |
| Project checks | `D287-CHECK=PASS` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded waits and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, asynchronous session/recovery completion, clean-machine
behavior, native window lifetime, cross-machine repeatability, signing,
installer/update, registry, and release-owner acceptance remain unrun under
the active no-launch/non-destructive policy. Release verification is expected
to remain `NO-GO` with historical artifact-bound reports and enterprise gates
open.

## Known risks and limits

- A passing no-window report does not prove native Windows window creation,
  platform plugin loadability, rendering, or asynchronous restore completion.
- The recovery probe parses existing snapshot payloads but reports counts only;
  it does not prove every recovery decision or document decoder.
- The independent review and architecture consultation returned no conclusion;
  no independent approval is claimed.

## Acceptance and evidence IDs

- Acceptance: `S327`.
- Architecture slice: `ARCH-257`.
- Evidence: `D287-STARTUP-STATE-CONTRACT=PASS`,
  `D287-SOURCE-DIAGNOSTIC=PASS`, `D287-CHECK=PASS`,
  `D287-MANIFEST-IDENTITY=PASS` after packaging.

## Next owner and next action

- Owner: `architect`.
- Action: rebuild the portable candidate, bind its package identity, and run
  the release verifier while keeping native startup unrun.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `C5A5CF1E314DAA45880694D192F9E9BA22B3D3D916F1FE63791F6396B19F7861` / `38588243` bytes; source `tree-sha256:1b368cd1644fde05cbad13f776a3d68d19259db78958b1f116c63ef238950ac9`.
- Packaging note: portable candidate rebuilt; root/dist copies match; no native launch.

## Disposition

`accepted-with-limits`: the source change and authorized no-window evidence are
complete. Package identity and release-dossier refresh remain for the next
handoff step; native startup and asynchronous restore remain outside scope.
