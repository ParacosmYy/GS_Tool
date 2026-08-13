# Handoff: 2026-08-12-d292-runtime-shutdown-activity-boundary

| Field | Value |
|---|---|
| ID | `2026-08-12-d292-runtime-shutdown-activity-boundary` |
| Delivery / slice | `D292 / ARCH-262 Runtime shutdown activity boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:00:00+08:00` |

## User outcome

If the desktop runtime fails after construction, periodic recovery and
debounced session-save timers are now stopped before plugin deactivation. The
normal close path reuses the same idempotent timer boundary.

## Scope and boundaries

### In scope

- `DesktopRuntime.stop()` lifecycle ordering.
- A presentation-owned shutdown port for periodic UI activity.
- A targeted static contract for the shutdown order.

### Out of scope

- Forced worker termination, native EXE launch, clean-machine validation,
  native rendering, and external release gates.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Lifecycle boundary and final integration |
| Project Manager | parent record | Dependencies, risks, and status |
| Product | parent record | Startup-failure reliability outcome |
| Developer 1 | parent | Composition and presentation implementation |
| Developer 2 | parent | Static contract and package integration |
| QA | parent | Source, package, and non-destructive verification |

## Changed files and modules

- `src/quillforge/composition.py` — shutdown order and lifecycle delegation.
- `src/quillforge/presentation/main_window.py` — public timer-stop port.
- `scripts/audit_presentation_contracts.py` — targeted shutdown contract.
- `docs/adr/0328-runtime-shutdown-activity-boundary.md` — decision and limits.

## Decisions and constraints

- Stop only periodic timers; do not force or synchronously wait for
  `TaskRunner` callbacks that require the Qt event-loop boundary.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- Runtime launch policy: EXE/Qt startup was not allowed; source, static,
  package, archive, and no-window diagnostic evidence were authorized.

## Review and source applicability

Parent review: PASS. Simplification assessment: PASS. Architecture
consultation: `PASS_WITH_LIMITS`; it accepts the timer-only high-cohesion
boundary and explicitly leaves worker teardown, queued-callback draining, and
runtime reuse out of scope. Independent review is recorded in the companion
review file and is not presumed to pass.

This is Python/PyQt desktop code. Embedded public-vendor source applicability
is N/A; no manufacturer, MCU, SDK, RTOS, MISRA, ISO 26262, ASPICE,
certification, or private ByteDance-standard claim is made. Python and Qt
references are engineering references only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src scripts` | PASS | Source compiles. |
| `uv run ruff check src scripts` | PASS | Ruff clean. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Shutdown and existing presentation contracts pass. |
| `uv run ruff format --check src scripts` | PASS | Formatting check recorded after mechanical formatting. |
| `uv run python -m quillforge --diagnose-startup --report .diagnostics-d292-startup.json` | PASS | Source startup composition passed; no window/exec entered. |
| `uv run python -m quillforge --diagnose-file-open .\README.md --report .diagnostics-d292-file-open.json` | PASS | Existing regular file reached one startup-open tab; no window/exec entered. |
| `scripts/check.ps1` | PASS | Handoff, presentation audit, formatting, and project checks passed. |
| `scripts/package.ps1` | PASS | Portable EXE and root copy rebuilt with matching identity. |
| PE/archive inspection | PASS | AMD64, PE32+, Windows GUI, required 9 archive entries present. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | 10 open gates; three artifact-bound native report gates remain open. |

## Unrun checks and reason

- Native EXE/Qt startup and normal window close — prohibited by the active
  no-launch policy.
- Clean-machine startup, shell activation, native rendering, signing,
  installer, updater, and release-owner gates — external evidence remains
  open.
- Forced worker teardown — intentionally not introduced or exercised because
  no safe cancellation contract exists.

## Known risks and limits

- The independent reviewer may return `NO_CONCLUSION`; no native lifecycle
  behavior is inferred from static or source diagnostics.
- Queued worker callbacks are not forcefully drained by this slice; the Qt
  object/event-loop lifecycle remains the governing owner.

## Acceptance and evidence IDs

- Acceptance: `S332`
- Evidence: `D292-SHUTDOWN-CONTRACT=PASS`,
  `D292-SIMPLIFICATION-ASSESSMENT=PASS`,
  `D292-ARCHITECTURE-CONSULTATION=PASS_WITH_LIMITS`,
  `D292-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: retain the rebuilt D292 candidate and authorize native/clean-machine
  startup evidence before release consideration.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `ABCF8529B5D67B817F1DD21CD500D9EC7A1D73D7F1DC5EF523261AF2972DA30F` / `38592545` bytes
- Source revision: `tree-sha256:cccba35651e2f7a9520ab0412bb97ab8ee331fa86793c5bac39236db58e74a9e`
- Packaging note: rebuilt portable one-file candidate; root copy matches.

## Disposition

Accepted with limits. D292 is source/package verified; native startup and
worker teardown remain explicit release gates.
