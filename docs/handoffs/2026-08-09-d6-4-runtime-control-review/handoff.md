# Handoff: 2026-08-09-d6-4-runtime-control-review

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-4-runtime-control-review` |
| Delivery / slice | `D6 / D6.4 independent runtime-control review closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:24:54+08:00` |

## User outcome

Plugin Status now has an independent source review for explicit enable/disable
of already registered in-process plugins, with trust immutable and cleanup
owned by the plugin manager.

## Scope and boundaries

### In scope

- Immutable runtime status protocol and manager lifecycle ownership.
- Explicit enable/disable, cleanup, re-enable, and UI projection evidence.
- D6.2 dependency and handoff traceability.

### Out of scope

- External catalog loading, signatures, sandboxing, process isolation,
  installation, updates, runtime launch, visual review, unit tests, mocks,
  fixtures, harnesses, and test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate review and final disposition |
| Project Manager | Parent role record | D6 dependency and lifecycle risk |
| Product | Parent role record | Explicit Plugin Status outcome |
| Developer 1 | Parent role record | Runtime protocol and manager ownership |
| Developer 2 | Parent role record | Status dialog projection and ledger sync |
| QA | Helmholtz / Luna, read-only | Independent conditional accepted-with-limits review |

## Changed files and modules

- `docs/agent-team/reviews/D6.4-independent-luna-follow-up.md` — record the
  independent review.
- `docs/handoffs/2026-08-09-d6-4-runtime-control-review/handoff.md` — add the
  material-slice handoff.
- `docs/handoffs/index.json`, `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, and `docs/ROADMAP.md` — synchronize
  D6.4 evidence.

## Decisions and constraints

- Enablement never promotes trust; only explicitly trusted compatible in-process
  registrations may activate.
- Disable deactivates before removing manager-owned resources.
- Shared-checkout writer: Architect only; the Luna reviewer was read-only.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `Helmholtz / Luna independent source review` | `ACCEPTED WITH LIMITS` | Direct source evidence supports the bounded in-process lifecycle; no files changed. |
| Historical D6.4 runtime-control source smoke | `RECORDED` | Immutable status, activation, cleanup, re-enable, unknown-ID, and untrusted-enable cases were previously reported. |
| Historical D6.4 Qt offscreen status smoke | `RECORDED` | Plugin Status command and lifecycle projection were previously reported. |
| `scripts/verify_handoff.ps1` | `PENDING` | Run after indexing this handoff. |
| `scripts/check.ps1` | `PENDING` | Required static/format/compile/acceptance gate. |

## Unrun checks and reason

- QuillForge.exe launch, live Plugin Status, visual review, cross-machine,
  clean-machine, and external-plugin execution — prohibited or out of scope.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Signature, sandbox, installation, and update evidence — future gates.

## Known risks and limits

- In-process callbacks are synchronous and are not an untrusted execution
  boundary.
- Enablement is session-local in D6.4; persistence is D6.5.
- D6.6–D6.8 host containment/execution-gate and D8 release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D6-AC04`, `S16`, `S17`
- Evidence: `src/quillforge/application/plugin_runtime.py`,
  `src/quillforge/plugins/manager.py`, `src/quillforge/presentation/main_window.py`,
  `docs/agent-team/reviews/D6.4.1-parent-review.md`,
  `docs/agent-team/reviews/D6.4-independent-luna-follow-up.md`, and current
  package identity `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4`.

## Next owner and next action

- Owner: Architect
- Action: complete the D6.5 persistent policy review closure, then continue
  the D6.6–D6.8 high-risk host/security audit.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4` / `38,327,064`
- Packaging note: documentation-only closure; no package input changed.

## Disposition

`accepted-with-limits`: D6.4 review and traceability are complete for the
session-local in-process lifecycle; trust, external execution, and release
gates remain open.
