# Handoff: 2026-08-09-d6-6-pid-provenance

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-6-pid-provenance` |
| Delivery / slice | `D6 / D6.6 plugin-host PID provenance` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:35:00+08:00` |

## User outcome

Plugin-host diagnostics now distinguish the PID observed by the launcher from
the PID reported by the host protocol. A pre-handshake failure cannot be
mistaken for proof that a particular host PID was decoded, and no PID is used
as external execution authorization.

## Scope and boundaries

### In scope

- Split the application result into `launcher_pid` and `reported_host_pid`.
- Preserve hello/result consistency and allow the documented launcher/re-exec
  PID difference.
- Keep the global execution gate disabled and update D6 protocol/containment
  provenance records.

### Out of scope

- Dynamic loading, signatures, code identity, executor implementation, or
  external plugin execution.
- Unconditional launcher/host PID equality, process authentication, or a
  complete security sandbox.
- Starting QuillForge, running a packaged host probe, or interactive visual
  acceptance under the current no-launch instruction.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integration, security boundary decision, and final verification |
| Project Manager | Parent role record | Bounded D6.6 follow-up scope and dependency/risk record |
| Product | Parent role record | Confirmed diagnostics must not imply executable trust |
| Developer 1 | Parent role record | Audited application result and execution-gate separation |
| Developer 2 | Parent role record | Applied infrastructure PID-source split |
| QA | Jason / Terra, read-only | High-risk cross-process review and wrapper/re-exec disposition |

## Changed files and modules

- `src/quillforge/application/plugin_host.py` — validate and present separate
  launcher-observed and host-reported PID fields.
- `src/quillforge/infrastructure/plugin_host.py` — populate provenance fields
  according to the process and handshake boundary.
- `docs/adr/0031-plugin-host-pid-provenance.md` — record the decision and
  public platform reference scope.
- `docs/agent-team/reviews/D6.6-pid-provenance-parent-review.md` — record
  parent integration and Terra review disposition.
- `docs/agent-team/acceptance.json` and `docs/agent-team/delivery-register.json`
  — add the provenance evidence to D6.6.
- `docs/ROADMAP.md` and `docs/handoffs/index.json` — synchronize the delivery
  record.

## Decisions and constraints

- `reported_host_pid` is set only after the hello frame is decoded; it is not a
  substitute for an OS process handle.
- Launcher/report PID equality is not required because the two-process
  launcher/re-exec allowance is part of the existing containment contract.
- PID values never enter `PluginExecutionEvidence`; the global external
  execution threshold remains false.
- Shared-checkout writer: Architect only; no child writer was used.
- Runtime launch policy: launch is forbidden by the current project
  instruction; static checks and packaging are allowed.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D6.6 Luna/Terra read-only review` | `PASS WITH LIMITS` | The review identified the source ambiguity and recommended the two-field fix; no child completion PASS is claimed. |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff index, path, headings, and evidence coverage passed before packaging. |
| `scripts/check.ps1` | `PASS` | Formatting, compile, JSON/acceptance policy, and project checks passed before packaging. |
| `scripts/package.ps1` | `PASS` | Root/dist copies rebuilt and synchronized after the PID provenance fix. |

## Unrun checks and reason

- QuillForge.exe startup, packaged `--plugin-host --probe`, wrapper/re-exec
  coverage, Qt inspection, and visual review — intentionally unrun because
  the project instruction prohibits software launch.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Process-image/signature/Job-membership authentication — future security and
  release work, not established by this diagnostic-only change.
- Independent after-source simplification child PASS — not claimed.

## Known risks and limits

- D6.6/D6.7 remain `in-progress`; this fixes provenance ambiguity, not process
  authentication or external execution safety.
- A host-reported PID is still untrusted protocol data; future authorization
  must use OS-owned handles, containment membership, and code-identity proof.
- Current package-bound runtime evidence becomes stale after this source edit
  and must not be reused without an authorized refresh.

## Acceptance and evidence IDs

- Acceptance: `D6-AC06`, `D6-AC07`, `S19`, `S20`
- Evidence: `src/quillforge/application/plugin_host.py`,
  `src/quillforge/infrastructure/plugin_host.py`,
  `docs/adr/0031-plugin-host-pid-provenance.md`,
  `docs/agent-team/reviews/D6.6-pid-provenance-parent-review.md`,
  `scripts/verify_handoff.ps1`, `scripts/check.ps1`, and the post-fix package
  identity.

## Next owner and next action

- Owner: Architect
- Action: run static/package gates and bind the new artifact identity, then
  continue with source-only D8/D9 gaps while retaining the no-launch release
  blockers.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `E87C43084DB1E09809F780E58F4D9C5ACB1D37E795D9F200288CC7F5FF4B1DF3` / `38,326,370` bytes; root/dist copies match
- Packaging note: rebuilt after the PID provenance fix; host/startup/runtime
  evidence remains unrun or bound to older artifacts.

## Disposition

`accepted-with-limits`: the provenance split is implemented and the required
static/package gates pass. Runtime wrapper coverage and future
security/release gates remain open, so D6.6/D6.7 stay `in-progress`.
