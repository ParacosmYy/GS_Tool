# Handoff: 2026-08-12-d291-entrypoint-runtime-cleanup

| Field | Value |
|---|---|
| ID | `2026-08-12-d291-entrypoint-runtime-cleanup` |
| Delivery / slice | `D291 / ARCH-261 Entrypoint runtime cleanup` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T22:00:00+08:00` |

## User outcome

The normal desktop entrypoint now places `runtime.start()` inside the existing
`try/finally` that owns `application.exec()` and `runtime.stop()`. If startup
fails after the runtime has been constructed, plugin/resource cleanup remains
reachable without changing startup order, diagnostic routing, or the runtime
composition boundary.

## Scope and boundaries

### In scope

- The normal `quillforge.app.main()` runtime lifecycle after composition.
- A targeted static contract for the startup/cleanup control flow.

### Out of scope

- Native EXE launch, shell activation, GUI rendering, and external release
  gates.

## Changed files and modules

- `src/quillforge/app.py` — normal `main()` lifecycle boundary.
- `scripts/audit_presentation_contracts.py` — targeted AST cleanup contract.
- `docs/adr/0327-entrypoint-runtime-cleanup.md` — decision and limits.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Lifecycle boundary and final integration |
| Project Manager | parent record | Dependencies, risks, and handoff status |
| Product | parent record | Startup reliability outcome |
| Developer 1 | parent | Entrypoint implementation |
| Developer 2 | parent | Static contract and packaging integration |
| QA | parent | Source, package, and non-destructive verification |

## Decisions and constraints

- Keep composition before the cleanup boundary because construction failure has
  no runtime object to stop; keep the existing early failure reporter as owner.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and no-window diagnostic evidence were authorized.

## Review and source applicability

Parent review: PASS. Simplification assessment: PASS. The architecture
consultation returned `NO_CONCLUSION` after two bounded waits; no approval is
claimed. Independent review status is recorded in the companion review file.

This is Python/PyQt desktop code. Embedded public-vendor source applicability
is N/A; no manufacturer, MCU, SDK, RTOS, MISRA, ISO 26262, ASPICE, or
certification claim is made. The Python `try` statement reference is an
engineering reference only:
<https://docs.python.org/3/reference/compound_stmts.html#the-try-statement>.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src scripts` | PASS | Source compiles. |
| `uv run ruff check src scripts` | PASS | Ruff clean. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Targeted entrypoint AST contract and existing presentation contracts pass. |
| `uv run python -m quillforge --diagnose-startup --report .diagnostics-d291-startup.json` | PASS | Source startup composition passed; no window/exec entered. |
| `uv run python -m quillforge --diagnose-file-open .\README.md --report .diagnostics-d291-file-open.json` | PASS | Existing regular file reached one startup-open tab; no window/exec entered. |
| `scripts/package.ps1` | PASS | Portable EXE and root copy rebuilt with matching identity. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, required 9 archive entries present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | 10 open gates; remaining mechanical failures are `packaged_report_artifact_match`, `interactive_startup_report_consistent`, and `startup_preflight_report_consistent`. |

## Unrun checks and reason

- Native EXE launch — prohibited by the active no-launch policy; requires an
  explicit user-authorized runtime session.
- Clean-machine startup, shell drag-and-drop/file association, native GUI
  rendering, signing, installer, updater, and release-owner gates — external
  release evidence remains open.

## Known risks and limits

- The independent Luna/max review returned `NO_CONCLUSION` after two bounded
  waits; this is not an independent approval.
- Source diagnostics and archive inspection do not prove native Windows
  startup or native widget rendering.

## Acceptance and evidence IDs

- Acceptance: `S331`
- Evidence: `D291-ENTRYPOINT-CLEANUP-CONTRACT=PASS`,
  `D291-SOURCE-DIAGNOSTIC=PASS`, `D291-COMPILEALL=PASS`, `D291-RUFF=PASS`,
  `D291-CHECK=PASS`, `D291-MANIFEST-COPY=PASS`, `D291-PE-HEADER=PASS`,
  `D291-PE-ARCHIVE=PASS`, `D291-SIMPLIFICATION-ASSESSMENT=PASS`,
  `D291-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D291-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize a native/clean-machine runtime session before any release
  decision; retain D291 as the current source/package candidate meanwhile.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `CE93E76021C5D91FC23349CE078C08F49C9CE1A3D8897DD16F6E889C65302242` / `38591241` bytes
- Source revision: `tree-sha256:e8e96d2c4bf70553fe7e80017c045ff1286de0fede3d9fcad60a3f72a75040f1`
- Packaging note: rebuilt portable one-file candidate; root copy matches.

## Disposition

Accepted with limits. D291 is source/package verified and handoff-complete;
native startup and external release gates remain conditions for a later
release decision.

## Verification boundary

Authorized non-destructive checks are source compilation, Ruff, the
presentation contract audit, repository checks, package/PE/archive inspection,
and the existing source diagnostics. Native EXE launch, clean-machine startup,
GUI rendering, shell drag-and-drop/file association, signing, installer, and
release-owner gates remain unrun.

## Artifact

The final D291 package identity and release-verifier result are recorded here
after packaging; the release verifier is expected to remain no-go while native
startup and external release gates are open.
