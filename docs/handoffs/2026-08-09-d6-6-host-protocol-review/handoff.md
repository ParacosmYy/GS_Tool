# Handoff: 2026-08-09-d6-6-host-protocol-review

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-6-host-protocol-review` |
| Delivery / slice | `D6 / D6.6 process-isolated plugin host protocol review` |
| Status | `in-progress` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:31:08+08:00` |

## User outcome

The diagnostic plugin-host protocol and its PID provenance now have a complete
main-slice handoff record. The host remains a bounded, probe-only failure-
diagnostic path and cannot load or execute catalog code.

## Scope and boundaries

### In scope

- Versioned JSONL codec, bounded frames, typed failure mapping, and TaskRunner
  projection as documented by the parent review.
- Explicit launcher-PID versus reported-host-PID provenance from the D6.6
  follow-up.
- Main-slice handoff traceability.

### Out of scope

- Dynamic loading, signatures, code identity, Windows sandboxing, restricted
  tokens, installation, updates, or external execution.
- Fresh process/Qt/package runtime verification, visual review, unit tests,
  mocks, fixtures, harnesses, and test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate protocol/PID evidence and retain limits |
| Project Manager | Parent role record | Cross-process dependency and release risk |
| Product | Parent role record | Diagnostic host outcome and no-execution boundary |
| Developer 1 | Parent role record | Protocol/application contracts |
| Developer 2 | Parent role record | Infrastructure host and presentation projection |
| QA | Parent record; current Terra review unavailable | Independent high-risk review status, with no PASS claimed |

## Changed files and modules

- `docs/handoffs/2026-08-09-d6-6-host-protocol-review/handoff.md` — add the
  missing main D6.6 handoff.
- `docs/handoffs/index.json`, `docs/agent-team/acceptance.json`, and
  `docs/agent-team/delivery-register.json` — link D6.6 traceability.

## Decisions and constraints

- The protocol is a crash/failure-isolation and diagnostics seam, not a trust
  or sandbox boundary.
- `launcher_pid` is parent-observed process provenance; `reported_host_pid` is
  populated only from the decoded hello frame. PID values are not authorization.
- Shared-checkout writer: Architect only; child reviewers are read-only.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Historical D6.6 protocol source smoke | `RECORDED` | Codec bounds, duplicate-key rejection, distinct PID, disabled execution, crash/timeout/malformed mapping were previously reported. |
| Historical D6.6 Qt offscreen host diagnostic | `RECORDED` | TaskRunner projection was previously reported. |
| `D6.6 PID provenance Luna/Terra review` | `RECORDED` | Terra recommended the two-field repair; parent implemented it and recorded the review. |
| Current high-risk Terra review | `NO RESULT` | Three bounded waits produced no conclusion; agent was closed and no child PASS is claimed. |
| `scripts/verify_handoff.ps1` | `PENDING` | Run after indexing this handoff. |
| `scripts/check.ps1` | `PENDING` | Required static/format/compile/acceptance gate. |

## Unrun checks and reason

- QuillForge.exe `--plugin-host`, fresh process containment, packaged host
  probe, startup, and visual review — prohibited by the project no-launch rule.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under project policy.
- Fresh Terra protocol/concurrency review — no conclusion returned in the
  bounded runtime window; it remains a closure requirement.

## Known risks and limits

- D6.6 remains `in-progress` pending independent high-risk review and fresh
  permitted runtime evidence.
- The host is diagnostic-only; PID consistency is provenance telemetry, not OS
  identity or signature proof.
- D6.7 containment and D6.8 execution gate remain separate dependencies.

## Acceptance and evidence IDs

- Acceptance: `D6-AC06`, `S19`, `S20`
- Evidence: `src/quillforge/plugins/host_protocol.py`,
  `src/quillforge/plugins/host_process.py`,
  `src/quillforge/application/plugin_host.py`,
  `src/quillforge/infrastructure/plugin_host.py`,
  `docs/agent-team/reviews/D6.6-parent-review.md`,
  `docs/agent-team/reviews/D6.6-pid-provenance-parent-review.md`,
  `docs/handoffs/2026-08-09-d6-6-pid-provenance/handoff.md`, and this handoff.

## Next owner and next action

- Owner: Architect
- Action: preserve the diagnostic-only invariant and obtain an authorized
  high-risk review/runtime evidence before promoting D6.6.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4` / `38,327,064`
- Packaging note: documentation-only handoff; no package input changed.

## Disposition

`in-progress`: the protocol/PID evidence is traceable, but independent
high-risk closure and fresh runtime evidence remain open.
